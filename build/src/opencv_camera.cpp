#include "opencv_camera.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono>

#ifdef __linux__
#include <unistd.h>
#endif

using namespace godot;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static int fourcc_for(CVCamera::PixelFormat f) {
    switch (f) {
        case CVCamera::FORMAT_MJPG: return cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
        case CVCamera::FORMAT_YUYV: return cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V');
        case CVCamera::FORMAT_H264: return cv::VideoWriter::fourcc('H', '2', '6', '4');
        case CVCamera::FORMAT_NV12: return cv::VideoWriter::fourcc('N', 'V', '1', '2');
        case CVCamera::FORMAT_AUTO:
        default:
            return 0;
    }
}

static std::string fourcc_to_string(double v) {
    int fcc = (int)v;
    char s[5] = {
        (char)(fcc & 255),
        (char)((fcc >> 8) & 255),
        (char)((fcc >> 16) & 255),
        (char)((fcc >> 24) & 255),
        0
    };
    return std::string(s);
}

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

void CVCamera::_bind_methods() {
    BIND_ENUM_CONSTANT(FORMAT_AUTO);
    BIND_ENUM_CONSTANT(FORMAT_MJPG);
    BIND_ENUM_CONSTANT(FORMAT_YUYV);
    BIND_ENUM_CONSTANT(FORMAT_H264);
    BIND_ENUM_CONSTANT(FORMAT_NV12);

    ClassDB::bind_method(D_METHOD("set_pixel_format", "format"), &CVCamera::set_pixel_format);
    ClassDB::bind_method(D_METHOD("get_pixel_format"), &CVCamera::get_pixel_format);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "pixel_format", PROPERTY_HINT_ENUM,
            "Auto,MJPG,YUYV,H264,NV12"), "set_pixel_format", "get_pixel_format");

    ClassDB::bind_method(D_METHOD("open", "index"), &CVCamera::open, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("open_file", "path"), &CVCamera::open_file);
    ClassDB::bind_method(D_METHOD("close"), &CVCamera::close);
    ClassDB::bind_method(D_METHOD("is_open"), &CVCamera::is_open);

    ClassDB::bind_method(D_METHOD("read_frame"), &CVCamera::read_frame);
    ClassDB::bind_method(D_METHOD("read_image"), &CVCamera::read_image);
    ClassDB::bind_method(D_METHOD("read_texture"), &CVCamera::read_texture);

    ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &CVCamera::set_resolution);
    ClassDB::bind_method(D_METHOD("get_resolution"), &CVCamera::get_resolution);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "resolution"), "set_resolution", "get_resolution");

    ClassDB::bind_method(D_METHOD("set_fps", "fps"), &CVCamera::set_fps);
    ClassDB::bind_method(D_METHOD("get_fps"), &CVCamera::get_fps);

    ClassDB::bind_method(D_METHOD("get_fourcc"), &CVCamera::get_fourcc);
    ClassDB::bind_method(D_METHOD("get_capture_info"), &CVCamera::get_capture_info);

    ClassDB::bind_method(D_METHOD("get_frame_count"), &CVCamera::get_frame_count);
    ClassDB::bind_method(D_METHOD("get_current_frame"), &CVCamera::get_current_frame);
    ClassDB::bind_method(D_METHOD("set_current_frame", "frame"), &CVCamera::set_current_frame);

    ClassDB::bind_method(D_METHOD("set_property", "prop_id", "value"), &CVCamera::set_property);
    ClassDB::bind_method(D_METHOD("get_property", "prop_id"), &CVCamera::get_property);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

CVCamera::CVCamera() {}

CVCamera::~CVCamera() {
    close();
}

// ---------------------------------------------------------------------------
// Frame format safety net
//
// With CAP_PROP_CONVERT_RGB left at its default (1), backends should always
// deliver 3-channel BGR. Some backend/camera combinations (notably MSMF with
// NV12-only cameras) hand through raw buffers anyway — this converts them
// instead of letting them reach Godot as grayscale.
// ---------------------------------------------------------------------------

cv::Mat CVCamera::ensure_bgr(const cv::Mat &in, int expected_height) {
    if (in.channels() == 3) {
        return in; // normal case, zero cost
    }

    cv::Mat out;
    if (in.channels() == 1) {
        if (expected_height > 0 && in.rows == expected_height * 3 / 2) {
            // Raw NV12/I420 passed through
            cv::cvtColor(in, out, cv::COLOR_YUV2BGR_NV12);
        } else {
            cv::cvtColor(in, out, cv::COLOR_GRAY2BGR);
        }
    } else if (in.channels() == 2) {
        // Raw YUYV passed through
        cv::cvtColor(in, out, cv::COLOR_YUV2BGR_YUY2);
    } else if (in.channels() == 4) {
        cv::cvtColor(in, out, cv::COLOR_BGRA2BGR);
    }
    // anything else: out stays empty, caller skips the frame
    return out;
}

// ---------------------------------------------------------------------------
// Capture thread
//
// Threading contract: while thread_running is true, this thread is the SOLE
// owner of `cap`. cap.read() blocks for up to a full frame interval, so it
// must never run under a mutex the main thread might want — that starves the
// main thread (seconds-long stalls; std::mutex has no fairness guarantee).
// Main-thread communication happens only via:
//   - pending_props (in):  property changes, applied between reads
//   - latest_frame  (out): most recent BGR frame, swapped under frame_mutex
// ---------------------------------------------------------------------------

void CVCamera::capture_loop() {
    cv::Mat frame;
    bool warned = false;

    while (thread_running.load()) {
        // Apply property changes requested from the main thread.
        {
            std::lock_guard<std::mutex> lock(prop_mutex);
            for (const auto &p : pending_props) {
                cap.set(p.first, p.second);
            }
            pending_props.clear();
        }

        // Blocking read — intentionally NOT under any lock.
        bool ok = cap.read(frame);

        if (!ok || frame.empty()) {
            // Don't spin at 100% CPU if the device wedges or disconnects.
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        cv::Mat bgr = ensure_bgr(frame, resolution.y);
        if (bgr.empty()) {
            if (!warned) {
                UtilityFunctions::push_warning(
                        "CVCamera: unhandled frame format (channels=", frame.channels(),
                        ", rows=", frame.rows, ")");
                warned = true;
            }
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            latest_frame = bgr.clone();
        }
        new_frame_available.store(true);
    }
}

// ---------------------------------------------------------------------------
// Open / close
// ---------------------------------------------------------------------------

bool CVCamera::open(int index) {
    close();

#ifdef __linux__
    usleep(100000); // let V4L2 settle after a previous release()
#endif

#ifdef _WIN32
    cap.open(index, cv::CAP_DSHOW);
#else
    cap.open(index, cv::CAP_V4L2);
#endif

    if (!cap.isOpened()) {
        UtilityFunctions::push_warning("CVCamera: Failed to open camera ", index);
        return false;
    }

    // Order matters on some backends: FOURCC first, then resolution.
    int fcc = fourcc_for(pixel_format);
    if (fcc != 0) {
        if (!cap.set(cv::CAP_PROP_FOURCC, fcc)) {
            UtilityFunctions::print("CVCamera: backend rejected requested pixel format");
        }
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, resolution.x);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.y);

    // Snapshot everything the main thread may ask about later. This happens
    // BEFORE the capture thread starts, so no locking is needed — afterwards
    // these are read-only and cap belongs to the thread.
    negotiated_fourcc = fourcc_to_string(cap.get(cv::CAP_PROP_FOURCC));
    resolution.x = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    resolution.y = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    info.backend = cap.getBackendName();
    info.fourcc = negotiated_fourcc;
    info.fps = cap.get(cv::CAP_PROP_FPS);
    info.convert_rgb = cap.get(cv::CAP_PROP_CONVERT_RGB);

    camera_index = index;
    m_opened.store(true);
    use_thread = true;

    {
        std::lock_guard<std::mutex> lock(prop_mutex);
        pending_props.clear();
    }
    new_frame_available.store(false);

    thread_running.store(true);
    capture_thread = std::thread(&CVCamera::capture_loop, this);

    UtilityFunctions::print("CVCamera: Opened camera ", index,
            " backend=", String(info.backend.c_str()),
            " fourcc=", String(info.fourcc.c_str()),
            " ", resolution.x, "x", resolution.y, " @", info.fps, "fps");
    return true;
}

bool CVCamera::open_file(const String &path) {
    close();

    std::string std_path(path.utf8().get_data());
    if (!cap.open(std_path)) {
        UtilityFunctions::push_warning("CVCamera: Failed to open file ", path);
        return false;
    }

    m_opened.store(true);
    use_thread = false; // files are read sequentially on the calling thread
    negotiated_fourcc = fourcc_to_string(cap.get(cv::CAP_PROP_FOURCC));
    info.backend = cap.getBackendName();
    info.fourcc = negotiated_fourcc;
    info.fps = cap.get(cv::CAP_PROP_FPS);
    info.convert_rgb = cap.get(cv::CAP_PROP_CONVERT_RGB);
    UtilityFunctions::print("CVCamera: Opened file ", path);
    return true;
}

void CVCamera::close() {
    thread_running.store(false);
    if (capture_thread.joinable()) {
        capture_thread.join();
    }

    if (cap.isOpened()) {
        cap.release();
    }
    m_opened.store(false);
    new_frame_available.store(false);
    negotiated_fourcc.clear();
    camera_index = -1;
}

bool CVCamera::is_open() const {
    return m_opened.load();
}

// ---------------------------------------------------------------------------
// Frame access
// ---------------------------------------------------------------------------

Ref<CVImage> CVCamera::read_frame() {
    if (!is_open()) return Ref<CVImage>();

    if (use_thread) {
        // Non-blocking: latest frame, or null if nothing new since last read.
        if (!new_frame_available.load()) {
            return Ref<CVImage>();
        }
        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            frame = latest_frame.clone();
        }
        new_frame_available.store(false);

        if (frame.empty()) return Ref<CVImage>();
        return CVImage::_from_mat(frame);
    } else {
        // Video file: sequential read on the calling thread.
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) return Ref<CVImage>();

        cv::Mat bgr = ensure_bgr(frame, frame.rows);
        if (bgr.empty()) return Ref<CVImage>();
        return CVImage::_from_mat(bgr);
    }
}

Ref<Image> CVCamera::read_image() {
    Ref<CVImage> frame = read_frame();
    if (frame.is_null()) return Ref<Image>();
    return frame->to_image();
}

Ref<ImageTexture> CVCamera::read_texture() {
    Ref<CVImage> frame = read_frame();
    if (frame.is_null()) return Ref<ImageTexture>();
    return frame->to_image_texture();
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

void CVCamera::set_pixel_format(PixelFormat p_format) {
    pixel_format = p_format;
    // Takes effect on the next open(); changing FOURCC on a live capture
    // is unreliable on most backends.
}

CVCamera::PixelFormat CVCamera::get_pixel_format() const {
    return pixel_format;
}

void CVCamera::set_resolution(const Vector2i &res) {
    resolution = res;
    if (!is_open()) {
        return; // applied on next open()
    }
    if (use_thread) {
        UtilityFunctions::push_warning(
                "CVCamera: changing resolution on a live camera requires close() + open()");
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, res.x);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, res.y);
}

Vector2i CVCamera::get_resolution() const {
    return resolution;
}

void CVCamera::set_fps(double fps) {
    if (!is_open()) return;
    set_property(cv::CAP_PROP_FPS, fps); // routed via queue on live cameras
}

double CVCamera::get_fps() const {
    return is_open() ? info.fps : 0.0;
}

// ---------------------------------------------------------------------------
// Introspection (served from the open()-time snapshot — never touches cap)
// ---------------------------------------------------------------------------

String CVCamera::get_fourcc() const {
    return String(negotiated_fourcc.c_str());
}

Dictionary CVCamera::get_capture_info() const {
    Dictionary d;
    if (!is_open()) return d;
    d["backend"] = String(info.backend.c_str());
    d["fourcc"] = String(info.fourcc.c_str());
    d["width"] = resolution.x;
    d["height"] = resolution.y;
    d["fps"] = info.fps;
    d["convert_rgb"] = info.convert_rgb;
    return d;
}

// ---------------------------------------------------------------------------
// Video file controls / raw properties
// ---------------------------------------------------------------------------

int CVCamera::get_frame_count() const {
    if (!is_open() || use_thread) return 0; // live cameras have no frame count
    return (int)cap.get(cv::CAP_PROP_FRAME_COUNT);
}

int CVCamera::get_current_frame() const {
    if (!is_open() || use_thread) return 0;
    return (int)cap.get(cv::CAP_PROP_POS_FRAMES);
}

void CVCamera::set_current_frame(int frame) {
    if (!is_open() || use_thread) return;
    cap.set(cv::CAP_PROP_POS_FRAMES, frame);
}

void CVCamera::set_property(int prop_id, double value) {
    if (!is_open()) return;
    if (use_thread) {
        // Live camera: cap belongs to the capture thread. Queue the change;
        // it's applied before the next read.
        std::lock_guard<std::mutex> lock(prop_mutex);
        pending_props.emplace_back(prop_id, value);
    } else {
        cap.set(prop_id, value);
    }
}

double CVCamera::get_property(int prop_id) const {
    if (!is_open()) return 0;
    if (use_thread) {
        // Querying cap from here would reintroduce the contention this design
        // removes. Serve what we know from the open()-time snapshot.
        switch (prop_id) {
            case cv::CAP_PROP_FPS: return info.fps;
            case cv::CAP_PROP_FRAME_WIDTH: return resolution.x;
            case cv::CAP_PROP_FRAME_HEIGHT: return resolution.y;
            case cv::CAP_PROP_FOURCC: return (double)cv::VideoWriter::fourcc(
                    negotiated_fourcc.size() > 0 ? negotiated_fourcc[0] : ' ',
                    negotiated_fourcc.size() > 1 ? negotiated_fourcc[1] : ' ',
                    negotiated_fourcc.size() > 2 ? negotiated_fourcc[2] : ' ',
                    negotiated_fourcc.size() > 3 ? negotiated_fourcc[3] : ' ');
            default:
                UtilityFunctions::push_warning(
                        "CVCamera: get_property(", prop_id,
                        ") on a live camera only supports cached values");
                return 0;
        }
    }
    return cap.get(prop_id);
}
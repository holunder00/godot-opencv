#include "opencv_camera.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <opencv2/imgproc.hpp>

#ifdef __linux__
#include <unistd.h>
#endif

using namespace godot;

void CVCamera::_bind_methods() {
    ClassDB::bind_method(D_METHOD("open", "index"), &CVCamera::open, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("open_file", "path"), &CVCamera::open_file);
    ClassDB::bind_method(D_METHOD("close"), &CVCamera::close);
    ClassDB::bind_method(D_METHOD("is_open"), &CVCamera::is_open);
    ClassDB::bind_method(D_METHOD("read_frame"), &CVCamera::read_frame);
    ClassDB::bind_method(D_METHOD("read_image"), &CVCamera::read_image);
    ClassDB::bind_method(D_METHOD("read_texture"), &CVCamera::read_texture);
    ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &CVCamera::set_resolution);
    ClassDB::bind_method(D_METHOD("get_resolution"), &CVCamera::get_resolution);
    ClassDB::bind_method(D_METHOD("get_fps"), &CVCamera::get_fps);
    ClassDB::bind_method(D_METHOD("set_fps", "fps"), &CVCamera::set_fps);
    ClassDB::bind_method(D_METHOD("get_frame_count"), &CVCamera::get_frame_count);
    ClassDB::bind_method(D_METHOD("get_current_frame"), &CVCamera::get_current_frame);
    ClassDB::bind_method(D_METHOD("set_current_frame", "frame"), &CVCamera::set_current_frame);
    ClassDB::bind_method(D_METHOD("set_property", "prop_id", "value"), &CVCamera::set_property);
    ClassDB::bind_method(D_METHOD("get_property", "prop_id"), &CVCamera::get_property);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "resolution"), "set_resolution", "get_resolution");
}

CVCamera::CVCamera() {}

CVCamera::~CVCamera() {
    close();
}

void CVCamera::capture_loop() {
    cv::Mat frame;
    while (thread_running.load()) {
        if (cap.read(frame) && !frame.empty()) {
            std::lock_guard<std::mutex> lock(frame_mutex);
            latest_frame = frame.clone();
            new_frame_available.store(true);
        }
    }
}

bool CVCamera::open(int index) {
    close();

    #ifdef __linux__
    usleep(100000);
    #endif

    cap.set(cv::CAP_PROP_FRAME_WIDTH, resolution.x);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.y);
    bool success = cap.open(index, cv::CAP_V4L2);
    if (!success) {
        success = cap.open(index, cv::CAP_ANY);
    }

    if (!success) {
        UtilityFunctions::print("CVCamera: Failed to open camera ", index);
        return false;
    }

    
    camera_index = index;
    m_opened.store(true);
    use_thread = true;

    // Start capture thread
    thread_running.store(true);
    capture_thread = std::thread(&CVCamera::capture_loop, this);

    UtilityFunctions::print("CVCamera: Opened camera ", index, " with threaded capture");
    return true;
}

bool CVCamera::open_file(const String &path) {
    close();
    std::string std_path = std::string(path.utf8().get_data());
    bool success = cap.open(std_path);
    if (success) {
        m_opened.store(true);
        use_thread = false;  // Files don't need threading
        UtilityFunctions::print("CVCamera: Opened file ", path);
    }
    return success;
}

void CVCamera::close() {
    // Stop thread first
    thread_running.store(false);
    if (capture_thread.joinable()) {
        capture_thread.join();
    }

    if (cap.isOpened()) {
        cap.release();
    }
    m_opened.store(false);
    new_frame_available.store(false);
}

bool CVCamera::is_open() const {
    return m_opened.load() && cap.isOpened();
}

Ref<CVImage> CVCamera::read_frame() {
    if (!is_open()) return Ref<CVImage>();

    if (use_thread) {
        // Threaded path: return latest frame without blocking
        if (!new_frame_available.load()) {
            return Ref<CVImage>();
        }

        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            frame = latest_frame.clone();
            new_frame_available.store(false);
        }

        if (frame.empty()) return Ref<CVImage>();
        return CVImage::_from_mat(frame);
    } else {
        // Non-threaded path: for video files, read sequentially
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) return Ref<CVImage>();
        return CVImage::_from_mat(frame);
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

void CVCamera::set_resolution(const Vector2i &res) {
    resolution = res;
    if (is_open()) {
        cap.set(cv::CAP_PROP_FRAME_WIDTH, res.x);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, res.y);
    }
}

Vector2i CVCamera::get_resolution() const { return resolution; }
double CVCamera::get_fps() const { return is_open() ? cap.get(cv::CAP_PROP_FPS) : 0; }
void CVCamera::set_fps(double fps) { if (is_open()) cap.set(cv::CAP_PROP_FPS, fps); }
int CVCamera::get_frame_count() const { return is_open() ? (int)cap.get(cv::CAP_PROP_FRAME_COUNT) : 0; }
int CVCamera::get_current_frame() const { return is_open() ? (int)cap.get(cv::CAP_PROP_POS_FRAMES) : 0; }
void CVCamera::set_current_frame(int frame) { if (is_open()) cap.set(cv::CAP_PROP_POS_FRAMES, frame); }
void CVCamera::set_property(int prop_id, double value) { if (is_open()) cap.set(prop_id, value); }
double CVCamera::get_property(int prop_id) const { return is_open() ? cap.get(prop_id) : 0; }
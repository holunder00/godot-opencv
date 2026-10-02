#ifndef CV_CAMERA_H
#define CV_CAMERA_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <opencv2/videoio.hpp>

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "opencv_image.h"

namespace godot {

class CVCamera : public RefCounted {
    GDCLASS(CVCamera, RefCounted)

public:
    enum PixelFormat {
        FORMAT_AUTO = 0,
        FORMAT_MJPG,
        FORMAT_YUYV,
        FORMAT_H264,
        FORMAT_NV12,
    };

private:
    cv::VideoCapture cap;
    int camera_index = -1;
    PixelFormat pixel_format = FORMAT_AUTO;
    Vector2i resolution = Vector2i(1280, 720);
    std::string negotiated_fourcc;

    std::atomic<bool> m_opened{false};
    bool use_thread = false;

    // --- capture thread ---------------------------------------------------
    // While thread_running is true, the capture thread is the SOLE owner of
    // `cap`. The main thread must never touch `cap` in that state; it talks
    // to the thread via latest_frame (out) and pending_props (in).
    std::thread capture_thread;
    std::atomic<bool> thread_running{false};

    cv::Mat latest_frame;               // guarded by frame_mutex
    std::mutex frame_mutex;
    std::atomic<bool> new_frame_available{false};

    std::vector<std::pair<int, double>> pending_props; // guarded by prop_mutex
    std::mutex prop_mutex;

    // Snapshot of capture properties, filled in open() BEFORE the thread
    // starts; read-only afterwards, therefore safe without locking.
    struct CaptureInfo {
        std::string backend;
        std::string fourcc;
        double fps = 0.0;
        double convert_rgb = 1.0;
    } info;

    void capture_loop();
    static cv::Mat ensure_bgr(const cv::Mat &in, int expected_height);

protected:
    static void _bind_methods();

public:
    CVCamera();
    ~CVCamera();

    bool open(int index = 0);
    bool open_file(const String &path);
    void close();
    bool is_open() const;

    Ref<CVImage> read_frame();
    Ref<Image> read_image();
    Ref<ImageTexture> read_texture();

    void set_pixel_format(PixelFormat p_format);
    PixelFormat get_pixel_format() const;

    void set_resolution(const Vector2i &res);
    Vector2i get_resolution() const;

    void set_fps(double fps);
    double get_fps() const;

    String get_fourcc() const;
    Dictionary get_capture_info() const;

    // Video file controls (no effect on live cameras).
    int get_frame_count() const;
    int get_current_frame() const;
    void set_current_frame(int frame);

    // Raw cv::VideoCapture properties.
    void set_property(int prop_id, double value);
    double get_property(int prop_id) const;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::CVCamera::PixelFormat);

#endif // CV_CAMERA_H
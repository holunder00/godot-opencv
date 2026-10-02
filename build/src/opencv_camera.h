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

#include "opencv_image.h" // CVImage

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

    // Configuration (applied on open)
    PixelFormat pixel_format = FORMAT_AUTO;
    Vector2i resolution = Vector2i(640, 480);

    // Negotiated state
    std::string negotiated_fourcc;

    // Threading
    std::thread capture_thread;
    std::atomic<bool> thread_running{false};
    std::atomic<bool> m_opened{false};
    std::atomic<bool> new_frame_available{false};
    bool use_thread = false;

    cv::Mat latest_frame;
    std::mutex frame_mutex;          // guards latest_frame
    mutable std::mutex cap_mutex;    // guards cap (VideoCapture is not thread-safe)

    void capture_loop();
    static cv::Mat ensure_bgr(const cv::Mat &in, int expected_height);

protected:
    static void _bind_methods();

public:
    CVCamera();
    ~CVCamera();

    bool open(int index);
    bool open_file(const String &path);
    void close();
    bool is_open() const;

    Ref<CVImage> read_frame();
    Ref<Image> read_image();
    Ref<ImageTexture> read_texture();

    // Configuration
    void set_pixel_format(PixelFormat p_format);
    PixelFormat get_pixel_format() const;
    void set_resolution(const Vector2i &res);
    Vector2i get_resolution() const;
    void set_fps(double fps);
    double get_fps() const;

    // Introspection
    String get_fourcc() const;
    Dictionary get_capture_info() const;

    // Video file controls
    int get_frame_count() const;
    int get_current_frame() const;
    void set_current_frame(int frame);

    // Raw property access
    void set_property(int prop_id, double value);
    double get_property(int prop_id) const;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::CVCamera::PixelFormat);

#endif // CV_CAMERA_H
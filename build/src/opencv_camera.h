#ifndef OPENCV_CAMERA_H
#define OPENCV_CAMERA_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <thread>
#include <mutex>
#include <atomic>

#include "opencv_image.h"

namespace godot {

class CVCamera : public Node {
    GDCLASS(CVCamera, Node)

private:
    cv::VideoCapture cap;
    cv::Mat latest_frame;
    std::mutex frame_mutex;
    std::thread capture_thread;
    std::atomic<bool> m_opened{false};
    std::atomic<bool> thread_running{false};
    std::atomic<bool> new_frame_available{false};
    bool use_thread = true;  // threaded for cameras, not for files
    int camera_index = 0;
    Vector2i resolution = Vector2i(640, 480);

    void capture_loop();

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

    void set_resolution(const Vector2i &res);
    Vector2i get_resolution() const;
    double get_fps() const;
    void set_fps(double fps);
    int get_frame_count() const;
    int get_current_frame() const;
    void set_current_frame(int frame);
    void set_property(int prop_id, double value);
    double get_property(int prop_id) const;
};

}

#endif
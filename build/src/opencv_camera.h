#ifndef CV_CAMERA_H
#define CV_CAMERA_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <opencv2/videoio.hpp>
#include "opencv_image.h"

namespace godot {

class CVCamera : public Node {
    GDCLASS(CVCamera, Node);

private:
    cv::VideoCapture cap;
    int camera_index = 0;
    bool is_opened = false;
    Vector2i resolution = Vector2i(640, 480);

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

} // namespace godot

#endif

#ifndef CV_CAMERA_TEXTURE_H
#define CV_CAMERA_TEXTURE_H

#include <godot_cpp/classes/image_texture.hpp>
#include "opencv_camera.h"

namespace godot {

class CVCameraTexture : public ImageTexture {
    GDCLASS(CVCameraTexture, ImageTexture)

    Ref<CVCamera> camera;
    int camera_index = 0;
    CVCamera::PixelFormat pixel_format = CVCamera::FORMAT_MJPG;
    Vector2i camera_resolution = Vector2i(1280, 720);
    bool active = false;
    bool connected = false;
    Vector2i current_size = Vector2i(0, 0);

    void _update_frame();
    void _start();
    void _stop();

protected:
    static void _bind_methods();

public:
    CVCameraTexture();
    ~CVCameraTexture();

    void set_active(bool p_active);
    bool get_active() const;
    void set_camera_index(int p_index);
    int get_camera_index() const;
    void set_pixel_format(CVCamera::PixelFormat p_format);
    CVCamera::PixelFormat get_pixel_format() const;
    void set_camera_resolution(const Vector2i &p_res);
    Vector2i get_camera_resolution() const;

    Ref<CVCamera> get_camera() const; // escape hatch for advanced use
};

} // namespace godot

#endif
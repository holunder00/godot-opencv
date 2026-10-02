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
    bool editor_preview = true;
    bool connected = false;
    Vector2i current_size = Vector2i(0, 0);

    // Retry-on-open (handles "device busy" during editor→game handoff,
    // or camera held briefly by another app).
    bool open_pending = false;
    uint64_t retry_until_msec = 0;
    uint64_t last_attempt_msec = 0;

    bool suspended_by_editor = false;

    // Registry of all live instances, for the editor plugin's handoff.
    static std::mutex registry_mutex;
    static std::vector<CVCameraTexture *> registry;

    void _update_frame();
    void _start();
    void _stop();
    void _connect_to_main_loop();
    bool _try_open();

protected:
    static void _bind_methods();

public:
    CVCameraTexture();
    ~CVCameraTexture();

    static void editor_suspend_all();
    static void editor_resume_all();

    void _validate_property(PropertyInfo &p_property) const;
    void set_active(bool p_active);
    bool get_active() const;
    void set_editor_preview(bool p_enabled);
    bool get_editor_preview() const;
    bool _should_run() const;
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
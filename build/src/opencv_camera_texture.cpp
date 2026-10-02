#include "opencv_camera_texture.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void CVCameraTexture::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_active", "active"), &CVCameraTexture::set_active);
    ClassDB::bind_method(D_METHOD("get_active"), &CVCameraTexture::get_active);
    ClassDB::bind_method(D_METHOD("set_camera_index", "index"), &CVCameraTexture::set_camera_index);
    ClassDB::bind_method(D_METHOD("get_camera_index"), &CVCameraTexture::get_camera_index);
    ClassDB::bind_method(D_METHOD("set_pixel_format", "format"), &CVCameraTexture::set_pixel_format);
    ClassDB::bind_method(D_METHOD("get_pixel_format"), &CVCameraTexture::get_pixel_format);
    ClassDB::bind_method(D_METHOD("set_camera_resolution", "resolution"), &CVCameraTexture::set_camera_resolution);
    ClassDB::bind_method(D_METHOD("get_camera_resolution"), &CVCameraTexture::get_camera_resolution);
    ClassDB::bind_method(D_METHOD("get_camera"), &CVCameraTexture::get_camera);
    ClassDB::bind_method(D_METHOD("_update_frame"), &CVCameraTexture::_update_frame);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "get_active");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "camera_index"), "set_camera_index", "get_camera_index");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "pixel_format", PROPERTY_HINT_ENUM,
            "Auto,MJPG,YUYV,H264,NV12"), "set_pixel_format", "get_pixel_format");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "camera_resolution"),
            "set_camera_resolution", "get_camera_resolution");
}

CVCameraTexture::CVCameraTexture() {
    camera.instantiate();
}

CVCameraTexture::~CVCameraTexture() {
    _stop();
}

void CVCameraTexture::_start() {
    camera->set_pixel_format(pixel_format);
    camera->set_resolution(camera_resolution);
    if (!camera->open(camera_index)) {
        return;
    }
    if (!connected) {
        RenderingServer::get_singleton()->connect("frame_pre_draw",
                Callable(this, "_update_frame"));
        connected = true;
    }
}

void CVCameraTexture::_stop() {
   RenderingServer *rs = RenderingServer::get_singleton();
    if (rs && rs->is_connected("frame_pre_draw", Callable(this, "_update_frame"))) {
        rs->disconnect("frame_pre_draw", Callable(this, "_update_frame"));
    }
    connected = false;
    if (camera.is_valid()) camera->close();
}

void CVCameraTexture::_update_frame() {
    Ref<Image> img = camera->read_image();
    if (img.is_null()) {
        return; // no new frame this tick
    }
    Vector2i size(img->get_width(), img->get_height());
    if (size != current_size) {
        set_image(img);           // (re)allocates GPU texture
        current_size = size;
    } else {
        update(img);              // fast in-place GPU upload
    }
	 emit_changed();
}

void CVCameraTexture::set_active(bool p_active) {
    if (active == p_active) return;
    active = p_active;
    if (active) _start(); else _stop();
}

bool CVCameraTexture::get_active() const { return active; }

void CVCameraTexture::set_camera_index(int p_index) { camera_index = p_index; }
int CVCameraTexture::get_camera_index() const { return camera_index; }
void CVCameraTexture::set_pixel_format(CVCamera::PixelFormat p_format) { pixel_format = p_format; }
CVCamera::PixelFormat CVCameraTexture::get_pixel_format() const { return pixel_format; }
void CVCameraTexture::set_camera_resolution(const Vector2i &p_res) { camera_resolution = p_res; }
Vector2i CVCameraTexture::get_camera_resolution() const { return camera_resolution; }
Ref<CVCamera> CVCameraTexture::get_camera() const { return camera; }
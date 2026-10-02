#include "opencv_camera_texture.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp> 
#include <godot_cpp/classes/time.hpp>

using namespace godot;

std::mutex CVCameraTexture::registry_mutex;
std::vector<CVCameraTexture *> CVCameraTexture::registry;


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
    ClassDB::bind_method(D_METHOD("_connect_to_main_loop"), &CVCameraTexture::_connect_to_main_loop);
    ClassDB::bind_method(D_METHOD("set_editor_preview", "enabled"), &CVCameraTexture::set_editor_preview);
    ClassDB::bind_method(D_METHOD("get_editor_preview"), &CVCameraTexture::get_editor_preview);
    
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editor_preview"), "set_editor_preview", "get_editor_preview");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "get_active");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "camera_index"), "set_camera_index", "get_camera_index");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "pixel_format", PROPERTY_HINT_ENUM,
            "Auto,MJPG,YUYV,H264,NV12"), "set_pixel_format", "get_pixel_format");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "camera_resolution"),
            "set_camera_resolution", "get_camera_resolution");
}

CVCameraTexture::CVCameraTexture() {
    camera.instantiate();
    std::lock_guard<std::mutex> lock(registry_mutex);
    registry.push_back(this);
}

CVCameraTexture::~CVCameraTexture() {
    _stop();
    std::lock_guard<std::mutex> lock(registry_mutex);
    registry.erase(std::remove(registry.begin(), registry.end(), this), registry.end());
}

bool CVCameraTexture::_should_run() const {
    if (!active) return false;
    if (Engine::get_singleton()->is_editor_hint() && !editor_preview) return false;
    return true;
}

void CVCameraTexture::editor_suspend_all() {
    std::lock_guard<std::mutex> lock(registry_mutex);
    for (CVCameraTexture *t : registry) {
        if (t->camera.is_valid() && t->camera->is_open()) {
            t->suspended_by_editor = true;
            t->_stop();
        }
    }
}

void CVCameraTexture::editor_resume_all() {
    std::lock_guard<std::mutex> lock(registry_mutex);
    for (CVCameraTexture *t : registry) {
        if (t->suspended_by_editor) {
            t->suspended_by_editor = false;
            if (t->_should_run()) {
                t->_start();
            }
        }
    }
}

void CVCameraTexture::_start() {
    // Don't open here — connect to the main loop and let _update_frame
    // attempt it, with retries. Covers the editor→game handoff window
    // where the device is still being released by the other process.
    open_pending = true;
    retry_until_msec = Time::get_singleton()->get_ticks_msec() + 3000; // 3 s window
    last_attempt_msec = 0;

    if (!connected) {
        SceneTree *tree = Object::cast_to<SceneTree>(
                Engine::get_singleton()->get_main_loop());
        if (tree == nullptr) {
            call_deferred("_connect_to_main_loop");
            return;
        }
        tree->connect("process_frame", Callable(this, "_update_frame"));
        connected = true;
    }
}

void CVCameraTexture::_connect_to_main_loop() {
    if (connected || !active) {
        return;
    }
    SceneTree *tree = Object::cast_to<SceneTree>(
            Engine::get_singleton()->get_main_loop());
    if (tree == nullptr) {
        return;
    }
    tree->connect("process_frame", Callable(this, "_update_frame"));
    connected = true;
}

void CVCameraTexture::_stop() {
    if (connected) {
        SceneTree *tree = Object::cast_to<SceneTree>(
                Engine::get_singleton()->get_main_loop());
        if (tree && tree->is_connected("process_frame", Callable(this, "_update_frame"))) {
            tree->disconnect("process_frame", Callable(this, "_update_frame"));
        }
        connected = false;
    }
    if (camera.is_valid()) {
        camera->close();
    }
}

void CVCameraTexture::_update_frame() {
    if (!camera->is_open()) {
        if (!open_pending) return;
        uint64_t now = Time::get_singleton()->get_ticks_msec();
        if (now - last_attempt_msec < 300) return;   // throttle attempts
        last_attempt_msec = now;
        if (_try_open()) {
            open_pending = false;
        } else if (now >= retry_until_msec) {
            open_pending = false;
            UtilityFunctions::push_warning(
                    "CVCameraTexture: could not open camera ", camera_index,
                    " (device busy or missing)");
        }
        return;
    }

    Ref<Image> img = camera->read_image();
    if (img.is_null()) return;

    Vector2i size(img->get_width(), img->get_height());
    if (size != current_size) {
        set_image(img);
        current_size = size;
    } else {
        RenderingServer::get_singleton()->texture_2d_update(get_rid(), img, 0);
    }
}

void CVCameraTexture::set_editor_preview(bool p_enabled) {
    if (editor_preview == p_enabled) return;
    editor_preview = p_enabled;
    // Only affects behavior inside the editor; in game it's a no-op.
    if (!Engine::get_singleton()->is_editor_hint()) return;
    if (_should_run()) _start(); else _stop();
}

bool CVCameraTexture::get_editor_preview() const {
    return editor_preview;
}

void CVCameraTexture::set_active(bool p_active) {
    if (active == p_active) return;
    active = p_active;
    if (active) _start(); else _stop();
}

bool CVCameraTexture::_try_open() {
    camera->set_pixel_format(pixel_format);
    camera->set_resolution(camera_resolution);
    return camera->open(camera_index);
}

void CVCameraTexture::_validate_property(PropertyInfo &p_property) const {
    // Don't serialize the current camera frame into the scene file —
    // it's transient data, regenerated the moment the camera starts.
    if (p_property.name == StringName("image")) {
        p_property.usage = PROPERTY_USAGE_NONE;
    }
}

bool CVCameraTexture::get_active() const { return active; }

void CVCameraTexture::set_camera_index(int p_index) { camera_index = p_index; }
int CVCameraTexture::get_camera_index() const { return camera_index; }
void CVCameraTexture::set_pixel_format(CVCamera::PixelFormat p_format) { pixel_format = p_format; }
CVCamera::PixelFormat CVCameraTexture::get_pixel_format() const { return pixel_format; }
void CVCameraTexture::set_camera_resolution(const Vector2i &p_res) { camera_resolution = p_res; }
Vector2i CVCameraTexture::get_camera_resolution() const { return camera_resolution; }
Ref<CVCamera> CVCameraTexture::get_camera() const { return camera; }
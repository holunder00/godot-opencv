#include "cv_camera_editor_plugin.h"
#include "opencv_camera_texture.h"

#include <godot_cpp/classes/editor_interface.hpp>

using namespace godot;

void CVCameraEditorPlugin::_enter_tree() {
    //UtilityFunctions::print("CVCameraEditorPlugin: alive");
    set_process(true);
}

void CVCameraEditorPlugin::_process(double) {
    // No "play pressed" signal is exposed to plugins; poll the state instead.
    bool playing = EditorInterface::get_singleton()->is_playing_scene();
    if (playing == was_playing) return;
    was_playing = playing;
    //UtilityFunctions::print("CVCameraEditorPlugin: playing=", playing);

    if (playing) {
        // Game process is starting — release every editor-held camera so the
        // game can open the device. Its retry window covers the gap.
        CVCameraTexture::editor_suspend_all();
    } else {
        // Game stopped (or crashed) — take the cameras back for preview.
        CVCameraTexture::editor_resume_all();
    }
}
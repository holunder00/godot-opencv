#ifndef CV_CAMERA_EDITOR_PLUGIN_H
#define CV_CAMERA_EDITOR_PLUGIN_H

#include <godot_cpp/classes/editor_plugin.hpp>

namespace godot {

class CVCameraEditorPlugin : public EditorPlugin {
    GDCLASS(CVCameraEditorPlugin, EditorPlugin)

    bool was_playing = false;

protected:
    static void _bind_methods() {}

public:
    void _enter_tree() override;
    void _process(double delta) override;
};

} // namespace godot

#endif
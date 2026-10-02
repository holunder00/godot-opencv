#include "register_types.h"

#include "opencv_image.h"
#include "opencv_camera.h"
#include "opencv_camera_texture.h"
#include "opencv_utils.h"
#include "cv_camera_editor_plugin.h"
#include <godot_cpp/classes/editor_plugin_registration.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_opencv_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        ClassDB::register_class<CVImage>();
        ClassDB::register_class<CVCamera>();
        ClassDB::register_class<CVCameraTexture>();
        ClassDB::register_class<CVUtils>();
        //GDREGISTER_CLASS(CVCamera);
        //GDREGISTER_CLASS(CVImage);
        //GDREGISTER_CLASS(CVCameraTexture);
        // ... whatever else you register ...
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        UtilityFunctions::print("CVCamera: registering editor plugin");
        GDREGISTER_INTERNAL_CLASS(CVCameraEditorPlugin);
        EditorPlugins::add_by_type<CVCameraEditorPlugin>();
    }
    
}

void uninitialize_opencv_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT opencv_library_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization
) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_opencv_module);
    init_obj.register_terminator(uninitialize_opencv_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}

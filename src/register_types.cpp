#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include "js_sandbox.h"
#include "js_script.h"
#include "js_script_language.h"
#include "js_resource_loader.h"
#include "gltf_resource_loader.h"
#include "js_runtime_manager.h"
#include "deletion_tracker.h"
#include "async_scene_loader.h"
#include "sandbox_logger.h"

using namespace godot;

static jsb::JSScriptLanguage* script_language = nullptr;
static Ref<jsb::JSResourceLoader> resource_loader;
static Ref<jsb::JSResourceSaver> resource_saver;
static Ref<jsb::GLTFResourceLoader> gltf_resource_loader;

void initialize_godot_js_runtime_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    // Initialize the shared JSRuntimeManager first (TDD Section 14.2)
    jsb::JSRuntimeManager::initialize();

    // Register classes
    ClassDB::register_class<jsb::JSSandbox>();
    ClassDB::register_class<jsb::JSScript>();
    ClassDB::register_class<jsb::JSScriptLanguage>();
    ClassDB::register_class<jsb::JSResourceLoader>();
    ClassDB::register_class<jsb::JSResourceSaver>();
    ClassDB::register_class<jsb::GLTFResourceLoader>();
    ClassDB::register_class<jsb::DeletionCallback>();
    ClassDB::register_class<jsb::AsyncSceneLoader>();
    ClassDB::register_class<jsb::SandboxLogger>();

    // Create and register script language
    script_language = memnew(jsb::JSScriptLanguage);
    Engine::get_singleton()->register_script_language(script_language);

    // Create and register resource loader/saver
    resource_loader.instantiate();
    resource_saver.instantiate();
    gltf_resource_loader.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(resource_loader);
    ResourceSaver::get_singleton()->add_resource_format_saver(resource_saver);
    // Add GLTF loader with high priority to intercept user:// glb/gltf files
    ResourceLoader::get_singleton()->add_resource_format_loader(gltf_resource_loader, true);
}

void uninitialize_godot_js_runtime_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    // Unregister resource loader/saver
    if (gltf_resource_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(gltf_resource_loader);
        gltf_resource_loader.unref();
    }
    if (resource_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(resource_loader);
        resource_loader.unref();
    }
    if (resource_saver.is_valid()) {
        ResourceSaver::get_singleton()->remove_resource_format_saver(resource_saver);
        resource_saver.unref();
    }

    // Unregister script language
    if (script_language) {
        Engine::get_singleton()->unregister_script_language(script_language);
        memdelete(script_language);
        script_language = nullptr;
    }

    // Shutdown JSRuntimeManager last (after all contexts are freed)
    jsb::JSRuntimeManager::shutdown();
}

extern "C" {
    GDExtensionBool GDE_EXPORT godot_js_runtime_library_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address,
        const GDExtensionClassLibraryPtr p_library,
        GDExtensionInitialization *r_initialization
    ) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_godot_js_runtime_module);
        init_obj.register_terminator(uninitialize_godot_js_runtime_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}

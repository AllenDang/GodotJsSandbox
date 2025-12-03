#ifndef GODOT_JS_RUNTIME_SCENE_LOADER_H
#define GODOT_JS_RUNTIME_SCENE_LOADER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

namespace jsb {

class JSSandbox;

// SceneLoader handles loading saved scenes and reattaching JS scripts
// Works with scenes saved by SceneSaver that have _js_script_path metadata
class SceneLoader {
public:
    struct LoadResult {
        godot::Error error = godot::OK;
        godot::Node* root = nullptr;
        godot::PackedStringArray loaded_scripts;
        godot::PackedStringArray warnings;
        godot::String error_message;
    };

    // Load a level from a directory and reattach JS scripts
    // The sandbox is used to create script instances that share its context
    static LoadResult load(JSSandbox* sandbox, const godot::String& directory);

    // Reattach JS scripts to an already-instantiated scene tree
    // Use this when you've loaded a scene manually and want to reattach scripts
    static void reattach_scripts(JSSandbox* sandbox, godot::Node* root,
                                  godot::PackedStringArray& loaded_scripts,
                                  godot::PackedStringArray& warnings);

private:
    // Validate input path (must be user://)
    static bool validate_path(const godot::String& path, godot::String& error);

    // Recursively process nodes to reattach scripts
    static void reattach_scripts_recursive(JSSandbox* sandbox, godot::Node* node,
                                           godot::PackedStringArray& loaded_scripts,
                                           godot::PackedStringArray& warnings);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_SCENE_LOADER_H

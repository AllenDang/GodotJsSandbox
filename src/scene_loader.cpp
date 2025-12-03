#include "scene_loader.h"
#include "js_sandbox.h"
#include "js_script.h"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

bool SceneLoader::validate_path(const String& path, String& error) {
    if (!path.begins_with("user://")) {
        error = "Levels can only be loaded from user:// directory";
        return false;
    }

    if (path.find("..") != -1) {
        error = "Path traversal not allowed";
        return false;
    }

    return true;
}

SceneLoader::LoadResult SceneLoader::load(JSSandbox* sandbox, const String& directory) {
    LoadResult result;

    // Validate path
    if (!validate_path(directory, result.error_message)) {
        result.error = ERR_INVALID_PARAMETER;
        return result;
    }

    if (!sandbox) {
        result.error = ERR_INVALID_PARAMETER;
        result.error_message = "Sandbox is null";
        return result;
    }

    // Construct scene path
    String scene_path = directory;
    if (!scene_path.ends_with("/")) scene_path += "/";
    scene_path += "level.tscn";

    // Check if scene file exists
    if (!FileAccess::file_exists(scene_path)) {
        result.error = ERR_FILE_NOT_FOUND;
        result.error_message = "Scene file not found: " + scene_path;
        return result;
    }

    // Load the PackedScene
    Ref<PackedScene> packed_scene = ResourceLoader::get_singleton()->load(scene_path);
    if (!packed_scene.is_valid()) {
        result.error = ERR_CANT_OPEN;
        result.error_message = "Failed to load scene: " + scene_path;
        return result;
    }

    // Instantiate the scene
    Node* root = packed_scene->instantiate();
    if (!root) {
        result.error = ERR_CANT_CREATE;
        result.error_message = "Failed to instantiate scene";
        return result;
    }

    // Reattach JS scripts
    reattach_scripts(sandbox, root, result.loaded_scripts, result.warnings);

    result.root = root;
    result.error = OK;
    return result;
}

void SceneLoader::reattach_scripts(JSSandbox* sandbox, Node* root,
                                    PackedStringArray& loaded_scripts,
                                    PackedStringArray& warnings) {
    if (!sandbox || !root) return;

    reattach_scripts_recursive(sandbox, root, loaded_scripts, warnings);
}

void SceneLoader::reattach_scripts_recursive(JSSandbox* sandbox, Node* node,
                                              PackedStringArray& loaded_scripts,
                                              PackedStringArray& warnings) {
    if (!node) return;

    // Check if this node has _js_script_path metadata
    if (node->has_meta("_js_script_path")) {
        String script_path = node->get_meta("_js_script_path");

        // Check if script file exists
        if (FileAccess::file_exists(script_path)) {
            // Load the JS source code
            Ref<FileAccess> file = FileAccess::open(script_path, FileAccess::READ);
            if (file.is_valid()) {
                String source = file->get_as_text();
                file->close();

                // Create JSScript using the sandbox (shares context)
                Ref<Script> script = sandbox->create_script(source);
                if (script.is_valid()) {
                    // Set the path on the script
                    JSScript* js_script = Object::cast_to<JSScript>(script.ptr());
                    if (js_script) {
                        js_script->set_path(script_path);
                    }

                    // Attach the script to the node
                    node->set_script(script);
                    loaded_scripts.push_back(script_path);
                } else {
                    warnings.push_back("Failed to create script for node '" +
                                      String(node->get_name()) + "': " + script_path);
                }
            } else {
                warnings.push_back("Failed to read script file: " + script_path);
            }
        } else {
            warnings.push_back("Script file not found: " + script_path);
        }

        // Remove the metadata after processing (optional - keep for debugging)
        // node->remove_meta("_js_script_path");
    }

    // Process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            reattach_scripts_recursive(sandbox, child, loaded_scripts, warnings);
        }
    }
}

} // namespace jsb

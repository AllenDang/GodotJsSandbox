#include "scene_saver.h"
#include "js_script.h"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

bool SceneSaver::validate_path(const String& path, String& error) {
    if (!path.begins_with("user://")) {
        error = "Levels can only be saved to user:// directory";
        return false;
    }

    if (path.find("..") != -1) {
        error = "Path traversal not allowed";
        return false;
    }

    return true;
}

SceneSaver::SaveResult SceneSaver::save(Node* root, const String& directory,
                                         const SaveOptions& options) {
    SaveResult result;

    // Validate path
    if (!validate_path(directory, result.error_message)) {
        result.error = ERR_INVALID_PARAMETER;
        return result;
    }

    if (!root) {
        result.error = ERR_INVALID_PARAMETER;
        result.error_message = "Root node is null";
        return result;
    }

    // Ensure directory exists
    Ref<DirAccess> dir = DirAccess::open("user://");
    if (!dir.is_valid()) {
        result.error = ERR_CANT_OPEN;
        result.error_message = "Cannot access user:// directory";
        return result;
    }

    // Extract relative path from user://
    String relative_path = directory.substr(7);  // Remove "user://"
    if (!relative_path.is_empty() && !dir->dir_exists(relative_path)) {
        Error err = dir->make_dir_recursive(relative_path);
        if (err != OK) {
            result.error = err;
            result.error_message = "Failed to create directory: " + directory;
            return result;
        }
    }

    // Collect JS scripts from the tree
    Dictionary script_map = collect_js_scripts(root, result.warnings);

    // Save each script file
    Array script_names = script_map.keys();
    for (int i = 0; i < script_names.size(); i++) {
        String script_name = script_names[i];
        String source_code = script_map[script_name];

        String script_path = directory;
        if (!script_path.ends_with("/")) script_path += "/";
        script_path += script_name;

        Error err = save_script(source_code, script_path);
        if (err != OK) {
            result.warnings.push_back("Failed to save script: " + script_name);
        } else {
            result.script_paths.push_back(script_path);
        }
    }

    // Save the scene file
    String scene_path = directory;
    if (!scene_path.ends_with("/")) scene_path += "/";
    scene_path += "level.tscn";

    Error scene_err = save_scene(root, scene_path, script_map, result.error_message);
    if (scene_err != OK) {
        result.error = scene_err;
        return result;
    }
    result.scene_path = scene_path;

    // Save metadata
    save_metadata(directory, result.script_paths);

    result.error = OK;
    return result;
}

Dictionary SceneSaver::collect_js_scripts(Node* root, PackedStringArray& warnings) {
    Dictionary scripts;

    // Process root
    process_node_for_save(root, "", scripts, warnings);

    // Process all children recursively
    TypedArray<Node> children = root->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            collect_js_scripts_recursive(child, scripts, warnings);
        }
    }

    return scripts;
}

void SceneSaver::collect_js_scripts_recursive(Node* node, Dictionary& scripts, PackedStringArray& warnings) {
    // Check for JS script
    Ref<Script> script = node->get_script();
    if (script.is_valid()) {
        JSScript* js_script = Object::cast_to<JSScript>(script.ptr());
        if (js_script) {
            String source = js_script->_get_source_code();
            if (!source.is_empty()) {
                // Generate filename from node name
                String filename = node->get_name().to_lower().replace(" ", "_") + ".js";

                // Ensure unique filename
                int counter = 1;
                String base_name = filename.get_basename();
                while (scripts.has(filename)) {
                    filename = base_name + "_" + String::num_int64(counter) + ".js";
                    counter++;
                }

                scripts[filename] = source;

                // Store mapping on the node (for scene saving)
                node->set_meta("_js_script_file", filename);
            }
        } else {
            // Non-JS script - add warning
            warnings.push_back("Node '" + String(node->get_name()) +
                              "' has non-JS script, it will not be saved");
        }
    }

    // Process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            collect_js_scripts_recursive(child, scripts, warnings);
        }
    }
}

void SceneSaver::process_node_for_save(Node* node, const String& directory,
                                        Dictionary& script_map,
                                        PackedStringArray& warnings) {
    if (!node) return;

    Ref<Script> script = node->get_script();
    if (script.is_valid()) {
        JSScript* js_script = Object::cast_to<JSScript>(script.ptr());
        if (js_script) {
            String source = js_script->_get_source_code();
            if (!source.is_empty()) {
                String filename = generate_script_filename(node, script_map);
                script_map[filename] = source;
                node->set_meta("_js_script_file", filename);
            }
        }
    }
}

String SceneSaver::generate_script_filename(Node* node, const Dictionary& existing) {
    String base = node->get_name().to_lower().validate_filename();
    if (base.is_empty()) base = "script";

    String filename = base + ".js";
    int counter = 1;

    while (existing.has(filename)) {
        filename = base + "_" + String::num_int64(counter) + ".js";
        counter++;
    }

    return filename;
}

Error SceneSaver::save_script(const String& source, const String& path) {
    Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE);
    if (!file.is_valid()) {
        return ERR_FILE_CANT_WRITE;
    }

    file->store_string(source);
    file->close();

    return OK;
}

Error SceneSaver::save_scene(Node* root, const String& path,
                              const Dictionary& script_map, String& error) {
    // Get directory from path
    String directory = path.get_base_dir();

    // IMPORTANT: Clear all scripts from the original tree BEFORE duplicating
    // This avoids crashes during duplication/packing with JSScript instances
    // We've already saved the script source code to .js files
    // Store the script paths as metadata first
    store_script_paths_recursive(root, directory);

    // Create a duplicate of the root for saving
    // Use flags that exclude scripts to avoid serialization issues
    // DUPLICATE_GROUPS (4) | DUPLICATE_SIGNALS (1) = 5
    Node* save_root = Object::cast_to<Node>(root->duplicate(5));
    if (!save_root) {
        error = "Failed to duplicate root node for saving";
        return ERR_CANT_CREATE;
    }

    // Copy the script path metadata to the duplicated tree
    copy_script_metadata_recursive(root, save_root);

    // Set owner for all children so they get included in the packed scene
    set_owners_recursive(save_root, save_root);

    // Create PackedScene
    Ref<PackedScene> packed_scene;
    packed_scene.instantiate();
    Error pack_err = packed_scene->pack(save_root);

    // Clean up the duplicate
    memdelete(save_root);

    if (pack_err != OK) {
        error = "Failed to pack scene";
        return pack_err;
    }

    // Save the PackedScene
    Error save_err = ResourceSaver::get_singleton()->save(packed_scene, path);
    if (save_err != OK) {
        error = "Failed to save scene file: " + path;
        return save_err;
    }

    return OK;
}

void SceneSaver::update_script_references(Node* node, const String& directory) {
    if (!node) return;

    // Check if this node has a JS script filename stored
    if (node->has_meta("_js_script_file")) {
        String script_filename = node->get_meta("_js_script_file");
        String script_path = directory;
        if (!script_path.ends_with("/")) script_path += "/";
        script_path += script_filename;

        // Load the JS script from the saved file
        // For now, we'll clear the script - it will be loaded when the scene is loaded
        // The .tscn file will reference the .js file
        node->set_script(Variant());

        // Remove the temporary meta
        node->remove_meta("_js_script_file");
    }

    // Process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            update_script_references(child, directory);
        }
    }
}

void SceneSaver::update_script_paths_recursive(Node* node, const String& directory) {
    if (!node) return;

    // Check if this node has a JS script filename stored (from collect_js_scripts)
    if (node->has_meta("_js_script_file")) {
        String script_filename = node->get_meta("_js_script_file");
        String script_path = directory;
        if (!script_path.ends_with("/")) script_path += "/";
        script_path += script_filename;

        // Store the script path as metadata instead of trying to set a JSScript directly
        // This avoids crashes during PackedScene serialization
        // The metadata will be preserved in the .tscn file and can be used to reload scripts
        node->set_meta("_js_script_path", script_path);

        // Clear the existing script to avoid serialization issues
        node->set_script(Variant());

        // Remove the temporary meta
        node->remove_meta("_js_script_file");
    }

    // Process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            update_script_paths_recursive(child, directory);
        }
    }
}

void SceneSaver::store_script_paths_recursive(Node* node, const String& directory) {
    if (!node) return;

    // If this node has _js_script_file metadata (from collect_js_scripts), convert to path
    if (node->has_meta("_js_script_file")) {
        String script_filename = node->get_meta("_js_script_file");
        String script_path = directory;
        if (!script_path.ends_with("/")) script_path += "/";
        script_path += script_filename;

        // Store the full path
        node->set_meta("_js_script_path", script_path);
        node->remove_meta("_js_script_file");
    }

    // Process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            store_script_paths_recursive(child, directory);
        }
    }
}

void SceneSaver::copy_script_metadata_recursive(Node* source, Node* dest) {
    if (!source || !dest) return;

    // Copy _js_script_path metadata if present
    if (source->has_meta("_js_script_path")) {
        dest->set_meta("_js_script_path", source->get_meta("_js_script_path"));
    }

    // Process children (assuming same structure)
    TypedArray<Node> source_children = source->get_children();
    TypedArray<Node> dest_children = dest->get_children();

    int count = source_children.size() < dest_children.size() ? source_children.size() : dest_children.size();
    for (int i = 0; i < count; i++) {
        Node* src_child = Object::cast_to<Node>(source_children[i].operator Object*());
        Node* dst_child = Object::cast_to<Node>(dest_children[i].operator Object*());
        if (src_child && dst_child) {
            copy_script_metadata_recursive(src_child, dst_child);
        }
    }
}

void SceneSaver::clear_scripts_recursive(Node* node) {
    if (!node) return;

    // Clear the script from this node
    if (node->get_script().get_type() != Variant::NIL) {
        node->set_script(Variant());
    }

    // Remove temporary meta
    if (node->has_meta("_js_script_file")) {
        node->remove_meta("_js_script_file");
    }

    // Process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            clear_scripts_recursive(child);
        }
    }
}

void SceneSaver::set_owners_recursive(Node* node, Node* owner) {
    // Set the owner for this node (skip the root itself)
    if (node != owner) {
        node->set_owner(owner);
    }

    // Process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            set_owners_recursive(child, owner);
        }
    }
}

Error SceneSaver::save_metadata(const String& directory, const PackedStringArray& scripts) {
    String meta_path = directory;
    if (!meta_path.ends_with("/")) meta_path += "/";
    meta_path += "level.json";

    Dictionary metadata;
    metadata["version"] = 1;
    metadata["sandbox_version"] = "1.0.0";

    // Get current time as ISO string
    Dictionary datetime = Time::get_singleton()->get_datetime_dict_from_system();
    String timestamp = String::num_int64((int)datetime["year"]) + "-" +
                      String::num_int64((int)datetime["month"]).pad_zeros(2) + "-" +
                      String::num_int64((int)datetime["day"]).pad_zeros(2) + "T" +
                      String::num_int64((int)datetime["hour"]).pad_zeros(2) + ":" +
                      String::num_int64((int)datetime["minute"]).pad_zeros(2) + ":" +
                      String::num_int64((int)datetime["second"]).pad_zeros(2) + "Z";
    metadata["created_at"] = timestamp;

    // Script list (just filenames)
    Array script_list;
    for (int i = 0; i < scripts.size(); i++) {
        String full_path = scripts[i];
        script_list.push_back(full_path.get_file());
    }
    metadata["scripts"] = script_list;

    // Write JSON
    Ref<FileAccess> file = FileAccess::open(meta_path, FileAccess::WRITE);
    if (!file.is_valid()) {
        return ERR_FILE_CANT_WRITE;
    }

    String json_str = JSON::stringify(metadata, "\t");
    file->store_string(json_str);
    file->close();

    return OK;
}

} // namespace jsb

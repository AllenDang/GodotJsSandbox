#ifndef GODOT_JS_RUNTIME_SCENE_SAVER_H
#define GODOT_JS_RUNTIME_SCENE_SAVER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace jsb {

class JSScript;

// SceneSaver handles saving dynamically created scenes and their JS scripts
// According to TDD: saves node trees, collects JS scripts, generates .tscn and .js files
class SceneSaver {
public:
    struct SaveOptions {
        bool include_external_resources;  // Copy external resources (textures, etc.)
        bool fail_on_gdscript;            // Fail if GDScript is encountered

        SaveOptions() : include_external_resources(false), fail_on_gdscript(true) {}
    };

    struct SaveResult {
        godot::Error error = godot::OK;
        godot::String scene_path;
        godot::PackedStringArray script_paths;
        godot::PackedStringArray warnings;
        godot::String error_message;
    };

    // Save a node tree to a directory
    // Creates: level.tscn, *.js scripts, level.json metadata
    static SaveResult save(godot::Node* root, const godot::String& directory,
                          const SaveOptions& options = SaveOptions());

private:
    // Validate output path (must be user://)
    static bool validate_path(const godot::String& path, godot::String& error);

    // Collect all JS scripts from the node tree
    // Returns Dictionary: { script_name -> source_code }
    static godot::Dictionary collect_js_scripts(godot::Node* root,
                                                 godot::PackedStringArray& warnings);

    // Save a JS script file
    static godot::Error save_script(const godot::String& source,
                                    const godot::String& path);

    // Create and save the PackedScene
    static godot::Error save_scene(godot::Node* root,
                                   const godot::String& path,
                                   const godot::Dictionary& script_map,
                                   godot::String& error);

    // Save metadata JSON
    static godot::Error save_metadata(const godot::String& directory,
                                      const godot::PackedStringArray& scripts);

    // Process a single node for scene saving
    static void process_node_for_save(godot::Node* node,
                                      const godot::String& directory,
                                      godot::Dictionary& script_map,
                                      godot::PackedStringArray& warnings);

    // Generate a unique script filename
    static godot::String generate_script_filename(godot::Node* node,
                                                   const godot::Dictionary& existing);

    // Recursively collect JS scripts from node tree
    static void collect_js_scripts_recursive(godot::Node* node,
                                              godot::Dictionary& scripts,
                                              godot::PackedStringArray& warnings);

    // Update script references for saving
    static void update_script_references(godot::Node* node, const godot::String& directory);

    // Set owner recursively for scene packing
    static void set_owners_recursive(godot::Node* node, godot::Node* owner);

    // Clear scripts recursively before packing
    static void clear_scripts_recursive(godot::Node* node);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_SCENE_SAVER_H

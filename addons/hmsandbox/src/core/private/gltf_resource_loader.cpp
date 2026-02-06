#include "gltf_resource_loader.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/gltf_document.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

GLTFResourceLoader::GLTFResourceLoader() {
}

GLTFResourceLoader::~GLTFResourceLoader() {
}

void GLTFResourceLoader::_bind_methods() {
    // No additional methods to bind
}

PackedStringArray GLTFResourceLoader::_get_recognized_extensions() const {
    PackedStringArray extensions;
    extensions.push_back("glb");
    extensions.push_back("gltf");
    return extensions;
}

bool GLTFResourceLoader::_handles_type(const StringName &p_type) const {
    return p_type == StringName("PackedScene");
}

String GLTFResourceLoader::_get_resource_type(const String &p_path) const {
    String ext = p_path.get_extension().to_lower();
    if (ext == "glb" || ext == "gltf") {
        return "PackedScene";
    }
    return "";
}

Variant GLTFResourceLoader::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
    // Handle user:// paths and absolute OS file paths
    // Let Godot's default loader handle res:// (imported) files
    bool is_user_path = p_path.begins_with("user://");
    bool is_absolute_path = p_path.begins_with("/") ||  // Unix/macOS absolute path
                            (p_path.length() >= 2 && p_path[1] == ':');  // Windows drive letter (e.g., C:)

    if (!is_user_path && !is_absolute_path) {
        return Variant();
    }

    // Security: Prevent path traversal for absolute paths
    if (is_absolute_path) {
        if (p_path.find("/../") != -1 ||
            p_path.ends_with("/..") ||
            p_path.find("\\..") != -1 ||
            p_path.find("..\\") != -1) {
            UtilityFunctions::printerr("GLTF path traversal not allowed: ", p_path);
            return Variant();
        }
    }

    // Check if file exists
    if (!FileAccess::file_exists(p_path)) {
        UtilityFunctions::printerr("GLTF file not found: ", p_path);
        return Variant();
    }

    // Read the file as binary
    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
    if (!file.is_valid()) {
        UtilityFunctions::printerr("Failed to open GLTF file: ", p_path);
        return Variant();
    }

    int64_t file_length = file->get_length();
    PackedByteArray buffer = file->get_buffer(file_length);
    file->close();

    if (buffer.size() == 0) {
        UtilityFunctions::printerr("Failed to read GLTF file or file is empty: ", p_path);
        return Variant();
    }

    // Create GLTFDocument and GLTFState for runtime loading
    Ref<GLTFDocument> gltf_doc;
    gltf_doc.instantiate();

    Ref<GLTFState> gltf_state;
    gltf_state.instantiate();

    // Get base path for resolving relative references (textures, etc.)
    String base_path = p_path.get_base_dir();

    // Append from buffer - this parses the GLTF/GLB data
    Error err = gltf_doc->append_from_buffer(buffer, base_path, gltf_state);
    if (err != OK) {
        UtilityFunctions::printerr("Failed to parse GLTF file: ", p_path, " Error: ", err);
        return Variant();
    }

    // Generate the scene tree from the parsed GLTF data
    Node* root = gltf_doc->generate_scene(gltf_state);
    if (!root) {
        UtilityFunctions::printerr("Failed to generate scene from GLTF: ", p_path);
        return Variant();
    }

    // Pack the generated scene tree into a PackedScene
    Ref<PackedScene> packed_scene;
    packed_scene.instantiate();

    err = packed_scene->pack(root);

    // Clean up the temporary scene tree
    memdelete(root);

    if (err != OK) {
        UtilityFunctions::printerr("Failed to pack GLTF scene: ", p_path, " Error: ", err);
        return Variant();
    }

    return packed_scene;
}

} // namespace jsb

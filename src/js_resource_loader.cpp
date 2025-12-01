#include "js_resource_loader.h"
#include "js_script.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

// ============================================================================
// JSResourceLoader
// ============================================================================

JSResourceLoader::JSResourceLoader() {
}

JSResourceLoader::~JSResourceLoader() {
}

void JSResourceLoader::_bind_methods() {
    // No additional methods to bind
}

PackedStringArray JSResourceLoader::_get_recognized_extensions() const {
    PackedStringArray extensions;
    extensions.push_back("js");
    return extensions;
}

bool JSResourceLoader::_handles_type(const StringName &p_type) const {
    return p_type == StringName("Script") || p_type == StringName("JSScript");
}

String JSResourceLoader::_get_resource_type(const String &p_path) const {
    String ext = p_path.get_extension().to_lower();
    if (ext == "js") {
        return "JSScript";
    }
    return "";
}

Variant JSResourceLoader::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
    if (!file.is_valid()) {
        UtilityFunctions::printerr("Failed to open JS file: ", p_path);
        return Variant();
    }

    String source = file->get_as_text();
    file->close();

    Ref<JSScript> script;
    script.instantiate();

    script->set_path(p_path);
    script->_set_source_code(source);

    Error err = script->_reload(false);
    if (err != OK) {
        UtilityFunctions::printerr("Failed to parse JS file: ", p_path);
        return Variant();
    }

    return script;
}

// ============================================================================
// JSResourceSaver
// ============================================================================

JSResourceSaver::JSResourceSaver() {
}

JSResourceSaver::~JSResourceSaver() {
}

void JSResourceSaver::_bind_methods() {
    // No additional methods to bind
}

PackedStringArray JSResourceSaver::_get_recognized_extensions(const Ref<Resource> &p_resource) const {
    PackedStringArray extensions;
    Ref<JSScript> script = p_resource;
    if (script.is_valid()) {
        extensions.push_back("js");
    }
    return extensions;
}

bool JSResourceSaver::_recognize(const Ref<Resource> &p_resource) const {
    Ref<JSScript> script = p_resource;
    return script.is_valid();
}

Error JSResourceSaver::_save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
    Ref<JSScript> script = p_resource;
    if (!script.is_valid()) {
        return ERR_INVALID_PARAMETER;
    }

    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
    if (!file.is_valid()) {
        return ERR_FILE_CANT_OPEN;
    }

    String source = script->_get_source_code();
    file->store_string(source);
    file->close();

    return OK;
}

} // namespace jsb

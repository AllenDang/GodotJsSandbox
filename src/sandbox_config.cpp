#include "sandbox_config.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>

using namespace godot;

namespace jsb {

SandboxConfig::SandboxConfig() {
    setup_default_blocklist();
}

SandboxConfig::~SandboxConfig() {
}

void SandboxConfig::setup_default_blocklist() {
    // ===== Blocked Classes =====

    // File system access
    blocked_classes_.insert("DirAccess");
    blocked_classes_.insert("FileAccess");

    // OS-level access
    blocked_classes_.insert("OS");

    // Networking
    blocked_classes_.insert("HTTPClient");
    blocked_classes_.insert("HTTPRequest");
    blocked_classes_.insert("StreamPeerTCP");
    blocked_classes_.insert("TCPServer");
    blocked_classes_.insert("UDPServer");
    blocked_classes_.insert("PacketPeerUDP");
    blocked_classes_.insert("WebSocketPeer");
    blocked_classes_.insert("ENetConnection");
    blocked_classes_.insert("ENetMultiplayerPeer");
    blocked_classes_.insert("MultiplayerPeer");

    // Threading
    blocked_classes_.insert("Thread");
    blocked_classes_.insert("Mutex");
    blocked_classes_.insert("Semaphore");
    blocked_classes_.insert("WorkerThreadPool");

    // Script/code execution (prevent code injection)
    blocked_classes_.insert("GDScript");
    blocked_classes_.insert("CSharpScript");
    blocked_classes_.insert("JavaScriptBridge");
    blocked_classes_.insert("Expression");

    // Resource management (use safe load() instead)
    blocked_classes_.insert("ResourceLoader");
    blocked_classes_.insert("ResourceSaver");

    // Native extensions
    blocked_classes_.insert("NativeExtension");
    blocked_classes_.insert("GDExtensionManager");

    // Editor classes
    blocked_classes_.insert("ProjectSettings");
    blocked_classes_.insert("EditorInterface");
    blocked_classes_.insert("EditorPlugin");
    blocked_classes_.insert("EditorScript");

    // ===== Blocked Methods =====

    // Object - block reflection and dynamic method calls
    blocked_methods_.insert("Object.call");
    blocked_methods_.insert("Object.callv");
    blocked_methods_.insert("Object.set_script");
    blocked_methods_.insert("Object.get_script");

    // ClassDB - block dynamic instantiation
    blocked_methods_.insert("ClassDB.instantiate");
    blocked_methods_.insert("ClassDB.instance");
    blocked_methods_.insert("ClassDB.can_instantiate");
    blocked_methods_.insert("ClassDB.get_class_list");

    // Engine - block singleton access
    blocked_methods_.insert("Engine.get_singleton");
    blocked_methods_.insert("Engine.register_singleton");
    blocked_methods_.insert("Engine.unregister_singleton");

    // Node - prevent script replacement
    blocked_methods_.insert("Node.set_script");

    // ===== Allowed Paths =====
    allowed_path_prefixes_.insert("res://");
    allowed_path_prefixes_.insert("user://");
}

Error SandboxConfig::load(const String& path) {
    if (!FileAccess::file_exists(path)) {
        return ERR_FILE_NOT_FOUND;
    }

    Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
    if (!file.is_valid()) {
        return ERR_CANT_OPEN;
    }

    String content = file->get_as_text();
    file.unref();

    Ref<JSON> json;
    json.instantiate();
    Error err = json->parse(content);
    if (err != OK) {
        UtilityFunctions::printerr("Failed to parse blocklist JSON: ", json->get_error_message());
        return err;
    }

    Variant data = json->get_data();
    if (data.get_type() != Variant::DICTIONARY) {
        return ERR_INVALID_DATA;
    }

    Dictionary dict = data;

    // Load blocked classes
    if (dict.has("blocked_classes")) {
        Array classes = dict["blocked_classes"];
        for (int i = 0; i < classes.size(); i++) {
            block_class(classes[i]);
        }
    }

    // Load blocked methods
    if (dict.has("blocked_methods")) {
        Array methods = dict["blocked_methods"];
        for (int i = 0; i < methods.size(); i++) {
            String method_spec = methods[i];
            int dot_pos = method_spec.find(".");
            if (dot_pos > 0) {
                String class_name = method_spec.substr(0, dot_pos);
                String method_name = method_spec.substr(dot_pos + 1);
                block_method(class_name, method_name);
            }
        }
    }

    // Load blocked properties
    if (dict.has("blocked_properties")) {
        Array properties = dict["blocked_properties"];
        for (int i = 0; i < properties.size(); i++) {
            String prop_spec = properties[i];
            int dot_pos = prop_spec.find(".");
            if (dot_pos > 0) {
                String class_name = prop_spec.substr(0, dot_pos);
                String prop_name = prop_spec.substr(dot_pos + 1);
                block_property(class_name, prop_name);
            }
        }
    }

    // Load allowed path prefixes
    if (dict.has("allowed_paths")) {
        Array paths = dict["allowed_paths"];
        for (int i = 0; i < paths.size(); i++) {
            add_allowed_path_prefix(paths[i]);
        }
    }

    return OK;
}

void SandboxConfig::block_class(const String& class_name) {
    blocked_classes_.insert(class_name);
}

void SandboxConfig::unblock_class(const String& class_name) {
    blocked_classes_.erase(class_name);
}

bool SandboxConfig::is_class_blocked(const String& class_name) const {
    return blocked_classes_.has(class_name);
}

bool SandboxConfig::is_class_or_parent_blocked(const StringName& class_name) const {
    // Walk up the inheritance chain to check if any parent class is blocked
    StringName current = class_name;

    while (!current.is_empty()) {
        if (blocked_classes_.has(String(current))) {
            return true;
        }
        // Get parent class using ClassDB
        current = ClassDB::get_parent_class(current);
    }

    return false;
}

void SandboxConfig::block_method(const String& class_name, const String& method_name) {
    blocked_methods_.insert(class_name + String(".") + method_name);
}

void SandboxConfig::unblock_method(const String& class_name, const String& method_name) {
    blocked_methods_.erase(class_name + String(".") + method_name);
}

bool SandboxConfig::is_method_blocked(const String& class_name, const String& method_name) const {
    return blocked_methods_.has(class_name + String(".") + method_name);
}

void SandboxConfig::block_property(const String& class_name, const String& property_name) {
    blocked_properties_.insert(class_name + String(".") + property_name);
}

bool SandboxConfig::is_property_blocked(const String& class_name, const String& property_name) const {
    return blocked_properties_.has(class_name + String(".") + property_name);
}

bool SandboxConfig::is_path_allowed(const String& path) const {
    // Check for path traversal
    if (path.find("..") != -1) {
        return false;
    }

    // Check if path starts with an allowed prefix
    return std::any_of(allowed_path_prefixes_.begin(), allowed_path_prefixes_.end(),
        [&path](const String& prefix) { return path.begins_with(prefix); });
}

void SandboxConfig::add_allowed_path_prefix(const String& prefix) {
    allowed_path_prefixes_.insert(prefix);
}

void SandboxConfig::reset() {
    blocked_classes_.clear();
    blocked_methods_.clear();
    blocked_properties_.clear();
    allowed_path_prefixes_.clear();
    setup_default_blocklist();
}

} // namespace jsb

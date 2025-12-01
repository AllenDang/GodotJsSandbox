#ifndef GODOT_JS_RUNTIME_SANDBOX_CONFIG_H
#define GODOT_JS_RUNTIME_SANDBOX_CONFIG_H

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/classes/global_constants.hpp>

namespace jsb {

// SandboxConfig manages security configuration for the JS sandbox
// Handles blocklists for classes, methods, and paths
class SandboxConfig {
public:
    SandboxConfig();
    ~SandboxConfig();

    // Load blocklist from JSON file
    godot::Error load(const godot::String& path);

    // Class blocklist
    void block_class(const godot::String& class_name);
    void unblock_class(const godot::String& class_name);
    bool is_class_blocked(const godot::String& class_name) const;

    // Method blocklist (format: "ClassName.method_name")
    void block_method(const godot::String& class_name, const godot::String& method_name);
    void unblock_method(const godot::String& class_name, const godot::String& method_name);
    bool is_method_blocked(const godot::String& class_name, const godot::String& method_name) const;

    // Property blocklist
    void block_property(const godot::String& class_name, const godot::String& property_name);
    bool is_property_blocked(const godot::String& class_name, const godot::String& property_name) const;

    // Path checking
    bool is_path_allowed(const godot::String& path) const;
    void add_allowed_path_prefix(const godot::String& prefix);

    // Reset to defaults
    void reset();

private:
    godot::HashSet<godot::String> blocked_classes_;
    godot::HashSet<godot::String> blocked_methods_;  // "ClassName.method"
    godot::HashSet<godot::String> blocked_properties_;  // "ClassName.property"
    godot::HashSet<godot::String> allowed_path_prefixes_;

    void setup_default_blocklist();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_SANDBOX_CONFIG_H

#ifndef GODOT_JS_RUNTIME_SAFE_WRAPPER_H
#define GODOT_JS_RUNTIME_SAFE_WRAPPER_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "execution_limiter.h"

namespace jsb {

class ObjectRegistry;
class SandboxConfig;
class DeletionTracker;

// SafeWrapper is the core security layer for all Godot API calls from JavaScript
// All API calls pass through this wrapper for validation and safety checks
class SafeWrapper {
public:
    SafeWrapper();
    ~SafeWrapper();

    void set_object_registry(ObjectRegistry* registry) { object_registry_ = registry; }
    void set_sandbox_config(SandboxConfig* config) { sandbox_config_ = config; }
    void set_execution_limiter(ExecutionLimiter* limiter) { execution_limiter_ = limiter; }
    void set_deletion_tracker(DeletionTracker* tracker) { deletion_tracker_ = tracker; }

    // Object creation - creates a new Godot object if allowed
    // Returns handle or 0 on failure
    uint64_t create_object(const godot::StringName& class_name, godot::String& error);

    // Method invocation - calls a method on an object
    godot::Variant call_method(uint64_t handle, const godot::StringName& method,
                               const godot::Variant** args, int argc,
                               godot::String& error);

    // Property access
    godot::Variant get_property(uint64_t handle, const godot::StringName& property,
                                godot::String& error);
    bool set_property(uint64_t handle, const godot::StringName& property,
                     const godot::Variant& value, godot::String& error);

    // Node tree operations (special handling for safety)
    bool add_child(uint64_t parent_handle, uint64_t child_handle, godot::String& error);
    bool remove_child(uint64_t parent_handle, uint64_t child_handle, godot::String& error);
    bool queue_free(uint64_t handle, godot::String& error);

    // Resource loading (with path validation)
    godot::Variant load_resource(const godot::String& path, godot::String& error);

    // Check if operation is allowed
    bool is_class_allowed(const godot::StringName& class_name) const;
    bool is_method_allowed(const godot::StringName& class_name, const godot::StringName& method) const;
    bool is_property_allowed(const godot::StringName& class_name, const godot::StringName& property) const;

    // Get last error message
    godot::String get_last_error() const { return last_error_; }

private:
    ObjectRegistry* object_registry_ = nullptr;
    SandboxConfig* sandbox_config_ = nullptr;
    ExecutionLimiter* execution_limiter_ = nullptr;
    DeletionTracker* deletion_tracker_ = nullptr;

    godot::String last_error_;

    // Validate object handle and return the object
    godot::Object* validate_handle(uint64_t handle, godot::String& error);

    // Check rate limits (tiered per PRD Section 6.4)
    // READ: unlimited, WRITE: 500/frame, HEAVY: 50/frame
    bool check_rate_limit(ApiCategory category, godot::String& error);

    // Get the class name of an object (handles inheritance)
    static godot::StringName get_object_class(godot::Object* obj);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_SAFE_WRAPPER_H

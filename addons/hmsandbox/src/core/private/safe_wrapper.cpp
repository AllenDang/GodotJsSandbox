#include "safe_wrapper.h"
#include "object_registry.h"
#include "sandbox_config.h"
#include "execution_limiter.h"
#include "deletion_tracker.h"
#include "js_script.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace jsb {

SafeWrapper::SafeWrapper() {
}

SafeWrapper::~SafeWrapper() {
}

Object* SafeWrapper::validate_handle(uint64_t handle, String& error) {
    if (!object_registry_) {
        error = "Object registry not initialized";
        return nullptr;
    }

    Object* obj = object_registry_->get_object(handle);
    if (!obj) {
        error = "Invalid object handle or object has been freed";
        return nullptr;
    }

    return obj;
}

bool SafeWrapper::check_rate_limit(ApiCategory category, String& error) {
    if (execution_limiter_ && !execution_limiter_->check_api_rate_limit(category)) {
        switch (category) {
            case ApiCategory::WRITE:
                error = "Write operation rate limit exceeded (max " +
                        String::num_int64(execution_limiter_->get_max_write_ops_per_frame()) + "/frame)";
                break;
            case ApiCategory::HEAVY:
                error = "Heavy operation rate limit exceeded (max " +
                        String::num_int64(execution_limiter_->get_max_heavy_ops_per_frame()) + "/frame)";
                break;
            default:
                error = "API rate limit exceeded";
                break;
        }
        return false;
    }
    return true;
}

StringName SafeWrapper::get_object_class(Object* obj) {
    if (!obj) return StringName();
    return obj->get_class();
}

bool SafeWrapper::is_class_allowed(const StringName& class_name) const {
    if (!sandbox_config_) return true;
    // Security: Walk the inheritance chain to block subclasses of blocked classes
    // e.g., if "Node" is blocked, Node2D, Node3D, Control, etc. are also blocked
    return !sandbox_config_->is_class_or_parent_blocked(class_name);
}

bool SafeWrapper::is_method_allowed(const StringName& class_name, const StringName& method) const {
    if (!sandbox_config_) return true;

    // Security: Block methods using inheritance-aware checking
    // This walks up the class hierarchy, so blocking "Object.call" will block
    // call() on Node, Node3D, CharacterBody3D, etc.
    // And blocking "Node.set_script" will block set_script() on Node3D, Control, etc.
    return !sandbox_config_->is_method_blocked_with_inheritance(class_name, String(method));
}

bool SafeWrapper::is_property_allowed(const StringName& class_name, const StringName& property) const {
    if (!sandbox_config_) return true;
    // Security: Use inheritance-aware check, so blocking "Object.script" blocks it on all subclasses
    return !sandbox_config_->is_property_blocked_with_inheritance(class_name, String(property));
}

uint64_t SafeWrapper::create_object(const StringName& class_name, String& error) {
    // Check rate limit - instantiate is a HEAVY operation per PRD Section 6.4
    if (!check_rate_limit(ApiCategory::HEAVY, error)) {
        return 0;
    }

    // Check if class is allowed
    if (!is_class_allowed(class_name)) {
        error = "Class is blocked: " + String(class_name);
        return 0;
    }

    // Check if class can be instantiated
    if (!ClassDB::can_instantiate(class_name)) {
        error = "Class cannot be instantiated: " + String(class_name);
        return 0;
    }

    // Create the object
    Variant instance = ClassDB::instantiate(class_name);
    if (instance.get_type() != Variant::OBJECT) {
        error = "Failed to instantiate class: " + String(class_name);
        return 0;
    }

    Object* obj = instance;
    if (!obj) {
        error = "Failed to instantiate class: " + String(class_name);
        return 0;
    }

    // Register the object
    if (!object_registry_) {
        error = "Object registry not initialized";
        // Clean up the created object
        const RefCounted* ref = Object::cast_to<RefCounted>(obj);
        if (!ref) {
            memdelete(obj);
        }
        return 0;
    }

    // Mark as JS-created (true) so we can track objects created by JavaScript
    uint64_t handle = object_registry_->create_handle(obj, true);

    // Track Node objects for deletion notification
    Node* node = Object::cast_to<Node>(obj);
    if (node && deletion_tracker_) {
        deletion_tracker_->track_node(node);
    }

    return handle;
}

Variant SafeWrapper::call_method(uint64_t handle, const StringName& method,
                                  const Variant** args, int argc, String& error) {
    // Check rate limit - method calls are WRITE operations per PRD Section 6.4
    if (!check_rate_limit(ApiCategory::WRITE, error)) {
        return Variant();
    }

    // Validate handle
    Object* obj = validate_handle(handle, error);
    if (!obj) {
        return Variant();
    }

    // Get object class
    StringName class_name = get_object_class(obj);

    // Check if method is allowed
    if (!is_method_allowed(class_name, method)) {
        error = "Method is blocked: " + String(class_name) + "." + String(method);
        return Variant();
    }

    // Special handling for set_script: only allow JSScript types (PRD requirement)
    // This allows JS to attach scripts to nodes while preventing GDScript/CSharpScript injection
    if (method == StringName("set_script")) {
        if (argc < 1) {
            error = "set_script requires a script argument";
            return Variant();
        }

        const Variant& script_arg = *args[0];

        // Allow null to clear the script
        if (script_arg.get_type() == Variant::NIL) {
            obj->set_script(Variant());
            return Variant();
        }

        // Must be an Object
        if (script_arg.get_type() != Variant::OBJECT) {
            error = "set_script argument must be a Script or null";
            return Variant();
        }

        Object* script_obj = script_arg;
        if (!script_obj) {
            error = "set_script argument is invalid";
            return Variant();
        }

        // Verify it's a JSScript - block GDScript, CSharpScript, etc.
        const JSScript* js_script = Object::cast_to<JSScript>(script_obj);
        if (!js_script) {
            error = "Only JSScript can be attached via set_script (GDScript/CSharpScript not allowed)";
            return Variant();
        }

        // Safe to set the script
        obj->set_script(script_arg);
        return Variant();
    }

    // Check if method exists
    if (!obj->has_method(method)) {
        error = "Method not found: " + String(class_name) + "." + String(method);
        return Variant();
    }

    // Call the method using Variant::callp which provides error information
    // This allows us to capture runtime errors for AI feedback
    Variant obj_variant = obj;
    Variant result;
    GDExtensionCallError call_error;

    obj_variant.callp(method, args, argc, result, call_error);

    // Check for call errors and convert to error string
    if (call_error.error != GDEXTENSION_CALL_OK) {
        switch (call_error.error) {
            case GDEXTENSION_CALL_ERROR_INVALID_METHOD:
                error = "Invalid method: " + String(class_name) + "." + String(method);
                break;
            case GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT:
                error = "Invalid argument at index " + String::num_int64(call_error.argument) +
                        " for method " + String(class_name) + "." + String(method) +
                        " (expected type " + String::num_int64(call_error.expected) + ")";
                break;
            case GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS:
                error = "Too many arguments for method " + String(class_name) + "." + String(method) +
                        " (got " + String::num_int64(argc) + ", expected " + String::num_int64(call_error.expected) + ")";
                break;
            case GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS:
                error = "Too few arguments for method " + String(class_name) + "." + String(method) +
                        " (got " + String::num_int64(argc) + ", expected " + String::num_int64(call_error.expected) + ")";
                break;
            case GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL:
                error = "Instance is null when calling " + String(class_name) + "." + String(method);
                break;
            case GDEXTENSION_CALL_ERROR_METHOD_NOT_CONST:
                error = "Method " + String(class_name) + "." + String(method) + " is not const";
                break;
            default:
                error = "Unknown error calling " + String(class_name) + "." + String(method);
                break;
        }
        return Variant();
    }

    return result;
}

Variant SafeWrapper::get_property(uint64_t handle, const StringName& property, String& error) {
    // Property reads are READ operations - unlimited per PRD Section 6.4
    // No rate limit check needed for reads

    // Validate handle
    Object* obj = validate_handle(handle, error);
    if (!obj) {
        return Variant();
    }

    // Get object class
    StringName class_name = get_object_class(obj);

    // Check if property is allowed
    if (!is_property_allowed(class_name, property)) {
        error = "Property is blocked: " + String(class_name) + "." + String(property);
        return Variant();
    }

    // Get the property using Variant API with validation for error capture
    Variant obj_variant = obj;
    bool valid = false;
    Variant result = obj_variant.get_named(property, valid);

    if (!valid) {
        error = "Property not found or not accessible: " + String(class_name) + "." + String(property);
        return Variant();
    }

    return result;
}

bool SafeWrapper::set_property(uint64_t handle, const StringName& property,
                               const Variant& value, String& error) {
    // Property writes are WRITE operations per PRD Section 6.4
    if (!check_rate_limit(ApiCategory::WRITE, error)) {
        return false;
    }

    // Validate handle
    Object* obj = validate_handle(handle, error);
    if (!obj) {
        return false;
    }

    // Get object class
    StringName class_name = get_object_class(obj);

    // Check if property is allowed
    if (!is_property_allowed(class_name, property)) {
        error = "Property is blocked: " + String(class_name) + "." + String(property);
        return false;
    }

    // Set the property using Variant API with validation for error capture
    Variant obj_variant = obj;
    bool valid = false;
    obj_variant.set_named(property, value, valid);

    if (!valid) {
        error = "Property not found or cannot be set: " + String(class_name) + "." + String(property);
        return false;
    }

    return true;
}

bool SafeWrapper::add_child(uint64_t parent_handle, uint64_t child_handle, String& error) {
    // add_child is a WRITE operation per PRD Section 6.4
    if (!check_rate_limit(ApiCategory::WRITE, error)) {
        return false;
    }

    // Validate handles
    Object* parent_obj = validate_handle(parent_handle, error);
    if (!parent_obj) {
        return false;
    }

    Object* child_obj = validate_handle(child_handle, error);
    if (!child_obj) {
        return false;
    }

    // Cast to Node
    Node* parent = Object::cast_to<Node>(parent_obj);
    if (!parent) {
        error = "Parent is not a Node";
        return false;
    }

    Node* child = Object::cast_to<Node>(child_obj);
    if (!child) {
        error = "Child is not a Node";
        return false;
    }

    // Check if child already has a parent
    if (child->get_parent()) {
        error = "Child already has a parent";
        return false;
    }

    // Add child
    parent->add_child(child);
    return true;
}

bool SafeWrapper::remove_child(uint64_t parent_handle, uint64_t child_handle, String& error) {
    // remove_child is a WRITE operation per PRD Section 6.4
    if (!check_rate_limit(ApiCategory::WRITE, error)) {
        return false;
    }

    // Validate handles
    Object* parent_obj = validate_handle(parent_handle, error);
    if (!parent_obj) {
        return false;
    }

    Object* child_obj = validate_handle(child_handle, error);
    if (!child_obj) {
        return false;
    }

    // Cast to Node
    Node* parent = Object::cast_to<Node>(parent_obj);
    if (!parent) {
        error = "Parent is not a Node";
        return false;
    }

    Node* child = Object::cast_to<Node>(child_obj);
    if (!child) {
        error = "Child is not a Node";
        return false;
    }

    // Verify parent-child relationship
    if (child->get_parent() != parent) {
        error = "Child is not a child of the specified parent";
        return false;
    }

    // Remove child
    parent->remove_child(child);
    return true;
}

bool SafeWrapper::queue_free(uint64_t handle, String& error) {
    // queue_free is a HEAVY operation per PRD Section 6.4
    if (!check_rate_limit(ApiCategory::HEAVY, error)) {
        return false;
    }

    // Validate handle
    Object* obj = validate_handle(handle, error);
    if (!obj) {
        return false;
    }

    // Cast to Node
    Node* node = Object::cast_to<Node>(obj);
    if (!node) {
        error = "Object is not a Node, cannot queue_free";
        return false;
    }

    // Queue free - the ObjectRegistry will be notified via NOTIFICATION_PREDELETE
    node->queue_free();
    return true;
}

Variant SafeWrapper::load_resource(const String& path, String& error) {
    // Resource loading is a HEAVY operation per PRD Section 6.4
    if (!check_rate_limit(ApiCategory::HEAVY, error)) {
        return Variant();
    }

    // Validate path using sandbox config (supports res://, user://, and configured OS paths)
    if (sandbox_config_ && !sandbox_config_->is_path_allowed(path)) {
        error = "Path not allowed: " + path;
        return Variant();
    }

    // Check for path traversal patterns
    // Detect "/../", "/..\" or paths ending/starting with ".."
    // But NOT reject legitimate filenames like "file..name.png"
    if (path.find("/../") != -1 ||
        path.ends_with("/..") ||
        path.find("\\..") != -1 ||
        path.find("..\\") != -1) {
        error = "Path traversal not allowed";
        return Variant();
    }

    // Check file extension
    String ext = path.get_extension().to_lower();

    // Blocked extensions
    if (ext == "gd" || ext == "cs" || ext == "gdns" || ext == "gdnlib" ||
        ext == "so" || ext == "dll" || ext == "dylib" || ext == "gdextension") {
        error = "Resource type not allowed: " + ext;
        return Variant();
    }

    // Load the resource
    Ref<Resource> resource = ResourceLoader::get_singleton()->load(path);
    if (!resource.is_valid()) {
        error = "Failed to load resource: " + path;
        return Variant();
    }

    return resource;
}

} // namespace jsb

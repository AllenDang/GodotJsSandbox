#ifndef GODOT_JS_RUNTIME_OBJECT_REGISTRY_H
#define GODOT_JS_RUNTIME_OBJECT_REGISTRY_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <cstdint>

namespace jsb {

// ObjectRegistry manages safe references to Godot objects from JavaScript
// Uses handles to prevent dangling pointers and invalid access
// Handles RefCounted objects with proper reference counting
class ObjectRegistry {
public:
    ObjectRegistry();
    ~ObjectRegistry();

    // Create a handle for an object
    // For RefCounted objects, this increments the reference count
    uint64_t create_handle(godot::Object* obj);

    // Get object from handle (returns nullptr if invalid)
    godot::Object* get_object(uint64_t handle);

    // Release a handle (called when JS object is garbage collected)
    // For RefCounted objects, this decrements the reference count
    void release_handle(uint64_t handle);

    // Check if handle is valid
    bool is_valid(uint64_t handle) const;

    // Mark an object as deleted (called on NOTIFICATION_PREDELETE)
    void mark_deleted(uint64_t object_id);

    // Clear all handles
    void clear_all();

    // Get count of active handles
    int get_handle_count() const;

    // Find handle by object ID
    uint64_t find_handle_by_object_id(uint64_t object_id) const;

    // Get all valid object IDs (for tracking created objects)
    godot::Vector<uint64_t> get_all_object_ids() const;

private:
    struct HandleEntry {
        godot::Object* object = nullptr;
        uint64_t object_id = 0;
        uint32_t version = 0;       // Generation number to detect stale handles
        bool is_ref_counted = false;
        bool is_valid = true;       // Set to false when object is deleted
    };

    godot::HashMap<uint64_t, HandleEntry> handles_;
    godot::HashMap<uint64_t, uint64_t> object_id_to_handle_;  // Reverse mapping
    uint64_t next_handle_ = 1;
    uint32_t current_version_ = 1;
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_OBJECT_REGISTRY_H

#include "object_registry.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

ObjectRegistry::ObjectRegistry() {
}

ObjectRegistry::~ObjectRegistry() {
    clear_all();
}

uint64_t ObjectRegistry::create_handle(Object* obj, bool js_created) {
    if (!obj) {
        return 0;
    }

    uint64_t obj_id = obj->get_instance_id();

    // Check if we already have a handle for this object
    if (object_id_to_handle_.has(obj_id)) {
        uint64_t existing_handle = object_id_to_handle_[obj_id];
        if (handles_.has(existing_handle) && handles_[existing_handle].is_valid) {
            // If this is now being marked as JS-created, update the flag
            if (js_created && !handles_[existing_handle].is_js_created) {
                handles_[existing_handle].is_js_created = true;
            }
            return existing_handle;
        }
    }

    uint64_t handle = next_handle_++;

    HandleEntry entry;
    entry.object = obj;
    entry.object_id = obj_id;
    entry.version = current_version_++;
    entry.is_ref_counted = Object::cast_to<RefCounted>(obj) != nullptr;
    entry.is_valid = true;
    entry.is_js_created = js_created;
    entry.is_node = Object::cast_to<Node>(obj) != nullptr;

    // For RefCounted objects, increment reference count to prevent premature deletion
    if (entry.is_ref_counted) {
        RefCounted* ref = Object::cast_to<RefCounted>(obj);
        if (ref) {
            ref->reference();
        }
    }

    handles_[handle] = entry;
    object_id_to_handle_[obj_id] = handle;

    return handle;
}

Object* ObjectRegistry::get_object(uint64_t handle) {
    if (handle == 0) {
        return nullptr;
    }

    if (!handles_.has(handle)) {
        return nullptr;
    }

    HandleEntry& entry = handles_[handle];

    // Check if marked as deleted
    if (!entry.is_valid) {
        return nullptr;
    }

    // Verify object is still valid by checking instance ID
    Object* obj = ObjectDB::get_instance(ObjectID(entry.object_id));
    if (!obj) {
        // Object was freed externally, mark as invalid
        entry.is_valid = false;
        return nullptr;
    }

    return obj;
}

void ObjectRegistry::release_handle(uint64_t handle) {
    if (handle == 0) {
        return;
    }

    if (!handles_.has(handle)) {
        return;
    }

    HandleEntry& entry = handles_[handle];

    // For RefCounted objects, decrement reference count
    if (entry.is_ref_counted && entry.is_valid) {
        Object* obj = ObjectDB::get_instance(ObjectID(entry.object_id));
        if (obj) {
            RefCounted* ref = Object::cast_to<RefCounted>(obj);
            if (ref) {
                // unreference() returns true when refcount goes to 0
                // In that case, we need to delete the object
                if (ref->unreference()) {
                    memdelete(ref);
                }
            }
        }
    }
    // For non-RefCounted, non-Node, JS-created objects (like local RenderingDevice),
    // we need to explicitly free them using memdelete()
    else if (entry.is_js_created && !entry.is_ref_counted && !entry.is_node && entry.is_valid) {
        Object* obj = ObjectDB::get_instance(ObjectID(entry.object_id));
        if (obj) {
            memdelete(obj);
        }
    }

    // Remove from reverse mapping
    object_id_to_handle_.erase(entry.object_id);

    // Remove handle
    handles_.erase(handle);
}

bool ObjectRegistry::is_valid(uint64_t handle) const {
    if (handle == 0) {
        return false;
    }

    if (!handles_.has(handle)) {
        return false;
    }

    const HandleEntry& entry = handles_[handle];

    if (!entry.is_valid) {
        return false;
    }

    const Object* obj = ObjectDB::get_instance(ObjectID(entry.object_id));
    return obj != nullptr;
}

void ObjectRegistry::mark_deleted(uint64_t object_id) {
    if (!object_id_to_handle_.has(object_id)) {
        return;
    }

    uint64_t handle = object_id_to_handle_[object_id];

    if (handles_.has(handle)) {
        handles_[handle].is_valid = false;
        handles_[handle].object = nullptr;
    }
}

uint64_t ObjectRegistry::find_handle_by_object_id(uint64_t object_id) const {
    if (object_id_to_handle_.has(object_id)) {
        return object_id_to_handle_[object_id];
    }
    return 0;
}

uint64_t ObjectRegistry::get_or_create_handle(Object* obj, bool js_created) {
    if (!obj) return 0;

    // Check if we already have a handle for this object
    uint64_t object_id = obj->get_instance_id();
    uint64_t existing = find_handle_by_object_id(object_id);
    if (existing != 0) {
        // If caller indicates JS-created, update the flag on existing handle
        if (js_created && handles_.has(existing) && !handles_[existing].is_js_created) {
            handles_[existing].is_js_created = true;
        }
        return existing;
    }

    // Create new handle with the js_created flag
    return create_handle(obj, js_created);
}

void ObjectRegistry::clear_all() {
    // Release all RefCounted objects
    for (auto& pair : handles_) {
        HandleEntry& entry = pair.value;
        if (entry.is_ref_counted && entry.is_valid) {
            Object* obj = ObjectDB::get_instance(ObjectID(entry.object_id));
            if (obj) {
                RefCounted* ref = Object::cast_to<RefCounted>(obj);
                if (ref) {
                    // unreference() returns true when refcount goes to 0
                    // In that case, we need to delete the object
                    if (ref->unreference()) {
                        memdelete(ref);
                    }
                }
            }
        }
        // Free non-RefCounted, non-Node, JS-created objects (like local RenderingDevice)
        else if (entry.is_js_created && !entry.is_ref_counted && !entry.is_node && entry.is_valid) {
            Object* obj = ObjectDB::get_instance(ObjectID(entry.object_id));
            if (obj) {
                memdelete(obj);
            }
        }
    }

    handles_.clear();
    object_id_to_handle_.clear();
    next_handle_ = 1;
}

int ObjectRegistry::get_handle_count() const {
    return handles_.size();
}

Vector<uint64_t> ObjectRegistry::get_all_object_ids() const {
    Vector<uint64_t> result;
    for (const KeyValue<uint64_t, HandleEntry>& E : handles_) {
        if (E.value.is_valid) {
            result.push_back(E.value.object_id);
        }
    }
    return result;
}

Vector<uint64_t> ObjectRegistry::get_js_created_node_ids() const {
    Vector<uint64_t> result;
    for (const KeyValue<uint64_t, HandleEntry>& E : handles_) {
        if (E.value.is_valid && E.value.is_js_created && E.value.is_node) {
            result.push_back(E.value.object_id);
        }
    }
    return result;
}

} // namespace jsb

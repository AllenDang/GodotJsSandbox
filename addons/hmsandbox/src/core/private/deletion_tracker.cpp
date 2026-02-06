#include "deletion_tracker.h"
#include "object_registry.h"
#include "signal_registry.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

namespace jsb {

// ============================================================================
// DeletionCallback Implementation
// ============================================================================

DeletionCallback::DeletionCallback() {
}

DeletionCallback::~DeletionCallback() {
}

void DeletionCallback::setup(DeletionTracker* tracker, uint64_t object_id) {
    tracker_ = tracker;
    object_id_ = object_id;
}

void DeletionCallback::on_tree_exiting() {
    if (tracker_) {
        tracker_->on_node_deleted(object_id_);
    }
}

void DeletionCallback::_bind_methods() {
    ClassDB::bind_method(D_METHOD("on_tree_exiting"), &DeletionCallback::on_tree_exiting);
}

// ============================================================================
// DeletionTracker Implementation
// ============================================================================

DeletionTracker::DeletionTracker() {
}

DeletionTracker::~DeletionTracker() {
    clear();
}

void DeletionTracker::track_node(Node* node) {
    if (!node) {
        return;
    }

    uint64_t object_id = node->get_instance_id();

    // Already tracked
    if (tracked_object_ids_.has(object_id)) {
        return;
    }

    tracked_object_ids_.insert(object_id);

    // Create a callback object to receive the signal
    Ref<DeletionCallback> callback;
    callback.instantiate();
    callback->setup(this, object_id);

    // Connect to tree_exiting signal - emitted when node is about to leave the tree
    // This happens before queue_free completes deletion
    Callable callable = Callable(callback.ptr(), "on_tree_exiting");
    Error err = node->connect("tree_exiting", callable, Object::CONNECT_ONE_SHOT);

    if (err == OK) {
        callbacks_[object_id] = callback;
    } else {
        // Connection failed, but we still track the object_id
        // ObjectRegistry's get_object() will handle validation via ObjectDB
        UtilityFunctions::print_verbose("DeletionTracker: Failed to connect tree_exiting signal for object ", object_id);
    }
}

void DeletionTracker::untrack_node(Node* node) {
    if (!node) {
        return;
    }

    uint64_t object_id = node->get_instance_id();

    // Disconnect the signal if we have a callback
    if (callbacks_.has(object_id)) {
        Ref<DeletionCallback> callback = callbacks_[object_id];
        if (callback.is_valid() && node->is_connected("tree_exiting", Callable(callback.ptr(), "on_tree_exiting"))) {
            node->disconnect("tree_exiting", Callable(callback.ptr(), "on_tree_exiting"));
        }
        callbacks_.erase(object_id);
    }

    tracked_object_ids_.erase(object_id);
}

bool DeletionTracker::is_tracked(Node* node) const {
    if (!node) {
        return false;
    }
    return tracked_object_ids_.has(node->get_instance_id());
}

void DeletionTracker::clear() {
    // Disconnect all signals
    for (const KeyValue<uint64_t, Ref<DeletionCallback>>& E : callbacks_) {
        Ref<DeletionCallback> callback = E.value;

        if (callback.is_valid()) {
            // Try to get the node - it might already be deleted
            Object* obj = ObjectDB::get_instance(ObjectID(E.key));
            if (obj) {
                Node* node = Object::cast_to<Node>(obj);
                if (node && node->is_connected("tree_exiting", Callable(callback.ptr(), "on_tree_exiting"))) {
                    node->disconnect("tree_exiting", Callable(callback.ptr(), "on_tree_exiting"));
                }
            }
        }
    }

    callbacks_.clear();
    tracked_object_ids_.clear();
}

void DeletionTracker::on_node_deleted(uint64_t object_id) {
    if (!tracked_object_ids_.has(object_id)) {
        return;
    }

    // Notify ObjectRegistry to invalidate the handle
    if (object_registry_) {
        object_registry_->mark_deleted(object_id);
    }

    // Disconnect all signals from this object
    if (signal_registry_) {
        signal_registry_->disconnect_all_from_object(object_id);
    }

    // Clean up callback (signal was one-shot, so already disconnected)
    callbacks_.erase(object_id);

    // Remove from tracking
    tracked_object_ids_.erase(object_id);
}

} // namespace jsb

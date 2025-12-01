#include "deletion_tracker.h"
#include "object_registry.h"
#include "signal_registry.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

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

    // Note: In a full implementation, we would connect to the tree_exiting signal
    // However, GDExtension Callable limitations make this complex
    // The ObjectRegistry already validates objects via ObjectDB::get_instance
}

void DeletionTracker::untrack_node(Node* node) {
    if (!node) {
        return;
    }

    uint64_t object_id = node->get_instance_id();
    tracked_object_ids_.erase(object_id);
}

bool DeletionTracker::is_tracked(Node* node) const {
    if (!node) {
        return false;
    }
    return tracked_object_ids_.has(node->get_instance_id());
}

void DeletionTracker::clear() {
    tracked_object_ids_.clear();
}

void DeletionTracker::on_node_deleted(uint64_t object_id) {
    if (!tracked_object_ids_.has(object_id)) {
        return;
    }

    // Notify ObjectRegistry
    if (object_registry_) {
        object_registry_->mark_deleted(object_id);
    }

    // Disconnect all signals from this object
    if (signal_registry_) {
        signal_registry_->disconnect_all_from_object(object_id);
    }

    // Remove from tracking
    tracked_object_ids_.erase(object_id);
}

} // namespace jsb

#ifndef GODOT_JS_RUNTIME_DELETION_TRACKER_H
#define GODOT_JS_RUNTIME_DELETION_TRACKER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <cstdint>
#include <functional>

namespace jsb {

class ObjectRegistry;
class SignalRegistry;

// DeletionTracker monitors Node deletions via tree_exiting signal
// This provides notification before objects are deleted
class DeletionTracker {
public:
    DeletionTracker();
    ~DeletionTracker();

    void set_object_registry(ObjectRegistry* registry) { object_registry_ = registry; }
    void set_signal_registry(SignalRegistry* signal_registry) { signal_registry_ = signal_registry; }

    // Start tracking a node for deletion
    void track_node(godot::Node* node);

    // Stop tracking a node
    void untrack_node(godot::Node* node);

    // Check if a node is being tracked
    bool is_tracked(godot::Node* node) const;

    // Clear all tracked nodes
    void clear();

    // Called when a tracked node is about to be deleted
    void on_node_deleted(uint64_t object_id);

private:
    ObjectRegistry* object_registry_ = nullptr;
    SignalRegistry* signal_registry_ = nullptr;

    godot::HashSet<uint64_t> tracked_object_ids_;
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_DELETION_TRACKER_H

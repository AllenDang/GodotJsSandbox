#ifndef GODOT_JS_RUNTIME_DELETION_TRACKER_H
#define GODOT_JS_RUNTIME_DELETION_TRACKER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <cstdint>

namespace jsb {

class ObjectRegistry;
class SignalRegistry;
class DeletionTracker;

// DeletionCallback is a RefCounted helper object that can receive signal callbacks
// It stores the object_id and a pointer back to the DeletionTracker
class DeletionCallback : public godot::RefCounted {
    GDCLASS(DeletionCallback, godot::RefCounted);

public:
    DeletionCallback();
    ~DeletionCallback();

    void setup(DeletionTracker* tracker, uint64_t object_id);

    // Called when the node emits tree_exiting
    void on_tree_exiting();

    uint64_t get_object_id() const { return object_id_; }

protected:
    static void _bind_methods();

private:
    DeletionTracker* tracker_ = nullptr;
    uint64_t object_id_ = 0;
};

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

    // Called when a tracked node is about to be deleted (from DeletionCallback)
    void on_node_deleted(uint64_t object_id);

private:
    ObjectRegistry* object_registry_ = nullptr;
    SignalRegistry* signal_registry_ = nullptr;

    godot::HashSet<uint64_t> tracked_object_ids_;

    // Map from object_id to the callback ref (to keep it alive and for cleanup)
    godot::HashMap<uint64_t, godot::Ref<DeletionCallback>> callbacks_;
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_DELETION_TRACKER_H

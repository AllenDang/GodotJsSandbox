#ifndef GODOT_JS_RUNTIME_RID_REGISTRY_H
#define GODOT_JS_RUNTIME_RID_REGISTRY_H

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <cstdint>

namespace jsb {

// RidRegistry tracks RIDs created by JavaScript on RenderingDevices
// This enables automatic cleanup when the sandbox is reset
class RidRegistry {
public:
    RidRegistry();
    ~RidRegistry();

    // Register an RID created on a specific RenderingDevice
    // Called when JS creates shaders, buffers, pipelines, etc.
    void register_rid(godot::RenderingDevice* rd, const godot::RID& rid);

    // Unregister an RID (called when JS explicitly frees it)
    void unregister_rid(const godot::RID& rid);

    // Free all registered RIDs and clear the registry
    // Called on sandbox cleanup
    void free_all_rids();

    // Get count of registered RIDs
    int get_rid_count() const;

private:
    struct RidEntry {
        godot::RID rid;
        godot::RenderingDevice* device;  // The device that created this RID
        uint64_t device_object_id;       // Object ID to verify device is still valid
    };

    godot::Vector<RidEntry> rids_;
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_RID_REGISTRY_H

#include "rid_registry.h"

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

RidRegistry::RidRegistry() {
}

RidRegistry::~RidRegistry() {
    // Don't free RIDs in destructor - the devices may already be freed
    // Cleanup should be done explicitly via free_all_rids() before destruction
    rids_.clear();
}

void RidRegistry::register_rid(RenderingDevice* rd, const RID& rid) {
    if (!rd || !rid.is_valid()) {
        return;
    }

    RidEntry entry;
    entry.rid = rid;
    entry.device = rd;
    entry.device_object_id = rd->get_instance_id();

    rids_.push_back(entry);
}

void RidRegistry::unregister_rid(const RID& rid) {
    if (!rid.is_valid()) {
        return;
    }

    // Find and remove the RID
    for (int i = 0; i < rids_.size(); i++) {
        if (rids_[i].rid == rid) {
            rids_.remove_at(i);
            return;
        }
    }
}

void RidRegistry::free_all_rids() {
    // Free RIDs in reverse order (later created resources may depend on earlier ones)
    // Actually, for safety, we should free in dependency order, but since we don't track
    // dependencies, reverse order is a reasonable heuristic (pipelines created after shaders)

    for (int i = rids_.size() - 1; i >= 0; i--) {
        RidEntry& entry = rids_.write[i];

        // Verify the device is still valid
        Object* obj = ObjectDB::get_instance(ObjectID(entry.device_object_id));
        if (!obj) {
            // Device was already freed, skip
            continue;
        }

        RenderingDevice* rd = Object::cast_to<RenderingDevice>(obj);
        if (!rd) {
            continue;
        }

        // Free the RID - RenderingDevice always has free_rid method
        if (entry.rid.is_valid()) {
            rd->free_rid(entry.rid);
        }
    }

    rids_.clear();
}

int RidRegistry::get_rid_count() const {
    return rids_.size();
}

} // namespace jsb

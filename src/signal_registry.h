#ifndef GODOT_JS_RUNTIME_SIGNAL_REGISTRY_H
#define GODOT_JS_RUNTIME_SIGNAL_REGISTRY_H

#include "../quickjs/quickjs.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <cstdint>

namespace jsb {

class ObjectRegistry;

// SignalRegistry manages JavaScript callback connections to Godot signals
// Handles automatic cleanup when objects are deleted or sandbox is destroyed
class SignalRegistry {
public:
    SignalRegistry();
    ~SignalRegistry();

    void set_context(JSContext* ctx) { ctx_ = ctx; }
    void set_object_registry(ObjectRegistry* registry) { object_registry_ = registry; }

    // Connect a JS callback to a Godot signal
    // Returns connection ID (0 on failure)
    uint64_t connect(godot::Object* target, const godot::StringName& signal, JSValue callback);

    // Disconnect by connection ID
    void disconnect(uint64_t connection_id);

    // Check if a connection is still valid
    bool is_connected(uint64_t connection_id) const;

    // Disconnect all signals from a specific object (called when object is deleted)
    void disconnect_all_from_object(uint64_t object_id);

    // Clean up all connections (called when sandbox is destroyed)
    void cleanup_all();

    // Get connection count
    int get_connection_count() const;

private:
    struct SignalConnection {
        uint64_t target_object_id;    // Object ID of signal emitter
        godot::StringName signal_name;
        JSValue callback;              // JS callback function (JS_DupValue'd)
        uint64_t connection_id;
        bool connected;
    };

    JSContext* ctx_ = nullptr;
    ObjectRegistry* object_registry_ = nullptr;

    godot::HashMap<uint64_t, SignalConnection> connections_;
    godot::HashMap<uint64_t, godot::Vector<uint64_t>> connections_by_object_;  // object_id -> connection_ids
    uint64_t next_connection_id_ = 1;

    // Invoke a JS callback (called from Godot signal)
    void invoke_callback(uint64_t connection_id, const godot::Array& args);

    // Internal disconnect helper
    void disconnect_internal(uint64_t connection_id, bool from_godot);

    // Callable wrapper for Godot signals
    static void signal_callback_wrapper(void* userdata, const godot::Variant** args, int argc, godot::Variant& ret);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_SIGNAL_REGISTRY_H

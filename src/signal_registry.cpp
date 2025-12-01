#include "signal_registry.h"
#include "object_registry.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

SignalRegistry::SignalRegistry() {
}

SignalRegistry::~SignalRegistry() {
    cleanup_all();
}

uint64_t SignalRegistry::connect(Object* target, const StringName& signal, JSValue callback) {
    if (!target || !ctx_) {
        return 0;
    }

    // Check if signal exists
    if (!target->has_signal(signal)) {
        return 0;
    }

    uint64_t connection_id = next_connection_id_++;
    uint64_t object_id = target->get_instance_id();

    SignalConnection conn;
    conn.target_object_id = object_id;
    conn.signal_name = signal;
    conn.callback = JS_DupValue(ctx_, callback);  // Increment JS reference count
    conn.connection_id = connection_id;
    conn.connected = true;

    // Store connection
    connections_[connection_id] = conn;

    // Add to object's connection list
    if (!connections_by_object_.has(object_id)) {
        connections_by_object_[object_id] = Vector<uint64_t>();
    }
    connections_by_object_[object_id].push_back(connection_id);

    // Note: Actual Godot signal connection would require a Callable
    // For now, we just track the connection. The actual signal dispatch
    // would need to be implemented with custom Callable or polling mechanism.
    // A full implementation would use a custom CallableCustom.

    return connection_id;
}

void SignalRegistry::disconnect(uint64_t connection_id) {
    disconnect_internal(connection_id, false);
}

void SignalRegistry::disconnect_internal(uint64_t connection_id, bool from_godot) {
    if (!connections_.has(connection_id)) {
        return;
    }

    SignalConnection& conn = connections_[connection_id];

    if (!conn.connected) {
        return;
    }

    // Disconnect from Godot signal if not already disconnected
    if (!from_godot) {
        Object* target = ObjectDB::get_instance(ObjectID(conn.target_object_id));
        if (target) {
            // Would disconnect from actual Godot signal here
        }
    }

    // Free JS callback reference
    if (ctx_) {
        JS_FreeValue(ctx_, conn.callback);
    }

    conn.connected = false;
    conn.callback = JS_UNDEFINED;

    // Remove from object's connection list
    if (connections_by_object_.has(conn.target_object_id)) {
        Vector<uint64_t>& obj_conns = connections_by_object_[conn.target_object_id];
        int idx = obj_conns.find(connection_id);
        if (idx >= 0) {
            obj_conns.remove_at(idx);
        }
        if (obj_conns.is_empty()) {
            connections_by_object_.erase(conn.target_object_id);
        }
    }

    // Remove connection
    connections_.erase(connection_id);
}

bool SignalRegistry::is_connected(uint64_t connection_id) const {
    if (!connections_.has(connection_id)) {
        return false;
    }
    return connections_[connection_id].connected;
}

void SignalRegistry::disconnect_all_from_object(uint64_t object_id) {
    if (!connections_by_object_.has(object_id)) {
        return;
    }

    // Copy the list since we'll be modifying it
    Vector<uint64_t> conn_ids = connections_by_object_[object_id];

    for (int i = 0; i < conn_ids.size(); i++) {
        disconnect_internal(conn_ids[i], true);
    }
}

void SignalRegistry::cleanup_all() {
    // Free all JS callback references
    if (ctx_) {
        for (auto& pair : connections_) {
            SignalConnection& conn = pair.value;
            if (conn.connected) {
                JS_FreeValue(ctx_, conn.callback);
                conn.connected = false;
            }
        }
    }

    connections_.clear();
    connections_by_object_.clear();
}

int SignalRegistry::get_connection_count() const {
    return connections_.size();
}

void SignalRegistry::invoke_callback(uint64_t connection_id, const Array& args) {
    if (!ctx_ || !connections_.has(connection_id)) {
        return;
    }

    SignalConnection& conn = connections_[connection_id];
    if (!conn.connected) {
        return;
    }

    // Convert Godot args to JS args
    int argc = args.size();
    JSValue* js_args = nullptr;

    if (argc > 0) {
        js_args = static_cast<JSValue*>(js_malloc(ctx_, sizeof(JSValue) * argc));
        // Note: Would need QuickJSContext to convert Variant to JS
        // For now, this is a placeholder
        for (int i = 0; i < argc; i++) {
            js_args[i] = JS_UNDEFINED;
        }
    }

    // Call the JS function
    JSValue global = JS_GetGlobalObject(ctx_);
    JSValue result = JS_Call(ctx_, conn.callback, global, argc, js_args);
    JS_FreeValue(ctx_, global);

    // Handle exception
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx_);
        const char* msg = JS_ToCString(ctx_, exception);
        if (msg) {
            UtilityFunctions::printerr("[JS Signal Error] ", msg);
            JS_FreeCString(ctx_, msg);
        }
        JS_FreeValue(ctx_, exception);
    }

    JS_FreeValue(ctx_, result);

    // Free JS args
    if (js_args) {
        for (int i = 0; i < argc; i++) {
            JS_FreeValue(ctx_, js_args[i]);
        }
        js_free(ctx_, js_args);
    }
}

} // namespace jsb

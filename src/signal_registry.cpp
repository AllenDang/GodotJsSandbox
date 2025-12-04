#include "signal_registry.h"
#include "object_registry.h"
#include "quickjs_context.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

// ============================================================================
// JSSignalCallable implementation
// ============================================================================

JSSignalCallable::JSSignalCallable(SignalRegistry* registry, uint64_t connection_id)
    : registry_(registry), connection_id_(connection_id) {
}

uint32_t JSSignalCallable::hash() const {
    return static_cast<uint32_t>(connection_id_);
}

String JSSignalCallable::get_as_text() const {
    return String("JSSignalCallable(") + String::num_int64(connection_id_) + ")";
}

CallableCustom::CompareEqualFunc JSSignalCallable::get_compare_equal_func() const {
    return &JSSignalCallable::compare_equal;
}

CallableCustom::CompareLessFunc JSSignalCallable::get_compare_less_func() const {
    return &JSSignalCallable::compare_less;
}

bool JSSignalCallable::is_valid() const {
    return registry_ && registry_->is_connected(connection_id_);
}

ObjectID JSSignalCallable::get_object() const {
    return ObjectID();  // No associated object
}

void JSSignalCallable::call(const Variant** p_arguments, int p_argcount, Variant& r_return_value, GDExtensionCallError& r_call_error) const {
    if (registry_) {
        registry_->invoke_callback(connection_id_, p_arguments, p_argcount);
    }
    r_return_value = Variant();
    r_call_error.error = GDEXTENSION_CALL_OK;
}

bool JSSignalCallable::compare_equal(const CallableCustom* a, const CallableCustom* b) {
    const JSSignalCallable* sa = static_cast<const JSSignalCallable*>(a);
    const JSSignalCallable* sb = static_cast<const JSSignalCallable*>(b);
    return sa->connection_id_ == sb->connection_id_ && sa->registry_ == sb->registry_;
}

bool JSSignalCallable::compare_less(const CallableCustom* a, const CallableCustom* b) {
    const JSSignalCallable* sa = static_cast<const JSSignalCallable*>(a);
    const JSSignalCallable* sb = static_cast<const JSSignalCallable*>(b);
    return sa->connection_id_ < sb->connection_id_;
}

// ============================================================================
// SignalRegistry implementation
// ============================================================================

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
        UtilityFunctions::printerr("[JS] Signal '", signal, "' does not exist on target object");
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

    // Store connection first (before creating callable, which needs it)
    connections_[connection_id] = conn;

    // Add to object's connection list
    if (!connections_by_object_.has(object_id)) {
        connections_by_object_[object_id] = Vector<uint64_t>();
    }
    connections_by_object_[object_id].push_back(connection_id);

    // Create the custom callable and connect to the actual Godot signal
    JSSignalCallable* custom_callable = memnew(JSSignalCallable(this, connection_id));
    Callable callable = Callable(custom_callable);

    // Store the callable for later disconnection
    connections_[connection_id].godot_callable = callable;

    // Connect to the Godot signal
    Error err = target->connect(signal, callable);
    if (err != OK) {
        UtilityFunctions::printerr("[JS] Failed to connect to signal '", signal, "': ", err);
        // Clean up on failure
        JS_FreeValue(ctx_, conn.callback);
        connections_.erase(connection_id);
        if (connections_by_object_.has(object_id)) {
            Vector<uint64_t>& obj_conns = connections_by_object_[object_id];
            int idx = obj_conns.find(connection_id);
            if (idx >= 0) {
                obj_conns.remove_at(idx);
            }
            if (obj_conns.is_empty()) {
                connections_by_object_.erase(object_id);
            }
        }
        return 0;
    }

    return connection_id;
}

Callable SignalRegistry::create_callable(JSValue callback) {
    if (!ctx_) {
        return Callable();
    }

    uint64_t connection_id = next_connection_id_++;

    SignalConnection conn;
    conn.target_object_id = 0;  // No target object for standalone callables
    conn.signal_name = StringName();
    conn.callback = JS_DupValue(ctx_, callback);
    conn.connection_id = connection_id;
    conn.connected = true;

    connections_[connection_id] = conn;

    // Create the custom callable
    JSSignalCallable* custom_callable = memnew(JSSignalCallable(this, connection_id));
    Callable callable = Callable(custom_callable);

    // Store the callable for cleanup
    connections_[connection_id].godot_callable = callable;

    return callable;
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
        if (target && target->is_connected(conn.signal_name, conn.godot_callable)) {
            target->disconnect(conn.signal_name, conn.godot_callable);
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
    // Must be called while JSContext is still valid
    if (ctx_ && !connections_.is_empty()) {
        for (auto& pair : connections_) {
            SignalConnection& conn = pair.value;
            if (conn.connected) {
                JS_FreeValue(ctx_, conn.callback);
                conn.callback = JS_UNDEFINED;
                conn.connected = false;
            }
        }
    }

    connections_.clear();
    connections_by_object_.clear();

    // Clear context reference to prevent use-after-free on double cleanup
    ctx_ = nullptr;
}

int SignalRegistry::get_connection_count() const {
    return connections_.size();
}

void SignalRegistry::invoke_callback(uint64_t connection_id, const Variant** args, int argc) {
    if (!ctx_ || !connections_.has(connection_id)) {
        return;
    }

    SignalConnection& conn = connections_[connection_id];
    if (!conn.connected) {
        return;
    }

    // Convert Godot args to JS args using QuickJSContext
    JSValue* js_args = nullptr;

    if (argc > 0) {
        js_args = static_cast<JSValue*>(js_malloc(ctx_, sizeof(JSValue) * argc));
        for (int i = 0; i < argc; i++) {
            if (qjs_context_) {
                js_args[i] = qjs_context_->variant_to_js(*args[i]);
            } else {
                js_args[i] = JS_UNDEFINED;
            }
        }
    }

    // Call the JS function
    JSValue global = JS_GetGlobalObject(ctx_);
    JSValue result = JS_Call(ctx_, conn.callback, global, argc, js_args);
    JS_FreeValue(ctx_, global);

    // Handle exception with detailed error message
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx_);
        String error_message = "[JS Signal Error] ";

        // Get exception message
        const char* msg = JS_ToCString(ctx_, exception);
        if (msg) {
            error_message += msg;
            JS_FreeCString(ctx_, msg);
        }

        // Get stack trace
        JSValue stack = JS_GetPropertyStr(ctx_, exception, "stack");
        if (!JS_IsUndefined(stack)) {
            const char* stack_str = JS_ToCString(ctx_, stack);
            if (stack_str) {
                error_message += "\nStack trace:\n";
                error_message += stack_str;
                JS_FreeCString(ctx_, stack_str);
            }
        }
        JS_FreeValue(ctx_, stack);

        // Include signal info if available
        if (conn.signal_name.length() > 0) {
            error_message += "\nSignal: " + String(conn.signal_name);
        }

        UtilityFunctions::printerr(error_message);
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

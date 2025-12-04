#ifndef GODOT_JS_RUNTIME_SIGNAL_REGISTRY_H
#define GODOT_JS_RUNTIME_SIGNAL_REGISTRY_H

#include "../quickjs/quickjs.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/callable_custom.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <cstdint>

namespace jsb {

class ObjectRegistry;
class QuickJSContext;
class SignalRegistry;

// SignalRegistry manages JavaScript callback connections to Godot signals
// Handles automatic cleanup when objects are deleted or sandbox is destroyed
class SignalRegistry {
    friend class JSSignalCallable;  // Allow callable to invoke callbacks

public:
    SignalRegistry();
    ~SignalRegistry();

    void set_context(JSContext* ctx) { ctx_ = ctx; }
    void set_quickjs_context(QuickJSContext* qjs_ctx) { qjs_context_ = qjs_ctx; }
    void set_object_registry(ObjectRegistry* registry) { object_registry_ = registry; }

    // Connect a JS callback to a Godot signal
    // Returns connection ID (0 on failure)
    uint64_t connect(godot::Object* target, const godot::StringName& signal, JSValue callback);

    // Create a Callable from a JS callback (for use with tween_callback, etc.)
    // Returns a Callable that will invoke the JS function when called
    // The callback is stored and will be freed when cleanup_all() is called
    godot::Callable create_callable(JSValue callback);

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
        godot::Callable godot_callable; // The Callable used for Godot signal connection
        uint64_t connection_id;
        bool connected;
    };

    JSContext* ctx_ = nullptr;
    QuickJSContext* qjs_context_ = nullptr;
    ObjectRegistry* object_registry_ = nullptr;

    godot::HashMap<uint64_t, SignalConnection> connections_;
    godot::HashMap<uint64_t, godot::Vector<uint64_t>> connections_by_object_;  // object_id -> connection_ids
    uint64_t next_connection_id_ = 1;

    // Invoke a JS callback (called from Godot signal)
    void invoke_callback(uint64_t connection_id, const godot::Variant** args, int argc);

    // Internal disconnect helper
    void disconnect_internal(uint64_t connection_id, bool from_godot);
};

// Custom Callable for bridging Godot signals to JavaScript callbacks
// This allows Godot to call back into JS when a signal is emitted
class JSSignalCallable : public godot::CallableCustom {
public:
    JSSignalCallable(SignalRegistry* registry, uint64_t connection_id);
    virtual ~JSSignalCallable() override = default;

    // CallableCustom interface
    virtual uint32_t hash() const override;
    virtual godot::String get_as_text() const override;
    virtual CompareEqualFunc get_compare_equal_func() const override;
    virtual CompareLessFunc get_compare_less_func() const override;
    virtual bool is_valid() const override;
    virtual godot::ObjectID get_object() const override;
    virtual void call(const godot::Variant** p_arguments, int p_argcount, godot::Variant& r_return_value, GDExtensionCallError& r_call_error) const override;

private:
    SignalRegistry* registry_;
    uint64_t connection_id_;

    static bool compare_equal(const CallableCustom* a, const CallableCustom* b);
    static bool compare_less(const CallableCustom* a, const CallableCustom* b);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_SIGNAL_REGISTRY_H

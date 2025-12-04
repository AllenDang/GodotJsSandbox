#ifndef GODOT_JS_RUNTIME_GODOT_BINDINGS_H
#define GODOT_JS_RUNTIME_GODOT_BINDINGS_H

#include "../quickjs/quickjs.h"
#include <godot_cpp/variant/variant.hpp>

namespace jsb {

class QuickJSContext;
class ObjectRegistry;

// Data stored in GodotObject JS wrapper opaque pointer
// Contains both the handle and the registry that owns it
// This allows the finalizer to work correctly even with multiple contexts sharing a runtime
struct GodotObjectData {
    uint64_t handle;
    ObjectRegistry* registry;
};

// GodotBindings sets up JavaScript bindings for Godot classes
// This allows JS code to create Godot objects, call methods, and access properties
class GodotBindings {
public:
    explicit GodotBindings(QuickJSContext* context);
    ~GodotBindings();

    // Initialize all bindings in the context
    bool initialize();

    // Cleanup bindings
    void shutdown();

    // Get class ID for GodotObject wrapper
    JSClassID get_godot_object_class_id() const { return godot_object_class_id_; }

    // Convert Variant to JSValue (uses context's type conversion)
    JSValue variant_to_js(const godot::Variant& value);

    // Convert JSValue to Variant
    godot::Variant js_to_variant(JSValue value);

    // Helper to get QuickJSContext from JSContext (public for generated bindings)
    static QuickJSContext* get_context(JSContext* ctx);

private:
    QuickJSContext* context_;
    JSClassID godot_object_class_id_ = 0;

    // Setup core JavaScript classes and functions
    void setup_global_functions();
    void setup_math_types();
    void setup_godot_class_constructor();
    void setup_proxy_handler();

    // Register common Godot classes
    void register_node_classes();
    void register_resource_classes();

    // JS callback implementations
    static JSValue js_godot_new(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_load(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_connect(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_emit_signal(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_await_signal(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_tween_callback(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_tween_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_call_script_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_has_script_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

    // GodotObject class callbacks
    static void godot_object_finalizer(JSRuntime* rt, JSValueConst val);

    // Math type constructors
    static JSValue js_vector2_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
    static JSValue js_vector3_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
    static JSValue js_color_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
    static JSValue js_quaternion_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
    static JSValue js_basis_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
    static JSValue js_transform3d_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_GODOT_BINDINGS_H

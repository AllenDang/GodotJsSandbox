#ifndef GODOT_JS_RUNTIME_GODOT_BINDINGS_H
#define GODOT_JS_RUNTIME_GODOT_BINDINGS_H

#include "quickjs.h"
#include <godot_cpp/variant/variant.hpp>

namespace jsb {

class QuickJSContext;
class ObjectRegistry;
class ArrayRegistry;

// Data stored in GodotObject JS wrapper opaque pointer
// Contains both the handle and the registry that owns it
// This allows the finalizer to work correctly even with multiple contexts sharing a runtime
struct GodotObjectData {
    uint64_t handle;
    ObjectRegistry* registry;
};

// Data stored in GodotArray/PackedArray JS wrapper opaque pointer
// Used by finalizer to release array handles when JS object is garbage collected
struct GodotArrayData {
    uint64_t handle;
    ArrayRegistry* registry;
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
    static void shutdown();

    // Get class ID for GodotObject wrapper
    static JSClassID get_godot_object_class_id() { return godot_object_class_id_; }

    // Get class ID for GodotArray wrapper (used for finalizer-based cleanup)
    static JSClassID get_godot_array_class_id() { return godot_array_class_id_; }

    // Convert Variant to JSValue (uses context's type conversion)
    JSValue variant_to_js(const godot::Variant& value);

    // Convert JSValue to Variant
    godot::Variant js_to_variant(JSValue value);

    // Helper to get QuickJSContext from JSContext (public for generated bindings)
    static QuickJSContext* get_context(JSContext* ctx);

private:
    QuickJSContext* context_;

    // Class IDs are static because they must be shared across all contexts using
    // the same JSRuntime. JS_NewClassID() only allocates a new ID when the value is 0,
    // so subsequent calls with the same static variable are no-ops.
    static JSClassID godot_object_class_id_;
    static JSClassID godot_array_class_id_;  // For Array/PackedArray handles with finalizer

    // Setup core JavaScript classes and functions
    void setup_global_functions();
    void setup_math_types();
    void setup_godot_class_constructor();
    static void setup_proxy_handler();

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
    static JSValue js_godot_has_signal(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

    // Generic property/method access (for objects without pre-generated bindings, like GDScript)
    static JSValue js_godot_get(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_set(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_call(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_has_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

    // Array proxy functions
    static JSValue js_godot_array_get(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_array_set(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_array_size(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_array_push(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_godot_array_pop(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

    // Packed array proxy functions (zero-copy access)
    static JSValue js_packed_array_get(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_packed_array_size(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_packed_array_type(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_packed_array_push(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_packed_array_resize(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_packed_array_set(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

    // Math type proxy functions (zero-copy access)
    static JSValue js_math_get_property(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_math_set_property(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_math_get_type(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

    // GodotObject class callbacks
    static void godot_object_finalizer(JSRuntime* rt, JSValueConst val);

    // GodotArray class callbacks (for Array/PackedArray handles)
    static void godot_array_finalizer(JSRuntime* rt, JSValueConst val);

    // NOTE: Math type constructors and packed array constructors are now generated
    // See generated/math_constructors.gen.cpp and generated/packed_array_bindings.gen.cpp
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_GODOT_BINDINGS_H

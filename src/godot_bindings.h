#ifndef GODOT_JS_RUNTIME_GODOT_BINDINGS_H
#define GODOT_JS_RUNTIME_GODOT_BINDINGS_H

#include "../quickjs/quickjs.h"
#include <godot_cpp/variant/variant.hpp>

namespace jsb {

class QuickJSContext;

// GodotBindings sets up JavaScript bindings for Godot classes
// This allows JS code to create Godot objects, call methods, and access properties
class GodotBindings {
public:
    GodotBindings(QuickJSContext* context);
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

private:
    QuickJSContext* context_;
    JSClassID godot_object_class_id_ = 0;

    // Setup core JavaScript classes and functions
    void setup_global_functions();
    void setup_math_types();
    void setup_godot_class_constructor();

    // Register common Godot classes
    void register_node_classes();
    void register_resource_classes();

    // JS callback implementations
    static JSValue js_godot_new(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
    static JSValue js_load(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

    // GodotObject class callbacks
    static void godot_object_finalizer(JSRuntime* rt, JSValue val);
    static JSValue godot_object_get_property(JSContext* ctx, JSValueConst obj, JSAtom atom, JSValueConst receiver);
    static int godot_object_set_property(JSContext* ctx, JSValueConst obj, JSAtom atom, JSValueConst value, JSValueConst receiver, int flags);
    static JSValue godot_object_call_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data);

    // Math type constructors
    static JSValue js_vector2_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
    static JSValue js_vector3_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
    static JSValue js_color_constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);

    // Helper to get QuickJSContext from JSContext
    static QuickJSContext* get_context(JSContext* ctx);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_GODOT_BINDINGS_H

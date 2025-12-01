#include "godot_bindings.h"
#include "quickjs_context.h"
#include "safe_wrapper.h"
#include "object_registry.h"
#include "sandbox_config.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/sprite3d.hpp>
#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/static_body2d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

// Store context pointer in JSRuntime opaque
GodotBindings::GodotBindings(QuickJSContext* context)
    : context_(context) {
}

GodotBindings::~GodotBindings() {
    shutdown();
}

QuickJSContext* GodotBindings::get_context(JSContext* ctx) {
    JSRuntime* rt = JS_GetRuntime(ctx);
    return static_cast<QuickJSContext*>(JS_GetRuntimeOpaque(rt));
}

bool GodotBindings::initialize() {
    if (!context_ || !context_->ctx()) {
        return false;
    }

    JSContext* ctx = context_->ctx();
    JSRuntime* rt = context_->rt();

    // Store context pointer in runtime for callbacks
    JS_SetRuntimeOpaque(rt, context_);

    // Create GodotObject class with exotic handlers
    JS_NewClassID(&godot_object_class_id_);

    JSClassExoticMethods exotic = {};
    exotic.get_property = godot_object_get_property;
    exotic.set_property = godot_object_set_property;

    JSClassDef godot_class_def = {};
    godot_class_def.class_name = "GodotObject";
    godot_class_def.finalizer = godot_object_finalizer;
    godot_class_def.exotic = &exotic;

    JS_NewClass(rt, godot_object_class_id_, &godot_class_def);

    // Set up prototype with common methods
    JSValue proto = JS_NewObject(ctx);
    JS_SetClassProto(ctx, godot_object_class_id_, proto);

    // Setup bindings
    setup_global_functions();
    setup_math_types();
    setup_godot_class_constructor();

    return true;
}

void GodotBindings::shutdown() {
    godot_object_class_id_ = 0;
}

// Time singleton wrapper functions
static JSValue js_time_get_ticks_msec(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewInt64(ctx, Time::get_singleton()->get_ticks_msec());
}

static JSValue js_time_get_ticks_usec(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewInt64(ctx, Time::get_singleton()->get_ticks_usec());
}

static JSValue js_time_get_unix_time(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewFloat64(ctx, Time::get_singleton()->get_unix_time_from_system());
}

void GodotBindings::setup_global_functions() {
    JSContext* ctx = context_->ctx();
    JSValue global = JS_GetGlobalObject(ctx);

    // load() function for resources
    JS_SetPropertyStr(ctx, global, "load",
        JS_NewCFunction(ctx, js_load, "load", 1));

    // Time singleton (safe subset of methods)
    JSValue time_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, time_obj, "get_ticks_msec",
        JS_NewCFunction(ctx, js_time_get_ticks_msec, "get_ticks_msec", 0));
    JS_SetPropertyStr(ctx, time_obj, "get_ticks_usec",
        JS_NewCFunction(ctx, js_time_get_ticks_usec, "get_ticks_usec", 0));
    JS_SetPropertyStr(ctx, time_obj, "get_unix_time_from_system",
        JS_NewCFunction(ctx, js_time_get_unix_time, "get_unix_time_from_system", 0));
    JS_SetPropertyStr(ctx, global, "Time", time_obj);

    JS_FreeValue(ctx, global);
}

void GodotBindings::setup_math_types() {
    JSContext* ctx = context_->ctx();
    JSValue global = JS_GetGlobalObject(ctx);

    // Vector2 constructor
    JSValue vector2_ctor = JS_NewCFunction2(ctx, js_vector2_constructor, "Vector2", 2,
                                            JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Vector2", vector2_ctor);

    // Vector3 constructor
    JSValue vector3_ctor = JS_NewCFunction2(ctx, js_vector3_constructor, "Vector3", 3,
                                            JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Vector3", vector3_ctor);

    // Color constructor
    JSValue color_ctor = JS_NewCFunction2(ctx, js_color_constructor, "Color", 4,
                                          JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Color", color_ctor);

    JS_FreeValue(ctx, global);
}

void GodotBindings::setup_godot_class_constructor() {
    JSContext* ctx = context_->ctx();
    JSValue global = JS_GetGlobalObject(ctx);

    // Create constructor function for common Godot classes
    // Format: ClassName = function() { return __godot_new("ClassName"); }

    const char* class_names[] = {
        // Core nodes
        "Node", "Node2D", "Node3D",
        // 2D nodes
        "Sprite2D", "Camera2D", "Area2D", "RigidBody2D", "StaticBody2D",
        "CharacterBody2D", "CollisionShape2D",
        // 3D nodes
        "Sprite3D", "Camera3D", "Area3D", "RigidBody3D", "StaticBody3D",
        "CharacterBody3D", "CollisionShape3D",
        // UI nodes
        "Control", "Label", "Button",
        // Utility nodes
        "Timer", "AudioStreamPlayer",
        nullptr
    };

    // Register __godot_new function
    JS_SetPropertyStr(ctx, global, "__godot_new",
        JS_NewCFunction(ctx, js_godot_new, "__godot_new", 1));

    // Create constructor for each class
    for (int i = 0; class_names[i] != nullptr; i++) {
        String class_name = class_names[i];

        // Create a constructor function
        String code = "function " + class_name + "() { return __godot_new('" + class_name + "'); }";
        JSValue result = JS_Eval(ctx, code.utf8().get_data(), code.utf8().length(),
                                 "<builtin>", JS_EVAL_TYPE_GLOBAL);
        JS_FreeValue(ctx, result);
    }

    JS_FreeValue(ctx, global);
}

// Global function: __godot_new(class_name) - creates a new Godot object
JSValue GodotBindings::js_godot_new(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected class name");
    }

    const char* class_name_cstr = JS_ToCString(ctx, argv[0]);
    if (!class_name_cstr) {
        return JS_ThrowTypeError(ctx, "Invalid class name");
    }

    String class_name = class_name_cstr;
    JS_FreeCString(ctx, class_name_cstr);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    SafeWrapper* wrapper = qjs_ctx->get_safe_wrapper();
    if (!wrapper) {
        return JS_ThrowInternalError(ctx, "SafeWrapper not initialized");
    }

    String error;
    uint64_t handle = wrapper->create_object(StringName(class_name), error);

    if (handle == 0) {
        return JS_ThrowTypeError(ctx, "%s", error.utf8().get_data());
    }

    // Create JS wrapper object
    GodotBindings* bindings = qjs_ctx->get_bindings();
    if (!bindings) {
        return JS_ThrowInternalError(ctx, "Bindings not initialized");
    }

    JSValue obj = JS_NewObjectClass(ctx, bindings->get_godot_object_class_id());
    JS_SetOpaque(obj, reinterpret_cast<void*>(handle));

    return obj;
}

// Global function: load(path) - loads a resource
JSValue GodotBindings::js_load(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "Expected resource path");
    }

    const char* path_cstr = JS_ToCString(ctx, argv[0]);
    if (!path_cstr) {
        return JS_ThrowTypeError(ctx, "Invalid path");
    }

    String path = path_cstr;
    JS_FreeCString(ctx, path_cstr);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    SafeWrapper* wrapper = qjs_ctx->get_safe_wrapper();
    if (!wrapper) {
        return JS_ThrowInternalError(ctx, "SafeWrapper not initialized");
    }

    String error;
    Variant result = wrapper->load_resource(path, error);

    if (!error.is_empty()) {
        return JS_ThrowTypeError(ctx, "%s", error.utf8().get_data());
    }

    return qjs_ctx->variant_to_js(result);
}

// GodotObject finalizer - called when JS object is garbage collected
void GodotBindings::godot_object_finalizer(JSRuntime* rt, JSValue val) {
    void* ptr = JS_GetOpaque(val, 0);  // Class ID doesn't matter for GetOpaque
    if (!ptr) return;

    uint64_t handle = reinterpret_cast<uint64_t>(ptr);

    QuickJSContext* ctx = static_cast<QuickJSContext*>(JS_GetRuntimeOpaque(rt));
    if (ctx && ctx->get_object_registry()) {
        ctx->get_object_registry()->release_handle(handle);
    }
}

// Exotic property getter - intercepts all property access on GodotObject
JSValue GodotBindings::godot_object_get_property(JSContext* ctx, JSValueConst obj, JSAtom atom, JSValueConst receiver) {
    void* ptr = JS_GetOpaque(obj, 0);
    if (!ptr) {
        return JS_ThrowTypeError(ctx, "Invalid GodotObject");
    }

    uint64_t handle = reinterpret_cast<uint64_t>(ptr);

    const char* prop_name = JS_AtomToCString(ctx, atom);
    if (!prop_name) {
        return JS_UNDEFINED;
    }

    String property = prop_name;
    JS_FreeCString(ctx, prop_name);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    SafeWrapper* wrapper = qjs_ctx->get_safe_wrapper();
    ObjectRegistry* registry = qjs_ctx->get_object_registry();

    if (!wrapper || !registry) {
        return JS_ThrowInternalError(ctx, "SafeWrapper or ObjectRegistry not initialized");
    }

    Object* gd_obj = registry->get_object(handle);
    if (!gd_obj) {
        return JS_ThrowTypeError(ctx, "Object has been freed");
    }

    // Check if it's a method call (return a callable)
    if (gd_obj->has_method(StringName(property))) {
        // Return a function that calls the method
        JSValue func_data[2];
        func_data[0] = JS_NewInt64(ctx, handle);
        func_data[1] = JS_NewString(ctx, property.utf8().get_data());

        JSValue func = JS_NewCFunctionData(ctx, godot_object_call_method, 0, 0, 2, func_data);

        JS_FreeValue(ctx, func_data[0]);
        JS_FreeValue(ctx, func_data[1]);

        return func;
    }

    // It's a property access
    String error;
    Variant value = wrapper->get_property(handle, StringName(property), error);

    if (!error.is_empty()) {
        return JS_ThrowTypeError(ctx, "%s", error.utf8().get_data());
    }

    return qjs_ctx->variant_to_js(value);
}

// Exotic property setter - intercepts all property writes on GodotObject
int GodotBindings::godot_object_set_property(JSContext* ctx, JSValueConst obj, JSAtom atom,
                                              JSValueConst value, JSValueConst receiver, int flags) {
    void* ptr = JS_GetOpaque(obj, 0);
    if (!ptr) {
        JS_ThrowTypeError(ctx, "Invalid GodotObject");
        return -1;
    }

    uint64_t handle = reinterpret_cast<uint64_t>(ptr);

    const char* prop_name = JS_AtomToCString(ctx, atom);
    if (!prop_name) {
        return -1;
    }

    String property = prop_name;
    JS_FreeCString(ctx, prop_name);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        JS_ThrowInternalError(ctx, "Context not initialized");
        return -1;
    }

    SafeWrapper* wrapper = qjs_ctx->get_safe_wrapper();
    if (!wrapper) {
        JS_ThrowInternalError(ctx, "SafeWrapper not initialized");
        return -1;
    }

    Variant gd_value = qjs_ctx->js_to_variant(value);

    String error;
    bool success = wrapper->set_property(handle, StringName(property), gd_value, error);

    if (!success) {
        JS_ThrowTypeError(ctx, "%s", error.utf8().get_data());
        return -1;
    }

    return 0;
}

// Method call handler - called when a method on GodotObject is invoked
JSValue GodotBindings::godot_object_call_method(JSContext* ctx, JSValueConst this_val,
                                                 int argc, JSValueConst* argv,
                                                 int magic, JSValue* func_data) {
    int64_t handle;
    JS_ToInt64(ctx, &handle, func_data[0]);

    const char* method_name = JS_ToCString(ctx, func_data[1]);
    if (!method_name) {
        return JS_ThrowTypeError(ctx, "Invalid method name");
    }

    String method = method_name;
    JS_FreeCString(ctx, method_name);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    SafeWrapper* wrapper = qjs_ctx->get_safe_wrapper();
    if (!wrapper) {
        return JS_ThrowInternalError(ctx, "SafeWrapper not initialized");
    }

    // Convert JS arguments to Variant
    Vector<Variant> args;
    args.resize(argc);
    for (int i = 0; i < argc; i++) {
        args.write[i] = qjs_ctx->js_to_variant(argv[i]);
    }

    // Create array of pointers
    Vector<const Variant*> arg_ptrs;
    arg_ptrs.resize(argc);
    for (int i = 0; i < argc; i++) {
        arg_ptrs.write[i] = &args[i];
    }

    String error;
    const Variant** args_ptr = const_cast<const Variant**>(arg_ptrs.ptrw());
    Variant result = wrapper->call_method(handle, StringName(method),
                                          args_ptr, argc, error);

    if (!error.is_empty()) {
        return JS_ThrowTypeError(ctx, "%s", error.utf8().get_data());
    }

    return qjs_ctx->variant_to_js(result);
}

// Vector2 constructor
JSValue GodotBindings::js_vector2_constructor(JSContext* ctx, JSValueConst new_target,
                                               int argc, JSValueConst* argv) {
    double x = 0, y = 0;

    if (argc >= 1) JS_ToFloat64(ctx, &x, argv[0]);
    if (argc >= 2) JS_ToFloat64(ctx, &y, argv[1]);

    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, y));

    return obj;
}

// Vector3 constructor
JSValue GodotBindings::js_vector3_constructor(JSContext* ctx, JSValueConst new_target,
                                               int argc, JSValueConst* argv) {
    double x = 0, y = 0, z = 0;

    if (argc >= 1) JS_ToFloat64(ctx, &x, argv[0]);
    if (argc >= 2) JS_ToFloat64(ctx, &y, argv[1]);
    if (argc >= 3) JS_ToFloat64(ctx, &z, argv[2]);

    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, y));
    JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, z));

    return obj;
}

// Color constructor
JSValue GodotBindings::js_color_constructor(JSContext* ctx, JSValueConst new_target,
                                             int argc, JSValueConst* argv) {
    double r = 0, g = 0, b = 0, a = 1;

    if (argc >= 1) JS_ToFloat64(ctx, &r, argv[0]);
    if (argc >= 2) JS_ToFloat64(ctx, &g, argv[1]);
    if (argc >= 3) JS_ToFloat64(ctx, &b, argv[2]);
    if (argc >= 4) JS_ToFloat64(ctx, &a, argv[3]);

    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "r", JS_NewFloat64(ctx, r));
    JS_SetPropertyStr(ctx, obj, "g", JS_NewFloat64(ctx, g));
    JS_SetPropertyStr(ctx, obj, "b", JS_NewFloat64(ctx, b));
    JS_SetPropertyStr(ctx, obj, "a", JS_NewFloat64(ctx, a));

    return obj;
}

JSValue GodotBindings::variant_to_js(const Variant& value) {
    return context_->variant_to_js(value);
}

Variant GodotBindings::js_to_variant(JSValue value) {
    return context_->js_to_variant(value);
}

} // namespace jsb

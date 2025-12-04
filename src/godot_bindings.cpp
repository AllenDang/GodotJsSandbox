#include "godot_bindings.h"
#include "quickjs_context.h"
#include "safe_wrapper.h"
#include "object_registry.h"
#include "sandbox_config.h"
#include "signal_registry.h"
#include "js_script.h"
#include "js_script_instance.h"
#include "generated_classes.gen.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/callback_tweener.hpp>
#include <godot_cpp/classes/method_tweener.hpp>
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
    return static_cast<QuickJSContext*>(JS_GetContextOpaque(ctx));
}

bool GodotBindings::initialize() {
    if (!context_ || !context_->ctx()) {
        return false;
    }

    JSContext* ctx = context_->ctx();
    JSRuntime* rt = context_->rt();

    // Store context pointer in context opaque for callbacks
    // NOTE: We use context opaque, not runtime opaque, because multiple
    // QuickJSContext instances share the same runtime via JSRuntimeManager
    JS_SetContextOpaque(ctx, context_);

    // Create GodotObject class (simple class without exotic handlers)
    JS_NewClassID(rt, &godot_object_class_id_);

    JSClassDef godot_class_def = {};
    godot_class_def.class_name = "GodotObject";
    godot_class_def.finalizer = godot_object_finalizer;
    godot_class_def.exotic = nullptr;

    JS_NewClass(rt, godot_object_class_id_, &godot_class_def);

    // Set up prototype with common methods
    JSValue proto = JS_NewObject(ctx);
    JS_SetClassProto(ctx, godot_object_class_id_, proto);

    // Setup bindings
    setup_global_functions();
    setup_math_types();

    // Register all generated class bindings (creates __godot_classes registry)
    JSValue global = JS_GetGlobalObject(ctx);
    generated::register_all_classes(ctx, global);
    JS_FreeValue(ctx, global);

    setup_proxy_handler();  // No-op, but kept for structure
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

// Input singleton wrapper functions
static JSValue js_input_is_anything_pressed(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewBool(ctx, Input::get_singleton()->is_anything_pressed());
}

static JSValue js_input_is_key_pressed(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "is_key_pressed: missing keycode argument");
    int64_t keycode;
    JS_ToInt64(ctx, &keycode, argv[0]);
    return JS_NewBool(ctx, Input::get_singleton()->is_key_pressed((Key)keycode));
}

static JSValue js_input_is_mouse_button_pressed(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "is_mouse_button_pressed: missing button argument");
    int64_t button;
    JS_ToInt64(ctx, &button, argv[0]);
    return JS_NewBool(ctx, Input::get_singleton()->is_mouse_button_pressed((MouseButton)button));
}

static JSValue js_input_is_action_pressed(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "is_action_pressed: missing action argument");
    const char* action = JS_ToCString(ctx, argv[0]);
    bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
    bool result = Input::get_singleton()->is_action_pressed(StringName(action), exact_match);
    JS_FreeCString(ctx, action);
    return JS_NewBool(ctx, result);
}

static JSValue js_input_is_action_just_pressed(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "is_action_just_pressed: missing action argument");
    const char* action = JS_ToCString(ctx, argv[0]);
    bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
    bool result = Input::get_singleton()->is_action_just_pressed(StringName(action), exact_match);
    JS_FreeCString(ctx, action);
    return JS_NewBool(ctx, result);
}

static JSValue js_input_is_action_just_released(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "is_action_just_released: missing action argument");
    const char* action = JS_ToCString(ctx, argv[0]);
    bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
    bool result = Input::get_singleton()->is_action_just_released(StringName(action), exact_match);
    JS_FreeCString(ctx, action);
    return JS_NewBool(ctx, result);
}

static JSValue js_input_get_action_strength(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "get_action_strength: missing action argument");
    const char* action = JS_ToCString(ctx, argv[0]);
    bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
    double result = Input::get_singleton()->get_action_strength(StringName(action), exact_match);
    JS_FreeCString(ctx, action);
    return JS_NewFloat64(ctx, result);
}

static JSValue js_input_get_axis(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_ThrowTypeError(ctx, "get_axis: missing action arguments");
    const char* neg_action = JS_ToCString(ctx, argv[0]);
    const char* pos_action = JS_ToCString(ctx, argv[1]);
    double result = Input::get_singleton()->get_axis(StringName(neg_action), StringName(pos_action));
    JS_FreeCString(ctx, neg_action);
    JS_FreeCString(ctx, pos_action);
    return JS_NewFloat64(ctx, result);
}

static JSValue js_input_get_vector(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 4) return JS_ThrowTypeError(ctx, "get_vector: missing action arguments");
    const char* neg_x = JS_ToCString(ctx, argv[0]);
    const char* pos_x = JS_ToCString(ctx, argv[1]);
    const char* neg_y = JS_ToCString(ctx, argv[2]);
    const char* pos_y = JS_ToCString(ctx, argv[3]);
    double deadzone = argc > 4 ? 0.0 : -1.0;
    if (argc > 4) JS_ToFloat64(ctx, &deadzone, argv[4]);
    Vector2 result = Input::get_singleton()->get_vector(
        StringName(neg_x), StringName(pos_x), StringName(neg_y), StringName(pos_y), deadzone);
    JS_FreeCString(ctx, neg_x);
    JS_FreeCString(ctx, pos_x);
    JS_FreeCString(ctx, neg_y);
    JS_FreeCString(ctx, pos_y);
    JSValue ret = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret, "x", JS_NewFloat64(ctx, result.x));
    JS_SetPropertyStr(ctx, ret, "y", JS_NewFloat64(ctx, result.y));
    return ret;
}

static JSValue js_input_get_last_mouse_velocity(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    Vector2 vel = Input::get_singleton()->get_last_mouse_velocity();
    JSValue ret = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret, "x", JS_NewFloat64(ctx, vel.x));
    JS_SetPropertyStr(ctx, ret, "y", JS_NewFloat64(ctx, vel.y));
    return ret;
}

static JSValue js_input_get_mouse_button_mask(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return JS_NewInt64(ctx, (int64_t)Input::get_singleton()->get_mouse_button_mask());
}

void GodotBindings::setup_global_functions() {
    JSContext* ctx = context_->ctx();
    JSValue global = JS_GetGlobalObject(ctx);

    // load() function for resources
    JS_SetPropertyStr(ctx, global, "load",
        JS_NewCFunction(ctx, js_load, "load", 1));

    // __godot_connect(handle, signal_name, callback) - connect JS callback to Godot signal
    JS_SetPropertyStr(ctx, global, "__godot_connect",
        JS_NewCFunction(ctx, js_godot_connect, "__godot_connect", 3));

    // __godot_emit_signal(handle, signal_name, ...args) - emit a signal from an object
    JS_SetPropertyStr(ctx, global, "__godot_emit_signal",
        JS_NewCFunction(ctx, js_godot_emit_signal, "__godot_emit_signal", 2));

    // __godot_await_signal(handle, signal_name) - returns a Promise that resolves when the signal fires
    JS_SetPropertyStr(ctx, global, "__godot_await_signal",
        JS_NewCFunction(ctx, js_godot_await_signal, "__godot_await_signal", 2));

    // __godot_tween_callback(tween_handle, callback) - add a callback to a tween
    JS_SetPropertyStr(ctx, global, "__godot_tween_callback",
        JS_NewCFunction(ctx, js_godot_tween_callback, "__godot_tween_callback", 2));

    // __godot_tween_method(tween_handle, callback, from, to, duration) - add a method tween
    JS_SetPropertyStr(ctx, global, "__godot_tween_method",
        JS_NewCFunction(ctx, js_godot_tween_method, "__godot_tween_method", 5));

    // __godot_call_script_method(handle, method_name, ...args) - call JS script method on object
    JS_SetPropertyStr(ctx, global, "__godot_call_script_method",
        JS_NewCFunction(ctx, js_godot_call_script_method, "__godot_call_script_method", 2));

    // __godot_has_script_method(handle, method_name) - check if object has JS script method
    JS_SetPropertyStr(ctx, global, "__godot_has_script_method",
        JS_NewCFunction(ctx, js_godot_has_script_method, "__godot_has_script_method", 2));

    // Time singleton (safe subset of methods)
    JSValue time_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, time_obj, "get_ticks_msec",
        JS_NewCFunction(ctx, js_time_get_ticks_msec, "get_ticks_msec", 0));
    JS_SetPropertyStr(ctx, time_obj, "get_ticks_usec",
        JS_NewCFunction(ctx, js_time_get_ticks_usec, "get_ticks_usec", 0));
    JS_SetPropertyStr(ctx, time_obj, "get_unix_time_from_system",
        JS_NewCFunction(ctx, js_time_get_unix_time, "get_unix_time_from_system", 0));
    JS_SetPropertyStr(ctx, global, "Time", time_obj);

    // Input singleton (safe subset of methods)
    JSValue input_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, input_obj, "is_anything_pressed",
        JS_NewCFunction(ctx, js_input_is_anything_pressed, "is_anything_pressed", 0));
    JS_SetPropertyStr(ctx, input_obj, "is_key_pressed",
        JS_NewCFunction(ctx, js_input_is_key_pressed, "is_key_pressed", 1));
    JS_SetPropertyStr(ctx, input_obj, "is_mouse_button_pressed",
        JS_NewCFunction(ctx, js_input_is_mouse_button_pressed, "is_mouse_button_pressed", 1));
    JS_SetPropertyStr(ctx, input_obj, "is_action_pressed",
        JS_NewCFunction(ctx, js_input_is_action_pressed, "is_action_pressed", 2));
    JS_SetPropertyStr(ctx, input_obj, "is_action_just_pressed",
        JS_NewCFunction(ctx, js_input_is_action_just_pressed, "is_action_just_pressed", 2));
    JS_SetPropertyStr(ctx, input_obj, "is_action_just_released",
        JS_NewCFunction(ctx, js_input_is_action_just_released, "is_action_just_released", 2));
    JS_SetPropertyStr(ctx, input_obj, "get_action_strength",
        JS_NewCFunction(ctx, js_input_get_action_strength, "get_action_strength", 2));
    JS_SetPropertyStr(ctx, input_obj, "get_axis",
        JS_NewCFunction(ctx, js_input_get_axis, "get_axis", 2));
    JS_SetPropertyStr(ctx, input_obj, "get_vector",
        JS_NewCFunction(ctx, js_input_get_vector, "get_vector", 5));
    JS_SetPropertyStr(ctx, input_obj, "get_last_mouse_velocity",
        JS_NewCFunction(ctx, js_input_get_last_mouse_velocity, "get_last_mouse_velocity", 0));
    JS_SetPropertyStr(ctx, input_obj, "get_mouse_button_mask",
        JS_NewCFunction(ctx, js_input_get_mouse_button_mask, "get_mouse_button_mask", 0));
    JS_SetPropertyStr(ctx, global, "Input", input_obj);

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

    // Quaternion constructor
    JSValue quat_ctor = JS_NewCFunction2(ctx, js_quaternion_constructor, "Quaternion", 4,
                                         JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Quaternion", quat_ctor);

    // Basis constructor
    JSValue basis_ctor = JS_NewCFunction2(ctx, js_basis_constructor, "Basis", 0,
                                          JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Basis", basis_ctor);

    // Transform3D constructor
    JSValue transform3d_ctor = JS_NewCFunction2(ctx, js_transform3d_constructor, "Transform3D", 0,
                                                 JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Transform3D", transform3d_ctor);

    JS_FreeValue(ctx, global);
}

void GodotBindings::setup_godot_class_constructor() {
    JSContext* ctx = context_->ctx();
    JSValue global = JS_GetGlobalObject(ctx);

    // Register __godot_new function
    JS_SetPropertyStr(ctx, global, "__godot_new",
        JS_NewCFunction(ctx, js_godot_new, "__godot_new", 1));

    // Step 1: Create helper function
    const char* helper_code = R"(
        // Helper to find method/property in class hierarchy
        globalThis.__findBinding = function(className, prop, type) {
            var current = className;
            while (current && __godot_classes[current]) {
                var classInfo = __godot_classes[current];
                if (type === 'method' && classInfo.methods && classInfo.methods[prop]) {
                    return classInfo.methods[prop];
                }
                if (type === 'property' && classInfo.properties && classInfo.properties[prop]) {
                    return classInfo.properties[prop];
                }
                current = classInfo.parent;
            }
            return null;
        };
        'helper done';
    )";

    JSValue result = JS_Eval(ctx, helper_code, strlen(helper_code), "<helper>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx);
        const char* err = JS_ToCString(ctx, exception);
        UtilityFunctions::printerr("Failed to setup JS helper: ", err ? err : "unknown");
        if (err) JS_FreeCString(ctx, err);
        JS_FreeValue(ctx, exception);
        JS_FreeValue(ctx, result);
        JS_FreeValue(ctx, global);
        return;
    }
    JS_FreeValue(ctx, result);

    // Step 2: Create proxy handler
    const char* proxy_code = R"(
        globalThis.__godot_proxy_handler = {
            get: function(target, prop, receiver) {
                if (prop === '__handle' || prop === '__class') return target[prop];
                if (typeof prop === 'symbol') {
                    if (prop === Symbol.toStringTag) return 'GodotObject';
                    return undefined;
                }
                if (prop === 'toString') {
                    return function() { return '[GodotObject ' + target.__class + ']'; };
                }

                // Special handling for connect() - uses SignalRegistry for JS callbacks
                if (prop === 'connect') {
                    var handle = target.__handle;
                    return function(signalName, callback) {
                        return __godot_connect(handle, signalName, callback);
                    };
                }

                // Special handling for emit_signal() - emit custom signals
                if (prop === 'emit_signal') {
                    var handle = target.__handle;
                    return function(signalName) {
                        var args = [handle, signalName];
                        for (var i = 1; i < arguments.length; i++) {
                            var arg = arguments[i];
                            if (arg && typeof arg === 'object' && arg.__handle !== undefined) {
                                args.push(arg.__handle);
                            } else {
                                args.push(arg);
                            }
                        }
                        return __godot_emit_signal.apply(null, args);
                    };
                }

                // Special handling for await_signal() - returns a Promise that resolves when signal fires
                if (prop === 'await_signal') {
                    var handle = target.__handle;
                    return function(signalName) {
                        return __godot_await_signal(handle, signalName);
                    };
                }

                // Special handling for tween_callback() - for Tween objects
                if (prop === 'tween_callback') {
                    var handle = target.__handle;
                    return function(callback) {
                        return __godot_tween_callback(handle, callback);
                    };
                }

                // Special handling for tween_method() - for Tween objects
                if (prop === 'tween_method') {
                    var handle = target.__handle;
                    return function(callback, from, to, duration) {
                        return __godot_tween_method(handle, callback, from, to, duration);
                    };
                }

                var methodFn = __findBinding(target.__class, prop, 'method');
                if (methodFn) {
                    var handle = target.__handle;
                    return function() {
                        var args = [handle];
                        for (var i = 0; i < arguments.length; i++) {
                            var arg = arguments[i];
                            if (arg && typeof arg === 'object' && arg.__handle !== undefined) {
                                args.push(arg.__handle);
                            } else {
                                args.push(arg);
                            }
                        }
                        return methodFn.apply(null, args);
                    };
                }

                // Support get_<property>() method pattern (e.g., get_name() for name property)
                if (prop.startsWith('get_')) {
                    var propName = prop.substring(4);  // Remove 'get_' prefix
                    var propBinding = __findBinding(target.__class, propName, 'property');
                    if (propBinding && propBinding.get) {
                        var handle = target.__handle;
                        return function() {
                            return propBinding.get(handle);
                        };
                    }
                }

                // Support set_<property>() method pattern (e.g., set_name() for name property)
                if (prop.startsWith('set_')) {
                    var propName = prop.substring(4);  // Remove 'set_' prefix
                    var propBinding = __findBinding(target.__class, propName, 'property');
                    if (propBinding && propBinding.set) {
                        var handle = target.__handle;
                        return function(value) {
                            var unwrapped = value;
                            if (value && typeof value === 'object' && value.__handle !== undefined) {
                                unwrapped = value.__handle;
                            }
                            propBinding.set(handle, unwrapped);
                        };
                    }
                }

                var propBinding = __findBinding(target.__class, prop, 'property');
                if (propBinding && propBinding.get) {
                    return propBinding.get(target.__handle);
                }

                // Check if this is a JS script method
                if (__godot_has_script_method(target.__handle, prop)) {
                    var handle = target.__handle;
                    return function() {
                        var args = [handle, prop];
                        for (var i = 0; i < arguments.length; i++) {
                            var arg = arguments[i];
                            if (arg && typeof arg === 'object' && arg.__handle !== undefined) {
                                args.push(arg.__handle);
                            } else {
                                args.push(arg);
                            }
                        }
                        return __godot_call_script_method.apply(null, args);
                    };
                }

                return undefined;
            },
            set: function(target, prop, value, receiver) {
                if (prop === '__handle' || prop === '__class') {
                    target[prop] = value;
                    return true;
                }

                var propBinding = __findBinding(target.__class, prop, 'property');
                if (propBinding && propBinding.set) {
                    var unwrapped = value;
                    if (value && typeof value === 'object' && value.__handle !== undefined) {
                        unwrapped = value.__handle;
                    }
                    propBinding.set(target.__handle, unwrapped);
                    return true;
                }

                return true;
            },
            has: function(target, prop) {
                if (prop === '__handle' || prop === '__class') return true;
                if (prop === 'connect' || prop === 'emit_signal' || prop === 'await_signal') return true;
                if (prop === 'tween_callback' || prop === 'tween_method') return true;
                // Check for get_/set_ method patterns
                if (prop.startsWith('get_') || prop.startsWith('set_')) {
                    var propName = prop.substring(4);
                    if (__findBinding(target.__class, propName, 'property') !== null) return true;
                }
                return __findBinding(target.__class, prop, 'method') !== null ||
                       __findBinding(target.__class, prop, 'property') !== null;
            }
        };
        'proxy handler done';
    )";

    result = JS_Eval(ctx, proxy_code, strlen(proxy_code), "<proxy>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx);
        const char* err = JS_ToCString(ctx, exception);
        UtilityFunctions::printerr("Failed to setup JS proxy handler: ", err ? err : "unknown");
        if (err) JS_FreeCString(ctx, err);
        JS_FreeValue(ctx, exception);
        JS_FreeValue(ctx, result);
        JS_FreeValue(ctx, global);
        return;
    }
    JS_FreeValue(ctx, result);

    // Step 3: Create object factory
    const char* factory_code = R"(
        globalThis.__create_godot_object = function(className) {
            var raw = __godot_new(className);
            if (!raw || raw.__handle === undefined) {
                throw new Error('Failed to create ' + className);
            }
            var target = {
                __handle: raw.__handle,
                __class: className
            };
            return new Proxy(target, __godot_proxy_handler);
        };

        // Wrap an existing Godot object handle (used when objects come from GDScript)
        globalThis.__wrap_existing_godot_object = function(handle, className) {
            var target = {
                __handle: handle,
                __class: className
            };
            return new Proxy(target, __godot_proxy_handler);
        };
        'factory done';
    )";

    result = JS_Eval(ctx, factory_code, strlen(factory_code), "<factory>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx);
        const char* err = JS_ToCString(ctx, exception);
        UtilityFunctions::printerr("Failed to setup JS factory: ", err ? err : "unknown");
        if (err) JS_FreeCString(ctx, err);
        JS_FreeValue(ctx, exception);
        JS_FreeValue(ctx, result);
        JS_FreeValue(ctx, global);
        return;
    }
    JS_FreeValue(ctx, result);

    // Step 4: Create constructors (only for instantiable classes)
    const char* ctor_code = R"(
        for (var className in __godot_classes) {
            (function(name) {
                var classInfo = __godot_classes[name];
                if (classInfo.instantiable) {
                    globalThis[name] = function() {
                        return __create_godot_object(name);
                    };
                }
            })(className);
        }
        'constructors done';
    )";

    result = JS_Eval(ctx, ctor_code, strlen(ctor_code), "<constructors>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(ctx);
        const char* err = JS_ToCString(ctx, exception);
        UtilityFunctions::printerr("Failed to setup JS constructors: ", err ? err : "unknown");
        if (err) JS_FreeCString(ctx, err);
        JS_FreeValue(ctx, exception);
    }
    JS_FreeValue(ctx, result);

    JS_FreeValue(ctx, global);
}

// setup_proxy_handler is now a no-op since the proxy uses __godot_classes directly
// The generated bindings are registered by register_all_classes() before this is called
void GodotBindings::setup_proxy_handler() {
    // No additional setup needed - __godot_classes is already populated
}

// Global function: __godot_new(class_name) - creates a new Godot object with JS wrapper
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

    // Create JS wrapper object that stores handle and class name
    JSValue obj = JS_NewObject(ctx);

    // Set __handle property
    JS_SetPropertyStr(ctx, obj, "__handle", JS_NewInt64(ctx, handle));

    // Set __class property
    JS_SetPropertyStr(ctx, obj, "__class", JS_NewString(ctx, class_name.utf8().get_data()));

    // Return the wrapper object. Properties will be accessed via JS code
    // that uses __godot_get/__godot_set with the handle
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

// Global function: __godot_connect(handle, signal_name, callback) - connects a JS callback to a Godot signal
JSValue GodotBindings::js_godot_connect(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 3) {
        return JS_ThrowTypeError(ctx, "__godot_connect requires 3 arguments: handle, signal_name, callback");
    }

    // Get object handle
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "First argument must be an object handle");
    }

    // Get signal name
    const char* signal_name_cstr = JS_ToCString(ctx, argv[1]);
    if (!signal_name_cstr) {
        return JS_ThrowTypeError(ctx, "Second argument must be a signal name string");
    }
    String signal_name = signal_name_cstr;
    JS_FreeCString(ctx, signal_name_cstr);

    // Get callback (must be a function)
    if (!JS_IsFunction(ctx, argv[2])) {
        return JS_ThrowTypeError(ctx, "Third argument must be a callback function");
    }

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    // Get the target object from the registry
    ObjectRegistry* registry = qjs_ctx->get_object_registry();
    if (!registry) {
        return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
    }

    Object* target = registry->get_object(handle);
    if (!target) {
        return JS_ThrowTypeError(ctx, "Invalid object handle");
    }

    // Get the signal registry
    SignalRegistry* signal_registry = qjs_ctx->get_signal_registry();
    if (!signal_registry) {
        return JS_ThrowInternalError(ctx, "SignalRegistry not initialized");
    }

    // Connect the callback
    // Note: SignalRegistry::connect() handles JS_DupValue internally
    uint64_t connection_id = signal_registry->connect(target, StringName(signal_name), argv[2]);

    if (connection_id == 0) {
        return JS_ThrowTypeError(ctx, "Failed to connect signal");
    }

    return JS_NewInt64(ctx, connection_id);
}

// Global function: __godot_emit_signal(handle, signal_name, ...args) - emits a signal from an object
JSValue GodotBindings::js_godot_emit_signal(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "__godot_emit_signal requires at least 2 arguments: handle, signal_name");
    }

    // Get object handle
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "First argument must be an object handle");
    }

    // Get signal name
    const char* signal_name_cstr = JS_ToCString(ctx, argv[1]);
    if (!signal_name_cstr) {
        return JS_ThrowTypeError(ctx, "Second argument must be a signal name string");
    }
    String signal_name = signal_name_cstr;
    JS_FreeCString(ctx, signal_name_cstr);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    // Get the target object from the registry
    ObjectRegistry* registry = qjs_ctx->get_object_registry();
    if (!registry) {
        return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
    }

    Object* target = registry->get_object(handle);
    if (!target) {
        return JS_ThrowTypeError(ctx, "Invalid object handle");
    }

    // Convert additional arguments to Godot Variants
    // Build args array with signal name first (for callv)
    // Note: The blocklist only affects JS->Godot calls through SafeWrapper.
    // C++ code here is trusted and can use callv directly.
    Array call_args;
    call_args.append(StringName(signal_name));
    for (int i = 2; i < argc; i++) {
        call_args.append(qjs_ctx->js_to_variant(argv[i]));
    }

    // Use callv to emit signal - supports unlimited arguments
    target->callv(StringName("emit_signal"), call_args);

    // callv returns Variant, emit_signal returns Error
    // If signal doesn't exist or other issues, Godot prints warnings internally

    return JS_UNDEFINED;
}

// Global function: __godot_await_signal(handle, signal_name) - returns a Promise that resolves when the signal fires
JSValue GodotBindings::js_godot_await_signal(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "__godot_await_signal requires 2 arguments: handle, signal_name");
    }

    // Get object handle
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "First argument must be an object handle");
    }

    // Get signal name
    const char* signal_name_cstr = JS_ToCString(ctx, argv[1]);
    if (!signal_name_cstr) {
        return JS_ThrowTypeError(ctx, "Second argument must be a signal name string");
    }
    String signal_name = signal_name_cstr;
    JS_FreeCString(ctx, signal_name_cstr);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    // Get the target object from the registry
    ObjectRegistry* registry = qjs_ctx->get_object_registry();
    if (!registry) {
        return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
    }

    Object* target = registry->get_object(handle);
    if (!target) {
        return JS_ThrowTypeError(ctx, "Invalid object handle");
    }

    // Get the signal registry
    SignalRegistry* signal_registry = qjs_ctx->get_signal_registry();
    if (!signal_registry) {
        return JS_ThrowInternalError(ctx, "SignalRegistry not initialized");
    }

    // Create a Promise using JS_NewPromiseCapability
    JSValue resolve_funcs[2];
    JSValue promise = JS_NewPromiseCapability(ctx, resolve_funcs);
    if (JS_IsException(promise)) {
        return promise;
    }

    // Store the resolve/reject functions and connection info in a closure
    // We create a callback that will:
    // 1. Resolve the promise with the signal arguments
    // 2. Disconnect itself (one-shot behavior)

    // Create a JS object to hold the promise state
    JSValue state_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, state_obj, "resolve", resolve_funcs[0]);
    JS_SetPropertyStr(ctx, state_obj, "reject", resolve_funcs[1]);
    JS_SetPropertyStr(ctx, state_obj, "connection_id", JS_NewInt64(ctx, 0));  // Will be set after connect

    // Create a callback function that resolves the promise and disconnects
    // We use a trampoline approach: store state and call __resolve_signal_promise from JS
    const char* callback_code = R"(
        (function(state) {
            return function() {
                // Convert arguments to array
                var args = Array.prototype.slice.call(arguments);
                // Resolve with single arg or array of args
                if (args.length === 0) {
                    state.resolve(undefined);
                } else if (args.length === 1) {
                    state.resolve(args[0]);
                } else {
                    state.resolve(args);
                }
            };
        })
    )";

    JSValue callback_factory = JS_Eval(ctx, callback_code, strlen(callback_code), "<await_signal>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(callback_factory)) {
        JS_FreeValue(ctx, promise);
        JS_FreeValue(ctx, state_obj);
        return callback_factory;
    }

    // Call the factory with state to get the actual callback
    JSValue callback = JS_Call(ctx, callback_factory, JS_UNDEFINED, 1, &state_obj);
    JS_FreeValue(ctx, callback_factory);
    JS_FreeValue(ctx, state_obj);

    if (JS_IsException(callback)) {
        JS_FreeValue(ctx, promise);
        return callback;
    }

    // Connect the callback
    uint64_t connection_id = signal_registry->connect(target, StringName(signal_name), callback);
    JS_FreeValue(ctx, callback);

    if (connection_id == 0) {
        JS_FreeValue(ctx, promise);
        return JS_ThrowTypeError(ctx, "Failed to connect signal");
    }

    return promise;
}

// GodotObject finalizer - called when JS object is garbage collected
void GodotBindings::godot_object_finalizer(JSRuntime* rt, JSValueConst val) {
    JSClassID class_id;
    void* ptr = JS_GetAnyOpaque(val, &class_id);
    if (!ptr) return;

    // Opaque data contains both handle and registry pointer
    // This allows correct cleanup even with multiple contexts sharing a runtime
    GodotObjectData* data = static_cast<GodotObjectData*>(ptr);

    if (data->registry) {
        data->registry->release_handle(data->handle);
    }

    // Free the data struct allocated by js_malloc
    js_free_rt(rt, data);
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

// Quaternion constructor
JSValue GodotBindings::js_quaternion_constructor(JSContext* ctx, JSValueConst new_target,
                                                  int argc, JSValueConst* argv) {
    double x = 0, y = 0, z = 0, w = 1;

    if (argc >= 1) JS_ToFloat64(ctx, &x, argv[0]);
    if (argc >= 2) JS_ToFloat64(ctx, &y, argv[1]);
    if (argc >= 3) JS_ToFloat64(ctx, &z, argv[2]);
    if (argc >= 4) JS_ToFloat64(ctx, &w, argv[3]);

    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, y));
    JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, z));
    JS_SetPropertyStr(ctx, obj, "w", JS_NewFloat64(ctx, w));

    return obj;
}

// Basis constructor - creates identity basis or from 3 row vectors
JSValue GodotBindings::js_basis_constructor(JSContext* ctx, JSValueConst new_target,
                                             int argc, JSValueConst* argv) {
    JSValue obj = JS_NewObject(ctx);
    JSValue x_row = JS_NewObject(ctx);
    JSValue y_row = JS_NewObject(ctx);
    JSValue z_row = JS_NewObject(ctx);

    // Default to identity matrix
    JS_SetPropertyStr(ctx, x_row, "x", JS_NewFloat64(ctx, 1));
    JS_SetPropertyStr(ctx, x_row, "y", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, x_row, "z", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, y_row, "x", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, y_row, "y", JS_NewFloat64(ctx, 1));
    JS_SetPropertyStr(ctx, y_row, "z", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, z_row, "x", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, z_row, "y", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, z_row, "z", JS_NewFloat64(ctx, 1));

    JS_SetPropertyStr(ctx, obj, "x", x_row);
    JS_SetPropertyStr(ctx, obj, "y", y_row);
    JS_SetPropertyStr(ctx, obj, "z", z_row);

    return obj;
}

// Transform3D constructor - creates identity transform
JSValue GodotBindings::js_transform3d_constructor(JSContext* ctx, JSValueConst new_target,
                                                   int argc, JSValueConst* argv) {
    JSValue obj = JS_NewObject(ctx);

    // Create identity basis
    JSValue basis = JS_NewObject(ctx);
    JSValue bx_row = JS_NewObject(ctx);
    JSValue by_row = JS_NewObject(ctx);
    JSValue bz_row = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, bx_row, "x", JS_NewFloat64(ctx, 1));
    JS_SetPropertyStr(ctx, bx_row, "y", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, bx_row, "z", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, by_row, "x", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, by_row, "y", JS_NewFloat64(ctx, 1));
    JS_SetPropertyStr(ctx, by_row, "z", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, bz_row, "x", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, bz_row, "y", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, bz_row, "z", JS_NewFloat64(ctx, 1));

    JS_SetPropertyStr(ctx, basis, "x", bx_row);
    JS_SetPropertyStr(ctx, basis, "y", by_row);
    JS_SetPropertyStr(ctx, basis, "z", bz_row);

    // Create origin at zero
    JSValue origin = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, origin, "x", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, origin, "y", JS_NewFloat64(ctx, 0));
    JS_SetPropertyStr(ctx, origin, "z", JS_NewFloat64(ctx, 0));

    JS_SetPropertyStr(ctx, obj, "basis", basis);
    JS_SetPropertyStr(ctx, obj, "origin", origin);

    return obj;
}

JSValue GodotBindings::variant_to_js(const Variant& value) {
    return context_->variant_to_js(value);
}

Variant GodotBindings::js_to_variant(JSValue value) {
    return context_->js_to_variant(value);
}

// Global function: __godot_tween_callback(tween_handle, callback) - add a callback to a tween
JSValue GodotBindings::js_godot_tween_callback(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "__godot_tween_callback requires 2 arguments: tween_handle, callback");
    }

    // Get tween handle
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "First argument must be a tween handle");
    }

    // Get callback (must be a function)
    if (!JS_IsFunction(ctx, argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument must be a callback function");
    }

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    // Get the tween object from the registry
    ObjectRegistry* registry = qjs_ctx->get_object_registry();
    if (!registry) {
        return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
    }

    Object* obj = registry->get_object(handle);
    if (!obj) {
        return JS_ThrowTypeError(ctx, "Invalid tween handle");
    }

    Tween* tween = Object::cast_to<Tween>(obj);
    if (!tween) {
        return JS_ThrowTypeError(ctx, "Object is not a Tween");
    }

    // Get the signal registry to create a callable
    SignalRegistry* signal_registry = qjs_ctx->get_signal_registry();
    if (!signal_registry) {
        return JS_ThrowInternalError(ctx, "SignalRegistry not initialized");
    }

    // Create a Callable from the JS function
    Callable callable = signal_registry->create_callable(argv[1]);

    // Call tween_callback with the callable
    Ref<CallbackTweener> result = tween->tween_callback(callable);

    if (result.is_null()) {
        return JS_NULL;
    }

    // Return the CallbackTweener as a wrapped object
    Object* ret_obj = result.ptr();
    int64_t ret_handle = registry->get_or_create_handle(ret_obj);
    String ret_class = ret_obj->get_class();

    // Create wrapped object
    const char* factory_code = "globalThis.__wrap_existing_godot_object";
    JSValue factory = JS_Eval(ctx, factory_code, strlen(factory_code), "<tween_callback>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(factory)) {
        return factory;
    }

    JSValue args[2] = {
        JS_NewInt64(ctx, ret_handle),
        JS_NewString(ctx, ret_class.utf8().get_data())
    };
    JSValue wrapped = JS_Call(ctx, factory, JS_UNDEFINED, 2, args);
    JS_FreeValue(ctx, factory);
    JS_FreeValue(ctx, args[0]);
    JS_FreeValue(ctx, args[1]);

    return wrapped;
}

// Global function: __godot_tween_method(tween_handle, callback, from, to, duration) - add a method tween
JSValue GodotBindings::js_godot_tween_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 5) {
        return JS_ThrowTypeError(ctx, "__godot_tween_method requires 5 arguments: tween_handle, callback, from, to, duration");
    }

    // Get tween handle
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "First argument must be a tween handle");
    }

    // Get callback (must be a function)
    if (!JS_IsFunction(ctx, argv[1])) {
        return JS_ThrowTypeError(ctx, "Second argument must be a callback function");
    }

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    // Get the tween object from the registry
    ObjectRegistry* registry = qjs_ctx->get_object_registry();
    if (!registry) {
        return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
    }

    Object* obj = registry->get_object(handle);
    if (!obj) {
        return JS_ThrowTypeError(ctx, "Invalid tween handle");
    }

    Tween* tween = Object::cast_to<Tween>(obj);
    if (!tween) {
        return JS_ThrowTypeError(ctx, "Object is not a Tween");
    }

    // Get the signal registry to create a callable
    SignalRegistry* signal_registry = qjs_ctx->get_signal_registry();
    if (!signal_registry) {
        return JS_ThrowInternalError(ctx, "SignalRegistry not initialized");
    }

    // Create a Callable from the JS function
    Callable callable = signal_registry->create_callable(argv[1]);

    // Convert from, to, and duration
    Variant from_val = qjs_ctx->js_to_variant(argv[2]);
    Variant to_val = qjs_ctx->js_to_variant(argv[3]);
    double duration;
    JS_ToFloat64(ctx, &duration, argv[4]);

    // Call tween_method with the callable
    Ref<MethodTweener> result = tween->tween_method(callable, from_val, to_val, duration);

    if (result.is_null()) {
        return JS_NULL;
    }

    // Return the MethodTweener as a wrapped object
    Object* ret_obj = result.ptr();
    int64_t ret_handle = registry->get_or_create_handle(ret_obj);
    String ret_class = ret_obj->get_class();

    // Create wrapped object
    const char* factory_code = "globalThis.__wrap_existing_godot_object";
    JSValue factory = JS_Eval(ctx, factory_code, strlen(factory_code), "<tween_method>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(factory)) {
        return factory;
    }

    JSValue args[2] = {
        JS_NewInt64(ctx, ret_handle),
        JS_NewString(ctx, ret_class.utf8().get_data())
    };
    JSValue wrapped = JS_Call(ctx, factory, JS_UNDEFINED, 2, args);
    JS_FreeValue(ctx, factory);
    JS_FreeValue(ctx, args[0]);
    JS_FreeValue(ctx, args[1]);

    return wrapped;
}

// Global function: __godot_has_script_method(handle, method_name) - check if object has JS script method
JSValue GodotBindings::js_godot_has_script_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "__godot_has_script_method requires 2 arguments: handle, method_name");
    }

    // Get object handle
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "First argument must be an object handle");
    }

    // Get method name
    const char* method_name_cstr = JS_ToCString(ctx, argv[1]);
    if (!method_name_cstr) {
        return JS_ThrowTypeError(ctx, "Second argument must be a method name string");
    }
    String method_name = method_name_cstr;
    JS_FreeCString(ctx, method_name_cstr);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_FALSE;
    }

    // Get the target object from the registry
    ObjectRegistry* registry = qjs_ctx->get_object_registry();
    if (!registry) {
        return JS_FALSE;
    }

    Object* target = registry->get_object(handle);
    if (!target) {
        return JS_FALSE;
    }

    // Check if object has a JS script
    Ref<Script> script = target->get_script();
    if (!script.is_valid()) {
        return JS_FALSE;
    }

    JSScript* js_script = Object::cast_to<JSScript>(script.ptr());
    if (!js_script) {
        return JS_FALSE;
    }

    // Check if the script has this method
    return JS_NewBool(ctx, js_script->_has_method(StringName(method_name)));
}

// Global function: __godot_call_script_method(handle, method_name, ...args) - call JS script method on object
JSValue GodotBindings::js_godot_call_script_method(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "__godot_call_script_method requires at least 2 arguments: handle, method_name");
    }

    // Get object handle
    int64_t handle;
    if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "First argument must be an object handle");
    }

    // Get method name
    const char* method_name_cstr = JS_ToCString(ctx, argv[1]);
    if (!method_name_cstr) {
        return JS_ThrowTypeError(ctx, "Second argument must be a method name string");
    }
    String method_name = method_name_cstr;
    JS_FreeCString(ctx, method_name_cstr);

    QuickJSContext* qjs_ctx = get_context(ctx);
    if (!qjs_ctx) {
        return JS_ThrowInternalError(ctx, "Context not initialized");
    }

    // Get the target object from the registry
    ObjectRegistry* registry = qjs_ctx->get_object_registry();
    if (!registry) {
        return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
    }

    Object* target = registry->get_object(handle);
    if (!target) {
        return JS_ThrowTypeError(ctx, "Invalid object handle");
    }

    // Check if object has a JS script
    Ref<Script> script = target->get_script();
    if (!script.is_valid()) {
        return JS_ThrowTypeError(ctx, "Object does not have a script");
    }

    JSScript* js_script = Object::cast_to<JSScript>(script.ptr());
    if (!js_script) {
        return JS_ThrowTypeError(ctx, "Object does not have a JavaScript script");
    }

    // Get the script instance from our registered instances
    JSScriptInstance* instance = js_script->get_instance(target);
    if (!instance) {
        return JS_ThrowTypeError(ctx, "Script instance not found");
    }

    // Convert JS arguments to Variant array
    Vector<Variant> variant_args;
    Vector<const Variant*> variant_arg_ptrs;
    for (int i = 2; i < argc; i++) {
        variant_args.push_back(qjs_ctx->js_to_variant(argv[i]));
    }
    for (int i = 0; i < variant_args.size(); i++) {
        variant_arg_ptrs.push_back(&variant_args[i]);
    }

    // Call the method
    Variant result;
    String error;
    // Cast is safe - we're passing a non-const array as const
    const Variant** args_ptr = variant_arg_ptrs.size() > 0 ?
        const_cast<const Variant**>(variant_arg_ptrs.ptr()) : nullptr;
    bool success = instance->call_method(
        StringName(method_name),
        args_ptr,
        variant_args.size(),
        result,
        error
    );

    if (!success) {
        if (!error.is_empty()) {
            return JS_ThrowTypeError(ctx, "%s", error.utf8().get_data());
        }
        return JS_UNDEFINED;
    }

    return qjs_ctx->variant_to_js(result);
}

} // namespace jsb

#include "godot_bindings.h"
#include "array_registry.h"
#include "generated_classes.gen.h"
#include "js_script.h"
#include "js_script_instance.h"
#include "object_registry.h"
#include "quickjs_context.h"
#include "safe_wrapper.h"
#include "sandbox_config.h"
#include "signal_registry.h"

#include <godot_cpp/classes/callback_tweener.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/method_tweener.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

// Static class IDs shared across all contexts using the same runtime.
// These are initialized to 0 and JS_NewClassID only allocates once.
JSClassID GodotBindings::godot_object_class_id_ = 0;
JSClassID GodotBindings::godot_array_class_id_ = 0;

// Store context pointer in JSRuntime opaque
GodotBindings::GodotBindings(QuickJSContext *context) : context_(context) {}

GodotBindings::~GodotBindings() { shutdown(); }

QuickJSContext *GodotBindings::get_context(JSContext *ctx) {
  return static_cast<QuickJSContext *>(JS_GetContextOpaque(ctx));
}

bool GodotBindings::initialize() {
  if (!context_ || !context_->ctx()) {
    return false;
  }

  JSContext *ctx = context_->ctx();
  JSRuntime *rt = context_->rt();

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

  // Create GodotArray class (for Array/PackedArray handles with finalizer)
  JS_NewClassID(rt, &godot_array_class_id_);

  JSClassDef array_class_def = {};
  array_class_def.class_name = "GodotArray";
  array_class_def.finalizer = godot_array_finalizer;
  array_class_def.exotic = nullptr;

  JS_NewClass(rt, godot_array_class_id_, &array_class_def);

  // Set up array prototype
  JSValue array_proto = JS_NewObject(ctx);
  JS_SetClassProto(ctx, godot_array_class_id_, array_proto);

  // Setup bindings
  setup_global_functions();
  setup_math_types();

  // Register all generated class bindings (creates __godot_classes registry)
  JSValue global = JS_GetGlobalObject(ctx);
  generated::register_all_classes(ctx, global);

  // Register singleton class enums (Input.MOUSE_MODE_*, etc.)
  // Must be after setup_global_functions() which creates the singleton objects
  generated::register_singleton_enums(ctx, global);

  // Register class enums for non-singleton classes (RenderingDevice constants,
  // etc.)
  generated::register_class_enums(ctx, global);
  JS_FreeValue(ctx, global);

  setup_proxy_handler(); // No-op, but kept for structure
  setup_godot_class_constructor();

  return true;
}

void GodotBindings::shutdown() {
  // Note: Do NOT reset static class IDs here. They are shared across all
  // contexts using the same runtime. Resetting them would cause new contexts to
  // register new class IDs, breaking finalizer calls for existing objects. The
  // class IDs are valid for the lifetime of the JSRuntime.
}

// Time singleton wrapper functions
static JSValue js_time_get_ticks_msec(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  return JS_NewInt64(ctx, Time::get_singleton()->get_ticks_msec());
}

static JSValue js_time_get_ticks_usec(JSContext *ctx, JSValueConst this_val,
                                      int argc, JSValueConst *argv) {
  return JS_NewInt64(ctx, Time::get_singleton()->get_ticks_usec());
}

static JSValue js_time_get_unix_time(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv) {
  return JS_NewFloat64(ctx, Time::get_singleton()->get_unix_time_from_system());
}

// Input singleton wrapper functions
static JSValue js_input_is_anything_pressed(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  return JS_NewBool(ctx, Input::get_singleton()->is_anything_pressed());
}

static JSValue js_input_is_key_pressed(JSContext *ctx, JSValueConst this_val,
                                       int argc, JSValueConst *argv) {
  if (argc < 1)
    return JS_ThrowTypeError(ctx, "is_key_pressed: missing keycode argument");
  int64_t keycode;
  JS_ToInt64(ctx, &keycode, argv[0]);
  return JS_NewBool(ctx, Input::get_singleton()->is_key_pressed((Key)keycode));
}

static JSValue js_input_is_mouse_button_pressed(JSContext *ctx,
                                                JSValueConst this_val, int argc,
                                                JSValueConst *argv) {
  if (argc < 1)
    return JS_ThrowTypeError(
        ctx, "is_mouse_button_pressed: missing button argument");
  int64_t button;
  JS_ToInt64(ctx, &button, argv[0]);
  return JS_NewBool(ctx, Input::get_singleton()->is_mouse_button_pressed(
                             (MouseButton)button));
}

static JSValue js_input_is_action_pressed(JSContext *ctx, JSValueConst this_val,
                                          int argc, JSValueConst *argv) {
  if (argc < 1)
    return JS_ThrowTypeError(ctx, "is_action_pressed: missing action argument");
  const char *action = JS_ToCString(ctx, argv[0]);
  bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
  bool result = Input::get_singleton()->is_action_pressed(StringName(action),
                                                          exact_match);
  JS_FreeCString(ctx, action);
  return JS_NewBool(ctx, result);
}

static JSValue js_input_is_action_just_pressed(JSContext *ctx,
                                               JSValueConst this_val, int argc,
                                               JSValueConst *argv) {
  if (argc < 1)
    return JS_ThrowTypeError(ctx,
                             "is_action_just_pressed: missing action argument");
  const char *action = JS_ToCString(ctx, argv[0]);
  bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
  bool result = Input::get_singleton()->is_action_just_pressed(
      StringName(action), exact_match);
  JS_FreeCString(ctx, action);
  return JS_NewBool(ctx, result);
}

static JSValue js_input_is_action_just_released(JSContext *ctx,
                                                JSValueConst this_val, int argc,
                                                JSValueConst *argv) {
  if (argc < 1)
    return JS_ThrowTypeError(
        ctx, "is_action_just_released: missing action argument");
  const char *action = JS_ToCString(ctx, argv[0]);
  bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
  bool result = Input::get_singleton()->is_action_just_released(
      StringName(action), exact_match);
  JS_FreeCString(ctx, action);
  return JS_NewBool(ctx, result);
}

static JSValue js_input_get_action_strength(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 1)
    return JS_ThrowTypeError(ctx,
                             "get_action_strength: missing action argument");
  const char *action = JS_ToCString(ctx, argv[0]);
  bool exact_match = argc > 1 ? JS_ToBool(ctx, argv[1]) : false;
  double result = Input::get_singleton()->get_action_strength(
      StringName(action), exact_match);
  JS_FreeCString(ctx, action);
  return JS_NewFloat64(ctx, result);
}

static JSValue js_input_get_axis(JSContext *ctx, JSValueConst this_val,
                                 int argc, JSValueConst *argv) {
  if (argc < 2)
    return JS_ThrowTypeError(ctx, "get_axis: missing action arguments");
  const char *neg_action = JS_ToCString(ctx, argv[0]);
  const char *pos_action = JS_ToCString(ctx, argv[1]);
  double result = Input::get_singleton()->get_axis(StringName(neg_action),
                                                   StringName(pos_action));
  JS_FreeCString(ctx, neg_action);
  JS_FreeCString(ctx, pos_action);
  return JS_NewFloat64(ctx, result);
}

static JSValue js_input_get_vector(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
  if (argc < 4)
    return JS_ThrowTypeError(ctx, "get_vector: missing action arguments");
  const char *neg_x = JS_ToCString(ctx, argv[0]);
  const char *pos_x = JS_ToCString(ctx, argv[1]);
  const char *neg_y = JS_ToCString(ctx, argv[2]);
  const char *pos_y = JS_ToCString(ctx, argv[3]);
  double deadzone = argc > 4 ? 0.0 : -1.0;
  if (argc > 4)
    JS_ToFloat64(ctx, &deadzone, argv[4]);
  Vector2 result = Input::get_singleton()->get_vector(
      StringName(neg_x), StringName(pos_x), StringName(neg_y),
      StringName(pos_y), deadzone);
  JS_FreeCString(ctx, neg_x);
  JS_FreeCString(ctx, pos_x);
  JS_FreeCString(ctx, neg_y);
  JS_FreeCString(ctx, pos_y);
  JSValue ret = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, ret, "x", JS_NewFloat64(ctx, result.x));
  JS_SetPropertyStr(ctx, ret, "y", JS_NewFloat64(ctx, result.y));
  return ret;
}

static JSValue js_input_get_last_mouse_velocity(JSContext *ctx,
                                                JSValueConst this_val, int argc,
                                                JSValueConst *argv) {
  Vector2 vel = Input::get_singleton()->get_last_mouse_velocity();
  JSValue ret = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, ret, "x", JS_NewFloat64(ctx, vel.x));
  JS_SetPropertyStr(ctx, ret, "y", JS_NewFloat64(ctx, vel.y));
  return ret;
}

static JSValue js_input_get_mouse_button_mask(JSContext *ctx,
                                              JSValueConst this_val, int argc,
                                              JSValueConst *argv) {
  return JS_NewInt64(ctx,
                     (int64_t)Input::get_singleton()->get_mouse_button_mask());
}

static JSValue js_input_set_mouse_mode(JSContext *ctx, JSValueConst this_val,
                                       int argc, JSValueConst *argv) {
  if (argc < 1)
    return JS_ThrowTypeError(ctx, "set_mouse_mode: missing mode argument");
  int64_t mode;
  JS_ToInt64(ctx, &mode, argv[0]);
  Input::get_singleton()->set_mouse_mode((Input::MouseMode)mode);
  return JS_UNDEFINED;
}

static JSValue js_input_get_mouse_mode(JSContext *ctx, JSValueConst this_val,
                                       int argc, JSValueConst *argv) {
  return JS_NewInt64(ctx, (int64_t)Input::get_singleton()->get_mouse_mode());
}

// RenderingServer singleton wrapper functions
static JSValue js_rendering_server_get_rendering_device(JSContext *ctx,
                                                        JSValueConst this_val,
                                                        int argc,
                                                        JSValueConst *argv) {
  RenderingDevice *rd =
      RenderingServer::get_singleton()->get_rendering_device();
  if (!rd) {
    return JS_NULL;
  }

  // Get context and registry
  QuickJSContext *qjs_ctx = GodotBindings::get_context(ctx);
  if (!qjs_ctx) {
    return JS_NULL;
  }

  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_NULL;
  }

  // Create a handle for the RenderingDevice
  int64_t handle = registry->get_or_create_handle(rd);

  // Create wrapped object using JS factory
  const char *factory_code = "globalThis.__wrap_existing_godot_object";
  JSValue factory = JS_Eval(ctx, factory_code, strlen(factory_code),
                            "<rendering_server>", JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(factory)) {
    return factory;
  }

  JSValue args[2] = {JS_NewInt64(ctx, handle),
                     JS_NewString(ctx, "RenderingDevice")};
  JSValue wrapped = JS_Call(ctx, factory, JS_UNDEFINED, 2, args);
  JS_FreeValue(ctx, factory);
  JS_FreeValue(ctx, args[0]);
  JS_FreeValue(ctx, args[1]);

  return wrapped;
}

static JSValue js_rendering_server_create_local_rendering_device(
    JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  RenderingDevice *rd =
      RenderingServer::get_singleton()->create_local_rendering_device();
  if (!rd) {
    return JS_NULL;
  }

  // Get context and registry
  QuickJSContext *qjs_ctx = GodotBindings::get_context(ctx);
  if (!qjs_ctx) {
    return JS_NULL;
  }

  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_NULL;
  }

  // Create a handle for the RenderingDevice
  // Mark as JS-created so it gets properly freed when JS GC collects it
  // Local RenderingDevice is NOT RefCounted, so ObjectRegistry needs to
  // memdelete() it
  int64_t handle = registry->get_or_create_handle(rd, true /* js_created */);

  // Create wrapped object using JS factory
  const char *factory_code = "globalThis.__wrap_existing_godot_object";
  JSValue factory = JS_Eval(ctx, factory_code, strlen(factory_code),
                            "<rendering_server>", JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(factory)) {
    return factory;
  }

  JSValue args[2] = {JS_NewInt64(ctx, handle),
                     JS_NewString(ctx, "RenderingDevice")};
  JSValue wrapped = JS_Call(ctx, factory, JS_UNDEFINED, 2, args);
  JS_FreeValue(ctx, factory);
  JS_FreeValue(ctx, args[0]);
  JS_FreeValue(ctx, args[1]);

  return wrapped;
}

// __get_shader_compile_errors(spirv_handle) - Extract shader compilation errors
// from RDShaderSPIRV Returns an object with stage-specific errors, useful for
// AI feedback on shader compilation failures Returns: { vertex: "", fragment:
// "", compute: "", tessellation_control: "", tessellation_evaluation: "" }
// Empty strings indicate no error for that stage
static JSValue js_get_shader_compile_errors(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 1) {
    return JS_ThrowTypeError(
        ctx, "__get_shader_compile_errors requires 1 argument: spirv_handle");
  }

  // Get object handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be an object handle");
  }

  QuickJSContext *qjs_ctx = GodotBindings::get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
  }

  Object *obj = registry->get_object(handle);
  if (!obj) {
    return JS_ThrowTypeError(ctx, "Invalid object handle");
  }

  // Cast to RDShaderSPIRV
  RDShaderSPIRV *spirv = Object::cast_to<RDShaderSPIRV>(obj);
  if (!spirv) {
    return JS_ThrowTypeError(ctx, "Object is not an RDShaderSPIRV");
  }

  // Create result object with errors from all shader stages
  JSValue result = JS_NewObject(ctx);

  // Get error for each shader stage
  String vertex_error =
      spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_VERTEX);
  String fragment_error =
      spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_FRAGMENT);
  String compute_error =
      spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_COMPUTE);
  String tess_control_error = spirv->get_stage_compile_error(
      RenderingDevice::SHADER_STAGE_TESSELATION_CONTROL);
  String tess_eval_error = spirv->get_stage_compile_error(
      RenderingDevice::SHADER_STAGE_TESSELATION_EVALUATION);

  JS_SetPropertyStr(ctx, result, "vertex",
                    JS_NewString(ctx, vertex_error.utf8().get_data()));
  JS_SetPropertyStr(ctx, result, "fragment",
                    JS_NewString(ctx, fragment_error.utf8().get_data()));
  JS_SetPropertyStr(ctx, result, "compute",
                    JS_NewString(ctx, compute_error.utf8().get_data()));
  JS_SetPropertyStr(ctx, result, "tessellation_control",
                    JS_NewString(ctx, tess_control_error.utf8().get_data()));
  JS_SetPropertyStr(ctx, result, "tessellation_evaluation",
                    JS_NewString(ctx, tess_eval_error.utf8().get_data()));

  // Also add a combined "hasErrors" flag and "all" string for convenience
  bool has_errors = !vertex_error.is_empty() || !fragment_error.is_empty() ||
                    !compute_error.is_empty() ||
                    !tess_control_error.is_empty() ||
                    !tess_eval_error.is_empty();
  JS_SetPropertyStr(ctx, result, "hasErrors", JS_NewBool(ctx, has_errors));

  // Combine all non-empty errors into one string
  String all_errors;
  if (!vertex_error.is_empty()) {
    all_errors += "[VERTEX] " + vertex_error + "\n";
  }
  if (!fragment_error.is_empty()) {
    all_errors += "[FRAGMENT] " + fragment_error + "\n";
  }
  if (!compute_error.is_empty()) {
    all_errors += "[COMPUTE] " + compute_error + "\n";
  }
  if (!tess_control_error.is_empty()) {
    all_errors += "[TESS_CONTROL] " + tess_control_error + "\n";
  }
  if (!tess_eval_error.is_empty()) {
    all_errors += "[TESS_EVAL] " + tess_eval_error + "\n";
  }
  JS_SetPropertyStr(ctx, result, "all",
                    JS_NewString(ctx, all_errors.utf8().get_data()));

  return result;
}

// Forward declaration for array handle wrapper creator (defined later with
// other array functions)
static JSValue js_create_array_handle_wrapper(JSContext *ctx,
                                              JSValueConst this_val, int argc,
                                              JSValueConst *argv);

void GodotBindings::setup_global_functions() {
  JSContext *ctx = context_->ctx();
  JSValue global = JS_GetGlobalObject(ctx);

  // load() function for resources
  JS_SetPropertyStr(ctx, global, "load",
                    JS_NewCFunction(ctx, js_load, "load", 1));

  // __godot_connect(handle, signal_name, callback) - connect JS callback to
  // Godot signal
  JS_SetPropertyStr(
      ctx, global, "__godot_connect",
      JS_NewCFunction(ctx, js_godot_connect, "__godot_connect", 3));

  // __godot_emit_signal(handle, signal_name, ...args) - emit a signal from an
  // object
  JS_SetPropertyStr(
      ctx, global, "__godot_emit_signal",
      JS_NewCFunction(ctx, js_godot_emit_signal, "__godot_emit_signal", 2));

  // __godot_await_signal(handle, signal_name) - returns a Promise that resolves
  // when the signal fires
  JS_SetPropertyStr(
      ctx, global, "__godot_await_signal",
      JS_NewCFunction(ctx, js_godot_await_signal, "__godot_await_signal", 2));

  // __godot_tween_callback(tween_handle, callback) - add a callback to a tween
  JS_SetPropertyStr(ctx, global, "__godot_tween_callback",
                    JS_NewCFunction(ctx, js_godot_tween_callback,
                                    "__godot_tween_callback", 2));

  // __godot_tween_method(tween_handle, callback, from, to, duration) - add a
  // method tween
  JS_SetPropertyStr(
      ctx, global, "__godot_tween_method",
      JS_NewCFunction(ctx, js_godot_tween_method, "__godot_tween_method", 5));

  // __godot_call_script_method(handle, method_name, ...args) - call JS script
  // method on object
  JS_SetPropertyStr(ctx, global, "__godot_call_script_method",
                    JS_NewCFunction(ctx, js_godot_call_script_method,
                                    "__godot_call_script_method", 2));

  // __godot_has_script_method(handle, method_name) - check if object has JS
  // script method
  JS_SetPropertyStr(ctx, global, "__godot_has_script_method",
                    JS_NewCFunction(ctx, js_godot_has_script_method,
                                    "__godot_has_script_method", 2));

  // __godot_has_signal(handle, signal_name) - check if object has a signal
  // (built-in or custom)
  JS_SetPropertyStr(
      ctx, global, "__godot_has_signal",
      JS_NewCFunction(ctx, js_godot_has_signal, "__godot_has_signal", 2));

  // Generic property/method access for objects without pre-generated bindings
  // (e.g., GDScript autoloads)
  JS_SetPropertyStr(ctx, global, "__godot_get",
                    JS_NewCFunction(ctx, js_godot_get, "__godot_get", 2));
  JS_SetPropertyStr(ctx, global, "__godot_set",
                    JS_NewCFunction(ctx, js_godot_set, "__godot_set", 3));
  JS_SetPropertyStr(ctx, global, "__godot_call",
                    JS_NewCFunction(ctx, js_godot_call, "__godot_call", 2));
  JS_SetPropertyStr(
      ctx, global, "__godot_has_method",
      JS_NewCFunction(ctx, js_godot_has_method, "__godot_has_method", 2));

  // Array proxy functions for zero-copy access to Godot arrays
  JS_SetPropertyStr(
      ctx, global, "__godot_array_get",
      JS_NewCFunction(ctx, js_godot_array_get, "__godot_array_get", 2));
  JS_SetPropertyStr(
      ctx, global, "__godot_array_set",
      JS_NewCFunction(ctx, js_godot_array_set, "__godot_array_set", 3));
  JS_SetPropertyStr(
      ctx, global, "__godot_array_size",
      JS_NewCFunction(ctx, js_godot_array_size, "__godot_array_size", 1));
  JS_SetPropertyStr(
      ctx, global, "__godot_array_push",
      JS_NewCFunction(ctx, js_godot_array_push, "__godot_array_push", 2));
  JS_SetPropertyStr(
      ctx, global, "__godot_array_pop",
      JS_NewCFunction(ctx, js_godot_array_pop, "__godot_array_pop", 1));

  // Array handle wrapper creator - creates JSObjectClass with finalizer
  // Used by __wrap_godot_array and __wrap_packed_array to ensure handles are
  // released
  JS_SetPropertyStr(ctx, global, "__create_array_handle_wrapper",
                    JS_NewCFunction(ctx, js_create_array_handle_wrapper,
                                    "__create_array_handle_wrapper", 1));

  // Packed array proxy functions for zero-copy access
  JS_SetPropertyStr(
      ctx, global, "__packed_array_get",
      JS_NewCFunction(ctx, js_packed_array_get, "__packed_array_get", 2));
  JS_SetPropertyStr(
      ctx, global, "__packed_array_size",
      JS_NewCFunction(ctx, js_packed_array_size, "__packed_array_size", 1));
  JS_SetPropertyStr(
      ctx, global, "__packed_array_type",
      JS_NewCFunction(ctx, js_packed_array_type, "__packed_array_type", 1));

  // Math type proxy functions for zero-copy access
  JS_SetPropertyStr(
      ctx, global, "__math_get_property",
      JS_NewCFunction(ctx, js_math_get_property, "__math_get_property", 2));
  JS_SetPropertyStr(
      ctx, global, "__math_set_property",
      JS_NewCFunction(ctx, js_math_set_property, "__math_set_property", 3));
  JS_SetPropertyStr(
      ctx, global, "__math_get_type",
      JS_NewCFunction(ctx, js_math_get_type, "__math_get_type", 1));

  // Time singleton (safe subset of methods)
  JSValue time_obj = JS_NewObject(ctx);
  JS_SetPropertyStr(
      ctx, time_obj, "get_ticks_msec",
      JS_NewCFunction(ctx, js_time_get_ticks_msec, "get_ticks_msec", 0));
  JS_SetPropertyStr(
      ctx, time_obj, "get_ticks_usec",
      JS_NewCFunction(ctx, js_time_get_ticks_usec, "get_ticks_usec", 0));
  JS_SetPropertyStr(ctx, time_obj, "get_unix_time_from_system",
                    JS_NewCFunction(ctx, js_time_get_unix_time,
                                    "get_unix_time_from_system", 0));
  JS_SetPropertyStr(ctx, global, "Time", time_obj);

  // Input singleton (safe subset of methods)
  JSValue input_obj = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, input_obj, "is_anything_pressed",
                    JS_NewCFunction(ctx, js_input_is_anything_pressed,
                                    "is_anything_pressed", 0));
  JS_SetPropertyStr(
      ctx, input_obj, "is_key_pressed",
      JS_NewCFunction(ctx, js_input_is_key_pressed, "is_key_pressed", 1));
  JS_SetPropertyStr(ctx, input_obj, "is_mouse_button_pressed",
                    JS_NewCFunction(ctx, js_input_is_mouse_button_pressed,
                                    "is_mouse_button_pressed", 1));
  JS_SetPropertyStr(
      ctx, input_obj, "is_action_pressed",
      JS_NewCFunction(ctx, js_input_is_action_pressed, "is_action_pressed", 2));
  JS_SetPropertyStr(ctx, input_obj, "is_action_just_pressed",
                    JS_NewCFunction(ctx, js_input_is_action_just_pressed,
                                    "is_action_just_pressed", 2));
  JS_SetPropertyStr(ctx, input_obj, "is_action_just_released",
                    JS_NewCFunction(ctx, js_input_is_action_just_released,
                                    "is_action_just_released", 2));
  JS_SetPropertyStr(ctx, input_obj, "get_action_strength",
                    JS_NewCFunction(ctx, js_input_get_action_strength,
                                    "get_action_strength", 2));
  JS_SetPropertyStr(ctx, input_obj, "get_axis",
                    JS_NewCFunction(ctx, js_input_get_axis, "get_axis", 2));
  JS_SetPropertyStr(ctx, input_obj, "get_vector",
                    JS_NewCFunction(ctx, js_input_get_vector, "get_vector", 5));
  JS_SetPropertyStr(ctx, input_obj, "get_last_mouse_velocity",
                    JS_NewCFunction(ctx, js_input_get_last_mouse_velocity,
                                    "get_last_mouse_velocity", 0));
  JS_SetPropertyStr(ctx, input_obj, "get_mouse_button_mask",
                    JS_NewCFunction(ctx, js_input_get_mouse_button_mask,
                                    "get_mouse_button_mask", 0));
  JS_SetPropertyStr(
      ctx, input_obj, "set_mouse_mode",
      JS_NewCFunction(ctx, js_input_set_mouse_mode, "set_mouse_mode", 1));
  JS_SetPropertyStr(
      ctx, input_obj, "get_mouse_mode",
      JS_NewCFunction(ctx, js_input_get_mouse_mode, "get_mouse_mode", 0));

  // Note: Mouse mode constants (MOUSE_MODE_VISIBLE, etc.) are now generated
  // by register_singleton_enums() from extension_api.json

  JS_SetPropertyStr(ctx, global, "Input", input_obj);

  // RenderingServer singleton (for compute shader access)
  JSValue rendering_server_obj = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, rendering_server_obj, "get_rendering_device",
                    JS_NewCFunction(ctx,
                                    js_rendering_server_get_rendering_device,
                                    "get_rendering_device", 0));
  JS_SetPropertyStr(
      ctx, rendering_server_obj, "create_local_rendering_device",
      JS_NewCFunction(ctx, js_rendering_server_create_local_rendering_device,
                      "create_local_rendering_device", 0));
  JS_SetPropertyStr(ctx, global, "RenderingServer", rendering_server_obj);

  // __get_shader_compile_errors(spirv_handle) - extract shader compilation
  // errors from RDShaderSPIRV Returns: { vertex, fragment, compute,
  // tessellation_control, tessellation_evaluation, hasErrors, all } Useful for
  // AI feedback on shader compilation failures
  JS_SetPropertyStr(ctx, global, "__get_shader_compile_errors",
                    JS_NewCFunction(ctx, js_get_shader_compile_errors,
                                    "__get_shader_compile_errors", 1));

  JS_FreeValue(ctx, global);
}

void GodotBindings::setup_math_types() {
  JSContext *ctx = context_->ctx();
  JSValue global = JS_GetGlobalObject(ctx);

  // Use generated math type constructors (Vector2, Vector3, Color, Quaternion,
  // etc.)
  generated::register_math_type_constructors(ctx, global);

  // Use generated packed array constructors and functions
  generated::register_packed_array_constructors(ctx, global);
  generated::register_packed_array_functions(ctx, global);

  JS_FreeValue(ctx, global);
}

void GodotBindings::setup_godot_class_constructor() {
  JSContext *ctx = context_->ctx();
  JSValue global = JS_GetGlobalObject(ctx);

  // Register __godot_new function
  JS_SetPropertyStr(ctx, global, "__godot_new",
                    JS_NewCFunction(ctx, js_godot_new, "__godot_new", 1));

  // Step 1: Create helper function
  const char *helper_code = R"(
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

        // Helper to find signal in class hierarchy (for built-in signals)
        globalThis.__findSignal = function(className, signalName) {
            var current = className;
            while (current && __godot_classes[current]) {
                var classInfo = __godot_classes[current];
                if (classInfo.signals) {
                    for (var i = 0; i < classInfo.signals.length; i++) {
                        if (classInfo.signals[i] === signalName) return true;
                    }
                }
                current = classInfo.parent;
            }
            return false;
        };
        'helper done';
    )";

  JSValue result = JS_Eval(ctx, helper_code, strlen(helper_code), "<helper>",
                           JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(result)) {
    JSValue exception = JS_GetException(ctx);
    const char *err = JS_ToCString(ctx, exception);
    UtilityFunctions::printerr("Failed to setup JS helper: ",
                               err ? err : "unknown");
    if (err)
      JS_FreeCString(ctx, err);
    JS_FreeValue(ctx, exception);
    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, global);
    return;
  }
  JS_FreeValue(ctx, result);

  // Step 2: Create proxy handler
  const char *proxy_code = R"(
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
                            // Don't unwrap __handle - js_to_variant handles proxied objects correctly
                            args.push(arguments[i]);
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

                // GDScript 4.x signal syntax: node.signal_name.connect(callback)
                // Check built-in signals first (fast), then custom signals (runtime check)
                var isBuiltinSignal = __findSignal(target.__class, prop);
                var isSignal = isBuiltinSignal || __godot_has_signal(target.__handle, prop);

                if (isSignal) {
                    var handle = target.__handle;
                    var signalName = prop;
                    return {
                        connect: function(callback) {
                            return __godot_connect(handle, signalName, callback);
                        },
                        emit: function() {
                            var args = [handle, signalName];
                            for (var i = 0; i < arguments.length; i++) {
                                // Don't unwrap __handle - js_to_variant handles proxied objects correctly
                                args.push(arguments[i]);
                            }
                            return __godot_emit_signal.apply(null, args);
                        }
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

                // Check if this is a JS script method first (has priority)
                if (__godot_has_script_method(target.__handle, prop)) {
                    var handle = target.__handle;
                    return function() {
                        var args = [handle, prop];
                        for (var i = 0; i < arguments.length; i++) {
                            // Don't unwrap __handle - js_to_variant handles proxied objects correctly
                            args.push(arguments[i]);
                        }
                        return __godot_call_script_method.apply(null, args);
                    };
                }

                // Fallback: use generic runtime property/method access for unbound objects (e.g., GDScript classes)
                // This allows accessing properties and methods on GDScript autoloads and custom classes
                var handle = target.__handle;

                // Check if it's a method first
                if (__godot_has_method(handle, prop)) {
                    return function() {
                        var args = [handle, prop];
                        for (var i = 0; i < arguments.length; i++) {
                            // Don't unwrap __handle - js_to_variant handles proxied objects correctly
                            args.push(arguments[i]);
                        }
                        return __godot_call.apply(null, args);
                    };
                }

                // Otherwise, get it as a property
                return __godot_get(handle, prop);
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

                // Fallback: use generic runtime property setter for unbound objects
                // NOTE: Don't unwrap __handle here - js_to_variant handles proxied objects correctly
                // by checking for __handle property and looking up the Object in the registry
                __godot_set(target.__handle, prop, value);
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
                // Check for signals (built-in and custom)
                if (__findSignal(target.__class, prop) || __godot_has_signal(target.__handle, prop)) return true;
                return __findBinding(target.__class, prop, 'method') !== null ||
                       __findBinding(target.__class, prop, 'property') !== null;
            }
        };
        'proxy handler done';
    )";

  result = JS_Eval(ctx, proxy_code, strlen(proxy_code), "<proxy>",
                   JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(result)) {
    JSValue exception = JS_GetException(ctx);
    const char *err = JS_ToCString(ctx, exception);
    UtilityFunctions::printerr("Failed to setup JS proxy handler: ",
                               err ? err : "unknown");
    if (err)
      JS_FreeCString(ctx, err);
    JS_FreeValue(ctx, exception);
    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, global);
    return;
  }
  JS_FreeValue(ctx, result);

  // Step 3: Create object factory
  const char *factory_code = R"(
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

        // Array proxy handler for zero-copy access to Godot arrays
        globalThis.__godot_array_proxy_handler = {
            get: function(target, prop, receiver) {
                if (prop === '__array_handle') return target.__array_handle;
                if (prop === '__is_godot_array') return true;
                if (typeof prop === 'symbol') {
                    if (prop === Symbol.toStringTag) return 'GodotArray';
                    if (prop === Symbol.iterator) {
                        var handle = target.__array_handle;
                        return function() {
                            var idx = 0;
                            var len = __godot_array_size(handle);
                            return {
                                next: function() {
                                    if (idx < len) {
                                        return { value: __godot_array_get(handle, idx++), done: false };
                                    }
                                    return { done: true };
                                }
                            };
                        };
                    }
                    return undefined;
                }
                if (prop === 'length') {
                    return __godot_array_size(target.__array_handle);
                }
                if (prop === 'toString') {
                    return function() { return '[GodotArray]'; };
                }
                if (prop === 'push') {
                    var handle = target.__array_handle;
                    return function(value) {
                        return __godot_array_push(handle, value);
                    };
                }
                if (prop === 'pop') {
                    var handle = target.__array_handle;
                    return function() {
                        return __godot_array_pop(handle);
                    };
                }
                if (prop === 'forEach') {
                    var handle = target.__array_handle;
                    return function(callback) {
                        var size = __godot_array_size(handle);
                        for (var i = 0; i < size; i++) {
                            callback(__godot_array_get(handle, i), i);
                        }
                    };
                }
                if (prop === 'map') {
                    var handle = target.__array_handle;
                    return function(callback) {
                        var size = __godot_array_size(handle);
                        var result = [];
                        for (var i = 0; i < size; i++) {
                            result.push(callback(__godot_array_get(handle, i), i));
                        }
                        return result;
                    };
                }
                if (prop === 'filter') {
                    var handle = target.__array_handle;
                    return function(callback) {
                        var size = __godot_array_size(handle);
                        var result = [];
                        for (var i = 0; i < size; i++) {
                            var item = __godot_array_get(handle, i);
                            if (callback(item, i)) {
                                result.push(item);
                            }
                        }
                        return result;
                    };
                }
                // Numeric index access
                var index = parseInt(prop);
                if (!isNaN(index) && index >= 0) {
                    return __godot_array_get(target.__array_handle, index);
                }
                return undefined;
            },
            set: function(target, prop, value) {
                var index = parseInt(prop);
                if (!isNaN(index) && index >= 0) {
                    __godot_array_set(target.__array_handle, index, value);
                    return true;
                }
                return false;
            },
            has: function(target, prop) {
                if (prop === 'length' || prop === '__array_handle' || prop === '__is_godot_array') return true;
                var index = parseInt(prop);
                if (!isNaN(index) && index >= 0) {
                    return index < __godot_array_size(target.__array_handle);
                }
                return prop === 'push' || prop === 'pop' || prop === 'forEach' || prop === 'map' || prop === 'filter';
            }
        };

        // Wrap a Godot array handle with a proxy
        // Uses __create_array_handle_wrapper to create a proper JSObjectClass with finalizer
        // This ensures ArrayRegistry::release_handle() is called when the proxy is garbage collected
        globalThis.__wrap_godot_array = function(handle) {
            // Create wrapper with finalizer - when GC'd, this calls ArrayRegistry::release_handle()
            var target = __create_array_handle_wrapper(handle);
            return new Proxy(target, __godot_array_proxy_handler);
        };

        // Packed array proxy handler for zero-copy access to Godot packed arrays
        globalThis.__packed_array_proxy_handler = {
            get: function(target, prop, receiver) {
                if (prop === '__packed_handle') return target.__packed_handle;
                if (prop === '__packed_type') return target.__packed_type;
                if (prop === '__is_packed_array') return true;
                if (typeof prop === 'symbol') {
                    if (prop === Symbol.toStringTag) return target.__packed_type || 'PackedArray';
                    if (prop === Symbol.iterator) {
                        var handle = target.__packed_handle;
                        return function() {
                            var idx = 0;
                            var len = __packed_array_size(handle);
                            return {
                                next: function() {
                                    if (idx < len) {
                                        return { value: __packed_array_get(handle, idx++), done: false };
                                    }
                                    return { done: true };
                                }
                            };
                        };
                    }
                    return undefined;
                }
                if (prop === 'length') {
                    return __packed_array_size(target.__packed_handle);
                }
                if (prop === 'toString') {
                    var type = target.__packed_type || 'PackedArray';
                    return function() { return '[' + type + ']'; };
                }
                if (prop === 'forEach') {
                    var handle = target.__packed_handle;
                    return function(callback) {
                        var size = __packed_array_size(handle);
                        for (var i = 0; i < size; i++) {
                            callback(__packed_array_get(handle, i), i);
                        }
                    };
                }
                if (prop === 'map') {
                    var handle = target.__packed_handle;
                    return function(callback) {
                        var size = __packed_array_size(handle);
                        var result = [];
                        for (var i = 0; i < size; i++) {
                            result.push(callback(__packed_array_get(handle, i), i));
                        }
                        return result;
                    };
                }
                if (prop === 'filter') {
                    var handle = target.__packed_handle;
                    return function(callback) {
                        var size = __packed_array_size(handle);
                        var result = [];
                        for (var i = 0; i < size; i++) {
                            var item = __packed_array_get(handle, i);
                            if (callback(item, i)) {
                                result.push(item);
                            }
                        }
                        return result;
                    };
                }
                if (prop === 'resize') {
                    var handle = target.__packed_handle;
                    var type = target.__packed_type;
                    return function(newSize) {
                        // Call type-specific resize function
                        var fn = globalThis['__' + type.toLowerCase().replace(/array$/, '_array') + '_resize'];
                        if (!fn) {
                            // Try the snake_case version for packed arrays
                            var snakeType = type.replace(/([A-Z])/g, function(m) { return '_' + m.toLowerCase(); }).replace(/^_/, '');
                            fn = globalThis['__' + snakeType + '_resize'];
                        }
                        return fn ? fn(handle, newSize) : false;
                    };
                }
                if (prop === 'push_back' || prop === 'push') {
                    var handle = target.__packed_handle;
                    var type = target.__packed_type;
                    return function(value) {
                        // Call type-specific push function
                        var fn = globalThis['__' + type.toLowerCase().replace(/array$/, '_array') + '_push'];
                        if (!fn) {
                            var snakeType = type.replace(/([A-Z])/g, function(m) { return '_' + m.toLowerCase(); }).replace(/^_/, '');
                            fn = globalThis['__' + snakeType + '_push'];
                        }
                        return fn ? fn(handle, value) : false;
                    };
                }
                // toArray() method - converts to native JS array in one C++ call (bulk decode)
                if (prop === 'toArray') {
                    var handle = target.__packed_handle;
                    var type = target.__packed_type;
                    return function() {
                        // Call type-specific bulk_decode function
                        var snakeType = type.replace(/([A-Z])/g, function(m) { return '_' + m.toLowerCase(); }).replace(/^_/, '');
                        var fn = globalThis['__' + snakeType + '_bulk_decode'];
                        return fn ? fn(handle) : [];
                    };
                }
                // append_array() method - bulk append JS array to packed array in one C++ call
                if (prop === 'append_array') {
                    var handle = target.__packed_handle;
                    var type = target.__packed_type;
                    return function(jsArray) {
                        // Call type-specific bulk_append function
                        var snakeType = type.replace(/([A-Z])/g, function(m) { return '_' + m.toLowerCase(); }).replace(/^_/, '');
                        var fn = globalThis['__' + snakeType + '_bulk_append'];
                        return fn ? fn(handle, jsArray) : false;
                    };
                }
                // PackedByteArray-specific encode/decode methods
                if (target.__packed_type === 'PackedByteArray') {
                    if (prop === 'encode_float') {
                        var handle = target.__packed_handle;
                        return function(byte_offset, value) {
                            return __packed_byte_array_encode_float(handle, byte_offset, value);
                        };
                    }
                    if (prop === 'encode_double') {
                        var handle = target.__packed_handle;
                        return function(byte_offset, value) {
                            return __packed_byte_array_encode_double(handle, byte_offset, value);
                        };
                    }
                    if (prop === 'encode_u32') {
                        var handle = target.__packed_handle;
                        return function(byte_offset, value) {
                            return __packed_byte_array_encode_u32(handle, byte_offset, value);
                        };
                    }
                    if (prop === 'encode_s32') {
                        var handle = target.__packed_handle;
                        return function(byte_offset, value) {
                            return __packed_byte_array_encode_s32(handle, byte_offset, value);
                        };
                    }
                    if (prop === 'encode_u64') {
                        var handle = target.__packed_handle;
                        return function(byte_offset, value) {
                            return __packed_byte_array_encode_u64(handle, byte_offset, value);
                        };
                    }
                    if (prop === 'encode_s64') {
                        var handle = target.__packed_handle;
                        return function(byte_offset, value) {
                            return __packed_byte_array_encode_s64(handle, byte_offset, value);
                        };
                    }
                    if (prop === 'decode_float') {
                        var handle = target.__packed_handle;
                        return function(byte_offset) {
                            return __packed_byte_array_decode_float(handle, byte_offset);
                        };
                    }
                    if (prop === 'decode_double') {
                        var handle = target.__packed_handle;
                        return function(byte_offset) {
                            return __packed_byte_array_decode_double(handle, byte_offset);
                        };
                    }
                    if (prop === 'decode_u32') {
                        var handle = target.__packed_handle;
                        return function(byte_offset) {
                            return __packed_byte_array_decode_u32(handle, byte_offset);
                        };
                    }
                    if (prop === 'decode_s32') {
                        var handle = target.__packed_handle;
                        return function(byte_offset) {
                            return __packed_byte_array_decode_s32(handle, byte_offset);
                        };
                    }
                    if (prop === 'decode_u64') {
                        var handle = target.__packed_handle;
                        return function(byte_offset) {
                            return __packed_byte_array_decode_u64(handle, byte_offset);
                        };
                    }
                    if (prop === 'decode_s64') {
                        var handle = target.__packed_handle;
                        return function(byte_offset) {
                            return __packed_byte_array_decode_s64(handle, byte_offset);
                        };
                    }
                    // Reinterpret methods - convert byte buffer to other packed array types (zero-copy)
                    if (prop === 'as_vector3_array') {
                        var handle = target.__packed_handle;
                        return function(count) {
                            return __packed_byte_array_to_packed_vector3_array(handle, count);
                        };
                    }
                    if (prop === 'as_vector2_array') {
                        var handle = target.__packed_handle;
                        return function(count) {
                            return __packed_byte_array_to_packed_vector2_array(handle, count);
                        };
                    }
                    if (prop === 'as_int32_array') {
                        var handle = target.__packed_handle;
                        return function(count) {
                            return __packed_byte_array_to_packed_int32_array(handle, count);
                        };
                    }
                }
                // Numeric index access
                var index = parseInt(prop);
                if (!isNaN(index) && index >= 0) {
                    return __packed_array_get(target.__packed_handle, index);
                }
                return undefined;
            },
            has: function(target, prop) {
                if (prop === 'length' || prop === '__packed_handle' || prop === '__is_packed_array') return true;
                var index = parseInt(prop);
                if (!isNaN(index) && index >= 0) {
                    return index < __packed_array_size(target.__packed_handle);
                }
                if (prop === 'forEach' || prop === 'map' || prop === 'filter' || prop === 'resize' || prop === 'push_back' || prop === 'push' || prop === 'toArray' || prop === 'append_array') return true;
                // PackedByteArray-specific methods
                if (target.__packed_type === 'PackedByteArray') {
                    if (prop === 'encode_float' || prop === 'encode_double' || prop === 'encode_u32' ||
                        prop === 'encode_s32' || prop === 'encode_u64' || prop === 'encode_s64' ||
                        prop === 'decode_float' || prop === 'decode_double' || prop === 'decode_u32' ||
                        prop === 'decode_s32' || prop === 'decode_u64' || prop === 'decode_s64' ||
                        prop === 'as_vector3_array' || prop === 'as_vector2_array' || prop === 'as_int32_array') return true;
                }
                return false;
            }
        };

        // Wrap a packed array handle with a proxy
        // Uses __create_array_handle_wrapper to create a proper JSObjectClass with finalizer
        // This ensures ArrayRegistry::release_handle() is called when the proxy is garbage collected
        globalThis.__wrap_packed_array = function(handle, type) {
            // Create wrapper with finalizer - when GC'd, this calls ArrayRegistry::release_handle()
            var target = __create_array_handle_wrapper(handle);
            // Also set packed-specific properties for JS-side access
            target.__packed_handle = handle;
            target.__packed_type = type;
            return new Proxy(target, __packed_array_proxy_handler);
        };

        // Math type proxy handler for zero-copy access to Vector2, Vector3, Color, etc.
        globalThis.__math_type_proxy_handler = {
            get: function(target, prop, receiver) {
                if (prop === '__math_handle') return target.__math_handle;
                if (prop === '__math_type') return target.__math_type;
                if (prop === '__is_math_type') return true;
                if (typeof prop === 'symbol') {
                    if (prop === Symbol.toStringTag) return target.__math_type || 'MathType';
                    return undefined;
                }
                if (prop === 'toString') {
                    var type = target.__math_type || 'MathType';
                    return function() { return '[' + type + ']'; };
                }
                // Property access (x, y, z, w, r, g, b, a, etc.)
                return __math_get_property(target.__math_handle, prop);
            },
            set: function(target, prop, value) {
                if (prop === '__math_handle' || prop === '__math_type') {
                    target[prop] = value;
                    return true;
                }
                // Property modification (x, y, z, w, r, g, b, a, etc.)
                return __math_set_property(target.__math_handle, prop, value) === true;
            },
            has: function(target, prop) {
                if (prop === '__math_handle' || prop === '__math_type' || prop === '__is_math_type') return true;
                var type = target.__math_type;
                if (type === 'Vector2') return prop === 'x' || prop === 'y';
                if (type === 'Vector3') return prop === 'x' || prop === 'y' || prop === 'z';
                if (type === 'Vector4' || type === 'Quaternion') return prop === 'x' || prop === 'y' || prop === 'z' || prop === 'w';
                if (type === 'Color') return prop === 'r' || prop === 'g' || prop === 'b' || prop === 'a';
                if (type === 'Transform3D') return prop === 'origin' || prop === 'basis';
                return false;
            }
        };

        // Wrap a math type handle with a proxy
        // Uses __create_array_handle_wrapper to create a proper JSObjectClass with finalizer
        // This ensures ArrayRegistry::release_handle() is called when the proxy is garbage collected
        globalThis.__wrap_math_type = function(handle, type) {
            // Create wrapper with finalizer - when GC'd, this calls ArrayRegistry::release_handle()
            var target = __create_array_handle_wrapper(handle);
            // Also set math-specific properties for JS-side access
            target.__math_handle = handle;
            target.__math_type = type;
            return new Proxy(target, __math_type_proxy_handler);
        };
        'factory done';
    )";

  result = JS_Eval(ctx, factory_code, strlen(factory_code), "<factory>",
                   JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(result)) {
    JSValue exception = JS_GetException(ctx);
    const char *err = JS_ToCString(ctx, exception);
    UtilityFunctions::printerr("Failed to setup JS factory: ",
                               err ? err : "unknown");
    if (err)
      JS_FreeCString(ctx, err);
    JS_FreeValue(ctx, exception);
    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, global);
    return;
  }
  JS_FreeValue(ctx, result);

  // Step 4: Create constructors (only for instantiable classes)
  const char *ctor_code = R"(
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

  result = JS_Eval(ctx, ctor_code, strlen(ctor_code), "<constructors>",
                   JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(result)) {
    JSValue exception = JS_GetException(ctx);
    const char *err = JS_ToCString(ctx, exception);
    UtilityFunctions::printerr("Failed to setup JS constructors: ",
                               err ? err : "unknown");
    if (err)
      JS_FreeCString(ctx, err);
    JS_FreeValue(ctx, exception);
  }
  JS_FreeValue(ctx, result);

  JS_FreeValue(ctx, global);
}

// setup_proxy_handler is now a no-op since the proxy uses __godot_classes
// directly The generated bindings are registered by register_all_classes()
// before this is called
void GodotBindings::setup_proxy_handler() {
  // No additional setup needed - __godot_classes is already populated
}

// Global function: __godot_new(class_name) - creates a new Godot object with JS
// wrapper
JSValue GodotBindings::js_godot_new(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  if (argc < 1) {
    return JS_ThrowTypeError(ctx, "Expected class name");
  }

  const char *class_name_cstr = JS_ToCString(ctx, argv[0]);
  if (!class_name_cstr) {
    return JS_ThrowTypeError(ctx, "Invalid class name");
  }

  String class_name = class_name_cstr;
  JS_FreeCString(ctx, class_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  SafeWrapper *wrapper = qjs_ctx->get_safe_wrapper();
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
  JS_SetPropertyStr(ctx, obj, "__class",
                    JS_NewString(ctx, class_name.utf8().get_data()));

  // Return the wrapper object. Properties will be accessed via JS code
  // that uses __godot_get/__godot_set with the handle
  return obj;
}

// Global function: load(path) - loads a resource (but NOT scenes)
// Scenes must be loaded via sandbox.load_scene() or sandbox.async_load_scene()
// to ensure proper sandbox association for any JS scripts inside
JSValue GodotBindings::js_load(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv) {
  if (argc < 1) {
    return JS_ThrowTypeError(ctx, "Expected resource path");
  }

  const char *path_cstr = JS_ToCString(ctx, argv[0]);
  if (!path_cstr) {
    return JS_ThrowTypeError(ctx, "Invalid path");
  }

  String path = path_cstr;
  JS_FreeCString(ctx, path_cstr);

  // Block scene loading - scenes with JS scripts must be loaded through
  // sandbox.load_scene() to ensure scripts are properly associated with the
  // sandbox
  String path_lower = path.to_lower();
  if (path_lower.ends_with(".tscn") || path_lower.ends_with(".scn")) {
    return JS_ThrowTypeError(
        ctx, "Cannot load scenes with load(). Use sandbox.load_scene() or "
             "sandbox.load_scene_async() instead to ensure proper sandbox "
             "association.");
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  SafeWrapper *wrapper = qjs_ctx->get_safe_wrapper();
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

// Global function: __godot_connect(handle, signal_name, callback) - connects a
// JS callback to a Godot signal
JSValue GodotBindings::js_godot_connect(JSContext *ctx, JSValueConst this_val,
                                        int argc, JSValueConst *argv) {
  if (argc < 3) {
    return JS_ThrowTypeError(
        ctx,
        "__godot_connect requires 3 arguments: handle, signal_name, callback");
  }

  // Get object handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be an object handle");
  }

  // Get signal name
  const char *signal_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!signal_name_cstr) {
    return JS_ThrowTypeError(ctx,
                             "Second argument must be a signal name string");
  }
  String signal_name = signal_name_cstr;
  JS_FreeCString(ctx, signal_name_cstr);

  // Get callback (must be a function)
  if (!JS_IsFunction(ctx, argv[2])) {
    return JS_ThrowTypeError(ctx, "Third argument must be a callback function");
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  // Get the target object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_ThrowTypeError(ctx, "Invalid object handle");
  }

  // Get the signal registry
  SignalRegistry *signal_registry = qjs_ctx->get_signal_registry();
  if (!signal_registry) {
    return JS_ThrowInternalError(ctx, "SignalRegistry not initialized");
  }

  // Connect the callback
  // Note: SignalRegistry::connect() handles JS_DupValue internally
  uint64_t connection_id =
      signal_registry->connect(target, StringName(signal_name), argv[2]);

  if (connection_id == 0) {
    return JS_ThrowTypeError(ctx, "Failed to connect signal");
  }

  return JS_NewInt64(ctx, connection_id);
}

// Global function: __godot_emit_signal(handle, signal_name, ...args) - emits a
// signal from an object
JSValue GodotBindings::js_godot_emit_signal(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 2) {
    return JS_ThrowTypeError(ctx, "__godot_emit_signal requires at least 2 "
                                  "arguments: handle, signal_name");
  }

  // Get object handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be an object handle");
  }

  // Get signal name
  const char *signal_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!signal_name_cstr) {
    return JS_ThrowTypeError(ctx,
                             "Second argument must be a signal name string");
  }
  String signal_name = signal_name_cstr;
  JS_FreeCString(ctx, signal_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  // Get the target object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_ThrowTypeError(ctx, "Invalid object handle");
  }

  // Convert additional arguments to Godot Variants for emit_signal call
  // Note: The blocklist only affects JS->Godot calls through SafeWrapper.
  // C++ code here is trusted and can call emit_signal directly.
  Vector<Variant> args_storage;
  args_storage.push_back(StringName(signal_name));
  for (int i = 2; i < argc; i++) {
    args_storage.push_back(qjs_ctx->js_to_variant(argv[i]));
  }

  // Build args pointer array
  Vector<const Variant *> args_ptrs;
  for (int i = 0; i < args_storage.size(); i++) {
    args_ptrs.push_back(&args_storage[i]);
  }

  // Use Variant::callp to emit signal with error capture
  Variant target_variant = target;
  Variant result;
  GDExtensionCallError call_error;
  const Variant **args_ptr = const_cast<const Variant **>(args_ptrs.ptr());
  target_variant.callp(StringName("emit_signal"), args_ptr, args_ptrs.size(),
                       result, call_error);

  // Check for errors and throw JS exception if needed
  if (call_error.error != GDEXTENSION_CALL_OK) {
    String error_msg;
    switch (call_error.error) {
    case GDEXTENSION_CALL_ERROR_INVALID_METHOD:
      error_msg = "emit_signal: invalid method";
      break;
    case GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT:
      error_msg = "emit_signal: invalid argument at index " +
                  String::num_int64(call_error.argument);
      break;
    case GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS:
      error_msg = "emit_signal '" + signal_name + "': too many arguments";
      break;
    case GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS:
      error_msg = "emit_signal '" + signal_name + "': too few arguments";
      break;
    default:
      error_msg = "emit_signal '" + signal_name + "': unknown error";
      break;
    }
    return JS_ThrowTypeError(ctx, "%s", error_msg.utf8().get_data());
  }

  return JS_UNDEFINED;
}

// Global function: __godot_await_signal(handle, signal_name) - returns a
// Promise that resolves when the signal fires
JSValue GodotBindings::js_godot_await_signal(JSContext *ctx,
                                             JSValueConst this_val, int argc,
                                             JSValueConst *argv) {
  if (argc < 2) {
    return JS_ThrowTypeError(
        ctx, "__godot_await_signal requires 2 arguments: handle, signal_name");
  }

  // Get object handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be an object handle");
  }

  // Get signal name
  const char *signal_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!signal_name_cstr) {
    return JS_ThrowTypeError(ctx,
                             "Second argument must be a signal name string");
  }
  String signal_name = signal_name_cstr;
  JS_FreeCString(ctx, signal_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  // Get the target object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_ThrowTypeError(ctx, "Invalid object handle");
  }

  // Get the signal registry
  SignalRegistry *signal_registry = qjs_ctx->get_signal_registry();
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
  JS_SetPropertyStr(ctx, state_obj, "connection_id",
                    JS_NewInt64(ctx, 0)); // Will be set after connect

  // Create a callback function that resolves the promise and disconnects
  // We use a trampoline approach: store state and call __resolve_signal_promise
  // from JS
  const char *callback_code = R"(
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

  JSValue callback_factory = JS_Eval(ctx, callback_code, strlen(callback_code),
                                     "<await_signal>", JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(callback_factory)) {
    JS_FreeValue(ctx, promise);
    JS_FreeValue(ctx, state_obj);
    return callback_factory;
  }

  // Call the factory with state to get the actual callback
  JSValue callback =
      JS_Call(ctx, callback_factory, JS_UNDEFINED, 1, &state_obj);
  JS_FreeValue(ctx, callback_factory);
  JS_FreeValue(ctx, state_obj);

  if (JS_IsException(callback)) {
    JS_FreeValue(ctx, promise);
    return callback;
  }

  // Connect the callback
  uint64_t connection_id =
      signal_registry->connect(target, StringName(signal_name), callback);
  JS_FreeValue(ctx, callback);

  if (connection_id == 0) {
    JS_FreeValue(ctx, promise);
    return JS_ThrowTypeError(ctx, "Failed to connect signal");
  }

  return promise;
}

// GodotObject finalizer - called when JS object is garbage collected
void GodotBindings::godot_object_finalizer(JSRuntime *rt, JSValueConst val) {
  JSClassID class_id;
  void *ptr = JS_GetAnyOpaque(val, &class_id);
  if (!ptr)
    return;

  // Opaque data contains both handle and registry pointer
  // This allows correct cleanup even with multiple contexts sharing a runtime
  GodotObjectData *data = static_cast<GodotObjectData *>(ptr);

  if (data->registry) {
    data->registry->release_handle(data->handle);
  }

  // Free the data struct allocated by js_malloc
  js_free_rt(rt, data);
}

// GodotArray finalizer - called when JS array proxy is garbage collected
// Releases Array/PackedArray/RID/MathType handles from ArrayRegistry
void GodotBindings::godot_array_finalizer(JSRuntime *rt, JSValueConst val) {
  JSClassID class_id;
  void *ptr = JS_GetAnyOpaque(val, &class_id);
  if (!ptr)
    return;

  // Opaque data contains both handle and registry pointer
  GodotArrayData *data = static_cast<GodotArrayData *>(ptr);

  if (data->registry) {
    data->registry->release_handle(data->handle);
  }

  // Free the data struct allocated by js_malloc
  js_free_rt(rt, data);
}

// NOTE: Math type constructors (Vector2, Vector3, Color, etc.) are now
// generated See generated/math_constructors.gen.cpp

JSValue GodotBindings::variant_to_js(const Variant &value) {
  return context_->variant_to_js(value);
}

Variant GodotBindings::js_to_variant(JSValue value) {
  return context_->js_to_variant(value);
}

// Global function: __godot_tween_callback(tween_handle, callback) - add a
// callback to a tween
JSValue GodotBindings::js_godot_tween_callback(JSContext *ctx,
                                               JSValueConst this_val, int argc,
                                               JSValueConst *argv) {
  if (argc < 2) {
    return JS_ThrowTypeError(
        ctx,
        "__godot_tween_callback requires 2 arguments: tween_handle, callback");
  }

  // Get tween handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be a tween handle");
  }

  // Get callback (must be a function)
  if (!JS_IsFunction(ctx, argv[1])) {
    return JS_ThrowTypeError(ctx,
                             "Second argument must be a callback function");
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  // Get the tween object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
  }

  Object *obj = registry->get_object(handle);
  if (!obj) {
    return JS_ThrowTypeError(ctx, "Invalid tween handle");
  }

  Tween *tween = Object::cast_to<Tween>(obj);
  if (!tween) {
    return JS_ThrowTypeError(ctx, "Object is not a Tween");
  }

  // Get the signal registry to create a callable
  SignalRegistry *signal_registry = qjs_ctx->get_signal_registry();
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
  Object *ret_obj = result.ptr();
  int64_t ret_handle = registry->get_or_create_handle(ret_obj);
  String ret_class = ret_obj->get_class();

  // Create wrapped object
  const char *factory_code = "globalThis.__wrap_existing_godot_object";
  JSValue factory = JS_Eval(ctx, factory_code, strlen(factory_code),
                            "<tween_callback>", JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(factory)) {
    return factory;
  }

  JSValue args[2] = {JS_NewInt64(ctx, ret_handle),
                     JS_NewString(ctx, ret_class.utf8().get_data())};
  JSValue wrapped = JS_Call(ctx, factory, JS_UNDEFINED, 2, args);
  JS_FreeValue(ctx, factory);
  JS_FreeValue(ctx, args[0]);
  JS_FreeValue(ctx, args[1]);

  return wrapped;
}

// Global function: __godot_tween_method(tween_handle, callback, from, to,
// duration) - add a method tween
JSValue GodotBindings::js_godot_tween_method(JSContext *ctx,
                                             JSValueConst this_val, int argc,
                                             JSValueConst *argv) {
  if (argc < 5) {
    return JS_ThrowTypeError(ctx, "__godot_tween_method requires 5 arguments: "
                                  "tween_handle, callback, from, to, duration");
  }

  // Get tween handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be a tween handle");
  }

  // Get callback (must be a function)
  if (!JS_IsFunction(ctx, argv[1])) {
    return JS_ThrowTypeError(ctx,
                             "Second argument must be a callback function");
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  // Get the tween object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
  }

  Object *obj = registry->get_object(handle);
  if (!obj) {
    return JS_ThrowTypeError(ctx, "Invalid tween handle");
  }

  Tween *tween = Object::cast_to<Tween>(obj);
  if (!tween) {
    return JS_ThrowTypeError(ctx, "Object is not a Tween");
  }

  // Get the signal registry to create a callable
  SignalRegistry *signal_registry = qjs_ctx->get_signal_registry();
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
  Ref<MethodTweener> result =
      tween->tween_method(callable, from_val, to_val, duration);

  if (result.is_null()) {
    return JS_NULL;
  }

  // Return the MethodTweener as a wrapped object
  Object *ret_obj = result.ptr();
  int64_t ret_handle = registry->get_or_create_handle(ret_obj);
  String ret_class = ret_obj->get_class();

  // Create wrapped object
  const char *factory_code = "globalThis.__wrap_existing_godot_object";
  JSValue factory = JS_Eval(ctx, factory_code, strlen(factory_code),
                            "<tween_method>", JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(factory)) {
    return factory;
  }

  JSValue args[2] = {JS_NewInt64(ctx, ret_handle),
                     JS_NewString(ctx, ret_class.utf8().get_data())};
  JSValue wrapped = JS_Call(ctx, factory, JS_UNDEFINED, 2, args);
  JS_FreeValue(ctx, factory);
  JS_FreeValue(ctx, args[0]);
  JS_FreeValue(ctx, args[1]);

  return wrapped;
}

// Global function: __godot_has_script_method(handle, method_name) - check if
// object has JS script method
JSValue GodotBindings::js_godot_has_script_method(JSContext *ctx,
                                                  JSValueConst this_val,
                                                  int argc,
                                                  JSValueConst *argv) {
  if (argc < 2) {
    return JS_ThrowTypeError(
        ctx,
        "__godot_has_script_method requires 2 arguments: handle, method_name");
  }

  // Get object handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be an object handle");
  }

  // Get method name
  const char *method_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!method_name_cstr) {
    return JS_ThrowTypeError(ctx,
                             "Second argument must be a method name string");
  }
  String method_name = method_name_cstr;
  JS_FreeCString(ctx, method_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_FALSE;
  }

  // Get the target object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_FALSE;
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_FALSE;
  }

  // Check if object has a JS script
  Ref<Script> script = target->get_script();
  if (!script.is_valid()) {
    return JS_FALSE;
  }

  const JSScript *js_script = Object::cast_to<JSScript>(script.ptr());
  if (!js_script) {
    return JS_FALSE;
  }

  // Check if the script has this method
  return JS_NewBool(ctx, js_script->_has_method(StringName(method_name)));
}

// Global function: __godot_call_script_method(handle, method_name, ...args) -
// call JS script method on object
JSValue GodotBindings::js_godot_call_script_method(JSContext *ctx,
                                                   JSValueConst this_val,
                                                   int argc,
                                                   JSValueConst *argv) {
  if (argc < 2) {
    return JS_ThrowTypeError(ctx, "__godot_call_script_method requires at "
                                  "least 2 arguments: handle, method_name");
  }

  // Get object handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "First argument must be an object handle");
  }

  // Get method name
  const char *method_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!method_name_cstr) {
    return JS_ThrowTypeError(ctx,
                             "Second argument must be a method name string");
  }
  String method_name = method_name_cstr;
  JS_FreeCString(ctx, method_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "Context not initialized");
  }

  // Get the target object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "ObjectRegistry not initialized");
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_ThrowTypeError(ctx, "Invalid object handle");
  }

  // Check if object has a JS script
  Ref<Script> script = target->get_script();
  if (!script.is_valid()) {
    return JS_ThrowTypeError(ctx, "Object does not have a script");
  }

  JSScript *js_script = Object::cast_to<JSScript>(script.ptr());
  if (!js_script) {
    return JS_ThrowTypeError(ctx, "Object does not have a JavaScript script");
  }

  // Get the script instance from our registered instances
  JSScriptInstance *instance = js_script->get_instance(target);
  if (!instance) {
    return JS_ThrowTypeError(ctx, "Script instance not found");
  }

  // Convert JS arguments to Variant array
  Vector<Variant> variant_args;
  Vector<const Variant *> variant_arg_ptrs;
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
  const Variant **args_ptr =
      variant_arg_ptrs.size() > 0
          ? const_cast<const Variant **>(variant_arg_ptrs.ptr())
          : nullptr;
  bool success = instance->call_method(StringName(method_name), args_ptr,
                                       variant_args.size(), result, error);

  if (!success) {
    if (!error.is_empty()) {
      return JS_ThrowTypeError(ctx, "%s", error.utf8().get_data());
    }
    return JS_UNDEFINED;
  }

  return qjs_ctx->variant_to_js(result);
}

// Global function: __godot_has_signal(handle, signal_name) - check if object
// has a signal (built-in or custom)
JSValue GodotBindings::js_godot_has_signal(JSContext *ctx,
                                           JSValueConst this_val, int argc,
                                           JSValueConst *argv) {
  if (argc < 2) {
    return JS_FALSE;
  }

  // Get object handle
  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_FALSE;
  }

  // Get signal name
  const char *signal_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!signal_name_cstr) {
    return JS_FALSE;
  }
  String signal_name = signal_name_cstr;
  JS_FreeCString(ctx, signal_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_FALSE;
  }

  // Get the target object from the registry
  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_FALSE;
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_FALSE;
  }

  // Check if the object has this signal (works for both built-in and custom
  // signals)
  return JS_NewBool(ctx, target->has_signal(StringName(signal_name)));
}

// Generic property getter for objects without pre-generated bindings (e.g.,
// GDScript classes)
JSValue GodotBindings::js_godot_get(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  if (argc < 2) {
    return JS_ThrowTypeError(ctx,
                             "__godot_get requires handle and property name");
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "Invalid handle");
  }

  const char *prop_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!prop_name_cstr) {
    return JS_ThrowTypeError(ctx, "Invalid property name");
  }
  String prop_name = prop_name_cstr;
  JS_FreeCString(ctx, prop_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "No context available");
  }

  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "No object registry available");
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_ThrowReferenceError(ctx, "Object no longer exists");
  }

  // Use Godot's generic property getter
  Variant value = target->get(StringName(prop_name));
  return qjs_ctx->variant_to_js(value);
}

// Generic property setter for objects without pre-generated bindings
JSValue GodotBindings::js_godot_set(JSContext *ctx, JSValueConst this_val,
                                    int argc, JSValueConst *argv) {
  if (argc < 3) {
    return JS_ThrowTypeError(
        ctx, "__godot_set requires handle, property name, and value");
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "Invalid handle");
  }

  const char *prop_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!prop_name_cstr) {
    return JS_ThrowTypeError(ctx, "Invalid property name");
  }
  String prop_name = prop_name_cstr;
  JS_FreeCString(ctx, prop_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "No context available");
  }

  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "No object registry available");
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_ThrowReferenceError(ctx, "Object no longer exists");
  }

  // Convert JS value to Variant and set
  Variant value = qjs_ctx->js_to_variant(argv[2]);
  target->set(StringName(prop_name), value);
  return JS_UNDEFINED;
}

// Generic method caller for objects without pre-generated bindings
JSValue GodotBindings::js_godot_call(JSContext *ctx, JSValueConst this_val,
                                     int argc, JSValueConst *argv) {
  if (argc < 2) {
    return JS_ThrowTypeError(ctx,
                             "__godot_call requires handle and method name");
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "Invalid handle");
  }

  const char *method_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!method_name_cstr) {
    return JS_ThrowTypeError(ctx, "Invalid method name");
  }
  String method_name = method_name_cstr;
  JS_FreeCString(ctx, method_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_ThrowInternalError(ctx, "No context available");
  }

  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "No object registry available");
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_ThrowReferenceError(ctx, "Object no longer exists");
  }

  // Convert arguments (skip handle and method name)
  Array godot_args;
  for (int i = 2; i < argc; i++) {
    godot_args.append(qjs_ctx->js_to_variant(argv[i]));
  }

  // Call the method using callv
  Variant result = target->callv(StringName(method_name), godot_args);
  return qjs_ctx->variant_to_js(result);
}

// Check if an object has a method with the given name
JSValue GodotBindings::js_godot_has_method(JSContext *ctx,
                                           JSValueConst this_val, int argc,
                                           JSValueConst *argv) {
  if (argc < 2) {
    return JS_FALSE;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_FALSE;
  }

  const char *method_name_cstr = JS_ToCString(ctx, argv[1]);
  if (!method_name_cstr) {
    return JS_FALSE;
  }
  String method_name = method_name_cstr;
  JS_FreeCString(ctx, method_name_cstr);

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_FALSE;
  }

  ObjectRegistry *registry = qjs_ctx->get_object_registry();
  if (!registry) {
    return JS_FALSE;
  }

  Object *target = registry->get_object(handle);
  if (!target) {
    return JS_FALSE;
  }

  return JS_NewBool(ctx, target->has_method(StringName(method_name)));
}

// Array proxy functions for zero-copy access to Godot arrays

// __create_array_handle_wrapper(handle) - creates JSObjectClass with finalizer
// for array handle This is the key to preventing memory leaks - the finalizer
// calls ArrayRegistry::release_handle()
static JSValue js_create_array_handle_wrapper(JSContext *ctx,
                                              JSValueConst this_val, int argc,
                                              JSValueConst *argv) {
  if (argc < 1) {
    return JS_ThrowTypeError(
        ctx, "__create_array_handle_wrapper requires handle argument");
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_ThrowTypeError(ctx, "Invalid handle");
  }

  QuickJSContext *qjs_ctx = GodotBindings::get_context(ctx);
  if (!qjs_ctx || !qjs_ctx->get_bindings()) {
    return JS_ThrowInternalError(ctx, "No context available");
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry) {
    return JS_ThrowInternalError(ctx, "No array registry available");
  }

  // Create JS object with our array class (has finalizer)
  JSClassID class_id = qjs_ctx->get_bindings()->get_godot_array_class_id();
  JSValue wrapper = JS_NewObjectClass(ctx, class_id);
  if (JS_IsException(wrapper)) {
    return wrapper;
  }

  // Allocate opaque data to store handle and registry
  GodotArrayData *data =
      static_cast<GodotArrayData *>(js_malloc(ctx, sizeof(GodotArrayData)));
  if (!data) {
    JS_FreeValue(ctx, wrapper);
    return JS_ThrowOutOfMemory(ctx);
  }
  data->handle = handle;
  data->registry = registry;

  JS_SetOpaque(wrapper, data);

  // Also set __array_handle property for JS-side access
  JS_SetPropertyStr(ctx, wrapper, "__array_handle", JS_NewInt64(ctx, handle));

  return wrapper;
}

// __godot_array_get(handle, index) - get element at index
JSValue GodotBindings::js_godot_array_get(JSContext *ctx, JSValueConst this_val,
                                          int argc, JSValueConst *argv) {
  if (argc < 2) {
    return JS_UNDEFINED;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_UNDEFINED;
  }

  int64_t index;
  if (JS_ToInt64(ctx, &index, argv[1]) != 0) {
    return JS_UNDEFINED;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_UNDEFINED;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry) {
    return JS_UNDEFINED;
  }

  Array arr = registry->get_array(handle);
  if (index < 0 || index >= arr.size()) {
    return JS_UNDEFINED;
  }

  return qjs_ctx->variant_to_js(arr[index]);
}

// __godot_array_set(handle, index, value) - set element at index
JSValue GodotBindings::js_godot_array_set(JSContext *ctx, JSValueConst this_val,
                                          int argc, JSValueConst *argv) {
  if (argc < 3) {
    return JS_FALSE;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_FALSE;
  }

  int64_t index;
  if (JS_ToInt64(ctx, &index, argv[1]) != 0) {
    return JS_FALSE;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_FALSE;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry) {
    return JS_FALSE;
  }

  Array arr = registry->get_array(handle);
  if (index < 0 || index >= arr.size()) {
    return JS_FALSE;
  }

  arr[index] = qjs_ctx->js_to_variant(argv[2]);
  return JS_TRUE;
}

// __godot_array_size(handle) - get array size
JSValue GodotBindings::js_godot_array_size(JSContext *ctx,
                                           JSValueConst this_val, int argc,
                                           JSValueConst *argv) {
  if (argc < 1) {
    return JS_NewInt32(ctx, 0);
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_NewInt32(ctx, 0);
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_NewInt32(ctx, 0);
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry) {
    return JS_NewInt32(ctx, 0);
  }

  Array arr = registry->get_array(handle);
  return JS_NewInt32(ctx, arr.size());
}

// __godot_array_push(handle, value) - push element to end
JSValue GodotBindings::js_godot_array_push(JSContext *ctx,
                                           JSValueConst this_val, int argc,
                                           JSValueConst *argv) {
  if (argc < 2) {
    return JS_UNDEFINED;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_UNDEFINED;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_UNDEFINED;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry) {
    return JS_UNDEFINED;
  }

  Array arr = registry->get_array(handle);
  arr.push_back(qjs_ctx->js_to_variant(argv[1]));

  return JS_NewInt32(ctx, arr.size());
}

// __godot_array_pop(handle) - remove and return last element
JSValue GodotBindings::js_godot_array_pop(JSContext *ctx, JSValueConst this_val,
                                          int argc, JSValueConst *argv) {
  if (argc < 1) {
    return JS_UNDEFINED;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_UNDEFINED;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_UNDEFINED;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry) {
    return JS_UNDEFINED;
  }

  Array arr = registry->get_array(handle);
  if (arr.is_empty()) {
    return JS_UNDEFINED;
  }

  Variant last = arr.back();
  arr.pop_back();

  return qjs_ctx->variant_to_js(last);
}

// Packed array proxy functions for zero-copy access

// __packed_array_get(handle, index) - get element at index from any packed
// array type
JSValue GodotBindings::js_packed_array_get(JSContext *ctx,
                                           JSValueConst this_val, int argc,
                                           JSValueConst *argv) {
  if (argc < 2) {
    return JS_UNDEFINED;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_UNDEFINED;
  }

  int64_t index;
  if (JS_ToInt64(ctx, &index, argv[1]) != 0) {
    return JS_UNDEFINED;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_UNDEFINED;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_valid_handle(handle)) {
    return JS_UNDEFINED;
  }

  CollectionType type = registry->get_handle_type(handle);

  switch (type) {
  case CollectionType::PACKED_BYTE_ARRAY: {
    PackedByteArray arr = registry->get_packed_byte_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    return JS_NewInt32(ctx, arr[index]);
  }
  case CollectionType::PACKED_INT32_ARRAY: {
    PackedInt32Array arr = registry->get_packed_int32_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    return JS_NewInt32(ctx, arr[index]);
  }
  case CollectionType::PACKED_INT64_ARRAY: {
    PackedInt64Array arr = registry->get_packed_int64_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    return JS_NewInt64(ctx, arr[index]);
  }
  case CollectionType::PACKED_FLOAT32_ARRAY: {
    PackedFloat32Array arr = registry->get_packed_float32_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    return JS_NewFloat64(ctx, arr[index]);
  }
  case CollectionType::PACKED_FLOAT64_ARRAY: {
    PackedFloat64Array arr = registry->get_packed_float64_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    return JS_NewFloat64(ctx, arr[index]);
  }
  case CollectionType::PACKED_STRING_ARRAY: {
    PackedStringArray arr = registry->get_packed_string_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    return JS_NewString(ctx, arr[index].utf8().get_data());
  }
  case CollectionType::PACKED_VECTOR2_ARRAY: {
    PackedVector2Array arr = registry->get_packed_vector2_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    Vector2 v = arr[index];
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, v.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, v.y));
    return obj;
  }
  case CollectionType::PACKED_VECTOR3_ARRAY: {
    PackedVector3Array arr = registry->get_packed_vector3_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    Vector3 v = arr[index];
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, v.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, v.y));
    JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, v.z));
    return obj;
  }
  case CollectionType::PACKED_COLOR_ARRAY: {
    PackedColorArray arr = registry->get_packed_color_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    Color c = arr[index];
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "r", JS_NewFloat64(ctx, c.r));
    JS_SetPropertyStr(ctx, obj, "g", JS_NewFloat64(ctx, c.g));
    JS_SetPropertyStr(ctx, obj, "b", JS_NewFloat64(ctx, c.b));
    JS_SetPropertyStr(ctx, obj, "a", JS_NewFloat64(ctx, c.a));
    return obj;
  }
  case CollectionType::PACKED_VECTOR4_ARRAY: {
    PackedVector4Array arr = registry->get_packed_vector4_array(handle);
    if (index < 0 || index >= arr.size())
      return JS_UNDEFINED;
    Vector4 v = arr[index];
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, v.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, v.y));
    JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, v.z));
    JS_SetPropertyStr(ctx, obj, "w", JS_NewFloat64(ctx, v.w));
    return obj;
  }
  default:
    return JS_UNDEFINED;
  }
}

// __packed_array_size(handle) - get size of packed array
JSValue GodotBindings::js_packed_array_size(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 1) {
    return JS_NewInt32(ctx, 0);
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_NewInt32(ctx, 0);
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_NewInt32(ctx, 0);
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry) {
    return JS_NewInt32(ctx, 0);
  }

  return JS_NewInt64(ctx, registry->get_size(handle));
}

// __packed_array_type(handle) - get type string of packed array
JSValue GodotBindings::js_packed_array_type(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 1) {
    return JS_NewString(ctx, "unknown");
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_NewString(ctx, "unknown");
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_NewString(ctx, "unknown");
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_valid_handle(handle)) {
    return JS_NewString(ctx, "unknown");
  }

  CollectionType type = registry->get_handle_type(handle);
  switch (type) {
  case CollectionType::PACKED_BYTE_ARRAY:
    return JS_NewString(ctx, "PackedByteArray");
  case CollectionType::PACKED_INT32_ARRAY:
    return JS_NewString(ctx, "PackedInt32Array");
  case CollectionType::PACKED_INT64_ARRAY:
    return JS_NewString(ctx, "PackedInt64Array");
  case CollectionType::PACKED_FLOAT32_ARRAY:
    return JS_NewString(ctx, "PackedFloat32Array");
  case CollectionType::PACKED_FLOAT64_ARRAY:
    return JS_NewString(ctx, "PackedFloat64Array");
  case CollectionType::PACKED_STRING_ARRAY:
    return JS_NewString(ctx, "PackedStringArray");
  case CollectionType::PACKED_VECTOR2_ARRAY:
    return JS_NewString(ctx, "PackedVector2Array");
  case CollectionType::PACKED_VECTOR3_ARRAY:
    return JS_NewString(ctx, "PackedVector3Array");
  case CollectionType::PACKED_COLOR_ARRAY:
    return JS_NewString(ctx, "PackedColorArray");
  case CollectionType::PACKED_VECTOR4_ARRAY:
    return JS_NewString(ctx, "PackedVector4Array");
  default:
    return JS_NewString(ctx, "unknown");
  }
}

// __packed_array_push(handle, value) - push element to packed array
JSValue GodotBindings::js_packed_array_push(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 2) {
    return JS_FALSE;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_FALSE;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_FALSE;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_valid_handle(handle)) {
    return JS_FALSE;
  }

  CollectionType type = registry->get_handle_type(handle);

  switch (type) {
  case CollectionType::PACKED_VECTOR3_ARRAY: {
    PackedVector3Array *arr = registry->get_packed_vector3_array_ptr(handle);
    if (arr && JS_IsObject(argv[1])) {
      JSValue x_val = JS_GetPropertyStr(ctx, argv[1], "x");
      JSValue y_val = JS_GetPropertyStr(ctx, argv[1], "y");
      JSValue z_val = JS_GetPropertyStr(ctx, argv[1], "z");
      double x = 0, y = 0, z = 0;
      JS_ToFloat64(ctx, &x, x_val);
      JS_ToFloat64(ctx, &y, y_val);
      JS_ToFloat64(ctx, &z, z_val);
      arr->push_back(Vector3(x, y, z));
      JS_FreeValue(ctx, x_val);
      JS_FreeValue(ctx, y_val);
      JS_FreeValue(ctx, z_val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_INT32_ARRAY: {
    PackedInt32Array *arr = registry->get_packed_int32_array_ptr(handle);
    if (arr) {
      int64_t val = 0;
      JS_ToInt64(ctx, &val, argv[1]);
      arr->push_back((int32_t)val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_FLOAT32_ARRAY: {
    PackedFloat32Array *arr = registry->get_packed_float32_array_ptr(handle);
    if (arr) {
      double val = 0;
      JS_ToFloat64(ctx, &val, argv[1]);
      arr->push_back((float)val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_VECTOR2_ARRAY: {
    PackedVector2Array *arr = registry->get_packed_vector2_array_ptr(handle);
    if (arr && JS_IsObject(argv[1])) {
      JSValue x_val = JS_GetPropertyStr(ctx, argv[1], "x");
      JSValue y_val = JS_GetPropertyStr(ctx, argv[1], "y");
      double x = 0, y = 0;
      JS_ToFloat64(ctx, &x, x_val);
      JS_ToFloat64(ctx, &y, y_val);
      arr->push_back(Vector2(x, y));
      JS_FreeValue(ctx, x_val);
      JS_FreeValue(ctx, y_val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_COLOR_ARRAY: {
    PackedColorArray *arr = registry->get_packed_color_array_ptr(handle);
    if (arr && JS_IsObject(argv[1])) {
      JSValue r_val = JS_GetPropertyStr(ctx, argv[1], "r");
      JSValue g_val = JS_GetPropertyStr(ctx, argv[1], "g");
      JSValue b_val = JS_GetPropertyStr(ctx, argv[1], "b");
      JSValue a_val = JS_GetPropertyStr(ctx, argv[1], "a");
      double r = 0, g = 0, b = 0, a = 1;
      JS_ToFloat64(ctx, &r, r_val);
      JS_ToFloat64(ctx, &g, g_val);
      JS_ToFloat64(ctx, &b, b_val);
      if (!JS_IsUndefined(a_val))
        JS_ToFloat64(ctx, &a, a_val);
      arr->push_back(Color(r, g, b, a));
      JS_FreeValue(ctx, r_val);
      JS_FreeValue(ctx, g_val);
      JS_FreeValue(ctx, b_val);
      JS_FreeValue(ctx, a_val);
      return JS_TRUE;
    }
    break;
  }
  default:
    break;
  }
  return JS_FALSE;
}

// __packed_array_resize(handle, size) - resize packed array
JSValue GodotBindings::js_packed_array_resize(JSContext *ctx,
                                              JSValueConst this_val, int argc,
                                              JSValueConst *argv) {
  if (argc < 2) {
    return JS_FALSE;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_FALSE;
  }

  int64_t new_size;
  if (JS_ToInt64(ctx, &new_size, argv[1]) != 0) {
    return JS_FALSE;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_FALSE;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_valid_handle(handle)) {
    return JS_FALSE;
  }

  CollectionType type = registry->get_handle_type(handle);

  switch (type) {
  case CollectionType::PACKED_VECTOR3_ARRAY: {
    PackedVector3Array *arr = registry->get_packed_vector3_array_ptr(handle);
    if (arr) {
      arr->resize(new_size);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_INT32_ARRAY: {
    PackedInt32Array *arr = registry->get_packed_int32_array_ptr(handle);
    if (arr) {
      arr->resize(new_size);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_FLOAT32_ARRAY: {
    PackedFloat32Array *arr = registry->get_packed_float32_array_ptr(handle);
    if (arr) {
      arr->resize(new_size);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_VECTOR2_ARRAY: {
    PackedVector2Array *arr = registry->get_packed_vector2_array_ptr(handle);
    if (arr) {
      arr->resize(new_size);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_COLOR_ARRAY: {
    PackedColorArray *arr = registry->get_packed_color_array_ptr(handle);
    if (arr) {
      arr->resize(new_size);
      return JS_TRUE;
    }
    break;
  }
  default:
    break;
  }
  return JS_FALSE;
}

// __packed_array_set(handle, index, value) - set element at index
JSValue GodotBindings::js_packed_array_set(JSContext *ctx,
                                           JSValueConst this_val, int argc,
                                           JSValueConst *argv) {
  if (argc < 3) {
    return JS_FALSE;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_FALSE;
  }

  int64_t index;
  if (JS_ToInt64(ctx, &index, argv[1]) != 0) {
    return JS_FALSE;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_FALSE;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_valid_handle(handle)) {
    return JS_FALSE;
  }

  CollectionType type = registry->get_handle_type(handle);

  switch (type) {
  case CollectionType::PACKED_VECTOR3_ARRAY: {
    PackedVector3Array *arr = registry->get_packed_vector3_array_ptr(handle);
    if (arr && index >= 0 && index < arr->size() && JS_IsObject(argv[2])) {
      JSValue x_val = JS_GetPropertyStr(ctx, argv[2], "x");
      JSValue y_val = JS_GetPropertyStr(ctx, argv[2], "y");
      JSValue z_val = JS_GetPropertyStr(ctx, argv[2], "z");
      double x = 0, y = 0, z = 0;
      JS_ToFloat64(ctx, &x, x_val);
      JS_ToFloat64(ctx, &y, y_val);
      JS_ToFloat64(ctx, &z, z_val);
      arr->set(index, Vector3(x, y, z));
      JS_FreeValue(ctx, x_val);
      JS_FreeValue(ctx, y_val);
      JS_FreeValue(ctx, z_val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_INT32_ARRAY: {
    PackedInt32Array *arr = registry->get_packed_int32_array_ptr(handle);
    if (arr && index >= 0 && index < arr->size()) {
      int64_t val = 0;
      JS_ToInt64(ctx, &val, argv[2]);
      arr->set(index, (int32_t)val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_FLOAT32_ARRAY: {
    PackedFloat32Array *arr = registry->get_packed_float32_array_ptr(handle);
    if (arr && index >= 0 && index < arr->size()) {
      double val = 0;
      JS_ToFloat64(ctx, &val, argv[2]);
      arr->set(index, (float)val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_VECTOR2_ARRAY: {
    PackedVector2Array *arr = registry->get_packed_vector2_array_ptr(handle);
    if (arr && index >= 0 && index < arr->size() && JS_IsObject(argv[2])) {
      JSValue x_val = JS_GetPropertyStr(ctx, argv[2], "x");
      JSValue y_val = JS_GetPropertyStr(ctx, argv[2], "y");
      double x = 0, y = 0;
      JS_ToFloat64(ctx, &x, x_val);
      JS_ToFloat64(ctx, &y, y_val);
      arr->set(index, Vector2(x, y));
      JS_FreeValue(ctx, x_val);
      JS_FreeValue(ctx, y_val);
      return JS_TRUE;
    }
    break;
  }
  case CollectionType::PACKED_COLOR_ARRAY: {
    PackedColorArray *arr = registry->get_packed_color_array_ptr(handle);
    if (arr && index >= 0 && index < arr->size() && JS_IsObject(argv[2])) {
      JSValue r_val = JS_GetPropertyStr(ctx, argv[2], "r");
      JSValue g_val = JS_GetPropertyStr(ctx, argv[2], "g");
      JSValue b_val = JS_GetPropertyStr(ctx, argv[2], "b");
      JSValue a_val = JS_GetPropertyStr(ctx, argv[2], "a");
      double r = 0, g = 0, b = 0, a = 1;
      JS_ToFloat64(ctx, &r, r_val);
      JS_ToFloat64(ctx, &g, g_val);
      JS_ToFloat64(ctx, &b, b_val);
      if (!JS_IsUndefined(a_val))
        JS_ToFloat64(ctx, &a, a_val);
      arr->set(index, Color(r, g, b, a));
      JS_FreeValue(ctx, r_val);
      JS_FreeValue(ctx, g_val);
      JS_FreeValue(ctx, b_val);
      JS_FreeValue(ctx, a_val);
      return JS_TRUE;
    }
    break;
  }
  default:
    break;
  }
  return JS_FALSE;
}

// NOTE: Packed array constructors are now generated
// See generated/packed_array_bindings.gen.cpp

// Math type proxy functions for zero-copy access

// __math_get_property(handle, prop_name) - get property from math type
JSValue GodotBindings::js_math_get_property(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 2) {
    return JS_UNDEFINED;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_UNDEFINED;
  }

  const char *prop_name = JS_ToCString(ctx, argv[1]);
  if (!prop_name) {
    return JS_UNDEFINED;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    JS_FreeCString(ctx, prop_name);
    return JS_UNDEFINED;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_math_handle(handle)) {
    JS_FreeCString(ctx, prop_name);
    return JS_UNDEFINED;
  }

  Variant var = registry->get_math_variant(handle);
  String prop = prop_name;
  JS_FreeCString(ctx, prop_name);

  // Get property based on type
  switch (var.get_type()) {
  case Variant::VECTOR2: {
    Vector2 v = var;
    if (prop == "x")
      return JS_NewFloat64(ctx, v.x);
    if (prop == "y")
      return JS_NewFloat64(ctx, v.y);
    break;
  }
  case Variant::VECTOR3: {
    Vector3 v = var;
    if (prop == "x")
      return JS_NewFloat64(ctx, v.x);
    if (prop == "y")
      return JS_NewFloat64(ctx, v.y);
    if (prop == "z")
      return JS_NewFloat64(ctx, v.z);
    break;
  }
  case Variant::VECTOR4: {
    Vector4 v = var;
    if (prop == "x")
      return JS_NewFloat64(ctx, v.x);
    if (prop == "y")
      return JS_NewFloat64(ctx, v.y);
    if (prop == "z")
      return JS_NewFloat64(ctx, v.z);
    if (prop == "w")
      return JS_NewFloat64(ctx, v.w);
    break;
  }
  case Variant::COLOR: {
    Color c = var;
    if (prop == "r")
      return JS_NewFloat64(ctx, c.r);
    if (prop == "g")
      return JS_NewFloat64(ctx, c.g);
    if (prop == "b")
      return JS_NewFloat64(ctx, c.b);
    if (prop == "a")
      return JS_NewFloat64(ctx, c.a);
    break;
  }
  case Variant::QUATERNION: {
    Quaternion q = var;
    if (prop == "x")
      return JS_NewFloat64(ctx, q.x);
    if (prop == "y")
      return JS_NewFloat64(ctx, q.y);
    if (prop == "z")
      return JS_NewFloat64(ctx, q.z);
    if (prop == "w")
      return JS_NewFloat64(ctx, q.w);
    break;
  }
  case Variant::TRANSFORM3D: {
    Transform3D t = var;
    if (prop == "origin") {
      // Return a new handle for the origin vector
      uint64_t origin_handle = registry->create_math_handle(Variant(t.origin));
      if (origin_handle == 0)
        return JS_UNDEFINED;
      // Wrap via JS helper
      JSValue global = JS_GetGlobalObject(ctx);
      JSValue wrap_fn = JS_GetPropertyStr(ctx, global, "__wrap_math_type");
      if (JS_IsFunction(ctx, wrap_fn)) {
        JSValue args[2] = {JS_NewInt64(ctx, origin_handle),
                           JS_NewString(ctx, "Vector3")};
        JSValue result = JS_Call(ctx, wrap_fn, JS_UNDEFINED, 2, args);
        JS_FreeValue(ctx, args[0]);
        JS_FreeValue(ctx, args[1]);
        JS_FreeValue(ctx, wrap_fn);
        JS_FreeValue(ctx, global);
        return result;
      }
      JS_FreeValue(ctx, wrap_fn);
      JS_FreeValue(ctx, global);
    }
    // For basis, return as nested object (keep value copy for now, complex
    // structure)
    if (prop == "basis") {
      JSValue obj = JS_NewObject(ctx);
      JSValue x_row = JS_NewObject(ctx);
      JSValue y_row = JS_NewObject(ctx);
      JSValue z_row = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, x_row, "x", JS_NewFloat64(ctx, t.basis.rows[0].x));
      JS_SetPropertyStr(ctx, x_row, "y", JS_NewFloat64(ctx, t.basis.rows[0].y));
      JS_SetPropertyStr(ctx, x_row, "z", JS_NewFloat64(ctx, t.basis.rows[0].z));
      JS_SetPropertyStr(ctx, y_row, "x", JS_NewFloat64(ctx, t.basis.rows[1].x));
      JS_SetPropertyStr(ctx, y_row, "y", JS_NewFloat64(ctx, t.basis.rows[1].y));
      JS_SetPropertyStr(ctx, y_row, "z", JS_NewFloat64(ctx, t.basis.rows[1].z));
      JS_SetPropertyStr(ctx, z_row, "x", JS_NewFloat64(ctx, t.basis.rows[2].x));
      JS_SetPropertyStr(ctx, z_row, "y", JS_NewFloat64(ctx, t.basis.rows[2].y));
      JS_SetPropertyStr(ctx, z_row, "z", JS_NewFloat64(ctx, t.basis.rows[2].z));
      JS_SetPropertyStr(ctx, obj, "x", x_row);
      JS_SetPropertyStr(ctx, obj, "y", y_row);
      JS_SetPropertyStr(ctx, obj, "z", z_row);
      return obj;
    }
    break;
  }
  default:
    break;
  }
  return JS_UNDEFINED;
}

// __math_set_property(handle, prop_name, value) - set property on math type
JSValue GodotBindings::js_math_set_property(JSContext *ctx,
                                            JSValueConst this_val, int argc,
                                            JSValueConst *argv) {
  if (argc < 3) {
    return JS_FALSE;
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_FALSE;
  }

  const char *prop_name = JS_ToCString(ctx, argv[1]);
  if (!prop_name) {
    return JS_FALSE;
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    JS_FreeCString(ctx, prop_name);
    return JS_FALSE;
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_math_handle(handle)) {
    JS_FreeCString(ctx, prop_name);
    return JS_FALSE;
  }

  Variant *var_ptr = registry->get_math_variant_ptr(handle);
  if (!var_ptr) {
    JS_FreeCString(ctx, prop_name);
    return JS_FALSE;
  }

  String prop = prop_name;
  JS_FreeCString(ctx, prop_name);

  double new_val;
  if (JS_ToFloat64(ctx, &new_val, argv[2]) != 0) {
    return JS_FALSE;
  }

  // Set property based on type
  switch (var_ptr->get_type()) {
  case Variant::VECTOR2: {
    Vector2 v = *var_ptr;
    if (prop == "x") {
      v.x = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    if (prop == "y") {
      v.y = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    break;
  }
  case Variant::VECTOR3: {
    Vector3 v = *var_ptr;
    if (prop == "x") {
      v.x = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    if (prop == "y") {
      v.y = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    if (prop == "z") {
      v.z = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    break;
  }
  case Variant::VECTOR4: {
    Vector4 v = *var_ptr;
    if (prop == "x") {
      v.x = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    if (prop == "y") {
      v.y = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    if (prop == "z") {
      v.z = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    if (prop == "w") {
      v.w = new_val;
      *var_ptr = v;
      return JS_TRUE;
    }
    break;
  }
  case Variant::COLOR: {
    Color c = *var_ptr;
    if (prop == "r") {
      c.r = new_val;
      *var_ptr = c;
      return JS_TRUE;
    }
    if (prop == "g") {
      c.g = new_val;
      *var_ptr = c;
      return JS_TRUE;
    }
    if (prop == "b") {
      c.b = new_val;
      *var_ptr = c;
      return JS_TRUE;
    }
    if (prop == "a") {
      c.a = new_val;
      *var_ptr = c;
      return JS_TRUE;
    }
    break;
  }
  case Variant::QUATERNION: {
    Quaternion q = *var_ptr;
    if (prop == "x") {
      q.x = new_val;
      *var_ptr = q;
      return JS_TRUE;
    }
    if (prop == "y") {
      q.y = new_val;
      *var_ptr = q;
      return JS_TRUE;
    }
    if (prop == "z") {
      q.z = new_val;
      *var_ptr = q;
      return JS_TRUE;
    }
    if (prop == "w") {
      q.w = new_val;
      *var_ptr = q;
      return JS_TRUE;
    }
    break;
  }
  default:
    break;
  }
  return JS_FALSE;
}

// __math_get_type(handle) - get type name of math value
JSValue GodotBindings::js_math_get_type(JSContext *ctx, JSValueConst this_val,
                                        int argc, JSValueConst *argv) {
  if (argc < 1) {
    return JS_NewString(ctx, "unknown");
  }

  int64_t handle;
  if (JS_ToInt64(ctx, &handle, argv[0]) != 0) {
    return JS_NewString(ctx, "unknown");
  }

  QuickJSContext *qjs_ctx = get_context(ctx);
  if (!qjs_ctx) {
    return JS_NewString(ctx, "unknown");
  }

  ArrayRegistry *registry = qjs_ctx->get_array_registry();
  if (!registry || !registry->is_math_handle(handle)) {
    return JS_NewString(ctx, "unknown");
  }

  CollectionType type = registry->get_handle_type(handle);
  switch (type) {
  case CollectionType::MATH_VECTOR2:
    return JS_NewString(ctx, "Vector2");
  case CollectionType::MATH_VECTOR3:
    return JS_NewString(ctx, "Vector3");
  case CollectionType::MATH_VECTOR4:
    return JS_NewString(ctx, "Vector4");
  case CollectionType::MATH_COLOR:
    return JS_NewString(ctx, "Color");
  case CollectionType::MATH_QUATERNION:
    return JS_NewString(ctx, "Quaternion");
  case CollectionType::MATH_BASIS:
    return JS_NewString(ctx, "Basis");
  case CollectionType::MATH_TRANSFORM3D:
    return JS_NewString(ctx, "Transform3D");
  case CollectionType::MATH_TRANSFORM2D:
    return JS_NewString(ctx, "Transform2D");
  case CollectionType::MATH_PLANE:
    return JS_NewString(ctx, "Plane");
  case CollectionType::MATH_AABB:
    return JS_NewString(ctx, "AABB");
  case CollectionType::MATH_RECT2:
    return JS_NewString(ctx, "Rect2");
  default:
    return JS_NewString(ctx, "unknown");
  }
}

} // namespace jsb

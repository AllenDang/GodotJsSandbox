#include "quickjs_context.h"
#include "js_runtime_manager.h"
#include "object_registry.h"
#include "sandbox_config.h"
#include "execution_limiter.h"
#include "godot_bindings.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// InputEvent includes
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>
#include <godot_cpp/classes/input_event_screen_touch.hpp>
#include <godot_cpp/classes/input_event_screen_drag.hpp>

using namespace godot;

namespace jsb {

// Console implementation (QuickJS callback signatures are fixed by the API)
static JSValue js_console_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    String output;
    for (int i = 0; i < argc; i++) {
        if (i > 0) output += " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            output += str;
            JS_FreeCString(ctx, str);
        }
    }
    UtilityFunctions::print("[JS] ", output);
    return JS_UNDEFINED;
}

static JSValue js_console_warn(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    String output;
    for (int i = 0; i < argc; i++) {
        if (i > 0) output += " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            output += str;
            JS_FreeCString(ctx, str);
        }
    }
    UtilityFunctions::print("[JS Warning] ", output);
    return JS_UNDEFINED;
}

static JSValue js_console_error(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    String output;
    for (int i = 0; i < argc; i++) {
        if (i > 0) output += " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            output += str;
            JS_FreeCString(ctx, str);
        }
    }
    UtilityFunctions::printerr("[JS Error] ", output);
    return JS_UNDEFINED;
}

QuickJSContext::QuickJSContext() {
}

QuickJSContext::~QuickJSContext() {
    shutdown();
}

bool QuickJSContext::initialize() {
    if (ctx_ != nullptr) {
        return true;  // Already initialized
    }

    // Try to use JSRuntimeManager for shared runtime
    JSRuntimeManager* manager = JSRuntimeManager::get_singleton();
    if (manager) {
        rt_ = manager->get_runtime();
        ctx_ = manager->create_context();
        owns_runtime_ = false;
    } else {
        // Fallback: create our own runtime (legacy mode)
        rt_ = JS_NewRuntime();
        if (!rt_) {
            UtilityFunctions::printerr("Failed to create QuickJS runtime");
            return false;
        }
        JS_SetMemoryLimit(rt_, 64 * 1024 * 1024);

        ctx_ = JS_NewContext(rt_);
        owns_runtime_ = true;
    }

    if (!ctx_) {
        UtilityFunctions::printerr("Failed to create QuickJS context");
        if (owns_runtime_ && rt_) {
            JS_FreeRuntime(rt_);
            rt_ = nullptr;
        }
        return false;
    }

    // Set up interrupt handler for timeout
    JS_SetInterruptHandler(rt_, interrupt_handler, this);

    // Set up module loader for ES6 imports
    setup_module_loader();

    setup_builtins();
    setup_godot_bindings();

    return true;
}

void QuickJSContext::shutdown() {
    // Free all script instances first (before freeing context)
    for (auto& pair : script_instances_) {
        if (pair.value.valid && ctx_) {
            JS_FreeValue(ctx_, pair.value.js_object);
        }
    }
    script_instances_.clear();

    bindings_.reset();

    if (ctx_) {
        // Use JSRuntimeManager to free context if available
        JSRuntimeManager* manager = JSRuntimeManager::get_singleton();
        if (manager && !owns_runtime_) {
            manager->free_context(ctx_);
        } else {
            JS_FreeContext(ctx_);
        }
        ctx_ = nullptr;
    }

    if (owns_runtime_ && rt_) {
        JS_FreeRuntime(rt_);
    }
    rt_ = nullptr;
    owns_runtime_ = false;
}

void QuickJSContext::setup_builtins() {
    JSValue global = JS_GetGlobalObject(ctx_);

    JSValue console = JS_NewObject(ctx_);
    JS_SetPropertyStr(ctx_, console, "log", JS_NewCFunction(ctx_, js_console_log, "log", 1));
    JS_SetPropertyStr(ctx_, console, "warn", JS_NewCFunction(ctx_, js_console_warn, "warn", 1));
    JS_SetPropertyStr(ctx_, console, "error", JS_NewCFunction(ctx_, js_console_error, "error", 1));
    JS_SetPropertyStr(ctx_, console, "info", JS_NewCFunction(ctx_, js_console_log, "info", 1));
    JS_SetPropertyStr(ctx_, global, "console", console);

    JS_FreeValue(ctx_, global);
}

void QuickJSContext::setup_godot_bindings() {
    bindings_ = std::make_unique<GodotBindings>(this);
    bindings_->initialize();
}

void QuickJSContext::setup_module_loader() {
    // Register module loader for ES6 import/export support
    JS_SetModuleLoaderFunc(rt_, module_normalize, module_loader, this);
}

// Module path normalizer - converts relative paths to absolute Godot paths
char* QuickJSContext::module_normalize(JSContext* ctx, const char* base_name,
                                        const char* module_name, void* opaque) {
    // QuickJSContext* self = static_cast<QuickJSContext*>(opaque);
    String base = base_name ? String::utf8(base_name) : "";
    String name = module_name ? String::utf8(module_name) : "";

    // If module_name is already an absolute Godot path, use it directly
    if (name.begins_with("res://") || name.begins_with("user://")) {
        // Security: block path traversal
        if (name.find("..") != -1) {
            JS_ThrowReferenceError(ctx, "Path traversal not allowed in import: %s", module_name);
            return nullptr;
        }

        // Ensure .js extension
        if (!name.ends_with(".js")) {
            name += ".js";
        }

        // Allocate and return the normalized path
        CharString utf8 = name.utf8();
        char* result = (char*)js_malloc(ctx, utf8.length() + 1);
        memcpy(result, utf8.get_data(), utf8.length() + 1);
        return result;
    }

    // Handle relative paths - resolve relative to base module
    String resolved;

    if (name.begins_with("./") || name.begins_with("../")) {
        // Get directory of base module
        String base_dir;
        if (base.begins_with("res://") || base.begins_with("user://")) {
            base_dir = base.get_base_dir();
        } else {
            // Default to user:// for script instances without a proper path
            base_dir = "user://";
        }

        // Resolve the relative path
        if (name.begins_with("./")) {
            resolved = base_dir.path_join(name.substr(2));
        } else {
            // Handle ../ by going up directories
            String rel_path = name;
            String current_dir = base_dir;

            while (rel_path.begins_with("../")) {
                current_dir = current_dir.get_base_dir();
                rel_path = rel_path.substr(3);
            }
            resolved = current_dir.path_join(rel_path);
        }
    } else {
        // Bare module name - treat as relative to user:// (sandboxed modules)
        resolved = String("user://") + name;
    }

    // Security: block path traversal in resolved path
    if (resolved.find("..") != -1) {
        JS_ThrowReferenceError(ctx, "Path traversal not allowed in import: %s", module_name);
        return nullptr;
    }

    // Security: must resolve to user:// or res://
    if (!resolved.begins_with("res://") && !resolved.begins_with("user://")) {
        JS_ThrowReferenceError(ctx, "Import must resolve to res:// or user:// path: %s", module_name);
        return nullptr;
    }

    // Ensure .js extension
    if (!resolved.ends_with(".js")) {
        resolved += ".js";
    }

    // Allocate and return the normalized path
    CharString utf8 = resolved.utf8();
    char* result = (char*)js_malloc(ctx, utf8.length() + 1);
    memcpy(result, utf8.get_data(), utf8.length() + 1);
    return result;
}

// Module loader - reads file content and compiles as ES6 module
JSModuleDef* QuickJSContext::module_loader(JSContext* ctx, const char* module_name, void* opaque) {
    QuickJSContext* self = static_cast<QuickJSContext*>(opaque);
    String path = String::utf8(module_name);

    // Security: verify path is allowed
    if (!path.begins_with("res://") && !path.begins_with("user://")) {
        JS_ThrowReferenceError(ctx, "Import path must be res:// or user://: %s", module_name);
        return nullptr;
    }

    if (path.find("..") != -1) {
        JS_ThrowReferenceError(ctx, "Path traversal not allowed: %s", module_name);
        return nullptr;
    }

    // Check sandbox config if available
    if (self->sandbox_config_ && !self->sandbox_config_->is_path_allowed(path)) {
        JS_ThrowReferenceError(ctx, "Import path blocked by sandbox config: %s", module_name);
        return nullptr;
    }

    // Read file content using Godot's FileAccess
    Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
    if (!file.is_valid()) {
        JS_ThrowReferenceError(ctx, "Could not load module: %s", module_name);
        return nullptr;
    }

    String source = file->get_as_text();
    file->close();

    // Compile as ES6 module
    CharString source_utf8 = source.utf8();
    JSValue func_val = JS_Eval(ctx, source_utf8.get_data(), source_utf8.length(),
                               module_name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(func_val)) {
        // Exception is already set by JS_Eval
        return nullptr;
    }

    // Get the module definition from the compiled value
    JSModuleDef* m = (JSModuleDef*)JS_VALUE_GET_PTR(func_val);
    JS_FreeValue(ctx, func_val);

    return m;
}

int QuickJSContext::interrupt_handler(JSRuntime* rt, void* opaque) {
    const QuickJSContext* self = static_cast<const QuickJSContext*>(opaque);
    if (self->deadline_ > 0) {
        int64_t now = Time::get_singleton()->get_ticks_msec();
        if (now >= self->deadline_) {
            return 1;
        }
    }
    return 0;
}

bool QuickJSContext::eval(const String &code, const String &filename,
                          Variant &result, String &error) {
    if (!is_valid()) {
        error = "Context not initialized";
        return false;
    }

    if (timeout_ms_ > 0) {
        deadline_ = Time::get_singleton()->get_ticks_msec() + timeout_ms_;
    } else {
        deadline_ = 0;
    }

    CharString code_utf8 = code.utf8();
    CharString filename_utf8 = filename.utf8();

    JSValue ret = JS_Eval(ctx_, code_utf8.get_data(), code_utf8.length(),
                          filename_utf8.get_data(), JS_EVAL_TYPE_GLOBAL);

    deadline_ = 0;

    if (JS_IsException(ret)) {
        error = get_exception_message();
        JS_FreeValue(ctx_, ret);
        return false;
    }

    result = js_to_variant(ret);
    JS_FreeValue(ctx_, ret);

    return true;
}

bool QuickJSContext::eval_module(const String &code, const String &filename,
                                  Variant &result, String &error) {
    if (!is_valid()) {
        error = "Context not initialized";
        return false;
    }

    if (timeout_ms_ > 0) {
        deadline_ = Time::get_singleton()->get_ticks_msec() + timeout_ms_;
    } else {
        deadline_ = 0;
    }

    CharString code_utf8 = code.utf8();
    CharString filename_utf8 = filename.utf8();

    // Compile the module
    JSValue func_val = JS_Eval(ctx_, code_utf8.get_data(), code_utf8.length(),
                                filename_utf8.get_data(),
                                JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(func_val)) {
        deadline_ = 0;
        error = get_exception_message();
        return false;
    }

    // Evaluate (instantiate and run) the module
    JSValue ret = JS_EvalFunction(ctx_, func_val);

    deadline_ = 0;

    if (JS_IsException(ret)) {
        error = get_exception_message();
        JS_FreeValue(ctx_, ret);
        return false;
    }

    // Module evaluation returns undefined on success, but imports are executed
    result = js_to_variant(ret);
    JS_FreeValue(ctx_, ret);

    return true;
}

bool QuickJSContext::eval_file(const String &path, Variant &result, String &error) {
    if (!path.begins_with("res://") && !path.begins_with("user://")) {
        error = "Only res:// and user:// paths are allowed";
        return false;
    }

    if (path.find("..") != -1) {
        error = "Path traversal not allowed";
        return false;
    }

    Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
    if (!file.is_valid()) {
        error = "Failed to open file: " + path;
        return false;
    }

    String code = file->get_as_text();
    return eval(code, path, result, error);
}

void QuickJSContext::set_global(const String &name, const Variant &value) {
    if (!is_valid()) return;

    JSValue global = JS_GetGlobalObject(ctx_);
    JSValue js_val = variant_to_js(value);
    JS_SetPropertyStr(ctx_, global, name.utf8().get_data(), js_val);
    JS_FreeValue(ctx_, global);
}

Variant QuickJSContext::get_global(const String &name) {
    if (!is_valid()) return Variant();

    JSValue global = JS_GetGlobalObject(ctx_);
    JSValue val = JS_GetPropertyStr(ctx_, global, name.utf8().get_data());
    JS_FreeValue(ctx_, global);

    Variant result = js_to_variant(val);
    JS_FreeValue(ctx_, val);
    return result;
}

void QuickJSContext::set_memory_limit(size_t bytes) {
    if (rt_) {
        JS_SetMemoryLimit(rt_, bytes);
    }
}

void QuickJSContext::set_timeout_ms(int64_t ms) {
    timeout_ms_ = ms;
}

String QuickJSContext::get_exception_message() {
    JSValue exception = JS_GetException(ctx_);
    String message;

    if (!JS_IsNull(exception) && !JS_IsUndefined(exception)) {
        const char* str = JS_ToCString(ctx_, exception);
        if (str) {
            message = str;
            JS_FreeCString(ctx_, str);
        }

        JSValue stack = JS_GetPropertyStr(ctx_, exception, "stack");
        if (!JS_IsUndefined(stack)) {
            const char* stack_str = JS_ToCString(ctx_, stack);
            if (stack_str) {
                message += "\nStack trace:\n";
                message += stack_str;
                JS_FreeCString(ctx_, stack_str);
            }
        }
        JS_FreeValue(ctx_, stack);
    }

    JS_FreeValue(ctx_, exception);
    return message;
}

// Helper to convert InputEvent to a JS object with relevant properties
JSValue QuickJSContext::input_event_to_js(InputEvent* event) {
    if (!event) return JS_NULL;

    JSValue obj = JS_NewObject(ctx_);

    // Common InputEvent properties
    JS_SetPropertyStr(ctx_, obj, "type", JS_NewString(ctx_, event->get_class().utf8().get_data()));
    JS_SetPropertyStr(ctx_, obj, "device", JS_NewInt32(ctx_, event->get_device()));

    // InputEventKey
    if (InputEventKey* key_event = Object::cast_to<InputEventKey>(event)) {
        JS_SetPropertyStr(ctx_, obj, "pressed", JS_NewBool(ctx_, key_event->is_pressed()));
        JS_SetPropertyStr(ctx_, obj, "echo", JS_NewBool(ctx_, key_event->is_echo()));
        JS_SetPropertyStr(ctx_, obj, "keycode", JS_NewInt32(ctx_, (int)key_event->get_keycode()));
        JS_SetPropertyStr(ctx_, obj, "physical_keycode", JS_NewInt32(ctx_, (int)key_event->get_physical_keycode()));
        JS_SetPropertyStr(ctx_, obj, "key_label", JS_NewInt32(ctx_, (int)key_event->get_key_label()));
        JS_SetPropertyStr(ctx_, obj, "unicode", JS_NewInt32(ctx_, key_event->get_unicode()));
        JS_SetPropertyStr(ctx_, obj, "ctrl_pressed", JS_NewBool(ctx_, key_event->is_ctrl_pressed()));
        JS_SetPropertyStr(ctx_, obj, "alt_pressed", JS_NewBool(ctx_, key_event->is_alt_pressed()));
        JS_SetPropertyStr(ctx_, obj, "shift_pressed", JS_NewBool(ctx_, key_event->is_shift_pressed()));
        JS_SetPropertyStr(ctx_, obj, "meta_pressed", JS_NewBool(ctx_, key_event->is_meta_pressed()));
    }
    // InputEventMouseButton
    else if (InputEventMouseButton* mb_event = Object::cast_to<InputEventMouseButton>(event)) {
        JS_SetPropertyStr(ctx_, obj, "pressed", JS_NewBool(ctx_, mb_event->is_pressed()));
        JS_SetPropertyStr(ctx_, obj, "double_click", JS_NewBool(ctx_, mb_event->is_double_click()));
        JS_SetPropertyStr(ctx_, obj, "button_index", JS_NewInt32(ctx_, (int)mb_event->get_button_index()));
        JS_SetPropertyStr(ctx_, obj, "button_mask", JS_NewInt32(ctx_, (int)mb_event->get_button_mask()));

        Vector2 pos = mb_event->get_position();
        JSValue pos_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, pos_obj, "x", JS_NewFloat64(ctx_, pos.x));
        JS_SetPropertyStr(ctx_, pos_obj, "y", JS_NewFloat64(ctx_, pos.y));
        JS_SetPropertyStr(ctx_, obj, "position", pos_obj);

        Vector2 global_pos = mb_event->get_global_position();
        JSValue global_pos_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, global_pos_obj, "x", JS_NewFloat64(ctx_, global_pos.x));
        JS_SetPropertyStr(ctx_, global_pos_obj, "y", JS_NewFloat64(ctx_, global_pos.y));
        JS_SetPropertyStr(ctx_, obj, "global_position", global_pos_obj);

        JS_SetPropertyStr(ctx_, obj, "ctrl_pressed", JS_NewBool(ctx_, mb_event->is_ctrl_pressed()));
        JS_SetPropertyStr(ctx_, obj, "alt_pressed", JS_NewBool(ctx_, mb_event->is_alt_pressed()));
        JS_SetPropertyStr(ctx_, obj, "shift_pressed", JS_NewBool(ctx_, mb_event->is_shift_pressed()));
        JS_SetPropertyStr(ctx_, obj, "meta_pressed", JS_NewBool(ctx_, mb_event->is_meta_pressed()));
    }
    // InputEventMouseMotion
    else if (InputEventMouseMotion* mm_event = Object::cast_to<InputEventMouseMotion>(event)) {
        JS_SetPropertyStr(ctx_, obj, "button_mask", JS_NewInt32(ctx_, (int)mm_event->get_button_mask()));

        Vector2 pos = mm_event->get_position();
        JSValue pos_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, pos_obj, "x", JS_NewFloat64(ctx_, pos.x));
        JS_SetPropertyStr(ctx_, pos_obj, "y", JS_NewFloat64(ctx_, pos.y));
        JS_SetPropertyStr(ctx_, obj, "position", pos_obj);

        Vector2 global_pos = mm_event->get_global_position();
        JSValue global_pos_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, global_pos_obj, "x", JS_NewFloat64(ctx_, global_pos.x));
        JS_SetPropertyStr(ctx_, global_pos_obj, "y", JS_NewFloat64(ctx_, global_pos.y));
        JS_SetPropertyStr(ctx_, obj, "global_position", global_pos_obj);

        Vector2 relative = mm_event->get_relative();
        JSValue rel_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, rel_obj, "x", JS_NewFloat64(ctx_, relative.x));
        JS_SetPropertyStr(ctx_, rel_obj, "y", JS_NewFloat64(ctx_, relative.y));
        JS_SetPropertyStr(ctx_, obj, "relative", rel_obj);

        Vector2 velocity = mm_event->get_velocity();
        JSValue vel_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, vel_obj, "x", JS_NewFloat64(ctx_, velocity.x));
        JS_SetPropertyStr(ctx_, vel_obj, "y", JS_NewFloat64(ctx_, velocity.y));
        JS_SetPropertyStr(ctx_, obj, "velocity", vel_obj);

        JS_SetPropertyStr(ctx_, obj, "ctrl_pressed", JS_NewBool(ctx_, mm_event->is_ctrl_pressed()));
        JS_SetPropertyStr(ctx_, obj, "alt_pressed", JS_NewBool(ctx_, mm_event->is_alt_pressed()));
        JS_SetPropertyStr(ctx_, obj, "shift_pressed", JS_NewBool(ctx_, mm_event->is_shift_pressed()));
        JS_SetPropertyStr(ctx_, obj, "meta_pressed", JS_NewBool(ctx_, mm_event->is_meta_pressed()));
    }
    // InputEventAction
    else if (InputEventAction* action_event = Object::cast_to<InputEventAction>(event)) {
        JS_SetPropertyStr(ctx_, obj, "pressed", JS_NewBool(ctx_, action_event->is_pressed()));
        JS_SetPropertyStr(ctx_, obj, "action", JS_NewString(ctx_, String(action_event->get_action()).utf8().get_data()));
        JS_SetPropertyStr(ctx_, obj, "strength", JS_NewFloat64(ctx_, action_event->get_strength()));
    }
    // InputEventJoypadButton
    else if (InputEventJoypadButton* joy_btn = Object::cast_to<InputEventJoypadButton>(event)) {
        JS_SetPropertyStr(ctx_, obj, "pressed", JS_NewBool(ctx_, joy_btn->is_pressed()));
        JS_SetPropertyStr(ctx_, obj, "button_index", JS_NewInt32(ctx_, (int)joy_btn->get_button_index()));
        JS_SetPropertyStr(ctx_, obj, "pressure", JS_NewFloat64(ctx_, joy_btn->get_pressure()));
    }
    // InputEventJoypadMotion
    else if (InputEventJoypadMotion* joy_motion = Object::cast_to<InputEventJoypadMotion>(event)) {
        JS_SetPropertyStr(ctx_, obj, "axis", JS_NewInt32(ctx_, (int)joy_motion->get_axis()));
        JS_SetPropertyStr(ctx_, obj, "axis_value", JS_NewFloat64(ctx_, joy_motion->get_axis_value()));
    }
    // InputEventScreenTouch
    else if (InputEventScreenTouch* touch = Object::cast_to<InputEventScreenTouch>(event)) {
        JS_SetPropertyStr(ctx_, obj, "pressed", JS_NewBool(ctx_, touch->is_pressed()));
        JS_SetPropertyStr(ctx_, obj, "index", JS_NewInt32(ctx_, touch->get_index()));

        Vector2 pos = touch->get_position();
        JSValue pos_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, pos_obj, "x", JS_NewFloat64(ctx_, pos.x));
        JS_SetPropertyStr(ctx_, pos_obj, "y", JS_NewFloat64(ctx_, pos.y));
        JS_SetPropertyStr(ctx_, obj, "position", pos_obj);
    }
    // InputEventScreenDrag
    else if (InputEventScreenDrag* drag = Object::cast_to<InputEventScreenDrag>(event)) {
        JS_SetPropertyStr(ctx_, obj, "index", JS_NewInt32(ctx_, drag->get_index()));

        Vector2 pos = drag->get_position();
        JSValue pos_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, pos_obj, "x", JS_NewFloat64(ctx_, pos.x));
        JS_SetPropertyStr(ctx_, pos_obj, "y", JS_NewFloat64(ctx_, pos.y));
        JS_SetPropertyStr(ctx_, obj, "position", pos_obj);

        Vector2 relative = drag->get_relative();
        JSValue rel_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, rel_obj, "x", JS_NewFloat64(ctx_, relative.x));
        JS_SetPropertyStr(ctx_, rel_obj, "y", JS_NewFloat64(ctx_, relative.y));
        JS_SetPropertyStr(ctx_, obj, "relative", rel_obj);

        Vector2 velocity = drag->get_velocity();
        JSValue vel_obj = JS_NewObject(ctx_);
        JS_SetPropertyStr(ctx_, vel_obj, "x", JS_NewFloat64(ctx_, velocity.x));
        JS_SetPropertyStr(ctx_, vel_obj, "y", JS_NewFloat64(ctx_, velocity.y));
        JS_SetPropertyStr(ctx_, obj, "velocity", vel_obj);
    }

    // Add common helper methods as properties
    // is_action - checks if event matches an action
    JS_SetPropertyStr(ctx_, obj, "is_pressed", JS_NewBool(ctx_, event->is_pressed()));
    JS_SetPropertyStr(ctx_, obj, "is_released", JS_NewBool(ctx_, event->is_released()));
    JS_SetPropertyStr(ctx_, obj, "is_echo", JS_NewBool(ctx_, event->is_echo()));

    return obj;
}

JSValue QuickJSContext::variant_to_js(const Variant &value) {
    switch (value.get_type()) {
        case Variant::NIL:
            return JS_NULL;

        case Variant::BOOL:
            return JS_NewBool(ctx_, (bool)value);

        case Variant::INT:
            return JS_NewInt64(ctx_, (int64_t)value);

        case Variant::FLOAT:
            return JS_NewFloat64(ctx_, (double)value);

        case Variant::STRING: {
            String str = value;
            return JS_NewString(ctx_, str.utf8().get_data());
        }

        case Variant::VECTOR2: {
            Vector2 v = value;
            JSValue obj = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, obj, "x", JS_NewFloat64(ctx_, v.x));
            JS_SetPropertyStr(ctx_, obj, "y", JS_NewFloat64(ctx_, v.y));
            return obj;
        }

        case Variant::VECTOR3: {
            Vector3 v = value;
            JSValue obj = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, obj, "x", JS_NewFloat64(ctx_, v.x));
            JS_SetPropertyStr(ctx_, obj, "y", JS_NewFloat64(ctx_, v.y));
            JS_SetPropertyStr(ctx_, obj, "z", JS_NewFloat64(ctx_, v.z));
            return obj;
        }

        case Variant::COLOR: {
            Color c = value;
            JSValue obj = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, obj, "r", JS_NewFloat64(ctx_, c.r));
            JS_SetPropertyStr(ctx_, obj, "g", JS_NewFloat64(ctx_, c.g));
            JS_SetPropertyStr(ctx_, obj, "b", JS_NewFloat64(ctx_, c.b));
            JS_SetPropertyStr(ctx_, obj, "a", JS_NewFloat64(ctx_, c.a));
            return obj;
        }

        case Variant::ARRAY: {
            Array arr = value;
            JSValue js_arr = JS_NewArray(ctx_);
            for (int i = 0; i < arr.size(); i++) {
                JS_SetPropertyUint32(ctx_, js_arr, i, variant_to_js(arr[i]));
            }
            return js_arr;
        }

        case Variant::DICTIONARY: {
            Dictionary dict = value;
            JSValue obj = JS_NewObject(ctx_);
            Array keys = dict.keys();
            for (int i = 0; i < keys.size(); i++) {
                String key = keys[i];
                JS_SetPropertyStr(ctx_, obj, key.utf8().get_data(), variant_to_js(dict[keys[i]]));
            }
            return obj;
        }

        case Variant::OBJECT: {
            Object* obj = value;
            if (!obj) return JS_NULL;

            if (object_registry_ && bindings_) {
                uint64_t handle = object_registry_->create_handle(obj);

                // Get the class name for proper proxy wrapping
                String class_name = obj->get_class();

                // Use __create_godot_object_from_handle to wrap with proxy
                // This ensures the object has proper method bindings (like connect)
                JSValue global = JS_GetGlobalObject(ctx_);

                // Check if __wrap_existing_godot_object helper exists
                JSValue wrap_fn = JS_GetPropertyStr(ctx_, global, "__wrap_existing_godot_object");
                if (JS_IsFunction(ctx_, wrap_fn)) {
                    JSValue args[2];
                    args[0] = JS_NewInt64(ctx_, handle);
                    args[1] = JS_NewString(ctx_, class_name.utf8().get_data());
                    JSValue result = JS_Call(ctx_, wrap_fn, JS_UNDEFINED, 2, args);
                    JS_FreeValue(ctx_, args[0]);
                    JS_FreeValue(ctx_, args[1]);
                    JS_FreeValue(ctx_, wrap_fn);
                    JS_FreeValue(ctx_, global);
                    return result;
                }
                JS_FreeValue(ctx_, wrap_fn);
                JS_FreeValue(ctx_, global);

                // Fallback: create raw object (won't have method bindings)
                JSValue js_obj = JS_NewObjectClass(ctx_, bindings_->get_godot_object_class_id());
                JS_SetOpaque(js_obj, reinterpret_cast<void*>(handle));
                return js_obj;
            }
            return JS_NULL;
        }

        default:
            return JS_NewString(ctx_, String(value).utf8().get_data());
    }
}

Variant QuickJSContext::js_to_variant(JSValue value) {
    if (JS_IsNull(value) || JS_IsUndefined(value)) {
        return Variant();
    }

    if (JS_IsBool(value)) {
        return JS_ToBool(ctx_, value) != 0;
    }

    if (JS_IsNumber(value)) {
        double d;
        JS_ToFloat64(ctx_, &d, value);

        if (d == (int64_t)d && d >= INT64_MIN && d <= INT64_MAX) {
            return (int64_t)d;
        }
        return d;
    }

    if (JS_IsString(value)) {
        const char* str = JS_ToCString(ctx_, value);
        String result = str ? str : "";
        JS_FreeCString(ctx_, str);
        return result;
    }

    if (JS_IsArray(value)) {
        Array arr;
        JSValue length_val = JS_GetPropertyStr(ctx_, value, "length");
        int64_t length = 0;
        JS_ToInt64(ctx_, &length, length_val);
        JS_FreeValue(ctx_, length_val);

        for (int64_t i = 0; i < length; i++) {
            JSValue elem = JS_GetPropertyUint32(ctx_, value, i);
            arr.push_back(js_to_variant(elem));
            JS_FreeValue(ctx_, elem);
        }
        return arr;
    }

    if (JS_IsObject(value)) {
        // Try to get as GodotObject first (via opaque pointer)
        if (bindings_ && object_registry_) {
            void* ptr = JS_GetOpaque(value, bindings_->get_godot_object_class_id());
            if (ptr) {
                uint64_t handle = reinterpret_cast<uint64_t>(ptr);
                Object* obj = object_registry_->get_object(handle);
                if (obj) {
                    return obj;
                }
            }
        }

        // Check if this is a Proxy-wrapped Godot object (has __handle property)
        if (object_registry_) {
            JSValue handle_val = JS_GetPropertyStr(ctx_, value, "__handle");
            if (JS_IsNumber(handle_val)) {
                int64_t handle;
                JS_ToInt64(ctx_, &handle, handle_val);
                JS_FreeValue(ctx_, handle_val);
                Object* obj = object_registry_->get_object(handle);
                if (obj) {
                    return obj;
                }
            } else {
                JS_FreeValue(ctx_, handle_val);
            }
        }

        // Check for Vector2 (has x, y but not z)
        JSValue x_val = JS_GetPropertyStr(ctx_, value, "x");
        JSValue y_val = JS_GetPropertyStr(ctx_, value, "y");
        JSValue z_val = JS_GetPropertyStr(ctx_, value, "z");

        if (JS_IsNumber(x_val) && JS_IsNumber(y_val)) {
            double x, y;
            JS_ToFloat64(ctx_, &x, x_val);
            JS_ToFloat64(ctx_, &y, y_val);
            JS_FreeValue(ctx_, x_val);
            JS_FreeValue(ctx_, y_val);

            if (JS_IsNumber(z_val)) {
                // Vector3
                double z;
                JS_ToFloat64(ctx_, &z, z_val);
                JS_FreeValue(ctx_, z_val);
                return Vector3(x, y, z);
            } else {
                JS_FreeValue(ctx_, z_val);
                return Vector2(x, y);
            }
        } else {
            JS_FreeValue(ctx_, x_val);
            JS_FreeValue(ctx_, y_val);
            JS_FreeValue(ctx_, z_val);
        }

        // Check for Color (has r, g, b, a)
        JSValue r_val = JS_GetPropertyStr(ctx_, value, "r");
        JSValue g_val = JS_GetPropertyStr(ctx_, value, "g");
        JSValue b_val = JS_GetPropertyStr(ctx_, value, "b");

        if (JS_IsNumber(r_val) && JS_IsNumber(g_val) && JS_IsNumber(b_val)) {
            double r, g, b, a = 1.0;
            JS_ToFloat64(ctx_, &r, r_val);
            JS_ToFloat64(ctx_, &g, g_val);
            JS_ToFloat64(ctx_, &b, b_val);
            JS_FreeValue(ctx_, r_val);
            JS_FreeValue(ctx_, g_val);
            JS_FreeValue(ctx_, b_val);

            JSValue a_val = JS_GetPropertyStr(ctx_, value, "a");
            if (JS_IsNumber(a_val)) {
                JS_ToFloat64(ctx_, &a, a_val);
            }
            JS_FreeValue(ctx_, a_val);

            return Color(r, g, b, a);
        } else {
            JS_FreeValue(ctx_, r_val);
            JS_FreeValue(ctx_, g_val);
            JS_FreeValue(ctx_, b_val);
        }

        Dictionary dict;
        JSPropertyEnum* props = nullptr;
        uint32_t prop_count = 0;

        if (JS_GetOwnPropertyNames(ctx_, &props, &prop_count, value, JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) == 0) {
            for (uint32_t i = 0; i < prop_count; i++) {
                const char* key = JS_AtomToCString(ctx_, props[i].atom);
                if (key) {
                    JSValue prop_val = JS_GetProperty(ctx_, value, props[i].atom);
                    dict[key] = js_to_variant(prop_val);
                    JS_FreeValue(ctx_, prop_val);
                    JS_FreeCString(ctx_, key);
                }
                JS_FreeAtom(ctx_, props[i].atom);
            }
            js_free(ctx_, props);
        }
        return dict;
    }

    return Variant();
}

// ============================================================================
// Script Instance Management
// ============================================================================

int64_t QuickJSContext::create_script_instance(const String &source, const String &filename,
                                                Object* owner, String &error) {
    if (!is_valid()) {
        error = "Context not initialized";
        return 0;
    }

    if (source.is_empty()) {
        error = "Empty script source";
        return 0;
    }

    // Parse method names from source code (same logic as JSScript::parse_script)
    Vector<String> method_names;
    int pos = 0;
    while ((pos = source.find("function ", pos)) >= 0) {
        pos += 9; // Skip "function "
        // Skip whitespace
        while (pos < source.length() && (source[pos] == ' ' || source[pos] == '\t')) {
            pos++;
        }
        // Read function name
        int name_start = pos;
        while (pos < source.length()) {
            char32_t c = source[pos];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_' || c == '$') {
                pos++;
            } else {
                break;
            }
        }
        int name_end = pos;
        // Skip whitespace before '('
        while (pos < source.length() && (source[pos] == ' ' || source[pos] == '\t')) {
            pos++;
        }
        // Verify it's followed by '('
        if (name_end > name_start && pos < source.length() && source[pos] == '(') {
            String method_name = source.substr(name_start, name_end - name_start);
            if (!method_name.is_empty()) {
                method_names.push_back(method_name);
            }
        }
    }

    // Build JSON array of method names
    String method_names_json = "[";
    for (int i = 0; i < method_names.size(); i++) {
        if (i > 0) method_names_json += ",";
        method_names_json += "\"" + method_names[i] + "\"";
    }
    method_names_json += "]";

    // Wrap the script source to create an object with methods
    // The script can define _ready(), _process(delta), etc. as functions
    // We create an object that captures these as methods and binds 'this' to the owner node
    String wrapped_source = String(R"(
(function() {
    var __instance = {};
    var __owner_proxy = null;
    var __method_names = )") + method_names_json + String(R"(;

    // Store owner reference for 'this' context
    __instance.__set_owner = function(ownerHandle, ownerClass) {
        if (typeof __wrap_existing_godot_object === 'function') {
            __owner_proxy = __wrap_existing_godot_object(ownerHandle, ownerClass);
        } else {
            __owner_proxy = { __handle: ownerHandle, __class: ownerClass };
        }
    };

    __instance.__get_owner = function() {
        return __owner_proxy;
    };

    // Helper to register a function to the instance
    var __registerMethod = function(name, fn) {
        if (typeof fn === 'function' && !__instance[name]) {
            __instance[name] = function() {
                return fn.apply(__owner_proxy, arguments);
            };
        }
    };

    // Execute user script and capture functions
    (function(__reg, __names) {
        // User code - functions will be hoisted within this scope
)") + source + String(R"(

        // Capture all parsed method names
        for (var i = 0; i < __names.length; i++) {
            try {
                var fn = eval(__names[i]);
                if (typeof fn === 'function') __reg(__names[i], fn);
            } catch(e) {}
        }
    })(__registerMethod, __method_names);

    return __instance;
})()
)");

    // Set deadline for timeout
    if (timeout_ms_ > 0) {
        deadline_ = Time::get_singleton()->get_ticks_msec() + timeout_ms_;
    } else {
        deadline_ = 0;
    }

    CharString code_utf8 = wrapped_source.utf8();
    CharString filename_utf8 = filename.utf8();

    JSValue result = JS_Eval(ctx_, code_utf8.get_data(), code_utf8.length(),
                              filename_utf8.get_data(), JS_EVAL_TYPE_GLOBAL);

    deadline_ = 0;

    if (JS_IsException(result)) {
        error = get_exception_message();
        JS_FreeValue(ctx_, result);
        return 0;
    }

    if (!JS_IsObject(result)) {
        error = "Script did not return an object";
        JS_FreeValue(ctx_, result);
        return 0;
    }

    // Store the instance
    int64_t instance_id = next_instance_id_++;

    ScriptInstanceData data;
    data.js_object = result;  // Takes ownership of the JSValue
    data.owner = owner;
    data.valid = true;

    script_instances_[instance_id] = data;

    // Set the owner handle on the instance if we have object registry
    if (owner && object_registry_) {
        uint64_t owner_handle = object_registry_->get_or_create_handle(owner);
        String owner_class = owner->get_class();
        JSValue set_owner_fn = JS_GetPropertyStr(ctx_, result, "__set_owner");
        if (JS_IsFunction(ctx_, set_owner_fn)) {
            JSValue args[2] = {
                JS_NewInt64(ctx_, owner_handle),
                JS_NewString(ctx_, owner_class.utf8().get_data())
            };
            JSValue call_result = JS_Call(ctx_, set_owner_fn, result, 2, args);
            JS_FreeValue(ctx_, call_result);
            JS_FreeValue(ctx_, args[0]);
            JS_FreeValue(ctx_, args[1]);
        }
        JS_FreeValue(ctx_, set_owner_fn);
    }

    return instance_id;
}

bool QuickJSContext::call_instance_method(int64_t instance_id, const StringName &method,
                                           const Variant** args, int argc,
                                           Variant &result, String &error) {
    if (!is_valid()) {
        error = "Context not initialized";
        return false;
    }

    if (!script_instances_.has(instance_id)) {
        error = "Invalid script instance ID";
        return false;
    }

    ScriptInstanceData& data = script_instances_[instance_id];
    if (!data.valid) {
        error = "Script instance has been invalidated";
        return false;
    }

    // Get the method from the instance
    String method_str = String(method);
    JSValue method_fn = JS_GetPropertyStr(ctx_, data.js_object, method_str.utf8().get_data());

    if (!JS_IsFunction(ctx_, method_fn)) {
        JS_FreeValue(ctx_, method_fn);
        // Not an error - method simply doesn't exist
        result = Variant();
        return true;
    }

    // Convert arguments to JS
    JSValue* js_args = nullptr;
    if (argc > 0) {
        js_args = (JSValue*)alloca(sizeof(JSValue) * argc);
        for (int i = 0; i < argc; i++) {
            js_args[i] = variant_to_js(*args[i]);
        }
    }

    // Set deadline for timeout
    if (timeout_ms_ > 0) {
        deadline_ = Time::get_singleton()->get_ticks_msec() + timeout_ms_;
    } else {
        deadline_ = 0;
    }

    // Call the method
    JSValue call_result = JS_Call(ctx_, method_fn, data.js_object, argc, js_args);

    deadline_ = 0;

    // Free JS arguments
    for (int i = 0; i < argc; i++) {
        JS_FreeValue(ctx_, js_args[i]);
    }
    JS_FreeValue(ctx_, method_fn);

    if (JS_IsException(call_result)) {
        error = get_exception_message();
        JS_FreeValue(ctx_, call_result);
        return false;
    }

    result = js_to_variant(call_result);
    JS_FreeValue(ctx_, call_result);

    return true;
}

bool QuickJSContext::call_instance_input_method(int64_t instance_id, const StringName &method,
                                                 InputEvent* event, String &error) {
    if (!is_valid()) {
        error = "Context not initialized";
        return false;
    }

    if (!script_instances_.has(instance_id)) {
        error = "Invalid script instance ID";
        return false;
    }

    ScriptInstanceData& data = script_instances_[instance_id];
    if (!data.valid) {
        error = "Script instance has been invalidated";
        return false;
    }

    // Get the method from the instance
    String method_str = String(method);
    JSValue method_fn = JS_GetPropertyStr(ctx_, data.js_object, method_str.utf8().get_data());

    if (!JS_IsFunction(ctx_, method_fn)) {
        JS_FreeValue(ctx_, method_fn);
        // Not an error - method simply doesn't exist
        return true;
    }

    // Convert InputEvent to JS using optimized converter
    JSValue js_event = input_event_to_js(event);

    // Set deadline for timeout
    if (timeout_ms_ > 0) {
        deadline_ = Time::get_singleton()->get_ticks_msec() + timeout_ms_;
    } else {
        deadline_ = 0;
    }

    // Call the method
    JSValue call_result = JS_Call(ctx_, method_fn, data.js_object, 1, &js_event);

    deadline_ = 0;

    // Free JS event and method
    JS_FreeValue(ctx_, js_event);
    JS_FreeValue(ctx_, method_fn);

    if (JS_IsException(call_result)) {
        error = get_exception_message();
        JS_FreeValue(ctx_, call_result);
        return false;
    }

    JS_FreeValue(ctx_, call_result);
    return true;
}

void QuickJSContext::release_script_instance(int64_t instance_id) {
    if (!script_instances_.has(instance_id)) {
        return;
    }

    ScriptInstanceData& data = script_instances_[instance_id];

    // Free the JS object
    if (ctx_ && data.valid) {
        JS_FreeValue(ctx_, data.js_object);
    }

    script_instances_.erase(instance_id);
}

bool QuickJSContext::has_script_instance(int64_t instance_id) const {
    return script_instances_.has(instance_id) && script_instances_[instance_id].valid;
}

} // namespace jsb

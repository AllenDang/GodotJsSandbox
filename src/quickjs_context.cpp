#include "quickjs_context.h"
#include "js_runtime_manager.h"
#include "object_registry.h"
#include "sandbox_config.h"
#include "execution_limiter.h"
#include "godot_bindings.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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

    // Wrap the script source to create an object with methods
    // The script can define _ready(), _process(delta), etc. as functions
    // We create an object that captures these as methods
    String wrapped_source = String(R"(
(function() {
    var __instance = {};
    var __owner = null;

    // Store owner reference for 'this' context
    __instance.__set_owner = function(ownerHandle) {
        __owner = ownerHandle;
    };

    // Execute the script in this scope
    (function() {
)") + source + String(R"(

        // Capture any defined lifecycle methods
        if (typeof _ready === 'function') __instance._ready = _ready;
        if (typeof _process === 'function') __instance._process = _process;
        if (typeof _physics_process === 'function') __instance._physics_process = _physics_process;
        if (typeof _input === 'function') __instance._input = _input;
        if (typeof _unhandled_input === 'function') __instance._unhandled_input = _unhandled_input;
        if (typeof _enter_tree === 'function') __instance._enter_tree = _enter_tree;
        if (typeof _exit_tree === 'function') __instance._exit_tree = _exit_tree;

        // Also capture any other properties/methods defined
        for (var key in this) {
            if (this.hasOwnProperty(key) && typeof this[key] === 'function' && !__instance[key]) {
                __instance[key] = this[key];
            }
        }
    })();

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
        JSValue set_owner_fn = JS_GetPropertyStr(ctx_, result, "__set_owner");
        if (JS_IsFunction(ctx_, set_owner_fn)) {
            JSValue args[1] = { JS_NewInt64(ctx_, owner_handle) };
            JSValue call_result = JS_Call(ctx_, set_owner_fn, result, 1, args);
            JS_FreeValue(ctx_, call_result);
            JS_FreeValue(ctx_, args[0]);
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

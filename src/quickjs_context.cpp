#include "quickjs_context.h"
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
    if (rt_ != nullptr) {
        return true;
    }

    rt_ = JS_NewRuntime();
    if (!rt_) {
        UtilityFunctions::printerr("Failed to create QuickJS runtime");
        return false;
    }

    JS_SetMemoryLimit(rt_, 64 * 1024 * 1024);

    ctx_ = JS_NewContext(rt_);
    if (!ctx_) {
        UtilityFunctions::printerr("Failed to create QuickJS context");
        JS_FreeRuntime(rt_);
        rt_ = nullptr;
        return false;
    }

    JS_SetInterruptHandler(rt_, interrupt_handler, this);

    setup_builtins();
    setup_godot_bindings();

    return true;
}

void QuickJSContext::shutdown() {
    bindings_.reset();

    if (ctx_) {
        JS_FreeContext(ctx_);
        ctx_ = nullptr;
    }
    if (rt_) {
        JS_FreeRuntime(rt_);
        rt_ = nullptr;
    }
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

    if (JS_IsArray(ctx_, value)) {
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
        // Try to get as GodotObject first
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

} // namespace jsb

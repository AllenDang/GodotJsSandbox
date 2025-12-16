#include "quickjs_context.h"
#include "js_runtime_manager.h"
#include "object_registry.h"
#include "array_registry.h"
#include "sandbox_config.h"
#include "execution_limiter.h"
#include "godot_bindings.h"
#include "../generated/generated_classes.gen.h"

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
    for (const auto& pair : script_instances_) {
        if (pair.value.valid && ctx_) {
            JS_FreeValue(ctx_, pair.value.js_object);
        }
    }
    script_instances_.clear();

    bindings_.reset();

    if (ctx_) {
        // Run garbage collection to trigger finalizers before freeing context
        // This ensures ArrayRegistry::release_handle() and ObjectRegistry::release_handle()
        // are called for all JS proxy objects, preventing memory leaks
        if (rt_) {
            JS_RunGC(rt_);
        }

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

    // Disable dangerous evaluation functions for sandbox security
    // eval() - allows arbitrary code execution
    JS_SetPropertyStr(ctx_, global, "eval", JS_UNDEFINED);

    // Function constructor - allows dynamic function creation (equivalent to eval)
    // We delete it from global scope to prevent: new Function('return 1')()
    JS_DeleteProperty(ctx_, global, JS_NewAtom(ctx_, "Function"), 0);

    // Setup console
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

    // Check timeout via deadline
    if (self->deadline_ > 0) {
        int64_t now = Time::get_singleton()->get_ticks_msec();
        if (now >= self->deadline_) {
            return 1;  // Interrupt: timeout exceeded
        }
    }

    // Check ExecutionLimiter timeout (more granular check)
    if (self->execution_limiter_ && self->execution_limiter_->is_timeout_exceeded()) {
        return 1;  // Interrupt: execution limiter timeout
    }

    // Check memory limit
    if (self->execution_limiter_) {
        // Update current memory usage from QuickJS runtime
        JSMemoryUsage mem;
        JS_ComputeMemoryUsage(rt, &mem);
        // Use a const_cast since set_current_memory_usage is non-const
        // This is safe because we're updating stats, not modifying core state
        const_cast<ExecutionLimiter*>(self->execution_limiter_)->set_current_memory_usage(mem.memory_used_size);

        if (self->execution_limiter_->is_memory_limit_exceeded()) {
            return 1;  // Interrupt: memory limit exceeded
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
    ExceptionInfo info = get_exception_info();
    String message = info.message;
    if (!info.stack.is_empty()) {
        message += "\nStack trace:\n" + info.stack;
    }
    return message;
}

QuickJSContext::ExceptionInfo QuickJSContext::get_exception_info() {
    ExceptionInfo info;
    JSValue exception = JS_GetException(ctx_);

    if (!JS_IsNull(exception) && !JS_IsUndefined(exception)) {
        const char* str = JS_ToCString(ctx_, exception);
        if (str) {
            info.message = str;
            JS_FreeCString(ctx_, str);
        }

        // Try to get lineNumber and columnNumber properties (QuickJS provides these)
        JSValue line_val = JS_GetPropertyStr(ctx_, exception, "lineNumber");
        if (JS_IsNumber(line_val)) {
            int64_t line;
            JS_ToInt64(ctx_, &line, line_val);
            info.line = (int)line;
        }
        JS_FreeValue(ctx_, line_val);

        JSValue col_val = JS_GetPropertyStr(ctx_, exception, "columnNumber");
        if (JS_IsNumber(col_val)) {
            int64_t col;
            JS_ToInt64(ctx_, &col, col_val);
            info.column = (int)col;
        }
        JS_FreeValue(ctx_, col_val);

        // Get fileName property
        JSValue file_val = JS_GetPropertyStr(ctx_, exception, "fileName");
        if (JS_IsString(file_val)) {
            const char* file_str = JS_ToCString(ctx_, file_val);
            if (file_str) {
                info.file = file_str;
                JS_FreeCString(ctx_, file_str);
            }
        }
        JS_FreeValue(ctx_, file_val);

        // Get stack trace
        JSValue stack = JS_GetPropertyStr(ctx_, exception, "stack");
        if (!JS_IsUndefined(stack)) {
            const char* stack_str = JS_ToCString(ctx_, stack);
            if (stack_str) {
                info.stack = stack_str;
                JS_FreeCString(ctx_, stack_str);

                // Always try to parse from first stack line if no line info yet
                // Format: "    at function (file:line:col)" or "    at file:line:col"
                // QuickJS often doesn't set lineNumber property, so we parse from stack
                if (!info.stack.is_empty()) {
                    int at_pos = info.stack.find(" at ");
                    if (at_pos >= 0) {
                        int paren_pos = info.stack.find("(", at_pos);
                        int colon_pos = -1;
                        if (paren_pos >= 0) {
                            colon_pos = info.stack.find(":", paren_pos);
                        } else {
                            colon_pos = info.stack.find(":", at_pos + 4);
                        }
                        if (colon_pos >= 0) {
                            // Parse "file:line:col" pattern
                            int start = (paren_pos >= 0) ? paren_pos + 1 : at_pos + 4;
                            int newline_pos = info.stack.find("\n", start);
                            if (newline_pos < 0) newline_pos = info.stack.length();
                            String location = info.stack.substr(start, newline_pos - start);
                            location = location.strip_edges();
                            if (location.ends_with(")")) {
                                location = location.substr(0, location.length() - 1);
                            }
                            // Handle various path formats:
                            // - "user://path/to/file.js:line:col" (has multiple colons from ://)
                            // - ":line:col" (anonymous/inline code, no filename)
                            // - "file.js:line:col" (simple filename)
                            // We need to find the last two colons for line:col
                            int last_colon = location.rfind(":");
                            int second_last_colon = -1;
                            if (last_colon > 0) {
                                second_last_colon = location.rfind(":", last_colon - 1);
                            }
                            if (second_last_colon >= 0 && last_colon > second_last_colon) {
                                // second_last_colon can be 0 for ":line:col" format
                                if (info.file.is_empty() && second_last_colon > 0) {
                                    info.file = location.substr(0, second_last_colon);
                                }
                                if (info.line == 0) {
                                    info.line = location.substr(second_last_colon + 1, last_colon - second_last_colon - 1).to_int();
                                }
                                if (info.column == 0) {
                                    info.column = location.substr(last_colon + 1).to_int();
                                }
                            }
                        }
                    }
                }
            }
        }
        JS_FreeValue(ctx_, stack);
    }

    JS_FreeValue(ctx_, exception);
    return info;
}

String QuickJSContext::get_source_context(int64_t instance_id, int error_line, int context_lines) {
    if (!script_instances_.has(instance_id)) {
        return "";
    }

    const ScriptInstanceData& data = script_instances_[instance_id];
    if (data.wrapped_source.is_empty()) {
        return "";
    }

    // Split source into lines
    PackedStringArray lines = data.wrapped_source.split("\n");
    int total_lines = lines.size();

    if (error_line < 1 || error_line > total_lines) {
        return "";
    }

    // Calculate range (1-indexed to 0-indexed)
    int start = MAX(0, error_line - 1 - context_lines);
    int end = MIN(total_lines - 1, error_line - 1 + context_lines);

    String result = "Source context:\n";
    for (int i = start; i <= end; i++) {
        String line_num = String::num_int64(i + 1);
        // Pad line numbers for alignment
        while (line_num.length() < 4) {
            line_num = " " + line_num;
        }

        // Mark the error line with an arrow
        String marker = (i == error_line - 1) ? " >> " : "    ";
        result += marker + line_num + " | " + lines[i] + "\n";
    }

    return result;
}

String QuickJSContext::get_wrapped_source(int64_t instance_id) const {
    if (!script_instances_.has(instance_id)) {
        return "";
    }
    return script_instances_[instance_id].wrapped_source;
}

// Helper function for input event get_class method
static JSValue js_input_event_get_class(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue type_val = JS_GetPropertyStr(ctx, this_val, "type");
    return type_val;
}

// Helper to convert InputEvent to a JS object with relevant properties
JSValue QuickJSContext::input_event_to_js(InputEvent* event) {
    if (!event) return JS_NULL;

    JSValue obj = JS_NewObject(ctx_);

    // Common InputEvent properties
    String class_name = event->get_class();
    JS_SetPropertyStr(ctx_, obj, "type", JS_NewString(ctx_, class_name.utf8().get_data()));
    JS_SetPropertyStr(ctx_, obj, "device", JS_NewInt32(ctx_, event->get_device()));

    // Add get_class() method for compatibility with standard Godot API
    JS_SetPropertyStr(ctx_, obj, "get_class", JS_NewCFunction(ctx_, js_input_event_get_class, "get_class", 0));

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

        // Math types - use direct value copy (lightweight, no handle/proxy overhead)
        // This is more efficient for small types like Vector2/Vector3 that are frequently
        // created (e.g., input events). Handle-based proxies would cause memory leaks
        // since there's no GC integration for math type handles.
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

        case Variant::VECTOR4: {
            Vector4 v = value;
            JSValue obj = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, obj, "x", JS_NewFloat64(ctx_, v.x));
            JS_SetPropertyStr(ctx_, obj, "y", JS_NewFloat64(ctx_, v.y));
            JS_SetPropertyStr(ctx_, obj, "z", JS_NewFloat64(ctx_, v.z));
            JS_SetPropertyStr(ctx_, obj, "w", JS_NewFloat64(ctx_, v.w));
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

        case Variant::QUATERNION: {
            Quaternion q = value;
            JSValue obj = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, obj, "x", JS_NewFloat64(ctx_, q.x));
            JS_SetPropertyStr(ctx_, obj, "y", JS_NewFloat64(ctx_, q.y));
            JS_SetPropertyStr(ctx_, obj, "z", JS_NewFloat64(ctx_, q.z));
            JS_SetPropertyStr(ctx_, obj, "w", JS_NewFloat64(ctx_, q.w));
            return obj;
        }

        case Variant::BASIS: {
            Basis b = value;
            JSValue obj = JS_NewObject(ctx_);
            // Basis has 3 row vectors (x, y, z)
            JSValue x = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, x, "x", JS_NewFloat64(ctx_, b.rows[0].x));
            JS_SetPropertyStr(ctx_, x, "y", JS_NewFloat64(ctx_, b.rows[0].y));
            JS_SetPropertyStr(ctx_, x, "z", JS_NewFloat64(ctx_, b.rows[0].z));
            JSValue y = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, y, "x", JS_NewFloat64(ctx_, b.rows[1].x));
            JS_SetPropertyStr(ctx_, y, "y", JS_NewFloat64(ctx_, b.rows[1].y));
            JS_SetPropertyStr(ctx_, y, "z", JS_NewFloat64(ctx_, b.rows[1].z));
            JSValue z = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, z, "x", JS_NewFloat64(ctx_, b.rows[2].x));
            JS_SetPropertyStr(ctx_, z, "y", JS_NewFloat64(ctx_, b.rows[2].y));
            JS_SetPropertyStr(ctx_, z, "z", JS_NewFloat64(ctx_, b.rows[2].z));
            JS_SetPropertyStr(ctx_, obj, "x", x);
            JS_SetPropertyStr(ctx_, obj, "y", y);
            JS_SetPropertyStr(ctx_, obj, "z", z);
            return obj;
        }

        case Variant::TRANSFORM3D: {
            Transform3D t = value;
            JSValue obj = JS_NewObject(ctx_);
            // Origin
            JSValue origin = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, origin, "x", JS_NewFloat64(ctx_, t.origin.x));
            JS_SetPropertyStr(ctx_, origin, "y", JS_NewFloat64(ctx_, t.origin.y));
            JS_SetPropertyStr(ctx_, origin, "z", JS_NewFloat64(ctx_, t.origin.z));
            JS_SetPropertyStr(ctx_, obj, "origin", origin);
            // Basis
            JSValue basis = JS_NewObject(ctx_);
            JSValue bx = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, bx, "x", JS_NewFloat64(ctx_, t.basis.rows[0].x));
            JS_SetPropertyStr(ctx_, bx, "y", JS_NewFloat64(ctx_, t.basis.rows[0].y));
            JS_SetPropertyStr(ctx_, bx, "z", JS_NewFloat64(ctx_, t.basis.rows[0].z));
            JSValue by = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, by, "x", JS_NewFloat64(ctx_, t.basis.rows[1].x));
            JS_SetPropertyStr(ctx_, by, "y", JS_NewFloat64(ctx_, t.basis.rows[1].y));
            JS_SetPropertyStr(ctx_, by, "z", JS_NewFloat64(ctx_, t.basis.rows[1].z));
            JSValue bz = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, bz, "x", JS_NewFloat64(ctx_, t.basis.rows[2].x));
            JS_SetPropertyStr(ctx_, bz, "y", JS_NewFloat64(ctx_, t.basis.rows[2].y));
            JS_SetPropertyStr(ctx_, bz, "z", JS_NewFloat64(ctx_, t.basis.rows[2].z));
            JS_SetPropertyStr(ctx_, basis, "x", bx);
            JS_SetPropertyStr(ctx_, basis, "y", by);
            JS_SetPropertyStr(ctx_, basis, "z", bz);
            JS_SetPropertyStr(ctx_, obj, "basis", basis);
            return obj;
        }

        case Variant::TRANSFORM2D: {
            Transform2D t = value;
            JSValue obj = JS_NewObject(ctx_);
            // Transform2D has x, y (column vectors) and origin
            JSValue x = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, x, "x", JS_NewFloat64(ctx_, t.columns[0].x));
            JS_SetPropertyStr(ctx_, x, "y", JS_NewFloat64(ctx_, t.columns[0].y));
            JSValue y = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, y, "x", JS_NewFloat64(ctx_, t.columns[1].x));
            JS_SetPropertyStr(ctx_, y, "y", JS_NewFloat64(ctx_, t.columns[1].y));
            JSValue origin = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, origin, "x", JS_NewFloat64(ctx_, t.columns[2].x));
            JS_SetPropertyStr(ctx_, origin, "y", JS_NewFloat64(ctx_, t.columns[2].y));
            JS_SetPropertyStr(ctx_, obj, "x", x);
            JS_SetPropertyStr(ctx_, obj, "y", y);
            JS_SetPropertyStr(ctx_, obj, "origin", origin);
            return obj;
        }

        case Variant::PLANE: {
            Plane p = value;
            JSValue obj = JS_NewObject(ctx_);
            JSValue normal = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, normal, "x", JS_NewFloat64(ctx_, p.normal.x));
            JS_SetPropertyStr(ctx_, normal, "y", JS_NewFloat64(ctx_, p.normal.y));
            JS_SetPropertyStr(ctx_, normal, "z", JS_NewFloat64(ctx_, p.normal.z));
            JS_SetPropertyStr(ctx_, obj, "normal", normal);
            JS_SetPropertyStr(ctx_, obj, "d", JS_NewFloat64(ctx_, p.d));
            return obj;
        }

        case Variant::AABB: {
            AABB a = value;
            JSValue obj = JS_NewObject(ctx_);
            JSValue position = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, position, "x", JS_NewFloat64(ctx_, a.position.x));
            JS_SetPropertyStr(ctx_, position, "y", JS_NewFloat64(ctx_, a.position.y));
            JS_SetPropertyStr(ctx_, position, "z", JS_NewFloat64(ctx_, a.position.z));
            JSValue size = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, size, "x", JS_NewFloat64(ctx_, a.size.x));
            JS_SetPropertyStr(ctx_, size, "y", JS_NewFloat64(ctx_, a.size.y));
            JS_SetPropertyStr(ctx_, size, "z", JS_NewFloat64(ctx_, a.size.z));
            JS_SetPropertyStr(ctx_, obj, "position", position);
            JS_SetPropertyStr(ctx_, obj, "size", size);
            return obj;
        }

        case Variant::RECT2: {
            Rect2 r = value;
            JSValue obj = JS_NewObject(ctx_);
            JSValue position = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, position, "x", JS_NewFloat64(ctx_, r.position.x));
            JS_SetPropertyStr(ctx_, position, "y", JS_NewFloat64(ctx_, r.position.y));
            JSValue size = JS_NewObject(ctx_);
            JS_SetPropertyStr(ctx_, size, "x", JS_NewFloat64(ctx_, r.size.x));
            JS_SetPropertyStr(ctx_, size, "y", JS_NewFloat64(ctx_, r.size.y));
            JS_SetPropertyStr(ctx_, obj, "position", position);
            JS_SetPropertyStr(ctx_, obj, "size", size);
            return obj;
        }

        case Variant::ARRAY: {
            // Use zero-copy proxy wrapper for Godot arrays
            if (array_registry_) {
                Array arr = value;
                uint64_t handle = array_registry_->create_array_handle(arr);

                // Call JS wrapper function
                JSValue global = JS_GetGlobalObject(ctx_);
                JSValue wrap_fn = JS_GetPropertyStr(ctx_, global, "__wrap_godot_array");
                if (JS_IsFunction(ctx_, wrap_fn)) {
                    JSValue args[1];
                    args[0] = JS_NewInt64(ctx_, handle);
                    JSValue result = JS_Call(ctx_, wrap_fn, JS_UNDEFINED, 1, args);
                    JS_FreeValue(ctx_, args[0]);
                    JS_FreeValue(ctx_, wrap_fn);
                    JS_FreeValue(ctx_, global);
                    return result;
                }
                JS_FreeValue(ctx_, wrap_fn);
                JS_FreeValue(ctx_, global);
            }

            // Fallback: copy array (old behavior)
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

                // Allocate GodotObjectData to store both handle and registry
                // This allows the finalizer to work correctly even with shared runtime
                GodotObjectData* data = static_cast<GodotObjectData*>(js_malloc(ctx_, sizeof(GodotObjectData)));
                data->handle = handle;
                data->registry = object_registry_;
                JS_SetOpaque(js_obj, data);

                return js_obj;
            }
            return JS_NULL;
        }

        case Variant::RID: {
            // RID is a resource identifier - wrap it in an object with is_valid() and get_id() methods
            // Store the RID Variant in array_registry for proper round-trip conversion
            godot::RID rid = value;
            JSValue obj = JS_NewObject(ctx_);

            // Store the RID in the registry for later retrieval
            if (array_registry_) {
                uint64_t handle = array_registry_->create_rid_handle(value);
                JS_SetPropertyStr(ctx_, obj, "__rid_handle", JS_NewInt64(ctx_, handle));
            }

            // Store the RID ID for is_valid() check
            JS_SetPropertyStr(ctx_, obj, "__rid_id", JS_NewInt64(ctx_, rid.get_id()));
            // Add is_valid() method
            JS_SetPropertyStr(ctx_, obj, "is_valid", JS_NewCFunction(ctx_, [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) -> JSValue {
                JSValue id_val = JS_GetPropertyStr(ctx, this_val, "__rid_id");
                int64_t id = 0;
                JS_ToInt64(ctx, &id, id_val);
                JS_FreeValue(ctx, id_val);
                return JS_NewBool(ctx, id != 0);
            }, "is_valid", 0));
            // Add get_id() method
            JS_SetPropertyStr(ctx_, obj, "get_id", JS_NewCFunction(ctx_, [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) -> JSValue {
                JSValue id_val = JS_GetPropertyStr(ctx, this_val, "__rid_id");
                int64_t id = 0;
                JS_ToInt64(ctx, &id, id_val);
                JS_FreeValue(ctx, id_val);
                return JS_NewInt64(ctx, id);
            }, "get_id", 0));
            return obj;
        }

        default:
            // Check if this is a packed array type and use generated conversion
            if (generated::is_packed_array_type(value.get_type())) {
                JSValue result = generated::wrap_packed_array_variant(ctx_, array_registry_, value);
                if (!JS_IsNull(result)) {
                    return result;
                }
            }
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
                // Opaque data is GodotObjectData struct containing handle and registry
                GodotObjectData* data = static_cast<GodotObjectData*>(ptr);
                Object* obj = object_registry_->get_object(data->handle);
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

        // Check if this is a packed array proxy (has __packed_handle property)
        // Uses generated conversion function to avoid hardcoded type list
        if (array_registry_) {
            JSValue packed_handle_val = JS_GetPropertyStr(ctx_, value, "__packed_handle");
            if (JS_IsNumber(packed_handle_val)) {
                int64_t handle;
                JS_ToInt64(ctx_, &handle, packed_handle_val);
                JS_FreeValue(ctx_, packed_handle_val);

                Variant result;
                if (generated::convert_packed_array_to_variant(array_registry_, handle, result)) {
                    return result;
                }
            } else {
                JS_FreeValue(ctx_, packed_handle_val);
            }
        }

        // Check if this is an RID object (has __rid_id property)
        // Note: We store the original RID Variant in array_registry for proper round-trip
        JSValue rid_handle_val = JS_GetPropertyStr(ctx_, value, "__rid_handle");
        if (JS_IsNumber(rid_handle_val)) {
            int64_t handle;
            JS_ToInt64(ctx_, &handle, rid_handle_val);
            JS_FreeValue(ctx_, rid_handle_val);
            if (array_registry_) {
                Variant rid_var = array_registry_->get_rid_variant(handle);
                if (rid_var.get_type() == Variant::RID) {
                    return rid_var;
                }
            }
        } else {
            JS_FreeValue(ctx_, rid_handle_val);
        }

        // Check if this is a math type proxy (has __math_handle property)
        // Uses zero-copy handle to retrieve original Godot math value
        if (array_registry_) {
            JSValue math_handle_val = JS_GetPropertyStr(ctx_, value, "__math_handle");
            if (JS_IsNumber(math_handle_val)) {
                int64_t handle;
                JS_ToInt64(ctx_, &handle, math_handle_val);
                JS_FreeValue(ctx_, math_handle_val);

                Variant math_var = array_registry_->get_math_variant(handle);
                if (math_var.get_type() != Variant::NIL) {
                    return math_var;
                }
            } else {
                JS_FreeValue(ctx_, math_handle_val);
            }
        }

        // Fallback: Check for Vector2 (has x, y but not z) - for backward compatibility
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

        // Check for Quaternion (has x, y, z, w)
        JSValue qx_val = JS_GetPropertyStr(ctx_, value, "x");
        JSValue qy_val = JS_GetPropertyStr(ctx_, value, "y");
        JSValue qz_val = JS_GetPropertyStr(ctx_, value, "z");
        JSValue qw_val = JS_GetPropertyStr(ctx_, value, "w");

        if (JS_IsNumber(qx_val) && JS_IsNumber(qy_val) && JS_IsNumber(qz_val) && JS_IsNumber(qw_val)) {
            double qx, qy, qz, qw;
            JS_ToFloat64(ctx_, &qx, qx_val);
            JS_ToFloat64(ctx_, &qy, qy_val);
            JS_ToFloat64(ctx_, &qz, qz_val);
            JS_ToFloat64(ctx_, &qw, qw_val);
            JS_FreeValue(ctx_, qx_val);
            JS_FreeValue(ctx_, qy_val);
            JS_FreeValue(ctx_, qz_val);
            JS_FreeValue(ctx_, qw_val);
            return Quaternion(qx, qy, qz, qw);
        } else {
            JS_FreeValue(ctx_, qx_val);
            JS_FreeValue(ctx_, qy_val);
            JS_FreeValue(ctx_, qz_val);
            JS_FreeValue(ctx_, qw_val);
        }

        // Check for Transform3D (has basis and origin)
        JSValue basis_val = JS_GetPropertyStr(ctx_, value, "basis");
        JSValue origin_val = JS_GetPropertyStr(ctx_, value, "origin");

        if (JS_IsObject(basis_val) && JS_IsObject(origin_val)) {
            Transform3D t;
            // Parse origin
            JSValue ox_val = JS_GetPropertyStr(ctx_, origin_val, "x");
            JSValue oy_val = JS_GetPropertyStr(ctx_, origin_val, "y");
            JSValue oz_val = JS_GetPropertyStr(ctx_, origin_val, "z");
            if (JS_IsNumber(ox_val) && JS_IsNumber(oy_val) && JS_IsNumber(oz_val)) {
                double ox, oy, oz;
                JS_ToFloat64(ctx_, &ox, ox_val);
                JS_ToFloat64(ctx_, &oy, oy_val);
                JS_ToFloat64(ctx_, &oz, oz_val);
                t.origin = Vector3(ox, oy, oz);
            }
            JS_FreeValue(ctx_, ox_val);
            JS_FreeValue(ctx_, oy_val);
            JS_FreeValue(ctx_, oz_val);

            // Parse basis (has x, y, z rows)
            JSValue bx_val = JS_GetPropertyStr(ctx_, basis_val, "x");
            JSValue by_val = JS_GetPropertyStr(ctx_, basis_val, "y");
            JSValue bz_val = JS_GetPropertyStr(ctx_, basis_val, "z");
            if (JS_IsObject(bx_val) && JS_IsObject(by_val) && JS_IsObject(bz_val)) {
                for (int row = 0; row < 3; row++) {
                    JSValue row_val = (row == 0) ? bx_val : (row == 1) ? by_val : bz_val;
                    JSValue rx = JS_GetPropertyStr(ctx_, row_val, "x");
                    JSValue ry = JS_GetPropertyStr(ctx_, row_val, "y");
                    JSValue rz = JS_GetPropertyStr(ctx_, row_val, "z");
                    if (JS_IsNumber(rx) && JS_IsNumber(ry) && JS_IsNumber(rz)) {
                        double x, y, z;
                        JS_ToFloat64(ctx_, &x, rx);
                        JS_ToFloat64(ctx_, &y, ry);
                        JS_ToFloat64(ctx_, &z, rz);
                        t.basis.rows[row] = Vector3(x, y, z);
                    }
                    JS_FreeValue(ctx_, rx);
                    JS_FreeValue(ctx_, ry);
                    JS_FreeValue(ctx_, rz);
                }
            }
            JS_FreeValue(ctx_, bx_val);
            JS_FreeValue(ctx_, by_val);
            JS_FreeValue(ctx_, bz_val);
            JS_FreeValue(ctx_, basis_val);
            JS_FreeValue(ctx_, origin_val);
            return t;
        } else {
            JS_FreeValue(ctx_, basis_val);
            JS_FreeValue(ctx_, origin_val);
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

// Helper function to detect ES6 module syntax in source code
static bool source_uses_es6_modules(const String& source) {
    // Look for import statements: import ... from or import '...'
    // Look for export statements: export function, export const, export default, export {

    // Simple regex-like detection (avoid full regex for performance)
    // Check for "import " at start of line or after semicolon/newline
    int pos = 0;
    while (pos < source.length()) {
        // Skip whitespace
        while (pos < source.length() && (source[pos] == ' ' || source[pos] == '\t' || source[pos] == '\n' || source[pos] == '\r')) {
            pos++;
        }
        if (pos >= source.length()) break;

        // Check for import keyword
        if (source.substr(pos, 7) == "import " || source.substr(pos, 7) == "import\t") {
            // Make sure it's not inside a string or comment
            // For simplicity, if we see "import " at start of a line-ish position, assume ES6
            return true;
        }

        // Check for export keyword
        if (source.substr(pos, 7) == "export " || source.substr(pos, 7) == "export\t") {
            return true;
        }

        // Move to next line or statement
        while (pos < source.length() && source[pos] != '\n' && source[pos] != ';') {
            pos++;
        }
        pos++;
    }

    return false;
}

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

    // Check if source uses ES6 module syntax (import/export)
    bool is_es6_module = source_uses_es6_modules(source);

    if (is_es6_module) {
        return create_module_script_instance(source, filename, owner, error);
    }

    // Script instance wrapper approach:
    // We support multiple patterns for defining script methods:
    //
    // Pattern 1: exports object (RECOMMENDED - works with all JS syntax)
    //   exports._ready = function() { console.log("ready"); };
    //   exports._process = (delta) => { this.position.x += delta; };
    //
    // Pattern 2: Class-based (RECOMMENDED for complex scripts)
    //   class PlayerController {
    //       _ready() { this.speed = 100; }
    //       _process(delta) { this.position.x += this.speed * delta; }
    //   }
    //   exports = new PlayerController();
    //
    // Pattern 3: Object literal
    //   exports = {
    //       _ready() { console.log("ready"); },
    //       _process(delta) { /* ... */ }
    //   };
    //
    // Pattern 4: Legacy function declarations (backward compatible)
    //   function _ready() { console.log("ready"); }
    //   function _process(delta) { /* ... */ }
    //   NOTE: This pattern has limitations - only predefined lifecycle methods
    //   are captured. For custom methods, use the exports pattern.
    //
    // The exports pattern is preferred because:
    // - Works with all JS syntax (arrow functions, classes, async, etc.)
    // - No parsing required - pure JS introspection
    // - Explicit intent - clear what methods are exposed
    // - Works on all platforms (iOS/Android/desktop)

    // For legacy pattern, we still check for common lifecycle method names
    // These are the Godot lifecycle methods plus common signal handlers
    static const char* lifecycle_methods[] = {
        "_ready", "_process", "_physics_process", "_input", "_unhandled_input",
        "_enter_tree", "_exit_tree", "_draw", "_gui_input", "_notification",
        "_init", "_to_string", "_get", "_set", "_get_property_list",
        nullptr
    };

    // Build legacy method capture code for known lifecycle methods
    String legacy_capture;
    for (int i = 0; lifecycle_methods[i] != nullptr; i++) {
        String method = lifecycle_methods[i];
        legacy_capture += "    try { if (typeof " + method + " === 'function') __legacy['" + method + "'] = " + method + "; } catch(e) {}\n";
    }

    String wrapped_source = String(R"(
(function() {
    var __instance = {};
    var __owner_proxy = null;
    var __legacy = {};  // Captures legacy function declarations
    var exports = {};   // User assigns methods here (recommended)
    var signals = null; // User can define custom signals: var signals = ['health_changed', 'game_over'];

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

    // Get custom signals defined by the script
    __instance.__get_signals = function() {
        return signals;
    };

    // Execute user script
)") + source + String(R"(

    // Try to capture legacy function declarations (predefined names only)
)") + legacy_capture + String(R"(

    // Helper to wrap a method with proper 'this' binding
    var __wrapMethod = function(name, fn, context) {
        __instance[name] = function() {
            return fn.apply(__owner_proxy, arguments);
        };
    };

    // First, capture legacy function declarations
    for (var key in __legacy) {
        if (typeof __legacy[key] === 'function') {
            __wrapMethod(key, __legacy[key], null);
        }
    }

    // Then, capture methods from exports (overrides legacy if same name)
    var __source = exports;
    if (__source !== null && typeof __source === 'object') {
        // Get own properties (for object literals and direct assignments)
        for (var key in __source) {
            if (typeof __source[key] === 'function') {
                __wrapMethod(key, __source[key], __source);
            }
        }

        // Get prototype methods (for class instances)
        var proto = Object.getPrototypeOf(__source);
        while (proto && proto !== Object.prototype) {
            var names = Object.getOwnPropertyNames(proto);
            for (var i = 0; i < names.length; i++) {
                var name = names[i];
                if (name !== 'constructor' && typeof proto[name] === 'function' && !__instance[name]) {
                    __wrapMethod(name, proto[name].bind(__source), __source);
                }
            }
            proto = Object.getPrototypeOf(proto);
        }
    }

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
    data.wrapped_source = wrapped_source;  // Store for error context
    data.file_path = filename;

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

    // Register custom signals declared in the script (var signals = ['signal_name', ...])
    if (owner) {
        JSValue get_signals_fn = JS_GetPropertyStr(ctx_, result, "__get_signals");
        if (JS_IsFunction(ctx_, get_signals_fn)) {
            JSValue signals_val = JS_Call(ctx_, get_signals_fn, result, 0, nullptr);
            if (JS_IsArray(signals_val)) {
                JSValue length_val = JS_GetPropertyStr(ctx_, signals_val, "length");
                int64_t length = 0;
                JS_ToInt64(ctx_, &length, length_val);
                JS_FreeValue(ctx_, length_val);

                for (int64_t i = 0; i < length; i++) {
                    JSValue sig = JS_GetPropertyUint32(ctx_, signals_val, i);
                    const char* sig_name = JS_ToCString(ctx_, sig);
                    if (sig_name) {
                        // Register the signal with Godot
                        owner->add_user_signal(String(sig_name));
                        JS_FreeCString(ctx_, sig_name);
                    }
                    JS_FreeValue(ctx_, sig);
                }
            }
            JS_FreeValue(ctx_, signals_val);
        }
        JS_FreeValue(ctx_, get_signals_fn);
    }

    return instance_id;
}

int64_t QuickJSContext::create_module_script_instance(const String &source, const String &filename,
                                                       Object* owner, String &error) {
    // ES6 module-based script instance
    // For scripts that use import/export syntax, we evaluate them as proper ES6 modules
    // and capture their exports to build the script instance

    // Strategy:
    // 1. Evaluate the user's script as an ES6 module to load it and its imports
    // 2. Create a wrapper that collects exported functions and provides 'this' binding
    //
    // The module must export lifecycle methods (e.g., export function _ready() { })
    // We store a global reference to capture exports from the module

    // Generate unique instance ID and storage key
    int64_t instance_id = next_instance_id_++;
    String instance_key = "__module_instance_" + String::num_int64(instance_id);

    // Create wrapper module that:
    // 1. Imports all exports from the user script
    // 2. Creates an instance object with bound methods
    // 3. Stores it in globalThis for retrieval

    // First, we need to make the user script available for import
    // We'll store it in a map and use the module loader to serve it
    // For now, since the module loader already handles file paths, we rely on the
    // script being at its actual file path

    String module_path = filename;
    if (!module_path.begins_with("res://") && !module_path.begins_with("user://")) {
        module_path = "user://" + filename;
    }

    // Create wrapper module code that imports from the user script and captures exports
    String wrapper_code = String(R"(
// Import all exports from user script
import * as __userModule from ')") + module_path + String(R"(';

// Create instance object to hold bound methods
var __instance = {};
var __owner_proxy = null;

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

// Get custom signals if defined
__instance.__get_signals = function() {
    return __userModule.signals || null;
};

// Bind all exported functions to use owner as 'this'
for (var key in __userModule) {
    if (typeof __userModule[key] === 'function') {
        (function(methodName, methodFn) {
            __instance[methodName] = function() {
                return methodFn.apply(__owner_proxy, arguments);
            };
        })(key, __userModule[key]);
    }
}

// Store instance globally for retrieval
globalThis.)") + instance_key + String(R"( = __instance;
)");

    // Set deadline for timeout
    if (timeout_ms_ > 0) {
        deadline_ = Time::get_singleton()->get_ticks_msec() + timeout_ms_;
    } else {
        deadline_ = 0;
    }

    // Evaluate wrapper as module
    CharString wrapper_utf8 = wrapper_code.utf8();
    String wrapper_filename = "user://__wrapper_" + String::num_int64(instance_id) + ".js";
    CharString wrapper_filename_utf8 = wrapper_filename.utf8();

    JSValue func_val = JS_Eval(ctx_, wrapper_utf8.get_data(), wrapper_utf8.length(),
                                wrapper_filename_utf8.get_data(),
                                JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(func_val)) {
        deadline_ = 0;
        error = "Failed to compile module wrapper: " + get_exception_message();
        return 0;
    }

    JSValue ret = JS_EvalFunction(ctx_, func_val);
    deadline_ = 0;

    if (JS_IsException(ret)) {
        error = "Failed to evaluate module: " + get_exception_message();
        JS_FreeValue(ctx_, ret);
        return 0;
    }
    JS_FreeValue(ctx_, ret);

    // Retrieve the instance from globalThis
    JSValue global = JS_GetGlobalObject(ctx_);
    JSValue result = JS_GetPropertyStr(ctx_, global, instance_key.utf8().get_data());

    if (!JS_IsObject(result)) {
        JS_FreeValue(ctx_, result);
        JS_FreeValue(ctx_, global);
        error = "Module wrapper did not create instance object";
        return 0;
    }

    // Clean up global reference (we hold our own reference now)
    JS_DeleteProperty(ctx_, global, JS_NewAtom(ctx_, instance_key.utf8().get_data()), 0);
    JS_FreeValue(ctx_, global);

    // Store the instance
    ScriptInstanceData data;
    data.js_object = result;  // Takes ownership
    data.owner = owner;
    data.valid = true;
    data.wrapped_source = source;  // Store original for error context
    data.file_path = filename;

    script_instances_[instance_id] = data;

    // Set the owner handle on the instance
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

    // Register custom signals
    if (owner) {
        JSValue get_signals_fn = JS_GetPropertyStr(ctx_, result, "__get_signals");
        if (JS_IsFunction(ctx_, get_signals_fn)) {
            JSValue signals_val = JS_Call(ctx_, get_signals_fn, result, 0, nullptr);
            if (JS_IsArray(signals_val)) {
                JSValue length_val = JS_GetPropertyStr(ctx_, signals_val, "length");
                int64_t length = 0;
                JS_ToInt64(ctx_, &length, length_val);
                JS_FreeValue(ctx_, length_val);

                for (int64_t i = 0; i < length; i++) {
                    JSValue sig = JS_GetPropertyUint32(ctx_, signals_val, i);
                    const char* sig_name = JS_ToCString(ctx_, sig);
                    if (sig_name) {
                        owner->add_user_signal(String(sig_name));
                        JS_FreeCString(ctx_, sig_name);
                    }
                    JS_FreeValue(ctx_, sig);
                }
            }
            JS_FreeValue(ctx_, signals_val);
        }
        JS_FreeValue(ctx_, get_signals_fn);
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
    Vector<JSValue> js_args;
    if (argc > 0) {
        js_args.resize(argc);
        for (int i = 0; i < argc; i++) {
            js_args.write[i] = variant_to_js(*args[i]);
        }
    }

    // Set deadline for timeout
    if (timeout_ms_ > 0) {
        deadline_ = Time::get_singleton()->get_ticks_msec() + timeout_ms_;
    } else {
        deadline_ = 0;
    }

    // Call the method
    JSValue call_result = JS_Call(ctx_, method_fn, data.js_object, argc, argc > 0 ? js_args.ptrw() : nullptr);

    deadline_ = 0;

    // Free JS arguments
    for (int i = 0; i < argc; i++) {
        JS_FreeValue(ctx_, js_args[i]);
    }
    JS_FreeValue(ctx_, method_fn);

    if (JS_IsException(call_result)) {
        ExceptionInfo info = get_exception_info();
        error = info.message;
        if (!info.stack.is_empty()) {
            error += "\nStack trace:\n" + info.stack;
        }
        // Add source context around error line
        if (info.line > 0) {
            String context = get_source_context(instance_id, info.line, 3);
            if (!context.is_empty()) {
                error += "\n" + context;
            }
        }
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
        ExceptionInfo info = get_exception_info();
        error = info.message;
        if (!info.stack.is_empty()) {
            error += "\nStack trace:\n" + info.stack;
        }
        // Add source context around error line
        if (info.line > 0) {
            String context = get_source_context(instance_id, info.line, 3);
            if (!context.is_empty()) {
                error += "\n" + context;
            }
        }
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

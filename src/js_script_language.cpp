#include "js_script_language.h"
#include "js_script.h"
#include "quickjs_context.h"
#include "object_registry.h"
#include "sandbox_config.h"
#include "execution_limiter.h"
#include "safe_wrapper.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

namespace jsb {

JSScriptLanguage* JSScriptLanguage::singleton_ = nullptr;

JSScriptLanguage::JSScriptLanguage() {
    singleton_ = this;
}

JSScriptLanguage::~JSScriptLanguage() {
    if (singleton_ == this) {
        singleton_ = nullptr;
    }
}

void JSScriptLanguage::_bind_methods() {
    // No additional methods to bind
}

String JSScriptLanguage::_get_name() const {
    return "JavaScript";
}

String JSScriptLanguage::_get_type() const {
    return "JSScript";
}

String JSScriptLanguage::_get_extension() const {
    return "js";
}

void JSScriptLanguage::_init() {
    initialize_runtime();
}

void JSScriptLanguage::_finish() {
    shutdown_runtime();
}

void JSScriptLanguage::_frame() {
    // Reset per-frame counters
    if (execution_limiter_) {
        execution_limiter_->reset_frame_counters();
    }
}

void JSScriptLanguage::initialize_runtime() {
    if (initialized_) {
        return;
    }

    object_registry_ = std::make_unique<ObjectRegistry>();
    sandbox_config_ = std::make_unique<SandboxConfig>();
    execution_limiter_ = std::make_unique<ExecutionLimiter>();
    safe_wrapper_ = std::make_unique<SafeWrapper>();

    safe_wrapper_->set_object_registry(object_registry_.get());
    safe_wrapper_->set_sandbox_config(sandbox_config_.get());
    safe_wrapper_->set_execution_limiter(execution_limiter_.get());

    context_ = std::make_unique<QuickJSContext>();
    context_->set_object_registry(object_registry_.get());
    context_->set_sandbox_config(sandbox_config_.get());
    context_->set_execution_limiter(execution_limiter_.get());
    context_->set_safe_wrapper(safe_wrapper_.get());

    if (!context_->initialize()) {
        UtilityFunctions::printerr("Failed to initialize JavaScript runtime");
        return;
    }

    initialized_ = true;
    UtilityFunctions::print("JavaScript runtime initialized");
}

void JSScriptLanguage::shutdown_runtime() {
    if (!initialized_) {
        return;
    }

    context_.reset();
    safe_wrapper_.reset();
    execution_limiter_.reset();
    sandbox_config_.reset();
    object_registry_.reset();

    initialized_ = false;
}

Object* JSScriptLanguage::_create_script() const {
    return memnew(JSScript);
}

bool JSScriptLanguage::_handles_global_class_type(const String &p_type) const {
    return p_type == "JSScript";
}

PackedStringArray JSScriptLanguage::_get_recognized_extensions() const {
    PackedStringArray extensions;
    extensions.push_back("js");
    return extensions;
}

PackedStringArray JSScriptLanguage::_get_reserved_words() const {
    PackedStringArray words;
    // JavaScript reserved words
    const char* reserved[] = {
        "break", "case", "catch", "continue", "debugger", "default", "delete",
        "do", "else", "finally", "for", "function", "if", "in", "instanceof",
        "new", "return", "switch", "this", "throw", "try", "typeof", "var",
        "void", "while", "with", "class", "const", "enum", "export", "extends",
        "import", "super", "implements", "interface", "let", "package", "private",
        "protected", "public", "static", "yield", "async", "await", nullptr
    };
    for (int i = 0; reserved[i] != nullptr; i++) {
        words.push_back(reserved[i]);
    }
    return words;
}

PackedStringArray JSScriptLanguage::_get_comment_delimiters() const {
    PackedStringArray delims;
    delims.push_back("//");
    delims.push_back("/* */");
    return delims;
}

PackedStringArray JSScriptLanguage::_get_string_delimiters() const {
    PackedStringArray delims;
    delims.push_back("\" \"");
    delims.push_back("' '");
    delims.push_back("` `");
    return delims;
}

PackedStringArray JSScriptLanguage::_get_doc_comment_delimiters() const {
    PackedStringArray delims;
    delims.push_back("/** */");
    return delims;
}

bool JSScriptLanguage::_is_control_flow_keyword(const String &p_keyword) const {
    return p_keyword == "if" || p_keyword == "else" || p_keyword == "for" ||
           p_keyword == "while" || p_keyword == "do" || p_keyword == "switch" ||
           p_keyword == "case" || p_keyword == "break" || p_keyword == "continue" ||
           p_keyword == "return" || p_keyword == "try" || p_keyword == "catch" ||
           p_keyword == "finally" || p_keyword == "throw";
}

bool JSScriptLanguage::_supports_builtin_mode() const {
    return false;
}

bool JSScriptLanguage::_can_inherit_from_file() const {
    return true;
}

bool JSScriptLanguage::_supports_documentation() const {
    return false;
}

int32_t JSScriptLanguage::_find_function(const String &p_function, const String &p_code) const {
    // Simple search for function definition
    String search = "function " + p_function;
    int pos = p_code.find(search);
    if (pos >= 0) {
        // Count lines up to this position
        int line = 1;
        for (int i = 0; i < pos; i++) {
            if (p_code[i] == '\n') {
                line++;
            }
        }
        return line;
    }
    return -1;
}

String JSScriptLanguage::_make_function(const String &p_class_name, const String &p_function_name, const PackedStringArray &p_function_args) const {
    String args;
    for (int i = 0; i < p_function_args.size(); i++) {
        if (i > 0) args += String(", ");
        args += p_function_args[i];
    }
    return p_function_name + String("(") + args + String(") {\n\t\n}\n");
}

String JSScriptLanguage::_debug_get_error() const {
    return "";
}

int32_t JSScriptLanguage::_debug_get_stack_level_count() const {
    return 0;
}

bool JSScriptLanguage::_has_named_classes() const {
    return true;
}

TypedArray<Dictionary> JSScriptLanguage::_debug_get_current_stack_info() {
    // Return empty stack info for now
    return TypedArray<Dictionary>();
}

void JSScriptLanguage::_thread_enter() {
    // Nothing to do for now - single-threaded
}

void JSScriptLanguage::_thread_exit() {
    // Nothing to do for now - single-threaded
}

Dictionary JSScriptLanguage::_get_global_class_name(const String &p_path) const {
    // Return empty dictionary - we don't define global classes from JS files yet
    return Dictionary();
}

void JSScriptLanguage::_reload_all_scripts() {
    // For now, do nothing - could be expanded to reload all loaded JS scripts
}

} // namespace jsb

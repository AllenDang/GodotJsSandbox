#ifndef GODOT_JS_RUNTIME_JS_SCRIPT_LANGUAGE_H
#define GODOT_JS_RUNTIME_JS_SCRIPT_LANGUAGE_H

#include <godot_cpp/classes/script_language_extension.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <memory>

namespace jsb {

class QuickJSContext;
class ObjectRegistry;
class SandboxConfig;
class ExecutionLimiter;
class SafeWrapper;
class SignalRegistry;
class DeletionTracker;

// JSScriptLanguage registers JavaScript as a scripting language in Godot
// This is a simplified version focused on runtime execution (no editor features)
class JSScriptLanguage : public godot::ScriptLanguageExtension {
    GDCLASS(JSScriptLanguage, godot::ScriptLanguageExtension);

public:
    JSScriptLanguage();
    ~JSScriptLanguage();

    // Get singleton instance
    static JSScriptLanguage* get_singleton() { return singleton_; }

    // ScriptLanguageExtension interface
    virtual godot::String _get_name() const override;
    virtual godot::String _get_type() const override;
    virtual godot::String _get_extension() const override;

    virtual void _init() override;
    virtual void _finish() override;
    virtual void _frame() override;

    // Script creation
    virtual godot::Object* _create_script() const override;
    virtual bool _handles_global_class_type(const godot::String &p_type) const override;

    // Extensions
    virtual godot::PackedStringArray _get_recognized_extensions() const override;
    virtual godot::PackedStringArray _get_reserved_words() const override;
    virtual godot::PackedStringArray _get_comment_delimiters() const override;
    virtual godot::PackedStringArray _get_string_delimiters() const override;
    virtual godot::PackedStringArray _get_doc_comment_delimiters() const override;

    // Validation (simplified - always valid for runtime)
    virtual bool _is_control_flow_keyword(const godot::String &p_keyword) const override;
    virtual bool _supports_builtin_mode() const override;
    virtual bool _can_inherit_from_file() const override;

    // Threading
    virtual bool _supports_documentation() const override;
    virtual int32_t _find_function(const godot::String &p_function, const godot::String &p_code) const override;
    virtual godot::String _make_function(const godot::String &p_class_name, const godot::String &p_function_name, const godot::PackedStringArray &p_function_args) const override;

    // Debugging (minimal)
    virtual godot::String _debug_get_error() const override;
    virtual int32_t _debug_get_stack_level_count() const override;
    virtual godot::TypedArray<godot::Dictionary> _debug_get_current_stack_info() override;

    // Threading
    virtual void _thread_enter() override;
    virtual void _thread_exit() override;

    // Global classes
    virtual godot::Dictionary _get_global_class_name(const godot::String &p_path) const override;

    // Script management
    virtual void _reload_all_scripts() override;

    // Global constants
    virtual void _add_global_constant(const godot::StringName &p_name, const godot::Variant &p_value) override;
    virtual void _add_named_global_constant(const godot::StringName &p_name, const godot::Variant &p_value) override;
    virtual void _remove_named_global_constant(const godot::StringName &p_name) override;

    // Access global constants (for sandboxes to query)
    const godot::HashMap<godot::StringName, godot::Variant>& get_global_constants() const { return global_constants_; }
    const godot::HashMap<godot::StringName, godot::Variant>& get_named_global_constants() const { return named_global_constants_; }

    // Templates (not needed for runtime)
    virtual bool _has_named_classes() const override;

    // Get shared components
    QuickJSContext* get_context() const { return context_.get(); }
    ObjectRegistry* get_object_registry() const { return object_registry_.get(); }
    SandboxConfig* get_sandbox_config() const { return sandbox_config_.get(); }
    SafeWrapper* get_safe_wrapper() const { return safe_wrapper_.get(); }

protected:
    static void _bind_methods();

private:
    static JSScriptLanguage* singleton_;

    std::unique_ptr<QuickJSContext> context_;
    std::unique_ptr<ObjectRegistry> object_registry_;
    std::unique_ptr<SandboxConfig> sandbox_config_;
    std::unique_ptr<ExecutionLimiter> execution_limiter_;
    std::unique_ptr<SafeWrapper> safe_wrapper_;
    std::unique_ptr<SignalRegistry> signal_registry_;
    std::unique_ptr<DeletionTracker> deletion_tracker_;

    // Global constants storage (for constants added before context is ready)
    godot::HashMap<godot::StringName, godot::Variant> global_constants_;
    godot::HashMap<godot::StringName, godot::Variant> named_global_constants_;

    bool initialized_ = false;

    void initialize_runtime();
    void shutdown_runtime();
    void apply_global_constants();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_SCRIPT_LANGUAGE_H

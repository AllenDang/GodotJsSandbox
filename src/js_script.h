#ifndef GODOT_JS_RUNTIME_JS_SCRIPT_H
#define GODOT_JS_RUNTIME_JS_SCRIPT_H

#include <godot_cpp/classes/script_extension.hpp>
#include <godot_cpp/classes/script_language.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace jsb {

class JSScriptInstance;
class JSSandbox;

// JSScript is the Script resource that represents a .js file
// It can be attached to nodes like GDScript
class JSScript : public godot::ScriptExtension {
    GDCLASS(JSScript, godot::ScriptExtension);

public:
    JSScript();
    ~JSScript();

    // ScriptExtension interface
    virtual bool _editor_can_reload_from_file() override { return true; }
    virtual bool _can_instantiate() const override;
    virtual godot::Ref<godot::Script> _get_base_script() const override;
    virtual godot::StringName _get_global_name() const override;
    virtual bool _inherits_script(const godot::Ref<godot::Script> &p_script) const override;

    virtual godot::StringName _get_instance_base_type() const override;
    virtual void* _instance_create(godot::Object *p_for_object) const override;
    virtual void* _placeholder_instance_create(godot::Object *p_for_object) const override;
    virtual bool _instance_has(godot::Object *p_object) const override;

    virtual bool _has_source_code() const override;
    virtual godot::String _get_source_code() const override;
    virtual void _set_source_code(const godot::String &p_code) override;
    virtual godot::Error _reload(bool p_keep_state) override;

    // Documentation (not supported)
    virtual godot::TypedArray<godot::Dictionary> _get_documentation() const override;
    virtual godot::String _get_class_icon_path() const override;

    // Methods
    virtual bool _has_method(const godot::StringName &p_method) const override;
    virtual bool _has_static_method(const godot::StringName &p_method) const override;

    virtual godot::Dictionary _get_method_info(const godot::StringName &p_method) const override;

    // Properties
    virtual bool _is_tool() const override { return false; }
    virtual bool _is_valid() const override { return is_valid_; }
    virtual bool _is_abstract() const override { return false; }
    virtual godot::ScriptLanguage* _get_language() const override;

    virtual bool _has_script_signal(const godot::StringName &p_signal) const override;
    virtual godot::TypedArray<godot::Dictionary> _get_script_signal_list() const override;

    virtual bool _has_property_default_value(const godot::StringName &p_property) const override;
    virtual godot::Variant _get_property_default_value(const godot::StringName &p_property) const override;

    virtual void _update_exports() override;
    virtual godot::TypedArray<godot::Dictionary> _get_script_method_list() const override;
    virtual godot::TypedArray<godot::Dictionary> _get_script_property_list() const override;

    virtual int32_t _get_member_line(const godot::StringName &p_member) const override;

    virtual godot::Dictionary _get_constants() const override;
    virtual godot::TypedArray<godot::StringName> _get_members() const override;

    virtual bool _is_placeholder_fallback_enabled() const override { return false; }

    virtual godot::Variant _get_rpc_config() const override;

    // Custom methods
    void set_path(const godot::String &p_path);
    godot::String get_path() const { return path_; }

    // Source code property (for GDScript access)
    void set_source_code(const godot::String &p_code);
    godot::String get_source_code() const;

    // Instance management
    void register_instance(godot::Object* p_object, JSScriptInstance* p_instance);
    void unregister_instance(godot::Object* p_object);

    // Sandbox association - allows JSScript to use a specific sandbox's context
    void set_sandbox(JSSandbox* p_sandbox);
    JSSandbox* get_sandbox() const { return sandbox_; }

protected:
    static void _bind_methods();

private:
    godot::String source_code_;
    godot::String path_;
    bool is_valid_ = false;

    // Associated sandbox (if any) - script will use sandbox's context
    JSSandbox* sandbox_ = nullptr;

    // Track instances
    mutable godot::HashMap<godot::Object*, JSScriptInstance*> instances_;

    // Parsed script data
    godot::Vector<godot::StringName> methods_;
    godot::Vector<godot::StringName> signals_;
    godot::HashMap<godot::StringName, godot::Variant> properties_;

    // Parse script - uses QuickJS at runtime for actual method detection
    // Only parses annotation comments (@signal, @export) here
    bool parse_script();
    void parse_annotations();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_SCRIPT_H

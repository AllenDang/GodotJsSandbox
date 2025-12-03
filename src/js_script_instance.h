#ifndef GODOT_JS_RUNTIME_JS_SCRIPT_INSTANCE_H
#define GODOT_JS_RUNTIME_JS_SCRIPT_INSTANCE_H

#include <gdextension_interface.h>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace jsb {

class JSScript;
class QuickJSContext;

// JSScriptInstance represents a script attached to a specific object
// Implements the full GDExtension ScriptInstance API (GDExtensionScriptInstanceInfo3)
class JSScriptInstance {
public:
    JSScriptInstance();
    ~JSScriptInstance();

    // Setup
    void set_owner(godot::Object* p_owner) { owner_ = p_owner; }
    void set_script(JSScript* p_script);

    godot::Object* get_owner() const { return owner_; }
    JSScript* get_script() const { return script_; }

    // Initialize the JS context for this instance
    bool initialize();

    // Property access (called from GDExtension callbacks)
    bool set(const godot::StringName &p_name, const godot::Variant &p_value);
    bool get(const godot::StringName &p_name, godot::Variant &r_ret) const;

    const GDExtensionPropertyInfo* get_property_list(uint32_t *r_count) const;
    void free_property_list(const GDExtensionPropertyInfo *p_list, uint32_t p_count) const;

    GDExtensionVariantType get_property_type(const godot::StringName &p_name, bool *r_is_valid) const;
    bool validate_property(GDExtensionPropertyInfo *p_property) const;

    bool property_can_revert(const godot::StringName &p_name) const;
    bool property_get_revert(const godot::StringName &p_name, godot::Variant &r_ret) const;

    void get_property_state(GDExtensionScriptInstancePropertyStateAdd p_add_func, void *p_userdata) const;

    // Method access
    const GDExtensionMethodInfo* get_method_list(uint32_t *r_count) const;
    void free_method_list(const GDExtensionMethodInfo *p_list, uint32_t p_count) const;

    bool has_method(const godot::StringName &p_method) const;
    int get_method_argument_count(const godot::StringName &p_method, bool *r_is_valid) const;

    // Method calling
    void call(const godot::StringName &p_method, const GDExtensionConstVariantPtr *p_args,
              GDExtensionInt p_argument_count, GDExtensionVariantPtr r_return, GDExtensionCallError *r_error);

    // Notification handling
    void notification(int32_t p_what, bool p_reversed);

    // String conversion
    void to_string(bool *r_is_valid, godot::String *r_out) const;

    // Reference counting (for RefCounted owners)
    void refcount_incremented();
    bool refcount_decremented();

    // Placeholder support
    bool is_placeholder() const { return false; }

    // Fallback property access (for placeholder)
    bool set_fallback(const godot::StringName &p_name, const godot::Variant &p_value);
    bool get_fallback(const godot::StringName &p_name, godot::Variant &r_ret) const;

    // Create the GDExtension script instance
    static GDExtensionScriptInstancePtr create_instance(JSScriptInstance* p_instance);

    // Get the static info struct
    static const GDExtensionScriptInstanceInfo3& get_script_instance_info();

private:
    godot::Object* owner_ = nullptr;
    JSScript* script_ = nullptr;
    bool initialized_ = false;

    // JS instance ID (from QuickJSContext)
    int64_t js_instance_id_ = 0;

    // Property storage
    mutable godot::HashMap<godot::StringName, godot::Variant> properties_;

    // Cached property/method lists for GDExtension
    mutable godot::Vector<GDExtensionPropertyInfo> cached_property_list_;
    mutable godot::Vector<GDExtensionMethodInfo> cached_method_list_;
    mutable godot::Vector<godot::StringName> cached_string_names_; // Keep StringNames alive
    mutable godot::Vector<godot::StringName> cached_class_names_;  // Keep class names alive
    mutable godot::Vector<godot::String> cached_hint_strings_;     // Keep hint strings alive

    QuickJSContext* get_context() const;
    bool call_js_method(const godot::StringName &p_method, const godot::Variant** p_args,
                        int p_argcount, godot::Variant &r_result, godot::String& r_error);

    // Static callback functions for GDExtensionScriptInstanceInfo3
    static GDExtensionBool _set(GDExtensionScriptInstanceDataPtr p_instance,
                                 GDExtensionConstStringNamePtr p_name,
                                 GDExtensionConstVariantPtr p_value);
    static GDExtensionBool _get(GDExtensionScriptInstanceDataPtr p_instance,
                                 GDExtensionConstStringNamePtr p_name,
                                 GDExtensionVariantPtr r_ret);
    static const GDExtensionPropertyInfo* _get_property_list(GDExtensionScriptInstanceDataPtr p_instance,
                                                              uint32_t *r_count);
    static void _free_property_list(GDExtensionScriptInstanceDataPtr p_instance,
                                     const GDExtensionPropertyInfo *p_list, uint32_t p_count);
    static GDExtensionBool _get_class_category(GDExtensionScriptInstanceDataPtr p_instance,
                                                GDExtensionPropertyInfo *p_class_category);
    static GDExtensionBool _property_can_revert(GDExtensionScriptInstanceDataPtr p_instance,
                                                 GDExtensionConstStringNamePtr p_name);
    static GDExtensionBool _property_get_revert(GDExtensionScriptInstanceDataPtr p_instance,
                                                 GDExtensionConstStringNamePtr p_name,
                                                 GDExtensionVariantPtr r_ret);
    static GDExtensionObjectPtr _get_owner(GDExtensionScriptInstanceDataPtr p_instance);
    static void _get_property_state(GDExtensionScriptInstanceDataPtr p_instance,
                                     GDExtensionScriptInstancePropertyStateAdd p_add_func,
                                     void *p_userdata);
    static const GDExtensionMethodInfo* _get_method_list(GDExtensionScriptInstanceDataPtr p_instance,
                                                          uint32_t *r_count);
    static void _free_method_list(GDExtensionScriptInstanceDataPtr p_instance,
                                   const GDExtensionMethodInfo *p_list, uint32_t p_count);
    static GDExtensionVariantType _get_property_type(GDExtensionScriptInstanceDataPtr p_instance,
                                                      GDExtensionConstStringNamePtr p_name,
                                                      GDExtensionBool *r_is_valid);
    static GDExtensionBool _validate_property(GDExtensionScriptInstanceDataPtr p_instance,
                                               GDExtensionPropertyInfo *p_property);
    static GDExtensionBool _has_method(GDExtensionScriptInstanceDataPtr p_instance,
                                        GDExtensionConstStringNamePtr p_name);
    static GDExtensionInt _get_method_argument_count(GDExtensionScriptInstanceDataPtr p_instance,
                                                      GDExtensionConstStringNamePtr p_name,
                                                      GDExtensionBool *r_is_valid);
    static void _call(GDExtensionScriptInstanceDataPtr p_self,
                      GDExtensionConstStringNamePtr p_method,
                      const GDExtensionConstVariantPtr *p_args,
                      GDExtensionInt p_argument_count,
                      GDExtensionVariantPtr r_return,
                      GDExtensionCallError *r_error);
    static void _notification(GDExtensionScriptInstanceDataPtr p_instance,
                               int32_t p_what, GDExtensionBool p_reversed);
    static void _to_string(GDExtensionScriptInstanceDataPtr p_instance,
                            GDExtensionBool *r_is_valid, GDExtensionStringPtr r_out);
    static void _refcount_incremented(GDExtensionScriptInstanceDataPtr p_instance);
    static GDExtensionBool _refcount_decremented(GDExtensionScriptInstanceDataPtr p_instance);
    static GDExtensionObjectPtr _get_script(GDExtensionScriptInstanceDataPtr p_instance);
    static GDExtensionBool _is_placeholder(GDExtensionScriptInstanceDataPtr p_instance);
    static GDExtensionBool _set_fallback(GDExtensionScriptInstanceDataPtr p_instance,
                                          GDExtensionConstStringNamePtr p_name,
                                          GDExtensionConstVariantPtr p_value);
    static GDExtensionBool _get_fallback(GDExtensionScriptInstanceDataPtr p_instance,
                                          GDExtensionConstStringNamePtr p_name,
                                          GDExtensionVariantPtr r_ret);
    static GDExtensionScriptLanguagePtr _get_language(GDExtensionScriptInstanceDataPtr p_instance);
    static void _free(GDExtensionScriptInstanceDataPtr p_instance);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_SCRIPT_INSTANCE_H

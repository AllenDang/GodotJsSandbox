#include "js_script_instance.h"
#include "js_script.h"
#include "js_script_language.h"
#include "js_sandbox.h"
#include "quickjs_context.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

// Static instance info - initialized once
static GDExtensionScriptInstanceInfo3 script_instance_info = {};
static bool script_instance_info_initialized = false;

JSScriptInstance::JSScriptInstance() {
}

JSScriptInstance::~JSScriptInstance() {
    // Release the JS instance
    if (js_instance_id_ != 0) {
        QuickJSContext* ctx = get_context();
        if (ctx) {
            ctx->release_script_instance(js_instance_id_);
        }
        js_instance_id_ = 0;
    }

    if (script_) {
        script_->unregister_instance(owner_);
    }
}

void JSScriptInstance::set_script(JSScript* p_script) {
    script_ = p_script;
}

bool JSScriptInstance::initialize() {
    if (initialized_) return true;
    if (!script_ || !owner_) return false;

    QuickJSContext* ctx = get_context();
    if (!ctx) return false;

    String source = script_->_get_source_code();
    if (source.is_empty()) return false;

    // Create persistent JS instance
    String error;
    js_instance_id_ = ctx->create_script_instance(source, script_->get_path(), owner_, error);

    if (js_instance_id_ == 0) {
        UtilityFunctions::printerr("Failed to create JS instance: ", error);
        // Route error to sandbox for AI feedback
        JSSandbox* sandbox = script_->get_sandbox();
        if (sandbox) {
            String file_path = script_->get_path();
            sandbox->add_error("SyntaxError", error, file_path);
        }
        return false;
    }

    initialized_ = true;
    return true;
}

QuickJSContext* JSScriptInstance::get_context() const {
    // If script is associated with a sandbox, use its context
    if (script_) {
        JSSandbox* sandbox = script_->get_sandbox();
        if (sandbox) {
            return sandbox->get_context();
        }
    }
    // Fallback to language context (for scripts without sandbox association)
    JSScriptLanguage* lang = JSScriptLanguage::get_singleton();
    return lang ? lang->get_context() : nullptr;
}

// ============================================================================
// Property Access
// ============================================================================

bool JSScriptInstance::set(const StringName &p_name, const Variant &p_value) {
    properties_[p_name] = p_value;
    return true;
}

bool JSScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
    if (properties_.has(p_name)) {
        r_ret = properties_[p_name];
        return true;
    }
    return false;
}

const GDExtensionPropertyInfo* JSScriptInstance::get_property_list(uint32_t *r_count) const {
    cached_property_list_.clear();
    cached_string_names_.clear();
    cached_class_names_.clear();
    cached_hint_strings_.clear();

    for (const KeyValue<StringName, Variant> &E : properties_) {
        cached_string_names_.push_back(E.key);
        cached_class_names_.push_back(StringName());  // Empty class name
        cached_hint_strings_.push_back(String());     // Empty hint string

        int idx = cached_string_names_.size() - 1;

        GDExtensionPropertyInfo info = {};
        info.type = (GDExtensionVariantType)E.value.get_type();
        info.name = cached_string_names_[idx]._native_ptr();
        info.class_name = cached_class_names_[idx]._native_ptr();
        info.hint = PROPERTY_HINT_NONE;
        info.hint_string = cached_hint_strings_[idx]._native_ptr();
        info.usage = PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_SCRIPT_VARIABLE;
        cached_property_list_.push_back(info);
    }

    *r_count = cached_property_list_.size();
    return cached_property_list_.size() > 0 ? cached_property_list_.ptr() : nullptr;
}

void JSScriptInstance::free_property_list(const GDExtensionPropertyInfo *p_list, uint32_t p_count) const {
    // Nothing to free - we use cached vectors
}

GDExtensionVariantType JSScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
    if (properties_.has(p_name)) {
        *r_is_valid = true;
        return (GDExtensionVariantType)properties_[p_name].get_type();
    }
    *r_is_valid = false;
    return GDEXTENSION_VARIANT_TYPE_NIL;
}

bool JSScriptInstance::validate_property(GDExtensionPropertyInfo *p_property) const {
    return true;
}

bool JSScriptInstance::property_can_revert(const StringName &p_name) const {
    return false;
}

bool JSScriptInstance::property_get_revert(const StringName &p_name, Variant &r_ret) const {
    return false;
}

void JSScriptInstance::get_property_state(GDExtensionScriptInstancePropertyStateAdd p_add_func, void *p_userdata) const {
    for (const KeyValue<StringName, Variant> &E : properties_) {
        p_add_func(E.key._native_ptr(), E.value._native_ptr(), p_userdata);
    }
}

// ============================================================================
// Method Access
// ============================================================================

const GDExtensionMethodInfo* JSScriptInstance::get_method_list(uint32_t *r_count) const {
    *r_count = 0;
    return nullptr;
}

void JSScriptInstance::free_method_list(const GDExtensionMethodInfo *p_list, uint32_t p_count) const {
}

bool JSScriptInstance::has_method(const StringName &p_method) const {
    if (script_) {
        return script_->_has_method(p_method);
    }
    return false;
}

int JSScriptInstance::get_method_argument_count(const StringName &p_method, bool *r_is_valid) const {
    *r_is_valid = has_method(p_method);
    return 0; // We don't track argument counts
}

// ============================================================================
// Method Calling
// ============================================================================

bool JSScriptInstance::call_js_method(const StringName &p_method, const Variant** p_args,
                                       int p_argcount, Variant &r_result, String& r_error) {
    if (!initialized_ && !initialize()) {
        r_error = "Failed to initialize script instance";
        return false;
    }

    QuickJSContext* ctx = get_context();
    if (!ctx) {
        r_error = "No JavaScript context available";
        return false;
    }

    if (js_instance_id_ == 0) {
        r_error = "No JS instance created";
        return false;
    }

    // Call the method on the persistent JS instance
    return ctx->call_instance_method(js_instance_id_, p_method, p_args, p_argcount, r_result, r_error);
}

// Public method for calling JS script methods from other JS code
bool JSScriptInstance::call_method(const StringName &p_method, const Variant** p_args,
                                    int p_argcount, Variant &r_result, String& r_error) {
    return call_js_method(p_method, p_args, p_argcount, r_result, r_error);
}

void JSScriptInstance::call(const StringName &p_method, const GDExtensionConstVariantPtr *p_args,
                            GDExtensionInt p_argument_count, GDExtensionVariantPtr r_return,
                            GDExtensionCallError *r_error) {
    Variant* ret = (Variant*)r_return;
    *ret = Variant();

    if (!has_method(p_method)) {
        r_error->error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
        return;
    }

    // Check if this is an input method - use optimized InputEvent conversion
    static const StringName input_method("_input");
    static const StringName unhandled_input_method("_unhandled_input");
    static const StringName unhandled_key_input_method("_unhandled_key_input");
    static const StringName shortcut_input_method("_shortcut_input");

    if ((p_method == input_method || p_method == unhandled_input_method ||
         p_method == unhandled_key_input_method || p_method == shortcut_input_method) &&
        p_argument_count == 1) {
        // Extract InputEvent from the Variant
        const Variant* event_var = (const Variant*)p_args[0];
        if (event_var->get_type() == Variant::OBJECT) {
            Object* obj = *event_var;
            InputEvent* event = Object::cast_to<InputEvent>(obj);
            if (event) {
                if (call_input_method(p_method, event)) {
                    r_error->error = GDEXTENSION_CALL_OK;
                } else {
                    r_error->error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
                }
                return;
            }
        }
    }

    // Standard method call path
    Vector<const Variant*> args;
    for (int i = 0; i < p_argument_count; i++) {
        args.push_back((const Variant*)p_args[i]);
    }

    String error;
    Variant result;
    const Variant** args_ptr = const_cast<const Variant**>(args.ptr());
    if (call_js_method(p_method, args_ptr, p_argument_count, result, error)) {
        *ret = result;
        r_error->error = GDEXTENSION_CALL_OK;
    } else {
        UtilityFunctions::printerr("JSScript call error: ", error);
        // Route error to sandbox for AI feedback
        if (script_) {
            JSSandbox* sandbox = script_->get_sandbox();
            if (sandbox) {
                String file_path = script_->get_path();
                sandbox->add_error("RuntimeError", error, file_path);
            }
        }
        r_error->error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
    }
}

// ============================================================================
// Input Method Helper
// ============================================================================

bool JSScriptInstance::call_input_method(const StringName &p_method, InputEvent* event) {
    if (!initialized_ && !initialize()) {
        return false;
    }

    QuickJSContext* ctx = get_context();
    if (!ctx || js_instance_id_ == 0) {
        return false;
    }

    String error;
    bool success = ctx->call_instance_input_method(js_instance_id_, p_method, event, error);
    if (!success && !error.is_empty()) {
        UtilityFunctions::printerr("JSScript input error: ", error);
        // Route error to sandbox for AI feedback
        if (script_) {
            JSSandbox* sandbox = script_->get_sandbox();
            if (sandbox) {
                String file_path = script_->get_path();
                sandbox->add_error("RuntimeError", error, file_path);
            }
        }
    }
    return success;
}

// ============================================================================
// Notification
// ============================================================================

void JSScriptInstance::notification(int32_t p_what, bool p_reversed) {
    // Note: Godot calls lifecycle methods (_ready, _enter_tree, etc.) directly via call_func
    // We do NOT need to handle them here in notification() - that would cause double calls.
    // This function is for handling notifications that don't have corresponding methods,
    // or for custom notification handling if needed in the future.

    // The following notifications are handled by Godot calling the method directly:
    // - NOTIFICATION_READY (13) -> _ready()
    // - NOTIFICATION_PROCESS (17) -> _process(delta)
    // - NOTIFICATION_PHYSICS_PROCESS (18) -> _physics_process(delta)
    // - NOTIFICATION_ENTER_TREE (10) -> _enter_tree()
    // - NOTIFICATION_EXIT_TREE (11) -> _exit_tree()
    // etc.
}

void JSScriptInstance::to_string(bool *r_is_valid, String *r_out) const {
    *r_is_valid = true;
    *r_out = String("[JSScriptInstance]");
}

void JSScriptInstance::refcount_incremented() {}
bool JSScriptInstance::refcount_decremented() { return true; }

bool JSScriptInstance::set_fallback(const StringName &p_name, const Variant &p_value) {
    return false;
}

bool JSScriptInstance::get_fallback(const StringName &p_name, Variant &r_ret) const {
    return false;
}

// ============================================================================
// Static GDExtension Callbacks
// ============================================================================

GDExtensionBool JSScriptInstance::_set(GDExtensionScriptInstanceDataPtr p_instance,
                                        GDExtensionConstStringNamePtr p_name,
                                        GDExtensionConstVariantPtr p_value) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    return self->set(*(const StringName*)p_name, *(const Variant*)p_value);
}

GDExtensionBool JSScriptInstance::_get(GDExtensionScriptInstanceDataPtr p_instance,
                                        GDExtensionConstStringNamePtr p_name,
                                        GDExtensionVariantPtr r_ret) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->get(*reinterpret_cast<const StringName*>(p_name), *reinterpret_cast<Variant*>(r_ret));
}

const GDExtensionPropertyInfo* JSScriptInstance::_get_property_list(GDExtensionScriptInstanceDataPtr p_instance,
                                                                     uint32_t *r_count) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->get_property_list(r_count);
}

void JSScriptInstance::_free_property_list(GDExtensionScriptInstanceDataPtr p_instance,
                                            const GDExtensionPropertyInfo *p_list, uint32_t p_count) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    self->free_property_list(p_list, p_count);
}

GDExtensionBool JSScriptInstance::_get_class_category(GDExtensionScriptInstanceDataPtr p_instance,
                                                       GDExtensionPropertyInfo *p_class_category) {
    return false;
}

GDExtensionBool JSScriptInstance::_property_can_revert(GDExtensionScriptInstanceDataPtr p_instance,
                                                        GDExtensionConstStringNamePtr p_name) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->property_can_revert(*reinterpret_cast<const StringName*>(p_name));
}

GDExtensionBool JSScriptInstance::_property_get_revert(GDExtensionScriptInstanceDataPtr p_instance,
                                                        GDExtensionConstStringNamePtr p_name,
                                                        GDExtensionVariantPtr r_ret) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->property_get_revert(*reinterpret_cast<const StringName*>(p_name), *reinterpret_cast<Variant*>(r_ret));
}

GDExtensionObjectPtr JSScriptInstance::_get_owner(GDExtensionScriptInstanceDataPtr p_instance) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    return self->owner_ ? self->owner_->_owner : nullptr;
}

void JSScriptInstance::_get_property_state(GDExtensionScriptInstanceDataPtr p_instance,
                                            GDExtensionScriptInstancePropertyStateAdd p_add_func,
                                            void *p_userdata) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    self->get_property_state(p_add_func, p_userdata);
}

const GDExtensionMethodInfo* JSScriptInstance::_get_method_list(GDExtensionScriptInstanceDataPtr p_instance,
                                                                 uint32_t *r_count) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->get_method_list(r_count);
}

void JSScriptInstance::_free_method_list(GDExtensionScriptInstanceDataPtr p_instance,
                                          const GDExtensionMethodInfo *p_list, uint32_t p_count) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    self->free_method_list(p_list, p_count);
}

GDExtensionVariantType JSScriptInstance::_get_property_type(GDExtensionScriptInstanceDataPtr p_instance,
                                                             GDExtensionConstStringNamePtr p_name,
                                                             GDExtensionBool *r_is_valid) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    bool is_valid;
    auto result = self->get_property_type(*reinterpret_cast<const StringName*>(p_name), &is_valid);
    *r_is_valid = is_valid;
    return result;
}

GDExtensionBool JSScriptInstance::_validate_property(GDExtensionScriptInstanceDataPtr p_instance,
                                                      GDExtensionPropertyInfo *p_property) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->validate_property(p_property);
}

GDExtensionBool JSScriptInstance::_has_method(GDExtensionScriptInstanceDataPtr p_instance,
                                               GDExtensionConstStringNamePtr p_name) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->has_method(*reinterpret_cast<const StringName*>(p_name));
}

GDExtensionInt JSScriptInstance::_get_method_argument_count(GDExtensionScriptInstanceDataPtr p_instance,
                                                             GDExtensionConstStringNamePtr p_name,
                                                             GDExtensionBool *r_is_valid) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    bool is_valid;
    int result = self->get_method_argument_count(*reinterpret_cast<const StringName*>(p_name), &is_valid);
    *r_is_valid = is_valid;
    return result;
}

void JSScriptInstance::_call(GDExtensionScriptInstanceDataPtr p_self,
                              GDExtensionConstStringNamePtr p_method,
                              const GDExtensionConstVariantPtr *p_args,
                              GDExtensionInt p_argument_count,
                              GDExtensionVariantPtr r_return,
                              GDExtensionCallError *r_error) {
    JSScriptInstance* self = (JSScriptInstance*)p_self;
    self->call(*(const StringName*)p_method, p_args, p_argument_count, r_return, r_error);
}

void JSScriptInstance::_notification(GDExtensionScriptInstanceDataPtr p_instance,
                                      int32_t p_what, GDExtensionBool p_reversed) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    self->notification(p_what, p_reversed);
}

void JSScriptInstance::_to_string(GDExtensionScriptInstanceDataPtr p_instance,
                                   GDExtensionBool *r_is_valid, GDExtensionStringPtr r_out) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    bool is_valid;
    String out;
    self->to_string(&is_valid, &out);
    *r_is_valid = is_valid;
    memnew_placement(r_out, String(out));
}

void JSScriptInstance::_refcount_incremented(GDExtensionScriptInstanceDataPtr p_instance) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    self->refcount_incremented();
}

GDExtensionBool JSScriptInstance::_refcount_decremented(GDExtensionScriptInstanceDataPtr p_instance) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    return self->refcount_decremented();
}

GDExtensionObjectPtr JSScriptInstance::_get_script(GDExtensionScriptInstanceDataPtr p_instance) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    if (self->script_) {
        return self->script_->_owner;
    }
    return nullptr;
}

GDExtensionBool JSScriptInstance::_is_placeholder(GDExtensionScriptInstanceDataPtr p_instance) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->is_placeholder();
}

GDExtensionBool JSScriptInstance::_set_fallback(GDExtensionScriptInstanceDataPtr p_instance,
                                                 GDExtensionConstStringNamePtr p_name,
                                                 GDExtensionConstVariantPtr p_value) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    return self->set_fallback(*(const StringName*)p_name, *(const Variant*)p_value);
}

GDExtensionBool JSScriptInstance::_get_fallback(GDExtensionScriptInstanceDataPtr p_instance,
                                                 GDExtensionConstStringNamePtr p_name,
                                                 GDExtensionVariantPtr r_ret) {
    const JSScriptInstance* self = static_cast<const JSScriptInstance*>(p_instance);
    return self->get_fallback(*reinterpret_cast<const StringName*>(p_name), *reinterpret_cast<Variant*>(r_ret));
}

GDExtensionScriptLanguagePtr JSScriptInstance::_get_language(GDExtensionScriptInstanceDataPtr p_instance) {
    JSScriptLanguage* lang = JSScriptLanguage::get_singleton();
    return lang ? lang->_owner : nullptr;
}

void JSScriptInstance::_free(GDExtensionScriptInstanceDataPtr p_instance) {
    JSScriptInstance* self = (JSScriptInstance*)p_instance;
    memdelete(self);
}

// ============================================================================
// Instance Creation
// ============================================================================

const GDExtensionScriptInstanceInfo3& JSScriptInstance::get_script_instance_info() {
    if (!script_instance_info_initialized) {
        script_instance_info.set_func = _set;
        script_instance_info.get_func = _get;
        script_instance_info.get_property_list_func = _get_property_list;
        script_instance_info.free_property_list_func = _free_property_list;
        script_instance_info.get_class_category_func = _get_class_category;
        script_instance_info.property_can_revert_func = _property_can_revert;
        script_instance_info.property_get_revert_func = _property_get_revert;
        script_instance_info.get_owner_func = _get_owner;
        script_instance_info.get_property_state_func = _get_property_state;
        script_instance_info.get_method_list_func = _get_method_list;
        script_instance_info.free_method_list_func = _free_method_list;
        script_instance_info.get_property_type_func = _get_property_type;
        script_instance_info.validate_property_func = _validate_property;
        script_instance_info.has_method_func = _has_method;
        script_instance_info.get_method_argument_count_func = _get_method_argument_count;
        script_instance_info.call_func = _call;
        script_instance_info.notification_func = _notification;
        script_instance_info.to_string_func = _to_string;
        script_instance_info.refcount_incremented_func = _refcount_incremented;
        script_instance_info.refcount_decremented_func = _refcount_decremented;
        script_instance_info.get_script_func = _get_script;
        script_instance_info.is_placeholder_func = _is_placeholder;
        script_instance_info.set_fallback_func = _set_fallback;
        script_instance_info.get_fallback_func = _get_fallback;
        script_instance_info.get_language_func = _get_language;
        script_instance_info.free_func = _free;
        script_instance_info_initialized = true;
    }
    return script_instance_info;
}

GDExtensionScriptInstancePtr JSScriptInstance::create_instance(JSScriptInstance* p_instance) {
    // Get the script_instance_create3 function
    static GDExtensionInterfaceScriptInstanceCreate3 script_instance_create3 = nullptr;
    if (!script_instance_create3) {
        script_instance_create3 = (GDExtensionInterfaceScriptInstanceCreate3)
            internal::gdextension_interface_get_proc_address("script_instance_create3");
    }

    if (!script_instance_create3) {
        UtilityFunctions::printerr("Failed to get script_instance_create3");
        return nullptr;
    }

    return script_instance_create3(&get_script_instance_info(), p_instance);
}

} // namespace jsb

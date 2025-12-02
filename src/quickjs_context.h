#ifndef GODOT_JS_RUNTIME_QUICKJS_CONTEXT_H
#define GODOT_JS_RUNTIME_QUICKJS_CONTEXT_H

#include "../quickjs/quickjs.h"

#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include <memory>

namespace jsb {

class ObjectRegistry;
class SandboxConfig;
class ExecutionLimiter;
class SafeWrapper;
class GodotBindings;

// QuickJS context wrapper
class QuickJSContext {
public:
    QuickJSContext();
    ~QuickJSContext();

    // Non-copyable
    QuickJSContext(const QuickJSContext&) = delete;
    QuickJSContext& operator=(const QuickJSContext&) = delete;

    bool initialize();
    void shutdown();

    bool eval(const godot::String &code, const godot::String &filename,
              godot::Variant &result, godot::String &error);
    bool eval_file(const godot::String &path, godot::Variant &result, godot::String &error);

    void set_global(const godot::String &name, const godot::Variant &value);
    godot::Variant get_global(const godot::String &name);

    void set_memory_limit(size_t bytes);
    void set_timeout_ms(int64_t ms);

    JSContext* ctx() const { return ctx_; }
    JSRuntime* rt() const { return rt_; }
    bool is_valid() const { return ctx_ != nullptr && rt_ != nullptr; }

    // Type conversion
    JSValue variant_to_js(const godot::Variant &value);
    godot::Variant js_to_variant(JSValue value);

    // Script instance management
    // Creates a persistent JS object for a script instance and returns its ID
    // The script source is evaluated and the object is stored internally
    int64_t create_script_instance(const godot::String &source, const godot::String &filename,
                                   godot::Object* owner, godot::String &error);

    // Call a method on a script instance
    bool call_instance_method(int64_t instance_id, const godot::StringName &method,
                              const godot::Variant** args, int argc,
                              godot::Variant &result, godot::String &error);

    // Release a script instance (called when JSScriptInstance is destroyed)
    void release_script_instance(int64_t instance_id);

    // Check if instance exists
    bool has_script_instance(int64_t instance_id) const;

    // Sandbox components
    void set_object_registry(ObjectRegistry* registry) { object_registry_ = registry; }
    void set_sandbox_config(SandboxConfig* config) { sandbox_config_ = config; }
    void set_execution_limiter(ExecutionLimiter* limiter) { execution_limiter_ = limiter; }
    void set_safe_wrapper(SafeWrapper* wrapper) { safe_wrapper_ = wrapper; }

    ObjectRegistry* get_object_registry() const { return object_registry_; }
    SandboxConfig* get_sandbox_config() const { return sandbox_config_; }
    ExecutionLimiter* get_execution_limiter() const { return execution_limiter_; }
    SafeWrapper* get_safe_wrapper() const { return safe_wrapper_; }
    GodotBindings* get_bindings() const { return bindings_.get(); }

private:
    JSRuntime* rt_ = nullptr;
    JSContext* ctx_ = nullptr;

    ObjectRegistry* object_registry_ = nullptr;
    SandboxConfig* sandbox_config_ = nullptr;
    ExecutionLimiter* execution_limiter_ = nullptr;
    SafeWrapper* safe_wrapper_ = nullptr;

    std::unique_ptr<GodotBindings> bindings_;

    int64_t timeout_ms_ = 1000;
    mutable int64_t deadline_ = 0;  // mutable: accessed from const interrupt_handler

    // Script instance storage: maps instance_id -> JSValue (persistent object)
    struct ScriptInstanceData {
        JSValue js_object;      // The JS object instance
        godot::Object* owner;   // The Godot object this script is attached to
        bool valid;
    };
    godot::HashMap<int64_t, ScriptInstanceData> script_instances_;
    int64_t next_instance_id_ = 1;

    void setup_builtins();
    void setup_godot_bindings();

    static int interrupt_handler(JSRuntime* rt, void* opaque);

    godot::String get_exception_message();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_QUICKJS_CONTEXT_H

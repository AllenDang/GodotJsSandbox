#ifndef GODOT_JS_RUNTIME_JS_SANDBOX_H
#define GODOT_JS_RUNTIME_JS_SANDBOX_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "quickjs_context.h"
#include "object_registry.h"
#include "sandbox_config.h"
#include "execution_limiter.h"
#include "safe_wrapper.h"
#include "signal_registry.h"
#include "deletion_tracker.h"

#include <memory>

namespace jsb {

// JSSandbox is the main API for executing JavaScript code in a sandboxed environment
class JSSandbox : public godot::RefCounted {
    GDCLASS(JSSandbox, godot::RefCounted);

public:
    JSSandbox();
    ~JSSandbox();

    // Configuration
    void set_timeout_ms(int ms);
    void set_memory_limit_mb(int mb);
    godot::Error load_blocklist(const godot::String &path);

    // Execution
    godot::Variant eval(const godot::String &code);
    godot::Variant eval_module(const godot::String &code, const godot::String &filename);
    godot::Variant eval_file(const godot::String &path);

    // Global variables
    void set_global(const godot::String &name, const godot::Variant &value);
    godot::Variant get_global(const godot::String &name);

    // Level persistence
    godot::Error save_level(godot::Node *root, const godot::String &directory);
    godot::Node* load_level(const godot::String &directory);
    godot::Array get_created_nodes();
    godot::Dictionary get_attached_scripts();

    // Utility
    godot::String get_last_error() const { return last_error_; }
    bool is_valid() const;
    void reset();

    // Context access (for JSScriptInstance integration)
    QuickJSContext* get_context() const { return context_.get(); }

    // Create a script that uses this sandbox's context
    godot::Ref<godot::Script> create_script(const godot::String &source_code);

protected:
    static void _bind_methods();

private:
    std::unique_ptr<QuickJSContext> context_;
    std::unique_ptr<ObjectRegistry> object_registry_;
    std::unique_ptr<SandboxConfig> sandbox_config_;
    std::unique_ptr<ExecutionLimiter> execution_limiter_;
    std::unique_ptr<SafeWrapper> safe_wrapper_;
    std::unique_ptr<SignalRegistry> signal_registry_;
    std::unique_ptr<DeletionTracker> deletion_tracker_;

    godot::HashMap<uint64_t, godot::String> attached_scripts_;

    godot::String last_error_;

    bool initialize();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_SANDBOX_H

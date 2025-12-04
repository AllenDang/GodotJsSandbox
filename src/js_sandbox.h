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

    // Rate limiting configuration (per PRD Section 6.4)
    void set_write_ops_per_frame(int count);
    void set_heavy_ops_per_frame(int count);
    void reset_frame_counters();  // Call at start of each frame or between tests

    // Execution
    godot::Variant eval(const godot::String &code);
    godot::Variant eval_module(const godot::String &code, const godot::String &filename);
    godot::Variant eval_file(const godot::String &path);

    // Global variables
    void set_global(const godot::String &name, const godot::Variant &value);
    godot::Variant get_global(const godot::String &name);

    // Scene loading with sandbox isolation
    // Loads a .tscn scene and reattaches all JS scripts to use this sandbox's context
    godot::Node* load_scene(const godot::String &scene_path);

    // Level persistence
    godot::Error save_level(godot::Node *root, const godot::String &directory);
    godot::Node* load_level(const godot::String &directory);
    godot::Array get_created_nodes();
    godot::Dictionary get_attached_scripts();

    // Utility
    godot::String get_last_error() const { return last_error_; }
    godot::Array get_all_errors() const { return errors_; }
    void clear_errors();
    bool is_valid() const;
    void reset();

    // Async support - execute pending microtasks/promise callbacks
    int execute_pending_jobs();

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
    godot::Array errors_;  // Accumulated errors for AI feedback

    bool initialize();
    void add_error(const godot::String& type, const godot::String& message,
                   const godot::String& file = "", int line = 0, int column = 0);

    // Helper to recursively reattach JS scripts to this sandbox
    void reattach_scripts_recursive(godot::Node* node);
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_SANDBOX_H

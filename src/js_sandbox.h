#ifndef GODOT_JS_RUNTIME_JS_SANDBOX_H
#define GODOT_JS_RUNTIME_JS_SANDBOX_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "quickjs_context.h"
#include "object_registry.h"
#include "array_registry.h"
#include "sandbox_config.h"
#include "execution_limiter.h"
#include "safe_wrapper.h"
#include "signal_registry.h"
#include "deletion_tracker.h"
#include "async_scene_loader.h"
#include "rid_registry.h"
#include "sandbox_logger.h"

#include <memory>

namespace jsb {

// Execution phase for error categorization
enum class ExecutionPhase {
    LOAD,       // During code loading/parsing
    INIT,       // During _ready() and initial setup
    RUNTIME     // During gameplay/user interaction
};

// Enhanced error entry with context for AI feedback
struct ErrorEntry {
    godot::String id;               // Unique ID for deduplication (hash of type+message+file+line)
    godot::String type;             // "javascript", "godot_engine", "timeout", "security", "scene", "script"
    godot::String severity;         // "error", "warning"
    godot::String message;          // Human-readable message
    godot::String file;             // Source file path
    int line = 0;                   // Line number
    int column = 0;                 // Column number
    godot::String stack_trace;      // Full stack trace if available
    godot::String trigger_context;  // What triggered: "_ready", "_process", "signal:pressed", etc.
    godot::String phase;            // "load", "init", "runtime"
    int64_t timestamp = 0;          // When first occurred (Unix time ms)
    int64_t last_occurrence = 0;    // When last occurred
    int occurrence_count = 1;       // How many times this error happened

    // Convert to Dictionary for GDScript access
    godot::Dictionary to_dict() const;

    // Compute unique ID for deduplication
    static godot::String compute_id(const godot::String& type, const godot::String& message,
                                     const godot::String& file, int line);
};

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

    // Async scene loading - returns a loader object that can be polled for progress
    // Usage:
    //   var loader = sandbox.load_scene_async("res://scene.tscn")
    //   loader.progress_changed.connect(func(progress, stage): ...)
    //   loader.completed.connect(func(scene): ...)
    //   loader.failed.connect(func(error): ...)
    //   # Call loader.poll() each frame until is_completed() or is_failed()
    godot::Ref<AsyncSceneLoader> load_scene_async(const godot::String &scene_path);

    // Level persistence
    godot::Error save_level(godot::Node *root, const godot::String &directory);
    godot::Node* load_level(const godot::String &directory);
    godot::Array get_created_nodes();
    godot::Dictionary get_attached_scripts();

    // Utility
    godot::String get_last_error() const { return last_error_; }
    godot::Array get_all_errors() const;  // Returns Array of Dictionary (deduplicated)
    void clear_errors();
    bool is_valid() const;
    void reset();

    // Async support - execute pending microtasks/promise callbacks
    int execute_pending_jobs();

    // Enhanced error reporting for AI feedback
    // Returns a markdown-formatted report suitable for AI consumption
    godot::String get_error_report() const;

    // Get errors as structured array (same as get_all_errors but explicit)
    godot::Array get_errors_for_ai() const;

    // Phase management - allows external control of error categorization
    void set_phase(ExecutionPhase phase);
    ExecutionPhase get_phase() const { return current_phase_; }

    // Set execution context for better error attribution
    // Call before executing user code to track what triggered potential errors
    void set_execution_context(const godot::String& context);
    void clear_execution_context();

    // Start init phase timer (called after successful load)
    // After timeout, transitions to RUNTIME phase and emits init_completed
    void start_init_phase(float timeout_seconds = 0.5f);

    // Context access (for JSScriptInstance integration)
    QuickJSContext* get_context() const { return context_.get(); }

    // Create a script that uses this sandbox's context
    godot::Ref<godot::Script> create_script(const godot::String &source_code);

    // Error reporting (public for JSScriptInstance to report errors)
    // Enhanced version with optional stack trace and severity
    void add_error(const godot::String& type, const godot::String& message,
                   const godot::String& file = "", int line = 0, int column = 0,
                   const godot::String& stack_trace = "",
                   const godot::String& severity = "error");

    // Get current execution context (for SandboxLogger)
    godot::String get_current_context() const { return current_context_; }

    // Reattach JS scripts recursively to use this sandbox's context
    // Called after PackedScene.instantiate() to ensure scripts use sandbox limits
    void reattach_scripts_recursive(godot::Node* node);

protected:
    static void _bind_methods();

private:
    std::unique_ptr<QuickJSContext> context_;
    std::unique_ptr<ObjectRegistry> object_registry_;
    std::unique_ptr<ArrayRegistry> array_registry_;
    std::unique_ptr<SandboxConfig> sandbox_config_;
    std::unique_ptr<ExecutionLimiter> execution_limiter_;
    std::unique_ptr<SafeWrapper> safe_wrapper_;
    std::unique_ptr<SignalRegistry> signal_registry_;
    std::unique_ptr<DeletionTracker> deletion_tracker_;
    std::unique_ptr<RidRegistry> rid_registry_;
    godot::Ref<SandboxLogger> logger_;

    godot::HashMap<uint64_t, godot::String> attached_scripts_;

    godot::String last_error_;

    // Enhanced error tracking with deduplication
    godot::HashMap<godot::String, ErrorEntry> error_map_;  // Keyed by error ID
    godot::Vector<godot::String> error_order_;  // Maintain insertion order

    // Phase and context tracking
    ExecutionPhase current_phase_ = ExecutionPhase::LOAD;
    godot::String current_context_;  // Current execution context (e.g., "_process", "signal:clicked")

    // Init phase management
    bool init_phase_active_ = false;
    int errors_at_init_start_ = 0;  // Error count when init phase started

    // Debouncing for errors_updated signal
    bool errors_updated_pending_ = false;

    bool initialize();

    // Internal helpers
    godot::String phase_to_string(ExecutionPhase phase) const;
    void emit_errors_updated();
    void on_init_phase_timeout();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_SANDBOX_H

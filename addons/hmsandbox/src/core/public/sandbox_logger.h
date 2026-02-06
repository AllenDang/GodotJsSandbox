#ifndef GODOT_JS_RUNTIME_SANDBOX_LOGGER_H
#define GODOT_JS_RUNTIME_SANDBOX_LOGGER_H

#include <godot_cpp/classes/logger.hpp>
#include <godot_cpp/classes/script_backtrace.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/templates/vector.hpp>

#include <mutex>

namespace jsb {

class JSSandbox;

// SandboxLogger captures Godot engine errors and routes them to the sandbox
// for AI feedback. This catches errors that don't go through our SafeWrapper,
// including internal Godot errors, warnings, shader errors, and script errors.
//
// Thread-safety: _log_error and _log_message may be called from multiple threads,
// so we use a mutex to protect the error queue.
class SandboxLogger : public godot::Logger {
    GDCLASS(SandboxLogger, godot::Logger);

public:
    SandboxLogger();
    ~SandboxLogger();

    // Set the sandbox to route errors to
    // Pass nullptr to disable error routing
    void set_sandbox(JSSandbox* sandbox);

    // Virtual overrides from Logger
    virtual void _log_error(const godot::String &p_function, const godot::String &p_file,
                           int32_t p_line, const godot::String &p_code,
                           const godot::String &p_rationale, bool p_editor_notify,
                           int32_t p_error_type,
                           const godot::TypedArray<godot::Ref<godot::ScriptBacktrace>> &p_script_backtraces) override;

    virtual void _log_message(const godot::String &p_message, bool p_error) override;

    // Process queued errors on the main thread
    // Call this from JSSandbox to flush errors to the sandbox
    void flush_errors();

    // Error type names for human-readable output
    static godot::String get_error_type_name(int32_t error_type);

protected:
    static void _bind_methods();

private:
    // Error entry for thread-safe queueing
    struct ErrorEntry {
        godot::String type;
        godot::String severity;  // "error" or "warning"
        godot::String message;
        godot::String file;
        int line;
        int column;
    };

    JSSandbox* sandbox_ = nullptr;
    std::mutex mutex_;
    godot::Vector<ErrorEntry> error_queue_;
    bool is_flushing_ = false;  // Guard against recursive logging
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_SANDBOX_LOGGER_H

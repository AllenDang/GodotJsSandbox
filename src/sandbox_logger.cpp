#include "sandbox_logger.h"
#include "js_sandbox.h"

#include <godot_cpp/classes/script_backtrace.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

SandboxLogger::SandboxLogger() {
}

SandboxLogger::~SandboxLogger() {
}

void SandboxLogger::set_sandbox(JSSandbox* sandbox) {
    std::lock_guard<std::mutex> lock(mutex_);
    sandbox_ = sandbox;
}

String SandboxLogger::get_error_type_name(int32_t error_type) {
    switch (error_type) {
        case Logger::ERROR_TYPE_ERROR:
            return "error";
        case Logger::ERROR_TYPE_WARNING:
            return "warning";
        case Logger::ERROR_TYPE_SCRIPT:
            return "script_error";
        case Logger::ERROR_TYPE_SHADER:
            return "shader_error";
        default:
            return "unknown";
    }
}

void SandboxLogger::_log_error(const String &p_function, const String &p_file,
                               int32_t p_line, const String &p_code,
                               const String &p_rationale, bool p_editor_notify,
                               int32_t p_error_type,
                               const TypedArray<Ref<ScriptBacktrace>> &p_script_backtraces) {
    // Skip if we're currently flushing to avoid recursive capture
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (is_flushing_) {
            return;
        }
    }

    // Build error message
    String message;

    // Use rationale if available, otherwise use code
    if (!p_rationale.is_empty()) {
        message = p_rationale;
    } else if (!p_code.is_empty()) {
        message = p_code;
    } else {
        message = "Unknown error";
    }

    // Add function context if available
    if (!p_function.is_empty()) {
        message = p_function + String(": ") + message;
    }

    // Add backtrace info if available
    if (p_script_backtraces.size() > 0) {
        message += String("\n\nBacktrace:");
        for (int i = 0; i < p_script_backtraces.size(); i++) {
            Ref<ScriptBacktrace> bt = p_script_backtraces[i];
            if (bt.is_valid() && !bt->is_empty()) {
                // Use format() to get a formatted backtrace string
                message += String("\n") + bt->format(2, 4);
            }
        }
    }

    // Queue the error for processing on main thread
    ErrorEntry entry;
    entry.type = get_error_type_name(p_error_type);
    // Determine severity based on error type
    entry.severity = (p_error_type == Logger::ERROR_TYPE_WARNING) ? "warning" : "error";
    entry.message = message;
    entry.file = p_file;
    entry.line = p_line;
    entry.column = 0;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        error_queue_.push_back(entry);
    }
}

void SandboxLogger::_log_message(const String &p_message, bool p_error) {
    // Only capture error messages, not regular prints
    if (!p_error) {
        return;
    }

    // Skip if we're currently flushing to avoid recursive capture
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (is_flushing_) {
            return;
        }
    }

    ErrorEntry entry;
    entry.type = "message";
    entry.severity = "error";  // _log_message with p_error=true is an error
    entry.message = p_message;
    entry.file = "";
    entry.line = 0;
    entry.column = 0;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        error_queue_.push_back(entry);
    }
}

void SandboxLogger::flush_errors() {
    Vector<ErrorEntry> errors_to_process;

    // Move errors out of queue while holding lock
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (error_queue_.is_empty() || is_flushing_) {
            return;
        }
        is_flushing_ = true;
        errors_to_process = error_queue_;
        error_queue_.clear();
    }

    // Process errors without holding lock
    if (sandbox_) {
        for (int i = 0; i < errors_to_process.size(); i++) {
            const ErrorEntry& entry = errors_to_process[i];
            sandbox_->add_error(entry.type, entry.message, entry.file, entry.line, entry.column,
                                "", entry.severity);  // No stack trace from Godot errors
        }
    }

    // Clear the flushing flag
    {
        std::lock_guard<std::mutex> lock(mutex_);
        is_flushing_ = false;
    }
}

void SandboxLogger::_bind_methods() {
    // No methods need to be exposed to GDScript
}

} // namespace jsb

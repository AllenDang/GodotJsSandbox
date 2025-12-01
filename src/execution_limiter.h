#ifndef GODOT_JS_RUNTIME_EXECUTION_LIMITER_H
#define GODOT_JS_RUNTIME_EXECUTION_LIMITER_H

#include <cstdint>
#include <cstddef>

namespace jsb {

// ExecutionLimiter tracks resource usage and enforces limits
class ExecutionLimiter {
public:
    ExecutionLimiter();
    ~ExecutionLimiter();

    // Configuration
    void set_timeout_ms(int64_t ms);
    void set_memory_limit_mb(int mb);
    void set_max_api_calls_per_frame(int count);

    // Execution tracking
    void begin_execution();
    void end_execution();
    bool is_executing() const { return is_executing_; }

    // Timeout checking
    bool is_timeout_exceeded() const;
    int64_t get_remaining_time_ms() const;
    int64_t get_timeout_ms() const { return timeout_ms_; }

    // API call rate limiting
    bool check_api_rate_limit();
    void reset_frame_counters();
    int get_api_calls_this_frame() const { return api_calls_this_frame_; }

    // Memory tracking
    void set_current_memory_usage(size_t bytes);
    bool is_memory_limit_exceeded() const;
    size_t get_memory_limit_bytes() const { return memory_limit_bytes_; }

    // Statistics
    int64_t get_last_execution_time_ms() const { return last_execution_time_ms_; }
    int get_total_api_calls() const { return total_api_calls_; }

    // Reset statistics
    void reset_stats();

private:
    // Timeout settings
    int64_t timeout_ms_ = 1000;  // Default 1 second
    int64_t execution_start_time_ = 0;
    int64_t last_execution_time_ms_ = 0;
    bool is_executing_ = false;

    // Memory limits
    size_t memory_limit_bytes_ = 64 * 1024 * 1024;  // Default 64MB
    size_t current_memory_usage_ = 0;

    // API rate limiting
    int max_api_calls_per_frame_ = 1000;
    int api_calls_this_frame_ = 0;
    int total_api_calls_ = 0;
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_EXECUTION_LIMITER_H

#ifndef GODOT_JS_RUNTIME_EXECUTION_LIMITER_H
#define GODOT_JS_RUNTIME_EXECUTION_LIMITER_H

#include <cstdint>
#include <cstddef>

namespace jsb {

// API operation categories per PRD Section 6.4
enum class ApiCategory {
    READ,   // get_position, get_name, etc. - unlimited
    WRITE,  // set_position, add_child, etc. - limited per frame
    HEAVY   // instantiate, queue_free, etc. - heavily limited per frame
};

// ExecutionLimiter tracks resource usage and enforces limits
class ExecutionLimiter {
public:
    ExecutionLimiter();
    ~ExecutionLimiter();

    // Configuration
    void set_timeout_ms(int64_t ms);
    void set_memory_limit_mb(int mb);
    void set_max_api_calls_per_frame(int count);  // Legacy: sets write limit

    // Tiered rate limit configuration (per PRD Section 6.4)
    void set_write_ops_per_frame(int count);
    void set_heavy_ops_per_frame(int count);

    // Execution tracking
    void begin_execution();
    void end_execution();
    bool is_executing() const { return is_executing_; }

    // Timeout checking
    bool is_timeout_exceeded() const;
    int64_t get_remaining_time_ms() const;
    int64_t get_timeout_ms() const { return timeout_ms_; }

    // API call rate limiting (legacy - treats all as WRITE)
    bool check_api_rate_limit();

    // Tiered API call rate limiting (per PRD Section 6.4)
    bool check_api_rate_limit(ApiCategory category);

    void reset_frame_counters();
    int get_api_calls_this_frame() const { return api_calls_this_frame_; }
    int get_write_ops_this_frame() const { return write_ops_this_frame_; }
    int get_heavy_ops_this_frame() const { return heavy_ops_this_frame_; }
    int get_max_write_ops_per_frame() const { return max_write_ops_per_frame_; }
    int get_max_heavy_ops_per_frame() const { return max_heavy_ops_per_frame_; }

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

    // Legacy API rate limiting (backwards compatibility)
    int max_api_calls_per_frame_ = 1000;
    int api_calls_this_frame_ = 0;
    int total_api_calls_ = 0;

    // Tiered rate limiting per PRD Section 6.4
    // Read operations: unlimited (not tracked)
    // Write operations: 500/frame default
    // Heavy operations: 50/frame default
    int max_write_ops_per_frame_ = 500;
    int max_heavy_ops_per_frame_ = 50;
    int write_ops_this_frame_ = 0;
    int heavy_ops_this_frame_ = 0;

    // Auto-reset based on engine frame count
    uint64_t last_frame_count_ = 0;
    void check_auto_reset();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_EXECUTION_LIMITER_H

#include "execution_limiter.h"

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

namespace jsb {

ExecutionLimiter::ExecutionLimiter() {
}

void ExecutionLimiter::check_auto_reset() {
    // Auto-reset frame counters when a new frame starts
    uint64_t current_frame = Engine::get_singleton()->get_process_frames();
    if (current_frame != last_frame_count_) {
        last_frame_count_ = current_frame;
        api_calls_this_frame_ = 0;
        write_ops_this_frame_ = 0;
        heavy_ops_this_frame_ = 0;
    }
}

ExecutionLimiter::~ExecutionLimiter() {
}

void ExecutionLimiter::set_timeout_ms(int64_t ms) {
    timeout_ms_ = ms > 0 ? ms : 1000;
}

void ExecutionLimiter::set_memory_limit_mb(int mb) {
    memory_limit_bytes_ = static_cast<size_t>(mb > 0 ? mb : 64) * 1024 * 1024;
}

void ExecutionLimiter::set_max_api_calls_per_frame(int count) {
    max_api_calls_per_frame_ = count > 0 ? count : 1000;
    // Also update write ops for backwards compatibility
    max_write_ops_per_frame_ = count > 0 ? count : 500;
}

void ExecutionLimiter::set_write_ops_per_frame(int count) {
    max_write_ops_per_frame_ = count > 0 ? count : 500;
}

void ExecutionLimiter::set_heavy_ops_per_frame(int count) {
    max_heavy_ops_per_frame_ = count > 0 ? count : 50;
}

void ExecutionLimiter::begin_execution() {
    is_executing_ = true;
    execution_start_time_ = Time::get_singleton()->get_ticks_msec();
}

void ExecutionLimiter::end_execution() {
    if (is_executing_) {
        int64_t now = Time::get_singleton()->get_ticks_msec();
        last_execution_time_ms_ = now - execution_start_time_;
        is_executing_ = false;
    }
}

bool ExecutionLimiter::is_timeout_exceeded() const {
    if (!is_executing_ || timeout_ms_ <= 0) {
        return false;
    }

    int64_t now = Time::get_singleton()->get_ticks_msec();
    return (now - execution_start_time_) >= timeout_ms_;
}

int64_t ExecutionLimiter::get_remaining_time_ms() const {
    if (!is_executing_) {
        return timeout_ms_;
    }

    int64_t now = Time::get_singleton()->get_ticks_msec();
    int64_t elapsed = now - execution_start_time_;
    int64_t remaining = timeout_ms_ - elapsed;
    return remaining > 0 ? remaining : 0;
}

bool ExecutionLimiter::check_api_rate_limit() {
    // Legacy: treat as WRITE operation
    return check_api_rate_limit(ApiCategory::WRITE);
}

bool ExecutionLimiter::check_api_rate_limit(ApiCategory category) {
    // Auto-reset counters if we're in a new frame
    check_auto_reset();

    total_api_calls_++;
    api_calls_this_frame_++;

    switch (category) {
        case ApiCategory::READ:
            // Read operations are unlimited
            return true;

        case ApiCategory::WRITE:
            write_ops_this_frame_++;
            if (max_write_ops_per_frame_ > 0 && write_ops_this_frame_ > max_write_ops_per_frame_) {
                return false;  // Write rate limit exceeded
            }
            return true;

        case ApiCategory::HEAVY:
            heavy_ops_this_frame_++;
            // Heavy operations also count as write operations
            write_ops_this_frame_++;
            if (max_heavy_ops_per_frame_ > 0 && heavy_ops_this_frame_ > max_heavy_ops_per_frame_) {
                return false;  // Heavy rate limit exceeded
            }
            if (max_write_ops_per_frame_ > 0 && write_ops_this_frame_ > max_write_ops_per_frame_) {
                return false;  // Write rate limit exceeded
            }
            return true;
    }

    return true;
}

void ExecutionLimiter::reset_frame_counters() {
    api_calls_this_frame_ = 0;
    write_ops_this_frame_ = 0;
    heavy_ops_this_frame_ = 0;
}

void ExecutionLimiter::set_current_memory_usage(size_t bytes) {
    current_memory_usage_ = bytes;
}

bool ExecutionLimiter::is_memory_limit_exceeded() const {
    return current_memory_usage_ > memory_limit_bytes_;
}

void ExecutionLimiter::reset_stats() {
    last_execution_time_ms_ = 0;
    total_api_calls_ = 0;
    api_calls_this_frame_ = 0;
    write_ops_this_frame_ = 0;
    heavy_ops_this_frame_ = 0;
    current_memory_usage_ = 0;
}

} // namespace jsb

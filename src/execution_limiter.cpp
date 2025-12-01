#include "execution_limiter.h"

#include <godot_cpp/classes/time.hpp>

using namespace godot;

namespace jsb {

ExecutionLimiter::ExecutionLimiter() {
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
    api_calls_this_frame_++;
    total_api_calls_++;

    if (max_api_calls_per_frame_ > 0 && api_calls_this_frame_ > max_api_calls_per_frame_) {
        return false;  // Rate limit exceeded
    }

    return true;
}

void ExecutionLimiter::reset_frame_counters() {
    api_calls_this_frame_ = 0;
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
    current_memory_usage_ = 0;
}

} // namespace jsb

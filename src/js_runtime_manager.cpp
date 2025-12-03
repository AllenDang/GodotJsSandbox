#include "js_runtime_manager.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

JSRuntimeManager* JSRuntimeManager::singleton_ = nullptr;

JSRuntimeManager::JSRuntimeManager() {
}

JSRuntimeManager::~JSRuntimeManager() {
    // Free all remaining contexts
    for (JSContext* ctx : active_contexts_) {
        JS_FreeContext(ctx);
    }
    active_contexts_.clear();
    context_count_ = 0;

    // Free the runtime
    if (runtime_) {
        JS_FreeRuntime(runtime_);
        runtime_ = nullptr;
    }
}

JSRuntimeManager* JSRuntimeManager::get_singleton() {
    return singleton_;
}

bool JSRuntimeManager::initialize() {
    if (singleton_) {
        return true;  // Already initialized
    }

    singleton_ = new JSRuntimeManager();
    return singleton_->ensure_runtime();
}

void JSRuntimeManager::shutdown() {
    if (singleton_) {
        delete singleton_;
        singleton_ = nullptr;
    }
}

bool JSRuntimeManager::ensure_runtime() {
    if (runtime_) {
        return true;
    }

    runtime_ = JS_NewRuntime();
    if (!runtime_) {
        UtilityFunctions::printerr("JSRuntimeManager: Failed to create QuickJS runtime");
        return false;
    }

    // Set memory limit for all contexts combined
    JS_SetMemoryLimit(runtime_, memory_limit_);

    UtilityFunctions::print("JSRuntimeManager: QuickJS runtime created");
    return true;
}

JSRuntime* JSRuntimeManager::get_runtime() {
    ensure_runtime();
    return runtime_;
}

JSContext* JSRuntimeManager::create_context() {
    if (!ensure_runtime()) {
        return nullptr;
    }

    JSContext* ctx = JS_NewContext(runtime_);
    if (!ctx) {
        UtilityFunctions::printerr("JSRuntimeManager: Failed to create QuickJS context");
        return nullptr;
    }

    active_contexts_.insert(ctx);
    context_count_++;

    return ctx;
}

void JSRuntimeManager::free_context(JSContext* ctx) {
    if (!ctx) {
        return;
    }

    if (active_contexts_.has(ctx)) {
        active_contexts_.erase(ctx);
        JS_FreeContext(ctx);
        context_count_--;

        // If no more contexts, we could optionally free the runtime
        // But we keep it alive for potential reuse
    }
}

void JSRuntimeManager::set_total_memory_limit(size_t bytes) {
    memory_limit_ = bytes;
    if (runtime_) {
        JS_SetMemoryLimit(runtime_, bytes);
    }
}

size_t JSRuntimeManager::get_memory_usage() const {
    if (!runtime_) {
        return 0;
    }

    JSMemoryUsage usage;
    JS_ComputeMemoryUsage(runtime_, &usage);
    return usage.malloc_size;
}

void JSRuntimeManager::register_runtime_bindings() {
    // This is a placeholder for any runtime-level bindings
    // Individual contexts will register their own bindings via GodotBindings
    bindings_registered_ = true;
}

} // namespace jsb

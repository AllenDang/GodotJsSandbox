#ifndef GODOT_JS_RUNTIME_JS_RUNTIME_MANAGER_H
#define GODOT_JS_RUNTIME_JS_RUNTIME_MANAGER_H

#include "../quickjs/quickjs.h"

#include <godot_cpp/templates/hash_set.hpp>
#include <cstdint>

namespace jsb {

class ObjectRegistry;
class SandboxConfig;
class ExecutionLimiter;
class SafeWrapper;

// JSRuntimeManager is a singleton that manages the shared QuickJS runtime.
// According to TDD Section 14.2: "Single JSRuntime + Multiple JSContext" architecture
//
// - JSRuntime is shared globally (manages memory pool, Godot class bindings)
// - Each JSSandbox/JSScriptLanguage gets its own JSContext (isolated global vars, scripts)
//
// Benefits:
// - Shared Godot class bindings reduce memory overhead
// - Contexts are isolated - one crashing doesn't affect others
// - Clean destruction - contexts can be destroyed independently
class JSRuntimeManager {
public:
    // Get singleton instance
    static JSRuntimeManager* get_singleton();

    // Initialize the manager (called once at plugin init)
    static bool initialize();

    // Shutdown the manager (called at plugin uninit)
    static void shutdown();

    // Get the shared runtime (creates if needed)
    JSRuntime* get_runtime();

    // Create a new context on the shared runtime
    // Returns nullptr on failure
    JSContext* create_context();

    // Free a context
    void free_context(JSContext* ctx);

    // Get the count of active contexts
    int get_context_count() const { return context_count_; }

    // Memory management
    void set_total_memory_limit(size_t bytes);
    size_t get_memory_usage() const;

    // Register Godot bindings on runtime (called once after runtime creation)
    // Note: Individual contexts still need setup_godot_bindings() for constructors
    void register_runtime_bindings();

private:
    JSRuntimeManager();
    ~JSRuntimeManager();

    // Non-copyable
    JSRuntimeManager(const JSRuntimeManager&) = delete;
    JSRuntimeManager& operator=(const JSRuntimeManager&) = delete;

    static JSRuntimeManager* singleton_;

    JSRuntime* runtime_ = nullptr;
    int context_count_ = 0;
    bool bindings_registered_ = false;

    size_t memory_limit_ = 128 * 1024 * 1024;  // Default 128MB total for all contexts

    // Track active contexts for cleanup
    godot::HashSet<JSContext*> active_contexts_;

    // Internal helper
    bool ensure_runtime();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_RUNTIME_MANAGER_H

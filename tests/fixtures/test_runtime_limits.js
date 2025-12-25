// Test script for verifying runtime instantiated scenes use sandbox limits
// This script is attached to a scene that gets instantiated via JS
// When _ready runs, it sets up a global function that creates nodes
// The test will verify this function is limited by the SANDBOX's limits,
// not the global default limits

function _ready() {
    // Store a function that creates nodes
    // This function should be rate-limited by the sandbox's ExecutionLimiter
    globalThis.__runtime_test_create_nodes = function(count) {
        var nodes = [];
        var error = null;
        try {
            for (var i = 0; i < count; i++) {
                nodes.push(new Node());
            }
        } catch (e) {
            error = e.toString();
        }
        return { created: nodes.length, error: error };
    };

    // Signal that the script is ready
    globalThis.__runtime_test_ready = true;
}

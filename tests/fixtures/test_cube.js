// Test script for async scene loading
globalThis.__async_test_loaded = true;
globalThis.__async_test_ready = false;

function _ready() {
    globalThis.__async_test_ready = true;
}

function _process(delta) {
    // Rotate the node
    if (this.rotation) {
        this.rotation.y += delta;
    }
}

// Test: Shared Runtime (JSRuntimeManager)
// Tests that multiple sandboxes share runtime but have isolated contexts

console.log('=== Shared Runtime Test ===');

// Set a unique identifier in this context
globalThis.__contextId = Math.random().toString(36).substr(2, 9);
console.log('This context ID: ' + globalThis.__contextId);

// The shared runtime means:
// 1. Memory is shared at the runtime level (more efficient)
// 2. But each sandbox has its own JS context (isolated globals)

// Try to detect if we're sharing memory with another context
// This should NOT see values from other sandboxes
if (typeof globalThis.__otherContextValue !== 'undefined') {
    console.log('WARNING: Seeing value from another context: ' + globalThis.__otherContextValue);
    console.log('Contexts may not be properly isolated!');
} else {
    console.log('SUCCESS: No cross-context leakage detected');
}

// Set a value that another sandbox should NOT be able to see
globalThis.__otherContextValue = 'secret_' + globalThis.__contextId;

// Test basic functionality to verify context works
var node = new Node3D();
node.name = 'SharedRuntimeTestNode';
console.log('Created node: ' + node.name);

console.log('Shared runtime test completed');

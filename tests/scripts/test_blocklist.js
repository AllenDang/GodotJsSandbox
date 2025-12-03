// Test: Blocklist Security
// Tests that dangerous methods and classes are properly blocked

console.log('=== Blocklist Security Test ===');

var results = [];

function test(name, fn) {
    try {
        fn();
        results.push({ name: name, passed: false, error: 'No exception thrown' });
    } catch (e) {
        results.push({ name: name, passed: true, error: e.message });
    }
}

// Test blocked methods
test('Object.set', function() {
    var node = new Node3D();
    node.set('name', 'test');
});

test('Object.call_deferred', function() {
    var node = new Node3D();
    node.call_deferred('queue_free');
});

test('Object.set_deferred', function() {
    var node = new Node3D();
    node.set_deferred('name', 'test');
});

// Test blocked classes
test('FileAccess', function() {
    var file = new FileAccess();
});

test('OS', function() {
    var os = new OS();
});

test('Thread', function() {
    var thread = new Thread();
});

test('HTTPRequest', function() {
    var http = new HTTPRequest();
});

test('StreamPeerTCP', function() {
    var peer = new StreamPeerTCP();
});

test('PacketPeerUDP', function() {
    var peer = new PacketPeerUDP();
});

test('IP', function() {
    var ip = new IP();
});

// Print results
console.log('\n--- Results ---');
var passed = 0;
var failed = 0;
for (var i = 0; i < results.length; i++) {
    var r = results[i];
    if (r.passed) {
        console.log('PASS: ' + r.name + ' (blocked: ' + r.error + ')');
        passed++;
    } else {
        console.log('FAIL: ' + r.name + ' - ' + r.error);
        failed++;
    }
}

console.log('\nTotal: ' + passed + ' passed, ' + failed + ' failed');

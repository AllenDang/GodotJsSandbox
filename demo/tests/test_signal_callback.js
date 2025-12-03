// Test: Signal Registry with CallableCustom
// This tests that JS callbacks are actually invoked when Godot signals emit

console.log('=== Signal Callback Test ===');

// This script should be attached to a node that has a child Timer
// The timer's timeout signal will trigger our callback

var callCount = 0;

function _ready() {
    console.log('Signal callback test: _ready');

    // Get the parent node (which has the timer)
    var timer = this.get_node_or_null('Timer');
    if (timer) {
        console.log('Found timer, connecting signal...');

        timer.connect('timeout', function() {
            callCount++;
            console.log('Signal received! Call count: ' + callCount);
        });

        timer.start();
    } else {
        console.log('No Timer child found - create one manually for testing');
    }
}

function _process(delta) {
    // Report status periodically
    if (callCount > 0 && callCount <= 3) {
        console.log('Signal has been received ' + callCount + ' time(s)');
    }
}

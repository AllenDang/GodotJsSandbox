// This script is designed to be attached to a Node3D
// It will receive _ready, _process, etc. callbacks

function _ready() {
    console.log("attached_script.js: _ready called!");
    console.log("This script is attached to a node and receiving callbacks.");
}

function _enter_tree() {
    console.log("attached_script.js: _enter_tree called!");
}

function _exit_tree() {
    console.log("attached_script.js: _exit_tree called!");
}

function _process(delta) {
    // Called every frame - uncomment to test
    // console.log("attached_script.js: _process delta =", delta);
}

// Test JavaScript script for GodotJSRuntime
// This script can be loaded and executed through JSSandbox

function _ready() {
    console.log("JavaScript _ready called!");
    console.log("Hello from test_script.js");
}

function _process(delta) {
    // Called every frame
    // console.log("Processing with delta:", delta);
}

function create_test_scene() {
    console.log("Creating test scene from JavaScript...");

    // Create a parent node
    var parent = new Node3D();
    parent.name = "JSGeneratedScene";

    // Create some child nodes
    for (var i = 0; i < 3; i++) {
        var child = new Node3D();
        child.name = "Child_" + i;
        child.position = new Vector3(i * 2, 0, 0);
        parent.add_child(child);
    }

    console.log("Created parent with", parent.get_child_count(), "children");
    return parent;
}

function multiply(a, b) {
    return a * b;
}

function greet(name) {
    return "Hello, " + name + " from JavaScript!";
}

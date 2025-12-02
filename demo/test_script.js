// Test JavaScript script for GodotJSRuntime
// This script can be loaded and executed through JSSandbox

function _ready() {
    console.log("JavaScript _ready called!");
    console.log("Hello from test_script.js");

    // Test the new static bindings
    test_static_bindings();
}

function _process(delta) {
    // Called every frame
    // console.log("Processing with delta:", delta);
}

function test_static_bindings() {
    console.log("\n=== Testing Static Bindings ===");

    // Test Node3D
    console.log("\n--- Node3D Tests ---");
    var node = new Node3D();
    node.name = "TestNode";
    console.log("Created node:", node.name);

    // Test position property (Vector3)
    node.position = {x: 1, y: 2, z: 3};
    console.log("Position after set:", JSON.stringify(node.position));

    // Test rotation property (float)
    node.rotation = {x: 0.5, y: 1.0, z: 0};
    console.log("Rotation after set:", JSON.stringify(node.rotation));

    // Test scale property (Vector3)
    node.scale = {x: 2, y: 2, z: 2};
    console.log("Scale after set:", JSON.stringify(node.scale));

    // Test Node2D
    console.log("\n--- Node2D Tests ---");
    var node2d = new Node2D();
    node2d.name = "TestNode2D";
    node2d.position = {x: 100, y: 200};
    console.log("Node2D position:", JSON.stringify(node2d.position));

    node2d.rotation = 1.57;
    console.log("Node2D rotation:", node2d.rotation);

    // Test Sprite2D
    console.log("\n--- Sprite2D Tests ---");
    var sprite = new Sprite2D();
    sprite.name = "TestSprite";
    sprite.centered = false;
    console.log("Sprite centered:", sprite.centered);
    sprite.centered = true;
    console.log("Sprite centered after set:", sprite.centered);

    // Test Label
    console.log("\n--- Label Tests ---");
    var label = new Label();
    label.name = "TestLabel";
    label.text = "Hello from JS!";
    console.log("Label text:", label.text);

    // Test Timer
    console.log("\n--- Timer Tests ---");
    var timer = new Timer();
    timer.wait_time = 2.5;
    console.log("Timer wait_time:", timer.wait_time);
    timer.one_shot = true;
    console.log("Timer one_shot:", timer.one_shot);

    // Test Control
    console.log("\n--- Control Tests ---");
    var control = new Control();
    control.custom_minimum_size = {x: 100, y: 50};
    console.log("Control custom_minimum_size:", JSON.stringify(control.custom_minimum_size));

    // Test method calls
    console.log("\n--- Method Call Tests ---");
    var parent = new Node3D();
    parent.name = "Parent";
    var child = new Node3D();
    child.name = "Child";
    parent.add_child(child);
    console.log("Parent child count:", parent.get_child_count());

    // Test error handling (accessing method on wrong type)
    console.log("\n--- Error Handling Tests ---");
    try {
        // This should work
        node.get_child_count();
        console.log("get_child_count() succeeded");
    } catch (e) {
        console.log("Caught error:", e.message);
    }

    console.log("\n=== Static Bindings Tests Complete ===");
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
        child.position = {x: i * 2, y: 0, z: 0};
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

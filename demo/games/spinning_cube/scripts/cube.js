// Spinning Cube - A simple demo game for the JS Sandbox
// Press Space to reverse spin direction

var speed = 2.0;
var direction = 1;
var time_elapsed = 0;

exports._ready = function() {
    console.log("Spinning Cube initialized!");
    console.log("Press SPACE to reverse direction");
};

exports._process = function(delta) {
    // Spin the cube
    this.rotate_y(speed * direction * delta);

    // Bob up and down
    time_elapsed += delta;
    var y_offset = Math.sin(time_elapsed * 2) * 0.2;
    this.position = {x: 0, y: y_offset, z: 0};

    // Check for input
    if (Input.is_action_just_pressed("reverse")) {
        direction *= -1;
        console.log("Direction reversed! Now: " + (direction > 0 ? "clockwise" : "counter-clockwise"));
    }
};

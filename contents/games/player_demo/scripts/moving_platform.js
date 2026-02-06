// Moving Platform - Oscillates back and forth
// Attach to AnimatableBody3D nodes
//
// Note: Each script instance has its own isolated scope (like GDScript),
// so module-level variables are per-instance, not shared.

var move_axis = 'x';
var move_distance = 5.0;
var move_speed = 2.0;
var time = 0;
var start_position = null;

exports._ready = function() {
    var name = this.name;
    console.log("Moving platform ready: " + name);

    // Store starting position
    start_position = {
        x: this.global_position.x,
        y: this.global_position.y,
        z: this.global_position.z
    };

    // Configure based on platform name for variety
    if (name === 'MovingPlatform1') {
        move_axis = 'z';
        move_distance = 4.0;
        move_speed = 1.5;
    } else if (name === 'MovingPlatform2') {
        move_axis = 'y';
        move_distance = 3.0;
        move_speed = 1.0;
    } else if (name === 'MovingPlatform3') {
        move_axis = 'z';
        move_distance = 5.0;
        move_speed = 2.5;
    }

    console.log("Platform " + name + " will move on " + move_axis + " axis");
};

exports._physics_process = function(delta) {
    if (!start_position) return;

    time += delta * move_speed;

    // Calculate offset using sine wave for smooth oscillation
    var offset = Math.sin(time) * move_distance;

    // Create new position based on axis
    var new_pos = {
        x: start_position.x,
        y: start_position.y,
        z: start_position.z
    };

    if (move_axis === 'x') {
        new_pos.x = start_position.x + offset;
    } else if (move_axis === 'y') {
        new_pos.y = start_position.y + offset;
    } else if (move_axis === 'z') {
        new_pos.z = start_position.z + offset;
    }

    this.global_position = new_pos;
};

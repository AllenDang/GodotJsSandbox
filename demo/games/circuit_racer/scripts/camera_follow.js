// Circuit Racer - Camera Follow
// Smooth third-person camera that follows behind the vehicle

var self = null;
var target = null;

// Camera settings
var follow_distance = 8.0;
var follow_height = 3.0;
var look_ahead = 2.0;
var smooth_speed = 5.0;

exports._ready = function() {
    self = this;

    // Get target (Vehicle is sibling node)
    var parent = this.get_parent();
    if (parent) {
        target = parent.get_node("Vehicle");
    }

    if (target) {
        console.log("Camera following vehicle");
    } else {
        console.log("Warning: Camera has no target");
    }
};

exports._physics_process = function(delta) {
    if (!target) return;

    var target_pos = target.global_position;

    // Get the vehicle's velocity to determine which way it's moving
    var vel = target.linear_velocity;
    var speed = Math.sqrt(vel.x * vel.x + vel.z * vel.z);

    // Use velocity direction if moving, otherwise use transform basis
    var forward_x, forward_z;

    if (speed > 1.0) {
        // Use velocity direction (normalized)
        forward_x = vel.x / speed;
        forward_z = vel.z / speed;
    } else {
        // Use transform's -Z axis (Godot's forward direction)
        var basis = target.global_transform.basis;
        forward_x = -basis.z.x;
        forward_z = -basis.z.z;
    }

    // Camera position: behind the vehicle
    var ideal_pos = {
        x: target_pos.x - forward_x * follow_distance,
        y: target_pos.y + follow_height,
        z: target_pos.z - forward_z * follow_distance
    };

    // Smooth camera position
    var current_pos = self.global_position;
    var new_pos = {
        x: lerp(current_pos.x, ideal_pos.x, smooth_speed * delta),
        y: lerp(current_pos.y, ideal_pos.y, smooth_speed * delta),
        z: lerp(current_pos.z, ideal_pos.z, smooth_speed * delta)
    };

    self.global_position = new_pos;

    // Look at point ahead of vehicle
    var look_target = {
        x: target_pos.x + forward_x * look_ahead,
        y: target_pos.y + 0.5,
        z: target_pos.z + forward_z * look_ahead
    };

    self.look_at(look_target, { x: 0, y: 1, z: 0 });
};

function lerp(a, b, t) {
    return a + (b - a) * Math.min(t, 1.0);
}

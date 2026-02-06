// Circuit Racer - Vehicle Controller
// Controls a VehicleBody3D using WASD/Arrow keys

var self = null;
var race_manager = null;

// Wheel references
var wheel_fl = null; // Front left
var wheel_fr = null; // Front right
var wheel_rl = null; // Rear left
var wheel_rr = null; // Rear right

// Vehicle settings
var engine_power = 150.0;
var brake_power = 5.0;
var max_steer_angle = 0.4; // radians (~23 degrees)
var steer_speed = 3.0;

// Current state
var current_steer = 0;
var current_engine = 0;
var current_brake = 0;

// Spawn position for reset
var spawn_position = { x: 0, y: 1, z: 0 };
var spawn_rotation = { x: 0, y: 0, z: 0 };

exports._ready = function() {
    console.log("=== Circuit Racer - Vehicle Controller ===");
    console.log("WASD/Arrows: Drive | Space: Handbrake | R: Reset");

    self = this;

    // Get wheel references
    wheel_fl = this.get_node("WheelFL");
    wheel_fr = this.get_node("WheelFR");
    wheel_rl = this.get_node("WheelRL");
    wheel_rr = this.get_node("WheelRR");

    // Get race manager (parent)
    race_manager = this.get_parent();

    // Store spawn position
    var pos = this.global_position;
    spawn_position = { x: pos.x, y: pos.y, z: pos.z };
    var rot = this.rotation;
    spawn_rotation = { x: rot.x, y: rot.y, z: rot.z };

    console.log("Vehicle ready with " + (wheel_fl ? "4" : "0") + " wheels");
};

exports._physics_process = function(delta) {
    // Get input
    var accelerate = Input.is_action_pressed("accelerate");
    var brake = Input.is_action_pressed("brake");
    var steer_left = Input.is_action_pressed("steer_left");
    var steer_right = Input.is_action_pressed("steer_right");
    var handbrake = Input.is_action_pressed("handbrake");

    // Calculate target steering
    var target_steer = 0;
    if (steer_left) target_steer -= max_steer_angle;
    if (steer_right) target_steer += max_steer_angle;

    // Smooth steering
    current_steer = lerp(current_steer, target_steer, steer_speed * delta);

    // Apply steering to front wheels
    if (wheel_fl) wheel_fl.steering = current_steer;
    if (wheel_fr) wheel_fr.steering = current_steer;

    // Calculate engine force (negative = forward in Godot, car faces -Z)
    if (accelerate) {
        current_engine = -engine_power;
    } else if (brake) {
        current_engine = engine_power * 0.5; // Reverse
    } else {
        current_engine = 0;
    }

    // Apply engine force
    this.engine_force = current_engine;

    // Braking
    if (handbrake) {
        this.brake = brake_power * 2;
    } else if (brake && get_speed() > 1) {
        this.brake = brake_power;
    } else {
        this.brake = 0;
    }

    // Update UI with speed
    if (race_manager && race_manager.update_speed) {
        race_manager.update_speed(get_speed());
    }
};

exports._input = function(event) {
    var event_class = event.get_class();
    if (event_class === "InputEventKey" && event.pressed) {
        if (event.keycode === Key.KEY_R) {
            reset_vehicle();
        }
    }
};

function get_speed() {
    // Get velocity magnitude in km/h
    var vel = self.linear_velocity;
    var speed_ms = Math.sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
    return speed_ms * 3.6; // Convert m/s to km/h
}

function reset_vehicle() {
    console.log("Resetting vehicle position");

    // Reset position and rotation
    self.global_position = spawn_position;
    self.rotation = spawn_rotation;

    // Stop all movement
    self.linear_velocity = { x: 0, y: 0, z: 0 };
    self.angular_velocity = { x: 0, y: 0, z: 0 };

    // Reset steering
    current_steer = 0;
    if (wheel_fl) wheel_fl.steering = 0;
    if (wheel_fr) wheel_fr.steering = 0;
}

function lerp(a, b, t) {
    return a + (b - a) * t;
}

// Called by race manager to set spawn point
exports.set_spawn = function(pos, rot) {
    spawn_position = { x: pos.x, y: pos.y, z: pos.z };
    spawn_rotation = { x: rot.x, y: rot.y, z: rot.z };
};

// Get current speed for external use
exports.get_current_speed = function() {
    return get_speed();
};

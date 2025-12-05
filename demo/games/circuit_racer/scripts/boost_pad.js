// Speed Boost Pad - Gives vehicle a speed boost when driven over
// Uses Area3D body_entered signal

var self = null;
var boost_power = 30000.0; // Impulse force (needs to be high for VehicleBody3D mass)
var cooldown = 0;
var cooldown_time = 1.0; // Seconds before can boost again

exports._ready = function() {
    self = this;

    // Connect body_entered signal
    this.connect("body_entered", exports.on_body_entered);

    console.log("Boost pad ready");
};

exports._process = function(delta) {
    if (cooldown > 0) {
        cooldown -= delta;
    }
};

// Called when a body enters the boost pad area
exports.on_body_entered = function(body) {
    // Check if it's the vehicle (layer 2) and not on cooldown
    if (body.collision_layer === 2 && cooldown <= 0) {
        // Get vehicle's forward direction from velocity
        var vel = body.linear_velocity;
        var current_speed = Math.sqrt(vel.x * vel.x + vel.z * vel.z);

        var forward_x, forward_z;
        if (current_speed > 1) {
            // Use velocity direction
            forward_x = vel.x / current_speed;
            forward_z = vel.z / current_speed;
        } else {
            // Use transform forward (-Z)
            var basis = body.global_transform.basis;
            forward_x = -basis.z.x;
            forward_z = -basis.z.z;
        }

        // Apply impulse in the direction the car is moving
        var impulse = {
            x: forward_x * boost_power,
            y: 0,
            z: forward_z * boost_power
        };

        body.apply_central_impulse(impulse);

        cooldown = cooldown_time;

        console.log("BOOST! Applied impulse at " + Math.floor(current_speed * 3.6) + " km/h");
    }
};

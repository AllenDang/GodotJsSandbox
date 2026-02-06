// Tire Particles - Dust/smoke when drifting or braking
// Attached to GPUParticles3D under vehicle wheels

var self = null;
var vehicle = null;

// Particle settings
var emit_threshold = 20; // Speed (km/h) above which particles emit
var max_particles_speed = 60; // Speed at which max particles emit

exports._ready = function() {
    self = this;

    // Get vehicle (grandparent: Vehicle > WheelXX > DustParticles)
    var wheel = this.get_parent();
    if (wheel) {
        vehicle = wheel.get_parent();
    }

    // Start with particles disabled
    self.emitting = false;

    console.log("Tire particles ready");
};

exports._process = function(delta) {
    if (!vehicle) return;

    // Get vehicle speed
    var vel = vehicle.linear_velocity;
    var speed_ms = Math.sqrt(vel.x * vel.x + vel.z * vel.z);
    var speed_kmh = speed_ms * 3.6;

    // Check if we should emit particles
    // Emit when braking hard or moving fast with steering
    var should_emit = false;
    var brake_value = vehicle.brake || 0;

    if (brake_value > 0 && speed_kmh > 5) {
        // Braking - emit smoke
        should_emit = true;
    } else if (speed_kmh > emit_threshold) {
        // Check if we're steering (drifting)
        var steering = Math.abs(vehicle.steering || 0);
        if (steering > 0.1) {
            should_emit = true;
        }
    }

    // Apply emission state
    self.emitting = should_emit;

    // Adjust particle amount based on speed
    if (should_emit) {
        var speed_ratio = Math.min(speed_kmh / max_particles_speed, 1.0);
        self.amount_ratio = 0.3 + speed_ratio * 0.7;
    }
};

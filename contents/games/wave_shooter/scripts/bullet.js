// Wave Shooter - Bullet System
// Physical bullet projectile with proper lifecycle management

// Bullet properties
var speed = 50.0;           // Units per second
var damage = 25;
var max_lifetime = 3.0;     // Auto-destroy after 3 seconds (for sky shots)
var max_distance = 100.0;   // Max travel distance before destruction

// State
var lifetime = 0;
var start_position = null;
var direction = { x: 0, y: 0, z: -1 };
var is_destroyed = false;

// References
var self = null;  // Reference to the node (this)
var game_manager = null;
var blood_splash_template = null;
var effects_container = null;

exports._ready = function() {
    // Store reference to this node for use in regular functions
    self = this;

    // Check if this is the template (direct child of Main) or a spawned bullet (child of Bullets)
    var parent = this.get_parent();
    if (!parent) return;

    var parent_name = parent.name;
    if (parent_name !== "Bullets") {
        // This is BulletTemplate - don't initialize
        is_destroyed = true;  // Prevent physics processing
        return;
    }

    // This is a spawned bullet - initialize properly
    is_destroyed = false;

    // Store starting position for distance check
    start_position = {
        x: this.global_position.x,
        y: this.global_position.y,
        z: this.global_position.z
    };

    // Find game manager (bullets are added to Bullets container which is child of Main)
    game_manager = parent.get_parent();

    // Get blood splash template and effects container for hit effects
    if (game_manager) {
        blood_splash_template = game_manager.get_node("BloodSplashTemplate");
        effects_container = game_manager.get_node("Effects");
    }
};

exports._physics_process = function(delta) {
    if (is_destroyed) return;

    // Update lifetime
    lifetime += delta;
    if (lifetime >= max_lifetime) {
        destroy_bullet("timeout");
        return;
    }

    // Move bullet forward
    var pos = this.global_position;
    pos.x += direction.x * speed * delta;
    pos.y += direction.y * speed * delta;
    pos.z += direction.z * speed * delta;
    this.global_position = pos;

    // Check max distance
    var dx = pos.x - start_position.x;
    var dy = pos.y - start_position.y;
    var dz = pos.z - start_position.z;
    var distance = Math.sqrt(dx * dx + dy * dy + dz * dz);

    if (distance >= max_distance) {
        destroy_bullet("max_distance");
        return;
    }
};

// Called when bullet's Area3D detects a collision
exports.on_body_entered = function(body) {
    if (is_destroyed) return;

    // Get hit position before destroying
    var hit_pos = {
        x: self.global_position.x,
        y: self.global_position.y,
        z: self.global_position.z
    };

    // Check if we hit an enemy
    if (body && body.take_damage) {
        body.take_damage(damage);
        spawn_blood_splash(hit_pos);
        destroy_bullet("hit_enemy");
    } else {
        // Hit something else (wall, floor, etc.)
        destroy_bullet("hit_world");
    }
};

// Set bullet direction (called by player controller after spawning)
exports.set_direction = function(dir) {
    direction = {
        x: dir.x || 0,
        y: dir.y || 0,
        z: dir.z || 0
    };

    // Normalize direction
    var len = Math.sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (len > 0) {
        direction.x /= len;
        direction.y /= len;
        direction.z /= len;
    }
};

// Set bullet damage (can be modified by powerups, etc.)
exports.set_damage = function(dmg) {
    damage = dmg;
};

// Set bullet speed
exports.set_speed = function(spd) {
    speed = spd;
};

function spawn_blood_splash(pos) {
    if (!blood_splash_template || !effects_container) return;

    // Duplicate the blood splash template
    var splash = blood_splash_template.duplicate();
    if (!splash) return;

    // Add to effects container first
    effects_container.add_child(splash);

    // Set position and make visible
    splash.global_position = pos;
    splash.visible = true;
    splash.emitting = true;

    // Connect finished signal to auto-remove
    splash.finished.connect(function() {
        splash.queue_free();
    });
}

function destroy_bullet(reason) {
    if (is_destroyed) return;
    is_destroyed = true;

    // Remove from scene using stored self reference
    if (self) {
        self.queue_free();
    }
}

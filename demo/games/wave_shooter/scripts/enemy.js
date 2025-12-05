// Wave Shooter - Enemy AI
// Simple enemy that moves toward player and attacks

var player = null;
var game_manager = null;
var self = null;  // Reference to this node

// Stats
var health = 100;
var max_health = 100;
var move_speed = 3.0;
var attack_damage = 10;
var attack_range = 2.0;
var attack_cooldown = 1.0;
var time_since_attack = 0;

// State
var is_dead = false;
var death_timer = 0;
var death_phase = 0;  // 0 = falling, 1 = laying on ground

// Hit feedback state
var is_hit_stunned = false;
var hit_stun_timer = 0;
var hit_stun_duration = 0.15;  // Brief stun when hit
var hit_knockback = { x: 0, y: 0, z: 0 };
var original_scale = { x: 1, y: 1, z: 1 };

// Navigation
var target_position = { x: 0, y: 0, z: 0 };

// Direction enemy was facing when killed (for death animation)
var death_direction = { x: 0, z: 1 };

exports._ready = function() {
    self = this;

    // Get references - spawned enemies are added to "Enemies" node which is child of Main
    // EnemyTemplate is a direct child of Main, so we need to detect which case we're in
    var parent = this.get_parent();
    if (!parent) return;

    var parent_name = parent.name;

    if (parent_name === "Enemies") {
        // This is a spawned enemy inside the Enemies container
        // Go up one more level to get Main (game manager)
        game_manager = parent.get_parent();
        if (game_manager) {
            player = game_manager.get_node("Player");
        }
    } else {
        // This is EnemyTemplate (direct child of Main) - don't initialize
        // Template enemies are invisible and disabled, they get duplicated for spawning
        return;
    }
};

exports._physics_process = function(delta) {
    if (is_dead) {
        handle_death_animation(delta);
        return;
    }

    // Handle hit stun recovery
    if (is_hit_stunned) {
        hit_stun_timer += delta;

        // Apply knockback
        var pos = this.global_position;
        pos.x += hit_knockback.x * delta * 10;
        pos.z += hit_knockback.z * delta * 10;
        this.global_position = pos;

        // Decay knockback
        hit_knockback.x *= 0.85;
        hit_knockback.z *= 0.85;

        // Wobble effect during stun - use uniform scale to avoid Jolt Physics errors
        var wobble = Math.sin(hit_stun_timer * 50) * 0.05;
        var uniform_scale = 1 + wobble;
        this.scale = {
            x: original_scale.x * uniform_scale,
            y: original_scale.y * uniform_scale,
            z: original_scale.z * uniform_scale
        };

        if (hit_stun_timer >= hit_stun_duration) {
            is_hit_stunned = false;
            hit_stun_timer = 0;
            this.scale = original_scale;
        }
        return;
    }

    if (!player) return;

    // Update attack timer
    time_since_attack += delta;

    // Get player position
    var player_pos = player.global_position;
    var my_pos = this.global_position;

    // Calculate direction to player
    var dir = {
        x: player_pos.x - my_pos.x,
        y: 0,
        z: player_pos.z - my_pos.z
    };

    // Calculate distance
    var distance = Math.sqrt(dir.x * dir.x + dir.z * dir.z);

    // Normalize direction
    if (distance > 0.1) {
        dir.x /= distance;
        dir.z /= distance;
    }

    // Store direction for death animation
    death_direction = { x: dir.x, z: dir.z };

    // Face player
    var angle = Math.atan2(dir.x, dir.z);
    this.rotation = { x: 0, y: angle, z: 0 };

    // Move toward player if not in attack range
    if (distance > attack_range) {
        var vel = {
            x: dir.x * move_speed,
            y: -10, // Gravity
            z: dir.z * move_speed
        };
        this.velocity = vel;
        this.move_and_slide();
    } else {
        // In attack range - attack!
        if (time_since_attack >= attack_cooldown) {
            attack();
        }
    }
};

function handle_death_animation(delta) {
    death_timer += delta;

    if (death_phase === 0) {
        // Phase 0: Fall backward rotation (0.3 seconds)
        var fall_progress = Math.min(death_timer / 0.3, 1.0);

        // Rotate to fall backward (away from player)
        var fall_angle = (Math.PI / 2) * fall_progress;  // 90 degrees

        // Calculate rotation - fall backward from facing direction
        var facing_angle = Math.atan2(death_direction.x, death_direction.z);
        self.rotation = {
            x: -fall_angle,  // Tilt backward
            y: facing_angle,
            z: 0
        };

        // Slight upward then downward motion
        var pos = self.global_position;
        if (fall_progress < 0.3) {
            pos.y += delta * 2;  // Brief upward
        } else {
            pos.y -= delta * 3;  // Fall down
        }
        // Also push back slightly
        pos.x -= death_direction.x * delta * 2;
        pos.z -= death_direction.z * delta * 2;
        self.global_position = pos;

        if (fall_progress >= 1.0) {
            death_phase = 1;
            death_timer = 0;
        }
    } else if (death_phase === 1) {
        // Phase 1: Lay on ground for 1 second, then sink and disappear
        if (death_timer > 1.0) {
            // Start sinking
            var pos = self.global_position;
            pos.y -= delta * 2;
            self.global_position = pos;

            // Fade out by scaling down - keep minimum scale of 0.01 to avoid physics errors
            var sink_progress = (death_timer - 1.0) / 0.5;
            var scale_factor = Math.max(0.01, 1 - sink_progress);
            self.scale = { x: scale_factor, y: scale_factor, z: scale_factor };

            if (death_timer > 1.5) {
                self.queue_free();
            }
        }
    }
}

function attack() {
    time_since_attack = 0;

    // Deal damage to player
    if (player && player.take_damage) {
        player.take_damage(attack_damage);
    }
}

// Called when hit by player
exports.take_damage = function(amount) {
    if (is_dead) return;

    health -= amount;

    if (health <= 0) {
        die();
    } else {
        // Hit feedback - knockback and stun
        apply_hit_feedback();
    }
};

function apply_hit_feedback() {
    is_hit_stunned = true;
    hit_stun_timer = 0;

    // Calculate knockback direction (away from player)
    if (player && self) {
        var player_pos = player.global_position;
        var my_pos = self.global_position;

        var knock_dir = {
            x: my_pos.x - player_pos.x,
            z: my_pos.z - player_pos.z
        };

        // Normalize
        var len = Math.sqrt(knock_dir.x * knock_dir.x + knock_dir.z * knock_dir.z);
        if (len > 0.1) {
            knock_dir.x /= len;
            knock_dir.z /= len;
        }

        // Apply knockback force
        hit_knockback = {
            x: knock_dir.x * 0.5,
            y: 0,
            z: knock_dir.z * 0.5
        };
    }
}

function die() {
    is_dead = true;
    death_timer = 0;
    death_phase = 0;

    // Notify game manager
    if (game_manager && game_manager.on_enemy_killed) {
        game_manager.on_enemy_killed();
    }

    console.log("Enemy killed!");
}

// Set enemy stats based on wave difficulty
exports.set_stats = function(wave_number) {
    // Scale difficulty with wave
    var multiplier = 1 + (wave_number - 1) * 0.2;

    max_health = Math.floor(100 * multiplier);
    health = max_health;
    move_speed = 3.0 + wave_number * 0.3;
    attack_damage = Math.floor(10 * multiplier);
};

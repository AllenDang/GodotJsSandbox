// Wave Shooter - FPS Player Controller
// WASD: Move, Mouse: Look, Left Click: Shoot, R: Reload, ESC: Release mouse

var camera = null;
var gun_model = null;
var muzzle_flash = null;
var muzzle_point = null;
var bullet_template = null;
var bullet_container = null;
var shoot_sound = null;

// Movement settings
var move_speed = 8.0;
var mouse_sensitivity = 0.002;
var gravity = 20.0;
var jump_force = 8.0;

// Camera rotation
var camera_rotation_x = 0;
var camera_rotation_y = 0;

// Velocity
var velocity = { x: 0, y: 0, z: 0 };

// Gun settings
var ammo = 30;
var max_ammo = 30;
var damage = 25;
var fire_rate = 0.1; // seconds between shots
var time_since_shot = 0;
var is_reloading = false;
var reload_time = 1.5;
var reload_timer = 0;

// Gun animation
var gun_bob_time = 0;
var gun_recoil = 0;
var gun_original_pos = { x: 0.07, y: -0.06, z: -0.12 };

// Game state (shared with game_manager)
var game_manager = null;

// Mouse captured state
var mouse_captured = true;

exports._ready = function() {
    console.log("=== Wave Shooter ===");
    console.log("WASD: Move | Mouse: Look | Click: Shoot | R: Reload | ESC: Menu");

    // Get child nodes
    camera = this.get_node("Camera3D");
    gun_model = this.get_node("Camera3D/GunModel");
    muzzle_flash = this.get_node("Camera3D/GunModel/MuzzleFlash");
    muzzle_point = this.get_node("Camera3D/MuzzlePoint");
    shoot_sound = this.get_node("Camera3D/ShootSound");

    // Get game manager reference (parent node)
    game_manager = this.get_parent();

    // Get bullet system references from game manager
    if (game_manager) {
        bullet_template = game_manager.get_node("BulletTemplate");
        bullet_container = game_manager.get_node("Bullets");
    }

    // Capture mouse
    Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);

    // Hide muzzle flash initially
    if (muzzle_flash) {
        muzzle_flash.visible = false;
    }
};

exports._input = function(event) {
    var event_class = event.get_class();

    // Handle mouse motion for camera look
    if (mouse_captured && event_class === "InputEventMouseMotion") {
        var relative = event.relative;

        // Rotate camera
        camera_rotation_y -= relative.x * mouse_sensitivity;
        camera_rotation_x -= relative.y * mouse_sensitivity;

        // Clamp vertical rotation
        camera_rotation_x = Math.max(-1.5, Math.min(1.5, camera_rotation_x));

        // Apply rotation to player (Y axis) and camera (X axis)
        this.rotation = { x: 0, y: camera_rotation_y, z: 0 };
        if (camera) {
            camera.rotation = { x: camera_rotation_x, y: 0, z: 0 };
        }
    }

    // Toggle mouse capture with ESC
    if (event_class === "InputEventKey") {
        if (event.keycode === Key.KEY_ESCAPE && event.pressed) {
            mouse_captured = !mouse_captured;
            if (mouse_captured) {
                Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);
            } else {
                Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
            }
        }
    }
};

exports._physics_process = function(delta) {
    if (!mouse_captured) return;

    // Update timers
    time_since_shot += delta;

    // Handle reload
    if (is_reloading) {
        reload_timer += delta;
        if (reload_timer >= reload_time) {
            ammo = max_ammo;
            is_reloading = false;
            reload_timer = 0;
            console.log("Reloaded!");
            if (game_manager && game_manager.update_ammo) {
                game_manager.update_ammo(ammo, max_ammo);
            }
        }
    }

    // Get input direction
    var input_dir = { x: 0, z: 0 };

    if (Input.is_action_pressed("move_forward")) {
        input_dir.z -= 1;
    }
    if (Input.is_action_pressed("move_back")) {
        input_dir.z += 1;
    }
    if (Input.is_action_pressed("move_left")) {
        input_dir.x -= 1;
    }
    if (Input.is_action_pressed("move_right")) {
        input_dir.x += 1;
    }

    // Normalize input
    var len = Math.sqrt(input_dir.x * input_dir.x + input_dir.z * input_dir.z);
    if (len > 0) {
        input_dir.x /= len;
        input_dir.z /= len;
    }

    // Transform direction relative to player rotation
    var sin_y = Math.sin(camera_rotation_y);
    var cos_y = Math.cos(camera_rotation_y);
    var move_dir = {
        x: input_dir.x * cos_y + input_dir.z * sin_y,
        z: -input_dir.x * sin_y + input_dir.z * cos_y
    };

    // Apply movement
    velocity.x = move_dir.x * move_speed;
    velocity.z = move_dir.z * move_speed;

    // Apply gravity
    if (!this.is_on_floor()) {
        velocity.y -= gravity * delta;
    } else {
        velocity.y = -0.1; // Small downward force to stay grounded

        // Jump
        if (Input.is_action_just_pressed("jump")) {
            velocity.y = jump_force;
        }
    }

    // Move the character
    this.velocity = velocity;
    this.move_and_slide();

    // Update gun bob animation
    if (len > 0 && this.is_on_floor()) {
        gun_bob_time += delta * 10;
    }

    // Shooting
    if (Input.is_action_pressed("shoot") && !is_reloading) {
        shoot();
    }

    // Reload
    if (Input.is_action_just_pressed("reload") && !is_reloading && ammo < max_ammo) {
        start_reload();
    }

    // Update gun position with bob and recoil
    update_gun_animation(delta);
};

function shoot() {
    if (time_since_shot < fire_rate) return;
    if (ammo <= 0) {
        // Auto reload when empty
        start_reload();
        return;
    }

    time_since_shot = 0;
    ammo--;
    gun_recoil = 0.05;

    // Play shoot sound
    if (shoot_sound) {
        shoot_sound.play();
    }

    // Update UI
    if (game_manager && game_manager.update_ammo) {
        game_manager.update_ammo(ammo, max_ammo);
    }

    // Show muzzle flash briefly
    if (muzzle_flash) {
        muzzle_flash.visible = true;
        // Will hide in update_gun_animation
    }

    // Spawn bullet projectile
    spawn_bullet();
}

function spawn_bullet() {
    if (!bullet_template || !bullet_container) {
        console.log("Error: bullet_template or bullet_container not found");
        return;
    }

    // Get spawn position from muzzle point BEFORE duplicating
    // (muzzle_point is in the tree, so global_position works)
    var spawn_pos;
    if (muzzle_point) {
        spawn_pos = {
            x: muzzle_point.global_position.x,
            y: muzzle_point.global_position.y,
            z: muzzle_point.global_position.z
        };
    } else if (camera) {
        spawn_pos = {
            x: camera.global_position.x,
            y: camera.global_position.y,
            z: camera.global_position.z
        };
    } else {
        var pos = this.global_position;
        spawn_pos = { x: pos.x, y: pos.y, z: pos.z };
    }

    // Calculate direction from camera rotation
    // Use the stored camera rotation values which we already track
    // Forward direction in world space based on player Y rotation and camera X rotation
    var sin_y = Math.sin(camera_rotation_y);
    var cos_y = Math.cos(camera_rotation_y);
    var sin_x = Math.sin(camera_rotation_x);
    var cos_x = Math.cos(camera_rotation_x);

    // Forward vector: looking down -Z in local space, rotated by camera angles
    var forward = {
        x: -sin_y * cos_x,
        y: sin_x,
        z: -cos_y * cos_x
    };

    // Duplicate the bullet template
    var bullet = bullet_template.duplicate();
    if (!bullet) {
        console.log("Error: failed to duplicate bullet template");
        return;
    }

    // Make visible before adding to tree
    bullet.visible = true;

    // Add to bullet container FIRST (so it's in the tree)
    bullet_container.add_child(bullet);

    // NOW set position (bullet is in tree, global_position works)
    bullet.global_position = spawn_pos;

    // Set bullet direction and damage (script is initialized after add_child)
    if (bullet.set_direction) {
        bullet.set_direction(forward);
    }
    if (bullet.set_damage) {
        bullet.set_damage(damage);
    }
}

function start_reload() {
    if (is_reloading) return;
    is_reloading = true;
    reload_timer = 0;
    console.log("Reloading...");
}

function update_gun_animation(delta) {
    if (!gun_model) return;

    // Calculate bob offset
    var bob_x = Math.sin(gun_bob_time) * 0.01;
    var bob_y = Math.abs(Math.cos(gun_bob_time)) * 0.01;

    // Decay recoil
    gun_recoil *= 0.85;

    // Apply position
    gun_model.position = {
        x: gun_original_pos.x + bob_x,
        y: gun_original_pos.y + bob_y - gun_recoil,
        z: gun_original_pos.z + gun_recoil * 2
    };

    // Hide muzzle flash after brief time
    if (muzzle_flash && muzzle_flash.visible && time_since_shot > 0.05) {
        muzzle_flash.visible = false;
    }
}

// Public method for game manager to get player position
exports.get_player_position = function() {
    return this.global_position;
};

// Take damage from enemies
exports.take_damage = function(amount) {
    if (game_manager && game_manager.player_hit) {
        game_manager.player_hit(amount);
    }
};

// Reset player state for game restart
exports.reset_state = function() {
    // Reset camera rotation
    camera_rotation_x = 0;
    camera_rotation_y = 0;

    if (camera) {
        camera.rotation = { x: 0, y: 0, z: 0 };
    }

    // Reset ammo
    ammo = max_ammo;
    is_reloading = false;
    reload_timer = 0;

    // Reset velocity
    velocity = { x: 0, y: 0, z: 0 };

    // Reset gun animation
    gun_bob_time = 0;
    gun_recoil = 0;
    time_since_shot = fire_rate;

    // Ensure mouse is captured
    mouse_captured = true;

    // Update ammo display
    if (game_manager && game_manager.update_ammo) {
        game_manager.update_ammo(ammo, max_ammo);
    }

    console.log("Player state reset");
};

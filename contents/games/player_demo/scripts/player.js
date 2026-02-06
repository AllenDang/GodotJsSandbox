// Third-Person Platform Jump Game - Reach the goal without falling!
// WASD to move (camera-relative), Space to jump, Mouse to look around

var speed = 6.0;
var jump_velocity = 10.0;
var gravity = 25.0;
var mouse_sensitivity = 0.003;

var spawn_position = null;
var spawn_rotation = 0;
var is_dead = false;
var death_y = -10.0;

// Camera rotation
var camera_rotation_x = 0;  // Vertical (pitch)
var camera_rotation_y = 0;  // Horizontal (yaw)
var min_pitch = -1.2;  // ~-70 degrees
var max_pitch = 1.0;   // ~57 degrees

// Node references
var camera_pivot = null;

exports._ready = function() {
    console.log("=== Third-Person Platform Game ===");
    console.log("Controls: WASD to move, Space to jump");
    console.log("Mouse to look around (click to capture)");
    console.log("Goal: Reach the green platform!");

    // Store spawn position and rotation for respawn
    spawn_position = {
        x: this.position.x,
        y: this.position.y,
        z: this.position.z
    };
    spawn_rotation = camera_rotation_y;

    // Get camera pivot node
    camera_pivot = this.get_node("CameraPivot");

    // Don't capture mouse immediately - wait for user click
};

exports._input = function(event) {
    if (is_dead) return;

    // Click to capture mouse (for first-time capture)
    if (event.type === 'InputEventMouseButton') {
        if (event.pressed && Input.get_mouse_mode() !== Input.MOUSE_MODE_CAPTURED) {
            Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);
        }
    }

    // Handle mouse motion for camera (only when captured)
    if (event.type === 'InputEventMouseMotion') {
        if (Input.get_mouse_mode() !== Input.MOUSE_MODE_CAPTURED) return;

        var relative = event.relative;

        // Horizontal rotation (yaw) - rotate the player body
        camera_rotation_y -= relative.x * mouse_sensitivity;

        // Vertical rotation (pitch) - rotate camera pivot only
        camera_rotation_x -= relative.y * mouse_sensitivity;
        camera_rotation_x = Math.max(min_pitch, Math.min(max_pitch, camera_rotation_x));

        // Apply rotations
        this.rotation = { x: 0, y: camera_rotation_y, z: 0 };
        if (camera_pivot) {
            camera_pivot.rotation = { x: camera_rotation_x, y: 0, z: 0 };
        }
    }

    // Tab key to release mouse
    if (event.type === 'InputEventKey') {
        if (event.keycode === 4194306 && event.pressed) {  // Tab key
            Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
        }
    }
};

exports._physics_process = function(delta) {
    if (is_dead) return;

    var vel = this.velocity;

    // Check for death (fell too far)
    if (this.position.y < death_y) {
        die.call(this);
        return;
    }

    // Apply gravity
    if (!this.is_on_floor()) {
        vel.y -= gravity * delta;
    }

    // Handle jump
    if (Input.is_action_just_pressed("jump") && this.is_on_floor()) {
        vel.y = jump_velocity;
    }

    // Get movement input
    var input_x = 0;
    var input_z = 0;

    if (Input.is_action_pressed("move_forward")) {
        input_z += 1;
    }
    if (Input.is_action_pressed("move_back")) {
        input_z -= 1;
    }
    if (Input.is_action_pressed("move_left")) {
        input_x -= 1;
    }
    if (Input.is_action_pressed("move_right")) {
        input_x += 1;
    }

    // Normalize diagonal movement
    var len = Math.sqrt(input_x * input_x + input_z * input_z);
    if (len > 0) {
        input_x /= len;
        input_z /= len;
    }

    // Transform input direction to be relative to camera/player facing
    // The player body rotates with the camera, so we use the player's transform
    var forward_x = -Math.sin(camera_rotation_y);
    var forward_z = -Math.cos(camera_rotation_y);
    var right_x = Math.cos(camera_rotation_y);
    var right_z = -Math.sin(camera_rotation_y);

    // Calculate world-space movement direction
    var move_x = (right_x * input_x + forward_x * input_z) * speed;
    var move_z = (right_z * input_x + forward_z * input_z) * speed;

    vel.x = move_x;
    vel.z = move_z;

    this.velocity = vel;
    this.move_and_slide();
};

function die() {
    is_dead = true;
    console.log("You fell! Respawning...");

    // Release mouse on death
    Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);

    var tree = this.get_tree();
    var timer = tree.create_timer(1.0);
    var self = this;

    timer.connect('timeout', function() {
        respawn.call(self);
    });
}

function respawn() {
    // Reset position to spawn
    this.position = spawn_position;
    this.velocity = { x: 0, y: 0, z: 0 };

    // Reset camera rotation
    camera_rotation_x = 0;
    camera_rotation_y = spawn_rotation;
    this.rotation = { x: 0, y: camera_rotation_y, z: 0 };
    if (camera_pivot) {
        camera_pivot.rotation = { x: 0, y: 0, z: 0 };
    }

    is_dead = false;

    // Re-capture mouse
    Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);
    console.log("Respawned! Try again!");
}

function win() {
    is_dead = true;
    console.log("=== YOU WIN! ===");
    console.log("Congratulations! Resetting level...");

    Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);

    var tree = this.get_tree();
    var timer = tree.create_timer(2.0);
    var self = this;

    timer.connect('timeout', function() {
        respawn.call(self);
    });
}

// Called when entering goal area
exports.on_goal_reached = function() {
    if (!is_dead) {
        win.call(this);
    }
};

exports._exit_tree = function() {
    // Release mouse when leaving scene
    Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
};

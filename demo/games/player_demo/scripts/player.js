// Platform Jump Game - Reach the goal without falling!
// WASD to move, Space to jump

var speed = 6.0;
var jump_velocity = 10.0;
var gravity = 25.0;

var spawn_position = null;
var is_dead = false;
var death_y = -10.0;  // Fall below this Y to die

exports._ready = function() {
    console.log("=== Platform Jump Game ===");
    console.log("Controls: WASD to move, Space to jump");
    console.log("Goal: Reach the green platform!");
    console.log("Don't fall off the platforms!");

    // Store spawn position for respawn
    spawn_position = {
        x: this.position.x,
        y: this.position.y,
        z: this.position.z
    };
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
        input_z -= 1;
    }
    if (Input.is_action_pressed("move_back")) {
        input_z += 1;
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

    // Apply movement
    vel.x = input_x * speed;
    vel.z = input_z * speed;

    this.velocity = vel;
    this.move_and_slide();
};

function die() {
    is_dead = true;
    console.log("You fell! Respawning...");

    // Use a timer to respawn after a short delay
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
    this.velocity = {x: 0, y: 0, z: 0};
    is_dead = false;
    console.log("Respawned! Try again!");
}

function win() {
    is_dead = true;  // Stop movement
    console.log("=== YOU WIN! ===");
    console.log("Congratulations! Resetting level...");

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

// Player Movement Demo - Shows CharacterBody3D physics
// WASD to move, Space to jump

var speed = 5.0;
var jump_velocity = 8.0;
var gravity = 20.0;

exports._ready = function() {
    console.log("Player Demo initialized!");
    console.log("Controls: WASD to move, Space to jump");
};

exports._physics_process = function(delta) {
    var vel = this.velocity;

    // Apply gravity
    if (!this.is_on_floor()) {
        vel.y -= gravity * delta;
    }

    // Handle jump
    if (Input.is_action_just_pressed("jump") && this.is_on_floor()) {
        vel.y = jump_velocity;
        console.log("Jump!");
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

// Wave Shooter - Game Manager
// Handles waves, spawning, scoring, and UI

var self = null; // Reference to this node
var player = null;
var ui_score = null;
var ui_wave = null;
var ui_health = null;
var ui_ammo = null;
var ui_message = null;
var enemy_template = null;

// Spawn points (set in _ready based on arena size)
var spawn_points = [];

// Game state
var score = 0;
var wave = 0;
var enemies_alive = 0;
var enemies_to_spawn = 0;
var player_health = 100;
var max_player_health = 100;
var game_over = false;
var wave_in_progress = false;

// Wave settings
var base_enemies = 3;
var enemies_per_wave = 2;
var spawn_delay = 1.5;
var time_since_spawn = 0;
var wave_start_delay = 3.0;
var wave_start_timer = 0;

// Enemy template reference
var enemy_container = null;

exports._ready = function() {
    console.log("=== Wave Shooter - Game Manager ===");

    // Store self reference for use in regular functions
    self = this;

    // Get references
    player = this.get_node("Player");
    enemy_container = this.get_node("Enemies");
    enemy_template = this.get_node("EnemyTemplate");

    // UI references
    ui_score = this.get_node("UI/ScoreLabel");
    ui_wave = this.get_node("UI/WaveLabel");
    ui_health = this.get_node("UI/HealthBar");
    ui_ammo = this.get_node("UI/AmmoLabel");
    ui_message = this.get_node("UI/MessageLabel");

    // Setup spawn points around the arena
    var arena_size = 15;
    spawn_points = [
        { x: arena_size, y: 1, z: 0 },
        { x: -arena_size, y: 1, z: 0 },
        { x: 0, y: 1, z: arena_size },
        { x: 0, y: 1, z: -arena_size },
        { x: arena_size * 0.7, y: 1, z: arena_size * 0.7 },
        { x: -arena_size * 0.7, y: 1, z: arena_size * 0.7 },
        { x: arena_size * 0.7, y: 1, z: -arena_size * 0.7 },
        { x: -arena_size * 0.7, y: 1, z: -arena_size * 0.7 }
    ];

    // Update UI
    update_ui();

    // Show start message
    show_message("Wave 1 starting...");
    wave_start_timer = wave_start_delay;
};

exports._input = function(event) {
    // Handle restart when game is over
    if (game_over) {
        var event_class = event.get_class();
        if (event_class === "InputEventKey" && event.pressed) {
            if (event.keycode === Key.KEY_R) {
                // Mark event as handled to prevent launcher from processing it
                self.get_viewport().set_input_as_handled();
                restart_game();
            }
        }
    }
};

exports._process = function(delta) {
    if (game_over) return;

    // Wave start countdown
    if (wave_start_timer > 0) {
        wave_start_timer -= delta;
        if (wave_start_timer <= 0) {
            start_wave();
        }
        return;
    }

    // Spawning during wave
    if (wave_in_progress && enemies_to_spawn > 0) {
        time_since_spawn += delta;
        if (time_since_spawn >= spawn_delay) {
            spawn_enemy();
            time_since_spawn = 0;
        }
    }

    // Check wave completion
    if (wave_in_progress && enemies_to_spawn === 0 && enemies_alive === 0) {
        wave_complete();
    }
};

function start_wave() {
    wave++;
    wave_in_progress = true;

    // Calculate enemies for this wave
    enemies_to_spawn = base_enemies + (wave - 1) * enemies_per_wave;
    enemies_alive = 0;
    time_since_spawn = spawn_delay; // Spawn first enemy immediately

    show_message("Wave " + wave + " - " + enemies_to_spawn + " enemies!");
    update_ui();

    console.log("Starting wave " + wave + " with " + enemies_to_spawn + " enemies");
}

function wave_complete() {
    wave_in_progress = false;

    // Bonus score for completing wave
    var wave_bonus = wave * 100;
    score += wave_bonus;

    show_message("Wave " + wave + " complete! +" + wave_bonus + " bonus");
    update_ui();

    // Start next wave after delay
    wave_start_timer = wave_start_delay;

    console.log("Wave " + wave + " complete!");
}

function spawn_enemy() {
    if (enemies_to_spawn <= 0) return;
    if (!enemy_template) {
        console.log("Error: enemy_template not found");
        return;
    }

    // Pick random spawn point
    var spawn_index = Math.floor(Math.random() * spawn_points.length);
    var spawn_pos = spawn_points[spawn_index];

    // Create enemy using the template in scene
    var enemy = enemy_template.duplicate();
    if (!enemy) {
        console.log("Error: failed to duplicate enemy template");
        return;
    }

    enemy.visible = true;

    // Ensure collision mask is set (layer 4 = world/floor)
    enemy.collision_mask = 4;

    // Enable collision shape
    var collision = enemy.get_node("CollisionShape3D");
    if (collision) {
        collision.disabled = false;
    }

    // Add to enemy container FIRST (so it's in tree)
    if (enemy_container) {
        enemy_container.add_child(enemy);
    } else if (self) {
        self.add_child(enemy);
    }

    // Set position AFTER adding to tree
    enemy.global_position = spawn_pos;

    // Set stats based on wave
    if (enemy.set_stats) {
        enemy.set_stats(wave);
    }

    enemies_alive++;
    enemies_to_spawn--;

    console.log("Spawned enemy at: " + spawn_pos.x + ", " + spawn_pos.z);
}

// Called by enemy when killed
exports.on_enemy_killed = function() {
    enemies_alive--;

    // Add score
    var kill_score = 50 + wave * 10;
    score += kill_score;

    update_ui();
};

// Called by player when shooting
exports.on_hit = function(hit_position) {
    // Could spawn hit particles here
};

// Called by enemy when player is hit
exports.player_hit = function(damage) {
    player_health -= damage;

    if (player_health <= 0) {
        player_health = 0;
        game_over_screen();
    }

    update_ui();

    // Screen shake or flash effect could go here
    console.log("Player hit! Health: " + player_health);
};

function game_over_screen() {
    game_over = true;
    show_message("GAME OVER - Score: " + score + " - Press R to restart");
    Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);

    // Disable player
    if (player) {
        player.set_physics_process(false);
    }

    // Pause all enemies using get_children() array proxy
    if (enemy_container) {
        var enemies = enemy_container.get_children();
        enemies.forEach(function(enemy) {
            enemy.set_physics_process(false);
        });
    }

    console.log("Game Over! Final score: " + score);
}

function restart_game() {
    console.log("Restarting game...");

    // Clear all enemies
    if (enemy_container) {
        var enemies = enemy_container.get_children();
        enemies.forEach(function(enemy) {
            enemy.queue_free();
        });
    }

    // Reset game state
    score = 0;
    wave = 0;
    enemies_alive = 0;
    enemies_to_spawn = 0;
    player_health = max_player_health;
    game_over = false;
    wave_in_progress = false;
    time_since_spawn = 0;
    wave_start_timer = wave_start_delay;

    // Reset player position and state
    if (player) {
        player.global_position = { x: 0, y: 1, z: 0 };
        player.rotation = { x: 0, y: 0, z: 0 };
        player.set_physics_process(true);

        // Reset player controller state if it has a reset method
        if (player.reset_state) {
            player.reset_state();
        }
    }

    // Recapture mouse
    Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);

    // Update UI
    update_ui();
    show_message("Wave 1 starting...");

    console.log("Game restarted!");
}

// Called by player controller to update ammo display
exports.update_ammo = function(current, max) {
    if (ui_ammo) {
        ui_ammo.text = "Ammo: " + current + "/" + max;
    }
};

function update_ui() {
    if (ui_score) {
        ui_score.text = "Score: " + score;
    }
    if (ui_wave) {
        ui_wave.text = "Wave: " + wave;
    }
    if (ui_health) {
        ui_health.value = player_health;
    }
}

function show_message(text) {
    if (ui_message) {
        ui_message.text = text;
        ui_message.visible = true;
    }
    console.log(text);
}

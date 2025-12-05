// Circuit Racer - Race Manager
// Handles lap counting, timing, and UI

var self = null;
var vehicle = null;

// UI references
var ui_speed = null;
var ui_lap = null;
var ui_time = null;
var ui_best = null;
var ui_message = null;

// Race state
var current_lap = 0;
var total_laps = 3;
var lap_time = 0;
var total_time = 0;
var best_lap_time = 0;
var race_started = false;
var race_finished = false;

// Checkpoint tracking
var checkpoints_passed = 0;
var total_checkpoints = 0;
var checkpoint_areas = [];

exports._ready = function() {
    console.log("=== Circuit Racer - Race Manager ===");

    self = this;

    // Get vehicle reference
    vehicle = this.get_node("Vehicle");

    // Get UI references
    ui_speed = this.get_node("UI/SpeedLabel");
    ui_lap = this.get_node("UI/LapLabel");
    ui_time = this.get_node("UI/TimeLabel");
    ui_best = this.get_node("UI/BestLabel");
    ui_message = this.get_node("UI/MessageLabel");

    // Find checkpoint areas
    var checkpoints_node = this.get_node("Checkpoints");
    if (checkpoints_node) {
        var children = checkpoints_node.get_children();
        total_checkpoints = children.length;
        console.log("Found " + total_checkpoints + " checkpoints");
    }

    // Initialize UI
    update_ui();
    show_message("Press W to start!");
};

exports._process = function(delta) {
    if (race_finished) return;

    // Start race when player accelerates
    if (!race_started) {
        if (Input.is_action_pressed("accelerate")) {
            start_race();
        }
        return;
    }

    // Update timers
    lap_time += delta;
    total_time += delta;

    // Update time display
    if (ui_time) {
        ui_time.text = "Time: " + format_time(lap_time);
    }
};

function start_race() {
    race_started = true;
    current_lap = 1;
    lap_time = 0;
    total_time = 0;
    checkpoints_passed = 0;

    show_message("GO!");
    update_ui();

    console.log("Race started!");
}

// Called when vehicle crosses finish line
exports.on_finish_line = function() {
    if (!race_started || race_finished) return;

    // Check if all checkpoints were passed
    if (checkpoints_passed < total_checkpoints) {
        show_message("Missed checkpoint!");
        console.log("Lap invalid - missed checkpoints: " + checkpoints_passed + "/" + total_checkpoints);
        return;
    }

    // Valid lap completion
    console.log("Lap " + current_lap + " complete: " + format_time(lap_time));

    // Update best lap time
    if (best_lap_time === 0 || lap_time < best_lap_time) {
        best_lap_time = lap_time;
        show_message("New best lap: " + format_time(best_lap_time));
    } else {
        show_message("Lap " + current_lap + ": " + format_time(lap_time));
    }

    // Check race completion
    if (current_lap >= total_laps) {
        finish_race();
        return;
    }

    // Next lap
    current_lap++;
    lap_time = 0;
    checkpoints_passed = 0;

    update_ui();
};

// Called when vehicle passes a checkpoint
exports.on_checkpoint = function(checkpoint_index) {
    if (!race_started || race_finished) return;

    // Simple sequential checkpoint tracking
    checkpoints_passed++;
    console.log("Checkpoint " + checkpoints_passed + "/" + total_checkpoints);
};

function finish_race() {
    race_finished = true;

    show_message("FINISHED! Total: " + format_time(total_time) + " | Best Lap: " + format_time(best_lap_time));

    console.log("Race finished! Total time: " + format_time(total_time));
    console.log("Best lap: " + format_time(best_lap_time));
}

// Called by vehicle controller to update speed display
exports.update_speed = function(speed_kmh) {
    if (ui_speed) {
        ui_speed.text = Math.floor(speed_kmh) + " km/h";
    }
};

function update_ui() {
    if (ui_lap) {
        ui_lap.text = "Lap: " + current_lap + "/" + total_laps;
    }
    if (ui_best && best_lap_time > 0) {
        ui_best.text = "Best: " + format_time(best_lap_time);
    }
}

function show_message(text) {
    if (ui_message) {
        ui_message.text = text;
        ui_message.visible = true;
    }
    console.log(text);
}

function format_time(seconds) {
    var mins = Math.floor(seconds / 60);
    var secs = seconds % 60;
    return mins + ":" + (secs < 10 ? "0" : "") + secs.toFixed(2);
}

// Reset race state
exports.reset_race = function() {
    race_started = false;
    race_finished = false;
    current_lap = 0;
    lap_time = 0;
    total_time = 0;
    checkpoints_passed = 0;

    update_ui();
    show_message("Press W to start!");
};

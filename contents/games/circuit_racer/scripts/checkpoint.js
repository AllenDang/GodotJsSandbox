// Checkpoint trigger - detects when vehicle passes through
// Uses Area3D body_entered signal

var self = null;
var checkpoint_index = 0;
var race_manager = null;
var is_finish_line = false;

exports._ready = function() {
    self = this;

    // Check if this is the finish line
    var name = this.name;
    is_finish_line = (name === "FinishLine");

    if (!is_finish_line) {
        // Get checkpoint index from node name (e.g., "Checkpoint1" -> 1)
        var match = name.match(/\d+/);
        if (match) {
            checkpoint_index = parseInt(match[0]);
        }
    }

    // Find race manager (grandparent - Checkpoints -> RaceManager)
    var parent = this.get_parent();
    if (parent) {
        race_manager = parent.get_parent();
    }

    // Connect body_entered signal
    this.connect("body_entered", exports.on_body_entered);

    if (is_finish_line) {
        console.log("Finish line ready");
    } else {
        console.log("Checkpoint " + checkpoint_index + " ready");
    }
};

// Called when a body enters the checkpoint area
exports.on_body_entered = function(body) {
    // Check if it's the vehicle (layer 2)
    if (body.collision_layer === 2) {
        if (race_manager) {
            if (is_finish_line) {
                if (race_manager.on_finish_line) {
                    race_manager.on_finish_line();
                }
                console.log("Vehicle crossed finish line!");
            } else {
                if (race_manager.on_checkpoint) {
                    race_manager.on_checkpoint(checkpoint_index);
                }
                console.log("Vehicle passed checkpoint " + checkpoint_index);
            }
        }
    }
};

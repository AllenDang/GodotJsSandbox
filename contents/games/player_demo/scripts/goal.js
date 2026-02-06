// Goal Area - Triggers win when player enters
// Uses body_entered signal to detect when player reaches the goal

var triggered = false;

exports._ready = function() {
    console.log("Goal area ready!");

    var self = this;

    // When player enters the goal area
    this.connect('body_entered', function(body) {
        if (triggered) return;

        // Filter to only CharacterBody3D (the player)
        // StaticBody3D is the platform beneath us
        if (!body || body.__class !== 'CharacterBody3D') {
            return;
        }

        // Call the player's on_goal_reached method (cross-script call)
        if (body.on_goal_reached) {
            triggered = true;
            body.on_goal_reached();
        }
    });

    // When player exits the goal area (after respawn), reset triggered state
    this.connect('body_exited', function(body) {
        if (!body || body.__class !== 'CharacterBody3D') {
            return;
        }
        // Reset so player can trigger win again after respawn
        triggered = false;
    });
};

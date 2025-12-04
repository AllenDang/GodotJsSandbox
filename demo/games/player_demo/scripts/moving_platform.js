// Moving Platform - Oscillates back and forth
// Attach to AnimatableBody3D nodes

exports._ready = function() {
    console.log("Moving platform ready: " + this.name);

    // Store instance data on this object
    this._move_axis = 'x';
    this._move_distance = 5.0;
    this._move_speed = 2.0;
    this._time = 0;

    // Store starting position
    this._start_position = {
        x: this.global_position.x,
        y: this.global_position.y,
        z: this.global_position.z
    };

    // Configure based on platform name for variety
    var name = this.name;

    if (name === 'MovingPlatform1') {
        // First platform: moves side to side (z axis)
        this._move_axis = 'z';
        this._move_distance = 4.0;
        this._move_speed = 1.5;
    } else if (name === 'MovingPlatform2') {
        // Second platform: moves up and down
        this._move_axis = 'y';
        this._move_distance = 3.0;
        this._move_speed = 1.0;
    } else if (name === 'MovingPlatform3') {
        // Third platform: moves forward/back faster
        this._move_axis = 'z';
        this._move_distance = 5.0;
        this._move_speed = 2.5;
    }

    console.log("Platform " + name + " will move on " + this._move_axis + " axis");
};

exports._physics_process = function(delta) {
    if (!this._start_position) return;

    this._time += delta * this._move_speed;

    // Calculate offset using sine wave for smooth oscillation
    var offset = Math.sin(this._time) * this._move_distance;

    // Create new position based on axis
    var new_pos = {
        x: this._start_position.x,
        y: this._start_position.y,
        z: this._start_position.z
    };

    if (this._move_axis === 'x') {
        new_pos.x = this._start_position.x + offset;
    } else if (this._move_axis === 'y') {
        new_pos.y = this._start_position.y + offset;
    } else if (this._move_axis === 'z') {
        new_pos.z = this._start_position.z + offset;
    }

    // Use global_position for AnimatableBody3D
    this.global_position = new_pos;
};

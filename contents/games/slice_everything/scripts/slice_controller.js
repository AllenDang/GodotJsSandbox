// Slice Controller - Handles mouse input and creates slice planes
// Attached to the Camera3D node

var slice_line = null;
var game_manager = null;

// Slice state
var is_slicing = false;
var slice_start = null;
var slice_end = null;
var slice_points = [];  // Array of {x, y} screen points during drag

// Camera orbit controls
var orbit_distance = 8.0;
var orbit_angle_y = 0;
var orbit_angle_x = 0.5;  // Radians, looking down
var orbit_center = { x: 0, y: 1, z: 0 };

// Right mouse for camera rotation
var is_orbiting = false;
var last_mouse_pos = null;

var self = null;

exports._ready = function() {
    self = this;

    // Get slice line for visual feedback (it's under UI CanvasLayer)
    slice_line = this.get_parent().get_node("UI/SliceLine");

    if (slice_line) {
        slice_line.clear_points();
    }

    // Get game manager (parent of camera)
    game_manager = this.get_parent();

    // Set initial camera position
    update_camera_position();

    console.log("Slice Controller ready!");
    console.log("Left-click and drag to slice objects");
};

exports._input = function(event) {
    var event_class = event.get_class();

    if (event_class === "InputEventMouseButton") {
        handle_mouse_button(event);
    } else if (event_class === "InputEventMouseMotion") {
        handle_mouse_motion(event);
    }
};

function handle_mouse_button(event) {
    var button = event.button_index;

    // Left mouse button - slicing
    if (button === 1) {
        if (event.pressed) {
            // Start slice
            is_slicing = true;
            slice_start = { x: event.position.x, y: event.position.y };
            slice_points = [slice_start];

            if (slice_line) {
                slice_line.clear_points();
                slice_line.add_point({ x: slice_start.x, y: slice_start.y });
            }
        } else {
            // End slice
            if (is_slicing && slice_points.length >= 2) {
                slice_end = { x: event.position.x, y: event.position.y };
                perform_slice();
            }
            is_slicing = false;
            slice_points = [];

            if (slice_line) {
                slice_line.clear_points();
            }
        }
    }

    // Right mouse button - camera orbit
    if (button === 2) {
        is_orbiting = event.pressed;
        if (event.pressed) {
            last_mouse_pos = { x: event.position.x, y: event.position.y };
            Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED);
        } else {
            Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
        }
    }

    // Mouse wheel - zoom
    if (button === 4) { // Wheel up
        orbit_distance = Math.max(3, orbit_distance - 1);
        update_camera_position();
    }
    if (button === 5) { // Wheel down
        orbit_distance = Math.min(20, orbit_distance + 1);
        update_camera_position();
    }
}

function handle_mouse_motion(event) {
    // Update slice line while dragging
    if (is_slicing) {
        var pos = event.position;
        var current_pos = { x: pos.x, y: pos.y };
        slice_points.push(current_pos);

        if (slice_line) {
            slice_line.add_point(current_pos);
        }
    }

    // Camera orbit
    if (is_orbiting) {
        var sensitivity = 0.005;
        var relative = event.relative;

        orbit_angle_y -= relative.x * sensitivity;
        orbit_angle_x -= relative.y * sensitivity;

        // Clamp vertical angle
        orbit_angle_x = Math.max(0.1, Math.min(1.4, orbit_angle_x));

        update_camera_position();
    }
}

function update_camera_position() {
    // Calculate camera position from orbit angles
    var sin_y = Math.sin(orbit_angle_y);
    var cos_y = Math.cos(orbit_angle_y);
    var sin_x = Math.sin(orbit_angle_x);
    var cos_x = Math.cos(orbit_angle_x);

    var cam_pos = {
        x: orbit_center.x + sin_y * cos_x * orbit_distance,
        y: orbit_center.y + sin_x * orbit_distance,
        z: orbit_center.z + cos_y * cos_x * orbit_distance
    };

    self.global_position = cam_pos;

    // Look at center
    self.look_at(orbit_center, { x: 0, y: 1, z: 0 });
}

function perform_slice() {
    if (!game_manager || !game_manager.perform_slice) {
        console.log("Error: game_manager.perform_slice not available");
        return;
    }

    // Use start and end points of the slice
    var start_2d = slice_start;
    var end_2d = slice_end;

    // Calculate distance of slice
    var dx = end_2d.x - start_2d.x;
    var dy = end_2d.y - start_2d.y;
    var slice_length = Math.sqrt(dx * dx + dy * dy);

    // Minimum slice length
    if (slice_length < 30) {
        console.log("Slice too short");
        return;
    }

    // Create slice plane from 2D line
    var plane = calculate_slice_plane(start_2d, end_2d);

    if (plane) {
        game_manager.perform_slice(plane.normal, plane.point, plane.direction);
    }
}

// Calculate 3D slice plane from 2D screen coordinates
function calculate_slice_plane(start_2d, end_2d) {
    // Project rays from camera through start and end points
    var start_origin = self.project_ray_origin({ x: start_2d.x, y: start_2d.y });
    var start_dir = self.project_ray_normal({ x: start_2d.x, y: start_2d.y });

    var end_origin = self.project_ray_origin({ x: end_2d.x, y: end_2d.y });
    var end_dir = self.project_ray_normal({ x: end_2d.x, y: end_2d.y });

    // Find a reasonable depth by averaging where rays might hit objects
    // For simplicity, use a fixed depth from camera
    var depth = 5.0;

    var start_3d = {
        x: start_origin.x + start_dir.x * depth,
        y: start_origin.y + start_dir.y * depth,
        z: start_origin.z + start_dir.z * depth
    };

    var end_3d = {
        x: end_origin.x + end_dir.x * depth,
        y: end_origin.y + end_dir.y * depth,
        z: end_origin.z + end_dir.z * depth
    };

    // Slice direction (along the drawn line in 3D space)
    var slice_dir = {
        x: end_3d.x - start_3d.x,
        y: end_3d.y - start_3d.y,
        z: end_3d.z - start_3d.z
    };

    var len = Math.sqrt(slice_dir.x * slice_dir.x + slice_dir.y * slice_dir.y + slice_dir.z * slice_dir.z);
    if (len < 0.001) return null;

    slice_dir.x /= len;
    slice_dir.y /= len;
    slice_dir.z /= len;

    // Camera forward direction (average of the two ray directions)
    var cam_forward = {
        x: (start_dir.x + end_dir.x) / 2,
        y: (start_dir.y + end_dir.y) / 2,
        z: (start_dir.z + end_dir.z) / 2
    };

    var cam_len = Math.sqrt(cam_forward.x * cam_forward.x + cam_forward.y * cam_forward.y + cam_forward.z * cam_forward.z);
    cam_forward.x /= cam_len;
    cam_forward.y /= cam_len;
    cam_forward.z /= cam_len;

    // The slice plane should cut ALONG the drawn line direction.
    // The plane normal should be perpendicular to the slice direction,
    // but it should lie in the "screen plane" (perpendicular to camera forward).
    //
    // To achieve this:
    // 1. The plane contains the slice_dir vector (the drawn line)
    // 2. The plane contains the cam_forward vector (extends into the scene)
    // 3. So the normal is: cross(cam_forward, slice_dir)
    //
    // This makes the plane "stand up" from the screen along the drawn line.
    var plane_normal = {
        x: cam_forward.y * slice_dir.z - cam_forward.z * slice_dir.y,
        y: cam_forward.z * slice_dir.x - cam_forward.x * slice_dir.z,
        z: cam_forward.x * slice_dir.y - cam_forward.y * slice_dir.x
    };

    var normal_len = Math.sqrt(plane_normal.x * plane_normal.x + plane_normal.y * plane_normal.y + plane_normal.z * plane_normal.z);
    if (normal_len < 0.001) return null;

    plane_normal.x /= normal_len;
    plane_normal.y /= normal_len;
    plane_normal.z /= normal_len;

    // Plane point is midpoint between start and end
    var plane_point = {
        x: (start_3d.x + end_3d.x) / 2,
        y: (start_3d.y + end_3d.y) / 2,
        z: (start_3d.z + end_3d.z) / 2
    };

    return {
        normal: plane_normal,
        point: plane_point,
        direction: slice_dir
    };
}

exports._process = function(delta) {
    // Handle keyboard input for camera reset
    if (Input.is_action_just_pressed("reset_camera")) {
        orbit_angle_y = 0;
        orbit_angle_x = 0.5;
        orbit_distance = 8.0;
        update_camera_position();
        console.log("Camera reset");
    }
};

// Public method to get camera for raycasting
exports.get_camera = function() {
    return self;
};

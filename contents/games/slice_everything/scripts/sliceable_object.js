// Sliceable Object - Behavior script for objects that can be sliced
// Attached to RigidBody3D nodes

var mesh_instance = null;
var material = null;
var current_color = { r: 0.8, g: 0.4, b: 0.2 };

var self = null;

exports._ready = function() {
    self = this;

    // Find mesh instance child
    mesh_instance = this.get_node("Mesh");
    if (!mesh_instance) {
        var children = this.get_children();
        for (var i = 0; i < children.length; i++) {
            if (children[i].__class === "MeshInstance3D") {
                mesh_instance = children[i];
                break;
            }
        }
    }

    // Get or create material
    if (mesh_instance) {
        material = mesh_instance.get_surface_override_material(0);
        if (!material) {
            material = new StandardMaterial3D();
            material.albedo_color = new Color(current_color.r, current_color.g, current_color.b, 1.0);
            mesh_instance.set_surface_override_material(0, material);
        }
    }
};

exports._physics_process = function(delta) {
    // Optional: Add some visual effect or behavior
    // For now, just let physics handle it
};

// Set the color of this object
exports.set_color = function(color) {
    current_color = color;

    if (mesh_instance) {
        if (!material) {
            material = new StandardMaterial3D();
            mesh_instance.set_surface_override_material(0, material);
        }
        material.albedo_color = new Color(color.r, color.g, color.b, 1.0);
    }
};

// Get the current color
exports.get_color = function() {
    return current_color;
};

// Get mesh data for slicing
exports.get_mesh_data = function() {
    if (!mesh_instance || !mesh_instance.mesh) {
        return null;
    }

    var mesh = mesh_instance.mesh;

    // Check if surface_get_arrays exists
    if (typeof mesh.surface_get_arrays !== "function") {
        console.log("surface_get_arrays not available on mesh");
        return null;
    }

    var surface_count = mesh.get_surface_count();
    if (surface_count === 0) {
        return null;
    }

    var arrays = mesh.surface_get_arrays(0);
    if (!arrays) {
        console.log("surface_get_arrays returned null");
        return null;
    }

    // Debug: check what we got
    console.log("arrays type: " + typeof arrays);
    console.log("arrays length: " + (arrays.length || "no length"));

    // Try to access vertices - arrays[0] is ARRAY_VERTEX
    var vertices_packed = null;
    var indices_packed = null;

    if (typeof arrays.get === "function") {
        // It might be a Godot Array, not a JS array
        vertices_packed = arrays.get(0);
        indices_packed = arrays.get(12);
    } else if (Array.isArray(arrays) || arrays.length !== undefined) {
        vertices_packed = arrays[0];
        indices_packed = arrays[12];
    } else {
        console.log("Cannot access arrays data");
        return null;
    }

    if (!vertices_packed) {
        console.log("No vertices in mesh arrays");
        return null;
    }

    var vertices = [];
    var vert_len = vertices_packed.length || (vertices_packed.size ? vertices_packed.size() : 0);

    for (var i = 0; i < vert_len; i++) {
        var v;
        if (typeof vertices_packed.get === "function") {
            v = vertices_packed.get(i);
        } else {
            v = vertices_packed[i];
        }
        if (v) {
            vertices.push({ x: v.x, y: v.y, z: v.z });
        }
    }

    var indices = [];
    if (indices_packed) {
        var idx_len = indices_packed.length || (indices_packed.size ? indices_packed.size() : 0);
        for (var j = 0; j < idx_len; j++) {
            var idx;
            if (typeof indices_packed.get === "function") {
                idx = indices_packed.get(j);
            } else {
                idx = indices_packed[j];
            }
            indices.push(idx);
        }
    } else {
        // No index array - create sequential indices
        for (var k = 0; k < vertices.length; k++) {
            indices.push(k);
        }
    }

    console.log("Extracted " + vertices.length + " vertices, " + indices.length + " indices");
    return { vertices: vertices, indices: indices };
};

// Slice Everything - Game Manager
// Handles object spawning, slicing orchestration, and scoring

import * as ComputeSlicer from "./compute_slicer.js";

// Compute slicer state
let compute_slicer_ready = false;

// Node references
let sliceable_container = null;
let sliced_pieces_container = null;
let sliceable_template = null;
let score_label = null;
let self = null;

// Loaded GLB scene and material
let loaded_glb_scene = null;
let original_material = null;

// Stats
let slice_count = 0;
let piece_count = 0;

// Colors for sliced pieces (fallback if no material)
const colors = [
    { r: 0.8, g: 0.4, b: 0.2 },  // Orange
    { r: 0.2, g: 0.6, b: 0.8 },  // Blue
    { r: 0.6, g: 0.8, b: 0.2 },  // Green
    { r: 0.8, g: 0.2, b: 0.6 },  // Pink
    { r: 0.8, g: 0.8, b: 0.2 },  // Yellow
    { r: 0.6, g: 0.2, b: 0.8 },  // Purple
];
let color_index = 0;

export function _ready() {
    self = this;
    console.log("=== SLICE EVERYTHING ===");
    console.log("Hold left mouse and drag to slice!");
    console.log("Press SPACE to spawn objects");
    console.log("Right-click drag to orbit camera");
    console.log("Mouse wheel to zoom");

    // Get containers
    sliceable_container = this.get_node("SliceableObjects");
    sliced_pieces_container = this.get_node("SlicedPieces");
    sliceable_template = this.get_node("SliceableTemplate");
    score_label = this.get_node("UI/ScoreLabel");

    // Load GLB model from user:// directory
    load_glb_model();

    // Initialize compute slicer (GPU-accelerated)
    compute_slicer_ready = ComputeSlicer.init();
    if (compute_slicer_ready) {
        console.log("Compute shader slicer ready (GPU-accelerated)");
    } else {
        console.log("ERROR: Compute shader slicer failed to initialize");
    }

    // Spawn initial objects
    spawn_object({ x: 0, y: 2, z: 0 });
    spawn_object({ x: -2, y: 2.5, z: 1 });
    spawn_object({ x: 2, y: 3, z: -1 });

    update_score();
}

// Load GLB model from assets folder
function load_glb_model() {
    const glb_path = "user://games/slice_everything/assets/model.glb";

    console.log("Loading GLB from: " + glb_path);

    // Load the GLB file using the global load() function
    const glb_resource = load(glb_path);

    if (!glb_resource) {
        console.log("Failed to load GLB from " + glb_path);
        return;
    }

    console.log("GLB loaded successfully!");

    // Instantiate the scene to access its contents
    loaded_glb_scene = glb_resource.instantiate();

    if (!loaded_glb_scene) {
        console.log("Failed to instantiate GLB scene");
        return;
    }

    // Find mesh instance and extract material
    const mesh_instance = find_mesh_instance(loaded_glb_scene);
    if (mesh_instance) {
        // Try to get material from the mesh instance
        original_material = mesh_instance.get_surface_override_material(0);
        if (!original_material && mesh_instance.mesh) {
            // Try to get material from mesh itself
            original_material = mesh_instance.mesh.surface_get_material(0);
        }

        if (original_material) {
            console.log("Original material extracted from GLB");
        } else {
            console.log("No material found in GLB, will use default colors");
        }
    }

    console.log("GLB model ready for spawning");
}

// Find MeshInstance3D in a node tree
function find_mesh_instance(node) {
    if (node.__class === "MeshInstance3D") {
        return node;
    }

    const children = node.get_children();
    for (let i = 0; i < children.length; i++) {
        const result = find_mesh_instance(children[i]);
        if (result) {
            return result;
        }
    }

    return null;
}

export function _process(delta) {
    // Spawn new object on space
    if (Input.is_action_just_pressed("spawn")) {
        const pos = {
            x: (Math.random() - 0.5) * 4,
            y: 2 + Math.random() * 2,
            z: (Math.random() - 0.5) * 4
        };
        spawn_object(pos);
        console.log("Spawned new object!");
    }

    // Clean up pieces that fell too far
    cleanup_fallen_pieces();
}

function spawn_object(position) {
    if (!sliceable_container) {
        console.log("Error: container not found");
        return null;
    }

    let obj = null;

    // Try to use loaded GLB model first
    if (loaded_glb_scene) {
        obj = loaded_glb_scene.duplicate();
        obj.visible = true;

        // Ensure it's a RigidBody3D or wrap it
        if (obj.__class !== "RigidBody3D") {
            // Create a RigidBody3D to hold the model
            const rigid_body = new RigidBody3D();

            // Find and duplicate the mesh instance from the GLB
            const mesh_instance = find_mesh_instance(obj);
            if (mesh_instance) {
                const new_mesh_instance = mesh_instance.duplicate();
                new_mesh_instance.name = "Mesh";
                rigid_body.add_child(new_mesh_instance);

                // Create collision shape from mesh
                if (new_mesh_instance.mesh) {
                    const collision = new CollisionShape3D();
                    const shape = new_mesh_instance.mesh.create_convex_shape();
                    if (shape) {
                        collision.shape = shape;
                        rigid_body.add_child(collision);
                    }
                }
            }

            obj.queue_free();
            obj = rigid_body;
        }
    } else if (sliceable_template) {
        // Fallback to template
        obj = sliceable_template.duplicate();
        obj.visible = true;

        // Set color for fallback objects
        const color = colors[color_index % colors.length];
        color_index++;
        if (obj.set_color) {
            obj.set_color(color);
        }
    } else {
        console.log("Error: no GLB loaded and no template available");
        return null;
    }

    // Add to container first
    sliceable_container.add_child(obj);

    // Then set position
    obj.global_position = position;

    // Give it a random rotation
    obj.rotation = {
        x: Math.random() * 0.5,
        y: Math.random() * 6.28,
        z: Math.random() * 0.5
    };

    // Random scale
    const scale = 0.8 + Math.random() * 0.6;
    obj.scale = { x: scale, y: scale, z: scale };

    piece_count++;
    update_score();

    return obj;
}

// Called by slice_controller when a slice is performed
// Only slices ONE object per call to stay within sandbox heavy op limits
export function perform_slice(plane_normal, plane_point, slice_direction) {
    const t_start = Time.get_ticks_usec();

    // First try sliceable objects container
    const children = sliceable_container.get_children();

    for (let i = 0; i < children.length; i++) {
        const obj = children[i];

        // Check if slice plane intersects this object's bounding area
        if (should_attempt_slice(obj, plane_normal, plane_point)) {
            const result = try_slice_object(obj, plane_normal, plane_point, slice_direction);
            if (result) {
                slice_count++;
                update_score();
                const t_total = Time.get_ticks_usec() - t_start;
                console.log("[PERF] perform_slice total: " + (t_total / 1000).toFixed(2) + "ms");
                return; // Only slice ONE object per frame
            }
        }
    }

    // Then try sliced pieces container
    const pieces = sliced_pieces_container.get_children();
    for (let j = 0; j < pieces.length; j++) {
        const piece = pieces[j];
        if (should_attempt_slice(piece, plane_normal, plane_point)) {
            const piece_result = try_slice_object(piece, plane_normal, plane_point, slice_direction);
            if (piece_result) {
                slice_count++;
                update_score();
                return; // Only slice ONE object per frame
            }
        }
    }
}

function should_attempt_slice(obj, plane_normal, plane_point) {
    // Simple distance check - is the plane close enough to the object?
    const obj_pos = obj.global_position;
    const obj_scale = obj.scale;
    const radius = Math.max(obj_scale.x, Math.max(obj_scale.y, obj_scale.z)) * 1.5;

    // Distance from object center to plane
    const diff = {
        x: obj_pos.x - plane_point.x,
        y: obj_pos.y - plane_point.y,
        z: obj_pos.z - plane_point.z
    };

    const distance = Math.abs(
        diff.x * plane_normal.x +
        diff.y * plane_normal.y +
        diff.z * plane_normal.z
    );

    return distance < radius;
}

function try_slice_object(obj, plane_normal, plane_point, slice_direction) {
    const perf = {};
    let t0 = Time.get_ticks_usec();

    if (!obj) {
        return false;
    }

    // Get mesh data from the object
    let mesh_instance = obj.get_node("Mesh");
    if (!mesh_instance) {
        // Try finding MeshInstance3D child
        const children = obj.get_children();
        for (let c = 0; c < children.length; c++) {
            if (children[c].__class === "MeshInstance3D") {
                mesh_instance = children[c];
                break;
            }
        }
    }

    if (!mesh_instance) {
        return false;
    }

    const mesh = mesh_instance.mesh;
    if (!mesh) {
        return false;
    }

    // Transform plane to local space
    const obj_transform = obj.global_transform;
    const obj_pos = obj.global_position;

    // Transform plane point to local space
    let local_plane_point = {
        x: plane_point.x - obj_pos.x,
        y: plane_point.y - obj_pos.y,
        z: plane_point.z - obj_pos.z
    };

    // Transform by inverse basis (approximate - assumes orthonormal)
    const scale = obj.scale;
    local_plane_point = {
        x: local_plane_point.x / scale.x,
        y: local_plane_point.y / scale.y,
        z: local_plane_point.z / scale.z
    };

    perf.setup = Time.get_ticks_usec() - t0;
    t0 = Time.get_ticks_usec();

    // Get mesh data (including UVs and material)
    const mesh_data = extract_mesh_data(mesh, mesh_instance);
    if (!mesh_data) {
        return false;
    }

    perf.extract = Time.get_ticks_usec() - t0;
    t0 = Time.get_ticks_usec();

    // Check if compute slicer is ready
    if (!compute_slicer_ready) {
        return false;
    }

    // Perform GPU-accelerated slicing via compute shader
    const slice_result = ComputeSlicer.slice(
        mesh_data.vertices,
        mesh_data.indices,
        mesh_data.normals,
        mesh_data.uvs,
        plane_normal,
        local_plane_point
    );

    perf.gpu_slice = Time.get_ticks_usec() - t0;
    t0 = Time.get_ticks_usec();

    if (!slice_result.success) {
        return false;
    }

    // Generate caps for the cut surfaces
    add_cap_to_mesh(slice_result.front, plane_normal, -1);  // Front mesh: cap faces backward
    add_cap_to_mesh(slice_result.back, plane_normal, 1);    // Back mesh: cap faces forward

    perf.add_caps = Time.get_ticks_usec() - t0;
    t0 = Time.get_ticks_usec();

    // Get current object properties
    let current_color = null;
    if (obj.get_color) {
        current_color = obj.get_color();
    }

    // Create two new pieces - offset them slightly along the plane normal to separate
    const offset = 0.05;  // Small offset to prevent overlap
    const front_pos = {
        x: obj_pos.x + plane_normal.x * offset,
        y: obj_pos.y + plane_normal.y * offset,
        z: obj_pos.z + plane_normal.z * offset
    };
    const back_pos = {
        x: obj_pos.x - plane_normal.x * offset,
        y: obj_pos.y - plane_normal.y * offset,
        z: obj_pos.z - plane_normal.z * offset
    };

    // Pass material from mesh_data to preserve original textures
    create_sliced_piece(slice_result.front, front_pos, obj.rotation, scale, current_color, plane_normal, 1, mesh_data.material);
    create_sliced_piece(slice_result.back, back_pos, obj.rotation, scale, current_color, plane_normal, -1, mesh_data.material);

    perf.create_pieces = Time.get_ticks_usec() - t0;

    // Remove original object
    obj.queue_free();
    piece_count--; // Will be +2 from new pieces

    // Log performance breakdown
    console.log("[PERF] try_slice_object breakdown (ms): setup=" + (perf.setup/1000).toFixed(2) +
        " extract=" + (perf.extract/1000).toFixed(2) +
        " gpu_slice=" + (perf.gpu_slice/1000).toFixed(2) +
        " add_caps=" + (perf.add_caps/1000).toFixed(2) +
        " create_pieces=" + (perf.create_pieces/1000).toFixed(2));

    return true;
}

// Generate cap for cut surface by finding boundary edges and triangulating them
// The cap is added directly to the mesh_data arrays
// OPTIMIZED: Uses toArray() for bulk conversion, plain objects instead of Map/Set
function add_cap_to_mesh(mesh_data, plane_normal, normal_direction) {
    const vertices = mesh_data.vertices;
    const normals = mesh_data.normals;
    const uvs = mesh_data.uvs;
    const indices = mesh_data.indices;

    if (!vertices || vertices.length < 3) {
        return;
    }

    // Convert packed arrays to native JS arrays in a single C++ call
    const vertices_arr = vertices.toArray ? vertices.toArray() : vertices;
    const indices_arr = indices.toArray ? indices.toArray() : indices;

    // Find boundary edges - use simple count tracking
    const edge_count = {};  // Plain object is faster than Map in QuickJS
    const idx_count = indices_arr.length;

    // Build edge counts
    for (let i = 0; i < idx_count; i += 3) {
        const i0 = indices_arr[i];
        const i1 = indices_arr[i + 1];
        const i2 = indices_arr[i + 2];

        // Add edges (smaller index first for consistent key)
        const k1 = i0 < i1 ? i0 + "," + i1 : i1 + "," + i0;
        const k2 = i1 < i2 ? i1 + "," + i2 : i2 + "," + i1;
        const k3 = i2 < i0 ? i2 + "," + i0 : i0 + "," + i2;

        edge_count[k1] = (edge_count[k1] || 0) + 1;
        edge_count[k2] = (edge_count[k2] || 0) + 1;
        edge_count[k3] = (edge_count[k3] || 0) + 1;
    }

    // Find boundary edges (count == 1)
    const boundary_edges = [];
    const keys = Object.keys(edge_count);
    for (let i = 0; i < keys.length; i++) {
        if (edge_count[keys[i]] === 1) {
            const parts = keys[i].split(",");
            boundary_edges.push([parseInt(parts[0]), parseInt(parts[1])]);
        }
    }

    if (boundary_edges.length < 3) {
        return; // Not enough boundary edges for a cap
    }

    // Build boundary loop by connecting edges
    const boundary_loop = build_boundary_loop(boundary_edges);
    if (!boundary_loop || boundary_loop.length < 3) {
        return;
    }

    // Get unique boundary vertex positions
    const boundary_positions = [];
    for (let i = 0; i < boundary_loop.length; i++) {
        boundary_positions.push(vertices_arr[boundary_loop[i]]);
    }

    // Compute centroid of boundary
    let cx = 0, cy = 0, cz = 0;
    for (const v of boundary_positions) {
        cx += v.x;
        cy += v.y;
        cz += v.z;
    }
    cx /= boundary_positions.length;
    cy /= boundary_positions.length;
    cz /= boundary_positions.length;
    const centroid = { x: cx, y: cy, z: cz };

    // Cap normal
    const cap_normal = {
        x: plane_normal.x * normal_direction,
        y: plane_normal.y * normal_direction,
        z: plane_normal.z * normal_direction
    };

    // Build cap geometry in native JS arrays first (avoid per-element proxy calls)
    const n = boundary_positions.length;
    const cap_vertices = new Array(1 + n);  // centroid + boundary verts
    const cap_normals = new Array(1 + n);
    const cap_uvs = new Array(1 + n);
    const cap_indices = new Array(n * 3);   // n triangles, 3 indices each

    // Centroid vertex
    cap_vertices[0] = centroid;
    cap_normals[0] = cap_normal;
    cap_uvs[0] = { x: 0.5, y: 0.5 };

    // Boundary vertices
    for (let i = 0; i < n; i++) {
        const v = boundary_positions[i];
        cap_vertices[1 + i] = { x: v.x, y: v.y, z: v.z };
        cap_normals[1 + i] = cap_normal;
        cap_uvs[1 + i] = { x: 0.5, y: 0.5 };
    }

    // Triangle fan indices (offset by current vertex count)
    const base_idx = vertices.length;
    const centroid_idx = base_idx;
    const first_cap_idx = base_idx + 1;

    for (let i = 0; i < n; i++) {
        const i0 = centroid_idx;
        const i1 = first_cap_idx + i;
        const i2 = first_cap_idx + ((i + 1) % n);

        if (normal_direction > 0) {
            cap_indices[i * 3] = i0;
            cap_indices[i * 3 + 1] = i2;
            cap_indices[i * 3 + 2] = i1;
        } else {
            cap_indices[i * 3] = i0;
            cap_indices[i * 3 + 1] = i1;
            cap_indices[i * 3 + 2] = i2;
        }
    }

    // Bulk append to packed arrays (single C++ call per array)
    if (vertices.append_array) {
        vertices.append_array(cap_vertices);
        normals.append_array(cap_normals);
        uvs.append_array(cap_uvs);
        indices.append_array(cap_indices);
    } else {
        // Fallback for non-packed arrays
        for (const v of cap_vertices) vertices.push(v);
        for (const n of cap_normals) normals.push(n);
        for (const uv of cap_uvs) uvs.push(uv);
        for (const idx of cap_indices) indices.push(idx);
    }
}

function add_edge(edge_map, a, b) {
    // Use consistent key ordering
    const key = a < b ? a + "," + b : b + "," + a;
    edge_map.set(key, (edge_map.get(key) || 0) + 1);
}

function build_boundary_loop(edges) {
    if (edges.length === 0) return null;

    // Build adjacency using plain object (much faster than Map in QuickJS)
    const adj = {};
    for (let i = 0; i < edges.length; i++) {
        const a = edges[i][0];
        const b = edges[i][1];
        if (!adj[a]) adj[a] = [];
        if (!adj[b]) adj[b] = [];
        adj[a].push(b);
        adj[b].push(a);
    }

    // Start from first edge - use plain object for visited (faster than Set)
    const loop = [edges[0][0]];
    const visited = {};
    visited[edges[0][0]] = true;
    let current = edges[0][0];

    while (true) {
        const neighbors = adj[current];
        if (!neighbors) break;

        let found = false;
        for (let i = 0; i < neighbors.length; i++) {
            const next = neighbors[i];
            if (!visited[next]) {
                loop.push(next);
                visited[next] = true;
                current = next;
                found = true;
                break;
            }
        }

        if (!found) break;
        if (loop.length > edges.length + 1) break; // Prevent infinite loop
    }

    return loop;
}

function extract_mesh_data(mesh, mesh_instance) {
    // For built-in meshes like BoxMesh, we need to get the surface arrays

    // Check if surface_get_arrays exists
    if (typeof mesh.surface_get_arrays !== "function") {
        console.log("surface_get_arrays not available on mesh");
        return null;
    }

    const surface_count = mesh.get_surface_count();
    if (surface_count === 0) {
        console.log("Mesh has no surfaces");
        return null;
    }

    // Get arrays from first surface
    const arrays = mesh.surface_get_arrays(0);
    if (!arrays) {
        console.log("Failed to get surface arrays");
        return null;
    }

    // Godot mesh arrays:
    // Index 0 = ARRAY_VERTEX (PackedVector3Array)
    // Index 1 = ARRAY_NORMAL (PackedVector3Array)
    // Index 4 = ARRAY_TEX_UV (PackedVector2Array)
    // Index 12 = ARRAY_INDEX (PackedInt32Array)
    const vertices_arr = arrays[0];
    const normals_arr = arrays[1];
    const uvs_arr = arrays[4];
    const indices_arr = arrays[12];

    if (!vertices_arr || !vertices_arr.length) {
        console.log("No vertices in mesh");
        return null;
    }

    // Pass PackedArrays directly - C++ bulk functions handle the encoding
    // This avoids the slow JS iteration through 30K+ vertices
    const vertices = vertices_arr;  // PackedVector3Array
    const normals = normals_arr || vertices_arr;  // Use vertices as fallback (will generate default normals)
    const uvs = uvs_arr;  // PackedVector2Array (may be null)
    const indices = indices_arr;  // PackedInt32Array (may be null)

    // Extract material from mesh instance
    let material = null;
    if (mesh_instance) {
        material = mesh_instance.get_surface_override_material(0);
        if (!material) {
            material = mesh.surface_get_material(0);
        }
    }
    // Fallback to original_material from GLB
    if (!material && original_material) {
        material = original_material;
    }

    return { vertices, indices, uvs, normals, material, is_packed: true };
}

function create_sliced_piece(mesh_data, position, rotation, scale, color, slice_dir, side, source_material) {
    // Create new RigidBody3D
    const piece = new RigidBody3D();

    // Create MeshInstance3D with generated mesh
    const mesh_instance = new MeshInstance3D();
    mesh_instance.name = "Mesh";
    const new_mesh = create_array_mesh(mesh_data);
    mesh_instance.mesh = new_mesh;

    // Apply material - preserve original if available
    let material = null;

    if (source_material) {
        // Duplicate the original material to preserve textures
        material = source_material.duplicate();
        // Make it double-sided for sliced pieces
        if (material.cull_mode !== undefined) {
            material.cull_mode = 0; // CULL_DISABLED
        }
    } else {
        // Fallback to colored material
        material = new StandardMaterial3D();
        if (color) {
            material.albedo_color = new Color(color.r, color.g, color.b, 1.0);
        } else {
            const c = colors[color_index % colors.length];
            color_index++;
            material.albedo_color = new Color(c.r, c.g, c.b, 1.0);
        }
        // Make material double-sided to see mesh even if normals are wrong
        material.cull_mode = 0; // CULL_DISABLED
    }
    mesh_instance.set_surface_override_material(0, material);

    piece.add_child(mesh_instance);

    // Create collision shape (use convex hull)
    const collision = new CollisionShape3D();
    const shape = new_mesh.create_convex_shape();
    if (shape) {
        collision.shape = shape;
        piece.add_child(collision);
    }

    // Add to sliced pieces container
    sliced_pieces_container.add_child(piece);

    // Set transform
    piece.global_position = position;
    piece.rotation = rotation;
    piece.scale = scale;

    // Apply impulse to separate pieces
    const impulse_strength = 2.0;
    const impulse = {
        x: slice_dir.x * side * impulse_strength + (Math.random() - 0.5),
        y: 1.0 + Math.random(),
        z: slice_dir.z * side * impulse_strength + (Math.random() - 0.5)
    };
    piece.apply_central_impulse(impulse);

    // Add some rotation
    piece.apply_torque_impulse({
        x: (Math.random() - 0.5) * 2,
        y: (Math.random() - 0.5) * 2,
        z: (Math.random() - 0.5) * 2
    });

    piece_count++;

    return piece;
}

function create_array_mesh(mesh_data) {
    const array_mesh = new ArrayMesh();

    // Use existing PackedArray proxy if available, otherwise create new
    const packed_verts = mesh_data.vertices.__is_packed_array
        ? mesh_data.vertices
        : new PackedVector3Array(mesh_data.vertices);

    // Use preserved normals if available, otherwise calculate them
    let packed_normals;
    if (mesh_data.normals && mesh_data.normals.length === mesh_data.vertices.length) {
        packed_normals = mesh_data.normals.__is_packed_array
            ? mesh_data.normals
            : new PackedVector3Array(mesh_data.normals);
    } else {
        // Fallback: Calculate normals (for legacy meshes without normals)
        const normals = calculate_normals(mesh_data.vertices, mesh_data.indices);
        packed_normals = new PackedVector3Array(normals);
    }

    // Use existing PackedArray proxy if available, otherwise create new
    const packed_indices = mesh_data.indices.__is_packed_array
        ? mesh_data.indices
        : new PackedInt32Array(mesh_data.indices);

    // Convert UVs to PackedVector2Array if available
    let packed_uvs = null;
    if (mesh_data.uvs && mesh_data.uvs.length > 0) {
        packed_uvs = mesh_data.uvs.__is_packed_array
            ? mesh_data.uvs
            : new PackedVector2Array(mesh_data.uvs);
    }

    // Create surface arrays - Godot expects Array with 13 elements
    const arrays = [
        packed_verts,    // 0: ARRAY_VERTEX
        packed_normals,  // 1: ARRAY_NORMAL
        null,            // 2: ARRAY_TANGENT
        null,            // 3: ARRAY_COLOR
        packed_uvs,      // 4: ARRAY_TEX_UV
        null,            // 5: ARRAY_TEX_UV2
        null,            // 6: ARRAY_CUSTOM0
        null,            // 7: ARRAY_CUSTOM1
        null,            // 8: ARRAY_CUSTOM2
        null,            // 9: ARRAY_CUSTOM3
        null,            // 10: ARRAY_BONES
        null,            // 11: ARRAY_WEIGHTS
        packed_indices   // 12: ARRAY_INDEX
    ];

    // Add surface to mesh - PRIMITIVE_TRIANGLES = 3
    array_mesh.add_surface_from_arrays(3, arrays);

    return array_mesh;
}

function calculate_normals(vertices, indices) {
    // Initialize normals to zero
    const normals = [];
    for (let i = 0; i < vertices.length; i++) {
        normals.push({ x: 0, y: 0, z: 0 });
    }

    // Accumulate face normals
    for (let j = 0; j < indices.length; j += 3) {
        const i0 = indices[j];
        const i1 = indices[j + 1];
        const i2 = indices[j + 2];

        const v0 = vertices[i0];
        const v1 = vertices[i1];
        const v2 = vertices[i2];

        // Calculate face normal
        const e1 = { x: v1.x - v0.x, y: v1.y - v0.y, z: v1.z - v0.z };
        const e2 = { x: v2.x - v0.x, y: v2.y - v0.y, z: v2.z - v0.z };

        const normal = {
            x: e1.y * e2.z - e1.z * e2.y,
            y: e1.z * e2.x - e1.x * e2.z,
            z: e1.x * e2.y - e1.y * e2.x
        };

        // Add to vertex normals
        normals[i0].x += normal.x;
        normals[i0].y += normal.y;
        normals[i0].z += normal.z;

        normals[i1].x += normal.x;
        normals[i1].y += normal.y;
        normals[i1].z += normal.z;

        normals[i2].x += normal.x;
        normals[i2].y += normal.y;
        normals[i2].z += normal.z;
    }

    // Normalize - use plain {x,y,z} objects for PackedVector3Array
    const result = [];
    for (let k = 0; k < normals.length; k++) {
        const n = normals[k];
        const len = Math.sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        if (len > 0.0001) {
            result.push({ x: n.x / len, y: n.y / len, z: n.z / len });
        } else {
            result.push({ x: 0, y: 1, z: 0 });
        }
    }

    return result;
}

function cleanup_fallen_pieces() {
    // Remove pieces that fell below the floor
    const children = sliced_pieces_container.get_children();
    for (let i = 0; i < children.length; i++) {
        const piece = children[i];
        if (piece.global_position.y < -10) {
            piece.queue_free();
            piece_count--;
            update_score();
        }
    }

    // Also check original objects
    const originals = sliceable_container.get_children();
    for (let j = 0; j < originals.length; j++) {
        const obj = originals[j];
        if (obj.global_position.y < -10) {
            obj.queue_free();
            piece_count--;
            update_score();
        }
    }
}

function update_score() {
    if (score_label) {
        score_label.text = "Slices: " + slice_count + " | Pieces: " + piece_count;
    }
}

// Cleanup when exiting
export function _exit_tree() {
    ComputeSlicer.cleanup();
}

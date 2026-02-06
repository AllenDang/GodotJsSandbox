// Compute Shader-based Mesh Slicer
// Uses RenderingDevice for GPU-accelerated mesh slicing

import earcut, { deviation } from './earcut.js';

let rd = null;  // RenderingDevice (local device for compute shaders)
let vertex_shader = null;
let triangle_shader = null;
let cap_shader = null;
let vertex_pipeline = null;
let triangle_pipeline = null;
let cap_pipeline = null;
let initialized = false;

// Maximum vertices/triangles we can handle
const MAX_VERTICES = 65536;
const MAX_TRIANGLES = 65536;
const MAX_OUTPUT_VERTICES = MAX_VERTICES * 3;  // Slicing can create more vertices

// Persistent buffers (reused across slices)
let input_vertex_buffer = null;
let input_normal_buffer = null;
let input_uv_buffer = null;
let input_index_buffer = null;
let params_buffer = null;
let vertex_class_buffer = null;
let front_vertex_buffer = null;
let front_normal_buffer = null;
let front_uv_buffer = null;
let front_index_buffer = null;
let back_vertex_buffer = null;
let back_normal_buffer = null;
let back_uv_buffer = null;
let back_index_buffer = null;
let counters_buffer = null;

// Boundary edge buffers for cap generation
let front_boundary_buffer = null;
let back_boundary_buffer = null;

// Cap generation buffers
let front_cap_centroid_buffer = null;
let back_cap_centroid_buffer = null;
let cap_params_buffer = null;

// Maximum boundary edges (one per split triangle)
const MAX_BOUNDARY_EDGES = 65536;

// Uniform sets
let vertex_uniform_set = null;
let triangle_uniform_set = null;
let front_cap_uniform_set = null;
let back_cap_uniform_set = null;

// Track if a compute list is currently open (for cleanup on error)
let compute_list_is_open = false;

export function init() {
    if (initialized) return true;

    console.log("Initializing compute slicer...");

    // Create a local RenderingDevice for compute work
    // Local devices support submit()/sync() for synchronous compute operations
    rd = RenderingServer.create_local_rendering_device();
    if (!rd) {
        console.log("ERROR: Could not create local RenderingDevice");
        return false;
    }
    console.log("Created local RenderingDevice");

    // Load and compile shaders
    if (!load_shaders()) {
        console.log("ERROR: Failed to load shaders");
        return false;
    }

    // Create pipelines
    if (!create_pipelines()) {
        console.log("ERROR: Failed to create pipelines");
        return false;
    }

    // Create persistent buffers
    if (!create_buffers()) {
        console.log("ERROR: Failed to create buffers");
        return false;
    }

    initialized = true;
    console.log("Compute slicer initialized successfully");
    return true;
}

function load_shaders() {
    // Load vertex classification shader
    const vertex_shader_file = load("user://games/slice_everything/shaders/mesh_slicer.glsl");
    if (!vertex_shader_file) {
        console.log("ERROR: Could not load mesh_slicer.glsl");
        return false;
    }

    const vertex_spirv = vertex_shader_file.get_spirv();
    if (!vertex_spirv) {
        console.log("ERROR: Could not compile mesh_slicer.glsl to SPIR-V");
        return false;
    }

    vertex_shader = rd.shader_create_from_spirv(vertex_spirv);
    if (!vertex_shader.is_valid()) {
        console.log("ERROR: Could not create vertex shader");
        return false;
    }

    // Load triangle processing shader
    const triangle_shader_file = load("user://games/slice_everything/shaders/triangle_slicer.glsl");
    if (!triangle_shader_file) {
        console.log("ERROR: Could not load triangle_slicer.glsl");
        return false;
    }

    const triangle_spirv = triangle_shader_file.get_spirv();
    if (!triangle_spirv) {
        console.log("ERROR: Could not compile triangle_slicer.glsl to SPIR-V");
        return false;
    }

    triangle_shader = rd.shader_create_from_spirv(triangle_spirv);
    if (!triangle_shader.is_valid()) {
        console.log("ERROR: Could not create triangle shader");
        return false;
    }

    // Load cap generator shader
    const cap_shader_file = load("user://games/slice_everything/shaders/cap_generator.glsl");
    if (!cap_shader_file) {
        console.log("ERROR: Could not load cap_generator.glsl");
        return false;
    }

    const cap_spirv = cap_shader_file.get_spirv();
    if (!cap_spirv) {
        console.log("ERROR: Could not compile cap_generator.glsl to SPIR-V");
        return false;
    }

    cap_shader = rd.shader_create_from_spirv(cap_spirv);
    if (!cap_shader.is_valid()) {
        console.log("ERROR: Could not create cap shader");
        return false;
    }

    console.log("Shaders loaded successfully");
    return true;
}

function create_pipelines() {
    vertex_pipeline = rd.compute_pipeline_create(vertex_shader);
    if (!vertex_pipeline.is_valid()) {
        console.log("ERROR: Could not create vertex pipeline");
        return false;
    }

    triangle_pipeline = rd.compute_pipeline_create(triangle_shader);
    if (!triangle_pipeline.is_valid()) {
        console.log("ERROR: Could not create triangle pipeline");
        return false;
    }

    cap_pipeline = rd.compute_pipeline_create(cap_shader);
    if (!cap_pipeline.is_valid()) {
        console.log("ERROR: Could not create cap pipeline");
        return false;
    }

    console.log("Pipelines created successfully");
    return true;
}

function create_buffers() {
    // Buffer sizes
    const vertex_size = 16;  // vec4 (16 bytes)
    const normal_size = 16;  // vec4 (16 bytes)
    const uv_size = 8;       // vec2 (8 bytes)
    const index_size = 4;    // uint (4 bytes)
    const params_size = 48;  // 2 vec4 + 4 uint (32 + 16 bytes, aligned)
    const counter_size = 24; // 6 uint (front_vert, back_vert, front_idx, back_idx, front_boundary, back_boundary)

    // Note: storage buffer usage flag not needed for basic compute shaders

    // Input buffers
    input_vertex_buffer = rd.storage_buffer_create(MAX_VERTICES * vertex_size);
    input_normal_buffer = rd.storage_buffer_create(MAX_VERTICES * normal_size);
    input_uv_buffer = rd.storage_buffer_create(MAX_VERTICES * uv_size);
    input_index_buffer = rd.storage_buffer_create(MAX_TRIANGLES * 3 * index_size);

    // Parameters uniform buffer
    params_buffer = rd.uniform_buffer_create(params_size);

    // Intermediate buffer
    vertex_class_buffer = rd.storage_buffer_create(MAX_VERTICES * 4);  // int per vertex

    // Output buffers for front mesh
    front_vertex_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * vertex_size);
    front_normal_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * normal_size);
    front_uv_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * uv_size);
    front_index_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * index_size);

    // Output buffers for back mesh
    back_vertex_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * vertex_size);
    back_normal_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * normal_size);
    back_uv_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * uv_size);
    back_index_buffer = rd.storage_buffer_create(MAX_OUTPUT_VERTICES * index_size);

    // Atomic counters buffer (now includes boundary edge counts)
    counters_buffer = rd.storage_buffer_create(counter_size);

    // Boundary edge buffers for cap generation (pairs of vec4: edge_start, edge_end)
    front_boundary_buffer = rd.storage_buffer_create(MAX_BOUNDARY_EDGES * 2 * vertex_size);
    back_boundary_buffer = rd.storage_buffer_create(MAX_BOUNDARY_EDGES * 2 * vertex_size);

    // Cap centroid accumulation buffers (4 floats: x, y, z, point_count)
    front_cap_centroid_buffer = rd.storage_buffer_create(16);
    back_cap_centroid_buffer = rd.storage_buffer_create(16);

    // Cap parameters uniform buffer (vec4 plane_normal with w=direction, vec4 centroid, uint edge_count, base_vert_idx, base_idx_idx, pad)
    cap_params_buffer = rd.uniform_buffer_create(48);

    console.log("Buffers created successfully");
    return true;
}

function create_uniform(binding, buffer, type) {
    const uniform = new RDUniform();
    uniform.uniform_type = type;
    uniform.binding = binding;
    uniform.add_id(buffer);
    return uniform;
}

// Pack float array to bytes for GPU buffer - uses C++ bulk encode for zero-copy performance
function pack_vec4_array(arr) {
    const bytes = new PackedByteArray([]);
    const handle = bytes.__packed_handle;
    // Use C++ bulk encode: writes vec3 data as vec4 padded buffer (16 bytes per vec)
    __packed_byte_array_encode_vec4_array(handle, arr);
    return bytes;
}

function pack_vec2_array(arr) {
    const bytes = new PackedByteArray([]);
    const handle = bytes.__packed_handle;
    // Use C++ bulk encode: writes vec2 data (8 bytes per vec)
    __packed_byte_array_encode_vec2_array(handle, arr);
    return bytes;
}

function pack_uint_array(arr) {
    const bytes = new PackedByteArray([]);
    const handle = bytes.__packed_handle;
    // Use C++ bulk encode: writes uint32 data (4 bytes each)
    __packed_byte_array_encode_uint32_array(handle, arr);
    return bytes;
}

function pack_params(plane_normal, plane_point, triangle_count, vertex_count) {
    const bytes = new PackedByteArray();
    bytes.resize(48);
    // plane_normal (vec4)
    bytes.encode_float(0, plane_normal.x);
    bytes.encode_float(4, plane_normal.y);
    bytes.encode_float(8, plane_normal.z);
    bytes.encode_float(12, 0.0);
    // plane_point (vec4)
    bytes.encode_float(16, plane_point.x);
    bytes.encode_float(20, plane_point.y);
    bytes.encode_float(24, plane_point.z);
    bytes.encode_float(28, 0.0);
    // counts (4 uints)
    bytes.encode_u32(32, triangle_count);
    bytes.encode_u32(36, vertex_count);
    bytes.encode_u32(40, 0);  // padding
    bytes.encode_u32(44, 0);  // padding
    return bytes;
}

function pack_cap_params(plane_normal, normal_direction, centroid, edge_count, base_vert_idx, base_idx_idx) {
    const bytes = new PackedByteArray();
    bytes.resize(48);
    // plane_normal (vec4) with w = normal_direction
    bytes.encode_float(0, plane_normal.x);
    bytes.encode_float(4, plane_normal.y);
    bytes.encode_float(8, plane_normal.z);
    bytes.encode_float(12, normal_direction);  // -1 for front cap, +1 for back cap
    // centroid (vec4)
    bytes.encode_float(16, centroid.x);
    bytes.encode_float(20, centroid.y);
    bytes.encode_float(24, centroid.z);
    bytes.encode_float(28, 0.0);  // w unused
    // cap counts (4 uints)
    bytes.encode_u32(32, edge_count);
    bytes.encode_u32(36, base_vert_idx);
    bytes.encode_u32(40, base_idx_idx);
    bytes.encode_u32(44, 0);  // padding
    return bytes;
}

// Extract edges from boundary data as array of {p0, p1} objects
function extract_edges_from_boundary(boundary_data, edge_count) {
    const edges = [];
    for (let i = 0; i < edge_count; i++) {
        const offset0 = i * 32;
        const offset1 = offset0 + 16;
        edges.push({
            p0: {
                x: boundary_data.decode_float(offset0),
                y: boundary_data.decode_float(offset0 + 4),
                z: boundary_data.decode_float(offset0 + 8)
            },
            p1: {
                x: boundary_data.decode_float(offset1),
                y: boundary_data.decode_float(offset1 + 4),
                z: boundary_data.decode_float(offset1 + 8)
            }
        });
    }
    return edges;
}

// Snap vertices to a grid to handle GPU floating-point precision differences
// Returns a string key for the snapped position
// Note: GPU interpolation can produce vertices that differ by small amounts
// even when they should be identical.
const SNAP_GRID = 0.003;  // 3mm grid - fine precision
function snap_vertex_key(v) {
    const sx = Math.round(v.x / SNAP_GRID);
    const sy = Math.round(v.y / SNAP_GRID);
    const sz = Math.round(v.z / SNAP_GRID);
    return sx + "," + sy + "," + sz;
}

// Check if two points are approximately equal
// Using larger epsilon for GPU float precision differences
const EPSILON = 0.001;
function points_equal(a, b) {
    return Math.abs(a.x - b.x) < EPSILON &&
           Math.abs(a.y - b.y) < EPSILON &&
           Math.abs(a.z - b.z) < EPSILON;
}

// Order boundary edges into connected loops by following edge connectivity
// Uses nearest-neighbor matching to handle GPU precision issues
// Returns array of arrays - each inner array is a separate closed loop of vertices
function order_boundary_edges_multi(edges) {
    if (edges.length === 0) return [];
    if (edges.length === 1) return [[edges[0].p0, edges[0].p1]];

    // Build an array of all edge endpoints for nearest-neighbor search
    // Each edge has two endpoints (p0, p1)
    // We need to find which endpoints from different edges are "the same point"

    const MERGE_DIST = 0.003;  // 3mm merge distance for matching endpoints
    const MERGE_DIST_SQ = MERGE_DIST * MERGE_DIST;

    // Minimum edge length to consider valid (skip very short edges from GPU noise)
    const MIN_EDGE_LEN = 0.005;  // 5mm
    const MIN_EDGE_LEN_SQ = MIN_EDGE_LEN * MIN_EDGE_LEN;

    // Collect all unique vertices by merging nearby ones
    const unique_verts = [];  // Array of {x, y, z}
    const edge_vert_indices = [];  // For each edge: [idx0, idx1] into unique_verts

    function find_or_add_vertex(v) {
        // Search for existing vertex within merge distance
        for (let i = 0; i < unique_verts.length; i++) {
            const u = unique_verts[i];
            const dx = v.x - u.x;
            const dy = v.y - u.y;
            const dz = v.z - u.z;
            if (dx * dx + dy * dy + dz * dz < MERGE_DIST_SQ) {
                return i;
            }
        }
        // No match found, add new vertex
        unique_verts.push({ x: v.x, y: v.y, z: v.z });
        return unique_verts.length - 1;
    }

    // Process all edges, filtering out very short ones (GPU noise)
    let skipped_short = 0;
    for (let i = 0; i < edges.length; i++) {
        const p0 = edges[i].p0;
        const p1 = edges[i].p1;

        // Check edge length
        const dx = p1.x - p0.x;
        const dy = p1.y - p0.y;
        const dz = p1.z - p0.z;
        const len_sq = dx * dx + dy * dy + dz * dz;

        if (len_sq < MIN_EDGE_LEN_SQ) {
            // Skip very short edges - they're GPU precision artifacts
            edge_vert_indices.push([-1, -1]);  // Placeholder to keep indices aligned
            skipped_short++;
            continue;
        }

        const idx0 = find_or_add_vertex(p0);
        const idx1 = find_or_add_vertex(p1);
        edge_vert_indices.push([idx0, idx1]);
    }

    console.log("[CAP] Merged " + (edges.length * 2) + " endpoints into " + unique_verts.length + " unique vertices (skipped " + skipped_short + " short edges)");

    // Build adjacency list: for each unique vertex, list of connected vertices via edges
    const adj = {};  // vertex_idx -> [{neighbor_idx, edge_idx}]

    for (let i = 0; i < edge_vert_indices.length; i++) {
        const idx0 = edge_vert_indices[i][0];
        const idx1 = edge_vert_indices[i][1];

        // Skip edges that were filtered out or are degenerate
        if (idx0 < 0 || idx1 < 0 || idx0 === idx1) continue;

        if (!adj[idx0]) adj[idx0] = [];
        if (!adj[idx1]) adj[idx1] = [];

        adj[idx0].push({ neighbor: idx1, edge_idx: i });
        adj[idx1].push({ neighbor: idx0, edge_idx: i });
    }

    // Find all closed loops by traversing the graph
    const used_edges = {};
    const all_loops = [];

    const all_vert_indices = Object.keys(adj);
    for (let vi = 0; vi < all_vert_indices.length; vi++) {
        const start_idx = parseInt(all_vert_indices[vi]);
        const start_neighbors = adj[start_idx];
        if (!start_neighbors) continue;

        // Check if there are unused edges from this vertex
        let has_unused = false;
        for (let ni = 0; ni < start_neighbors.length; ni++) {
            if (!used_edges[start_neighbors[ni].edge_idx]) {
                has_unused = true;
                break;
            }
        }
        if (!has_unused) continue;

        // Start a new loop from this vertex
        const loop_vert_indices = [start_idx];
        let current_idx = start_idx;
        let iterations = 0;

        while (iterations < edges.length * 2) {
            iterations++;
            const neighbors = adj[current_idx];
            if (!neighbors) break;

            let found_next = false;
            for (let ni = 0; ni < neighbors.length; ni++) {
                const edge_idx = neighbors[ni].edge_idx;
                if (used_edges[edge_idx]) continue;

                const next_idx = neighbors[ni].neighbor;
                used_edges[edge_idx] = true;

                // Check if we've closed the loop
                if (next_idx === start_idx && loop_vert_indices.length >= 3) {
                    found_next = true;
                    break;
                }

                loop_vert_indices.push(next_idx);
                current_idx = next_idx;
                found_next = true;
                break;
            }

            if (!found_next) break;
            if (current_idx === start_idx) break;
        }

        // Convert vertex indices to actual positions
        if (loop_vert_indices.length >= 3) {
            const loop_verts = [];
            for (let li = 0; li < loop_vert_indices.length; li++) {
                loop_verts.push(unique_verts[loop_vert_indices[li]]);
            }
            all_loops.push(loop_verts);
        }
    }

    // Count unused edges (only count non-skipped edges)
    let unused_count = 0;
    let degenerate_count = 0;
    let disconnected_count = 0;
    let valid_edges = 0;

    for (let i = 0; i < edges.length; i++) {
        const idx0 = edge_vert_indices[i][0];
        const idx1 = edge_vert_indices[i][1];

        // Skip pre-filtered short edges
        if (idx0 < 0) continue;

        valid_edges++;

        if (!used_edges[i]) {
            unused_count++;

            if (idx0 === idx1) {
                degenerate_count++;
            } else {
                disconnected_count++;
                // Log first few disconnected edges for debugging
                if (disconnected_count <= 3) {
                    const v0 = edges[i].p0;
                    const v1 = edges[i].p1;
                    console.log("[CAP] Disconnected edge " + i + ": (" +
                        v0.x.toFixed(3) + "," + v0.y.toFixed(3) + "," + v0.z.toFixed(3) + ") -> (" +
                        v1.x.toFixed(3) + "," + v1.y.toFixed(3) + "," + v1.z.toFixed(3) + ") merged to verts " + idx0 + "," + idx1);
                }
            }
        }
    }

    // Build loop sizes string
    let loop_sizes = "";
    for (let i = 0; i < all_loops.length; i++) {
        loop_sizes += (i > 0 ? ", " : "") + all_loops[i].length;
    }

    console.log("[CAP] Found " + all_loops.length + " loops from " + valid_edges + " valid edges (of " + edges.length + " total), sizes=[" + loop_sizes + "], unused=" + unused_count + " (degenerate=" + degenerate_count + ", disconnected=" + disconnected_count + ")");

    return all_loops;
}


// Legacy single-loop function for compatibility
function order_boundary_edges(edges) {
    const loops = order_boundary_edges_multi(edges);
    if (loops.length === 0) return [];
    // Return the largest loop (most vertices)
    let largest = loops[0];
    for (let i = 1; i < loops.length; i++) {
        if (loops[i].length > largest.length) {
            largest = loops[i];
        }
    }
    return largest;
}

// Compute centroid from ordered vertices
function compute_centroid_from_vertices(verts) {
    let cx = 0, cy = 0, cz = 0;
    for (let i = 0; i < verts.length; i++) {
        cx += verts[i].x;
        cy += verts[i].y;
        cz += verts[i].z;
    }
    if (verts.length > 0) {
        cx /= verts.length;
        cy /= verts.length;
        cz /= verts.length;
    }
    return { x: cx, y: cy, z: cz };
}

// Remove duplicate, spike, and collinear vertices from a polygon
// - Duplicate: vertex[i] == vertex[i+1]
// - Spike: vertex[i] == vertex[i+2] (goes to a point and back)
// - Collinear: three consecutive vertices lie on a line
// Returns cleaned vertex array and mapping from new indices to original indices
function clean_polygon_vertices(verts) {
    if (verts.length < 3) return { verts: verts, mapping: [] };

    const dominated_epsilon = 0.0001;
    const collinear_epsilon = 0.0001;  // Area threshold for collinearity

    function verts_equal(a, b) {
        return Math.abs(a.x - b.x) < dominated_epsilon &&
               Math.abs(a.y - b.y) < dominated_epsilon &&
               Math.abs(a.z - b.z) < dominated_epsilon;
    }

    // Check if three points are collinear using cross product magnitude
    function are_collinear(a, b, c) {
        // Vector AB and AC
        const abx = b.x - a.x, aby = b.y - a.y, abz = b.z - a.z;
        const acx = c.x - a.x, acy = c.y - a.y, acz = c.z - a.z;
        // Cross product AB x AC
        const cx = aby * acz - abz * acy;
        const cy = abz * acx - abx * acz;
        const cz = abx * acy - aby * acx;
        // Magnitude of cross product (area of parallelogram)
        const area = Math.sqrt(cx * cx + cy * cy + cz * cz);
        // Length of AB for normalization
        const ab_len = Math.sqrt(abx * abx + aby * aby + abz * abz);
        // If AB is too short, consider it collinear (degenerate)
        if (ab_len < dominated_epsilon) return true;
        // Normalized area (height of triangle)
        return (area / ab_len) < collinear_epsilon;
    }

    // Multiple passes to remove all issues
    let cleaned = verts.slice();
    let mapping = [];
    for (let i = 0; i < verts.length; i++) mapping.push(i);

    let changed = true;
    let passes = 0;
    while (changed && passes < 10) {
        changed = false;
        passes++;

        // Remove consecutive duplicates
        let new_cleaned = [];
        let new_mapping = [];
        for (let i = 0; i < cleaned.length; i++) {
            const next = (i + 1) % cleaned.length;
            if (!verts_equal(cleaned[i], cleaned[next])) {
                new_cleaned.push(cleaned[i]);
                new_mapping.push(mapping[i]);
            } else {
                changed = true;
            }
        }
        cleaned = new_cleaned;
        mapping = new_mapping;

        if (cleaned.length < 3) break;

        // Remove spikes (v[i] == v[i+2])
        new_cleaned = [];
        new_mapping = [];
        for (let i = 0; i < cleaned.length; i++) {
            const next2 = (i + 2) % cleaned.length;
            if (!verts_equal(cleaned[i], cleaned[next2])) {
                new_cleaned.push(cleaned[i]);
                new_mapping.push(mapping[i]);
            } else {
                // Skip this vertex - it's the tip of a spike
                changed = true;
            }
        }
        cleaned = new_cleaned;
        mapping = new_mapping;

        if (cleaned.length < 3) break;

        // Remove collinear points (middle point of three collinear points)
        new_cleaned = [];
        new_mapping = [];
        for (let i = 0; i < cleaned.length; i++) {
            const prev = (i - 1 + cleaned.length) % cleaned.length;
            const next = (i + 1) % cleaned.length;
            if (!are_collinear(cleaned[prev], cleaned[i], cleaned[next])) {
                new_cleaned.push(cleaned[i]);
                new_mapping.push(mapping[i]);
            } else {
                // Skip this vertex - it's collinear with its neighbors
                changed = true;
            }
        }
        cleaned = new_cleaned;
        mapping = new_mapping;
    }

    return { verts: cleaned, mapping: mapping };
}

// Triangulate polygon using earcut.js - industry-standard robust triangulation
// Returns array of triangle indices (relative to input vertex array)
function triangulate_polygon(verts, normal) {
    if (verts.length < 3) return [];
    if (verts.length === 3) return [0, 1, 2];

    // Clean the polygon first - remove duplicates and spikes
    const cleaned = clean_polygon_vertices(verts);
    const clean_verts = cleaned.verts;
    const index_mapping = cleaned.mapping;

    if (clean_verts.length !== verts.length) {
        console.log("[CLEAN] Removed " + (verts.length - clean_verts.length) + " degenerate verts: " +
                    verts.length + " -> " + clean_verts.length);
    }

    if (clean_verts.length < 3) return [];
    if (clean_verts.length === 3) {
        return [index_mapping[0], index_mapping[1], index_mapping[2]];
    }

    // Determine which axis to project out (the dominant normal component)
    const ax = Math.abs(normal.x);
    const ay = Math.abs(normal.y);
    const az = Math.abs(normal.z);

    // Project vertices to 2D flat array for earcut: [x0, y0, x1, y1, ...]
    const flatCoords = [];
    for (let i = 0; i < clean_verts.length; i++) {
        const v = clean_verts[i];
        if (ax >= ay && ax >= az) {
            // Project onto YZ plane (X is dominant)
            flatCoords.push(v.y, v.z);
        } else if (ay >= ax && ay >= az) {
            // Project onto XZ plane (Y is dominant)
            flatCoords.push(v.x, v.z);
        } else {
            // Project onto XY plane (Z is dominant)
            flatCoords.push(v.x, v.y);
        }
    }

    // Use earcut for robust triangulation
    // earcut returns indices into the cleaned vertex array
    let earcut_indices = earcut(flatCoords, null, 2);

    const expected = (clean_verts.length - 2) * 3;
    // Calculate triangulation deviation to understand quality
    const dev = deviation(flatCoords, null, 2, earcut_indices);

    if (earcut_indices.length !== expected || dev > 0.01) {
        console.log("[EARCUT] clean_verts=" + clean_verts.length + " expected=" + expected +
                    " got=" + earcut_indices.length + " deviation=" + (dev * 100).toFixed(2) + "%");

        // Log first few coordinates to see the shape
        let coordStr = "";
        for (let i = 0; i < Math.min(10, clean_verts.length); i++) {
            const idx = i * 2;
            coordStr += "(" + flatCoords[idx].toFixed(3) + "," + flatCoords[idx+1].toFixed(3) + ") ";
        }
        if (clean_verts.length > 10) coordStr += "...";
        console.log("[EARCUT] coords: " + coordStr);
    }

    // Only skip if earcut produced NO triangles (complete failure)
    // High deviation is acceptable - we want caps visible even if imperfect
    if (earcut_indices.length === 0) {
        console.log("[EARCUT] WARNING: earcut produced no triangles for " + clean_verts.length + " verts");
        return [];
    }

    // Map back to original vertex indices
    let indices = [];
    for (let i = 0; i < earcut_indices.length; i++) {
        indices.push(index_mapping[earcut_indices[i]]);
    }

    return indices;
}

// Ensure boundary vertices are wound CCW when viewed from the plane normal direction
// This ensures consistent triangle winding for cap generation
function ensure_ccw_winding(verts, plane_normal) {
    if (verts.length < 3) return verts;

    // Compute signed area of the polygon projected onto the plane
    // Using the first two edges to determine orientation
    const centroid = compute_centroid_from_vertices(verts);

    // Sum of cross products of edges from centroid gives normal direction
    let nx = 0, ny = 0, nz = 0;
    for (let i = 0; i < verts.length; i++) {
        const v0 = verts[i];
        const v1 = verts[(i + 1) % verts.length];

        // Edge vectors from centroid
        const e0x = v0.x - centroid.x;
        const e0y = v0.y - centroid.y;
        const e0z = v0.z - centroid.z;
        const e1x = v1.x - centroid.x;
        const e1y = v1.y - centroid.y;
        const e1z = v1.z - centroid.z;

        // Cross product e0 x e1
        nx += e0y * e1z - e0z * e1y;
        ny += e0z * e1x - e0x * e1z;
        nz += e0x * e1y - e0y * e1x;
    }

    // Dot product with plane normal - positive means CCW from normal's view
    const dot = nx * plane_normal.x + ny * plane_normal.y + nz * plane_normal.z;

    if (dot < 0) {
        // Reverse order to get CCW winding
        return verts.slice().reverse();
    }
    return verts;
}

// Pack ordered boundary vertices for GPU (as sequential vec4s)
function pack_ordered_boundary(verts) {
    const bytes = new PackedByteArray();
    bytes.resize(verts.length * 16);  // 16 bytes per vec4
    for (let i = 0; i < verts.length; i++) {
        const offset = i * 16;
        bytes.encode_float(offset, verts[i].x);
        bytes.encode_float(offset + 4, verts[i].y);
        bytes.encode_float(offset + 8, verts[i].z);
        bytes.encode_float(offset + 12, 1.0);
    }
    return bytes;
}

// Unpack output buffers to Godot PackedArrays - returns proxy objects, not JS arrays
// Uses PackedByteArray proxy methods for intuitive zero-copy API
function unpack_to_packed_vector3_array(bytes, count) {
    // Reinterpret byte buffer as PackedVector3Array (vec4 format, 16 bytes per element)
    return bytes.as_vector3_array(count);
}

function unpack_to_packed_vector2_array(bytes, count) {
    // Reinterpret byte buffer as PackedVector2Array (vec2 format, 8 bytes per element)
    return bytes.as_vector2_array(count);
}

function unpack_to_packed_int32_array(bytes, count) {
    // Reinterpret byte buffer as PackedInt32Array (4 bytes per element)
    return bytes.as_int32_array(count);
}

export function slice(vertices, indices, normals, uvs, plane_normal, plane_point) {
    const perf = {};
    let t0 = Time.get_ticks_usec();

    if (!initialized) {
        if (!init()) {
            return { success: false, error: "Failed to initialize compute slicer" };
        }
    }

    const vertex_count = vertices.length;
    const triangle_count = Math.floor(indices.length / 3);

    if (vertex_count > MAX_VERTICES || triangle_count > MAX_TRIANGLES) {
        return { success: false, error: "Mesh too large for compute slicer" };
    }

    // Helper to clean up uniform sets on error or completion
    function cleanup_uniform_sets() {
        if (vertex_uniform_set && vertex_uniform_set.is_valid()) {
            rd.free_rid(vertex_uniform_set);
        }
        if (triangle_uniform_set && triangle_uniform_set.is_valid()) {
            rd.free_rid(triangle_uniform_set);
        }
        if (front_cap_uniform_set && front_cap_uniform_set.is_valid()) {
            rd.free_rid(front_cap_uniform_set);
        }
        if (back_cap_uniform_set && back_cap_uniform_set.is_valid()) {
            rd.free_rid(back_cap_uniform_set);
        }
        vertex_uniform_set = null;
        triangle_uniform_set = null;
        front_cap_uniform_set = null;
        back_cap_uniform_set = null;
    }

    try {
        // Upload input data to GPU
        const packed_verts = pack_vec4_array(vertices);
        rd.buffer_update(input_vertex_buffer, 0, vertex_count * 16, packed_verts);
        rd.buffer_update(input_normal_buffer, 0, vertex_count * 16, pack_vec4_array(normals));

        perf.upload_verts = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // Handle null/empty UVs - create default UVs if none exist
        if (uvs && uvs.length > 0) {
            rd.buffer_update(input_uv_buffer, 0, vertex_count * 8, pack_vec2_array(uvs));
        } else {
            // Create default UVs (all zeros) for meshes without UVs
            const default_uvs = new PackedByteArray();
            default_uvs.resize(vertex_count * 8);
            rd.buffer_update(input_uv_buffer, 0, vertex_count * 8, default_uvs);
        }

        rd.buffer_update(input_index_buffer, 0, indices.length * 4, pack_uint_array(indices));
        rd.buffer_update(params_buffer, 0, 48, pack_params(plane_normal, plane_point, triangle_count, vertex_count));

        // Reset counters (6 uints: front_vert, back_vert, front_idx, back_idx, front_boundary, back_boundary)
        const zero_counters = new PackedByteArray();
        zero_counters.resize(24);
        rd.buffer_update(counters_buffer, 0, 24, zero_counters);

        // Reset cap centroid accumulators
        const zero_centroid = new PackedByteArray();
        zero_centroid.resize(16);
        rd.buffer_update(front_cap_centroid_buffer, 0, 16, zero_centroid);
        rd.buffer_update(back_cap_centroid_buffer, 0, 16, zero_centroid);

        // Create uniform sets for vertex shader (bindings 0-5)
        const vertex_uniforms = [
            create_uniform(0, input_vertex_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(1, input_normal_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(2, input_uv_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(3, input_index_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(4, params_buffer, RenderingDevice.UNIFORM_TYPE_UNIFORM_BUFFER),
            create_uniform(5, vertex_class_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
        ];

        vertex_uniform_set = rd.uniform_set_create(vertex_uniforms, vertex_shader, 0);

        // Run vertex classification shader
        const vertex_groups = Math.ceil(vertex_count / 64);
        const compute_list = rd.compute_list_begin();
        compute_list_is_open = true;
        rd.compute_list_bind_compute_pipeline(compute_list, vertex_pipeline);
        rd.compute_list_bind_uniform_set(compute_list, vertex_uniform_set, 0);
        rd.compute_list_dispatch(compute_list, vertex_groups, 1, 1);
        rd.compute_list_add_barrier(compute_list);

        // Create uniform set for triangle shader (bindings 0-16)
        const triangle_uniforms = [
            create_uniform(0, input_vertex_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(1, input_normal_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(2, input_uv_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(3, input_index_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(4, params_buffer, RenderingDevice.UNIFORM_TYPE_UNIFORM_BUFFER),
            create_uniform(5, vertex_class_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(6, front_vertex_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(7, front_normal_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(8, front_uv_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(9, front_index_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(10, back_vertex_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(11, back_normal_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(12, back_uv_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(13, back_index_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(14, counters_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(15, front_boundary_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
            create_uniform(16, back_boundary_buffer, RenderingDevice.UNIFORM_TYPE_STORAGE_BUFFER),
        ];

        triangle_uniform_set = rd.uniform_set_create(triangle_uniforms, triangle_shader, 0);

        // Run triangle processing shader
        const triangle_groups = Math.ceil(triangle_count / 64);
        rd.compute_list_bind_compute_pipeline(compute_list, triangle_pipeline);
        rd.compute_list_bind_uniform_set(compute_list, triangle_uniform_set, 0);
        rd.compute_list_dispatch(compute_list, triangle_groups, 1, 1);
        rd.compute_list_end();
        compute_list_is_open = false;

        perf.gpu_dispatch = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // Sync and read back results from triangle pass
        rd.submit();
        rd.sync();

        perf.gpu_sync = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // Read counters (6 uints now)
        const counter_data = rd.buffer_get_data(counters_buffer, 0, 24);
        let front_vert_count = counter_data.decode_u32(0);
        let back_vert_count = counter_data.decode_u32(4);
        let front_idx_count = counter_data.decode_u32(8);
        let back_idx_count = counter_data.decode_u32(12);
        const front_boundary_count = counter_data.decode_u32(16);
        const back_boundary_count = counter_data.decode_u32(20);

        if (front_vert_count === 0 || back_vert_count === 0) {
            cleanup_uniform_sets();
            return { success: false, error: "Slice did not split the mesh" };
        }

        // Sanity check - don't process unreasonable counts
        if (front_vert_count > MAX_OUTPUT_VERTICES || back_vert_count > MAX_OUTPUT_VERTICES) {
            cleanup_uniform_sets();
            return { success: false, error: "Invalid vertex count from compute shader" };
        }

        perf.counters_read = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // Run cap generation if there are boundary edges
        let front_cap_vert_count = 0;
        let front_cap_idx_count = 0;
        let back_cap_vert_count = 0;
        let back_cap_idx_count = 0;

        // Store cap geometry for later merging
        let front_cap_data = null;
        let back_cap_data = null;

        if (front_boundary_count > 0 && back_boundary_count > 0) {
            // Read boundary edges from GPU
            const front_boundary_data = rd.buffer_get_data(front_boundary_buffer, 0, front_boundary_count * 32);
            const back_boundary_data = rd.buffer_get_data(back_boundary_buffer, 0, back_boundary_count * 32);

            // Extract and order boundary edges into ALL connected loops
            const front_edges = extract_edges_from_boundary(front_boundary_data, front_boundary_count);
            const back_edges = extract_edges_from_boundary(back_boundary_data, back_boundary_count);

            const front_loops = order_boundary_edges_multi(front_edges);
            const back_loops = order_boundary_edges_multi(back_edges);

            // Ensure consistent winding direction for each loop
            // Front cap faces TOWARD the back piece (negative plane normal direction)
            // Back cap faces TOWARD the front piece (positive plane normal direction)
            const neg_plane_normal = { x: -plane_normal.x, y: -plane_normal.y, z: -plane_normal.z };
            for (let i = 0; i < front_loops.length; i++) {
                front_loops[i] = ensure_ccw_winding(front_loops[i], neg_plane_normal);
            }
            for (let i = 0; i < back_loops.length; i++) {
                back_loops[i] = ensure_ccw_winding(back_loops[i], plane_normal);
            }

            console.log("[CAP] front_edges=" + front_edges.length + " back_edges=" + back_edges.length +
                        " front_loops=" + front_loops.length + " back_loops=" + back_loops.length);

            // Debug: log first edge from each side to verify data is being read correctly
            if (front_edges.length > 0) {
                const e = front_edges[0];
                console.log("[CAP] First front edge: (" + e.p0.x.toFixed(3) + "," + e.p0.y.toFixed(3) + "," + e.p0.z.toFixed(3) + ") -> (" +
                            e.p1.x.toFixed(3) + "," + e.p1.y.toFixed(3) + "," + e.p1.z.toFixed(3) + ")");
            }
            console.log("[CAP] Front cap normal (neg_plane): (" + neg_plane_normal.x.toFixed(3) + "," + neg_plane_normal.y.toFixed(3) + "," + neg_plane_normal.z.toFixed(3) + ")");
            console.log("[CAP] Back cap normal (plane): (" + plane_normal.x.toFixed(3) + "," + plane_normal.y.toFixed(3) + "," + plane_normal.z.toFixed(3) + ")");

            // Simple fan triangulation: center + boundary vertices
            // For N boundary vertices: 1 center + N boundary = N+1 vertices, N triangles

            // Build front cap geometry
            const front_cap_verts = [];
            const front_cap_norms = [];
            const front_cap_uvs = [];
            const front_cap_indices = [];

            for (let li = 0; li < front_loops.length; li++) {
                const loop = front_loops[li];
                if (loop.length < 3) continue;

                const cap_normal = neg_plane_normal;
                const first_vert_idx = front_cap_verts.length;

                // 1. Calculate center point
                let cx = 0, cy = 0, cz = 0;
                for (let i = 0; i < loop.length; i++) {
                    cx += loop[i].x;
                    cy += loop[i].y;
                    cz += loop[i].z;
                }
                cx /= loop.length;
                cy /= loop.length;
                cz /= loop.length;

                // 2. Add center vertex first
                front_cap_verts.push({ x: cx, y: cy, z: cz });
                front_cap_norms.push(cap_normal);
                front_cap_uvs.push({ x: 0.5, y: 0.5 });

                // 3. Add boundary vertices
                for (let i = 0; i < loop.length; i++) {
                    front_cap_verts.push({ x: loop[i].x, y: loop[i].y, z: loop[i].z });
                    front_cap_norms.push(cap_normal);
                    front_cap_uvs.push({ x: 0.5, y: 0.5 });
                }

                // 4. Create triangles: center + two neighbor boundary vertices
                const center_idx = first_vert_idx;
                for (let i = 0; i < loop.length; i++) {
                    const v1_idx = first_vert_idx + 1 + i;
                    const v2_idx = first_vert_idx + 1 + ((i + 1) % loop.length);

                    // Winding order for front cap (facing negative plane normal direction)
                    front_cap_indices.push(center_idx);
                    front_cap_indices.push(v2_idx);
                    front_cap_indices.push(v1_idx);
                }
            }

            // Build back cap geometry
            const back_cap_verts = [];
            const back_cap_norms = [];
            const back_cap_uvs = [];
            const back_cap_indices = [];

            for (let li = 0; li < back_loops.length; li++) {
                const loop = back_loops[li];
                if (loop.length < 3) continue;

                const cap_normal = plane_normal;
                const first_vert_idx = back_cap_verts.length;

                // 1. Calculate center point
                let cx = 0, cy = 0, cz = 0;
                for (let i = 0; i < loop.length; i++) {
                    cx += loop[i].x;
                    cy += loop[i].y;
                    cz += loop[i].z;
                }
                cx /= loop.length;
                cy /= loop.length;
                cz /= loop.length;

                // 2. Add center vertex first
                back_cap_verts.push({ x: cx, y: cy, z: cz });
                back_cap_norms.push(cap_normal);
                back_cap_uvs.push({ x: 0.5, y: 0.5 });

                // 3. Add boundary vertices
                for (let i = 0; i < loop.length; i++) {
                    back_cap_verts.push({ x: loop[i].x, y: loop[i].y, z: loop[i].z });
                    back_cap_norms.push(cap_normal);
                    back_cap_uvs.push({ x: 0.5, y: 0.5 });
                }

                // 4. Create triangles: center + two neighbor boundary vertices
                const center_idx = first_vert_idx;
                for (let i = 0; i < loop.length; i++) {
                    const v1_idx = first_vert_idx + 1 + i;
                    const v2_idx = first_vert_idx + 1 + ((i + 1) % loop.length);

                    // Winding order for back cap (facing positive plane normal direction)
                    back_cap_indices.push(center_idx);
                    back_cap_indices.push(v1_idx);
                    back_cap_indices.push(v2_idx);
                }
            }

            // Store cap data (indices are already 0-based, no base_vert needed)
            if (front_cap_verts.length > 0) {
                front_cap_data = {
                    verts: front_cap_verts,
                    norms: front_cap_norms,
                    uvs: front_cap_uvs,
                    indices: front_cap_indices
                };
                front_cap_vert_count = front_cap_verts.length;
                front_cap_idx_count = front_cap_indices.length;
            }

            if (back_cap_verts.length > 0) {
                back_cap_data = {
                    verts: back_cap_verts,
                    norms: back_cap_norms,
                    uvs: back_cap_uvs,
                    indices: back_cap_indices
                };
                back_cap_vert_count = back_cap_verts.length;
                back_cap_idx_count = back_cap_indices.length;
            }

            console.log("[CAP] Generated: front=" + front_cap_verts.length + " verts, " + (front_cap_indices.length/3) + " tris | back=" + back_cap_verts.length + " verts, " + (back_cap_indices.length/3) + " tris");
        }

        perf.cap_gen = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // front_vert_count and front_idx_count are the sliced mesh counts (from GPU)
        // Cap data is stored separately in front_cap_data/back_cap_data
        const sliced_front_vert_count = front_vert_count;
        const sliced_front_idx_count = front_idx_count;
        const sliced_back_vert_count = back_vert_count;
        const sliced_back_idx_count = back_idx_count;

        const front_verts_data = rd.buffer_get_data(front_vertex_buffer, 0, sliced_front_vert_count * 16);
        const front_norms_data = rd.buffer_get_data(front_normal_buffer, 0, sliced_front_vert_count * 16);
        const front_uvs_data = rd.buffer_get_data(front_uv_buffer, 0, sliced_front_vert_count * 8);
        const front_idx_data = rd.buffer_get_data(front_index_buffer, 0, sliced_front_idx_count * 4);

        const back_verts_data = rd.buffer_get_data(back_vertex_buffer, 0, sliced_back_vert_count * 16);
        const back_norms_data = rd.buffer_get_data(back_normal_buffer, 0, sliced_back_vert_count * 16);
        const back_uvs_data = rd.buffer_get_data(back_uv_buffer, 0, sliced_back_vert_count * 8);
        const back_idx_data = rd.buffer_get_data(back_index_buffer, 0, sliced_back_idx_count * 4);

        perf.readback = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // Convert GPU sliced data to Godot PackedArrays
        let front_vertices = unpack_to_packed_vector3_array(front_verts_data, sliced_front_vert_count);
        let front_normals = unpack_to_packed_vector3_array(front_norms_data, sliced_front_vert_count);
        let front_uvs = unpack_to_packed_vector2_array(front_uvs_data, sliced_front_vert_count);
        let front_indices = unpack_to_packed_int32_array(front_idx_data, sliced_front_idx_count);

        let back_vertices = unpack_to_packed_vector3_array(back_verts_data, sliced_back_vert_count);
        let back_normals = unpack_to_packed_vector3_array(back_norms_data, sliced_back_vert_count);
        let back_uvs = unpack_to_packed_vector2_array(back_uvs_data, sliced_back_vert_count);
        let back_indices = unpack_to_packed_int32_array(back_idx_data, sliced_back_idx_count);

        // Caps are returned separately (not merged) for debug visualization
        // Log cap info
        if (front_cap_data) {
            console.log("[CAP] Front cap: " + front_cap_data.verts.length + " verts, " +
                        front_cap_data.indices.length + " indices");
        }
        if (back_cap_data) {
            console.log("[CAP] Back cap: " + back_cap_data.verts.length + " verts, " +
                        back_cap_data.indices.length + " indices");
        }

        // Log final mesh sizes (without caps)
        console.log("[MESH] Sliced mesh (no caps): front=" + front_vertices.length + " verts, back=" + back_vertices.length + " verts");

        // Return sliced mesh WITHOUT caps merged - caps are separate for debug visualization
        const front_mesh = {
            vertices: front_vertices,
            normals: front_normals,
            uvs: front_uvs,
            indices: front_indices
        };

        const back_mesh = {
            vertices: back_vertices,
            normals: back_normals,
            uvs: back_uvs,
            indices: back_indices
        };

        // Prepare separate cap mesh data (indices are already 0-based)
        let front_cap_mesh = null;
        let back_cap_mesh = null;

        if (front_cap_data && front_cap_data.verts.length > 0) {
            front_cap_mesh = {
                vertices: new PackedVector3Array(front_cap_data.verts),
                normals: new PackedVector3Array(front_cap_data.norms),
                uvs: new PackedVector2Array(front_cap_data.uvs),
                indices: new PackedInt32Array(front_cap_data.indices)
            };
        }

        if (back_cap_data && back_cap_data.verts.length > 0) {
            back_cap_mesh = {
                vertices: new PackedVector3Array(back_cap_data.verts),
                normals: new PackedVector3Array(back_cap_data.norms),
                uvs: new PackedVector2Array(back_cap_data.uvs),
                indices: new PackedInt32Array(back_cap_data.indices)
            };
        }

        perf.unpack = Time.get_ticks_usec() - t0;

        // Clean up temporary uniform sets
        cleanup_uniform_sets();

        // Log performance breakdown
        console.log("[PERF] compute_slicer.slice (ms): upload=" + (perf.upload_verts/1000).toFixed(2) +
            " gpu_dispatch=" + (perf.gpu_dispatch/1000).toFixed(2) +
            " gpu_sync=" + (perf.gpu_sync/1000).toFixed(2) +
            " cap_gen=" + (perf.cap_gen/1000).toFixed(2) +
            " readback=" + (perf.readback/1000).toFixed(2) +
            " unpack=" + (perf.unpack/1000).toFixed(2) +
            " | verts=" + vertex_count + " tris=" + triangle_count +
            " | boundary_edges=" + front_boundary_count);

        return {
            success: true,
            front: front_mesh,
            back: back_mesh,
            front_cap: front_cap_mesh,
            back_cap: back_cap_mesh
        };
    } catch (e) {
        // Clean up on error
        if (compute_list_is_open) {
            try {
                rd.compute_list_end();
                compute_list_is_open = false;
            } catch (e2) {
                // Ignore errors when ending compute list during cleanup
                // Will be handled in cleanup() function
            }
        }
        cleanup_uniform_sets();
        console.log("Slice error: " + e.message);
        return { success: false, error: e.message };
    }
}

export function cleanup() {
    if (!rd) return;

    // Close any open compute list first (can happen if slice was interrupted)
    if (compute_list_is_open) {
        try {
            rd.compute_list_end();
        } catch (e) {
            // Ignore - just trying to clean up
        }
        compute_list_is_open = false;
    }

    // Clean up any temporary uniform sets that might still be around
    if (vertex_uniform_set && vertex_uniform_set.is_valid()) {
        try { rd.free_rid(vertex_uniform_set); } catch (e) {}
    }
    if (triangle_uniform_set && triangle_uniform_set.is_valid()) {
        try { rd.free_rid(triangle_uniform_set); } catch (e) {}
    }
    vertex_uniform_set = null;
    triangle_uniform_set = null;

    // Free all RIDs before destroying the RenderingDevice
    // Order matters: free dependents first (pipelines use shaders, etc.)

    // Free pipelines first (they depend on shaders)
    if (vertex_pipeline && vertex_pipeline.is_valid()) rd.free_rid(vertex_pipeline);
    if (triangle_pipeline && triangle_pipeline.is_valid()) rd.free_rid(triangle_pipeline);
    if (cap_pipeline && cap_pipeline.is_valid()) rd.free_rid(cap_pipeline);

    // Free shaders
    if (vertex_shader && vertex_shader.is_valid()) rd.free_rid(vertex_shader);
    if (triangle_shader && triangle_shader.is_valid()) rd.free_rid(triangle_shader);
    if (cap_shader && cap_shader.is_valid()) rd.free_rid(cap_shader);

    // Free buffers
    if (input_vertex_buffer && input_vertex_buffer.is_valid()) rd.free_rid(input_vertex_buffer);
    if (input_normal_buffer && input_normal_buffer.is_valid()) rd.free_rid(input_normal_buffer);
    if (input_uv_buffer && input_uv_buffer.is_valid()) rd.free_rid(input_uv_buffer);
    if (input_index_buffer && input_index_buffer.is_valid()) rd.free_rid(input_index_buffer);
    if (params_buffer && params_buffer.is_valid()) rd.free_rid(params_buffer);
    if (vertex_class_buffer && vertex_class_buffer.is_valid()) rd.free_rid(vertex_class_buffer);
    if (front_vertex_buffer && front_vertex_buffer.is_valid()) rd.free_rid(front_vertex_buffer);
    if (front_normal_buffer && front_normal_buffer.is_valid()) rd.free_rid(front_normal_buffer);
    if (front_uv_buffer && front_uv_buffer.is_valid()) rd.free_rid(front_uv_buffer);
    if (front_index_buffer && front_index_buffer.is_valid()) rd.free_rid(front_index_buffer);
    if (back_vertex_buffer && back_vertex_buffer.is_valid()) rd.free_rid(back_vertex_buffer);
    if (back_normal_buffer && back_normal_buffer.is_valid()) rd.free_rid(back_normal_buffer);
    if (back_uv_buffer && back_uv_buffer.is_valid()) rd.free_rid(back_uv_buffer);
    if (back_index_buffer && back_index_buffer.is_valid()) rd.free_rid(back_index_buffer);
    if (counters_buffer && counters_buffer.is_valid()) rd.free_rid(counters_buffer);

    // Free boundary edge buffers
    if (front_boundary_buffer && front_boundary_buffer.is_valid()) rd.free_rid(front_boundary_buffer);
    if (back_boundary_buffer && back_boundary_buffer.is_valid()) rd.free_rid(back_boundary_buffer);

    // Free cap generation buffers
    if (front_cap_centroid_buffer && front_cap_centroid_buffer.is_valid()) rd.free_rid(front_cap_centroid_buffer);
    if (back_cap_centroid_buffer && back_cap_centroid_buffer.is_valid()) rd.free_rid(back_cap_centroid_buffer);
    if (cap_params_buffer && cap_params_buffer.is_valid()) rd.free_rid(cap_params_buffer);

    // Clear reference to local RenderingDevice (will be freed by ObjectRegistry)
    rd = null;

    // Reset all references
    vertex_shader = null;
    triangle_shader = null;
    cap_shader = null;
    vertex_pipeline = null;
    triangle_pipeline = null;
    cap_pipeline = null;
    input_vertex_buffer = null;
    input_normal_buffer = null;
    input_uv_buffer = null;
    input_index_buffer = null;
    params_buffer = null;
    vertex_class_buffer = null;
    front_vertex_buffer = null;
    front_normal_buffer = null;
    front_uv_buffer = null;
    front_index_buffer = null;
    back_vertex_buffer = null;
    back_normal_buffer = null;
    back_uv_buffer = null;
    back_index_buffer = null;
    counters_buffer = null;
    front_boundary_buffer = null;
    back_boundary_buffer = null;
    front_cap_centroid_buffer = null;
    back_cap_centroid_buffer = null;
    cap_params_buffer = null;
    vertex_uniform_set = null;
    triangle_uniform_set = null;
    front_cap_uniform_set = null;
    back_cap_uniform_set = null;
    compute_list_is_open = false;

    initialized = false;
    console.log("Compute slicer cleaned up");
}

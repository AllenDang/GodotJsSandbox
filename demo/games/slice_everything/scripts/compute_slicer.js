// Compute Shader-based Mesh Slicer
// Uses RenderingDevice for GPU-accelerated mesh slicing

let rd = null;  // RenderingDevice (local device for compute shaders)
let vertex_shader = null;
let triangle_shader = null;
let vertex_pipeline = null;
let triangle_pipeline = null;
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

// Uniform sets
let vertex_uniform_set = null;
let triangle_uniform_set = null;

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
    const counter_size = 16; // 4 uint

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

    // Atomic counters buffer
    counters_buffer = rd.storage_buffer_create(counter_size);

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
        vertex_uniform_set = null;
        triangle_uniform_set = null;
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

        // Reset counters
        const zero_counters = new PackedByteArray();
        zero_counters.resize(16);
        rd.buffer_update(counters_buffer, 0, 16, zero_counters);

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

        // Create uniform set for triangle shader
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

        // Sync and read back results
        // Local device supports submit/sync for blocking synchronization
        rd.submit();
        rd.sync();

        perf.gpu_sync = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // Read counters
        const counter_data = rd.buffer_get_data(counters_buffer, 0, 16);
        const front_vert_count = counter_data.decode_u32(0);
        const back_vert_count = counter_data.decode_u32(4);
        const front_idx_count = counter_data.decode_u32(8);
        const back_idx_count = counter_data.decode_u32(12);

        if (front_vert_count === 0 || back_vert_count === 0) {
            cleanup_uniform_sets();
            return { success: false, error: "Slice did not split the mesh" };
        }

        // Sanity check - don't process unreasonable counts
        if (front_vert_count > MAX_OUTPUT_VERTICES || back_vert_count > MAX_OUTPUT_VERTICES) {
            cleanup_uniform_sets();
            return { success: false, error: "Invalid vertex count from compute shader" };
        }

        // Read output data
        const front_verts_data = rd.buffer_get_data(front_vertex_buffer, 0, front_vert_count * 16);
        const front_norms_data = rd.buffer_get_data(front_normal_buffer, 0, front_vert_count * 16);
        const front_uvs_data = rd.buffer_get_data(front_uv_buffer, 0, front_vert_count * 8);
        const front_idx_data = rd.buffer_get_data(front_index_buffer, 0, front_idx_count * 4);

        const back_verts_data = rd.buffer_get_data(back_vertex_buffer, 0, back_vert_count * 16);
        const back_norms_data = rd.buffer_get_data(back_normal_buffer, 0, back_vert_count * 16);
        const back_uvs_data = rd.buffer_get_data(back_uv_buffer, 0, back_vert_count * 8);
        const back_idx_data = rd.buffer_get_data(back_index_buffer, 0, back_idx_count * 4);

        perf.readback = Time.get_ticks_usec() - t0;
        t0 = Time.get_ticks_usec();

        // Convert to Godot PackedArrays (proxy objects, not JS arrays)
        const front_mesh = {
            vertices: unpack_to_packed_vector3_array(front_verts_data, front_vert_count),
            normals: unpack_to_packed_vector3_array(front_norms_data, front_vert_count),
            uvs: unpack_to_packed_vector2_array(front_uvs_data, front_vert_count),
            indices: unpack_to_packed_int32_array(front_idx_data, front_idx_count)
        };

        const back_mesh = {
            vertices: unpack_to_packed_vector3_array(back_verts_data, back_vert_count),
            normals: unpack_to_packed_vector3_array(back_norms_data, back_vert_count),
            uvs: unpack_to_packed_vector2_array(back_uvs_data, back_vert_count),
            indices: unpack_to_packed_int32_array(back_idx_data, back_idx_count)
        };

        perf.unpack = Time.get_ticks_usec() - t0;

        // Clean up temporary uniform sets
        cleanup_uniform_sets();

        // Log performance breakdown
        console.log("[PERF] compute_slicer.slice (ms): upload=" + (perf.upload_verts/1000).toFixed(2) +
            " gpu_dispatch=" + (perf.gpu_dispatch/1000).toFixed(2) +
            " gpu_sync=" + (perf.gpu_sync/1000).toFixed(2) +
            " readback=" + (perf.readback/1000).toFixed(2) +
            " unpack=" + (perf.unpack/1000).toFixed(2) +
            " | verts=" + vertex_count + " tris=" + triangle_count);

        return {
            success: true,
            front: front_mesh,
            back: back_mesh
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

    // Free shaders
    if (vertex_shader && vertex_shader.is_valid()) rd.free_rid(vertex_shader);
    if (triangle_shader && triangle_shader.is_valid()) rd.free_rid(triangle_shader);

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

    // Clear reference to local RenderingDevice (will be freed by ObjectRegistry)
    rd = null;

    // Reset all references
    vertex_shader = null;
    triangle_shader = null;
    vertex_pipeline = null;
    triangle_pipeline = null;
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
    vertex_uniform_set = null;
    triangle_uniform_set = null;
    compute_list_is_open = false;

    initialized = false;
    console.log("Compute slicer cleaned up");
}

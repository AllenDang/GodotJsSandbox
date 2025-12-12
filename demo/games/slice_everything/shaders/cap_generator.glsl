#[compute]
#version 450

// Cap Generator Compute Shader
// Generates cap triangles from ordered boundary vertices
// Uses triangle fan from centroid to consecutive boundary vertices
// NOTE: Boundary vertices must be pre-ordered into a connected loop

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

// Input: ordered boundary vertices (forming a closed loop)
layout(set = 0, binding = 0, std430) restrict readonly buffer BoundaryVertices {
    vec4 boundary_verts[];  // sequential vertices: [v0, v1, v2, ...]
};

// Parameters
layout(set = 0, binding = 1, std140) uniform CapParams {
    vec4 plane_normal;    // xyz = slice plane normal, w = normal_direction (-1 or 1)
    vec4 cap_centroid;    // xyz = precomputed centroid of boundary, w = unused
    uint vert_count;      // number of boundary vertices (= number of triangles)
    uint base_vert_idx;   // starting index for cap vertices in output buffer
    uint base_idx_idx;    // starting index for cap indices in output buffer
    uint _pad;
};

// Output: vertices/normals/uvs/indices for cap
layout(set = 0, binding = 2, std430) restrict buffer CapVertices {
    vec4 cap_verts[];
};

layout(set = 0, binding = 3, std430) restrict buffer CapNormals {
    vec4 cap_norms[];
};

layout(set = 0, binding = 4, std430) restrict buffer CapUVs {
    vec2 cap_uvs[];
};

layout(set = 0, binding = 5, std430) restrict buffer CapIndices {
    uint cap_indices[];
};

void main() {
    uint gid = gl_GlobalInvocationID.x;

    if (gid >= vert_count) return;

    // Get this vertex and next vertex (wrapping around)
    vec3 v_curr = boundary_verts[gid].xyz;
    vec3 v_next = boundary_verts[(gid + 1) % vert_count].xyz;

    // Cap normal (plane normal with direction)
    float normal_dir = plane_normal.w;
    vec3 cap_normal = plane_normal.xyz * normal_dir;

    // Output layout for triangle fan:
    // - Vertex 0 is centroid (shared)
    // - Vertices 1..vert_count are the boundary vertices
    // - Each triangle: (centroid, v[i], v[i+1])

    // Each thread writes its boundary vertex
    uint v_idx = base_vert_idx + 1 + gid;  // +1 because centroid is at base_vert_idx

    cap_verts[v_idx] = vec4(v_curr, 1.0);
    cap_norms[v_idx] = vec4(cap_normal, 0.0);
    cap_uvs[v_idx] = vec2(0.5, 0.5);  // Simple UV at center

    // First thread also writes centroid vertex
    if (gid == 0) {
        cap_verts[base_vert_idx] = vec4(cap_centroid.xyz, 1.0);
        cap_norms[base_vert_idx] = vec4(cap_normal, 0.0);
        cap_uvs[base_vert_idx] = vec2(0.5, 0.5);
    }

    // Write triangle indices for this segment of the fan
    // Triangle: centroid -> v_curr -> v_next
    uint tri_base = base_idx_idx + gid * 3;
    uint centroid_idx = base_vert_idx;
    uint curr_idx = base_vert_idx + 1 + gid;
    uint next_idx = base_vert_idx + 1 + ((gid + 1) % vert_count);

    // Boundary vertices are ordered CCW when viewed from cap normal direction
    // For CCW triangles (front-facing when viewed from normal): centroid -> curr -> next
    // Since we ensured CCW winding on CPU, we always use the same triangle order
    // The normal direction doesn't affect winding - it was used for winding correction on CPU
    cap_indices[tri_base] = centroid_idx;
    cap_indices[tri_base + 1] = curr_idx;
    cap_indices[tri_base + 2] = next_idx;
}

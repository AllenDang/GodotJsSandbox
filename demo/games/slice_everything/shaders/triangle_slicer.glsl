#[compute]
#version 450

// Triangle Slicer Compute Shader
// Processes triangles and generates sliced output

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

// Input vertex data
layout(set = 0, binding = 0, std430) restrict readonly buffer InputVertices {
    vec4 vertices[];
};

layout(set = 0, binding = 1, std430) restrict readonly buffer InputNormals {
    vec4 normals[];
};

layout(set = 0, binding = 2, std430) restrict readonly buffer InputUVs {
    vec2 uvs[];
};

layout(set = 0, binding = 3, std430) restrict readonly buffer InputIndices {
    uint indices[];
};

// Slicing parameters
layout(set = 0, binding = 4, std140) uniform SliceParams {
    vec4 plane_normal;
    vec4 plane_point;
    uint triangle_count;
    uint vertex_count;
    uint _pad1;
    uint _pad2;
};

// Vertex classification from first pass
layout(set = 0, binding = 5, std430) restrict readonly buffer VertexClassification {
    int vertex_class[];
};

// Output vertices for front mesh
layout(set = 0, binding = 6, std430) restrict writeonly buffer FrontVertices {
    vec4 front_verts[];
};

layout(set = 0, binding = 7, std430) restrict writeonly buffer FrontNormals {
    vec4 front_norms[];
};

layout(set = 0, binding = 8, std430) restrict writeonly buffer FrontUVs {
    vec2 front_uvs[];
};

layout(set = 0, binding = 9, std430) restrict writeonly buffer FrontIndices {
    uint front_indices[];
};

// Output vertices for back mesh
layout(set = 0, binding = 10, std430) restrict writeonly buffer BackVertices {
    vec4 back_verts[];
};

layout(set = 0, binding = 11, std430) restrict writeonly buffer BackNormals {
    vec4 back_norms[];
};

layout(set = 0, binding = 12, std430) restrict writeonly buffer BackUVs {
    vec2 back_uvs[];
};

layout(set = 0, binding = 13, std430) restrict writeonly buffer BackIndices {
    uint back_indices[];
};

// Atomic counters
layout(set = 0, binding = 14, std430) restrict buffer AtomicCounters {
    uint front_vert_count;
    uint back_vert_count;
    uint front_idx_count;
    uint back_idx_count;
};

const float EPSILON = 0.001;

float point_plane_distance(vec3 point) {
    return dot(point - plane_point.xyz, plane_normal.xyz);
}

// Compute intersection point between edge and plane
float edge_plane_t(vec3 v1, vec3 v2) {
    float d1 = point_plane_distance(v1);
    float d2 = point_plane_distance(v2);
    float denom = d1 - d2;
    if (abs(denom) < EPSILON) return 0.5;
    return clamp(d1 / denom, 0.0, 1.0);
}

// Add a vertex to front mesh and return its index
uint add_front_vertex(vec3 pos, vec3 norm, vec2 uv) {
    uint idx = atomicAdd(front_vert_count, 1);
    front_verts[idx] = vec4(pos, 1.0);
    front_norms[idx] = vec4(norm, 0.0);
    front_uvs[idx] = uv;
    return idx;
}

// Add a vertex to back mesh and return its index
uint add_back_vertex(vec3 pos, vec3 norm, vec2 uv) {
    uint idx = atomicAdd(back_vert_count, 1);
    back_verts[idx] = vec4(pos, 1.0);
    back_norms[idx] = vec4(norm, 0.0);
    back_uvs[idx] = uv;
    return idx;
}

// Add triangle indices to front mesh
void add_front_triangle(uint i0, uint i1, uint i2) {
    uint base = atomicAdd(front_idx_count, 3);
    front_indices[base] = i0;
    front_indices[base + 1] = i1;
    front_indices[base + 2] = i2;
}

// Add triangle indices to back mesh
void add_back_triangle(uint i0, uint i1, uint i2) {
    uint base = atomicAdd(back_idx_count, 3);
    back_indices[base] = i0;
    back_indices[base + 1] = i1;
    back_indices[base + 2] = i2;
}

// Interpolate vertex attributes
void lerp_vertex(uint idx1, uint idx2, float t,
                 out vec3 pos, out vec3 norm, out vec2 uv) {
    pos = mix(vertices[idx1].xyz, vertices[idx2].xyz, t);
    norm = normalize(mix(normals[idx1].xyz, normals[idx2].xyz, t));
    uv = mix(uvs[idx1], uvs[idx2], t);
}

void main() {
    uint tri_id = gl_GlobalInvocationID.x;
    if (tri_id >= triangle_count) return;

    // Get triangle vertex indices
    uint i0 = indices[tri_id * 3];
    uint i1 = indices[tri_id * 3 + 1];
    uint i2 = indices[tri_id * 3 + 2];

    // Get vertex classifications
    int c0 = vertex_class[i0];
    int c1 = vertex_class[i1];
    int c2 = vertex_class[i2];

    // Count vertices on each side
    int front_count = (c0 > 0 ? 1 : 0) + (c1 > 0 ? 1 : 0) + (c2 > 0 ? 1 : 0);
    int back_count = (c0 < 0 ? 1 : 0) + (c1 < 0 ? 1 : 0) + (c2 < 0 ? 1 : 0);

    // Get vertex data
    vec3 v0 = vertices[i0].xyz;
    vec3 v1 = vertices[i1].xyz;
    vec3 v2 = vertices[i2].xyz;
    vec3 n0 = normals[i0].xyz;
    vec3 n1 = normals[i1].xyz;
    vec3 n2 = normals[i2].xyz;
    vec2 uv0 = uvs[i0];
    vec2 uv1 = uvs[i1];
    vec2 uv2 = uvs[i2];

    // Case 1: All vertices on front side (or on plane)
    if (back_count == 0) {
        uint fi0 = add_front_vertex(v0, n0, uv0);
        uint fi1 = add_front_vertex(v1, n1, uv1);
        uint fi2 = add_front_vertex(v2, n2, uv2);
        add_front_triangle(fi0, fi1, fi2);
        return;
    }

    // Case 2: All vertices on back side (or on plane)
    if (front_count == 0) {
        uint bi0 = add_back_vertex(v0, n0, uv0);
        uint bi1 = add_back_vertex(v1, n1, uv1);
        uint bi2 = add_back_vertex(v2, n2, uv2);
        add_back_triangle(bi0, bi1, bi2);
        return;
    }

    // Case 3: Triangle is split by the plane
    // Find the lone vertex (the one on a different side than the other two)
    int lone_idx = -1;
    int lone_side = 0;

    // Determine which vertex is alone
    if (c0 != 0 && c0 != c1 && c0 != c2) {
        lone_idx = 0;
        lone_side = c0;
    } else if (c1 != 0 && c1 != c0 && c1 != c2) {
        lone_idx = 1;
        lone_side = c1;
    } else if (c2 != 0 && c2 != c0 && c2 != c1) {
        lone_idx = 2;
        lone_side = c2;
    }

    if (lone_idx < 0) {
        // Edge case: one vertex on plane, others on opposite sides
        // Just assign to front for simplicity
        uint fi0 = add_front_vertex(v0, n0, uv0);
        uint fi1 = add_front_vertex(v1, n1, uv1);
        uint fi2 = add_front_vertex(v2, n2, uv2);
        add_front_triangle(fi0, fi1, fi2);
        return;
    }

    // Reorder vertices so lone vertex is first
    vec3 vLone, vA, vB;
    vec3 nLone, nA, nB;
    vec2 uvLone, uvA, uvB;
    uint iLone, iA, iB;

    if (lone_idx == 0) {
        vLone = v0; vA = v1; vB = v2;
        nLone = n0; nA = n1; nB = n2;
        uvLone = uv0; uvA = uv1; uvB = uv2;
        iLone = i0; iA = i1; iB = i2;
    } else if (lone_idx == 1) {
        vLone = v1; vA = v2; vB = v0;
        nLone = n1; nA = n2; nB = n0;
        uvLone = uv1; uvA = uv2; uvB = uv0;
        iLone = i1; iA = i2; iB = i0;
    } else {
        vLone = v2; vA = v0; vB = v1;
        nLone = n2; nA = n0; nB = n1;
        uvLone = uv2; uvA = uv0; uvB = uv1;
        iLone = i2; iA = i0; iB = i1;
    }

    // Compute intersection points
    float tA = edge_plane_t(vLone, vA);
    float tB = edge_plane_t(vLone, vB);

    vec3 intA_pos = mix(vLone, vA, tA);
    vec3 intA_norm = normalize(mix(nLone, nA, tA));
    vec2 intA_uv = mix(uvLone, uvA, tA);

    vec3 intB_pos = mix(vLone, vB, tB);
    vec3 intB_norm = normalize(mix(nLone, nB, tB));
    vec2 intB_uv = mix(uvLone, uvB, tB);

    // Create triangles (no caps - caps are generated in JS using all intersection points)
    if (lone_side > 0) {
        // Lone vertex is on front: 1 front triangle, 2 back triangles
        uint f_lone = add_front_vertex(vLone, nLone, uvLone);
        uint f_intA = add_front_vertex(intA_pos, intA_norm, intA_uv);
        uint f_intB = add_front_vertex(intB_pos, intB_norm, intB_uv);
        add_front_triangle(f_lone, f_intA, f_intB);

        uint b_intA = add_back_vertex(intA_pos, intA_norm, intA_uv);
        uint b_A = add_back_vertex(vA, nA, uvA);
        uint b_B = add_back_vertex(vB, nB, uvB);
        uint b_intB = add_back_vertex(intB_pos, intB_norm, intB_uv);
        add_back_triangle(b_intA, b_A, b_B);
        add_back_triangle(b_intA, b_B, b_intB);
    } else {
        // Lone vertex is on back: 1 back triangle, 2 front triangles
        uint b_lone = add_back_vertex(vLone, nLone, uvLone);
        uint b_intA = add_back_vertex(intA_pos, intA_norm, intA_uv);
        uint b_intB = add_back_vertex(intB_pos, intB_norm, intB_uv);
        add_back_triangle(b_lone, b_intA, b_intB);

        uint f_intA = add_front_vertex(intA_pos, intA_norm, intA_uv);
        uint f_A = add_front_vertex(vA, nA, uvA);
        uint f_B = add_front_vertex(vB, nB, uvB);
        uint f_intB = add_front_vertex(intB_pos, intB_norm, intB_uv);
        add_front_triangle(f_intA, f_A, f_B);
        add_front_triangle(f_intA, f_B, f_intB);
    }
}

#[compute]
#version 450

// Mesh Slicer Compute Shader
// Classifies triangles relative to a slicing plane and outputs front/back meshes

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

// Input vertex data (read-only)
layout(set = 0, binding = 0, std430) restrict readonly buffer InputVertices {
    vec4 vertices[];  // xyz = position, w = unused
};

layout(set = 0, binding = 1, std430) restrict readonly buffer InputNormals {
    vec4 normals[];  // xyz = normal, w = unused
};

layout(set = 0, binding = 2, std430) restrict readonly buffer InputUVs {
    vec2 uvs[];
};

layout(set = 0, binding = 3, std430) restrict readonly buffer InputIndices {
    uint indices[];
};

// Slicing parameters
layout(set = 0, binding = 4, std140) uniform SliceParams {
    vec4 plane_normal;  // xyz = normal, w = unused
    vec4 plane_point;   // xyz = point on plane, w = unused
    uint triangle_count;
    uint vertex_count;
    uint _pad1;
    uint _pad2;
};

// Output: vertex classification (-1 = back, 0 = on plane, 1 = front)
layout(set = 0, binding = 5, std430) restrict writeonly buffer VertexClassification {
    int vertex_class[];
};

const float EPSILON = 0.001;

// Signed distance from point to plane
float point_plane_distance(vec3 point) {
    return dot(point - plane_point.xyz, plane_normal.xyz);
}

// Classify a point: 1 = front, -1 = back, 0 = on plane
int classify_point(vec3 point) {
    float d = point_plane_distance(point);
    if (d > EPSILON) return 1;
    if (d < -EPSILON) return -1;
    return 0;
}

void main() {
    uint gid = gl_GlobalInvocationID.x;

    // First pass: classify vertices
    if (gid < vertex_count) {
        vec3 pos = vertices[gid].xyz;
        vertex_class[gid] = classify_point(pos);
    }

    // Second dispatch will handle triangles
    // We use a separate dispatch for triangles to ensure vertex classification is complete
}

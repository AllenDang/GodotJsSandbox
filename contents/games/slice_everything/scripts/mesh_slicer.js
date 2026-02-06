// Mesh Slicer - Procedural mesh slicing algorithm with UV and normal support
// This module handles the geometric calculations for slicing meshes along a plane

// Vector3 math utilities (since we use {x,y,z} objects)
export var Vec3 = {
    add: function(a, b) {
        return { x: a.x + b.x, y: a.y + b.y, z: a.z + b.z };
    },
    sub: function(a, b) {
        return { x: a.x - b.x, y: a.y - b.y, z: a.z - b.z };
    },
    mul: function(v, s) {
        return { x: v.x * s, y: v.y * s, z: v.z * s };
    },
    dot: function(a, b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    },
    cross: function(a, b) {
        return {
            x: a.y * b.z - a.z * b.y,
            y: a.z * b.x - a.x * b.z,
            z: a.x * b.y - a.y * b.x
        };
    },
    length: function(v) {
        return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    },
    normalize: function(v) {
        var len = Vec3.length(v);
        if (len < 0.0001) return { x: 0, y: 1, z: 0 };
        return { x: v.x / len, y: v.y / len, z: v.z / len };
    },
    lerp: function(a, b, t) {
        return {
            x: a.x + (b.x - a.x) * t,
            y: a.y + (b.y - a.y) * t,
            z: a.z + (b.z - a.z) * t
        };
    },
    copy: function(v) {
        return { x: v.x, y: v.y, z: v.z };
    },
    equals: function(a, b, epsilon) {
        epsilon = epsilon || 0.0001;
        return Math.abs(a.x - b.x) < epsilon &&
               Math.abs(a.y - b.y) < epsilon &&
               Math.abs(a.z - b.z) < epsilon;
    }
};

// UV (2D) lerp
function lerp_uv(uv1, uv2, t) {
    return {
        x: uv1.x + (uv2.x - uv1.x) * t,
        y: uv1.y + (uv2.y - uv1.y) * t
    };
}

function copy_uv(uv) {
    return { x: uv.x, y: uv.y };
}

// Calculate signed distance from point to plane
export function point_plane_distance(point, plane_normal, plane_point) {
    var diff = Vec3.sub(point, plane_point);
    return Vec3.dot(diff, plane_normal);
}

// Find intersection point and interpolation factor between edge and plane
function edge_plane_intersection_t(v1, v2, plane_normal, plane_point) {
    var d1 = point_plane_distance(v1, plane_normal, plane_point);
    var d2 = point_plane_distance(v2, plane_normal, plane_point);

    var denom = d1 - d2;
    if (Math.abs(denom) < 0.0001) {
        return 0.5;
    }

    var t = d1 / denom;
    return Math.max(0, Math.min(1, t));
}

// Classify a vertex relative to the plane
// Returns: 1 = front, -1 = back, 0 = on plane
export function classify_vertex(v, plane_normal, plane_point, epsilon) {
    epsilon = epsilon || 0.001;
    var d = point_plane_distance(v, plane_normal, plane_point);
    if (d > epsilon) return 1;
    if (d < -epsilon) return -1;
    return 0;
}

// Vertex with position, UV, and normal
function make_vertex(pos, uv, normal) {
    return {
        pos: Vec3.copy(pos),
        uv: uv ? copy_uv(uv) : { x: 0, y: 0 },
        normal: normal ? Vec3.copy(normal) : { x: 0, y: 1, z: 0 }
    };
}

// Interpolate between two vertices (position, UV, and normal)
function lerp_vertex(v1, v2, t) {
    // Interpolate normal and renormalize
    var lerped_normal = Vec3.lerp(v1.normal, v2.normal, t);
    return {
        pos: Vec3.lerp(v1.pos, v2.pos, t),
        uv: lerp_uv(v1.uv, v2.uv, t),
        normal: Vec3.normalize(lerped_normal)
    };
}

// Slice a single triangle with UV support
// Each vertex is { pos: {x,y,z}, uv: {x,y} }
function slice_triangle_with_uv(vert0, vert1, vert2, plane_normal, plane_point) {
    var result = {
        front: [],
        back: [],
        cut_edge: null
    };

    var c0 = classify_vertex(vert0.pos, plane_normal, plane_point);
    var c1 = classify_vertex(vert1.pos, plane_normal, plane_point);
    var c2 = classify_vertex(vert2.pos, plane_normal, plane_point);

    var front_count = (c0 > 0 ? 1 : 0) + (c1 > 0 ? 1 : 0) + (c2 > 0 ? 1 : 0);
    var back_count = (c0 < 0 ? 1 : 0) + (c1 < 0 ? 1 : 0) + (c2 < 0 ? 1 : 0);

    // All on front
    if (back_count === 0) {
        result.front.push([vert0, vert1, vert2]);
        return result;
    }

    // All on back
    if (front_count === 0) {
        result.back.push([vert0, vert1, vert2]);
        return result;
    }

    // Triangle is split
    var verts = [vert0, vert1, vert2];
    var classes = [c0, c1, c2];

    // Find the lone vertex
    var lone_idx = -1;
    var lone_side = 0;

    for (var i = 0; i < 3; i++) {
        var next = (i + 1) % 3;
        var prev = (i + 2) % 3;

        if (classes[i] !== 0) {
            var same_as_next = (classes[i] === classes[next]) || classes[next] === 0;
            var same_as_prev = (classes[i] === classes[prev]) || classes[prev] === 0;

            if (!same_as_next && !same_as_prev) {
                lone_idx = i;
                lone_side = classes[i];
                break;
            }
        }
    }

    // Handle case where one vertex is on the plane
    if (lone_idx === -1) {
        for (var j = 0; j < 3; j++) {
            if (classes[j] === 0) {
                var next_j = (j + 1) % 3;
                var prev_j = (j + 2) % 3;

                if (classes[next_j] !== classes[prev_j] && classes[next_j] !== 0 && classes[prev_j] !== 0) {
                    var t = edge_plane_intersection_t(verts[next_j].pos, verts[prev_j].pos, plane_normal, plane_point);
                    var int_vert = lerp_vertex(verts[next_j], verts[prev_j], t);

                    if (classes[next_j] > 0) {
                        result.front.push([verts[j], verts[next_j], int_vert]);
                        result.back.push([verts[j], int_vert, verts[prev_j]]);
                    } else {
                        result.back.push([verts[j], verts[next_j], int_vert]);
                        result.front.push([verts[j], int_vert, verts[prev_j]]);
                    }
                    result.cut_edge = [verts[j].pos, int_vert.pos];
                    return result;
                }
            }
        }

        // Fallback
        if (front_count >= back_count) {
            result.front.push([vert0, vert1, vert2]);
        } else {
            result.back.push([vert0, vert1, vert2]);
        }
        return result;
    }

    // Standard case: one vertex alone
    var lone_v = verts[lone_idx];
    var v_a = verts[(lone_idx + 1) % 3];
    var v_b = verts[(lone_idx + 2) % 3];

    var t_a = edge_plane_intersection_t(lone_v.pos, v_a.pos, plane_normal, plane_point);
    var t_b = edge_plane_intersection_t(lone_v.pos, v_b.pos, plane_normal, plane_point);

    var int_a = lerp_vertex(lone_v, v_a, t_a);
    var int_b = lerp_vertex(lone_v, v_b, t_b);

    result.cut_edge = [int_a.pos, int_b.pos];

    var lone_tri = [lone_v, int_a, int_b];
    var other_tri1 = [int_a, v_a, v_b];
    var other_tri2 = [int_a, v_b, int_b];

    if (lone_side > 0) {
        result.front.push(lone_tri);
        result.back.push(other_tri1);
        result.back.push(other_tri2);
    } else {
        result.back.push(lone_tri);
        result.front.push(other_tri1);
        result.front.push(other_tri2);
    }

    return result;
}

// Generate cap triangles for the cut surface
function generate_cap_triangles(cut_edges, plane_normal, flip_winding) {
    if (cut_edges.length < 3) return [];

    var points = [];
    var epsilon = 0.001;

    for (var i = 0; i < cut_edges.length; i++) {
        var edge = cut_edges[i];
        for (var j = 0; j < 2; j++) {
            var p = edge[j];
            var found = false;
            for (var k = 0; k < points.length; k++) {
                if (Vec3.equals(points[k], p, epsilon)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                points.push(Vec3.copy(p));
            }
        }
    }

    if (points.length < 3) return [];

    var centroid = { x: 0, y: 0, z: 0 };
    for (var m = 0; m < points.length; m++) {
        centroid.x += points[m].x;
        centroid.y += points[m].y;
        centroid.z += points[m].z;
    }
    centroid.x /= points.length;
    centroid.y /= points.length;
    centroid.z /= points.length;

    var up = { x: 0, y: 1, z: 0 };
    if (Math.abs(Vec3.dot(plane_normal, up)) > 0.9) {
        up = { x: 1, y: 0, z: 0 };
    }
    var tangent = Vec3.normalize(Vec3.cross(plane_normal, up));
    var bitangent = Vec3.cross(plane_normal, tangent);

    var projected = [];
    for (var n = 0; n < points.length; n++) {
        var rel = Vec3.sub(points[n], centroid);
        var u = Vec3.dot(rel, tangent);
        var v = Vec3.dot(rel, bitangent);
        var angle = Math.atan2(v, u);
        projected.push({ point: points[n], angle: angle });
    }

    projected.sort(function(a, b) { return a.angle - b.angle; });

    var triangles = [];
    // Cap triangles use a simple UV (centered at 0.5, 0.5)
    var center_uv = { x: 0.5, y: 0.5 };

    // Cap normal is the plane normal (flipped for back cap)
    var cap_normal = flip_winding
        ? { x: -plane_normal.x, y: -plane_normal.y, z: -plane_normal.z }
        : Vec3.copy(plane_normal);

    for (var t = 0; t < projected.length; t++) {
        var next_t = (t + 1) % projected.length;

        // Calculate UV based on position relative to centroid
        var rel1 = Vec3.sub(projected[t].point, centroid);
        var rel2 = Vec3.sub(projected[next_t].point, centroid);
        var uv1 = { x: 0.5 + Vec3.dot(rel1, tangent) * 0.5, y: 0.5 + Vec3.dot(rel1, bitangent) * 0.5 };
        var uv2 = { x: 0.5 + Vec3.dot(rel2, tangent) * 0.5, y: 0.5 + Vec3.dot(rel2, bitangent) * 0.5 };

        if (flip_winding) {
            triangles.push([
                make_vertex(centroid, center_uv, cap_normal),
                make_vertex(projected[next_t].point, uv2, cap_normal),
                make_vertex(projected[t].point, uv1, cap_normal)
            ]);
        } else {
            triangles.push([
                make_vertex(centroid, center_uv, cap_normal),
                make_vertex(projected[t].point, uv1, cap_normal),
                make_vertex(projected[next_t].point, uv2, cap_normal)
            ]);
        }
    }

    return triangles;
}

// Main slicing function with UV and normal support
// vertices: array of {x,y,z}
// uvs: array of {x,y} (same length as vertices, or null)
// normals: array of {x,y,z} (same length as vertices, or null)
// indices: array of integers (triangle indices)
export function slice_mesh(vertices, indices, plane_normal, plane_point, uvs, normals) {
    var front_triangles = [];
    var back_triangles = [];
    var cut_edges = [];

    // Check if UVs and normals are provided
    var has_uvs = uvs && uvs.length === vertices.length;
    var has_normals = normals && normals.length === vertices.length;

    // Default values for missing data
    var default_uv = { x: 0, y: 0 };
    var default_normal = { x: 0, y: 1, z: 0 };

    // Pre-calculate plane normal components for faster access
    var pnx = plane_normal.x;
    var pny = plane_normal.y;
    var pnz = plane_normal.z;

    var idx_len = indices.length;
    for (var i = 0; i < idx_len; i += 3) {
        var idx0 = indices[i];
        var idx1 = indices[i + 1];
        var idx2 = indices[i + 2];

        // Get source vertex data
        var v0 = vertices[idx0];
        var v1 = vertices[idx1];
        var v2 = vertices[idx2];

        // Create vertex objects inline to reduce function call overhead
        var vert0 = {
            pos: { x: v0.x, y: v0.y, z: v0.z },
            uv: has_uvs ? { x: uvs[idx0].x, y: uvs[idx0].y } : default_uv,
            normal: has_normals ? { x: normals[idx0].x, y: normals[idx0].y, z: normals[idx0].z } : default_normal
        };
        var vert1 = {
            pos: { x: v1.x, y: v1.y, z: v1.z },
            uv: has_uvs ? { x: uvs[idx1].x, y: uvs[idx1].y } : default_uv,
            normal: has_normals ? { x: normals[idx1].x, y: normals[idx1].y, z: normals[idx1].z } : default_normal
        };
        var vert2 = {
            pos: { x: v2.x, y: v2.y, z: v2.z },
            uv: has_uvs ? { x: uvs[idx2].x, y: uvs[idx2].y } : default_uv,
            normal: has_normals ? { x: normals[idx2].x, y: normals[idx2].y, z: normals[idx2].z } : default_normal
        };

        var sliced = slice_triangle_with_uv(vert0, vert1, vert2, plane_normal, plane_point);

        // Add results using concat-style for fewer operations
        var sf = sliced.front;
        var sb = sliced.back;
        for (var f = 0; f < sf.length; f++) {
            front_triangles.push(sf[f]);
        }
        for (var b = 0; b < sb.length; b++) {
            back_triangles.push(sb[b]);
        }

        if (sliced.cut_edge) {
            cut_edges.push(sliced.cut_edge);
        }
    }

    if (front_triangles.length === 0 || back_triangles.length === 0) {
        return { success: false };
    }

    // Generate cap triangles
    var front_cap = generate_cap_triangles(cut_edges, plane_normal, false);
    var back_cap = generate_cap_triangles(cut_edges, plane_normal, true);

    for (var fc = 0; fc < front_cap.length; fc++) {
        front_triangles.push(front_cap[fc]);
    }
    for (var bc = 0; bc < back_cap.length; bc++) {
        back_triangles.push(back_cap[bc]);
    }

    var front_mesh = triangles_to_mesh_with_uv_and_normals(front_triangles);
    var back_mesh = triangles_to_mesh_with_uv_and_normals(back_triangles);

    return {
        success: true,
        front: front_mesh,
        back: back_mesh
    };
}

// Convert triangle list to indexed mesh with UVs and normals
// Optimized to reduce object allocations
function triangles_to_mesh_with_uv_and_normals(triangles) {
    var tri_count = triangles.length;
    var vert_count = tri_count * 3;

    // Pre-allocate arrays for better performance
    var vertices = new Array(vert_count);
    var uvs = new Array(vert_count);
    var normals = new Array(vert_count);
    var indices = new Array(vert_count);

    for (var i = 0; i < tri_count; i++) {
        var tri = triangles[i];
        var base_idx = i * 3;

        // Vertex 0
        var v0 = tri[0];
        vertices[base_idx] = { x: v0.pos.x, y: v0.pos.y, z: v0.pos.z };
        uvs[base_idx] = { x: v0.uv.x, y: v0.uv.y };
        normals[base_idx] = { x: v0.normal.x, y: v0.normal.y, z: v0.normal.z };
        indices[base_idx] = base_idx;

        // Vertex 1
        var v1 = tri[1];
        vertices[base_idx + 1] = { x: v1.pos.x, y: v1.pos.y, z: v1.pos.z };
        uvs[base_idx + 1] = { x: v1.uv.x, y: v1.uv.y };
        normals[base_idx + 1] = { x: v1.normal.x, y: v1.normal.y, z: v1.normal.z };
        indices[base_idx + 1] = base_idx + 1;

        // Vertex 2
        var v2 = tri[2];
        vertices[base_idx + 2] = { x: v2.pos.x, y: v2.pos.y, z: v2.pos.z };
        uvs[base_idx + 2] = { x: v2.uv.x, y: v2.uv.y };
        normals[base_idx + 2] = { x: v2.normal.x, y: v2.normal.y, z: v2.normal.z };
        indices[base_idx + 2] = base_idx + 2;
    }

    return { vertices: vertices, uvs: uvs, normals: normals, indices: indices };
}

// Legacy function without UV (for backwards compatibility)
export function slice_mesh_simple(vertices, indices, plane_normal, plane_point) {
    return slice_mesh(vertices, indices, plane_normal, plane_point, null);
}

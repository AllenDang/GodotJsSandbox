// Test: PackedArray append_array and as_vector3_array behavior
// Specifically tests the flow used in compute_slicer.js for GPU mesh slicing with caps

console.log('=== PackedArray Append Test ===');

var results = [];
var passed = 0;
var failed = 0;

function test(name, fn) {
    try {
        var result = fn();
        if (result === true) {
            results.push({ name: name, passed: true });
            passed++;
        } else {
            results.push({ name: name, passed: false, error: 'Assertion failed: ' + result });
            failed++;
        }
    } catch (e) {
        results.push({ name: name, passed: false, error: e.message });
        failed++;
    }
}

function vecEquals(a, b, eps) {
    eps = eps || 0.0001;
    return Math.abs(a.x - b.x) < eps &&
           Math.abs(a.y - b.y) < eps &&
           Math.abs(a.z - b.z) < eps;
}

// ============================================================================
// Test 1: Basic as_vector3_array conversion
// ============================================================================

test('PackedByteArray.as_vector3_array creates valid proxy', function() {
    var bytes = new PackedByteArray();
    bytes.resize(3 * 16);  // 3 vertices, 16 bytes each (vec4 format)

    // Write vertex (1,2,3)
    bytes.encode_float(0, 1.0);
    bytes.encode_float(4, 2.0);
    bytes.encode_float(8, 3.0);
    bytes.encode_float(12, 0.0);

    // Write vertex (4,5,6)
    bytes.encode_float(16, 4.0);
    bytes.encode_float(20, 5.0);
    bytes.encode_float(24, 6.0);
    bytes.encode_float(28, 0.0);

    // Write vertex (7,8,9)
    bytes.encode_float(32, 7.0);
    bytes.encode_float(36, 8.0);
    bytes.encode_float(40, 9.0);
    bytes.encode_float(44, 0.0);

    var vertices = bytes.as_vector3_array(3);

    if (vertices.length !== 3) return 'length should be 3, got ' + vertices.length;
    if (vertices.__packed_handle === undefined) return 'missing __packed_handle';

    var v0 = vertices[0];
    var v1 = vertices[1];
    var v2 = vertices[2];

    if (!vecEquals(v0, {x:1, y:2, z:3})) return 'v0 wrong: ' + JSON.stringify(v0);
    if (!vecEquals(v1, {x:4, y:5, z:6})) return 'v1 wrong: ' + JSON.stringify(v1);
    if (!vecEquals(v2, {x:7, y:8, z:9})) return 'v2 wrong: ' + JSON.stringify(v2);

    return true;
});

// ============================================================================
// Test 2: append_array on PackedVector3Array proxy
// ============================================================================

test('PackedVector3Array.append_array appends data correctly', function() {
    // Create initial array from bytes (like GPU output)
    var bytes = new PackedByteArray();
    bytes.resize(2 * 16);
    bytes.encode_float(0, 1.0); bytes.encode_float(4, 2.0); bytes.encode_float(8, 3.0); bytes.encode_float(12, 0.0);
    bytes.encode_float(16, 4.0); bytes.encode_float(20, 5.0); bytes.encode_float(24, 6.0); bytes.encode_float(28, 0.0);

    var vertices = bytes.as_vector3_array(2);
    var origHandle = vertices.__packed_handle;

    console.log('  Initial handle: ' + origHandle + ', length: ' + vertices.length);

    // Append new vertices (like cap vertices)
    var capVerts = [
        { x: 10.0, y: 11.0, z: 12.0 },
        { x: 13.0, y: 14.0, z: 15.0 }
    ];

    vertices.append_array(capVerts);

    console.log('  After append handle: ' + vertices.__packed_handle + ', length: ' + vertices.length);

    if (vertices.__packed_handle !== origHandle) return 'handle changed from ' + origHandle + ' to ' + vertices.__packed_handle;
    if (vertices.length !== 4) return 'expected length 4, got ' + vertices.length;

    // Read back ALL vertices
    var v0 = vertices[0];
    var v1 = vertices[1];
    var v2 = vertices[2];
    var v3 = vertices[3];

    console.log('  v0: ' + JSON.stringify(v0));
    console.log('  v1: ' + JSON.stringify(v1));
    console.log('  v2: ' + JSON.stringify(v2));
    console.log('  v3: ' + JSON.stringify(v3));

    if (!vecEquals(v0, {x:1, y:2, z:3})) return 'v0 wrong after append: ' + JSON.stringify(v0);
    if (!vecEquals(v1, {x:4, y:5, z:6})) return 'v1 wrong after append: ' + JSON.stringify(v1);
    if (!vecEquals(v2, {x:10, y:11, z:12})) return 'v2 (appended) wrong: ' + JSON.stringify(v2);
    if (!vecEquals(v3, {x:13, y:14, z:15})) return 'v3 (appended) wrong: ' + JSON.stringify(v3);

    return true;
});

// ============================================================================
// Test 3: Append then pass to ArrayMesh (the critical path)
// ============================================================================

test('Appended PackedVector3Array passed to ArrayMesh correctly', function() {
    // Create initial vertices from bytes
    var bytes = new PackedByteArray();
    bytes.resize(2 * 16);
    bytes.encode_float(0, 1.0); bytes.encode_float(4, 2.0); bytes.encode_float(8, 3.0); bytes.encode_float(12, 0.0);
    bytes.encode_float(16, 4.0); bytes.encode_float(20, 5.0); bytes.encode_float(24, 6.0); bytes.encode_float(28, 0.0);

    var vertices = bytes.as_vector3_array(2);

    // Create normals
    var normBytes = new PackedByteArray();
    normBytes.resize(4 * 16);
    for (var i = 0; i < 4; i++) {
        normBytes.encode_float(i * 16, 0.0);
        normBytes.encode_float(i * 16 + 4, 1.0);
        normBytes.encode_float(i * 16 + 8, 0.0);
        normBytes.encode_float(i * 16 + 12, 0.0);
    }
    var normals = normBytes.as_vector3_array(2);

    // Create UVs
    var uvBytes = new PackedByteArray();
    uvBytes.resize(4 * 8);
    for (var i = 0; i < 4; i++) {
        uvBytes.encode_float(i * 8, 0.5);
        uvBytes.encode_float(i * 8 + 4, 0.5);
    }
    var uvs = uvBytes.as_vector2_array(2);

    // Append cap data
    vertices.append_array([
        { x: 10.0, y: 11.0, z: 12.0 },
        { x: 13.0, y: 14.0, z: 15.0 }
    ]);
    normals.append_array([
        { x: 0.0, y: 1.0, z: 0.0 },
        { x: 0.0, y: 1.0, z: 0.0 }
    ]);
    uvs.append_array([
        { x: 0.5, y: 0.5 },
        { x: 0.5, y: 0.5 }
    ]);

    console.log('  Before ArrayMesh: verts=' + vertices.length + ' norms=' + normals.length + ' uvs=' + uvs.length);

    // Create indices for 1 triangle using vertices 0,2,3
    var indices = new PackedInt32Array([0, 2, 3]);

    // Create ArrayMesh
    var mesh = new ArrayMesh();
    var arrays = [];
    arrays[0] = vertices;    // ARRAY_VERTEX
    arrays[1] = normals;     // ARRAY_NORMAL
    arrays[4] = uvs;         // ARRAY_TEX_UV
    arrays[12] = indices;    // ARRAY_INDEX

    mesh.add_surface_from_arrays(3, arrays);  // PRIMITIVE_TRIANGLES

    // Extract and verify
    var extracted = mesh.surface_get_arrays(0);
    var extractedVerts = extracted[0];

    console.log('  Extracted verts length: ' + extractedVerts.length);

    if (extractedVerts.length !== 4) return 'expected 4 verts in mesh, got ' + extractedVerts.length;

    // Check the appended vertices
    var ev2 = extractedVerts[2];
    var ev3 = extractedVerts[3];

    console.log('  Extracted v2: ' + JSON.stringify(ev2));
    console.log('  Extracted v3: ' + JSON.stringify(ev3));

    if (!vecEquals(ev2, {x:10, y:11, z:12})) return 'extracted v2 wrong: ' + JSON.stringify(ev2);
    if (!vecEquals(ev3, {x:13, y:14, z:15})) return 'extracted v3 wrong: ' + JSON.stringify(ev3);

    return true;
});

// ============================================================================
// Test 4: Multiple appends
// ============================================================================

test('Multiple append_array calls accumulate correctly', function() {
    var bytes = new PackedByteArray();
    bytes.resize(1 * 16);
    bytes.encode_float(0, 1.0); bytes.encode_float(4, 1.0); bytes.encode_float(8, 1.0); bytes.encode_float(12, 0.0);

    var vertices = bytes.as_vector3_array(1);

    vertices.append_array([{ x: 2.0, y: 2.0, z: 2.0 }]);
    vertices.append_array([{ x: 3.0, y: 3.0, z: 3.0 }]);
    vertices.append_array([{ x: 4.0, y: 4.0, z: 4.0 }]);

    if (vertices.length !== 4) return 'expected 4, got ' + vertices.length;

    for (var i = 0; i < 4; i++) {
        var v = vertices[i];
        var expected = i + 1;
        if (!vecEquals(v, {x: expected, y: expected, z: expected})) {
            return 'v' + i + ' wrong: ' + JSON.stringify(v) + ', expected all ' + expected;
        }
    }

    return true;
});

// ============================================================================
// Test 5: Large append (similar to real cap generation)
// ============================================================================

test('Large append_array (50 vertices like real cap)', function() {
    var bytes = new PackedByteArray();
    bytes.resize(100 * 16);  // 100 initial vertices
    for (var i = 0; i < 100; i++) {
        bytes.encode_float(i * 16, i);
        bytes.encode_float(i * 16 + 4, i);
        bytes.encode_float(i * 16 + 8, i);
        bytes.encode_float(i * 16 + 12, 0.0);
    }

    var vertices = bytes.as_vector3_array(100);

    // Append 50 cap vertices
    var capVerts = [];
    for (var i = 0; i < 50; i++) {
        capVerts.push({ x: 1000 + i, y: 1000 + i, z: 1000 + i });
    }

    vertices.append_array(capVerts);

    if (vertices.length !== 150) return 'expected 150, got ' + vertices.length;

    // Check a few samples
    var v0 = vertices[0];
    var v99 = vertices[99];
    var v100 = vertices[100];  // First cap vertex
    var v149 = vertices[149];  // Last cap vertex

    if (!vecEquals(v0, {x:0, y:0, z:0})) return 'v0 wrong';
    if (!vecEquals(v99, {x:99, y:99, z:99})) return 'v99 wrong';
    if (!vecEquals(v100, {x:1000, y:1000, z:1000})) return 'v100 wrong: ' + JSON.stringify(v100);
    if (!vecEquals(v149, {x:1049, y:1049, z:1049})) return 'v149 wrong: ' + JSON.stringify(v149);

    return true;
});

// ============================================================================
// Print results
// ============================================================================

console.log('\n--- Results ---');
for (var i = 0; i < results.length; i++) {
    var r = results[i];
    if (r.passed) {
        console.log('PASS: ' + r.name);
    } else {
        console.log('FAIL: ' + r.name + ' - ' + r.error);
    }
}

console.log('\nTotal: ' + passed + ' passed, ' + failed + ' failed');

// Return success/failure for test runner
if (failed > 0) {
    throw new Error(failed + ' tests failed');
}

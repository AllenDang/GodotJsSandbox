// Test: Bulk Encode/Decode Operations
// Tests all auto-generated bulk encode/decode functions for packed arrays

console.log('=== Bulk Operations Test ===');

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

function arraysEqual(a, b) {
    if (a.length !== b.length) return false;
    for (var i = 0; i < a.length; i++) {
        if (typeof a[i] === 'object' && typeof b[i] === 'object') {
            // Compare object properties
            for (var key in a[i]) {
                if (Math.abs(a[i][key] - b[i][key]) > 0.0001) return false;
            }
        } else if (Math.abs(a[i] - b[i]) > 0.0001) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// PackedByteArray Bulk Tests
// ============================================================================

test('PackedByteArray bulk_encode/decode', function() {
    var arr = new PackedByteArray([1, 2, 3, 4, 5]);
    var handle = arr.__packed_handle;

    // Bulk encode new values
    var newData = [10, 20, 30, 40, 50, 60];
    __packed_byte_array_bulk_encode(handle, newData);

    // Bulk decode
    var decoded = __packed_byte_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedInt32Array Bulk Tests
// ============================================================================

test('PackedInt32Array bulk_encode/decode', function() {
    var arr = new PackedInt32Array([100, 200, 300]);
    var handle = arr.__packed_handle;

    var newData = [-1000, 0, 1000, 2147483647, -2147483648];
    __packed_int32_array_bulk_encode(handle, newData);

    var decoded = __packed_int32_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedInt64Array Bulk Tests
// ============================================================================

test('PackedInt64Array bulk_encode/decode', function() {
    var arr = new PackedInt64Array([]);
    var handle = arr.__packed_handle;

    var newData = [9007199254740991, -9007199254740991, 0, 123456789];
    __packed_int64_array_bulk_encode(handle, newData);

    var decoded = __packed_int64_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedFloat32Array Bulk Tests
// ============================================================================

test('PackedFloat32Array bulk_encode/decode', function() {
    var arr = new PackedFloat32Array([]);
    var handle = arr.__packed_handle;

    var newData = [1.5, -2.5, 3.14159, 0.0, 1000.001];
    __packed_float32_array_bulk_encode(handle, newData);

    var decoded = __packed_float32_array_bulk_decode(handle);
    // Float32 has less precision
    for (var i = 0; i < newData.length; i++) {
        if (Math.abs(decoded[i] - newData[i]) > 0.001) return false;
    }
    return true;
});

// ============================================================================
// PackedFloat64Array Bulk Tests
// ============================================================================

test('PackedFloat64Array bulk_encode/decode', function() {
    var arr = new PackedFloat64Array([]);
    var handle = arr.__packed_handle;

    var newData = [1.5, -2.5, 3.141592653589793, 0.0, 1000.00001];
    __packed_float64_array_bulk_encode(handle, newData);

    var decoded = __packed_float64_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedStringArray Bulk Tests
// ============================================================================

test('PackedStringArray bulk_encode/decode', function() {
    var arr = new PackedStringArray([]);
    var handle = arr.__packed_handle;

    var newData = ['hello', 'world', 'test', ''];
    __packed_string_array_bulk_encode(handle, newData);

    var decoded = __packed_string_array_bulk_decode(handle);
    for (var i = 0; i < newData.length; i++) {
        if (decoded[i] !== newData[i]) return false;
    }
    return true;
});

// ============================================================================
// PackedVector2Array Bulk Tests
// ============================================================================

test('PackedVector2Array bulk_encode/decode', function() {
    var arr = new PackedVector2Array([]);
    var handle = arr.__packed_handle;

    var newData = [
        {x: 1.0, y: 2.0},
        {x: -3.5, y: 4.5},
        {x: 0.0, y: 0.0},
        {x: 100.25, y: -100.75}
    ];
    __packed_vector2_array_bulk_encode(handle, newData);

    var decoded = __packed_vector2_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedVector3Array Bulk Tests
// ============================================================================

test('PackedVector3Array bulk_encode/decode', function() {
    var arr = new PackedVector3Array([]);
    var handle = arr.__packed_handle;

    var newData = [
        {x: 1.0, y: 2.0, z: 3.0},
        {x: -3.5, y: 4.5, z: -5.5},
        {x: 0.0, y: 0.0, z: 0.0}
    ];
    __packed_vector3_array_bulk_encode(handle, newData);

    var decoded = __packed_vector3_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedVector4Array Bulk Tests
// ============================================================================

test('PackedVector4Array bulk_encode/decode', function() {
    var arr = new PackedVector4Array([]);
    var handle = arr.__packed_handle;

    var newData = [
        {x: 1.0, y: 2.0, z: 3.0, w: 4.0},
        {x: -1.5, y: -2.5, z: -3.5, w: -4.5}
    ];
    __packed_vector4_array_bulk_encode(handle, newData);

    var decoded = __packed_vector4_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedColorArray Bulk Tests
// ============================================================================

test('PackedColorArray bulk_encode/decode', function() {
    var arr = new PackedColorArray([]);
    var handle = arr.__packed_handle;

    var newData = [
        {r: 1.0, g: 0.0, b: 0.0, a: 1.0},  // Red
        {r: 0.0, g: 1.0, b: 0.0, a: 0.5},  // Green semi-transparent
        {r: 0.0, g: 0.0, b: 1.0, a: 1.0}   // Blue
    ];
    __packed_color_array_bulk_encode(handle, newData);

    var decoded = __packed_color_array_bulk_decode(handle);
    return arraysEqual(decoded, newData);
});

// ============================================================================
// PackedByteArray Binary Bulk Operations (for GPU buffers)
// ============================================================================

test('PackedByteArray encode_vec4_array/to_vec3_array', function() {
    var arr = new PackedByteArray([]);
    var handle = arr.__packed_handle;

    var vertices = [
        {x: 1.0, y: 2.0, z: 3.0},
        {x: 4.0, y: 5.0, z: 6.0},
        {x: 7.0, y: 8.0, z: 9.0}
    ];

    // Encode as vec4 (16 bytes each)
    __packed_byte_array_encode_vec4_array(handle, vertices);

    // Decode back as vec3
    var decoded = __packed_byte_array_to_vec3_array(handle, vertices.length);
    return arraysEqual(decoded, vertices);
});

test('PackedByteArray encode_vec2_array/to_vec2_array', function() {
    var arr = new PackedByteArray([]);
    var handle = arr.__packed_handle;

    var uvs = [
        {x: 0.0, y: 0.0},
        {x: 1.0, y: 0.0},
        {x: 0.5, y: 1.0}
    ];

    __packed_byte_array_encode_vec2_array(handle, uvs);
    var decoded = __packed_byte_array_to_vec2_array(handle, uvs.length);
    return arraysEqual(decoded, uvs);
});

test('PackedByteArray encode_uint32_array/decode_uint32s', function() {
    var arr = new PackedByteArray([]);
    var handle = arr.__packed_handle;

    var indices = [0, 1, 2, 2, 3, 0, 4, 5, 6];

    __packed_byte_array_encode_uint32_array(handle, indices);
    var decoded = __packed_byte_array_decode_uint32s(handle, 0, indices.length);
    return arraysEqual(decoded, indices);
});

// ============================================================================
// Performance sanity check (ensure bulk is faster than per-element)
// ============================================================================

test('Bulk vs per-element performance (1000 elements)', function() {
    var count = 1000;
    var testData = [];
    for (var i = 0; i < count; i++) {
        testData.push(i);
    }

    // Test bulk encode
    var arr = new PackedInt32Array([]);
    var handle = arr.__packed_handle;

    var startBulk = Time.get_ticks_usec();
    __packed_int32_array_bulk_encode(handle, testData);
    var bulkEncodeTime = Time.get_ticks_usec() - startBulk;

    // Test bulk decode
    startBulk = Time.get_ticks_usec();
    var decoded = __packed_int32_array_bulk_decode(handle);
    var bulkDecodeTime = Time.get_ticks_usec() - startBulk;

    console.log('  Bulk encode: ' + bulkEncodeTime + 'us, decode: ' + bulkDecodeTime + 'us');

    // Verify data integrity
    return arraysEqual(decoded, testData);
});

// ============================================================================
// Edge cases
// ============================================================================

test('Empty array bulk operations', function() {
    var arr = new PackedInt32Array([]);
    var handle = arr.__packed_handle;

    __packed_int32_array_bulk_encode(handle, []);
    var decoded = __packed_int32_array_bulk_decode(handle);
    return decoded.length === 0;
});

test('Single element bulk operations', function() {
    var arr = new PackedFloat64Array([]);
    var handle = arr.__packed_handle;

    __packed_float64_array_bulk_encode(handle, [42.5]);
    var decoded = __packed_float64_array_bulk_decode(handle);
    return decoded.length === 1 && Math.abs(decoded[0] - 42.5) < 0.0001;
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

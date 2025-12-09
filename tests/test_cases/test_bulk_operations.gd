class_name TestBulkOperations
extends TestBase

func get_suite_name() -> String:
	return "Bulk Operations"

func get_tests() -> Array[String]:
	return [
		# Bulk encode/decode for all packed array types
		"test_packed_byte_array_bulk",
		"test_packed_int32_array_bulk",
		"test_packed_int64_array_bulk",
		"test_packed_float32_array_bulk",
		"test_packed_float64_array_bulk",
		"test_packed_string_array_bulk",
		"test_packed_vector2_array_bulk",
		"test_packed_vector3_array_bulk",
		"test_packed_vector4_array_bulk",
		"test_packed_color_array_bulk",
		# PackedByteArray binary bulk operations (for GPU buffers)
		"test_packed_byte_array_encode_vec4_array",
		"test_packed_byte_array_encode_vec2_array",
		"test_packed_byte_array_encode_uint32_array",
		# Edge cases
		"test_bulk_empty_array",
		"test_bulk_single_element",
		"test_bulk_large_array",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_packed_byte_array_bulk":
			var code = """
				var arr = new PackedByteArray([1, 2, 3]);
				var handle = arr.__packed_handle;
				var newData = [10, 20, 30, 40, 50];
				__packed_byte_array_bulk_encode(handle, newData);
				var decoded = __packed_byte_array_bulk_decode(handle);
				decoded.length === 5 && decoded[0] === 10 && decoded[4] === 50;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedByteArray bulk encode/decode should work")

		"test_packed_int32_array_bulk":
			var code = """
				var arr = new PackedInt32Array([]);
				var handle = arr.__packed_handle;
				var newData = [-1000, 0, 1000, 2147483647];
				__packed_int32_array_bulk_encode(handle, newData);
				var decoded = __packed_int32_array_bulk_decode(handle);
				decoded.length === 4 && decoded[0] === -1000 && decoded[3] === 2147483647;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedInt32Array bulk encode/decode should work")

		"test_packed_int64_array_bulk":
			var code = """
				var arr = new PackedInt64Array([]);
				var handle = arr.__packed_handle;
				var newData = [9007199254740991, -9007199254740991, 0];
				__packed_int64_array_bulk_encode(handle, newData);
				var decoded = __packed_int64_array_bulk_decode(handle);
				decoded.length === 3 && decoded[0] === 9007199254740991;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedInt64Array bulk encode/decode should work")

		"test_packed_float32_array_bulk":
			var code = """
				var arr = new PackedFloat32Array([]);
				var handle = arr.__packed_handle;
				var newData = [1.5, -2.5, 3.14159];
				__packed_float32_array_bulk_encode(handle, newData);
				var decoded = __packed_float32_array_bulk_decode(handle);
				decoded.length === 3 && Math.abs(decoded[0] - 1.5) < 0.001;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedFloat32Array bulk encode/decode should work")

		"test_packed_float64_array_bulk":
			var code = """
				var arr = new PackedFloat64Array([]);
				var handle = arr.__packed_handle;
				var newData = [1.5, -2.5, 3.141592653589793];
				__packed_float64_array_bulk_encode(handle, newData);
				var decoded = __packed_float64_array_bulk_decode(handle);
				decoded.length === 3 && Math.abs(decoded[2] - 3.141592653589793) < 0.0000001;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedFloat64Array bulk encode/decode should work")

		"test_packed_string_array_bulk":
			var code = """
				var arr = new PackedStringArray([]);
				var handle = arr.__packed_handle;
				var newData = ['hello', 'world', 'test'];
				__packed_string_array_bulk_encode(handle, newData);
				var decoded = __packed_string_array_bulk_decode(handle);
				decoded.length === 3 && decoded[0] === 'hello' && decoded[2] === 'test';
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedStringArray bulk encode/decode should work")

		"test_packed_vector2_array_bulk":
			var code = """
				var arr = new PackedVector2Array([]);
				var handle = arr.__packed_handle;
				var newData = [{x: 1, y: 2}, {x: 3, y: 4}, {x: 5, y: 6}];
				__packed_vector2_array_bulk_encode(handle, newData);
				var decoded = __packed_vector2_array_bulk_decode(handle);
				decoded.length === 3 && decoded[0].x === 1 && decoded[2].y === 6;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedVector2Array bulk encode/decode should work")

		"test_packed_vector3_array_bulk":
			var code = """
				var arr = new PackedVector3Array([]);
				var handle = arr.__packed_handle;
				var newData = [{x: 1, y: 2, z: 3}, {x: 4, y: 5, z: 6}];
				__packed_vector3_array_bulk_encode(handle, newData);
				var decoded = __packed_vector3_array_bulk_decode(handle);
				decoded.length === 2 && decoded[0].x === 1 && decoded[1].z === 6;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedVector3Array bulk encode/decode should work")

		"test_packed_vector4_array_bulk":
			var code = """
				var arr = new PackedVector4Array([]);
				var handle = arr.__packed_handle;
				var newData = [{x: 1, y: 2, z: 3, w: 4}, {x: 5, y: 6, z: 7, w: 8}];
				__packed_vector4_array_bulk_encode(handle, newData);
				var decoded = __packed_vector4_array_bulk_decode(handle);
				decoded.length === 2 && decoded[0].w === 4 && decoded[1].x === 5;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedVector4Array bulk encode/decode should work")

		"test_packed_color_array_bulk":
			var code = """
				var arr = new PackedColorArray([]);
				var handle = arr.__packed_handle;
				var newData = [{r: 1, g: 0, b: 0, a: 1}, {r: 0, g: 1, b: 0, a: 0.5}];
				__packed_color_array_bulk_encode(handle, newData);
				var decoded = __packed_color_array_bulk_decode(handle);
				decoded.length === 2 && decoded[0].r === 1 && Math.abs(decoded[1].a - 0.5) < 0.001;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedColorArray bulk encode/decode should work")

		"test_packed_byte_array_encode_vec4_array":
			var code = """
				var arr = new PackedByteArray([]);
				var handle = arr.__packed_handle;
				var vertices = [{x: 1, y: 2, z: 3}, {x: 4, y: 5, z: 6}];
				__packed_byte_array_encode_vec4_array(handle, vertices);
				var decoded = __packed_byte_array_to_vec3_array(handle, 2);
				decoded.length === 2 && decoded[0].x === 1 && decoded[1].z === 6;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedByteArray encode_vec4_array/to_vec3_array should work")

		"test_packed_byte_array_encode_vec2_array":
			var code = """
				var arr = new PackedByteArray([]);
				var handle = arr.__packed_handle;
				var uvs = [{x: 0, y: 0}, {x: 1, y: 0}, {x: 0.5, y: 1}];
				__packed_byte_array_encode_vec2_array(handle, uvs);
				var decoded = __packed_byte_array_to_vec2_array(handle, 3);
				decoded.length === 3 && decoded[2].x === 0.5 && decoded[2].y === 1;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedByteArray encode_vec2_array/to_vec2_array should work")

		"test_packed_byte_array_encode_uint32_array":
			var code = """
				var arr = new PackedByteArray([]);
				var handle = arr.__packed_handle;
				var indices = [0, 1, 2, 2, 3, 0];
				__packed_byte_array_encode_uint32_array(handle, indices);
				var decoded = __packed_byte_array_decode_uint32s(handle, 0, 6);
				decoded.length === 6 && decoded[0] === 0 && decoded[3] === 2 && decoded[5] === 0;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedByteArray encode_uint32_array/decode_uint32s should work")

		"test_bulk_empty_array":
			var code = """
				var arr = new PackedInt32Array([]);
				var handle = arr.__packed_handle;
				__packed_int32_array_bulk_encode(handle, []);
				var decoded = __packed_int32_array_bulk_decode(handle);
				decoded.length === 0;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "Empty array bulk operations should work")

		"test_bulk_single_element":
			var code = """
				var arr = new PackedFloat64Array([]);
				var handle = arr.__packed_handle;
				__packed_float64_array_bulk_encode(handle, [42.5]);
				var decoded = __packed_float64_array_bulk_decode(handle);
				decoded.length === 1 && Math.abs(decoded[0] - 42.5) < 0.0001;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "Single element bulk operations should work")

		"test_bulk_large_array":
			var code = """
				var arr = new PackedInt32Array([]);
				var handle = arr.__packed_handle;
				var testData = [];
				for (var i = 0; i < 1000; i++) {
					testData.push(i);
				}
				__packed_int32_array_bulk_encode(handle, testData);
				var decoded = __packed_int32_array_bulk_decode(handle);
				decoded.length === 1000 && decoded[0] === 0 && decoded[999] === 999;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "Large array (1000 elements) bulk operations should work")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

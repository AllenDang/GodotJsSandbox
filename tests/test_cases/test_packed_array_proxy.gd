class_name TestPackedArrayProxy
extends TestBase

func get_suite_name() -> String:
	return "Packed Array Proxy"

func get_tests() -> Array[String]:
	return [
		"test_packed_vector3_array_length",
		"test_packed_vector3_array_index_access",
		"test_packed_vector3_array_foreach",
		"test_packed_vector3_array_map",
		"test_packed_vector3_array_iteration",
		"test_packed_int32_array_basic",
		"test_packed_float32_array_basic",
		"test_packed_string_array_basic",
		"test_packed_color_array_basic",
		"test_packed_byte_array_basic",
		"test_mesh_surface_arrays",
		# JS constructors for packed arrays (generated)
		"test_js_packed_vector3_array_constructor",
		"test_js_packed_vector2_array_constructor",
		"test_js_packed_int32_array_constructor",
		"test_js_packed_int64_array_constructor",
		"test_js_packed_float32_array_constructor",
		"test_js_packed_float64_array_constructor",
		"test_js_packed_byte_array_constructor",
		"test_js_packed_string_array_constructor",
		"test_js_packed_color_array_constructor",
		"test_js_packed_vector4_array_constructor",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_packed_vector3_array_length":
			# Create a PackedVector3Array and pass it to JS
			var arr = PackedVector3Array([
				Vector3(1, 2, 3),
				Vector3(4, 5, 6),
				Vector3(7, 8, 9)
			])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				arr.length;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, 3, "PackedVector3Array length should be 3")

		"test_packed_vector3_array_index_access":
			var arr = PackedVector3Array([
				Vector3(10, 20, 30),
				Vector3(40, 50, 60)
			])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				var v = arr[0];
				v && v.x === 10 && v.y === 20 && v.z === 30;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, true, "Index access should return Vector3 with correct values")

		"test_packed_vector3_array_foreach":
			var arr = PackedVector3Array([
				Vector3(1, 0, 0),
				Vector3(0, 1, 0),
				Vector3(0, 0, 1)
			])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				var sum = 0;
				arr.forEach(function(v, i) {
					sum += v.x + v.y + v.z;
				});
				sum;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, 3, "forEach should iterate and sum to 3")

		"test_packed_vector3_array_map":
			var arr = PackedVector3Array([
				Vector3(1, 2, 3),
				Vector3(4, 5, 6)
			])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				var magnitudes = arr.map(function(v) {
					return Math.sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
				});
				magnitudes.length === 2 && Math.abs(magnitudes[0] - 3.7416573867739413) < 0.001;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, true, "map should transform packed array elements")

		"test_packed_vector3_array_iteration":
			var arr = PackedVector3Array([
				Vector3(1, 1, 1),
				Vector3(2, 2, 2),
				Vector3(3, 3, 3)
			])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				var count = 0;
				for (var v of arr) {
					count++;
				}
				count;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, 3, "for...of should iterate 3 times")

		"test_packed_int32_array_basic":
			var arr = PackedInt32Array([10, 20, 30, 40, 50])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				arr.length === 5 && arr[0] === 10 && arr[4] === 50;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, true, "PackedInt32Array should work correctly")

		"test_packed_float32_array_basic":
			var arr = PackedFloat32Array([1.5, 2.5, 3.5])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				arr.length === 3 && Math.abs(arr[0] - 1.5) < 0.01 && Math.abs(arr[2] - 3.5) < 0.01;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, true, "PackedFloat32Array should work correctly")

		"test_packed_string_array_basic":
			var arr = PackedStringArray(["hello", "world", "test"])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				arr.length === 3 && arr[0] === 'hello' && arr[1] === 'world' && arr[2] === 'test';
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, true, "PackedStringArray should work correctly")

		"test_packed_color_array_basic":
			var arr = PackedColorArray([
				Color(1, 0, 0, 1),
				Color(0, 1, 0, 1),
				Color(0, 0, 1, 1)
			])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				var c = arr[0];
				arr.length === 3 && c && c.r === 1 && c.g === 0 && c.b === 0 && c.a === 1;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, true, "PackedColorArray should work correctly")

		"test_packed_byte_array_basic":
			var arr = PackedByteArray([0, 127, 255])
			sandbox.set_global("__test_arr", arr)
			var code = """
				var arr = __test_arr;
				arr.length === 3 && arr[0] === 0 && arr[1] === 127 && arr[2] === 255;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_arr;")

			return assert_eq(result, true, "PackedByteArray should work correctly")

		"test_mesh_surface_arrays":
			# Test that mesh.surface_get_arrays returns proper packed arrays
			var mesh = BoxMesh.new()
			sandbox.set_global("__test_mesh", mesh)
			var code = """
				var mesh = __test_mesh;
				var surface_count = mesh.get_surface_count();
				if (surface_count === 0) {
					'no surfaces';
				} else {
					var arrays = mesh.surface_get_arrays(0);
					if (!arrays || arrays.length === 0) {
						'no arrays';
					} else {
						var vertices = arrays[0];  // ARRAY_VERTEX is index 0
						if (!vertices) {
							'no vertices';
						} else {
							// Check it's a proper packed array proxy
							var hasLength = typeof vertices.length === 'number';
							var canIndex = vertices[0] && typeof vertices[0].x === 'number';
							hasLength && canIndex ? 'success' : 'failed: hasLength=' + hasLength + ' canIndex=' + canIndex;
						}
					}
				}
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_mesh;")

			return assert_eq(result, "success", "Mesh surface arrays should return proper packed array proxies")

		# ============ JS Constructor Tests (Generated Packed Arrays) ============

		"test_js_packed_vector3_array_constructor":
			var code = """
				var arr = new PackedVector3Array([
					{x: 1, y: 2, z: 3},
					{x: 4, y: 5, z: 6}
				]);
				arr.length === 2 && arr[0].x === 1 && arr[1].z === 6;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedVector3Array JS constructor should work")

		"test_js_packed_vector2_array_constructor":
			var code = """
				var arr = new PackedVector2Array([
					{x: 10, y: 20},
					{x: 30, y: 40}
				]);
				arr.length === 2 && arr[0].x === 10 && arr[1].y === 40;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedVector2Array JS constructor should work")

		"test_js_packed_int32_array_constructor":
			var code = """
				var arr = new PackedInt32Array([100, 200, 300]);
				arr.length === 3 && arr[0] === 100 && arr[2] === 300;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedInt32Array JS constructor should work")

		"test_js_packed_int64_array_constructor":
			var code = """
				var arr = new PackedInt64Array([1000000000, 2000000000, 3000000000]);
				arr.length === 3 && arr[0] === 1000000000 && arr[2] === 3000000000;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedInt64Array JS constructor should work")

		"test_js_packed_float32_array_constructor":
			var code = """
				var arr = new PackedFloat32Array([1.5, 2.5, 3.5]);
				arr.length === 3 && Math.abs(arr[0] - 1.5) < 0.01 && Math.abs(arr[2] - 3.5) < 0.01;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedFloat32Array JS constructor should work")

		"test_js_packed_float64_array_constructor":
			var code = """
				var arr = new PackedFloat64Array([1.23456789, 2.34567890, 3.45678901]);
				arr.length === 3 && Math.abs(arr[0] - 1.23456789) < 0.0000001;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedFloat64Array JS constructor should work")

		"test_js_packed_byte_array_constructor":
			var code = """
				var arr = new PackedByteArray([0, 127, 255]);
				arr.length === 3 && arr[0] === 0 && arr[1] === 127 && arr[2] === 255;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedByteArray JS constructor should work")

		"test_js_packed_string_array_constructor":
			var code = """
				var arr = new PackedStringArray(['hello', 'world', 'test']);
				arr.length === 3 && arr[0] === 'hello' && arr[2] === 'test';
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedStringArray JS constructor should work")

		"test_js_packed_color_array_constructor":
			var code = """
				var arr = new PackedColorArray([
					{r: 1, g: 0, b: 0, a: 1},
					{r: 0, g: 1, b: 0}  // alpha defaults to 1
				]);
				arr.length === 2 && arr[0].r === 1 && arr[0].g === 0 && arr[1].g === 1;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedColorArray JS constructor should work")

		"test_js_packed_vector4_array_constructor":
			var code = """
				var arr = new PackedVector4Array([
					{x: 1, y: 2, z: 3, w: 4},
					{x: 5, y: 6, z: 7, w: 8}
				]);
				arr.length === 2 && arr[0].x === 1 && arr[1].w === 8;
			"""
			var result = sandbox.eval(code)
			return assert_eq(result, true, "PackedVector4Array JS constructor should work")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.eval("""
		delete globalThis.__test_arr;
		delete globalThis.__test_mesh;
	""")
	super.teardown()

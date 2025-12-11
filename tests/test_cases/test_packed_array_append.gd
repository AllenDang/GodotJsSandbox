class_name TestPackedArrayAppend
extends TestBase

func get_suite_name() -> String:
	return "Packed Array Append"

func get_tests() -> Array[String]:
	return [
		"test_as_vector3_array_creates_proxy",
		"test_append_array_increases_length",
		"test_append_array_data_readable",
		"test_append_array_to_arraymesh",
		"test_multiple_appends",
		"test_large_array_append_high_index_read",
		"test_large_append_to_large_base",
		"test_gdscript_byte_array_to_vector3_append",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_as_vector3_array_creates_proxy":
			# Test that as_vector3_array creates a valid PackedVector3Array proxy
			var code = """
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

				var result = {
					length: vertices.length,
					hasHandle: vertices.__packed_handle !== undefined,
					v0: vertices[0],
					v1: vertices[1],
					v2: vertices[2]
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }
			if result.length != 3:
				return { "passed": false, "message": "Expected length 3, got %s" % result.length }
			if not result.hasHandle:
				return { "passed": false, "message": "Missing __packed_handle" }
			if abs(result.v0.x - 1.0) > 0.001 or abs(result.v0.y - 2.0) > 0.001 or abs(result.v0.z - 3.0) > 0.001:
				return { "passed": false, "message": "v0 wrong: %s" % result.v0 }
			return { "passed": true, "message": "" }

		"test_append_array_increases_length":
			# Test that append_array increases the length correctly
			var code = """
				var bytes = new PackedByteArray();
				bytes.resize(2 * 16);
				bytes.encode_float(0, 1.0); bytes.encode_float(4, 2.0); bytes.encode_float(8, 3.0); bytes.encode_float(12, 0.0);
				bytes.encode_float(16, 4.0); bytes.encode_float(20, 5.0); bytes.encode_float(24, 6.0); bytes.encode_float(28, 0.0);

				var vertices = bytes.as_vector3_array(2);
				var origLength = vertices.length;
				var origHandle = vertices.__packed_handle;

				// Append new vertices
				vertices.append_array([
					{ x: 10.0, y: 11.0, z: 12.0 },
					{ x: 13.0, y: 14.0, z: 15.0 }
				]);

				var result = {
					origLength: origLength,
					newLength: vertices.length,
					handleSame: vertices.__packed_handle === origHandle
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }
			if result.origLength != 2:
				return { "passed": false, "message": "Original length should be 2, got %s" % result.origLength }
			if result.newLength != 4:
				return { "passed": false, "message": "New length should be 4, got %s" % result.newLength }
			if not result.handleSame:
				return { "passed": false, "message": "Handle changed after append" }
			return { "passed": true, "message": "" }

		"test_append_array_data_readable":
			# Test that appended data can be read back correctly
			var code = """
				var bytes = new PackedByteArray();
				bytes.resize(2 * 16);
				bytes.encode_float(0, 1.0); bytes.encode_float(4, 2.0); bytes.encode_float(8, 3.0); bytes.encode_float(12, 0.0);
				bytes.encode_float(16, 4.0); bytes.encode_float(20, 5.0); bytes.encode_float(24, 6.0); bytes.encode_float(28, 0.0);

				var vertices = bytes.as_vector3_array(2);

				vertices.append_array([
					{ x: 10.0, y: 11.0, z: 12.0 },
					{ x: 13.0, y: 14.0, z: 15.0 }
				]);

				// Read back ALL vertices
				var result = {
					v0: vertices[0],
					v1: vertices[1],
					v2: vertices[2],
					v3: vertices[3]
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }
			# Check original vertices
			if abs(result.v0.x - 1.0) > 0.001:
				return { "passed": false, "message": "v0.x wrong: %s" % result.v0.x }
			if abs(result.v1.x - 4.0) > 0.001:
				return { "passed": false, "message": "v1.x wrong: %s" % result.v1.x }
			# Check appended vertices - THIS IS THE CRITICAL TEST
			if abs(result.v2.x - 10.0) > 0.001 or abs(result.v2.y - 11.0) > 0.001 or abs(result.v2.z - 12.0) > 0.001:
				return { "passed": false, "message": "v2 (appended) wrong: (%s, %s, %s)" % [result.v2.x, result.v2.y, result.v2.z] }
			if abs(result.v3.x - 13.0) > 0.001 or abs(result.v3.y - 14.0) > 0.001 or abs(result.v3.z - 15.0) > 0.001:
				return { "passed": false, "message": "v3 (appended) wrong: (%s, %s, %s)" % [result.v3.x, result.v3.y, result.v3.z] }
			return { "passed": true, "message": "" }

		"test_append_array_to_arraymesh":
			# Test that appended data survives being passed to ArrayMesh
			var code = """
				// Create initial vertices from bytes (simulating GPU output)
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

				// Append cap data (simulating cap generation)
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

				var result = {
					extractedLength: extractedVerts.length,
					ev0: extractedVerts[0],
					ev1: extractedVerts[1],
					ev2: extractedVerts[2],
					ev3: extractedVerts[3]
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }
			if result.extractedLength != 4:
				return { "passed": false, "message": "Expected 4 verts in mesh, got %s" % result.extractedLength }
			# Check appended vertices survived the mesh creation
			if abs(result.ev2.x - 10.0) > 0.001 or abs(result.ev2.y - 11.0) > 0.001 or abs(result.ev2.z - 12.0) > 0.001:
				return { "passed": false, "message": "extracted v2 wrong: (%s, %s, %s)" % [result.ev2.x, result.ev2.y, result.ev2.z] }
			if abs(result.ev3.x - 13.0) > 0.001 or abs(result.ev3.y - 14.0) > 0.001 or abs(result.ev3.z - 15.0) > 0.001:
				return { "passed": false, "message": "extracted v3 wrong: (%s, %s, %s)" % [result.ev3.x, result.ev3.y, result.ev3.z] }
			return { "passed": true, "message": "" }

		"test_multiple_appends":
			# Test multiple append_array calls accumulate correctly
			var code = """
				var bytes = new PackedByteArray();
				bytes.resize(1 * 16);
				bytes.encode_float(0, 1.0); bytes.encode_float(4, 1.0); bytes.encode_float(8, 1.0); bytes.encode_float(12, 0.0);

				var vertices = bytes.as_vector3_array(1);

				vertices.append_array([{ x: 2.0, y: 2.0, z: 2.0 }]);
				vertices.append_array([{ x: 3.0, y: 3.0, z: 3.0 }]);
				vertices.append_array([{ x: 4.0, y: 4.0, z: 4.0 }]);

				var result = {
					length: vertices.length,
					v0: vertices[0],
					v1: vertices[1],
					v2: vertices[2],
					v3: vertices[3]
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }
			if result.length != 4:
				return { "passed": false, "message": "Expected 4, got %s" % result.length }
			for i in range(4):
				var v = result["v%d" % i]
				var expected = float(i + 1)
				if abs(v.x - expected) > 0.001 or abs(v.y - expected) > 0.001 or abs(v.z - expected) > 0.001:
					return { "passed": false, "message": "v%d wrong: expected all %s, got (%s, %s, %s)" % [i, expected, v.x, v.y, v.z] }
			return { "passed": true, "message": "" }

		"test_large_array_append_high_index_read":
			# Test reading appended data at HIGH indices (simulates the cap bug scenario)
			# Creates 15000 base vertices, appends 100 cap vertices, reads indices > 15000
			var code = """
				// Create a large base array (15000 vertices) from bytes
				var BASE_COUNT = 15000;
				var CAP_COUNT = 100;

				var bytes = new PackedByteArray();
				bytes.resize(BASE_COUNT * 16);  // 16 bytes per vec4

				// Fill base vertices with pattern: vertex[i] = (i, i, i)
				for (var i = 0; i < BASE_COUNT; i++) {
					bytes.encode_float(i * 16, i);
					bytes.encode_float(i * 16 + 4, i);
					bytes.encode_float(i * 16 + 8, i);
					bytes.encode_float(i * 16 + 12, 0.0);
				}

				var vertices = bytes.as_vector3_array(BASE_COUNT);
				var origLength = vertices.length;

				// Verify base array is correct
				var baseCheck0 = vertices[0];
				var baseCheck100 = vertices[100];
				var baseCheckLast = vertices[BASE_COUNT - 1];

				// Append cap vertices with distinctive values: cap[i] = (1000000 + i, ...)
				var capVerts = [];
				for (var i = 0; i < CAP_COUNT; i++) {
					capVerts.push({ x: 1000000 + i, y: 1000000 + i, z: 1000000 + i });
				}
				vertices.append_array(capVerts);

				var newLength = vertices.length;

				// Read back cap vertices at high indices
				var firstCapIdx = BASE_COUNT;  // = 15000
				var lastCapIdx = BASE_COUNT + CAP_COUNT - 1;  // = 15099

				var readFirstCap = vertices[firstCapIdx];
				var readMidCap = vertices[firstCapIdx + 50];  // index 15050
				var readLastCap = vertices[lastCapIdx];  // index 15099

				// Also verify base data wasn't corrupted
				var verifyBase0 = vertices[0];
				var verifyBase100 = vertices[100];

				var result = {
					origLength: origLength,
					newLength: newLength,
					expectedNewLength: BASE_COUNT + CAP_COUNT,
					baseCheck0: baseCheck0,
					baseCheck100: baseCheck100,
					baseCheckLast: baseCheckLast,
					firstCapIdx: firstCapIdx,
					readFirstCap: readFirstCap,
					expectedFirstCap: { x: 1000000, y: 1000000, z: 1000000 },
					readMidCap: readMidCap,
					expectedMidCap: { x: 1000050, y: 1000050, z: 1000050 },
					readLastCap: readLastCap,
					expectedLastCap: { x: 1000099, y: 1000099, z: 1000099 },
					verifyBase0: verifyBase0,
					verifyBase100: verifyBase100
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }

			# Check length is correct
			if result.newLength != result.expectedNewLength:
				return { "passed": false, "message": "Length wrong: expected %d, got %d" % [result.expectedNewLength, result.newLength] }

			# Check base vertices weren't corrupted
			if abs(result.baseCheck0.x) > 0.001:
				return { "passed": false, "message": "Base vertex 0 corrupted: %s" % result.baseCheck0 }
			if abs(result.baseCheck100.x - 100.0) > 0.001:
				return { "passed": false, "message": "Base vertex 100 corrupted: %s" % result.baseCheck100 }

			# THE CRITICAL CHECK: Verify cap vertices at high indices
			if abs(result.readFirstCap.x - 1000000.0) > 0.001:
				return { "passed": false, "message": "First cap vertex WRONG at idx %d: got (%s,%s,%s), expected (1000000,1000000,1000000)" % [result.firstCapIdx, result.readFirstCap.x, result.readFirstCap.y, result.readFirstCap.z] }
			if abs(result.readMidCap.x - 1000050.0) > 0.001:
				return { "passed": false, "message": "Mid cap vertex WRONG at idx 15050: got (%s,%s,%s), expected (1000050,1000050,1000050)" % [result.readMidCap.x, result.readMidCap.y, result.readMidCap.z] }
			if abs(result.readLastCap.x - 1000099.0) > 0.001:
				return { "passed": false, "message": "Last cap vertex WRONG at idx 15099: got (%s,%s,%s), expected (1000099,1000099,1000099)" % [result.readLastCap.x, result.readLastCap.y, result.readLastCap.z] }

			return { "passed": true, "message": "" }

		"test_large_append_to_large_base":
			# Test appending 100+ vertices and reading at various indices (similar to real cap)
			# This tests the specific indices from the bug report: base=15122, read idx=15158
			var code = """
				var BASE_COUNT = 15122;  // Exact count from debug output
				var CAP_COUNT = 135;     // Exact count from debug output

				var bytes = new PackedByteArray();
				bytes.resize(BASE_COUNT * 16);

				// Fill with pattern vertex[i] = (i * 0.001, i * 0.002, i * 0.003)
				for (var i = 0; i < BASE_COUNT; i++) {
					bytes.encode_float(i * 16, i * 0.001);
					bytes.encode_float(i * 16 + 4, i * 0.002);
					bytes.encode_float(i * 16 + 8, i * 0.003);
					bytes.encode_float(i * 16 + 12, 0.0);
				}

				var vertices = bytes.as_vector3_array(BASE_COUNT);

				// Append cap vertices with values that match real cap data pattern
				var capVerts = [];
				for (var i = 0; i < CAP_COUNT; i++) {
					// Use distinctive negative values like real cap vertices
					capVerts.push({
						x: -100.0 - i * 0.1,
						y: -200.0 - i * 0.1,
						z: -300.0 - i * 0.1
					});
				}
				vertices.append_array(capVerts);

				// Test the specific index from bug: 15158 = 15122 + 36 (cap vertex 36)
				var targetIdx = 15158;
				var capLocalIdx = targetIdx - BASE_COUNT;  // = 36

				var readAtTarget = vertices[targetIdx];
				var expectedAtTarget = {
					x: -100.0 - capLocalIdx * 0.1,  // -103.6
					y: -200.0 - capLocalIdx * 0.1,  // -203.6
					z: -300.0 - capLocalIdx * 0.1   // -303.6
				};

				// Also test first cap vertex
				var readFirstCap = vertices[BASE_COUNT];

				var result = {
					length: vertices.length,
					expectedLength: BASE_COUNT + CAP_COUNT,
					targetIdx: targetIdx,
					capLocalIdx: capLocalIdx,
					readAtTarget: readAtTarget,
					expectedAtTarget: expectedAtTarget,
					readFirstCap: readFirstCap,
					expectedFirstCap: { x: -100.0, y: -200.0, z: -300.0 }
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }

			if result.length != result.expectedLength:
				return { "passed": false, "message": "Length wrong: %d vs %d" % [result.length, result.expectedLength] }

			# Check first cap vertex
			if abs(result.readFirstCap.x - (-100.0)) > 0.01:
				return { "passed": false, "message": "First cap wrong: got (%s,%s,%s) expected (-100,-200,-300)" % [result.readFirstCap.x, result.readFirstCap.y, result.readFirstCap.z] }

			# THE BUG TEST: Check vertex at index 15158 (cap vertex 36)
			if abs(result.readAtTarget.x - result.expectedAtTarget.x) > 0.01:
				return { "passed": false, "message": "CRITICAL: Index %d (cap vertex %d) WRONG: got (%s,%s,%s) expected (%s,%s,%s)" % [
					result.targetIdx, result.capLocalIdx,
					result.readAtTarget.x, result.readAtTarget.y, result.readAtTarget.z,
					result.expectedAtTarget.x, result.expectedAtTarget.y, result.expectedAtTarget.z
				] }

			return { "passed": true, "message": "" }

		"test_gdscript_byte_array_to_vector3_append":
			# Test the scenario where PackedByteArray is PASSED FROM GDSCRIPT (like rd.buffer_get_data)
			# This simulates the actual game scenario more closely
			var BASE_COUNT = 1000
			var CAP_COUNT = 50

			# Create byte array in GDScript (simulating rd.buffer_get_data return)
			var gd_bytes := PackedByteArray()
			gd_bytes.resize(BASE_COUNT * 16)
			for i in range(BASE_COUNT):
				gd_bytes.encode_float(i * 16, float(i))
				gd_bytes.encode_float(i * 16 + 4, float(i))
				gd_bytes.encode_float(i * 16 + 8, float(i))
				gd_bytes.encode_float(i * 16 + 12, 0.0)

			# Pass the GDScript-created byte array to JS and test append
			sandbox.set_global("external_bytes", gd_bytes)
			sandbox.set_global("BASE_COUNT", BASE_COUNT)
			sandbox.set_global("CAP_COUNT", CAP_COUNT)

			var code = """
				// Use the externally-passed byte array (simulates rd.buffer_get_data)
				var vertices = external_bytes.as_vector3_array(BASE_COUNT);
				var origLength = vertices.length;

				// Verify base data
				var baseCheck0 = vertices[0];
				var baseCheck500 = vertices[500];

				// Append cap vertices
				var capVerts = [];
				for (var i = 0; i < CAP_COUNT; i++) {
					capVerts.push({ x: 9000 + i, y: 9000 + i, z: 9000 + i });
				}
				vertices.append_array(capVerts);

				// Read back cap vertices
				var firstCapIdx = BASE_COUNT;
				var midCapIdx = BASE_COUNT + 25;
				var lastCapIdx = BASE_COUNT + CAP_COUNT - 1;

				var result = {
					origLength: origLength,
					newLength: vertices.length,
					expectedLength: BASE_COUNT + CAP_COUNT,
					baseCheck0: baseCheck0,
					baseCheck500: baseCheck500,
					readFirstCap: vertices[firstCapIdx],
					readMidCap: vertices[midCapIdx],
					readLastCap: vertices[lastCapIdx],
					expectedFirstCap: { x: 9000, y: 9000, z: 9000 },
					expectedMidCap: { x: 9025, y: 9025, z: 9025 },
					expectedLastCap: { x: 9049, y: 9049, z: 9049 }
				};
				result;
			"""
			var result = sandbox.eval(code)
			if result == null:
				return { "passed": false, "message": "eval returned null" }

			if result.newLength != result.expectedLength:
				return { "passed": false, "message": "Length wrong: %d vs %d" % [result.newLength, result.expectedLength] }

			# Check base vertices weren't corrupted
			if abs(result.baseCheck0.x) > 0.001:
				return { "passed": false, "message": "Base vertex 0 corrupted" }
			if abs(result.baseCheck500.x - 500.0) > 0.001:
				return { "passed": false, "message": "Base vertex 500 corrupted" }

			# Check appended cap vertices
			if abs(result.readFirstCap.x - 9000.0) > 0.001:
				return { "passed": false, "message": "First cap WRONG: got (%s,%s,%s)" % [result.readFirstCap.x, result.readFirstCap.y, result.readFirstCap.z] }
			if abs(result.readMidCap.x - 9025.0) > 0.001:
				return { "passed": false, "message": "Mid cap WRONG: got (%s,%s,%s)" % [result.readMidCap.x, result.readMidCap.y, result.readMidCap.z] }
			if abs(result.readLastCap.x - 9049.0) > 0.001:
				return { "passed": false, "message": "Last cap WRONG: got (%s,%s,%s)" % [result.readLastCap.x, result.readLastCap.y, result.readLastCap.z] }

			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

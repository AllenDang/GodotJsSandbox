class_name TestFallbackPaths
extends TestBase

## Tests for fallback paths (unbound properties/methods)
## These test cases verify that Godot objects are correctly passed through
## generic __godot_set/__godot_call paths when properties/methods are not
## in generated bindings.

func get_suite_name() -> String:
	return "Fallback Paths"

func get_tests() -> Array[String]:
	return [
		# Unbound property setters
		"test_material_override_assignment",
		"test_material_override_retrieval",
		"test_material_override_same_handle",
		"test_material_color_after_assignment",
		"test_material_color_update_via_retrieved",
		"test_material_override_null",
		# Multiple object properties
		"test_multiple_object_properties",
		# Color conversion
		"test_color_conversion_red",
		"test_color_conversion_green",
		"test_color_conversion_hp_bar",
		# Vector3 conversion
		"test_vector3_position",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		# ================================================================
		# Unbound property setters (material_override not in bindings)
		# ================================================================

		"test_material_override_assignment":
			# Test that assigning a Godot object to unbound property works
			var code = """
				var mesh = new MeshInstance3D();
				var mat = new StandardMaterial3D();
				mesh.material_override = mat;
				// Should not throw, and material should be set
				mesh.material_override !== null;
			"""
			var result = assert_eval(code, true)
			# Clean up
			sandbox.eval("mesh.queue_free();")
			return result

		"test_material_override_retrieval":
			# Test that retrieved object has valid handle
			var code = """
				var mesh = new MeshInstance3D();
				var mat = new StandardMaterial3D();
				mesh.material_override = mat;
				var retrieved = mesh.material_override;
				retrieved !== null && typeof retrieved.__handle === 'number';
			"""
			var result = assert_eval(code, true)
			sandbox.eval("mesh.queue_free();")
			return result

		"test_material_override_same_handle":
			# Test that retrieved object is the same as assigned
			var code = """
				var mesh = new MeshInstance3D();
				var mat = new StandardMaterial3D();
				mesh.material_override = mat;
				var retrieved = mesh.material_override;
				retrieved.__handle === mat.__handle;
			"""
			var result = assert_eval(code, true)
			sandbox.eval("mesh.queue_free();")
			return result

		"test_material_color_after_assignment":
			# Test full flow: set color, assign material, verify color
			var code = """
				var mesh = new MeshInstance3D();
				mesh.mesh = new BoxMesh();
				var mat = new StandardMaterial3D();
				mat.albedo_color = new Color(0.5, 0.25, 0.75, 1.0);
				mesh.material_override = mat;

				var retrieved = mesh.material_override;
				var color = retrieved.albedo_color;
				Math.abs(color.r - 0.5) < 0.01 &&
				Math.abs(color.g - 0.25) < 0.01 &&
				Math.abs(color.b - 0.75) < 0.01;
			"""
			var result = assert_eval(code, true)
			sandbox.eval("mesh.queue_free();")
			return result

		"test_material_color_update_via_retrieved":
			# Test updating color through retrieved material reference
			var code = """
				var mesh = new MeshInstance3D();
				mesh.mesh = new BoxMesh();
				var mat = new StandardMaterial3D();
				mat.albedo_color = new Color(1, 0, 0, 1);
				mesh.material_override = mat;

				// Update via retrieved reference
				var retrieved = mesh.material_override;
				retrieved.albedo_color = new Color(0, 1, 0, 1);

				// Verify change persisted
				var color = mesh.material_override.albedo_color;
				Math.abs(color.g - 1.0) < 0.01 && Math.abs(color.r - 0.0) < 0.01;
			"""
			var result = assert_eval(code, true)
			sandbox.eval("mesh.queue_free();")
			return result

		"test_material_override_null":
			# Test setting property to null
			var code = """
				var mesh = new MeshInstance3D();
				var mat = new StandardMaterial3D();
				mesh.material_override = mat;
				var wasSet = mesh.material_override !== null;
				mesh.material_override = null;
				var isNull = mesh.material_override === null;
				wasSet && isNull;
			"""
			var result = assert_eval(code, true)
			sandbox.eval("mesh.queue_free();")
			return result

		# ================================================================
		# Multiple object properties
		# ================================================================

		"test_multiple_object_properties":
			var code = """
				var mesh = new MeshInstance3D();
				var box = new BoxMesh();
				var mat = new StandardMaterial3D();

				mesh.mesh = box;
				mesh.material_override = mat;

				mesh.mesh !== null &&
				mesh.material_override !== null &&
				mesh.mesh.__handle === box.__handle &&
				mesh.material_override.__handle === mat.__handle;
			"""
			var result = assert_eval(code, true)
			sandbox.eval("mesh.queue_free();")
			return result

		# ================================================================
		# Color conversion tests
		# ================================================================

		"test_color_conversion_red":
			var code = """
				var mat = new StandardMaterial3D();
				mat.albedo_color = new Color(1, 0, 0, 1);
				var c = mat.albedo_color;
				Math.abs(c.r - 1) < 0.01 && Math.abs(c.g) < 0.01 && Math.abs(c.b) < 0.01;
			"""
			return assert_eval(code, true)

		"test_color_conversion_green":
			var code = """
				var mat = new StandardMaterial3D();
				mat.albedo_color = new Color(0, 1, 0, 1);
				var c = mat.albedo_color;
				Math.abs(c.r) < 0.01 && Math.abs(c.g - 1) < 0.01 && Math.abs(c.b) < 0.01;
			"""
			return assert_eval(code, true)

		"test_color_conversion_hp_bar":
			# The exact HP bar color that was failing
			var code = """
				var mat = new StandardMaterial3D();
				mat.albedo_color = new Color(0.2, 0.9, 0.2, 1);
				var c = mat.albedo_color;
				Math.abs(c.r - 0.2) < 0.01 && Math.abs(c.g - 0.9) < 0.01 && Math.abs(c.b - 0.2) < 0.01;
			"""
			return assert_eval(code, true)

		# ================================================================
		# Vector3 conversion tests
		# ================================================================

		"test_vector3_position":
			var code = """
				var node = new Node3D();
				node.position = new Vector3(1.5, 2.5, 3.5);
				var pos = node.position;
				var result = Math.abs(pos.x - 1.5) < 0.01 &&
				             Math.abs(pos.y - 2.5) < 0.01 &&
				             Math.abs(pos.z - 3.5) < 0.01;
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

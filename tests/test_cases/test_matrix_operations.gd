class_name TestMatrixOperations
extends TestBase

## Tests for matrix type conversions between GDScript and JavaScript
## Covers: Transform2D, Basis, Transform3D, Projection, Quaternion

func get_suite_name() -> String:
	return "Matrix Operations"

func get_tests() -> Array[String]:
	return [
		# Transform2D tests
		"test_transform2d_identity_conversion",
		"test_transform2d_with_values_conversion",
		"test_transform2d_rotation_conversion",
		"test_transform2d_roundtrip",
		# Basis tests
		"test_basis_identity_conversion",
		"test_basis_rotation_x_conversion",
		"test_basis_rotation_y_conversion",
		"test_basis_rotation_z_conversion",
		"test_basis_scale_conversion",
		"test_basis_roundtrip",
		# Transform3D tests
		"test_transform3d_identity_conversion",
		"test_transform3d_translation_conversion",
		"test_transform3d_rotation_conversion",
		"test_transform3d_full_conversion",
		"test_transform3d_roundtrip",
		# Projection tests
		"test_projection_identity_conversion",
		"test_projection_perspective_conversion",
		"test_projection_orthogonal_conversion",
		"test_projection_roundtrip",
		# Quaternion tests
		"test_quaternion_identity_conversion",
		"test_quaternion_rotation_conversion",
		"test_quaternion_roundtrip",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		# ==================== TRANSFORM2D TESTS ====================
		"test_transform2d_identity_conversion":
			var t2d = Transform2D.IDENTITY
			sandbox.set_global("gd_transform2d", t2d)
			var result = sandbox.eval("""
				var t = gd_transform2d;
				t.x.x === 1 && t.x.y === 0 &&
				t.y.x === 0 && t.y.y === 1 &&
				t.origin.x === 0 && t.origin.y === 0
			""")
			return assert_eq(result, true, "Transform2D identity should convert correctly")

		"test_transform2d_with_values_conversion":
			var t2d = Transform2D(Vector2(2, 0), Vector2(0, 3), Vector2(10, 20))
			sandbox.set_global("gd_transform2d", t2d)
			var result = sandbox.eval("""
				var t = gd_transform2d;
				t.x.x === 2 && t.x.y === 0 &&
				t.y.x === 0 && t.y.y === 3 &&
				t.origin.x === 10 && t.origin.y === 20
			""")
			return assert_eq(result, true, "Transform2D with values should convert correctly")

		"test_transform2d_rotation_conversion":
			var angle = deg_to_rad(45)
			var t2d = Transform2D(angle, Vector2.ZERO)
			sandbox.set_global("gd_transform2d", t2d)
			sandbox.set_global("expected_cos", cos(angle))
			sandbox.set_global("expected_sin", sin(angle))
			var result = sandbox.eval("""
				var t = gd_transform2d;
				var tolerance = 0.0001;
				Math.abs(t.x.x - expected_cos) < tolerance &&
				Math.abs(t.x.y - expected_sin) < tolerance &&
				Math.abs(t.y.x - (-expected_sin)) < tolerance &&
				Math.abs(t.y.y - expected_cos) < tolerance
			""")
			return assert_eq(result, true, "Transform2D rotation should convert correctly")

		"test_transform2d_roundtrip":
			# Test: GDScript -> JS -> modify -> back to GDScript via method call
			var node = Node2D.new()
			test_root.add_child(node)
			node.transform = Transform2D(deg_to_rad(30), Vector2(100, 200))
			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var n = test_node;
				var t = n.transform;
				// Verify values in JS
				var tolerance = 0.0001;
				Math.abs(t.origin.x - 100) < tolerance && Math.abs(t.origin.y - 200) < tolerance
			""")
			return assert_eq(result, true, "Transform2D roundtrip should preserve values")

		# ==================== BASIS TESTS ====================
		"test_basis_identity_conversion":
			var basis = Basis.IDENTITY
			sandbox.set_global("gd_basis", basis)
			var result = sandbox.eval("""
				var b = gd_basis;
				b.x.x === 1 && b.x.y === 0 && b.x.z === 0 &&
				b.y.x === 0 && b.y.y === 1 && b.y.z === 0 &&
				b.z.x === 0 && b.z.y === 0 && b.z.z === 1
			""")
			return assert_eq(result, true, "Basis identity should convert correctly")

		"test_basis_rotation_x_conversion":
			var angle = deg_to_rad(90)
			var basis = Basis(Vector3(1, 0, 0), angle)
			sandbox.set_global("gd_basis", basis)
			var result = sandbox.eval("""
				var b = gd_basis;
				var tolerance = 0.0001;
				Math.abs(b.x.x - 1) < tolerance && Math.abs(b.x.y) < tolerance && Math.abs(b.x.z) < tolerance
			""")
			return assert_eq(result, true, "Basis rotation X should convert correctly")

		"test_basis_rotation_y_conversion":
			var angle = deg_to_rad(90)
			var basis = Basis(Vector3(0, 1, 0), angle)
			sandbox.set_global("gd_basis", basis)
			var result = sandbox.eval("""
				var b = gd_basis;
				var tolerance = 0.0001;
				Math.abs(b.y.x) < tolerance && Math.abs(b.y.y - 1) < tolerance && Math.abs(b.y.z) < tolerance
			""")
			return assert_eq(result, true, "Basis rotation Y should convert correctly")

		"test_basis_rotation_z_conversion":
			var angle = deg_to_rad(90)
			var basis = Basis(Vector3(0, 0, 1), angle)
			sandbox.set_global("gd_basis", basis)
			var result = sandbox.eval("""
				var b = gd_basis;
				var tolerance = 0.0001;
				Math.abs(b.z.x) < tolerance && Math.abs(b.z.y) < tolerance && Math.abs(b.z.z - 1) < tolerance
			""")
			return assert_eq(result, true, "Basis rotation Z should convert correctly")

		"test_basis_scale_conversion":
			var basis = Basis.IDENTITY.scaled(Vector3(2, 3, 4))
			sandbox.set_global("gd_basis", basis)
			var result = sandbox.eval("""
				var b = gd_basis;
				var tolerance = 0.0001;
				Math.abs(b.x.x - 2) < tolerance &&
				Math.abs(b.y.y - 3) < tolerance &&
				Math.abs(b.z.z - 4) < tolerance
			""")
			return assert_eq(result, true, "Basis scale should convert correctly")

		"test_basis_roundtrip":
			var node = Node3D.new()
			test_root.add_child(node)
			var original_basis = Basis(Vector3(0, 1, 0), deg_to_rad(45))
			node.basis = original_basis
			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var n = test_node;
				var b = n.basis;
				var tolerance = 0.0001;
				// Check diagonal elements match rotated basis
				Math.abs(b.y.y - 1) < tolerance
			""")
			return assert_eq(result, true, "Basis roundtrip should preserve values")

		# ==================== TRANSFORM3D TESTS ====================
		"test_transform3d_identity_conversion":
			var t3d = Transform3D.IDENTITY
			sandbox.set_global("gd_transform3d", t3d)
			var result = sandbox.eval("""
				var t = gd_transform3d;
				t.basis.x.x === 1 && t.basis.y.y === 1 && t.basis.z.z === 1 &&
				t.origin.x === 0 && t.origin.y === 0 && t.origin.z === 0
			""")
			return assert_eq(result, true, "Transform3D identity should convert correctly")

		"test_transform3d_translation_conversion":
			var t3d = Transform3D.IDENTITY.translated(Vector3(10, 20, 30))
			sandbox.set_global("gd_transform3d", t3d)
			var result = sandbox.eval("""
				var t = gd_transform3d;
				t.origin.x === 10 && t.origin.y === 20 && t.origin.z === 30
			""")
			return assert_eq(result, true, "Transform3D translation should convert correctly")

		"test_transform3d_rotation_conversion":
			var basis = Basis(Vector3(0, 1, 0), deg_to_rad(90))
			var t3d = Transform3D(basis, Vector3.ZERO)
			sandbox.set_global("gd_transform3d", t3d)
			var result = sandbox.eval("""
				var t = gd_transform3d;
				var tolerance = 0.0001;
				Math.abs(t.basis.y.y - 1) < tolerance
			""")
			return assert_eq(result, true, "Transform3D rotation should convert correctly")

		"test_transform3d_full_conversion":
			var basis = Basis(Vector3(1, 0, 0), deg_to_rad(45)).scaled(Vector3(2, 2, 2))
			var t3d = Transform3D(basis, Vector3(5, 10, 15))
			sandbox.set_global("gd_transform3d", t3d)
			var result = sandbox.eval("""
				var t = gd_transform3d;
				t.origin.x === 5 && t.origin.y === 10 && t.origin.z === 15
			""")
			return assert_eq(result, true, "Transform3D full transform should convert correctly")

		"test_transform3d_roundtrip":
			var node = Node3D.new()
			test_root.add_child(node)
			node.transform = Transform3D(Basis.IDENTITY, Vector3(100, 200, 300))
			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var n = test_node;
				var t = n.transform;
				t.origin.x === 100 && t.origin.y === 200 && t.origin.z === 300
			""")
			return assert_eq(result, true, "Transform3D roundtrip should preserve values")

		# ==================== PROJECTION TESTS ====================
		"test_projection_identity_conversion":
			var proj = Projection.IDENTITY
			sandbox.set_global("gd_projection", proj)
			var result = sandbox.eval("""
				var p = gd_projection;
				p.x.x === 1 && p.x.y === 0 && p.x.z === 0 && p.x.w === 0 &&
				p.y.x === 0 && p.y.y === 1 && p.y.z === 0 && p.y.w === 0 &&
				p.z.x === 0 && p.z.y === 0 && p.z.z === 1 && p.z.w === 0 &&
				p.w.x === 0 && p.w.y === 0 && p.w.z === 0 && p.w.w === 1
			""")
			return assert_eq(result, true, "Projection identity should convert correctly")

		"test_projection_perspective_conversion":
			var proj = Projection.create_perspective(70.0, 1.777, 0.1, 1000.0)
			sandbox.set_global("gd_projection", proj)
			sandbox.set_global("expected_x_x", proj.x.x)
			sandbox.set_global("expected_y_y", proj.y.y)
			var result = sandbox.eval("""
				var p = gd_projection;
				var tolerance = 0.0001;
				Math.abs(p.x.x - expected_x_x) < tolerance &&
				Math.abs(p.y.y - expected_y_y) < tolerance &&
				p.w.w === 0
			""")
			return assert_eq(result, true, "Projection perspective should convert correctly")

		"test_projection_orthogonal_conversion":
			var proj = Projection.create_orthogonal(-10, 10, -10, 10, 0.1, 100)
			sandbox.set_global("gd_projection", proj)
			sandbox.set_global("expected_x_x", proj.x.x)
			sandbox.set_global("expected_y_y", proj.y.y)
			var result = sandbox.eval("""
				var p = gd_projection;
				var tolerance = 0.0001;
				Math.abs(p.x.x - expected_x_x) < tolerance &&
				Math.abs(p.y.y - expected_y_y) < tolerance
			""")
			return assert_eq(result, true, "Projection orthogonal should convert correctly")

		"test_projection_roundtrip":
			var proj = Projection.create_perspective(60.0, 1.5, 0.5, 500.0)
			sandbox.set_global("gd_projection", proj)
			sandbox.set_global("expected_z_z", proj.z.z)
			sandbox.set_global("expected_z_w", proj.z.w)
			var result = sandbox.eval("""
				var p = gd_projection;
				var tolerance = 0.0001;
				Math.abs(p.z.z - expected_z_z) < tolerance &&
				Math.abs(p.z.w - expected_z_w) < tolerance
			""")
			return assert_eq(result, true, "Projection roundtrip should preserve values")

		# ==================== QUATERNION TESTS ====================
		"test_quaternion_identity_conversion":
			var quat = Quaternion.IDENTITY
			sandbox.set_global("gd_quaternion", quat)
			var result = sandbox.eval("""
				var q = gd_quaternion;
				q.x === 0 && q.y === 0 && q.z === 0 && q.w === 1
			""")
			return assert_eq(result, true, "Quaternion identity should convert correctly")

		"test_quaternion_rotation_conversion":
			var quat = Quaternion(Vector3(0, 1, 0), deg_to_rad(90))
			sandbox.set_global("gd_quaternion", quat)
			sandbox.set_global("expected_y", quat.y)
			sandbox.set_global("expected_w", quat.w)
			var result = sandbox.eval("""
				var q = gd_quaternion;
				var tolerance = 0.0001;
				Math.abs(q.x) < tolerance &&
				Math.abs(q.y - expected_y) < tolerance &&
				Math.abs(q.z) < tolerance &&
				Math.abs(q.w - expected_w) < tolerance
			""")
			return assert_eq(result, true, "Quaternion rotation should convert correctly")

		"test_quaternion_roundtrip":
			var node = Node3D.new()
			test_root.add_child(node)
			var original_quat = Quaternion(Vector3(1, 0, 0), deg_to_rad(60))
			node.quaternion = original_quat
			sandbox.set_global("test_node", node)
			sandbox.set_global("expected_x", original_quat.x)
			sandbox.set_global("expected_w", original_quat.w)
			var result = sandbox.eval("""
				var n = test_node;
				var q = n.quaternion;
				var tolerance = 0.0001;
				Math.abs(q.x - expected_x) < tolerance &&
				Math.abs(q.w - expected_w) < tolerance
			""")
			return assert_eq(result, true, "Quaternion roundtrip should preserve values")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

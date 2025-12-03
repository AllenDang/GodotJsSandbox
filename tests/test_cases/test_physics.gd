class_name TestPhysics
extends TestBase

## Tests for physics body bindings (CharacterBody2D/3D, RigidBody2D/3D)

func get_suite_name() -> String:
	return "Physics"

func get_tests() -> Array[String]:
	return [
		# CharacterBody2D
		"test_character_body2d_create",
		"test_character_body2d_velocity",
		"test_character_body2d_move_and_slide",
		"test_character_body2d_floor_detection",
		"test_character_body2d_properties",
		# CharacterBody3D
		"test_character_body3d_create",
		"test_character_body3d_velocity",
		"test_character_body3d_floor_detection",
		# RigidBody2D
		"test_rigid_body2d_create",
		"test_rigid_body2d_mass",
		"test_rigid_body2d_apply_force",
		"test_rigid_body2d_linear_velocity",
		# RigidBody3D
		"test_rigid_body3d_create",
		"test_rigid_body3d_mass",
		"test_rigid_body3d_linear_velocity",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		# ===== CharacterBody2D =====
		"test_character_body2d_create":
			var result = sandbox.eval("""
				var body = new CharacterBody2D();
				body.name = 'TestPlayer';
				body.name;
			""")
			return assert_eq(result, "TestPlayer")

		"test_character_body2d_velocity":
			var result = sandbox.eval("""
				var body = new CharacterBody2D();
				body.velocity = { x: 100, y: -50 };
				var vel = body.velocity;
				vel.x + ',' + vel.y;
			""")
			return assert_eq(result, "100,-50")

		"test_character_body2d_move_and_slide":
			# Test that move_and_slide method exists and can be called
			var result = sandbox.eval("""
				var body = new CharacterBody2D();
				typeof body.move_and_slide;
			""")
			return assert_eq(result, "function")

		"test_character_body2d_floor_detection":
			# Test floor detection methods exist
			var result = sandbox.eval("""
				var body = new CharacterBody2D();
				var methods = [
					typeof body.is_on_floor,
					typeof body.is_on_wall,
					typeof body.is_on_ceiling
				];
				methods.join(',');
			""")
			return assert_eq(result, "function,function,function")

		"test_character_body2d_properties":
			# Test various CharacterBody2D properties
			var result = sandbox.eval("""
				var body = new CharacterBody2D();
				body.floor_max_angle = 0.8;
				body.floor_snap_length = 5.0;
				body.max_slides = 6;

				var results = [];
				results.push(Math.abs(body.floor_max_angle - 0.8) < 0.01);
				results.push(Math.abs(body.floor_snap_length - 5.0) < 0.01);
				results.push(body.max_slides === 6);
				results.every(function(r) { return r; });
			""")
			return assert_eq(result, true)

		# ===== CharacterBody3D =====
		"test_character_body3d_create":
			var result = sandbox.eval("""
				var body = new CharacterBody3D();
				body.name = 'Player3D';
				body.name;
			""")
			return assert_eq(result, "Player3D")

		"test_character_body3d_velocity":
			var result = sandbox.eval("""
				var body = new CharacterBody3D();
				body.velocity = { x: 10, y: 5, z: -3 };
				var vel = body.velocity;
				vel.x + ',' + vel.y + ',' + vel.z;
			""")
			return assert_eq(result, "10,5,-3")

		"test_character_body3d_floor_detection":
			var result = sandbox.eval("""
				var body = new CharacterBody3D();
				var methods = [
					typeof body.is_on_floor,
					typeof body.is_on_wall,
					typeof body.is_on_ceiling,
					typeof body.move_and_slide
				];
				methods.join(',');
			""")
			return assert_eq(result, "function,function,function,function")

		# ===== RigidBody2D =====
		"test_rigid_body2d_create":
			var result = sandbox.eval("""
				var body = new RigidBody2D();
				body.name = 'PhysicsBox';
				body.name;
			""")
			return assert_eq(result, "PhysicsBox")

		"test_rigid_body2d_mass":
			var result = sandbox.eval("""
				var body = new RigidBody2D();
				body.mass = 5.0;
				body.mass;
			""")
			if typeof(result) == TYPE_FLOAT or typeof(result) == TYPE_INT:
				return assert_eq(abs(result - 5.0) < 0.01, true, "Mass should be ~5.0")
			return { "passed": false, "message": "Mass should be a number, got %s" % type_string(typeof(result)) }

		"test_rigid_body2d_apply_force":
			# Test that apply methods exist
			var result = sandbox.eval("""
				var body = new RigidBody2D();
				var methods = [
					typeof body.apply_central_force,
					typeof body.apply_force,
					typeof body.apply_central_impulse,
					typeof body.apply_impulse
				];
				methods.join(',');
			""")
			return assert_eq(result, "function,function,function,function")

		"test_rigid_body2d_linear_velocity":
			var result = sandbox.eval("""
				var body = new RigidBody2D();
				body.linear_velocity = { x: 50, y: 100 };
				var vel = body.linear_velocity;
				vel.x + ',' + vel.y;
			""")
			return assert_eq(result, "50,100")

		# ===== RigidBody3D =====
		"test_rigid_body3d_create":
			var result = sandbox.eval("""
				var body = new RigidBody3D();
				body.name = 'PhysicsCube';
				body.name;
			""")
			return assert_eq(result, "PhysicsCube")

		"test_rigid_body3d_mass":
			var result = sandbox.eval("""
				var body = new RigidBody3D();
				body.mass = 10.0;
				body.mass;
			""")
			if typeof(result) == TYPE_FLOAT or typeof(result) == TYPE_INT:
				return assert_eq(abs(result - 10.0) < 0.01, true, "Mass should be ~10.0")
			return { "passed": false, "message": "Mass should be a number, got %s" % type_string(typeof(result)) }

		"test_rigid_body3d_linear_velocity":
			var result = sandbox.eval("""
				var body = new RigidBody3D();
				body.linear_velocity = { x: 1, y: 2, z: 3 };
				var vel = body.linear_velocity;
				vel.x + ',' + vel.y + ',' + vel.z;
			""")
			return assert_eq(result, "1,2,3")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

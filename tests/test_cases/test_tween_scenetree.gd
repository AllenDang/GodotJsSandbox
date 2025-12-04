class_name TestTweenSceneTree
extends TestBase

## Tests for SceneTree and Tween bindings
## These are critical features for game development

func get_suite_name() -> String:
	return "Tween & SceneTree"

func get_tests() -> Array[String]:
	return [
		# SceneTree access
		"test_get_tree",
		"test_scene_tree_has_group",
		# Tween creation
		"test_create_tween",
		"test_tween_is_valid",
		"test_tween_bind_node",
		"test_tween_set_loops",
		"test_tween_set_trans",
		"test_tween_set_ease",
		# Tween chaining
		"test_tween_parallel",
		"test_tween_chain",
		# tween_property()
		"test_tween_property_returns_tweener",
		"test_tween_property_position",
		"test_tween_property_chaining",
		# tween_callback() and tween_method()
		"test_tween_callback_returns_tweener",
		"test_tween_method_returns_tweener",
		# Math type constructors
		"test_quaternion_constructor",
		"test_basis_constructor",
		"test_transform3d_constructor",
		# Math type conversion
		"test_quaternion_from_godot",
		"test_transform3d_from_godot",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_get_tree":
			# Test that get_tree() returns a SceneTree object
			var node = Node3D.new()
			node.name = "GetTreeTestNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tree = node.get_tree();
				tree !== null && typeof tree === 'object'
			""")

			return assert_eq(result, true, "get_tree() should return SceneTree object")

		"test_scene_tree_has_group":
			# Test SceneTree.has_group()
			var node = Node3D.new()
			node.name = "GroupTestNode"
			node.add_to_group("test_tween_group")
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tree = node.get_tree();
				tree.has_group("test_tween_group")
			""")

			return assert_eq(result, true, "has_group should return true for existing group")

		"test_create_tween":
			# Test creating a tween via node.create_tween()
			var node = Node3D.new()
			node.name = "TweenTestNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				tween !== null && typeof tween === 'object'
			""")

			return assert_eq(result, true, "create_tween() should return Tween object")

		"test_tween_is_valid":
			# Test Tween.is_valid()
			var node = Node3D.new()
			node.name = "TweenValidNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				tween.is_valid()
			""")

			return assert_eq(result, true, "New tween should be valid")

		"test_tween_bind_node":
			# Test Tween.bind_node() returns chainable tween
			var node = Node3D.new()
			node.name = "TweenBindNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var bound = tween.bind_node(node);
				bound !== null && bound.is_valid()
			""")

			return assert_eq(result, true, "bind_node should return valid chainable tween")

		"test_tween_set_loops":
			# Test Tween.set_loops() returns chainable tween
			var node = Node3D.new()
			node.name = "TweenLoopsNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var looped = tween.set_loops(3);
				looped !== null && looped.is_valid()
			""")

			return assert_eq(result, true, "set_loops should return valid chainable tween")

		"test_tween_set_trans":
			# Test Tween.set_trans() returns chainable tween
			var node = Node3D.new()
			node.name = "TweenTransNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				// TRANS_LINEAR = 0
				var transitioned = tween.set_trans(0);
				transitioned !== null && transitioned.is_valid()
			""")

			return assert_eq(result, true, "set_trans should return valid chainable tween")

		"test_tween_set_ease":
			# Test Tween.set_ease() returns chainable tween
			var node = Node3D.new()
			node.name = "TweenEaseNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				// EASE_IN = 0
				var eased = tween.set_ease(0);
				eased !== null && eased.is_valid()
			""")

			return assert_eq(result, true, "set_ease should return valid chainable tween")

		"test_tween_parallel":
			# Test Tween.parallel() returns chainable tween
			var node = Node3D.new()
			node.name = "TweenParallelNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var parallel = tween.parallel();
				parallel !== null && parallel.is_valid()
			""")

			return assert_eq(result, true, "parallel() should return valid chainable tween")

		"test_tween_chain":
			# Test Tween.chain() returns chainable tween
			var node = Node3D.new()
			node.name = "TweenChainNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var chained = tween.chain();
				chained !== null && chained.is_valid()
			""")

			return assert_eq(result, true, "chain() should return valid chainable tween")

		"test_tween_property_returns_tweener":
			# Test tween_property() returns a PropertyTweener
			var node = Node3D.new()
			node.name = "TweenPropertyNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var tweener = tween.tween_property(node, "position:x", 10.0, 1.0);
				tweener !== null && typeof tweener === 'object'
			""")

			return assert_eq(result, true, "tween_property should return PropertyTweener object")

		"test_tween_property_position":
			# Test tween_property() with Vector3 position
			var node = Node3D.new()
			node.name = "TweenPropertyPosNode"
			node.position = Vector3.ZERO
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var target = new Vector3(5, 10, 15);
				var tweener = tween.tween_property(node, "position", target, 0.5);
				tweener !== null
			""")

			return assert_eq(result, true, "tween_property should accept Vector3 as final value")

		"test_tween_property_chaining":
			# Test tween_property() chaining with multiple properties
			var node = Node3D.new()
			node.name = "TweenPropertyChainNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				tween.tween_property(node, "position:x", 10.0, 1.0);
				tween.parallel().tween_property(node, "position:y", 5.0, 1.0);
				tween.is_valid()
			""")

			return assert_eq(result, true, "Multiple tween_property calls should work with parallel")

		"test_tween_callback_returns_tweener":
			# Test tween_callback() returns a CallbackTweener
			var node = Node3D.new()
			node.name = "TweenCallbackNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var tweener = tween.tween_callback(function() {
					// This callback will be called when tween reaches this point
				});
				tweener !== null && typeof tweener === 'object'
			""")

			return assert_eq(result, true, "tween_callback should return CallbackTweener object")

		"test_tween_method_returns_tweener":
			# Test tween_method() returns a MethodTweener
			var node = Node3D.new()
			node.name = "TweenMethodNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var tween = node.create_tween();
				var tweener = tween.tween_method(function(value) {
					// This will be called with interpolated value
				}, 0.0, 1.0, 0.5);
				tweener !== null && typeof tweener === 'object'
			""")

			return assert_eq(result, true, "tween_method should return MethodTweener object")

		"test_quaternion_constructor":
			# Test Quaternion constructor
			var result = sandbox.eval("""
				var q = new Quaternion(0, 0, 0, 1);
				q.x === 0 && q.y === 0 && q.z === 0 && q.w === 1
			""")

			return assert_eq(result, true, "Quaternion constructor should create identity quaternion")

		"test_basis_constructor":
			# Test Basis constructor (identity matrix)
			var result = sandbox.eval("""
				var b = new Basis();
				b.x.x === 1 && b.x.y === 0 && b.x.z === 0 &&
				b.y.x === 0 && b.y.y === 1 && b.y.z === 0 &&
				b.z.x === 0 && b.z.y === 0 && b.z.z === 1
			""")

			return assert_eq(result, true, "Basis constructor should create identity matrix")

		"test_transform3d_constructor":
			# Test Transform3D constructor (identity transform)
			var result = sandbox.eval("""
				var t = new Transform3D();
				t.origin.x === 0 && t.origin.y === 0 && t.origin.z === 0 &&
				t.basis.x.x === 1 && t.basis.y.y === 1 && t.basis.z.z === 1
			""")

			return assert_eq(result, true, "Transform3D constructor should create identity transform")

		"test_quaternion_from_godot":
			# Test Quaternion from Godot
			var node = Node3D.new()
			node.name = "QuaternionTestNode"
			test_root.add_child(node)

			# Rotate node 90 degrees around Y axis
			node.quaternion = Quaternion(Vector3(0, 1, 0), deg_to_rad(90))

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var q = node.quaternion;
				typeof q.x === 'number' && typeof q.y === 'number' &&
				typeof q.z === 'number' && typeof q.w === 'number'
			""")

			return assert_eq(result, true, "Quaternion property should return quaternion object")

		"test_transform3d_from_godot":
			# Test Transform3D from Godot
			var node = Node3D.new()
			node.name = "TransformTestNode"
			node.position = Vector3(1, 2, 3)
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				var t = node.transform;
				t.origin.x === 1 && t.origin.y === 2 && t.origin.z === 3
			""")

			return assert_eq(result, true, "Transform property should return transform with correct origin")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

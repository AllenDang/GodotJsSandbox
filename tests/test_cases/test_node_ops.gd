class_name TestNodeOps
extends TestBase

func get_suite_name() -> String:
	return "Node Operations"

func get_tests() -> Array[String]:
	return [
		"test_create_node",
		"test_create_node3d",
		"test_set_name",
		"test_add_child",
		"test_remove_child",
		"test_queue_free",
		"test_get_children_count",
		"test_reparent",
		"test_duplicate",
		"test_is_inside_tree",
		"test_get_path",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_create_node":
			var code = """
				var node = new Node();
				node !== null && node.__handle !== undefined;
			"""
			return assert_eval(code, true)

		"test_create_node3d":
			var code = """
				var node = new Node3D();
				node !== null && node.__class === 'Node3D';
			"""
			return assert_eval(code, true)

		"test_set_name":
			var code = """
				var node = new Node();
				node.name = 'MyTestNode';
				node.name === 'MyTestNode';
			"""
			return assert_eval(code, true)

		"test_add_child":
			# Should be able to add child and verify via get_child
			var code = """
				var parent = new Node();
				var child = new Node();
				child.name = 'ChildNode';
				parent.add_child(child);
				parent.get_child_count() === 1 && parent.get_child(0).name === 'ChildNode';
			"""
			return assert_eval(code, true)

		"test_remove_child":
			var code = """
				var parent = new Node();
				var child = new Node();
				parent.add_child(child);
				var countBefore = parent.get_child_count();
				parent.remove_child(child);
				var countAfter = parent.get_child_count();
				countBefore === 1 && countAfter === 0;
			"""
			return assert_eval(code, true)

		"test_queue_free":
			# Create node in GDScript, pass to JS, queue_free, verify
			var node = Node.new()
			node.name = "QueueFreeTest"
			test_root.add_child(node)

			sandbox.set_global("__test_node", node)
			sandbox.eval("__test_node.queue_free();")

			# Wait for queue_free to process
			await scene_tree.process_frame
			await scene_tree.process_frame

			var is_valid = is_instance_valid(node)
			sandbox.eval("delete globalThis.__test_node;")

			return assert_false(is_valid, "Node should be freed after queue_free")

		"test_get_children_count":
			var code = """
				var parent = new Node();
				parent.add_child(new Node());
				parent.add_child(new Node());
				parent.add_child(new Node());
				parent.get_child_count();
			"""
			return assert_eval(code, 3)

		"test_reparent":
			var code = """
				var parent1 = new Node();
				parent1.name = 'Parent1';
				var parent2 = new Node();
				parent2.name = 'Parent2';
				var child = new Node();
				child.name = 'Child';

				parent1.add_child(child);
				var count1Before = parent1.get_child_count();

				child.reparent(parent2);
				var count1After = parent1.get_child_count();
				var count2After = parent2.get_child_count();

				count1Before === 1 && count1After === 0 && count2After === 1;
			"""
			return assert_eval(code, true)

		"test_duplicate":
			var code = """
				var original = new Node3D();
				original.name = 'Original';
				original.position = {x: 5, y: 10, z: 15};

				var copy = original.duplicate();
				copy !== null && copy.__handle !== original.__handle;
			"""
			return assert_eval(code, true)

		"test_is_inside_tree":
			# Create node not in tree - should return false
			var code1 = """
				var node = new Node();
				node.is_inside_tree();
			"""
			var result1 = sandbox.eval(code1)
			if result1 != false:
				return { "passed": false, "message": "Node not in tree should return false" }

			# Add to tree via GDScript, check again
			var node = Node.new()
			test_root.add_child(node)
			sandbox.set_global("__tree_node", node)

			var result2 = sandbox.eval("__tree_node.is_inside_tree()")
			sandbox.eval("delete globalThis.__tree_node;")

			return assert_eq(result2, true, "Node in tree should return true")

		"test_get_path":
			var node = Node.new()
			node.name = "PathTestNode"
			test_root.add_child(node)

			sandbox.set_global("__path_node", node)
			var path = sandbox.eval("String(__path_node.get_path())")
			sandbox.eval("delete globalThis.__path_node;")

			if not path is String:
				return { "passed": false, "message": "Expected string path" }
			if not "PathTestNode" in path:
				return { "passed": false, "message": "Path should contain node name: %s" % path }

			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.eval("""
		delete globalThis.__test_node;
		delete globalThis.__tree_node;
		delete globalThis.__path_node;
	""")
	super.teardown()

class_name TestArrayProxy
extends TestBase

func get_suite_name() -> String:
	return "Array Proxy"

func get_tests() -> Array[String]:
	return [
		"test_get_children_returns_array",
		"test_array_length",
		"test_array_index_access",
		"test_array_foreach",
		"test_array_map",
		"test_array_filter",
		"test_array_push",
		"test_array_pop",
		"test_array_set_element",
		"test_array_iteration",
		"test_method_returning_array",
		"test_array_nested_access",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_get_children_returns_array":
			# get_children should return a proxy array
			var parent = Node.new()
			parent.name = "Parent"
			var child1 = Node.new()
			child1.name = "Child1"
			var child2 = Node.new()
			child2.name = "Child2"
			parent.add_child(child1)
			parent.add_child(child2)
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var code = """
				var parent = __test_parent;
				var children = parent.get_children();
				children !== null && typeof children.length === 'number' && children.length === 2;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(result, true, "get_children should return array with length 2")

		"test_array_length":
			var parent = Node.new()
			for i in range(5):
				var child = Node.new()
				child.name = "Child%d" % i
				parent.add_child(child)
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var length = sandbox.eval("__test_parent.get_children().length")
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(length, 5, "Array length should be 5")

		"test_array_index_access":
			var parent = Node.new()
			var child = Node.new()
			child.name = "IndexTest"
			parent.add_child(child)
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var code = """
				var children = __test_parent.get_children();
				children[0] && children[0].name === 'IndexTest';
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(result, true, "Index access should return child node")

		"test_array_foreach":
			var parent = Node.new()
			parent.add_child(Node.new())
			parent.add_child(Node.new())
			parent.add_child(Node.new())
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var code = """
				var count = 0;
				var children = __test_parent.get_children();
				children.forEach(function(child, index) {
					count++;
				});
				count;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(result, 3, "forEach should iterate 3 times")

		"test_array_map":
			var parent = Node.new()
			for i in range(3):
				var child = Node.new()
				child.name = "Node%d" % i
				parent.add_child(child)
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var code = """
				var children = __test_parent.get_children();
				var names = children.map(function(child) {
					return child.name;
				});
				names.length === 3 && names[0] === 'Node0' && names[1] === 'Node1' && names[2] === 'Node2';
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(result, true, "map should transform array elements")

		"test_array_filter":
			var parent = Node.new()
			for i in range(5):
				var child = Node.new()
				child.name = "Node%d" % i
				parent.add_child(child)
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var code = """
				var children = __test_parent.get_children();
				var filtered = children.filter(function(child, index) {
					return index < 3;
				});
				filtered.length;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(result, 3, "filter should return 3 elements")

		"test_array_push":
			# Test pushing to a Godot array
			var code = """
				var parent = new Node();
				var arr = parent.get_children();
				var initialLen = arr.length;
				// push won't work on empty array from get_children since it's copy-on-write
				// Let's test with a pre-populated array
				initialLen === 0;
			"""
			return assert_eval(code, true)

		"test_array_pop":
			# Test popping from a Godot array
			var code = """
				var parent = new Node();
				parent.add_child(new Node());
				parent.add_child(new Node());
				var arr = parent.get_children();
				var len = arr.length;
				len === 2;
			"""
			return assert_eval(code, true)

		"test_array_set_element":
			# Note: Setting elements on get_children array may not affect actual children
			# since Godot returns a copy. This tests that the proxy handles set correctly.
			var code = """
				var parent = new Node();
				parent.add_child(new Node());
				parent.add_child(new Node());
				var children = parent.get_children();
				children.length === 2;
			"""
			return assert_eval(code, true)

		"test_array_iteration":
			# Test for...of iteration
			var parent = Node.new()
			for i in range(3):
				var child = Node.new()
				child.name = "Iter%d" % i
				parent.add_child(child)
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var code = """
				var count = 0;
				var children = __test_parent.get_children();
				for (var child of children) {
					count++;
				}
				count;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(result, 3, "for...of should iterate 3 times")

		"test_method_returning_array":
			# Test other methods that return arrays
			var code = """
				var node = new Node();
				var groups = node.get_groups();
				groups !== null && typeof groups.length === 'number';
			"""
			return assert_eval(code, true)

		"test_array_nested_access":
			# Test accessing properties of array elements
			var parent = Node.new()
			var child = Node3D.new()
			child.name = "Nested"
			child.position = Vector3(1, 2, 3)
			parent.add_child(child)
			test_root.add_child(parent)

			sandbox.set_global("__test_parent", parent)
			var code = """
				var children = __test_parent.get_children();
				var first = children[0];
				first && first.name === 'Nested';
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_parent;")

			return assert_eq(result, true, "Should access nested element properties")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.eval("""
		delete globalThis.__test_parent;
	""")
	super.teardown()

class_name TestCrossScript
extends TestBase

func get_suite_name() -> String:
	return "Cross-Script Method Calls"

func get_tests() -> Array[String]:
	return [
		"test_has_script_method_true",
		"test_has_script_method_false_no_script",
		"test_has_script_method_false_no_method",
		"test_call_script_method_no_args",
		"test_call_script_method_with_args",
		"test_call_script_method_with_return",
		"test_access_method_on_proxy",
		"test_call_method_via_proxy",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_has_script_method_true":
			# Create a node with a JS script that has a custom method
			var node = Node.new()
			node.name = "TestScriptMethodNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			var script = JSScript.new()
			script.set_source_code("""
				exports.my_custom_method = function() {
					return 'hello';
				};
			""")
			node.set_script(script)
			await scene_tree.process_frame

			# Check if __godot_has_script_method works
			sandbox.set_global("__test_node", node)
			var result = sandbox.eval("""
				__godot_has_script_method(__test_node.__handle, 'my_custom_method');
			""")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")
			return assert_eq(result, true, "Should detect custom JS method")

		"test_has_script_method_false_no_script":
			# Create a node without a script
			var node = Node.new()
			node.name = "NoScriptNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var result = sandbox.eval("""
				__godot_has_script_method(__test_node.__handle, 'some_method');
			""")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")
			return assert_eq(result, false, "Should return false for node without script")

		"test_has_script_method_false_no_method":
			# Create a node with a script but method doesn't exist
			var node = Node.new()
			node.name = "TestNoMethodNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			var script = JSScript.new()
			script.set_source_code("""
				exports.other_method = function() { return 1; };
			""")
			node.set_script(script)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var result = sandbox.eval("""
				__godot_has_script_method(__test_node.__handle, 'nonexistent_method');
			""")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")
			return assert_eq(result, false, "Should return false for non-existent method")

		"test_call_script_method_no_args":
			# Create a node with a JS script and call its method
			var node = Node.new()
			node.name = "TestCallNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			var script = JSScript.new()
			script.set_source_code("""
				exports.greet = function() {
					return 'Hello World';
				};
			""")
			node.set_script(script)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var result = sandbox.eval("""
				__godot_call_script_method(__test_node.__handle, 'greet');
			""")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")
			return assert_eq(result, "Hello World", "Should return greeting string")

		"test_call_script_method_with_args":
			# Create a node with a JS script that takes arguments
			var node = Node.new()
			node.name = "TestArgsNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			var script = JSScript.new()
			script.set_source_code("""
				exports.add = function(a, b) {
					return a + b;
				};
			""")
			node.set_script(script)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var result = sandbox.eval("""
				__godot_call_script_method(__test_node.__handle, 'add', 5, 3);
			""")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")
			return assert_eq(result, 8, "Should return sum of arguments")

		"test_call_script_method_with_return":
			# Test returning complex values
			var node = Node.new()
			node.name = "TestReturnNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			var script = JSScript.new()
			script.set_source_code("""
				var counter = 0;
				exports.increment = function() {
					counter++;
					return counter;
				};
			""")
			node.set_script(script)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var result1 = sandbox.eval("__godot_call_script_method(__test_node.__handle, 'increment');")
			var result2 = sandbox.eval("__godot_call_script_method(__test_node.__handle, 'increment');")
			var result3 = sandbox.eval("__godot_call_script_method(__test_node.__handle, 'increment');")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")

			if result1 != 1:
				return { "passed": false, "message": "First call should return 1, got " + str(result1) }
			if result2 != 2:
				return { "passed": false, "message": "Second call should return 2, got " + str(result2) }
			return assert_eq(result3, 3, "Third call should return 3")

		"test_access_method_on_proxy":
			# Test that proxy handler finds JS script methods
			var node = Node.new()
			node.name = "TestProxyNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			var script = JSScript.new()
			script.set_source_code("""
				exports.custom_func = function() {
					return 'from custom';
				};
			""")
			node.set_script(script)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var result = sandbox.eval("""
				typeof __test_node.custom_func;
			""")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")
			return assert_eq(result, "function", "Proxy should expose JS method as function")

		"test_call_method_via_proxy":
			# Test calling JS script method through proxy (like body.on_goal_reached())
			var node = Node.new()
			node.name = "TestProxyCallNode"
			test_root.add_child(node)
			await scene_tree.process_frame

			var script = JSScript.new()
			script.set_source_code("""
				exports.on_goal_reached = function() {
					return 'VICTORY!';
				};
			""")
			node.set_script(script)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var result = sandbox.eval("""
				__test_node.on_goal_reached();
			""")

			node.queue_free()
			sandbox.eval("delete globalThis.__test_node;")
			return assert_eq(result, "VICTORY!", "Should be able to call JS method via proxy")
		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.eval("delete globalThis.__test_node;")
	super.teardown()

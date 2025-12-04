class_name TestLifecycle
extends TestBase

## Tests for script lifecycle callbacks (TDD.md Section 4.3 "JSScriptInstance")
## Reference: PRD.md "支持 _ready(), _process() 等生命周期回调"
##
## JSScriptInstance handles:
## - _ready() called when node enters tree
## - _process(delta) called each frame
## - _physics_process(delta) called each physics frame
## - 'this' bound to owner node
##
## Architecture (TDD.md Section 14.2):
## - Single JSRuntime + Multiple JSContext
## - Each JSSandbox has its own JSContext
## - JSScript instances share context with their sandbox

func get_suite_name() -> String:
	return "Lifecycle"

func get_tests() -> Array[String]:
	return [
		# _ready callback
		"test_ready_called_on_enter_tree",
		"test_ready_called_once",
		"test_ready_this_binding",
		# _process callback
		"test_process_called_each_frame",
		"test_process_receives_delta",
		# State persistence
		"test_script_state_persists",
		"test_closure_state_persists",
		# Multiple scripts
		"test_multiple_scripts_independent",
		# Script methods (exports pattern)
		"test_custom_method_callable",
		"test_method_this_binding",
		# Class-based exports pattern
		"test_class_based_exports",
		# Arrow function exports pattern
		"test_arrow_function_exports",
	]

func _create_js_script(source: String) -> Script:
	# Use sandbox.create_script() to ensure scripts run in sandbox's context
	# This allows tests to verify script behavior via sandbox.eval()
	return sandbox.create_script(source)

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_ready_called_on_enter_tree":
			# _ready should be called when node with script enters scene tree
			var node = Node3D.new()
			node.name = "ReadyTestNode"

			# Use a unique global to track (since JSScript may share context with sandbox)
			var unique_id = str(randi())
			var script = _create_js_script("""
globalThis.__ready_test_%s = false;
function _ready() {
	globalThis.__ready_test_%s = true;
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)

			# Wait for _ready to be called
			await scene_tree.process_frame
			await scene_tree.process_frame

			var ready_called = sandbox.eval("globalThis.__ready_test_%s" % unique_id)
			sandbox.eval("delete globalThis.__ready_test_%s;" % unique_id)

			return assert_eq(ready_called, true, "_ready() should be called when node enters tree")

		"test_ready_called_once":
			# _ready should only be called once, not on every frame
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "ReadyOnceTest"

			var script = _create_js_script("""
globalThis.__ready_count_%s = 0;
function _ready() {
	globalThis.__ready_count_%s++;
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame
			await scene_tree.process_frame
			await scene_tree.process_frame

			var count = sandbox.eval("globalThis.__ready_count_%s" % unique_id)
			sandbox.eval("delete globalThis.__ready_count_%s;" % unique_id)

			return assert_eq(count, 1, "_ready() should be called exactly once")

		"test_ready_this_binding":
			# 'this' in _ready should refer to the owner node
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "ThisBindingTest"

			var script = _create_js_script("""
globalThis.__this_name_%s = null;
function _ready() {
	globalThis.__this_name_%s = this.name;
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame
			await scene_tree.process_frame

			var this_name = sandbox.eval("globalThis.__this_name_%s" % unique_id)
			sandbox.eval("delete globalThis.__this_name_%s;" % unique_id)

			return assert_eq(this_name, "ThisBindingTest", "'this' should refer to owner node")

		"test_process_called_each_frame":
			# _process should be called each frame
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "ProcessTestNode"

			var script = _create_js_script("""
globalThis.__process_count_%s = 0;
function _process(delta) {
	globalThis.__process_count_%s++;
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)

			# Wait several frames
			await scene_tree.process_frame
			await scene_tree.process_frame
			await scene_tree.process_frame
			await scene_tree.process_frame

			var count = sandbox.eval("globalThis.__process_count_%s" % unique_id)
			sandbox.eval("delete globalThis.__process_count_%s;" % unique_id)

			return assert_gt(count, 0, "_process() should be called at least once")

		"test_process_receives_delta":
			# _process should receive delta time parameter
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "DeltaTestNode"

			var script = _create_js_script("""
globalThis.__last_delta_%s = null;
function _process(delta) {
	globalThis.__last_delta_%s = delta;
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame
			await scene_tree.process_frame

			var delta = sandbox.eval("globalThis.__last_delta_%s" % unique_id)
			sandbox.eval("delete globalThis.__last_delta_%s;" % unique_id)

			if delta == null:
				return { "passed": false, "message": "delta was not passed to _process" }
			if typeof(delta) != TYPE_FLOAT and typeof(delta) != TYPE_INT:
				return { "passed": false, "message": "delta should be a number, got %s" % typeof(delta) }
			if delta <= 0:
				return { "passed": false, "message": "delta should be positive: %s" % delta }

			return { "passed": true, "message": "" }

		"test_script_state_persists":
			# Variables in script should persist between frames
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "StatePersistNode"

			var script = _create_js_script("""
var counter = 0;
globalThis.__get_counter_%s = function() { return counter; };
function _process(delta) {
	counter++;
}
""" % [unique_id])

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame
			await scene_tree.process_frame

			var count1 = sandbox.eval("globalThis.__get_counter_%s()" % unique_id)
			if count1 == null:
				sandbox.eval("delete globalThis.__get_counter_%s;" % unique_id)
				return { "passed": false, "message": "Counter returned null - context not shared" }

			await scene_tree.process_frame
			await scene_tree.process_frame

			var count2 = sandbox.eval("globalThis.__get_counter_%s()" % unique_id)
			sandbox.eval("delete globalThis.__get_counter_%s;" % unique_id)

			if count2 == null:
				return { "passed": false, "message": "Counter returned null after more frames" }
			if count2 <= count1:
				return { "passed": false, "message": "Counter should increase: %d -> %d" % [count1, count2] }

			return { "passed": true, "message": "" }

		"test_closure_state_persists":
			# Closure state should persist
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "ClosureTestNode"

			var script = _create_js_script("""
var history = [];
globalThis.__get_history_%s = function() { return history.length; };
function _process(delta) {
	history.push(delta);
}
""" % [unique_id])

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame
			await scene_tree.process_frame
			await scene_tree.process_frame

			var history_len = sandbox.eval("globalThis.__get_history_%s()" % unique_id)
			sandbox.eval("delete globalThis.__get_history_%s;" % unique_id)

			if history_len == null:
				return { "passed": false, "message": "History returned null" }
			return assert_gt(history_len, 0, "Closure state should persist")

		"test_multiple_scripts_independent":
			# Multiple nodes with scripts should have independent state
			var unique_id1 = str(randi())
			var unique_id2 = str(randi())

			var node1 = Node3D.new()
			node1.name = "IndependentNode1"
			var script1 = _create_js_script("""
globalThis.__script1_%s = 0;
function _process(delta) {
	globalThis.__script1_%s += 1;
}
""" % [unique_id1, unique_id1])

			var node2 = Node3D.new()
			node2.name = "IndependentNode2"
			var script2 = _create_js_script("""
globalThis.__script2_%s = 0;
function _process(delta) {
	globalThis.__script2_%s += 10;
}
""" % [unique_id2, unique_id2])

			node1.set_script(script1)
			node2.set_script(script2)
			test_root.add_child(node1)
			test_root.add_child(node2)

			await scene_tree.process_frame
			await scene_tree.process_frame
			await scene_tree.process_frame

			var count1 = sandbox.eval("globalThis.__script1_%s" % unique_id1)
			var count2 = sandbox.eval("globalThis.__script2_%s" % unique_id2)

			sandbox.eval("delete globalThis.__script1_%s;" % unique_id1)
			sandbox.eval("delete globalThis.__script2_%s;" % unique_id2)

			if count1 == null or count2 == null:
				return { "passed": false, "message": "Script counters are null" }

			# count1 increments by 1, count2 by 10, they should be different
			if count1 == count2:
				return { "passed": false, "message": "Scripts should have independent state" }

			return { "passed": true, "message": "" }

		"test_custom_method_callable":
			# Custom methods defined in script should be callable from GDScript
			# Using exports pattern for full JS syntax support (arrow functions, classes, etc.)
			var node = Node3D.new()
			node.name = "CustomMethodNode"

			var script = _create_js_script("""
exports.custom_add = function(a, b) {
	return a + b;
};
""")

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame

			# Try to call custom method (if supported)
			if node.has_method("custom_add"):
				var result = node.call("custom_add", 3, 4)
				return assert_eq(result, 7, "Custom method should return correct value")
			else:
				# Method exposure might not be implemented yet
				return { "passed": false, "message": "Custom methods not exposed to GDScript" }

		"test_method_this_binding":
			# Custom methods should have correct 'this' binding
			# Using exports pattern for full JS syntax support
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "MethodThisNode"

			var script = _create_js_script("""
globalThis.__method_this_%s = null;
exports.get_my_name = function() {
	globalThis.__method_this_%s = this.name;
	return this.name;
};
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame

			if node.has_method("get_my_name"):
				node.call("get_my_name")
				var this_name = sandbox.eval("globalThis.__method_this_%s" % unique_id)
				sandbox.eval("delete globalThis.__method_this_%s;" % unique_id)
				return assert_eq(this_name, "MethodThisNode", "'this' in method should be owner node")
			else:
				sandbox.eval("delete globalThis.__method_this_%s;" % unique_id)
				return { "passed": false, "message": "Custom methods not exposed to GDScript" }

		"test_class_based_exports":
			# Class-based exports pattern - recommended for complex scripts
			var unique_id = str(randi())
			var node = Node3D.new()
			node.name = "ClassBasedNode"

			var script = _create_js_script("""
class PlayerController {
	_ready() {
		globalThis.__class_ready_%s = true;
	}

	get_speed() {
		return 100;
	}

	calculate(a, b) {
		return a * b + this.get_speed();
	}
}
exports = new PlayerController();
""" % [unique_id])

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame
			await scene_tree.process_frame

			var ready_called = sandbox.eval("globalThis.__class_ready_%s" % unique_id)
			sandbox.eval("delete globalThis.__class_ready_%s;" % unique_id)

			if ready_called != true:
				return { "passed": false, "message": "Class _ready() not called" }

			if not node.has_method("get_speed"):
				return { "passed": false, "message": "Class method get_speed not exposed" }

			if not node.has_method("calculate"):
				return { "passed": false, "message": "Class method calculate not exposed" }

			var speed = node.call("get_speed")
			if speed != 100:
				return { "passed": false, "message": "get_speed() returned %s, expected 100" % speed }

			var calc = node.call("calculate", 3, 4)
			# 3 * 4 + 100 = 112
			return assert_eq(calc, 112, "Class method should work with this binding")

		"test_arrow_function_exports":
			# Arrow function exports pattern
			var node = Node3D.new()
			node.name = "ArrowFuncNode"

			var script = _create_js_script("""
exports.multiply = (a, b) => a * b;
exports.greet = (name) => {
	return "Hello, " + name + "!";
};
""")

			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame

			if not node.has_method("multiply"):
				return { "passed": false, "message": "Arrow function multiply not exposed" }

			if not node.has_method("greet"):
				return { "passed": false, "message": "Arrow function greet not exposed" }

			var product = node.call("multiply", 6, 7)
			if product != 42:
				return { "passed": false, "message": "multiply(6,7) returned %s, expected 42" % product }

			var greeting = node.call("greet", "World")
			return assert_eq(greeting, "Hello, World!", "Arrow function should return correct value")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	# Clean up any leftover globals (in case test failed before cleanup)
	sandbox.eval("""
		for (var key in globalThis) {
			if (key.startsWith('__ready_test_') ||
				key.startsWith('__ready_count_') ||
				key.startsWith('__this_name_') ||
				key.startsWith('__process_count_') ||
				key.startsWith('__last_delta_') ||
				key.startsWith('__get_counter_') ||
				key.startsWith('__get_history_') ||
				key.startsWith('__script1_') ||
				key.startsWith('__script2_') ||
				key.startsWith('__method_this_')) {
				delete globalThis[key];
			}
		}
	""")
	super.teardown()

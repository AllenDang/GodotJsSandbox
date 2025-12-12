class_name TestInput
extends TestBase

## Tests for input callbacks (_input, _unhandled_input, etc.)
## Reference: JSScriptInstance input handling

# Track nodes created in tests for cleanup
var _test_nodes: Array[Node] = []

func get_suite_name() -> String:
	return "Input"

func get_tests() -> Array[String]:
	return [
		# _input callback
		"test_input_method_registered",
		"test_input_receives_key_event",
		"test_input_receives_mouse_button_event",
		"test_input_receives_mouse_motion_event",
		"test_input_this_binding",
		# _unhandled_input callback
		"test_unhandled_input_registered",
		"test_unhandled_input_receives_event",
	]

func _create_js_script(source: String) -> Script:
	return sandbox.create_script(source)

func _create_test_node() -> Node2D:
	var node = Node2D.new()
	_test_nodes.append(node)
	return node

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_input_method_registered":
			# Test that _input method is recognized
			var node = _create_test_node()
			node.name = "InputTestNode"

			var script = _create_js_script("""
function _input(event) {
	// Just having the method is enough
}
""")
			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame

			var has_input = node.has_method("_input")
			return assert_true(has_input, "Node should have _input method")

		"test_input_receives_key_event":
			# Test that _input receives InputEventKey properties
			var unique_id = str(randi())
			var node = _create_test_node()
			node.name = "KeyInputTest"

			var script = _create_js_script("""
globalThis.__key_event_%s = null;
function _input(event) {
	if (event.type && event.type.indexOf('Key') >= 0) {
		globalThis.__key_event_%s = {
			type: event.type,
			pressed: event.pressed,
			keycode: event.keycode,
			physical_keycode: event.physical_keycode
		};
	}
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)
			await scene_tree.process_frame

			# Simulate a key press
			var key_event = InputEventKey.new()
			key_event.keycode = KEY_SPACE
			key_event.physical_keycode = KEY_SPACE
			key_event.pressed = true

			# Push the input event
			Input.parse_input_event(key_event)
			await scene_tree.process_frame
			await scene_tree.process_frame

			var result = sandbox.get_global("__key_event_%s" % unique_id)
			sandbox.eval("delete globalThis.__key_event_%s;" % unique_id)

			if result == null:
				return { "passed": false, "message": "Key event not received by _input" }

			if typeof(result) != TYPE_DICTIONARY:
				return { "passed": false, "message": "Result should be dictionary, got %s" % type_string(typeof(result)) }

			var dict = result as Dictionary
			if not ("type" in dict):
				return { "passed": false, "message": "Event should have type property" }

			if not ("pressed" in dict):
				return { "passed": false, "message": "Event should have pressed property" }

			return assert_true(dict["pressed"], "Event should be pressed")

		"test_input_receives_mouse_button_event":
			# Test that _input receives InputEventMouseButton properties
			var unique_id = str(randi())
			var node = _create_test_node()
			node.name = "MouseButtonTest"

			var script = _create_js_script("""
globalThis.__mouse_event_%s = null;
function _input(event) {
	if (event.type && event.type.indexOf('MouseButton') >= 0) {
		globalThis.__mouse_event_%s = {
			type: event.type,
			pressed: event.pressed,
			button_index: event.button_index,
			position: event.position
		};
	}
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)
			await scene_tree.process_frame

			# Simulate mouse click
			var mouse_event = InputEventMouseButton.new()
			mouse_event.button_index = MOUSE_BUTTON_LEFT
			mouse_event.pressed = true
			mouse_event.position = Vector2(100, 200)

			Input.parse_input_event(mouse_event)
			await scene_tree.process_frame
			await scene_tree.process_frame

			var result = sandbox.get_global("__mouse_event_%s" % unique_id)
			sandbox.eval("delete globalThis.__mouse_event_%s;" % unique_id)

			if result == null:
				return { "passed": false, "message": "Mouse event not received by _input" }

			var dict = result as Dictionary
			if not ("button_index" in dict):
				return { "passed": false, "message": "Event should have button_index" }

			if not ("position" in dict):
				return { "passed": false, "message": "Event should have position" }

			var pos = dict["position"]
			if typeof(pos) != TYPE_DICTIONARY and typeof(pos) != TYPE_VECTOR2:
				return { "passed": false, "message": "Position should be object/Vector2" }

			return { "passed": true, "message": "" }

		"test_input_receives_mouse_motion_event":
			# Test that _input receives InputEventMouseMotion properties
			var unique_id = str(randi())
			var node = _create_test_node()
			node.name = "MouseMotionTest"

			var script = _create_js_script("""
globalThis.__motion_event_%s = null;
function _input(event) {
	if (event.type && event.type.indexOf('MouseMotion') >= 0) {
		globalThis.__motion_event_%s = {
			type: event.type,
			position: event.position,
			relative: event.relative,
			velocity: event.velocity
		};
	}
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)
			await scene_tree.process_frame

			# Simulate mouse motion
			var motion_event = InputEventMouseMotion.new()
			motion_event.position = Vector2(100, 150)
			motion_event.relative = Vector2(10, 5)
			motion_event.velocity = Vector2(100, 50)

			Input.parse_input_event(motion_event)
			await scene_tree.process_frame
			await scene_tree.process_frame

			var result = sandbox.get_global("__motion_event_%s" % unique_id)
			sandbox.eval("delete globalThis.__motion_event_%s;" % unique_id)

			if result == null:
				return { "passed": false, "message": "Motion event not received by _input" }

			var dict = result as Dictionary
			if not ("relative" in dict):
				return { "passed": false, "message": "Event should have relative property" }

			if not ("velocity" in dict):
				return { "passed": false, "message": "Event should have velocity property" }

			return { "passed": true, "message": "" }

		"test_input_this_binding":
			# Test that 'this' in _input refers to owner node
			var unique_id = str(randi())
			var node = _create_test_node()
			node.name = "InputThisTest"

			var script = _create_js_script("""
globalThis.__input_this_%s = null;
function _input(event) {
	globalThis.__input_this_%s = this.name;
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)
			await scene_tree.process_frame

			# Simulate input
			var key_event = InputEventKey.new()
			key_event.keycode = KEY_A
			key_event.pressed = true

			Input.parse_input_event(key_event)
			await scene_tree.process_frame
			await scene_tree.process_frame

			var this_name = sandbox.get_global("__input_this_%s" % unique_id)
			sandbox.eval("delete globalThis.__input_this_%s;" % unique_id)

			return assert_eq(this_name, "InputThisTest", "'this' in _input should be owner node")

		"test_unhandled_input_registered":
			# Test that _unhandled_input method is recognized
			var node = _create_test_node()
			node.name = "UnhandledInputNode"

			var script = _create_js_script("""
function _unhandled_input(event) {
	// Just having the method is enough
}
""")
			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame

			var has_method = node.has_method("_unhandled_input")
			return assert_true(has_method, "Node should have _unhandled_input method")

		"test_unhandled_input_receives_event":
			# Test that _unhandled_input receives events
			var unique_id = str(randi())
			var node = _create_test_node()
			node.name = "UnhandledTest"

			var script = _create_js_script("""
globalThis.__unhandled_%s = false;
function _unhandled_input(event) {
	globalThis.__unhandled_%s = true;
}
""" % [unique_id, unique_id])

			node.set_script(script)
			test_root.add_child(node)
			await scene_tree.process_frame

			# Simulate input that should reach unhandled_input
			var key_event = InputEventKey.new()
			key_event.keycode = KEY_B
			key_event.pressed = true

			Input.parse_input_event(key_event)
			await scene_tree.process_frame
			await scene_tree.process_frame

			var result = sandbox.get_global("__unhandled_%s" % unique_id)
			sandbox.eval("delete globalThis.__unhandled_%s;" % unique_id)

			return assert_true(result, "_unhandled_input should receive event")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	# Clean up globals
	sandbox.eval("""
		for (var key in globalThis) {
			if (key.startsWith('__key_event_') ||
				key.startsWith('__mouse_event_') ||
				key.startsWith('__motion_event_') ||
				key.startsWith('__input_this_') ||
				key.startsWith('__unhandled_')) {
				delete globalThis[key];
			}
		}
	""")
	# Explicitly free all test nodes before super.teardown()
	for node in _test_nodes:
		if is_instance_valid(node):
			node.set_script(null)  # Clear script first to avoid any issues
			node.queue_free()
	_test_nodes.clear()
	super.teardown()

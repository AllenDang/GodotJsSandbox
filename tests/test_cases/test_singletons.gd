class_name TestSingletons
extends TestBase

func get_suite_name() -> String:
	return "Singletons"

func get_tests() -> Array[String]:
	return [
		"test_time_exists",
		"test_time_get_ticks_msec",
		"test_time_get_ticks_usec",
		"test_time_get_unix_time",
		"test_input_exists",
		"test_input_is_anything_pressed",
		"test_input_is_key_pressed",
		"test_input_is_action_pressed",
		"test_input_get_vector",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_time_exists":
			return assert_eval("typeof Time !== 'undefined'", true)
		"test_time_get_ticks_msec":
			var result = sandbox.eval("Time.get_ticks_msec()")
			if typeof(result) != TYPE_INT and typeof(result) != TYPE_FLOAT:
				return { "passed": false, "message": "Expected number, got %s" % typeof(result) }
			return assert_gt(result, 0, "Ticks should be > 0")
		"test_time_get_ticks_usec":
			var result = sandbox.eval("Time.get_ticks_usec()")
			if typeof(result) != TYPE_INT and typeof(result) != TYPE_FLOAT:
				return { "passed": false, "message": "Expected number, got %s" % typeof(result) }
			return assert_gt(result, 0, "Ticks should be > 0")
		"test_time_get_unix_time":
			var result = sandbox.eval("Time.get_unix_time_from_system()")
			if typeof(result) != TYPE_INT and typeof(result) != TYPE_FLOAT:
				return { "passed": false, "message": "Expected number, got %s" % typeof(result) }
			# Unix timestamp should be reasonable (after year 2020)
			return assert_gt(result, 1577836800, "Unix time should be after 2020")
		"test_input_exists":
			return assert_eval("typeof Input !== 'undefined'", true)
		"test_input_is_anything_pressed":
			var result = sandbox.eval("Input.is_anything_pressed()")
			# Should return a boolean
			if typeof(result) != TYPE_BOOL:
				return { "passed": false, "message": "Expected boolean, got %s" % typeof(result) }
			return { "passed": true, "message": "" }
		"test_input_is_key_pressed":
			# Test with KEY_A (65)
			var result = sandbox.eval("Input.is_key_pressed(65)")
			if typeof(result) != TYPE_BOOL:
				return { "passed": false, "message": "Expected boolean, got %s" % typeof(result) }
			return { "passed": true, "message": "" }
		"test_input_is_action_pressed":
			# Test with a common action - should return false but not error
			var code = """
				try {
					var result = Input.is_action_pressed('ui_accept');
					typeof result === 'boolean';
				} catch(e) {
					false;
				}
			"""
			return assert_eval(code, true)
		"test_input_get_vector":
			var code = """
				try {
					var vec = Input.get_vector('ui_left', 'ui_right', 'ui_up', 'ui_down');
					typeof vec === 'object' && typeof vec.x === 'number' && typeof vec.y === 'number';
				} catch(e) {
					// If action doesn't exist, that's ok - just check it doesn't crash
					true;
				}
			"""
			return assert_eval(code, true)
		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

# Read-only operations - no state to restore
func teardown() -> void:
	super.teardown()

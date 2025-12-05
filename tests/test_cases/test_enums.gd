class_name TestEnums
extends TestBase

## Tests for global enums and singleton class enums
## Reference: Global enums (Key, MouseButton, Error) and singleton enums (Input.MOUSE_MODE_*)

func get_suite_name() -> String:
	return "Enums"

func get_tests() -> Array[String]:
	return [
		# Global enum tests
		"test_key_enum_exists",
		"test_key_enum_values",
		"test_mouse_button_enum_exists",
		"test_mouse_button_enum_values",
		"test_error_enum_exists",
		"test_error_enum_values",
		# Singleton enum tests
		"test_input_mouse_mode_enum_exists",
		"test_input_mouse_mode_enum_values",
		"test_input_cursor_shape_enum_exists",
		"test_input_set_mouse_mode_function",
		"test_input_get_mouse_mode_function",
		# Integration tests - calling functions with enum values
		"test_input_set_mouse_mode_call",
		"test_input_get_mouse_mode_returns_number",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		# ===== Global Enum Tests =====
		"test_key_enum_exists":
			# Test that Key global enum exists
			var code = "typeof Key === 'object' && Key !== null"
			return assert_eval(code, true)

		"test_key_enum_values":
			# Test specific Key enum values
			var code = """
				Key.KEY_ESCAPE === 4194305 &&
				Key.KEY_ENTER === 4194309 &&
				Key.KEY_SPACE === 32 &&
				Key.KEY_A === 65
			"""
			return assert_eval(code, true)

		"test_mouse_button_enum_exists":
			# Test that MouseButton global enum exists
			var code = "typeof MouseButton === 'object' && MouseButton !== null"
			return assert_eval(code, true)

		"test_mouse_button_enum_values":
			# Test specific MouseButton enum values
			var code = """
				MouseButton.MOUSE_BUTTON_NONE === 0 &&
				MouseButton.MOUSE_BUTTON_LEFT === 1 &&
				MouseButton.MOUSE_BUTTON_RIGHT === 2 &&
				MouseButton.MOUSE_BUTTON_MIDDLE === 3
			"""
			return assert_eval(code, true)

		"test_error_enum_exists":
			# Test that Error global enum exists
			var code = "typeof Error === 'object' && Error !== null"
			return assert_eval(code, true)

		"test_error_enum_values":
			# Test specific Error enum values
			var code = """
				Error.OK === 0 &&
				Error.FAILED === 1 &&
				Error.ERR_FILE_NOT_FOUND === 7
			"""
			return assert_eval(code, true)

		# ===== Singleton Enum Tests =====
		"test_input_mouse_mode_enum_exists":
			# Test that Input singleton has MOUSE_MODE_* enums
			var code = """
				typeof Input === 'object' &&
				typeof Input.MOUSE_MODE_VISIBLE !== 'undefined' &&
				typeof Input.MOUSE_MODE_CAPTURED !== 'undefined'
			"""
			return assert_eval(code, true)

		"test_input_mouse_mode_enum_values":
			# Test specific Input.MOUSE_MODE_* values
			var code = """
				Input.MOUSE_MODE_VISIBLE === 0 &&
				Input.MOUSE_MODE_HIDDEN === 1 &&
				Input.MOUSE_MODE_CAPTURED === 2 &&
				Input.MOUSE_MODE_CONFINED === 3
			"""
			return assert_eval(code, true)

		"test_input_cursor_shape_enum_exists":
			# Test that Input singleton has CURSOR_* enums
			var code = """
				typeof Input.CURSOR_ARROW !== 'undefined' &&
				typeof Input.CURSOR_IBEAM !== 'undefined'
			"""
			return assert_eval(code, true)

		"test_input_set_mouse_mode_function":
			# Test that Input.set_mouse_mode is a function
			var code = "typeof Input.set_mouse_mode === 'function'"
			return assert_eval(code, true)

		"test_input_get_mouse_mode_function":
			# Test that Input.get_mouse_mode is a function
			var code = "typeof Input.get_mouse_mode === 'function'"
			return assert_eval(code, true)

		"test_input_set_mouse_mode_call":
			# Test actually calling Input.set_mouse_mode with enum value
			var code = """
				try {
					Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE);
					true;
				} catch (e) {
					'Error: ' + e.message;
				}
			"""
			return assert_eval(code, true)

		"test_input_get_mouse_mode_returns_number":
			# Test that Input.get_mouse_mode returns a number
			var code = "typeof Input.get_mouse_mode() === 'number'"
			return assert_eval(code, true)

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

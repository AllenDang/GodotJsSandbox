class_name TestGlobalConstants
extends TestBase

## Tests for global constants (autoloads) accessibility from JavaScript
## Verifies that GDScript autoloads registered in project.godot are accessible
## from JavaScript code running inside JSSandbox.

func get_suite_name() -> String:
	return "Global Constants"

func get_tests() -> Array[String]:
	return [
		"test_autoload_exists",
		"test_autoload_type",
		"test_autoload_has_handle",
		"test_autoload_property_read",
		"test_autoload_method_type",
		"test_autoload_method_call",
		"test_autoload_method_with_args",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_autoload_exists":
			# TestGlobals autoload should be accessible as a global
			return assert_eval("typeof TestGlobals !== 'undefined'", true,
				"TestGlobals autoload should be defined")

		"test_autoload_type":
			# Check what type TestGlobals is
			var result = sandbox.eval("typeof TestGlobals")
			return assert_eq(result, "object", "TestGlobals should be an object, got: " + str(result))

		"test_autoload_has_handle":
			# Check if it has __handle (indicates it's a wrapped Godot object)
			var result = sandbox.eval("TestGlobals.__handle !== undefined")
			return assert_eq(result, true, "TestGlobals should have __handle property")

		"test_autoload_property_read":
			# Should be able to read properties from the autoload
			var result = sandbox.eval("TestGlobals.test_value")
			return assert_eq(result, 42, "Should read test_value property as 42")

		"test_autoload_method_type":
			# Debug: check what type the method access returns
			var result = sandbox.eval("typeof TestGlobals.get_doubled_value")
			return assert_eq(result, "function", "Method should be a function, got: " + str(result))

		"test_autoload_method_call":
			# Should be able to call methods on the autoload
			var result = sandbox.eval("TestGlobals.get_doubled_value()")
			return assert_eq(result, 84, "get_doubled_value() should return 84")

		"test_autoload_method_with_args":
			# Should be able to call methods with arguments
			var result = sandbox.eval("TestGlobals.greet('World')")
			return assert_eq(result, "Hello, World!", "greet('World') should return greeting")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	super.teardown()

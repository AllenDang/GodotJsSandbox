extends TestBase
## Tests for error capture system - captures errors for AI feedback

func get_suite_name() -> String:
	return "Error Capture"

func get_tests() -> Array[String]:
	return [
		"test_js_syntax_error_captured",
		"test_js_runtime_error_captured",
		"test_error_has_line_info",
		"test_multiple_errors_accumulated",
		"test_clear_errors",
		"test_get_all_errors_returns_array",
		"test_error_has_type_field",
		"test_missing_file_error",
		"test_script_error_has_source_context",
	]

func run_test(test_name: String) -> Dictionary:
	# Clear errors before each test
	sandbox.clear_errors()

	match test_name:
		"test_js_syntax_error_captured":
			return test_js_syntax_error_captured()
		"test_js_runtime_error_captured":
			return test_js_runtime_error_captured()
		"test_error_has_line_info":
			return test_error_has_line_info()
		"test_multiple_errors_accumulated":
			return test_multiple_errors_accumulated()
		"test_clear_errors":
			return test_clear_errors()
		"test_get_all_errors_returns_array":
			return test_get_all_errors_returns_array()
		"test_error_has_type_field":
			return test_error_has_type_field()
		"test_missing_file_error":
			return test_missing_file_error()
		"test_script_error_has_source_context":
			return await test_script_error_has_source_context()
	return { "passed": false, "message": "Unknown test" }


func test_js_syntax_error_captured() -> Dictionary:
	# Intentional syntax error - missing closing paren
	sandbox.eval("function test( { return 1; }")

	var error = sandbox.get_last_error()
	if error.is_empty():
		return { "passed": false, "message": "Syntax error was not captured" }

	return { "passed": true, "message": "" }


func test_js_runtime_error_captured() -> Dictionary:
	# Runtime error - accessing undefined property
	sandbox.eval("var x = undefined; x.foo.bar;")

	var error = sandbox.get_last_error()
	if error.is_empty():
		return { "passed": false, "message": "Runtime error was not captured" }

	return { "passed": true, "message": "" }


func test_error_has_line_info() -> Dictionary:
	# Multi-line code with error on specific line
	var code = """
var a = 1;
var b = 2;
var c = undefined;
c.property;
"""
	sandbox.eval(code)

	var errors = sandbox.get_all_errors()
	if errors.is_empty():
		return { "passed": false, "message": "No errors captured" }

	var error: Dictionary = errors[0]
	# Check that we have line information (may be 0 if QuickJS doesn't provide it)
	if not error.has("line"):
		return { "passed": false, "message": "Error missing 'line' field" }

	return { "passed": true, "message": "" }


func test_multiple_errors_accumulated() -> Dictionary:
	# Generate multiple errors
	sandbox.eval("syntax error here {{{")
	sandbox.eval("another.error.here")
	sandbox.eval("third(error")

	var errors = sandbox.get_all_errors()
	if errors.size() < 2:
		return { "passed": false, "message": "Expected multiple errors, got %d" % errors.size() }

	return { "passed": true, "message": "" }


func test_clear_errors() -> Dictionary:
	# Generate an error
	sandbox.eval("syntax {{{ error")

	var errors_before = sandbox.get_all_errors()
	if errors_before.is_empty():
		return { "passed": false, "message": "Error was not captured initially" }

	# Clear errors
	sandbox.clear_errors()

	var errors_after = sandbox.get_all_errors()
	if not errors_after.is_empty():
		return { "passed": false, "message": "Errors not cleared, still have %d" % errors_after.size() }

	var last_error = sandbox.get_last_error()
	if not last_error.is_empty():
		return { "passed": false, "message": "last_error not cleared" }

	return { "passed": true, "message": "" }


func test_get_all_errors_returns_array() -> Dictionary:
	var errors = sandbox.get_all_errors()

	if typeof(errors) != TYPE_ARRAY:
		return { "passed": false, "message": "get_all_errors() should return Array, got %s" % typeof(errors) }

	return { "passed": true, "message": "" }


func test_error_has_type_field() -> Dictionary:
	# Generate a JS error
	sandbox.eval("undefined.foo")

	var errors = sandbox.get_all_errors()
	if errors.is_empty():
		return { "passed": false, "message": "No errors captured" }

	var error: Dictionary = errors[0]
	if not error.has("type"):
		return { "passed": false, "message": "Error missing 'type' field" }

	if not error.has("message"):
		return { "passed": false, "message": "Error missing 'message' field" }

	return { "passed": true, "message": "" }


func test_missing_file_error() -> Dictionary:
	# Try to eval a file that doesn't exist
	sandbox.eval_file("user://nonexistent_script_12345.js")

	var error = sandbox.get_last_error()
	if error.is_empty():
		return { "passed": false, "message": "Missing file error not captured" }

	if error.find("nonexistent") < 0 and error.find("open") < 0 and error.find("not found") < 0:
		return { "passed": false, "message": "Error message doesn't mention the file issue: %s" % error }

	return { "passed": true, "message": "" }


func test_script_error_has_source_context() -> Dictionary:
	# This test verifies that script instance errors are captured by the sandbox.
	# This allows AI to access errors via sandbox.get_last_error() for debugging.

	# Clear any existing errors
	sandbox.clear_errors()

	# Create a script that throws an error when called
	var node = Node2D.new()
	node.name = "SourceContextTestNode"

	# Multi-line script with an error on a specific line
	var script = sandbox.create_script("""
var counter = 0;

exports._ready = function() {
    // This line will cause an error - calling undefined as function
    var result = undefined();
};
""")

	node.set_script(script)
	test_root.add_child(node)

	# Wait for _ready to be called and error to occur
	await scene_tree.process_frame

	# Clean up
	node.queue_free()

	# Verify the error was captured by the sandbox
	var error = sandbox.get_last_error()
	if error.is_empty():
		return { "passed": false, "message": "Script instance error was not captured by sandbox" }

	# Verify error contains source context
	if error.find("Source context") < 0:
		return { "passed": false, "message": "Error missing source context: " + error }

	# Verify error has line numbers (the >> marker)
	if error.find(">>") < 0:
		return { "passed": false, "message": "Error missing line marker: " + error }

	return { "passed": true, "message": "Script error captured with source context" }

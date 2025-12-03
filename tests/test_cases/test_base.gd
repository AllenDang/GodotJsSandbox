class_name TestBase
extends RefCounted

var sandbox: JSSandbox
var scene_tree: SceneTree
var test_root: Node  # Temporary node for test objects

# Called before any tests run
func setup(p_sandbox: JSSandbox, p_scene_tree: SceneTree) -> void:
	sandbox = p_sandbox
	scene_tree = p_scene_tree
	test_root = Node.new()
	test_root.name = "TestRoot"
	scene_tree.root.add_child.call_deferred(test_root)
	# Wait for deferred add to complete
	await scene_tree.process_frame

# Called after all tests complete - MUST restore state
func teardown() -> void:
	# Clean up all test nodes
	if test_root and is_instance_valid(test_root):
		test_root.queue_free()
	test_root = null

# Override in subclasses
func get_suite_name() -> String:
	return "Unknown"

# Override in subclasses - return list of test method names
func get_tests() -> Array[String]:
	return []

# Override in subclasses
func run_test(test_name: String) -> Dictionary:
	# Returns: { "passed": bool, "message": String }
	return { "passed": false, "message": "Not implemented" }

# Helper: assert condition
func assert_true(condition: bool, message: String = "") -> Dictionary:
	if condition:
		return { "passed": true, "message": "" }
	return { "passed": false, "message": message if message else "Assertion failed" }

# Helper: assert false
func assert_false(condition: bool, message: String = "") -> Dictionary:
	return assert_true(not condition, message if message else "Expected false but got true")

# Helper: assert equality
func assert_eq(actual: Variant, expected: Variant, message: String = "") -> Dictionary:
	if actual == expected:
		return { "passed": true, "message": "" }
	var msg = message if message else "Expected %s but got %s" % [expected, actual]
	return { "passed": false, "message": msg }

# Helper: assert not equal
func assert_neq(actual: Variant, expected: Variant, message: String = "") -> Dictionary:
	if actual != expected:
		return { "passed": true, "message": "" }
	var msg = message if message else "Expected value to not equal %s" % [expected]
	return { "passed": false, "message": msg }

# Helper: assert greater than
func assert_gt(actual: Variant, expected: Variant, message: String = "") -> Dictionary:
	if actual == null:
		return { "passed": false, "message": message if message else "Expected value > %s but got null" % [expected] }
	if actual > expected:
		return { "passed": true, "message": "" }
	var msg = message if message else "Expected %s > %s" % [actual, expected]
	return { "passed": false, "message": msg }

# Helper: assert JS eval returns expected value
func assert_eval(code: String, expected: Variant, message: String = "") -> Dictionary:
	var result = sandbox.eval(code)
	return assert_eq(result, expected, message if message else "eval('%s') expected %s but got %s" % [code.substr(0, 50), expected, result])

# Helper: assert JS eval throws error
func assert_throws(code: String, message: String = "") -> Dictionary:
	sandbox.eval(code)
	var error = sandbox.get_last_error()
	if error != "":
		return { "passed": true, "message": "" }
	return { "passed": false, "message": message if message else "Expected error but none thrown" }

# Helper: assert JS eval does not throw error
func assert_no_error(code: String, message: String = "") -> Dictionary:
	sandbox.eval(code)
	var error = sandbox.get_last_error()
	if error == "":
		return { "passed": true, "message": "" }
	return { "passed": false, "message": message if message else "Unexpected error: %s" % error }

# Helper: assert value is not null
func assert_not_null(value: Variant, message: String = "") -> Dictionary:
	if value != null:
		return { "passed": true, "message": "" }
	return { "passed": false, "message": message if message else "Expected non-null value" }

# Helper: assert value is null
func assert_null(value: Variant, message: String = "") -> Dictionary:
	if value == null:
		return { "passed": true, "message": "" }
	return { "passed": false, "message": message if message else "Expected null but got %s" % value }

# Helper: assert type
func assert_type(value: Variant, expected_type: int, message: String = "") -> Dictionary:
	if typeof(value) == expected_type:
		return { "passed": true, "message": "" }
	return { "passed": false, "message": message if message else "Expected type %s but got %s" % [expected_type, typeof(value)] }

class_name TestBasicEval
extends TestBase

func get_suite_name() -> String:
	return "Basic Eval"

func get_tests() -> Array[String]:
	return [
		"test_arithmetic",
		"test_string_concat",
		"test_array_operations",
		"test_object_literals",
		"test_function_definition",
		"test_closures",
		"test_json_parse",
		"test_json_stringify",
		"test_typeof",
		"test_comparison_operators",
		"test_logical_operators",
		"test_ternary_operator",
		"test_template_literals",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_arithmetic":
			return assert_eval("1 + 2 * 3", 7)
		"test_string_concat":
			return assert_eval("'Hello' + ' ' + 'World'", "Hello World")
		"test_array_operations":
			return assert_eval("[1, 2, 3].length", 3)
		"test_object_literals":
			var result = sandbox.eval("({x: 10, y: 20})")
			# Result could be Dictionary or Vector2 (if x,y detected)
			if typeof(result) == TYPE_DICTIONARY:
				if result.get("x") != 10 or result.get("y") != 20:
					return { "passed": false, "message": "Object properties incorrect: %s" % result }
				return { "passed": true, "message": "" }
			elif typeof(result) == TYPE_VECTOR2:
				# JS {x, y} objects are converted to Vector2
				if result.x != 10 or result.y != 20:
					return { "passed": false, "message": "Vector2 values incorrect: %s" % result }
				return { "passed": true, "message": "" }
			else:
				return { "passed": false, "message": "Expected dictionary or Vector2, got type %s: %s" % [typeof(result), result] }
		"test_function_definition":
			return assert_eval("(function add(a, b) { return a + b; })(3, 4)", 7)
		"test_closures":
			var code = """
				(function() {
					var counter = 0;
					return function() { return ++counter; };
				})()()
			"""
			return assert_eval(code, 1)
		"test_json_parse":
			var result = sandbox.eval('JSON.parse(\'{"name":"test","value":42}\')')
			if typeof(result) != TYPE_DICTIONARY:
				return { "passed": false, "message": "Expected dictionary" }
			return assert_eq(result.get("value"), 42)
		"test_json_stringify":
			return assert_eval('JSON.stringify({a: 1, b: 2})', '{"a":1,"b":2}')
		"test_typeof":
			var r1 = assert_eval("typeof 42", "number")
			if not r1.passed: return r1
			var r2 = assert_eval("typeof 'hello'", "string")
			if not r2.passed: return r2
			var r3 = assert_eval("typeof true", "boolean")
			if not r3.passed: return r3
			return assert_eval("typeof {}", "object")
		"test_comparison_operators":
			var r1 = assert_eval("5 > 3", true)
			if not r1.passed: return r1
			var r2 = assert_eval("5 === 5", true)
			if not r2.passed: return r2
			return assert_eval("5 !== '5'", true)
		"test_logical_operators":
			var r1 = assert_eval("true && false", false)
			if not r1.passed: return r1
			var r2 = assert_eval("true || false", true)
			if not r2.passed: return r2
			return assert_eval("!false", true)
		"test_ternary_operator":
			return assert_eval("true ? 'yes' : 'no'", "yes")
		"test_template_literals":
			return assert_eval("`Hello ${'World'}`", "Hello World")
		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

# No state to restore for pure eval tests
func teardown() -> void:
	super.teardown()

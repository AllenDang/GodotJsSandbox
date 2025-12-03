class_name TestExecutionLimits
extends TestBase

## Tests for execution limits (PRD.md Section 6 "执行限制")
## Reference: TDD.md Section 6.6 "ExecutionLimiter"
##
## Three layers of protection:
## 1. Time slice limit (timeout)
## 2. API call rate limiting
## 3. Instruction count (bytecode interrupt)
##
## Memory limit via JS_SetMemoryLimit

func get_suite_name() -> String:
	return "Execution Limits"

func get_tests() -> Array[String]:
	return [
		# Timeout protection
		"test_infinite_loop_interrupted",
		"test_long_computation_interrupted",
		"test_normal_code_completes",
		# Memory limits
		"test_memory_limit_enforced",
		"test_large_array_allocation_limited",
		"test_normal_allocation_works",
		# API call limits (if implemented)
		"test_rapid_api_calls",
		# Configuration
		"test_timeout_configurable",
		"test_memory_limit_configurable",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_infinite_loop_interrupted":
			# Infinite loop should be interrupted by timeout
			var code = """
				while (true) {
					// Infinite loop - should be interrupted
				}
			"""
			var start = Time.get_ticks_msec()
			var result = sandbox.eval(code)
			var elapsed = Time.get_ticks_msec() - start

			# Should have been interrupted, not run forever
			# Timeout is typically 1000-5000ms
			if elapsed > 10000:
				return { "passed": false, "message": "Infinite loop was not interrupted (ran %dms)" % elapsed }

			# Check if error was reported
			var error = sandbox.get_last_error()
			if error == "":
				return { "passed": false, "message": "No error reported for interrupted execution" }

			return { "passed": true, "message": "" }

		"test_long_computation_interrupted":
			# Long computation should be interrupted
			var code = """
				var sum = 0;
				for (var i = 0; i < 10000000000; i++) {
					sum += i;
				}
				sum;
			"""
			var start = Time.get_ticks_msec()
			var result = sandbox.eval(code)
			var elapsed = Time.get_ticks_msec() - start

			# Should be interrupted within timeout
			if elapsed > 10000:
				return { "passed": false, "message": "Long computation not interrupted" }

			return { "passed": true, "message": "" }

		"test_normal_code_completes":
			# Normal code should complete successfully
			var code = """
				var sum = 0;
				for (var i = 0; i < 1000; i++) {
					sum += i;
				}
				sum;
			"""
			var result = sandbox.eval(code)
			var expected = 0
			for i in range(1000):
				expected += i

			return assert_eq(result, expected, "Normal computation should complete")

		"test_memory_limit_enforced":
			# Allocating too much memory should fail
			var code = """
				try {
					var arrays = [];
					// Try to allocate way more than limit (e.g., > 64MB)
					for (var i = 0; i < 1000; i++) {
						arrays.push(new Array(1000000).fill(i));
					}
					false;  // Should not reach here
				} catch (e) {
					// Memory allocation should fail
					true;
				}
			"""
			return assert_eval(code, true)

		"test_large_array_allocation_limited":
			# Very large single allocation should fail
			# Note: new Array(N) creates a sparse array that doesn't allocate memory
			# We need to actually fill the array to trigger memory allocation
			var code = """
				try {
					// Try to allocate array larger than memory limit
					// Using fill() to force actual memory allocation
					var huge = new Array(50000000).fill(0);
					false;
				} catch (e) {
					true;
				}
			"""
			return assert_eval(code, true)

		"test_normal_allocation_works":
			# Normal memory allocation should work
			var code = """
				var arr = new Array(1000);
				for (var i = 0; i < arr.length; i++) {
					arr[i] = i * 2;
				}
				arr.length === 1000 && arr[500] === 1000;
			"""
			return assert_eval(code, true)

		"test_rapid_api_calls":
			# Rapid API calls should work within limits
			# (Tests that rate limiting doesn't break normal use)
			var code = """
				var node = new Node();
				for (var i = 0; i < 100; i++) {
					node.name = 'Test' + i;
				}
				node.name === 'Test99';
			"""
			return assert_eval(code, true)

		"test_timeout_configurable":
			# Timeout should be configurable via set_timeout_ms
			# Test that the method exists and can be called
			sandbox.set_timeout_ms(2000)
			# If we get here without error, configuration works
			return { "passed": true, "message": "" }

		"test_memory_limit_configurable":
			# Memory limit should be configurable via set_memory_limit_mb
			# Test that the method exists and can be called
			sandbox.set_memory_limit_mb(32)
			# Reset to default
			sandbox.set_memory_limit_mb(64)
			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	# Reset limits to defaults
	sandbox.set_timeout_ms(5000)
	sandbox.set_memory_limit_mb(64)
	super.teardown()

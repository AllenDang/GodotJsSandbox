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
		# API call limits - sandbox specific
		"test_rapid_api_calls",
		"test_heavy_ops_limit_per_frame",
		"test_heavy_ops_reset_between_frames",
		"test_write_ops_limit_per_frame",
		# Configuration
		"test_timeout_configurable",
		"test_memory_limit_configurable",
		"test_heavy_ops_limit_configurable",
		# Exact limit boundary tests
		"test_heavy_ops_exact_limit_boundary",
		"test_write_ops_exact_limit_boundary",
		"test_error_message_shows_configured_limit",
		# Runtime scene instantiation limits
		"test_runtime_instantiated_scene_uses_sandbox_limits",
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

		"test_heavy_ops_limit_per_frame":
			# Heavy operations (like new Node()) should be limited per frame
			# Default limit is 50 per frame
			sandbox.set_heavy_ops_per_frame(10)  # Set low limit for testing
			sandbox.reset_frame_counters()

			var code = """
				var success = true;
				var error_msg = '';
				try {
					// Create more nodes than the limit allows
					for (var i = 0; i < 20; i++) {
						var node = new Node();
					}
				} catch (e) {
					error_msg = e.toString();
					success = false;
				}
				({ success: success, error: error_msg });
			"""
			var result = sandbox.eval(code)
			sandbox.set_heavy_ops_per_frame(50)  # Reset to default

			# Should have failed due to heavy ops limit
			if result is Dictionary and result.has("success"):
				if result["success"] == false:
					return { "passed": true, "message": "Heavy ops limit enforced" }
				else:
					return { "passed": false, "message": "Heavy ops limit not enforced - created 20 nodes without error" }
			return { "passed": false, "message": "Unexpected result: " + str(result) }

		"test_heavy_ops_reset_between_frames":
			# Heavy ops counter should reset between frames
			sandbox.set_heavy_ops_per_frame(10)
			sandbox.reset_frame_counters()

			# First batch should work
			var code1 = """
				var nodes = [];
				for (var i = 0; i < 5; i++) {
					nodes.push(new Node());
				}
				nodes.length;
			"""
			var result1 = sandbox.eval(code1)
			if result1 != 5:
				sandbox.set_heavy_ops_per_frame(50)
				return { "passed": false, "message": "First batch failed: " + str(result1) }

			# Simulate new frame by resetting counters
			sandbox.reset_frame_counters()

			# Second batch should also work after reset
			var code2 = """
				var nodes = [];
				for (var i = 0; i < 5; i++) {
					nodes.push(new Node());
				}
				nodes.length;
			"""
			var result2 = sandbox.eval(code2)
			sandbox.set_heavy_ops_per_frame(50)

			if result2 == 5:
				return { "passed": true, "message": "Frame counter reset works" }
			return { "passed": false, "message": "Second batch failed after reset: " + str(result2) }

		"test_write_ops_limit_per_frame":
			# Write operations should be limited per frame
			sandbox.set_write_ops_per_frame(100)
			sandbox.reset_frame_counters()

			var code = """
				var success = true;
				var count = 0;
				try {
					var node = new Node();
					// Try to exceed write limit with rapid property sets
					for (var i = 0; i < 200; i++) {
						node.name = 'Test' + i;
						count++;
					}
				} catch (e) {
					success = false;
				}
				({ success: success, count: count });
			"""
			var result = sandbox.eval(code)
			sandbox.set_write_ops_per_frame(500)  # Reset to default

			# Should have been limited
			if result is Dictionary and result.has("count"):
				if result["count"] < 200:
					return { "passed": true, "message": "Write ops limited at " + str(result["count"]) }
			return { "passed": false, "message": "Write ops limit may not be enforced: " + str(result) }

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

		"test_heavy_ops_limit_configurable":
			# Heavy ops limit should be configurable
			sandbox.set_heavy_ops_per_frame(100)
			sandbox.reset_frame_counters()

			# Should be able to create more nodes with higher limit
			var code = """
				var nodes = [];
				for (var i = 0; i < 60; i++) {
					nodes.push(new Node());
				}
				nodes.length;
			"""
			var result = sandbox.eval(code)
			sandbox.set_heavy_ops_per_frame(50)  # Reset to default

			if result == 60:
				return { "passed": true, "message": "Heavy ops limit is configurable" }
			return { "passed": false, "message": "Failed with limit=100: " + str(result) }

		"test_heavy_ops_exact_limit_boundary":
			# Verify that set_heavy_ops_per_frame(200) allows exactly 200 operations
			sandbox.set_heavy_ops_per_frame(200)
			sandbox.reset_frame_counters()

			# Create exactly 200 nodes - should succeed
			var code_at_limit = """
				var nodes = [];
				var error = null;
				try {
					for (var i = 0; i < 200; i++) {
						nodes.push(new Node());
					}
				} catch (e) {
					error = e.toString();
				}
				({ count: nodes.length, error: error });
			"""
			var result_at_limit = sandbox.eval(code_at_limit)
			sandbox.set_heavy_ops_per_frame(50)  # Reset

			if not (result_at_limit is Dictionary):
				return { "passed": false, "message": "Unexpected result type: " + str(result_at_limit) }

			if result_at_limit["error"] != null:
				return { "passed": false, "message": "Failed before reaching limit of 200: " + str(result_at_limit) }

			if result_at_limit["count"] != 200:
				return { "passed": false, "message": "Expected 200 nodes, got " + str(result_at_limit["count"]) }

			# Now test that 201 fails
			sandbox.set_heavy_ops_per_frame(200)
			sandbox.reset_frame_counters()

			var code_over_limit = """
				var nodes = [];
				var error = null;
				try {
					for (var i = 0; i < 201; i++) {
						nodes.push(new Node());
					}
				} catch (e) {
					error = e.toString();
				}
				({ count: nodes.length, error: error });
			"""
			var result_over_limit = sandbox.eval(code_over_limit)
			sandbox.set_heavy_ops_per_frame(50)  # Reset

			if not (result_over_limit is Dictionary):
				return { "passed": false, "message": "Unexpected result type for over-limit test" }

			if result_over_limit["error"] == null:
				return { "passed": false, "message": "Should have failed at 201, but created " + str(result_over_limit["count"]) + " nodes" }

			if result_over_limit["count"] != 200:
				return { "passed": false, "message": "Expected to stop at exactly 200, got " + str(result_over_limit["count"]) }

			return { "passed": true, "message": "Heavy ops limit of 200 is exactly enforced" }

		"test_write_ops_exact_limit_boundary":
			# Verify that set_write_ops_per_frame(400) allows exactly 400 operations
			# Note: Heavy operations (like new Node()) also count as write ops,
			# so we set limit to 401 to account for the node creation
			sandbox.set_write_ops_per_frame(401)
			sandbox.reset_frame_counters()

			# Do exactly 400 write operations after creating 1 node (which is 1 heavy + 1 write)
			# Total: 1 (node creation) + 400 (property sets) = 401 write ops
			var code_at_limit = """
				var node = new Node();  // 1 heavy op + 1 write op
				var error = null;
				var count = 0;
				try {
					for (var i = 0; i < 400; i++) {
						node.name = 'Test' + i;
						count++;
					}
				} catch (e) {
					error = e.toString();
				}
				({ count: count, error: error });
			"""
			var result_at_limit = sandbox.eval(code_at_limit)
			sandbox.set_write_ops_per_frame(500)  # Reset

			if not (result_at_limit is Dictionary):
				return { "passed": false, "message": "Unexpected result type: " + str(result_at_limit) }

			if result_at_limit["error"] != null:
				return { "passed": false, "message": "Failed before reaching limit of 400: " + str(result_at_limit) }

			if result_at_limit["count"] != 400:
				return { "passed": false, "message": "Expected 400 writes, got " + str(result_at_limit["count"]) }

			# Now test that 401 writes fail (402 total with node creation)
			sandbox.set_write_ops_per_frame(401)
			sandbox.reset_frame_counters()

			var code_over_limit = """
				var node = new Node();  // 1 heavy op + 1 write op
				var error = null;
				var count = 0;
				try {
					for (var i = 0; i < 401; i++) {
						node.name = 'Test' + i;
						count++;
					}
				} catch (e) {
					error = e.toString();
				}
				({ count: count, error: error });
			"""
			var result_over_limit = sandbox.eval(code_over_limit)
			sandbox.set_write_ops_per_frame(500)  # Reset

			if not (result_over_limit is Dictionary):
				return { "passed": false, "message": "Unexpected result type for over-limit test" }

			if result_over_limit["error"] == null:
				return { "passed": false, "message": "Should have failed at 401, but completed " + str(result_over_limit["count"]) + " writes" }

			if result_over_limit["count"] != 400:
				return { "passed": false, "message": "Expected to stop at exactly 400, got " + str(result_over_limit["count"]) }

			return { "passed": true, "message": "Write ops limit is correctly enforced (heavy ops also count as write ops)" }

		"test_error_message_shows_configured_limit":
			# Verify the error message shows the actual configured limit, not hardcoded values
			sandbox.set_heavy_ops_per_frame(200)
			sandbox.reset_frame_counters()

			var code = """
				var error_msg = '';
				try {
					for (var i = 0; i < 250; i++) {
						new Node();
					}
				} catch (e) {
					error_msg = e.toString();
				}
				error_msg;
			"""
			var error_msg = sandbox.eval(code)
			sandbox.set_heavy_ops_per_frame(50)  # Reset

			if not (error_msg is String) or error_msg == "":
				return { "passed": false, "message": "No error message captured" }

			# The error message should contain "200" (the configured limit), not "50" (the default)
			if "200" in error_msg:
				return { "passed": true, "message": "Error message correctly shows configured limit: " + error_msg }

			if "50" in error_msg:
				return { "passed": false, "message": "Error message shows hardcoded default (50) instead of configured limit (200): " + error_msg }

			return { "passed": false, "message": "Error message doesn't contain expected limit value: " + error_msg }

		"test_runtime_instantiated_scene_uses_sandbox_limits":
			# Verify that JS scripts INSIDE scenes loaded via sandbox.load_scene()
			# use the sandbox's configured limits, not the global default limits.
			#
			# Test approach:
			# 1. Set sandbox heavy ops limit to 100 (clearly different from default 50)
			# 2. Load a scene via sandbox.load_scene() - this properly attaches scripts
			# 3. The scene's script sets up a global function in _ready()
			# 4. Call that function to create 60 nodes
			# 5. If fix works: script uses sandbox limit (100), so 60 nodes succeed
			# 6. If fix broken: script uses default limit (50), so it fails around 50
			sandbox.set_heavy_ops_per_frame(100)  # Set higher than default 50
			sandbox.reset_frame_counters()

			# Load scene via sandbox.load_scene() - this ensures scripts are attached to sandbox
			var instance = sandbox.load_scene("res://fixtures/test_runtime_limits.tscn")
			if not instance:
				sandbox.set_heavy_ops_per_frame(50)  # Reset
				return { "passed": false, "message": "Failed to load test scene via sandbox.load_scene()" }

			# Add to tree so _ready() runs
			test_root.add_child(instance)

			# Give a frame for _ready to run
			await scene_tree.process_frame

			# Now test that the global function set up by the scene's script
			# uses the sandbox's configured limits
			var code = """
				if (!globalThis.__runtime_test_ready) {
					({ success: false, stage: 'ready', error: 'Scene script _ready did not run' });
				} else if (!globalThis.__runtime_test_create_nodes) {
					({ success: false, stage: 'setup', error: 'Global function not set up' });
				} else {
					// Call the function defined by the scene's script
					// This is the KEY TEST: does this function use sandbox limits?
					// Try to create 60 nodes - should succeed if limit is 100,
					// but fail if limit is default 50
					var result = globalThis.__runtime_test_create_nodes(60);
					result.success = true;
					result.stage = 'test';
					result;
				}
			"""
			var result = sandbox.eval(code)

			# Cleanup
			instance.queue_free()
			sandbox.set_heavy_ops_per_frame(50)  # Reset

			if result == null:
				var error = sandbox.get_last_error()
				return { "passed": false, "message": "Eval returned null. Last error: " + error }

			if not (result is Dictionary):
				return { "passed": false, "message": "Unexpected result type: " + str(result) }

			if result.get("success") == false:
				return { "passed": false, "message": "Failed at stage '" + str(result.get("stage", "?")) + "': " + str(result.get("error", "unknown")) }

			var created = result.get("created", 0)
			var error_msg = str(result.get("error", ""))

			# If we created 60 nodes without error, the sandbox limit (100) was used
			if created == 60 and error_msg == "null":
				return { "passed": true, "message": "Scene script uses sandbox limit - created all 60 nodes (limit 100)" }

			# If we got an error mentioning 50, the DEFAULT limit was used (BUG!)
			if "50" in error_msg:
				return { "passed": false, "message": "BUG: Scene script used DEFAULT limit (50) instead of sandbox limit (100). Created: " + str(created) + ", Error: " + error_msg }

			# If we got an error mentioning 100, sandbox limit was used but we hit it
			# (This shouldn't happen since we're only creating 60 with limit 100)
			if "100" in error_msg:
				return { "passed": false, "message": "Unexpected: Hit sandbox limit of 100 when creating only 60 nodes. Created: " + str(created) }

			# If created < 60 with some other error, something else is wrong
			if created < 60:
				return { "passed": false, "message": "Only created " + str(created) + " nodes (expected 60). Error: " + error_msg }

			return { "passed": true, "message": "Scene script correctly uses sandbox limits (created " + str(created) + " nodes)" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	# Reset limits to defaults
	sandbox.set_timeout_ms(5000)
	sandbox.set_memory_limit_mb(64)
	sandbox.set_heavy_ops_per_frame(50)
	sandbox.set_write_ops_per_frame(500)
	sandbox.reset_frame_counters()
	super.teardown()

class_name TestAsyncAwait
extends TestBase

## Tests for async/await support with signals (Promise-Signal bridge)

func get_suite_name() -> String:
	return "Async/Await"

func get_tests() -> Array[String]:
	return [
		"test_await_signal_exists",
		"test_await_signal_returns_promise",
		"test_await_timer_timeout",
		"test_await_custom_signal",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_await_signal_exists":
			# Test that await_signal method exists on objects
			var node = Node.new()
			node.name = "AwaitTestNode"
			test_root.add_child(node)

			sandbox.set_global("test_node", node)
			var result = sandbox.eval("""
				var node = test_node;
				typeof node.await_signal === 'function'
			""")

			return assert_eq(result, true, "await_signal should be a function on objects")

		"test_await_signal_returns_promise":
			# Test that await_signal returns a Promise
			var timer = Timer.new()
			timer.name = "PromiseTestTimer"
			timer.one_shot = true
			timer.wait_time = 1.0
			test_root.add_child(timer)

			sandbox.set_global("test_timer", timer)
			var result = sandbox.eval("""
				var timer = test_timer;
				var promise = timer.await_signal('timeout');
				promise instanceof Promise
			""")

			return assert_eq(result, true, "await_signal should return a Promise")

		"test_await_timer_timeout":
			# Test awaiting a timer timeout signal
			var timer = Timer.new()
			timer.name = "AsyncTimerTestNode"
			timer.one_shot = true
			timer.wait_time = 0.1
			test_root.add_child(timer)

			sandbox.set_global("test_timer", timer)

			# Set up a flag that will be set when the promise resolves
			sandbox.eval("""
				globalThis.timerResolved = false;
				var timer = test_timer;
				timer.await_signal('timeout').then(function() {
					globalThis.timerResolved = true;
				});
			""")

			# Start the timer
			timer.start()

			# Wait for the timer to complete
			await scene_tree.create_timer(0.2).timeout

			# Execute pending JS microtasks
			sandbox.execute_pending_jobs()

			# Check if the promise resolved
			var result = sandbox.eval("globalThis.timerResolved")

			return assert_eq(result, true, "Promise should resolve when timer fires")

		"test_await_custom_signal":
			# Test awaiting a custom signal
			var node = Node.new()
			node.name = "CustomSignalNode"
			node.add_user_signal("my_signal")
			test_root.add_child(node)

			sandbox.set_global("test_node", node)

			# Set up the await
			sandbox.eval("""
				globalThis.signalReceived = false;
				globalThis.receivedValue = null;
				var node = test_node;
				node.await_signal('my_signal').then(function(value) {
					globalThis.signalReceived = true;
					globalThis.receivedValue = value;
				});
			""")

			# Emit the signal with a value
			node.emit_signal("my_signal", 42)

			# Wait a frame for signal propagation
			await scene_tree.process_frame

			# Execute pending JS microtasks
			sandbox.execute_pending_jobs()

			var received = sandbox.eval("globalThis.signalReceived")
			var value = sandbox.eval("globalThis.receivedValue")

			if received != true:
				return { "passed": false, "message": "Signal was not received" }
			if value != 42:
				return { "passed": false, "message": "Received value %s but expected 42" % value }

			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

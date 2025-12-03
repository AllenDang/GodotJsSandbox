class_name TestSignals
extends TestBase

var _callback_fired := false
var _callback_value: Variant = null

func get_suite_name() -> String:
	return "Signals"

func get_tests() -> Array[String]:
	return [
		"test_connect_signal",
		"test_timer_timeout_signal",
		"test_callback_with_global",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_connect_signal":
			# Test that connect function exists and can be called
			var code = """
				var timer = new Timer();
				typeof timer.connect === 'function';
			"""
			return assert_eval(code, true)
		"test_timer_timeout_signal":
			# Create a timer in GDScript and pass to JS
			var timer = Timer.new()
			timer.wait_time = 0.1
			timer.one_shot = true
			test_root.add_child(timer)
			await scene_tree.process_frame  # Wait for add_child to complete

			# Set up JS callback tracking
			sandbox.eval("""
				globalThis.__signal_fired = false;
				globalThis.__signal_callback = function() {
					globalThis.__signal_fired = true;
				};
			""")

			# Connect and start timer - pass timer to JS
			sandbox.set_global("__test_timer", timer)
			var connect_result = sandbox.eval("""
				__test_timer.connect('timeout', __signal_callback);
				true;
			""")

			if connect_result != true:
				timer.queue_free()
				return { "passed": false, "message": "Failed to connect signal" }

			timer.start()

			# Wait for timer
			await scene_tree.create_timer(0.2).timeout

			# Check if callback fired
			var fired = sandbox.eval("globalThis.__signal_fired")

			# Cleanup
			sandbox.eval("delete globalThis.__signal_fired; delete globalThis.__signal_callback; delete globalThis.__test_timer;")
			timer.queue_free()

			return assert_eq(fired, true, "Signal callback should have fired")
		"test_callback_with_global":
			# Test that we can store callback result in a global
			var code = """
				globalThis.__test_counter = 0;
				var node = new Node();
				globalThis.__test_counter = 1;
				globalThis.__test_counter;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_counter;")
			return assert_eq(result, 1)
		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	_callback_fired = false
	_callback_value = null
	# Clean up any global test variables
	sandbox.eval("""
		delete globalThis.__signal_fired;
		delete globalThis.__signal_callback;
		delete globalThis.__test_timer;
		delete globalThis.__test_counter;
	""")
	super.teardown()

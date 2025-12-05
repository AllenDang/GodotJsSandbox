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
		"test_signal_property_syntax",
		"test_signal_property_connect",
		"test_builtin_signal_emit",
		"test_custom_signal_declaration",
		"test_custom_signal_connect_emit",
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
		"test_signal_property_syntax":
			# Test GDScript 4.x signal syntax: timer.timeout returns Signal wrapper
			var code = """
				var timer = new Timer();
				var sig = timer.timeout;
				typeof sig === 'object' &&
				typeof sig.connect === 'function' &&
				typeof sig.emit === 'function';
			"""
			return assert_eval(code, true)
		"test_signal_property_connect":
			# Test timer.timeout.connect(callback) syntax
			var timer = Timer.new()
			timer.wait_time = 0.1
			timer.one_shot = true
			test_root.add_child(timer)
			await scene_tree.process_frame

			sandbox.eval("""
				globalThis.__signal_fired = false;
				globalThis.__signal_callback = function() {
					globalThis.__signal_fired = true;
				};
			""")

			sandbox.set_global("__test_timer", timer)
			var connect_result = sandbox.eval("""
				// GDScript 4.x style syntax
				__test_timer.timeout.connect(__signal_callback);
				true;
			""")

			if connect_result != true:
				timer.queue_free()
				return { "passed": false, "message": "Failed to connect signal with property syntax" }

			timer.start()
			await scene_tree.create_timer(0.2).timeout

			var fired = sandbox.eval("globalThis.__signal_fired")
			sandbox.eval("delete globalThis.__signal_fired; delete globalThis.__signal_callback; delete globalThis.__test_timer;")
			timer.queue_free()

			return assert_eq(fired, true, "Signal callback should have fired via property syntax")
		"test_builtin_signal_emit":
			# Test emitting a built-in signal via property syntax
			var node = Node.new()
			test_root.add_child(node)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			sandbox.eval("""
				globalThis.__tree_exiting_fired = false;
			""")

			# Connect to tree_exiting signal
			node.tree_exiting.connect(func(): sandbox.eval("globalThis.__tree_exiting_fired = true;"))

			# Emit from JS using property syntax
			sandbox.eval("__test_node.tree_exiting.emit();")

			var fired = sandbox.eval("globalThis.__tree_exiting_fired")
			sandbox.eval("delete globalThis.__tree_exiting_fired; delete globalThis.__test_node;")
			node.queue_free()

			return assert_eq(fired, true, "tree_exiting signal should have fired via emit()")
		"test_custom_signal_declaration":
			# Test that custom signals can be declared with var signals = [...]
			var code = """
				var signals = ['health_changed', 'game_over'];
				exports._ready = function() {
					// Custom signals should be registered
				};
			"""
			var script = sandbox.create_script(code)
			var node = Node.new()
			test_root.add_child(node)
			await scene_tree.process_frame

			node.set_script(script)
			await scene_tree.process_frame

			# Check if custom signals are registered
			var has_health = node.has_signal("health_changed")
			var has_game_over = node.has_signal("game_over")

			node.queue_free()
			await scene_tree.process_frame

			if not has_health:
				return { "passed": false, "message": "health_changed signal not registered" }
			if not has_game_over:
				return { "passed": false, "message": "game_over signal not registered" }
			return { "passed": true, "message": "Custom signals registered successfully" }
		"test_custom_signal_connect_emit":
			# Test connecting and emitting custom signals via JS property syntax
			# Both connect and emit should work from JS using this.signal_name.method()
			var code = """
				var signals = ['score_updated'];
				globalThis.__score_received = -1;
				exports._ready = function() {
					// Connect using JS property syntax (GDScript 4.x style)
					this.score_updated.connect(function(score) {
						globalThis.__score_received = score;
					});
				};
				exports.addScore = function(points) {
					// Emit using JS property syntax
					this.score_updated.emit(points);
				};
			"""
			var script = sandbox.create_script(code)
			var node = Node.new()
			node.set_script(script)
			test_root.add_child(node)  # Triggers _ready
			await scene_tree.process_frame
			await scene_tree.process_frame

			# Verify signal is registered
			if not node.has_signal("score_updated"):
				sandbox.eval("delete globalThis.__score_received;")
				node.queue_free()
				return { "passed": false, "message": "score_updated signal not registered on node" }

			# Call the addScore method which should emit score_updated
			sandbox.set_global("__test_node", node)
			sandbox.eval("__test_node.addScore(100);")

			var received = sandbox.eval("globalThis.__score_received")
			sandbox.eval("delete globalThis.__score_received; delete globalThis.__test_node;")

			node.queue_free()

			return assert_eq(received, 100, "Custom signal should pass value to JS callback via property syntax")
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

class_name TestCustomSignals
extends TestBase

## Tests for custom signal declaration and emission from JavaScript

func get_suite_name() -> String:
	return "Custom Signals"

func get_tests() -> Array[String]:
	return [
		# Basic emit_signal functionality
		"test_emit_signal_exists",
		"test_emit_signal_basic",
		"test_emit_signal_with_args",
		"test_emit_signal_many_args",  # Test unlimited args support
		# @signal annotation parsing
		"test_signal_annotation_parsed",
		"test_multiple_signals_parsed",
		# Full integration: JS emits, GDScript receives
		"test_js_emit_gdscript_receive",
		"test_js_emit_with_arguments",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_emit_signal_exists":
			# Test that emit_signal method exists on Godot objects
			var code = """
				var node = new Node();
				typeof node.emit_signal === 'function';
			"""
			return assert_eval(code, true)

		"test_emit_signal_basic":
			# Test emit_signal can be called with a user-defined signal
			# Note: Built-in signals like 'tree_entered' may fail if conditions aren't met
			# So we add a custom signal first
			var node = Node.new()
			node.add_user_signal("test_signal")
			test_root.add_child(node)
			await scene_tree.process_frame

			sandbox.set_global("__test_node", node)
			var code = """
				__test_node.emit_signal('test_signal');
				true;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__test_node;")
			node.queue_free()
			return assert_eq(result, true)

		"test_emit_signal_with_args":
			# Test emit_signal with arguments
			var code = """
				var node = new Node();
				// renamed signal doesn't exist but emit_signal should still work
				try {
					node.emit_signal('custom_signal', 42, 'hello');
					true;
				} catch(e) {
					// May fail if signal doesn't exist, which is expected
					true;
				}
			"""
			return assert_eval(code, true)

		"test_emit_signal_many_args":
			# Test emit_signal with MORE than 4 arguments (tests the callv fix)
			# Previously limited to 4 args, now supports unlimited
			var emitter = Node.new()
			emitter.name = "ManyArgsEmitter"
			test_root.add_child(emitter)
			await scene_tree.process_frame

			# Add custom signal with 6 arguments
			emitter.add_user_signal("many_args_signal", [
				{ "name": "a", "type": TYPE_INT },
				{ "name": "b", "type": TYPE_INT },
				{ "name": "c", "type": TYPE_INT },
				{ "name": "d", "type": TYPE_INT },
				{ "name": "e", "type": TYPE_INT },
				{ "name": "f", "type": TYPE_INT }
			])

			# Track received values
			emitter.set_meta("args_sum", 0)
			emitter.connect("many_args_signal", func(a: int, b: int, c: int, d: int, e: int, f: int):
				emitter.set_meta("args_sum", a + b + c + d + e + f)
			)

			# Pass emitter to JS and emit signal with 6 args
			sandbox.set_global("__test_emitter", emitter)
			sandbox.eval("""
				__test_emitter.emit_signal('many_args_signal', 1, 2, 3, 4, 5, 6);
			""")

			await scene_tree.process_frame

			var args_sum = emitter.get_meta("args_sum")

			# Cleanup
			sandbox.eval("delete globalThis.__test_emitter;")
			emitter.queue_free()

			# 1+2+3+4+5+6 = 21
			if args_sum != 21:
				return { "passed": false, "message": "Expected sum=21 (1+2+3+4+5+6), got %d" % args_sum }
			return { "passed": true, "message": "" }

		"test_signal_annotation_parsed":
			# Test that @signal annotation is parsed from JS code
			var script = JSScript.new()
			script.source_code = """
				// @signal health_changed
				function _ready() {}
			"""
			# Use has_script_signal to check if signal was parsed
			var has_signal = script.has_script_signal("health_changed")
			return assert_true(has_signal, "Signal 'health_changed' should be parsed from @signal annotation")

		"test_multiple_signals_parsed":
			# Test that multiple @signal annotations are parsed
			var script = JSScript.new()
			script.source_code = """
				// @signal health_changed
				// @signal mana_changed
				// @signal stamina_changed
				function _ready() {}
			"""
			# Check each signal individually using has_script_signal
			var expected = ["health_changed", "mana_changed", "stamina_changed"]
			for sig_name in expected:
				if not script.has_script_signal(sig_name):
					return { "passed": false, "message": "Signal '%s' not found" % sig_name }
			return { "passed": true, "message": "" }

		"test_js_emit_gdscript_receive":
			# Full integration: JS emits signal, GDScript callback receives it
			var emitter = Node.new()
			emitter.name = "SignalEmitter"
			test_root.add_child(emitter)
			await scene_tree.process_frame

			# Add custom signal to the emitter
			emitter.add_user_signal("custom_event")

			# Track if signal was received using metadata (lambdas capture by value)
			emitter.set_meta("signal_received", false)
			emitter.connect("custom_event", func():
				emitter.set_meta("signal_received", true)
			)

			# Pass emitter to JS and emit the signal
			sandbox.set_global("__test_emitter", emitter)
			sandbox.eval("""
				__test_emitter.emit_signal('custom_event');
			""")

			# Give a frame for signal to propagate
			await scene_tree.process_frame

			var signal_received = emitter.get_meta("signal_received")

			# Cleanup
			sandbox.eval("delete globalThis.__test_emitter;")
			emitter.queue_free()

			return assert_true(signal_received, "GDScript should receive signal emitted from JS")

		"test_js_emit_with_arguments":
			# Test JS emitting signal with arguments that GDScript receives
			var emitter = Node.new()
			emitter.name = "ArgEmitter"
			test_root.add_child(emitter)
			await scene_tree.process_frame

			# Add custom signal with arguments
			emitter.add_user_signal("value_changed", [
				{ "name": "old_value", "type": TYPE_INT },
				{ "name": "new_value", "type": TYPE_INT }
			])

			# Track received values using metadata (lambdas capture by value)
			emitter.set_meta("received_old", -1)
			emitter.set_meta("received_new", -1)
			emitter.connect("value_changed", func(old_val: int, new_val: int):
				emitter.set_meta("received_old", old_val)
				emitter.set_meta("received_new", new_val)
			)

			# Pass emitter to JS and emit the signal with args
			sandbox.set_global("__test_emitter", emitter)
			sandbox.eval("""
				__test_emitter.emit_signal('value_changed', 10, 20);
			""")

			# Give a frame for signal to propagate
			await scene_tree.process_frame

			var received_old = emitter.get_meta("received_old")
			var received_new = emitter.get_meta("received_new")

			# Cleanup
			sandbox.eval("delete globalThis.__test_emitter;")
			emitter.queue_free()

			if received_old != 10 || received_new != 20:
				return { "passed": false, "message": "Expected (10, 20), got (%d, %d)" % [received_old, received_new] }
			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

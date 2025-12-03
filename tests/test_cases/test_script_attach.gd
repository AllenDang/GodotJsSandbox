class_name TestScriptAttach
extends TestBase

## Tests for JSScript attachment to nodes (PRD: JS scripts can attach to nodes like GDScript)
## Reference: PRD.md Section "原生脚本集成", TDD.md Section 4 "ScriptLanguage 集成"

func get_suite_name() -> String:
	return "Script Attachment"

func get_tests() -> Array[String]:
	return [
		# Core attachment functionality
		"test_create_jsscript",
		"test_set_script_on_node",
		"test_set_script_on_node3d",
		"test_script_has_source_code",
		# Security: only JSScript allowed
		"test_block_gdscript_string_eval",
		# Script instance behavior
		"test_script_instance_created",
		"test_multiple_nodes_same_script",
		"test_replace_script",
		"test_remove_script",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_create_jsscript":
			# JSScript.new() should create a valid script object
			var script = JSScript.new()
			if script == null:
				return { "passed": false, "message": "JSScript.new() returned null" }
			return { "passed": true, "message": "" }

		"test_set_script_on_node":
			# Should be able to attach JSScript to a Node
			var node = Node.new()
			node.name = "ScriptTestNode"
			var script = JSScript.new()
			script.source_code = """
				function _ready() {
					// Basic script
				}
			"""

			# This should work - set_script with JSScript is allowed
			node.set_script(script)

			var attached = node.get_script()
			node.queue_free()

			if attached == null:
				return { "passed": false, "message": "Script was not attached" }
			if not attached is JSScript:
				return { "passed": false, "message": "Attached script is not JSScript" }
			return { "passed": true, "message": "" }

		"test_set_script_on_node3d":
			# Should be able to attach JSScript to Node3D
			var node = Node3D.new()
			node.name = "Script3DTestNode"
			var script = JSScript.new()
			script.source_code = """
				function _ready() {
					this.position = {x: 1, y: 2, z: 3};
				}
			"""

			node.set_script(script)
			var attached = node.get_script()
			node.queue_free()

			if attached == null:
				return { "passed": false, "message": "Script was not attached to Node3D" }
			return { "passed": true, "message": "" }

		"test_script_has_source_code":
			# JSScript should store source code
			var script = JSScript.new()
			var source = """
				var counter = 0;
				function _process(delta) {
					counter++;
				}
			"""
			script.source_code = source

			if script.source_code != source:
				return { "passed": false, "message": "Source code not stored correctly" }
			return { "passed": true, "message": "" }

		"test_block_gdscript_string_eval":
			# Attempting to create/use GDScript via sandbox should be blocked
			# GDScript class should not be accessible
			var code = """
				try {
					var gd = new GDScript();
					false;  // Should not reach here
				} catch (e) {
					true;  // Expected - GDScript blocked
				}
			"""
			return assert_eval(code, true)

		"test_script_instance_created":
			# When script is attached and node enters tree, instance should be created
			var node = Node3D.new()
			node.name = "InstanceTestNode"
			# Use sandbox.create_script() to ensure script runs in sandbox's context
			var script = sandbox.create_script("""
				globalThis.__instance_test_ready = false;
				function _ready() {
					globalThis.__instance_test_ready = true;
				}
			""")
			node.set_script(script)
			test_root.add_child(node)

			await scene_tree.process_frame
			await scene_tree.process_frame

			var ready_called = sandbox.eval("globalThis.__instance_test_ready")
			sandbox.eval("delete globalThis.__instance_test_ready;")

			return assert_eq(ready_called, true, "Script instance _ready should be called")

		"test_multiple_nodes_same_script":
			# Multiple nodes can share the same script
			var script = JSScript.new()
			script.source_code = """
				function _ready() {
					// Shared script
				}
			"""

			var node1 = Node.new()
			var node2 = Node.new()
			node1.set_script(script)
			node2.set_script(script)

			var script1 = node1.get_script()
			var script2 = node2.get_script()

			node1.queue_free()
			node2.queue_free()

			if script1 == null or script2 == null:
				return { "passed": false, "message": "Script not attached to both nodes" }
			return { "passed": true, "message": "" }

		"test_replace_script":
			# Should be able to replace one JSScript with another
			var node = Node.new()
			var script1 = JSScript.new()
			script1.source_code = "function _ready() { /* script 1 */ }"
			var script2 = JSScript.new()
			script2.source_code = "function _ready() { /* script 2 */ }"

			node.set_script(script1)
			var attached1 = node.get_script()

			node.set_script(script2)
			var attached2 = node.get_script()

			node.queue_free()

			if attached1 == null or attached2 == null:
				return { "passed": false, "message": "Script replacement failed" }
			if attached1.source_code == attached2.source_code:
				return { "passed": false, "message": "Script was not replaced" }
			return { "passed": true, "message": "" }

		"test_remove_script":
			# Should be able to remove script by setting null
			var node = Node.new()
			var script = JSScript.new()
			script.source_code = "function _ready() {}"

			node.set_script(script)
			var has_script = node.get_script() != null

			node.set_script(null)
			var script_removed = node.get_script() == null

			node.queue_free()

			if not has_script:
				return { "passed": false, "message": "Script was not initially attached" }
			if not script_removed:
				return { "passed": false, "message": "Script was not removed" }
			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.eval("delete globalThis.__instance_test_ready;")
	super.teardown()

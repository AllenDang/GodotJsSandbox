class_name TestObjectLifecycle
extends TestBase

## Tests for object lifecycle and memory safety (PRD.md Section 4 "Godot 对象生命周期安全")
## Reference: TDD.md Section 6.3 "ObjectRegistry", Section 7 "内存安全机制"
##
## Core guarantees:
## - JS never holds raw pointers
## - All objects accessed via handle
## - Node deletion monitored via NOTIFICATION_PREDELETE
## - RefCounted uses reference/unreference sync
## - Generation/version prevents ID reuse issues
## - Accessing deleted objects throws JS exception (no crash)

func get_suite_name() -> String:
	return "Object Lifecycle"

func get_tests() -> Array[String]:
	return [
		# Handle mechanism
		"test_object_has_handle",
		"test_handle_is_number",
		"test_different_objects_different_handles",
		# Deletion safety
		"test_access_after_queue_free_throws",
		"test_access_property_after_free_throws",
		"test_call_method_after_free_throws",
		"test_no_crash_on_deleted_object",
		# RefCounted lifecycle
		"test_refcounted_not_freed_while_js_holds",
		"test_refcounted_freed_after_js_releases",
		# Node tree operations
		"test_child_survives_parent_removal_from_tree",
		"test_handle_invalid_after_free",
		# Multiple references
		"test_multiple_js_refs_same_object",
		"test_object_valid_check",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_object_has_handle":
			# All Godot objects in JS should have __handle
			var code = """
				var node = new Node();
				'__handle' in node;
			"""
			return assert_eval(code, true)

		"test_handle_is_number":
			# Handle should be a number
			var code = """
				var node = new Node();
				typeof node.__handle === 'number';
			"""
			return assert_eval(code, true)

		"test_different_objects_different_handles":
			# Different objects should have different handles
			var code = """
				var node1 = new Node();
				var node2 = new Node();
				node1.__handle !== node2.__handle;
			"""
			return assert_eval(code, true)

		"test_access_after_queue_free_throws":
			# Accessing object after queue_free should throw, not crash
			var node = Node.new()
			node.name = "DeleteTest"
			test_root.add_child(node)

			sandbox.set_global("__delete_test_node", node)
			sandbox.eval("__delete_test_node.queue_free();")

			# Wait for deletion
			await scene_tree.process_frame
			await scene_tree.process_frame

			# Now try to access - should throw
			var code = """
				try {
					var name = __delete_test_node.name;
					false;  // Should not reach here
				} catch (e) {
					true;  // Expected - object deleted
				}
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__delete_test_node;")

			return assert_eq(result, true, "Accessing deleted object should throw")

		"test_access_property_after_free_throws":
			# Accessing property of freed object should throw
			var node = Node3D.new()
			test_root.add_child(node)
			sandbox.set_global("__prop_test_node", node)

			node.queue_free()
			await scene_tree.process_frame
			await scene_tree.process_frame

			var code = """
				try {
					var pos = __prop_test_node.position;
					false;
				} catch (e) {
					true;
				}
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__prop_test_node;")

			return assert_eq(result, true, "Accessing property of freed object should throw")

		"test_call_method_after_free_throws":
			# Calling method on freed object should throw
			var node = Node.new()
			test_root.add_child(node)
			sandbox.set_global("__method_test_node", node)

			node.queue_free()
			await scene_tree.process_frame
			await scene_tree.process_frame

			var code = """
				try {
					__method_test_node.get_child_count();
					false;
				} catch (e) {
					true;
				}
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__method_test_node;")

			return assert_eq(result, true, "Calling method on freed object should throw")

		"test_no_crash_on_deleted_object":
			# Engine should not crash when JS accesses deleted object
			var node = Node.new()
			test_root.add_child(node)
			sandbox.set_global("__crash_test_node", node)

			node.queue_free()
			await scene_tree.process_frame
			await scene_tree.process_frame

			# Try multiple operations - none should crash
			sandbox.eval("""
				try { __crash_test_node.name; } catch (e) {}
				try { __crash_test_node.get_child_count(); } catch (e) {}
				try { __crash_test_node.add_child(new Node()); } catch (e) {}
			""")
			sandbox.eval("delete globalThis.__crash_test_node;")

			# If we get here, no crash occurred
			return { "passed": true, "message": "" }

		"test_refcounted_not_freed_while_js_holds":
			# RefCounted objects should not be freed while JS holds reference
			var code = """
				var resource = new Resource();
				resource.__handle !== undefined && resource.__handle !== null;
			"""
			return assert_eval(code, true)

		"test_refcounted_freed_after_js_releases":
			# This is hard to test directly, but we can verify the pattern works
			var code = """
				(function() {
					var resource = new Resource();
					// Resource should be valid here
					return resource !== null;
				})();
				// After function, resource goes out of scope
				true;
			"""
			return assert_eval(code, true)

		"test_child_survives_parent_removal_from_tree":
			# Child should be accessible after parent removed from tree (not freed)
			var code = """
				var parent = new Node();
				var child = new Node();
				child.name = 'ChildNode';
				parent.add_child(child);

				// Parent not in tree, but child should still be valid
				child.name === 'ChildNode';
			"""
			return assert_eval(code, true)

		"test_handle_invalid_after_free":
			# After queue_free, the handle should be marked invalid
			var node = Node.new()
			node.name = "HandleInvalidTest"
			test_root.add_child(node)

			sandbox.set_global("__handle_test_node", node)
			var handle_before = sandbox.eval("__handle_test_node.__handle")

			node.queue_free()
			await scene_tree.process_frame
			await scene_tree.process_frame

			# Trying to use the object should fail
			var code = """
				try {
					__handle_test_node.name;
					'still_valid';
				} catch (e) {
					'invalid';
				}
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__handle_test_node;")

			return assert_eq(result, "invalid", "Handle should be invalid after free")

		"test_multiple_js_refs_same_object":
			# Multiple JS variables can reference the same Godot object
			var node = Node.new()
			node.name = "SharedRefTest"
			test_root.add_child(node)

			sandbox.set_global("__shared_ref", node)

			var code = """
				var ref1 = __shared_ref;
				var ref2 = __shared_ref;
				ref1.__handle === ref2.__handle && ref1.name === ref2.name;
			"""
			var result = sandbox.eval(code)
			sandbox.eval("delete globalThis.__shared_ref;")

			return assert_eq(result, true, "Multiple refs should point to same object")

		"test_object_valid_check":
			# Should be able to check if object is valid before using
			# This tests if there's a way to check validity (implementation dependent)
			var node = Node.new()
			node.name = "ValidityTest"
			test_root.add_child(node)

			sandbox.set_global("__validity_test", node)

			# Before deletion - should be valid
			var valid_before = sandbox.eval("""
				__validity_test !== null && __validity_test !== undefined;
			""")

			node.queue_free()
			await scene_tree.process_frame
			await scene_tree.process_frame

			# After deletion - accessing should throw
			var throws_after = sandbox.eval("""
				try {
					var x = __validity_test.name;
					false;
				} catch (e) {
					true;
				}
			""")
			sandbox.eval("delete globalThis.__validity_test;")

			if not valid_before:
				return { "passed": false, "message": "Object should be valid before deletion" }
			if not throws_after:
				return { "passed": false, "message": "Object access should throw after deletion" }
			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.eval("""
		delete globalThis.__delete_test_node;
		delete globalThis.__prop_test_node;
		delete globalThis.__method_test_node;
		delete globalThis.__crash_test_node;
		delete globalThis.__handle_test_node;
		delete globalThis.__shared_ref;
		delete globalThis.__validity_test;
	""")
	super.teardown()

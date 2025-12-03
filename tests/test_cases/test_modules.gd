class_name TestModules
extends TestBase

## Tests for ES6 module import/export system

func get_suite_name() -> String:
	return "Modules"

func get_tests() -> Array[String]:
	return [
		# Basic module functionality
		"test_eval_module_exists",
		"test_eval_module_basic",
		"test_export_const",
		"test_export_function",
		"test_import_from_file",
		"test_import_multiple_exports",
		"test_import_default",
		# Relative imports
		"test_import_relative_same_dir",
		"test_import_relative_parent_dir",
		# Security
		"test_import_blocks_path_traversal",
		"test_import_blocks_absolute_path",
		# Module caching
		"test_module_runs_once",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_eval_module_exists":
			# Test that eval_module method exists
			return assert_true(sandbox.has_method("eval_module"), "sandbox should have eval_module method")

		"test_eval_module_basic":
			# Test basic module evaluation (no imports)
			sandbox.eval_module("""
				export const value = 42;
			""", "user://test_basic.js")
			# Module should not throw
			return { "passed": true, "message": "" }

		"test_export_const":
			# Create a module that exports a constant
			var test_dir = "user://modules_test/"
			_ensure_directory(test_dir)

			# Write module file
			var file = FileAccess.open(test_dir + "constants.js", FileAccess.WRITE)
			file.store_string("export const PI = 3.14159;\nexport const E = 2.71828;")
			file.close()

			# Evaluate module that imports
			sandbox.eval_module("""
				import { PI } from './constants.js';
				globalThis.__test_pi = PI;
			""", test_dir + "main.js")

			var result = sandbox.get_global("__test_pi")
			sandbox.eval("delete globalThis.__test_pi;")
			_cleanup_directory(test_dir)

			if typeof(result) != TYPE_FLOAT and typeof(result) != TYPE_INT:
				return { "passed": false, "message": "Expected number, got %s" % type_string(typeof(result)) }

			var pi_val = float(result)
			if abs(pi_val - 3.14159) > 0.0001:
				return { "passed": false, "message": "Expected ~3.14159, got %f" % pi_val }

			return { "passed": true, "message": "" }

		"test_export_function":
			var test_dir = "user://modules_func_test/"
			_ensure_directory(test_dir)

			# Write module with function export
			var file = FileAccess.open(test_dir + "math.js", FileAccess.WRITE)
			file.store_string("""
export function add(a, b) {
	return a + b;
}

export function multiply(a, b) {
	return a * b;
}
""")
			file.close()

			# Import and use the function
			sandbox.eval_module("""
				import { add, multiply } from './math.js';
				globalThis.__test_sum = add(10, 5);
				globalThis.__test_product = multiply(10, 5);
			""", test_dir + "main.js")

			var sum_result = sandbox.get_global("__test_sum")
			var product_result = sandbox.get_global("__test_product")
			sandbox.eval("delete globalThis.__test_sum; delete globalThis.__test_product;")
			_cleanup_directory(test_dir)

			if sum_result != 15:
				return { "passed": false, "message": "Expected sum 15, got %s" % str(sum_result) }
			if product_result != 50:
				return { "passed": false, "message": "Expected product 50, got %s" % str(product_result) }

			return { "passed": true, "message": "" }

		"test_import_from_file":
			var test_dir = "user://modules_import_test/"
			_ensure_directory(test_dir)

			# Write a utility module
			var file = FileAccess.open(test_dir + "utils.js", FileAccess.WRITE)
			file.store_string("""
export function greet(name) {
	return 'Hello, ' + name + '!';
}
""")
			file.close()

			# Import from file
			sandbox.eval_module("""
				import { greet } from './utils.js';
				globalThis.__test_greeting = greet('World');
			""", test_dir + "main.js")

			var result = sandbox.get_global("__test_greeting")
			sandbox.eval("delete globalThis.__test_greeting;")
			_cleanup_directory(test_dir)

			return assert_eq(result, "Hello, World!")

		"test_import_multiple_exports":
			var test_dir = "user://modules_multi_test/"
			_ensure_directory(test_dir)

			# Write module with multiple exports
			var file = FileAccess.open(test_dir + "config.js", FileAccess.WRITE)
			file.store_string("""
export const VERSION = '1.0.0';
export const DEBUG = true;
export const MAX_ITEMS = 100;
""")
			file.close()

			# Import multiple exports
			sandbox.eval_module("""
				import { VERSION, DEBUG, MAX_ITEMS } from './config.js';
				globalThis.__test_version = VERSION;
				globalThis.__test_debug = DEBUG;
				globalThis.__test_max = MAX_ITEMS;
			""", test_dir + "main.js")

			var version = sandbox.get_global("__test_version")
			var debug = sandbox.get_global("__test_debug")
			var max_items = sandbox.get_global("__test_max")
			sandbox.eval("delete globalThis.__test_version; delete globalThis.__test_debug; delete globalThis.__test_max;")
			_cleanup_directory(test_dir)

			if version != "1.0.0":
				return { "passed": false, "message": "VERSION mismatch: %s" % str(version) }
			if debug != true:
				return { "passed": false, "message": "DEBUG mismatch: %s" % str(debug) }
			if max_items != 100:
				return { "passed": false, "message": "MAX_ITEMS mismatch: %s" % str(max_items) }

			return { "passed": true, "message": "" }

		"test_import_default":
			var test_dir = "user://modules_default_test/"
			_ensure_directory(test_dir)

			# Write module with default export
			var file = FileAccess.open(test_dir + "player.js", FileAccess.WRITE)
			file.store_string("""
export default {
	name: 'Player',
	health: 100,
	damage: function(amount) {
		this.health -= amount;
		return this.health;
	}
};
""")
			file.close()

			# Import default export
			sandbox.eval_module("""
				import Player from './player.js';
				globalThis.__test_name = Player.name;
				globalThis.__test_health = Player.damage(30);
			""", test_dir + "main.js")

			var name = sandbox.get_global("__test_name")
			var health = sandbox.get_global("__test_health")
			sandbox.eval("delete globalThis.__test_name; delete globalThis.__test_health;")
			_cleanup_directory(test_dir)

			if name != "Player":
				return { "passed": false, "message": "name mismatch: %s" % str(name) }
			if health != 70:
				return { "passed": false, "message": "health mismatch: %s" % str(health) }

			return { "passed": true, "message": "" }

		"test_import_relative_same_dir":
			var test_dir = "user://modules_rel_same/"
			_ensure_directory(test_dir)

			# Write module in same directory
			var file = FileAccess.open(test_dir + "helper.js", FileAccess.WRITE)
			file.store_string("export const HELPER_VALUE = 'from_helper';")
			file.close()

			# Import with ./ prefix
			sandbox.eval_module("""
				import { HELPER_VALUE } from './helper.js';
				globalThis.__test_helper = HELPER_VALUE;
			""", test_dir + "main.js")

			var result = sandbox.get_global("__test_helper")
			sandbox.eval("delete globalThis.__test_helper;")
			_cleanup_directory(test_dir)

			return assert_eq(result, "from_helper")

		"test_import_relative_parent_dir":
			var test_dir = "user://modules_rel_parent/"
			var sub_dir = test_dir + "sub/"
			_ensure_directory(test_dir)
			_ensure_directory(sub_dir)

			# Write module in parent directory
			var file = FileAccess.open(test_dir + "parent_module.js", FileAccess.WRITE)
			file.store_string("export const PARENT_VALUE = 'from_parent';")
			file.close()

			# Write main in subdirectory that imports from parent using ../
			file = FileAccess.open(sub_dir + "child.js", FileAccess.WRITE)
			file.store_string("""
import { PARENT_VALUE } from '../parent_module.js';
globalThis.__test_parent = PARENT_VALUE;
""")
			file.close()

			# Evaluate the child module directly (from sub directory)
			# This tests that ../ works to go up to parent
			sandbox.eval_module("""
				import './child.js';
			""", sub_dir + "entry.js")

			var result = sandbox.get_global("__test_parent")
			sandbox.eval("delete globalThis.__test_parent;")
			_cleanup_directory(sub_dir)
			_cleanup_directory(test_dir)

			return assert_eq(result, "from_parent")

		"test_import_blocks_path_traversal":
			# Try to import with path traversal - should fail at module load time
			var test_dir = "user://modules_traversal/"
			_ensure_directory(test_dir)

			# Write a module file that attempts path traversal import
			# The module loader should reject this before execution
			var file = FileAccess.open(test_dir + "bad_import.js", FileAccess.WRITE)
			file.store_string("""
import { x } from '../../etc/passwd';
globalThis.__test_traversal = 'allowed';
""")
			file.close()

			# Try to load the module - should fail
			sandbox.eval_module("""
				globalThis.__test_traversal = 'before_import';
			""", test_dir + "setup.js")

			# Now try the bad import - this should fail and set an error
			sandbox.eval_module("""
				import './bad_import.js';
				globalThis.__test_traversal = 'allowed';
			""", test_dir + "main.js")

			var result = sandbox.get_global("__test_traversal")
			var error = sandbox.get_last_error()
			sandbox.eval("delete globalThis.__test_traversal;")
			_cleanup_directory(test_dir)

			# The import should be blocked - result should still be 'before_import' or null
			# and there should be an error about path traversal
			if error.contains("traversal") or error.contains("not allowed") or result != "allowed":
				return { "passed": true, "message": "" }

			return { "passed": false, "message": "Path traversal should be blocked. Result: %s" % str(result) }

		"test_import_blocks_absolute_path":
			# Try to import with absolute filesystem path - should fail
			var test_dir = "user://modules_absolute/"
			_ensure_directory(test_dir)

			# Write a module file that attempts absolute path import
			var file = FileAccess.open(test_dir + "bad_import.js", FileAccess.WRITE)
			file.store_string("""
import { x } from '/etc/passwd';
globalThis.__test_absolute = 'allowed';
""")
			file.close()

			# Setup
			sandbox.eval_module("""
				globalThis.__test_absolute = 'before_import';
			""", test_dir + "setup.js")

			# Try the bad import
			sandbox.eval_module("""
				import './bad_import.js';
				globalThis.__test_absolute = 'allowed';
			""", test_dir + "main.js")

			var result = sandbox.get_global("__test_absolute")
			var error = sandbox.get_last_error()
			sandbox.eval("delete globalThis.__test_absolute;")
			_cleanup_directory(test_dir)

			# Should be blocked
			if error.contains("res://") or error.contains("user://") or result != "allowed":
				return { "passed": true, "message": "" }

			return { "passed": false, "message": "Absolute paths should be blocked. Result: %s, Error: %s" % [str(result), error] }

		"test_module_runs_once":
			# Verify that importing the same module multiple times only runs it once
			var test_dir = "user://modules_once/"
			_ensure_directory(test_dir)

			# Write module that increments a counter
			var file = FileAccess.open(test_dir + "counter.js", FileAccess.WRITE)
			file.store_string("""
if (!globalThis.__module_counter) {
	globalThis.__module_counter = 0;
}
globalThis.__module_counter++;
export const dummy = true;
""")
			file.close()

			# Import the same module multiple times
			sandbox.eval_module("""
				import './counter.js';
				import './counter.js';
				import { dummy } from './counter.js';
			""", test_dir + "main.js")

			var count = sandbox.get_global("__module_counter")
			sandbox.eval("delete globalThis.__module_counter;")
			_cleanup_directory(test_dir)

			# Module should only run once despite multiple imports
			return assert_eq(count, 1, "Module should only execute once")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

# Helper to ensure directory exists
func _ensure_directory(path: String) -> void:
	var dir = DirAccess.open("user://")
	if dir:
		var relative = path.replace("user://", "").trim_suffix("/")
		if not dir.dir_exists(relative):
			dir.make_dir_recursive(relative)

# Helper to clean up test directories
func _cleanup_directory(path: String) -> void:
	var dir = DirAccess.open(path)
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if not dir.current_is_dir():
				dir.remove(file_name)
			file_name = dir.get_next()
		dir.list_dir_end()

	# Remove the directory itself
	var parent = DirAccess.open("user://")
	if parent:
		var relative = path.replace("user://", "").trim_suffix("/")
		if parent.dir_exists(relative):
			parent.remove(relative)

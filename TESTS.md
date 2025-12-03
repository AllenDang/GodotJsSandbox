# GodotJSRuntime Test System Design

## Overview

This document defines the test system architecture for GodotJSRuntime. Tests are organized by category, run independently, and restore state after completion to prevent interference between test suites.

## Directory Structure

```
tests/                            # Test project (Godot project)
├── main.gd                       # Entry point, invokes TestRunner
├── main.tscn
├── project.godot
├── test_cases/
│   ├── test_runner.gd            # Test framework and orchestrator
│   ├── test_base.gd              # Base class for all test suites
│   │
│   ├── test_basic_eval.gd        # Basic JavaScript evaluation
│   ├── test_security.gd          # Sandbox security & blocklist
│   ├── test_bindings.gd          # Class/method/property bindings
│   ├── test_singletons.gd        # Singleton access (Time, Input)
│   ├── test_signals.gd           # Signal connection & callbacks
│   ├── test_lifecycle.gd         # Script lifecycle (_ready, _process)
│   ├── test_node_ops.gd          # Node operations (create, add_child, queue_free)
│   └── test_persistence.gd       # Scene saving/loading (future)
│
├── scripts/                      # JS test scripts (if needed)
│   └── *.js
```

## Test Framework

### TestBase Class

All test suites extend `TestBase`:

```gdscript
class_name TestBase
extends RefCounted

var sandbox: JSSandbox
var scene_tree: SceneTree
var test_root: Node  # Temporary node for test objects

# Called before any tests run
func setup(p_sandbox: JSSandbox, p_scene_tree: SceneTree) -> void:
    sandbox = p_sandbox
    scene_tree = p_scene_tree
    test_root = Node.new()
    test_root.name = "TestRoot"
    scene_tree.root.add_child(test_root)

# Called after all tests complete - MUST restore state
func teardown() -> void:
    # Clean up all test nodes
    if test_root and is_instance_valid(test_root):
        test_root.queue_free()
    test_root = null

# Override in subclasses
func get_suite_name() -> String:
    return "Unknown"

# Override in subclasses - return list of test method names
func get_tests() -> Array[String]:
    return []

# Override in subclasses
func run_test(name: String) -> Dictionary:
    # Returns: { "passed": bool, "message": String }
    return { "passed": false, "message": "Not implemented" }

# Helper: assert condition
func assert_true(condition: bool, message: String = "") -> Dictionary:
    if condition:
        return { "passed": true, "message": "" }
    return { "passed": false, "message": message if message else "Assertion failed" }

# Helper: assert equality
func assert_eq(actual, expected, message: String = "") -> Dictionary:
    if actual == expected:
        return { "passed": true, "message": "" }
    var msg = message if message else "Expected %s but got %s" % [expected, actual]
    return { "passed": false, "message": msg }

# Helper: assert JS eval returns expected value
func assert_eval(code: String, expected, message: String = "") -> Dictionary:
    var result = sandbox.eval(code)
    return assert_eq(result, expected, message)

# Helper: assert JS eval throws error
func assert_throws(code: String, message: String = "") -> Dictionary:
    sandbox.eval(code)
    var error = sandbox.get_last_error()
    if error != "":
        return { "passed": true, "message": "" }
    return { "passed": false, "message": message if message else "Expected error but none thrown" }
```

### TestRunner Class

```gdscript
class_name TestRunner
extends RefCounted

signal all_completed(passed: int, failed: int)

var _suites: Array[TestBase] = []

func add_suite(suite: TestBase) -> void:
    _suites.append(suite)

func run_all(sandbox: JSSandbox, scene_tree: SceneTree) -> Dictionary:
    var total_passed := 0
    var total_failed := 0

    print("\n" + "=".repeat(60))
    print("  GodotJSRuntime Test Suite")
    print("=".repeat(60))

    for suite in _suites:
        var result = await _run_suite(suite, sandbox, scene_tree)
        total_passed += result.passed
        total_failed += result.failed

    _print_summary(total_passed, total_failed)
    all_completed.emit(total_passed, total_failed)
    return { "passed": total_passed, "failed": total_failed }

func _run_suite(suite: TestBase, sandbox: JSSandbox, scene_tree: SceneTree) -> Dictionary:
    print("\n--- %s ---" % suite.get_suite_name())

    suite.setup(sandbox, scene_tree)

    var passed := 0
    var failed := 0

    for test_name in suite.get_tests():
        var result = await suite.run_test(test_name)
        if result.passed:
            passed += 1
            print("  ✓ %s" % test_name)
        else:
            failed += 1
            print("  ✗ %s: %s" % [test_name, result.message])

    suite.teardown()  # IMPORTANT: Restore state

    return { "passed": passed, "failed": failed }
```

## Test Suites

### 1. Basic Eval (`test_basic_eval.gd`)

Tests fundamental JavaScript evaluation.

| Test | Description |
|------|-------------|
| `test_arithmetic` | Basic math: `1 + 2 * 3` = 7 |
| `test_string_concat` | String operations |
| `test_array_operations` | Array creation and methods |
| `test_object_literals` | Object creation `{x: 1, y: 2}` |
| `test_function_definition` | Define and call functions |
| `test_closures` | Closure variable capture |
| `test_json_parse` | JSON.parse/stringify |

**State to Restore:** None (pure eval tests)

---

### 2. Security (`test_security.gd`)

Tests sandbox security restrictions.

| Test | Description |
|------|-------------|
| `test_blocked_class_file_access` | `new FileAccess()` throws |
| `test_blocked_class_os` | `new OS()` throws |
| `test_blocked_class_thread` | `new Thread()` throws |
| `test_blocked_class_http` | `new HTTPRequest()` throws |
| `test_blocked_class_network` | TCP/UDP classes throw |
| `test_blocked_method_call_deferred` | `call_deferred()` throws |
| `test_blocked_method_set_deferred` | `set_deferred()` throws |
| `test_blocked_method_set` | `Object.set()` throws |
| `test_blocked_resource_gdscript` | `load("*.gd")` throws |
| `test_path_traversal` | `load("../..")` throws |
| `test_absolute_path` | `load("/etc/passwd")` throws |
| `test_timeout_protection` | Infinite loop interrupted |

**State to Restore:** Reset timeout to default after timeout test

---

### 3. Bindings (`test_bindings.gd`)

Tests generated class bindings.

| Test | Description |
|------|-------------|
| `test_node3d_create` | `new Node3D()` works |
| `test_node3d_properties` | position, rotation, scale |
| `test_node2d_create` | `new Node2D()` works |
| `test_node2d_properties` | position, rotation |
| `test_sprite2d_properties` | centered, flip_h, etc. |
| `test_label_text` | Label.text property |
| `test_timer_properties` | wait_time, one_shot |
| `test_control_properties` | custom_minimum_size |
| `test_method_add_child` | parent.add_child(child) |
| `test_method_get_child` | parent.get_child(0) |
| `test_method_get_child_count` | Returns correct count |
| `test_method_get_parent` | Returns parent node |
| `test_method_get_node` | get_node("path") works |

**State to Restore:** All created nodes via `test_root.queue_free()`

---

### 4. Singletons (`test_singletons.gd`)

Tests singleton access.

| Test | Description |
|------|-------------|
| `test_time_get_ticks_msec` | Returns number > 0 |
| `test_time_get_ticks_usec` | Returns number > 0 |
| `test_time_get_unix_time` | Returns reasonable timestamp |
| `test_input_is_anything_pressed` | Returns boolean |
| `test_input_is_key_pressed` | Returns boolean |
| `test_input_is_action_pressed` | Returns boolean |
| `test_input_get_vector` | Returns {x, y} object |

**State to Restore:** None (read-only operations)

---

### 5. Signals (`test_signals.gd`)

Tests signal connection and callbacks.

| Test | Description |
|------|-------------|
| `test_connect_signal` | `timer.connect('timeout', fn)` works |
| `test_callback_invoked` | Callback actually fires |
| `test_callback_with_args` | Signal args passed to callback |
| `test_disconnect_signal` | `disconnect()` works |
| `test_multiple_connections` | Multiple callbacks on one signal |

**State to Restore:** Disconnect all signals, queue_free test nodes

---

### 6. Lifecycle (`test_lifecycle.gd`)

Tests script lifecycle methods.

| Test | Description |
|------|-------------|
| `test_ready_called` | `_ready()` is invoked |
| `test_process_called` | `_process(delta)` is invoked |
| `test_state_persists` | Variables persist between calls |
| `test_this_binding` | `this` refers to owner node |
| `test_this_position` | `this.position` works |
| `test_this_get_child` | `this.get_child()` works |
| `test_enter_exit_tree` | `_enter_tree`/`_exit_tree` called |

**State to Restore:** Remove test node from tree, queue_free

---

### 7. Node Operations (`test_node_ops.gd`)

Tests node manipulation.

| Test | Description |
|------|-------------|
| `test_create_node` | `new Node3D()` returns valid object |
| `test_set_name` | `node.name = "X"` works |
| `test_add_child` | `parent.add_child(child)` works |
| `test_remove_child` | `parent.remove_child(child)` works |
| `test_queue_free` | `node.queue_free()` works |
| `test_get_children` | Iterate children |
| `test_reparent` | Move node between parents |
| `test_duplicate` | `node.duplicate()` works |

**State to Restore:** queue_free all test nodes

---

### 8. Persistence (`test_persistence.gd`)

Tests scene saving/loading.

| Test | Description |
|------|-------------|
| `test_save_level_user_path` | Save to `user://` succeeds |
| `test_save_level_res_blocked` | Save to `res://` fails |
| `test_save_level_traversal_blocked` | Path traversal fails |
| `test_js_file_created` | `.js` file exists after save |
| `test_tscn_file_created` | `.tscn` file exists after save |
| `test_metadata_created` | `level.json` exists after save |

**State to Restore:** Delete test files from `user://test_*`

---

## Integration in main.gd

```gdscript
extends Node3D

var sandbox: JSSandbox

func _ready() -> void:
    print("=== GodotJSRuntime Demo ===")

    sandbox = JSSandbox.new()
    sandbox.set_timeout_ms(5000)
    sandbox.set_memory_limit_mb(64)
    sandbox.error_occurred.connect(_on_error)
    sandbox.console_output.connect(_on_console)

    # Run tests
    await run_tests()

    print("\n=== Demo Complete ===")

func run_tests() -> void:
    var runner = TestRunner.new()

    # Add all test suites
    runner.add_suite(TestBasicEval.new())
    runner.add_suite(TestSecurity.new())
    runner.add_suite(TestBindings.new())
    runner.add_suite(TestSingletons.new())
    runner.add_suite(TestSignals.new())
    runner.add_suite(TestLifecycle.new())
    runner.add_suite(TestNodeOps.new())
    runner.add_suite(TestPersistence.new())

    # Run all
    var results = await runner.run_all(sandbox, get_tree())

    if results.failed > 0:
        print("WARNING: %d tests failed!" % results.failed)

func _on_error(message: String, line: int, column: int) -> void:
    printerr("JS Error at %d:%d - %s" % [line, column, message])

func _on_console(message: String) -> void:
    print("[JS] %s" % message)
```

## State Restoration Guidelines

Each test suite MUST restore state in `teardown()`:

1. **Nodes**: Call `test_root.queue_free()` to remove all test nodes
2. **Sandbox Settings**: Reset timeout/memory to defaults
3. **Signals**: Disconnect any connected signals
4. **Files**: Delete any created test files in `user://`
5. **Globals**: Clear any JS globals set during tests

Example teardown:

```gdscript
func teardown() -> void:
    # 1. Clean up nodes
    if test_root and is_instance_valid(test_root):
        test_root.queue_free()

    # 2. Reset sandbox settings
    sandbox.set_timeout_ms(5000)

    # 3. Clear JS globals
    sandbox.eval("delete globalThis.__test_value;")

    # 4. Wait for queue_free to process
    await scene_tree.process_frame
```

## Running Tests

### From Editor
Run the demo project - tests execute automatically in `_ready()`.

### From Command Line
```bash
cd demo
godot --headless --quit-after 10
```

### Selective Testing
Modify `main.gd` to only add specific suites:
```gdscript
runner.add_suite(TestSecurity.new())  # Only run security tests
```

## Adding New Tests

1. Create new file `tests/test_<category>.gd`
2. Extend `TestBase`
3. Implement `get_suite_name()`, `get_tests()`, `run_test()`
4. Implement `teardown()` to restore state
5. Add suite to `main.gd`: `runner.add_suite(TestNewCategory.new())`

## Test Naming Convention

- Suite classes: `TestCategoryName` (PascalCase)
- Test methods: `test_feature_behavior` (snake_case)
- Test files: `test_category.gd` (snake_case)

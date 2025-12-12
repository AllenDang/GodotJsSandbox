extends Node3D

const TestRefCountedTypesClass = preload("res://test_cases/test_refcounted_types.gd")
const TestCustomSignalsClass = preload("res://test_cases/test_custom_signals.gd")
const TestLevelPersistenceClass = preload("res://test_cases/test_level_persistence.gd")
const TestModulesClass = preload("res://test_cases/test_modules.gd")
const TestInputClass = preload("res://test_cases/test_input.gd")
const TestPhysicsClass = preload("res://test_cases/test_physics.gd")
const TestAnimationClass = preload("res://test_cases/test_animation.gd")
const TestAudioClass = preload("res://test_cases/test_audio.gd")
const TestTweenSceneTreeClass = preload("res://test_cases/test_tween_scenetree.gd")
const TestAsyncAwaitClass = preload("res://test_cases/test_async_await.gd")
const TestErrorCaptureClass = preload("res://test_cases/test_error_capture.gd")
const TestAsyncSceneLoadingClass = preload("res://test_cases/test_async_scene_loading.gd")
const TestEnumsClass = preload("res://test_cases/test_enums.gd")
const TestArrayProxyClass = preload("res://test_cases/test_array_proxy.gd")
const TestPackedArrayProxyClass = preload("res://test_cases/test_packed_array_proxy.gd")
const TestBulkOperationsClass = preload("res://test_cases/test_bulk_operations.gd")
const TestPackedArrayAppendClass = preload("res://test_cases/test_packed_array_append.gd")

var sandbox: JSSandbox

func _ready() -> void:
	print("=== GodotJSRuntime Test Suite ===")

	# Check if JSSandbox class exists
	if not ClassDB.class_exists("JSSandbox"):
		printerr("ERROR: JSSandbox class not found - extension not loaded")
		return

	# Create sandbox
	sandbox = JSSandbox.new()
	sandbox.set_timeout_ms(5000)
	sandbox.set_memory_limit_mb(64)

	# Connect signals
	sandbox.error_occurred.connect(_on_error)
	sandbox.console_output.connect(_on_console)

	# Run all tests
	await run_tests()

	# Clean up sandbox to free all JS-created objects and RIDs
	sandbox.reset()

	# Wait for deferred deletions to process (need multiple frames for queue_free)
	for i in range(5):
		await get_tree().process_frame

	print("\n=== Test Suite Complete ===")

func run_tests() -> void:
	var runner = TestRunner.new()

	# Add all test suites (organized by category)
	# Core functionality
	runner.add_suite(TestBasicEval.new())
	runner.add_suite(TestBindings.new())
	runner.add_suite(TestNodeOps.new())
	runner.add_suite(TestSingletons.new())

	# Security (PRD.md blocklist)
	runner.add_suite(TestSecurity.new())
	runner.add_suite(TestResourceLoading.new())

	# Lifecycle and script integration
	runner.add_suite(TestScriptAttach.new())
	runner.add_suite(TestLifecycle.new())
	runner.add_suite(TestSignals.new())

	# Memory safety and limits
	runner.add_suite(TestObjectLifecycle.new())
	runner.add_suite(TestExecutionLimits.new())

	# RefCounted types (Resource subclasses)
	runner.add_suite(TestRefCountedTypesClass.new())

	# Custom signals
	runner.add_suite(TestCustomSignalsClass.new())

	# Level persistence
	runner.add_suite(TestLevelPersistenceClass.new())

	# ES6 Modules
	runner.add_suite(TestModulesClass.new())

	# Input handling
	runner.add_suite(TestInputClass.new())

	# Physics bodies
	runner.add_suite(TestPhysicsClass.new())

	# Animation
	runner.add_suite(TestAnimationClass.new())

	# Audio
	runner.add_suite(TestAudioClass.new())

	# Tween and SceneTree bindings
	runner.add_suite(TestTweenSceneTreeClass.new())

	# Async/await support (Promise-Signal bridge)
	runner.add_suite(TestAsyncAwaitClass.new())

	# Cross-script method calls (calling JS methods on other nodes)
	runner.add_suite(TestCrossScript.new())

	# Error capture for AI feedback
	runner.add_suite(TestErrorCaptureClass.new())

	# Async scene loading
	runner.add_suite(TestAsyncSceneLoadingClass.new())

	# Enums (global enums and singleton class enums)
	runner.add_suite(TestEnumsClass.new())

	# Array proxy (zero-copy array access)
	runner.add_suite(TestArrayProxyClass.new())

	# Packed array proxy (zero-copy packed array access)
	runner.add_suite(TestPackedArrayProxyClass.new())

	# Bulk operations (zero-copy bulk encode/decode for all packed array types)
	runner.add_suite(TestBulkOperationsClass.new())

	# Packed array append (as_vector3_array -> append_array -> ArrayMesh flow)
	runner.add_suite(TestPackedArrayAppendClass.new())

	# Run all tests
	var results = await runner.run_all(sandbox, get_tree())

	if results.failed > 0:
		printerr("WARNING: %d tests failed!" % results.failed)
		# Exit with error code for CI
		get_tree().quit(1)
	else:
		print("All tests passed!")
		get_tree().quit(0)

func _on_error(type: String, message: String, file: String, line: int, column: int) -> void:
	printerr("JS Error [%s] at %s:%d:%d - %s" % [type, file, line, column, message])

func _on_console(message: String) -> void:
	print("[JS] %s" % message)

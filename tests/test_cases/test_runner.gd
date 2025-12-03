class_name TestRunner
extends RefCounted

signal all_completed(passed: int, failed: int)

var _suites: Array = []

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
		var result = await _run_single_test(suite, test_name)
		if result.passed:
			passed += 1
			print("  [PASS] %s" % test_name)
		else:
			failed += 1
			print("  [FAIL] %s: %s" % [test_name, result.message])

	suite.teardown()  # IMPORTANT: Restore state

	# Wait for queue_free to process
	await scene_tree.process_frame

	return { "passed": passed, "failed": failed }

func _run_single_test(suite: TestBase, test_name: String) -> Dictionary:
	# Run the test and handle any exceptions
	var result = await suite.run_test(test_name)
	if result == null:
		return { "passed": false, "message": "Test returned null" }
	return result

func _print_summary(passed: int, failed: int) -> void:
	var total = passed + failed
	print("\n" + "=".repeat(60))
	print("  Results: %d/%d tests passed" % [passed, total])
	if failed > 0:
		print("  WARNING: %d tests FAILED" % failed)
	else:
		print("  All tests passed!")
	print("=".repeat(60) + "\n")

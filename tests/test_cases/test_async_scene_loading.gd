class_name TestAsyncSceneLoading
extends TestBase

## Tests for async scene loading (JSSandbox.load_scene_async)
## AsyncSceneLoader should:
## - Load scenes asynchronously with progress reporting
## - Reattach JS scripts to sandbox context
## - Emit signals: progress_changed, completed, failed
## - Support polling via poll() method

func get_suite_name() -> String:
	return "Async Scene Loading"

func get_tests() -> Array[String]:
	return [
		# Basic functionality
		"test_load_scene_async_returns_loader",
		"test_loader_initial_state",
		"test_loader_has_progress_signal",
		"test_loader_has_completed_signal",
		"test_loader_has_failed_signal",
		# Loading stages
		"test_loader_stages_exist",
		"test_loader_starts_loading",
		# Error handling
		"test_load_nonexistent_scene_fails",
		"test_load_invalid_path_fails",
		# Completion
		"test_async_load_completes",
		"test_async_load_returns_scene",
		"test_async_load_progress_increases",
		# Script reattachment
		"test_scripts_reattached_to_sandbox",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_load_scene_async_returns_loader":
			# load_scene_async should return an AsyncSceneLoader
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			if loader == null:
				return { "passed": false, "message": "load_scene_async returned null" }
			if not loader is AsyncSceneLoader:
				return { "passed": false, "message": "Expected AsyncSceneLoader, got %s" % typeof(loader) }
			return { "passed": true, "message": "" }

		"test_loader_initial_state":
			# Loader should start in loading state
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			if loader.is_completed():
				return { "passed": false, "message": "Loader should not be completed immediately" }
			if loader.is_failed():
				return { "passed": false, "message": "Loader should not be failed immediately" }
			if not loader.is_loading():
				return { "passed": false, "message": "Loader should be in loading state" }
			return { "passed": true, "message": "" }

		"test_loader_has_progress_signal":
			# Loader should have progress_changed signal
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			if not loader.has_signal("progress_changed"):
				return { "passed": false, "message": "Missing progress_changed signal" }
			return { "passed": true, "message": "" }

		"test_loader_has_completed_signal":
			# Loader should have completed signal
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			if not loader.has_signal("completed"):
				return { "passed": false, "message": "Missing completed signal" }
			return { "passed": true, "message": "" }

		"test_loader_has_failed_signal":
			# Loader should have failed signal
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			if not loader.has_signal("failed"):
				return { "passed": false, "message": "Missing failed signal" }
			return { "passed": true, "message": "" }

		"test_loader_stages_exist":
			# All loading stages should be defined
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			# Check stage enum values exist
			if AsyncSceneLoader.STAGE_NOT_STARTED != 0:
				return { "passed": false, "message": "STAGE_NOT_STARTED should be 0" }
			if AsyncSceneLoader.STAGE_LOADING_SCENE != 1:
				return { "passed": false, "message": "STAGE_LOADING_SCENE should be 1" }
			if AsyncSceneLoader.STAGE_INSTANTIATING != 2:
				return { "passed": false, "message": "STAGE_INSTANTIATING should be 2" }
			if AsyncSceneLoader.STAGE_LOADING_SCRIPTS != 3:
				return { "passed": false, "message": "STAGE_LOADING_SCRIPTS should be 3" }
			if AsyncSceneLoader.STAGE_ATTACHING != 4:
				return { "passed": false, "message": "STAGE_ATTACHING should be 4" }
			if AsyncSceneLoader.STAGE_COMPLETED != 5:
				return { "passed": false, "message": "STAGE_COMPLETED should be 5" }
			if AsyncSceneLoader.STAGE_FAILED != 6:
				return { "passed": false, "message": "STAGE_FAILED should be 6" }
			return { "passed": true, "message": "" }

		"test_loader_starts_loading":
			# Loader should start in STAGE_LOADING_SCENE
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			var stage = loader.get_stage()
			if stage != AsyncSceneLoader.STAGE_LOADING_SCENE:
				return { "passed": false, "message": "Expected STAGE_LOADING_SCENE, got stage %d" % stage }
			return { "passed": true, "message": "" }

		"test_load_nonexistent_scene_fails":
			# Loading nonexistent scene should fail
			var loader = sandbox.load_scene_async("res://nonexistent_scene.tscn")
			# Poll until done
			var max_polls = 100
			while not loader.poll() and max_polls > 0:
				await scene_tree.process_frame
				max_polls -= 1
			if not loader.is_failed():
				return { "passed": false, "message": "Expected loader to fail for nonexistent scene" }
			var error = loader.get_error()
			if error.is_empty():
				return { "passed": false, "message": "Expected error message for failed load" }
			return { "passed": true, "message": "" }

		"test_load_invalid_path_fails":
			# Loading with invalid path should fail
			var loader = sandbox.load_scene_async("")
			var max_polls = 100
			while not loader.poll() and max_polls > 0:
				await scene_tree.process_frame
				max_polls -= 1
			if not loader.is_failed():
				return { "passed": false, "message": "Expected loader to fail for empty path" }
			return { "passed": true, "message": "" }

		"test_async_load_completes":
			# Loading valid scene should complete
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			var max_polls = 200
			while not loader.poll() and max_polls > 0:
				await scene_tree.process_frame
				max_polls -= 1
			if loader.is_failed():
				return { "passed": false, "message": "Load failed: %s" % loader.get_error() }
			if not loader.is_completed():
				return { "passed": false, "message": "Load did not complete in time" }
			var scene = loader.get_scene()
			if scene:
				scene.queue_free()
			return { "passed": true, "message": "" }

		"test_async_load_returns_scene":
			# get_scene() should return the loaded scene
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			var max_polls = 200
			while not loader.poll() and max_polls > 0:
				await scene_tree.process_frame
				max_polls -= 1
			if not loader.is_completed():
				return { "passed": false, "message": "Load did not complete" }
			var scene = loader.get_scene()
			if scene == null:
				return { "passed": false, "message": "get_scene() returned null" }
			if not scene is Node:
				scene.queue_free() if scene else null
				return { "passed": false, "message": "get_scene() did not return a Node" }
			if scene.name != "TestRoot":
				var name = scene.name
				scene.queue_free()
				return { "passed": false, "message": "Expected scene name 'TestRoot', got '%s'" % name }
			scene.queue_free()
			return { "passed": true, "message": "" }

		"test_async_load_progress_increases":
			# Progress should increase during loading
			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			var progress_values: Array[float] = []
			var max_polls = 200

			while not loader.poll() and max_polls > 0:
				progress_values.append(loader.get_progress())
				await scene_tree.process_frame
				max_polls -= 1

			# Add final progress
			progress_values.append(loader.get_progress())

			if progress_values.size() < 2:
				var scene = loader.get_scene()
				if scene:
					scene.queue_free()
				return { "passed": false, "message": "Not enough progress samples" }

			# Check progress increased
			var min_progress = progress_values[0]
			var max_progress = progress_values[progress_values.size() - 1]

			var scene = loader.get_scene()
			if scene:
				scene.queue_free()

			if max_progress <= min_progress:
				return { "passed": false, "message": "Progress did not increase: %f -> %f" % [min_progress, max_progress] }
			if max_progress < 1.0 and loader.is_completed():
				return { "passed": false, "message": "Completed but progress < 1.0: %f" % max_progress }
			return { "passed": true, "message": "" }

		"test_scripts_reattached_to_sandbox":
			# Scripts in loaded scene should execute in sandbox context
			sandbox.eval("delete globalThis.__async_test_loaded; delete globalThis.__async_test_ready;")

			var loader = sandbox.load_scene_async("res://fixtures/test_scene.tscn")
			var max_polls = 200
			while not loader.poll() and max_polls > 0:
				await scene_tree.process_frame
				max_polls -= 1

			if not loader.is_completed():
				return { "passed": false, "message": "Load did not complete: %s" % loader.get_error() }

			var scene = loader.get_scene()
			if scene == null:
				return { "passed": false, "message": "No scene returned" }

			# Add to tree to trigger _ready
			test_root.add_child(scene)
			await scene_tree.process_frame
			await scene_tree.process_frame

			# Check if script ran in our sandbox
			var loaded_flag = sandbox.eval("globalThis.__async_test_loaded")
			var ready_flag = sandbox.eval("globalThis.__async_test_ready")

			# Cleanup
			sandbox.eval("delete globalThis.__async_test_loaded; delete globalThis.__async_test_ready;")

			if loaded_flag != true:
				return { "passed": false, "message": "Script not loaded in sandbox context" }
			if ready_flag != true:
				return { "passed": false, "message": "_ready not called in sandbox context" }
			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.eval("delete globalThis.__async_test_loaded; delete globalThis.__async_test_ready;")
	super.teardown()

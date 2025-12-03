class_name TestLevelPersistence
extends TestBase

## Tests for level persistence (save_level functionality)
## Verifies that node trees with JS scripts can be saved and loaded

func get_suite_name() -> String:
	return "Level Persistence"

func get_tests() -> Array[String]:
	return [
		# Basic save functionality
		"test_save_level_creates_directory",
		"test_save_level_creates_scene_file",
		"test_save_level_creates_script_files",
		"test_save_level_creates_metadata",
		# Security
		"test_save_level_blocks_res_path",
		"test_save_level_blocks_absolute_path",
		"test_save_level_blocks_path_traversal",
		# Script handling
		"test_save_level_extracts_js_source",
		"test_save_level_multiple_scripts",
		# Load and verify
		"test_saved_scene_can_be_loaded",
		"test_loaded_scene_has_correct_structure",
		# Load level with script reattachment
		"test_load_level_returns_node",
		"test_load_level_reattaches_script",
		"test_load_level_script_has_source",
		"test_load_level_multiple_scripts",
		"test_load_level_security_blocks_res_path",
		"test_load_level_security_blocks_path_traversal",
		"test_load_level_nonexistent_returns_null",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_save_level_creates_directory":
			# Clean up any existing test directory
			var test_dir = "user://test_level_save/"
			_cleanup_directory(test_dir)

			# Create a simple node tree
			var root = Node3D.new()
			root.name = "TestLevel"

			# Save the level
			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed with error: %d" % err }

			# Check directory was created
			var dir = DirAccess.open("user://")
			var exists = dir.dir_exists("test_level_save")

			_cleanup_directory(test_dir)
			return assert_true(exists, "Directory should be created")

		"test_save_level_creates_scene_file":
			var test_dir = "user://test_level_scene/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "TestLevel"

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			var scene_exists = FileAccess.file_exists(test_dir + "level.tscn")

			_cleanup_directory(test_dir)
			return assert_true(scene_exists, "level.tscn should be created")

		"test_save_level_creates_script_files":
			var test_dir = "user://test_level_scripts/"
			_cleanup_directory(test_dir)

			# Create node with JS script
			var root = Node3D.new()
			root.name = "TestLevel"

			var child = Node3D.new()
			child.name = "ScriptedNode"
			root.add_child(child)
			child.owner = root

			var script = JSScript.new()
			script.source_code = """
				function _ready() {
					console.log('Hello from JS');
				}
			"""
			child.set_script(script)

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed with error: %d" % err }

			# Check for .js file
			var dir = DirAccess.open(test_dir)
			var found_js = false
			if dir:
				dir.list_dir_begin()
				var file_name = dir.get_next()
				while file_name != "":
					if file_name.ends_with(".js"):
						found_js = true
						break
					file_name = dir.get_next()
				dir.list_dir_end()

			_cleanup_directory(test_dir)
			return assert_true(found_js, "JS script file should be created")

		"test_save_level_creates_metadata":
			var test_dir = "user://test_level_meta/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "TestLevel"

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			var meta_exists = FileAccess.file_exists(test_dir + "level.json")

			_cleanup_directory(test_dir)
			return assert_true(meta_exists, "level.json metadata should be created")

		"test_save_level_blocks_res_path":
			var root = Node3D.new()
			root.name = "TestLevel"

			var err = sandbox.save_level(root, "res://levels/test/")
			root.queue_free()

			return assert_eq(err, ERR_INVALID_PARAMETER, "Should block res:// path")

		"test_save_level_blocks_absolute_path":
			var root = Node3D.new()
			root.name = "TestLevel"

			var err = sandbox.save_level(root, "/tmp/test_level/")
			root.queue_free()

			return assert_eq(err, ERR_INVALID_PARAMETER, "Should block absolute path")

		"test_save_level_blocks_path_traversal":
			var root = Node3D.new()
			root.name = "TestLevel"

			var err = sandbox.save_level(root, "user://../etc/")
			root.queue_free()

			return assert_eq(err, ERR_INVALID_PARAMETER, "Should block path traversal")

		"test_save_level_extracts_js_source":
			var test_dir = "user://test_level_source/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "TestLevel"

			var child = Node3D.new()
			child.name = "Player"
			root.add_child(child)
			child.owner = root

			var script = JSScript.new()
			var expected_source = """function _ready() {
	this.position = {x: 0, y: 10, z: 0};
}

function _process(delta) {
	// Update logic
}"""
			script.source_code = expected_source
			child.set_script(script)

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Read the saved script file
			var dir = DirAccess.open(test_dir)
			var script_content = ""
			if dir:
				dir.list_dir_begin()
				var file_name = dir.get_next()
				while file_name != "":
					if file_name.ends_with(".js"):
						var file = FileAccess.open(test_dir + file_name, FileAccess.READ)
						if file:
							script_content = file.get_as_text()
						break
					file_name = dir.get_next()
				dir.list_dir_end()

			_cleanup_directory(test_dir)

			if script_content.is_empty():
				return { "passed": false, "message": "Script file is empty or not found" }

			# Check that source was preserved (allowing for whitespace differences)
			var contains_ready = script_content.contains("_ready")
			var contains_process = script_content.contains("_process")

			if not contains_ready or not contains_process:
				return { "passed": false, "message": "Script source not preserved correctly" }

			return { "passed": true, "message": "" }

		"test_save_level_multiple_scripts":
			var test_dir = "user://test_level_multi/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "TestLevel"

			# Add multiple scripted nodes
			for i in range(3):
				var child = Node3D.new()
				child.name = "Node%d" % i
				root.add_child(child)
				child.owner = root

				var script = JSScript.new()
				script.source_code = "function _ready() { console.log('Node %d'); }" % i
				child.set_script(script)

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Count .js files
			var js_count = 0
			var dir = DirAccess.open(test_dir)
			if dir:
				dir.list_dir_begin()
				var file_name = dir.get_next()
				while file_name != "":
					if file_name.ends_with(".js"):
						js_count += 1
					file_name = dir.get_next()
				dir.list_dir_end()

			_cleanup_directory(test_dir)
			return assert_eq(js_count, 3, "Should create 3 separate JS files")

		"test_saved_scene_can_be_loaded":
			var test_dir = "user://test_level_load/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "LoadTestLevel"

			var child = Node3D.new()
			child.name = "ChildNode"
			root.add_child(child)
			child.owner = root

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Try to load the scene
			var loaded_scene = load(test_dir + "level.tscn")

			_cleanup_directory(test_dir)

			if loaded_scene == null:
				return { "passed": false, "message": "Failed to load saved scene" }

			return { "passed": true, "message": "" }

		"test_loaded_scene_has_correct_structure":
			var test_dir = "user://test_level_struct/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "StructTestLevel"

			var child1 = Node3D.new()
			child1.name = "Platform"
			root.add_child(child1)
			child1.owner = root

			var child2 = Node3D.new()
			child2.name = "Enemy"
			root.add_child(child2)
			child2.owner = root

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Load and verify structure
			var loaded_scene = load(test_dir + "level.tscn")
			if loaded_scene == null:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "Failed to load scene" }

			var instance = loaded_scene.instantiate()
			var has_platform = instance.has_node("Platform")
			var has_enemy = instance.has_node("Enemy")
			var child_count = instance.get_child_count()

			instance.queue_free()
			_cleanup_directory(test_dir)

			if not has_platform:
				return { "passed": false, "message": "Missing Platform node" }
			if not has_enemy:
				return { "passed": false, "message": "Missing Enemy node" }
			if child_count != 2:
				return { "passed": false, "message": "Expected 2 children, got %d" % child_count }

			return { "passed": true, "message": "" }

		# ======================================================================
		# Load Level Tests (with script reattachment)
		# ======================================================================

		"test_load_level_returns_node":
			var test_dir = "user://test_load_returns/"
			_cleanup_directory(test_dir)

			# Create and save a simple level
			var root = Node3D.new()
			root.name = "LoadReturnTest"

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Load using sandbox.load_level
			var loaded = sandbox.load_level(test_dir)

			if loaded == null:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "load_level returned null" }

			var is_node = loaded is Node
			loaded.queue_free()
			_cleanup_directory(test_dir)

			return assert_true(is_node, "load_level should return a Node")

		"test_load_level_reattaches_script":
			var test_dir = "user://test_load_script/"
			_cleanup_directory(test_dir)

			# Create node with JS script
			var root = Node3D.new()
			root.name = "ScriptedLevel"

			var child = Node3D.new()
			child.name = "Player"
			root.add_child(child)
			child.owner = root

			var script = JSScript.new()
			script.source_code = """
				function _ready() {
					console.log('Player ready');
				}
			"""
			child.set_script(script)

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Load using sandbox.load_level
			var loaded = sandbox.load_level(test_dir)

			if loaded == null:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "load_level returned null" }

			# Check if Player node has a script attached
			var player = loaded.get_node_or_null("Player")
			var has_script = player != null and player.get_script() != null

			loaded.queue_free()
			_cleanup_directory(test_dir)

			return assert_true(has_script, "Loaded node should have script reattached")

		"test_load_level_script_has_source":
			var test_dir = "user://test_load_source/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "SourceLevel"

			var child = Node3D.new()
			child.name = "Enemy"
			root.add_child(child)
			child.owner = root

			var expected_content = "_process"
			var script = JSScript.new()
			script.source_code = """
				function _ready() {}
				function _process(delta) {
					// Update enemy
				}
			"""
			child.set_script(script)

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Load using sandbox.load_level
			var loaded = sandbox.load_level(test_dir)

			if loaded == null:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "load_level returned null" }

			var enemy = loaded.get_node_or_null("Enemy")
			var source_ok = false
			if enemy and enemy.get_script():
				var loaded_script = enemy.get_script() as JSScript
				if loaded_script:
					var source = loaded_script.source_code
					source_ok = source.contains(expected_content)

			loaded.queue_free()
			_cleanup_directory(test_dir)

			return assert_true(source_ok, "Loaded script should contain original source")

		"test_load_level_multiple_scripts":
			var test_dir = "user://test_load_multi/"
			_cleanup_directory(test_dir)

			var root = Node3D.new()
			root.name = "MultiScriptLevel"

			# Add 3 scripted nodes
			for i in range(3):
				var child = Node3D.new()
				child.name = "Entity%d" % i
				root.add_child(child)
				child.owner = root

				var script = JSScript.new()
				script.source_code = "function _ready() { console.log('Entity %d'); }" % i
				child.set_script(script)

			var err = sandbox.save_level(root, test_dir)
			root.queue_free()

			if err != OK:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "save_level failed" }

			# Load using sandbox.load_level
			var loaded = sandbox.load_level(test_dir)

			if loaded == null:
				_cleanup_directory(test_dir)
				return { "passed": false, "message": "load_level returned null" }

			# Count nodes with scripts
			var script_count = 0
			for i in range(3):
				var entity = loaded.get_node_or_null("Entity%d" % i)
				if entity and entity.get_script():
					script_count += 1

			loaded.queue_free()
			_cleanup_directory(test_dir)

			return assert_eq(script_count, 3, "All 3 nodes should have scripts reattached")

		"test_load_level_security_blocks_res_path":
			var loaded = sandbox.load_level("res://levels/test/")
			return assert_eq(loaded, null, "Should block res:// path")

		"test_load_level_security_blocks_path_traversal":
			var loaded = sandbox.load_level("user://../etc/")
			return assert_eq(loaded, null, "Should block path traversal")

		"test_load_level_nonexistent_returns_null":
			var loaded = sandbox.load_level("user://nonexistent_level_xyz/")
			return assert_eq(loaded, null, "Should return null for nonexistent level")

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

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

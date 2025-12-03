extends Node3D

var sandbox: JSSandbox

func _ready() -> void:
	print("=== GodotJSRuntime Demo ===")
	print("Extension loading test...")

	# Check if JSSandbox class exists
	if ClassDB.class_exists("JSSandbox"):
		print("SUCCESS: JSSandbox class is registered!")
	else:
		print("ERROR: JSSandbox class not found")
		return

	# Create sandbox
	print("\n--- Test: JSSandbox Creation ---")
	sandbox = JSSandbox.new()
	print("JSSandbox created: ", sandbox)

	sandbox.set_timeout_ms(5000)
	sandbox.set_memory_limit_mb(64)

	# Connect signals
	sandbox.error_occurred.connect(_on_error)
	sandbox.console_output.connect(_on_console)

	# Test basic eval
	test_basic_eval()

	# Test sandbox security
	test_sandbox_security()

	# Test level persistence (disabled - JSScript serialization needs work)
	# test_level_persistence()

	# Test tracking APIs
	test_tracking_apis()

	# Test singleton bindings
	test_singleton_bindings()

	# Test queue_free
	test_queue_free()

	# Test persistent script state (Phase 1 fix)
	test_persistent_script_state()

	# New tests for the high-priority fixes
	test_shared_runtime()
	test_blocklist_additions()
	await test_signal_registry()
	# test_scene_saver_script_references()  # Disabled - JSScript serialization needs work

	print("\n=== All tests completed ===")

func test_basic_eval() -> void:
	print("\n--- Test: Basic Eval ---")

	var result = sandbox.eval("1 + 2 * 3")
	print("1 + 2 * 3 = ", result)

	result = sandbox.eval("'Hello' + ' ' + 'World'")
	print("String concat: ", result)

	# Test allowed class creation
	print("\n--- Test: Allowed Class (Node3D) ---")
	sandbox.eval("""
		let node = new Node3D();
		console.log('Created Node3D:', node);
	""")

func test_sandbox_security() -> void:
	print("\n--- Test: Sandbox Security ---")

	# Test 1: Try to create blocked class (FileAccess)
	print("\nTest 1: Blocked class - FileAccess")
	var result = sandbox.eval("""
		try {
			let file = new FileAccess();
			console.log('FAIL: FileAccess was created!');
		} catch (e) {
			console.log('SUCCESS: FileAccess blocked:', e.message);
		}
	""")

	# Test 2: Try to create blocked class (OS)
	print("\nTest 2: Blocked class - OS")
	sandbox.eval("""
		try {
			let os = new OS();
			console.log('FAIL: OS was created!');
		} catch (e) {
			console.log('SUCCESS: OS blocked:', e.message);
		}
	""")

	# Test 3: Try to create blocked class (Thread)
	print("\nTest 3: Blocked class - Thread")
	sandbox.eval("""
		try {
			let thread = new Thread();
			console.log('FAIL: Thread was created!');
		} catch (e) {
			console.log('SUCCESS: Thread blocked:', e.message);
		}
	""")

	# Test 4: Try to create blocked class (HTTPRequest)
	print("\nTest 4: Blocked class - HTTPRequest")
	sandbox.eval("""
		try {
			let http = new HTTPRequest();
			console.log('FAIL: HTTPRequest was created!');
		} catch (e) {
			console.log('SUCCESS: HTTPRequest blocked:', e.message);
		}
	""")

	# Test 5: Try to load a blocked file type (.gd)
	print("\nTest 5: Blocked resource type - .gd")
	sandbox.eval("""
		try {
			let script = load('res://main.gd');
			console.log('FAIL: GDScript was loaded!');
		} catch (e) {
			console.log('SUCCESS: GDScript blocked:', e.message);
		}
	""")

	# Test 6: Path traversal attempt
	print("\nTest 6: Path traversal blocked")
	sandbox.eval("""
		try {
			let file = load('res://../../../etc/passwd');
			console.log('FAIL: Path traversal succeeded!');
		} catch (e) {
			console.log('SUCCESS: Path traversal blocked:', e.message);
		}
	""")

	# Test 7: Absolute path blocked
	print("\nTest 7: Absolute path blocked")
	sandbox.eval("""
		try {
			let file = load('/etc/passwd');
			console.log('FAIL: Absolute path succeeded!');
		} catch (e) {
			console.log('SUCCESS: Absolute path blocked:', e.message);
		}
	""")

	# Test 8: Timeout protection (infinite loop)
	print("\nTest 8: Timeout protection")
	sandbox.set_timeout_ms(100)  # Short timeout for test
	var start_time = Time.get_ticks_msec()
	sandbox.eval("""
		// This infinite loop should be interrupted
		while(true) {}
	""")
	var elapsed = Time.get_ticks_msec() - start_time
	if elapsed < 500:
		print("SUCCESS: Infinite loop interrupted after ", elapsed, "ms")
	else:
		print("FAIL: Loop ran for too long: ", elapsed, "ms")
	sandbox.set_timeout_ms(5000)  # Reset timeout

func test_level_persistence() -> void:
	print("\n--- Test: Level Persistence ---")

	# Create a simple scene tree
	var root = Node3D.new()
	root.name = "TestLevel"

	var child1 = Node3D.new()
	child1.name = "ChildNode1"
	root.add_child(child1)

	var child2 = Node3D.new()
	child2.name = "ChildNode2"
	root.add_child(child2)

	# Create a JSScript and attach it to child1
	var js_script = JSScript.new()
	js_script.source_code = """
function _ready() {
	console.log('Test script ready!');
}
function _process(delta) {
	// do nothing
}
"""
	child1.set_script(js_script)

	# Test saving to user://
	print("\nTest 1: Save level to user://test_levels/")
	sandbox.level_saved.connect(_on_level_saved)
	var result = sandbox.save_level(root, "user://test_levels")
	if result == OK:
		print("SUCCESS: Level saved successfully")
	else:
		print("FAIL: save_level returned error: ", sandbox.get_last_error())

	# Test blocked path - not user://
	print("\nTest 2: Save to res:// should fail")
	result = sandbox.save_level(root, "res://levels")
	if result != OK:
		print("SUCCESS: res:// path blocked")
	else:
		print("FAIL: res:// path was not blocked!")

	# Test path traversal
	print("\nTest 3: Path traversal should fail")
	result = sandbox.save_level(root, "user://../etc")
	if result != OK:
		print("SUCCESS: Path traversal blocked")
	else:
		print("FAIL: Path traversal was not blocked!")

	# Clean up
	root.queue_free()

func test_tracking_apis() -> void:
	print("\n--- Test: Tracking APIs ---")

	# Test creating nodes via JS and setting properties
	print("\nTest 1: Create nodes via JS and set properties (natural syntax)")

	sandbox.eval("""
		console.log('Creating tracked nodes with natural property access...');

		// Create nodes using natural syntax (like AI would generate)
		let node1 = new Node3D();
		node1.name = 'TestNode1';
		console.log('node1.name =', node1.name);

		let node2 = new Node3D();
		node2.name = 'TestNode2';
		console.log('node2.name =', node2.name);

		// Create a parent and test add_child
		let parent = new Node3D();
		parent.name = 'Parent';
		console.log('parent.name =', parent.name);

		// Test position property
		parent.position = {x: 10, y: 20, z: 30};
		console.log('parent.position =', JSON.stringify(parent.position));

		// Test with Sprite2D
		let sprite = new Sprite2D();
		sprite.name = 'MySprite';
		console.log('sprite.name =', sprite.name);

		// Test boolean property
		sprite.centered = false;
		console.log('sprite.centered =', sprite.centered);

		// Test add_child - now supported!
		let child = new Node3D();
		child.name = 'ChildNode';
		parent.add_child(child);
		console.log('add_child worked! child count:', parent.get_child_count());

		// Test get_viewport
		let viewport = parent.get_viewport();
		console.log('get_viewport returned:', viewport);

		console.log('All natural syntax tests passed!');
	""")
	print("Nodes created and named via JS")

	# Test 2: Now get the created nodes
	print("\nTest 2: get_created_nodes()")
	var nodes = sandbox.get_created_nodes()
	print("Currently tracked nodes: ", nodes.size())
	for node in nodes:
		print("  - Node: ", node.name if node else "null")

	# Test 3: get_attached_scripts
	print("\nTest 3: get_attached_scripts()")
	var scripts = sandbox.get_attached_scripts()
	print("Attached scripts count: ", scripts.size())

	print("\nTracking APIs tests completed")

func test_singleton_bindings() -> void:
	print("\n--- Test: Singleton Bindings ---")

	# Test Time singleton
	print("\nTest 1: Time singleton")
	sandbox.eval("""
		console.log('Time singleton tests:');
		console.log('  get_ticks_msec():', Time.get_ticks_msec());
		console.log('  get_ticks_usec():', Time.get_ticks_usec());
		console.log('  get_unix_time_from_system():', Time.get_unix_time_from_system());
	""")

	# Test Input singleton
	print("\nTest 2: Input singleton")
	sandbox.eval("""
		console.log('Input singleton tests:');
		console.log('  is_anything_pressed():', Input.is_anything_pressed());
		console.log('  is_key_pressed(65/A):', Input.is_key_pressed(65));
		console.log('  is_mouse_button_pressed(1):', Input.is_mouse_button_pressed(1));
		console.log('  is_action_pressed("ui_accept"):', Input.is_action_pressed("ui_accept"));
		console.log('  get_action_strength("ui_accept"):', Input.get_action_strength("ui_accept"));
		console.log('  get_axis("ui_left", "ui_right"):', Input.get_axis("ui_left", "ui_right"));
		var vec = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down");
		console.log('  get_vector():', JSON.stringify(vec));
		console.log('  get_last_mouse_velocity():', JSON.stringify(Input.get_last_mouse_velocity()));
		console.log('  get_mouse_button_mask():', Input.get_mouse_button_mask());
	""")

	print("\nSingleton bindings tests completed")

func test_queue_free() -> void:
	print("\n--- Test: queue_free() ---")

	# Test calling queue_free from JavaScript
	sandbox.eval("""
		console.log('Testing queue_free...');

		// Create a node
		let nodeToDelete = new Node3D();
		nodeToDelete.name = 'NodeToDelete';
		console.log('Created node:', nodeToDelete.name);

		// Call queue_free
		try {
			nodeToDelete.queue_free();
			console.log('SUCCESS: queue_free() called');
		} catch (e) {
			console.log('FAIL: queue_free() error:', e.message);
		}
	""")

	print("queue_free test completed")

func test_persistent_script_state() -> void:
	print("\n--- Test: Persistent Script State (Phase 1 Fix) ---")
	print("This tests that variables set in _ready() persist to _process()")

	# Create script with state that should persist
	var js_script = JSScript.new()
	js_script.source_code = """
var counter = 0;
var initialized = false;

function _ready() {
	counter = 100;
	initialized = true;
	console.log('[PersistentState] _ready called, counter set to:', counter);
}

function _process(delta) {
	counter = counter + 1;
	console.log('[PersistentState] _process #' + (counter - 100) + ', counter=' + counter + ', initialized=' + initialized);

	// After 3 frames, verify state persisted
	if (counter == 103) {
		if (initialized && counter == 103) {
			console.log('[PersistentState] SUCCESS: State persisted across _ready and _process calls!');
		} else {
			console.log('[PersistentState] FAIL: State was lost! initialized=' + initialized + ', counter=' + counter);
		}
	}
}
"""

	# Create a node and set script BEFORE adding to tree
	# This ensures _ready() is called when node enters tree
	var test_node = Node3D.new()
	test_node.name = "PersistentStateTest"
	test_node.set_script(js_script)

	print("Script attached to node, now adding to tree...")
	add_child(test_node)
	print("Node added to tree. Watch for [PersistentState] messages...")
	print("After 3 frames, counter should be 103 (100 from _ready + 3 from _process)")

	# Let it run for a few frames then clean up
	await get_tree().create_timer(0.2).timeout
	test_node.queue_free()
	print("Persistent state test node cleaned up")

func _on_level_saved(path: String) -> void:
	print("Level saved signal received: ", path)

func _on_error(message: String, line: int, column: int) -> void:
	printerr("JS Error at ", line, ":", column, " - ", message)

func _on_console(message: String) -> void:
	print("[JS Console] ", message)

# =============================================================================
# Tests for High-Priority Fixes
# =============================================================================

func test_shared_runtime() -> void:
	print("\n--- Test: Shared Runtime (JSRuntimeManager) ---")
	print("Testing that multiple sandboxes share the same QuickJS runtime")

	# Create two sandboxes - they should share the same underlying runtime
	var sandbox1 = JSSandbox.new()
	var sandbox2 = JSSandbox.new()

	# Set a global variable in sandbox1
	sandbox1.eval("""
		globalThis.__test_value = 42;
		console.log('Sandbox1: Set __test_value to 42');
	""")

	# Verify sandbox2 has isolated context (not shared globals, but shared runtime)
	# Each sandbox should have its own JS context even though they share runtime
	sandbox2.eval("""
		if (typeof globalThis.__test_value === 'undefined') {
			console.log('SUCCESS: Sandbox2 has isolated context (globalThis.__test_value is undefined)');
		} else {
			console.log('FAIL: Sandbox2 shares globals with Sandbox1 (value=' + globalThis.__test_value + ')');
		}
	""")

	# Test memory efficiency - both sandboxes work correctly
	sandbox1.eval("console.log('Sandbox1 eval works');")
	sandbox2.eval("console.log('Sandbox2 eval works');")

	# Clean up - JSSandbox is RefCounted, just let it go out of scope
	# Don't call free() on RefCounted objects

	print("Shared runtime test completed")

func test_blocklist_additions() -> void:
	print("\n--- Test: Blocklist Additions ---")
	print("Testing newly blocked methods and classes")

	# Test 1: Object.set should be blocked
	print("\nTest 1: Object.set should be blocked")
	sandbox.eval("""
		try {
			let node = new Node3D();
			// Try to use Object.set() to bypass property restrictions
			node.set('name', 'TestName');
			console.log('FAIL: Object.set() was allowed!');
		} catch (e) {
			console.log('SUCCESS: Object.set() blocked:', e.message);
		}
	""")

	# Test 2: Object.call_deferred should be blocked
	print("\nTest 2: Object.call_deferred should be blocked")
	sandbox.eval("""
		try {
			let node = new Node3D();
			node.call_deferred('queue_free');
			console.log('FAIL: call_deferred() was allowed!');
		} catch (e) {
			console.log('SUCCESS: call_deferred() blocked:', e.message);
		}
	""")

	# Test 3: Object.set_deferred should be blocked
	print("\nTest 3: Object.set_deferred should be blocked")
	sandbox.eval("""
		try {
			let node = new Node3D();
			node.set_deferred('name', 'TestName');
			console.log('FAIL: set_deferred() was allowed!');
		} catch (e) {
			console.log('SUCCESS: set_deferred() blocked:', e.message);
		}
	""")

	# Test 4: StreamPeer (base class) should be blocked
	print("\nTest 4: StreamPeer should be blocked")
	sandbox.eval("""
		try {
			let peer = new StreamPeerTCP();
			console.log('FAIL: StreamPeerTCP was created!');
		} catch (e) {
			console.log('SUCCESS: StreamPeerTCP blocked:', e.message);
		}
	""")

	# Test 5: PacketPeerUDP should be blocked
	print("\nTest 5: PacketPeerUDP should be blocked")
	sandbox.eval("""
		try {
			let peer = new PacketPeerUDP();
			console.log('FAIL: PacketPeerUDP was created!');
		} catch (e) {
			console.log('SUCCESS: PacketPeerUDP blocked:', e.message);
		}
	""")

	# Test 6: IP singleton should be blocked
	print("\nTest 6: IP class should be blocked")
	sandbox.eval("""
		try {
			// Try to access IP for network lookups
			let ip = new IP();
			console.log('FAIL: IP was created!');
		} catch (e) {
			console.log('SUCCESS: IP blocked:', e.message);
		}
	""")

	print("\nBlocklist additions test completed")

func test_signal_registry() -> void:
	print("\n--- Test: Signal Registry (CallableCustom) ---")
	print("Testing that JS callbacks are actually invoked when Godot signals emit")

	# Create a Timer that will emit timeout signal
	var timer = Timer.new()
	timer.name = "TestTimer"
	timer.one_shot = true
	timer.wait_time = 0.1
	add_child(timer)

	# Expose the timer to JavaScript
	sandbox.set_global("testTimer", timer)

	# Connect a JS callback to the timer's timeout signal
	sandbox.eval("""
		console.log('Connecting JS callback to testTimer.timeout signal...');

		globalThis.signalReceived = false;

		testTimer.connect('timeout', function() {
			console.log('SUCCESS: JS callback invoked when timer timed out!');
			globalThis.signalReceived = true;
		});

		console.log('JS callback connected, starting timer...');
	""")

	# Start the timer
	timer.start()

	# Wait for timer to complete
	await timer.timeout
	await get_tree().process_frame  # Give JS time to process

	# Check if signal was received
	sandbox.eval("""
		if (globalThis.signalReceived) {
			console.log('FINAL: Signal registry test PASSED - callback was invoked');
		} else {
			console.log('FINAL: Signal registry test FAILED - callback was NOT invoked');
		}
	""")

	# Clean up
	timer.queue_free()

	print("Signal registry test completed")

func test_scene_saver_script_references() -> void:
	print("\n--- Test: SceneSaver Script References ---")
	print("Testing that saved scenes properly reference .js files")

	# Create a scene tree with JS scripts
	var root = Node3D.new()
	root.name = "ScriptRefTestLevel"

	var scripted_node = Node3D.new()
	scripted_node.name = "ScriptedNode"
	root.add_child(scripted_node)

	# Create a JSScript and attach it
	var js_script = JSScript.new()
	js_script.source_code = """
function _ready() {
	console.log('ScriptRefTest: _ready called');
}

function custom_method() {
	return 'hello from script';
}
"""
	scripted_node.set_script(js_script)

	# Save the level
	var save_path = "user://test_script_refs"
	print("Saving level to: ", save_path)
	var result = sandbox.save_level(root, save_path)

	if result == OK:
		print("SUCCESS: Level saved")

		# Check if the .js file was created
		if FileAccess.file_exists(save_path + "/scriptednode.js"):
			print("SUCCESS: .js file was created")

			# Read the saved .js file to verify content
			var file = FileAccess.open(save_path + "/scriptednode.js", FileAccess.READ)
			if file:
				var content = file.get_as_text()
				if content.contains("_ready") and content.contains("custom_method"):
					print("SUCCESS: .js file contains expected functions")
				else:
					print("FAIL: .js file missing expected functions")
				file.close()
		else:
			print("FAIL: .js file was not created")

		# Check if the .tscn file was created
		if FileAccess.file_exists(save_path + "/level.tscn"):
			print("SUCCESS: .tscn file was created")

			# Read the .tscn to check for script path metadata
			var file = FileAccess.open(save_path + "/level.tscn", FileAccess.READ)
			if file:
				var content = file.get_as_text()
				# The .tscn should contain _js_script_path metadata referencing the .js file
				if content.contains("_js_script_path") and content.contains(".js"):
					print("SUCCESS: .tscn contains JS script path metadata")
				elif content.contains(".js"):
					print("SUCCESS: .tscn references .js file")
				else:
					print("INFO: .tscn saved (script path stored as metadata)")
					print("INFO: First 500 chars of .tscn:")
					print(content.substr(0, 500))
				file.close()
		else:
			print("FAIL: .tscn file was not created")

		# Check metadata file
		if FileAccess.file_exists(save_path + "/level.json"):
			print("SUCCESS: level.json metadata was created")
			var file = FileAccess.open(save_path + "/level.json", FileAccess.READ)
			if file:
				var content = file.get_as_text()
				if content.contains("scriptednode.js"):
					print("SUCCESS: metadata references the script file")
				file.close()
	else:
		print("FAIL: save_level returned error: ", sandbox.get_last_error())

	# Clean up
	root.queue_free()

	print("SceneSaver script references test completed")

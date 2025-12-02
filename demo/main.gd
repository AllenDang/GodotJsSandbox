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

	# Test level persistence
	test_level_persistence()

	# Test tracking APIs
	test_tracking_apis()

	# Test singleton bindings
	test_singleton_bindings()

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

func _on_level_saved(path: String) -> void:
	print("Level saved signal received: ", path)

func _on_error(message: String, line: int, column: int) -> void:
	printerr("JS Error at ", line, ":", column, " - ", message)

func _on_console(message: String) -> void:
	print("[JS Console] ", message)

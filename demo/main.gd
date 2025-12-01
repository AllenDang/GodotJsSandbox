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

	# Check if JSScript class exists
	if ClassDB.class_exists("JSScript"):
		print("SUCCESS: JSScript class is registered!")
	else:
		print("ERROR: JSScript class not found")

	# Create sandbox and test basic eval
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

	# Test JSScript loading
	test_js_script_loading()

	print("\n=== All tests completed ===")

func test_basic_eval() -> void:
	print("\n--- Test: Basic Eval ---")

	var result = sandbox.eval("1 + 2 * 3")
	print("1 + 2 * 3 = ", result)

	result = sandbox.eval("'Hello' + ' ' + 'World'")
	print("String concat: ", result)

	result = sandbox.eval("Math.sqrt(16)")
	print("Math.sqrt(16) = ", result)

	# Test console
	print("\n--- Test: Console ---")
	sandbox.eval("console.log('Hello from JavaScript!')")

func test_js_script_loading() -> void:
	print("\n--- Test: JSScript Resource Loading ---")

	# Try to load the JS file as a resource
	var script = load("res://attached_script.js")
	if script:
		print("SUCCESS: Loaded JSScript resource: ", script)
		print("Script class: ", script.get_class())

		# Create a node and attach the script
		var test_node = Node3D.new()
		test_node.name = "JSScriptedNode"
		test_node.set_script(script)

		# Add to tree (this should trigger _ready)
		$LevelRoot.add_child(test_node)
		print("Added node with JS script to scene tree")
	else:
		print("SKIPPED: JSScript resource loading not working yet")

func _on_error(message: String, line: int, column: int) -> void:
	printerr("JS Error at ", line, ":", column, " - ", message)

func _on_console(message: String) -> void:
	print("[JS Console] ", message)

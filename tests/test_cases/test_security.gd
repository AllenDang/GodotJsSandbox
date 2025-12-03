class_name TestSecurity
extends TestBase

## Comprehensive security tests based on PRD.md blocklist
## Reference: PRD.md Section 1 "精准黑名单", TDD.md Section 6.4 "SandboxConfig"
##
## Blocked Classes:
## - System: OS, FileAccess, DirAccess
## - Threading: Thread, Mutex, Semaphore, WorkerThreadPool
## - Network: TCPServer, UDPServer, HTTPClient, HTTPRequest, WebSocketPeer, ENetConnection, ENetMultiplayerPeer
## - Scripts: JavaScriptBridge, Expression, GDScript, CSharpScript
## - Extensions: NativeExtension, GDExtensionManager
## - Resources: ResourceLoader, ResourceSaver
##
## Blocked Methods:
## - Object: call, callv, set, get_script
## - ClassDB: instantiate, instance, can_instantiate, get_class_list
## - Engine: get_singleton, register_singleton, unregister_singleton
##
## Note: set_script is ALLOWED (with JSScript only, validated by SafeWrapper)

func get_suite_name() -> String:
	return "Security"

func get_tests() -> Array[String]:
	return [
		# === Blocked Classes - System ===
		"test_blocked_class_os",
		"test_blocked_class_file_access",
		"test_blocked_class_dir_access",

		# === Blocked Classes - Threading ===
		"test_blocked_class_thread",
		"test_blocked_class_mutex",
		"test_blocked_class_semaphore",
		"test_blocked_class_worker_thread_pool",

		# === Blocked Classes - Network ===
		"test_blocked_class_tcp_server",
		"test_blocked_class_udp_server",
		"test_blocked_class_http_client",
		"test_blocked_class_http_request",
		"test_blocked_class_websocket_peer",
		"test_blocked_class_enet_connection",
		"test_blocked_class_enet_multiplayer_peer",

		# === Blocked Classes - Scripts ===
		"test_blocked_class_javascript_bridge",
		"test_blocked_class_expression",
		"test_blocked_class_gdscript",
		"test_blocked_class_csharp_script",

		# === Blocked Classes - Extensions ===
		"test_blocked_class_native_extension",
		"test_blocked_class_gdextension_manager",

		# === Blocked Classes - Resources ===
		"test_blocked_class_resource_loader",
		"test_blocked_class_resource_saver",

		# === Blocked Methods - Object ===
		"test_blocked_method_call",
		"test_blocked_method_callv",
		"test_blocked_method_set",
		"test_blocked_method_get_script",

		# === Blocked Methods - ClassDB ===
		"test_blocked_classdb_instantiate",
		"test_blocked_classdb_can_instantiate",
		"test_blocked_classdb_get_class_list",

		# === Blocked Methods - Engine ===
		"test_blocked_engine_get_singleton",
		"test_blocked_engine_register_singleton",

		# === Blocked Methods - Dangerous ===
		"test_blocked_method_free",
		"test_blocked_method_call_deferred",
		"test_blocked_method_set_deferred",

		# === JS Sandbox Restrictions ===
		"test_no_eval_function",
		"test_no_function_constructor",

		# === Allowed Operations (should work) ===
		"test_allowed_queue_free",
		"test_allowed_node_operations",
		"test_allowed_property_access",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		# === Blocked Classes - System ===
		"test_blocked_class_os":
			return assert_throws("OS;", "OS should not be accessible")

		"test_blocked_class_file_access":
			return assert_throws("new FileAccess();", "FileAccess should be blocked")

		"test_blocked_class_dir_access":
			return assert_throws("new DirAccess();", "DirAccess should be blocked")

		# === Blocked Classes - Threading ===
		"test_blocked_class_thread":
			return assert_throws("new Thread();", "Thread should be blocked")

		"test_blocked_class_mutex":
			return assert_throws("new Mutex();", "Mutex should be blocked")

		"test_blocked_class_semaphore":
			return assert_throws("new Semaphore();", "Semaphore should be blocked")

		"test_blocked_class_worker_thread_pool":
			return assert_throws("WorkerThreadPool;", "WorkerThreadPool should not be accessible")

		# === Blocked Classes - Network ===
		"test_blocked_class_tcp_server":
			return assert_throws("new TCPServer();", "TCPServer should be blocked")

		"test_blocked_class_udp_server":
			return assert_throws("new UDPServer();", "UDPServer should be blocked")

		"test_blocked_class_http_client":
			return assert_throws("new HTTPClient();", "HTTPClient should be blocked")

		"test_blocked_class_http_request":
			return assert_throws("new HTTPRequest();", "HTTPRequest should be blocked")

		"test_blocked_class_websocket_peer":
			return assert_throws("new WebSocketPeer();", "WebSocketPeer should be blocked")

		"test_blocked_class_enet_connection":
			return assert_throws("new ENetConnection();", "ENetConnection should be blocked")

		"test_blocked_class_enet_multiplayer_peer":
			return assert_throws("new ENetMultiplayerPeer();", "ENetMultiplayerPeer should be blocked")

		# === Blocked Classes - Scripts ===
		"test_blocked_class_javascript_bridge":
			return assert_throws("JavaScriptBridge;", "JavaScriptBridge should not be accessible")

		"test_blocked_class_expression":
			return assert_throws("new Expression();", "Expression should be blocked")

		"test_blocked_class_gdscript":
			return assert_throws("new GDScript();", "GDScript should be blocked")

		"test_blocked_class_csharp_script":
			return assert_throws("new CSharpScript();", "CSharpScript should be blocked")

		# === Blocked Classes - Extensions ===
		"test_blocked_class_native_extension":
			return assert_throws("new NativeExtension();", "NativeExtension should be blocked")

		"test_blocked_class_gdextension_manager":
			return assert_throws("GDExtensionManager;", "GDExtensionManager should not be accessible")

		# === Blocked Classes - Resources ===
		"test_blocked_class_resource_loader":
			return assert_throws("ResourceLoader;", "ResourceLoader should not be accessible")

		"test_blocked_class_resource_saver":
			return assert_throws("ResourceSaver;", "ResourceSaver should not be accessible")

		# === Blocked Methods - Object ===
		"test_blocked_method_call":
			var code = """
				var node = new Node();
				node.call('get_child_count');
			"""
			return assert_throws(code, "call() should be blocked")

		"test_blocked_method_callv":
			var code = """
				var node = new Node();
				node.callv('get_child_count', []);
			"""
			return assert_throws(code, "callv() should be blocked")

		"test_blocked_method_set":
			var code = """
				var node = new Node();
				node.set('name', 'test');
			"""
			return assert_throws(code, "set() should be blocked")

		"test_blocked_method_get_script":
			var code = """
				var node = new Node();
				node.get_script();
			"""
			return assert_throws(code, "get_script() should be blocked")

		# === Blocked Methods - ClassDB ===
		"test_blocked_classdb_instantiate":
			var code = """
				ClassDB.instantiate('Node');
			"""
			return assert_throws(code, "ClassDB.instantiate should be blocked")

		"test_blocked_classdb_can_instantiate":
			var code = """
				ClassDB.can_instantiate('Node');
			"""
			return assert_throws(code, "ClassDB.can_instantiate should be blocked")

		"test_blocked_classdb_get_class_list":
			var code = """
				ClassDB.get_class_list();
			"""
			return assert_throws(code, "ClassDB.get_class_list should be blocked")

		# === Blocked Methods - Engine ===
		"test_blocked_engine_get_singleton":
			var code = """
				Engine.get_singleton('OS');
			"""
			return assert_throws(code, "Engine.get_singleton should be blocked")

		"test_blocked_engine_register_singleton":
			var code = """
				Engine.register_singleton('Test', new Node());
			"""
			return assert_throws(code, "Engine.register_singleton should be blocked")

		# === Blocked Methods - Dangerous ===
		"test_blocked_method_free":
			var code = """
				var node = new Node();
				node.free();
			"""
			return assert_throws(code, "free() should be blocked (use queue_free)")

		"test_blocked_method_call_deferred":
			var code = """
				var node = new Node();
				node.call_deferred('queue_free');
			"""
			return assert_throws(code, "call_deferred should be blocked")

		"test_blocked_method_set_deferred":
			var code = """
				var node = new Node();
				node.set_deferred('name', 'test');
			"""
			return assert_throws(code, "set_deferred should be blocked")

		# === JS Sandbox Restrictions ===
		"test_no_eval_function":
			return assert_throws("eval('1 + 1')", "eval() should not be available")

		"test_no_function_constructor":
			return assert_throws("new Function('return 1')()", "Function constructor should be blocked")

		# === Allowed Operations ===
		"test_allowed_queue_free":
			# queue_free IS allowed (PRD explicitly states this)
			var code = """
				var node = new Node();
				typeof node.queue_free === 'function';
			"""
			return assert_eval(code, true)

		"test_allowed_node_operations":
			# Normal node operations should work
			var code = """
				var parent = new Node();
				var child = new Node();
				parent.add_child(child);
				parent.get_child_count() === 1;
			"""
			return assert_eval(code, true)

		"test_allowed_property_access":
			# Direct property access should work (not via set() method)
			var code = """
				var node = new Node();
				node.name = 'TestNode';
				node.name === 'TestNode';
			"""
			return assert_eval(code, true)

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	sandbox.set_timeout_ms(5000)
	super.teardown()

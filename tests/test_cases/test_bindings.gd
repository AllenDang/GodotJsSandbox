class_name TestBindings
extends TestBase

func get_suite_name() -> String:
	return "Bindings"

func get_tests() -> Array[String]:
	return [
		"test_node_create",
		"test_node2d_create",
		"test_node3d_create",
		"test_node3d_position",
		"test_node2d_position",
		"test_node2d_rotation",
		"test_sprite2d_properties",
		"test_label_text",
		"test_timer_properties",
		"test_control_properties",
		"test_method_add_child",
		"test_method_get_child",
		"test_method_get_child_count",
		"test_method_get_parent",
		"test_method_get_node",
		"test_method_set_name",
		"test_method_get_name",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_node_create":
			var code = """
				var node = new Node();
				var result = node !== null && typeof node.__handle === 'number';
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_node2d_create":
			var code = """
				var node = new Node2D();
				var result = node !== null && typeof node.__handle === 'number';
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_node3d_create":
			var code = """
				var node = new Node3D();
				var result = node !== null && typeof node.__handle === 'number';
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_node3d_position":
			var code = """
				var node = new Node3D();
				node.position = {x: 1, y: 2, z: 3};
				var pos = node.position;
				var result = pos.x === 1 && pos.y === 2 && pos.z === 3;
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_node2d_position":
			var code = """
				var node = new Node2D();
				node.position = {x: 100, y: 200};
				var pos = node.position;
				var result = pos.x === 100 && pos.y === 200;
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_node2d_rotation":
			var code = """
				var node = new Node2D();
				node.rotation = 1.5;
				var result = Math.abs(node.rotation - 1.5) < 0.001;
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_sprite2d_properties":
			var code = """
				var sprite = new Sprite2D();
				sprite.centered = false;
				sprite.flip_h = true;
				var result = !sprite.centered && sprite.flip_h;
				sprite.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_label_text":
			var code = """
				var label = new Label();
				label.text = 'Hello World';
				var result = label.text === 'Hello World';
				label.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_timer_properties":
			var code = """
				var timer = new Timer();
				timer.wait_time = 2.5;
				timer.one_shot = true;
				var result = timer.wait_time === 2.5 && timer.one_shot === true;
				timer.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_control_properties":
			var code = """
				var ctrl = new Control();
				ctrl.custom_minimum_size = {x: 100, y: 50};
				var size = ctrl.custom_minimum_size;
				var result = size.x === 100 && size.y === 50;
				ctrl.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_method_add_child":
			var code = """
				var parent = new Node();
				var child = new Node();
				parent.add_child(child);
				var result = parent.get_child_count() === 1;
				parent.queue_free();  // Also frees children
				result;
			"""
			return assert_eval(code, true)
		"test_method_get_child":
			# get_child should return a fully wrapped proxy with .name property
			var code = """
				var parent = new Node();
				var child = new Node();
				child.name = 'TestChild';
				parent.add_child(child);
				var retrieved = parent.get_child(0);
				var result = retrieved.name === 'TestChild';
				parent.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_method_get_child_count":
			var code = """
				var parent = new Node();
				parent.add_child(new Node());
				parent.add_child(new Node());
				parent.add_child(new Node());
				var result = parent.get_child_count() === 3;
				parent.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_method_get_parent":
			# get_parent should return a fully wrapped proxy with .name property
			var code = """
				var parent = new Node();
				parent.name = 'ParentNode';
				var child = new Node();
				parent.add_child(child);
				var p = child.get_parent();
				var result = p.name === 'ParentNode';
				parent.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_method_get_node":
			# get_node should return a fully wrapped proxy with .name property
			var code = """
				var parent = new Node();
				var child = new Node();
				child.name = 'MyChild';
				parent.add_child(child);
				var found = parent.get_node('MyChild');
				var result = found !== null && found.name === 'MyChild';
				parent.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_method_set_name":
			var code = """
				var node = new Node();
				node.name = 'TestName';
				var result = node.name === 'TestName';
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		"test_method_get_name":
			# get_name() method should work
			var code = """
				var node = new Node();
				node.name = 'GetNameTest';
				var result = node.get_name() === 'GetNameTest';
				node.queue_free();
				result;
			"""
			return assert_eval(code, true)
		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	# Clean up JS-created nodes
	sandbox.eval("if (typeof __cleanup_created_nodes === 'function') __cleanup_created_nodes();")
	super.teardown()

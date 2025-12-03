class_name TestAnimation
extends TestBase

## Tests for AnimationPlayer bindings

func get_suite_name() -> String:
	return "Animation"

func get_tests() -> Array[String]:
	return [
		# AnimationPlayer creation
		"test_animation_player_create",
		"test_animation_player_play_method",
		"test_animation_player_stop_method",
		"test_animation_player_properties",
		"test_animation_player_current_animation",
		"test_animation_player_speed_scale",
		"test_animation_player_playback_active",
		# AnimationTree (basic)
		"test_animation_tree_create",
		"test_animation_tree_active",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_animation_player_create":
			var result = sandbox.eval("""
				var player = new AnimationPlayer();
				player.name = 'MyAnimPlayer';
				player.name;
			""")
			return assert_eq(result, "MyAnimPlayer")

		"test_animation_player_play_method":
			var result = sandbox.eval("""
				var player = new AnimationPlayer();
				typeof player.play;
			""")
			return assert_eq(result, "function")

		"test_animation_player_stop_method":
			var result = sandbox.eval("""
				var player = new AnimationPlayer();
				var methods = [
					typeof player.stop,
					typeof player.pause,
					typeof player.seek
				];
				methods.join(',');
			""")
			return assert_eq(result, "function,function,function")

		"test_animation_player_properties":
			# Test basic properties exist
			var result = sandbox.eval("""
				var player = new AnimationPlayer();
				// These should be accessible (even if they return defaults)
				var hasSpeedScale = 'speed_scale' in player || typeof player.speed_scale !== 'undefined';
				hasSpeedScale;
			""")
			return assert_eq(result, true)

		"test_animation_player_current_animation":
			var result = sandbox.eval("""
				var player = new AnimationPlayer();
				// current_animation should be empty string by default
				var ca = player.current_animation;
				ca === '' || ca === null || ca === undefined;
			""")
			return assert_eq(result, true)

		"test_animation_player_speed_scale":
			var result = sandbox.eval("""
				var player = new AnimationPlayer();
				player.speed_scale = 2.0;
				player.speed_scale;
			""")
			if typeof(result) == TYPE_FLOAT or typeof(result) == TYPE_INT:
				return assert_eq(abs(result - 2.0) < 0.01, true, "Speed scale should be ~2.0")
			return { "passed": false, "message": "Speed scale should be a number" }

		"test_animation_player_playback_active":
			# Test is_playing method exists
			var result = sandbox.eval("""
				var player = new AnimationPlayer();
				typeof player.is_playing;
			""")
			return assert_eq(result, "function")

		"test_animation_tree_create":
			var result = sandbox.eval("""
				var tree = new AnimationTree();
				tree.name = 'MyAnimTree';
				tree.name;
			""")
			return assert_eq(result, "MyAnimTree")

		"test_animation_tree_active":
			var result = sandbox.eval("""
				var tree = new AnimationTree();
				tree.active = true;
				tree.active;
			""")
			return assert_eq(result, true)

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

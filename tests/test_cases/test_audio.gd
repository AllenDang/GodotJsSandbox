class_name TestAudio
extends TestBase

## Tests for AudioStreamPlayer bindings

func get_suite_name() -> String:
	return "Audio"

func get_tests() -> Array[String]:
	return [
		# AudioStreamPlayer (non-positional)
		"test_audio_stream_player_create",
		"test_audio_stream_player_play_method",
		"test_audio_stream_player_stop_method",
		"test_audio_stream_player_volume",
		"test_audio_stream_player_pitch_scale",
		"test_audio_stream_player_autoplay",
		"test_audio_stream_player_is_playing",
		# AudioStreamPlayer2D
		"test_audio_stream_player2d_create",
		"test_audio_stream_player2d_position",
		"test_audio_stream_player2d_max_distance",
		# AudioStreamPlayer3D
		"test_audio_stream_player3d_create",
		"test_audio_stream_player3d_position",
		"test_audio_stream_player3d_unit_size",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		# ===== AudioStreamPlayer =====
		"test_audio_stream_player_create":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer();
				player.name = 'BGMusic';
				player.name;
			""")
			return assert_eq(result, "BGMusic")

		"test_audio_stream_player_play_method":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer();
				typeof player.play;
			""")
			return assert_eq(result, "function")

		"test_audio_stream_player_stop_method":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer();
				typeof player.stop;
			""")
			return assert_eq(result, "function")

		"test_audio_stream_player_volume":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer();
				player.volume_db = -10.0;
				player.volume_db;
			""")
			if typeof(result) == TYPE_FLOAT or typeof(result) == TYPE_INT:
				return assert_eq(abs(result - (-10.0)) < 0.01, true, "Volume should be ~-10.0 dB")
			return { "passed": false, "message": "Volume should be a number" }

		"test_audio_stream_player_pitch_scale":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer();
				player.pitch_scale = 1.5;
				player.pitch_scale;
			""")
			if typeof(result) == TYPE_FLOAT or typeof(result) == TYPE_INT:
				return assert_eq(abs(result - 1.5) < 0.01, true, "Pitch scale should be ~1.5")
			return { "passed": false, "message": "Pitch scale should be a number" }

		"test_audio_stream_player_autoplay":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer();
				player.autoplay = true;
				player.autoplay;
			""")
			return assert_eq(result, true)

		"test_audio_stream_player_is_playing":
			# is_playing is exposed as a property called 'playing'
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer();
				// Check playing property exists and returns false by default
				player.playing === false;
			""")
			return assert_eq(result, true)

		# ===== AudioStreamPlayer2D =====
		"test_audio_stream_player2d_create":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer2D();
				player.name = 'SFX2D';
				player.name;
			""")
			return assert_eq(result, "SFX2D")

		"test_audio_stream_player2d_position":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer2D();
				player.position = { x: 100, y: 200 };
				var pos = player.position;
				pos.x + ',' + pos.y;
			""")
			return assert_eq(result, "100,200")

		"test_audio_stream_player2d_max_distance":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer2D();
				player.max_distance = 500.0;
				player.max_distance;
			""")
			if typeof(result) == TYPE_FLOAT or typeof(result) == TYPE_INT:
				return assert_eq(abs(result - 500.0) < 0.01, true, "Max distance should be ~500.0")
			return { "passed": false, "message": "Max distance should be a number" }

		# ===== AudioStreamPlayer3D =====
		"test_audio_stream_player3d_create":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer3D();
				player.name = 'SFX3D';
				player.name;
			""")
			return assert_eq(result, "SFX3D")

		"test_audio_stream_player3d_position":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer3D();
				player.position = { x: 1, y: 2, z: 3 };
				var pos = player.position;
				pos.x + ',' + pos.y + ',' + pos.z;
			""")
			return assert_eq(result, "1,2,3")

		"test_audio_stream_player3d_unit_size":
			var result = sandbox.eval("""
				var player = new AudioStreamPlayer3D();
				player.unit_size = 15.0;
				player.unit_size;
			""")
			if typeof(result) == TYPE_FLOAT or typeof(result) == TYPE_INT:
				return assert_eq(abs(result - 15.0) < 0.01, true, "Unit size should be ~15.0")
			return { "passed": false, "message": "Unit size should be a number" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

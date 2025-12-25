class_name TestUtf8Encoding
extends TestBase

## Tests for UTF-8 encoding support
## Ensures Chinese and other Unicode characters are handled correctly
## through the JavaScript runtime.

func get_suite_name() -> String:
	return "UTF-8 Encoding"

func get_tests() -> Array[String]:
	return [
		# String literal tests
		"test_chinese_string_literal",
		"test_chinese_string_concatenation",
		"test_mixed_language_string",
		"test_emoji_string",
		# JSON tests
		"test_json_parse_chinese",
		"test_json_stringify_chinese",
		# Return value tests
		"test_chinese_in_array",
		"test_chinese_in_object",
		# Template literal tests
		"test_template_literal_chinese",
		# Multi-byte character tests
		"test_japanese_string",
		"test_korean_string",
		"test_russian_string",
		# Error message tests
		"test_error_message_chinese",
	]

func run_test(test_name: String) -> Dictionary:
	match test_name:
		"test_chinese_string_literal":
			# Basic Chinese string literal
			return assert_eval("'你好世界'", "你好世界")

		"test_chinese_string_concatenation":
			# Concatenating Chinese strings
			return assert_eval("'你好' + '世界'", "你好世界")

		"test_mixed_language_string":
			# Mixed English and Chinese
			return assert_eval("'Hello 世界! 你好 World!'", "Hello 世界! 你好 World!")

		"test_emoji_string":
			# Emoji characters (also UTF-8)
			return assert_eval("'Hello 🎮 World 🎯'", "Hello 🎮 World 🎯")

		"test_json_parse_chinese":
			# JSON.parse with Chinese content
			var result = sandbox.eval('JSON.parse(\'{"name":"玩家","message":"你好"}\')')
			if typeof(result) != TYPE_DICTIONARY:
				return { "passed": false, "message": "Expected dictionary, got: " + str(typeof(result)) }
			if result.get("name") != "玩家":
				return { "passed": false, "message": "Expected name='玩家', got: " + str(result.get("name")) }
			if result.get("message") != "你好":
				return { "passed": false, "message": "Expected message='你好', got: " + str(result.get("message")) }
			return { "passed": true, "message": "" }

		"test_json_stringify_chinese":
			# JSON.stringify with Chinese content
			var result = sandbox.eval('JSON.stringify({name: "敌人", health: 100})')
			# The result should contain the Chinese characters properly
			if typeof(result) != TYPE_STRING:
				return { "passed": false, "message": "Expected string, got: " + str(typeof(result)) }
			if result.find("敌人") == -1:
				return { "passed": false, "message": "Chinese characters not found in JSON output: " + result }
			return { "passed": true, "message": "" }

		"test_chinese_in_array":
			# Array with Chinese strings
			var code = """
				var arr = ['苹果', '香蕉', '橙子'];
				arr[1];
			"""
			return assert_eval(code, "香蕉")

		"test_chinese_in_object":
			# Object with Chinese values returned
			var code = """
				var obj = { fruit: '苹果', color: '红色' };
				obj.fruit + ' - ' + obj.color;
			"""
			return assert_eval(code, "苹果 - 红色")

		"test_template_literal_chinese":
			# Template literals with Chinese
			var code = """
				var name = '玩家';
				var action = '攻击';
				`${name}使用了${action}!`;
			"""
			return assert_eval(code, "玩家使用了攻击!")

		"test_japanese_string":
			# Japanese characters (Hiragana, Katakana, Kanji)
			return assert_eval("'こんにちは世界'", "こんにちは世界")

		"test_korean_string":
			# Korean characters (Hangul)
			return assert_eval("'안녕하세요'", "안녕하세요")

		"test_russian_string":
			# Russian/Cyrillic characters
			return assert_eval("'Привет мир'", "Привет мир")

		"test_error_message_chinese":
			# Error messages should preserve Chinese characters
			# Use throw with string directly since Error constructor may not be available
			var code = """
				try {
					throw '错误信息：无效操作';
				} catch (e) {
					e;
				}
			"""
			var result = sandbox.eval(code)
			if typeof(result) != TYPE_STRING:
				return { "passed": false, "message": "Expected string, got type: " + str(typeof(result)) }
			if result.find("错误信息") == -1:
				return { "passed": false, "message": "Chinese not preserved in thrown string: " + result }
			return { "passed": true, "message": "" }

		_:
			return { "passed": false, "message": "Unknown test: " + test_name }

func teardown() -> void:
	super.teardown()

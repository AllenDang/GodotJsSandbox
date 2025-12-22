extends Node

var test_value: int = 42
var test_string: String = "Hello from autoload"

func get_doubled_value() -> int:
	return test_value * 2

func greet(name: String) -> String:
	return "Hello, " + name + "!"

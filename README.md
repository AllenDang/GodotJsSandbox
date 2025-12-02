# GodotJSRuntime - Sandbox

A secure JavaScript/TypeScript runtime sandbox for Godot 4.x, designed for AI-assisted game level generation.

## Overview

GodotJSRuntime provides a sandboxed JavaScript execution environment that allows players to generate complete game levels, gameplay logic, and behaviors at runtime through AI assistance.

### Core Features

- **Secure Sandbox**: Blocks dangerous APIs (file system, network, threading)
- **Godot Integration**: JS scripts can attach to nodes like GDScript
- **Level Persistence**: Save generated levels as .tscn + .js files
- **Memory Safety**: Object lifecycle management prevents crashes

## Building

This is a GDExtension plugin - no need to rebuild Godot from source.

### Prerequisites

- Godot 4.2+
- SCons build system
- C++ compiler (MSVC, GCC, or Clang)
- Python 3.6+

### Build Steps

1. Clone this repository:
   ```bash
   git clone <repo-url> GodotJSRuntime
   cd GodotJSRuntime
   ```

2. Clone godot-cpp (matching your Godot version):
   ```bash
   git clone https://github.com/godotengine/godot-cpp.git
   cd godot-cpp
   git checkout 4.2  # or 4.3, etc.
   git submodule update --init
   cd ..
   ```

3. Build the extension:
   ```bash
   # Debug build
   scons platform=<your_platform> target=template_debug

   # Release build
   scons platform=<your_platform> target=template_release
   ```

   Supported platforms: `macos`, `windows`, `linux`, `android`

4. Copy the `demo/bin/` folder to your Godot project.

### Using in Your Project

1. Copy the `bin/` folder (containing the `.gdextension` file and built libraries) to your project.

2. Enable the extension in Project Settings if needed.

3. The `JSSandbox` class will be available for use.

## Usage

### Basic Evaluation

```gdscript
var sandbox = JSSandbox.new()
sandbox.set_timeout_ms(1000)  # 1 second timeout
sandbox.set_memory_limit_mb(64)  # 64 MB memory limit

# Execute JavaScript
var result = sandbox.eval("1 + 2")
print(result)  # 3

# Set global variables accessible from JS
sandbox.set_global("scene", $LevelRoot)
sandbox.set_global("player", $Player)

# Execute more complex code
sandbox.eval("""
    console.log('Hello from JavaScript!');
    let sum = 0;
    for (let i = 0; i < 10; i++) {
        sum += i;
    }
    sum;  // Return value
""")
```

### Level Generation Example

```gdscript
var sandbox = JSSandbox.new()
sandbox.set_global("scene", $LevelRoot)

# AI-generated code
var ai_code = """
    // Create a platform
    let platform = new StaticBody3D();
    platform.position = new Vector3(0, 5, 0);
    scene.add_child(platform);

    // Add collision shape
    let collision = new CollisionShape3D();
    let shape = new BoxShape3D();
    shape.size = new Vector3(10, 1, 10);
    collision.shape = shape;
    platform.add_child(collision);
"""

sandbox.eval(ai_code)

# Save the level when player is satisfied
sandbox.save_level("user://levels/my_level/")
```

### Loading Saved Levels

```gdscript
# Load a previously saved level
var level = load("user://levels/my_level/level.tscn")
var instance = level.instantiate()
add_child(instance)
# JS scripts automatically execute
```

## Security Model

### Blacklisted Classes

The following classes are blocked from JS access:

- **System**: `OS`, `FileAccess`, `DirAccess`
- **Threading**: `Thread`, `Mutex`, `Semaphore`, `WorkerThreadPool`
- **Network**: `TCPServer`, `HTTPClient`, `HTTPRequest`, `WebSocketPeer`
- **Script Execution**: `Expression`, `GDScript`, `CSharpScript`
- **Extensions**: `NativeExtension`, `GDExtensionManager`

### Blocked Methods

- `Object.call`, `Object.callv`, `Object.set_script`
- `ClassDB.instantiate`, `Engine.get_singleton`

### Rate Limiting

- Write operations: 500/frame
- Heavy operations (instantiate, queue_free): 50/frame

### Path Restrictions

- Only `res://` and `user://` paths allowed
- Path traversal (`..`) blocked

## Project Structure

```
GodotJSRuntime/
├── quickjs/            # QuickJS JavaScript engine
├── src/                # GDExtension source files
│   ├── register_types     # GDExtension entry point
│   ├── js_sandbox         # Main JSSandbox class
│   ├── quickjs_context    # QuickJS wrapper
│   ├── object_registry    # Object lifecycle management
│   ├── sandbox_config     # Blacklist configuration
│   └── execution_limiter  # Timeout/rate limiting
├── demo/               # Example Godot project
│   └── bin/            # Built extension files
├── godot-cpp/          # godot-cpp bindings (clone separately)
└── SConstruct          # Build configuration
```

## Configuration

Create a `blocklist.json` file to customize blocked classes/methods:

```json
{
    "blocked_classes": ["OS", "FileAccess", "CustomDangerousClass"],
    "blocked_methods": ["Object.call", "Object.callv", "ClassDB.instantiate"],
    "blocked_properties": ["Engine.physics_ticks_per_second"],
    "allowed_paths": ["res://", "user://"]
}
```

Load it with:
```gdscript
sandbox.load_blocklist("res://config/blocklist.json")
```

## API Reference

### JSSandbox

| Method | Description |
|--------|-------------|
| `eval(code: String) -> Variant` | Execute JavaScript code and return result |
| `eval_file(path: String) -> Variant` | Execute JavaScript from a file |
| `set_global(name: String, value: Variant)` | Set a global variable in JS |
| `get_global(name: String) -> Variant` | Get a global variable from JS |
| `set_timeout_ms(ms: int)` | Set execution timeout in milliseconds |
| `set_memory_limit_mb(mb: int)` | Set memory limit in megabytes |
| `load_blocklist(path: String) -> Error` | Load custom blocklist configuration |
| `get_last_error() -> String` | Get the last error message |
| `is_valid() -> bool` | Check if sandbox is initialized |
| `reset()` | Reset the sandbox to initial state |

### Signals

| Signal | Description |
|--------|-------------|
| `error_occurred(message: String, line: int, column: int)` | Emitted on JS error |
| `console_output(message: String)` | Emitted on console.log output |
| `level_saved(path: String)` | Emitted when level is saved |

## License

MIT License - see LICENSE file

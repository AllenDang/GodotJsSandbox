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
sandbox.save_level($LevelRoot, "user://levels/my_level/")
```

### Loading Scenes with Sandbox

The sandbox provides both synchronous and asynchronous scene loading that automatically attaches JS scripts to nodes.

#### Synchronous Loading

```gdscript
var sandbox = JSSandbox.new()

# Load a scene synchronously
var scene = sandbox.load_scene("user://games/my_game/main.tscn")
add_child(scene)
# JS scripts automatically execute on _ready
```

#### Asynchronous Loading (Recommended)

For larger scenes, use async loading to avoid blocking the main thread:

```gdscript
var sandbox = JSSandbox.new()
var loader: AsyncSceneLoader = null

func load_game(scene_path: String) -> void:
    loader = sandbox.load_scene_async(scene_path)
    loader.progress_changed.connect(_on_loading_progress)
    loader.completed.connect(_on_loading_completed)
    loader.failed.connect(_on_loading_failed)

func _process(delta: float) -> void:
    # Poll the loader each frame
    if loader and loader.is_loading():
        loader.poll()

func _on_loading_progress(progress: float, stage: String) -> void:
    print("Loading: %.0f%% - %s" % [progress * 100, stage])

func _on_loading_completed(scene: Node) -> void:
    add_child(scene)
    loader = null
    print("Scene loaded!")

func _on_loading_failed(error: String) -> void:
    print("Failed to load: ", error)
    loader = null
```

### Capturing Error Logs

The sandbox provides comprehensive error capture for both JavaScript and Godot engine errors, designed for AI-assisted development workflows.

#### Phase-Based Error Handling (Recommended for AI Workflows)

```gdscript
var sandbox = JSSandbox.new()

func _ready() -> void:
    # Phase completion signals - for auto-fix loops
    sandbox.load_completed.connect(_on_load_completed)
    sandbox.init_completed.connect(_on_init_completed)

    # Runtime error signals - for user-triggered errors
    sandbox.runtime_error.connect(_on_runtime_error)
    sandbox.console_output.connect(_on_js_console)

func load_ai_code(code: String) -> void:
    sandbox.clear_errors()
    sandbox.eval_module(code, "game.js")
    # load_completed signal emitted automatically

func _on_load_completed(success: bool, errors: Array) -> void:
    if not success:
        # Auto-fix loop: send errors back to AI
        var report = sandbox.get_error_report()
        print(report)  # Markdown-formatted for AI
        # ai_agent.fix_code(report)
    else:
        # Start init phase to catch _ready() errors
        sandbox.start_init_phase(0.5)

func _on_init_completed(success: bool, errors: Array) -> void:
    if not success:
        var report = sandbox.get_error_report()
        # ai_agent.fix_code(report)
    else:
        print("Ready to play!")

func _on_runtime_error(error: Dictionary) -> void:
    # Show in dev panel - user can click "Ask AI to fix"
    print("Runtime error: %s at %s:%d" % [error.message, error.file, error.line])

func _on_js_console(message: String) -> void:
    print("JS Console: ", message)
```

#### Error Dictionary Structure

Each error is a Dictionary with these fields:

```gdscript
{
    "id": "unique_error_id",           # For deduplication
    "type": "javascript",              # javascript, godot_engine, security, scene, script
    "severity": "error",               # error, warning
    "message": "Cannot read property...",
    "file": "player.js",
    "line": 42,
    "column": 15,
    "stack_trace": "at move_player...",
    "trigger_context": "_physics_process",  # What was running when error occurred
    "phase": "runtime",                # load, init, runtime
    "timestamp": 1702400000000,
    "last_occurrence": 1702400001000,
    "occurrence_count": 3              # Deduplication count
}
```

#### AI-Friendly Error Report

```gdscript
# Get markdown-formatted report for AI consumption
var report = sandbox.get_error_report()
```

Example output:
```markdown
## Errors Detected

### Error 1: javascript
- **Severity**: error
- **File**: player.js:42:15
- **Message**: Cannot read property 'velocity' of undefined
- **Context**: Called during `_physics_process`
- **Phase**: runtime
- **Occurrences**: 3 times
- **Stack Trace**:
` ` `
at move_player (player.js:42:15)
at _physics_process (player.js:28:5)
` ` `

---
**Summary**: 1 unique issues (1 errors), 3 total occurrences
```

#### Execution Context Tracking

For better error attribution, set the execution context before running user code:

```gdscript
# In your game loop
func _process(delta: float) -> void:
    sandbox.set_execution_context("_process")
    sandbox.execute_pending_jobs()
    sandbox.clear_execution_context()
```

#### Legacy API (Still Supported)

```gdscript
# Simple signal (backward compatible)
sandbox.error_occurred.connect(func(type, message, file, line, column):
    print("Error: %s" % message))

# Get the most recent error
var last_error = sandbox.get_last_error()

# Get all errors as an array of dictionaries
var all_errors = sandbox.get_all_errors()
for error in all_errors:
    print("Error: %s at line %d" % [error["message"], error["line"]])

# Clear error history
sandbox.clear_errors()
```

### Loading Saved Levels

```gdscript
# Option 1: Use sandbox.load_level (recommended)
var sandbox = JSSandbox.new()
var level = sandbox.load_level("user://levels/my_level/")
add_child(level)
# JS scripts automatically execute

# Option 2: Use standard Godot load (if scene has JSScript resources)
var scene = load("user://levels/my_level/level.tscn")
var instance = scene.instantiate()
add_child(instance)
```

## JavaScript API

### Creating Godot Class Instances

```javascript
// Vector and math types
var v2 = new Vector2(10, 20);
var v3 = new Vector3(1, 2, 3);
var color = new Color(1, 0, 0, 1);
var quat = new Quaternion(0, 0, 0, 1);
var basis = new Basis();
var transform = new Transform3D();

// Node creation
var timer = new Timer();
var mesh = new MeshInstance3D();
parent.add_child(timer);
```

### Godot Singletons

```javascript
// Input
Input.is_action_pressed("move_forward")
Input.is_action_just_pressed("jump")
Input.get_vector("left", "right", "up", "down")

// Time
Time.get_ticks_msec()
Time.get_unix_time_from_system()

// Engine
Engine.get_frames_per_second()
```

### Constants and Enums

Access via enum class name (not bare globals):

```javascript
// Key constants
if (event.keycode === Key.KEY_ESCAPE) { }
if (event.keycode === Key.KEY_SPACE) { }

// Mouse buttons
MouseButton.MOUSE_BUTTON_LEFT
```

### Loading Resources

Use the global `load()` function to load resources at runtime:

```javascript
// Load and instantiate a scene
var scene = load("user://games/my_game/enemy.tscn");
var enemy = scene.instantiate();
this.add_child(enemy);

// Load a shader
var shader = load("user://games/my_game/shaders/effect.gdshader");
var material = new ShaderMaterial();
material.shader = shader;

// Load a texture
var texture = load("user://games/my_game/assets/icon.png");

// Load a 3D model (GLB/GLTF)
var model = load("user://games/my_game/models/character.glb");
```

### ES6 Modules

**utils.js:**
```javascript
export function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}
export const GRAVITY = 9.8;
```

**player.js:**
```javascript
import { clamp, GRAVITY } from "./utils.js";

exports._physics_process = function(delta) {
    var vel = this.velocity;
    vel.y -= GRAVITY * delta;
    this.velocity = vel;
    this.move_and_slide();
};
```

Import path rules:
- Relative: `./module.js`, `../shared/utils.js`
- Absolute: `user://games/mygame/utils.js`

### PackedArray Bulk Functions

For high-performance array operations:

```javascript
// Create packed array and get handle
var arr = new PackedVector3Array([]);
var handle = arr.__packed_handle;

// Bulk encode - set many values at once
var vertices = [{x: 0, y: 0, z: 0}, {x: 1, y: 0, z: 0}];
__packed_vector3_array_bulk_encode(handle, vertices);

// Bulk decode - get all values
var decoded = __packed_vector3_array_bulk_decode(handle);
```

Available for: `PackedByteArray`, `PackedInt32Array`, `PackedInt64Array`, `PackedFloat32Array`, `PackedFloat64Array`, `PackedStringArray`, `PackedVector2Array`, `PackedVector3Array`, `PackedVector4Array`, `PackedColorArray`

### Compute Shaders (RenderingDevice)

For GPU-accelerated operations, use a local RenderingDevice:

```javascript
// Create local rendering device
var rd = RenderingServer.create_local_rendering_device();

// Load and compile shader
var shader_file = load("user://games/my_game/shaders/compute.glsl");
var spirv = shader_file.get_spirv();
var shader = rd.shader_create_from_spirv(spirv);

// Create pipeline, buffers, dispatch compute work...

// CRITICAL: Clean up RIDs before freeing RenderingDevice
// Order: uniform_sets -> pipelines -> shaders -> buffers
if (uniform_set && uniform_set.is_valid()) rd.free_rid(uniform_set);
if (pipeline && pipeline.is_valid()) rd.free_rid(pipeline);
if (shader && shader.is_valid()) rd.free_rid(shader);
if (buffer && buffer.is_valid()) rd.free_rid(buffer);

// Finally free the RenderingDevice
rd.free();
```

**RID Cleanup Rules:**
- Use `rd.free_rid()` to free shader, pipeline, buffer, and uniform_set RIDs
- Free in dependency order: uniform_sets → pipelines → shaders → buffers
- Check `is_valid()` before freeing
- Free all RIDs **before** calling `rd.free()` on the RenderingDevice

See `demo/games/slice_everything/scripts/compute_slicer.js` for a complete example.

## Security Model

### Blocked Classes

The following classes are blocked from JS access:

- **System**: `OS`, `FileAccess`, `DirAccess`
- **Threading**: `Thread`, `Mutex`, `Semaphore`, `WorkerThreadPool`
- **Network**: `HTTPClient`, `HTTPRequest`, `TCPServer`, `UDPServer`, `WebSocketPeer`, `ENetConnection`, `MultiplayerPeer`, `IP`
- **Script Execution**: `Expression`, `GDScript`, `CSharpScript`, `JavaScriptBridge`
- **Resources**: `ResourceLoader`, `ResourceSaver`
- **Extensions**: `NativeExtension`, `GDExtensionManager`
- **Editor**: `ProjectSettings`, `EditorInterface`, `EditorPlugin`, `EditorScript`

### Blocked Methods

- `Object.call`, `Object.callv`, `Object.set_script`
- `ClassDB.instantiate`, `Engine.get_singleton`

### Execution Limits

Configure sandbox limits from GDScript:

```gdscript
var sandbox = JSSandbox.new()

# Timeout for single execution (default: 1000ms)
sandbox.set_timeout_ms(2000)

# Memory limit (default: 64MB)
sandbox.set_memory_limit_mb(128)

# Write operations per frame (default: 500)
sandbox.set_write_ops_per_frame(1000)

# Heavy operations per frame - instantiate, queue_free (default: 50)
sandbox.set_heavy_ops_per_frame(100)

# Reset frame counters manually if needed
sandbox.reset_frame_counters()
```

Default rate limits:
- Read operations: unlimited
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
| `eval_module(code: String, filename: String) -> Variant` | Execute JS as ES module with exports |
| `eval_file(path: String) -> Variant` | Execute JavaScript from a file |
| `set_global(name: String, value: Variant)` | Set a global variable in JS |
| `get_global(name: String) -> Variant` | Get a global variable from JS |
| `set_timeout_ms(ms: int)` | Set execution timeout in milliseconds (default: 1000) |
| `set_memory_limit_mb(mb: int)` | Set memory limit in megabytes (default: 64) |
| `set_write_ops_per_frame(count: int)` | Set write operations limit per frame (default: 500) |
| `set_heavy_ops_per_frame(count: int)` | Set heavy operations limit per frame (default: 50) |
| `reset_frame_counters()` | Reset per-frame operation counters |
| `load_scene(path: String) -> Node` | Load a scene synchronously with JS scripts |
| `load_scene_async(path: String) -> AsyncSceneLoader` | Load a scene asynchronously |
| `save_level(root: Node, directory: String) -> Error` | Save node tree with JS scripts to directory |
| `load_level(directory: String) -> Node` | Load a saved level from directory |
| `get_created_nodes() -> Array` | Get all nodes created by the sandbox |
| `load_blocklist(path: String) -> Error` | Load custom blocklist configuration |
| `get_last_error() -> String` | Get the last error message |
| `get_all_errors() -> Array` | Get all errors as array of dictionaries (deduplicated) |
| `clear_errors()` | Clear the error history |
| `is_valid() -> bool` | Check if sandbox is initialized |
| `reset()` | Reset the sandbox to initial state |
| `execute_pending_jobs() -> int` | Execute pending JS microtasks/promises |
| `get_error_report() -> String` | Get markdown-formatted error report for AI |
| `get_errors_for_ai() -> Array` | Get all errors as structured array for AI |
| `set_execution_context(context: String)` | Set current execution context for error attribution |
| `clear_execution_context()` | Clear the execution context |
| `start_init_phase(timeout: float)` | Start init phase timer (default: 0.5s) |

### AsyncSceneLoader

| Method/Signal | Description |
|---------------|-------------|
| `is_loading() -> bool` | Check if loading is in progress |
| `poll()` | Advance the loading process (call each frame) |
| `progress_changed(progress: float, stage: String)` | Signal: Loading progress updated |
| `completed(scene: Node)` | Signal: Scene loaded successfully |
| `failed(error: String)` | Signal: Loading failed |

### JSSandbox Signals

| Signal | Description |
|--------|-------------|
| `load_completed(success: bool, errors: Array)` | Emitted after eval/eval_module/eval_file completes |
| `init_completed(success: bool, errors: Array)` | Emitted after init phase timeout (catches _ready errors) |
| `runtime_error(error: Dictionary)` | Emitted for each new unique runtime error |
| `errors_updated(all_errors: Array)` | Emitted when error list changes (batched/debounced) |
| `error_occurred(type, message, file, line, column)` | Legacy signal for backward compatibility |
| `console_output(message: String)` | Emitted on console.log output |
| `level_saved(path: String)` | Emitted when level is saved |
| `level_loaded(directory: String, script_count: int)` | Emitted when level is loaded |

## License

MIT License - see LICENSE file

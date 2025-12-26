# Mini Game Coder Agent

You create mini-games for the Godot JavaScript Sandbox. Most Godot APIs work the same as GDScript. This document covers only what's **different**.

## Game Structure

```
game_name/
├── game.json           # Manifest (required)
├── main.tscn           # Entry scene (required)
└── scripts/*.js
```

## game.json

```json
{
  "name": "Game Name",
  "version": "1.0.0",
  "entry_scene": "main.tscn",
  "scene_type": "3D",
  "input_actions": {
    "move_forward": {"keys": ["W", "Up"]},
    "jump": {"keys": ["Space"]}
  }
}
```

## Script Pattern (the key difference)

```javascript
var speed = 5.0;
var health = 100;
var self = null;  // Store for callbacks

exports._ready = function() {
    self = this;  // 'this' is the attached Godot node
};

exports._process = function(delta) {
    this.rotate_y(speed * delta);
};

exports._physics_process = function(delta) {
    var vel = this.velocity;
    vel.y -= 9.8 * delta;
    if (Input.is_action_just_pressed("jump") && this.is_on_floor()) {
        vel.y = 10.0;
    }
    this.velocity = vel;
    this.move_and_slide();
};

exports._input = function(event) {
    if (event.get_class() === "InputEventKey" && event.pressed) {
        // handle key
    }
};

// Custom method - other scripts call: body.take_damage(10)
exports.take_damage = function(amount) {
    health -= amount;
};
```

## Key Differences from GDScript

| GDScript | JavaScript |
|----------|------------|
| `Vector3(1,2,3)` | `new Vector3(1,2,3)` or `{x:1, y:2, z:3}` |
| `$Child` | `this.get_node("Child")` |
| `func _ready():` | `exports._ready = function() {}` |
| `self` | `this` (store as `var self = this` for callbacks) |
| `preload()` | `load()` (non-scenes) or `sandbox.load_scene()` |
| `is_instance_of(node, T)` | `node.__class === "ClassName"` |
| `setTimeout()` | Use `this.get_tree().create_timer(1.0)` |

## Property Access - Same as GDScript

Property access syntax is **identical** to GDScript. Don't overthink it:

```javascript
// These work exactly like GDScript:
var pos = this.global_position;           // Works
var transform = this.global_transform;    // Works
var basis = this.global_transform.basis;  // Works
var forward = this.global_transform.basis.z;  // Works - returns {x, y, z}
var origin = this.global_transform.origin;    // Works - returns {x, y, z}

// Nested access works:
var z_forward = this.global_transform.basis.z.z;  // Works

// Setting properties:
this.global_position = {x: 1, y: 2, z: 3};  // Works
this.rotation = {x: 0, y: Math.PI, z: 0};   // Works
```

**What's different:** Math types from properties are plain JS objects, not wrapped Godot types. You can READ all data, but can't call Godot methods on them:

```javascript
// This works - reading data:
var basis = this.global_transform.basis;
var forward = basis.z;  // {x: 0, y: 0, z: -1}

// This does NOT work - no Godot methods on plain objects:
// basis.rotated(axis, angle)  // ERROR - basis is a plain JS object

// For vector math, do it manually:
var neg_forward = {x: -forward.x, y: -forward.y, z: -forward.z};

// Or create a new Godot type if you need methods:
var v = new Vector3(forward.x, forward.y, forward.z);
// Now v has Vector3 methods (if any are bound)
```

## Creating Godot Class Instances

```javascript
// Vector and math types
var v2 = new Vector2(10, 20);
var v3 = new Vector3(1, 2, 3);
var color = new Color(1, 0, 0, 1);  // Red
var quat = new Quaternion(0, 0, 0, 1);
var basis = new Basis();
var transform = new Transform3D();

// Node creation
var timer = new Timer();
var sprite = new Sprite2D();
var mesh = new MeshInstance3D();
var body = new CharacterBody3D();

// Add to scene tree
this.add_child(timer);
```

## Godot Singletons

Access singletons directly by name:

```javascript
// Input
Input.is_action_pressed("move_forward")
Input.is_action_just_pressed("jump")
Input.get_axis("move_left", "move_right")
Input.get_vector("left", "right", "up", "down")

// Time
Time.get_ticks_msec()
Time.get_unix_time_from_system()

// Engine
Engine.get_frames_per_second()
```

## Loading Resources

Use the global `load()` function to load non-scene resources:

```javascript
// Load a shader
var shader = load("user://games/my_game/shaders/effect.gdshader");
var material = new ShaderMaterial();
material.shader = shader;

// Load a texture
var texture = load("user://games/my_game/assets/icon.png");

// Load a 3D model (GLB/GLTF)
var model = load("user://games/my_game/models/character.glb");
var instance = model.instantiate();
this.add_child(instance);
```

Path rules:
- Use `user://` for game assets
- Use `res://` for built-in resources

## Loading Scenes

**Important:** Scenes (.tscn/.scn) cannot be loaded with `load()`. Use `sandbox.load_scene()` or `sandbox.load_scene_async()` instead. This ensures all JavaScript scripts inside the scene use the correct sandbox limits.

```javascript
// Synchronous scene loading (blocks until loaded)
var enemy = sandbox.load_scene("user://games/my_game/enemy.tscn");
this.add_child(enemy);

// Async scene loading (non-blocking, use for large scenes)
var loader = sandbox.load_scene_async("user://games/my_game/level.tscn");
loader.completed.connect(function(scene) {
    self.add_child(scene);
});
loader.failed.connect(function(error) {
    console.log("Failed to load: " + error);
});
// Poll each frame until done
exports._process = function(delta) {
    if (loader && !loader.is_completed() && !loader.is_failed()) {
        loader.poll();
    }
};
```

Why this matters:
- Scripts inside scenes need sandbox rate limits enforced
- Using `load()` for scenes would bypass sandbox security
- `sandbox.load_scene()` properly associates all scripts with the sandbox

### Resolving Scene Paths at Runtime

**Problem:** Hardcoded paths like `"user://games/my_game/enemy.tscn"` break when game folders are renamed or have generated IDs.

**Solution:** Use the current scene's path to construct absolute paths:

```javascript
// Get the directory where the current scene lives
var scene_path = this.scene_file_path;  // e.g., "user://games/abc123/main.tscn"
var base_dir = scene_path.get_base_dir();  // e.g., "user://games/abc123"

// Load another scene relative to current scene
var enemy = sandbox.load_scene(base_dir + "/scenes/enemy.tscn");
this.add_child(enemy);

// Load resources the same way
var texture = load(base_dir + "/assets/icon.png");
```

**Key points:**
- `this.scene_file_path` returns the full path of the .tscn file this script is attached to
- `get_base_dir()` extracts the directory portion
- Use string concatenation to build paths to sibling files
- This works regardless of where the game folder is located

## Constants and Enums

Access via enum class name (not bare globals):

```javascript
// Key constants - use Key.KEY_*
if (event.keycode === Key.KEY_R) { }
if (event.keycode === Key.KEY_ESCAPE) { }

// Mouse buttons - use MouseButton.*
MouseButton.MOUSE_BUTTON_LEFT

// Other enums follow the same pattern
// EnumClass.ENUM_VALUE
```

## Signals (two syntaxes)

```javascript
this.connect("body_entered", function(body) { });
this.body_entered.connect(function(body) { });  // signal-as-property
```

## Spawning (order matters)

```javascript
var enemy = template.duplicate();
container.add_child(enemy);        // Add to tree FIRST
enemy.global_position = spawn_pos; // THEN set position
```

## Iterating Children

```javascript
this.get_children().forEach(function(child) {
    child.queue_free();
});
```

## JavaScript Modules (Import/Export)

**utils.js:**
```javascript
export function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

export const GRAVITY = 9.8;

export class Helper {
    constructor(node) {
        this.node = node;
    }
}
```

**player.js:**
```javascript
import { clamp, GRAVITY, Helper } from "./utils.js";

exports._physics_process = function(delta) {
    var vel = this.velocity;
    vel.y -= GRAVITY * delta;
    vel.y = clamp(vel.y, -50, 50);
    this.velocity = vel;
    this.move_and_slide();
};
```

Import path rules:
- Relative: `./module.js`, `../shared/utils.js`
- Absolute: `user://games/mygame/utils.js`
- `.js` extension auto-added if missing

## PackedArray Bulk Functions

For high-performance array operations (e.g., meshes, particles):

```javascript
// Create packed array
var arr = new PackedVector3Array([]);
var handle = arr.__packed_handle;

// Bulk encode - set many values at once
var vertices = [
    {x: 0, y: 0, z: 0},
    {x: 1, y: 0, z: 0},
    {x: 0, y: 1, z: 0}
];
__packed_vector3_array_bulk_encode(handle, vertices);

// Bulk decode - get all values
var decoded = __packed_vector3_array_bulk_decode(handle);

// Available for all packed types:
// __packed_byte_array_bulk_encode/decode
// __packed_int32_array_bulk_encode/decode
// __packed_int64_array_bulk_encode/decode
// __packed_float32_array_bulk_encode/decode
// __packed_float64_array_bulk_encode/decode
// __packed_string_array_bulk_encode/decode
// __packed_vector2_array_bulk_encode/decode
// __packed_vector3_array_bulk_encode/decode
// __packed_vector4_array_bulk_encode/decode
// __packed_color_array_bulk_encode/decode
```

## Compute Shaders (RenderingDevice)

For GPU-accelerated operations, use a local RenderingDevice:

```javascript
// Create local rendering device
var rd = RenderingServer.create_local_rendering_device();

// Load and compile shader
var shader_file = load("user://games/my_game/shaders/compute.glsl");
var spirv = shader_file.get_spirv();
var shader = rd.shader_create_from_spirv(spirv);

// Create pipeline
var pipeline = rd.compute_pipeline_create(shader);

// Create buffers, uniform sets, dispatch compute work...
// (see slice_everything demo for full example)

// CRITICAL: Clean up RIDs in correct order before freeing RenderingDevice
// Free dependents first: uniform_sets -> pipelines -> shaders -> buffers
if (uniform_set && uniform_set.is_valid()) rd.free_rid(uniform_set);
if (pipeline && pipeline.is_valid()) rd.free_rid(pipeline);
if (shader && shader.is_valid()) rd.free_rid(shader);
if (buffer && buffer.is_valid()) rd.free_rid(buffer);

// Finally free the RenderingDevice itself
rd.free();
```

**Important RID cleanup rules:**
- Always use `rd.free_rid()` to free shader, pipeline, buffer, and uniform_set RIDs
- Free in dependency order: uniform_sets → pipelines → shaders → buffers
- Check `is_valid()` before freeing
- Free all RIDs **before** calling `rd.free()` on the RenderingDevice
- Failure to free RIDs causes memory leaks

## Restrictions

- NO `eval()`, `Function()`, `require()`
- NO `setTimeout`/`setInterval` - use Timer
- NO browser/Node.js APIs
- NO path traversal (`..`) outside game folder
- NO `load()` for scenes - use `sandbox.load_scene()` or `sandbox.load_scene_async()`

## Execution Limits

The sandbox enforces per-frame limits to prevent infinite loops and resource abuse:

- **Read operations**: unlimited
- **Write operations**: 500/frame (set_position, add_child, etc.)
- **Heavy operations**: 50/frame (instantiate, queue_free)
- **Timeout**: 1000ms per execution
- **Memory**: 64MB

If you hit these limits, split work across multiple frames using `_process`.

### CRITICAL: Batched Initialization for Tile-Based Games

**Problem:** Creating many nodes in `_ready()` will exceed write limits and crash. Each node creation involves multiple writes (position, size, color, name, add_child = 5+ writes). A 10x10 grid = 100 tiles × 5 writes = 500 writes just for floor tiles!

**Solution:** Queue tiles and create them across multiple frames:

```javascript
var loadQueue = [];       // Tiles waiting to be created
var isLoading = false;
var TILES_PER_FRAME = 10; // Create 10 tiles per frame (~50 writes)

exports._ready = function() {
    self = this;
    queueLevelLoad(0);  // Queue tiles instead of creating them
};

function queueLevelLoad(levelIndex) {
    isLoading = true;
    loadQueue = [];

    var map = LEVELS[levelIndex].map;
    var offsetX = (1280 - map[0].length * TILE_SIZE) / 2;
    var offsetY = (720 - map.length * TILE_SIZE) / 2;

    // Queue all tiles - DON'T create them yet
    for (var y = 0; y < map.length; y++) {
        for (var x = 0; x < map[y].length; x++) {
            var posX = offsetX + x * TILE_SIZE;
            var posY = offsetY + y * TILE_SIZE;

            // Always queue floor tile
            loadQueue.push({type: "floor", x: posX, y: posY, gridX: x, gridY: y});

            // Queue element based on map tile type
            var tile = map[y][x];
            if (tile === 1) loadQueue.push({type: "wall", x: posX, y: posY, gridX: x, gridY: y});
            else if (tile === 2) loadQueue.push({type: "player", x: posX, y: posY, gridX: x, gridY: y});
            // ... etc for other tile types
        }
    }
}

function processLoadQueue() {
    var processed = 0;
    while (loadQueue.length > 0 && processed < TILES_PER_FRAME) {
        var item = loadQueue.shift();
        createTileByType(item);  // Actually create the node
        processed++;
    }

    if (loadQueue.length === 0) {
        isLoading = false;
        console.log("Level loaded!");
    }
}

exports._process = function(delta) {
    if (isLoading) {
        processLoadQueue();
        return;  // Don't process input while loading
    }
    // Normal game logic here
};
```

**Key points:**
- `_ready()` only queues data, doesn't create nodes
- `_process()` creates a few tiles each frame
- Block player input until loading completes
- 10 tiles/frame × 5 writes = 50 writes/frame (safe margin)

## Error Handling and AI Feedback

When you write code, the platform automatically captures errors and provides structured feedback.

### Error Phases

1. **Load Phase** - Syntax errors, missing imports (immediate feedback)
2. **Init Phase** - Errors in `_ready()` callbacks (within ~500ms of load)
3. **Runtime Phase** - Errors during gameplay (user must report)

### Common Errors You'll See

```markdown
## Errors Detected

### Error 1: javascript
- **File**: player.js:42:15
- **Message**: Cannot read property 'velocity' of undefined
- **Context**: Called during `_physics_process`
- **Stack Trace**:
at move_player (player.js:42:15)
at _physics_process (player.js:28:5)
```

### How to Fix Common Issues

| Error | Cause | Fix |
|-------|-------|-----|
| `Cannot read property X of undefined` | Accessing property on null | Check node exists with `if (node)` before access |
| `Node not found: 'Name'` | Invalid node path | Use correct path, check spelling |
| `X is not a function` | Wrong method name | Check Godot docs for correct method |
| `X is not defined` | Missing import or typo | Add import or fix variable name |

### Defensive Coding Tips

```javascript
// Always check nodes exist before using
var target = this.get_node_or_null("Enemy");
if (target) {
    target.take_damage(10);
}

// Initialize variables in _ready
var self = null;
var initialized = false;

exports._ready = function() {
    self = this;
    initialized = true;
};

exports._process = function(delta) {
    if (!initialized) return;  // Guard clause
    // ... rest of code
};
```

## Scene File (.tscn)

```
[gd_scene load_steps=2 format=3]

[ext_resource type="Script" path="scripts/player.js" id="1"]
[sub_resource type="BoxMesh" id="BoxMesh_1"]
[sub_resource type="BoxShape3D" id="BoxShape_1"]

[node name="Main" type="Node3D"]

[node name="Player" type="CharacterBody3D" parent="."]
script = ExtResource("1")

[node name="Mesh" type="MeshInstance3D" parent="Player"]
mesh = SubResource("BoxMesh_1")

[node name="Collision" type="CollisionShape3D" parent="Player"]
shape = SubResource("BoxShape_1")
```

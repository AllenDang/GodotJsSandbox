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
| `preload()` | `load()` |
| `is_instance_of(node, T)` | `node.__class === "ClassName"` |
| `setTimeout()` | Use `this.get_tree().create_timer(1.0)` |

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

// OS
OS.get_name()
```

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

## Restrictions

- NO `eval()`, `Function()`, `require()`
- NO `setTimeout`/`setInterval` - use Timer
- NO browser/Node.js APIs
- NO path traversal (`..`) outside game folder

## Execution Limits

The sandbox enforces per-frame limits to prevent infinite loops and resource abuse:

- **Read operations**: unlimited
- **Write operations**: 500/frame (set_position, add_child, etc.)
- **Heavy operations**: 50/frame (instantiate, queue_free)
- **Timeout**: 1000ms per execution
- **Memory**: 64MB

If you hit these limits, split work across multiple frames using `_process`.

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

# JS Game Sandbox - Design Document

## 1. Overview

This document describes the architecture for a sandboxed JavaScript game launcher that can load and run AI-generated mini-games. The key insight is that games are **Godot projects** with JavaScript behavior scripts, not standalone JavaScript applications.

## 2. Core Concept

### What the Sandbox Does
- JavaScript runs inside Godot via QuickJS
- JS scripts attach to Godot nodes (like GDScript)
- JS can create nodes, manipulate scene tree, handle input
- JS CANNOT access filesystem, network, or dangerous APIs

### What the Sandbox Does NOT Do
- It does NOT run games in a browser
- It does NOT replace GDScript entirely
- It does NOT have direct access to Godot editor APIs

## 3. Game Structure

A mini-game is a **folder** containing:

```
user://games/my_game/
├── game.json           # Manifest (metadata, entry point, assets)
├── main.tscn           # Root scene (PackedScene)
├── scripts/
│   ├── player.js       # JS script for player
│   ├── enemy.js        # JS script for enemies
│   └── game_manager.js # Main game logic
├── scenes/
│   ├── player.tscn     # Player prefab (optional)
│   └── enemy.tscn      # Enemy prefab (optional)
└── assets/
    ├── icon.png        # Game icon for launcher
    └── (other assets declared in manifest)
```

### Why Scenes (.tscn)?
- Godot's native format for scene graphs
- Can be created by AI or tools
- Contains node hierarchy, transforms, resource references
- The launcher loads entry scenes, JS scripts use `sandbox.load_scene()` for runtime loading

### Why Not Just JavaScript?
- JavaScript alone cannot define:
  - Node types and hierarchy
  - 3D transforms/positions
  - Mesh geometries
  - Material assignments
  - Physics collision shapes
- Games need both **structure** (scenes) and **behavior** (scripts)

## 4. Sandbox JavaScript API

### 4.1 Global Objects

```javascript
// Singletons (safe subset)
Time.get_ticks_msec()
Time.get_ticks_usec()
Time.get_unix_time_from_system()

Input.is_action_pressed("action_name")
Input.is_action_just_pressed("action_name")
Input.is_action_just_released("action_name")
Input.is_key_pressed(KEY_SPACE)
Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT)
Input.get_action_strength("action_name")
Input.get_axis("negative", "positive")
Input.get_vector("left", "right", "up", "down")
Input.get_last_mouse_velocity()

// Resource loading (non-scene resources only)
load("res://path/to/resource.png")   // Returns Texture, Mesh, Shader, etc.
// For scenes, use sandbox.load_scene() - see Section 4.13
```

### 4.2 Math Type Constructors

```javascript
// Vector types
new Vector2(x, y)           // or {x: 0, y: 0}
new Vector3(x, y, z)        // or {x: 0, y: 0, z: 0}
new Color(r, g, b, a)       // or {r: 1, g: 1, b: 1, a: 1}
new Quaternion(x, y, z, w)
new Basis()                 // Identity basis
new Transform3D()           // Identity transform
```

### 4.3 Node Creation

```javascript
// Create new nodes
var sprite = new Sprite2D();
var mesh = new MeshInstance3D();
var body = new CharacterBody3D();
var timer = new Timer();

// All 267 bound classes available as constructors
```

### 4.4 Node Methods (via Proxy)

```javascript
// Node manipulation
node.add_child(child);
node.remove_child(child);
node.get_node("path/to/child");
node.get_node_or_null("path");
node.get_children();
node.get_parent();
node.queue_free();

// Properties (get/set via proxy)
node.position = {x: 1, y: 2, z: 3};
var pos = node.position;
node.rotation = {x: 0, y: Math.PI, z: 0};
node.scale = {x: 2, y: 2, z: 2};
node.visible = false;

// Methods are called directly
node.translate({x: 1, y: 0, z: 0});
node.rotate_y(0.1);
node.look_at({x: 0, y: 0, z: 0});
```

### 4.5 Script Patterns (attached to nodes)

The sandbox supports multiple patterns for defining script behavior:

#### Pattern 1: exports object (RECOMMENDED)
```javascript
// Works with all JS syntax (arrow functions, classes, async)
exports._ready = function() {
    console.log("Node ready!");
    this.position = {x: 0, y: 1, z: 0};
};

exports._process = function(delta) {
    this.rotate_y(delta);
};

exports._physics_process = function(delta) {
    // For physics-related updates
};

exports._input = function(event) {
    // event has properties: type, pressed, keycode, position, etc.
    if (event.pressed && event.keycode) {
        console.log("Key pressed:", event.keycode);
    }
};
```

#### Pattern 2: Class-based (RECOMMENDED for complex scripts)
```javascript
class PlayerController {
    _ready() {
        this.speed = 100;
        this.health = 100;
    }

    _process(delta) {
        if (Input.is_action_pressed("move_right")) {
            this.position.x += this.speed * delta;
        }
    }

    take_damage(amount) {
        this.health -= amount;
    }
}
exports = new PlayerController();
```

#### Pattern 3: Legacy function declarations
```javascript
// Only predefined lifecycle methods are captured
function _ready() {
    console.log("ready");
}

function _process(delta) {
    this.rotate_y(delta);
}
```

**Note**: `this` inside lifecycle methods refers to the Godot node the script is attached to (wrapped as a Proxy object).

### 4.6 Signals

```javascript
// Connect to signals
timer.connect("timeout", function() {
    console.log("Timer fired!");
});

// With node reference
button.connect("pressed", function() {
    this.queue_free();
}.bind(this));

// Emit signals (custom signals)
this.emit_signal("my_signal", arg1, arg2);

// Await signals (returns Promise)
var result = await this.await_signal("signal_name");
// or with SceneTree timer
var timer = this.get_tree().create_timer(1.0);
await timer.await_signal("timeout");
```

### 4.7 Tweens

```javascript
var tween = this.create_tween();

// Property tweening
tween.tween_property(this, "position", {x: 10, y: 0, z: 0}, 1.0);
tween.tween_property(this, "rotation:y", Math.PI, 0.5);

// Callbacks
tween.tween_callback(function() {
    console.log("Tween complete!");
});

// Method tweening
tween.tween_method(function(value) {
    this.scale = {x: value, y: value, z: value};
}.bind(this), 1.0, 2.0, 1.0);

// Chaining
tween.set_trans(Tween.TRANS_BOUNCE)
     .set_ease(Tween.EASE_OUT)
     .set_loops(3);
```

### 4.8 Physics

```javascript
// CharacterBody3D
function _physics_process(delta) {
    var velocity = this.velocity;
    velocity.y -= 9.8 * delta;  // gravity

    if (Input.is_action_pressed("move_forward")) {
        velocity.z = -5;
    }

    this.velocity = velocity;
    this.move_and_slide();
}

// RayCast
raycast.force_raycast_update();
if (raycast.is_colliding()) {
    var point = raycast.get_collision_point();
    var normal = raycast.get_collision_normal();
    var collider = raycast.get_collider();
}
```

### 4.9 InputEvent Object Structure

When `_input(event)` is called, the event object contains:

```javascript
// Common properties (all events)
event.type         // "InputEventKey", "InputEventMouseButton", etc.
event.device       // Device ID
event.is_pressed   // true if pressed
event.is_released  // true if released
event.is_echo      // true if key repeat

// InputEventKey
event.pressed      // true if pressed
event.echo         // true if key repeat
event.keycode      // Key enum value (KEY_SPACE, KEY_A, etc.)
event.physical_keycode
event.unicode      // Unicode character
event.ctrl_pressed, event.alt_pressed, event.shift_pressed, event.meta_pressed

// InputEventMouseButton
event.button_index // MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, etc.
event.button_mask  // Currently pressed buttons
event.position     // {x, y} local position
event.global_position // {x, y} global position
event.double_click // true if double-clicked

// InputEventMouseMotion
event.position     // {x, y}
event.relative     // {x, y} relative movement
event.velocity     // {x, y} mouse velocity

// InputEventJoypadButton
event.button_index // Joypad button
event.pressure     // 0.0 to 1.0

// InputEventJoypadMotion
event.axis         // Axis index
event.axis_value   // -1.0 to 1.0
```

### 4.10 Key Differences from GDScript

| GDScript | JavaScript Sandbox |
|----------|-------------------|
| `Vector3(1, 2, 3)` | `new Vector3(1, 2, 3)` or `{x:1, y:2, z:3}` |
| `$ChildNode` | `this.get_node("ChildNode")` |
| `@onready var x` | Use `exports` pattern: `exports._ready = function() { ... }` |
| `signal my_signal` | Not directly declared, just emit |
| `emit_signal("name")` | `this.emit_signal("name", args...)` |
| `preload("res://")` | `load("res://")` (non-scenes) or `sandbox.load_scene()` |
| `extends Node3D` | Script attaches to Node3D in scene (.tscn) |
| `class_name X` | Use `exports = new ClassName()` pattern |
| `@export var` | Not supported (use game.json manifest) |
| `await signal` | `await node.await_signal("signal_name")` |
| `func _ready():` | `exports._ready = function() { }` |
| `self` | `this` (automatically bound to owner node) |

### 4.11 ES6 Module Support

The sandbox supports ES6 import/export for code organization:

**utils.js:**
```javascript
export function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

export const GRAVITY = 9.8;

export class Vector {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
    length() {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    }
}
```

**player.js:**
```javascript
import { clamp, GRAVITY, Vector } from "./utils.js";

exports._physics_process = function(delta) {
    var vel = this.velocity;
    vel.y -= GRAVITY * delta;
    vel.y = clamp(vel.y, -50, 50);
    this.velocity = vel;
    this.move_and_slide();
};
```

**Import path rules:**
- Relative paths: `./module.js`, `../shared/utils.js`
- Absolute paths: `res://scripts/module.js`, `user://games/mygame/utils.js`
- `.js` extension auto-added if missing
- Path traversal (`..`) blocked for security

### 4.12 Security Restrictions

The following are **blocked** for sandbox security:
- `eval()` - disabled
- `Function()` constructor - deleted from global
- `require()` - not available (use ES6 imports)
- File system access - no direct file I/O
- Network access - no fetch/XMLHttpRequest
- `setTimeout`/`setInterval` - use Godot Timers instead
- `load()` for scenes (.tscn/.scn) - use `sandbox.load_scene()` instead

**Safe alternatives:**
```javascript
// Instead of setTimeout, use Timer node:
var timer = new Timer();
timer.one_shot = true;
timer.wait_time = 1.0;
this.add_child(timer);
timer.connect("timeout", function() {
    console.log("1 second passed");
});
timer.start();

// Or use SceneTree timer:
var sceneTimer = this.get_tree().create_timer(1.0);
await sceneTimer.await_signal("timeout");
```

### 4.13 Scene Loading

Scenes (.tscn/.scn) cannot be loaded with `load()` because scripts inside scenes need to be associated with the sandbox's execution limits. Use the sandbox's dedicated scene loading methods:

```javascript
// Synchronous scene loading (blocks until loaded)
var enemy = sandbox.load_scene("user://games/my_game/enemy.tscn");
this.add_child(enemy);

// Async scene loading (non-blocking, use for large scenes)
var self = this;
var loader = sandbox.load_scene_async("user://games/my_game/level.tscn");
loader.completed.connect(function(scene) {
    self.add_child(scene);
});
loader.failed.connect(function(error) {
    console.log("Failed to load: " + error);
});

// Poll the loader each frame until done
exports._process = function(delta) {
    if (loader && !loader.is_completed() && !loader.is_failed()) {
        loader.poll();
    }
};
```

**Why this matters:**
- Scripts inside scenes need sandbox rate limits enforced
- Using `load()` for scenes would bypass sandbox security
- `sandbox.load_scene()` properly associates all scripts with the sandbox

## 5. Game Manifest (game.json)

```json
{
  "name": "Space Shooter",
  "version": "1.0.0",
  "description": "Shoot asteroids in space",
  "author": "AI Agent",
  "icon": "assets/icon.png",

  "entry_scene": "main.tscn",
  "scene_type": "3D",

  "input_actions": {
    "move_left": {"keys": ["A", "Left"]},
    "move_right": {"keys": ["D", "Right"]},
    "shoot": {"keys": ["Space"]},
    "pause": {"keys": ["Escape"]}
  },

  "resolution": {
    "width": 1280,
    "height": 720,
    "stretch_mode": "canvas_items"
  }
}
```

## 6. Launcher Architecture

### 6.1 Responsibilities
1. Scan `user://games/` for valid game folders
2. Display game list with icons and descriptions
3. Load selected game's entry scene
4. Set up input actions from manifest
5. Provide "back to launcher" functionality
6. Clean up when game exits

### 6.2 Game Loading Flow
1. Read `game.json` manifest
2. Validate required files exist
3. Register input actions
4. Load entry scene: `load("user://games/x/main.tscn").instantiate()`
5. Add to viewport
6. JS scripts auto-execute via JSScript resource

### 6.3 Game Exit Flow
1. User presses designated exit key
2. Launcher receives notification
3. Remove game scene from tree
4. Clear JS runtime state
5. Return to launcher UI

## 7. Scene File Format (.tscn)

Scenes are Godot's text-based scene format. Example:

```
[gd_scene load_steps=3 format=3]

[ext_resource type="Script" path="res://scripts/player.js" id="1"]
[ext_resource type="Texture2D" path="res://assets/player.png" id="2"]

[sub_resource type="BoxMesh" id="BoxMesh_abc"]
size = Vector3(1, 2, 1)

[node name="Player" type="CharacterBody3D"]
script = ExtResource("1")

[node name="MeshInstance" type="MeshInstance3D" parent="."]
mesh = SubResource("BoxMesh_abc")

[node name="CollisionShape" type="CollisionShape3D" parent="."]
shape = BoxShape3D
```

### AI Agent Considerations for Scenes
- Must use valid node types
- Must use correct property names
- Must use proper resource references
- Can embed simple sub-resources
- External resources must exist

## 8. What the AI Agent Needs to Know

### 8.1 To Generate Valid Games
1. **Scene structure**: Valid .tscn format with correct syntax
2. **Node types**: Only use bound node classes (267 available)
3. **JS API**: Use sandbox API, not browser or Node.js APIs
4. **Lifecycle**: Implement `_ready`, `_process`, `_input` correctly
5. **Math types**: Use `{x, y, z}` objects or constructors
6. **No forbidden APIs**: No `eval`, `Function`, `require`, file I/O

### 8.2 Example: Minimal Spinning Cube Game

**game.json:**
```json
{
  "name": "Spinning Cube",
  "version": "1.0.0",
  "entry_scene": "main.tscn",
  "scene_type": "3D"
}
```

**main.tscn:**
```
[gd_scene load_steps=2 format=3]

[ext_resource type="Script" path="scripts/cube.js" id="1_cube"]

[sub_resource type="BoxMesh" id="BoxMesh_1"]

[node name="Main" type="Node3D"]

[node name="Camera" type="Camera3D" parent="."]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 2, 5)

[node name="Light" type="DirectionalLight3D" parent="."]
transform = Transform3D(0.7, -0.5, 0.5, 0, 0.7, 0.7, -0.7, -0.5, 0.5, 0, 5, 0)

[node name="Cube" type="MeshInstance3D" parent="."]
mesh = SubResource("BoxMesh_1")
script = ExtResource("1_cube")
```

**scripts/cube.js:**
```javascript
var speed = 2.0;

exports._ready = function() {
    console.log("Cube ready!");
};

exports._process = function(delta) {
    this.rotate_y(speed * delta);

    if (Input.is_action_just_pressed("ui_accept")) {
        speed *= -1;
    }
};
```

### 8.3 Example: Player Controller with Physics

**player.js:**
```javascript
var speed = 5.0;
var jump_force = 10.0;
var gravity = 20.0;

exports._ready = function() {
    console.log("Player ready!");
};

exports._physics_process = function(delta) {
    var vel = this.velocity;

    // Apply gravity
    if (!this.is_on_floor()) {
        vel.y -= gravity * delta;
    }

    // Handle jump
    if (Input.is_action_just_pressed("jump") && this.is_on_floor()) {
        vel.y = jump_force;
    }

    // Get input direction
    var input_dir = Input.get_vector("move_left", "move_right", "move_up", "move_down");
    vel.x = input_dir.x * speed;
    vel.z = input_dir.y * speed;

    this.velocity = vel;
    this.move_and_slide();
};
```

### 8.4 Available Node Classes (267 bound)

The sandbox provides bindings for 267 Godot classes. Key categories:

**Core Nodes:**
- Node, Node2D, Node3D
- Control (UI base)
- Viewport, SubViewport

**2D Nodes:**
- Sprite2D, AnimatedSprite2D
- CharacterBody2D, RigidBody2D, StaticBody2D, Area2D
- CollisionShape2D, CollisionPolygon2D
- Camera2D, TileMap, TileMapLayer
- Line2D, Path2D, PathFollow2D
- Skeleton2D, Bone2D

**3D Nodes:**
- MeshInstance3D, MultiMeshInstance3D
- CharacterBody3D, RigidBody3D, StaticBody3D, Area3D
- CollisionShape3D, CollisionPolygon3D
- Camera3D, DirectionalLight3D, OmniLight3D, SpotLight3D
- Skeleton3D, BoneAttachment3D
- Path3D, PathFollow3D
- RayCast3D, ShapeCast3D
- GPUParticles3D, CPUParticles3D
- Decal, FogVolume, ReflectionProbe

**UI Controls:**
- Button, Label, TextEdit, LineEdit
- Panel, PanelContainer, MarginContainer, VBoxContainer, HBoxContainer
- ScrollContainer, TabContainer, TabBar
- ProgressBar, TextureRect, ColorRect
- CheckBox, CheckButton, OptionButton
- Slider, HSlider, VSlider, SpinBox
- Tree, ItemList, GraphEdit

**Animation:**
- AnimationPlayer, AnimationTree
- Tween (created via node.create_tween())
- Animation, AnimationLibrary

**Audio:**
- AudioStreamPlayer, AudioStreamPlayer2D, AudioStreamPlayer3D

**Resources (via load()):**
- PackedScene, Texture2D, AudioStream
- Mesh, ArrayMesh, BoxMesh, SphereMesh, CylinderMesh, CapsuleMesh
- Material, StandardMaterial3D, ShaderMaterial
- Shape2D, Shape3D (BoxShape3D, SphereShape3D, CapsuleShape3D)
- Curve, Curve2D, Curve3D, Gradient

**Physics Query:**
- PhysicsDirectSpaceState2D, PhysicsDirectSpaceState3D
- PhysicsRayQueryParameters2D, PhysicsRayQueryParameters3D

**CSG (Constructive Solid Geometry):**
- CSGBox3D, CSGSphere3D, CSGCylinder3D
- CSGMesh3D, CSGPolygon3D, CSGCombiner3D

**Pathfinding:**
- NavigationAgent2D, NavigationAgent3D
- AStar2D, AStar3D, AStarGrid2D

**Utility:**
- Timer, RandomNumberGenerator
- Tween (PropertyTweener, CallbackTweener, MethodTweener)

## 9. Open Questions

1. **Asset generation**: How does AI create textures, meshes, audio?
   - Option A: Use only built-in primitives (BoxMesh, SphereMesh)
   - Option B: Reference allowed asset library
   - Option C: Generate simple assets procedurally

2. **Scene complexity**: How complex can AI-generated scenes be?
   - Start simple: single scene, few nodes
   - Gradually increase complexity

3. **Debugging**: How do we help AI fix broken games?
   - Return detailed error messages
   - Validate scenes before loading

4. **Iteration**: Can AI modify games based on feedback?
   - Yes, if we provide clear error context

## 10. AI Game Creator Agent Prompt

Below is the system prompt for an AI agent tasked with generating games for this sandbox:

---

**SYSTEM PROMPT:**

You are a game creator agent that generates mini-games for the Godot JavaScript Sandbox. You create complete game packages consisting of:
1. A `game.json` manifest file
2. One or more `.tscn` scene files
3. JavaScript behavior scripts (`.js`)

### Game Package Structure
```
game_name/
├── game.json           # Required: manifest
├── main.tscn           # Required: entry scene
├── scripts/            # JavaScript files
│   └── *.js
└── scenes/             # Additional scenes (optional)
    └── *.tscn
```

### JavaScript API Reference

**Global singletons:**
- `Time.get_ticks_msec()`, `Time.get_ticks_usec()`, `Time.get_unix_time_from_system()`
- `Input.is_action_pressed(name)`, `Input.is_action_just_pressed(name)`, `Input.is_key_pressed(keycode)`
- `Input.get_vector(neg_x, pos_x, neg_y, pos_y)`, `Input.get_axis(negative, positive)`
- `load("res://path")` - loads Texture2D, Mesh, Shader, etc. (NOT scenes)
- `sandbox.load_scene("path")` - loads scenes with proper sandbox association
- `console.log()`, `console.warn()`, `console.error()`

**Math constructors:**
- `new Vector2(x, y)` or `{x: 0, y: 0}`
- `new Vector3(x, y, z)` or `{x: 0, y: 0, z: 0}`
- `new Color(r, g, b, a)` or `{r: 1, g: 1, b: 1, a: 1}`
- `new Quaternion(x, y, z, w)`, `new Basis()`, `new Transform3D()`

**Script pattern (REQUIRED):**
```javascript
exports._ready = function() {
    // Called when node enters tree
    // 'this' = the Godot node
};

exports._process = function(delta) {
    // Called every frame
};

exports._physics_process = function(delta) {
    // Called on physics tick
};

exports._input = function(event) {
    // event.type, event.pressed, event.keycode, event.position, etc.
};
```

**Node operations:**
```javascript
this.add_child(node);
this.get_node("path/to/child");
this.get_parent();
this.queue_free();
this.position = {x: 1, y: 2, z: 3};
this.rotate_y(radians);
this.connect("signal_name", callback);
this.emit_signal("signal_name", args...);
```

**Creating nodes in code:**
```javascript
var sprite = new Sprite2D();
var mesh = new MeshInstance3D();
var timer = new Timer();
this.add_child(timer);
```

### RESTRICTIONS - DO NOT USE:
- `eval()`, `Function()` constructor
- `require()` - use ES6 `import` instead
- `setTimeout`, `setInterval` - use Timer nodes
- Browser APIs (fetch, XMLHttpRequest, DOM)
- File I/O, network access
- `load()` for scenes - use `sandbox.load_scene()` instead

### Scene File Format (.tscn)
```
[gd_scene load_steps=N format=3]

[ext_resource type="Script" path="scripts/name.js" id="id_string"]

[sub_resource type="BoxMesh" id="Mesh_id"]

[node name="Root" type="Node3D"]

[node name="Child" type="MeshInstance3D" parent="."]
mesh = SubResource("Mesh_id")
script = ExtResource("id_string")
```

### game.json Format
```json
{
  "name": "Game Title",
  "version": "1.0.0",
  "entry_scene": "main.tscn",
  "scene_type": "3D",
  "input_actions": {
    "move_left": {"keys": ["A", "Left"]},
    "move_right": {"keys": ["D", "Right"]},
    "jump": {"keys": ["Space"]}
  }
}
```

### Guidelines
1. Keep games simple - single scene, few nodes
2. Use built-in primitives (BoxMesh, SphereMesh, CylinderMesh)
3. Use the `exports` pattern for all scripts
4. Test input with `ui_accept`, `ui_cancel`, or define custom actions
5. Include camera and light in 3D scenes
6. Provide collision shapes for physics bodies

---

## 11. Next Steps

1. Implement launcher MVP
2. Create example games manually to validate design
3. Test with AI-generated games
4. Iterate on agent prompt based on failures
5. Add validation for game packages

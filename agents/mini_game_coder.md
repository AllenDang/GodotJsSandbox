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
| `Vector3(1,2,3)` | `{x:1, y:2, z:3}` |
| `$Child` | `this.get_node("Child")` |
| `func _ready():` | `exports._ready = function() {}` |
| `self` | `this` (store as `var self = this` for callbacks) |
| `preload()` | `load()` |
| `is_instance_of(node, T)` | `node.__class === "ClassName"` |
| `setTimeout()` | Use `this.get_tree().create_timer(1.0)` |

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

## Restrictions

- NO `eval()`, `Function()`, `require()`
- NO `setTimeout`/`setInterval` - use Timer
- NO browser/Node.js APIs

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

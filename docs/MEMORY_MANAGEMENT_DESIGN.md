# Memory Management & Data Exchange Design

## Overview

This document defines the architecture for managing memory and optimizing data exchange between JavaScript (QuickJS) and C++ (Godot) in the GodotJSRuntime sandbox.

**Core Principle:** The sandbox provides a JavaScript runtime with wrappers for Godot C++ classes, enabling AI-generated JavaScript code to create games. To maximize performance, we must minimize data exchange between JS and C++.

---

## 1. Handle-Proxy Architecture

### Two Patterns for C++ Data in JS

| Pattern | Use Case | JS Side | C++ Side | Lifecycle |
|---------|----------|---------|----------|-----------|
| **Handle-Proxy** | Large/mutable data (Objects, Arrays, PackedArrays) | Proxy with `__handle` | Registry holds reference | QuickJS finalizer triggers release |
| **Value-Copy** | Small/immutable data (Math types: Vector2, Vector3, Color) | Plain JS object `{x,y,z}` | No C++ storage | JS GC handles automatically |

### Why Two Patterns?

- **Handle-Proxy**: Avoids copying large data structures. JS holds only an integer handle; all operations go through C++ bindings.
- **Value-Copy**: For small types (16-48 bytes), the overhead of handle management exceeds the cost of copying. Simpler lifecycle.

---

## 2. Object Registry (Godot Objects)

### Purpose
Manages handles for all Godot Object instances (Node, Resource, RefCounted, etc.)

### Data Structure

```cpp
struct ObjectEntry {
    Object* ptr;                    // Raw pointer to Godot object
    bool is_ref_counted;            // True for RefCounted subclasses
    Ref<RefCounted> ref_holder;     // Prevents premature Godot GC
    int js_ref_count;               // Number of JS proxies holding this handle
};

class ObjectRegistry {
    HashMap<uint64_t, ObjectEntry> handles_;
    HashMap<Object*, uint64_t> reverse_lookup_;  // For get_or_create_handle()

    uint64_t create_handle(Object* obj);
    void release_handle(uint64_t handle);
    void increment_js_ref(uint64_t handle);
    void decrement_js_ref(uint64_t handle);
    Object* get_object(uint64_t handle);
    uint64_t get_or_create_handle(Object* obj);
};
```

### Lifecycle

1. **JS creates/receives object** → `create_handle()` → handle stored in JS proxy
2. **For RefCounted objects**: Registry holds `Ref<>` to prevent Godot from freeing
3. **JS proxy garbage collected** → QuickJS finalizer → `release_handle()`
4. **When `js_ref_count` reaches 0**: Clear `ref_holder`, allowing Godot GC

### RefCounted vs Non-RefCounted

| Type | Example | Godot Manages Lifetime? | Registry Holds Ref? |
|------|---------|------------------------|---------------------|
| RefCounted | Resource, Texture, Material | Yes (ref counting) | Yes (`Ref<>`) |
| Non-RefCounted | Node, Viewport | No (manual/tree) | No (weak reference) |

**Important**: For Nodes, the registry should NOT prevent Godot from freeing them. If a Node is freed (e.g., `queue_free()`), the handle becomes invalid. JS code should handle this gracefully.

---

## 3. Array Registry (Array & PackedArray)

### Purpose
Manages handles for Godot Array and all PackedArray types (PackedVector3Array, PackedInt32Array, etc.)

### Data Structure

```cpp
enum class ArrayOwnership {
    OWNED,      // Registry owns the data (JS created it)
    VIEW        // Borrowed reference (C++ returned it, C++ owns it)
};

struct ArrayEntry {
    Variant data;              // Holds Array or PackedXXXArray
    Variant::Type type;        // ARRAY, PACKED_INT32_ARRAY, etc.
    ArrayOwnership ownership;
};

class ArrayRegistry {
    HashMap<uint64_t, ArrayEntry> arrays_;

    uint64_t create_handle(const Variant& arr, ArrayOwnership ownership);
    void release_handle(uint64_t handle);
    Variant* get_array(uint64_t handle);
    bool is_valid(uint64_t handle);
};
```

### Ownership Modes

| Mode | When Used | C++ Owns Data? | Release Strategy |
|------|-----------|----------------|------------------|
| **OWNED** | JS creates `new PackedVector3Array()` | Yes, registry owns | JS finalizer calls `release_handle()` |
| **VIEW** | C++ returns array to JS | No, borrowed | Invalidate when source is freed |

### Lifecycle for OWNED Arrays

1. JS: `let arr = new PackedVector3Array()`
2. C++: `create_handle(PackedVector3Array(), OWNED)` → returns handle
3. JS proxy stores handle, uses it for all operations
4. JS proxy garbage collected → finalizer → `release_handle()`
5. C++ removes entry from registry, Variant destructor frees memory

### Lifecycle for VIEW Arrays

1. C++: `mesh.surface_get_arrays()` returns arrays
2. C++: `create_handle(arrays, VIEW)` → returns handle
3. JS uses handle for read operations
4. When mesh is freed, handles become invalid
5. JS must not hold VIEW handles long-term

---

## 4. Math Types (Value-Copy)

### Types Using Value-Copy

- Vector2, Vector2i
- Vector3, Vector3i
- Vector4, Vector4i
- Color
- Quaternion
- Rect2, Rect2i
- Plane
- AABB

### Rationale

| Type | Size (bytes) | Handle Overhead | Decision |
|------|--------------|-----------------|----------|
| Vector2 | 8 | ~40+ (handle + lookup + storage) | Value-copy |
| Vector3 | 12 | ~40+ | Value-copy |
| Color | 16 | ~40+ | Value-copy |
| Transform3D | 48 | ~40+ | Value-copy (borderline) |
| Basis | 36 | ~40+ | Value-copy |

### Implementation

```cpp
// C++ to JS
JSValue variant_to_js(const Variant& value) {
    if (value.get_type() == Variant::VECTOR3) {
        Vector3 v = value;
        JSValue obj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, v.x));
        JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, v.y));
        JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, v.z));
        return obj;
    }
    // ...
}

// JS to C++
Variant js_to_variant(JSValue value) {
    // Check for {x, y, z} pattern
    JSValue x = JS_GetPropertyStr(ctx, value, "x");
    JSValue y = JS_GetPropertyStr(ctx, value, "y");
    JSValue z = JS_GetPropertyStr(ctx, value, "z");
    if (JS_IsNumber(x) && JS_IsNumber(y) && JS_IsNumber(z)) {
        // Convert to Vector3
    }
    // ...
}
```

### No Registry for Math Types

Math types do NOT use any registry. They are:
- Created as plain JS objects
- Passed by value across the JS/C++ boundary
- Garbage collected by QuickJS automatically

---

## 5. QuickJS Finalizer Integration

### The Key to Memory Management

QuickJS supports class finalizers that are called when an object is garbage collected. This is how we trigger `release_handle()`.

### Implementation

```cpp
// Define class with finalizer
static JSClassDef godot_object_class_def = {
    .class_name = "GodotObject",
    .finalizer = godot_object_class_finalizer,
};

// Opaque data stored in each proxy
struct GodotObjectData {
    uint64_t handle;
    ObjectRegistry* registry;
};

// Finalizer called by QuickJS GC
static void godot_object_class_finalizer(JSRuntime* rt, JSValue val) {
    GodotObjectData* data = (GodotObjectData*)JS_GetOpaque(val, godot_object_class_id);
    if (data) {
        data->registry->release_handle(data->handle);
        js_free_rt(rt, data);
    }
}

// Creating a proxy
JSValue create_godot_object_proxy(Object* obj) {
    uint64_t handle = object_registry->create_handle(obj);

    JSValue proxy = JS_NewObjectClass(ctx, godot_object_class_id);
    GodotObjectData* data = (GodotObjectData*)js_malloc(ctx, sizeof(GodotObjectData));
    data->handle = handle;
    data->registry = object_registry;
    JS_SetOpaque(proxy, data);

    return proxy;
}
```

### Same Pattern for Arrays

```cpp
static JSClassDef godot_array_class_def = {
    .class_name = "GodotArray",
    .finalizer = godot_array_class_finalizer,
};

static void godot_array_class_finalizer(JSRuntime* rt, JSValue val) {
    GodotArrayData* data = (GodotArrayData*)JS_GetOpaque(val, godot_array_class_id);
    if (data) {
        data->registry->release_handle(data->handle);
        js_free_rt(rt, data);
    }
}
```

---

## 6. Data Exchange Optimization

### Principle: Minimize Round-Trips

Each JS→C++ call has overhead. Batch operations when possible.

### Anti-Pattern (Many Round-Trips)

```javascript
// BAD: 100 C++ calls
for (let i = 0; i < 100; i++) {
    array.push_back(values[i]);
}
```

### Preferred Pattern (Batched)

```javascript
// GOOD: 1 C++ call
array.append_array(values);  // Pass entire array at once
```

### Bulk Operations to Provide

| Operation | Individual API | Bulk API |
|-----------|---------------|----------|
| Array append | `push_back(val)` | `append_array(arr)` |
| Array read | `arr[i]` × N | `slice(start, end)` returns sub-array |
| Mesh creation | vertex-by-vertex | `add_surface_from_arrays(arrays)` |
| Node properties | `node.position = v` × N | `set_properties_bulk({pos, rot, scale})` |

### Zero-Copy Where Possible

For large data (meshes, textures), avoid copying entirely:

1. C++ creates data
2. C++ returns handle to JS
3. JS passes handle to another C++ API
4. Data never leaves C++ memory

Example:
```javascript
let arrays = mesh.surface_get_arrays(0);  // Returns handle, not copy
new_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays);  // Handle passed directly
```

---

## 7. Current Issues & Fixes Needed

### Issue 1: Array Handle Leak
**Problem**: `ArrayRegistry::create_handle()` is called but `release_handle()` is never called.
**Fix**: Add QuickJS class with finalizer for array proxies.

### Issue 2: Math Type Handle Leak (FIXED)
**Problem**: `create_math_handle()` created handles that were never released.
**Fix**: Changed to value-copy for math types. No handles needed.

### Issue 3: Object Finalizer Not Implemented
**Problem**: `GodotObjectData` is allocated but no finalizer frees it.
**Fix**: Ensure `godot_object_class_def` has proper finalizer.

### Issue 4: RefCounted Objects May Be Freed
**Problem**: If JS holds handle to RefCounted but doesn't prevent GC, object may be freed.
**Fix**: Registry should hold `Ref<>` for RefCounted objects.

---

## 8. Implementation Phases

### Phase 1: Fix Memory Leaks (Critical)
- [ ] Verify QuickJS finalizer is registered for GodotObject class
- [ ] Add QuickJS finalizer for GodotArray class
- [ ] Add QuickJS finalizer for PackedArray classes
- [ ] Test with long-running game (memory should stabilize)

### Phase 2: Improve Object Registry
- [ ] Add `js_ref_count` tracking
- [ ] Hold `Ref<>` for RefCounted objects
- [ ] Add object validity check (handle may become invalid if Godot frees object)

### Phase 3: Improve Array Registry
- [ ] Add `ArrayOwnership` enum (OWNED vs VIEW)
- [ ] Implement different lifecycle for each ownership type
- [ ] Add array pooling for frequently created/destroyed arrays

### Phase 4: Bulk Operations
- [ ] Identify performance bottlenecks
- [ ] Add bulk getter/setter APIs
- [ ] Profile and measure improvement

---

## 9. Testing Strategy

### Memory Leak Tests

```javascript
// Test 1: Create and discard many arrays
for (let i = 0; i < 10000; i++) {
    let arr = new PackedVector3Array();
    arr.push_back({x: 1, y: 2, z: 3});
    // arr goes out of scope, should be GC'd
}
// Memory should return to baseline after GC

// Test 2: Create and discard many objects
for (let i = 0; i < 1000; i++) {
    let node = new Node3D();
    node.queue_free();
}
// Memory should return to baseline

// Test 3: Long-running game loop
// Run for 10 minutes, memory should not grow unbounded
```

### Handle Validity Tests

```javascript
// Test: Object freed by Godot
let node = get_node("SomeNode");
node.queue_free();
// Later...
node.position;  // Should throw error or return null, not crash
```

---

## 10. Summary

| Data Type | Storage Pattern | Registry | Finalizer | Lifecycle Owner |
|-----------|-----------------|----------|-----------|-----------------|
| Node, Object | Handle-Proxy | ObjectRegistry | Yes | Godot (weak ref) |
| RefCounted | Handle-Proxy | ObjectRegistry | Yes | Registry holds Ref<> |
| Array | Handle-Proxy | ArrayRegistry | Yes | Registry (OWNED) or C++ (VIEW) |
| PackedXXXArray | Handle-Proxy | ArrayRegistry | Yes | Registry (OWNED) or C++ (VIEW) |
| Vector2/3, Color | Value-Copy | None | No | JS GC |
| RID | Value-Copy + ID | ArrayRegistry (optional) | No | Godot |

This design ensures:
1. **No memory leaks**: QuickJS finalizers release C++ resources
2. **Minimal data exchange**: Handles avoid copying large data
3. **Proper lifecycle**: RefCounted objects stay alive while JS holds reference
4. **Simple math types**: Value-copy avoids complexity for small types

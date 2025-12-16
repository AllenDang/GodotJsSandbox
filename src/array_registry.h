#ifndef GODOT_JS_RUNTIME_ARRAY_REGISTRY_H
#define GODOT_JS_RUNTIME_ARRAY_REGISTRY_H

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_vector4_array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <cstdint>

namespace jsb {

// Handle types for all supported collection types
enum class CollectionType {
    ARRAY,
    DICTIONARY,
    PACKED_BYTE_ARRAY,
    PACKED_INT32_ARRAY,
    PACKED_INT64_ARRAY,
    PACKED_FLOAT32_ARRAY,
    PACKED_FLOAT64_ARRAY,
    PACKED_STRING_ARRAY,
    PACKED_VECTOR2_ARRAY,
    PACKED_VECTOR3_ARRAY,
    PACKED_COLOR_ARRAY,
    PACKED_VECTOR4_ARRAY,
    RID_VARIANT,  // For storing RID values (opaque, requires round-trip via Variant)
    // Math types - stored as Variant for zero-copy architecture
    MATH_VECTOR2,
    MATH_VECTOR3,
    MATH_VECTOR4,
    MATH_COLOR,
    MATH_QUATERNION,
    MATH_BASIS,
    MATH_TRANSFORM3D,
    MATH_TRANSFORM2D,
    MATH_PLANE,
    MATH_AABB,
    MATH_RECT2
};

// ArrayRegistry manages Godot Array, Dictionary, and PackedArray references for JavaScript
// Uses handles to allow zero-copy access from JS to Godot collections
class ArrayRegistry {
public:
    ArrayRegistry();
    ~ArrayRegistry();

    // Create a handle for an array (copies the Array reference, not the data)
    uint64_t create_array_handle(const godot::Array& arr);

    // Create a handle for an RID (stores as Variant for proper round-trip)
    uint64_t create_rid_handle(const godot::Variant& rid_var);

    // Get RID variant from handle
    godot::Variant get_rid_variant(uint64_t handle);

    // Create a handle for math types (stores as Variant for zero-copy)
    uint64_t create_math_handle(const godot::Variant& math_var);

    // Get math variant from handle
    godot::Variant get_math_variant(uint64_t handle);

    // Get mutable pointer to math variant (for in-place modifications)
    godot::Variant* get_math_variant_ptr(uint64_t handle);

    // Check if handle is a math type
    bool is_math_handle(uint64_t handle) const;

    // Create handles for packed arrays (copies reference, not data - copy-on-write)
    uint64_t create_packed_byte_array_handle(const godot::PackedByteArray& arr);
    uint64_t create_packed_int32_array_handle(const godot::PackedInt32Array& arr);
    uint64_t create_packed_int64_array_handle(const godot::PackedInt64Array& arr);
    uint64_t create_packed_float32_array_handle(const godot::PackedFloat32Array& arr);
    uint64_t create_packed_float64_array_handle(const godot::PackedFloat64Array& arr);
    uint64_t create_packed_string_array_handle(const godot::PackedStringArray& arr);
    uint64_t create_packed_vector2_array_handle(const godot::PackedVector2Array& arr);
    uint64_t create_packed_vector3_array_handle(const godot::PackedVector3Array& arr);
    uint64_t create_packed_color_array_handle(const godot::PackedColorArray& arr);
    uint64_t create_packed_vector4_array_handle(const godot::PackedVector4Array& arr);

    // Get array from handle (returns empty array if invalid)
    godot::Array get_array(uint64_t handle);

    // Get packed arrays from handle (returns copy for read-only access)
    godot::PackedByteArray get_packed_byte_array(uint64_t handle);
    godot::PackedInt32Array get_packed_int32_array(uint64_t handle);
    godot::PackedInt64Array get_packed_int64_array(uint64_t handle);
    godot::PackedFloat32Array get_packed_float32_array(uint64_t handle);
    godot::PackedFloat64Array get_packed_float64_array(uint64_t handle);
    godot::PackedStringArray get_packed_string_array(uint64_t handle);
    godot::PackedVector2Array get_packed_vector2_array(uint64_t handle);
    godot::PackedVector3Array get_packed_vector3_array(uint64_t handle);
    godot::PackedColorArray get_packed_color_array(uint64_t handle);
    godot::PackedVector4Array get_packed_vector4_array(uint64_t handle);

    // Get mutable references to packed arrays (for push_back, resize, set operations)
    godot::PackedByteArray* get_packed_byte_array_ptr(uint64_t handle);
    godot::PackedInt32Array* get_packed_int32_array_ptr(uint64_t handle);
    godot::PackedInt64Array* get_packed_int64_array_ptr(uint64_t handle);
    godot::PackedFloat32Array* get_packed_float32_array_ptr(uint64_t handle);
    godot::PackedFloat64Array* get_packed_float64_array_ptr(uint64_t handle);
    godot::PackedStringArray* get_packed_string_array_ptr(uint64_t handle);
    godot::PackedVector2Array* get_packed_vector2_array_ptr(uint64_t handle);
    godot::PackedVector3Array* get_packed_vector3_array_ptr(uint64_t handle);
    godot::PackedVector4Array* get_packed_vector4_array_ptr(uint64_t handle);
    godot::PackedColorArray* get_packed_color_array_ptr(uint64_t handle);

    // Get type of handle
    CollectionType get_handle_type(uint64_t handle) const;

    // Check if handle is valid
    bool is_valid_handle(uint64_t handle) const;

    // Release a handle (called when JS object is garbage collected)
    void release_handle(uint64_t handle);

    // Clear all handles
    void clear_all();

    // Get count of active handles
    int get_handle_count() const;

    // Get size of collection at handle
    int64_t get_size(uint64_t handle) const;

private:
    // Use union-like struct to avoid Variant overhead
    struct HandleEntry {
        CollectionType type;
        bool is_valid = true;

        // Storage for different types (only one is used based on type)
        godot::Array array;
        godot::Dictionary dictionary;
        godot::PackedByteArray packed_byte;
        godot::PackedInt32Array packed_int32;
        godot::PackedInt64Array packed_int64;
        godot::PackedFloat32Array packed_float32;
        godot::PackedFloat64Array packed_float64;
        godot::PackedStringArray packed_string;
        godot::PackedVector2Array packed_vector2;
        godot::PackedVector3Array packed_vector3;
        godot::PackedColorArray packed_color;
        godot::PackedVector4Array packed_vector4;
        godot::Variant rid_variant;  // For storing RID values
        godot::Variant math_variant; // For storing math types (Vector2, Vector3, etc.)
    };

    godot::HashMap<uint64_t, HandleEntry> handles_;
    uint64_t next_handle_ = 1;
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_ARRAY_REGISTRY_H

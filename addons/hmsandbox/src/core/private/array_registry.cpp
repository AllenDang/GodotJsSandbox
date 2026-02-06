#include "array_registry.h"

using namespace godot;

namespace jsb {

ArrayRegistry::ArrayRegistry() {
}

ArrayRegistry::~ArrayRegistry() {
    clear_all();
}

uint64_t ArrayRegistry::create_array_handle(const Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::ARRAY;
    entry.array = arr;  // Copy the Array reference (copy-on-write, cheap)
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_rid_handle(const Variant& rid_var) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::RID_VARIANT;
    entry.rid_variant = rid_var;  // Store the RID as Variant for proper round-trip
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

Variant ArrayRegistry::get_rid_variant(uint64_t handle) {
    if (!handles_.has(handle)) {
        return Variant();
    }
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::RID_VARIANT) {
        return Variant();
    }
    return entry.rid_variant;
}

uint64_t ArrayRegistry::create_packed_byte_array_handle(const PackedByteArray& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_BYTE_ARRAY;
    entry.packed_byte = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_int32_array_handle(const PackedInt32Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_INT32_ARRAY;
    entry.packed_int32 = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_int64_array_handle(const PackedInt64Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_INT64_ARRAY;
    entry.packed_int64 = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_float32_array_handle(const PackedFloat32Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_FLOAT32_ARRAY;
    entry.packed_float32 = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_float64_array_handle(const PackedFloat64Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_FLOAT64_ARRAY;
    entry.packed_float64 = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_string_array_handle(const PackedStringArray& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_STRING_ARRAY;
    entry.packed_string = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_vector2_array_handle(const PackedVector2Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_VECTOR2_ARRAY;
    entry.packed_vector2 = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_vector3_array_handle(const PackedVector3Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_VECTOR3_ARRAY;
    entry.packed_vector3 = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_color_array_handle(const PackedColorArray& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_COLOR_ARRAY;
    entry.packed_color = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_packed_vector4_array_handle(const PackedVector4Array& arr) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::PACKED_VECTOR4_ARRAY;
    entry.packed_vector4 = arr;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

Array ArrayRegistry::get_array(uint64_t handle) {
    if (!handles_.has(handle)) {
        return Array();
    }
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::ARRAY) {
        return Array();
    }
    return entry.array;
}

PackedByteArray ArrayRegistry::get_packed_byte_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedByteArray();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_BYTE_ARRAY) return PackedByteArray();
    return entry.packed_byte;
}

PackedInt32Array ArrayRegistry::get_packed_int32_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedInt32Array();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_INT32_ARRAY) return PackedInt32Array();
    return entry.packed_int32;
}

PackedInt64Array ArrayRegistry::get_packed_int64_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedInt64Array();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_INT64_ARRAY) return PackedInt64Array();
    return entry.packed_int64;
}

PackedFloat32Array ArrayRegistry::get_packed_float32_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedFloat32Array();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_FLOAT32_ARRAY) return PackedFloat32Array();
    return entry.packed_float32;
}

PackedFloat64Array ArrayRegistry::get_packed_float64_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedFloat64Array();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_FLOAT64_ARRAY) return PackedFloat64Array();
    return entry.packed_float64;
}

PackedStringArray ArrayRegistry::get_packed_string_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedStringArray();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_STRING_ARRAY) return PackedStringArray();
    return entry.packed_string;
}

PackedVector2Array ArrayRegistry::get_packed_vector2_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedVector2Array();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_VECTOR2_ARRAY) return PackedVector2Array();
    return entry.packed_vector2;
}

PackedVector3Array ArrayRegistry::get_packed_vector3_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedVector3Array();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_VECTOR3_ARRAY) return PackedVector3Array();
    return entry.packed_vector3;
}

PackedColorArray ArrayRegistry::get_packed_color_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedColorArray();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_COLOR_ARRAY) return PackedColorArray();
    return entry.packed_color;
}

PackedVector4Array ArrayRegistry::get_packed_vector4_array(uint64_t handle) {
    if (!handles_.has(handle)) return PackedVector4Array();
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_VECTOR4_ARRAY) return PackedVector4Array();
    return entry.packed_vector4;
}

CollectionType ArrayRegistry::get_handle_type(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return CollectionType::ARRAY;  // Default, caller should check is_valid first
    }
    return handles_[handle].type;
}

bool ArrayRegistry::is_valid_handle(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return false;
    }
    return handles_[handle].is_valid;
}

void ArrayRegistry::release_handle(uint64_t handle) {
    if (handles_.has(handle)) {
        handles_.erase(handle);
    }
}

void ArrayRegistry::clear_all() {
    handles_.clear();
}

int ArrayRegistry::get_handle_count() const {
    return handles_.size();
}

int64_t ArrayRegistry::get_size(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return 0;
    }
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid) return 0;

    switch (entry.type) {
        case CollectionType::ARRAY: return entry.array.size();
        case CollectionType::DICTIONARY: return entry.dictionary.size();
        case CollectionType::PACKED_BYTE_ARRAY: return entry.packed_byte.size();
        case CollectionType::PACKED_INT32_ARRAY: return entry.packed_int32.size();
        case CollectionType::PACKED_INT64_ARRAY: return entry.packed_int64.size();
        case CollectionType::PACKED_FLOAT32_ARRAY: return entry.packed_float32.size();
        case CollectionType::PACKED_FLOAT64_ARRAY: return entry.packed_float64.size();
        case CollectionType::PACKED_STRING_ARRAY: return entry.packed_string.size();
        case CollectionType::PACKED_VECTOR2_ARRAY: return entry.packed_vector2.size();
        case CollectionType::PACKED_VECTOR3_ARRAY: return entry.packed_vector3.size();
        case CollectionType::PACKED_COLOR_ARRAY: return entry.packed_color.size();
        case CollectionType::PACKED_VECTOR4_ARRAY: return entry.packed_vector4.size();
        case CollectionType::RID_VARIANT: return 1;  // RID is a single value, not a collection
        default: return 0;
    }
}

// Mutable pointer getters for push_back, resize, set operations
PackedByteArray* ArrayRegistry::get_packed_byte_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_BYTE_ARRAY) return nullptr;
    return &entry.packed_byte;
}

PackedInt32Array* ArrayRegistry::get_packed_int32_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_INT32_ARRAY) return nullptr;
    return &entry.packed_int32;
}

PackedInt64Array* ArrayRegistry::get_packed_int64_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_INT64_ARRAY) return nullptr;
    return &entry.packed_int64;
}

PackedFloat32Array* ArrayRegistry::get_packed_float32_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_FLOAT32_ARRAY) return nullptr;
    return &entry.packed_float32;
}

PackedFloat64Array* ArrayRegistry::get_packed_float64_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_FLOAT64_ARRAY) return nullptr;
    return &entry.packed_float64;
}

PackedStringArray* ArrayRegistry::get_packed_string_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_STRING_ARRAY) return nullptr;
    return &entry.packed_string;
}

PackedVector2Array* ArrayRegistry::get_packed_vector2_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_VECTOR2_ARRAY) return nullptr;
    return &entry.packed_vector2;
}

PackedVector3Array* ArrayRegistry::get_packed_vector3_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_VECTOR3_ARRAY) return nullptr;
    return &entry.packed_vector3;
}

PackedVector4Array* ArrayRegistry::get_packed_vector4_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_VECTOR4_ARRAY) return nullptr;
    return &entry.packed_vector4;
}

PackedColorArray* ArrayRegistry::get_packed_color_array_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::PACKED_COLOR_ARRAY) return nullptr;
    return &entry.packed_color;
}

// Helper to get CollectionType from Variant::Type for math types
static CollectionType variant_type_to_collection_type(Variant::Type vtype) {
    switch (vtype) {
        case Variant::VECTOR2: return CollectionType::MATH_VECTOR2;
        case Variant::VECTOR3: return CollectionType::MATH_VECTOR3;
        case Variant::VECTOR4: return CollectionType::MATH_VECTOR4;
        case Variant::COLOR: return CollectionType::MATH_COLOR;
        case Variant::QUATERNION: return CollectionType::MATH_QUATERNION;
        case Variant::BASIS: return CollectionType::MATH_BASIS;
        case Variant::TRANSFORM3D: return CollectionType::MATH_TRANSFORM3D;
        case Variant::TRANSFORM2D: return CollectionType::MATH_TRANSFORM2D;
        case Variant::PLANE: return CollectionType::MATH_PLANE;
        case Variant::AABB: return CollectionType::MATH_AABB;
        case Variant::RECT2: return CollectionType::MATH_RECT2;
        default: return CollectionType::ARRAY;  // Invalid, caller should validate
    }
}

uint64_t ArrayRegistry::create_math_handle(const Variant& math_var) {
    Variant::Type vtype = math_var.get_type();
    CollectionType ctype = variant_type_to_collection_type(vtype);

    // Validate it's actually a math type
    if (ctype == CollectionType::ARRAY) {
        return 0;  // Invalid math type
    }

    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = ctype;
    entry.math_variant = math_var;
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

Variant ArrayRegistry::get_math_variant(uint64_t handle) {
    if (!handles_.has(handle)) {
        return Variant();
    }
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || !is_math_handle(handle)) {
        return Variant();
    }
    return entry.math_variant;
}

Variant* ArrayRegistry::get_math_variant_ptr(uint64_t handle) {
    if (!handles_.has(handle)) return nullptr;
    HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || !is_math_handle(handle)) return nullptr;
    return &entry.math_variant;
}

bool ArrayRegistry::is_math_handle(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return false;
    }
    CollectionType type = handles_[handle].type;
    return type == CollectionType::MATH_VECTOR2 ||
           type == CollectionType::MATH_VECTOR3 ||
           type == CollectionType::MATH_VECTOR4 ||
           type == CollectionType::MATH_COLOR ||
           type == CollectionType::MATH_QUATERNION ||
           type == CollectionType::MATH_BASIS ||
           type == CollectionType::MATH_TRANSFORM3D ||
           type == CollectionType::MATH_TRANSFORM2D ||
           type == CollectionType::MATH_PLANE ||
           type == CollectionType::MATH_AABB ||
           type == CollectionType::MATH_RECT2;
}

} // namespace jsb

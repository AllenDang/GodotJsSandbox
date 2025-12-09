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

uint64_t ArrayRegistry::create_dict_handle(const Dictionary& dict) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = CollectionType::DICTIONARY;
    entry.dictionary = dict;  // Copy the Dictionary reference (copy-on-write, cheap)
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
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

Dictionary ArrayRegistry::get_dictionary(uint64_t handle) {
    if (!handles_.has(handle)) {
        return Dictionary();
    }
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != CollectionType::DICTIONARY) {
        return Dictionary();
    }
    return entry.dictionary;
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

bool ArrayRegistry::is_valid_array(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return false;
    }
    const HandleEntry& entry = handles_[handle];
    return entry.is_valid && entry.type == CollectionType::ARRAY;
}

bool ArrayRegistry::is_valid_dict(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return false;
    }
    const HandleEntry& entry = handles_[handle];
    return entry.is_valid && entry.type == CollectionType::DICTIONARY;
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

} // namespace jsb

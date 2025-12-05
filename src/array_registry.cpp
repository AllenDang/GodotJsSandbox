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
    entry.type = TYPE_ARRAY;
    entry.array = arr;  // Copy the Array reference (copy-on-write, cheap)
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

uint64_t ArrayRegistry::create_dict_handle(const Dictionary& dict) {
    uint64_t handle = next_handle_++;
    HandleEntry entry;
    entry.type = TYPE_DICTIONARY;
    entry.dictionary = dict;  // Copy the Dictionary reference (copy-on-write, cheap)
    entry.is_valid = true;
    handles_[handle] = entry;
    return handle;
}

Array ArrayRegistry::get_array(uint64_t handle) {
    if (!handles_.has(handle)) {
        return Array();
    }
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != TYPE_ARRAY) {
        return Array();
    }
    return entry.array;
}

Dictionary ArrayRegistry::get_dictionary(uint64_t handle) {
    if (!handles_.has(handle)) {
        return Dictionary();
    }
    const HandleEntry& entry = handles_[handle];
    if (!entry.is_valid || entry.type != TYPE_DICTIONARY) {
        return Dictionary();
    }
    return entry.dictionary;
}

bool ArrayRegistry::is_valid_array(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return false;
    }
    const HandleEntry& entry = handles_[handle];
    return entry.is_valid && entry.type == TYPE_ARRAY;
}

bool ArrayRegistry::is_valid_dict(uint64_t handle) const {
    if (!handles_.has(handle)) {
        return false;
    }
    const HandleEntry& entry = handles_[handle];
    return entry.is_valid && entry.type == TYPE_DICTIONARY;
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

} // namespace jsb

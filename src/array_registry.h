#ifndef GODOT_JS_RUNTIME_ARRAY_REGISTRY_H
#define GODOT_JS_RUNTIME_ARRAY_REGISTRY_H

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <cstdint>

namespace jsb {

// ArrayRegistry manages Godot Array and Dictionary references for JavaScript
// Uses handles to allow zero-copy access from JS to Godot collections
class ArrayRegistry {
public:
    ArrayRegistry();
    ~ArrayRegistry();

    // Create a handle for an array (copies the Array reference, not the data)
    uint64_t create_array_handle(const godot::Array& arr);

    // Create a handle for a dictionary (copies the Dictionary reference, not the data)
    uint64_t create_dict_handle(const godot::Dictionary& dict);

    // Get array from handle (returns empty array if invalid)
    godot::Array get_array(uint64_t handle);

    // Get dictionary from handle (returns empty dict if invalid)
    godot::Dictionary get_dictionary(uint64_t handle);

    // Check if handle is a valid array
    bool is_valid_array(uint64_t handle) const;

    // Check if handle is a valid dictionary
    bool is_valid_dict(uint64_t handle) const;

    // Release a handle (called when JS object is garbage collected)
    void release_handle(uint64_t handle);

    // Clear all handles
    void clear_all();

    // Get count of active handles
    int get_handle_count() const;

private:
    enum HandleType {
        TYPE_ARRAY,
        TYPE_DICTIONARY
    };

    struct HandleEntry {
        HandleType type;
        godot::Array array;
        godot::Dictionary dictionary;
        bool is_valid = true;
    };

    godot::HashMap<uint64_t, HandleEntry> handles_;
    uint64_t next_handle_ = 1;
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_ARRAY_REGISTRY_H

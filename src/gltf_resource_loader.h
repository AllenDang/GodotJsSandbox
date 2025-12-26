#ifndef GODOT_JS_RUNTIME_GLTF_RESOURCE_LOADER_H
#define GODOT_JS_RUNTIME_GLTF_RESOURCE_LOADER_H

#include <godot_cpp/classes/resource_format_loader.hpp>

namespace jsb {

// GLTFResourceLoader handles loading .glb/.gltf files from user:// directory
// This enables tscn files to directly reference glb files without Godot's import system
class GLTFResourceLoader : public godot::ResourceFormatLoader {
    GDCLASS(GLTFResourceLoader, godot::ResourceFormatLoader);

public:
    GLTFResourceLoader();
    ~GLTFResourceLoader();

    // ResourceFormatLoader interface
    virtual godot::PackedStringArray _get_recognized_extensions() const override;
    virtual bool _handles_type(const godot::StringName &p_type) const override;
    virtual godot::String _get_resource_type(const godot::String &p_path) const override;
    virtual godot::Variant _load(const godot::String &p_path, const godot::String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const override;

protected:
    static void _bind_methods();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_GLTF_RESOURCE_LOADER_H

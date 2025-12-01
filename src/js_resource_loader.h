#ifndef GODOT_JS_RUNTIME_JS_RESOURCE_LOADER_H
#define GODOT_JS_RUNTIME_JS_RESOURCE_LOADER_H

#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/resource_format_saver.hpp>

namespace jsb {

// JSResourceLoader handles loading .js files as JSScript resources
class JSResourceLoader : public godot::ResourceFormatLoader {
    GDCLASS(JSResourceLoader, godot::ResourceFormatLoader);

public:
    JSResourceLoader();
    ~JSResourceLoader();

    // ResourceFormatLoader interface
    virtual godot::PackedStringArray _get_recognized_extensions() const override;
    virtual bool _handles_type(const godot::StringName &p_type) const override;
    virtual godot::String _get_resource_type(const godot::String &p_path) const override;
    virtual godot::Variant _load(const godot::String &p_path, const godot::String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const override;

protected:
    static void _bind_methods();
};

// JSResourceSaver handles saving JSScript resources as .js files
class JSResourceSaver : public godot::ResourceFormatSaver {
    GDCLASS(JSResourceSaver, godot::ResourceFormatSaver);

public:
    JSResourceSaver();
    ~JSResourceSaver();

    // ResourceFormatSaver interface
    virtual godot::PackedStringArray _get_recognized_extensions(const godot::Ref<godot::Resource> &p_resource) const override;
    virtual bool _recognize(const godot::Ref<godot::Resource> &p_resource) const override;
    virtual godot::Error _save(const godot::Ref<godot::Resource> &p_resource, const godot::String &p_path, uint32_t p_flags) override;

protected:
    static void _bind_methods();
};

} // namespace jsb

#endif // GODOT_JS_RUNTIME_JS_RESOURCE_LOADER_H

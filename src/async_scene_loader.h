#ifndef GODOT_JS_RUNTIME_ASYNC_SCENE_LOADER_H
#define GODOT_JS_RUNTIME_ASYNC_SCENE_LOADER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/array.hpp>

namespace jsb {

class JSSandbox;

// AsyncSceneLoader handles async loading of scenes with sandbox isolation
// Usage:
//   var loader = sandbox.load_scene_async("res://scene.tscn")
//   loader.progress_changed.connect(func(progress, stage): ...)
//   loader.completed.connect(func(scene): ...)
//   loader.failed.connect(func(error): ...)
//   # Call poll() each frame or use await loader.wait()
class AsyncSceneLoader : public godot::RefCounted {
    GDCLASS(AsyncSceneLoader, godot::RefCounted);

public:
    enum LoadStage {
        STAGE_NOT_STARTED = 0,
        STAGE_LOADING_SCENE = 1,
        STAGE_INSTANTIATING = 2,
        STAGE_LOADING_SCRIPTS = 3,
        STAGE_ATTACHING = 4,
        STAGE_COMPLETED = 5,
        STAGE_FAILED = 6
    };

    AsyncSceneLoader();
    ~AsyncSceneLoader();

    // Start loading a scene (called by JSSandbox)
    void start(JSSandbox* sandbox, const godot::String& scene_path);

    // Poll loading progress - call each frame
    // Returns true when loading is complete (success or failure)
    bool poll();

    // Get current progress (0.0 to 1.0)
    float get_progress() const { return progress_; }

    // Get current loading stage
    LoadStage get_stage() const { return stage_; }
    godot::String get_stage_name() const;

    // Check if loading is complete
    bool is_completed() const { return stage_ == STAGE_COMPLETED; }
    bool is_failed() const { return stage_ == STAGE_FAILED; }
    bool is_loading() const { return stage_ > STAGE_NOT_STARTED && stage_ < STAGE_COMPLETED; }

    // Get results
    godot::Node* get_scene() const { return loaded_scene_; }
    godot::String get_error() const { return error_message_; }

protected:
    static void _bind_methods();

private:
    JSSandbox* sandbox_ = nullptr;
    godot::String scene_path_;

    LoadStage stage_ = STAGE_NOT_STARTED;
    float progress_ = 0.0f;
    godot::String error_message_;

    // Loading state
    godot::Ref<godot::PackedScene> packed_scene_;
    godot::Node* loaded_scene_ = nullptr;

    // Script loading state
    godot::Array script_paths_;  // Paths of JS scripts to load
    godot::Array script_contents_;  // Loaded script contents
    int current_script_index_ = 0;

    // Internal methods
    void collect_script_paths(godot::Node* node);
    void reattach_scripts_with_content(godot::Node* node);
    void emit_progress();
};

} // namespace jsb

VARIANT_ENUM_CAST(jsb::AsyncSceneLoader::LoadStage);

#endif // GODOT_JS_RUNTIME_ASYNC_SCENE_LOADER_H

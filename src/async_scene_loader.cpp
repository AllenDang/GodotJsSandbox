#include "async_scene_loader.h"
#include "js_sandbox.h"
#include "js_script.h"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace jsb {

AsyncSceneLoader::AsyncSceneLoader() {
}

AsyncSceneLoader::~AsyncSceneLoader() {
    // Note: loaded_scene_ ownership is transferred to caller via get_scene()
}

void AsyncSceneLoader::start(JSSandbox* sandbox, const String& scene_path) {
    sandbox_ = sandbox;
    scene_path_ = scene_path;
    stage_ = STAGE_LOADING_SCENE;
    progress_ = 0.0f;
    error_message_ = "";

    // Start threaded loading of the PackedScene
    Error err = ResourceLoader::get_singleton()->load_threaded_request(scene_path_, "PackedScene");
    if (err != OK) {
        stage_ = STAGE_FAILED;
        error_message_ = "Failed to start loading scene: " + scene_path_;
        emit_signal("failed", error_message_);
        return;
    }

    emit_progress();
}

bool AsyncSceneLoader::poll() {
    if (stage_ == STAGE_COMPLETED || stage_ == STAGE_FAILED) {
        return true;
    }

    switch (stage_) {
        case STAGE_LOADING_SCENE: {
            // Check threaded load status
            Array progress_arr;
            ResourceLoader::ThreadLoadStatus status =
                ResourceLoader::get_singleton()->load_threaded_get_status(scene_path_, progress_arr);

            if (status == ResourceLoader::THREAD_LOAD_IN_PROGRESS) {
                if (progress_arr.size() > 0) {
                    progress_ = float(progress_arr[0]) * 0.4f;  // Scene loading is 40% of total
                }
                emit_progress();
                return false;
            } else if (status == ResourceLoader::THREAD_LOAD_LOADED) {
                // Scene loaded, get the resource
                Ref<Resource> res = ResourceLoader::get_singleton()->load_threaded_get(scene_path_);
                packed_scene_ = res;

                if (!packed_scene_.is_valid()) {
                    stage_ = STAGE_FAILED;
                    error_message_ = "Failed to load scene as PackedScene: " + scene_path_;
                    emit_signal("failed", error_message_);
                    return true;
                }

                // Move to instantiation stage
                stage_ = STAGE_INSTANTIATING;
                progress_ = 0.4f;
                emit_progress();
                return false;
            } else {
                // Failed or invalid
                stage_ = STAGE_FAILED;
                error_message_ = "Scene loading failed: " + scene_path_;
                emit_signal("failed", error_message_);
                return true;
            }
        }

        case STAGE_INSTANTIATING: {
            // Instantiate the scene (this is synchronous but usually fast)
            loaded_scene_ = packed_scene_->instantiate();
            if (!loaded_scene_) {
                stage_ = STAGE_FAILED;
                error_message_ = "Failed to instantiate scene: " + scene_path_;
                emit_signal("failed", error_message_);
                return true;
            }

            // Collect all JS script paths that need to be loaded
            script_paths_.clear();
            script_contents_.clear();
            current_script_index_ = 0;
            collect_script_paths(loaded_scene_);

            if (script_paths_.size() == 0) {
                // No scripts to load, go directly to completion
                stage_ = STAGE_COMPLETED;
                progress_ = 1.0f;
                emit_progress();
                emit_signal("completed", loaded_scene_);
                return true;
            }

            // Move to script loading stage
            stage_ = STAGE_LOADING_SCRIPTS;
            progress_ = 0.5f;
            emit_progress();
            return false;
        }

        case STAGE_LOADING_SCRIPTS: {
            // Load scripts one at a time (could be optimized to load multiple in parallel)
            if (current_script_index_ < script_paths_.size()) {
                String path = script_paths_[current_script_index_];

                // Load script content
                Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
                if (file.is_valid()) {
                    String content = file->get_as_text();
                    file->close();
                    script_contents_.push_back(content);
                } else {
                    // Script not found - add empty content and log warning
                    script_contents_.push_back(String());
                    UtilityFunctions::print_rich("[color=yellow]Warning: Could not load script: ", path, "[/color]");
                }

                current_script_index_++;

                // Update progress (scripts are 40% of total, from 0.5 to 0.9)
                float script_progress = float(current_script_index_) / float(script_paths_.size());
                progress_ = 0.5f + script_progress * 0.4f;
                emit_progress();
                return false;
            }

            // All scripts loaded, move to attaching stage
            stage_ = STAGE_ATTACHING;
            progress_ = 0.9f;
            emit_progress();
            return false;
        }

        case STAGE_ATTACHING: {
            // Reattach all scripts to the sandbox
            // This replaces each JSScript with a sandbox-bound version
            if (sandbox_) {
                reattach_scripts_with_content(loaded_scene_);
            }

            // Done!
            stage_ = STAGE_COMPLETED;
            progress_ = 1.0f;
            emit_progress();
            emit_signal("completed", loaded_scene_);
            return true;
        }

        default:
            return true;
    }
}

void AsyncSceneLoader::collect_script_paths(Node* node) {
    if (!node) return;

    // Check if this node has a JSScript
    Ref<Script> script = node->get_script();
    if (script.is_valid()) {
        const JSScript* js_script = Object::cast_to<JSScript>(script.ptr());
        if (js_script) {
            String path = js_script->get_path();
            if (!path.is_empty() && script_paths_.find(path) < 0) {
                script_paths_.push_back(path);
            }
        }
    }

    // Recurse to children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            collect_script_paths(child);
        }
    }
}

void AsyncSceneLoader::reattach_scripts_with_content(Node* node) {
    if (!node || !sandbox_) return;

    // Check if this node has a JSScript
    Ref<Script> current_script = node->get_script();
    if (current_script.is_valid()) {
        const JSScript* js_script = Object::cast_to<JSScript>(current_script.ptr());
        if (js_script) {
            String script_path = js_script->get_path();

            // Find the pre-loaded content for this script
            int path_index = script_paths_.find(script_path);
            if (path_index >= 0 && path_index < script_contents_.size()) {
                String source_code = script_contents_[path_index];

                if (!source_code.is_empty()) {
                    // Create new script using sandbox's context
                    Ref<Script> sandbox_script = sandbox_->create_script(source_code);
                    if (sandbox_script.is_valid()) {
                        JSScript* new_js_script = Object::cast_to<JSScript>(sandbox_script.ptr());
                        if (new_js_script && !script_path.is_empty()) {
                            new_js_script->set_path(script_path);
                        }
                        node->set_script(sandbox_script);
                    }
                }
            }
        }
    }

    // Recurse to children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            reattach_scripts_with_content(child);
        }
    }
}

String AsyncSceneLoader::get_stage_name() const {
    switch (stage_) {
        case STAGE_NOT_STARTED: return "not_started";
        case STAGE_LOADING_SCENE: return "loading_scene";
        case STAGE_INSTANTIATING: return "instantiating";
        case STAGE_LOADING_SCRIPTS: return "loading_scripts";
        case STAGE_ATTACHING: return "attaching";
        case STAGE_COMPLETED: return "completed";
        case STAGE_FAILED: return "failed";
        default: return "unknown";
    }
}

void AsyncSceneLoader::emit_progress() {
    emit_signal("progress_changed", progress_, get_stage_name());
}

void AsyncSceneLoader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("poll"), &AsyncSceneLoader::poll);
    ClassDB::bind_method(D_METHOD("get_progress"), &AsyncSceneLoader::get_progress);
    ClassDB::bind_method(D_METHOD("get_stage"), &AsyncSceneLoader::get_stage);
    ClassDB::bind_method(D_METHOD("get_stage_name"), &AsyncSceneLoader::get_stage_name);
    ClassDB::bind_method(D_METHOD("is_completed"), &AsyncSceneLoader::is_completed);
    ClassDB::bind_method(D_METHOD("is_failed"), &AsyncSceneLoader::is_failed);
    ClassDB::bind_method(D_METHOD("is_loading"), &AsyncSceneLoader::is_loading);
    ClassDB::bind_method(D_METHOD("get_scene"), &AsyncSceneLoader::get_scene);
    ClassDB::bind_method(D_METHOD("get_error"), &AsyncSceneLoader::get_error);

    // Signals
    ADD_SIGNAL(MethodInfo("progress_changed",
        PropertyInfo(Variant::FLOAT, "progress"),
        PropertyInfo(Variant::STRING, "stage")));

    ADD_SIGNAL(MethodInfo("completed",
        PropertyInfo(Variant::OBJECT, "scene", PROPERTY_HINT_RESOURCE_TYPE, "Node")));

    ADD_SIGNAL(MethodInfo("failed",
        PropertyInfo(Variant::STRING, "error")));

    // Enum constants
    BIND_ENUM_CONSTANT(STAGE_NOT_STARTED);
    BIND_ENUM_CONSTANT(STAGE_LOADING_SCENE);
    BIND_ENUM_CONSTANT(STAGE_INSTANTIATING);
    BIND_ENUM_CONSTANT(STAGE_LOADING_SCRIPTS);
    BIND_ENUM_CONSTANT(STAGE_ATTACHING);
    BIND_ENUM_CONSTANT(STAGE_COMPLETED);
    BIND_ENUM_CONSTANT(STAGE_FAILED);
}

} // namespace jsb

#include "js_sandbox.h"
#include "js_script.h"
#include "scene_saver.h"
#include "scene_loader.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

JSSandbox::JSSandbox() {
    initialize();
}

JSSandbox::~JSSandbox() {
    // Unregister logger from OS before destruction
    if (logger_.is_valid()) {
        logger_->set_sandbox(nullptr);
        OS::get_singleton()->remove_logger(logger_);
        logger_.unref();
    }

    // Explicit cleanup order to ensure JS resources are freed before context
    // SignalRegistry must cleanup while JSContext is still valid
    if (signal_registry_) {
        signal_registry_->cleanup_all();
    }

    // Free all RIDs before RenderingDevice objects are destroyed
    if (rid_registry_) {
        rid_registry_->free_all_rids();
    }

    // Clear script references before context destruction
    attached_scripts_.clear();

    // Note: unique_ptr members are destroyed in reverse declaration order
    // context_ is destroyed last, which is correct
}

bool JSSandbox::initialize() {
    object_registry_ = std::make_unique<ObjectRegistry>();
    array_registry_ = std::make_unique<ArrayRegistry>();
    sandbox_config_ = std::make_unique<SandboxConfig>();
    execution_limiter_ = std::make_unique<ExecutionLimiter>();
    safe_wrapper_ = std::make_unique<SafeWrapper>();
    signal_registry_ = std::make_unique<SignalRegistry>();
    deletion_tracker_ = std::make_unique<DeletionTracker>();
    rid_registry_ = std::make_unique<RidRegistry>();

    // Configure SafeWrapper
    safe_wrapper_->set_object_registry(object_registry_.get());
    safe_wrapper_->set_sandbox_config(sandbox_config_.get());
    safe_wrapper_->set_execution_limiter(execution_limiter_.get());
    safe_wrapper_->set_deletion_tracker(deletion_tracker_.get());

    // Configure DeletionTracker
    deletion_tracker_->set_object_registry(object_registry_.get());
    deletion_tracker_->set_signal_registry(signal_registry_.get());

    context_ = std::make_unique<QuickJSContext>();

    // Configure context with all sandbox components
    context_->set_object_registry(object_registry_.get());
    context_->set_array_registry(array_registry_.get());
    context_->set_sandbox_config(sandbox_config_.get());
    context_->set_execution_limiter(execution_limiter_.get());
    context_->set_safe_wrapper(safe_wrapper_.get());
    context_->set_signal_registry(signal_registry_.get());
    context_->set_rid_registry(rid_registry_.get());

    if (!context_->initialize()) {
        last_error_ = "Failed to initialize QuickJS context";
        return false;
    }

    // Configure SignalRegistry with context
    signal_registry_->set_context(context_->ctx());
    signal_registry_->set_quickjs_context(context_.get());
    signal_registry_->set_object_registry(object_registry_.get());

    // Create and register logger to capture Godot engine errors
    logger_.instantiate();
    logger_->set_sandbox(this);
    OS::get_singleton()->add_logger(logger_);

    return true;
}

void JSSandbox::set_timeout_ms(int ms) {
    if (execution_limiter_) {
        execution_limiter_->set_timeout_ms(ms);
    }
    if (context_) {
        context_->set_timeout_ms(ms);
    }
}

void JSSandbox::set_memory_limit_mb(int mb) {
    if (execution_limiter_) {
        execution_limiter_->set_memory_limit_mb(mb);
    }
    if (context_) {
        context_->set_memory_limit(static_cast<size_t>(mb) * 1024 * 1024);
    }
}

void JSSandbox::set_write_ops_per_frame(int count) {
    if (execution_limiter_) {
        execution_limiter_->set_write_ops_per_frame(count);
    }
}

void JSSandbox::set_heavy_ops_per_frame(int count) {
    if (execution_limiter_) {
        execution_limiter_->set_heavy_ops_per_frame(count);
    }
}

void JSSandbox::reset_frame_counters() {
    if (execution_limiter_) {
        execution_limiter_->reset_frame_counters();
    }
}

Error JSSandbox::load_blocklist(const String &path) {
    if (!sandbox_config_) {
        return ERR_UNCONFIGURED;
    }
    return sandbox_config_->load(path);
}

Variant JSSandbox::eval(const String &code) {
    if (!context_ || !context_->is_valid()) {
        add_error("sandbox", "Sandbox not initialized", "<eval>");
        return Variant();
    }

    if (execution_limiter_) {
        execution_limiter_->begin_execution();
    }

    Variant result;
    String error;

    bool success = context_->eval(code, "<eval>", result, error);

    if (execution_limiter_) {
        execution_limiter_->end_execution();
    }

    if (!success) {
        // Get structured error info from QuickJS
        auto info = context_->get_exception_info();
        add_error("javascript", error, info.file.is_empty() ? "<eval>" : info.file, info.line, info.column);
    }

    // Flush any Godot errors that occurred during execution
    if (logger_.is_valid()) {
        logger_->flush_errors();
    }

    if (!success) {
        return Variant();
    }

    return result;
}

Variant JSSandbox::eval_module(const String &code, const String &filename) {
    if (!context_ || !context_->is_valid()) {
        add_error("sandbox", "Sandbox not initialized", filename);
        return Variant();
    }

    // Validate filename for module resolution
    String module_path = filename;
    if (!module_path.begins_with("res://") && !module_path.begins_with("user://")) {
        // Default to user:// for sandbox safety
        module_path = "user://" + module_path;
    }

    if (execution_limiter_) {
        execution_limiter_->begin_execution();
    }

    Variant result;
    String error;

    bool success = context_->eval_module(code, module_path, result, error);

    if (execution_limiter_) {
        execution_limiter_->end_execution();
    }

    if (!success) {
        auto info = context_->get_exception_info();
        add_error("javascript", error, info.file.is_empty() ? module_path : info.file, info.line, info.column);
    }

    // Flush any Godot errors that occurred during execution
    if (logger_.is_valid()) {
        logger_->flush_errors();
    }

    if (!success) {
        return Variant();
    }

    return result;
}

Variant JSSandbox::eval_file(const String &path) {
    if (!context_ || !context_->is_valid()) {
        add_error("sandbox", "Sandbox not initialized", path);
        return Variant();
    }

    // Check path security
    if (sandbox_config_ && !sandbox_config_->is_path_allowed(path)) {
        add_error("security", "Path not allowed: " + path, path);
        return Variant();
    }

    if (execution_limiter_) {
        execution_limiter_->begin_execution();
    }

    Variant result;
    String error;

    bool success = context_->eval_file(path, result, error);

    if (execution_limiter_) {
        execution_limiter_->end_execution();
    }

    if (!success) {
        auto info = context_->get_exception_info();
        add_error("javascript", error, info.file.is_empty() ? path : info.file, info.line, info.column);
    }

    // Flush any Godot errors that occurred during execution
    if (logger_.is_valid()) {
        logger_->flush_errors();
    }

    if (!success) {
        return Variant();
    }

    return result;
}

void JSSandbox::set_global(const String &name, const Variant &value) {
    if (!context_ || !context_->is_valid()) {
        return;
    }

    // If setting a Node, register it
    if (value.get_type() == Variant::OBJECT) {
        Object *obj = value;
        const Node *node = Object::cast_to<Node>(obj);
        if (node && object_registry_) {
            object_registry_->create_handle(obj);
        }
    }

    context_->set_global(name, value);
}

Variant JSSandbox::get_global(const String &name) {
    if (!context_ || !context_->is_valid()) {
        return Variant();
    }

    return context_->get_global(name);
}

Error JSSandbox::save_level(Node *root, const String &directory) {
    if (!root) {
        last_error_ = "Root node is null";
        return ERR_INVALID_PARAMETER;
    }

    SceneSaver::SaveOptions options;
    options.include_external_resources = false;
    options.fail_on_gdscript = true;

    SceneSaver::SaveResult result = SceneSaver::save(root, directory, options);

    if (result.error != OK) {
        last_error_ = result.error_message;
        return result.error;
    }

    // Report warnings
    for (int i = 0; i < result.warnings.size(); i++) {
        UtilityFunctions::print_rich("[color=yellow]Warning: ", result.warnings[i], "[/color]");
    }

    // Emit signal
    emit_signal("level_saved", result.scene_path);

    return OK;
}

Node* JSSandbox::load_level(const String& directory) {
    SceneLoader::LoadResult result = SceneLoader::load(this, directory);

    if (result.error != OK) {
        last_error_ = result.error_message;
        return nullptr;
    }

    // Report warnings
    for (int i = 0; i < result.warnings.size(); i++) {
        UtilityFunctions::print_rich("[color=yellow]Warning: ", result.warnings[i], "[/color]");
    }

    // Emit signal
    emit_signal("level_loaded", directory, result.loaded_scripts.size());

    return result.root;
}

Array JSSandbox::get_created_nodes() {
    Array result;

    if (!object_registry_) {
        return result;
    }

    // Get only JS-created node IDs from the registry
    Vector<uint64_t> js_node_ids = object_registry_->get_js_created_node_ids();

    for (int i = 0; i < js_node_ids.size(); i++) {
        uint64_t obj_id = js_node_ids[i];
        Object* obj = ObjectDB::get_instance(ObjectID(obj_id));
        if (obj) {
            Node* node = Object::cast_to<Node>(obj);
            if (node) {
                result.push_back(node);
            }
        }
    }

    return result;
}

Dictionary JSSandbox::get_attached_scripts() {
    Dictionary result;

    // Get all valid nodes with scripts
    for (const KeyValue<uint64_t, String>& E : attached_scripts_) {
        Object* obj = ObjectDB::get_instance(ObjectID(E.key));
        if (obj) {
            Node* node = Object::cast_to<Node>(obj);
            if (node) {
                result[node] = E.value;
            }
        }
    }

    return result;
}

bool JSSandbox::is_valid() const {
    return context_ && context_->is_valid();
}

Ref<Script> JSSandbox::create_script(const String &source_code) {
    JSScript* script = memnew(JSScript);
    // Pass Ref<JSSandbox> to keep sandbox alive as long as script exists
    script->set_sandbox(Ref<JSSandbox>(this));
    script->set_source_code(source_code);
    return Ref<Script>(script);
}

Node* JSSandbox::load_scene(const String &scene_path) {
    if (!context_ || !context_->is_valid()) {
        add_error("sandbox", "Sandbox not initialized", scene_path);
        return nullptr;
    }

    // Check if scene file exists
    if (!FileAccess::file_exists(scene_path)) {
        add_error("scene", "Scene file not found: " + scene_path, scene_path);
        return nullptr;
    }

    // Try to read and validate the scene file first
    Ref<FileAccess> file = FileAccess::open(scene_path, FileAccess::READ);
    if (!file.is_valid()) {
        add_error("scene", "Cannot read scene file: " + scene_path, scene_path);
        return nullptr;
    }
    String scene_content = file->get_as_text();
    file->close();

    // Basic scene format validation
    if (!scene_content.begins_with("[gd_scene") && !scene_content.begins_with("[gd_resource")) {
        add_error("scene", "Invalid scene format - must start with [gd_scene or [gd_resource", scene_path, 1);
        return nullptr;
    }

    // Validate external resource references before loading
    int line_num = 0;
    PackedStringArray lines = scene_content.split("\n");
    for (int i = 0; i < lines.size(); i++) {
        line_num = i + 1;
        String line = lines[i].strip_edges();

        // Check for ext_resource paths
        if (line.begins_with("[ext_resource")) {
            int path_start = line.find("path=\"");
            if (path_start != -1) {
                path_start += 6;
                int path_end = line.find("\"", path_start);
                if (path_end != -1) {
                    String resource_path = line.substr(path_start, path_end - path_start);
                    // Check if JS script exists
                    if (resource_path.ends_with(".js") && !FileAccess::file_exists(resource_path)) {
                        add_error("resource", "Referenced JS script not found: " + resource_path, scene_path, line_num);
                        // Continue to collect all errors
                    }
                }
            }
        }
    }

    // Load the PackedScene
    Ref<PackedScene> packed_scene = ResourceLoader::get_singleton()->load(scene_path, "PackedScene");
    if (!packed_scene.is_valid()) {
        add_error("scene", "Failed to parse scene - check for syntax errors", scene_path);
        return nullptr;
    }

    // Instantiate the scene
    Node* root = packed_scene->instantiate();
    if (!root) {
        add_error("scene", "Failed to instantiate scene - check node types and properties", scene_path);
        return nullptr;
    }

    // Reattach all JS scripts to use this sandbox's context
    reattach_scripts_recursive(root);

    return root;
}

Ref<AsyncSceneLoader> JSSandbox::load_scene_async(const String &scene_path) {
    Ref<AsyncSceneLoader> loader;
    loader.instantiate();
    loader->start(this, scene_path);
    return loader;
}

void JSSandbox::reattach_scripts_recursive(Node* node) {
    if (!node) return;

    // Check if this node has a script attached
    Ref<Script> current_script = node->get_script();
    if (current_script.is_valid()) {
        // Check if it's a JSScript
        JSScript* js_script = Object::cast_to<JSScript>(current_script.ptr());
        if (js_script) {
            String script_path = js_script->get_path();
            String source_code = js_script->_get_source_code();

            // If source is empty, try to load from path
            if (source_code.is_empty() && !script_path.is_empty()) {
                Ref<FileAccess> file = FileAccess::open(script_path, FileAccess::READ);
                if (file.is_valid()) {
                    source_code = file->get_as_text();
                    file->close();
                } else {
                    add_error("script", "Cannot read JS script file: " + script_path, script_path);
                }
            }

            if (!source_code.is_empty()) {
                // Create new script using this sandbox's context
                Ref<Script> sandbox_script = create_script(source_code);
                if (sandbox_script.is_valid()) {
                    // Set the path on the new script for debugging
                    JSScript* new_js_script = Object::cast_to<JSScript>(sandbox_script.ptr());
                    if (new_js_script && !script_path.is_empty()) {
                        new_js_script->set_path(script_path);
                    }

                    // Replace the script with sandbox version
                    node->set_script(sandbox_script);

                    // Track the attached script
                    attached_scripts_[node->get_instance_id()] = script_path;
                } else {
                    add_error("script", "Failed to create sandbox script for: " + script_path, script_path);
                }
            } else if (!script_path.is_empty()) {
                add_error("script", "Empty script source for: " + script_path, script_path);
            }
        }
    }

    // Recursively process children
    TypedArray<Node> children = node->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i].operator Object*());
        if (child) {
            reattach_scripts_recursive(child);
        }
    }
}

void JSSandbox::add_error(const String& type, const String& message,
                          const String& file, int line, int column) {
    Dictionary error;
    error["type"] = type;
    error["message"] = message;
    error["file"] = file;
    error["line"] = line;
    error["column"] = column;
    errors_.push_back(error);
    last_error_ = message;
    emit_signal("error_occurred", message, line, column);
}

void JSSandbox::clear_errors() {
    errors_.clear();
    last_error_ = "";
}

void JSSandbox::reset() {
    attached_scripts_.clear();
    last_error_ = "";
    errors_.clear();

    // Clean up signal connections first
    if (signal_registry_) {
        signal_registry_->cleanup_all();
    }

    // Clean up deletion tracking
    if (deletion_tracker_) {
        deletion_tracker_->clear();
    }

    // Free all RIDs before clearing object registry
    // (RenderingDevice objects must still be valid to free their RIDs)
    if (rid_registry_) {
        rid_registry_->free_all_rids();
    }

    // Clean up object registry
    if (object_registry_) {
        object_registry_->clear_all();
    }

    // Clean up array registry (PackedArrays, RID handles, math type handles)
    if (array_registry_) {
        array_registry_->clear_all();
    }

    // Reset context
    context_.reset();
    context_ = std::make_unique<QuickJSContext>();

    context_->set_object_registry(object_registry_.get());
    context_->set_array_registry(array_registry_.get());
    context_->set_sandbox_config(sandbox_config_.get());
    context_->set_execution_limiter(execution_limiter_.get());
    context_->set_safe_wrapper(safe_wrapper_.get());
    context_->set_signal_registry(signal_registry_.get());
    context_->set_rid_registry(rid_registry_.get());

    context_->initialize();

    // Re-configure SignalRegistry with new context
    if (signal_registry_) {
        signal_registry_->set_context(context_->ctx());
        signal_registry_->set_quickjs_context(context_.get());
    }
}

int JSSandbox::execute_pending_jobs() {
    if (!context_ || !context_->is_valid()) {
        return 0;
    }

    JSRuntime* rt = context_->rt();
    JSContext* ctx = nullptr;
    int executed = 0;

    // Execute all pending jobs (microtasks, promise callbacks)
    while (true) {
        int ret = JS_ExecutePendingJob(rt, &ctx);
        if (ret <= 0) {
            // ret == 0 means no more jobs, ret < 0 means error
            break;
        }
        executed++;
    }

    // Flush any errors captured by the logger
    if (logger_.is_valid()) {
        logger_->flush_errors();
    }

    return executed;
}

void JSSandbox::_bind_methods() {
    // Configuration
    ClassDB::bind_method(D_METHOD("set_timeout_ms", "ms"), &JSSandbox::set_timeout_ms);
    ClassDB::bind_method(D_METHOD("set_memory_limit_mb", "mb"), &JSSandbox::set_memory_limit_mb);
    ClassDB::bind_method(D_METHOD("load_blocklist", "path"), &JSSandbox::load_blocklist);

    // Rate limiting configuration (per PRD Section 6.4)
    ClassDB::bind_method(D_METHOD("set_write_ops_per_frame", "count"), &JSSandbox::set_write_ops_per_frame);
    ClassDB::bind_method(D_METHOD("set_heavy_ops_per_frame", "count"), &JSSandbox::set_heavy_ops_per_frame);
    ClassDB::bind_method(D_METHOD("reset_frame_counters"), &JSSandbox::reset_frame_counters);

    // Execution
    ClassDB::bind_method(D_METHOD("eval", "code"), &JSSandbox::eval);
    ClassDB::bind_method(D_METHOD("eval_module", "code", "filename"), &JSSandbox::eval_module);
    ClassDB::bind_method(D_METHOD("eval_file", "path"), &JSSandbox::eval_file);

    // Global variables
    ClassDB::bind_method(D_METHOD("set_global", "name", "value"), &JSSandbox::set_global);
    ClassDB::bind_method(D_METHOD("get_global", "name"), &JSSandbox::get_global);

    // Script creation - creates scripts that use this sandbox's context
    ClassDB::bind_method(D_METHOD("create_script", "source_code"), &JSSandbox::create_script);

    // Scene loading with sandbox isolation
    ClassDB::bind_method(D_METHOD("load_scene", "scene_path"), &JSSandbox::load_scene);
    ClassDB::bind_method(D_METHOD("load_scene_async", "scene_path"), &JSSandbox::load_scene_async);

    // Level persistence
    ClassDB::bind_method(D_METHOD("save_level", "root", "directory"), &JSSandbox::save_level);
    ClassDB::bind_method(D_METHOD("load_level", "directory"), &JSSandbox::load_level);
    ClassDB::bind_method(D_METHOD("get_created_nodes"), &JSSandbox::get_created_nodes);
    ClassDB::bind_method(D_METHOD("get_attached_scripts"), &JSSandbox::get_attached_scripts);

    // Utility
    ClassDB::bind_method(D_METHOD("get_last_error"), &JSSandbox::get_last_error);
    ClassDB::bind_method(D_METHOD("get_all_errors"), &JSSandbox::get_all_errors);
    ClassDB::bind_method(D_METHOD("clear_errors"), &JSSandbox::clear_errors);
    ClassDB::bind_method(D_METHOD("is_valid"), &JSSandbox::is_valid);
    ClassDB::bind_method(D_METHOD("reset"), &JSSandbox::reset);

    // Async support
    ClassDB::bind_method(D_METHOD("execute_pending_jobs"), &JSSandbox::execute_pending_jobs);

    // Signals
    ADD_SIGNAL(MethodInfo("error_occurred",
        PropertyInfo(Variant::STRING, "message"),
        PropertyInfo(Variant::INT, "line"),
        PropertyInfo(Variant::INT, "column")));

    ADD_SIGNAL(MethodInfo("console_output",
        PropertyInfo(Variant::STRING, "message")));

    ADD_SIGNAL(MethodInfo("level_saved",
        PropertyInfo(Variant::STRING, "path")));

    ADD_SIGNAL(MethodInfo("level_loaded",
        PropertyInfo(Variant::STRING, "directory"),
        PropertyInfo(Variant::INT, "script_count")));
}

} // namespace jsb

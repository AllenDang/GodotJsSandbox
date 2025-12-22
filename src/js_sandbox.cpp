#include "js_sandbox.h"
#include "js_script.h"
#include "js_script_language.h"
#include "scene_saver.h"
#include "scene_loader.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

// ErrorEntry implementation
Dictionary ErrorEntry::to_dict() const {
    Dictionary d;
    d["id"] = id;
    d["type"] = type;
    d["severity"] = severity;
    d["message"] = message;
    d["file"] = file;
    d["line"] = line;
    d["column"] = column;
    d["stack_trace"] = stack_trace;
    d["trigger_context"] = trigger_context;
    d["phase"] = phase;
    d["timestamp"] = timestamp;
    d["last_occurrence"] = last_occurrence;
    d["occurrence_count"] = occurrence_count;
    return d;
}

String ErrorEntry::compute_id(const String& type, const String& message,
                               const String& file, int line) {
    // Create a unique ID by combining key fields
    // Normalize message to first 100 chars to group similar errors
    String normalized_msg = message.length() > 100 ? message.substr(0, 100) : message;
    String sep = "|";
    return type + sep + normalized_msg + sep + file + sep + String::num_int64(line);
}

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

    // Apply global constants from JSScriptLanguage (autoloads, singletons)
    JSScriptLanguage* language = JSScriptLanguage::get_singleton();
    if (language) {
        for (const KeyValue<StringName, Variant>& kv : language->get_global_constants()) {
            context_->set_global(String(kv.key), kv.value);
        }
        for (const KeyValue<StringName, Variant>& kv : language->get_named_global_constants()) {
            context_->set_global(String(kv.key), kv.value);
        }
    }

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

    // Track errors before load
    int errors_before = error_order_.size();
    current_phase_ = ExecutionPhase::LOAD;

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
        add_error("javascript", error, info.file.is_empty() ? "<eval>" : info.file,
                  info.line, info.column, info.stack);
    }

    // Flush any Godot errors that occurred during execution
    if (logger_.is_valid()) {
        logger_->flush_errors();
    }

    // Collect errors that occurred during load
    Array load_errors;
    for (int i = errors_before; i < error_order_.size(); i++) {
        const String& error_id = error_order_[i];
        if (error_map_.has(error_id)) {
            load_errors.push_back(error_map_[error_id].to_dict());
        }
    }

    // Emit load_completed signal
    emit_signal("load_completed", load_errors.is_empty(), load_errors);

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

    // Track errors before load
    int errors_before = error_order_.size();
    current_phase_ = ExecutionPhase::LOAD;

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
        add_error("javascript", error, info.file.is_empty() ? module_path : info.file,
                  info.line, info.column, info.stack);
    }

    // Flush any Godot errors that occurred during execution
    if (logger_.is_valid()) {
        logger_->flush_errors();
    }

    // Collect errors that occurred during load
    Array load_errors;
    for (int i = errors_before; i < error_order_.size(); i++) {
        const String& error_id = error_order_[i];
        if (error_map_.has(error_id)) {
            load_errors.push_back(error_map_[error_id].to_dict());
        }
    }

    // Emit load_completed signal
    emit_signal("load_completed", load_errors.is_empty(), load_errors);

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

    // Track errors before load
    int errors_before = error_order_.size();
    current_phase_ = ExecutionPhase::LOAD;

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
        add_error("javascript", error, info.file.is_empty() ? path : info.file,
                  info.line, info.column, info.stack);
    }

    // Flush any Godot errors that occurred during execution
    if (logger_.is_valid()) {
        logger_->flush_errors();
    }

    // Collect errors that occurred during load
    Array load_errors;
    for (int i = errors_before; i < error_order_.size(); i++) {
        const String& error_id = error_order_[i];
        if (error_map_.has(error_id)) {
            load_errors.push_back(error_map_[error_id].to_dict());
        }
    }

    // Emit load_completed signal
    emit_signal("load_completed", load_errors.is_empty(), load_errors);

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
    PackedStringArray lines = scene_content.split("\n");
    for (int i = 0; i < lines.size(); i++) {
        int line_num = i + 1;
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
        const JSScript* js_script = Object::cast_to<JSScript>(current_script.ptr());
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
                          const String& file, int line, int column,
                          const String& stack_trace, const String& severity) {
    // Compute unique ID for deduplication
    String error_id = ErrorEntry::compute_id(type, message, file, line);
    int64_t now = Time::get_singleton()->get_unix_time_from_system() * 1000;

    // Check if this error already exists
    if (error_map_.has(error_id)) {
        // Update existing error
        ErrorEntry& existing = error_map_[error_id];
        existing.occurrence_count++;
        existing.last_occurrence = now;
        // Don't emit runtime_error again for duplicates, but schedule batch update
        errors_updated_pending_ = true;
    } else {
        // Create new error entry
        ErrorEntry entry;
        entry.id = error_id;
        entry.type = type;
        entry.severity = severity;
        entry.message = message;
        entry.file = file;
        entry.line = line;
        entry.column = column;
        entry.stack_trace = stack_trace;
        entry.trigger_context = current_context_;
        entry.phase = phase_to_string(current_phase_);
        entry.timestamp = now;
        entry.last_occurrence = now;
        entry.occurrence_count = 1;

        error_map_[error_id] = entry;
        error_order_.push_back(error_id);

        // Emit runtime_error signal for new errors (with full context)
        emit_signal("runtime_error", entry.to_dict());
        errors_updated_pending_ = true;
    }

    last_error_ = message;

    // Legacy signal for backward compatibility
    emit_signal("error_occurred", type, message, file, line, column);
}

void JSSandbox::clear_errors() {
    error_map_.clear();
    error_order_.clear();
    last_error_ = "";
    errors_updated_pending_ = false;
}

void JSSandbox::reset() {
    attached_scripts_.clear();
    last_error_ = "";
    error_map_.clear();
    error_order_.clear();
    current_phase_ = ExecutionPhase::LOAD;
    current_context_ = "";
    init_phase_active_ = false;
    errors_updated_pending_ = false;

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

    // Emit batched errors_updated signal if pending
    if (errors_updated_pending_) {
        emit_errors_updated();
    }

    return executed;
}

// Phase management
void JSSandbox::set_phase(ExecutionPhase phase) {
    current_phase_ = phase;
}

String JSSandbox::phase_to_string(ExecutionPhase phase) const {
    switch (phase) {
        case ExecutionPhase::LOAD: return "load";
        case ExecutionPhase::INIT: return "init";
        case ExecutionPhase::RUNTIME: return "runtime";
        default: return "unknown";
    }
}

// Execution context tracking
void JSSandbox::set_execution_context(const String& context) {
    current_context_ = context;
}

void JSSandbox::clear_execution_context() {
    current_context_ = "";
}

// Init phase management
void JSSandbox::start_init_phase(float timeout_seconds) {
    current_phase_ = ExecutionPhase::INIT;
    init_phase_active_ = true;
    errors_at_init_start_ = error_order_.size();

    // Note: In a real implementation, you'd use a Timer node or SceneTree timer
    // For now, the caller is responsible for calling on_init_phase_timeout()
    // after the timeout, or the phase can be manually advanced with set_phase()
}

void JSSandbox::on_init_phase_timeout() {
    if (!init_phase_active_) {
        return;
    }

    init_phase_active_ = false;

    // Collect errors that occurred during init phase
    Array init_errors;
    for (int i = errors_at_init_start_; i < error_order_.size(); i++) {
        const String& error_id = error_order_[i];
        if (error_map_.has(error_id)) {
            init_errors.push_back(error_map_[error_id].to_dict());
        }
    }

    // Transition to runtime phase
    current_phase_ = ExecutionPhase::RUNTIME;

    // Emit init_completed signal
    bool success = init_errors.is_empty();
    emit_signal("init_completed", success, init_errors);
}

// Get all errors as array (for GDScript)
Array JSSandbox::get_all_errors() const {
    Array result;
    for (int i = 0; i < error_order_.size(); i++) {
        const String& error_id = error_order_[i];
        if (error_map_.has(error_id)) {
            result.push_back(error_map_[error_id].to_dict());
        }
    }
    return result;
}

Array JSSandbox::get_errors_for_ai() const {
    return get_all_errors();
}

// Emit batched errors_updated signal
void JSSandbox::emit_errors_updated() {
    if (!errors_updated_pending_) {
        return;
    }
    errors_updated_pending_ = false;
    emit_signal("errors_updated", get_all_errors());
}

// Generate markdown-formatted error report for AI consumption
String JSSandbox::get_error_report() const {
    if (error_order_.is_empty()) {
        return "No errors detected.";
    }

    String report = "## Errors Detected\n\n";

    int error_num = 1;
    for (int i = 0; i < error_order_.size(); i++) {
        const String& error_id = error_order_[i];
        if (!error_map_.has(error_id)) {
            continue;
        }

        const ErrorEntry& entry = error_map_[error_id];

        report += "### Error " + String::num_int64(error_num++) + ": " + entry.type + "\n";
        report += "- **Severity**: " + entry.severity + "\n";
        report += "- **File**: " + entry.file;
        if (entry.line > 0) {
            report += ":" + String::num_int64(entry.line);
            if (entry.column > 0) {
                report += ":" + String::num_int64(entry.column);
            }
        }
        report += "\n";
        report += "- **Message**: " + entry.message + "\n";

        if (!entry.trigger_context.is_empty()) {
            report += "- **Context**: Called during `" + entry.trigger_context + "`\n";
        }

        report += "- **Phase**: " + entry.phase + "\n";

        if (entry.occurrence_count > 1) {
            report += "- **Occurrences**: " + String::num_int64(entry.occurrence_count) + " times\n";
        }

        if (!entry.stack_trace.is_empty()) {
            report += "- **Stack Trace**:\n```\n" + entry.stack_trace + "\n```\n";
        }

        report += "\n";
    }

    // Add summary
    int total_occurrences = 0;
    int error_count = 0;
    int warning_count = 0;

    for (const KeyValue<String, ErrorEntry>& E : error_map_) {
        total_occurrences += E.value.occurrence_count;
        if (E.value.severity == "error") {
            error_count++;
        } else if (E.value.severity == "warning") {
            warning_count++;
        }
    }

    report += "---\n";
    report += "**Summary**: " + String::num_int64(error_order_.size()) + " unique issues";
    if (error_count > 0) {
        report += " (" + String::num_int64(error_count) + " errors";
        if (warning_count > 0) {
            report += ", " + String::num_int64(warning_count) + " warnings";
        }
        report += ")";
    }
    if (total_occurrences > error_order_.size()) {
        report += ", " + String::num_int64(total_occurrences) + " total occurrences";
    }
    report += "\n";

    return report;
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

    // Enhanced error reporting for AI feedback
    ClassDB::bind_method(D_METHOD("get_error_report"), &JSSandbox::get_error_report);
    ClassDB::bind_method(D_METHOD("get_errors_for_ai"), &JSSandbox::get_errors_for_ai);

    // Phase and context management
    ClassDB::bind_method(D_METHOD("set_execution_context", "context"), &JSSandbox::set_execution_context);
    ClassDB::bind_method(D_METHOD("clear_execution_context"), &JSSandbox::clear_execution_context);
    ClassDB::bind_method(D_METHOD("start_init_phase", "timeout_seconds"), &JSSandbox::start_init_phase);

    // Signals - Legacy (backward compatible)
    ADD_SIGNAL(MethodInfo("error_occurred",
        PropertyInfo(Variant::STRING, "type"),
        PropertyInfo(Variant::STRING, "message"),
        PropertyInfo(Variant::STRING, "file"),
        PropertyInfo(Variant::INT, "line"),
        PropertyInfo(Variant::INT, "column")));

    // Signals - Enhanced for AI feedback
    // Emitted for each new unique error (with full context)
    ADD_SIGNAL(MethodInfo("runtime_error",
        PropertyInfo(Variant::DICTIONARY, "error")));

    // Emitted as a batch after errors settle (debounced)
    ADD_SIGNAL(MethodInfo("errors_updated",
        PropertyInfo(Variant::ARRAY, "all_errors")));

    // Phase completion signals for auto-fix loop
    ADD_SIGNAL(MethodInfo("load_completed",
        PropertyInfo(Variant::BOOL, "success"),
        PropertyInfo(Variant::ARRAY, "errors")));

    ADD_SIGNAL(MethodInfo("init_completed",
        PropertyInfo(Variant::BOOL, "success"),
        PropertyInfo(Variant::ARRAY, "errors")));

    // Other signals
    ADD_SIGNAL(MethodInfo("console_output",
        PropertyInfo(Variant::STRING, "message")));

    ADD_SIGNAL(MethodInfo("level_saved",
        PropertyInfo(Variant::STRING, "path")));

    ADD_SIGNAL(MethodInfo("level_loaded",
        PropertyInfo(Variant::STRING, "directory"),
        PropertyInfo(Variant::INT, "script_count")));
}

} // namespace jsb

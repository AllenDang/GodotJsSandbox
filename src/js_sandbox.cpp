#include "js_sandbox.h"
#include "js_script.h"
#include "scene_saver.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

JSSandbox::JSSandbox() {
    initialize();
}

JSSandbox::~JSSandbox() {
    attached_scripts_.clear();
}

bool JSSandbox::initialize() {
    object_registry_ = std::make_unique<ObjectRegistry>();
    sandbox_config_ = std::make_unique<SandboxConfig>();
    execution_limiter_ = std::make_unique<ExecutionLimiter>();
    safe_wrapper_ = std::make_unique<SafeWrapper>();
    signal_registry_ = std::make_unique<SignalRegistry>();
    deletion_tracker_ = std::make_unique<DeletionTracker>();

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
    context_->set_sandbox_config(sandbox_config_.get());
    context_->set_execution_limiter(execution_limiter_.get());
    context_->set_safe_wrapper(safe_wrapper_.get());
    context_->set_signal_registry(signal_registry_.get());

    if (!context_->initialize()) {
        last_error_ = "Failed to initialize QuickJS context";
        return false;
    }

    // Configure SignalRegistry with context
    signal_registry_->set_context(context_->ctx());
    signal_registry_->set_quickjs_context(context_.get());
    signal_registry_->set_object_registry(object_registry_.get());

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

Error JSSandbox::load_blocklist(const String &path) {
    if (!sandbox_config_) {
        return ERR_UNCONFIGURED;
    }
    return sandbox_config_->load(path);
}

Variant JSSandbox::eval(const String &code) {
    if (!context_ || !context_->is_valid()) {
        last_error_ = "Sandbox not initialized";
        emit_signal("error_occurred", last_error_, 0, 0);
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
        last_error_ = error;
        emit_signal("error_occurred", error, 0, 0);
        return Variant();
    }

    return result;
}

Variant JSSandbox::eval_file(const String &path) {
    if (!context_ || !context_->is_valid()) {
        last_error_ = "Sandbox not initialized";
        emit_signal("error_occurred", last_error_, 0, 0);
        return Variant();
    }

    // Check path security
    if (sandbox_config_ && !sandbox_config_->is_path_allowed(path)) {
        last_error_ = "Path not allowed: " + path;
        emit_signal("error_occurred", last_error_, 0, 0);
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
        last_error_ = error;
        emit_signal("error_occurred", error, 0, 0);
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
    script->set_sandbox(this);
    script->set_source_code(source_code);
    return Ref<Script>(script);
}

void JSSandbox::reset() {
    attached_scripts_.clear();
    last_error_ = "";

    // Clean up signal connections first
    if (signal_registry_) {
        signal_registry_->cleanup_all();
    }

    // Clean up deletion tracking
    if (deletion_tracker_) {
        deletion_tracker_->clear();
    }

    // Clean up object registry
    if (object_registry_) {
        object_registry_->clear_all();
    }

    // Reset context
    context_.reset();
    context_ = std::make_unique<QuickJSContext>();

    context_->set_object_registry(object_registry_.get());
    context_->set_sandbox_config(sandbox_config_.get());
    context_->set_execution_limiter(execution_limiter_.get());
    context_->set_safe_wrapper(safe_wrapper_.get());
    context_->set_signal_registry(signal_registry_.get());

    context_->initialize();

    // Re-configure SignalRegistry with new context
    if (signal_registry_) {
        signal_registry_->set_context(context_->ctx());
    }
}

void JSSandbox::_bind_methods() {
    // Configuration
    ClassDB::bind_method(D_METHOD("set_timeout_ms", "ms"), &JSSandbox::set_timeout_ms);
    ClassDB::bind_method(D_METHOD("set_memory_limit_mb", "mb"), &JSSandbox::set_memory_limit_mb);
    ClassDB::bind_method(D_METHOD("load_blocklist", "path"), &JSSandbox::load_blocklist);

    // Execution
    ClassDB::bind_method(D_METHOD("eval", "code"), &JSSandbox::eval);
    ClassDB::bind_method(D_METHOD("eval_file", "path"), &JSSandbox::eval_file);

    // Global variables
    ClassDB::bind_method(D_METHOD("set_global", "name", "value"), &JSSandbox::set_global);
    ClassDB::bind_method(D_METHOD("get_global", "name"), &JSSandbox::get_global);

    // Script creation - creates scripts that use this sandbox's context
    ClassDB::bind_method(D_METHOD("create_script", "source_code"), &JSSandbox::create_script);

    // Level persistence
    ClassDB::bind_method(D_METHOD("save_level", "root", "directory"), &JSSandbox::save_level);
    ClassDB::bind_method(D_METHOD("get_created_nodes"), &JSSandbox::get_created_nodes);
    ClassDB::bind_method(D_METHOD("get_attached_scripts"), &JSSandbox::get_attached_scripts);

    // Utility
    ClassDB::bind_method(D_METHOD("get_last_error"), &JSSandbox::get_last_error);
    ClassDB::bind_method(D_METHOD("is_valid"), &JSSandbox::is_valid);
    ClassDB::bind_method(D_METHOD("reset"), &JSSandbox::reset);

    // Signals
    ADD_SIGNAL(MethodInfo("error_occurred",
        PropertyInfo(Variant::STRING, "message"),
        PropertyInfo(Variant::INT, "line"),
        PropertyInfo(Variant::INT, "column")));

    ADD_SIGNAL(MethodInfo("console_output",
        PropertyInfo(Variant::STRING, "message")));

    ADD_SIGNAL(MethodInfo("level_saved",
        PropertyInfo(Variant::STRING, "path")));
}

} // namespace jsb

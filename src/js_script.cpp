#include "js_script.h"
#include "js_script_instance.h"
#include "js_script_language.h"
#include "js_sandbox.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace jsb {

JSScript::JSScript() {
}

JSScript::~JSScript() {
}

void JSScript::_bind_methods() {
    // Bind source_code property so it can be set from GDScript
    ClassDB::bind_method(D_METHOD("set_source_code", "code"), &JSScript::set_source_code);
    ClassDB::bind_method(D_METHOD("get_source_code"), &JSScript::get_source_code);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "source_code", PROPERTY_HINT_MULTILINE_TEXT), "set_source_code", "get_source_code");
}

void JSScript::set_source_code(const String &p_code) {
    _set_source_code(p_code);
}

String JSScript::get_source_code() const {
    return _get_source_code();
}

bool JSScript::_can_instantiate() const {
    return is_valid_;
}

Ref<Script> JSScript::_get_base_script() const {
    return Ref<Script>();
}

StringName JSScript::_get_global_name() const {
    return StringName();
}

bool JSScript::_inherits_script(const Ref<Script> &p_script) const {
    return false;
}

StringName JSScript::_get_instance_base_type() const {
    return StringName("Object");
}

void* JSScript::_instance_create(Object *p_for_object) const {
    // Create our JSScriptInstance
    JSScriptInstance* instance = memnew(JSScriptInstance);
    instance->set_owner(p_for_object);
    instance->set_script(const_cast<JSScript*>(this));
    instance->initialize();

    const_cast<JSScript*>(this)->register_instance(p_for_object, instance);

    // Create the GDExtension script instance using script_instance_create3
    return JSScriptInstance::create_instance(instance);
}

void* JSScript::_placeholder_instance_create(Object *p_for_object) const {
    return _instance_create(p_for_object);
}

bool JSScript::_instance_has(Object *p_object) const {
    return instances_.has(p_object);
}

bool JSScript::_has_source_code() const {
    return !source_code_.is_empty();
}

String JSScript::_get_source_code() const {
    return source_code_;
}

void JSScript::_set_source_code(const String &p_code) {
    source_code_ = p_code;
    is_valid_ = parse_script();
}

Error JSScript::_reload(bool p_keep_state) {
    is_valid_ = parse_script();
    return is_valid_ ? OK : ERR_PARSE_ERROR;
}

TypedArray<Dictionary> JSScript::_get_documentation() const {
    return TypedArray<Dictionary>();
}

String JSScript::_get_class_icon_path() const {
    return String();
}

bool JSScript::_has_method(const StringName &p_method) const {
    return methods_.find(p_method) >= 0;
}

bool JSScript::_has_static_method(const StringName &p_method) const {
    return false;
}

Dictionary JSScript::_get_method_info(const StringName &p_method) const {
    Dictionary info;
    if (_has_method(p_method)) {
        info["name"] = p_method;
        info["args"] = Array();
        info["default_args"] = Array();
        info["flags"] = 0;
        info["return"] = Dictionary();
    }
    return info;
}

ScriptLanguage* JSScript::_get_language() const {
    return JSScriptLanguage::get_singleton();
}

bool JSScript::_has_script_signal(const StringName &p_signal) const {
    return signals_.find(p_signal) >= 0;
}

TypedArray<Dictionary> JSScript::_get_script_signal_list() const {
    TypedArray<Dictionary> list;
    for (int i = 0; i < signals_.size(); i++) {
        Dictionary signal;
        signal["name"] = signals_[i];
        signal["args"] = Array();
        list.push_back(signal);
    }
    return list;
}

bool JSScript::_has_property_default_value(const StringName &p_property) const {
    return properties_.has(p_property);
}

Variant JSScript::_get_property_default_value(const StringName &p_property) const {
    if (properties_.has(p_property)) {
        return properties_[p_property];
    }
    return Variant();
}

void JSScript::_update_exports() {
    // Parse exports from source
}

TypedArray<Dictionary> JSScript::_get_script_method_list() const {
    TypedArray<Dictionary> list;
    for (int i = 0; i < methods_.size(); i++) {
        Dictionary method;
        method["name"] = methods_[i];
        method["args"] = Array();
        method["default_args"] = Array();
        method["flags"] = 0;
        method["return"] = Dictionary();
        list.push_back(method);
    }
    return list;
}

TypedArray<Dictionary> JSScript::_get_script_property_list() const {
    TypedArray<Dictionary> list;
    for (const KeyValue<StringName, Variant> &E : properties_) {
        Dictionary prop;
        prop["name"] = E.key;
        prop["type"] = E.value.get_type();
        list.push_back(prop);
    }
    return list;
}

int32_t JSScript::_get_member_line(const StringName &p_member) const {
    // Simple search for member in source
    String search = String(p_member);
    int pos = source_code_.find(search);
    if (pos >= 0) {
        int line = 1;
        for (int i = 0; i < pos; i++) {
            if (source_code_[i] == '\n') {
                line++;
            }
        }
        return line;
    }
    return -1;
}

Dictionary JSScript::_get_constants() const {
    return Dictionary();
}

TypedArray<StringName> JSScript::_get_members() const {
    TypedArray<StringName> members;
    for (int i = 0; i < methods_.size(); i++) {
        members.push_back(methods_[i]);
    }
    return members;
}

Variant JSScript::_get_rpc_config() const {
    return Dictionary();
}

void JSScript::set_path(const String &p_path) {
    path_ = p_path;
}

void JSScript::register_instance(Object* p_object, JSScriptInstance* p_instance) {
    instances_[p_object] = p_instance;
}

void JSScript::unregister_instance(Object* p_object) {
    instances_.erase(p_object);
}

void JSScript::set_sandbox(JSSandbox* p_sandbox) {
    sandbox_ = p_sandbox;
}

bool JSScript::parse_script() {
    if (source_code_.is_empty()) {
        return false;
    }

    methods_.clear();
    signals_.clear();
    properties_.clear();

    // Parse all function declarations: "function name(" pattern
    // This captures both lifecycle methods (_ready, _process) and custom methods
    int pos = 0;
    while ((pos = source_code_.find("function ", pos)) >= 0) {
        pos += 9; // Skip "function "

        // Skip whitespace
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Read function name (valid JS identifier: [a-zA-Z_$][a-zA-Z0-9_$]*)
        int name_start = pos;
        while (pos < source_code_.length()) {
            char32_t c = source_code_[pos];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_' || c == '$') {
                pos++;
            } else {
                break;
            }
        }

        int name_end = pos;

        // Skip whitespace before '('
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Verify it's followed by '(' (it's actually a function declaration)
        if (name_end > name_start && pos < source_code_.length() && source_code_[pos] == '(') {
            String method_name = source_code_.substr(name_start, name_end - name_start);
            if (!method_name.is_empty()) {
                methods_.push_back(StringName(method_name));
            }
        }
    }

    // Look for signal declarations
    // Pattern: // @signal signal_name
    // or: // @signal signal_name(arg1, arg2)
    pos = 0;
    while ((pos = source_code_.find("@signal", pos)) >= 0) {
        pos += 7; // Skip "@signal"

        // Skip whitespace
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Read signal name
        int name_start = pos;
        while (pos < source_code_.length()) {
            char32_t c = source_code_[pos];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_') {
                pos++;
            } else {
                break;
            }
        }

        if (pos > name_start) {
            String signal_name = source_code_.substr(name_start, pos - name_start);
            if (!signal_name.is_empty()) {
                signals_.push_back(StringName(signal_name));
            }
        }
    }

    // Look for export declarations
    // Pattern: // @export var_name = default_value
    // or: /* @export */ let var_name = default_value
    pos = 0;
    while ((pos = source_code_.find("@export", pos)) >= 0) {
        // Find the variable name after @export
        int line_end = source_code_.find("\n", pos);
        if (line_end < 0) line_end = source_code_.length();

        String line = source_code_.substr(pos, line_end - pos);
        // Simple parsing - look for var/let/const followed by name
        int var_pos = line.find("var ");
        if (var_pos < 0) var_pos = line.find("let ");
        if (var_pos < 0) var_pos = line.find("const ");

        if (var_pos >= 0) {
            int name_start = var_pos + 4; // skip "var " or "let "
            if (line.substr(var_pos, 5) == "const") name_start = var_pos + 6;

            int name_end = name_start;
            while (name_end < line.length()) {
                char32_t c = line[name_end];
                if (c == ' ' || c == '=' || c == ';' || c == '\n') break;
                name_end++;
            }

            if (name_end > name_start) {
                String var_name = line.substr(name_start, name_end - name_start).strip_edges();
                if (!var_name.is_empty()) {
                    properties_[StringName(var_name)] = Variant();
                }
            }
        }
        pos = line_end;
    }

    is_valid_ = true;
    return true;
}

} // namespace jsb

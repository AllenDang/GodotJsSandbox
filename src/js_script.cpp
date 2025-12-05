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

JSScriptInstance* JSScript::get_instance(Object* p_object) const {
    if (instances_.has(p_object)) {
        return instances_[p_object];
    }
    return nullptr;
}

void JSScript::set_sandbox(const Ref<JSSandbox>& p_sandbox) {
    sandbox_ = p_sandbox;
}

bool JSScript::parse_script() {
    if (source_code_.is_empty()) {
        return false;
    }

    methods_.clear();
    signals_.clear();
    properties_.clear();

    // Method detection supports multiple patterns:
    //
    // Pattern 1: Legacy function declarations
    //   function _ready() { ... }
    //
    // Pattern 2: exports pattern (RECOMMENDED for full JS syntax support)
    //   exports._ready = function() { ... };
    //   exports._ready = () => { ... };
    //   exports = { _ready() { ... } };
    //   exports = new PlayerController();  // class-based
    //
    // The exports pattern is preferred because it supports all JS syntax
    // (arrow functions, classes, async, etc.) and works on all platforms.

    // 1. Detect legacy function declarations: function name(
    int pos = 0;
    while ((pos = source_code_.find("function ", pos)) >= 0) {
        // Skip if in a single-line comment
        int line_start = source_code_.rfind("\n", pos);
        if (line_start < 0) line_start = 0;
        String line_before = source_code_.substr(line_start, pos - line_start);
        if (line_before.find("//") >= 0) {
            pos += 9;
            continue;
        }

        pos += 9; // Skip "function "

        // Skip whitespace
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Read function name (valid JS identifier)
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

        // Verify it's followed by '('
        if (name_end > name_start && pos < source_code_.length() && source_code_[pos] == '(') {
            String method_name = source_code_.substr(name_start, name_end - name_start);
            if (!method_name.is_empty() && methods_.find(StringName(method_name)) < 0) {
                methods_.push_back(StringName(method_name));
            }
        }
    }

    // 2. Detect exports pattern: exports.methodName = or exports["methodName"] =
    pos = 0;
    while ((pos = source_code_.find("exports.", pos)) >= 0) {
        // Skip if in a comment
        int line_start = source_code_.rfind("\n", pos);
        if (line_start < 0) line_start = 0;
        String line_before = source_code_.substr(line_start, pos - line_start);
        if (line_before.find("//") >= 0) {
            pos += 8;
            continue;
        }

        pos += 8; // Skip "exports."

        // Read method name
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

        // Skip whitespace
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Verify it's followed by '=' (assignment)
        if (name_end > name_start && pos < source_code_.length() && source_code_[pos] == '=') {
            String method_name = source_code_.substr(name_start, name_end - name_start);
            if (!method_name.is_empty() && methods_.find(StringName(method_name)) < 0) {
                methods_.push_back(StringName(method_name));
            }
        }
    }

    // 3. Detect exports object literal: exports = { methodName( or methodName:
    pos = 0;
    while ((pos = source_code_.find("exports", pos)) >= 0) {
        int check_pos = pos + 7;
        // Skip whitespace
        while (check_pos < source_code_.length() && (source_code_[check_pos] == ' ' || source_code_[check_pos] == '\t')) {
            check_pos++;
        }
        // Check for = {
        if (check_pos < source_code_.length() && source_code_[check_pos] == '=') {
            check_pos++;
            while (check_pos < source_code_.length() && (source_code_[check_pos] == ' ' || source_code_[check_pos] == '\t' || source_code_[check_pos] == '\n')) {
                check_pos++;
            }
            if (check_pos < source_code_.length() && source_code_[check_pos] == '{') {
                // Parse object literal methods
                int brace_count = 1;
                check_pos++;
                while (check_pos < source_code_.length() && brace_count > 0) {
                    if (source_code_[check_pos] == '{') brace_count++;
                    else if (source_code_[check_pos] == '}') brace_count--;
                    else if (brace_count == 1) {
                        // Look for method shorthand: methodName( or methodName:
                        // Skip whitespace/newlines
                        while (check_pos < source_code_.length() &&
                               (source_code_[check_pos] == ' ' || source_code_[check_pos] == '\t' ||
                                source_code_[check_pos] == '\n' || source_code_[check_pos] == ',')) {
                            check_pos++;
                        }
                        // Read identifier
                        int id_start = check_pos;
                        while (check_pos < source_code_.length()) {
                            char32_t c = source_code_[check_pos];
                            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                (c >= '0' && c <= '9') || c == '_' || c == '$') {
                                check_pos++;
                            } else {
                                break;
                            }
                        }
                        int id_end = check_pos;
                        // Skip whitespace
                        while (check_pos < source_code_.length() && (source_code_[check_pos] == ' ' || source_code_[check_pos] == '\t')) {
                            check_pos++;
                        }
                        // Check if followed by ( or :
                        if (id_end > id_start && check_pos < source_code_.length() &&
                            (source_code_[check_pos] == '(' || source_code_[check_pos] == ':')) {
                            String method_name = source_code_.substr(id_start, id_end - id_start);
                            if (!method_name.is_empty() && methods_.find(StringName(method_name)) < 0) {
                                methods_.push_back(StringName(method_name));
                            }
                        }
                    }
                    check_pos++;
                }
            }
        }
        pos++;
    }

    // 4. Detect class methods: class ClassName { methodName( or methodName = (
    pos = 0;
    while ((pos = source_code_.find("class ", pos)) >= 0) {
        // Skip if in a comment
        int line_start = source_code_.rfind("\n", pos);
        if (line_start < 0) line_start = 0;
        String line_before = source_code_.substr(line_start, pos - line_start);
        if (line_before.find("//") >= 0) {
            pos += 6;
            continue;
        }

        // Find the class body opening brace
        int brace_start = source_code_.find("{", pos);
        if (brace_start < 0) {
            pos += 6;
            continue;
        }

        // Parse class body
        int brace_count = 1;
        int check_pos = brace_start + 1;
        while (check_pos < source_code_.length() && brace_count > 0) {
            char32_t c = source_code_[check_pos];
            if (c == '{') brace_count++;
            else if (c == '}') brace_count--;
            else if (brace_count == 1) {
                // Look for method definitions at class level
                // Skip whitespace/newlines
                while (check_pos < source_code_.length() &&
                       (source_code_[check_pos] == ' ' || source_code_[check_pos] == '\t' ||
                        source_code_[check_pos] == '\n' || source_code_[check_pos] == '\r')) {
                    check_pos++;
                }
                if (check_pos >= source_code_.length()) break;

                // Check for constructor (skip it)
                if (source_code_.substr(check_pos, 11) == "constructor") {
                    check_pos += 11;
                    continue;
                }

                // Read identifier (potential method name)
                int id_start = check_pos;
                while (check_pos < source_code_.length()) {
                    char32_t mc = source_code_[check_pos];
                    if ((mc >= 'a' && mc <= 'z') || (mc >= 'A' && mc <= 'Z') ||
                        (mc >= '0' && mc <= '9') || mc == '_' || mc == '$') {
                        check_pos++;
                    } else {
                        break;
                    }
                }
                int id_end = check_pos;

                // Skip whitespace
                while (check_pos < source_code_.length() &&
                       (source_code_[check_pos] == ' ' || source_code_[check_pos] == '\t')) {
                    check_pos++;
                }

                // Check if followed by ( (method) or = (class field with arrow function)
                if (id_end > id_start && check_pos < source_code_.length() &&
                    (source_code_[check_pos] == '(' || source_code_[check_pos] == '=')) {
                    String method_name = source_code_.substr(id_start, id_end - id_start);
                    // Skip common non-method keywords
                    if (!method_name.is_empty() && method_name != "static" && method_name != "get" &&
                        method_name != "set" && method_name != "async" &&
                        methods_.find(StringName(method_name)) < 0) {
                        methods_.push_back(StringName(method_name));
                    }
                }
            }
            check_pos++;
        }
        pos = check_pos;
    }

    // Parse annotation comments (@signal, @export)
    parse_annotations();

    is_valid_ = true;
    return true;
}

void JSScript::parse_annotations() {
    // Parse @signal annotations (in comments, safe to search)
    // Pattern: // @signal signal_name
    int pos = 0;
    while ((pos = source_code_.find("@signal", pos)) >= 0) {
        pos += 7; // Skip "@signal"

        // Skip whitespace
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Read signal name (valid JS identifier)
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

    // Parse "var signals = ['signal1', 'signal2']" pattern
    // This is the GDScript 4.x style signal declaration
    pos = 0;
    while ((pos = source_code_.find("var signals", pos)) >= 0) {
        pos += 11; // Skip "var signals"

        // Skip whitespace
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Expect '='
        if (pos >= source_code_.length() || source_code_[pos] != '=') {
            continue;
        }
        pos++; // Skip '='

        // Skip whitespace
        while (pos < source_code_.length() && (source_code_[pos] == ' ' || source_code_[pos] == '\t')) {
            pos++;
        }

        // Expect '[' for array
        if (pos >= source_code_.length() || source_code_[pos] != '[') {
            continue;
        }
        pos++; // Skip '['

        // Parse array contents until ']'
        while (pos < source_code_.length() && source_code_[pos] != ']') {
            // Skip whitespace and commas
            while (pos < source_code_.length() &&
                   (source_code_[pos] == ' ' || source_code_[pos] == '\t' ||
                    source_code_[pos] == ',' || source_code_[pos] == '\n' || source_code_[pos] == '\r')) {
                pos++;
            }

            if (pos >= source_code_.length() || source_code_[pos] == ']') {
                break;
            }

            // Expect quote (single or double)
            char32_t quote_char = source_code_[pos];
            if (quote_char != '\'' && quote_char != '"') {
                pos++;
                continue;
            }
            pos++; // Skip opening quote

            // Read signal name
            int name_start = pos;
            while (pos < source_code_.length() && source_code_[pos] != quote_char) {
                pos++;
            }

            if (pos > name_start) {
                String signal_name = source_code_.substr(name_start, pos - name_start);
                if (!signal_name.is_empty() && signals_.find(StringName(signal_name)) < 0) {
                    signals_.push_back(StringName(signal_name));
                }
            }

            if (pos < source_code_.length()) {
                pos++; // Skip closing quote
            }
        }
    }

    // Parse @export annotations
    // Pattern: // @export var_name
    pos = 0;
    while ((pos = source_code_.find("@export", pos)) >= 0) {
        int line_end = source_code_.find("\n", pos);
        if (line_end < 0) line_end = source_code_.length();

        String line = source_code_.substr(pos, line_end - pos);

        // Look for variable declaration keywords
        int var_pos = line.find("var ");
        if (var_pos < 0) var_pos = line.find("let ");
        if (var_pos < 0) var_pos = line.find("const ");

        if (var_pos >= 0) {
            int name_start = var_pos + 4;
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
}

} // namespace jsb

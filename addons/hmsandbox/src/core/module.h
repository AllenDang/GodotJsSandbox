#ifndef GODOT_JS_RUNTIME_REGISTER_TYPES_H
#define GODOT_JS_RUNTIME_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_godot_js_runtime_module(ModuleInitializationLevel p_level);
void uninitialize_godot_js_runtime_module(ModuleInitializationLevel p_level);

#endif // GODOT_JS_RUNTIME_REGISTER_TYPES_H

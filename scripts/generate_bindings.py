#!/usr/bin/env python3
"""
GodotJSRuntime Binding Generator

Generates C++ bindings for Godot classes from extension_api.json.
Uses Jinja2 templates for code generation.

Usage:
    uv run generate_bindings.py [--api-json PATH] [--output-dir PATH]
"""

import json
import os
import sys
import argparse
from pathlib import Path
from typing import Any
from dataclasses import dataclass, field

from jinja2 import Environment, FileSystemLoader


# Type mapping from Godot types to C++ types
GODOT_TO_CPP_TYPE = {
    "Nil": "Variant",
    "bool": "bool",
    "int": "int64_t",
    "float": "double",
    "String": "String",
    "StringName": "StringName",
    "Vector2": "Vector2",
    "Vector2i": "Vector2i",
    "Vector3": "Vector3",
    "Vector3i": "Vector3i",
    "Vector4": "Vector4",
    "Vector4i": "Vector4i",
    "Rect2": "Rect2",
    "Rect2i": "Rect2i",
    "Transform2D": "Transform2D",
    "Transform3D": "Transform3D",
    "Plane": "Plane",
    "Quaternion": "Quaternion",
    "AABB": "AABB",
    "Basis": "Basis",
    "Projection": "Projection",
    "Color": "Color",
    "NodePath": "NodePath",
    "RID": "RID",
    "Callable": "Callable",
    "Signal": "Signal",
    "Dictionary": "Dictionary",
    "Array": "Array",
    "PackedByteArray": "PackedByteArray",
    "PackedInt32Array": "PackedInt32Array",
    "PackedInt64Array": "PackedInt64Array",
    "PackedFloat32Array": "PackedFloat32Array",
    "PackedFloat64Array": "PackedFloat64Array",
    "PackedStringArray": "PackedStringArray",
    "PackedVector2Array": "PackedVector2Array",
    "PackedVector3Array": "PackedVector3Array",
    "PackedColorArray": "PackedColorArray",
    "PackedVector4Array": "PackedVector4Array",
    "Variant": "Variant",
}

# Types that need special JS conversion
JS_CONVERSION_TEMPLATES = {
    "bool": {
        "to_js": "return JS_NewBool(ctx, value);",
        "from_js": "bool value = JS_ToBool(ctx, argv[1]);",
    },
    "int64_t": {
        "to_js": "return JS_NewInt64(ctx, value);",
        "from_js": "int64_t value; JS_ToInt64(ctx, &value, argv[1]);",
    },
    "double": {
        "to_js": "return JS_NewFloat64(ctx, value);",
        "from_js": "double value; JS_ToFloat64(ctx, &value, argv[1]);",
    },
    "String": {
        "to_js": "return JS_NewString(ctx, value.utf8().get_data());",
        "from_js": 'const char* str = JS_ToCString(ctx, argv[1]); String value = str ? String::utf8(str) : ""; JS_FreeCString(ctx, str);',
    },
    "StringName": {
        "to_js": "return JS_NewString(ctx, String(value).utf8().get_data());",
        "from_js": 'const char* str = JS_ToCString(ctx, argv[1]); StringName value = str ? String::utf8(str) : ""; JS_FreeCString(ctx, str);',
    },
    "Vector2": {
        "to_js": """JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, value.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, value.y));
    return obj;""",
        "from_js": """Vector2 value;
    JSValue x_val = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue y_val = JS_GetPropertyStr(ctx, argv[1], "y");
    JS_ToFloat64(ctx, &value.x, x_val);
    JS_ToFloat64(ctx, &value.y, y_val);
    JS_FreeValue(ctx, x_val);
    JS_FreeValue(ctx, y_val);""",
    },
    "Vector3": {
        "to_js": """JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, value.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, value.y));
    JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, value.z));
    return obj;""",
        "from_js": """Vector3 value;
    JSValue x_val = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue y_val = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue z_val = JS_GetPropertyStr(ctx, argv[1], "z");
    JS_ToFloat64(ctx, &value.x, x_val);
    JS_ToFloat64(ctx, &value.y, y_val);
    JS_ToFloat64(ctx, &value.z, z_val);
    JS_FreeValue(ctx, x_val);
    JS_FreeValue(ctx, y_val);
    JS_FreeValue(ctx, z_val);""",
    },
    "Color": {
        "to_js": """JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "r", JS_NewFloat64(ctx, value.r));
    JS_SetPropertyStr(ctx, obj, "g", JS_NewFloat64(ctx, value.g));
    JS_SetPropertyStr(ctx, obj, "b", JS_NewFloat64(ctx, value.b));
    JS_SetPropertyStr(ctx, obj, "a", JS_NewFloat64(ctx, value.a));
    return obj;""",
        "from_js": """Color value;
    JSValue r_val = JS_GetPropertyStr(ctx, argv[1], "r");
    JSValue g_val = JS_GetPropertyStr(ctx, argv[1], "g");
    JSValue b_val = JS_GetPropertyStr(ctx, argv[1], "b");
    JSValue a_val = JS_GetPropertyStr(ctx, argv[1], "a");
    JS_ToFloat64(ctx, &value.r, r_val);
    JS_ToFloat64(ctx, &value.g, g_val);
    JS_ToFloat64(ctx, &value.b, b_val);
    if (!JS_IsUndefined(a_val)) JS_ToFloat64(ctx, &value.a, a_val); else value.a = 1.0;
    JS_FreeValue(ctx, r_val);
    JS_FreeValue(ctx, g_val);
    JS_FreeValue(ctx, b_val);
    JS_FreeValue(ctx, a_val);""",
    },
    "Quaternion": {
        "to_js": """JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, value.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, value.y));
    JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, value.z));
    JS_SetPropertyStr(ctx, obj, "w", JS_NewFloat64(ctx, value.w));
    return obj;""",
        "from_js": """Quaternion value;
    JSValue x_val = JS_GetPropertyStr(ctx, argv[1], "x");
    JSValue y_val = JS_GetPropertyStr(ctx, argv[1], "y");
    JSValue z_val = JS_GetPropertyStr(ctx, argv[1], "z");
    JSValue w_val = JS_GetPropertyStr(ctx, argv[1], "w");
    JS_ToFloat64(ctx, &value.x, x_val);
    JS_ToFloat64(ctx, &value.y, y_val);
    JS_ToFloat64(ctx, &value.z, z_val);
    JS_ToFloat64(ctx, &value.w, w_val);
    JS_FreeValue(ctx, x_val);
    JS_FreeValue(ctx, y_val);
    JS_FreeValue(ctx, z_val);
    JS_FreeValue(ctx, w_val);""",
    },
}


@dataclass
class MethodArg:
    name: str
    type: str
    cpp_type: str
    conversion: str
    default_value: str | None = None
    is_optional: bool = False
    arg_index: int = 0  # Index in argv (0-based, excluding handle)


@dataclass
class MethodInfo:
    name: str  # Name from API (used for function naming)
    return_type: str
    return_cpp_type: str
    return_conversion: str
    arguments: list[MethodArg] = field(default_factory=list)
    call_args: str = ""
    is_static: bool = False
    is_virtual: bool = False
    required_arg_count: int = 0  # Number of required arguments (without defaults)
    js_name: str = ""  # Name to expose in JS (if different from API name)
    cpp_name: str = ""  # Actual C++ method name to call (if different from API name)
    api_category: str = "WRITE"  # READ, WRITE, or HEAVY - for rate limiting per PRD Section 6.4


@dataclass
class PropertyInfo:
    name: str
    type: str
    cpp_type: str
    getter: str
    setter: str | None
    readonly: bool
    to_js_conversion: str
    from_js_conversion: str
    setter_cast: str = ""  # Cast expression for enum types


@dataclass
class ClassInfo:
    name: str
    parent_class: str
    header_name: str
    methods: list[MethodInfo] = field(default_factory=list)
    properties: list[PropertyInfo] = field(default_factory=list)
    signals: list[str] = field(default_factory=list)  # Signal names for GDScript 4.x syntax
    extra_includes: list[str] = field(default_factory=list)
    is_instantiable: bool = True  # Whether JS can create instances with new


class BindingGenerator:
    def __init__(self, api_json_path: Path, output_dir: Path, config_dir: Path, templates_dir: Path):
        self.api_json_path = api_json_path
        self.output_dir = output_dir
        self.config_dir = config_dir
        self.templates_dir = templates_dir

        self.api_data: dict = {}
        self.blocklist: dict = {}
        self.classes: dict[str, ClassInfo] = {}

        # godot-cpp headers directory for header name resolution
        self.godot_cpp_headers_dir = Path("godot-cpp/gen/include/godot_cpp/classes")
        self.header_cache: dict[str, str] = {}  # class_name -> actual header name

        # Setup Jinja2
        self.jinja_env = Environment(
            loader=FileSystemLoader(str(templates_dir)),
            trim_blocks=True,
            lstrip_blocks=True,
        )

    def build_header_cache(self):
        """Build a cache mapping class names to actual header file names."""
        if not self.godot_cpp_headers_dir.exists():
            print(f"Warning: godot-cpp headers directory not found: {self.godot_cpp_headers_dir}")
            return

        # Scan all .hpp files and build reverse mapping
        for header_file in self.godot_cpp_headers_dir.glob("*.hpp"):
            # Convert header name back to potential class name
            # e.g., "animation_node_blend_space1_d.hpp" -> header stem "animation_node_blend_space1_d"
            header_stem = header_file.stem

            # Try to match with class names - we'll do this after loading API
            self.header_cache[header_stem] = header_stem

    def find_header_for_class(self, class_name: str) -> str:
        """Find the actual header file name for a class.
        Returns the header name without extension.

        godot-cpp has inconsistent naming:
        - 2D/3D stay as 2d/3d (e.g., node3d, area2d, compressed_texture2d_array)
        - 1D becomes 1_d (e.g., animation_node_blend_space1_d, gradient_texture1_d)
        - 6DOF becomes 6_dof (e.g., generic6_dof_joint3d)
        """
        # Generate candidate names using our conversion
        base_name = self._class_name_to_snake_case(class_name)

        # Check if the file exists directly
        if (self.godot_cpp_headers_dir / f"{base_name}.hpp").exists():
            return base_name

        # Try alternative patterns for 1D classes (they use 1_d instead of 1d)
        # e.g., AnimationNodeBlendSpace1D -> animation_node_blend_space1_d
        if "1D" in class_name:
            alt_name = base_name.replace("1d", "1_d")
            if (self.godot_cpp_headers_dir / f"{alt_name}.hpp").exists():
                return alt_name

        # Return base name even if not found (will cause compile error, but that's better than silent skip)
        return base_name

    def _class_name_to_snake_case(self, class_name: str) -> str:
        """Convert class name to snake_case.

        Rules based on godot-cpp naming:
        - Add underscore before uppercase when preceded by lowercase (CamelCase -> camel_case)
        - Add underscore when uppercase followed by lowercase after uppercase sequence (CPUParticles -> cpu_particles)
        - DO NOT add underscore after digits for 2D/3D (Node3DGizmo -> node3d_gizmo)
        - DO add underscore after digit for multi-char sequences like 6DOF (Generic6DOFJoint3D -> generic6_dof_joint3d)
        - DO add underscore after digit when followed by non-D uppercase (FBX2GLTF -> fbx2_gltf)
        """
        result = ""
        i = 0

        while i < len(class_name):
            c = class_name[i]

            if c.isupper() and i > 0:
                prev = class_name[i-1]

                # Check if this is part of 2D/3D pattern - these stay together as 2d/3d
                # e.g., Node3D -> node3d, Texture2DArray -> texture2d_array
                if prev.isdigit() and c == 'D' and prev in ('2', '3'):
                    # Just append 'd', the next iteration will handle underscore if needed
                    result += c.lower()
                    i += 1
                    continue

                # Add underscore before uppercase if preceded by lowercase
                if prev.islower():
                    result += "_"
                # Add underscore when uppercase followed by lowercase after uppercase sequence
                elif prev.isupper() and i + 1 < len(class_name) and class_name[i+1].islower():
                    result += "_"
                # Handle digit followed by uppercase (not D for 2D/3D)
                # e.g., FBX2GLTF -> fbx2_gltf, Generic6DOFJoint3D -> generic6_dof_joint3d
                elif prev.isdigit():
                    # Always add underscore after digit when followed by uppercase (except 2D/3D handled above)
                    result += "_"

            result += c.lower()
            i += 1

        return result

    def load_config(self):
        """Load blocklist and other config files."""
        blocklist_path = self.config_dir / "blocklist.json"
        if blocklist_path.exists():
            with open(blocklist_path) as f:
                self.blocklist = json.load(f)
        else:
            self.blocklist = {
                "blocked_classes": [],
                "blocked_methods": {},
                "blocked_properties": {},
            }

    def load_api(self):
        """Load extension_api.json."""
        with open(self.api_json_path) as f:
            self.api_data = json.load(f)

        # Extract singleton names - these should not be instantiated
        self.singletons = set()
        for singleton in self.api_data.get("singletons", []):
            name = singleton.get("name", "")
            if name:
                self.singletons.add(name)

    def is_class_blocked(self, class_name: str) -> bool:
        # Check exact match
        if class_name in self.blocklist.get("blocked_classes", []):
            return True

        # Check prefix patterns
        for prefix in self.blocklist.get("blocked_class_prefixes", []):
            if class_name.startswith(prefix):
                return True

        # Check suffix patterns
        for suffix in self.blocklist.get("blocked_class_suffixes", []):
            if class_name.endswith(suffix):
                return True

        # Block singletons - they should not be instantiated directly
        # (they're accessed via global variables like Input, Time, etc.)
        if hasattr(self, 'singletons') and class_name in self.singletons:
            return True

        return False

    def is_method_blocked(self, class_name: str, method_name: str) -> bool:
        blocked_methods = self.blocklist.get("blocked_methods", {})
        if method_name in blocked_methods.get("Object", []):
            return True
        return method_name in blocked_methods.get(class_name, [])

    def get_method_alias(self, class_name: str, method_name: str) -> str | None:
        """Get JS alias for a method if one is defined.
        Returns the alias name, or None if no alias exists."""
        aliases = self.blocklist.get("method_aliases", {})
        class_aliases = aliases.get(class_name, {})
        return class_aliases.get(method_name)

    def get_method_cpp_name(self, class_name: str, method_name: str) -> str:
        """Get the actual C++ method name to call (handles API vs C++ name mismatches).
        For example, 'get_node' in the API maps to 'get_node_internal' in C++."""
        cpp_names = self.blocklist.get("method_cpp_names", {})
        class_cpp_names = cpp_names.get(class_name, {})
        return class_cpp_names.get(method_name, method_name)

    def get_method_api_category(self, method_name: str) -> str:
        """Determine API category for rate limiting per PRD Section 6.4.
        Returns: READ, WRITE, or HEAVY"""
        # HEAVY operations (50/frame) - create/destroy objects, major tree changes
        heavy_methods = {
            "queue_free", "free", "duplicate", "instantiate",
            "add_child", "remove_child", "reparent", "add_sibling",
            "move_child", "move_to_front", "move_to_back",
            "create_instance", "create_child", "create_item",
        }
        heavy_prefixes = ("create_", "instantiate_", "spawn_")

        if method_name in heavy_methods:
            return "HEAVY"
        if method_name.startswith(heavy_prefixes):
            return "HEAVY"

        # READ operations (unlimited) - only read state, no side effects
        read_prefixes = (
            "get_", "is_", "has_", "can_", "find_", "are_",
            "was_", "were_", "should_", "will_", "does_",
        )
        if method_name.startswith(read_prefixes):
            return "READ"

        # WRITE operations (500/frame) - everything else that modifies state
        return "WRITE"

    def is_property_blocked(self, class_name: str, property_name: str) -> bool:
        blocked_props = self.blocklist.get("blocked_properties", {})
        if property_name in blocked_props.get("Object", []):
            return True
        return property_name in blocked_props.get(class_name, [])

    def get_cpp_type(self, godot_type: str, for_return: bool = False) -> str:
        """Convert Godot type to C++ type.

        Args:
            godot_type: The Godot type string
            for_return: If True, use Ref<T> for RefCounted types (for return values)
        """
        # Handle typed arrays like "typedarray::Node"
        # Use plain Array to avoid incomplete type issues with forward-declared classes
        # Godot will handle the type conversion internally
        if godot_type.startswith("typedarray::"):
            return "Array"

        # Handle typed dictionaries like "typeddictionary::Color;Color"
        # Use plain Dictionary - Godot handles the type conversion internally
        if godot_type.startswith("typeddictionary::"):
            return "Dictionary"

        # Handle enum types
        if godot_type.startswith("enum::"):
            return godot_type[6:].replace(".", "::")

        # Handle bitfield types
        if godot_type.startswith("bitfield::"):
            return "BitField<" + godot_type[10:].replace(".", "::") + ">"

        # Basic type mapping
        if godot_type in GODOT_TO_CPP_TYPE:
            return GODOT_TO_CPP_TYPE[godot_type]

        # RefCounted types use Ref<T> smart pointers
        if self.is_refcounted_type(godot_type):
            return f"Ref<{godot_type}>"

        # Node and other Object types use raw pointers
        return godot_type + "*"

    def convert_default_value(self, default_value: str, cpp_type: str, godot_type: str = None) -> str:
        """Convert Godot default value format to C++ format."""
        if default_value is None:
            return None

        # Enum types: cast numeric default to enum type
        if godot_type and godot_type.startswith("enum::"):
            # Default is usually a number like "0", cast to enum
            return f"({cpp_type}){default_value}"

        # StringName: &"" -> StringName()
        if cpp_type == "StringName":
            if default_value == '&""' or default_value == "":
                return "StringName()"
            # &"something" -> StringName("something")
            if default_value.startswith('&"') and default_value.endswith('"'):
                return f'StringName({default_value[1:]})'
            return f'StringName("{default_value}")'

        # String: "" -> String()
        if cpp_type == "String":
            if default_value == '""' or default_value == "":
                return "String()"
            return default_value  # Keep as is

        # NodePath: ^"" -> NodePath()
        if cpp_type == "NodePath":
            if default_value.startswith('^"'):
                inner = default_value[2:-1] if default_value.endswith('"') else ""
                if inner == "":
                    return "NodePath()"
                return f'NodePath("{inner}")'
            return default_value

        # Array: [] -> Array() (also handles typedarray:: which maps to Array)
        if cpp_type == "Array":
            if default_value == "[]":
                return "Array()"
            return "Array()"  # Default to empty array

        # Dictionary: {} -> Dictionary()
        if cpp_type == "Dictionary":
            if default_value == "{}":
                return "Dictionary()"
            return "Dictionary()"

        # bool: keep as is (true/false)
        # int/float: keep as is
        # null -> nullptr
        if default_value == "null":
            return "nullptr"

        return default_value

    def get_js_to_cpp_conversion(self, cpp_type: str, arg_index: int, arg_name: str, godot_type: str = None) -> str:
        """Generate code to convert JS argument to C++ type.
        godot_type: The original Godot type (e.g., "enum::Side" or "bitfield::...") if different from cpp_type.
        """
        # Handle enum types first
        if godot_type and godot_type.startswith("enum::"):
            return f"int64_t tmp_{arg_name}; JS_ToInt64(ctx, &tmp_{arg_name}, argv[{arg_index}]); {cpp_type} arg_{arg_name} = ({cpp_type})tmp_{arg_name};"

        # Handle bitfield types - similar to enums, convert from int64
        if godot_type and godot_type.startswith("bitfield::"):
            return f"int64_t tmp_{arg_name}; JS_ToInt64(ctx, &tmp_{arg_name}, argv[{arg_index}]); {cpp_type} arg_{arg_name} = ({cpp_type})tmp_{arg_name};"

        if cpp_type == "bool":
            return f"bool arg_{arg_name} = JS_ToBool(ctx, argv[{arg_index}]);"
        if cpp_type == "int64_t":
            return f"int64_t arg_{arg_name}; JS_ToInt64(ctx, &arg_{arg_name}, argv[{arg_index}]);"
        if cpp_type == "double":
            return f"double arg_{arg_name}; JS_ToFloat64(ctx, &arg_{arg_name}, argv[{arg_index}]);"
        if cpp_type == "String":
            return f'const char* cstr_{arg_name} = JS_ToCString(ctx, argv[{arg_index}]); String arg_{arg_name} = cstr_{arg_name} ? String::utf8(cstr_{arg_name}) : ""; JS_FreeCString(ctx, cstr_{arg_name});'
        if cpp_type == "StringName":
            return f'const char* cstr_{arg_name} = JS_ToCString(ctx, argv[{arg_index}]); StringName arg_{arg_name} = cstr_{arg_name} ? String::utf8(cstr_{arg_name}) : ""; JS_FreeCString(ctx, cstr_{arg_name});'
        if cpp_type == "NodePath":
            return f'const char* cstr_{arg_name} = JS_ToCString(ctx, argv[{arg_index}]); NodePath arg_{arg_name} = cstr_{arg_name} ? NodePath(String::utf8(cstr_{arg_name})) : NodePath(); JS_FreeCString(ctx, cstr_{arg_name});'
        if cpp_type == "Vector2":
            return f"""double tmp_x_{arg_name}, tmp_y_{arg_name};
    JSValue jx_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "x");
    JSValue jy_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "y");
    JS_ToFloat64(ctx, &tmp_x_{arg_name}, jx_{arg_name});
    JS_ToFloat64(ctx, &tmp_y_{arg_name}, jy_{arg_name});
    JS_FreeValue(ctx, jx_{arg_name});
    JS_FreeValue(ctx, jy_{arg_name});
    Vector2 arg_{arg_name}(tmp_x_{arg_name}, tmp_y_{arg_name});"""
        if cpp_type == "Vector3":
            return f"""double tmp_x_{arg_name}, tmp_y_{arg_name}, tmp_z_{arg_name};
    JSValue jx_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "x");
    JSValue jy_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "y");
    JSValue jz_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "z");
    JS_ToFloat64(ctx, &tmp_x_{arg_name}, jx_{arg_name});
    JS_ToFloat64(ctx, &tmp_y_{arg_name}, jy_{arg_name});
    JS_ToFloat64(ctx, &tmp_z_{arg_name}, jz_{arg_name});
    JS_FreeValue(ctx, jx_{arg_name});
    JS_FreeValue(ctx, jy_{arg_name});
    JS_FreeValue(ctx, jz_{arg_name});
    Vector3 arg_{arg_name}(tmp_x_{arg_name}, tmp_y_{arg_name}, tmp_z_{arg_name});"""
        if cpp_type == "Color":
            return f"""double tmp_r_{arg_name}, tmp_g_{arg_name}, tmp_b_{arg_name}, tmp_a_{arg_name} = 1.0;
    JSValue jr_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "r");
    JSValue jg_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "g");
    JSValue jb_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "b");
    JSValue ja_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "a");
    JS_ToFloat64(ctx, &tmp_r_{arg_name}, jr_{arg_name});
    JS_ToFloat64(ctx, &tmp_g_{arg_name}, jg_{arg_name});
    JS_ToFloat64(ctx, &tmp_b_{arg_name}, jb_{arg_name});
    if (!JS_IsUndefined(ja_{arg_name})) JS_ToFloat64(ctx, &tmp_a_{arg_name}, ja_{arg_name});
    JS_FreeValue(ctx, jr_{arg_name});
    JS_FreeValue(ctx, jg_{arg_name});
    JS_FreeValue(ctx, jb_{arg_name});
    JS_FreeValue(ctx, ja_{arg_name});
    Color arg_{arg_name}(tmp_r_{arg_name}, tmp_g_{arg_name}, tmp_b_{arg_name}, tmp_a_{arg_name});"""
        if cpp_type == "Quaternion":
            return f"""double tmp_x_{arg_name}, tmp_y_{arg_name}, tmp_z_{arg_name}, tmp_w_{arg_name};
    JSValue jx_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "x");
    JSValue jy_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "y");
    JSValue jz_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "z");
    JSValue jw_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "w");
    JS_ToFloat64(ctx, &tmp_x_{arg_name}, jx_{arg_name});
    JS_ToFloat64(ctx, &tmp_y_{arg_name}, jy_{arg_name});
    JS_ToFloat64(ctx, &tmp_z_{arg_name}, jz_{arg_name});
    JS_ToFloat64(ctx, &tmp_w_{arg_name}, jw_{arg_name});
    JS_FreeValue(ctx, jx_{arg_name});
    JS_FreeValue(ctx, jy_{arg_name});
    JS_FreeValue(ctx, jz_{arg_name});
    JS_FreeValue(ctx, jw_{arg_name});
    Quaternion arg_{arg_name}(tmp_x_{arg_name}, tmp_y_{arg_name}, tmp_z_{arg_name}, tmp_w_{arg_name});"""
        if cpp_type == "Basis":
            return f"""Basis arg_{arg_name};
    JSValue jx_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "x");
    JSValue jy_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "y");
    JSValue jz_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "z");
    if (!JS_IsUndefined(jx_{arg_name}) && !JS_IsUndefined(jy_{arg_name}) && !JS_IsUndefined(jz_{arg_name})) {{
        double xx, xy, xz, yx, yy, yz, zx, zy, zz;
        JSValue jxx_{arg_name} = JS_GetPropertyStr(ctx, jx_{arg_name}, "x");
        JSValue jxy_{arg_name} = JS_GetPropertyStr(ctx, jx_{arg_name}, "y");
        JSValue jxz_{arg_name} = JS_GetPropertyStr(ctx, jx_{arg_name}, "z");
        JSValue jyx_{arg_name} = JS_GetPropertyStr(ctx, jy_{arg_name}, "x");
        JSValue jyy_{arg_name} = JS_GetPropertyStr(ctx, jy_{arg_name}, "y");
        JSValue jyz_{arg_name} = JS_GetPropertyStr(ctx, jy_{arg_name}, "z");
        JSValue jzx_{arg_name} = JS_GetPropertyStr(ctx, jz_{arg_name}, "x");
        JSValue jzy_{arg_name} = JS_GetPropertyStr(ctx, jz_{arg_name}, "y");
        JSValue jzz_{arg_name} = JS_GetPropertyStr(ctx, jz_{arg_name}, "z");
        JS_ToFloat64(ctx, &xx, jxx_{arg_name}); JS_ToFloat64(ctx, &xy, jxy_{arg_name}); JS_ToFloat64(ctx, &xz, jxz_{arg_name});
        JS_ToFloat64(ctx, &yx, jyx_{arg_name}); JS_ToFloat64(ctx, &yy, jyy_{arg_name}); JS_ToFloat64(ctx, &yz, jyz_{arg_name});
        JS_ToFloat64(ctx, &zx, jzx_{arg_name}); JS_ToFloat64(ctx, &zy, jzy_{arg_name}); JS_ToFloat64(ctx, &zz, jzz_{arg_name});
        arg_{arg_name} = Basis(Vector3(xx, xy, xz), Vector3(yx, yy, yz), Vector3(zx, zy, zz));
        JS_FreeValue(ctx, jxx_{arg_name}); JS_FreeValue(ctx, jxy_{arg_name}); JS_FreeValue(ctx, jxz_{arg_name});
        JS_FreeValue(ctx, jyx_{arg_name}); JS_FreeValue(ctx, jyy_{arg_name}); JS_FreeValue(ctx, jyz_{arg_name});
        JS_FreeValue(ctx, jzx_{arg_name}); JS_FreeValue(ctx, jzy_{arg_name}); JS_FreeValue(ctx, jzz_{arg_name});
    }}
    JS_FreeValue(ctx, jx_{arg_name});
    JS_FreeValue(ctx, jy_{arg_name});
    JS_FreeValue(ctx, jz_{arg_name});"""
        if cpp_type == "Transform3D":
            return f"""Transform3D arg_{arg_name};
    JSValue jbasis_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "basis");
    JSValue jorigin_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "origin");
    if (!JS_IsUndefined(jorigin_{arg_name})) {{
        double ox, oy, oz;
        JSValue jox_{arg_name} = JS_GetPropertyStr(ctx, jorigin_{arg_name}, "x");
        JSValue joy_{arg_name} = JS_GetPropertyStr(ctx, jorigin_{arg_name}, "y");
        JSValue joz_{arg_name} = JS_GetPropertyStr(ctx, jorigin_{arg_name}, "z");
        JS_ToFloat64(ctx, &ox, jox_{arg_name});
        JS_ToFloat64(ctx, &oy, joy_{arg_name});
        JS_ToFloat64(ctx, &oz, joz_{arg_name});
        arg_{arg_name}.origin = Vector3(ox, oy, oz);
        JS_FreeValue(ctx, jox_{arg_name}); JS_FreeValue(ctx, joy_{arg_name}); JS_FreeValue(ctx, joz_{arg_name});
    }}
    if (!JS_IsUndefined(jbasis_{arg_name})) {{
        JSValue jbx_{arg_name} = JS_GetPropertyStr(ctx, jbasis_{arg_name}, "x");
        JSValue jby_{arg_name} = JS_GetPropertyStr(ctx, jbasis_{arg_name}, "y");
        JSValue jbz_{arg_name} = JS_GetPropertyStr(ctx, jbasis_{arg_name}, "z");
        if (!JS_IsUndefined(jbx_{arg_name}) && !JS_IsUndefined(jby_{arg_name}) && !JS_IsUndefined(jbz_{arg_name})) {{
            double xx, xy, xz, yx, yy, yz, zx, zy, zz;
            JSValue jbxx = JS_GetPropertyStr(ctx, jbx_{arg_name}, "x"); JSValue jbxy = JS_GetPropertyStr(ctx, jbx_{arg_name}, "y"); JSValue jbxz = JS_GetPropertyStr(ctx, jbx_{arg_name}, "z");
            JSValue jbyx = JS_GetPropertyStr(ctx, jby_{arg_name}, "x"); JSValue jbyy = JS_GetPropertyStr(ctx, jby_{arg_name}, "y"); JSValue jbyz = JS_GetPropertyStr(ctx, jby_{arg_name}, "z");
            JSValue jbzx = JS_GetPropertyStr(ctx, jbz_{arg_name}, "x"); JSValue jbzy = JS_GetPropertyStr(ctx, jbz_{arg_name}, "y"); JSValue jbzz = JS_GetPropertyStr(ctx, jbz_{arg_name}, "z");
            JS_ToFloat64(ctx, &xx, jbxx); JS_ToFloat64(ctx, &xy, jbxy); JS_ToFloat64(ctx, &xz, jbxz);
            JS_ToFloat64(ctx, &yx, jbyx); JS_ToFloat64(ctx, &yy, jbyy); JS_ToFloat64(ctx, &yz, jbyz);
            JS_ToFloat64(ctx, &zx, jbzx); JS_ToFloat64(ctx, &zy, jbzy); JS_ToFloat64(ctx, &zz, jbzz);
            arg_{arg_name}.basis = Basis(Vector3(xx, xy, xz), Vector3(yx, yy, yz), Vector3(zx, zy, zz));
            JS_FreeValue(ctx, jbxx); JS_FreeValue(ctx, jbxy); JS_FreeValue(ctx, jbxz);
            JS_FreeValue(ctx, jbyx); JS_FreeValue(ctx, jbyy); JS_FreeValue(ctx, jbyz);
            JS_FreeValue(ctx, jbzx); JS_FreeValue(ctx, jbzy); JS_FreeValue(ctx, jbzz);
        }}
        JS_FreeValue(ctx, jbx_{arg_name}); JS_FreeValue(ctx, jby_{arg_name}); JS_FreeValue(ctx, jbz_{arg_name});
    }}
    JS_FreeValue(ctx, jbasis_{arg_name});
    JS_FreeValue(ctx, jorigin_{arg_name});"""

        if cpp_type == "Rect2":
            return f"""double tmp_x_{arg_name}, tmp_y_{arg_name}, tmp_w_{arg_name}, tmp_h_{arg_name};
    JSValue jpos_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "position");
    JSValue jsize_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "size");
    JSValue jpx_{arg_name} = JS_GetPropertyStr(ctx, jpos_{arg_name}, "x");
    JSValue jpy_{arg_name} = JS_GetPropertyStr(ctx, jpos_{arg_name}, "y");
    JSValue jsx_{arg_name} = JS_GetPropertyStr(ctx, jsize_{arg_name}, "x");
    JSValue jsy_{arg_name} = JS_GetPropertyStr(ctx, jsize_{arg_name}, "y");
    JS_ToFloat64(ctx, &tmp_x_{arg_name}, jpx_{arg_name});
    JS_ToFloat64(ctx, &tmp_y_{arg_name}, jpy_{arg_name});
    JS_ToFloat64(ctx, &tmp_w_{arg_name}, jsx_{arg_name});
    JS_ToFloat64(ctx, &tmp_h_{arg_name}, jsy_{arg_name});
    JS_FreeValue(ctx, jpx_{arg_name}); JS_FreeValue(ctx, jpy_{arg_name});
    JS_FreeValue(ctx, jsx_{arg_name}); JS_FreeValue(ctx, jsy_{arg_name});
    JS_FreeValue(ctx, jpos_{arg_name}); JS_FreeValue(ctx, jsize_{arg_name});
    Rect2 arg_{arg_name}(tmp_x_{arg_name}, tmp_y_{arg_name}, tmp_w_{arg_name}, tmp_h_{arg_name});"""

        # Enum types - convert to int64 then cast
        if "::" in cpp_type and not cpp_type.endswith("*"):
            return f"int64_t tmp_{arg_name}; JS_ToInt64(ctx, &tmp_{arg_name}, argv[{arg_index}]); {cpp_type} arg_{arg_name} = ({cpp_type})tmp_{arg_name};"

        # Ref<T> types (RefCounted objects) - extract handle and wrap in Ref
        if cpp_type.startswith("Ref<") and cpp_type.endswith(">"):
            inner_type = cpp_type[4:-1]  # Extract T from Ref<T>
            return f"""{cpp_type} arg_{arg_name};
    if (JS_IsNumber(argv[{arg_index}])) {{
        // Direct handle (unwrapped by JS proxy)
        int64_t h_{arg_name}; JS_ToInt64(ctx, &h_{arg_name}, argv[{arg_index}]);
        Object* obj_{arg_name} = qjs_ctx->get_object_registry()->get_object(h_{arg_name});
        arg_{arg_name} = Ref<{inner_type}>(Object::cast_to<{inner_type}>(obj_{arg_name}));
    }} else {{
        // Object with __handle property
        JSValue jh_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "__handle");
        if (!JS_IsUndefined(jh_{arg_name})) {{
            int64_t h_{arg_name}; JS_ToInt64(ctx, &h_{arg_name}, jh_{arg_name});
            Object* obj_{arg_name} = qjs_ctx->get_object_registry()->get_object(h_{arg_name});
            arg_{arg_name} = Ref<{inner_type}>(Object::cast_to<{inner_type}>(obj_{arg_name}));
        }}
        JS_FreeValue(ctx, jh_{arg_name});
    }}"""

        # Object pointer types - extract handle and look up object
        # Handle both: direct integer handle (from JS proxy unwrap) OR object with __handle property
        # Use reinterpret_cast to handle forward-declared types (runtime type safety handled by Godot)
        if cpp_type.endswith("*"):
            return f"""{cpp_type} arg_{arg_name} = nullptr;
    if (JS_IsNumber(argv[{arg_index}])) {{
        // Direct handle (unwrapped by JS proxy)
        int64_t h_{arg_name}; JS_ToInt64(ctx, &h_{arg_name}, argv[{arg_index}]);
        Object* obj_{arg_name} = qjs_ctx->get_object_registry()->get_object(h_{arg_name});
        arg_{arg_name} = reinterpret_cast<{cpp_type}>(obj_{arg_name});
    }} else {{
        // Object with __handle property
        JSValue jh_{arg_name} = JS_GetPropertyStr(ctx, argv[{arg_index}], "__handle");
        if (!JS_IsUndefined(jh_{arg_name})) {{
            int64_t h_{arg_name}; JS_ToInt64(ctx, &h_{arg_name}, jh_{arg_name});
            Object* obj_{arg_name} = qjs_ctx->get_object_registry()->get_object(h_{arg_name});
            arg_{arg_name} = reinterpret_cast<{cpp_type}>(obj_{arg_name});
        }}
        JS_FreeValue(ctx, jh_{arg_name});
    }}"""

        # PackedVector2Array - convert from JS array of Vector2 objects
        if cpp_type == "PackedVector2Array":
            return f"""PackedVector2Array arg_{arg_name};
    if (JS_IsArray(argv[{arg_index}])) {{
        JSValue len_val = JS_GetPropertyStr(ctx, argv[{arg_index}], "length");
        int64_t len = 0; JS_ToInt64(ctx, &len, len_val); JS_FreeValue(ctx, len_val);
        arg_{arg_name}.resize(len);
        for (int64_t i = 0; i < len; i++) {{
            JSValue elem = JS_GetPropertyUint32(ctx, argv[{arg_index}], i);
            double x = 0, y = 0;
            JSValue jx = JS_GetPropertyStr(ctx, elem, "x");
            JSValue jy = JS_GetPropertyStr(ctx, elem, "y");
            JS_ToFloat64(ctx, &x, jx); JS_ToFloat64(ctx, &y, jy);
            JS_FreeValue(ctx, jx); JS_FreeValue(ctx, jy);
            JS_FreeValue(ctx, elem);
            arg_{arg_name}.set(i, Vector2(x, y));
        }}
    }}"""

        # PackedVector3Array - convert from JS array of Vector3 objects
        if cpp_type == "PackedVector3Array":
            return f"""PackedVector3Array arg_{arg_name};
    if (JS_IsArray(argv[{arg_index}])) {{
        JSValue len_val = JS_GetPropertyStr(ctx, argv[{arg_index}], "length");
        int64_t len = 0; JS_ToInt64(ctx, &len, len_val); JS_FreeValue(ctx, len_val);
        arg_{arg_name}.resize(len);
        for (int64_t i = 0; i < len; i++) {{
            JSValue elem = JS_GetPropertyUint32(ctx, argv[{arg_index}], i);
            double x = 0, y = 0, z = 0;
            JSValue jx = JS_GetPropertyStr(ctx, elem, "x");
            JSValue jy = JS_GetPropertyStr(ctx, elem, "y");
            JSValue jz = JS_GetPropertyStr(ctx, elem, "z");
            JS_ToFloat64(ctx, &x, jx); JS_ToFloat64(ctx, &y, jy); JS_ToFloat64(ctx, &z, jz);
            JS_FreeValue(ctx, jx); JS_FreeValue(ctx, jy); JS_FreeValue(ctx, jz);
            JS_FreeValue(ctx, elem);
            arg_{arg_name}.set(i, Vector3(x, y, z));
        }}
    }}"""

        # PackedColorArray - convert from JS array of Color objects
        if cpp_type == "PackedColorArray":
            return f"""PackedColorArray arg_{arg_name};
    if (JS_IsArray(argv[{arg_index}])) {{
        JSValue len_val = JS_GetPropertyStr(ctx, argv[{arg_index}], "length");
        int64_t len = 0; JS_ToInt64(ctx, &len, len_val); JS_FreeValue(ctx, len_val);
        arg_{arg_name}.resize(len);
        for (int64_t i = 0; i < len; i++) {{
            JSValue elem = JS_GetPropertyUint32(ctx, argv[{arg_index}], i);
            double r = 0, g = 0, b = 0, a = 1;
            JSValue jr = JS_GetPropertyStr(ctx, elem, "r");
            JSValue jg = JS_GetPropertyStr(ctx, elem, "g");
            JSValue jb = JS_GetPropertyStr(ctx, elem, "b");
            JSValue ja = JS_GetPropertyStr(ctx, elem, "a");
            JS_ToFloat64(ctx, &r, jr); JS_ToFloat64(ctx, &g, jg); JS_ToFloat64(ctx, &b, jb);
            if (!JS_IsUndefined(ja)) JS_ToFloat64(ctx, &a, ja);
            JS_FreeValue(ctx, jr); JS_FreeValue(ctx, jg); JS_FreeValue(ctx, jb); JS_FreeValue(ctx, ja);
            JS_FreeValue(ctx, elem);
            arg_{arg_name}.set(i, Color(r, g, b, a));
        }}
    }}"""

        # PackedFloat32Array - convert from JS array of numbers
        if cpp_type == "PackedFloat32Array":
            return f"""PackedFloat32Array arg_{arg_name};
    if (JS_IsArray(argv[{arg_index}])) {{
        JSValue len_val = JS_GetPropertyStr(ctx, argv[{arg_index}], "length");
        int64_t len = 0; JS_ToInt64(ctx, &len, len_val); JS_FreeValue(ctx, len_val);
        arg_{arg_name}.resize(len);
        for (int64_t i = 0; i < len; i++) {{
            JSValue elem = JS_GetPropertyUint32(ctx, argv[{arg_index}], i);
            double val = 0; JS_ToFloat64(ctx, &val, elem);
            JS_FreeValue(ctx, elem);
            arg_{arg_name}.set(i, (float)val);
        }}
    }}"""

        # Default: use variant conversion
        return f"{cpp_type} arg_{arg_name} = qjs_ctx->js_to_variant(argv[{arg_index}]);"

    def get_cpp_to_js_conversion(self, cpp_type: str, var_name: str = "result") -> str:
        """Generate code to convert C++ result to JS value."""
        if cpp_type == "bool":
            return f"return JS_NewBool(ctx, {var_name});"
        if cpp_type == "int64_t":
            return f"return JS_NewInt64(ctx, {var_name});"
        if cpp_type == "double":
            return f"return JS_NewFloat64(ctx, {var_name});"
        if cpp_type == "String":
            return f"return JS_NewString(ctx, {var_name}.utf8().get_data());"
        if cpp_type == "StringName":
            return f"return JS_NewString(ctx, String({var_name}).utf8().get_data());"
        if cpp_type == "NodePath":
            return f"return JS_NewString(ctx, String({var_name}).utf8().get_data());"
        if cpp_type == "Vector2":
            return f"""JSValue ret_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret_obj, "x", JS_NewFloat64(ctx, {var_name}.x));
    JS_SetPropertyStr(ctx, ret_obj, "y", JS_NewFloat64(ctx, {var_name}.y));
    return ret_obj;"""
        if cpp_type == "Vector3":
            return f"""JSValue ret_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret_obj, "x", JS_NewFloat64(ctx, {var_name}.x));
    JS_SetPropertyStr(ctx, ret_obj, "y", JS_NewFloat64(ctx, {var_name}.y));
    JS_SetPropertyStr(ctx, ret_obj, "z", JS_NewFloat64(ctx, {var_name}.z));
    return ret_obj;"""
        if cpp_type == "Color":
            return f"""JSValue ret_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret_obj, "r", JS_NewFloat64(ctx, {var_name}.r));
    JS_SetPropertyStr(ctx, ret_obj, "g", JS_NewFloat64(ctx, {var_name}.g));
    JS_SetPropertyStr(ctx, ret_obj, "b", JS_NewFloat64(ctx, {var_name}.b));
    JS_SetPropertyStr(ctx, ret_obj, "a", JS_NewFloat64(ctx, {var_name}.a));
    return ret_obj;"""

        if cpp_type == "Quaternion":
            return f"""JSValue ret_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret_obj, "x", JS_NewFloat64(ctx, {var_name}.x));
    JS_SetPropertyStr(ctx, ret_obj, "y", JS_NewFloat64(ctx, {var_name}.y));
    JS_SetPropertyStr(ctx, ret_obj, "z", JS_NewFloat64(ctx, {var_name}.z));
    JS_SetPropertyStr(ctx, ret_obj, "w", JS_NewFloat64(ctx, {var_name}.w));
    return ret_obj;"""
        if cpp_type == "Basis":
            # Note: Basis stores internally as rows but exposes x/y/z as columns (axis directions)
            # Use get_column() to get the correct axis vectors
            return f"""JSValue ret_obj = JS_NewObject(ctx);
    JSValue x_obj = JS_NewObject(ctx);
    JSValue y_obj = JS_NewObject(ctx);
    JSValue z_obj = JS_NewObject(ctx);
    Vector3 col_x = {var_name}.get_column(0);
    Vector3 col_y = {var_name}.get_column(1);
    Vector3 col_z = {var_name}.get_column(2);
    JS_SetPropertyStr(ctx, x_obj, "x", JS_NewFloat64(ctx, col_x.x));
    JS_SetPropertyStr(ctx, x_obj, "y", JS_NewFloat64(ctx, col_x.y));
    JS_SetPropertyStr(ctx, x_obj, "z", JS_NewFloat64(ctx, col_x.z));
    JS_SetPropertyStr(ctx, y_obj, "x", JS_NewFloat64(ctx, col_y.x));
    JS_SetPropertyStr(ctx, y_obj, "y", JS_NewFloat64(ctx, col_y.y));
    JS_SetPropertyStr(ctx, y_obj, "z", JS_NewFloat64(ctx, col_y.z));
    JS_SetPropertyStr(ctx, z_obj, "x", JS_NewFloat64(ctx, col_z.x));
    JS_SetPropertyStr(ctx, z_obj, "y", JS_NewFloat64(ctx, col_z.y));
    JS_SetPropertyStr(ctx, z_obj, "z", JS_NewFloat64(ctx, col_z.z));
    JS_SetPropertyStr(ctx, ret_obj, "x", x_obj);
    JS_SetPropertyStr(ctx, ret_obj, "y", y_obj);
    JS_SetPropertyStr(ctx, ret_obj, "z", z_obj);
    return ret_obj;"""
        if cpp_type == "Transform3D":
            # Note: Basis stores internally as rows but exposes x/y/z as columns (axis directions)
            # Use get_column() to get the correct axis vectors
            return f"""JSValue ret_obj = JS_NewObject(ctx);
    JSValue basis_obj = JS_NewObject(ctx);
    JSValue origin_obj = JS_NewObject(ctx);
    JSValue bx_obj = JS_NewObject(ctx);
    JSValue by_obj = JS_NewObject(ctx);
    JSValue bz_obj = JS_NewObject(ctx);
    Vector3 col_x = {var_name}.basis.get_column(0);
    Vector3 col_y = {var_name}.basis.get_column(1);
    Vector3 col_z = {var_name}.basis.get_column(2);
    JS_SetPropertyStr(ctx, bx_obj, "x", JS_NewFloat64(ctx, col_x.x));
    JS_SetPropertyStr(ctx, bx_obj, "y", JS_NewFloat64(ctx, col_x.y));
    JS_SetPropertyStr(ctx, bx_obj, "z", JS_NewFloat64(ctx, col_x.z));
    JS_SetPropertyStr(ctx, by_obj, "x", JS_NewFloat64(ctx, col_y.x));
    JS_SetPropertyStr(ctx, by_obj, "y", JS_NewFloat64(ctx, col_y.y));
    JS_SetPropertyStr(ctx, by_obj, "z", JS_NewFloat64(ctx, col_y.z));
    JS_SetPropertyStr(ctx, bz_obj, "x", JS_NewFloat64(ctx, col_z.x));
    JS_SetPropertyStr(ctx, bz_obj, "y", JS_NewFloat64(ctx, col_z.y));
    JS_SetPropertyStr(ctx, bz_obj, "z", JS_NewFloat64(ctx, col_z.z));
    JS_SetPropertyStr(ctx, basis_obj, "x", bx_obj);
    JS_SetPropertyStr(ctx, basis_obj, "y", by_obj);
    JS_SetPropertyStr(ctx, basis_obj, "z", bz_obj);
    JS_SetPropertyStr(ctx, origin_obj, "x", JS_NewFloat64(ctx, {var_name}.origin.x));
    JS_SetPropertyStr(ctx, origin_obj, "y", JS_NewFloat64(ctx, {var_name}.origin.y));
    JS_SetPropertyStr(ctx, origin_obj, "z", JS_NewFloat64(ctx, {var_name}.origin.z));
    JS_SetPropertyStr(ctx, ret_obj, "basis", basis_obj);
    JS_SetPropertyStr(ctx, ret_obj, "origin", origin_obj);
    return ret_obj;"""

        if cpp_type == "Rect2":
            return f"""JSValue ret_obj = JS_NewObject(ctx);
    JSValue pos_obj = JS_NewObject(ctx);
    JSValue size_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, pos_obj, "x", JS_NewFloat64(ctx, {var_name}.position.x));
    JS_SetPropertyStr(ctx, pos_obj, "y", JS_NewFloat64(ctx, {var_name}.position.y));
    JS_SetPropertyStr(ctx, size_obj, "x", JS_NewFloat64(ctx, {var_name}.size.x));
    JS_SetPropertyStr(ctx, size_obj, "y", JS_NewFloat64(ctx, {var_name}.size.y));
    JS_SetPropertyStr(ctx, ret_obj, "position", pos_obj);
    JS_SetPropertyStr(ctx, ret_obj, "size", size_obj);
    return ret_obj;"""

        if cpp_type == "void":
            return "return JS_UNDEFINED;"

        # PackedVector2Array - return as JS array of Vector2 objects
        if cpp_type == "PackedVector2Array":
            return f"""JSValue arr = JS_NewArray(ctx);
    for (int i = 0; i < {var_name}.size(); i++) {{
        JSValue vec = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, vec, "x", JS_NewFloat64(ctx, {var_name}[i].x));
        JS_SetPropertyStr(ctx, vec, "y", JS_NewFloat64(ctx, {var_name}[i].y));
        JS_SetPropertyUint32(ctx, arr, i, vec);
    }}
    return arr;"""

        # PackedVector3Array - return as JS array of Vector3 objects
        if cpp_type == "PackedVector3Array":
            return f"""JSValue arr = JS_NewArray(ctx);
    for (int i = 0; i < {var_name}.size(); i++) {{
        JSValue vec = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, vec, "x", JS_NewFloat64(ctx, {var_name}[i].x));
        JS_SetPropertyStr(ctx, vec, "y", JS_NewFloat64(ctx, {var_name}[i].y));
        JS_SetPropertyStr(ctx, vec, "z", JS_NewFloat64(ctx, {var_name}[i].z));
        JS_SetPropertyUint32(ctx, arr, i, vec);
    }}
    return arr;"""

        # PackedColorArray - return as JS array of Color objects
        if cpp_type == "PackedColorArray":
            return f"""JSValue arr = JS_NewArray(ctx);
    for (int i = 0; i < {var_name}.size(); i++) {{
        JSValue col = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, col, "r", JS_NewFloat64(ctx, {var_name}[i].r));
        JS_SetPropertyStr(ctx, col, "g", JS_NewFloat64(ctx, {var_name}[i].g));
        JS_SetPropertyStr(ctx, col, "b", JS_NewFloat64(ctx, {var_name}[i].b));
        JS_SetPropertyStr(ctx, col, "a", JS_NewFloat64(ctx, {var_name}[i].a));
        JS_SetPropertyUint32(ctx, arr, i, col);
    }}
    return arr;"""

        # PackedFloat32Array - return as JS array of numbers
        if cpp_type == "PackedFloat32Array":
            return f"""JSValue arr = JS_NewArray(ctx);
    for (int i = 0; i < {var_name}.size(); i++) {{
        JS_SetPropertyUint32(ctx, arr, i, JS_NewFloat64(ctx, {var_name}[i]));
    }}
    return arr;"""

        # Enum types - return as int
        if "::" in cpp_type and not cpp_type.endswith("*") and not cpp_type.startswith("Ref<") and not cpp_type.startswith("BitField<"):
            return f"return JS_NewInt64(ctx, (int64_t){var_name});"

        # BitField types - return as int64
        if cpp_type.startswith("BitField<"):
            return f"return JS_NewInt64(ctx, (int64_t){var_name});"

        # Ref<T> types (RefCounted objects) - unwrap and wrap as JS object
        if cpp_type.startswith("Ref<") and cpp_type.endswith(">"):
            return f"""if ({var_name}.is_null()) return JS_NULL;
    Object* ret_obj_ptr = {var_name}.ptr();
    int64_t ret_handle = qjs_ctx->get_object_registry()->get_or_create_handle(ret_obj_ptr);
    // Store class name in local String to avoid dangling pointer from temporary
    String ret_class_str = ret_obj_ptr->get_class();
    CharString ret_class_utf8 = ret_class_str.utf8();
    const char* ret_class_name = ret_class_utf8.get_data();
    // Use __wrap_existing_godot_object to create a proper Proxy with method/property access
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue wrap_fn = JS_GetPropertyStr(ctx, global, "__wrap_existing_godot_object");
    if (JS_IsFunction(ctx, wrap_fn)) {{
        JSValue args[2] = {{ JS_NewInt64(ctx, ret_handle), JS_NewString(ctx, ret_class_name) }};
        JSValue wrapped = JS_Call(ctx, wrap_fn, JS_UNDEFINED, 2, args);
        JS_FreeValue(ctx, args[0]);
        JS_FreeValue(ctx, args[1]);
        JS_FreeValue(ctx, wrap_fn);
        JS_FreeValue(ctx, global);
        return wrapped;
    }}
    JS_FreeValue(ctx, wrap_fn);
    JS_FreeValue(ctx, global);
    // Fallback: return raw object
    JSValue ret_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret_obj, "__handle", JS_NewInt64(ctx, ret_handle));
    JS_SetPropertyStr(ctx, ret_obj, "__class", JS_NewString(ctx, ret_class_name));
    return ret_obj;"""

        # Object pointer types - wrap with JS Proxy using __wrap_existing_godot_object
        # Use reinterpret_cast to handle forward-declared types (all Node types inherit from Object)
        if cpp_type.endswith("*"):
            return f"""if (!{var_name}) return JS_NULL;
    Object* ret_obj_ptr = reinterpret_cast<Object*>({var_name});
    int64_t ret_handle = qjs_ctx->get_object_registry()->get_or_create_handle(ret_obj_ptr);
    // Store class name in local String to avoid dangling pointer from temporary
    String ret_class_str = ret_obj_ptr->get_class();
    CharString ret_class_utf8 = ret_class_str.utf8();
    const char* ret_class_name = ret_class_utf8.get_data();
    // Use __wrap_existing_godot_object to create a proper Proxy with method/property access
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue wrap_fn = JS_GetPropertyStr(ctx, global, "__wrap_existing_godot_object");
    if (JS_IsFunction(ctx, wrap_fn)) {{
        JSValue args[2] = {{ JS_NewInt64(ctx, ret_handle), JS_NewString(ctx, ret_class_name) }};
        JSValue wrapped = JS_Call(ctx, wrap_fn, JS_UNDEFINED, 2, args);
        JS_FreeValue(ctx, args[0]);
        JS_FreeValue(ctx, args[1]);
        JS_FreeValue(ctx, wrap_fn);
        JS_FreeValue(ctx, global);
        return wrapped;
    }}
    JS_FreeValue(ctx, wrap_fn);
    JS_FreeValue(ctx, global);
    // Fallback: return raw object (should not happen if bindings are set up correctly)
    JSValue ret_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ret_obj, "__handle", JS_NewInt64(ctx, ret_handle));
    JS_SetPropertyStr(ctx, ret_obj, "__class", JS_NewString(ctx, ret_class_name));
    return ret_obj;"""

        # Default: use variant conversion
        return f"return qjs_ctx->variant_to_js(Variant({var_name}));"

    def class_name_to_header(self, class_name: str) -> str:
        """Convert class name to header file name by looking up actual godot-cpp headers.

        This handles godot-cpp's inconsistent naming:
        - Node3D -> node3d
        - AnimationNodeBlendSpace1D -> animation_node_blend_space1_d (1D uses 1_d)
        - AnimationNodeBlendSpace2D -> animation_node_blend_space2d (2D stays 2d)
        - Generic6DOFJoint3D -> generic6_dof_joint3d
        """
        return self.find_header_for_class(class_name)

    def is_supported_type(self, godot_type: str) -> bool:
        """Check if a type is supported for binding.

        All types are supported EXCEPT raw pointers (unsafe low-level types).
        - Value types (bool, int, float, Vector2, etc.) are copied
        - Container types (Array, Dictionary, typedarray::*) use proxy
        - Object types (Node, Resource, etc.) use handle/proxy
        - Enums/bitfields are converted to int
        """
        # UNSAFE: Skip raw pointer types (low-level, unsafe)
        if godot_type.endswith("*"):
            return False

        # Support enum types - they're converted to int
        if godot_type.startswith("enum::"):
            return True

        # Support bitfield types - they're converted to int64
        if godot_type.startswith("bitfield::"):
            return True

        # Support typed arrays (returned as Godot Array via variant_to_js proxy)
        if godot_type.startswith("typedarray::"):
            return True

        # Support typed dictionaries (returned as Godot Dictionary)
        if godot_type.startswith("typeddictionary::"):
            return True

        # All value types we support (copied by value or converted via variant_to_js)
        value_types = {
            "void", "bool", "int", "float",
            "String", "StringName", "NodePath",
            "Vector2", "Vector2i", "Vector3", "Vector3i", "Vector4", "Vector4i",
            "Rect2", "Rect2i", "AABB", "Plane", "Projection",
            "Transform2D", "Transform3D", "Basis", "Quaternion",
            "Color", "RID", "Callable", "Signal",
            "Array", "Dictionary", "Variant",
            "PackedByteArray", "PackedInt32Array", "PackedInt64Array",
            "PackedFloat32Array", "PackedFloat64Array", "PackedStringArray",
            "PackedVector2Array", "PackedVector3Array", "PackedVector4Array", "PackedColorArray",
        }
        if godot_type in value_types:
            return True

        # Object types - support all Godot Object subclasses (Node, Resource, etc.)
        # These use handle/proxy wrapping via variant_to_js
        if self.is_object_type(godot_type):
            return True

        return False

    def is_node_type(self, godot_type: str) -> bool:
        """Check if a type is a Node subclass (not RefCounted/Resource)."""
        classes_by_name = {c.get("name"): c for c in self.api_data.get("classes", [])}
        if godot_type not in classes_by_name:
            return False

        # Walk up inheritance chain
        current = godot_type
        while current:
            if current == "Node":
                return True
            if current in ("RefCounted", "Resource", "Object"):
                return False  # Stop at these - not a Node
            cls_data = classes_by_name.get(current)
            if not cls_data:
                break
            current = cls_data.get("inherits", "")
        return False

    def is_object_type(self, godot_type: str) -> bool:
        """Check if a type is any Object subclass (not Node or RefCounted)."""
        classes_by_name = {c.get("name"): c for c in self.api_data.get("classes", [])}
        if godot_type not in classes_by_name:
            return False

        # Walk up inheritance chain to verify it inherits from Object
        current = godot_type
        while current:
            if current == "Object":
                return True
            cls_data = classes_by_name.get(current)
            if not cls_data:
                break
            current = cls_data.get("inherits", "")
        return False

    def extract_inner_type(self, godot_type: str) -> str | None:
        """Extract the inner type from typed arrays or other container types.
        Returns the inner type name or None if not a container type."""
        if godot_type.startswith("typedarray::"):
            return godot_type[12:]  # Remove "typedarray::" prefix
        return None

    def is_refcounted_type(self, godot_type: str) -> bool:
        """Check if a type is a RefCounted/Resource subclass (returns Ref<T>)."""
        classes_by_name = {c.get("name"): c for c in self.api_data.get("classes", [])}
        if godot_type not in classes_by_name:
            return False

        # Walk up inheritance chain
        current = godot_type
        while current:
            if current in ("RefCounted", "Resource"):
                return True
            if current in ("Node", "Object"):
                return False  # Stop at these - not a RefCounted
            cls_data = classes_by_name.get(current)
            if not cls_data:
                break
            current = cls_data.get("inherits", "")
        return False

    def collect_type_includes(self, godot_type: str, includes_set: set):
        """Collect all header includes needed for a type, including inner types of containers."""
        # Check the type itself
        if self.is_refcounted_type(godot_type):
            includes_set.add(self.class_name_to_header(godot_type))

        # Check inner types (e.g., typedarray::RDFramebufferPass)
        inner_type = self.extract_inner_type(godot_type)
        if inner_type and self.is_refcounted_type(inner_type):
            includes_set.add(self.class_name_to_header(inner_type))

    def is_supported_return_type(self, godot_type: str) -> bool:
        """Check if a type is supported as a method return type.
        Uses same logic as is_supported_type - all types except raw pointers are supported.
        """
        return self.is_supported_type(godot_type)

    def is_object_type(self, godot_type: str) -> bool:
        """Check if a type is a Godot Object subclass."""
        # Known object base classes and common types
        object_types = {"Object", "Node", "Resource", "RefCounted"}
        if godot_type in object_types:
            return True

        # Check if it's a class we know about from the API
        classes_by_name = {c.get("name"): c for c in self.api_data.get("classes", [])}
        if godot_type in classes_by_name:
            # Check if it inherits from Object
            current = godot_type
            while current:
                if current == "Object":
                    return True
                cls_data = classes_by_name.get(current)
                if not cls_data:
                    break
                current = cls_data.get("inherits", "")
            return False
        return False

    # Keep old name as alias for compatibility
    def is_simple_type(self, godot_type: str) -> bool:
        return self.is_supported_type(godot_type)

    def process_method(self, class_name: str, method_data: dict) -> MethodInfo | None:
        """Process a method from API data into MethodInfo."""
        method_name = method_data.get("name", "")

        # Skip internal methods (prefixed with _)
        if method_name.startswith("_"):
            return None

        # Skip blocked methods
        if self.is_method_blocked(class_name, method_name):
            return None

        # Skip virtual methods (they're for overriding, not calling)
        if method_data.get("is_virtual", False):
            return None

        # Skip vararg methods (complex to bind)
        if method_data.get("is_vararg", False):
            return None

        # Check return type is supported (stricter for return types due to forward declarations)
        return_type = method_data.get("return_value", {}).get("type", "void")
        if not self.is_supported_return_type(return_type):
            return None

        # Check all argument types are simple (parameters are more flexible)
        for arg_data in method_data.get("arguments", []):
            arg_type = arg_data.get("type", "Variant")
            if not self.is_simple_type(arg_type):
                return None

        return_type = method_data.get("return_value", {}).get("type", "void")
        return_cpp_type = self.get_cpp_type(return_type, for_return=True) if return_type != "void" else "void"

        method = MethodInfo(
            name=method_name,
            return_type=return_type,
            return_cpp_type=return_cpp_type,
            return_conversion=self.get_cpp_to_js_conversion(return_cpp_type),
            is_static=method_data.get("is_static", False),
            is_virtual=method_data.get("is_virtual", False),
        )

        # Process arguments
        args_data = method_data.get("arguments", [])
        call_args = []

        for i, arg_data in enumerate(args_data):
            arg_name = arg_data.get("name", f"arg{i}")
            arg_type = arg_data.get("type", "Variant")
            cpp_type = self.get_cpp_type(arg_type)
            raw_default = arg_data.get("default_value")
            # Convert Godot default values to C++ format
            default_value = self.convert_default_value(raw_default, cpp_type, arg_type) if raw_default is not None else None
            is_optional = default_value is not None

            # Generate conversion code
            arg_index = i + 1  # +1 because argv[0] is the handle
            conversion = self.get_js_to_cpp_conversion(cpp_type, arg_index, arg_name, arg_type)

            arg = MethodArg(
                name=arg_name,
                type=arg_type,
                cpp_type=cpp_type,
                conversion=conversion,
                default_value=default_value,
                is_optional=is_optional,
                arg_index=i,  # 0-based index (template adds 1 for handle)
            )
            method.arguments.append(arg)
            call_args.append(f"arg_{arg_name}")

        method.call_args = ", ".join(call_args)

        # Calculate required argument count (args without default values)
        method.required_arg_count = sum(1 for arg in method.arguments if arg.default_value is None)

        # Check for method alias (e.g., get_node_internal -> get_node in JS)
        alias = self.get_method_alias(class_name, method_name)
        method.js_name = alias if alias else method_name

        # Check for C++ name override (e.g., API 'get_node' -> C++ 'get_node_internal')
        method.cpp_name = self.get_method_cpp_name(class_name, method_name)

        # Determine API category for rate limiting (PRD Section 6.4)
        method.api_category = self.get_method_api_category(method_name)

        return method

    def process_property(self, class_name: str, prop_data: dict, class_methods: list) -> PropertyInfo | None:
        """Process a property from API data into PropertyInfo."""
        prop_name = prop_data.get("name", "")

        # Skip blocked properties
        if self.is_property_blocked(class_name, prop_name):
            return None

        prop_type = prop_data.get("type", "Variant")

        # Check if type is supported (simple types OR RefCounted types)
        is_simple = self.is_simple_type(prop_type)
        is_refcounted = self.is_refcounted_type(prop_type)
        if not is_simple and not is_refcounted:
            return None

        getter = prop_data.get("getter", "")
        setter = prop_data.get("setter", "")

        if not getter:
            return None

        # Skip properties with internal getter/setter (prefixed with _)
        if getter.startswith("_") or (setter and setter.startswith("_")):
            return None

        # Check if getter has parameters (indexed properties like get_param(enum))
        # Also check if getter return type matches property type
        getter_return_type = None
        for method in class_methods:
            if method.get("name") == getter:
                if method.get("arguments"):
                    return None  # Indexed property - getter takes parameters
                getter_return_type = method.get("return_value", {}).get("type", prop_type)
                break

        # Skip if getter return type differs from property type (type mismatch like Viewport vs Node)
        if getter_return_type and getter_return_type != prop_type:
            # Allow int vs enum mismatches (common pattern)
            if not (getter_return_type == "int" or prop_type == "int"):
                return None

        # Check if setter has unsupported parameter types or multiple parameters (indexed properties)
        if setter:
            for method in class_methods:
                if method.get("name") == setter:
                    args = method.get("arguments", [])
                    # Skip if setter takes more than 1 argument (indexed property)
                    if len(args) > 1:
                        return None
                    for arg in args:
                        arg_type = arg.get("type", "")
                        # Allow simple types and RefCounted types
                        if not self.is_simple_type(arg_type) and not self.is_refcounted_type(arg_type):
                            return None  # Setter uses unsupported type
                    break

        cpp_type = self.get_cpp_type(prop_type)

        # Determine the actual setter parameter type from the setter method
        setter_param_type = prop_type
        setter_cast = ""
        if setter:
            for method in class_methods:
                if method.get("name") == setter:
                    args = method.get("arguments", [])
                    if args:
                        setter_param_type = args[0].get("type", prop_type)
                    break

        # Use setter param type for the conversion (it may differ from property type)
        setter_cpp_type = self.get_cpp_type(setter_param_type)

        # If setter param is an enum, we need a cast
        if setter_param_type.startswith("enum::"):
            setter_cast = f"({setter_cpp_type})"
            from_js_conv = "int64_t value; JS_ToInt64(ctx, &value, argv[1]);"
        elif prop_type.startswith("enum::"):
            setter_cast = f"({cpp_type})"
            from_js_conv = "int64_t value; JS_ToInt64(ctx, &value, argv[1]);"
        # Handle bitfield types - similar to enums
        elif setter_param_type.startswith("bitfield::"):
            setter_cast = f"({setter_cpp_type})"
            from_js_conv = "int64_t value; JS_ToInt64(ctx, &value, argv[1]);"
        elif prop_type.startswith("bitfield::"):
            setter_cast = f"({cpp_type})"
            from_js_conv = "int64_t value; JS_ToInt64(ctx, &value, argv[1]);"
        else:
            # Use setter param type for conversion (e.g., setter expects Node* even if property is Viewport)
            from_js_conv = self.get_js_to_cpp_conversion(setter_cpp_type, 1, "value").replace("arg_value", "value")

        return PropertyInfo(
            name=prop_name,
            type=prop_type,
            cpp_type=cpp_type,
            getter=getter,
            setter=setter if setter else None,
            readonly=not bool(setter),
            to_js_conversion=self.get_cpp_to_js_conversion(cpp_type, "value"),  # Property getter uses "value" variable
            from_js_conversion=from_js_conv,
            setter_cast=setter_cast,
        )

    def get_all_methods_for_class(self, class_name: str) -> list:
        """Get all methods for a class including inherited methods from parents."""
        all_methods = []
        classes_by_name = {c.get("name"): c for c in self.api_data.get("classes", [])}

        current = class_name
        while current and current in classes_by_name:
            class_data = classes_by_name[current]
            all_methods.extend(class_data.get("methods", []))
            current = class_data.get("inherits", "")

        return all_methods

    def process_class(self, class_data: dict) -> ClassInfo | None:
        """Process a class from API data into ClassInfo."""
        class_name = class_data.get("name", "")

        if self.is_class_blocked(class_name):
            return None

        # Generate bindings for both instantiable and non-instantiable classes
        # Non-instantiable (abstract) classes are still needed for inheritance lookup
        is_instantiable = class_data.get("is_instantiable", True)

        parent = class_data.get("inherits", "")

        class_info = ClassInfo(
            name=class_name,
            parent_class=parent,
            header_name=self.class_name_to_header(class_name),
            is_instantiable=is_instantiable,
        )

        # Collect property getters/setters to skip as methods
        property_methods = set()
        # Also collect method names that would conflict with auto-generated property accessors
        # Template generates: js_Class_get_PropertyName and js_Class_set_PropertyName
        # So methods named "get_PropertyName" or "set_PropertyName" would conflict
        property_name_conflicts = set()
        for prop_data in class_data.get("properties", []):
            prop_name = prop_data.get("name", "")
            if prop_data.get("getter"):
                property_methods.add(prop_data["getter"])
            if prop_data.get("setter"):
                property_methods.add(prop_data["setter"])
            # Track potential naming conflicts
            property_name_conflicts.add(f"get_{prop_name}")
            property_name_conflicts.add(f"set_{prop_name}")

        class_methods = class_data.get("methods", [])

        # Track RefCounted types that need extra includes
        refcounted_includes = set()

        # Process methods (skip property accessors and conflicting names)
        for method_data in class_methods:
            method_name = method_data.get("name", "")
            if method_name in property_methods:
                continue  # Skip, will be generated as property accessor
            if method_name in property_name_conflicts:
                continue  # Skip, would conflict with auto-generated property accessor name
            method = self.process_method(class_name, method_data)
            if method:
                class_info.methods.append(method)
                # Collect RefCounted types used in method arguments and return type
                # (including inner types of typed arrays)
                return_type = method_data.get("return_value", {}).get("type", "void")
                self.collect_type_includes(return_type, refcounted_includes)
                for arg_data in method_data.get("arguments", []):
                    arg_type = arg_data.get("type", "Variant")
                    self.collect_type_includes(arg_type, refcounted_includes)

        # Get all methods including from parent classes for property getter/setter lookup
        all_methods = self.get_all_methods_for_class(class_name)

        # Process properties
        for prop_data in class_data.get("properties", []):
            prop = self.process_property(class_name, prop_data, all_methods)
            if prop:
                class_info.properties.append(prop)
                # Collect RefCounted types used in properties (including inner types)
                prop_type = prop_data.get("type", "Variant")
                self.collect_type_includes(prop_type, refcounted_includes)

        # Add extra includes for RefCounted types (from methods and properties)
        class_info.extra_includes = list(refcounted_includes)

        # Extract signal names for GDScript 4.x syntax support
        for signal_data in class_data.get("signals", []):
            signal_name = signal_data.get("name", "")
            if signal_name:
                class_info.signals.append(signal_name)

        return class_info

    def generate(self):
        """Generate all binding files."""
        self.load_config()
        self.load_api()

        # Process all classes except blocked ones
        for class_data in self.api_data.get("classes", []):
            class_name = class_data.get("name", "")

            # Skip blocked classes
            if self.is_class_blocked(class_name):
                continue

            class_info = self.process_class(class_data)
            if class_info:
                self.classes[class_name] = class_info

        # Create output directory
        self.output_dir.mkdir(parents=True, exist_ok=True)

        # Generate individual class binding files
        class_template = self.jinja_env.get_template("class_binding.cpp.j2")

        for class_name, class_info in self.classes.items():
            output_path = self.output_dir / f"{class_info.header_name}_bindings.gen.cpp"

            content = class_template.render(
                class_name=class_info.name,
                parent_class=class_info.parent_class,
                header_name=class_info.header_name,
                methods=class_info.methods,
                properties=class_info.properties,
                signals=class_info.signals,
                extra_includes=class_info.extra_includes,
                is_instantiable=class_info.is_instantiable,
            )

            with open(output_path, "w") as f:
                f.write(content)

            print(f"Generated: {output_path.name}")

        # Generate header file
        header_template = self.jinja_env.get_template("generated_classes.h.j2")
        header_content = header_template.render(classes=sorted(self.classes.keys()))

        with open(self.output_dir / "generated_classes.gen.h", "w") as f:
            f.write(header_content)

        print(f"Generated: generated_classes.gen.h")

        # Generate registration file
        register_template = self.jinja_env.get_template("register_all.cpp.j2")
        register_content = register_template.render(classes=sorted(self.classes.keys()))

        with open(self.output_dir / "register_all.gen.cpp", "w") as f:
            f.write(register_content)

        print(f"Generated: register_all.gen.cpp")

        # Generate global enums file
        self.generate_global_enums()

        # Generate singleton class enums file
        self.generate_singleton_enums()

        # Generate packed array bindings
        self.generate_packed_array_bindings()

        # Generate math type constructors
        self.generate_math_constructors()

        print(f"\nGenerated bindings for {len(self.classes)} classes")

    def generate_global_enums(self):
        """Generate global enums (Key, MouseButton, etc.) as JS constants."""
        global_enums = self.api_data.get("global_enums", [])

        if not global_enums:
            return

        # Generate the C++ file for global enums
        enums_template = self.jinja_env.get_template("global_enums.cpp.j2")
        enums_content = enums_template.render(enums=global_enums)

        with open(self.output_dir / "global_enums.gen.cpp", "w") as f:
            f.write(enums_content)

        print(f"Generated: global_enums.gen.cpp ({len(global_enums)} enums)")

    def generate_singleton_enums(self):
        """Generate singleton class enums (Input.MouseMode, etc.) as properties on singleton objects."""
        # Get list of singletons
        singletons_list = self.api_data.get("singletons", [])
        singleton_names = {s.get("type", s.get("name", "")) for s in singletons_list}

        # Find classes that are singletons and have enums
        classes_by_name = {c.get("name"): c for c in self.api_data.get("classes", [])}

        singletons_with_enums = []
        total_enum_values = 0

        for singleton_info in singletons_list:
            singleton_name = singleton_info.get("name", "")
            singleton_type = singleton_info.get("type", singleton_name)

            class_data = classes_by_name.get(singleton_type)
            if not class_data:
                continue

            class_enums = class_data.get("enums", [])
            if not class_enums:
                continue

            # Convert enum data to simple dict format for template
            enums_data = []
            for enum in class_enums:
                enum_values = []
                for value in enum.get("values", []):
                    enum_values.append({
                        "name": value.get("name", ""),
                        "value": value.get("value", 0)
                    })
                    total_enum_values += 1
                enums_data.append({
                    "name": enum.get("name", ""),
                    "values": enum_values
                })

            singletons_with_enums.append({
                "name": singleton_name,
                "enums": enums_data
            })

        if not singletons_with_enums:
            return

        # Generate the C++ file for singleton enums
        enums_template = self.jinja_env.get_template("singleton_enums.cpp.j2")
        enums_content = enums_template.render(singletons=singletons_with_enums)

        with open(self.output_dir / "singleton_enums.gen.cpp", "w") as f:
            f.write(enums_content)

        print(f"Generated: singleton_enums.gen.cpp ({len(singletons_with_enums)} singletons, {total_enum_values} enum values)")

        # Also generate class enums for non-singleton classes that have important enums
        self.generate_class_enums()

    def generate_class_enums(self):
        """Generate class enums for non-singleton classes (RenderingDevice, etc.) as global objects."""
        # List of non-singleton classes whose enums should be exposed globally
        # These are classes that JS code commonly needs enum constants from
        target_classes = ["RenderingDevice"]

        classes_by_name = {c.get("name"): c for c in self.api_data.get("classes", [])}

        classes_with_enums = []
        total_enum_values = 0

        for class_name in target_classes:
            class_data = classes_by_name.get(class_name)
            if not class_data:
                continue

            class_enums = class_data.get("enums", [])
            if not class_enums:
                continue

            # Convert enum data to simple dict format for template
            enums_data = []
            for enum in class_enums:
                enum_values = []
                for value in enum.get("values", []):
                    enum_values.append({
                        "name": value.get("name", ""),
                        "value": value.get("value", 0)
                    })
                    total_enum_values += 1
                enums_data.append({
                    "name": enum.get("name", ""),
                    "values": enum_values
                })

            classes_with_enums.append({
                "name": class_name,
                "enums": enums_data
            })

        if not classes_with_enums:
            return

        # Generate the C++ file for class enums
        enums_template = self.jinja_env.get_template("class_enums.cpp.j2")
        enums_content = enums_template.render(classes=classes_with_enums)

        with open(self.output_dir / "class_enums.gen.cpp", "w") as f:
            f.write(enums_content)

        print(f"Generated: class_enums.gen.cpp ({len(classes_with_enums)} classes, {total_enum_values} enum values)")

    def generate_packed_array_bindings(self):
        """Generate packed array bindings (constructors, get, set, push, size, resize)."""
        # Define packed array types with their properties
        packed_array_types = [
            {
                "name": "PackedByteArray",
                "snake_name": "packed_byte_array",
                "element_type": "uint8_t",
                "is_struct": False,
                "is_float": False,
                "fields": [],
            },
            {
                "name": "PackedInt32Array",
                "snake_name": "packed_int32_array",
                "element_type": "int32_t",
                "is_struct": False,
                "is_float": False,
                "fields": [],
            },
            {
                "name": "PackedInt64Array",
                "snake_name": "packed_int64_array",
                "element_type": "int64_t",
                "is_struct": False,
                "is_float": False,
                "fields": [],
            },
            {
                "name": "PackedFloat32Array",
                "snake_name": "packed_float32_array",
                "element_type": "float",
                "is_struct": False,
                "is_float": True,
                "fields": [],
            },
            {
                "name": "PackedFloat64Array",
                "snake_name": "packed_float64_array",
                "element_type": "double",
                "is_struct": False,
                "is_float": True,
                "fields": [],
            },
            {
                "name": "PackedStringArray",
                "snake_name": "packed_string_array",
                "element_type": "String",
                "is_struct": False,
                "is_float": False,
                "fields": [],
            },
            {
                "name": "PackedVector2Array",
                "snake_name": "packed_vector2_array",
                "element_type": "Vector2",
                "is_struct": True,
                "is_float": True,
                "fields": ["x", "y"],
            },
            {
                "name": "PackedVector3Array",
                "snake_name": "packed_vector3_array",
                "element_type": "Vector3",
                "is_struct": True,
                "is_float": True,
                "fields": ["x", "y", "z"],
            },
            {
                "name": "PackedVector4Array",
                "snake_name": "packed_vector4_array",
                "element_type": "Vector4",
                "is_struct": True,
                "is_float": True,
                "fields": ["x", "y", "z", "w"],
            },
            {
                "name": "PackedColorArray",
                "snake_name": "packed_color_array",
                "element_type": "Color",
                "is_struct": True,
                "is_float": True,
                "fields": ["r", "g", "b", "a"],
            },
        ]

        # Generate packed array bindings (constructors, get, set, push, size, resize)
        template = self.jinja_env.get_template("packed_array_bindings.cpp.j2")
        content = template.render(packed_array_types=packed_array_types)

        with open(self.output_dir / "packed_array_bindings.gen.cpp", "w") as f:
            f.write(content)

        print(f"Generated: packed_array_bindings.gen.cpp ({len(packed_array_types)} packed array types)")

        # Generate packed array conversion functions (variant_to_js and js_to_variant helpers)
        conversion_template = self.jinja_env.get_template("packed_array_conversion.cpp.j2")
        conversion_content = conversion_template.render(packed_array_types=packed_array_types)

        with open(self.output_dir / "packed_array_conversion.gen.cpp", "w") as f:
            f.write(conversion_content)

        print(f"Generated: packed_array_conversion.gen.cpp")

    def generate_math_constructors(self):
        """Generate math type constructors (Vector2, Vector3, Color, etc.)."""
        # Define simple vector types that follow the same pattern
        simple_vector_types = [
            {
                "name": "Vector2",
                "snake_name": "vector2",
                "fields": ["x", "y"],
                "component_type": "double",
                "is_int": False,
            },
            {
                "name": "Vector2i",
                "snake_name": "vector2i",
                "fields": ["x", "y"],
                "component_type": "int32_t",
                "is_int": True,
            },
            {
                "name": "Vector3",
                "snake_name": "vector3",
                "fields": ["x", "y", "z"],
                "component_type": "double",
                "is_int": False,
            },
            {
                "name": "Vector3i",
                "snake_name": "vector3i",
                "fields": ["x", "y", "z"],
                "component_type": "int32_t",
                "is_int": True,
            },
            {
                "name": "Vector4",
                "snake_name": "vector4",
                "fields": ["x", "y", "z", "w"],
                "component_type": "double",
                "is_int": False,
            },
            {
                "name": "Vector4i",
                "snake_name": "vector4i",
                "fields": ["x", "y", "z", "w"],
                "component_type": "int32_t",
                "is_int": True,
            },
            {
                "name": "Color",
                "snake_name": "color",
                "fields": ["r", "g", "b", "a"],
                "component_type": "double",
                "is_int": False,
            },
            {
                "name": "Quaternion",
                "snake_name": "quaternion",
                "fields": ["x", "y", "z", "w"],
                "component_type": "double",
                "is_int": False,
            },
        ]

        # Generate using template
        template = self.jinja_env.get_template("math_constructors.cpp.j2")
        content = template.render(simple_vector_types=simple_vector_types)

        with open(self.output_dir / "math_constructors.gen.cpp", "w") as f:
            f.write(content)

        print(f"Generated: math_constructors.gen.cpp ({len(simple_vector_types)} simple types + complex types)")


def main():
    parser = argparse.ArgumentParser(description="Generate GodotJSRuntime bindings")
    parser.add_argument(
        "--api-json",
        type=Path,
        default=Path(__file__).parent.parent / "extension_api.json",
        help="Path to extension_api.json",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).parent.parent / "generated",
        help="Output directory for generated files",
    )
    parser.add_argument(
        "--config-dir",
        type=Path,
        default=Path(__file__).parent / "config",
        help="Directory containing config files",
    )
    parser.add_argument(
        "--templates-dir",
        type=Path,
        default=Path(__file__).parent / "templates",
        help="Directory containing Jinja2 templates",
    )

    args = parser.parse_args()

    if not args.api_json.exists():
        print(f"Error: extension_api.json not found at {args.api_json}")
        print("Generate it with: godot --dump-extension-api")
        sys.exit(1)

    generator = BindingGenerator(
        api_json_path=args.api_json,
        output_dir=args.output_dir,
        config_dir=args.config_dir,
        templates_dir=args.templates_dir,
    )

    generator.generate()


if __name__ == "__main__":
    main()

/* godot-cpp integration testing project.
 *
 * This is free and unencumbered software released into the public domain.
 */

#pragma once

#include <cstdarg>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/cowdata.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/hashfuncs.hpp>
#include <godot_cpp/templates/list.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/templates/pair.hpp>
#include <godot_cpp/templates/rb_map.hpp>
#include <godot_cpp/templates/rb_set.hpp>
#include <godot_cpp/templates/rid_owner.hpp>
#include <godot_cpp/templates/safe_refcount.hpp>
#include <godot_cpp/templates/search_array.hpp>
#include <godot_cpp/templates/self_list.hpp>
#include <godot_cpp/templates/sort_array.hpp>
#include <godot_cpp/templates/spin_lock.hpp>
#include <godot_cpp/templates/thread_work_pool.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/vmap.hpp>
#include <godot_cpp/templates/vset.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector4.hpp>

using namespace godot;

/* Unit conversion helpers for Unreal <-> Godot interoperability.
 *
 * Notes:
 * - Unreal Engine's default unit is centimeters (cm).
 * - Godot's conventional 3D unit is meters (m).
 *
 * Usage:
 * - To convert an Unreal value (in cm) to Godot meters: meters = cm * CM_TO_M
 * - To convert a Godot value (in m) to Unreal centimeters: cm = meters * M_TO_CM
 *
 * Examples:
 * - Unreal location X = 100.0f (cm) -> Godot: 100.0f * CM_TO_M = 1.0f (m)
 * - Godot scale or length = 2.5f (m) -> Unreal: 2.5f * M_TO_CM = 250.0f (cm)
 *
 * These are constexpr floats so they can be used in compile-time calculations
 * and avoid implicit double-to-float conversions (notice the 'f' suffix).
 */
constexpr float CM_TO_M = 0.01f;
constexpr float M_TO_CM = 100.0f;

// Define a project-local TCHAR_TO_UTF8 macro that produces a UTF-8 Godot String.
#define TCHAR_TO_UTF8(str) String::utf8(str)

// Platform defaults
// ===============================
#define PLATFORM_WINDOWS 0
#define PLATFORM_LINUX 0
#define PLATFORM_MAC 0

// ===============================
// Platform detect
// ===============================
#if defined(_WIN32) || defined(_WIN64)
#undef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 1

#elif defined(__linux__)
#undef PLATFORM_LINUX
#define PLATFORM_LINUX 1

#elif defined(__APPLE__)
#undef PLATFORM_MAC
#define PLATFORM_MAC 1
#endif

// ===============================
// Sanity check
// ===============================
#if (PLATFORM_WINDOWS + PLATFORM_LINUX + PLATFORM_MAC) != 1
#error "Platform detection error: exactly one platform must be defined"
#endif

// ============================================================
// Variant type mapping utilities
// ============================================================

namespace VariantUtils
{

    template <typename T> constexpr Variant::Type get_variant_type();

    // ------------------------
    // Basic Types
    // ------------------------
    template <> constexpr Variant::Type get_variant_type<float>()
    {
        return Variant::FLOAT;
    }
    template <> constexpr Variant::Type get_variant_type<double>()
    {
        return Variant::FLOAT;
    } // double maps to FLOAT in Godot 4
    template <> constexpr Variant::Type get_variant_type<int>()
    {
        return Variant::INT;
    }
    template <> constexpr godot::Variant::Type get_variant_type<int64_t>()
    {
        return Variant::INT;
    }
    template <> constexpr Variant::Type get_variant_type<bool>()
    {
        return Variant::BOOL;
    }
    template <> constexpr Variant::Type get_variant_type<String>()
    {
        return Variant::STRING;
    }

    // ------------------------
    // Vectors and Color
    // ------------------------
    template <> constexpr Variant::Type get_variant_type<Vector2>()
    {
        return Variant::VECTOR2;
    }
    template <> constexpr Variant::Type get_variant_type<Vector3>()
    {
        return Variant::VECTOR3;
    }
    template <> constexpr Variant::Type get_variant_type<Vector4>()
    {
        return Variant::VECTOR4;
    }
    template <> constexpr Variant::Type get_variant_type<Color>()
    {
        return Variant::COLOR;
    }

    // ------------------------
    // Transform-related types
    // ------------------------
    template <> constexpr Variant::Type get_variant_type<Transform2D>()
    {
        return Variant::TRANSFORM2D;
    }
    template <> constexpr Variant::Type get_variant_type<Transform3D>()
    {
        return Variant::TRANSFORM3D;
    }
    template <> constexpr Variant::Type get_variant_type<Basis>()
    {
        return Variant::BASIS;
    }
    template <> constexpr Variant::Type get_variant_type<Quaternion>()
    {
        return Variant::QUATERNION;
    } // Changed from QUAT

    // ------------------------
    // Godot Container Types
    // ------------------------
    template <> constexpr Variant::Type get_variant_type<Array>()
    {
        return Variant::ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<Dictionary>()
    {
        return Variant::DICTIONARY;
    }
    template <> constexpr Variant::Type get_variant_type<PackedByteArray>()
    {
        return Variant::PACKED_BYTE_ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<PackedInt32Array>()
    {
        return Variant::PACKED_INT32_ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<PackedFloat32Array>()
    {
        return Variant::PACKED_FLOAT32_ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<PackedVector2Array>()
    {
        return Variant::PACKED_VECTOR2_ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<PackedVector3Array>()
    {
        return Variant::PACKED_VECTOR3_ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<PackedColorArray>()
    {
        return Variant::PACKED_COLOR_ARRAY;
    }

    // TypedArray variants
    template <> constexpr Variant::Type get_variant_type<godot::TypedArray<int>>()
    {
        return Variant::ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<godot::TypedArray<Vector2>>()
    {
        return Variant::ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<godot::TypedArray<Vector3>>()
    {
        return Variant::ARRAY;
    }
    template <> constexpr Variant::Type get_variant_type<godot::TypedArray<Vector4>>()
    {
        return Variant::ARRAY;
    }

} // namespace VariantUtils

// ============================================================
// GDPROPERTY macro definition (supports class_name parameter, improved semantics)
// ============================================================

/**
 * @brief Declares a property with automatic getter/setter and binding to Godot's ClassDB.
 *
 * @param class_name     The name of the class where this property is defined
 * @param variable_type  The type of the property
 * @param variable_name  The property name
 * @param default_val    Default value
 *
 * Example usage:
 * ```
 * class AFloatingActor : public Node3D {
 *     GDCLASS(AFloatingActor, Node3D);
 *
 *     GDPROPERTY(AFloatingActor, float, HeightAmplitude, 20.0f);
 *
 * public:
 *     static void _bind_methods() {
 *         _bind_HeightAmplitude();
 *     }
 * };
 * ```
 */
#define GPROPERTY(class_name, variable_type, variable_name, default_val)                                                                                                                               \
public:                                                                                                                                                                                                \
    variable_type variable_name = default_val;                                                                                                                                                         \
                                                                                                                                                                                                       \
    void set_##variable_name(const variable_type& value)                                                                                                                                               \
    {                                                                                                                                                                                                  \
        variable_name = value;                                                                                                                                                                         \
    }                                                                                                                                                                                                  \
    variable_type get_##variable_name() const                                                                                                                                                          \
    {                                                                                                                                                                                                  \
        return variable_name;                                                                                                                                                                          \
    }                                                                                                                                                                                                  \
                                                                                                                                                                                                       \
    static void bind_property_##variable_name()                                                                                                                                                        \
    {                                                                                                                                                                                                  \
        ClassDB::bind_method(D_METHOD("get_" #variable_name), &class_name::get_##variable_name);                                                                                                       \
        ClassDB::bind_method(D_METHOD("set_" #variable_name, #variable_name), &class_name::set_##variable_name);                                                                                       \
        ADD_PROPERTY(PropertyInfo(VariantUtils::get_variant_type<variable_type>(), #variable_name), "set_" #variable_name, "get_" #variable_name);                                                     \
    }

/**
 * @brief Declares a property with automatic getter/setter and binds it to Godot's ClassDB,
 *        while allowing the property to appear under a custom category in the Inspector.
 *
 * This macro generates:
 *   - A member variable with a default value
 *   - Getter and setter functions
 *   - A static binder that registers the property to Godot with an optional category
 *
 * @param class_name     The class in which this property is defined
 * @param variable_type  The C++ type of the property
 * @param variable_name  The property name (Godot Inspector key)
 * @param default_val    Default value of the property
 * @param category       Inspector category (string). If you want the class name as category,
 *                       simply pass #class_name when invoking this macro.
 *
 * Example:
 * ```
 * GPROPERTY_CAT(MyActor, float, Speed, 5.0f, "Movement")
 * ```
 *
 * Inspector result:
 *   Movement/Speed
 */
#define GPROPERTY_CAT(class_name, variable_type, variable_name, default_val, category)                                                                                                                 \
public:                                                                                                                                                                                                \
    /* Actual property storage with default value */                                                                                                                                                   \
    variable_type variable_name = default_val;                                                                                                                                                         \
                                                                                                                                                                                                       \
    /* Setter: assigns the value */                                                                                                                                                                    \
    void set_##variable_name(const variable_type& value)                                                                                                                                               \
    {                                                                                                                                                                                                  \
        variable_name = value;                                                                                                                                                                         \
    }                                                                                                                                                                                                  \
                                                                                                                                                                                                       \
    /* Getter: returns the stored value */                                                                                                                                                             \
    variable_type get_##variable_name() const                                                                                                                                                          \
    {                                                                                                                                                                                                  \
        return variable_name;                                                                                                                                                                          \
    }                                                                                                                                                                                                  \
                                                                                                                                                                                                       \
    /* Property binding for Godot's ClassDB */                                                                                                                                                         \
    static void bind_property_##variable_name()                                                                                                                                                        \
    {                                                                                                                                                                                                  \
        /* Register getter and setter */                                                                                                                                                               \
        ClassDB::bind_method(D_METHOD("get_" #variable_name), &class_name::get_##variable_name);                                                                                                       \
        ClassDB::bind_method(D_METHOD("set_" #variable_name, #variable_name), &class_name::set_##variable_name);                                                                                       \
                                                                                                                                                                                                       \
        ADD_GROUP(#category, #category);                                                                                                                                                               \
        /* Register the property in the Inspector */                                                                                                                                                   \
        ADD_PROPERTY(PropertyInfo(VariantUtils::get_variant_type<variable_type>(), #variable_name), "set_" #variable_name, "get_" #variable_name);                                                     \
    }

/**============================================================================
GFUNCTION(ClassName, MethodName, ...)

Summary:
   Registers a C++ member function to the Godot ClassDB using a compact,
   Unreal-style macro format. This macro supports zero or multiple argument
   names and automatically forwards them to D_METHOD.

Usage Examples:
   GFUNCTION(MyClass, DoSomething);
   GFUNCTION(MyClass, GetValue, "id");
   GFUNCTION(MyClass, GetPixelColor, "x", "y");
   GFUNCTION(MyClass, RegisterMethod, "object", "method");

Expanded Form:
   ClassDB::bind_method(
       D_METHOD("MethodName", ...),
       &ClassName::MethodName
   );

Notes:
   - #MethodName converts the symbol into a string literal (e.g. Foo → "Foo").
   - __VA_ARGS__ allows passing any number of argument names.
   - The '##' operator removes the trailing comma when no arguments are given.
   - This macro mirrors the declarative style of Unreal's UFUNCTION,
     providing a clean, readable registration syntax for Godot modules.

============================================================================*/
#define GFUNCTION(ClassName, MethodName, ...) ClassDB::bind_method(D_METHOD(#MethodName, ##__VA_ARGS__), &ClassName::MethodName)

/**============================================================================
GFUNCTION_DISPLAY(ClassName, MethodName, DisplayNameStr, ...)

Summary:
   Registers a C++ member function to Godot's ClassDB with a custom
   display name. Supports zero or multiple argument names.

Usage Examples:
   GFUNCTION_DISPLAY(MyClass, SetPosition, "Set Position", "position");
   GFUNCTION_DISPLAY(MyClass, RegisterMethod, "Register Custom", "object", "method");

Expanded Form:
   ClassDB::bind_method(
       D_METHOD("DisplayName", ...),
       &ClassName::MethodName
   );

Notes:
   - DisplayNameStr overrides the default method name for the editor.
   - __VA_ARGS__ allows passing any number of argument names.
   - '##__VA_ARGS__' ensures no trailing comma when no arguments are given.
   - Safe on MSVC; fully compatible with Godot ClassDB.
============================================================================*/
#define GFUNCTION_DISPLAY(ClassName, MethodName, DisplayNameStr, ...)                                                                                                                                  \
    ClassDB::bind_method(D_METHOD(DisplayNameStr, ##__VA_ARGS__), &ClassName::MethodName) // Binds a class method with a custom display name, supports zero or more parameters

#pragma region GD_Log

// ============================================================
// Godot Logging System - UE_LOG style with color support
// ============================================================

// ------------------------------------------------------------
// Choose text color based on verbosity
// ------------------------------------------------------------
inline String GetColorByVerbosity(const String& verbosity)
{
    if (verbosity == "Error")
        return "red";
    if (verbosity == "Warning" || verbosity == "Warn")
        return "yellow";
    if (verbosity == "Info")
        return "lightblue";
    if (verbosity == "Debug")
        return "gray";
    return "white";
}

// ------------------------------------------------------------
// Core log function (no String::vformat, fully C++ safe)
// ------------------------------------------------------------
// Uses UtilityFunctions::str() for concatenation.
// Example:
// GodotLogInternal("Render", "INFO", "Mesh ", mesh_name, " loaded");
inline void GodotLogInternal(const String& category, const String& verbosity, const String& format, const Array& args = Array())
{
    const String color_tag = GetColorByVerbosity(verbosity);

    // Build the message manually (simple concatenation)
    String formatted_msg = format;
    for (int i = 0; i < args.size(); i++)
    {
        formatted_msg += " " + args[i].stringify();
    }

    // 💡 Now the *entire* log line is colorized
    const String final_text = "[color=" + color_tag + "]" + "[" + verbosity + "] [" + category + "] " + formatted_msg + "[/color]";

    UtilityFunctions::print_rich(final_text);
}

// ------------------------------------------------------------
// GD_LOG Macro (UE_LOG style)
// ------------------------------------------------------------
// Usage:
// GD_LOG(Render, INFO, "Mesh loaded:", mesh_name, vertex_count);
#define GD_LOG(Category, Verbosity, Format, ...)                                                                                                                                                       \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        Array __gdlog_args;                                                                                                                                                                            \
        __GDLOG_VA_ARGS_TO_ARRAY(__gdlog_args, ##__VA_ARGS__);                                                                                                                                         \
        GodotLogInternal(#Category, #Verbosity, Format, __gdlog_args);                                                                                                                                 \
    } while (0)

// ------------------------------------------------------------
// 🪄 Helper Macros - Convert variadic args into Godot Array
// ------------------------------------------------------------
#define __GDLOG_EXPAND(x) x
#define __GDLOG_NARG(...) __GDLOG_EXPAND(__GDLOG_NARG_(__VA_ARGS__, 5, 4, 3, 2, 1, 0))
#define __GDLOG_NARG_(_1, _2, _3, _4, _5, N, ...) N

#define __GDLOG_VA_ARGS_TO_ARRAY(arr, ...)                                                                                                                                                             \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        constexpr int __n = __GDLOG_NARG(__VA_ARGS__);                                                                                                                                                 \
        if constexpr (__n > 0)                                                                                                                                                                         \
        {                                                                                                                                                                                              \
            Variant __args[] = {__VA_ARGS__};                                                                                                                                                          \
            for (int i = 0; i < __n; ++i)                                                                                                                                                              \
                arr.append(__args[i]);                                                                                                                                                                 \
        }                                                                                                                                                                                              \
    } while (0)
#pragma endregion GD_Log

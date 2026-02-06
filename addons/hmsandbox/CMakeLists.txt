cmake_minimum_required(VERSION 3.17)
project(HMSandbox)

# ============================================================
# 通用编译配置
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ============================================================
# 多配置支持 (Visual Studio)
# ============================================================
if(MSVC)
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "" FORCE)
    endif()

    list(APPEND _configs "DebugGame" "DebugGameEditor")
    foreach(cfg IN LISTS _configs)
        list(FIND CMAKE_CONFIGURATION_TYPES ${cfg} _found)
        if(_found EQUAL -1)
            set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};${cfg}" CACHE STRING "" FORCE)
        endif()
    endforeach()

    # Initialize required CMake flag variables for custom configurations (based on Debug)
    foreach(cfg IN ITEMS DEBUGGAME DEBUGGAMEEDITOR)
        set(CMAKE_C_FLAGS_${cfg} "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_${cfg} "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "" FORCE)
        set(CMAKE_EXE_LINKER_FLAGS_${cfg} "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS_${cfg} "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
        set(CMAKE_STATIC_LINKER_FLAGS_${cfg} "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
        set(CMAKE_MODULE_LINKER_FLAGS_${cfg} "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
    endforeach()
endif()

# ============================================================
# godot-cpp 路径配置
# ============================================================
set(GODOT_CPP_PATH "${CMAKE_SOURCE_DIR}/godot-cpp")
add_subdirectory(${GODOT_CPP_PATH} build)

# ============================================================
# quickjs 路径配置
# ============================================================
set(JS_PATH "${CMAKE_SOURCE_DIR}/quickjs")
add_subdirectory(${JS_PATH} quickjs_build)
# Static library qjs will be linked into a shared library, so it needs -fPIC on Linux
set_target_properties(qjs PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ============================================================
# generated 路径配置
# ============================================================
set(GEN_PATH "${CMAKE_SOURCE_DIR}/generated")

# ============================================================
# 收集生成的源文件
# ============================================================
file(GLOB GEN_CPP_FILES CONFIGURE_DEPENDS "${GEN_PATH}/*.gen.cpp")

# ============================================================
# 包含目录
# ============================================================
set(PROJECT_SOURCE_ROOT "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src")
    set(PROJECT_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
else()
    message(FATAL_ERROR "No source directory found. Expected '${CMAKE_CURRENT_SOURCE_DIR}/src'")
endif()

include_directories(
    ${GODOT_CPP_PATH}/include
    ${GODOT_CPP_PATH}/include/godot_cpp
    ${GODOT_CPP_PATH}/godot_headers

    ${CMAKE_CURRENT_SOURCE_DIR}/src

    ${JS_PATH}
    ${GEN_PATH}

    ${PROJECT_SOURCE_ROOT}
)

# Add any <Module>/Public folders as include dirs (e.g. src/SandboxCore/Public)
file(GLOB_RECURSE _public_dirs LIST_DIRECTORIES true "${PROJECT_SOURCE_ROOT}/*/Public")
foreach(_dir IN LISTS _public_dirs)
    if(IS_DIRECTORY "${_dir}")
        list(APPEND INCLUDE_DIRS "${_dir}")
    endif()
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

# ============================================================
# 收集源文件
# ============================================================
file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_ROOT}/*.cpp")
file(GLOB_RECURSE H_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_ROOT}/*.h" "${PROJECT_SOURCE_ROOT}/*.hpp")
set(SOURCES ${CPP_FILES} ${H_FILES} ${GEN_CPP_FILES})

# ============================================================
# VS 过滤器
# ============================================================
function(assign_source_group_by_dir base_dir)
    foreach(_source IN ITEMS ${ARGN})
        get_filename_component(_source_path "${_source}" PATH)
        file(RELATIVE_PATH _group_path "${base_dir}" "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        if(_group_path STREQUAL "")
            set(_group_path "\\")
        endif()
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endfunction()
assign_source_group_by_dir(${CMAKE_CURRENT_SOURCE_DIR}/src ${SOURCES})

# ============================================================
# 构建 GDExtension DLL
# ============================================================
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} godot-cpp qjs)

# ============================================================
# Windows 特定宏和增强调试设置
# ============================================================
if(MSVC)
    # 通用宏
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)

    # 增强调试配置 - 确保所有配置都能调试
    target_compile_options(${PROJECT_NAME} PRIVATE
        # 所有配置都包含调试信息
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:DebugGame>:/Zi>
        $<$<CONFIG:DebugGameEditor>:/Zi>
        $<$<CONFIG:RelWithDebInfo>:/Zi>
        
        # 禁用优化以确保可调试性
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:DebugGame>:/Od>
        $<$<CONFIG:DebugGameEditor>:/Od>
        
        # 禁用函数内联
        $<$<CONFIG:Debug>:/Ob0>
        $<$<CONFIG:DebugGame>:/Ob0>
        $<$<CONFIG:DebugGameEditor>:/Ob0>
        
        # 启用所有调试信息
        $<$<CONFIG:Debug>:/DEBUG>
        $<$<CONFIG:DebugGame>:/DEBUG>
        $<$<CONFIG:DebugGameEditor>:/DEBUG>
    )

    # 链接器调试选项
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
        $<$<CONFIG:DebugGame>:/DEBUG>
        $<$<CONFIG:DebugGameEditor>:/DEBUG>
        $<$<CONFIG:RelWithDebInfo>:/DEBUG>
        
        # 增量链接以加快调试构建
        $<$<CONFIG:Debug>:/INCREMENTAL>
        $<$<CONFIG:DebugGame>:/INCREMENTAL>
        $<$<CONFIG:DebugGameEditor>:/INCREMENTAL>
    )

    # 确保所有调试配置使用相同的 PDB 设置
    set_target_properties(${PROJECT_NAME} PROPERTIES
        # 所有 Debug 配置使用相同的 PDB 名称和位置
        COMPILE_PDB_NAME "HMSandbox"
        COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug"
        PDB_NAME "HMSandbox"
        PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Debug"
    )

    # 为每个配置单独设置（确保覆盖）
    foreach(cfg IN ITEMS Debug DebugGame DebugGameEditor)
        string(TOUPPER ${cfg} UPPER_CFG)
        
        set_target_properties(${PROJECT_NAME} PROPERTIES
            COMPILE_PDB_NAME_${UPPER_CFG} "HMSandbox"
            COMPILE_PDB_OUTPUT_DIRECTORY_${UPPER_CFG} "${CMAKE_SOURCE_DIR}/bin/Debug"
            PDB_NAME_${UPPER_CFG} "HMSandbox"
            PDB_OUTPUT_DIRECTORY_${UPPER_CFG} "${CMAKE_SOURCE_DIR}/bin/Debug"
        )
    endforeach()

    # 关键：为自定义配置设置与 Debug 相同的编译定义
    foreach(cfg IN ITEMS DebugGame DebugGameEditor)
        string(TOUPPER ${cfg} UPPER_CFG)
        
        # 使用与 Debug 配置相同的定义
        set_target_properties(${PROJECT_NAME} PROPERTIES
            COMPILE_DEFINITIONS_${UPPER_CFG} "BUILD_${cfg};_DEBUG;DEBUG"
        )
    endforeach()
endif()

# ============================================================
# 输出目录统一
# ============================================================
set(TARGET_BIN_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug")
set(TARGET_BIN_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release")

set_target_properties(${PROJECT_NAME} PROPERTIES
    # Debug 系列
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${TARGET_BIN_DEBUG}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${TARGET_BIN_DEBUG}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${TARGET_BIN_DEBUG}"

    RUNTIME_OUTPUT_DIRECTORY_DEBUGGAME "${TARGET_BIN_DEBUG}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUGGAME "${TARGET_BIN_DEBUG}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUGGAME "${TARGET_BIN_DEBUG}"

    RUNTIME_OUTPUT_DIRECTORY_DEBUGGAMEEDITOR "${TARGET_BIN_DEBUG}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUGGAMEEDITOR "${TARGET_BIN_DEBUG}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUGGAMEEDITOR "${TARGET_BIN_DEBUG}"

    # Release 系列
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${TARGET_BIN_RELEASE}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${TARGET_BIN_RELEASE}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${TARGET_BIN_RELEASE}"

    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${TARGET_BIN_RELEASE}"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${TARGET_BIN_RELEASE}"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${TARGET_BIN_RELEASE}"

    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${TARGET_BIN_RELEASE}"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${TARGET_BIN_RELEASE}"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${TARGET_BIN_RELEASE}"
)

# 默认输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TARGET_BIN_DEBUG}"
    LIBRARY_OUTPUT_DIRECTORY "${TARGET_BIN_DEBUG}"
    ARCHIVE_OUTPUT_DIRECTORY "${TARGET_BIN_DEBUG}"
)

# ============================================================
# 构建信息提示
# ============================================================
message(STATUS "============================================")
message(STATUS " Project Name      : ${PROJECT_NAME}")
message(STATUS " Godot-cpp Path    : ${GODOT_CPP_PATH}")
message(STATUS " Output Directory  : ${CMAKE_SOURCE_DIR}/bin")
message(STATUS "============================================")

# ============================================================
# Auto-generate VS launch configuration
# ============================================================
if(MSVC)
    option(GODOT_EDITOR_PATH "Path to Godot executable" "")
    option(GODOT_PROJECT_PATH "Path to Godot project" "")

    set(VS_LAUNCH_DIR "${CMAKE_SOURCE_DIR}/Build/.vs/${PROJECT_NAME}")
    set(VS_LAUNCH_FILE "${VS_LAUNCH_DIR}/launch.vs.json")
    set(TEMPLATE_LAUNCH_FILE "${CMAKE_SOURCE_DIR}/launch.json")

    file(MAKE_DIRECTORY "${VS_LAUNCH_DIR}")

    if(EXISTS "${TEMPLATE_LAUNCH_FILE}")
        file(READ "${TEMPLATE_LAUNCH_FILE}" LAUNCH_TEMPLATE_CONTENT)

        if(GODOT_EDITOR_PATH)
            string(REPLACE "<GODOT_EDITOR_PATH>" "${GODOT_EDITOR_PATH}" LAUNCH_TEMPLATE_CONTENT "${LAUNCH_TEMPLATE_CONTENT}")
        endif()
        if(GODOT_PROJECT_PATH)
            string(REPLACE "<GODOT_PROJECT_PATH>" "${GODOT_PROJECT_PATH}" LAUNCH_TEMPLATE_CONTENT "${LAUNCH_TEMPLATE_CONTENT}")
        endif()

        # 写主 launch.vs.json
        file(WRITE "${VS_LAUNCH_FILE}" "${LAUNCH_TEMPLATE_CONTENT}")

        message(STATUS "✅ launch.vs.json generated/updated")
    else()
        message(WARNING "⚠ launch.json template not found at: ${TEMPLATE_LAUNCH_FILE}")
    endif()
endif()
#!/usr/bin/env python
import os
import sys

libname = "godot_js_runtime"
projectdir = "demo"

# Try to detect godot-cpp location
godot_cpp_path = "godot-cpp"
if not os.path.exists(godot_cpp_path):
    print("Error: godot-cpp not found. Please run:")
    print("  git clone https://github.com/godotengine/godot-cpp.git")
    print("  cd godot-cpp && git checkout 4.2 && git submodule update --init")
    sys.exit(1)

env = SConscript(godot_cpp_path + "/SConstruct")

# Add include paths
env.Append(CPPPATH=[
    "src/",
    "quickjs/",
    "generated/",
])

# QuickJS-ng source files (C code) - v0.11.0
quickjs_sources = [
    "quickjs/cutils.c",
    "quickjs/dtoa.c",
    "quickjs/libregexp.c",
    "quickjs/libunicode.c",
    "quickjs/quickjs.c",
]

# Module C++ source files
cpp_sources = Glob("src/*.cpp")

# Generated binding files
generated_sources = Glob("generated/*.gen.cpp")

# Build QuickJS as C
env_quickjs = env.Clone()
if env["platform"] == "windows":
    # MSVC flags for QuickJS
    if "/fp:strict" in env_quickjs["CCFLAGS"]:
        env_quickjs["CCFLAGS"].remove("/fp:strict")
    env_quickjs.Append(CCFLAGS=["/fp:precise"])
else:
    # GCC/Clang flags
    env_quickjs.Append(CFLAGS=["-std=c11", "-Wno-sign-compare", "-Wno-unused-parameter", "-Wno-implicit-fallthrough"])

quickjs_objects = [env_quickjs.SharedObject(src) for src in quickjs_sources]

# Build C++ sources
cpp_objects = [env.SharedObject(src) for src in cpp_sources]

# Build generated binding sources
generated_objects = [env.SharedObject(src) for src in generated_sources]

# Combine all objects
all_objects = quickjs_objects + cpp_objects + generated_objects

# Build library name following godot-cpp-template pattern
suffix = env['suffix'].replace(".dev", "").replace(".universal", "")
lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

# Build to bin/{platform}/
library = env.SharedLibrary(
    "bin/{}/{}".format(env['platform'], lib_filename),
    source=all_objects,
)

# Copy to demo/bin/{platform}/
copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

default_args = [library, copy]
Default(*default_args)

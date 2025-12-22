generate:
  uv run ./scripts/generate_bindings.py

build:
  scons platform=macos

build_macos:
  scons platform=macos target=template_debug arch=universal
  scons platform=macos target=template_release arch=universal

build_win:
  scons platform=windows target=template_debug arch=x86_64
  scons platform=windows target=template_release arch=x86_64

build_android:
  scons platform=android target=template_debug arch=x86_32
  scons platform=android target=template_release arch=x86_32
  scons platform=android target=template_debug arch=x86_64
  scons platform=android target=template_release arch=x86_64
  scons platform=android target=template_release arch=arm32
  scons platform=android target=template_debug arch=arm32
  scons platform=android target=template_release arch=arm32
  scons platform=android target=template_debug arch=arm64
  scons platform=android target=template_release arch=arm64

build_ios:
  scons platform=ios target=template_debug
  scons platform=ios target=template_release

test:
  /Applications/Godot.app/Contents/MacOS/Godot --headless --path tests

lint:
  #!/bin/bash
  set -e
  echo "=== Running cppcheck ==="
  cppcheck --enable=all --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=unmatchedSuppression --suppressions-list=src/cppcheck_suppressions.txt --template=gcc --error-exitcode=1 -I src/ src/
  echo ""
  echo "=== Analyzing unused functions ==="
  ./scripts/analyze_unused.sh

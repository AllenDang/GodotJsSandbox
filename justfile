generate:
  uv run ./scripts/generate_bindings.py

build:
  scons platform=macos

build_android:
  scons platform=android target=template_debug
  scons platform=android target=template_release

build_ios:
  scons platform=ios target=template_debug
  scons platform=ios target=template_release

test:
  /Applications/Godot.app/Contents/MacOS/Godot --headless --path tests

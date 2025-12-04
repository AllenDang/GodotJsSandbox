generate:
  uv run ./scripts/generate_bindings.py

build:
  scons platform=macos

test:
  /Applications/Godot.app/Contents/MacOS/Godot --headless --path tests

#!/bin/bash
# Lint script for GodotJSRuntime - Zero warnings policy (like Rust's clippy)
# Run: ./scripts/lint.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "=== Running cppcheck (zero warnings policy) ==="
echo ""

# Run cppcheck with strict settings, only on our source files
# Excludes: godot-cpp, quickjs (third-party)
# Language: C++ (not C, avoids false positives about namespaces)
# Suppressions: documented in src/cppcheck_suppressions.txt

cppcheck \
    --language=c++ \
    --std=c++17 \
    --enable=warning,style,performance,portability \
    --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    --suppress=unmatchedSuppression \
    --suppress=*:quickjs/* \
    --suppress=*:godot-cpp/* \
    --suppressions-list=src/cppcheck_suppressions.txt \
    --inline-suppr \
    --error-exitcode=1 \
    --template='{file}:{line}: [{severity}] {id}: {message}' \
    --quiet \
    src/*.cpp src/*.h \
    2>&1

RESULT=$?

if [ $RESULT -eq 0 ]; then
    echo ""
    echo "✓ All checks passed! No warnings."
else
    echo ""
    echo "✗ Warnings found. Please fix before committing."
    exit 1
fi

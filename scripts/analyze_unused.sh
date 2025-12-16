#!/bin/bash
# Analyze unused functions reported by cppcheck
# Categorizes them as: DEAD CODE, RUNTIME (Godot/QuickJS), or REVIEW

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_DIR/src"
GEN_DIR="$PROJECT_DIR/generated"

# Colors for output
RED='\033[0;31m'
YELLOW='\033[0;33m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Counters
dead_count=0
runtime_count=0
review_count=0

echo "Analyzing unused functions..."
echo ""

# Run cppcheck and save to temp file
CPPCHECK_OUTPUT=$(mktemp)
cppcheck --enable=all \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --suppressions-list="$SRC_DIR/cppcheck_suppressions.txt" \
    --template='{file}:{line}:{message}' \
    -I "$SRC_DIR" \
    "$SRC_DIR" 2>&1 | grep "is never used" > "$CPPCHECK_OUTPUT"

while read -r line; do
    # Extract function name and file
    func=$(echo "$line" | sed "s/.*The function '\([^']*\)'.*/\1/")
    file=$(echo "$line" | cut -d: -f1)
    lineno=$(echo "$line" | cut -d: -f2)

    # Get just filename for display
    filename=$(basename "$file")

    # Check if registered with Godot's ClassDB::bind_method
    if grep -q "bind_method.*D_METHOD.*\"$func\"" "$SRC_DIR"/*.cpp 2>/dev/null; then
        echo -e "${GREEN}RUNTIME${NC}  $func ($filename:$lineno) - registered with ClassDB::bind_method"
        ((runtime_count++))
        continue
    fi

    # Check if registered with JS_NewCFunction
    if grep -q "JS_NewCFunction.*$func" "$SRC_DIR"/*.cpp 2>/dev/null; then
        echo -e "${GREEN}RUNTIME${NC}  $func ($filename:$lineno) - registered with JS_NewCFunction"
        ((runtime_count++))
        continue
    fi

    # Check if it's _bind_methods (always called by Godot)
    if [ "$func" = "_bind_methods" ]; then
        echo -e "${GREEN}RUNTIME${NC}  $func ($filename:$lineno) - called by Godot ClassDB"
        ((runtime_count++))
        continue
    fi

    # Count references in codebase (excluding the definition file for that line)
    # We look for the function name as a word boundary
    # Also check generated/ directory for generated code
    ref_count=$(grep -rw "$func" "$SRC_DIR"/*.cpp "$SRC_DIR"/*.h "$GEN_DIR"/*.cpp 2>/dev/null | grep -v "^${file}:${lineno}:" | wc -l | tr -d ' ')

    # If only 1 reference, it's likely just the header declaration
    if [ "$ref_count" -le 1 ]; then
        echo -e "${RED}DEAD${NC}     $func ($filename:$lineno) - no references found"
        ((dead_count++))
    else
        # Check if all references are just declarations (in .h files) or generated code
        cpp_refs=$(grep -rw "$func" "$SRC_DIR"/*.cpp "$GEN_DIR"/*.cpp 2>/dev/null | grep -v "^${file}:" | wc -l | tr -d ' ')
        if [ "$cpp_refs" -eq 0 ]; then
            echo -e "${RED}DEAD${NC}     $func ($filename:$lineno) - only header declarations"
            ((dead_count++))
        else
            echo -e "${YELLOW}REVIEW${NC}   $func ($filename:$lineno) - $ref_count refs, needs manual check"
            ((review_count++))
        fi
    fi
done < "$CPPCHECK_OUTPUT"

rm -f "$CPPCHECK_OUTPUT"

echo ""
echo -e "${CYAN}Summary:${NC}"
echo -e "  ${RED}DEAD${NC}    : $dead_count (safe to remove)"
echo -e "  ${GREEN}RUNTIME${NC} : $runtime_count (called by Godot/QuickJS - add to suppressions)"
echo -e "  ${YELLOW}REVIEW${NC}  : $review_count (needs manual check)"
echo ""

if [ "$dead_count" -gt 0 ]; then
    echo -e "${RED}Found $dead_count unused functions that can be removed.${NC}"
    exit 1
fi

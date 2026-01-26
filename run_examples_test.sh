#!/bin/bash

# LavaCake Examples Test Script
# Builds and runs all examples, checking for compilation and validation errors

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
TIMEOUT_SECONDS=3

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Track results
PASSED=0
FAILED=0
FAILED_EXAMPLES=()

echo "========================================"
echo "LavaCake Examples Test Suite"
echo "========================================"
echo ""

# Step 1: Build all examples
echo -e "${YELLOW}[BUILD]${NC} Building all examples..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if ! cmake .. -DCMAKE_BUILD_TYPE=Debug 2>&1; then
    echo -e "${RED}[ERROR]${NC} CMake configuration failed!"
    exit 1
fi

if ! cmake --build . --parallel 2>&1; then
    echo -e "${RED}[ERROR]${NC} Build failed!"
    exit 1
fi

echo -e "${GREEN}[BUILD]${NC} Build successful!"
echo ""

# Step 2: Find and run all examples
echo "========================================"
echo "Running Examples (${TIMEOUT_SECONDS}s timeout each)"
echo "========================================"
echo ""

# Enable Vulkan validation layers
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation

# Cross-platform timeout function
run_with_timeout() {
    local timeout_sec="$1"
    shift
    local cmd="$@"

    # Start the process in background
    "$@" &
    local pid=$!

    # Wait for timeout or process to finish
    local count=0
    while kill -0 $pid 2>/dev/null; do
        sleep 0.1
        count=$((count + 1))
        if [ $count -ge $((timeout_sec * 10)) ]; then
            kill -TERM $pid 2>/dev/null
            sleep 0.2
            kill -KILL $pid 2>/dev/null
            return 124  # timeout exit code
        fi
    done

    wait $pid
    return $?
}

run_example() {
    local exe_path="$1"
    local exe_name=$(basename "$exe_path")
    local output_file=$(mktemp)
    local exit_code=0

    echo -n "Testing: $exe_name ... "

    # Run the example with timeout, capture all output
    run_with_timeout "$TIMEOUT_SECONDS" "$exe_path" > "$output_file" 2>&1 || exit_code=$?

    # Check for validation errors in output
    local has_validation_error=0
    if grep -qi "validation error\|VUID-\|ERROR: \[Validation\]" "$output_file" 2>/dev/null; then
        has_validation_error=1
    fi

    # Check for crash/segfault (exit code 139 = segfault, 134 = abort)
    local has_crash=0
    if [[ $exit_code -eq 139 ]] || [[ $exit_code -eq 134 ]] || [[ $exit_code -eq 136 ]]; then
        has_crash=1
    fi

    # Timeout (exit code 124) is expected and OK
    # Also accept exit code 143 (SIGTERM) and 137 (SIGKILL) as timeout
    if [[ $exit_code -eq 124 ]] || [[ $exit_code -eq 143 ]] || [[ $exit_code -eq 137 ]]; then
        if [[ $has_validation_error -eq 0 ]]; then
            echo -e "${GREEN}PASS${NC} (timeout - expected)"
            ((PASSED++))
        else
            echo -e "${RED}FAIL${NC} (validation errors)"
            echo "  Validation errors found:"
            grep -i "validation error\|VUID-\|ERROR: \[Validation\]" "$output_file" | head -5 | sed 's/^/    /'
            ((FAILED++))
            FAILED_EXAMPLES+=("$exe_name (validation)")
        fi
    elif [[ $exit_code -eq 0 ]] && [[ $has_validation_error -eq 0 ]]; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    elif [[ $has_validation_error -eq 1 ]]; then
        echo -e "${RED}FAIL${NC} (validation errors)"
        echo "  Validation errors found:"
        grep -i "validation error\|VUID-\|ERROR: \[Validation\]" "$output_file" | head -5 | sed 's/^/    /'
        ((FAILED++))
        FAILED_EXAMPLES+=("$exe_name (validation)")
    elif [[ $has_crash -eq 1 ]]; then
        echo -e "${RED}FAIL${NC} (crashed, exit code: $exit_code)"
        ((FAILED++))
        FAILED_EXAMPLES+=("$exe_name (crash)")
    else
        echo -e "${RED}FAIL${NC} (exit code: $exit_code)"
        tail -5 "$output_file" | sed 's/^/    /'
        ((FAILED++))
        FAILED_EXAMPLES+=("$exe_name (error)")
    fi

    rm -f "$output_file"
}

# Find all example executables
EXAMPLES_DIR="$BUILD_DIR/examples"

if [[ -d "$EXAMPLES_DIR" ]]; then
    # Find executables in basic/, advanced/, and compute/ subdirectories
    for subdir in basic advanced compute imgui; do
        if [[ -d "$EXAMPLES_DIR/$subdir" ]]; then
            for exe in "$EXAMPLES_DIR/$subdir"/*; do
                # Check if it's an executable file (not a directory, not a .cmake/.txt file)
                if [[ -x "$exe" ]] && [[ -f "$exe" ]] && [[ ! "$exe" =~ \.(cmake|txt|o|a)$ ]] && [[ ! -d "$exe" ]]; then
                    # Skip if it's a Makefile or cmake file
                    exe_basename=$(basename "$exe")
                    if [[ "$exe_basename" != "Makefile" ]] && [[ "$exe_basename" != "CMakeFiles" ]]; then
                        run_example "$exe"
                    fi
                fi
            done
        fi
    done
else
    echo -e "${RED}[ERROR]${NC} Examples directory not found: $EXAMPLES_DIR"
    exit 1
fi

# Summary
echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

if [[ ${#FAILED_EXAMPLES[@]} -gt 0 ]]; then
    echo ""
    echo "Failed examples:"
    for example in "${FAILED_EXAMPLES[@]}"; do
        echo -e "  ${RED}-${NC} $example"
    done
    echo ""
    exit 1
else
    echo ""
    echo -e "${GREEN}All examples passed!${NC}"
    exit 0
fi

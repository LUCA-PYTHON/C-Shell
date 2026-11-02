
#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

SHELL_PROGRAM="$PROJECT_ROOT/c-shell"

passed=0
failed=0

assert_contains()
{
    local output="$1"
    local expected="$2"
    local description="$3"

    if grep -Fq -- "$expected" <<< "$output"; then
        ((passed += 1))
    else
        echo "[FAIL] $description"
        echo "Expected: $expected"
        ((failed += 1))
    fi
}

if [[ ! -x "$SHELL_PROGRAM" ]]; then
    echo "[FAIL] C-Shell executable not found"
    exit 1
fi

echo "C-Shell Integration Tests"
echo ""

# Test 1: Echo
output=$(printf 'echo Hello World\nexit\n' | "$SHELL_PROGRAM")
assert_contains "$output" "Hello World" "Echo prints text"

# Test 2: PWD
output=$(printf 'pwd\nexit\n' | "$SHELL_PROGRAM")
assert_contains "$output" "$PROJECT_ROOT" "PWD prints current directory"

# Test 3: CD
output=$(printf 'cd /tmp\npwd\nexit\n' | "$SHELL_PROGRAM")
assert_contains "$output" "/tmp" "CD changes directory"

# Test 4: Help
output=$(printf 'help\nexit\n' | "$SHELL_PROGRAM")
assert_contains "$output" "C-Shell" "Help displays command overview"

# Test 5: Exit
set +e
printf 'exit 5\n' | "$SHELL_PROGRAM"
status=$?
set -e

if [[ "$status" -eq 5 ]]; then
    ((passed += 1))
else
    echo "[FAIL] Exit returns status 5"
    ((failed += 1))
fi

echo ""
echo "Integration tests: $passed passed, $failed failed"

if (( failed > 0 )); then
    exit 1
fi
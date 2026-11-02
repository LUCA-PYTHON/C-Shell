set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

SHELL_PROGRAM="$PROJECT_ROOT/c-shell"

if [[ ! -x "$SHELL_PROGRAM" ]]; then
    echo "[FAIL] C-Shell executable not found"
    exit 1
fi

output=$(printf 'echo Hello\nexit\n' | "$SHELL_PROGRAM")

if [[ "$output" == *"Hello"* ]]; then
    echo "[PASS] Shell executes echo"
else
    echo "[FAIL] Shell executes echo"
    exit 1
fi

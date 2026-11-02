set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

echo "C-Shell Tests"

echo ""
echo "[1/2] Unit Tests"
./build/test_parser

echo ""
echo "[2/2] Integration Tests"
bash tests/integration/test_shell.sh

echo ""
echo "All tests completed"
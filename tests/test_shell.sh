#!/bin/bash
set -eu

SHELL_PROGRAM="./c-shell"

if [ ! -x "$SHELL_PROGRAM" ]; then
    echo "Error: c-shell not found. Run make first."
    exit 1
fi

output=$(printf 'echo Hello\nexit\n' | "$SHELL_PROGRAM")

if [[ "$output" == *"Hello"* ]]; then
    echo "Test passed: echo Hello"
else
    echo "Test failed: echo Hello"
    exit 1
fi
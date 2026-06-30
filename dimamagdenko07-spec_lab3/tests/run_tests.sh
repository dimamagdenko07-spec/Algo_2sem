#!/usr/bin/env bash
set -euo pipefail

BINARY="${1:-./path_processor}"
TESTS_DIR="$(dirname "$0")"
PASS=0
FAIL=0

for test_dir in "$TESTS_DIR"/*/; do
    name=$(basename "$test_dir")
    expected_hash_file="$test_dir/expected.sha256"

    if [ ! -f "$expected_hash_file" ]; then
        echo "SKIP $name (no expected.sha256)"
        continue
    fi

    expected_hash=$(cat "$expected_hash_file")
    out=$(mktemp)

    if ! "$BINARY" "$test_dir" "$test_dir/input.txt" "$out" > /dev/null 2>&1; then
        echo "FAIL $name (binary exited with error)"
        FAIL=$((FAIL + 1))
        rm "$out"
        continue
    fi

    actual_hash=$(shasum -a 256 "$out" | awk '{print $1}')
    rm "$out"

    if [ "$actual_hash" = "$expected_hash" ]; then
        echo "PASS $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL $name (hash mismatch)"
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]

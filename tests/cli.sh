#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${1:-${SCLIPPLE_TEST_BIN:-$ROOT/src/sclipple}}"
export SCLIPPLE_TEST_BIN="$BIN"

suites=(
  basic.sh
  tags.sh
  selectors.sh
  tag-match.sh
  config.sh
  mv-rm.sh
  storage.sh
  git.sh
)

for suite in "${suites[@]}"; do
  echo "=== CLI suite: $suite ==="
  if ! "$ROOT/tests/cli/$suite"; then
    echo "FAIL: CLI suite $suite" >&2
    exit 1
  fi
done

echo "All CLI test suites passed."

#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="git"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_48() {
reset_home
setup_rc

run_cmd "$BIN" add gitnote
assert_success

run_cmd "$BIN" git init
assert_success
assert_file_exists "$HOME/.sclipple/.git"

run_cmd "$BIN" git status --short
assert_success
assert_contains "$STDOUT$STDERR" ".list.csv"
}

test_49() {
reset_home
setup_rc

run_cmd "$BIN" git status
assert_status 2
assert_diagnostic
}

run_test "git subcommand runs inside storage" test_48
run_test "git before storage reports diagnostic and exits with status 2" test_49

finish_suite

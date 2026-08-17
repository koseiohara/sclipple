#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="selectors"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_10() {
fresh_home
  run_cmd "$BIN" add alpha
  assert_success
  run_cmd "$BIN" add tagged1 tagged2 -t red -t red
  assert_success
  write_note tagged1 $'tagged one red body\nneedle-red\n'
  write_note tagged2 $'tagged two red body\n'
run_cmd "$BIN" search needle-red -t red
assert_success
assert_contains "$STDOUT" "tagged1"
assert_contains "$STDOUT" "needle-red"
assert_not_contains "$STDOUT" "[alpha]"

run_cmd "$BIN" alpha -t red
assert_success
assert_contains "$STDOUT" "tagged one red body"
assert_contains "$STDOUT" "tagged two red body"
}

test_13() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma
  assert_success
write_note alpha $'\n\nfirst alpha line\nsecond alpha line\nurgent task\n'
write_note beta $'first beta line\nsecond beta line\n'
write_note gamma $'first gamma line\n'

run_cmd "$BIN" ls
assert_success
assert_contains "$STDOUT" "alpha"
assert_contains "$STDOUT" "first alpha line"
assert_contains "$STDOUT" "beta"
assert_contains "$STDOUT" "first beta line"
}

test_14() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma
  assert_success
run_cmd "$BIN" ls beta alpha
assert_success
assert_contains "$STDOUT" "beta"
assert_contains "$STDOUT" "alpha"
assert_not_contains "$STDOUT" "[gamma]"
}

test_15() {
fresh_home
  run_cmd "$BIN" add gamma
  assert_success
long_line="$(printf '%*s' 160 '' | tr ' ' x)"
write_note gamma "$long_line"$'\n'

run_cmd "$BIN" ls gamma
assert_success
assert_contains "$STDOUT" "..."
}

test_16() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma
  assert_success
  write_note alpha $'\n\nfirst alpha line\nsecond alpha line\nurgent task\n'
  write_note beta $'first beta line\nsecond beta line\n'
  write_note gamma $'first gamma line\n'
run_cmd "$BIN" show
assert_success
assert_contains "$STDOUT" "[alpha]"
assert_contains "$STDOUT" "second alpha line"
assert_contains "$STDOUT" "[beta]"
assert_contains "$STDOUT" "second beta line"

run_cmd "$BIN" show beta alpha
assert_success
assert_contains "$STDOUT" "[beta]"
assert_contains "$STDOUT" "[alpha]"
assert_not_contains "$STDOUT" "[gamma]"
}

test_17() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma
  assert_success
  write_note alpha $'\n\nfirst alpha line\nsecond alpha line\nurgent task\n'
  write_note beta $'first beta line\nsecond beta line\n'
  write_note gamma $'first gamma line\n'
run_cmd "$BIN" search urgent
assert_success
assert_contains "$STDOUT" "alpha"
assert_contains "$STDOUT" "urgent task"

run_cmd "$BIN" search 'URGENT|nothing'
assert_success
assert_contains "$STDOUT" "urgent task"

run_cmd "$BIN" search urgent beta
assert_success
assert_not_contains "$STDOUT" "urgent task"

run_cmd "$BIN" search '['
assert_error_stop
[ -z "$STDOUT" ] || fail "invalid regex unexpectedly wrote to stdout: $STDOUT"
[ -n "$STDERR" ] || fail "invalid regex did not produce a diagnostic on stderr"
}

test_18() {
fresh_home
  run_cmd "$BIN" add alpha
  assert_success
  write_note alpha $'\n\nfirst alpha line\nsecond alpha line\nurgent task\n'
run_cmd "$BIN" alpha
assert_success
assert_contains "$STDOUT" "first alpha line"
assert_contains "$STDOUT" "urgent task"
}

run_test "tag selectors are honored by search and edit" test_10
run_test "ls shows first non-empty lines" test_13
run_test "ls supports selected keys" test_14
run_test "ls abbreviates long first lines" test_15
run_test "show prints full notes" test_16
run_test "search supports case-insensitive extended regex" test_17
run_test "edit command invokes configured editor" test_18

finish_suite

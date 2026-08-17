#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="basic"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_1() {
fresh_home
run_cmd "$BIN"
assert_success
assert_contains "$STDOUT$STDERR" "sclipple"

run_cmd "$BIN" --help
assert_success
assert_contains "$STDOUT$STDERR" "sclipple"
long_help_status="$STATUS"
long_help_stdout="$STDOUT"
long_help_stderr="$STDERR"

run_cmd "$BIN" -h
assert_success

[ "$STATUS" -eq "$long_help_status" ] \
  || fail "-h and --help returned different statuses"

[ "$STDOUT" = "$long_help_stdout" ] \
  || fail "-h and --help produced different stdout"

[ "$STDERR" = "$long_help_stderr" ] \
  || fail "-h and --help produced different stderr"

before_count="$(note_count)"

assert_command_help "add" add foo
assert_note_count "$before_count"
assert_file_not_exists "$HOME/.sclipple"

assert_command_help "rm" rm foo
assert_command_help "mv" mv old new
assert_command_help "ls" ls foo
assert_command_help "search" search pattern foo
assert_command_help "show" show foo
assert_command_help "tag" tag foo
assert_command_help "untag" untag foo
}

test_2() {
fresh_home
run_cmd "$BIN" --version
assert_success
assert_contains "$STDOUT" "sclipple version "
[ -z "$STDERR" ] || fail "--version unexpectedly wrote to stderr: $STDERR"
assert_file_not_exists "$HOME/.sclipple"
}

test_3() {
fresh_home
run_cmd "$BIN" add alpha
assert_success

assert_file_exists "$HOME/.sclipple"
assert_file_exists "$HOME/.sclipple/notes"
assert_file_exists "$HOME/.sclipple/.list.csv"
assert_file_exists "$(note_path alpha)"
assert_contains "$(cat "$HOME/.sclipple/.list.csv")" "alpha,"
assert_note_count 1
}

test_4() {
fresh_home
  run_cmd "$BIN" add alpha
  assert_success
run_cmd "$BIN" add alpha
assert_negative_stop
assert_stderr_contains "alpha"
assert_note_count 1
}

test_5() {
fresh_home
  run_cmd "$BIN" add alpha
  assert_success
run_cmd "$BIN" add beta gamma
assert_success

assert_file_exists "$(note_path beta)"
assert_file_exists "$(note_path gamma)"
assert_note_count 3
}

test_11() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma tagged1 tagged2 greenonly duptag
  assert_success
run_cmd "$BIN" add A_1-b
assert_success
assert_file_exists "$(note_path A_1-b)"
assert_note_count 8
}

test_12() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma tagged1 tagged2 greenonly duptag A_1-b
  assert_success
before_count="$(note_count)"

for key in "." ".." "bad/key" "bad,key" "bad key" "git" "add" "tag" "rm" "mv" "ls" "search" "show"; do
  run_cmd "$BIN" add "$key"
  assert_diagnostic

  current_count="$(note_count)"
  [ "$current_count" -eq "$before_count" ] \
    || fail "invalid or reserved key created a note: key=$key before=$before_count after=$current_count"
done

after_count="$(note_count)"
[ "$before_count" -eq "$after_count" ] \
  || fail "invalid or reserved keys changed note count: before=$before_count after=$after_count"
}

run_test "help works without storage" test_1
run_test "version works without storage" test_2
run_test "add initializes storage" test_3
run_test "duplicate add is a non-fatal warning/no-op" test_4
run_test "add accepts multiple keys" test_5
run_test "valid key characters are accepted" test_11
run_test "invalid and reserved keys do not create notes" test_12

finish_suite

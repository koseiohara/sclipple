#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BIN="${SCLIPPLE_TEST_BIN:-$ROOT/src/sclipple}"

STDOUT=""
STDERR=""
STATUS=0
BUG_STOP_STATUS=3
NEGATIVE_STOP_STATUS=1
ERROR_STOP_STATUS=2
TEST_HOME=""
DEFAULT_EXT="txt"
CURRENT_SUITE="${CURRENT_SUITE:-unknown}"
CURRENT_TEST="${CURRENT_TEST:-unknown}"
SUITE_TEST_COUNT=0

fail() {
  echo "FAIL [$CURRENT_SUITE :: $CURRENT_TEST]: $*" >&2
  exit 1
}

dump_last_command_output() {
  echo "--- command output [$CURRENT_SUITE :: $CURRENT_TEST] ---" >&2
  echo "STDOUT:" >&2
  echo "$STDOUT" >&2
  echo "STDERR:" >&2
  echo "$STDERR" >&2
  echo "STATUS: $STATUS" >&2
}

run_cmd() {
  local out
  local err

  out="$(mktemp)"
  err="$(mktemp)"

  set +e
  "$@" >"$out" 2>"$err"
  STATUS=$?
  set -e

  STDOUT="$(cat "$out")"
  STDERR="$(cat "$err")"

  rm -f "$out" "$err"

  if [ "$STATUS" -eq "$BUG_STOP_STATUS" ]; then
    dump_last_command_output
    fail "BUG_STOP detected while running: $*"
  fi
}

assert_success() {
  if [ "$STATUS" -ne 0 ]; then
    dump_last_command_output
    fail "expected success, got status $STATUS"
  fi
}

assert_failure() {
  if [ "$STATUS" -eq 0 ]; then
    dump_last_command_output
    fail "expected failure, got status 0"
  fi
}

assert_status() {
  local expected="$1"
  if [ "$STATUS" -ne "$expected" ]; then
    dump_last_command_output
    fail "expected status $expected, got $STATUS"
  fi
}

assert_negative_stop() { assert_status "$NEGATIVE_STOP_STATUS"; }
assert_error_stop() { assert_status "$ERROR_STOP_STATUS"; }

assert_contains() {
  local text="$1"
  local expected="$2"
  if ! printf '%s' "$text" | grep -F -- "$expected" >/dev/null; then
    dump_last_command_output
    fail "expected text to contain: $expected"
  fi
}

assert_not_contains() {
  local text="$1"
  local unexpected="$2"
  if printf '%s' "$text" | grep -F -- "$unexpected" >/dev/null; then
    dump_last_command_output
    fail "expected text not to contain: $unexpected"
  fi
}

assert_diagnostic() {
  if [ -z "$STDOUT$STDERR" ]; then
    dump_last_command_output
    fail "expected diagnostic output"
  fi
}

assert_stderr_contains() {
  local expected="$1"
  if [ -n "$STDOUT" ]; then
    dump_last_command_output
    fail "expected empty stdout for diagnostic command"
  fi
  assert_contains "$STDERR" "$expected"
}

assert_file_exists() { [ -e "$1" ] || fail "expected file to exist: $1"; }
assert_file_not_exists() { [ ! -e "$1" ] || fail "expected file not to exist: $1"; }

note_count() {
  if [ -d "$HOME/.sclipple/notes" ]; then
    find "$HOME/.sclipple/notes" -type f | wc -l | tr -d ' '
  else
    printf '0'
  fi
}

assert_note_count() {
  local expected="$1"
  local actual
  actual="$(note_count)"
  [ "$actual" -eq "$expected" ] || fail "expected $expected note files, got $actual"
}

note_path() {
  local key="$1"
  local ext="${2:-$DEFAULT_EXT}"
  printf '%s/.sclipple/notes/%s.%s' "$HOME" "$key" "$ext"
}

find_note() {
  local key="$1"
  local path
  path="$(note_path "$key")"
  if [ -f "$path" ]; then printf '%s' "$path"; fi
}

write_note() {
  local key="$1"
  local content="$2"
  local path
  path="$(find_note "$key")"
  assert_file_exists "$path"
  printf '%s' "$content" > "$path"
}

new_home() {
  TEST_HOME="$(mktemp -d)"
  export HOME="$TEST_HOME"
  trap 'rm -rf "$TEST_HOME"' EXIT
}

reset_home() {
  rm -rf "$TEST_HOME"
  TEST_HOME="$(mktemp -d)"
  export HOME="$TEST_HOME"
}

setup_rc() {
  cat > "$HOME/.sclipplerc" <<'RC'
editor = cat
extension = txt
RC
}

fresh_home() {
  reset_home
  setup_rc
}

setup_tag_match_fixture() {
  local mode="$1"
  reset_home
  cat > "$HOME/.sclipplerc" <<RC
editor = cat
extension = txt
tag-match = $mode
RC
  run_cmd "$BIN" add tag-both -t red -t blue
  assert_success
  run_cmd "$BIN" add tag-red -t red
  assert_success
  run_cmd "$BIN" add tag-blue -t blue
  assert_success
  run_cmd "$BIN" add tag-none
  assert_success
  write_note tag-both $'both body\nneedle-match\n'
  write_note tag-red $'red body\nneedle-match\n'
  write_note tag-blue $'blue body\nneedle-match\n'
  write_note tag-none $'none body\nneedle-match\n'
}

setup_mv_fixture() {
  fresh_home
  run_cmd "$BIN" add old other
  assert_success
  write_note old $'content for old\n'
  write_note other $'content for other\n'
  old_path="$(find_note old)"
  other_path="$(find_note other)"
  assert_file_exists "$old_path"
  assert_file_exists "$other_path"
  run_cmd "$BIN" mv old new
  assert_success
  new_path="$(find_note new)"
  assert_file_not_exists "$old_path"
  assert_file_exists "$new_path"
}

assert_storage_intact_for_new_other() {
  assert_file_exists "$new_path"
  assert_file_exists "$other_path"
  run_cmd "$BIN" show new
  assert_success
  assert_contains "$STDOUT" "content for old"
  run_cmd "$BIN" show other
  assert_success
  assert_contains "$STDOUT" "content for other"
}

assert_command_help() {
  local heading="$1"
  shift
  run_cmd "$BIN" "$@" --help
  assert_success
  assert_contains "$STDOUT$STDERR" "$heading"
}

run_test() {
  local name="$1"
  local fn="$2"
  SUITE_TEST_COUNT=$((SUITE_TEST_COUNT + 1))
  CURRENT_TEST="$name"
  printf '==> [%s] %s
' "$CURRENT_SUITE" "$CURRENT_TEST"
  "$fn"
  printf 'PASS [%s] %s
' "$CURRENT_SUITE" "$CURRENT_TEST"
}

finish_suite() {
  printf 'PASS [%s] all %d tests
' "$CURRENT_SUITE" "$SUITE_TEST_COUNT"
}

#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="tags"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_6() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma
  assert_success
run_cmd "$BIN" add tagged1 tagged2 -t red -t red
assert_success
assert_note_count 5

write_note tagged1 $'tagged one red body\nneedle-red\n'
write_note tagged2 $'tagged two red body\n'

run_cmd "$BIN" ls -t red
assert_success
assert_contains "$STDOUT" "[tagged1]"
assert_contains "$STDOUT" "[tagged2]"
assert_not_contains "$STDOUT" "[alpha]"

run_cmd "$BIN" show alpha -t red
assert_success
assert_contains "$STDOUT" "[alpha]"
assert_contains "$STDOUT" "[tagged1]"
assert_contains "$STDOUT" "[tagged2]"

run_cmd "$BIN" show tagged1 -t red
assert_success
tagged1_count="$(printf '%s' "$STDOUT" | grep -F -c '[tagged1]' || true)"
[ "$tagged1_count" -eq 1 ] \
  || fail "KEY/TAG union duplicated tagged1: count=$tagged1_count"
}

test_7() {
fresh_home
  run_cmd "$BIN" add tagged1 tagged2 -t red -t red
  assert_success
run_cmd "$BIN" add greenonly -t greenonlytag
assert_success
write_note greenonly $'green-only body\n'

run_cmd "$BIN" show -t red -t greenonlytag
assert_success
assert_contains "$STDOUT" "[tagged1]"
assert_contains "$STDOUT" "[tagged2]"
assert_contains "$STDOUT" "[greenonly]"
assert_not_contains "$STDOUT" "[alpha]"
}

test_8() {
fresh_home
run_cmd "$BIN" add duptag -t dupcheck -t dupcheck
assert_success
write_note duptag $'neutral body\n'

run_cmd "$BIN" ls duptag
assert_success
dupcheck_count="$(printf '%s' "$STDOUT" | { grep -oF 'dupcheck' || true; } | wc -l | tr -d ' ')"
[ "$dupcheck_count" -eq 1 ] \
  || fail "duplicate tag persisted more than once: count=$dupcheck_count"
}

test_9() {
fresh_home
  run_cmd "$BIN" add alpha beta gamma
  assert_success
run_cmd "$BIN" tag alpha beta -t blue -t blue
assert_success
assert_contains "$STDOUT" "blue"
[ -z "$STDERR" ] || fail "tag unexpectedly wrote to stderr: $STDERR"

run_cmd "$BIN" ls -t blue
assert_success
assert_contains "$STDOUT" "[alpha]"
assert_contains "$STDOUT" "[beta]"
assert_not_contains "$STDOUT" "[gamma]"

run_cmd "$BIN" tag alpha missing -t green
assert_negative_stop
assert_contains "$STDOUT" "green"
assert_contains "$STDOUT" "alpha"
assert_contains "$STDERR" "missing"

run_cmd "$BIN" ls -t green
assert_success
assert_contains "$STDOUT" "[alpha]"
assert_not_contains "$STDOUT" "[beta]"

run_cmd "$BIN" untag alpha -t blue
assert_success

run_cmd "$BIN" ls -t blue
assert_success
assert_not_contains "$STDOUT" "[alpha]"
assert_contains "$STDOUT" "[beta]"

run_cmd "$BIN" show alpha beta
assert_success
assert_contains "$STDOUT" "[alpha]"
assert_contains "$STDOUT" "[beta]"

run_cmd "$BIN" untag alpha -t does-not-exist
assert_success

run_cmd "$BIN" show alpha
assert_success
assert_contains "$STDOUT" "[alpha]"
}

run_test "add accepts initial tags and tag selectors use OR semantics" test_6
run_test "multiple distinct tag selectors use OR semantics" test_7
run_test "duplicate tags are persisted only once" test_8
run_test "tag and untag mutate metadata and partial missing keys are reported" test_9

finish_suite

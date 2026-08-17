#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="mv-rm"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_37() {
reset_home
setup_rc

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

assert_file_not_exists "$old_path"

new_path="$(find_note new)"
assert_file_exists "$new_path"
assert_file_exists "$other_path"
assert_contains "$(cat "$new_path")" "content for old"

run_cmd "$BIN" ls
assert_success
assert_contains "$STDOUT" "new"
assert_contains "$STDOUT" "other"
assert_not_contains "$STDOUT" "[old]"
}

test_38() {
setup_mv_fixture
run_cmd "$BIN" mv missing dst
assert_status 1
assert_diagnostic
assert_storage_intact_for_new_other
}

test_39() {
setup_mv_fixture
run_cmd "$BIN" mv new other
assert_status 1
assert_diagnostic
assert_storage_intact_for_new_other
}

test_40() {
setup_mv_fixture
run_cmd "$BIN" mv new ..
assert_status 2
assert_diagnostic

assert_file_exists "$new_path"

run_cmd "$BIN" show new
assert_success
assert_contains "$STDOUT" "content for old"
}

test_41() {
setup_mv_fixture
run_cmd "$BIN" rm new
assert_success

assert_file_not_exists "$new_path"

run_cmd "$BIN" show new
assert_failure
assert_diagnostic
}

test_42() {
fresh_home
  run_cmd "$BIN" add other
  assert_success
run_cmd "$BIN" rm missing
assert_negative_stop
assert_stderr_contains "missing"
}

test_43() {
fresh_home
run_cmd "$BIN" add trash1 trash2 -t disposable
assert_success
run_cmd "$BIN" add keep -t permanent
assert_success
write_note trash1 $'trash one\n'
write_note trash2 $'trash two\n'
write_note keep $'keep me\n'

run_cmd "$BIN" rm -t disposable
assert_success
assert_file_not_exists "$(note_path trash1)"
assert_file_not_exists "$(note_path trash2)"
assert_file_exists "$(note_path keep)"

run_cmd "$BIN" show keep
assert_success
assert_contains "$STDOUT" "keep me"
}

test_44() {
fresh_home
run_cmd "$BIN" add rmkey -t retained
assert_success
run_cmd "$BIN" add rmtag -t disposable2
assert_success
run_cmd "$BIN" add survivor -t retained
assert_success

write_note rmkey $'remove by key\n'
write_note rmtag $'remove by tag\n'
write_note survivor $'survive rm union\n'

run_cmd "$BIN" rm rmkey -t disposable2
assert_success
assert_file_not_exists "$(note_path rmkey)"
assert_file_not_exists "$(note_path rmtag)"
assert_file_exists "$(note_path survivor)"

run_cmd "$BIN" show survivor
assert_success
assert_contains "$STDOUT" "survive rm union"
}

run_test "mv succeeds and preserves content" test_37
run_test "mv missing old key fails with status 1 and preserves existing notes" test_38
run_test "mv existing new key fails with status 1 and preserves existing notes" test_39
run_test "mv invalid new key fails and preserves existing note" test_40
run_test "rm removes key and note file" test_41
run_test "rm missing key fails" test_42
run_test "rm supports tag selection" test_43
run_test "rm combines key and tag selectors with OR semantics" test_44

finish_suite

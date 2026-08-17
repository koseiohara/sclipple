#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="tag-match"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_19() {
setup_tag_match_fixture and
run_cmd "$BIN" ls -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_not_contains "$STDOUT" "[tag-red]"
assert_not_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"

setup_tag_match_fixture or
run_cmd "$BIN" ls -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_contains "$STDOUT" "[tag-red]"
assert_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"
}

test_20() {
setup_tag_match_fixture or
run_cmd "$BIN" --tag-match and ls -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_not_contains "$STDOUT" "[tag-red]"
assert_not_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"

setup_tag_match_fixture and
run_cmd "$BIN" --tag-match or ls -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_contains "$STDOUT" "[tag-red]"
assert_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"
}

test_21() {
setup_tag_match_fixture and
run_cmd "$BIN" show -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_contains "$STDOUT" "both body"
assert_not_contains "$STDOUT" "[tag-red]"
assert_not_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"

setup_tag_match_fixture or
run_cmd "$BIN" show -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_contains "$STDOUT" "[tag-red]"
assert_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"
}

test_22() {
setup_tag_match_fixture or
run_cmd "$BIN" --tag-match and show -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_not_contains "$STDOUT" "[tag-red]"
assert_not_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"

setup_tag_match_fixture and
run_cmd "$BIN" --tag-match or show -t red -t blue
assert_success
assert_contains "$STDOUT" "[tag-both]"
assert_contains "$STDOUT" "[tag-red]"
assert_contains "$STDOUT" "[tag-blue]"
assert_not_contains "$STDOUT" "[tag-none]"
}

test_23() {
setup_tag_match_fixture and
run_cmd "$BIN" search needle-match -t red -t blue
assert_success
assert_contains "$STDOUT" "tag-both"
assert_not_contains "$STDOUT" "tag-red"
assert_not_contains "$STDOUT" "tag-blue"
assert_not_contains "$STDOUT" "tag-none"

setup_tag_match_fixture or
run_cmd "$BIN" search needle-match -t red -t blue
assert_success
assert_contains "$STDOUT" "tag-both"
assert_contains "$STDOUT" "tag-red"
assert_contains "$STDOUT" "tag-blue"
assert_not_contains "$STDOUT" "tag-none"
}

test_24() {
setup_tag_match_fixture or
run_cmd "$BIN" --tag-match and search needle-match -t red -t blue
assert_success
assert_contains "$STDOUT" "tag-both"
assert_not_contains "$STDOUT" "tag-red"
assert_not_contains "$STDOUT" "tag-blue"
assert_not_contains "$STDOUT" "tag-none"

setup_tag_match_fixture and
run_cmd "$BIN" --tag-match or search needle-match -t red -t blue
assert_success
assert_contains "$STDOUT" "tag-both"
assert_contains "$STDOUT" "tag-red"
assert_contains "$STDOUT" "tag-blue"
assert_not_contains "$STDOUT" "tag-none"
}

test_25() {
setup_tag_match_fixture and
run_cmd "$BIN" -t red -t blue
assert_success
assert_contains "$STDOUT" "both body"
assert_not_contains "$STDOUT" "red body"
assert_not_contains "$STDOUT" "blue body"
assert_not_contains "$STDOUT" "none body"

setup_tag_match_fixture or
run_cmd "$BIN" -t red -t blue
assert_success
assert_contains "$STDOUT" "both body"
assert_contains "$STDOUT" "red body"
assert_contains "$STDOUT" "blue body"
assert_not_contains "$STDOUT" "none body"
}

test_26() {
setup_tag_match_fixture or
run_cmd "$BIN" --tag-match and -t red -t blue
assert_success
assert_contains "$STDOUT" "both body"
assert_not_contains "$STDOUT" "red body"
assert_not_contains "$STDOUT" "blue body"
assert_not_contains "$STDOUT" "none body"

setup_tag_match_fixture and
run_cmd "$BIN" --tag-match or -t red -t blue
assert_success
assert_contains "$STDOUT" "both body"
assert_contains "$STDOUT" "red body"
assert_contains "$STDOUT" "blue body"
assert_not_contains "$STDOUT" "none body"
}

test_27() {
setup_tag_match_fixture and
run_cmd "$BIN" rm -t red -t blue
assert_success
assert_file_not_exists "$(note_path tag-both)"
assert_file_exists "$(note_path tag-red)"
assert_file_exists "$(note_path tag-blue)"
assert_file_exists "$(note_path tag-none)"

setup_tag_match_fixture or
run_cmd "$BIN" rm -t red -t blue
assert_success
assert_file_not_exists "$(note_path tag-both)"
assert_file_not_exists "$(note_path tag-red)"
assert_file_not_exists "$(note_path tag-blue)"
assert_file_exists "$(note_path tag-none)"
}

test_28() {
setup_tag_match_fixture or
run_cmd "$BIN" --tag-match and rm -t red -t blue
assert_success
assert_file_not_exists "$(note_path tag-both)"
assert_file_exists "$(note_path tag-red)"
assert_file_exists "$(note_path tag-blue)"
assert_file_exists "$(note_path tag-none)"

setup_tag_match_fixture and
run_cmd "$BIN" --tag-match or rm -t red -t blue
assert_success
assert_file_not_exists "$(note_path tag-both)"
assert_file_not_exists "$(note_path tag-red)"
assert_file_not_exists "$(note_path tag-blue)"
assert_file_exists "$(note_path tag-none)"
}

run_test "rc tag-match controls ls for and and or" test_19
run_test "command-line tag-match controls ls for and and or" test_20
run_test "rc tag-match controls show for and and or" test_21
run_test "command-line tag-match controls show for and and or" test_22
run_test "rc tag-match controls search for and and or" test_23
run_test "command-line tag-match controls search for and and or" test_24
run_test "rc tag-match controls tag-only edit for and and or" test_25
run_test "command-line tag-match controls tag-only edit for and and or" test_26
run_test "rc tag-match controls rm for and and or" test_27
run_test "command-line tag-match controls rm for and and or" test_28

finish_suite

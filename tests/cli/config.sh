#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="config"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_29() {
reset_home

cat > "$HOME/.sclipplerc" <<'RC'
editor = cat
extension = md
RC

run_cmd "$BIN" add mdnote
assert_success

md_note="$(find "$HOME/.sclipple/notes" -type f -name 'mdnote.md' | head -n 1)"
assert_file_exists "$md_note"

printf 'markdown body\n' > "$md_note"

run_cmd "$BIN" show mdnote
assert_success
assert_contains "$STDOUT" "markdown body"
}

test_30() {
reset_home

cat > "$HOME/.sclipplerc" <<'RC'
# comment
   editor   =   "cat"
   extension = 'memo'
RC

run_cmd "$BIN" add quoted
assert_success

quoted_note="$(find "$HOME/.sclipple/notes" -type f -name 'quoted.memo' | head -n 1)"
assert_file_exists "$quoted_note"

printf 'quoted rc body\n' > "$quoted_note"

run_cmd "$BIN" quoted
assert_success
assert_contains "$STDOUT" "quoted rc body"
}

test_31() {
reset_home

cat > "$HOME/.sclipplerc" <<'RC'
editor = cat
extension = txt # not a comment
RC

run_cmd "$BIN" add hash
assert_failure
assert_diagnostic
assert_note_count 0
}

test_32() {
reset_home

custom_dir="$HOME/custom-sclipple"
cat > "$HOME/.sclipplerc" <<RC
editor = cat
extension = log
directory = $custom_dir
RC

run_cmd "$BIN" add custom
assert_success

custom_note="$(find "$custom_dir/notes" -type f -name 'custom.log' | head -n 1)"
assert_file_exists "$custom_note"
assert_file_not_exists "$HOME/.sclipple"
assert_contains "$(cat "$custom_dir/.list.csv")" "custom,"

printf 'custom directory body\n' > "$custom_note"

run_cmd "$BIN" show custom
assert_success
assert_contains "$STDOUT" "custom directory body"

run_cmd "$BIN" git init
assert_success
assert_file_exists "$custom_dir/.git"
}

test_33() {
reset_home

rc_dir="$HOME/rc-storage"
cli_dir="$HOME/cli-storage"
cat > "$HOME/.sclipplerc" <<RC
editor = cat
extension = txt
directory = $rc_dir
RC

run_cmd "$BIN" --directory "$cli_dir" add cli-dir
assert_success

cli_dir_note="$cli_dir/notes/cli-dir.txt"
assert_file_exists "$cli_dir_note"
assert_file_not_exists "$rc_dir/notes/cli-dir.txt"
assert_contains "$(cat "$cli_dir/.list.csv")" "cli-dir,"

printf 'cli directory body\n' > "$cli_dir_note"

run_cmd "$BIN" --directory "$cli_dir" show cli-dir
assert_success
assert_contains "$STDOUT" "cli directory body"
}

test_34() {
reset_home

cat > "$HOME/.sclipplerc" <<'RC'
editor = cat
extension = rcx
RC

run_cmd "$BIN" --extension md add cli-ext
assert_success

assert_file_exists "$HOME/.sclipple/notes/cli-ext.md"
assert_file_not_exists "$HOME/.sclipple/notes/cli-ext.rcx"
assert_contains "$(cat "$HOME/.sclipple/.list.csv")" "cli-ext.md"
}

test_35() {
reset_home

cat > "$HOME/.sclipplerc" <<'RC'
editor = /bin/false
extension = txt
RC

run_cmd "$BIN" add cli-editor
assert_success
write_note cli-editor $'editor override body\n'

run_cmd "$BIN" cli-editor
assert_error_stop

run_cmd "$BIN" --editor cat cli-editor
assert_success
assert_contains "$STDOUT" "editor override body"
}

test_36() {
reset_home

cat > "$HOME/.sclipplerc" <<'RC'
extension = bad/ext
RC

run_cmd "$BIN" add x
assert_failure
assert_diagnostic
assert_note_count 0
}

run_test "rc extension is used for new notes" test_29
run_test "rc parser handles whitespace, line-head comments, and quotes" test_30
run_test "rc parser keeps non-leading hash characters" test_31
run_test "rc directory option is used for storage" test_32
run_test "--directory overrides rc directory" test_33
run_test "--extension overrides rc extension" test_34
run_test "--editor overrides rc editor" test_35
run_test "invalid rc extension is fatal" test_36

finish_suite

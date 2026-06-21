#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${1:-$ROOT/src/sclipple}"

STDOUT=""
STDERR=""
STATUS=0
TEST_HOME=""

fail() {
  echo "FAIL: $*" >&2
  exit 1
}

dump_last_command_output() {
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

assert_file_exists() {
  [ -e "$1" ] || fail "expected file to exist: $1"
}

assert_file_not_exists() {
  [ ! -e "$1" ] || fail "expected file not to exist: $1"
}

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

  [ "$actual" -eq "$expected" ] \
    || fail "expected $expected note files, got $actual"
}

find_note() {
  local key="$1"

  if [ -d "$HOME/.sclipple/notes" ]; then
    find "$HOME/.sclipple/notes" -type f -name "$key--*" | head -n 1
  fi
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

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"

new_home
setup_rc

echo "1. help works without storage"

run_cmd "$BIN"
assert_success
assert_contains "$STDOUT$STDERR" "sclipple"

run_cmd "$BIN" help add
assert_success
assert_contains "$STDOUT$STDERR" "add"

echo "2. add initializes storage"

run_cmd "$BIN" add alpha
assert_success

assert_file_exists "$HOME/.sclipple"
assert_file_exists "$HOME/.sclipple/notes"
assert_file_exists "$HOME/.sclipple/.list.csv"
assert_file_exists "$(find_note alpha)"
assert_contains "$(cat "$HOME/.sclipple/.list.csv")" "alpha,"
assert_note_count 1

echo "3. duplicate add is a non-fatal warning/no-op"

run_cmd "$BIN" add alpha
assert_success
assert_diagnostic
assert_note_count 1

echo "4. add accepts multiple keys"

run_cmd "$BIN" add beta gamma
assert_success

assert_file_exists "$(find_note beta)"
assert_file_exists "$(find_note gamma)"
assert_note_count 3

echo "5. valid key characters are accepted"

run_cmd "$BIN" add A_1-b
assert_success
assert_file_exists "$(find_note A_1-b)"
assert_note_count 4

echo "6. invalid and reserved keys do not create notes"

before_count="$(note_count)"

for key in "." ".." "bad/key" "bad,key" "bad key" "git" "help" "add" "rm" "mv" "ls" "search" "show"; do
  run_cmd "$BIN" add "$key"
  assert_diagnostic

  current_count="$(note_count)"
  [ "$current_count" -eq "$before_count" ] \
    || fail "invalid or reserved key created a note: key=$key before=$before_count after=$current_count"
done

after_count="$(note_count)"
[ "$before_count" -eq "$after_count" ] \
  || fail "invalid or reserved keys changed note count: before=$before_count after=$after_count"

echo "7. ls shows first non-empty lines"

write_note alpha $'\n\nfirst alpha line\nsecond alpha line\nurgent task\n'
write_note beta $'first beta line\nsecond beta line\n'
write_note gamma $'first gamma line\n'

run_cmd "$BIN" ls
assert_success
assert_contains "$STDOUT" "alpha"
assert_contains "$STDOUT" "first alpha line"
assert_contains "$STDOUT" "beta"
assert_contains "$STDOUT" "first beta line"

echo "8. ls supports selected keys"

run_cmd "$BIN" ls beta alpha
assert_success
assert_contains "$STDOUT" "beta"
assert_contains "$STDOUT" "alpha"
assert_not_contains "$STDOUT" "gamma:"

echo "9. ls abbreviates long first lines"

long_line="$(printf '%*s' 160 '' | tr ' ' x)"
write_note gamma "$long_line"$'\n'

run_cmd "$BIN" ls gamma
assert_success
assert_contains "$STDOUT" "..."

echo "10. show prints full notes"

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

echo "11. search supports case-insensitive extended regex"

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
assert_failure
assert_diagnostic

echo "12. edit command invokes configured editor"

run_cmd "$BIN" alpha
assert_success
assert_contains "$STDOUT" "first alpha line"
assert_contains "$STDOUT" "urgent task"

echo "13. rc extension is used for new notes"

reset_home

cat > "$HOME/.sclipplerc" <<'RC'
editor = cat
extension = md
RC

run_cmd "$BIN" add mdnote
assert_success

md_note="$(find "$HOME/.sclipple/notes" -type f -name 'mdnote--*.md' | head -n 1)"
assert_file_exists "$md_note"

printf 'markdown body\n' > "$md_note"

run_cmd "$BIN" show mdnote
assert_success
assert_contains "$STDOUT" "markdown body"

echo "14. rc parser handles whitespace, line-head comments, and quotes"

reset_home

cat > "$HOME/.sclipplerc" <<'RC'
# comment
   editor   =   "cat"
   extension = 'memo'
RC

run_cmd "$BIN" add quoted
assert_success

quoted_note="$(find "$HOME/.sclipple/notes" -type f -name 'quoted--*.memo' | head -n 1)"
assert_file_exists "$quoted_note"

printf 'quoted rc body\n' > "$quoted_note"

run_cmd "$BIN" quoted
assert_success
assert_contains "$STDOUT" "quoted rc body"

echo "15. rc parser keeps non-leading hash characters"

reset_home

cat > "$HOME/.sclipplerc" <<'RC'
editor = cat
extension = txt # not a comment
RC

run_cmd "$BIN" add hash
assert_failure
assert_diagnostic
assert_note_count 0

echo "16. rc directory option is used for storage"

reset_home

custom_dir="$HOME/custom-sclipple"
cat > "$HOME/.sclipplerc" <<RC
editor = cat
extension = log
directory = $custom_dir
RC

run_cmd "$BIN" add custom
assert_success

custom_note="$(find "$custom_dir/notes" -type f -name 'custom--*.log' | head -n 1)"
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

echo "17. invalid rc extension is fatal"

reset_home

cat > "$HOME/.sclipplerc" <<'RC'
extension = bad/ext
RC

run_cmd "$BIN" add x
assert_failure
assert_diagnostic
assert_note_count 0

echo "18. mv succeeds and preserves content"

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
assert_not_contains "$STDOUT" "old:"

echo "19. mv missing old key fails with status 1 and preserves existing notes"

run_cmd "$BIN" mv missing dst
assert_status 1
assert_diagnostic
assert_storage_intact_for_new_other

echo "20. mv existing new key fails with status 1 and preserves existing notes"

run_cmd "$BIN" mv new other
assert_status 1
assert_diagnostic
assert_storage_intact_for_new_other

echo "21. mv invalid new key fails and preserves existing note"

run_cmd "$BIN" mv new ..
assert_status 1
assert_diagnostic

assert_file_exists "$new_path"

run_cmd "$BIN" show new
assert_success
assert_contains "$STDOUT" "content for old"

echo "22. rm removes key and note file"

run_cmd "$BIN" rm new
assert_success

assert_file_not_exists "$new_path"

run_cmd "$BIN" show new
assert_failure
assert_diagnostic

echo "23. rm missing key fails"

run_cmd "$BIN" rm missing
assert_failure
assert_diagnostic

echo "24. storage-dependent commands fail before storage exists"

reset_home
setup_rc

for cmd in ls show; do
  run_cmd "$BIN" "$cmd"
  assert_failure
  assert_diagnostic
done

run_cmd "$BIN" search pattern
assert_failure
assert_diagnostic

run_cmd "$BIN" rm x
assert_failure
assert_diagnostic

run_cmd "$BIN" mv x y
assert_failure
assert_diagnostic

echo "25. storage path conflicts are rejected"

reset_home
setup_rc

printf 'not a directory\n' > "$HOME/.sclipple"

run_cmd "$BIN" add x
assert_failure
assert_diagnostic

reset_home
setup_rc

mkdir "$HOME/.sclipple"
printf 'not a directory\n' > "$HOME/.sclipple/notes"

run_cmd "$BIN" add x
assert_failure
assert_diagnostic

echo "26. broken list file is detected"

reset_home
setup_rc

mkdir -p "$HOME/.sclipple/notes"
cat > "$HOME/.sclipple/.list.csv" <<'LIST'
broken,line,with,too,many,columns
LIST

run_cmd "$BIN" ls
assert_failure
assert_diagnostic

run_cmd "$BIN" show
assert_failure
assert_diagnostic

run_cmd "$BIN" search anything
assert_failure
assert_diagnostic

echo "27. git subcommand runs inside storage"

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

echo "28. git before storage reports diagnostic and returns success in the uploaded source"

reset_home
setup_rc

run_cmd "$BIN" git status
assert_success
assert_diagnostic

echo "All CLI tests passed."

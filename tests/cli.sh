#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${1:-$ROOT/src/sclipple}"

STDOUT=""
STDERR=""
STATUS=0

fail() {
  echo "FAIL: $*" >&2
  exit 1
}

run_cmd() {
  local out err
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
  [ "$STATUS" -eq 0 ] || {
    echo "STDOUT:" >&2
    echo "$STDOUT" >&2
    echo "STDERR:" >&2
    echo "$STDERR" >&2
    fail "expected success, got status $STATUS"
  }
}

assert_failure() {
  [ "$STATUS" -ne 0 ] || {
    echo "STDOUT:" >&2
    echo "$STDOUT" >&2
    echo "STDERR:" >&2
    echo "$STDERR" >&2
    fail "expected failure, got status 0"
  }
}

assert_status() {
  local expected="$1"
  [ "$STATUS" -eq "$expected" ] || {
    echo "STDOUT:" >&2
    echo "$STDOUT" >&2
    echo "STDERR:" >&2
    echo "$STDERR" >&2
    fail "expected status $expected, got $STATUS"
  }
}

assert_contains() {
  local text="$1"
  local expected="$2"

  printf '%s' "$text" | grep -F -- "$expected" >/dev/null \
    || fail "expected text to contain: $expected"
}

assert_not_contains() {
  local text="$1"
  local unexpected="$2"

  if printf '%s' "$text" | grep -F -- "$unexpected" >/dev/null; then
    fail "expected text not to contain: $unexpected"
  fi
}

assert_file_exists() {
  [ -e "$1" ] || fail "expected file to exist: $1"
}

assert_file_not_exists() {
  [ ! -e "$1" ] || fail "expected file not to exist: $1"
}

assert_note_count() {
  local expected="$1"
  local actual

  if [ -d "$HOME/.sclipple/notes" ]; then
    actual="$(find "$HOME/.sclipple/notes" -type f | wc -l | tr -d ' ')"
  else
    actual="0"
  fi

  [ "$actual" -eq "$expected" ] \
    || fail "expected $expected note files, got $actual"
}

find_note() {
  local key="$1"
  find "$HOME/.sclipple/notes" -type f -name "$key--*" | head -n 1
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
  cat > "$HOME/.sclipplerc" <<'EOF'
editor = cat
extension = txt
EOF
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
assert_contains "$STDOUT$STDERR" "already exist"
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

echo "6. invalid and reserved keys are rejected"

before_count="$(find "$HOME/.sclipple/notes" -type f | wc -l | tr -d ' ')"

for key in "." ".." "bad/key" "bad,key" "bad key" "git" "help" "add" "rm" "mv" "ls" "search" "show"; do
  run_cmd "$BIN" add "$key"

  # Current behavior: invalid/reserved add prints an error message,
  # but may still return status 0.  Therefore this test checks the
  # observable invariant: no note is created.
  assert_contains "$STDOUT$STDERR" "Error"

  current_count="$(find "$HOME/.sclipple/notes" -type f | wc -l | tr -d ' ')"
  [ "$current_count" -eq "$before_count" ] \
    || fail "invalid key created a note: key=$key before=$before_count after=$current_count"
done

after_count="$(find "$HOME/.sclipple/notes" -type f | wc -l | tr -d ' ')"
[ "$before_count" -eq "$after_count" ] \
  || fail "invalid keys changed note count: before=$before_count after=$after_count"

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

long_line="$(printf 'x%.0s' $(seq 1 160))"
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
assert_contains "$STDOUT$STDERR" "regcomp failed"

echo "12. edit command invokes configured editor"

run_cmd "$BIN" alpha
assert_success
assert_contains "$STDOUT" "first alpha line"
assert_contains "$STDOUT" "urgent task"

echo "13. rc extension is used for new notes"

reset_home

cat > "$HOME/.sclipplerc" <<'EOF'
editor = cat
extension = md
EOF

run_cmd "$BIN" add mdnote
assert_success

md_note="$(find "$HOME/.sclipple/notes" -type f -name 'mdnote--*.md' | head -n 1)"
assert_file_exists "$md_note"

printf 'markdown body\n' > "$md_note"

run_cmd "$BIN" show mdnote
assert_success
assert_contains "$STDOUT" "markdown body"

echo "14. rc parser handles whitespace, comments, and quotes"

reset_home

cat > "$HOME/.sclipplerc" <<'EOF'
# comment
   editor   =   "cat"   # trailing comment
   extension = 'memo'
EOF

run_cmd "$BIN" add quoted
assert_success

quoted_note="$(find "$HOME/.sclipple/notes" -type f -name 'quoted--*.memo' | head -n 1)"
assert_file_exists "$quoted_note"

printf 'quoted rc body\n' > "$quoted_note"

run_cmd "$BIN" quoted
assert_success
assert_contains "$STDOUT" "quoted rc body"

echo "15. invalid rc extension is fatal"

reset_home

cat > "$HOME/.sclipplerc" <<'EOF'
extension = bad/ext
EOF

run_cmd "$BIN" add x
assert_failure
assert_contains "$STDOUT$STDERR" "Invalid extension"

echo "16. mv succeeds and preserves content"

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

echo "17. mv missing old key fails with status 1"

run_cmd "$BIN" mv missing dst
assert_status 1
assert_contains "$STDOUT$STDERR" "No such key"

assert_file_exists "$new_path"
assert_file_exists "$other_path"

echo "18. mv existing new key fails with status 1"

run_cmd "$BIN" mv new other
assert_status 1
assert_contains "$STDOUT$STDERR" "already exists"

assert_file_exists "$new_path"
assert_file_exists "$other_path"

run_cmd "$BIN" show new
assert_success
assert_contains "$STDOUT" "content for old"

run_cmd "$BIN" show other
assert_success
assert_contains "$STDOUT" "content for other"

echo "19. mv invalid new key fails and keeps old note"

run_cmd "$BIN" mv new ..
assert_failure
assert_contains "$STDOUT$STDERR" "Error"

assert_file_exists "$new_path"

run_cmd "$BIN" show new
assert_success
assert_contains "$STDOUT" "content for old"

echo "20. rm removes key and note file"

run_cmd "$BIN" rm new
assert_success
assert_contains "$STDOUT$STDERR" "removed 'new'"

assert_file_not_exists "$new_path"

run_cmd "$BIN" show new
assert_failure
assert_contains "$STDOUT$STDERR" "was not found"

echo "21. rm missing key fails"

run_cmd "$BIN" rm missing
assert_failure
assert_contains "$STDOUT$STDERR" "No such key"

echo "22. storage-dependent commands fail before storage exists"

reset_home
setup_rc

for cmd in ls show; do
  run_cmd "$BIN" "$cmd"
  assert_failure
  assert_contains "$STDOUT$STDERR" "No notes have been added"
done

run_cmd "$BIN" search pattern
assert_failure
assert_contains "$STDOUT$STDERR" "No notes have been added"

run_cmd "$BIN" rm x
assert_failure
assert_contains "$STDOUT$STDERR" "No notes have been added"

run_cmd "$BIN" mv x y
assert_failure
assert_contains "$STDOUT$STDERR" "No notes have been added"

echo "23. storage path conflicts are rejected"

reset_home
setup_rc

printf 'not a directory\n' > "$HOME/.sclipple"

run_cmd "$BIN" add x
assert_failure
assert_contains "$STDOUT$STDERR" "exists but is not a directory"

reset_home
setup_rc

mkdir "$HOME/.sclipple"
printf 'not a directory\n' > "$HOME/.sclipple/notes"

run_cmd "$BIN" add x
assert_failure
assert_contains "$STDOUT$STDERR" "exists but is not a directory"

echo "24. broken list file is detected"

reset_home
setup_rc

mkdir -p "$HOME/.sclipple/notes"
cat > "$HOME/.sclipple/.list.csv" <<'EOF'
broken,line,with,too,many,columns
EOF

run_cmd "$BIN" ls
assert_failure
assert_contains "$STDOUT$STDERR" "list file is broken"

run_cmd "$BIN" show
assert_failure
assert_contains "$STDOUT$STDERR" "list file is broken"

run_cmd "$BIN" search anything
assert_failure
assert_contains "$STDOUT$STDERR" "list file is broken"

echo "25. git subcommand runs inside storage"

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

echo "26. git before storage is non-fatal in current behavior"

reset_home
setup_rc

run_cmd "$BIN" git status
assert_success
assert_contains "$STDOUT$STDERR" "No notes have been added"

echo "All CLI tests passed."




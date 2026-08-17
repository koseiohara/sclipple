#!/usr/bin/env bash
set -euo pipefail

CURRENT_SUITE="storage"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

[ -x "$BIN" ] || fail "sclipple binary is not executable: $BIN"
new_home

test_45() {
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
}

test_46() {
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
}

test_47() {
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
}

run_test "storage-dependent commands fail before storage exists" test_45
run_test "storage path conflicts are rejected" test_46
run_test "broken list file is detected" test_47

finish_suite

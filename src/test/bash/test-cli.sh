#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

GREEN='\033[0;32m'
RED='\033[0;31m'
OFF='\033[0m'
STATUS=0
TEMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TEMP_DIR"' EXIT

pass() {
	echo -e "    $1, ${GREEN}and it does${OFF}"
}

fail() {
	STATUS=1
	echo -e "    $1, ${RED}but it does not${OFF}"
}

SOURCE="src/test/c/generate/01-functions-and-prototypes.tel"
EXPECTED="src/test/c/generate/01-functions-and-prototypes.expected"
C_SOURCE="src/test/c/translate/01-functions.c"
C_EXPECTED="src/test/c/translate/01-functions.expected"

actual="$(".build/tel" "$SOURCE" 2>/dev/null)"
if [ "$?" = "0" ] && [ "$actual" = "$(cat "$EXPECTED")" ]; then
	pass "a .tel path selects TEL to C"
else
	fail "a .tel path should select TEL to C"
fi

actual="$(".build/tel" "$C_SOURCE" 2>/dev/null)"
if [ "$?" = "0" ] && [ "$actual" = "$(cat "$C_EXPECTED")" ]; then
	pass "a .c path selects C to TEL"
else
	fail "a .c path should select C to TEL"
fi

actual="$(cat "$C_SOURCE" | ".build/tel" --from c 2>/dev/null)"
if [ "$?" = "0" ] && [ "$actual" = "$(cat "$C_EXPECTED")" ]; then
	pass "explicit C stdin is supported"
else
	fail "explicit C stdin should be supported"
fi

actual="$(cat "$SOURCE" | ".build/tel" --from tel 2>/dev/null)"
if [ "$?" = "0" ] && [ "$actual" = "$(cat "$EXPECTED")" ]; then
	pass "explicit TEL stdin remains supported"
else
	fail "explicit TEL stdin should remain supported"
fi

if ".build/tel" "$C_SOURCE" -o "$TEMP_DIR/output.tel" >"$TEMP_DIR/c-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/c-stdout" ] \
	&& [ "$(cat "$TEMP_DIR/output.tel" 2>/dev/null)" = "$(cat "$C_EXPECTED")" ]; then
	pass "-o also writes C-to-TEL artifacts"
else
	fail "-o should write C-to-TEL artifacts"
fi

if ".build/tel" "$SOURCE" -o "$TEMP_DIR/output.c" >"$TEMP_DIR/stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/stdout" ] \
	&& [ "$(cat "$TEMP_DIR/output.c" 2>/dev/null)" = "$(cat "$EXPECTED")" ]; then
	pass "-o writes the artifact without stdout"
else
	fail "-o should write the artifact without stdout"
fi

cp "src/test/c/reject/001-unit-c-style-declaration" "$TEMP_DIR/invalid.tel"
echo "preserve me" >"$TEMP_DIR/preserved.c"
if ! ".build/tel" "$TEMP_DIR/invalid.tel" -o "$TEMP_DIR/preserved.c" \
		>"$TEMP_DIR/failed-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/failed-stdout" ] \
	&& [ "$(cat "$TEMP_DIR/preserved.c")" = "preserve me" ]; then
	pass "failed compilation preserves an existing output"
else
	fail "failed compilation should preserve an existing output"
fi

echo "preserve tel" >"$TEMP_DIR/preserved.tel"
if ! ".build/tel" "src/test/c/reject-c/01-cast.c" -o "$TEMP_DIR/preserved.tel" \
		>"$TEMP_DIR/failed-c-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/failed-c-stdout" ] \
	&& [ "$(cat "$TEMP_DIR/preserved.tel")" = "preserve tel" ]; then
	pass "failed C parsing preserves an existing output"
else
	fail "failed C parsing should preserve an existing output"
fi

cp "$SOURCE" "$TEMP_DIR/program.txt"
if ! ".build/tel" "$TEMP_DIR/program.txt" >"$TEMP_DIR/unknown-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/unknown-stdout" ]; then
	pass "an unknown input extension is rejected"
else
	fail "an unknown input extension should be rejected"
fi

cp "$SOURCE" "$TEMP_DIR/program.C"
if ! ".build/tel" "$TEMP_DIR/program.C" >"$TEMP_DIR/uppercase-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/uppercase-stdout" ]; then
	pass "uppercase .C is not treated as C"
else
	fail "uppercase .C should not be treated as C"
fi

if ! ".build/tel" --from tel "$SOURCE" >"$TEMP_DIR/conflict-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/conflict-stdout" ]; then
	pass "--from cannot override a named input"
else
	fail "--from should not override a named input"
fi

if ! ".build/tel" "$SOURCE" "$SOURCE" >"$TEMP_DIR/duplicate-input-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/duplicate-input-stdout" ]; then
	pass "duplicate inputs are rejected"
else
	fail "duplicate inputs should be rejected"
fi

if ! ".build/tel" "$SOURCE" -o "$TEMP_DIR/one" -o "$TEMP_DIR/two" \
	>"$TEMP_DIR/duplicate-output-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/duplicate-output-stdout" ]; then
	pass "duplicate -o options are rejected"
else
	fail "duplicate -o options should be rejected"
fi

if ! ".build/tel" --from cpp >"$TEMP_DIR/invalid-from-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/invalid-from-stdout" ]; then
	pass "invalid stdin languages are rejected"
else
	fail "invalid stdin languages should be rejected"
fi

if ! ".build/tel" "$TEMP_DIR/missing.c" >"$TEMP_DIR/missing-stdout" 2>/dev/null \
	&& [ ! -s "$TEMP_DIR/missing-stdout" ]; then
	pass "unreadable named files are rejected"
else
	fail "unreadable named files should be rejected"
fi

if ".build/tel" --help >"$TEMP_DIR/help" 2>/dev/null \
	&& grep -q "^Usage:" "$TEMP_DIR/help"; then
	pass "--help documents the command"
else
	fail "--help should document the command"
fi

exit "$STATUS"

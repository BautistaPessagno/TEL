#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

GREEN='\033[0;32m'
RED='\033[0;31m'
OFF='\033[0m'
STATUS=0

echo "Compiler should accept..."
echo ""

for test in $(ls src/test/c/accept/); do
	cat "src/test/c/accept/$test" | ".build/tel" >/dev/null 2>&1
	RESULT="$?"
	if [ "$RESULT" == "0" ]; then
		echo -e "    $test, ${GREEN}and it does${OFF} (status $RESULT)"
	else
		STATUS=1
		echo -e "    $test, ${RED}but it rejects${OFF} (status $RESULT)"
	fi
done
echo ""

echo "Compiler should reject..."
echo ""

for test in $(ls src/test/c/reject/); do
	cat "src/test/c/reject/$test" | ".build/tel" >/dev/null 2>&1
	RESULT="$?"
	if [ "$RESULT" != "0" ]; then
		echo -e "    $test, ${GREEN}and it does${OFF} (status $RESULT)"
	else
		STATUS=1
		echo -e "    $test, ${RED}but it accepts${OFF} (status $RESULT)"
	fi
done
echo ""

echo "Compiler should generate C matching the golden output..."
echo ""

for source in src/test/c/generate/*.tel; do
	expected="${source%.tel}.expected"
	name="$(basename "$source" .tel)"
	if [ ! -f "$expected" ]; then
		STATUS=1
		echo -e "    $name, ${RED}but no .expected golden file exists${OFF}"
		continue
	fi
	actual="$(cat "$source" | ".build/tel" 2>/dev/null)"
	if [ "$?" != "0" ]; then
		STATUS=1
		echo -e "    $name, ${RED}but the compiler rejected the source${OFF}"
		continue
	fi
	if [ "$actual" != "$(cat "$expected")" ]; then
		STATUS=1
		echo -e "    $name, ${RED}but the generated C differs from the golden output${OFF}"
		diff <(echo "$actual") "$expected" | sed 's/^/        /'
		continue
	fi
	# The generated translation unit must also be valid C.
	if ! echo "$actual" | gcc -fsyntax-only -xc - 2>/dev/null; then
		STATUS=1
		echo -e "    $name, ${RED}but the generated C failed gcc -fsyntax-only${OFF}"
		continue
	fi
	echo -e "    $name, ${GREEN}and it does${OFF}"
done
echo ""

echo "Generated C for accepted programs should compile..."
echo ""

if ! command -v gcc >/dev/null 2>&1; then
	echo -e "    ${RED}gcc not found; skipping generated-C compilation checks${OFF}"
	STATUS=1
else
	for test in $(ls src/test/c/accept/); do
		generated="$(cat "src/test/c/accept/$test" | ".build/tel" 2>/dev/null)"
		if [ "$?" != "0" ]; then
			# Acceptance is already covered above; only check programs we accept.
			continue
		fi
		if echo "$generated" | gcc -fsyntax-only -xc - 2>/dev/null; then
			echo -e "    $test, ${GREEN}and it does${OFF}"
		else
			STATUS=1
			echo -e "    $test, ${RED}but gcc rejects the generated C${OFF}"
			echo "$generated" | gcc -fsyntax-only -xc - 2>&1 | grep "error:" | sed 's/^/        /'
		fi
	done
fi
echo ""

echo "Generated C for runnable programs should compile and run..."
echo ""

if ! command -v gcc >/dev/null 2>&1; then
	echo -e "    ${RED}gcc not found; skipping runnable-program checks${OFF}"
	STATUS=1
else
	for source in src/test/c/run/*.tel; do
		[ -e "$source" ] || continue
		name="$(basename "$source" .tel)"
		generated="$(cat "$source" | ".build/tel" 2>/dev/null)"
		if [ "$?" != "0" ]; then
			STATUS=1
			echo -e "    $name, ${RED}but the compiler rejected the source${OFF}"
			continue
		fi
		binary="$(mktemp)"
		compile_errors="$(echo "$generated" | gcc -xc - -o "$binary" 2>&1)"
		if [ "$?" != "0" ]; then
			STATUS=1
			echo -e "    $name, ${RED}but the generated C failed to compile${OFF}"
			echo "$compile_errors" | grep "error:" | sed 's/^/        /'
			rm -f "$binary"
			continue
		fi
		actual_output="$("$binary")"
		actual_status="$?"
		rm -f "$binary"
		# Expected exit status defaults to 0 unless an .exit sidecar overrides it.
		# Note: Unix exit statuses wrap mod 256, so .exit golden values must stay < 256.
		expected_status=0
		if [ -f "src/test/c/run/$name.exit" ]; then
			expected_status="$(cat "src/test/c/run/$name.exit")"
		fi
		if [ "$actual_status" != "$expected_status" ]; then
			STATUS=1
			echo -e "    $name, ${RED}but it exited with status $actual_status (expected $expected_status)${OFF}"
			continue
		fi
		# Stdout is only checked when an .out golden sidecar exists.
		if [ -f "src/test/c/run/$name.out" ]; then
			if [ "$actual_output" != "$(cat "src/test/c/run/$name.out")" ]; then
				STATUS=1
				echo -e "    $name, ${RED}but its stdout differs from the expected output${OFF}"
				diff <(echo "$actual_output") "src/test/c/run/$name.out" | sed 's/^/        /'
				continue
			fi
		fi
		echo -e "    $name, ${GREEN}and it does${OFF}"
	done
fi
echo ""

echo "All done."
exit $STATUS

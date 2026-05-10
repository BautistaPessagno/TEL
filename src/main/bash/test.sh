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
	cat "src/test/c/accept/$test" | ".build/Flex-Bison-Compiler" >/dev/null 2>&1
	RESULT="$?"
	if [ "$RESULT" == "0" ]; then
		echo -e "    $test, ${GREEN}and it does${OFF} (status $RESULT)"
	else
		STATUS=1
		echo -e "    $test, ${RED}but it rejects${OFF} (status $RESULT)"
	fi
done
echo ""

echo "Compiler should convert implicit returns exactly when expected..."
echo ""

assert_implicit_return_count() {
	TEST="$1"
	EXPECTED_COUNT="$2"
	OUTPUT="$(LOGGING_LEVEL=DEBUGGING ".build/Flex-Bison-Compiler" < "src/test/c/accept/$TEST" 2>&1)"
	RESULT="$?"
	COUNT="$(printf "%s\n" "$OUTPUT" | grep -c "_convertExpressionStatementToImplicitReturn")"
	if [ "$RESULT" == "0" ] && [ "$COUNT" == "$EXPECTED_COUNT" ]; then
		echo -e "    $TEST, ${GREEN}converted $COUNT implicit return(s)${OFF}"
	else
		STATUS=1
		echo -e "    $TEST, ${RED}expected $EXPECTED_COUNT implicit return conversion(s), got $COUNT (status $RESULT)${OFF}"
	fi
}

assert_implicit_return_count "05-unit-implicit-return" "5"
assert_implicit_return_count "06-unit-main-implicit-return" "1"
assert_implicit_return_count "07-unit-implicit-return-non-final" "1"
echo ""

echo "Compiler should reject..."
echo ""

for test in $(ls src/test/c/reject/); do
	cat "src/test/c/reject/$test" | ".build/Flex-Bison-Compiler" >/dev/null 2>&1
	RESULT="$?"
	if [ "$RESULT" != "0" ]; then
		echo -e "    $test, ${GREEN}and it does${OFF} (status $RESULT)"
	else
		STATUS=1
		echo -e "    $test, ${RED}but it accepts${OFF} (status $RESULT)"
	fi
done
echo ""

echo "All done."
exit $STATUS

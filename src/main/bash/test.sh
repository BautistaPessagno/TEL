#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

GREEN='\033[0;32m'
RED='\033[0;31m'
OFF='\033[0m'
STATUS=0

assert_no_return_semantic_action() {
    NAME="$1"
    SOURCE="$2"
    OUTPUT="$(printf "%s" "$SOURCE" | LOGGING_LEVEL=DEBUGGING LOG_IGNORED_LEXEMES=false ".build/Flex-Bison-Compiler" 2>&1)"
    RESULT="$?"
    if [ "$RESULT" == "0" ] && ! printf "%s" "$OUTPUT" | grep --quiet "ReturnStatementTopLevelItemSemanticAction"; then
        echo -e "    $NAME, ${GREEN}and it does${OFF} (status $RESULT)"
    else
        STATUS=1
        echo -e "    $NAME, ${RED}but it returns implicitly${OFF} (status $RESULT)"
    fi
}

assert_return_semantic_action_count() {
	NAME="$1"
	EXPECTED_COUNT="$2"
	SOURCE="$3"
	OUTPUT="$(printf "%s" "$SOURCE" | LOGGING_LEVEL=DEBUGGING LOG_IGNORED_LEXEMES=false ".build/Flex-Bison-Compiler" 2>&1)"
	RESULT="$?"
	ACTUAL_COUNT="$(printf "%s" "$OUTPUT" | grep --count "_convertExpressionStatementToImplicitReturn")"
    if [ "$RESULT" == "0" ] && [ "$ACTUAL_COUNT" == "$EXPECTED_COUNT" ]; then
        echo -e "    $NAME, ${GREEN}and it does${OFF} (status $RESULT, count $ACTUAL_COUNT)"
    else
        STATUS=1
        echo -e "    $NAME, ${RED}but it finds $ACTUAL_COUNT return actions${OFF} (status $RESULT)"
    fi
}

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

echo "Compiler should build the expected AST..."
echo ""

assert_no_return_semantic_action "semantic-top-level-expression-is-not-return" "x"
assert_return_semantic_action_count "semantic-only-final-expression-is-implicit-return" "1" "fn foo a:int b:int -> int
    a == b
    log(\"hi\")
    a"

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

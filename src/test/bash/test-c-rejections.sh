#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

STATUS=0
COUNT=0

for source in src/test/c/reject-c/*.c; do
	stdout_file="$(mktemp)"
	stderr_file="$(mktemp)"
	".build/tel" "$source" > "$stdout_file" 2> "$stderr_file"
	result="$?"
	if [ "$result" == "0" ]; then
		echo "    $(basename "$source"), but it was accepted"
		STATUS=1
	elif [ -s "$stdout_file" ]; then
		echo "    $(basename "$source"), but it wrote to stdout"
		STATUS=1
	elif ! grep -Eq "^${source}:[0-9]+:" "$stderr_file"; then
		echo "    $(basename "$source"), but it lacks a filename/line diagnostic"
		sed 's/^/        /' "$stderr_file"
		STATUS=1
	else
		COUNT=$((COUNT + 1))
	fi
	rm -f "$stdout_file" "$stderr_file"
done

if [ "$STATUS" == "0" ]; then
	echo "    $COUNT unsupported C fixtures were rejected cleanly"
fi

exit "$STATUS"

#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

ROUNDTRIP_TEMP="$(mktemp -d)"
trap 'rm -rf "$ROUNDTRIP_TEMP"' EXIT

STATUS=0
TEL_COUNT=0
C_COUNT=0

for source in src/test/c/accept/*; do
	name="$(basename "$source")"
	canonical_c="$ROUNDTRIP_TEMP/$name.c"
	canonical_tel="$ROUNDTRIP_TEMP/$name.tel"
	second_c="$ROUNDTRIP_TEMP/$name.second.c"
	second_tel="$ROUNDTRIP_TEMP/$name.second.tel"

	if ! ".build/tel" < "$source" > "$canonical_c" 2>/dev/null \
		|| ! ".build/tel" "$canonical_c" > "$canonical_tel" 2>/dev/null \
		|| ! ".build/tel" "$canonical_tel" > "$second_c" 2>/dev/null \
		|| ! cmp -s "$canonical_c" "$second_c" \
		|| ! ".build/tel" "$second_c" > "$second_tel" 2>/dev/null \
		|| ! cmp -s "$canonical_tel" "$second_tel"; then
		echo "    TEL inverse law failed: $name"
		STATUS=1
	else
		TEL_COUNT=$((TEL_COUNT + 1))
	fi
done

for source in src/test/c/translate/*.c; do
	name="$(basename "$source" .c)"
	expected_tel="${source%.c}.expected"
	actual_tel="$ROUNDTRIP_TEMP/$name.canonical.tel"
	canonical_c="$ROUNDTRIP_TEMP/$name.canonical.c"
	second_tel="$ROUNDTRIP_TEMP/$name.second.tel"
	second_c="$ROUNDTRIP_TEMP/$name.second.c"

	if ! ".build/tel" "$source" > "$actual_tel" 2>/dev/null \
		|| ! cmp -s "$expected_tel" "$actual_tel" \
		|| ! ".build/tel" "$actual_tel" > "$canonical_c" 2>/dev/null \
		|| ! ".build/tel" "$canonical_c" > "$second_tel" 2>/dev/null \
		|| ! cmp -s "$actual_tel" "$second_tel" \
		|| ! ".build/tel" "$second_tel" > "$second_c" 2>/dev/null \
		|| ! cmp -s "$canonical_c" "$second_c"; then
		echo "    C inverse law failed: $name"
		STATUS=1
	else
		C_COUNT=$((C_COUNT + 1))
	fi
done

if [ "$STATUS" == "0" ]; then
	echo "    $TEL_COUNT canonical TEL fixtures and $C_COUNT supported C fixtures are stable"
fi

exit "$STATUS"

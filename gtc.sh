#!/bin/bash

# Usage: bash gtc.sh <file.tel>
# Translates the TEL program to C and compiles it to a binary, in the container.

INPUT="$1"
OUTPUT="${INPUT%.tel}"

docker compose run --rm compiler bash -c "
  echo '[gtc] translating $INPUT -> $OUTPUT.c'
  src/main/bash/run.sh $INPUT > $OUTPUT.c &&
  echo '[gtc] compiling $OUTPUT.c -> $OUTPUT' &&
  gcc $OUTPUT.c -o $OUTPUT &&
  echo '[gtc] done: $OUTPUT'
"

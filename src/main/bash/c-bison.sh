#! /bin/bash

set -euxo pipefail

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

bison \
	-Wcounterexamples \
	-d "src/main/c/frontend/c-language/syntactic-analysis/CGrammar.y" \
	--output="src/main/c/frontend/c-language/syntactic-analysis/CParser.c"

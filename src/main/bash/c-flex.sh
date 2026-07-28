#! /bin/bash

set -euxo pipefail

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

flex \
	--bison-bridge \
	--bison-locations \
	--header-file="src/main/c/frontend/c-language/lexical-analysis/CScanner.h" \
	--never-interactive \
	--nounistd \
	--noyywrap \
	--outfile="src/main/c/frontend/c-language/lexical-analysis/CScanner.c" \
	--prefix="c_yy" \
	--reentrant \
	--yylineno \
	"src/main/c/frontend/c-language/lexical-analysis/CPatterns.l"

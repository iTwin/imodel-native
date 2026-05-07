#!/usr/bin/env bash
# Copyright (c) Bentley Systems, Incorporated. All rights reserved.
# See LICENSE.md in the repository root for full copyright notice.

# Rebuild SqlFlex.cpp and SqlBison.cpp/SqlBison.h from the grammar sources.
# Requires bison >= 3.8.2 and flex >= 2.6.
#
# On macOS the system bison is too old; install via Homebrew:
#   brew install bison flex
# Then the tools are available at:
#   /opt/homebrew/opt/bison/bin/bison
#   /opt/homebrew/opt/flex/bin/flex

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARSE_DIR="$SCRIPT_DIR/../ECDb/ECSql/Parser"

# ---------------------------------------------------------------------------
# Helper: compare two dotted version strings; succeeds if $1 >= $2
# ---------------------------------------------------------------------------
version_ge() {
    # Returns 0 (success) if version $1 >= version $2, 1 otherwise.
    local IFS=.
    local -a v1=($1) v2=($2)
    local i
    for (( i=0; i<${#v2[@]}; i++ )); do
        local a=${v1[i]:-0} b=${v2[i]:-0}
        if (( a > b )); then return 0; fi
        if (( a < b )); then return 1; fi
    done
    return 0
}

# ---------------------------------------------------------------------------
# Locate bison >= 3.8.2
# ---------------------------------------------------------------------------
BISON_MIN="3.8.2"

if [[ -x "/opt/homebrew/opt/bison/bin/bison" ]]; then
    BISON="/opt/homebrew/opt/bison/bin/bison"
elif command -v bison &>/dev/null; then
    BISON="$(command -v bison)"
else
    echo "ERROR: bison not found. Install with: brew install bison" >&2
    exit 1
fi

BISON_VER=$("$BISON" --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
if [[ -z "$BISON_VER" ]]; then
    echo "ERROR: could not determine bison version from '$BISON --version'" >&2
    exit 1
fi
if ! version_ge "$BISON_VER" "$BISON_MIN"; then
    echo "ERROR: bison $BISON_VER is too old; need >= $BISON_MIN." >&2
    echo "       On macOS, install a newer version with: brew install bison" >&2
    exit 1
fi
echo "Using bison $BISON_VER at $BISON"

# ---------------------------------------------------------------------------
# Locate flex >= 2.6
# ---------------------------------------------------------------------------
FLEX_MIN="2.6"

if [[ -x "/opt/homebrew/opt/flex/bin/flex" ]]; then
    FLEX="/opt/homebrew/opt/flex/bin/flex"
elif command -v flex &>/dev/null; then
    FLEX="$(command -v flex)"
else
    echo "ERROR: flex not found. Install with: brew install flex" >&2
    exit 1
fi

FLEX_VER=$("$FLEX" --version | head -1 | grep -oE '[0-9]+\.[0-9]+(\.[0-9]+)?' | head -1)
if [[ -z "$FLEX_VER" ]]; then
    echo "ERROR: could not determine flex version from '$FLEX --version'" >&2
    exit 1
fi
if ! version_ge "$FLEX_VER" "$FLEX_MIN"; then
    echo "ERROR: flex $FLEX_VER is too old; need >= $FLEX_MIN." >&2
    echo "       On macOS, install a newer version with: brew install flex" >&2
    exit 1
fi
echo "Using flex  $FLEX_VER at $FLEX"

# ---------------------------------------------------------------------------
# Compile Lexical Analyzer  (mirrors: win_flex -i -8 -PSQLyy -L -o ... )
#   -i          case-insensitive scanner
#   -8          8-bit characters
#   -PSQLyy     prefix all yy symbols with SQLyy
#   -L          suppress #line directives
# ---------------------------------------------------------------------------
echo ""
echo "Compiling Lexical Analyzer ..."
"$FLEX" -i -8 -PSQLyy -L \
    -o "$PARSE_DIR/SqlFlex.cpp" \
    "$PARSE_DIR/sqlflex.l"

# ---------------------------------------------------------------------------
# Compile Parser  (mirrors: win_bison -k -l -pSQLyy -bSql -o ... --defines=...)
#   -l / --no-lines   suppress #line directives
#   -pSQLyy           prefix all yy symbols with SQLyy
#   -bSql             file-prefix (not needed when -o/--defines are explicit,
#                     but kept for parity with the .bat)
# Note: -k (report conflicts as errors) was a very old flag; modern bison
#       ignores it, so it is omitted here.
# ---------------------------------------------------------------------------
echo "Compiling Parser ..."
"$BISON" -l -pSQLyy -bSql \
    -o "$PARSE_DIR/SqlBison.cpp" \
    --defines="$PARSE_DIR/SqlBison.h" \
    "$PARSE_DIR/sqlbison.y"

echo ""
echo "Done. Generated files:"
echo "  $PARSE_DIR/SqlFlex.cpp"
echo "  $PARSE_DIR/SqlBison.cpp"
echo "  $PARSE_DIR/SqlBison.h"

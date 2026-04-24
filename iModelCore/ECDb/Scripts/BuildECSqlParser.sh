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
# Locate bison >= 3.8.2
# ---------------------------------------------------------------------------
if [[ -x "/opt/homebrew/opt/bison/bin/bison" ]]; then
    BISON="/opt/homebrew/opt/bison/bin/bison"
elif command -v bison &>/dev/null; then
    BISON="$(command -v bison)"
else
    echo "ERROR: bison not found. Install with: brew install bison" >&2
    exit 1
fi

BISON_VER=$("$BISON" --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
echo "Using bison $BISON_VER at $BISON"

# ---------------------------------------------------------------------------
# Locate flex
# ---------------------------------------------------------------------------
if [[ -x "/opt/homebrew/opt/flex/bin/flex" ]]; then
    FLEX="/opt/homebrew/opt/flex/bin/flex"
elif command -v flex &>/dev/null; then
    FLEX="$(command -v flex)"
else
    echo "ERROR: flex not found. Install with: brew install flex" >&2
    exit 1
fi

FLEX_VER=$("$FLEX" --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
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

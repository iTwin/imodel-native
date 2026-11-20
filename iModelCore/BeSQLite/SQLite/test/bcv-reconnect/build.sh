#!/usr/bin/env bash
# Build a daemon (bcvd_<tag>) and a test client (client_<tag>) into OUT.
#
# usage: build.sh OUT TAG [GIT-REF]
#   GIT-REF  build the bcv sources as of this ref (e.g. origin/main) instead of
#            the working tree, to get a baseline to compare against.
# env:
#   ASAN=1        build with -fsanitize=address,undefined
#   CURL_INCLUDE  directory containing curl/curl.h (default /usr/include)
#   LIBCURL       libcurl to link (default: -lcurl)
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
SQLITE_DIR=$(cd "$HERE/../.." && pwd)
OUT=$(mkdir -p "$1" && cd "$1" && pwd); TAG=$2; REF=${3:-}
CURL_INCLUDE=${CURL_INCLUDE:-/usr/include}
LIBCURL=${LIBCURL:--lcurl}
CFLAGS="-O1 -g -w"
[ "${ASAN:-0}" = 1 ] && CFLAGS="$CFLAGS -fsanitize=address,undefined"

SRC=$SQLITE_DIR
if [ -n "$REF" ]; then
  SRC=$OUT/src-$TAG; rm -rf "$SRC"; mkdir -p "$SRC"
  cp "$SQLITE_DIR"/*.c "$SQLITE_DIR"/*.h "$SRC"/
  REPO=$(git -C "$SQLITE_DIR" rev-parse --show-toplevel)
  REL=${SQLITE_DIR#"$REPO"/}
  for f in bcv_int.h bcvutil.c blockcachevfs.c blockcachevfsd.c; do
    git -C "$REPO" show "$REF:$REL/$f" > "$SRC/$f"
  done
fi

BCV="$SRC/bcvutil.c $SRC/blockcachevfs.c $SRC/bcvencrypt.c $SRC/bcvlog.c $SRC/bcvmodule.c $SRC/simplexml.c"
LIBS="$LIBCURL -lssl -lcrypto -lpthread -ldl -lm"
INC="-I$SRC -I$CURL_INCLUDE"

# sqlite3.c is large; build it once per OUT.
SQLITE_O=$OUT/sqlite3${ASAN:+-asan$ASAN}.o
[ -f "$SQLITE_O" ] || gcc $CFLAGS $INC -c "$SQLITE_DIR/sqlite3.c" -o "$SQLITE_O"

gcc $CFLAGS $INC -o "$OUT/bcvd_$TAG" "$SRC/blockcachevfsd.c" $BCV "$SQLITE_O" $LIBS
gcc $CFLAGS $INC -o "$OUT/client_$TAG" "$HERE/client.c" $BCV "$SQLITE_O" $LIBS
echo "built $OUT/bcvd_$TAG $OUT/client_$TAG"

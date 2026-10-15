#!/usr/bin/env bash
set -euo pipefail

# Requires Linux, a C compiler, and curl/OpenSSL/SQLite development headers.
test_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
sqlite_dir="$test_dir/../../SQLite"
binary="$(mktemp "${TMPDIR:-/tmp}/blockcache-socket-test.XXXXXX")"
trap 'rm -f "$binary"' EXIT

# Discard unrelated VFS/HTTP functions; only the production socket path is linked.
"${CC:-cc}" ${CPPFLAGS:-} ${CFLAGS:-} \
  -O1 -ffunction-sections -fdata-sections -Wno-deprecated-declarations \
  "$test_dir/BlockCacheSocketTest.c" "$sqlite_dir/bcvutil.c" \
  -Wl,--gc-sections ${LDFLAGS:-} -lsqlite3 -o "$binary"
"$binary"

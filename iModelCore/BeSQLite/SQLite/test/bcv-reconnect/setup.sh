#!/usr/bin/env bash
# Start Azurite on a dedicated port (default 10099, avoiding any developer
# Azurite on 10000), generate the test database, and upload it to "cont1".
#
# usage: setup.sh OUT          (OUT must contain a bcvd_* binary from build.sh)
# The database has 40000 rows: id INTEGER PRIMARY KEY, v = sha256(str(id)) x 30.
# client.c verifies every row it reads against that formula.
set -euo pipefail
OUT=$(cd "$1" && pwd)
BCVD=$(ls "$OUT"/bcvd_* | head -1)
AZURITE_PORT=${AZURITE_PORT:-10099}
AZURITE_NAME=${AZURITE_NAME:-azurite-bcv-reconnect}
KEY='Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=='
SW=(-module "azure?emulator=127.0.0.1:$AZURITE_PORT" -user devstoreaccount1 -auth "$KEY")

if [ "$(docker inspect -f '{{.State.Running}}' "$AZURITE_NAME" 2>/dev/null || true)" != "true" ]; then
  if python3 -c "import socket,sys; s=socket.create_connection(('127.0.0.1',$AZURITE_PORT),0.2); s.close()" 2>/dev/null; then
    echo "Refusing to use 127.0.0.1:$AZURITE_PORT: port is already in use by another process" >&2
    exit 1
  fi
  docker rm -f "$AZURITE_NAME" >/dev/null 2>&1 || true
  docker run -d --rm --name "$AZURITE_NAME" --network host \
    mcr.microsoft.com/azure-storage/azurite \
    azurite-blob --blobHost 127.0.0.1 --blobPort "$AZURITE_PORT" --skipApiVersionCheck
  for _ in $(seq 30); do
    python3 -c "import socket; s=socket.create_connection(('127.0.0.1',$AZURITE_PORT),0.2); s.close()" 2>/dev/null && break
    sleep 1
  done
fi

if ASAN_OPTIONS=detect_leaks=0 "$BCVD" list "${SW[@]}" cont1 2>/dev/null | grep -q '^test\.db '; then
  echo "cont1/test.db already present on 127.0.0.1:$AZURITE_PORT"
  exit 0
fi

python3 - "$OUT/test.db" <<'EOF'
import sqlite3, os, sys, hashlib
p = sys.argv[1]
if os.path.exists(p): os.remove(p)
c = sqlite3.connect(p)
c.execute('pragma page_size=4096'); c.execute('pragma journal_mode=delete')
c.execute('create table t(id integer primary key, v blob)')
c.executemany('insert into t values(?,?)',
    ((i, hashlib.sha256(str(i).encode()).digest()*30) for i in range(40000)))
c.commit()
EOF

# Disable LSan for these one-shot CLI subcommands (upstream parse_switches is
# not freed in create/upload/list).
ASAN_OPTIONS=detect_leaks=0 "$BCVD" create "${SW[@]}" -blocksize 64K cont1
ASAN_OPTIONS=detect_leaks=0 "$BCVD" upload "${SW[@]}" -container cont1 "$OUT/test.db" test.db
ASAN_OPTIONS=detect_leaks=0 "$BCVD" list "${SW[@]}" cont1

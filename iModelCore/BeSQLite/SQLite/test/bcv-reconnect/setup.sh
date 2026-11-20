#!/usr/bin/env bash
# Start Azurite (if not already listening on 127.0.0.1:10000), generate the
# test database and upload it to container "cont1".
#
# usage: setup.sh OUT          (OUT must contain a bcvd_* binary from build.sh)
# The database has 40000 rows: id INTEGER PRIMARY KEY, v = sha256(str(id)) x 30.
# client.c verifies every row it reads against that formula.
set -euo pipefail
OUT=$(cd "$1" && pwd)
BCVD=$(ls "$OUT"/bcvd_* | head -1)
KEY='Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=='
SW=(-module 'azure?emulator=127.0.0.1:10000' -user devstoreaccount1 -auth "$KEY")

if ! (exec 3<>/dev/tcp/127.0.0.1/10000) 2>/dev/null; then
  docker run -d --rm --name azurite-bcv-reconnect --network host \
    mcr.microsoft.com/azure-storage/azurite \
    azurite-blob --blobHost 127.0.0.1 --blobPort 10000 --skipApiVersionCheck
  for _ in $(seq 30); do (exec 3<>/dev/tcp/127.0.0.1/10000) 2>/dev/null && break; sleep 1; done
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

# create fails if cont1 already exists (re-runs); upload below fails loudly if
# the container is genuinely unusable.
"$BCVD" create "${SW[@]}" -blocksize 64K cont1 || true
"$BCVD" upload "${SW[@]}" -container cont1 "$OUT/test.db" test.db
"$BCVD" list "${SW[@]}" cont1

#!/usr/bin/env bash
# Run the matrix or a single scenario inside the lifetime of a disposable fixture.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
label=org.itwin.bcv-reconnect.fixture

# The clients connect to loopback on this machine, not on a remote Docker host.
if [[ -n ${DOCKER_CONTEXT:-} || -z ${DOCKER_HOST:-} ]]; then
  DOCKER_HOST=$(docker context inspect --format '{{.Endpoints.docker.Host}}')
fi
[[ $DOCKER_HOST == unix://* ]] || { echo "A local Unix-socket Docker daemon is required" >&2; exit 1; }
unset DOCKER_CONTEXT
export DOCKER_HOST

fixture_port(){
  local id=$1 owner endpoint
  [[ $id =~ ^[0-9a-f]{64}$ ]] || { echo "Invalid fixture container ID" >&2; return 1; }
  owner=$(docker inspect --type container --format "{{.Id}} {{.State.Running}} {{index .Config.Labels \"$label\"}}" "$id") || return 1
  [[ $owner == "$id true 1" ]] || { echo "Not a running BCV fixture: $id" >&2; return 1; }
  endpoint=$(docker port "$id" 10000/tcp) || return 1
  [[ $endpoint =~ ^127\.0\.0\.1:([1-9][0-9]*)$ ]] || { echo "Unexpected fixture binding: $endpoint" >&2; return 1; }
  printf '%s\n' "${BASH_REMATCH[1]}"
}

if [[ ${1:-} == --port ]]; then
  fixture_port "${2:?missing fixture ID}"
  exit
fi
if (( $# < 2 || $# > 4 )); then
  echo "usage: $0 DAEMON CLIENT [SCENARIO [CACHESIZE]]" >&2
  exit 2
fi
BCVD=$1
seed=$(mktemp "${RUNROOT:-${TMPDIR:-/tmp}}/bcv-seed.XXXXXXXX")
cid=
cleanup(){
  local rc=$?
  rm -f -- "$seed" || rc=1
  if [[ -n $cid ]]; then docker stop "$cid" >/dev/null || rc=1; fi
  exit "$rc"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
cid=$(docker run --rm -d --label "$label=1" -p 127.0.0.1::10000/tcp \
  mcr.microsoft.com/azure-storage/azurite azurite-blob --blobHost 0.0.0.0 --skipApiVersionCheck)
port=$(fixture_port "$cid")
python3 - "$port" <<'PY'
import http.client, sys, time
deadline = time.monotonic() + 30
while time.monotonic() < deadline:
    c = http.client.HTTPConnection("127.0.0.1", int(sys.argv[1]), timeout=1)
    try:
        c.request("GET", "/")
        if c.getresponse().getheader("Server", "").startswith("Azurite-Blob/"):
            break
    except (OSError, http.client.HTTPException):
        pass
    finally:
        c.close()
    time.sleep(.1)
else:
    sys.exit("Run-owned Azurite did not become ready")
PY
KEY='Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=='
SW=(-module "azure?emulator=127.0.0.1:$port" -user devstoreaccount1 -auth "$KEY")
python3 - "$seed" <<'EOF'
import sqlite3, sys, hashlib
c = sqlite3.connect(sys.argv[1])
c.execute('pragma page_size=4096'); c.execute('pragma journal_mode=delete')
c.execute('create table t(id integer primary key, v blob)')
c.executemany('insert into t values(?,?)',
    ((i, hashlib.sha256(str(i).encode()).digest()*30) for i in range(40000)))
c.commit()
EOF

# Disable LSan for these one-shot CLI subcommands (upstream parse_switches is
# not freed in create/upload/list).
ASAN_OPTIONS=detect_leaks=0 "$BCVD" create "${SW[@]}" -blocksize 64K cont1
ASAN_OPTIONS=detect_leaks=0 "$BCVD" upload "${SW[@]}" -container cont1 "$seed" test.db
ASAN_OPTIONS=detect_leaks=0 "$BCVD" list "${SW[@]}" cont1
if (( $# == 2 )); then
  "$HERE/matrix.sh" --fixture "$cid" "$@"
else
  "$HERE/run.sh" --fixture "$cid" "$@"
fi

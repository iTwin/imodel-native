#!/usr/bin/env bash
# Run one fault-injection scenario against a daemon/client pair.
#
# usage: run.sh DAEMON CLIENT SCENARIO [CACHESIZE]
# scenarios:
#   none           no fault
#   cut            proxy RSTs every connection (both sides) - like ztunnel restart
#   cutmany        as cut, with 8 concurrent clients
#   halfcut        daemon side closed now, client side kept open HALF_OPEN_SECS
#   clientcut      client side RST now, daemon side kept open HALF_OPEN_SECS
#                  (client reconnects before the daemon sees old connection die)
#   restart        daemon killed with SIGKILL and restarted
#   wiperestart    daemon SIGKILL + cache directory wiped + restarted + re-attached
#   wiperestartmod wiperestart + remote db update (same iDbId, iVersion++)
#   wiperestartpoll wiperestart (same version), then remote db update + bcv_poll
#   blackhole      cut, then new connections accepted but never served for BH s
#   synblackhole   cut, then listen backlog saturated so connect() poll() times out
#   hellodrop      cut + first reconnect HELLO_REPLY dropped mid-flight
#   idlecut        cut while client is idle between transactions (no WAL_RETRY)
#   eintr          SIGUSR1 storm + synblackhole (EINTR on poll/connect/recv)
#   securecut      cut on a SQLITE_BCV_ATTACH_SECURE (AES-128-CTR) container
#   securerestart  daemon restart + re-attach on a secure container (new key)
#   secureauth     daemon restart + expired SAS 403 during PASS key refresh
#   autoexit       -autoexit daemon with a client killed mid-transaction
#   stale          client reads ROWS ids, sleeps, re-reads the SAME ids (tiny
#                  page cache => each page re-read via xRead). Mid-sleep, cut
#                  while a churn client full-scans to evict unpinned slots.
# env: ITER ROWS SLEEP T(fault time, s) BH(blackhole secs) DARGS(extra daemon args)
#      HALF_OPEN_SECS  CHURN_CLIENT(client binary used for churn; default CLIENT)
#      RUNROOT(parent dir for run directories; default ${TMPDIR:-/tmp})
# The client must already be able to attach container cont1 (see setup.sh).
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
D=$1; C=$2; SC=$3; CS=${4:-64M}; export HALF_OPEN_SECS=${HALF_OPEN_SECS:-3}
W=$(mktemp -d "${RUNROOT:-${TMPDIR:-/tmp}}/bcv-reconnect-run.XXXX"); mkdir -p "$W/dir"
export BLACKHOLE=$W/blackhole
export SYN_BLACKHOLE=$W/syn_blackhole
case $SC in synblackhole|eintr) export SYN_BLACKHOLE_MODE=1 ;; esac
DP=$((22500 + RANDOM % 400)); PP=$((DP + 500))
[ "$SC" = autoexit ] && DARGS="${DARGS:-} -autoexit -resumetimeout 1"
BCV_KEY='Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=='
BCV_SW=(-module 'azure?emulator=127.0.0.1:10000' -user devstoreaccount1 -auth "$BCV_KEY")
VCONT=
make_versioned_cont(){
  VCONT="cont-${SC}-$$-$RANDOM"
  mkdir -p "$W/wdir"
  python3 - "$W/test_v1.db" <<'PY'
import hashlib, sqlite3, sys
c = sqlite3.connect(sys.argv[1])
c.execute("pragma page_size=4096"); c.execute("pragma journal_mode=delete")
c.execute("create table t(id integer primary key, v blob)")
c.executemany("insert into t values(?,?)",
    ((i, hashlib.sha256(f"{i}".encode()).digest()*30) for i in range(2000)))
c.commit(); c.close()
PY
  ASAN_OPTIONS=detect_leaks=0 $D create "${BCV_SW[@]}" -blocksize 64K "$VCONT" >>"$W/attach.log" 2>&1
  ASAN_OPTIONS=detect_leaks=0 $D upload "${BCV_SW[@]}" -container "$VCONT" "$W/test_v1.db" test.db >>"$W/attach.log" 2>&1
}
upload_v2_cont(){
  CONT="$VCONT" MAX_ID=2000 UPLOAD_V2=1 $C "$W/wdir" 0 0 0 >>"$W/attach.log" 2>&1
}
start_daemon(){
  rm -f "$W/dir/portnumber.bcv"
  $D daemon -port $DP -cachesize $CS -log meh ${DARGS:-} "$W/dir" >>"$W/daemon.log" 2>&1 & DPID=$!
  for _ in $(seq 100); do [ -s "$W/dir/portnumber.bcv" ] && break; sleep 0.05; done
  if [ ! -s "$W/dir/portnumber.bcv" ]; then
    echo "daemon failed to write portnumber.bcv" >&2; exit 1
  fi
  if [ "${1:-proxy}" = proxy ]; then
    # Point clients at the proxy instead of the daemon.
    echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
  fi
}
start_daemon
python3 "$HERE/proxy.py" $PP $DP >"$W/proxy.log" 2>&1 & PPID_=$!
sleep 0.5
NCLI=1; [ "$SC" = cutmany ] && NCLI=8
SEC=0; case $SC in securecut|securerestart|secureauth) SEC=1 ;; esac
CP=(); CHP=; SIGP=
if [ "$SC" = stale ]; then
  REPEAT=1 PCACHE=${PCACHE:-10} $C "$W/dir" ${ITER:-3} ${ROWS:-200} ${SLEEP:-4000} >"$W/client1.log" 2>&1 & CP+=($!)
  sleep 1.5
  CHURN=1 ${CHURN_CLIENT:-$C} "$W/dir" 1000000 0 0 >"$W/churn.log" 2>&1 & CHP=$!
elif [ "$SC" = autoexit ]; then
  $C "$W/dir" 1 40 10000 >"$W/client1.log" 2>&1 & CP+=($!)
elif [ "$SC" = idlecut ]; then
  T=${T:-0.4}
  IDLE_SLEEP=${IDLE_SLEEP:-800} $C "$W/dir" ${ITER:-4} ${ROWS:-20} 0 >"$W/client1.log" 2>&1 & CP+=($!)
elif [ "$SC" = wiperestartmod ] || [ "$SC" = wiperestartpoll ]; then
  make_versioned_cont
  T=${T:-0.6}
  CONT="$VCONT" MAX_ID=2000 FIXED_FIRST_HALF=1 PCACHE=2000 IDLE_SLEEP=${IDLE_SLEEP:-2200} \
    $C "$W/dir" ${ITER:-3} ${ROWS:-10} 0 >"$W/client1.log" 2>&1 & CP+=($!)
elif [ "$SC" = eintr ]; then
  T=${T:-1.2}
  $C "$W/dir" ${ITER:-40} ${ROWS:-40} ${SLEEP:-120} stat >"$W/client1.log" 2>&1 & CP+=($!)
  ( sleep 0.25; while kill -0 "${CP[0]}" 2>/dev/null; do kill -USR1 "${CP[0]}" 2>/dev/null || break; sleep 0.005; done ) & SIGP=$!
elif [ "$SC" = secureauth ]; then
  eval "$(python3 - <<'PY'
import base64, datetime, hashlib, hmac, urllib.parse
key = base64.b64decode('Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==')
now = datetime.datetime.now(datetime.timezone.utc)
def sas(sec):
    se = (now + datetime.timedelta(seconds=sec)).strftime('%Y-%m-%dT%H:%M:%SZ')
    sts = f'rl\n\n{se}\n/blob/devstoreaccount1/cont1\n\n\n\n2020-10-02\nc\n\n\n\n\n\n'
    sig = urllib.parse.quote(base64.b64encode(hmac.new(key, sts.encode(), hashlib.sha256).digest()).decode(), safe='')
    return f'sv=2020-10-02&sr=c&sp=rl&se={urllib.parse.quote(se, safe="")}&sig={sig}'
print(f"SAS_SHORT='{sas(3)}'")
print(f"SAS_LONG='{sas(3600)}'")
PY
)"
  touch "$W/use_short_sas"
  T=${T:-1.2}
  MODULE="azure?emulator=127.0.0.1:10000&sas=1" USE_SHORT_SAS_FILE="$W/use_short_sas" \
    SAS_SHORT="$SAS_SHORT" SAS_LONG="$SAS_LONG" SECURE=1 IDLE_SLEEP=3200 \
    $C "$W/dir" ${ITER:-4} ${ROWS:-40} 0 stat >"$W/client1.log" 2>&1 & CP+=($!)
else
  for i in $(seq $NCLI); do
    if [ "$SEC" = 1 ]; then
      SECURE=1 $C "$W/dir" ${ITER:-40} ${ROWS:-40} ${SLEEP:-150} stat >"$W/client$i.log" 2>&1 & CP+=($!)
    elif [ "$SC" = cutmany ]; then
      $C "$W/dir" ${ITER:-40} ${ROWS:-40} ${SLEEP:-250} >"$W/client$i.log" 2>&1 & CP+=($!)
    else
      $C "$W/dir" ${ITER:-40} ${ROWS:-40} ${SLEEP:-150} stat >"$W/client$i.log" 2>&1 & CP+=($!)
    fi
  done
fi
sleep ${T:-2.5}
rc=0
case $SC in
  cut|cutmany|stale|securecut|idlecut) kill -USR1 $PPID_ ;;
  hellodrop) kill -QUIT $PPID_ ;;
  halfcut) kill -USR2 $PPID_ ;;
  clientcut) kill -HUP $PPID_ ;;
  restart) kill -9 $DPID; sleep 0.5; start_daemon ;;
  wiperestart)
    touch "$BLACKHOLE"; kill -9 $DPID; wait $DPID 2>/dev/null
    rm -rf "$W/dir"/*
    start_daemon direct
    ATTACH_ONLY=1 $C "$W/dir" 0 0 0 >>"$W/attach.log" 2>&1 || rc=$?
    echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
    rm -f "$BLACKHOLE"
    ;;
  wiperestartmod)
    touch "$BLACKHOLE"; kill -9 $DPID; wait $DPID 2>/dev/null
    upload_v2_cont || rc=$?
    rm -rf "$W/dir"/*
    start_daemon direct
    CONT="$VCONT" ATTACH_ONLY=1 $C "$W/dir" 0 0 0 >>"$W/attach.log" 2>&1 || rc=$?
    echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
    rm -f "$BLACKHOLE"
    CONT="$VCONT" MAX_ID=2000 ROW_PREFIX="v2:" $C "$W/dir" 2 10 0 >"$W/v2check.log" 2>&1 || rc=$?
    grep -q '^SUMMARY ok=2 err=0 bad_rows=0$' "$W/v2check.log" || { echo "V2_FRESH_OPEN_FAILED" >&2; rc=1; }
    ;;
  wiperestartpoll)
    touch "$BLACKHOLE"; kill -9 $DPID; wait $DPID 2>/dev/null
    rm -rf "$W/dir"/*
    start_daemon direct
    CONT="$VCONT" ATTACH_ONLY=1 $C "$W/dir" 0 0 0 >>"$W/attach.log" 2>&1 || rc=$?
    echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
    rm -f "$BLACKHOLE"
    sleep 2.2
    upload_v2_cont || rc=$?
    CONT="$VCONT" POLL_ONLY=1 $C "$W/dir" 0 0 0 >>"$W/attach.log" 2>&1 || rc=$?
    ;;
  securerestart)
    touch "$BLACKHOLE"; kill -9 $DPID; wait $DPID 2>/dev/null
    start_daemon direct
    SECURE=1 ATTACH_ONLY=1 $C "$W/dir" 0 0 0 >>"$W/attach.log" 2>&1 || rc=$?
    echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
    rm -f "$BLACKHOLE"
    ;;
  secureauth)
    touch "$BLACKHOLE"; kill -9 $DPID; wait $DPID 2>/dev/null
    rm -f "$W/use_short_sas"
    sleep 2.2
    start_daemon direct
    MODULE="azure?emulator=127.0.0.1:10000&sas=1" SAS_SHORT="$SAS_LONG" SAS_LONG="$SAS_LONG" \
      SECURE=1 ATTACH_ONLY=1 $C "$W/dir" 0 0 0 >>"$W/attach.log" 2>&1 || rc=$?
    echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
    rm -f "$BLACKHOLE"
    ;;
  blackhole) touch "$BLACKHOLE"; kill -USR1 $PPID_; sleep ${BH:-3}; rm -f "$BLACKHOLE" ;;
  synblackhole) touch "$SYN_BLACKHOLE"; sleep 0.15; kill -USR1 $PPID_; sleep ${BH:-2.4}; rm -f "$SYN_BLACKHOLE" ;;
  eintr) touch "$SYN_BLACKHOLE"; sleep 0.15; kill -USR1 $PPID_; sleep ${BH:-1.2}; rm -f "$SYN_BLACKHOLE" ;;
  autoexit)
    kill -9 "${CP[0]}" 2>/dev/null; wait "${CP[0]}" 2>/dev/null; CP=()
    kill -USR1 $PPID_
    for _ in $(seq 100); do kill -0 $DPID 2>/dev/null || break; sleep 0.1; done
    if kill -0 $DPID 2>/dev/null; then
      echo "AUTOEXIT_TIMEOUT" >>"$W/daemon.log"; kill -9 $DPID 2>/dev/null; rc=1
    else
      wait $DPID || rc=$?
    fi
    if [ "$rc" -eq 0 ] && ! grep -q "daemon leaked" "$W/daemon.log"; then
      echo "SUMMARY ok=1 err=0 bad_rows=0" >>"$W/client1.log"
    else
      echo "SUMMARY ok=0 err=1 bad_rows=0" >>"$W/client1.log"
    fi
    ;;
esac
for p in "${CP[@]}"; do wait $p || rc=$?; done
[ -n "$SIGP" ] && { kill $SIGP 2>/dev/null; wait $SIGP 2>/dev/null; }
if [ -n "$CHP" ]; then
  kill -TERM $CHP 2>/dev/null; wait $CHP || rc=$?
  pcw=$(awk '/disconnect\.\./{d=1} d && /writing .* to cache slot/{n++} END{print n+0}' "$W/daemon.log")
  [ "$pcw" -gt 0 ] || { echo "NO_POST_CUT_CHURN_WRITES" >&2; rc=1; }
fi
kill $PPID_ $DPID 2>/dev/null; wait 2>/dev/null
[ -n "$VCONT" ] && ASAN_OPTIONS=detect_leaks=0 $D destroy "${BCV_SW[@]}" "$VCONT" >>"$W/attach.log" 2>&1 || true
echo "== $SC ($(basename "${D##* }")/$(basename "${C##* }")) workdir=$W exit=$rc"
for f in "$W"/client*.log; do
  grep -E 'ERR|MISMATCH|create|attach|open' "$f" | sed 's/^iter [0-9]* //' | sort | uniq -c | sort -rn | head -3
  grep SUMMARY "$f"
done
[ -f "$W/churn.log" ] && grep SUMMARY "$W/churn.log"
[ "$SC" = stale ] && echo "post_cut_writes=${pcw:-0}"
[ "$SC" = idlecut ] && echo "read0_count=$(grep -c 'READ(0,' "$W/daemon.log" || true)"
grep -h 'slow read' "$W"/client*.log | head -2
grep -hoE 'parked session|resumed session|failed to resume|discarded parked|expired parked|supersedes|\(http=403\)' "$W/daemon.log" | sort | uniq -c
if grep -qE 'AddressSanitizer|runtime error|daemon leaked' "$W"/*.log; then echo "SANITIZER: report in $W"; fi
exit $rc

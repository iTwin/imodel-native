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
#   blackhole      cut, then new connections accepted but never served for BH s
#   securecut      cut on a SQLITE_BCV_ATTACH_SECURE (AES-128-CTR) container
#   securerestart  daemon restart + re-attach on a secure container (new key)
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
DP=$((22500 + RANDOM % 400)); PP=$((DP + 500))
[ "$SC" = autoexit ] && DARGS="${DARGS:-} -autoexit -resumetimeout 1"
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
SEC=0; case $SC in securecut|securerestart) SEC=1 ;; esac
CP=(); CHP=
if [ "$SC" = stale ]; then
  REPEAT=1 PCACHE=${PCACHE:-10} $C "$W/dir" ${ITER:-3} ${ROWS:-200} ${SLEEP:-4000} >"$W/client1.log" 2>&1 & CP+=($!)
  sleep 1.5
  CHURN=1 ${CHURN_CLIENT:-$C} "$W/dir" 1000000 0 0 >"$W/churn.log" 2>&1 & CHP=$!
elif [ "$SC" = autoexit ]; then
  $C "$W/dir" 1 40 10000 >"$W/client1.log" 2>&1 & CP+=($!)
else
  for i in $(seq $NCLI); do
    if [ "$SEC" = 1 ]; then
      SECURE=1 $C "$W/dir" ${ITER:-40} ${ROWS:-40} ${SLEEP:-150} stat >"$W/client$i.log" 2>&1 & CP+=($!)
    else
      $C "$W/dir" ${ITER:-40} ${ROWS:-40} ${SLEEP:-150} stat >"$W/client$i.log" 2>&1 & CP+=($!)
    fi
  done
fi
sleep ${T:-2.5}
rc=0
case $SC in
  cut|cutmany|stale|securecut) kill -USR1 $PPID_ ;;
  halfcut) kill -USR2 $PPID_ ;;
  clientcut) kill -HUP $PPID_ ;;
  restart) kill -9 $DPID; sleep 0.5; start_daemon ;;
  securerestart)
    touch "$BLACKHOLE"; kill -9 $DPID; wait $DPID 2>/dev/null
    start_daemon direct
    SECURE=1 ATTACH_ONLY=1 $C "$W/dir" 0 0 0 >>"$W/attach.log" 2>&1 || rc=$?
    echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
    rm -f "$BLACKHOLE"
    ;;
  blackhole) touch "$BLACKHOLE"; kill -USR1 $PPID_; sleep ${BH:-3}; rm -f "$BLACKHOLE" ;;
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
[ -n "$CHP" ] && { kill $CHP 2>/dev/null; wait $CHP 2>/dev/null; }
kill $PPID_ $DPID 2>/dev/null; wait 2>/dev/null
echo "== $SC ($(basename "${D##* }")/$(basename "${C##* }")) workdir=$W exit=$rc"
for f in "$W"/client*.log; do
  grep -E 'ERR|MISMATCH|create|attach|open' "$f" | sed 's/^iter [0-9]* //' | sort | uniq -c | sort -rn | head -3
  grep SUMMARY "$f"
done
grep -h 'slow read' "$W"/client*.log | head -2
grep -hoE 'parked session|resumed session|failed to resume|discarded parked|expired parked|supersedes' "$W/daemon.log" | sort | uniq -c
if grep -qE 'AddressSanitizer|runtime error|daemon leaked' "$W"/*.log; then echo "SANITIZER: report in $W"; fi
exit $rc

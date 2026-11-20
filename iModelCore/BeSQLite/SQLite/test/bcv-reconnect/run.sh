#!/usr/bin/env bash
# Run one fault-injection scenario against a daemon/client pair.
#
# usage: run.sh DAEMON CLIENT SCENARIO [CACHESIZE]
# scenarios:
#   none       no fault
#   cut        proxy RSTs every connection (both sides) - like a ztunnel restart
#   cutmany    as cut, with 8 concurrent clients
#   halfcut    daemon side closed now, client side kept open HALF_OPEN_SECS
#   clientcut  client side RST now, daemon side kept open HALF_OPEN_SECS
#              (client reconnects before the daemon sees the old connection die)
#   restart    daemon killed with SIGKILL and restarted
#   blackhole  cut, then new connections are accepted but never served for BH secs
#   stale      client reads ROWS ids, sleeps, re-reads the SAME ids (tiny page
#              cache => each page re-read via xRead). Mid-sleep, connections are
#              cut while a churn client full-scans to evict unpinned cache slots.
#              With a small CACHESIZE this exposes stale cache-slot reads.
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
start_daemon(){
  $D daemon -addr 127.0.0.1 -port $DP -cachesize $CS -log meh ${DARGS:-} "$W/dir" >>"$W/daemon.log" 2>&1 & DPID=$!
  for _ in $(seq 50); do [ -f "$W/dir/portnumber.bcv" ] && break; sleep 0.1; done
  sleep 0.3
  # Point clients at the proxy instead of the daemon.
  echo -n "127.0.0.1:$PP" > "$W/dir/portnumber.bcv"
}
start_daemon
python3 "$HERE/proxy.py" $PP $DP >"$W/proxy.log" 2>&1 & PPID_=$!
sleep 0.5
NCLI=1; [ "$SC" = cutmany ] && NCLI=8
CP=(); CHP=
if [ "$SC" = stale ]; then
  REPEAT=1 PCACHE=${PCACHE:-10} $C "$W/dir" ${ITER:-3} ${ROWS:-200} ${SLEEP:-4000} >"$W/client1.log" 2>&1 & CP+=($!)
  sleep 1.5
  CHURN=1 ${CHURN_CLIENT:-$C} "$W/dir" 1000000 0 0 >"$W/churn.log" 2>&1 & CHP=$!
else
  for i in $(seq $NCLI); do $C "$W/dir" ${ITER:-40} ${ROWS:-40} ${SLEEP:-150} stat >"$W/client$i.log" 2>&1 & CP+=($!); done
fi
sleep ${T:-2.5}
case $SC in
  cut|cutmany|stale) kill -USR1 $PPID_ ;;
  halfcut) kill -USR2 $PPID_ ;;
  clientcut) kill -HUP $PPID_ ;;
  restart) kill -9 $DPID; sleep 0.5; start_daemon ;;
  blackhole) touch "$BLACKHOLE"; kill -USR1 $PPID_; sleep ${BH:-3}; rm -f "$BLACKHOLE" ;;
esac
rc=0; for p in "${CP[@]}"; do wait $p || rc=$?; done
[ -n "$CHP" ] && { kill $CHP 2>/dev/null; wait $CHP 2>/dev/null; }
kill $PPID_ $DPID 2>/dev/null; wait 2>/dev/null
echo "== $SC ($(basename "${D##* }")/$(basename "${C##* }")) workdir=$W exit=$rc"
for f in "$W"/client*.log; do
  grep -E 'ERR|MISMATCH|create|attach|open' "$f" | sed 's/^iter [0-9]* //' | sort | uniq -c | sort -rn | head -3
  grep SUMMARY "$f"
done
grep -h 'slow read' "$W"/client*.log | head -2
grep -hoE 'parked session|resumed session|failed to resume|expired parked|supersedes' "$W/daemon.log" | sort | uniq -c
if grep -qE 'AddressSanitizer|runtime error' "$W"/*.log; then echo "SANITIZER: report in $W"; fi
exit $rc

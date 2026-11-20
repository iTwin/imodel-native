#!/usr/bin/env bash
# Run the whole fault-injection matrix and assert the expected outcome of each
# scenario. Exits non-zero if any scenario fails.
#
# usage: matrix.sh DAEMON CLIENT
# env:   STALE_RUNS (default 4), plus anything run.sh accepts.
#
# Expectations (for a daemon and client that both support session resume):
#   none cut cutmany halfcut clientcut blackhole   every read succeeds
#   restart, expiry, noresume                       at most 1 failed transaction
#                                                   per client, then recovery
#   stale (STALE_RUNS times)                        no wrong rows, ever
# In every scenario: no wrong rows, every client finishes, no sanitizer reports.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
D=$1; C=$2
FAILED=()

check(){  # name max_err_per_client expected_clients -- run.sh args...
  local name=$1 maxerr=$2 ncli=$3; shift 4
  local out rc=0; out=$("$HERE/run.sh" "$@" 2>&1) || rc=$?
  local sums; sums=$(grep -E '^SUMMARY ok=' <<<"$out" | grep -v churn || true)
  local n=0; [ -n "$sums" ] && n=$(grep -c . <<<"$sums")
  local why=""
  [ "$rc" -eq 0 ] || why="$why exit=$rc;"
  [ "$n" -eq "$ncli" ] || why="$why expected $ncli SUMMARY lines, got $n;"
  if [ -n "$sums" ]; then
    while read -r _ ok err bad; do
      ok=${ok#ok=}; err=${err#err=}; bad=${bad#bad_rows=}
      [ "${bad:-1}" -eq 0 ] || why="$why bad_rows=$bad;"
      [ "${err:-1}" -le "$maxerr" ] || why="$why err=$err>$maxerr;"
      [ "${ok:-0}" -gt 0 ] || why="$why ok=0;"
    done <<<"$sums"
  fi
  grep -q '^SANITIZER' <<<"$out" && why="$why sanitizer report;"
  if [ -z "$why" ]; then
    echo "PASS $name: $(tr '\n' ' ' <<<"$sums")"
  else
    echo "FAIL $name:$why"; echo "$out" | sed 's/^/    /'; FAILED+=("$name")
  fi
}

for sc in none cut halfcut clientcut blackhole securecut; do check $sc 0 1 -- "$D" "$C" $sc; done
check cutmany 0 8 -- "$D" "$C" cutmany
check restart 1 1 -- "$D" "$C" restart
check securerestart 1 1 -- "$D" "$C" securerestart
T=1 check autoexit 0 1 -- "$D" "$C" autoexit
HALF_OPEN_SECS=4 DARGS="-resumetimeout 1" check expiry 1 1 -- "$D" "$C" halfcut
DARGS="-resumetimeout 0" check noresume 1 1 -- "$D" "$C" cut
for i in $(seq "${STALE_RUNS:-4}"); do
  ROWS=8 T=3 CHURN_CLIENT=${CHURN_CLIENT:-$C} check "stale#$i" 3 1 -- "$D" "$C" stale 2M
done

if [ ${#FAILED[@]} -gt 0 ]; then echo "FAILED: ${FAILED[*]}"; exit 1; fi
echo "ALL PASSED"

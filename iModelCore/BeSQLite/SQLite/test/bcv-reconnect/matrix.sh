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

check(){  # name max_err min_total expected_clients expect_re -- run.sh args...
  local name=$1 maxerr=$2 mintot=$3 ncli=$4 expre=$5; shift 6
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
      [ $(( ${ok:-0} + ${err:-0} )) -ge "$mintot" ] || why="$why total<${mintot};"
    done <<<"$sums"
  fi
  if [ -n "$expre" ] && ! grep -qE "$expre" <<<"$out"; then
    why="$why missing '$expre';"
  fi
  grep -q '^SANITIZER' <<<"$out" && why="$why sanitizer report;"
  if [ -z "$why" ]; then
    echo "PASS $name: $(tr '\n' ' ' <<<"$sums")"
  else
    echo "FAIL $name:$why"; echo "$out" | sed 's/^/    /'; FAILED+=("$name")
  fi
}

check none          0 40 1 ""                  -- "$D" "$C" none
check cut           0 40 1 "resumed session"   -- "$D" "$C" cut
check halfcut       0 40 1 "resumed session"   -- "$D" "$C" halfcut
check clientcut     0 40 1 "supersedes"        -- "$D" "$C" clientcut
check blackhole     0 40 1 "slow read"         -- "$D" "$C" blackhole
check securecut     0 40 1 "resumed session"   -- "$D" "$C" securecut
check secureauth    0  4 1 "\(http=403\)"      -- "$D" "$C" secureauth
check cutmany       0 40 8 "8 resumed session" -- "$D" "$C" cutmany
check restart       1 40 1 "ERR rc="           -- "$D" "$C" restart
check securerestart 1 40 1 "failed to resume"  -- "$D" "$C" securerestart
T=1 check autoexit  0  1 1 "expired parked"    -- "$D" "$C" autoexit
HALF_OPEN_SECS=4 DARGS="-resumetimeout 1" check expiry 1 40 1 "expired parked" -- "$D" "$C" halfcut
DARGS="-resumetimeout 0" check noresume 1 40 1 "ERR rc=" -- "$D" "$C" cut
for i in $(seq "${STALE_RUNS:-4}"); do
  ROWS=8 T=3 CHURN_CLIENT=${CHURN_CLIENT:-$C} check "stale#$i" 3 3 1 "parked session" -- "$D" "$C" stale 2M
done

if [ ${#FAILED[@]} -gt 0 ]; then echo "FAILED: ${FAILED[*]}"; exit 1; fi
echo "ALL PASSED"

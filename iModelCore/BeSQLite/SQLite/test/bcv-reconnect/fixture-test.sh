#!/usr/bin/env bash
# Simulate ambiguous Docker metadata without touching a real listener or container.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
RUNROOT=$(mktemp -d)
trap 'rm -f -- "$RUNROOT/operations"; rmdir -- "$RUNROOT"' EXIT
export RUNROOT DOCKER_HOST=unix:///unused AZURITE_NAME=bystander AZURITE_PORT=10000
unset DOCKER_CONTEXT
id=$(printf '%064x' 1)
export id
docker(){
  printf '%s %s\n' "$1" "${@: -1}" >>"$RUNROOT/operations"
  case $1 in
    run) echo "$id" ;;
    inspect) [[ ${@: -1} == "$id" ]] && echo "$id true $fixture_label" ;;
    port) [[ $2 == "$id" ]] && echo "$binding" ;;
    stop) [[ $2 == "$id" ]] ;;
    *) return 1 ;;
  esac
}
export -f docker
export fixture_label=1 binding=127.0.0.1:49152
[[ $("$HERE/setup.sh" --port "$id") == 49152 ]]

for fault in label binding; do
  : > "$RUNROOT/operations"
  fixture_label=1
  if [[ $fault == label ]]; then fixture_label=0; else binding=0.0.0.0:49152; fi
  if "$HERE/setup.sh" /bin/false /bin/false 2>/dev/null; then
    echo "Setup accepted invalid $fault" >&2; exit 1
  fi
  expected=$'run\ninspect'
  if [[ $fault == binding ]]; then expected+=$'\nport'; fi
  [[ $(awk '{print $1}' "$RUNROOT/operations") == "$expected"$'\nstop' ]]
  [[ $(tail -n1 "$RUNROOT/operations") == "stop $id" ]]
done
echo "PASS fixture endpoint selection and scoped cleanup"

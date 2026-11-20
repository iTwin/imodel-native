# CloudSQLite proxy-connection fault-injection harness

Regression harness for daemon-mode ("proxy") blockcachevfs clients whose TCP
connection to the daemon is cut, e.g. by an Istio ambient ztunnel restart
(iTwin/kubernetes-infrastructure#437). A Python TCP proxy sits between the
clients and the daemon and injects faults; `client.c` verifies every row it
reads.

## Run

Requires gcc, OpenSSL and libcurl (headers + library), python3, and a local
Unix-socket Docker daemon (for Azurite) on Linux.

```bash
cd iModelCore/BeSQLite/SQLite/test/bcv-reconnect
OUT=$(mktemp -d)                             # private build + scratch dir
ASAN=1 ./build.sh $OUT new                  # working tree (ASan/UBSan)
./build.sh $OUT base origin/main            # optional baseline to compare
RUNROOT=$OUT ./matrix.sh $OUT/bcvd_new $OUT/client_new   # exit 0 == all pass
```

If curl headers are not in `/usr/include`, set `CURL_INCLUDE=<dir>` and
`LIBCURL=<path to libcurl.so>`.

`run.sh DAEMON CLIENT SCENARIO [CACHESIZE]` runs a single scenario and keeps
its logs (`daemon.log`, `client*.log`, `proxy.log`) in the printed workdir.
Both entry points start a disposable Azurite instance with a Docker-assigned
loopback port, seed `cont1/test.db` there, and stop only the container ID they
started. The matrix shares that fixture across its scenarios. Neither entry
point needs a pre-existing emulator, fixed port, or separate setup command.
`bash fixture-test.sh` checks that ambiguous endpoint metadata is rejected
and cleanup targets only the newly created ID, without using Docker.

## Scenarios

| Scenario | Fault | Expected with resume support |
|---|---|---|
| none | - | all reads succeed |
| cut / cutmany | RST both sides (1 / 8 clients) | all succeed (session resumed) |
| hellodrop | cut + first reconnect `HELLO_REPLY` dropped mid-flight | all succeed (retry resumes preserved session id) |
| idlecut | cut while client is idle between transactions | all succeed, no extra `WAL_RETRY` `READ(0)` |
| halfcut | daemon side closed, client side open 3 s | all succeed (one slow read) |
| clientcut | client side RST, daemon side open 3 s | all succeed (old connection superseded) |
| blackhole / rootblackhole | cut, then accepted connections hang 3 s (db + unlabelled `/` handle) | all succeed (one slow read) |
| synblackhole | cut, then listen backlog saturated so `connect()` `poll()` times out | all succeed (one slow read, session resumed) |
| eintr | continuous `SIGUSR1` (no `SA_RESTART`) + `blackhole` cut (timed `HELLO` `recv`) | all succeed (one slow read, session resumed) |
| securecut | cut on a `SQLITE_BCV_ATTACH_SECURE` (AES-128-CTR) container | all succeed (session resumed) |
| restart / cachewipe | daemon SIGKILL + restart (in-place or `cachefile.bcv`/`blocksdb.bcv` wiped, `-shm` intact) | 1 failed transaction, then recovery on same handle |
| cachewipepoll | `cachewipe` + remote `iVersion++` + `bcv_poll` | linked `-shm` invalidates `PCache`; same handle reads v2 |
| wiperestart | daemon SIGKILL + full cache dir wiped (`-shm` unlinked) | unlinked `-shm` handle fails cleanly; reopened `db` + open `/` and `/cont1` handles recover |
| wiperestartmod / wiperestartrecreate | full cache dir wiped + remote updated (`iVersion++` or recreated container at `iDbId=1,iVersion=1`) | unlinked `-shm` handle rejects cleanly (`SQLITE_IOERR`, 0 bad rows); fresh handle reads v2 |
| securerestart | daemon restart + re-attach on a secure container (new key) | 1 failed transaction, key refreshed, then recovery |
| secureauth | daemon restart + expired SAS 403 during `PASS` key refresh | 403 refreshed, key + `READ` auth updated, all txns succeed |
| autoexit | `-autoexit` daemon with client killed mid-transaction | parked session expires cleanly, exit 0, 0 leaks |
| expiry | halfcut with `-resumetimeout 1` | 1 failed transaction, then recovery |
| noresume | cut with `-resumetimeout 0` | 1 failed transaction, then recovery |
| stale / stalenoresume | cut a paused reader, then scan through a 2 MB cache | same rows after resume; with resume disabled, one I/O error and subsequent transactions succeed |

`stale` is the data-integrity check. Without this change, a client whose
connection drops mid read-transaction keeps reading the cache slots it mapped
before the drop. The daemon released those slots' pins when it saw the
disconnect and may already have reused them for other blocks, so the client
returns **wrong rows with no error**. Against `origin/main` it reports
`bad_rows=8` in some runs.

The reader pauses inside its first transaction after reading the selected
rows. The harness waits for the daemon to log that reader's disconnect,
completes a separate full scan and counts at least 32 logged slot writes,
then releases the reader to reread those same rows. This establishes ordering
and cache pressure, not that any particular slot was overwritten or that
every possible disconnect is safe.

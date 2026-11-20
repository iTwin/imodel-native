# CloudSQLite proxy-connection fault-injection harness

Regression harness for daemon-mode ("proxy") blockcachevfs clients whose TCP
connection to the daemon is cut, e.g. by an Istio ambient ztunnel restart
(iTwin/kubernetes-infrastructure#437). A Python TCP proxy sits between the
clients and the daemon and injects faults; `client.c` verifies every row it
reads.

## Run

Requires gcc, OpenSSL and libcurl (headers + library), python3, and docker
(for Azurite) on Linux.

```bash
cd iModelCore/BeSQLite/SQLite/test/bcv-reconnect
OUT=/tmp/bcv-reconnect                      # build + scratch dir
ASAN=1 ./build.sh $OUT new                  # working tree (ASan/UBSan)
./build.sh $OUT base origin/main            # optional baseline to compare
./setup.sh $OUT                             # Azurite + test db in container cont1
RUNROOT=$OUT ./matrix.sh $OUT/bcvd_new $OUT/client_new   # exit 0 == all pass
```

If curl headers are not in `/usr/include`, set `CURL_INCLUDE=<dir>` and
`LIBCURL=<path to libcurl.so>`. To stop Azurite afterwards:
`docker stop azurite-bcv-reconnect`.

`run.sh DAEMON CLIENT SCENARIO [CACHESIZE]` runs a single scenario and keeps
its logs (`daemon.log`, `client*.log`, `proxy.log`) in the printed workdir.

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
| eintr | continuous `SIGUSR1` (no `SA_RESTART`) + `synblackhole` cut | all succeed (no false disconnects or aborted waits) |
| securecut | cut on a `SQLITE_BCV_ATTACH_SECURE` (AES-128-CTR) container | all succeed (session resumed) |
| restart / wiperestart | daemon SIGKILL + restart (in-place or wiped cache dir) | 1 failed transaction, then recovery |
| wiperestartmod / wiperestartpoll | wiped cache dir + remote `iVersion` update (across restart or later `bcv_poll`) | unlinked `-shm` handle rejects cleanly (`SQLITE_IOERR`, 0 bad rows); fresh handle reads v2 |
| securerestart | daemon restart + re-attach on a secure container (new key) | 1 failed transaction, key refreshed, then recovery |
| secureauth | daemon restart + expired SAS 403 during `PASS` key refresh | 403 refreshed, key + `READ` auth updated, all txns succeed |
| autoexit | `-autoexit` daemon with client killed mid-transaction | parked session expires cleanly, exit 0, 0 leaks |
| expiry | halfcut with `-resumetimeout 1` | 1 failed transaction, then recovery |
| noresume | cut with `-resumetimeout 0` | 1 failed transaction, then recovery |
| stale / stalenoresume | cut during a read transaction while another client evicts the 2 MB cache (resumed and `-resumetimeout 0`) | never any wrong rows |

`stale` is the data-integrity check. Without this change, a client whose
connection drops mid read-transaction keeps reading the cache slots it mapped
before the drop. The daemon released those slots' pins when it saw the
disconnect and may already have reused them for other blocks, so the client
returns **wrong rows with no error**. Against `origin/main` it reports
`bad_rows=8` in some runs.

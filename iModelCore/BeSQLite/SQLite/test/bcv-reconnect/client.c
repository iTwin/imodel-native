/* Test client: proxy-mode bcvfs, one long-lived connection, verifies every row.
** usage: client DIR NITER ROWS SLEEPMS [stat]
** Each iteration: BEGIN; read ROWS random rows; sleep SLEEPMS; read ROWS more; COMMIT.
** Rows are verified against v = sha256(str(id)) x 30. Prints one line per iteration. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <openssl/sha.h>
#include "sqlite3.h"
#include "blockcachevfs.h"

static const char *KEY = "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==";
static int xAuth(void *p, const char *a, const char *b, const char *c, char **pz){
  *pz = sqlite3_mprintf("%s", KEY); return SQLITE_OK;
}
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }

/* aId: if non-NULL and bReplay, read these ids; else pick random ids and
** record them into aId (if non-NULL). */
static int readRows(sqlite3_stmt *pStmt, int n, int *pnBad, double *pMaxMs, int *aId, int bReplay){
  int i, rc = SQLITE_OK;
  for(i=0; i<n; i++){
    int id = (aId && bReplay) ? aId[i] : rand() % 40000;
    if( aId && !bReplay ) aId[i] = id;
    double t0 = now(), ms;
    sqlite3_bind_int(pStmt, 1, id);
    rc = sqlite3_step(pStmt);
    if( rc==SQLITE_ROW ){
      char s[16]; unsigned char h[32]; int k;
      const unsigned char *v = sqlite3_column_blob(pStmt, 0);
      int nv = sqlite3_column_bytes(pStmt, 0);
      snprintf(s, sizeof(s), "%d", id);
      SHA256((unsigned char*)s, strlen(s), h);
      if( nv!=32*30 ){ (*pnBad)++; }
      else for(k=0; k<30; k++) if( memcmp(&v[k*32], h, 32) ){ (*pnBad)++; break; }
      rc = SQLITE_OK;
    }else if( rc==SQLITE_DONE ){
      (*pnBad)++; rc = SQLITE_OK;      /* every id must exist */
    }
    sqlite3_reset(pStmt);
    ms = (now()-t0)*1000; if( ms>*pMaxMs ) *pMaxMs = ms;
    if( rc!=SQLITE_OK ) break;
  }
  return rc;
}

int main(int argc, char **argv){
  sqlite3_bcvfs *pFs = 0; sqlite3 *db = 0; sqlite3_stmt *pStmt = 0, *pStat = 0;
  char *zErr = 0; int rc, it;
  const char *zDir = argv[1];
  int nIter = atoi(argv[2]), nRow = atoi(argv[3]), sleepMs = atoi(argv[4]);
  int bStat = argc>5;
  int bRepeat = getenv("REPEAT")!=0;
  int bChurn = getenv("CHURN")!=0;
  int bSecure = getenv("SECURE")!=0;
  int bAttachOnly = getenv("ATTACH_ONLY")!=0;
  int attachFlags = SQLITE_BCV_ATTACH_IFNOT | (bSecure ? SQLITE_BCV_ATTACH_SECURE : 0);
  int *aId = calloc(nRow>0?nRow:1, sizeof(int));
  int nOk = 0, nErr = 0, nBadTotal = 0;
  setvbuf(stdout, 0, _IOLBF, 0);
  srand(getpid());
  rc = sqlite3_bcvfs_create(zDir, "bcv", &pFs, &zErr);
  if( rc ){ printf("create: %d %s\n", rc, zErr); return 1; }
  sqlite3_bcvfs_auth_callback(pFs, 0, xAuth);
  rc = sqlite3_bcvfs_attach(pFs, "azure?emulator=127.0.0.1:10000", "devstoreaccount1", "cont1", 0, attachFlags, &zErr);
  if( rc ){ printf("attach: %d %s\n", rc, zErr); return 1; }
  if( bAttachOnly ){
    free(aId);
    sqlite3_bcvfs_destroy(pFs);
    return 0;
  }
  rc = sqlite3_open_v2("file:/cont1/test.db", &db, SQLITE_OPEN_READONLY|SQLITE_OPEN_URI, "bcv");
  if( rc ){ printf("open: %d %s\n", rc, sqlite3_errmsg(db)); return 1; }
  sqlite3_bcvfs_register_vtab(db);
  sqlite3_exec(db, "PRAGMA bcv_client = 'test-client'", 0, 0, 0);
  if( getenv("PCACHE") ){
    char *z = sqlite3_mprintf("PRAGMA cache_size=%s", getenv("PCACHE"));
    sqlite3_exec(db, z, 0, 0, 0); sqlite3_free(z);
  }
  if( bChurn ){
    /* Evict as much of the daemon cache as possible, forever (until killed
    ** or nIter scans). Errors are counted but not fatal. */
    sqlite3_stmt *pScan = 0;
    sqlite3_prepare_v2(db, "SELECT count(*), sum(length(v)) FROM t", -1, &pScan, 0);
    for(it=0; it<nIter; it++){
      rc = sqlite3_step(pScan);
      if( rc!=SQLITE_ROW ){ nErr++; printf("churn %d ERR rc=%d (%s)\n", it, rc, sqlite3_errmsg(db)); }
      else nOk++;
      sqlite3_reset(pScan);
    }
    printf("SUMMARY churn ok=%d err=%d\n", nOk, nErr);
    return 0;
  }
  sqlite3_prepare_v2(db, "SELECT v FROM t WHERE id=?", -1, &pStmt, 0);
  if( bStat ) sqlite3_prepare_v2(db, "SELECT value FROM bcv_stat WHERE name='nclient'", -1, &pStat, 0);
  for(it=0; it<nIter; it++){
    int nBad = 0; double maxMs = 0, t0 = now();
    rc = sqlite3_exec(db, "BEGIN", 0, 0, 0);
    if( rc==SQLITE_OK ) rc = readRows(pStmt, nRow, &nBad, &maxMs, aId, 0);
    if( rc==SQLITE_OK && sleepMs ) usleep(sleepMs*1000);
    if( rc==SQLITE_OK ) rc = readRows(pStmt, nRow, &nBad, &maxMs, aId, bRepeat);
    if( rc==SQLITE_OK && pStat ){
      rc = sqlite3_step(pStat); if( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;
      sqlite3_reset(pStat);
    }
    if( rc!=SQLITE_OK ){
      printf("iter %d ERR rc=%d (%s) bad=%d\n", it, rc, sqlite3_errmsg(db), nBad);
      nErr++;
    }else{
      nOk++;
    }
    sqlite3_exec(db, "COMMIT", 0, 0, 0);
    nBadTotal += nBad;
    if( nBad ) printf("iter %d MISMATCH bad=%d\n", it, nBad);
    if( maxMs>500 ) printf("iter %d slow read %.0f ms (iter %.0f ms)\n", it, maxMs, (now()-t0)*1000);
  }
  printf("SUMMARY ok=%d err=%d bad_rows=%d\n", nOk, nErr, nBadTotal);
  sqlite3_finalize(pStmt); sqlite3_finalize(pStat); sqlite3_close(db);
  sqlite3_bcvfs_destroy(pFs);
  return (nBadTotal ? 3 : 0);
}

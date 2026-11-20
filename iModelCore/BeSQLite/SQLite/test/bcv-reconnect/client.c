/* Test client: proxy-mode bcvfs, one long-lived connection, verifies every row.
** usage: client DIR NITER ROWS SLEEPMS [stat]
** Each iteration: BEGIN; read ROWS random rows; sleep SLEEPMS; read ROWS more; COMMIT.
** Rows are verified against v = sha256(str(id)) x 30. Prints one line per iteration. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <openssl/sha.h>
#include "sqlite3.h"
#include "blockcachevfs.h"

static volatile sig_atomic_t gStop = 0;
static void onTerm(int sig){ (void)sig; gStop = 1; }
static void onUsr1(int sig){ (void)sig; }

static const char *KEY = "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==";
static int xAuth(void *p, const char *a, const char *b, const char *c, char **pz){
  const char *zShortFile = getenv("USE_SHORT_SAS_FILE");
  const char *zShort = getenv("SAS_SHORT");
  const char *zLong = getenv("SAS_LONG");
  if( zShort && zLong ){
    int bShort = (zShortFile && access(zShortFile, F_OK)==0);
    *pz = sqlite3_mprintf("%s", bShort ? zShort : zLong);
  }else{
    *pz = sqlite3_mprintf("%s", KEY);
  }
  return SQLITE_OK;
}
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }

static int gMaxId = 40000;
static const char *gRowPrefix = "";
static const char *gRowPrefixFile = 0;

static void refreshRowPrefix(char *zBuf, size_t nBuf){
  if( gRowPrefixFile ){
    FILE *f = fopen(gRowPrefixFile, "r");
    zBuf[0] = '\0';
    if( f ){
      size_t n = fread(zBuf, 1, nBuf-1, f);
      zBuf[n] = '\0';
      fclose(f);
    }
    gRowPrefix = zBuf;
  }
}

/* Verify that per-handle bcv_stat nquery* counters never go backwards. */
static void checkQueryStats(sqlite3_stmt *pQStat, sqlite3_int64 aPrev[3], int *pnBad){
  sqlite3_int64 aCur[3] = {-1, -1, -1};
  int seen = 0;
  while( sqlite3_step(pQStat)==SQLITE_ROW ){
    const char *z = (const char*)sqlite3_column_text(pQStat, 0);
    sqlite3_int64 v = sqlite3_column_int64(pQStat, 1);
    if( z ){
      if( strcmp(z, "nqueryblock")==0 ){ aCur[0] = v; seen |= 1; }
      else if( strcmp(z, "nquerybyte")==0 ){ aCur[1] = v; seen |= 2; }
      else if( strcmp(z, "nqueryus")==0 ){ aCur[2] = v; seen |= 4; }
    }
  }
  sqlite3_reset(pQStat);
  if( seen==7 ){
    int j;
    for(j=0; j<3; j++){
      if( aCur[j]<0 || aCur[j]<aPrev[j] ) (*pnBad)++;
      else aPrev[j] = aCur[j];
    }
  }
}

/* aId: if non-NULL and bReplay, read these ids; else if iBase>=0 read
** iBase..iBase+n-1; else pick random ids and record them into aId. */
static int readRows(sqlite3_stmt *pStmt, int n, int *pnBad, double *pMaxMs, int *aId, int bReplay, int iBase){
  int i, rc = SQLITE_OK;
  for(i=0; i<n; i++){
    int id = (aId && bReplay) ? aId[i] : (iBase>=0 ? (iBase + i) % gMaxId : rand() % gMaxId);
    if( aId && !bReplay ) aId[i] = id;
    double t0 = now(), ms;
    sqlite3_bind_int(pStmt, 1, id);
    rc = sqlite3_step(pStmt);
    if( rc==SQLITE_ROW ){
      char s[64]; unsigned char h[32]; int k;
      const unsigned char *v = sqlite3_column_blob(pStmt, 0);
      int nv = sqlite3_column_bytes(pStmt, 0);
      snprintf(s, sizeof(s), "%s%d", gRowPrefix, id);
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
  sqlite3_bcvfs *pFs = 0;
  sqlite3 *db = 0, *dbRoot = 0, *dbCont = 0;
  sqlite3_stmt *pStmt = 0, *pStat = 0, *pQStat = 0, *pRootStat = 0, *pContStat = 0;
  char *zErr = 0; int rc, it;
  const char *zDir = argv[1];
  int nIter = atoi(argv[2]), nRow = atoi(argv[3]), sleepMs = atoi(argv[4]);
  int bStat = argc>5;
  int bRepeat = getenv("REPEAT")!=0;
  int bFixedHalf = getenv("FIXED_FIRST_HALF")!=0;
  int bChurn = getenv("CHURN")!=0;
  int bSecure = getenv("SECURE")!=0;
  int bAttachOnly = getenv("ATTACH_ONLY")!=0;
  int bPollOnly = getenv("POLL_ONLY")!=0;
  int bUploadV2 = getenv("UPLOAD_V2")!=0;
  int bRootOnly = getenv("ROOT_ONLY")!=0;
  int bReopenOnIoerr = getenv("REOPEN_ON_IOERR")!=0;
  int idleMs = getenv("IDLE_SLEEP") ? atoi(getenv("IDLE_SLEEP")) : 0;
  const char *zCont = getenv("CONT") ? getenv("CONT") : "cont1";
  const char *zAzPort = getenv("AZURITE_PORT") ? getenv("AZURITE_PORT") : "10099";
  char zDefaultMod[128];
  const char *zMod = 0;
  int attachFlags = SQLITE_BCV_ATTACH_IFNOT | (bSecure ? SQLITE_BCV_ATTACH_SECURE : 0);
  int *aId = calloc(nRow>0?nRow:1, sizeof(int));
  int nOk = 0, nErr = 0, nBadTotal = 0, nReopen = 0;
  sqlite3_int64 aPrevQStat[3] = {0, 0, 0};
  char zUri[128], zContUri[128], zRootSql[192], zPfxBuf[64];
  struct sigaction sa;
  snprintf(zDefaultMod, sizeof(zDefaultMod), "azure?emulator=127.0.0.1:%s", zAzPort);
  zMod = getenv("MODULE") ? getenv("MODULE") : zDefaultMod;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = onUsr1;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR1, &sa, 0);
  if( getenv("MAX_ID") ) gMaxId = atoi(getenv("MAX_ID"));
  if( getenv("ROW_PREFIX") ) gRowPrefix = getenv("ROW_PREFIX");
  gRowPrefixFile = getenv("ROW_PREFIX_FILE");
  setvbuf(stdout, 0, _IOLBF, 0);
  srand(getpid());
  rc = sqlite3_bcvfs_create(zDir, "bcv", &pFs, &zErr);
  if( rc ){ printf("create: %d %s\n", rc, zErr); sqlite3_free(zErr); free(aId); return 1; }
  sqlite3_bcvfs_auth_callback(pFs, 0, xAuth);
  if( bPollOnly ){
    rc = sqlite3_bcvfs_poll(pFs, zCont, &zErr);
    if( rc ){ printf("poll: %d %s\n", rc, zErr ? zErr : ""); sqlite3_free(zErr); }
    free(aId);
    sqlite3_bcvfs_destroy(pFs);
    return (rc ? 1 : 0);
  }
  rc = sqlite3_bcvfs_attach(pFs, zMod, "devstoreaccount1", zCont, 0, attachFlags, &zErr);
  if( rc ){ printf("attach: %d %s\n", rc, zErr); sqlite3_free(zErr); free(aId); sqlite3_bcvfs_destroy(pFs); return 1; }
  if( bUploadV2 ){
    /* Modify test.db in-place via local-mode VFS upload so iDbId stays 1
    ** while ManifestDb.iVersion increments to 2. */
    sqlite3_stmt *pUpd = 0;
    snprintf(zUri, sizeof(zUri), "file:/%s/test.db", zCont);
    rc = sqlite3_open_v2(zUri, &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_URI, "bcv");
    if( rc==SQLITE_OK ) rc = sqlite3_exec(db, "BEGIN", 0, 0, 0);
    if( rc==SQLITE_OK ) rc = sqlite3_prepare_v2(db, "UPDATE t SET v=? WHERE id=?", -1, &pUpd, 0);
    for(it=0; rc==SQLITE_OK && it<gMaxId; it++){
      char s[64]; unsigned char h[32], blob[32*30]; int k;
      snprintf(s, sizeof(s), "v2:%d", it);
      SHA256((unsigned char*)s, strlen(s), h);
      for(k=0; k<30; k++) memcpy(&blob[k*32], h, 32);
      sqlite3_bind_blob(pUpd, 1, blob, sizeof(blob), SQLITE_TRANSIENT);
      sqlite3_bind_int(pUpd, 2, it);
      rc = (sqlite3_step(pUpd)==SQLITE_DONE ? SQLITE_OK : SQLITE_ERROR);
      sqlite3_reset(pUpd);
    }
    sqlite3_finalize(pUpd);
    if( rc==SQLITE_OK ) rc = sqlite3_exec(db, "COMMIT", 0, 0, 0);
    sqlite3_close(db); db = 0;
    if( rc==SQLITE_OK ) rc = sqlite3_bcvfs_upload(pFs, zCont, 0, 0, &zErr);
    if( rc ){ printf("upload_v2: %d %s\n", rc, zErr ? zErr : ""); sqlite3_free(zErr); }
    free(aId);
    sqlite3_bcvfs_destroy(pFs);
    return (rc ? 1 : 0);
  }
  if( bAttachOnly ){
    free(aId);
    sqlite3_bcvfs_destroy(pFs);
    return 0;
  }
  snprintf(zUri, sizeof(zUri), "file:/%s/test.db", zCont);
  rc = sqlite3_open_v2(zUri, &db, SQLITE_OPEN_READONLY|SQLITE_OPEN_URI, "bcv");
  if( rc ){ printf("open: %d %s\n", rc, sqlite3_errmsg(db)); sqlite3_close(db); free(aId); sqlite3_bcvfs_destroy(pFs); return 1; }
  sqlite3_bcvfs_register_vtab(db);
  if( !bChurn ) sqlite3_exec(db, "PRAGMA bcv_client = 'test-client'", 0, 0, 0);
  if( bStat ){
    /* Exercise PRAGMA bcv_client on a disk-backed temp passthrough file
    ** (pPath==NULL, no proxy socket). */
    sqlite3_exec(db, "PRAGMA temp_store=FILE; CREATE TEMP TABLE _t(x); PRAGMA temp.bcv_client='ignored';", 0, 0, 0);

    /* Also hold open "/" (without PRAGMA bcv_client, matching CloudCache::m_cloudDb)
    ** and "/<cont>" metadata handles across proxy disconnects. */
    rc = sqlite3_open_v2("file:/", &dbRoot, SQLITE_OPEN_READONLY|SQLITE_OPEN_URI, "bcv");
    if( rc ){ printf("open root: %d %s\n", rc, sqlite3_errmsg(dbRoot)); return 1; }
    sqlite3_bcvfs_register_vtab(dbRoot);
    snprintf(zRootSql, sizeof(zRootSql), "SELECT count(*) FROM bcv_container WHERE container='%s'", zCont);
    sqlite3_prepare_v2(dbRoot, zRootSql, -1, &pRootStat, 0);

    snprintf(zContUri, sizeof(zContUri), "file:/%s", zCont);
    rc = sqlite3_open_v2(zContUri, &dbCont, SQLITE_OPEN_READONLY|SQLITE_OPEN_URI, "bcv");
    if( rc ){ printf("open cont: %d %s\n", rc, sqlite3_errmsg(dbCont)); return 1; }
    sqlite3_bcvfs_register_vtab(dbCont);
    sqlite3_exec(dbCont, "PRAGMA bcv_client = 'cont-client'", 0, 0, 0);
    sqlite3_prepare_v2(dbCont, "SELECT count(*) FROM bcv_database WHERE database='test.db'", -1, &pContStat, 0);
  }
  if( bRootOnly ){
    for(it=0; it<nIter; it++){
      double t0 = now(), ms;
      int r2 = sqlite3_step(pRootStat);
      if( r2!=SQLITE_ROW ){
        printf("iter %d ERR rc=%d (%s)\n", it, r2, sqlite3_errmsg(dbRoot));
        nErr++;
      }else{
        if( sqlite3_column_int(pRootStat, 0)!=1 ) nBadTotal++;
        nOk++;
      }
      sqlite3_reset(pRootStat);
      ms = (now()-t0)*1000;
      if( ms>500 ) printf("iter %d slow read %.0f ms\n", it, ms);
      if( sleepMs ) usleep(sleepMs*1000);
    }
    printf("SUMMARY ok=%d err=%d bad_rows=%d\n", nOk, nErr, nBadTotal);
    free(aId);
    sqlite3_finalize(pRootStat); sqlite3_finalize(pContStat);
    sqlite3_close(db); sqlite3_close(dbRoot); sqlite3_close(dbCont);
    sqlite3_bcvfs_destroy(pFs);
    return (nBadTotal ? 3 : 0);
  }
  if( getenv("PCACHE") ){
    char *z = sqlite3_mprintf("PRAGMA cache_size=%s", getenv("PCACHE"));
    sqlite3_exec(db, z, 0, 0, 0); sqlite3_free(z);
  }
  if( bChurn ){
    /* Evict as much of the daemon cache as possible, until SIGTERM or
    ** nIter scans. Errors are counted, but at least one scan must succeed. */
    sqlite3_stmt *pScan = 0;
    signal(SIGTERM, onTerm);
    sqlite3_prepare_v2(db, "SELECT count(*), sum(length(v)) FROM t", -1, &pScan, 0);
    for(it=0; it<nIter && !gStop; it++){
      rc = sqlite3_step(pScan);
      if( rc!=SQLITE_ROW ){ nErr++; printf("churn %d ERR rc=%d (%s)\n", it, rc, sqlite3_errmsg(db)); }
      else nOk++;
      sqlite3_reset(pScan);
    }
    printf("SUMMARY churn ok=%d err=%d\n", nOk, nErr);
    sqlite3_finalize(pScan);
    free(aId);
    sqlite3_close(db);
    sqlite3_bcvfs_destroy(pFs);
    return (nOk>0 ? 0 : 2);
  }
  sqlite3_prepare_v2(db, "SELECT v FROM t WHERE id=?", -1, &pStmt, 0);
  if( bStat ){
    sqlite3_prepare_v2(db, "SELECT value FROM bcv_stat WHERE name='nclient'", -1, &pStat, 0);
    sqlite3_prepare_v2(db, "SELECT name, value FROM bcv_stat WHERE name IN ('nqueryblock','nquerybyte','nqueryus')", -1, &pQStat, 0);
  }
  for(it=0; it<nIter; it++){
    int nBad = 0; double maxMs = 0, t0 = now();
    refreshRowPrefix(zPfxBuf, sizeof(zPfxBuf));
    if( pQStat ) checkQueryStats(pQStat, aPrevQStat, &nBad);
    rc = sqlite3_exec(db, "BEGIN", 0, 0, 0);
    if( rc==SQLITE_OK ) rc = readRows(pStmt, nRow, &nBad, &maxMs, aId, 0, bFixedHalf ? 0 : -1);
    if( rc==SQLITE_OK && sleepMs ) usleep(sleepMs*1000);
    if( rc==SQLITE_OK ) rc = readRows(pStmt, nRow, &nBad, &maxMs, aId, bRepeat, bFixedHalf ? (it * nRow + gMaxId / 2) : -1);
    if( rc==SQLITE_OK && pStat ){
      rc = sqlite3_step(pStat); if( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;
      sqlite3_reset(pStat);
    }
    if( rc!=SQLITE_OK ){
      printf("iter %d ERR rc=%d (%s) bad=%d\n", it, rc, sqlite3_errmsg(db), nBad);
      sqlite3_exec(db, "COMMIT", 0, 0, 0);
      nErr++;
      if( bReopenOnIoerr ){
        sqlite3_finalize(pStmt); pStmt = 0;
        sqlite3_finalize(pStat); pStat = 0;
        sqlite3_finalize(pQStat); pQStat = 0;
        sqlite3_close(db); db = 0;
        memset(aPrevQStat, 0, sizeof(aPrevQStat));
        if( sqlite3_open_v2(zUri, &db, SQLITE_OPEN_READONLY|SQLITE_OPEN_URI, "bcv")==SQLITE_OK ){
          sqlite3_bcvfs_register_vtab(db);
          sqlite3_exec(db, "PRAGMA bcv_client = 'test-client'", 0, 0, 0);
          sqlite3_prepare_v2(db, "SELECT v FROM t WHERE id=?", -1, &pStmt, 0);
          if( bStat ){
            sqlite3_prepare_v2(db, "SELECT value FROM bcv_stat WHERE name='nclient'", -1, &pStat, 0);
            sqlite3_prepare_v2(db, "SELECT name, value FROM bcv_stat WHERE name IN ('nqueryblock','nquerybyte','nqueryus')", -1, &pQStat, 0);
          }
          nReopen++;
        }else{
          nBad++;
        }
      }
    }else{
      sqlite3_exec(db, "COMMIT", 0, 0, 0);
      nOk++;
      if( pQStat ) checkQueryStats(pQStat, aPrevQStat, &nBad);
    }
    if( rc==SQLITE_OK && pRootStat ){
      int r2 = sqlite3_step(pRootStat);
      if( r2!=SQLITE_ROW || sqlite3_column_int(pRootStat, 0)!=1 ) nBad++;
      sqlite3_reset(pRootStat);
    }
    if( rc==SQLITE_OK && pContStat ){
      int r2 = sqlite3_step(pContStat);
      if( r2!=SQLITE_ROW || sqlite3_column_int(pContStat, 0)!=1 ) nBad++;
      sqlite3_reset(pContStat);
    }
    nBadTotal += nBad;
    if( nBad ) printf("iter %d MISMATCH bad=%d\n", it, nBad);
    if( maxMs>500 ) printf("iter %d slow read %.0f ms (iter %.0f ms)\n", it, maxMs, (now()-t0)*1000);
    if( idleMs && it+1<nIter ) usleep(idleMs*1000);
  }
  if( nReopen ) printf("REOPENED count=%d\n", nReopen);
  printf("SUMMARY ok=%d err=%d bad_rows=%d\n", nOk, nErr, nBadTotal);
  free(aId);
  sqlite3_finalize(pStmt); sqlite3_finalize(pStat); sqlite3_finalize(pQStat);
  sqlite3_finalize(pRootStat); sqlite3_finalize(pContStat);
  sqlite3_close(db); sqlite3_close(dbRoot); sqlite3_close(dbCont);
  sqlite3_bcvfs_destroy(pFs);
  return (nBadTotal ? 3 : 0);
}

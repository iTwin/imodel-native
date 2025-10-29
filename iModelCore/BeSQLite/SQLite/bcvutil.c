/*
** 2020-05-12
**
******************************************************************************
**
*/

#include "bcv_int.h"
#include "sqlite3.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef __WIN32__
#include <unistd.h>
#endif

#include <openssl/md5.h>

/*
** If SQLITE_BCV_CURL_HANDLE_CONFIG is defined, it must be set to the name
** of a function linked against this file with the following signature.
** Before each HTTPS request is made, the function is invoked with three
** arguments:
**
**  *  The (CURL*) handle that will be used to make the request.
**
**  *  An integer indicating the type of HTTPS request that will be issued.
**     This is one of the SQLITE_BCV_METHOD_XXX values defined in header
**     file bcvmodule.h - 1==GET, 2==PUT, 3==DELETE, 4==HEAD.
**
**  *  A nul-terminated string set to the URI that will be requested. This
**     URI includes all parameters, possibly including parameters containing
**     authentication information.
**
** The function may configure the CURL handle as required. It is anticipated
** that this will be used to configure proxy information for the curl
** request (using e.g. CURLOPT_PROXY).
**
** The function must return an SQLite error code. If the return value is
** SQLITE_OK, then the operation proceeds. Or, if an error is returned, that
** error is propagated up to the caller.
*/
#ifdef SQLITE_BCV_CURL_HANDLE_CONFIG
int SQLITE_BCV_CURL_HANDLE_CONFIG (CURL*, int, const char*);
#endif

/*
** The two supported environment variables.
*/
#define CLOUDSQLITE_REVOKE_BEST_EFFORT "CLOUDSQLITE_REVOKE_BEST_EFFORT"
#define CLOUDSQLITE_CAINFO "CLOUDSQLITE_CAINFO"

#define BCV_MIMETYPE_HDR    "Content-Type:application/octet-stream"
#define BCV_MIMETYPE_HDR_LC "content-type:application/octet-stream"

#define BCV_AWS_DATE_SZ 16
#define BCV_AWS_HASH_SZ (2*SHA256_DIGEST_LENGTH)

#define BCV_AWS_CONTENTHASH_HDR "x-amz-content-sha256:UNSIGNED-PAYLOAD"

/* Maximum value for SQLITE_BCVCONFIG_LOG */
#define BCV_MAX_NREQUEST 32

#define BCV_MAX_RESULTS 5000

#define BCV_DEFAULT_FINDORPHANS 1

#define BCV_MANIFEST_SCHEMA \
  "CREATE TABLE config(                                         \n" \
  "  name TEXT PRIMARY KEY,                                     \n" \
  "  value                                                      \n" \
  ");                                                           \n" \
  "CREATE TABLE database(                                       \n" \
  "  dbid INTEGER PRIMARY KEY AUTOINCREMENT,                    \n" \
  "  name TEXT,                                                 \n" \
  "  version INTEGER,                                           \n" \
  "  blocks INTEGER,                                            \n" \
  "  parent INTEGER REFERENCES database (dbid)                  \n" \
  ");                                                           \n" \
  "                                                             \n" \
  "CREATE TABLE block(                                          \n" \
  "  dbid INTEGER,           /* REFERENCES database(id) */      \n" \
  "  block_offset INTEGER,                                      \n" \
  "  block_file BLOB,                                           \n" \
  "  PRIMARY KEY(dbid, block_offset)                            \n" \
  ") WITHOUT ROWID;                                             \n" \
  "                                                             \n" \
  "CREATE TABLE deleted(block_file BLOB, timestamp INTEGER);    \n"

typedef int(*bcv_progress_cb)(void*, sqlite3_int64, sqlite3_int64);
typedef void(*bcv_log_cb)(void*, const char *);

struct sqlite3_bcv {
  BcvContainer *pCont;            /* Cloud module instance */
  BcvDispatch *pDisp;             /* Dispatcher to manage callbacks */

  char *zErrmsg;                  /* Error message from most recent API call */
  int errCode;                    /* Error code from most recent API call */

  int nRequest;                   /* Max outstanding requests for up/download */
  int bVerbose;                   /* True to use verbose curl handles */
  int nLogLevel;
  int nHttpTimeout;               /* SQLITE_BCVCONFIG_HTTPTIMEOUT option */
  void *pProgressCtx;             /* First argument to pass to xProgress() */
  bcv_progress_cb xProgress;      /* Progress-handler callback */

  int bTestNoKv;                  /* SQLITE_BCVCONFIG_TESTNOKV option */
  int bFindOrphans;               /* SQLITE_BCVCONFIG_FINDORPHANS option */

  bcv_log_cb xBcvLog;
  void *pBcvLogCtx;
  BcvLog *pBcvLogObj;
};

typedef struct sqlite3_bcv_job BcvDispatchJob;
typedef struct sqlite3_bcv_request BcvDispatchReq;

struct BcvDispatch {
  CURLM *pMulti;                  /* The libcurl dispatcher ("multi-handle") */
  int bVerbose;                   /* True to make libcurl verbose */
  int nHttpTimeout;
  int iNextRequestId;             /* Next request-id (for logging only) */
  int iDispatchId;                /* Dispatch id (for logging only) */
  char *zLogmsg;                  /* Logging caption for next http request */
  const char *zClientId;          /* Logging client name */
  BcvDispatchJob *pJobList;       /* List of current jobs */

  void *pLogApp;                  /* First argument to xLog() */
  void (*xLog)(void*,int,const char *zMsg);    /* Log function to invoke */
  BcvLog *pLog;                   /* Log object (may be NULL) */
};

struct sqlite3_bcv_job {
  BcvDispatch *pDispatch;
  int rc;
  int eType;                      /* Type of job (BCV_DISPATCH_XXX value) */
  int bLogEtag;                   /* True to log eTag of http replies */
  char *zLogmsg;                  /* Logging caption */
  const char *zClientId;          /* Logging client name */
  void *pCbApp;                   /* First argument for callback */
  struct {
    void (*xFetch)(void*, int rc, char *zETag, const u8*, int, const u8*, int);
    void (*xPut)(void*, int rc, char *zETag);
    void (*xCreate)(void*, int rc, char *zError);
    void (*xDestroy)(void*, int rc, char *zError);
    int (*xList)(void*, int rc, char *z);
    void (*xDelete)(void*, int rc, char *zError);
  } cb;

  /* Result values */
  char *zETag;
  u8 *aResult;
  int nResult;
  u8 *aHdrs;
  int nHdrs;

  char *zPrefix;                  /* Prefix for List operations */

  BcvContainer *pCont;            /* Associated container object */

  BcvDispatchReq *pPending;       /* Requests yet to be issued */
  BcvDispatchReq *pWaiting;       /* Waiting request list */
  BcvDispatchJob *pNext;          /* Next job belonging to same dispatcher */
};

#define BCV_LOGLEVEL_DEBUG 1

/*
** Values that may be stored in BcvDispatchJob.eType variables.
*/
#define BCV_DISPATCH_FETCH   1
#define BCV_DISPATCH_LIST    2
#define BCV_DISPATCH_PUT     3
#define BCV_DISPATCH_CREATE  4
#define BCV_DISPATCH_DESTROY 5
#define BCV_DISPATCH_DELETE  6

struct sqlite3_bcv_request {
  BcvDispatchJob *pJob;           /* Job that owns this request */
  int eMethod;                    /* SQLITE_BCV_METHOD_GET/PUT/DELETE */
  int nRetry;                     /* # of times request has already failed */
  i64 iRetryTime;                 /* Do not retry before this time */
  char *zUri;                     /* URI */
  char *zUriLog;                  /* URI value for logging */
  struct curl_slist *pList;       /* List of HTTP headers for request */
  void *pApp;                     /* 3rd argument to xCallback */
  void (*xCallback)(sqlite3_bcv_job*, sqlite3_bcv_request*, void*);

  i64 iLogId;                     /* Logging id of request */

  const u8 *aBody;                /* Buffer of data to upload (METHOD_PUT) */
  int nBody;                      /* Size of aBody[] in bytes */
  int iBody;                      /* Current offset in aBody[] */

  int rc;
  CURLcode code;
  char *zStatus;                  /* Most recent Status header from server */
  BcvBuffer body;                 /* Body of reply */
  BcvBuffer hdr;                  /* Headers */

  CURL *pCurl;                    /* Curl easy-handle */
  BcvDispatchReq *pNext;          /* Next in pPending or pWaiting list */
};

int bcvStrcmp(const char *zLeft, const char *zRight){
  if( zLeft==zRight ){
    return 0;
  }else if( zLeft==0 ){
    return -1;
  }else if( zRight==0 ){
    return +1;
  }
  return strcmp(zLeft, zRight);
}

int bcvStrlen(const char *z){
  return (z ? strlen(z) : 0);
}

void fatal_oom_error(void){
  fprintf(stderr, "FATAL: out-of-memory error\n");
  exit(-1);
}

/*
** Wrapper around sqlite3_mprintf() that calls fatal_oom_error() if an 
** OOM occurs.
*/
char *bcvMprintf(const char *zFmt, ...){
  char *zRet;
  va_list ap;
  va_start(ap, zFmt);
  zRet = sqlite3_vmprintf(zFmt, ap);
  va_end(ap);
  if( zRet==0 ) fatal_oom_error();
  return zRet;
}

char *bcvMprintfRc(int *pRc, const char *zFmt, ...){
  char *zRet = 0;
  va_list ap;
  va_start(ap, zFmt);
  if( *pRc==SQLITE_OK ){
    zRet = sqlite3_vmprintf(zFmt, ap);
    if( zRet==0 ) *pRc = SQLITE_NOMEM;
  }
  va_end(ap);
  return zRet;
}


void *bcvMalloc(i64 nByte){
  void *p = sqlite3_malloc64(nByte);
  assert( nByte!=0 );
  if( p==0 ) fatal_oom_error();
  return p;
}

void *bcvRealloc(void *a, i64 nByte){
  void *p = sqlite3_realloc64(a, nByte);
  assert( nByte!=0 );
  if( p==0 ) fatal_oom_error();
  return p;
}

void *bcvMallocZero(i64 nByte){
  void *p = bcvMalloc(nByte);
  memset(p, 0, nByte);
  return p;
}

void *bcvMallocRc(int *pRc, i64 nByte){
  void *pRet = 0;
  if( nByte>0 && *pRc==SQLITE_OK ){
    pRet = sqlite3_malloc64(nByte);
    if( pRet==0 ){
      *pRc = SQLITE_NOMEM;
    }else{
      memset(pRet, 0, nByte);
    }
  }
  return pRet;
}

static void *bcvReallocOrFreeRc(int *pRc, void *pIn, int nByte){
  void *pRet = 0;
  if( nByte>0 && *pRc==SQLITE_OK ){
    pRet = sqlite3_realloc(pIn, nByte);
    if( pRet==0 ){
      sqlite3_free(pIn);
      *pRc = SQLITE_NOMEM;
    }
  }else{
    sqlite3_free(pIn);
  }
  return pRet;
}

char *bcvStrdupRc(int *pRc, const char *z){
  char *zRet = 0;
  if( z ){
    int n = bcvStrlen(z);
    zRet = bcvMallocRc(pRc, n+1);
    if( zRet ){
      memcpy(zRet, z, n+1);
    }
  }
  return zRet;
}

void bcvBufferAppendRc(int *pRc, BcvBuffer *p, const void *a, int n){
  if( n ){
    int nByte = p->nAlloc;
    if( p->nData+n>nByte ){
      if( nByte==0 ){
        nByte = n;
      }else{
        nByte = p->nAlloc;
        while( nByte<p->nData+n ){ nByte = nByte*2; }
      }
      p->aData = bcvReallocOrFreeRc(pRc, p->aData, nByte);
    }
    if( p->aData ){
      memcpy(&p->aData[p->nData], a, n);
      p->nData += n;
      p->nAlloc = nByte;
    }
  }
}

void bcvBufferZero(BcvBuffer *p){
  sqlite3_free(p->aData);
  memset(p, 0, sizeof(BcvBuffer));
}

/*
** Return a duplicate of the nIn byte buffer at aIn[]. It is the 
** responsibility of the caller to eventually free the returned buffer
** by passing a pointer to it to sqlite3_free().
*/
u8 *bcvMemdup(int nIn, const u8 *aIn){
  u8 *aRet = bcvMalloc(nIn+1);
  memcpy(aRet, aIn, nIn);
  aRet[nIn] = 0x00;
  return aRet;
}

char *bcvStrdup(const char *zIn){
  char *zNew = 0;
  if( zIn ){
    zNew = (char*)bcvMemdup(strlen(zIn), (const u8*)zIn);
  }
  return zNew;
}

i64 sqlite_timestamp(void){
  i64 iRet = 0;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  pVfs->xCurrentTimeInt64(pVfs, &iRet);
  return iRet;
}


int bcv_isdigit(char c){
  return c>='0' && c<='9';
}
int bcv_isspace(char c){
  return (c==' ' || c=='\n' || c=='\r' || c=='\t');
}

/* 
** Primitives to read and write 32-bit big-endian integers. 
*/
u32 bcvGetU16(const u8 *a){
  return ((u32)a[0] << 8) + ((u32)a[1] << 0);
}
void bcvPutU32(u8 *a, u32 iVal){
  a[0] = (iVal>>24) & 0xFF;
  a[1] = (iVal>>16) & 0xFF;
  a[2] = (iVal>> 8) & 0xFF;
  a[3] = (iVal>> 0) & 0xFF;
}
u32 bcvGetU32(const u8 *a){
  return ((u32)a[0] << 24) + ((u32)a[1] << 16) 
       + ((u32)a[2] << 8) + ((u32)a[3] << 0);
}
void bcvPutU64(u8 *a, u64 iVal){
  a[0] = (iVal>>56) & 0xFF;
  a[1] = (iVal>>48) & 0xFF;
  a[2] = (iVal>>40) & 0xFF;
  a[3] = (iVal>>32) & 0xFF;
  a[4] = (iVal>>24) & 0xFF;
  a[5] = (iVal>>16) & 0xFF;
  a[6] = (iVal>> 8) & 0xFF;
  a[7] = (iVal>> 0) & 0xFF;
}

u64 bcvGetU64(const u8 *a){
  return ((u64)a[0] << 56)
       + ((u64)a[1] << 48)
       + ((u64)a[2] << 40)
       + ((u64)a[3] << 32)
       + ((u64)a[4] << 24)
       + ((u64)a[5] << 16)
       + ((u64)a[6] <<  8)
       + ((u64)a[7] <<  0);
}

static void bcvApiErrorClear(sqlite3_bcv *p){
  sqlite3_free(p->zErrmsg);
  p->zErrmsg = 0;
  p->errCode = SQLITE_OK;
}

static int bcvApiError(sqlite3_bcv *p, int rc, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  bcvApiErrorClear(p);
  p->zErrmsg = sqlite3_vmprintf(zFmt, ap);
  p->errCode = rc;
  va_end(ap);
  return rc;
}

/*
** Return the index of the hash bucket of pHash that may contain the block
** with binary name aBlk[].
*/
static int bcvMHashBucket(ManifestHash *pHash, u8 *aBlk){
  return (bcvGetU32(&aBlk[4]) % pHash->nHash);
}

/*
** Add aBlk to the hash passed as the first argument.
*/
void bcvMHashAdd(ManifestHash *pHash, u8 *aBlk, int nBlk){
  int iBucket = bcvMHashBucket(pHash, aBlk);
  while( pHash->aHash[iBucket] ){
    if( 0==memcmp(pHash->aHash[iBucket], aBlk, nBlk) ) return;
    iBucket = (iBucket+1) % pHash->nHash;
  }
  pHash->aHash[iBucket] = aBlk;
}

/*
** Free hash table pHash.
*/
void bcvMHashFree(ManifestHash *pHash){
  sqlite3_free(pHash);
}

/*
** Query the hash table for an entry with nBlk byte prefix aBlk.
*/
u8 *bcvMHashQuery(ManifestHash *pHash, u8 *aBlk, int nBlk){
  u8 *aEntry;
  int iBucket = bcvMHashBucket(pHash, aBlk);
  while( (aEntry = pHash->aHash[iBucket]) ){
    if( memcmp(aBlk, aEntry, nBlk)==0 ){
      return aEntry;
    }
    iBucket = (iBucket+1) % pHash->nHash;
  }
  return 0;
}

/*
** Build a hash table containing all blocks from all databases except
** pExclude. Or, if pExclude is NULL, from all databases.
**
** If parameter bDeleteList is true, then the hash table also contains
** elements from the delete list.
*/
int bcvMHashBuild(
  Manifest *pMan, 
  int bDeleteList,
  ManifestDb *pExclude, 
  ManifestHash **ppHash
){
  int rc = SQLITE_OK;
  int nName = NAMEBYTES(pMan);
  int nGCName = GCENTRYBYTES(pMan);
  i64 nBlk = 0;                   /* Number of blocks in hash table */
  i64 nHash = 0;                  /* Number of buckets in hash table */
  i64 nAlloc;                     /* Bytes of space to allocate */
  ManifestHash *pHash = 0;
  int ii;

  /* Count the number of blocks in all databases and on the delete list */
  if( bDeleteList ){
    nBlk = pMan->nDelBlk;
  }
  for(ii=0; ii<pMan->nDb; ii++){
    ManifestDb *pDb = &pMan->aDb[ii];
    ManifestDb *pPar = 0;
    int iBlk;
    if( pDb==pExclude ) continue;
    if( pDb->iParentId!=0 ){
      pPar = bcvManifestDbidToDb(pMan, pDb->iParentId);
      if( pPar==pExclude ) pPar = 0;
    }
    for(iBlk=0; iBlk<pDb->nBlkOrig; iBlk++){
      if( !pPar 
       || iBlk>=pPar->nBlkOrig 
       || memcmp(&pPar->aBlkOrig[iBlk*nName], &pDb->aBlkOrig[iBlk*nName], nName)
      ){
        nBlk++;
      }
    }
  }

  /* Figure out how many hash buckets to use */
  nHash = 128;
  while( nHash<nBlk*8 ){
    nHash = nHash*2;
  }

  /* Allocate and populate the hash table */
  nAlloc = sizeof(ManifestHash) + nHash*sizeof(u8*);
  pHash = (ManifestHash*)bcvMallocRc(&rc, nAlloc);
  if( rc==SQLITE_OK ){
    int iBlk;
    pHash->nHash = nHash;
    pHash->aHash = (u8**)&pHash[1];
    for(ii=0; ii<pMan->nDb; ii++){
      ManifestDb *pDb = &pMan->aDb[ii];
      if( pDb==pExclude ) continue;
      for(iBlk=0; iBlk<pDb->nBlkOrig; iBlk++){
        u8 *pBlk = &pDb->aBlkOrig[iBlk*nName];
        bcvMHashAdd(pHash, pBlk, nName);
      }
    }
    if( bDeleteList ){
      for(iBlk=0; iBlk<pMan->nDelBlk; iBlk++){
        u8 *pBlk = &pMan->aDelBlk[iBlk*nGCName];
        bcvMHashAdd(pHash, pBlk, nName);
      }
    }
  }

  *ppHash = pHash;
  return rc;
}

ManifestDb *bcvManifestDbidToDb(Manifest *p, i64 iDbId){
  if( p->nDb>0 ){
    int i1 = 0;
    int i2 = p->nDb-1;

    while( i1<=i2 ){
      int iTest = (i1+i2+1) / 2;
      i64 iTestId = p->aDb[iTest].iDbId;
      if( iTestId==iDbId ){
        return &p->aDb[iTest];
      }else if( iTestId>iDbId ){
        i2 = iTest-1;
      }else{
        i1 = iTest+1;
      }
    }

#ifndef NDEBUG
    {
      int ii;
      for(ii=0; ii<p->nDb; ii++) assert( p->aDb[ii].iDbId!=iDbId);
    }
#endif
  }
  return 0;
}

/*
** MANIFEST FILE FORMAT VERSION 4
**
** The manifest file format is an ad-hoc binary design. It does not use
** XML or any other formatting convention. This is to keep it as compact 
** as possible.
**
** All "integer" values mentioned below are unsigned values stored in 
** big-endian format.
**
** Part 1: Manifest Header
**
**     Manifest format version:    32-bit integer. (currently set to 3).
**     Block size in bytes:        32-bit integer.
**     Number of databases:        32-bit integer.
**     Number of delete entries:   32-bit integer.
**     Name-size in bytes:         32-bit integer.  (between 12 and 32)
**     Largest db id assigned:     32-bit integer.
**     Then, for each database:
**         Database id:            32-bit integer.  (1 or greater)
**         Parent id:              32-bit integer.  (0 means no parent)
**         Database version:       32-bit integer.
**         Database offset:        32-bit integer.
**         Number of blocks in db: 32-bit integer.
**         Entries in block array: 32-bit integer.
**         Database display name:  128 byte utf-8. (zero padded)
**
** Part 2: Delete Block list.
**
**   An array of entries immediately following the last database header.
**   Each entry consists of:
**
**     Block id:                   name-size bytes.
**     Timestamp:                  64-bit integer.
**
**   Timestamps indicate when a block was moved to the delete-list.
**
** Part 3: Database Block lists.
**
**   There is a database block list for each database identified in the
**   manifest header. Each block list begins at the offset identified in
**   the manifest header and contains the number of entries also specified
**   in the header.
**
**   There are two formats for a block-list, depending on whether or not
**   the parent id in the header is set to 0 or not. If it is set to 0, then
**   the block-list contains tightly packed block-ids, one for each block
**   in the database. Or, if the parent id is non-zero, each entry consists
**   of:
**
**     Block offset:               32-bit integer.
**     Block id:                   name-size bytes.
**
*/
#define MANIFEST_HDR_SIZE   (6*sizeof(u32))
#define MANIFEST_DBHDR_SIZE (6*sizeof(u32)+128)

/*
** Parse the manifest in buffer a[], size n bytes, and return a Manifest
** structure with ref-count set to 1 reflecting the results.
*/
int bcvManifestParse(
  u8 *a, int n,                   /* Buffer to parse */
  char *zETag,                    /* etag value for new manifest */
  Manifest **ppOut,               /* OUT: new parsed manifest object */
  char **pzErr                    /* OUT: error message */
){
  Manifest *pRet = 0;
  int iDb;
  int rc = SQLITE_OK;

  /* Header fields */
  u32 iVersion;
  u32 nBlocksize;
  u32 nDb;
  u32 nDelete;
  u32 nNamesize;
  u32 iMaxDbId;

  iVersion = bcvGetU32(&a[0]);
  nBlocksize = bcvGetU32(&a[4]);
  nDb = bcvGetU32(&a[8]);
  nDelete = bcvGetU32(&a[12]);
  nNamesize = bcvGetU32(&a[16]);
  iMaxDbId = bcvGetU32(&a[20]);

  if( iVersion!=BCV_MANIFEST_VERSION ){
    *pzErr = bcvMprintf(
        "bad manifest version - expected %d, found %d.",
        BCV_MANIFEST_VERSION, iVersion
    );
    rc = SQLITE_ERROR;
  }

  /* Allocate Manifest object */
  if( rc==SQLITE_OK ){
    int nByte = sizeof(Manifest) + (nDb+1)*sizeof(ManifestDb);
    pRet = (Manifest*)bcvMallocRc(&rc, nByte);
    if( pRet ){
      pRet->nRef = 1;
      pRet->szBlk = nBlocksize;
      pRet->nNamebytes = nNamesize;
      pRet->iMaxDbId = iMaxDbId;
      pRet->zETag = zETag;
      pRet->pFree = a;
      pRet->aDb = (ManifestDb*)&pRet[1];
      pRet->aDelBlk = &a[MANIFEST_HDR_SIZE + MANIFEST_DBHDR_SIZE*nDb];
      pRet->nDelBlk = nDelete;
    }
  }

  for(iDb=0; rc==SQLITE_OK && iDb<nDb; iDb++){
    ManifestDb *pDb = &pRet->aDb[iDb];
    int iDbOff = MANIFEST_HDR_SIZE + MANIFEST_DBHDR_SIZE*iDb;
    int iOff;
    int nEntry;
    int bDel = 0;

    pDb->iDbId = bcvGetU32(&a[iDbOff+0]);
    pDb->iParentId = bcvGetU32(&a[iDbOff+4]);
    pDb->iVersion = bcvGetU32(&a[iDbOff+8]);
    iOff = bcvGetU32(&a[iDbOff+12]);
    pDb->nBlkOrig = bcvGetU32(&a[iDbOff+16]);
    if( pDb->nBlkOrig & 0x80000000 ){
      pDb->nBlkOrig &= 0x7FFFFFFF;
      bDel = 1;
    }
    nEntry = bcvGetU32(&a[iDbOff+20]);
    memcpy(pDb->zDName, &a[iDbOff+24], BCV_DBNAME_SIZE);

    if( pDb->nBlkOrig ){
      if( pDb->iParentId ){
        pDb->nBlkOrigAlloc = pDb->nBlkOrig;
        pDb->aBlkOrig = (u8*)bcvMallocRc(&rc, pDb->nBlkOrig*NAMEBYTES(pRet));
        if( pDb->aBlkOrig ){
          int ii;
          ManifestDb *pPar = bcvManifestDbidToDb(pRet, pDb->iParentId);
          if( pPar ){
            int nCopy = MIN(pPar->nBlkOrig, pDb->nBlkOrig) * nNamesize;
            memcpy(pDb->aBlkOrig, pPar->aBlkOrig, nCopy);
          }
          for(ii=0; ii<nEntry; ii++){
            u32 iBlk = bcvGetU32(&a[iOff]);
            memcpy(&pDb->aBlkOrig[nNamesize*iBlk], &a[iOff+4], nNamesize);
            iOff += 4 + nNamesize;
          }
        }
        assert( iDb!=nDb-1 || iOff==n );
      }else{
        pDb->aBlkOrig = &a[iOff];
      }
    }

    if( bDel==0 ){
      pDb->aBlkLocal = pDb->aBlkOrig;
      pDb->nBlkLocal = pDb->nBlkOrig;
    }
    pRet->nDb++;
  }

  if( pRet==0 ){
    sqlite3_free(a);
    sqlite3_free(zETag);
  }
  if( rc!=SQLITE_OK ){
    bcvManifestDeref(pRet);
    pRet = 0;
  }
  *ppOut = pRet;
  return rc;
}

/*
** Return the number of bytes allocated for the manifest object, not 
** including malloc() overhead.
*/
i64 bcvManifestSize(Manifest *pMan){
  int ii;
  i64 nRet = 0;

  nRet += sizeof(Manifest) + (pMan->nDb+1)*sizeof(ManifestDb);
  nRet += sqlite3_msize(pMan->pFree);
  if( pMan->bDelFree ){
    nRet += sqlite3_msize(pMan->aDelBlk);
  }
  for(ii=0; ii<pMan->nDb; ii++){
    ManifestDb *pManDb = &pMan->aDb[ii];
    if( pManDb->nBlkLocalAlloc ){
      nRet += sqlite3_msize(pManDb->aBlkLocal);
    }
    if( pManDb->nBlkOrigAlloc ){
      nRet += sqlite3_msize(pManDb->aBlkOrig);
    }
  }

  return nRet;
}

int bcvManifestParseCopy(
  const u8 *a, int n, 
  const char *zETag, 
  Manifest **ppOut, 
  char **pz
){
  int rc = SQLITE_OK;
  u8 *aCopy = bcvMallocRc(&rc, n);
  char *zECopy = bcvStrdupRc(&rc, zETag);
  if( rc!=SQLITE_OK ){
    sqlite3_free(aCopy);
    sqlite3_free(zECopy);
    *ppOut = 0;
    *pz = 0;
  }else{
    memcpy(aCopy, a, n);
    rc = bcvManifestParse(aCopy, n, zECopy, ppOut, pz);
  }
  return rc;
}

u8 *bcvManifestCompose(Manifest *p, int *pnOut){
  int rc = SQLITE_OK;             /* Return Code */
  int iDb;
  u8 *aOut = 0;
  int nOut = 0;
  int nAlloc = 0;

  /* Write the db header */
  nAlloc = 1024*1024;
  aOut = sqlite3_malloc(nAlloc);
  if( aOut==0 ){
    rc = SQLITE_NOMEM;
  }else{
    bcvPutU32(&aOut[0], 4);
    bcvPutU32(&aOut[4], p->szBlk);
    bcvPutU32(&aOut[8], p->nDb);
    bcvPutU32(&aOut[12], p->nDelBlk);
    bcvPutU32(&aOut[16], p->nNamebytes);
    bcvPutU32(&aOut[20], p->iMaxDbId);
  }

  /* Write the delete block array */
  if( rc==SQLITE_OK ){
    int iOff = MANIFEST_HDR_SIZE + p->nDb * MANIFEST_DBHDR_SIZE;
    int nReq = (p->nNamebytes + sizeof(u64))*p->nDelBlk;

    nOut = iOff + nReq;
    if( nOut>nAlloc ){
      u8 *aNew = 0;
      while( nOut>nAlloc ) nAlloc = nAlloc*2;
      aNew = sqlite3_realloc(aOut, nAlloc);
      if( aNew==0 ){
        rc = SQLITE_NOMEM;
      }else{
        aOut = aNew;
      }
    }
    if( rc==SQLITE_OK && nReq>0 ){
      memcpy(&aOut[iOff], p->aDelBlk, nReq);
    }
  }

  /* Write the db header and array of blocks for each database. */
  for(iDb=0; rc==SQLITE_OK && iDb<p->nDb; iDb++){
    int nEntry = 0;
    int iHdrOff = MANIFEST_HDR_SIZE + iDb*MANIFEST_DBHDR_SIZE;
    ManifestDb *pDb = &p->aDb[iDb];
    ManifestDb *pPar = 0;
    int nMax;
    int iDbOff = nOut;
    u32 nBlkOrig = pDb->nBlkOrig | (pDb->nBlkLocal==0 ? 0x80000000 : 0);

    if( pDb->iParentId ){
      pPar = bcvManifestDbidToDb(p, pDb->iParentId);
    }

    nMax = nOut + pDb->nBlkOrig * (NAMEBYTES(p) + (pPar ? 4 : 0));
    if( nMax>nAlloc ){
      u8 *aNew = 0;
      while( nMax>nAlloc ) nAlloc = nAlloc*2;
      aNew = sqlite3_realloc(aOut, nAlloc);
      if( aNew==0 ){
        rc = SQLITE_NOMEM;
        break;
      }else{
        aOut = aNew;
      }
    }

    if( pPar ){
      int iBlk;
      for(iBlk=0; iBlk<pDb->nBlkOrig; iBlk++){
        int iBOff = iBlk*NAMEBYTES(p);
        if( iBlk>=pPar->nBlkOrig 
         || memcmp(&pDb->aBlkOrig[iBOff], &pPar->aBlkOrig[iBOff], NAMEBYTES(p))
        ){
          bcvPutU32(&aOut[nOut], iBlk);
          memcpy(&aOut[nOut+4], &pDb->aBlkOrig[iBOff], NAMEBYTES(p));
          nEntry++;
          nOut += 4 + NAMEBYTES(p);
        }
      }
    }else{
      int nByte = NAMEBYTES(p) * pDb->nBlkOrig;
      memcpy(&aOut[nOut], pDb->aBlkOrig, nByte);
      nOut += nByte;
    }

    bcvPutU32(&aOut[iHdrOff+0], pDb->iDbId);
    bcvPutU32(&aOut[iHdrOff+4], (pPar ? pDb->iParentId : 0));
    bcvPutU32(&aOut[iHdrOff+8], pDb->iVersion);
    bcvPutU32(&aOut[iHdrOff+12], iDbOff);
    bcvPutU32(&aOut[iHdrOff+16], nBlkOrig);
    bcvPutU32(&aOut[iHdrOff+20], nEntry);
    memset(&aOut[iHdrOff+24], 0, 128);
    memcpy(&aOut[iHdrOff+24], pDb->zDName, strlen(pDb->zDName));
  }

  if( rc!=SQLITE_OK ){
    sqlite3_free(aOut);
    nOut = 0;
    aOut = 0;
  }
  *pnOut = nOut;
  return aOut;
}

void bcvManifestFree(Manifest *p){
  if( p ){
    int i;
    for(i=0; i<p->nDb; i++){
      ManifestDb *pDb = &p->aDb[i];
      if( pDb->nBlkOrigAlloc ) sqlite3_free(pDb->aBlkOrig);
      if( pDb->nBlkLocalAlloc ) sqlite3_free(pDb->aBlkLocal);
    }
    if( p->bDelFree ){
      sqlite3_free(p->aDelBlk);
    }
    sqlite3_free(p->pFree);
    sqlite3_free(p->zETag);
    sqlite3_free(p);
  }
}

void bcvManifestDeref(Manifest *p){
  if( p ){
    p->nRef--;
    assert( p->nRef>=0 );
    if( p->nRef==0 ){
      bcvManifestFree(p);
    }
  }
}

Manifest *bcvManifestRef(Manifest *p){
  p->nRef++;
  return p;
}

/*
** This function is a no-op if (*pRc) is not SQLITE_OK when it is called.
** Otherwise, it reallocates the buffer used by (*pMan) so that 
** (*pMan)->aDb is large enough for nExtra more entries. If successful,
** (*pMan) points to the new manifest object after this function returns.
** Or, if the reallocation fails, then (*pMan) is left unchanged and (*pRc)
** set to SQLITE_NOMEM.
*/
void bcvManifestExpand(int *pRc, Manifest **ppMan, int nExtra){
  Manifest *pMan = *ppMan;
  Manifest *pNew = 0;
  int nByte = sizeof(Manifest) + (pMan->nDb * sizeof(ManifestDb));

  assert( pMan->aDb==(ManifestDb*)&pMan[1] );
  pNew = (Manifest*)bcvMallocRc(pRc, nByte + nExtra*sizeof(ManifestDb));
  if( pNew ){
    memcpy(pNew, pMan, nByte);
    sqlite3_free(pMan);
    *ppMan = pNew;
    pNew->aDb = (ManifestDb*)&pNew[1];
  }
}

/*
** Make a copy of the Manifest object passed as the only argument.
*/
int bcvManifestDup(Manifest *p, Manifest **ppNew){
  Manifest *pNew = 0;
  int nByte;
  int rc = SQLITE_OK;

  nByte = sizeof(Manifest) + (p->nDb+1) * sizeof(ManifestDb);
  pNew = (Manifest*)bcvMallocRc(&rc, nByte);
  if( pNew ){
    int i;
    pNew->nDb = p->nDb;
    pNew->szBlk = p->szBlk;
    pNew->nNamebytes = p->nNamebytes;
    pNew->iMaxDbId = p->iMaxDbId;
    pNew->aDb = (ManifestDb*)&pNew[1];
    for(i=0; rc==SQLITE_OK && i<pNew->nDb; i++){
      ManifestDb *pNewDb = &pNew->aDb[i];
      ManifestDb *pOldDb = &p->aDb[i];
  
      pNewDb->iDbId = pOldDb->iDbId;
      pNewDb->iParentId = pOldDb->iParentId;
      memcpy(pNewDb->zDName, pOldDb->zDName, BCV_DBNAME_SIZE);
  
      assert( pOldDb->nBlkLocalAlloc>0 
           || pOldDb->aBlkLocal==0
           || pOldDb->aBlkLocal==pOldDb->aBlkOrig
      );
      pNewDb->nBlkOrig = pOldDb->nBlkOrig;
      if( pOldDb->nBlkOrig>0 ){
        int nByte = pOldDb->nBlkOrig*NAMEBYTES(p);
        pNewDb->aBlkOrig = (u8*)bcvMallocRc(&rc, nByte);
        if( rc==SQLITE_OK ){
          memcpy(pNewDb->aBlkOrig, pOldDb->aBlkOrig, nByte);
          pNewDb->nBlkOrigAlloc = pNewDb->nBlkOrig = pOldDb->nBlkOrig;
        }
      }
      if( pOldDb->nBlkLocalAlloc ){
        int nByte = pOldDb->nBlkLocal*NAMEBYTES(p);
        pNewDb->aBlkLocal = (u8*)bcvMallocRc(&rc, nByte);
        if( rc==SQLITE_OK ){
          memcpy(pNewDb->aBlkLocal, pOldDb->aBlkLocal, nByte);
          pNewDb->nBlkLocal = pNewDb->nBlkLocalAlloc = pOldDb->nBlkLocal;
        }
      }else if( pOldDb->nBlkLocal>0 ){
        pNewDb->nBlkLocal = pNewDb->nBlkOrig;
        pNewDb->aBlkLocal = pNewDb->aBlkOrig;
      }
      pNewDb->iVersion = pOldDb->iVersion;
    }
  
    if( p->nDelBlk ){
      nByte = p->nDelBlk * GCENTRYBYTES(p);
      pNew->aDelBlk = (u8*)bcvMallocRc(&rc, nByte);
      if( rc==SQLITE_OK ){
        memcpy(pNew->aDelBlk, p->aDelBlk, nByte);
        pNew->nDelBlk = p->nDelBlk;
        pNew->bDelFree = 1;
      }
    }
  
    pNew->zETag = bcvStrdupRc(&rc, p->zETag);
    pNew->nRef = 1;
    if( rc!=SQLITE_OK ){
      bcvManifestFree(pNew);
      pNew = 0;
    }
  }

  *ppNew = pNew;
  return rc;
}

typedef struct BcvFetchParsed BcvFetchParsed;
struct BcvFetchParsed {
  BcvBuffer buf;
  char *zETag;
  int rc;
};

static void xFetchParsed(
  void *pApp, 
  int rc, 
  char *zETag, 
  const u8 *aData, 
  int nData,
  const u8 *aHdrs, int nHdrs
){
  BcvFetchParsed *p = (BcvFetchParsed*)pApp;
  if( rc!=SQLITE_OK ){
    assert( aData==0 && nData==0 );
    p->rc = rc;
    p->zETag = bcvMprintf("download manifest failed (%d) - %s", rc, zETag);
  }else if( nData>0 ){
    bcvBufferAppendRc(&p->rc, &p->buf, aData, nData);
    p->zETag = bcvStrdupRc(&p->rc, zETag);
  }
}

static int bcvManifestFetchParsed(sqlite3_bcv *pBcv, Manifest **ppOut){
  BcvContainer *p = pBcv->pCont;
  int rc = SQLITE_OK;
  BcvFetchParsed dl;

  memset(&dl, 0, sizeof(dl));
  rc = bcvDispatchFetch(pBcv->pDisp, p, BCV_MANIFEST_FILE,0,0,&dl,xFetchParsed);
  if( rc==SQLITE_OK ){
    rc = bcvDispatchRunAll(pBcv->pDisp);
  }

  if( rc==SQLITE_OK && dl.rc!=SQLITE_OK ){
    rc = dl.rc;
    pBcv->zErrmsg = dl.zETag;
    dl.zETag = 0;
  }
  if( rc==SQLITE_OK ){
    rc = bcvManifestParse(
        dl.buf.aData, dl.buf.nData, dl.zETag, ppOut, &pBcv->zErrmsg
    );
    dl.zETag = 0;
    dl.buf.aData = 0;
  }
  pBcv->errCode = rc;
  sqlite3_free(dl.buf.aData);
  sqlite3_free(dl.zETag);
  return rc;
}

static void bcvCollectError(void *pApp, int rc, char *zETag){
  sqlite3_bcv *pBcv = (sqlite3_bcv*)pApp;
  if( rc!=SQLITE_OK ){
    pBcv->errCode = rc;
    pBcv->zErrmsg = bcvStrdup(zETag);
  }
}

typedef struct ManifestCtx ManifestCtx;
struct ManifestCtx {
  sqlite3_bcv *p;
  Manifest *pMan;
};

static void bcvManifestUploadParsedCb(
  void *pApp, 
  int rc, 
  char *zErr 
){
  ManifestCtx *pCtx = (ManifestCtx*)pApp;
  if( rc==SQLITE_OK && pCtx->pMan ){
    sqlite3_free(pCtx->pMan->zETag);
    pCtx->pMan->zETag = bcvStrdupRc(&rc, zErr);
  }
  if( rc!=SQLITE_OK ){
    bcvApiError(pCtx->p, rc, "upload manifest failed (%d) - %s", rc, zErr);
  }
}


int bcvManifestUploadParsed(sqlite3_bcv *p, Manifest *pMan){
  ManifestCtx ctx;
  u8 *aMan = 0;
  int nMan = 0;

  ctx.p = p;
  ctx.pMan = pMan->zETag ? pMan : 0;

  assert( p->errCode==SQLITE_OK );
  assert( pMan->nDb==0 || pMan->zETag );
#ifndef NDEBUG
  {
    ManifestDb *pDb;
    for(pDb=pMan->aDb; pDb<&pMan->aDb[pMan->nDb]; pDb++){
      assert( pDb->iParentId==0 || pDb->iParentId<pDb->iDbId );
      assert( pDb==pMan->aDb || pDb[0].iDbId>pDb[-1].iDbId );
    }
  }
#endif

  aMan = bcvManifestCompose(pMan, &nMan);
  if( aMan==0 ){
    p->errCode = SQLITE_NOMEM;
  }else{
    int rc = bcvDispatchPut(p->pDisp, 
        p->pCont, BCV_MANIFEST_FILE, pMan->zETag, aMan, nMan, 
        (void*)&ctx, bcvManifestUploadParsedCb
    );
    if( rc==SQLITE_OK ){
      rc = bcvDispatchRunAll(p->pDisp);
      if( p->errCode ) rc = p->errCode;
    }
    p->errCode = rc;
  }
  sqlite3_free(aMan);

  return p->errCode;
}

static int bcvManifestNameToIndex(Manifest *p, const char *zDb){
  int i;
  for(i=0; i<p->nDb; i++){
    if( 0==bcvStrcmp(zDb, p->aDb[i].zDName) ){
      break;
    }
  }
  return (i<p->nDb ? i : -1);
}

/*
** The second argument points to a nName byte buffer
** containing a block-id. Format the block-id as text and write it to
** buffer aBuf[], which must be at least BCV_FILESYSTEM_BLOCKID_BYTES
** in size.
*/
void bcvBlockidToText(int nName, const u8 *pBlk, char *aBuf){
  hex_encode(pBlk, nName, aBuf, 1);
  memcpy(&aBuf[nName*2], ".bcv", 5);
}

static sqlite3_bcv sqlite3_bcv_oom_handle = {
  0, 0, "out of memory", SQLITE_NOMEM, 0, 0, 0
};

int sqlite3_bcv_open(
  const char *zMod,
  const char *zUser,              /* Cloud storage user name */
  const char *zKey,               /* Key or SAS used for cloud storage auth. */
  const char *zCont,              /* Cloud storage container/bucket */
  sqlite3_bcv **ppOut             /* OUT: New object */
){
  sqlite3_bcv *pNew = 0;
  int rc = SQLITE_OK;

  *ppOut = 0;
  if( zCont==0 ) return SQLITE_MISUSE;
  pNew = bcvMallocRc(&rc, sizeof(sqlite3_bcv));
  if( pNew ){
    pNew->nRequest = 1;
    pNew->nHttpTimeout = BCV_DEFAULT_HTTPTIMEOUT;
    pNew->bFindOrphans = BCV_DEFAULT_FINDORPHANS;
    pNew->errCode = bcvContainerOpen(
        zMod, zUser, zKey, zCont, &pNew->pCont, &pNew->zErrmsg
    );
    if( pNew->errCode==SQLITE_OK ){
      pNew->errCode = bcvDispatchNew(&pNew->pDisp);
      if( pNew->pDisp ) bcvDispatchTimeout(pNew->pDisp, pNew->nHttpTimeout);
    }
    if( pNew->errCode ){
      bcvContainerClose(pNew->pCont);
      pNew->pCont = 0;
    }
    rc = pNew->errCode;
  }else{
    pNew = &sqlite3_bcv_oom_handle;
  }
  *ppOut = pNew;
  return rc;
}

void sqlite3_bcv_close(sqlite3_bcv *p){
  if( p && p!=&sqlite3_bcv_oom_handle ){
    bcvContainerClose(p->pCont);
    bcvDispatchFree(p->pDisp);
    bcvLogDelete(p->pBcvLogObj);
    sqlite3_free(p->zErrmsg);
    sqlite3_free(p);
  }
}

int sqlite3_bcv_errcode(sqlite3_bcv *p){
  return p->errCode;
}

const char *sqlite3_bcv_errmsg(sqlite3_bcv *p){
  const char *zErr = "out of memory";
  if( p ) zErr = p->zErrmsg;
  return zErr;
}

static void bcvLogWrapper(void *pApp, int bRetry, const char *zMsg){
  sqlite3_bcv *p = (sqlite3_bcv*)pApp;
  if( p->xBcvLog ){
    p->xBcvLog(p->pBcvLogCtx, zMsg);
  }
}

int sqlite3_bcv_config(sqlite3_bcv *p, int eOp, ...){
  int rc = SQLITE_OK;
  va_list ap;
  va_start(ap, eOp);

  if( p->pCont==0 ){
    rc = p->errCode;
  }else{
    switch( eOp ){
      case SQLITE_BCVCONFIG_VERBOSE:
        p->bVerbose = va_arg(ap, int);
        bcvDispatchVerbose(p->pDisp, p->bVerbose);
        break;
      case SQLITE_BCVCONFIG_PROGRESS:
        p->pProgressCtx = va_arg(ap, void*);
        p->xProgress = va_arg(ap, bcv_progress_cb);
        break;
      case SQLITE_BCVCONFIG_LOG: {
        p->pBcvLogCtx = va_arg(ap, void*);
        p->xBcvLog = (bcv_log_cb)va_arg(ap, bcv_log_cb);
        if( p->pBcvLogObj==0 ){
          p->pBcvLogObj = bcvLogNew();
          if( p->pBcvLogObj==0 ) rc = SQLITE_NOMEM;
        }
        if( p->xBcvLog ){
          bcvDispatchLog(p->pDisp, (void*)p, bcvLogWrapper);
          bcvDispatchLogObj(p->pDisp, p->pBcvLogObj);
        }else{
          bcvDispatchLog(p->pDisp, 0, 0);
        }
        break;
      }
      case SQLITE_BCVCONFIG_NREQUEST: {
        int nReq = va_arg(ap, int);
        if( nReq<1 ) nReq = 1;
        if( nReq>BCV_MAX_NREQUEST ) nReq = BCV_MAX_NREQUEST;
        p->nRequest = nReq;
        break;
      }
      case SQLITE_BCVCONFIG_LOGLEVEL: {
        int nLogLevel = va_arg(ap, int);
        if( nLogLevel<0 ) nLogLevel = 0;
        p->nLogLevel = nLogLevel;
        break;
      }
      case SQLITE_BCVCONFIG_TESTNOKV: {
        p->bTestNoKv = va_arg(ap, int);
        break;
      }
      case SQLITE_BCVCONFIG_HTTPTIMEOUT:
        p->nHttpTimeout = va_arg(ap, int);
        bcvDispatchTimeout(p->pDisp, p->nHttpTimeout);
        break;
      case SQLITE_BCVCONFIG_FINDORPHANS: {
        p->bFindOrphans = va_arg(ap, int);
        break;
      }
      default:
        rc = SQLITE_MISUSE;
        break;
    }
  }

  va_end(ap);
  return rc;
}

typedef struct BcvUpload BcvUpload;
typedef struct BcvUploadJob BcvUploadJob;

struct BcvUpload {
  sqlite3_bcv *pBcv;              /* BCV handle */
  int nBlock;                     /* Total number of blocks to upload */
  int nDone;                      /* Total blocks finished */
  int iNextBlock;                 /* Next block to upload */
  sqlite3_file *pFd;              /* File descriptor open on file to upload */
  i64 szFile;                     /* Size of local file in bytes */
  Manifest *pMan;                 /* Manifest to update */
  ManifestDb *pDb;                /* Manifest db being uploaded */
  ManifestHash *pMHash;           /* Hash of existing blocks (or NULL) */
};

struct BcvUploadJob {
  BcvUpload *pUpload;             /* Upload context */
  u8 *aSpace;                     /* szBlock bytes of space for this job */
};

static void bcvUploadOneBlock(BcvUploadJob *pJob);
static void bcvUploadOneBlockCb(void *pApp, int rc, char *zErr){
  BcvUploadJob *pJob = (BcvUploadJob*)pApp;
  BcvUpload *p = pJob->pUpload;
  sqlite3_bcv *pBcv = p->pBcv;
  if( pBcv->errCode==SQLITE_OK ){
    if( rc!=SQLITE_OK ){
      bcvApiError(pBcv, rc, "%s", zErr);
    }else{
      p->nDone++;
      if( pBcv->xProgress ){
        i64 nByte = (i64)p->nDone * p->pMan->szBlk;
        if( nByte>p->szFile ) nByte = p->szFile;
        if( pBcv->xProgress(pBcv->pProgressCtx, nByte, p->szFile) ){
          bcvApiError(pBcv,SQLITE_ABORT,"abort requested by progress callback");
        }
      }
      bcvUploadOneBlock(pJob);
    }
  }
}

static void bcvUploadOneBlockRetry(BcvUploadJob *pJob, int *pbRetry){
  BcvUpload *p = pJob->pUpload;
  sqlite3_bcv *pBcv = p->pBcv;
  *pbRetry = 0;
  if( pBcv->errCode==SQLITE_OK ){
    u8 *aSpace = pJob->aSpace;
    Manifest *pMan = p->pMan;
    ManifestDb *pDb = p->pDb;
    char aBuf[BCV_MAX_FSNAMEBYTES];    /* Name of block file to upload */
    int nByte;                         /* Bytes to read from file */
    int rc;
    u8 *aBlk;
    i64 iBlk = (i64)(p->iNextBlock++);
    i64 iReadOff;                      /* Offset to read block from */

    /* If all blocks have either been uploaded or are being uploaded. */
    if( iBlk>=p->nBlock ) return;

    aBlk = &pDb->aBlkLocal[iBlk * NAMEBYTES(pMan)];

    /* Read the block of data from the file into a buffer */ 
    nByte = pMan->szBlk;
    iReadOff = iBlk*pMan->szBlk;
    if( iReadOff+nByte>p->szFile ){
      nByte = p->szFile - iReadOff;
    }
    rc = bcvReadfile(p->pFd, aSpace, nByte, iReadOff);
    if( rc!=SQLITE_OK ){
      bcvApiError(pBcv, (rc & 0xFF), 
          "failed to read %d bytes from offset %lld of file", nByte, iReadOff
      );
      return;
    }
    if( nByte<pMan->szBlk ){
      memset(&aSpace[nByte], 0, pMan->szBlk-nByte);
    }

    /* Check if an existing block can be used. */
    if( NAMEBYTES(pMan)>=BCV_MIN_MD5_NAMEBYTES ){
      u8 *pMatch;
      assert( MD5_DIGEST_LENGTH==16 );
      MD5(aSpace, pMan->szBlk, aBlk);
      pMatch = bcvMHashQuery(p->pMHash, aBlk, MD5_DIGEST_LENGTH);
      if( pMatch ){
        memcpy(aBlk, pMatch, NAMEBYTES(pMan));
        *pbRetry = 1;
        return;
      }else{
        sqlite3_randomness(
            NAMEBYTES(pMan) - MD5_DIGEST_LENGTH, &aBlk[MD5_DIGEST_LENGTH]
        );
      }
    }else{
      sqlite3_randomness(NAMEBYTES(pMan), aBlk);
    }

    bcvBlockidToText(NAMEBYTES(pMan), aBlk, aBuf);
    rc = bcvDispatchPut(
        pBcv->pDisp, pBcv->pCont, aBuf, 0, aSpace, pMan->szBlk, 
        pJob, bcvUploadOneBlockCb
    );
    if( rc!=SQLITE_OK ){
      pBcv->errCode = rc;
      return;
    }
  }
}

static void bcvUploadOneBlock(BcvUploadJob *pJob){
  int bRetry;
  do {
    bRetry = 0;
    bcvUploadOneBlockRetry(pJob, &bRetry);
  } while( bRetry );
}

/*
** Buffer zRemote (nRemote bytes in size) contains a proposed name for a new
** cloud database. This function checks that the database name is acceptable.
** If so, SQLITE_OK is returned. Otherwise, an SQLite error code is returned
** and error message left in bcv handle p.
*/
static int bcvCheckDbname(sqlite3_bcv *p, const char *zRemote){
  int nRemote = bcvStrlen(zRemote);
  if( nRemote>=BCV_DBNAME_SIZE ){
    return bcvApiError(p, SQLITE_ERROR, 
        "database name \"%s\" is too long (max = %d bytes)",
        zRemote, BCV_DBNAME_SIZE-1
    );
  }
  return SQLITE_OK;
}

/*
** Upload a database to cloud storage.
*/
int sqlite3_bcv_upload(
  sqlite3_bcv *p,                 /* BCV connection */
  const char *zLocal,             /* Local file-system entry to upload */
  const char *zRemote             /* Remote name to upload to */
){
  int i;                          /* Iterator */
  Manifest *pMan = 0;             /* Manifest being updated */
  ManifestDb *pDb = 0;            /* New database in manifest file */
  i64 iDbId;                      /* New database id */
  int nRemote;                    /* strlen(zRemote) */
  BcvUpload up;                   /* Context object passed to callbacks */
  int nJob = p->nRequest;         /* Number of concurrent block uploads */
  BcvUploadJob *aJob = 0;         /* Array of concurrent block uploads */
  int rc = SQLITE_OK;

  /* If sqlite3_bcv_open() failed */
  if( p->pCont==0 ) return p->errCode;

  memset(&up, 0, sizeof(up));
  up.pBcv = p;
  bcvApiErrorClear(p);

  /* Check that the database name is not too long for the manifest format */
  if( bcvCheckDbname(p, zRemote)!=SQLITE_OK ){
    return p->errCode;
  }
  nRemote = bcvStrlen(zRemote);

  /* Download the manifest file. */
  if( bcvManifestFetchParsed(p, &pMan) ){
    return p->errCode;
  }
  up.pMan = pMan;

  /* Check that the db name is not already in the manifest. */
  i = bcvManifestNameToIndex(pMan, zRemote);
  if( i>=0 ){
    bcvApiError(p, SQLITE_ERROR, "duplicate database name - \"%s\"", zRemote);
    goto update_out;
  }

  /* Open the database file to be uploaded. And read its size in bytes. */
  if( bcvOpenLocal(zLocal, 0, 1, &up.pFd) ){
    bcvApiError(p, SQLITE_IOERR, 
        "failed to open file \"%s\" for reading", zLocal
    );
    goto update_out;
  }
  if( SQLITE_OK!=up.pFd->pMethods->xFileSize(up.pFd, &up.szFile) ){
    bcvApiError(p, SQLITE_IOERR, 
        "failed to read size of file \"%s\" - %s\n", zLocal
    );
    goto update_out;
  }
  up.nBlock = ((u64)up.szFile + pMan->szBlk-1) / pMan->szBlk;

  /* Extend manifest to include new database */
  iDbId = ++pMan->iMaxDbId;
  up.pDb = pDb = &pMan->aDb[pMan->nDb];
  pMan->nDb++;
  pDb->iVersion = 1;
  memset(pDb->zDName, 0, BCV_DBNAME_SIZE);
  pDb->iDbId = iDbId;
  memcpy(pDb->zDName, zRemote, nRemote);
  if( up.nBlock>0 ){
    assert( pDb->nBlkLocalAlloc==0 );
    pDb->nBlkLocal = pDb->nBlkLocalAlloc = up.nBlock;
    pDb->aBlkLocal = (u8*)bcvMalloc(up.nBlock * NAMEBYTES(pMan));
  }
  if( NAMEBYTES(pMan)>=BCV_MIN_MD5_NAMEBYTES ){
    rc = bcvMHashBuild(pMan, 0, 0, &up.pMHash);
  }

  if( rc==SQLITE_OK ){
    int nByte = (sizeof(BcvUploadJob)+pMan->szBlk) *nJob;
    aJob = (BcvUploadJob*)bcvMallocZero(nByte);
    for(i=0; i<nJob; i++){
      aJob[i].pUpload = &up;
      aJob[i].aSpace = &((u8*)&aJob[nJob])[pMan->szBlk*i];
      bcvUploadOneBlock(&aJob[i]);
    }

    rc = bcvDispatchRunAll(p->pDisp);
  }
  if( rc!=SQLITE_OK ) p->errCode = rc;
  if( p->errCode==SQLITE_OK ){
    assert( pDb->nBlkOrigAlloc==0 );
    pDb->aBlkOrig = pDb->aBlkLocal;
    pDb->nBlkOrig = pDb->nBlkLocal;
    pDb->nBlkOrigAlloc = pDb->nBlkLocalAlloc;
    pDb->nBlkLocalAlloc = 0;
    bcvManifestUploadParsed(p, pMan);
  }

 update_out:
  sqlite3_free(aJob);
  bcvCloseLocal(up.pFd);
  bcvManifestFree(pMan);
  bcvMHashFree(up.pMHash);
  return p->errCode;
}

/*
** This function is called when deleting a database from a manifest file, 
** either via sqlite3_bcv_delete() or sqlite3_bcvfs_delete(). It adds
** all blocks from database iDb to the delete-list of the manifest, with
** the current time as the timestamp.
**
** SQLITE_OK is returned if successful, otherwise an SQLite error code.
*/
int bcvDeleteBlocks(Manifest *pMan, int iDb){
  ManifestDb *pDb = &pMan->aDb[iDb];
  u8 *aGC = 0;
  i64 iTime = 0;
  u8 aTime[8];
  int i;
  int iGC = 0;
  ManifestHash *pMH = 0;
  const int nName = NAMEBYTES(pMan);
  int rc = SQLITE_OK;

  iTime = sqlite_timestamp();
  bcvPutU64(aTime, iTime);

  assert( pDb->nBlkOrig==pDb->nBlkLocal );
  rc = bcvMHashBuild(pMan, 0, pDb, &pMH);
  aGC = (u8*)bcvMallocRc(&rc, (pMan->nDelBlk+pDb->nBlkOrig)*GCENTRYBYTES(pMan));
  if( rc==SQLITE_OK ){

    if( pMan->nDelBlk ){
      memcpy(aGC, pMan->aDelBlk, pMan->nDelBlk * GCENTRYBYTES(pMan));
      if( pMan->bDelFree ){
        sqlite3_free(pMan->aDelBlk);
        pMan->aDelBlk = 0;
      }
    }

    for(i=0; i<pDb->nBlkOrig; i++){
      u8 *pDel = &pDb->aBlkOrig[i*nName];
      if( 0==bcvMHashQuery(pMH, pDel, nName) ){
        assert( GCENTRYBYTES(pMan)==nName+8 );
        memcpy(&aGC[(pMan->nDelBlk+iGC)*GCENTRYBYTES(pMan)], pDel, nName);
        memcpy(&aGC[(pMan->nDelBlk+iGC)*GCENTRYBYTES(pMan)+nName], aTime, 8);
        iGC++;
      }
    }
    pMan->nDelBlk += iGC;
    pMan->aDelBlk = aGC;
    pMan->bDelFree = 1;
  }

  bcvMHashFree(pMH);
  return rc;
}

/*
** Create a copy of an existing cloud storage database within its container.
*/
int sqlite3_bcv_copy(
  sqlite3_bcv *p,
  const char *zFrom,
  const char *zTo
){
  int iTo;
  Manifest *pMan = 0;

  /* If sqlite3_bcv_open() failed */
  if( p->pCont==0 ) return p->errCode;

  bcvApiErrorClear(p);
  if( bcvCheckDbname(p, zTo) ){
    return p->errCode;
  }

  /* Download the manifest file. */
  if( bcvManifestFetchParsed(p, &pMan) ){
    return p->errCode;
  }

  iTo = bcvManifestNameToIndex(pMan, zTo);
  if( iTo>=0 ){
    bcvApiError(p, SQLITE_ERROR, "database %s already exists", zTo);
  }else{
    /* Check that the from-db name is in the manifest. */
    int iFrom = bcvManifestNameToIndex(pMan, zFrom);
    if( iFrom<0 ){
      bcvApiError(p, SQLITE_ERROR, "no such database: %s", zFrom);
    }else{
      ManifestDb *pFrom = &pMan->aDb[iFrom];
      ManifestDb *pTo = &pMan->aDb[pMan->nDb];

      pMan->nDb++;
      memset(pTo, 0, sizeof(ManifestDb));
      pTo->iDbId = ++pMan->iMaxDbId;
      memcpy(pTo->zDName, zTo, bcvStrlen(zTo));

      pTo->nBlkOrigAlloc = pTo->nBlkLocal = pTo->nBlkOrig = pFrom->nBlkOrig;
      pTo->aBlkOrig = (u8*)bcvMallocZero(pTo->nBlkOrig*NAMEBYTES(pMan));
      pTo->aBlkLocal = pTo->aBlkOrig;
      memcpy(pTo->aBlkOrig, pFrom->aBlkOrig, pTo->nBlkOrig*NAMEBYTES(pMan));
      pTo->iVersion++;
      pTo->iParentId = pFrom->iDbId;
      bcvManifestUploadParsed(p, pMan);
    }
  }

  bcvManifestFree(pMan);
  return p->errCode;
}

typedef struct BcvDownload BcvDownload;
typedef struct BcvDownloadJob BcvDownloadJob;

struct BcvDownload {
  sqlite3_bcv *pBcv;              /* BCV handle */
  sqlite3_file *pFd;              /* File descriptor open on file to write */
  i64 szInit;                     /* Size of file pFd when first opened */
  int iNextBlock;                 /* Next block to upload */
  int nDone;                      /* Total blocks finished */
  u8 *aSpace;                     /* Buffer large enough for one block */
  Manifest *pMan;
  ManifestDb *pDb;                /* Manifest db being downloaded */
};

struct BcvDownloadJob {
  BcvDownload *pDownload;         /* Download context */
  int iBlk;                       /* Block this job is currently downloading */
};

int bcvWritefile(
  sqlite3_file *pFd, 
  const u8 *aData, 
  int nData, 
  i64 iOff
){
  const int nMax = 32768;
  int i = 0;
  for(i=0; i<nData; i+=nMax){
    int nByte = nMax>(nData-i) ? (nData-i) : nMax;
    int rc = pFd->pMethods->xWrite(pFd, &aData[i], nByte, iOff+i);
    if( rc ) return rc;
  }
  return SQLITE_OK;
}

static void bcvDownloadProgress(BcvDownload *p){
  sqlite3_bcv *pBcv = p->pBcv;
  p->nDone++;
  if( pBcv->xProgress ){
    i64 nByte = (i64)p->nDone * p->pMan->szBlk;
    i64 nTotal = (i64)p->pDb->nBlkLocal * p->pMan->szBlk;
    if( pBcv->xProgress(pBcv->pProgressCtx, nByte, nTotal) ){
      bcvApiError(pBcv, 
          SQLITE_ABORT, "abort requested by progress callback"
      );
    }
  }
}

static void bcvDownloadOneFile(BcvDownloadJob *);
static void bcvDownloadOneFileCb(
  void *pApp, 
  int errCode, 
  char *zETag, 
  const u8 *aData,
  int nData,
  const u8 *aHdrs, int nHdrs
){
  BcvDownloadJob *pJob = (BcvDownloadJob*)pApp;
  BcvDownload *p = pJob->pDownload;
  sqlite3_bcv *pBcv = p->pBcv;

  if( pBcv->errCode==SQLITE_OK ){
    i64 iOff = (i64)pJob->iBlk*p->pMan->szBlk;
    if( errCode==HTTP_PRECONDITION_FAILED ){
      assert( p->szInit>=iOff+nData );
      bcvDownloadProgress(p);
      bcvDownloadOneFile(pJob);
    }else if( errCode!=SQLITE_OK ){
      bcvApiError(pBcv, errCode, "%s", zETag);
    }else{
      int rc = bcvWritefile(p->pFd, aData, nData, iOff);
      if( rc!=SQLITE_OK ){
        bcvApiError(pBcv, rc&0xFF, "");
      }else{
        bcvDownloadProgress(p);
        bcvDownloadOneFile(pJob);
      }
    }
  }
}

static void bcvDownloadOneFile(BcvDownloadJob *pJob){
  BcvDownload *p = pJob->pDownload;
  sqlite3_bcv *pBcv = p->pBcv;
  Manifest *pMan = p->pMan;          /* Manifest to download from */
  int szBlk = pMan->szBlk;
  const u8 *aBlk = 0;                /* Block id to download */
  i64 iBlk;                          /* Index of block within db file */
  int rc;                            /* Return Code */
  u8 aMd5[MD5_DIGEST_LENGTH];
  const void *pMd5 = 0;

  while( pBcv->errCode==SQLITE_OK ){
    iBlk = p->iNextBlock++;
    if( iBlk>=p->pDb->nBlkLocal ) return;
    aBlk = &p->pDb->aBlkLocal[NAMEBYTES(pMan)*iBlk];

    if( (iBlk+1)*szBlk<=p->szInit ){
      if( p->aSpace==0 ){
        p->aSpace = (u8*)bcvMalloc(szBlk);
        if( p->aSpace==0 ){
          pBcv->errCode = SQLITE_NOMEM;
          break;
        }
      }

      rc = bcvReadfile(p->pFd, p->aSpace, szBlk, iBlk*szBlk);
      if( rc!=SQLITE_OK ){
        bcvApiError(pBcv, (rc & 0xFF), 
            "failed to read %d bytes from offset %lld of file", 
            szBlk, iBlk*szBlk
        );
        break;
      }

      MD5(p->aSpace, szBlk, aMd5);
      if( NAMEBYTES(pMan)>=BCV_MIN_MD5_NAMEBYTES ){
        if( 0==memcmp(aMd5, aBlk, MD5_DIGEST_LENGTH) ){
          bcvDownloadProgress(p);
          continue;
        }
      }else{
        pMd5 = (const void*)aMd5;
      }
    }

    break;
  }

  if( pBcv->errCode==SQLITE_OK ){
    char aBuf[BCV_MAX_FSNAMEBYTES];
    bcvBlockidToText(NAMEBYTES(pMan), aBlk, aBuf);
    pJob->iBlk = iBlk;
    rc = bcvDispatchFetch(
        pBcv->pDisp, pBcv->pCont, aBuf, 0, pMd5, pJob, bcvDownloadOneFileCb
    );
    if( rc!=SQLITE_OK ){
      pBcv->errCode = rc;
    }
  }
}

/*
** Download a database from cloud storage.
*/
int sqlite3_bcv_download(
  sqlite3_bcv *p,
  const char *zRemote,
  const char *zLocal
){
  int iDb;                        /* Database index in manifest */
  int i;
  Manifest *pMan = 0;
  int rc;
  BcvDownload dl;
  int nJob = p->nRequest;
  BcvDownloadJob *aJob = 0;

  /* If sqlite3_bcv_open() failed */
  if( p->pCont==0 ) return p->errCode;

  memset(&dl, 0, sizeof(dl));
  dl.pBcv = p;
  bcvApiErrorClear(p);

  /* Grab the manifest file */
  if( bcvManifestFetchParsed(p, &pMan) ){
    return p->errCode;
  }
  dl.pMan = pMan;

  /* Search manifest for the requested file. Error out if it is not there. */
  iDb = bcvManifestNameToIndex(pMan, zRemote);
  if( iDb<0 ){
    bcvApiError(p, SQLITE_ERROR, "database not found: %s", zRemote);
    goto download_out;
  }
  dl.pDb = &pMan->aDb[iDb];

  /* Open the local file (to download to) */
  rc = bcvOpenLocal(zLocal, 0, 0, &dl.pFd);
  if( rc!=SQLITE_OK ){
    bcvApiError(
        p, SQLITE_ERROR, "cannot open local file for writing: %s", zLocal
    );
    goto download_out;
  }
  if( SQLITE_OK!=dl.pFd->pMethods->xFileSize(dl.pFd, &dl.szInit) ){
    bcvApiError(
        p, SQLITE_ERROR, "failed to read size of file \"%s\"", zLocal
    );
    goto download_out;
  }

  aJob = (BcvDownloadJob*)bcvMallocRc(&p->errCode, sizeof(BcvDownloadJob)*nJob);
  if( aJob==0 ) goto download_out;
  for(i=0; i<nJob; i++){
    aJob[i].pDownload = &dl;
    aJob[i].iBlk = -1;
    bcvDownloadOneFile(&aJob[i]);
  }

  rc = bcvDispatchRunAll(p->pDisp);
  if( rc!=SQLITE_OK ){
    bcvApiError(p, rc, "");
  }

download_out:
  sqlite3_free(aJob);
  sqlite3_free(dl.aSpace);
  bcvCloseLocal(dl.pFd);
  bcvManifestFree(pMan);
  return p->errCode;
}

/*
** Delete a database from cloud storage.
*/
int sqlite3_bcv_delete(
  sqlite3_bcv *p,
  const char *zDelete
){
  Manifest *pMan = 0;
  int iDb;
  int rc = SQLITE_OK;

  /* If sqlite3_bcv_open() failed */
  if( p->pCont==0 ) return p->errCode;

  bcvApiErrorClear(p);

  /* Download the manifest file. */
  if( bcvManifestFetchParsed(p, &pMan) ){
    return p->errCode;
  }

  /* Check that the db name is in the manifest. */
  iDb = bcvManifestNameToIndex(pMan, zDelete);
  if( iDb<0 ){
    rc = bcvApiError(p, SQLITE_ERROR, "no such database: %s", zDelete);
  }else{
    ManifestDb *pDb = &pMan->aDb[iDb];

    /* Move blocks to the delete list */
    rc = bcvDeleteBlocks(pMan, iDb);

    if( rc==SQLITE_OK ){
      assert( pDb->nBlkLocalAlloc==0 );
      if( pDb->nBlkOrigAlloc ) sqlite3_free(pDb->aBlkOrig);
      if( iDb<pMan->nDb-1 ){
        memmove(pDb, &pDb[1], (pMan->nDb-iDb-1)*sizeof(ManifestDb));
      }
      pMan->nDb--;

      rc = bcvManifestUploadParsed(p, pMan);
    }
  }

  bcvManifestFree(pMan);
  return rc;
}

typedef struct CollectCtx CollectCtx;
struct CollectCtx {
  int rc;
  char *zErr;
  Manifest *pMan;
  ManifestHash *pHash;
  u8 *aDel;
  int nDel;
  int nSeen;                      /* Number of invocations of bcvfsListCb */
  sqlite3_bcv *p;                 /* For logging messages */
};

/*
** aOld/nOld:
**   The original aDelBlk array.
**
** aNew/nNew:
**   The new aDelBlk array. When this object is created, aNew is set to
**   point at a buffer as large as aOld - this way it is guaranteed to be
**   large enough to accomodate the final aDelBlk array.
**
** nProgressDone:
**   Total number of blocks deleted, as last reported to xProgress.
**
** nProgressTotal:
**   Total number of blocks that will be deleted, as reported to xProgress.
*/
typedef struct DeleteCtx DeleteCtx;
struct DeleteCtx {
  Manifest* pMan;
  BcvDispatch *pDisp;
  BcvContainer *pBcv;
  i64 nGC;                        /* Size of GC entry in bytes */
  i64 nName;                      /* Size of block id in bytes */
  int iOld;
  int nOld;                       /* Number of entries in aOld[] */
  const u8 *aOld;
  int nNew;
  u8 *aNew;
  i64 iDelTime;
  i64 nProgressDone;
  i64 nProgressTotal;
  sqlite3_bcv *p;                 /* For errors and logging messages */
};

/*
** Output extra log messages. This is a no-op if the log-level is less
** than 1.
*/
static void bcvExtraLog(sqlite3_bcv *p, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  if( p->nLogLevel>=BCV_LOGLEVEL_DEBUG && p->pDisp->xLog ){
    char *zMsg = sqlite3_vmprintf(zFmt, ap);
    if( zMsg ){
      p->pDisp->xLog(p->pDisp->pLogApp, 0, zMsg);
      sqlite3_free(zMsg);
    }
  }
  va_end(ap);
}

static void bcvExtraLogManifest(
  sqlite3_bcv *p, 
  const char *zCaption, 
  Manifest *pMan
){
  if( p->nLogLevel>=BCV_LOGLEVEL_DEBUG && p->pDisp->xLog ){
    int i;
    bcvExtraLog(p, "======== %s", zCaption);
    bcvExtraLog(p, "Block size: %d", pMan->szBlk);
    bcvExtraLog(p, "Database count: %d", pMan->nDb);

    bcvExtraLog(p, "Delete Block list: (%d blocks)", pMan->nDelBlk);
    for(i=0; i<pMan->nDelBlk; i++){
      u8 *aEntry = &pMan->aDelBlk[i*GCENTRYBYTES(pMan)];
      i64 t;
      char aBuf[BCV_MAX_FSNAMEBYTES];
      bcvBlockidToText(NAMEBYTES(pMan), aEntry, aBuf);
      t = bcvGetU64(&aEntry[NAMEBYTES(pMan)]);
      bcvExtraLog(p, "    %s (t=%lld)", aBuf, t);
    }

    for(i=0; i<pMan->nDb; i++){
      int j;
      ManifestDb *pDb = &pMan->aDb[i];
      bcvExtraLog(p, "Database %d id: %lld", i, pDb->iDbId);
      bcvExtraLog(p, "Database %d name: %s", i, pDb->zDName);
      bcvExtraLog(p, "Database %d version: %d", i, pDb->iVersion);
      bcvExtraLog(p, "Database %d block list: (%d blocks)", i,pDb->nBlkLocal);
      for(j=0; j<pDb->nBlkLocal; j++){
        char aBuf[BCV_MAX_FSNAMEBYTES];
        bcvBlockidToText(
            NAMEBYTES(pMan), &pDb->aBlkLocal[j*NAMEBYTES(pMan)], aBuf
        );
        bcvExtraLog(p, "    %s", aBuf);
      }
    }
    bcvExtraLog(p, "========");
  }
}

/*
** Invoke the progress-handler callback with the supplied parameters. This
** is used for _cleanup() operations only, due to the special SQLITE_DONE
** handling.
*/
static void bcvfsInvokeProgress(sqlite3_bcv *p, i64 nDone, i64 nTotal){
  if( p->errCode==SQLITE_OK && p->xProgress ){
    int res = p->xProgress(p->pProgressCtx, nDone, nTotal);
    if( res!=SQLITE_OK ){
      if( res==SQLITE_DONE ){
        p->errCode = SQLITE_DONE;
      }else{
        bcvApiError(p, SQLITE_ABORT, "abort requested by progress callback");
      }
    }
  }
}

/* 
** This callback may be invoked one of three ways:
**
**     rc==SQLITE_OK, zFile!=0    ->    zFile is filename
**     rc==SQLITE_OK, zFile==0    ->    List files is finished
**     rc!=SQLITE_OK              ->    Error. zFile may be error message
*/
static int bcvfsListCb(void *pArg, int rc, char *zFile){
  CollectCtx *pCtx = (CollectCtx*)pArg;
  if( rc==SQLITE_OK ){
    if( zFile ){
      int nName = NAMEBYTES(pCtx->pMan);
      u8 aBlk[BCV_MAX_NAMEBYTES];
      int eBlock = 0;   /* 0==not a block. 1==block in use. 2==stray block */

      /* Set eBlock */
      eBlock = !bcvfsNameToBlockid(pCtx->pMan, zFile, aBlk);
      if( eBlock && bcvMHashQuery(pCtx->pHash, aBlk, nName)==0 ){
        eBlock = 2;
      }
      bcvExtraLog(pCtx->p, "cleanup: file \"%s\" is %s", zFile,
        eBlock==0 ? "not a block" :
        eBlock==1 ? "accounted for" : "a stray block"
      );
      assert( eBlock==0 || eBlock==1 || eBlock==2 );

      if( eBlock==2 ){
        int nGC = GCENTRYBYTES(pCtx->pMan);
        u8 *pWrite;
        if( (pCtx->nDel & (pCtx->nDel-1))==0 ){
          int nNew = pCtx->nDel ? pCtx->nDel*2 : 8;
          u8 *aNew = (u8*)sqlite3_realloc(pCtx->aDel, nNew*nGC);
          if( aNew==0 ){
            pCtx->rc = SQLITE_NOMEM;
            return pCtx->rc;
          }else{
            pCtx->aDel = aNew;
          }
        }
        pWrite = &pCtx->aDel[pCtx->nDel*nGC];
        memcpy(pWrite, aBlk, nName);
        bcvPutU64(&pWrite[nName], 0);
        pCtx->nDel++;
      }

      pCtx->nSeen++;
      if( (pCtx->nSeen % pCtx->p->pCont->nMaxResults)==0 ){
        bcvfsInvokeProgress(pCtx->p, 0, 1);
        if( pCtx->p->errCode ) return pCtx->p->errCode;
      }
    }
  }else{
    pCtx->rc = rc;
    pCtx->zErr = bcvStrdup(zFile);
  }
  return  pCtx->rc;
}

static void bcvfsDeleteOneBlock(DeleteCtx *pCtx);

static void bcvfsDeleteBlockDone(void *pArg, int rc, char *zErr){
  DeleteCtx *pCtx = (DeleteCtx*)pArg;
  sqlite3_bcv *p = pCtx->p;

  pCtx->nProgressDone++; 
  if( p->errCode==SQLITE_OK && rc!=SQLITE_OK && rc!=HTTP_NOT_FOUND ){
    bcvApiError(p, rc, "delete block failed (%d) - %s", rc, zErr);
  }
  bcvfsInvokeProgress(p, pCtx->nProgressDone, pCtx->nProgressTotal);
  bcvfsDeleteOneBlock(pCtx);
}

static void bcvfsDeleteOneBlock(DeleteCtx *pCtx){
  if( pCtx->p->errCode==SQLITE_OK ){
    int ii;
    for(ii=pCtx->iOld; ii<pCtx->nOld; ii++){
      int iOff = ii*pCtx->nGC;
      i64 iTime = (i64)bcvGetU64(&pCtx->aOld[iOff + pCtx->nName]);
      int bDel = pCtx->iDelTime==0 || iTime<=pCtx->iDelTime;

      /* Do some debug logging */
      if( pCtx->p->nLogLevel>=BCV_LOGLEVEL_DEBUG ){
        char zDebug[BCV_MAX_FSNAMEBYTES];
        bcvBlockidToText(NAMEBYTES(pCtx->pMan), &pCtx->aOld[iOff], zDebug);
        if( bDel==0 ){
          bcvExtraLog(pCtx->p, "cleanup: not deleting block %s "
              "(not eligible for another %lldms)", zDebug, iTime-pCtx->iDelTime
          );
        }else{
          bcvExtraLog(pCtx->p, "cleanup: deleting block %s", zDebug);
        }
      }

      if( bDel ){
        char zFile[BCV_MAX_FSNAMEBYTES];
        bcvBlockidToText(NAMEBYTES(pCtx->pMan), &pCtx->aOld[iOff], zFile);
        pCtx->p->errCode = bcvDispatchDelete(
            pCtx->pDisp, pCtx->pBcv, zFile, 0, (void*)pCtx, bcvfsDeleteBlockDone
        );
        ii++;
        break;
      }else{
        int iNOff = pCtx->nNew * pCtx->nGC;
        memcpy(&pCtx->aNew[iNOff], &pCtx->aOld[iOff], pCtx->nGC);
        pCtx->nNew++;
      }
    }
    pCtx->iOld = ii;
  }
}

static void bcvCleanupSetTotal(DeleteCtx *pCtx){
  assert( pCtx->nProgressTotal==0 );
  assert( pCtx->nProgressDone==0 );
  if( pCtx->p->xProgress ){
    int ii;
    for(ii=0; ii<pCtx->nOld; ii++){
      int iOff = ii*pCtx->nGC;
      i64 iTime = (i64)bcvGetU64(&pCtx->aOld[iOff + pCtx->nName]);
      if( pCtx->iDelTime==0 || iTime<=pCtx->iDelTime ){
        pCtx->nProgressTotal++;
      }
    }
  }
}

/*
** Remove old, unused block files from cloud storage.
*/
int sqlite3_bcv_cleanup(sqlite3_bcv *p, int nSecond){
  BcvDispatch *pDisp = p->pDisp;
  BcvContainer *pBcv = p->pCont;

  CollectCtx ctx1;
  DeleteCtx ctx2;
  memset(&ctx1, 0, sizeof(ctx1));
  memset(&ctx2, 0, sizeof(ctx2));
  ctx1.p = p;
  ctx2.p = p;

  /* If sqlite3_bcv_open() failed */
  if( p->pCont==0 ) return p->errCode;

  bcvApiErrorClear(p);

  /* Download the manifest file. */
  if( bcvManifestFetchParsed(p, &ctx1.pMan) ){
    return p->errCode;
  }

  bcvExtraLogManifest(p, "Manifest downloaded for cleanup:", ctx1.pMan);

  if( p->bFindOrphans ){
    /* Build a hash table of all blocks in the manifest */
    p->errCode = bcvMHashBuild(ctx1.pMan, 1, 0, &ctx1.pHash);
  
    /* Grab a list of files from the cloud container. Accumulate entries
    ** for any orphaned files in the ctx1.aDel array.  */
    if( p->errCode==SQLITE_OK ){
      p->errCode = bcvDispatchList(pDisp, pBcv, 0, (void*)&ctx1, bcvfsListCb);
      if( p->errCode==SQLITE_OK ){
        bcvDispatchRunAll(pDisp);
      }
    }
  
    /* If any orphaned blocks were found, add them to the manifest and 
    ** upload it.  */
    if( p->errCode==SQLITE_OK && ctx1.nDel ){
      int nGC = GCENTRYBYTES(ctx1.pMan);
      i64 nNew = ctx1.nDel + ctx1.pMan->nDelBlk;
      u8 *aNew = (u8*)bcvMallocRc(&p->errCode, nNew*nGC);
      if( aNew ){
        if( ctx1.pMan->nDelBlk ){
          memcpy(aNew, ctx1.pMan->aDelBlk, ctx1.pMan->nDelBlk*nGC);
        }
        memcpy(&aNew[nGC*ctx1.pMan->nDelBlk], ctx1.aDel, ctx1.nDel*nGC);
        ctx1.pMan->aDelBlk = aNew;
        ctx1.pMan->nDelBlk = nNew;
        ctx1.pMan->bDelFree = 1;
        bcvManifestUploadParsed(p, ctx1.pMan);
        bcvExtraLogManifest(p, 
            "Manifest uploaded by cleanup to add stray blocks:", ctx1.pMan
        );
      }
    }
  }

  /* If there are any blocks on the delete list... */
  if( p->errCode==SQLITE_OK && ctx1.pMan->nDelBlk ){
    int ii = 0;
    ctx2.pDisp = pDisp;
    ctx2.pBcv = pBcv;
    ctx2.aOld = ctx1.pMan->aDelBlk;
    ctx2.nOld = ctx1.pMan->nDelBlk;
    ctx2.nGC = GCENTRYBYTES(ctx1.pMan);
    ctx2.nName = NAMEBYTES(ctx1.pMan);
    ctx2.pMan = ctx1.pMan;
    ctx2.aNew = (u8*)bcvMallocRc(&p->errCode, ctx2.nGC*ctx2.nOld);
    if( nSecond>0 ){
      ctx2.iDelTime = sqlite_timestamp() - (i64)nSecond * 1000;
    }

    /* Set DeleteCtx.nProgressTotal, if required */
    bcvCleanupSetTotal(&ctx2);

    for(ii=0; p->errCode==SQLITE_OK && ii<p->nRequest; ii++){
      bcvfsDeleteOneBlock(&ctx2);
    }
    if( p->errCode==SQLITE_OK ){
      int rc = bcvDispatchRunAll(pDisp);
      if( p->errCode==SQLITE_OK || p->errCode==SQLITE_DONE ) p->errCode = rc;
    }
    if( p->errCode==SQLITE_OK && ctx2.nNew<ctx2.nOld ){
      if( ctx1.pMan->bDelFree ) sqlite3_free(ctx1.pMan->aDelBlk);
      ctx1.pMan->aDelBlk = ctx2.aNew;
      ctx1.pMan->nDelBlk = ctx2.nNew;
      ctx1.pMan->bDelFree = 1;
      if( ctx2.iOld<ctx2.nOld ){
        int nCopy = (ctx2.nOld - ctx2.iOld);
        const u8 *aCopy = &ctx2.aOld[ctx2.nGC * ctx2.iOld];
        memcpy(&ctx2.aNew[ctx2.nGC * ctx2.nNew], aCopy, ctx2.nGC * nCopy);
        ctx1.pMan->nDelBlk += nCopy;
      }
      ctx2.aNew = 0;
      bcvManifestUploadParsed(p, ctx1.pMan);
      bcvExtraLogManifest(p, 
          "Manifest uploaded by cleanup to remove deleted blocks", ctx1.pMan
      );
    }
  }

  if( p->errCode==SQLITE_DONE ) p->errCode = SQLITE_OK;
  bcvManifestFree(ctx1.pMan);
  bcvMHashFree(ctx1.pHash);
  sqlite3_free(ctx1.aDel);
  sqlite3_free(ctx2.aNew);
  return p->errCode;
}

/*
** Delete the entire cloud storage container or bucket.
*/
int sqlite3_bcv_destroy(sqlite3_bcv *p){
  int rc;

  /* If sqlite3_bcv_open() failed */
  if( p->pCont==0 ) return p->errCode;

  bcvApiErrorClear(p);
  rc = bcvDispatchDestroy(p->pDisp, p->pCont, (void*)p, bcvCollectError);
  if( rc!=SQLITE_OK ){
    p->errCode = rc;
  }else{
    rc = bcvDispatchRunAll(p->pDisp);
    if( rc ) p->errCode = rc;
  }
  return p->errCode;
}

static void bcvKVUploadCb(void *pApp, int rc, char *zErr){
  sqlite3_bcv *p = (sqlite3_bcv*)pApp;
  if( rc!=SQLITE_OK && (p->bTestNoKv==0 || rc!=HTTP_NOT_FOUND) ){
    bcvApiError(p, rc, "upload kv.bcv failed (%d) - %s", rc, zErr);
  }
}

/*
** Create a new cloud storage container and manifest file. If the container
** already exists, it is not an error, but any existing manifest file is
** simply clobbered.
*/
int sqlite3_bcv_create(sqlite3_bcv *p, int szName, int szBlock){
  Manifest man;

  /* If sqlite3_bcv_open() failed */
  if( p->pCont==0 ) return p->errCode;

  if( szName<=0 ) szName = BCV_DEFAULT_NAMEBYTES;
  if( szBlock<=0 ) szBlock = BCV_DEFAULT_BLOCKSIZE;
  if( szName<BCV_MIN_NAMEBYTES || szName>BCV_MAX_NAMEBYTES ){
    return bcvApiError(p, SQLITE_RANGE, "parameter szName out of range");
  }

  /* Create an empty manifest to upload. */
  memset(&man, 0, sizeof(man));
  man.szBlk = szBlock;
  man.nNamebytes = szName;

  /* First just try to upload the manifest. If that fails for any reason,
  ** assume the container does not exist and attempt to create it. If that
  ** succeeds, attempt to upload the manifest again.  */
  bcvApiErrorClear(p);
  if( bcvManifestUploadParsed(p, &man) ){
    BcvDispatch *pDisp = p->pDisp;
    bcvApiErrorClear(p);

    p->errCode = bcvDispatchCreate(pDisp, p->pCont, (void*)p, bcvCollectError);
    if( p->errCode==SQLITE_OK ){
      bcvDispatchRunAll(pDisp);
    }
    if( p->errCode==SQLITE_OK ){
      bcvManifestUploadParsed(p, &man);
    }
  }

  /* If the manifest has been uploaded, upload an empty KV database. */
  if( p->errCode==SQLITE_OK ){
    int nKV = 0;
    u8 *aKV = bcvEmptyKV(&p->errCode, &nKV);
    if( aKV ){
      int rc;
      if( p->bTestNoKv==0 ){
        rc = bcvDispatchPut(p->pDisp, 
            p->pCont, BCV_KV_FILE, 0, aKV, nKV, (void*)p, bcvKVUploadCb
        );
      }else{
        rc = bcvDispatchDelete(p->pDisp, 
            p->pCont, BCV_KV_FILE, 0, (void*)p, bcvKVUploadCb
        );
      }
      if( rc==SQLITE_OK ){
        rc = bcvDispatchRunAll(p->pDisp);
        if( p->errCode ) rc = p->errCode;
      }
      p->errCode = rc;
      sqlite3_free(aKV);
    }
  }

  return p->errCode;
}

sqlite3_bcv_request *sqlite3_bcv_job_request(
  sqlite3_bcv_job *pJob,          /* Call contex */
  void *pApp,                     /* 3rd argument for xCallback */
  void (*xCallback)(sqlite3_bcv_job*, sqlite3_bcv_request*, void*)
){
  BcvDispatchReq *pNew = 0;
  pNew = bcvMallocRc(&pJob->rc, sizeof(BcvDispatchReq));
  if( pNew ){
    BcvContainer *pCont = pJob->pCont;
    pNew->eMethod = SQLITE_BCV_METHOD_GET;
    pNew->xCallback = xCallback;
    pNew->pApp = pApp;
    pNew->pNext = pJob->pPending;
    pNew->pJob = pJob;
    pNew->pCurl = curl_easy_init();
    pJob->pPending = pNew;

    if( pCont->bRevokeBestEffort ){
      curl_easy_setopt(
          pNew->pCurl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_REVOKE_BEST_EFFORT
      );
    }
    if( pCont->zCainfo ){
      curl_easy_setopt(pNew->pCurl, CURLOPT_CAINFO, pCont->zCainfo);
    }
  }
  return pNew;
}

void sqlite3_bcv_request_set_method(sqlite3_bcv_request *pReq, int eMethod){
  if( pReq ){
    pReq->eMethod = eMethod;
  }
}
void sqlite3_bcv_request_set_uri(sqlite3_bcv_request *pReq, const char *zUri){
  if( pReq ){
    pReq->zUri = bcvStrdupRc(&pReq->pJob->rc, zUri);
  }
}
void sqlite3_bcv_request_set_log(sqlite3_bcv_request *pReq, const char *zUri){
  if( pReq ){
    pReq->zUriLog = bcvStrdupRc(&pReq->pJob->rc, zUri);
  }
}
void sqlite3_bcv_request_set_header(sqlite3_bcv_request *pReq, const char *z){
  if( pReq && z ){
    pReq->pList = curl_slist_append(pReq->pList, z);
  }
}
void sqlite3_bcv_request_set_body(
  sqlite3_bcv_request *pReq, 
  const unsigned char *aBody,
  int nBody
){
  pReq->aBody = aBody;
  pReq->nBody = nBody;
}

int sqlite3_bcv_request_status(
  sqlite3_bcv_request *pReq, 
  const char **pzStatus
){
  long httpcode = 0;
  if( pReq->rc!=SQLITE_OK ){
    httpcode = (long)(pReq->rc);
    *pzStatus = pReq->zStatus;
  }else{
    curl_easy_getinfo(pReq->pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
    if( httpcode==0 || pReq->code!=CURLE_OK ){
      httpcode = SQLITE_ERROR;
      *pzStatus = curl_easy_strerror(pReq->code);
    }else{
      if( (httpcode/100)==2 ){
        httpcode = SQLITE_OK;
      }
      *pzStatus = pReq->zStatus;
    }
  }
  return (int)httpcode;
}

const char *bcvRequestHeader(
  const u8 *aHdrs,
  int nHdrs,
  const char *zHdr
){
  int nHdr = bcvStrlen(zHdr);
  int i;
  char *zVal = 0;

  for(i=0; i<nHdrs; i+=1+bcvStrlen((char*)&aHdrs[i])){
    char *z = (char*)&aHdrs[i];
    if( 0==sqlite3_strnicmp(z, zHdr, nHdr) && z[nHdr]==':' ){
      zVal = &z[nHdr+1];
      while( bcv_isspace(zVal[0]) ) zVal++;
      break;
    }
  }
  return zVal;
}

const char *sqlite3_bcv_request_header(
  sqlite3_bcv_request *pReq, 
  const char *zHdr
){
  return bcvRequestHeader(pReq->hdr.aData, pReq->hdr.nData, zHdr);
}
const unsigned char *sqlite3_bcv_request_body(
  sqlite3_bcv_request *pReq, 
  int *pn
){
  *pn = pReq->body.nData;
  return pReq->body.aData;
}
const unsigned char *sqlite3_bcv_request_hdrs(
  sqlite3_bcv_request *pReq, 
  int *pn
){
  *pn = pReq->hdr.nData;
  return pReq->hdr.aData;
}

void sqlite3_bcv_job_hdrs(
  sqlite3_bcv_job *p, 
  const unsigned char *pHdrs, 
  int nHdrs
){
  sqlite3_free(p->aHdrs);
  p->aHdrs = bcvMallocRc(&p->rc, nHdrs);
  if( p->aHdrs ){
    memcpy(p->aHdrs, pHdrs, nHdrs);
    p->nHdrs = nHdrs;
  }
}

void sqlite3_bcv_job_result(
  sqlite3_bcv_job *p, 
  const unsigned char *pData, 
  int nData
){
  switch( p->eType ){
    case BCV_DISPATCH_FETCH: {
      sqlite3_free(p->aResult);
      p->aResult = bcvMallocRc(&p->rc, nData);
      if( p->aResult ){
        memcpy(p->aResult, pData, nData);
        p->nResult = nData;
      }
      break;
    }
    case BCV_DISPATCH_LIST: {
      assert( nData<0 );
      p->rc = p->cb.xList(p->pCbApp, SQLITE_OK, (char*)pData);
      break;
    }
    default:
      assert( !"todo" );
  }
}

void sqlite3_bcv_job_etag(sqlite3_bcv_job *p, const char *zETag){
  sqlite3_free(p->zETag);
  p->zETag = bcvStrdupRc(&p->rc, zETag);
}
void sqlite3_bcv_job_error(sqlite3_bcv_job *p, int eCode, const char *zErr){
  int rc = SQLITE_OK;
  p->rc = eCode;
  sqlite3_free(p->zETag);
  p->zETag = bcvStrdupRc(&rc, zErr);
}

/*
** Return the prefix, if any, for list operations.
*/
const char *sqlite3_bcv_job_prefix(sqlite3_bcv_job *pJob){
  return pJob->zPrefix;
}

typedef struct BcvModule BcvModule;
struct BcvModule {
  char *zName;
  sqlite3_bcv_module mod;
  void *pCtx;
  BcvModule *pNext;
};

/*
** Globals used by the sqlite3_bcv_create_module() and bcvFindModule()
** functions.
*/
struct BcvGlobal {
  BcvModule *pModuleList;         /* List of installed modules */
  int bInit;                      /* True once built-ins have been installed */
  int iNextDispatchId;            /* Next logging id for dispatcher */
};
static struct BcvGlobal g_bcv;

/*
** This is called by both sqlite3_bcv_create_module() and bcvFindModule()
** to ensure that all built-in modules have been installed. The APP2
** mutex is always held when this function is called. Return SQLITE_OK if
** successful or an SQLite error code if an error occurred.
*/
static int bcvGlobalInit(void){
  int rc = SQLITE_OK;

  if( g_bcv.bInit==0 ){
    assert( g_bcv.pModuleList==0 );
    g_bcv.bInit = 1;
    rc = bcvInstallBuiltinModules();
    if( rc ){
      BcvModule *p;
      BcvModule *pNext;
      for(p=g_bcv.pModuleList; p; p=pNext){
        pNext = p->pNext;
        sqlite3_free(p);
      }
      g_bcv.pModuleList = 0;
      g_bcv.bInit = 0;
    }
  }

  return rc;
}

int bcvCreateModuleUnsafe(
  const char *zName, 
  sqlite3_bcv_module *pMod, 
  void *pCtx
){
  int nName = bcvStrlen(zName);
  int nByte;
  BcvModule *pNew;
  int rc = SQLITE_OK;

  nByte = sizeof(BcvModule) + nName+1;
  pNew = (BcvModule*)sqlite3_malloc(nByte);
  if( pNew ){
    memset(pNew, 0, nByte);
    pNew->zName = (char*)&pNew[1];
    memcpy(pNew->zName, zName, nName+1);
    memcpy(&pNew->mod, pMod, sizeof(sqlite3_bcv_module));
    pNew->pCtx = pCtx;

    pNew->pNext = g_bcv.pModuleList;
    g_bcv.pModuleList = pNew;
  }else{
    rc = SQLITE_NOMEM;
  }

  return rc;
}

/*
** Create a new module.
*/
int sqlite3_bcv_create_module(
  const char *zName, 
  sqlite3_bcv_module *pMod, 
  void *pCtx
){
  int rc;
  rc = sqlite3_initialize();
  if( rc==SQLITE_OK ){
    sqlite3_mutex_enter( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );
    rc = bcvGlobalInit();
    if( rc==SQLITE_OK ){
      rc = bcvCreateModuleUnsafe(zName, pMod, pCtx);
    }
    sqlite3_mutex_leave( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );
  }
  return rc;
}

void sqlite3_bcv_shutdown(){
  if( sqlite3_initialize()==SQLITE_OK ){
    BcvModule *p;
    BcvModule *pNext;
    sqlite3_mutex_enter( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );
    for(p=g_bcv.pModuleList; p; p=pNext){
      pNext = p->pNext;
      sqlite3_free(p);
    }
    memset(&g_bcv, 0, sizeof(g_bcv));
    sqlite3_mutex_leave( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );
  }
}

static int bcvParseModuleName(
  const char *zMod, 
  char ***pazField,
  char **pzErr                    /* OUT: error message */
){
  int rc = SQLITE_OK;
  int nMod = bcvStrlen(zMod);
  int nEntry = nMod / 2;
  char **azField;

  *pazField = azField = (char**)sqlite3_malloc(nMod+1+nEntry*sizeof(char*));
  if( azField==0 ){
    rc = SQLITE_NOMEM;
  }else{
    char *pCsr = (char*)&azField[nEntry];
    int iField = 0;
    int i = 0;
    char cStop = '?';

    azField[iField] = pCsr;
    while( zMod[i] ){
      if( zMod[i]==cStop ){
        *(pCsr++) = '\0';
        azField[++iField] = pCsr;
        cStop = (cStop=='=' ? '&' : '=');
      }else{
        *(pCsr++) = zMod[i];
      }
      i++;
    }
    *(pCsr++) = '\0';
    if( cStop=='=' ){
      azField[++iField] = pCsr;
      *(pCsr++) = '\0';
    }
    azField[++iField] = 0;
  }

  return rc;
}

void bcvContainerDeref(BcvContainer *p){
  p->nContRef--;
  if( p->nContRef==0 ){
    p->pMod->xClose(p->pCont);
    sqlite3_free(p->zCainfo);
    sqlite3_free(p);
  }
}

void bcvContainerClose(BcvContainer *p){
  if( p ){
    bcvContainerDeref(p);
  }
}

static void bcvDispatchReqFree(BcvDispatchReq *pReq){
  curl_easy_cleanup(pReq->pCurl);
  curl_slist_free_all(pReq->pList);
  sqlite3_free(pReq->zUri);
  sqlite3_free(pReq->zUriLog);
  sqlite3_free(pReq->zStatus);
  sqlite3_free(pReq->body.aData);
  sqlite3_free(pReq->hdr.aData);
  sqlite3_free(pReq);
}

int bcvContainerOpen(
  const char *zMod,
  const char *zUser,
  const char *zSecret,
  const char *zCont,
  BcvContainer **ppCont,          /* OUT: New container object (if no error */
  char **pzErr                    /* OUT: error message (if any) */
){
  char **azField = 0;
  int rc;

  *ppCont = 0;
  *pzErr = 0;
  rc = bcvParseModuleName(zMod, &azField, pzErr);
  if( rc==SQLITE_OK ){
    BcvModule *pMod = 0;
    /* Search for the named cloud module */

    sqlite3_mutex_enter( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );
    rc = bcvGlobalInit();
    if( rc==SQLITE_OK ){
      for(pMod=g_bcv.pModuleList; pMod; pMod=pMod->pNext){
        if( sqlite3_stricmp(azField[0], pMod->zName)==0 ) break;
      }
    }
    sqlite3_mutex_leave( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );

    if( pMod ){
      BcvContainer *pNew = sqlite3_malloc(sizeof(BcvContainer));
      if( pNew==0 ){
        rc = SQLITE_NOMEM;
      }else{
        memset(pNew, 0, sizeof(BcvContainer));
        pNew->pMod = &pMod->mod;
        pNew->nContRef = 1;
        pNew->nMaxResults = BCV_MAX_RESULTS;
        rc = pMod->mod.xOpen(pMod->pCtx, (const char**)&azField[1], 
            zUser, zSecret, zCont, &pNew->pCont, pzErr
        );
        if( rc!=SQLITE_OK ){
          sqlite3_free(pNew);
        }else{
          *ppCont = pNew;
          int ii;
          for(ii=1; azField[ii-1] && azField[ii]; ii+=2){
            if( sqlite3_stricmp("maxresults",azField[ii])==0 && azField[ii+1] ){
              const u8 *z = (const u8*)azField[ii+1];
              int nMax = bcvParseInt(z, strlen(azField[ii+1]));
              if( nMax>0 ) pNew->nMaxResults = nMax;
            }
          }
        }
      }

      if( rc==SQLITE_OK ){
        char *zRevokeBestEffort = getenv(CLOUDSQLITE_REVOKE_BEST_EFFORT);
        char *zCainfo = getenv(CLOUDSQLITE_CAINFO);
        if( zRevokeBestEffort ){
          pNew->bRevokeBestEffort = (0!=strtol(zRevokeBestEffort, 0, 0));
        }
        if( zCainfo ){
          pNew->zCainfo = sqlite3_mprintf("%s", zCainfo);
          if( pNew->zCainfo==0 ){
            bcvContainerClose(pNew);
            *ppCont = pNew = 0;
            rc = SQLITE_NOMEM;
          }
        }
      }

    }else if( rc==SQLITE_OK ){
      rc = SQLITE_ERROR;
      *pzErr = sqlite3_mprintf("no such module: %s", azField[0]);
    }

  }
  sqlite3_free(azField);
  return rc;
}

int bcvDispatchNew(BcvDispatch **pp){
  int rc = SQLITE_OK;
  BcvDispatch *pNew;

  *pp = pNew = bcvMallocRc(&rc, sizeof(BcvDispatch));
  if( pNew==0 ){
    rc = SQLITE_NOMEM;
  }else{
    pNew->pMulti = curl_multi_init();
    sqlite3_mutex_enter( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );
    pNew->iDispatchId = g_bcv.iNextDispatchId++;
    sqlite3_mutex_leave( sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP2) );
  }

  return rc;
}

void bcvDispatchVerbose(BcvDispatch *pDisp, int bVerbose){
  pDisp->bVerbose = bVerbose;
}

void bcvDispatchTimeout(BcvDispatch *pDisp, int nHttpTimeout){
  pDisp->nHttpTimeout = nHttpTimeout;
}


static void bcvDispatchJobFree(BcvDispatchJob *pJob){
  assert( pJob->pCont && pJob->pCont->nContRef );
  bcvContainerDeref(pJob->pCont);
  sqlite3_free(pJob->aResult);
  sqlite3_free(pJob->aHdrs);
  sqlite3_free(pJob->zPrefix);
  sqlite3_free(pJob->zETag);
  sqlite3_free(pJob->zLogmsg);
  sqlite3_free(pJob);
}

static void bcvRequestListFree(BcvDispatchReq *pReq){
  BcvDispatchReq *pNext;
  BcvDispatchReq *pDel;

  for(pDel=pReq; pDel; pDel=pNext){
    pNext = pDel->pNext;
    bcvDispatchReqFree(pDel);
  }
}

void bcvDispatchLogObj(
  BcvDispatch *pDisp,
  BcvLog *pLog
){
  pDisp->pLog = pLog;
}

void bcvDispatchLog(
  BcvDispatch *pDisp,                  /* Dispatcher object */
  void *pApp,                          /* First argument to pass to xLog() */
  void (*xLog)(void*, int, const char*)/* http log message callback */
){
  pDisp->xLog = xLog;
  pDisp->pLogApp = pApp;
}

void bcvDispatchFree(BcvDispatch *p){
  if( p ){
    BcvDispatchJob *pJob;
    BcvDispatchJob *pNext;
    for(pJob=p->pJobList; pJob; pJob=pNext){
      void *pApp = pJob->pCbApp;
      pNext = pJob->pNext;
      switch( pJob->eType ){
        case BCV_DISPATCH_FETCH:
          pJob->cb.xFetch(pApp, SQLITE_ERROR, 0, 0, 0, 0, 0);
          break;
        default:
          assert( 0 );
          break;
      }
      bcvRequestListFree(pJob->pPending);
      bcvRequestListFree(pJob->pWaiting);
      bcvDispatchJobFree(pJob);
    }
    
    curl_multi_cleanup(p->pMulti);
    sqlite3_free(p);
  }
}

static size_t bcvWriteFunction(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  BcvDispatchReq *pReq = (BcvDispatchReq*)pCtx;
  int nData = (int)(nSize*nMember);
  bcvBufferAppendRc(&pReq->rc, &pReq->body, pData, nData);
  return nData;
}

static size_t bcvHeaderFunction(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  BcvDispatchReq *pReq = (BcvDispatchReq*)pCtx;
  int nData = (int)(nSize*nMember);
  char *pHdr = pData;
  int nHdr = nData;

  while( nHdr>0 && bcv_isspace(pHdr[0]) ){
    pHdr++;
    nHdr--;
  }
  while( nHdr>0 && bcv_isspace(pHdr[nHdr-1]) ){
    nHdr--;
  }

  if( nHdr>5 && sqlite3_strnicmp(pHdr, "http/", 5)==0 ){
    sqlite3_free(pReq->zStatus);
    pReq->zStatus = bcvMprintfRc(&pReq->rc, "%.*s", nHdr, pHdr);
  }else if( nHdr>0 && pReq->rc==SQLITE_OK ){
    bcvBufferAppendRc(&pReq->rc, &pReq->hdr, pHdr, nHdr);
    bcvBufferAppendRc(&pReq->rc, &pReq->hdr, "", 1);
  }

  return nData;
}

/*
** The following two functions are used as CURLOPT_READFUNCTION and
** CURLOPT_SEEKFUNCTION callbacks when uploading a blob that is in 
** main memory (a manifest file). The context pointer points to an
** instance of type MemoryUpload.
*/
static size_t bcvReadFunction(char *pBuffer, size_t n1, size_t n2, void *pCtx){
  BcvDispatchReq *p = (BcvDispatchReq*)pCtx;
  int nReq = MIN(n1*n2, p->nBody-p->iBody);
  memcpy(pBuffer, &p->aBody[p->iBody], nReq);
  p->iBody += nReq;
  return nReq;
}
static size_t bcvSeekFunction(void *pCtx, curl_off_t offset, int origin){
  BcvDispatchReq *p = (BcvDispatchReq*)pCtx;
  p->iBody = (int)offset;
  return CURL_SEEKFUNC_OK;
}

static void bcvDispatchLogRequest(BcvDispatch *p, BcvDispatchReq *pReq){
  const char *zUri = pReq->zUriLog ? pReq->zUriLog : pReq->zUri;

  assert( pReq->rc==SQLITE_OK );
  pReq->rc = bcvLogRequest(p->pLog, pReq->pJob->zClientId, pReq->pJob->zLogmsg, 
      pReq->eMethod, pReq->nRetry, zUri, &pReq->iLogId
  );

  if( p->xLog ){
    char *zMsg;
    char *zRetry = 0;

    if( pReq->nRetry>0 ){
      zRetry = sqlite3_mprintf(" (nRetry=%d)", pReq->nRetry);
    }
    zMsg = sqlite3_mprintf("r%lld [%s] %s%s %s", 
      pReq->iLogId,
      pReq->pJob->zLogmsg,
      pReq->eMethod==SQLITE_BCV_METHOD_GET ? "GET" :
      pReq->eMethod==SQLITE_BCV_METHOD_PUT ? "PUT" : "DELETE",
      zRetry ? zRetry : "", zUri
    );
    if( zMsg ){
      p->xLog(p->pLogApp, pReq->nRetry>0, zMsg);
      sqlite3_free(zMsg);
    }
    sqlite3_free(zRetry);
  }
}

static void bcvDispatchLogReply(
  BcvDispatch *p, 
  BcvDispatchReq *pReq, 
  int bRetry, 
  int iDelay
){
  long httpcode = 0;
  i64 iMs = 0;

  curl_easy_getinfo(pReq->pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
  bcvLogReply(p->pLog, pReq->iLogId, httpcode, &iMs);

  if( p->xLog ){
    char *zMsg = 0;
    char *zRetry = 0;
    const char *zETag = 0;
    const char *zStatus = 0;
    int res = 0;
    res = sqlite3_bcv_request_status(pReq, &zStatus);
    if( pReq->pJob->bLogEtag && (httpcode/100)==2 ){
      zETag = pReq->pJob->zETag;
    }
    if( bRetry ){
      zRetry = sqlite3_mprintf(" (retry in %dms)", iDelay);
    }
    zMsg = sqlite3_mprintf("r%lld [%s] [%lldms] (http=%d) (rc=%d) %s%s%s%s%s",
      pReq->iLogId, 
      pReq->pJob->zLogmsg,
      iMs,
      httpcode, 
      res,
      zStatus,
      zETag ? " (eTag=" : "", zETag ? zETag : "", zETag ? ")" : "",
      zRetry
    );
    if( zMsg ){
      p->xLog(p->pLogApp, pReq->nRetry>0, zMsg);
      sqlite3_free(zMsg);
    }
    sqlite3_free(zRetry);
  }
}

/*
** This is called after a cloud module function (either a method or callback) 
** is called on a job. It does the following:
**
**   1) Issues any scheduled HTTPS requests.
**
**   2) Check if the job is finished. If so, issue the callback to the
**      application layer. A job is finished if either (a) the error code
**      has been set or (b) there are no outstanding HTTPS requests.
*/
static void bcvDispatchJobUpdate(BcvDispatchJob *pJob){
  BcvDispatch *p = pJob->pDispatch;
  BcvDispatchReq **ppReq;
  i64 iNow = 0;

  /* Issue any scheduled HTTP requests. */
  ppReq = &pJob->pPending;
  while( *ppReq ){
    BcvDispatchReq *pReq = *ppReq;
    CURL *pCurl = pReq->pCurl;

    /* If iRetryTime is non-zero, do not issue this request until the 
    ** time according to sqlite_timestamp() is equal to or greater than
    ** the value.  */
    if( pJob->rc==SQLITE_OK && pReq->iRetryTime ){
      if( iNow==0 ) iNow = sqlite_timestamp();
      if( iNow<pReq->iRetryTime ){
        ppReq=&(*ppReq)->pNext;
        continue;
      }
    }

    /* Remove pReq from the pPending list. */
    *ppReq = pReq->pNext;
    pReq->pNext = 0;

#ifdef SQLITE_BCV_CURL_HANDLE_CONFIG
    if( pJob->rc==SQLITE_OK ){
      pJob->rc = SQLITE_BCV_CURL_HANDLE_CONFIG(pCurl,pReq->eMethod,pReq->zUri);
    }
#endif

    /* If an error has already occurred, do not issue the request. Instead
    ** just have it fail immediately.  */
    if( pJob->rc!=SQLITE_OK ){
      pReq->rc = pJob->rc;
      sqlite3_free(pReq->zStatus);
      pReq->zStatus = 0;
      pReq->xCallback(pJob, pReq, pReq->pApp);
      bcvDispatchReqFree(pReq);
      continue;
    }

    /* Add pReq to the pWaiting list. */
    pReq->pNext = pJob->pWaiting;
    pJob->pWaiting = pReq;

    /* Invoke the log callback if one is configured */
    bcvDispatchLogRequest(p, pReq);

    curl_easy_setopt(pCurl, CURLOPT_PRIVATE, (void*)pReq);
    curl_easy_setopt(pCurl, CURLOPT_URL, pReq->zUri);
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pReq->pList);
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, (long)p->nHttpTimeout);
    curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, (long)1);

    switch( pReq->eMethod ){
      case SQLITE_BCV_METHOD_GET:
        break;

      case SQLITE_BCV_METHOD_HEAD:
        curl_easy_setopt(pCurl, CURLOPT_NOBODY, (long)1);
        break;

      case SQLITE_BCV_METHOD_PUT: {
        curl_off_t nBody = (curl_off_t)pReq->nBody;
        curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, nBody);
        curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, bcvReadFunction);
        curl_easy_setopt(pCurl, CURLOPT_READDATA, (void*)pReq);
        curl_easy_setopt(pCurl, CURLOPT_SEEKFUNCTION, bcvSeekFunction);
        curl_easy_setopt(pCurl, CURLOPT_SEEKDATA, (void*)pReq);
        break;
      }

      default: assert( pReq->eMethod==SQLITE_BCV_METHOD_DELETE );
        curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
        break;
    }

    curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, bcvHeaderFunction);
    curl_easy_setopt(pCurl, CURLOPT_HEADERDATA, (void*)pReq);
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, bcvWriteFunction);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)pReq);
    if( p->bVerbose ){
      curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
    }

    curl_multi_add_handle(p->pMulti, pCurl);
  }

  /* Check if job is finished. */
  if( pJob->pPending==0 && pJob->pWaiting==0 ){
    BcvDispatchJob **pp;
    void *pApp = pJob->pCbApp;

    for(pp=&p->pJobList; *pp!=pJob; pp=&(*pp)->pNext);
    *pp = pJob->pNext;

    switch( pJob->eType ){
      case BCV_DISPATCH_FETCH:
        pJob->cb.xFetch(pApp, pJob->rc, 
            pJob->zETag, pJob->aResult, pJob->nResult, pJob->aHdrs, pJob->nHdrs
        );
        break;
      case BCV_DISPATCH_PUT:
        pJob->cb.xPut(pApp, pJob->rc, pJob->zETag);
        break;
      case BCV_DISPATCH_CREATE:
        pJob->cb.xCreate(pApp, pJob->rc, pJob->zETag);
        break;
      case BCV_DISPATCH_DESTROY:
        pJob->cb.xDestroy(pApp, pJob->rc, pJob->zETag);
        break;
      case BCV_DISPATCH_LIST:
        pJob->cb.xList(pApp, pJob->rc, pJob->zETag);
        break;
      case BCV_DISPATCH_DELETE:
        pJob->cb.xDelete(pApp, pJob->rc, pJob->zETag);
        break;
      default:
        assert( 0 );
        break;
    }

    bcvDispatchJobFree(pJob);
  }
}


static void bcvDispatchFail(
  BcvDispatch *pDisp, 
  int rc, 
  const char *zErr
){
  while( pDisp->pJobList ){
    BcvDispatchJob *pJob = pDisp->pJobList;
    BcvDispatchReq *pReq;
    while( (pReq = pJob->pWaiting) ){
      pJob->pWaiting = pReq->pNext;
      pReq->rc = rc;
      sqlite3_free(pReq->zStatus);
      pReq->zStatus = (char*)zErr;
      pReq->xCallback(pJob, pReq, pReq->pApp);
      pReq->zStatus = 0;
      curl_multi_remove_handle(pDisp->pMulti, pReq->pCurl);
      bcvDispatchReqFree(pReq);
    }
    bcvDispatchJobUpdate(pJob);
  }
}

/*
** This function determines if and when a request should be retried. Requests
** are retried if:
**
**   * An error that occurred that is not an HTTP 403, 404 or 412 error.
**   * The request has not been retried too many times already.
**
** True is returned if the request should be retried. In this case output
** variable (*piMsDelay) is set to the delay in ms after which the request
** should be retried. If the request should not be retried, then zero is
** returned and the final value of (*piMsDelay) is undefined.
*/
static int bcvDispatchRetry(
  int eStatus,                    /* HTTP status (or 1 for CURL error) */
  int nRetryAfter,                /* Retry-After: value in seconds (or 0) */
  int nRetry,                     /* Number of retries so far */
  int *piMsDelay
){
  int aMs[] = { 0, 500, 1000, 2000, 4000, 8000 };
  int bRetry = 0;
  int nMs = 0;

  if( nRetry<sizeof(aMs)/sizeof(aMs[0])
   && ((eStatus/100)!=0 || eStatus==SQLITE_ERROR)
   && (eStatus/100)!=2 
   && (eStatus/100)!=3
   && eStatus!=HTTP_AUTH_ERROR
   && eStatus!=HTTP_BUSY_SNAPSHOT
   && eStatus!=HTTP_NOT_FOUND
  ){
    nMs = aMs[nRetry];
    if( nMs<(nRetryAfter*1000) ){
      int nMaxMs = aMs[ (sizeof(aMs)/sizeof(aMs[0]))-1 ];
      nMs = MIN( nRetryAfter*1000, nMaxMs );
    }
    bRetry = 1;
  }

  *piMsDelay = nMs;
  return bRetry;
}

int bcvDispatchShouldRetry(BcvDispatchReq *pReq, int *piMs){
  const char *zDummy = 0;
  int eStatus = sqlite3_bcv_request_status(pReq, &zDummy);
  curl_off_t nSecond = 0;
  curl_easy_getinfo(pReq->pCurl, CURLINFO_RETRY_AFTER, &nSecond);
  return bcvDispatchRetry(eStatus, nSecond, pReq->nRetry, piMs);
}

/*
** Invoke function bcvDispatchRetry with parameters derived from the
** BcvDispatchReq object passed as the only argument. If bcvDispatchRetry()
** indicates that the request should be retried, reset its internal fields
** and return non-zero. Otherwise, if the request should not be retried,
** return zero.
*/
static void bcvDispatchRetryRequest(BcvDispatchReq *pReq, int iDelay){
  curl_easy_reset(pReq->pCurl);
  sqlite3_free(pReq->zStatus);
  pReq->zStatus = 0;
  bcvBufferZero(&pReq->body);
  bcvBufferZero(&pReq->hdr);
  pReq->nRetry++;
  pReq->rc = 0;
  pReq->code = 0;
  pReq->iBody = 0;
  pReq->iRetryTime = iDelay ? (sqlite_timestamp() + iDelay) : 0;
  pReq->pNext = pReq->pJob->pPending;
  pReq->pJob->pPending = pReq;
}

i64 bcvDispatchJobRetryTime(BcvDispatchJob *pJob){
  BcvDispatchReq *pReq;
  i64 iRet = 0;
  for(pReq=pJob->pPending; pReq; pReq=pReq->pNext){
    if( pReq->iRetryTime && (iRet==0 || pReq->iRetryTime<iRet) ){
      iRet = pReq->iRetryTime;
    }
  }
  return iRet;
}

int bcvDispatchRun(BcvDispatch *p, struct curl_waitfd *aFd, int nFd, int ms){
  int nDummy = 0;
  CURLMcode mc;
  struct CURLMsg *pMsg;
  i64 iMinTime = 0;               /* Minimum retry-time for any job */
  BcvDispatchJob *pJob;           /* Job iterator */
  i64 nMs;                        /* ms to wait for */
  i64 iTime = 0;                  /* Current time */

#ifdef SQLITE_DEBUG
  {
    void *pRet = sqlite3_malloc64(64);
    if( pRet==0 ) return SQLITE_NOMEM;
    sqlite3_free(pRet);
  }
#endif

  for(pJob=p->pJobList; pJob; pJob=pJob->pNext){
    i64 iJobTime = bcvDispatchJobRetryTime(pJob);
    if( iJobTime && (iMinTime==0 || iJobTime<iMinTime) ) iMinTime = iJobTime;
  }

  nMs = ms;
  if( iMinTime ){
    if( iTime==0 ) iTime = sqlite_timestamp();
    if( (iMinTime - iTime)<nMs ) nMs = (iMinTime - iTime);
  }

  /* Wait for something to happen */
  if( nMs>0 ){
    mc = curl_multi_wait(p->pMulti, aFd, nFd, (int)nMs, &nDummy);
    if( mc!=CURLM_OK ) return SQLITE_ERROR;
  }
  mc = curl_multi_perform(p->pMulti, &nDummy);
  if( mc!=CURLM_OK ) return SQLITE_ERROR;

  while( (pMsg = curl_multi_info_read(p->pMulti, &nDummy)) ){
    int bRetry = 0;
    int iDelay = 0;
    BcvDispatchReq *pReq = 0;
    BcvDispatchReq **pp = 0;

    assert( pMsg->msg==CURLMSG_DONE );
    curl_easy_getinfo(pMsg->easy_handle, CURLINFO_PRIVATE, (void**)&pReq);
    pJob = pReq->pJob;
    if( pMsg->data.result!=CURLE_OK ){
      assert( pReq->code==CURLE_OK );
      pReq->code = pMsg->data.result;
    }

    /* Remove the request from the BcvDispatchJob.pWaiting list. And from
    ** the set of CURL handles to wait on next time this function is called.
    ** If logging is turned on, also log the reply.  */
    for(pp=&pJob->pWaiting; *pp!=pReq; pp=&(*pp)->pNext);
    *pp = pReq->pNext;
    pReq->pNext = 0;
    curl_multi_remove_handle(p->pMulti, pReq->pCurl);

    /* Log the reply */
    if( pReq->rc!=SQLITE_OK && pReq->rc<100 ){
      sqlite3_free(pReq->zStatus);
      pReq->zStatus = 0;
    }

    /* Check if this request should be automatically retried */
    bRetry = bcvDispatchShouldRetry(pReq, &iDelay);
    bcvDispatchLogReply(p, pReq, bRetry, iDelay);

    /* Either retry the request (bRetry!=0), or else issue the callback to
    ** indicate that the request has finished.  */
    if( bRetry ){
      bcvDispatchRetryRequest(pReq, iDelay);
    }else{
      pReq->xCallback(pReq->pJob, pReq, pReq->pApp);
      bcvDispatchReqFree(pReq);
    }

    assert( pJob->pWaiting==0 );
    bcvDispatchJobUpdate(pJob);
  }

  /* Call bcvDispatchJobUpdate() on any jobs with retry-timers that have
  ** expired.  */
  if( iMinTime ){
    iTime = sqlite_timestamp();
    for(pJob=p->pJobList; pJob; pJob=pJob->pNext){
      i64 iJobTime = bcvDispatchJobRetryTime(pJob);
      if( iJobTime && iJobTime<iTime ){
        bcvDispatchJobUpdate(pJob);
      }
    }
  }

  return SQLITE_OK;
}

int bcvDispatchRunAll(BcvDispatch *p){
  int rc = SQLITE_OK;
  while( rc==SQLITE_OK && p->pJobList ){
    rc = bcvDispatchRun(p, 0, 0, 1000);
  }
  if( rc!=SQLITE_OK ){
    bcvDispatchFail(p, rc, 0);
  }
  assert( p->pJobList==0 );
  return rc;
}

static BcvDispatchJob *bcvDispatchNewJob(
  BcvDispatch *p,
  BcvContainer *pCont,
  int eType,
  void *pApp,
  int *pRc
){
  BcvDispatchJob *pJob;
  pJob = bcvMallocRc(pRc, sizeof(BcvDispatchJob));
  if( pJob ){
    pJob->pDispatch = p;
    pJob->eType = eType;
    pJob->pCbApp = pApp;
    pJob->pNext = p->pJobList;
    pJob->pCont = pCont;
    pJob->zLogmsg = p->zLogmsg;
    pJob->zClientId = p->zClientId;
    p->pJobList = pJob;
    pCont->nContRef++;
  }else{
    sqlite3_free(p->zLogmsg);
  }
  p->zLogmsg = 0;
  return pJob;
}

int bcvDispatchLogmsg(BcvDispatch *p, const char *zFmt, ...){
  if( p->xLog!=0 || p->pLog!=0 ){
    char *zMsg = 0;
    va_list ap;
    va_start(ap, zFmt);
    zMsg = sqlite3_vmprintf(zFmt, ap);
    va_end(ap);
    sqlite3_free(p->zLogmsg);
    p->zLogmsg = zMsg;
    return zMsg ? SQLITE_OK : SQLITE_NOMEM;
  }
  return SQLITE_OK;
}

int bcvDispatchClientId(BcvDispatch *p, const char *z){
  p->zClientId = z;
  return SQLITE_OK;
}

int bcvDispatchFetch(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zETag,
  const void *pMd5,
  void *pApp,
  void (*x)(void*, int rc, char *zETag, const u8*, int, const u8*, int)
){
  int rc = SQLITE_OK;
  BcvDispatchJob *pJob;
  assert( zETag==0 || pMd5==0 );
  if( (pJob = bcvDispatchNewJob(p, pCont, BCV_DISPATCH_FETCH, pApp, &rc)) ){
    int flags = 0;
    const void *pData = 0;
    int nData = 0;
    if( zETag ){
      flags = BCV_MODULE_CONDITION_ETAG;
      pData = zETag;
      nData = -1;
    }
    else if( pMd5 ){
      flags = BCV_MODULE_CONDITION_MD5;
      pData = pMd5;
      nData = MD5_DIGEST_LENGTH;
    }
    if( pMd5 ) flags = BCV_MODULE_CONDITION_MD5;
    if( p->xLog && sqlite3_stricmp(zFile, BCV_MANIFEST_FILE)==0 ){
      pJob->bLogEtag = 1;
    }
    pJob->cb.xFetch = x;
    pCont->pMod->xFetch(pCont->pCont, pJob, zFile, flags, pData, nData);
    bcvDispatchJobUpdate(pJob);
  }
  return rc;
}

int bcvDispatchDelete(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zIfMatch,
  void *pApp,
  void (*x)(void*, int rc, char *zErr)
){
  int rc = SQLITE_OK;
  BcvDispatchJob *pJob;
  if( (pJob = bcvDispatchNewJob(p, pCont, BCV_DISPATCH_DELETE, pApp, &rc)) ){
    pJob->cb.xDelete = x;
    pCont->pMod->xDelete(pCont->pCont, pJob, zFile, zIfMatch);
    bcvDispatchJobUpdate(pJob);
  }
  return rc;
}


int bcvDispatchPut(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zIfMatch,
  const u8 *aData, int nData,
  void *pApp,
  void (*x)(void*, int rc, char *zETag)
){
  int rc = SQLITE_OK;
  BcvDispatchJob *pJob;
  if( (pJob = bcvDispatchNewJob(p, pCont, BCV_DISPATCH_PUT, pApp, &rc)) ){
    if( p->xLog && sqlite3_stricmp(zFile, BCV_MANIFEST_FILE)==0 ){
      pJob->bLogEtag = 1;
    }
    pJob->cb.xPut = x;
    pCont->pMod->xPut(pCont->pCont, pJob, zFile, aData, nData, zIfMatch);
    bcvDispatchJobUpdate(pJob);
  }
  return rc;
}

int bcvDispatchCreate(
  BcvDispatch *p,
  BcvContainer *pCont,
  void *pApp,
  void (*x)(void*, int rc, char *zError)
){
  int rc = SQLITE_OK;
  BcvDispatchJob *pJob;
  if( (pJob = bcvDispatchNewJob(p, pCont, BCV_DISPATCH_CREATE, pApp, &rc)) ){
    pJob->cb.xCreate = x;
    pCont->pMod->xCreate(pCont->pCont, pJob);
    bcvDispatchJobUpdate(pJob);
  }
  return rc;
}

int bcvDispatchDestroy(
  BcvDispatch *p,
  BcvContainer *pCont,
  void *pApp,
  void (*x)(void*, int rc, char *zError)
){
  int rc = SQLITE_OK;
  BcvDispatchJob *pJob;
  if( (pJob = bcvDispatchNewJob(p, pCont, BCV_DISPATCH_DESTROY, pApp, &rc)) ){
    pJob->cb.xDestroy = x;
    pCont->pMod->xDestroy(pCont->pCont, pJob);
    bcvDispatchJobUpdate(pJob);
  }
  return rc;
}

int bcvDispatchList(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zPrefix,
  void *pApp,
  int (*x)(void*, int rc, char *zError)
){
  int rc = SQLITE_OK;
  BcvDispatchJob *pJob;
  char *zCopy = bcvStrdupRc(&rc, zPrefix);
  if( (pJob = bcvDispatchNewJob(p, pCont, BCV_DISPATCH_LIST, pApp, &rc)) ){
    pJob->cb.xList = x;
    pJob->zPrefix = zCopy;
    pCont->pMod->xList(pCont->pCont, pJob);
    bcvDispatchJobUpdate(pJob);
  }
  if( rc!=SQLITE_OK ){
    sqlite3_free(zCopy);
  }
  return rc;
}

int bcvOpenLocal(
  const char *zFile, 
  int bWal,                       /* Specify SQLITE_OPEN_WAL */
  int bReadonly,
  sqlite3_file **ppFd
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  sqlite3_file *pFd = 0;
  char *zOpen = 0;
  int nByte;
  int flags = bWal ? SQLITE_OPEN_WAL : SQLITE_OPEN_MAIN_DB;
  int rc;

  if( bReadonly ){
    flags |= SQLITE_OPEN_READONLY;
  }else{
    flags |= SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;
  }

  *ppFd = 0;
  nByte = pVfs->szOsFile + 4 + pVfs->mxPathname + 3;
  pFd = (sqlite3_file*)sqlite3_malloc(nByte);
  if( pFd==0 ){
    return SQLITE_NOMEM;
  }
  memset(pFd, 0, nByte);
  zOpen = &((char*)pFd)[pVfs->szOsFile + 4];
  rc = pVfs->xFullPathname(pVfs, zFile, pVfs->mxPathname+1, zOpen);
  if( (rc & 0xFF)==SQLITE_OK ){
    int n = strlen(zOpen);
    zOpen[n+1] = '\0';
    rc = pVfs->xOpen(pVfs, zOpen, pFd, flags, &flags);
  }
  if( rc==SQLITE_OK && bReadonly==0 && (flags & SQLITE_OPEN_READONLY) ){
    rc = SQLITE_READONLY;
    pFd->pMethods->xClose(pFd);
  }
  if( rc!=SQLITE_OK ){
    sqlite3_free(pFd);
    return rc;
  }

  *ppFd = pFd;
  return SQLITE_OK;
}

void bcvCloseLocal(sqlite3_file *pFd){
  if( pFd ){
    pFd->pMethods->xClose(pFd);
    sqlite3_free(pFd);
  }
}

int bcvReadfile(
  sqlite3_file *pFd,
  void *aBuf,
  int iAmt,
  sqlite3_int64 iOff
){
  const int nMax = 32768;
  int i = 0;
  for(i=0; i<iAmt; i+=nMax){
    int nByte = nMax>(iAmt-i) ? (iAmt-i) : nMax;
    u8 *aDest = &((u8*)aBuf)[i];
    int rc = pFd->pMethods->xRead(pFd, aDest, nByte, iOff+i);
    if( rc==SQLITE_IOERR_SHORT_READ ){
      if( iAmt>i+nByte ){
        memset(&aDest[nByte], 0, iAmt-i-nByte);
      }
      rc = SQLITE_OK;
      break;
    }
    if( rc ) return rc;
  }
  return SQLITE_OK;
}

/*
** Argument z points to a buffer n bytes in size. Interpret the contents
** of the buffer as an integer value stored as utf-8 text and return the
** integer value.
*/
int bcvParseInt(const u8 *z, int n){
  int ii;
  int iPort = 0;
  for(ii=0; z[ii]>='0' && z[ii]<='9' && (ii<n || n<0); ii++){
    iPort = iPort*10 + (z[ii]-'0');
  }
  return iPort;
}

/*
** Hex encode the blob passed via the only two arguments to this function
** into buffer aBuf[].
*/
void hex_encode(const unsigned char *aIn, int nIn, char *aBuf, int bUpper){
  static const char *a[2] = { "0123456789abcdef", "0123456789ABCDEF" };
  const char *aDigit = a[bUpper];
  int i;

  for(i=0; i<nIn; i++){
    aBuf[i*2] = aDigit[ (aIn[i] >> 4) ];
    aBuf[i*2+1] = aDigit[ (aIn[i] & 0xF) ];
  }
  aBuf[i*2] = '\0';
}

/*
** Attempt to decode the nIn byte hexadecimal string indicated by aIn and write
** the results to aBuf[]. If a non-hex character is encountered, return
** non-zero immediately. The final contents of aBuf[] is undefined in this
** case. Otherwise, if decoding is successful, return zero.
*/
int hex_decode(const char *aIn, int nIn, u8 *aBuf){
  static const int aCharMap[] = {
   -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,      /* 0x00-0x0F */
   -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,      /* 0x10-0x1F */
   -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,      /* 0x20-0x2F */
    0, 1, 2, 3, 4, 5, 6, 7,   8, 9,-1,-1,-1,-1,-1,-1,      /* 0x30-0x3F */
   -1,10,11,12,13,14,15,-1,  -1,-1,-1,-1,-1,-1,-1,-1,      /* 0x40-0x4F */
   -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,      /* 0x50-0x5F */
   -1,10,11,12,13,14,15,-1,  -1,-1,-1,-1,-1,-1,-1,-1,      /* 0x60-0x6F */
  };
  int i;

  if( nIn%2 ) return 1;
  for(i=0; i<nIn; i+=2){
    u8 a = (u8)aIn[i];
    u8 b = (u8)aIn[i+1];
    if( a>=sizeof(aCharMap)/sizeof(aCharMap[0]) || aCharMap[a]<0 ) return 1;
    if( b>=sizeof(aCharMap)/sizeof(aCharMap[0]) || aCharMap[b]<0 ) return 1;
    aBuf[i/2] = (aCharMap[a]<<4) + aCharMap[b];
  }

  return 0;
}

int bcv_socket_is_valid(BCV_SOCKET_TYPE s){
#ifdef __WIN32__
  return (s!=INVALID_SOCKET);
#else
  return (s>=0);
#endif
}

void bcv_close_socket(BCV_SOCKET_TYPE s){
  if( bcv_socket_is_valid(s) ){
#ifdef __WIN32__
    shutdown(s, SD_BOTH);
    closesocket(s);
#else
    shutdown(s, SHUT_RDWR);
    close(s);
#endif
  }
}

void bcv_socket_init(){
#ifdef __WIN32__
  WORD v;
  WSADATA wsa;
  v = MAKEWORD(2,2);
  WSAStartup(v, &wsa);
#endif
}

static int bcvDefaultRecv(BCV_SOCKET_TYPE fd, void *aRead, int nRead){
  return recv(fd, aRead, nRead, 0);
}
static int bcvDefaultSend(BCV_SOCKET_TYPE fd, void *aSend, int nSend){
  return send(fd, aSend, nSend, MSG_NOSIGNAL);
}

static struct BcvSocketAPI {
  int (*xRecv)(BCV_SOCKET_TYPE, void *, int);
  int (*xSend)(BCV_SOCKET_TYPE, void *, int);
} bcv_socket_api = {
  bcvDefaultRecv,
  bcvDefaultSend,
};

void sqlite3_bcv_test_socket_api(
  int (*xRecv)(BCV_SOCKET_TYPE, void *, int),
  int (*xSend)(BCV_SOCKET_TYPE, void *, int),
  int (**pxRecv)(BCV_SOCKET_TYPE, void *, int),
  int (**pxSend)(BCV_SOCKET_TYPE, void *, int)
){
  *pxRecv = bcv_socket_api.xRecv;
  *pxSend = bcv_socket_api.xSend;
  bcv_socket_api.xRecv = xRecv;
  bcv_socket_api.xSend = xSend;
}

static int bcv_recv(BCV_SOCKET_TYPE fd, void *aRead, int nRead){
  u8 *a = (u8*)aRead;
  int nDone = 0;
  while( nDone<nRead ){
    int res = bcv_socket_api.xRecv(fd, &a[nDone], nRead-nDone);
    if( res<=0 ){
      return SQLITE_IOERR;
    }
    nDone += res;
  }
  return SQLITE_OK;
}

static int bcv_send(BCV_SOCKET_TYPE fd, void *aSend, int nSend){
  u8 *a = (u8*)aSend;
  int nDone = 0;
  while( nDone<nSend ){
    int res = bcv_socket_api.xSend(fd, &a[nDone], nSend-nDone);
    if( res<=0 ){
      return SQLITE_IOERR;
    }
    nDone += res;
  }
  return SQLITE_OK;
}


void bcvBufferAppendU32(int *pRc, BcvBuffer *p, u32 iVal){
  u8 a[4];
  bcvPutU32(a, iVal);
  bcvBufferAppendRc(pRc, p, a, 4);
}

void bcvBufferAppendU64(int *pRc, BcvBuffer *p, u64 iVal){
  u8 a[8];
  bcvPutU64(a, iVal);
  bcvBufferAppendRc(pRc, p, a, 8);
}

void bcvBufferMsgString(int *pRc, BcvBuffer *p, const char *zStr){
  if( zStr ){
    int n = bcvStrlen(zStr) + 1;
    bcvBufferAppendU32(pRc, p, (u32)n);
    bcvBufferAppendRc(pRc, p, zStr, n);
  }else{
    bcvBufferAppendU32(pRc, p, 0);
  }
}

void bcvBufferMsgBlob(
  int *pRc, 
  BcvBuffer *p, 
  const u8 *aData, 
  int nData
){
  bcvBufferAppendU32(pRc, p, (u32)nData);
  bcvBufferAppendRc(pRc, p, aData, nData);
}

static void bcvBufferMsgU32Array(int *pRc, BcvBuffer *p, u32 *a, u32 n){
  bcvBufferAppendU32(pRc, p, n);
  bcvBufferAppendRc(pRc, p, a, sizeof(u32)*n);
}

int bcvSendMsg(BCV_SOCKET_TYPE fd, BcvMessage *pMsg){
  int rc = SQLITE_OK;
  BcvBuffer buf = {0,0,0};

  bcvBufferAppendU32(&rc, &buf, pMsg->eType);
  bcvBufferAppendU32(&rc, &buf, 0);

  switch( pMsg->eType ){
    case BCV_MESSAGE_ATTACH:
      bcvBufferMsgString(&rc, &buf, pMsg->u.attach.zStorage);
      bcvBufferMsgString(&rc, &buf, pMsg->u.attach.zAccount);
      bcvBufferMsgString(&rc, &buf, pMsg->u.attach.zContainer);
      bcvBufferMsgString(&rc, &buf, pMsg->u.attach.zAlias);
      bcvBufferMsgString(&rc, &buf, pMsg->u.attach.zAuth);
      bcvBufferAppendU32(&rc, &buf, (u32)pMsg->u.attach.flags);
      break;
    case BCV_MESSAGE_REPLY:
      bcvBufferAppendU32(&rc, &buf, (u32)pMsg->u.error_r.errCode);
      bcvBufferAppendU32(&rc, &buf, (u32)pMsg->u.error_r.szBlk);
      bcvBufferMsgString(&rc, &buf, pMsg->u.error_r.zErrMsg);
      break;
    case BCV_MESSAGE_VTAB:
      bcvBufferMsgString(&rc, &buf, pMsg->u.vtab.zVtab);
      bcvBufferMsgString(&rc, &buf, pMsg->u.vtab.zContainer);
      bcvBufferMsgString(&rc, &buf, pMsg->u.vtab.zDatabase);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.vtab.colUsed);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.vtab.iVersion);
      break;
    case BCV_MESSAGE_VTAB_REPLY:
      bcvBufferMsgBlob(&rc, &buf, pMsg->u.vtab_r.aData, pMsg->u.vtab_r.nData);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.vtab_r.iVersion);
      break;
    case BCV_MESSAGE_DETACH:
      bcvBufferMsgString(&rc, &buf, pMsg->u.detach.zName);
      break;
    case BCV_MESSAGE_HELLO:
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello.zContainer);
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello.zDatabase);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.hello.bPrefetch);
      break;
    case BCV_MESSAGE_HELLO_REPLY:
      bcvBufferAppendU32(&rc, &buf, pMsg->u.hello_r.errCode);
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello_r.zErrMsg);
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello_r.zStorage);
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello_r.zAccount);
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello_r.zContainer);
      bcvBufferAppendU64(&rc, &buf, (u64)pMsg->u.hello_r.iDbId);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.hello_r.szBlk);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.hello_r.bEncrypted);
      break;
    case BCV_MESSAGE_READ: 
      bcvBufferAppendU32(&rc, &buf, pMsg->u.read.iBlk);
      bcvBufferMsgU32Array(&rc, &buf, pMsg->u.read.aMru, pMsg->u.read.nMru);
      bcvBufferMsgString(&rc, &buf, pMsg->u.read.zAuth);
      break;
    case BCV_MESSAGE_READ_REPLY: 
      bcvBufferAppendU32(&rc, &buf, pMsg->u.read_r.errCode);
      bcvBufferMsgString(&rc, &buf, pMsg->u.read_r.zErrMsg);
      bcvBufferMsgU32Array(&rc, &buf, pMsg->u.read_r.aBlk, pMsg->u.read_r.nBlk);
      break;
    case BCV_MESSAGE_END: 
      bcvBufferMsgU32Array(&rc, &buf, pMsg->u.end.aMru, pMsg->u.end.nMru);
      break;
    case BCV_MESSAGE_CMD: 
      bcvBufferMsgString(&rc, &buf, pMsg->u.cmd.zAuth);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.cmd.eCmd);
      break;
    case BCV_MESSAGE_PASS: 
      bcvBufferMsgString(&rc, &buf, pMsg->u.cmd.zAuth);
      break;
    case BCV_MESSAGE_PASS_REPLY: 
      bcvBufferAppendU32(&rc, &buf, pMsg->u.pass_r.errCode);
      bcvBufferMsgString(&rc, &buf, pMsg->u.pass_r.zErrMsg);
      bcvBufferAppendRc(&rc, &buf, pMsg->u.pass_r.aKey, BCV_LOCAL_KEYSIZE);
      break;
    case BCV_MESSAGE_PREFETCH: 
      bcvBufferMsgString(&rc, &buf, pMsg->u.prefetch.zAuth);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.prefetch.nRequest);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.prefetch.nMs);
      break;
    case BCV_MESSAGE_PREFETCH_REPLY: 
      bcvBufferAppendU32(&rc, &buf, pMsg->u.prefetch_r.errCode);
      bcvBufferMsgString(&rc, &buf, pMsg->u.prefetch_r.zErrMsg);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.prefetch_r.nOnDemand);
      bcvBufferAppendU32(&rc, &buf, pMsg->u.prefetch_r.nOutstanding);
      break;

    default:
      assert( 0 );
      break;
  }
  
  if( rc==SQLITE_OK ){
    bcvPutU32(&buf.aData[4], (u32)buf.nData-8);
    rc = bcv_send(fd, buf.aData, buf.nData);
  }
  bcvBufferZero(&buf);
  return rc;
}

static u64 bcvMsgGetU64(u8 **pa){
  u64 nRet = bcvGetU64(*pa);
  *pa += 8;
  return nRet;
}

static u32 bcvMsgGetU32(u8 **pa){
  u32 nRet = bcvGetU32(*pa);
  *pa += 4;
  return nRet;
}

static const u8 *bcvMsgGetBlob(u8 **pa, int *pnBlob){
  const u8 *pRet = 0;
  u32 nBlob = bcvMsgGetU32(pa);
  if( nBlob>0 ){
    pRet = (const u8*)*pa;
    *pa += nBlob;
  }
  *pnBlob = (int)nBlob;
  return pRet;
}

static const char *bcvMsgGetString(u8 **pa){
  int nDummy = 0;
  return (const char*)bcvMsgGetBlob(pa, &nDummy);
}

static u32 *bcvMsgGetU32Array(u8 **pa, u32 *pnEntry){
  u32 n = bcvMsgGetU32(pa);
  u32 *aRet = (u32*)*pa;
  *pa += n*sizeof(u32);
  *pnEntry = n;
  return aRet;
}

int bcvRecvMsg(
  BCV_SOCKET_TYPE fd, 
  BcvMessage **ppMsg
){
  BcvMessage *pMsg = 0;
  int rc = SQLITE_OK;
  u8 aHdr[8];

  memset(aHdr, 0, sizeof(aHdr));
  rc = bcv_recv(fd, aHdr, sizeof(aHdr));
  if( rc==SQLITE_OK ){
    u32 eType = bcvGetU32(&aHdr[0]);
    u32 nByte = bcvGetU32(&aHdr[4]);

    pMsg = (BcvMessage*)bcvMallocRc(&rc,sizeof(BcvMessage)+nByte);
    if( rc==SQLITE_OK ){
      u8 *aBody = (u8*)&pMsg[1];
      rc = bcv_recv(fd, aBody, nByte);
      if( rc==SQLITE_OK ){
        pMsg->eType = (int)eType;
        switch( eType ){
          case BCV_MESSAGE_ATTACH: {
            pMsg->u.attach.zStorage = bcvMsgGetString(&aBody);
            pMsg->u.attach.zAccount = bcvMsgGetString(&aBody);
            pMsg->u.attach.zContainer = bcvMsgGetString(&aBody);
            pMsg->u.attach.zAlias = bcvMsgGetString(&aBody);
            pMsg->u.attach.zAuth = bcvMsgGetString(&aBody);
            pMsg->u.attach.flags = bcvMsgGetU32(&aBody);
            break;
          }
          case BCV_MESSAGE_REPLY: {
            pMsg->u.error_r.errCode = bcvMsgGetU32(&aBody);
            pMsg->u.error_r.szBlk = (int)bcvMsgGetU32(&aBody);
            pMsg->u.error_r.zErrMsg = bcvMsgGetString(&aBody);
            break;
          }
          case BCV_MESSAGE_VTAB: {
            pMsg->u.vtab.zVtab = bcvMsgGetString(&aBody);
            pMsg->u.vtab.zContainer = bcvMsgGetString(&aBody);
            pMsg->u.vtab.zDatabase = bcvMsgGetString(&aBody);
            pMsg->u.vtab.colUsed = bcvMsgGetU32(&aBody);
            if( (aBody - (u8*)&pMsg[1])<nByte ){
              pMsg->u.vtab.iVersion = bcvMsgGetU32(&aBody);
            }
            break;
          }
          case BCV_MESSAGE_VTAB_REPLY: {
            pMsg->u.vtab_r.aData = bcvMsgGetBlob(&aBody, &pMsg->u.vtab_r.nData);
            if( (aBody - (u8*)&pMsg[1])<nByte ){
              pMsg->u.vtab_r.iVersion = bcvMsgGetU32(&aBody);
            }
            break;
          }
          case BCV_MESSAGE_DETACH: {
            pMsg->u.detach.zName = bcvMsgGetString(&aBody);
            break;
          }
          case BCV_MESSAGE_HELLO: {
            pMsg->u.hello.zContainer = bcvMsgGetString(&aBody);
            pMsg->u.hello.zDatabase = bcvMsgGetString(&aBody);
            if( (aBody - (u8*)&pMsg[1])<nByte ){
              pMsg->u.hello.bPrefetch = bcvMsgGetU32(&aBody);
            }
            break;
          }

          case BCV_MESSAGE_HELLO_REPLY: {
            pMsg->u.hello_r.errCode = bcvMsgGetU32(&aBody);
            pMsg->u.hello_r.zErrMsg = bcvMsgGetString(&aBody);
            pMsg->u.hello_r.zStorage = bcvMsgGetString(&aBody);
            pMsg->u.hello_r.zAccount = bcvMsgGetString(&aBody);
            pMsg->u.hello_r.zContainer = bcvMsgGetString(&aBody);
            pMsg->u.hello_r.iDbId = (i64)bcvMsgGetU64(&aBody);
            pMsg->u.hello_r.szBlk = bcvMsgGetU32(&aBody);
            pMsg->u.hello_r.bEncrypted = bcvMsgGetU32(&aBody);
            break;
          }

          case BCV_MESSAGE_READ: {
            pMsg->u.read.iBlk = bcvMsgGetU32(&aBody);
            pMsg->u.read.aMru = bcvMsgGetU32Array(&aBody, &pMsg->u.read.nMru);
            pMsg->u.read.zAuth = bcvMsgGetString(&aBody);
            break;
          }
          case BCV_MESSAGE_READ_REPLY: {
            u32 *pnBlk = &pMsg->u.read_r.nBlk;
            pMsg->u.read_r.errCode = bcvMsgGetU32(&aBody);
            pMsg->u.read_r.zErrMsg = bcvMsgGetString(&aBody);
            pMsg->u.read_r.aBlk = bcvMsgGetU32Array(&aBody, pnBlk);
            break;
          }

          case BCV_MESSAGE_END:
            pMsg->u.end.aMru = bcvMsgGetU32Array(&aBody, &pMsg->u.end.nMru);
            break;
          case BCV_MESSAGE_CMD: 
            pMsg->u.cmd.zAuth = bcvMsgGetString(&aBody);
            pMsg->u.cmd.eCmd = bcvMsgGetU32(&aBody);
            break;
    
          case BCV_MESSAGE_PASS: 
            pMsg->u.cmd.zAuth = bcvMsgGetString(&aBody);
            break;
          case BCV_MESSAGE_PASS_REPLY: 
            pMsg->u.pass_r.errCode = bcvMsgGetU32(&aBody);
            pMsg->u.pass_r.zErrMsg = bcvMsgGetString(&aBody);
            pMsg->u.pass_r.aKey = aBody; /* aBody += BCV_LOCAL_KEYSIZE; */
            break;
          case BCV_MESSAGE_PREFETCH: 
            pMsg->u.prefetch.zAuth = bcvMsgGetString(&aBody);
            pMsg->u.prefetch.nRequest = bcvMsgGetU32(&aBody);
            pMsg->u.prefetch.nMs = bcvMsgGetU32(&aBody);
            break;
          case BCV_MESSAGE_PREFETCH_REPLY: 
            pMsg->u.prefetch_r.errCode = bcvMsgGetU32(&aBody);
            pMsg->u.prefetch_r.zErrMsg = bcvMsgGetString(&aBody);
            pMsg->u.prefetch_r.nOnDemand = bcvMsgGetU32(&aBody);
            pMsg->u.prefetch_r.nOutstanding = bcvMsgGetU32(&aBody);
            break;

          default:
            assert( 0 );
            break;
        }
      }
    }

    if( rc!=SQLITE_OK ){
      sqlite3_free(pMsg);
      pMsg = 0;
    }
  }

  *ppMsg = pMsg;
  return rc;
}

struct BcvIntKey {
  BcvEncryptionKey *pKey;
  int nRef;
};

BcvIntKey *bcvIntEncryptionKeyRef(BcvIntKey *pKey){
  if( pKey ){
    pKey->nRef++;
  }
  return pKey;
}

BcvIntKey *bcvIntEncryptionKeyNew(const u8 *aKey){
  BcvEncryptionKey *pKey = bcvEncryptionKeyNew(aKey);
  BcvIntKey *pRet = 0;
  if( pKey ){
    pRet = (BcvIntKey*)sqlite3_malloc(sizeof(BcvIntKey));
    if( pRet==0 ){
      bcvEncryptionKeyFree(pKey);
    }else{
      pRet->pKey = pKey;
      pRet->nRef = 1;
    }
  }
  return pRet;
}

void bcvIntEncryptionKeyFree(BcvIntKey *pKey){
  if( pKey ){
    pKey->nRef--;
    if( pKey->nRef<=0 ){
      bcvEncryptionKeyFree(pKey->pKey);
      sqlite3_free(pKey);
    }
  }
}

int bcvIntDecrypt(
  BcvIntKey *pKey, 
  sqlite3_int64 iNonce, 
  u8 *aNonce, 
  u8 *aData, int nData
){
  u64 aLocal[2] = {0, 0};
  if( aNonce ) memcpy(&aLocal, aNonce, 16);
  aLocal[1] ^= (u64)iNonce;
  return bcvDecrypt(pKey->pKey, (const u8*)aLocal, aData, nData);
}

int bcvIntEncrypt(
  BcvIntKey *pKey, 
  sqlite3_int64 iNonce, 
  u8 *aNonce, 
  u8 *aData, int nData
){
  u64 aLocal[2] = {0, 0};
  if( aNonce ) memcpy(&aLocal, aNonce, 16);
  aLocal[1] ^= (u64)iNonce;
  return bcvEncrypt(pKey->pKey, (const u8*)aLocal, aData, nData);
}



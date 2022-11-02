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

#define BCV_MIMETYPE_HDR    "Content-Type:application/octet-stream"
#define BCV_MIMETYPE_HDR_LC "content-type:application/octet-stream"

#define BCV_AWS_DATE_SZ 16
#define BCV_AWS_HASH_SZ (2*SHA256_DIGEST_LENGTH)

#define BCV_AWS_CONTENTHASH_HDR "x-amz-content-sha256:UNSIGNED-PAYLOAD"

/* Maximum value for SQLITE_BCVCONFIG_LOG */
#define BCV_MAX_NREQUEST 32

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

  bcv_log_cb xLog;
  void *pLogCtx;
};

typedef struct sqlite3_bcv_job BcvDispatchJob;
typedef struct sqlite3_bcv_request BcvDispatchReq;

struct BcvDispatch {
  CURLM *pMulti;                  /* The libcurl dispatcher ("multi-handle") */
  int bVerbose;                   /* True to make libcurl verbose */
  int nHttpTimeout;
  void *pLogApp;                  /* First argument to xLog() */
  void (*xLog)(void*,int,const char *zMsg);    /* Log function to invoke */
  int iNextRequestId;             /* Next request-id (for logging only) */
  int iDispatchId;                /* Dispatch id (for logging only) */
  char *zLogmsg;                  /* Logging caption for next http request */
  BcvDispatchJob *pJobList;       /* List of current jobs */
};

struct sqlite3_bcv_job {
  BcvDispatch *pDispatch;
  int rc;
  int eType;                      /* Type of job (BCV_DISPATCH_XXX value) */
  int bLogEtag;                   /* True to log eTag of http replies */
  char *zLogmsg;                  /* Logging caption */
  void *pCbApp;                   /* First argument for callback */
  struct {
    void (*xFetch)(void*, int rc, char *zETag, const u8 *aData, int nData);
    void (*xPut)(void*, int rc, char *zETag);
    void (*xCreate)(void*, int rc, char *zError);
    void (*xDestroy)(void*, int rc, char *zError);
    void (*xList)(void*, int rc, char *z);
    void (*xDelete)(void*, int rc, char *zError);
  } cb;

  /* Result values */
  char *zETag;
  u8 *aResult;
  int nResult;

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
  int iRequestId;                 /* Unique id used for logging only */
  i64 iRequestTime;               /* Time request was made */

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
  int nData
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
** The first argument points to a NAMEBYTES byte buffer
** containing a block-id. Format the block-id as text and write it to
** buffer aBuf[], which must be at leat BCV_FILESYSTEM_BLOCKID_BYTES
** in size.
*/
void bcvBlockidToText(Manifest *p, const u8 *pBlk, char *aBuf){
  hex_encode(pBlk, NAMEBYTES(p), aBuf, 1);
  memcpy(&aBuf[NAMEBYTES(p)*2], ".bcv", 5);
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
  if( p->xLog ){
    p->xLog(p->pLogCtx, zMsg);
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
        p->pLogCtx = va_arg(ap, void*);
        p->xLog = (bcv_log_cb)va_arg(ap, bcv_log_cb);
        if( p->xLog ){
          bcvDispatchLog(p->pDisp, (void*)p, bcvLogWrapper);
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

    bcvBlockidToText(pMan, aBlk, aBuf);
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
  nRemote = bcvStrlen(zRemote);
  if( nRemote>=BCV_DBNAME_SIZE ){
    return bcvApiError(p, SQLITE_ERROR, 
        "database name \"%s\" is too long (max = %d bytes)",
        zRemote, BCV_DBNAME_SIZE-1
    );
  }

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

static void bcvDeleteBlocks(Manifest *pMan, int iDb){
  ManifestDb *pDb = &pMan->aDb[iDb];
  u8 *aGC = 0;
  i64 iTime = 0;
  u8 aTime[8];
  int i;
  int iGC = 0;
  ManifestHash *pMH = 0;
  const int nName = NAMEBYTES(pMan);

  iTime = sqlite_timestamp();
  bcvPutU64(aTime, iTime);

  bcvMHashBuild(pMan, 0, pDb, &pMH);
  assert( pDb->nBlkOrig==pDb->nBlkLocal );
  aGC = (u8*)bcvMalloc((pMan->nDelBlk+pDb->nBlkOrig) * GCENTRYBYTES(pMan));
  if( pMan->nDelBlk ){
    memcpy(aGC, pMan->aDelBlk, pMan->nDelBlk * GCENTRYBYTES(pMan));
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
  bcvMHashFree(pMH);
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
  int nData
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
    bcvBlockidToText(pMan, aBlk, aBuf);
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
    bcvDeleteBlocks(pMan, iDb);

    assert( pDb->nBlkLocalAlloc==0 );
    if( pDb->nBlkOrigAlloc ) sqlite3_free(pDb->aBlkOrig);
    if( iDb<pMan->nDb-1 ){
      memmove(pDb, &pDb[1], (pMan->nDb-iDb-1)*sizeof(ManifestDb));
    }
    pMan->nDb--;

    rc = bcvManifestUploadParsed(p, pMan);
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
*/
typedef struct DeleteCtx DeleteCtx;
struct DeleteCtx {
  Manifest* pMan;
  BcvDispatch *pDisp;
  BcvContainer *pBcv;
  i64 nGC;                        /* Size of GC entry in bytes */
  i64 nName;                      /* Size of block id in bytes */
  int iOld;
  int nOld;                       
  const u8 *aOld;
  int nNew;
  u8 *aNew;
  i64 iDelTime;
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
      bcvBlockidToText(pMan, aEntry, aBuf);
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
        bcvBlockidToText(pMan, &pDb->aBlkLocal[j*NAMEBYTES(pMan)], aBuf);
        bcvExtraLog(p, "    %s", aBuf);
      }
    }
    bcvExtraLog(p, "========");
  }
}

/* 
** This callback may be invoked one of three ways:
**
**     rc==SQLITE_OK, zFile!=0    ->    zFile is filename
**     rc==SQLITE_OK, zFile==0    ->    List files is finished
**     rc!=SQLITE_OK              ->    Error. zFile may be error message
*/
static void bcvfsListCb(void *pArg, int rc, char *zFile){
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
            return;
          }else{
            pCtx->aDel = aNew;
          }
        }
        pWrite = &pCtx->aDel[pCtx->nDel*nGC];
        memcpy(pWrite, aBlk, nName);
        bcvPutU64(&pWrite[nName], 0);
        pCtx->nDel++;
      }
    }
  }else{
    pCtx->rc = rc;
    pCtx->zErr = bcvStrdup(zFile);
  }
}

static void bcvfsDeleteOneBlock(DeleteCtx *pCtx);

static void bcvfsDeleteBlockDone(void *pArg, int rc, char *zErr){
  DeleteCtx *pCtx = (DeleteCtx*)pArg;
  if( pCtx->p->errCode==SQLITE_OK && rc!=SQLITE_OK && rc!=HTTP_NOT_FOUND ){
    bcvApiError(pCtx->p, rc, "delete block failed (%d) - %s", rc, zErr);
  }
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
        bcvBlockidToText(pCtx->pMan, &pCtx->aOld[iOff], zDebug);
        if( bDel==0 ){
          bcvExtraLog(pCtx->p, "cleanup: not deleting block %s "
              "(not eligible for another %lldms)", zDebug, iTime-pCtx->iDelTime
          );
        }else{
          bcvExtraLog(pCtx->p, "cleanup: deleting block %s", zDebug);
        }
      }

      if( pCtx->iDelTime==0 || iTime<=pCtx->iDelTime ){
        char zFile[BCV_MAX_FSNAMEBYTES];
        bcvBlockidToText(pCtx->pMan, &pCtx->aOld[iOff], zFile);
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

    for(ii=0; p->errCode==SQLITE_OK && ii<p->nRequest; ii++){
      bcvfsDeleteOneBlock(&ctx2);
    }
    if( p->errCode==SQLITE_OK ){
      int rc = bcvDispatchRunAll(pDisp);
      if( p->errCode==SQLITE_OK ) p->errCode = rc;
    }
    if( p->errCode==SQLITE_OK && ctx2.nNew<ctx2.nOld ){
      if( ctx1.pMan->bDelFree ) sqlite3_free(ctx1.pMan->aDelBlk);
      ctx1.pMan->aDelBlk = ctx2.aNew;
      ctx1.pMan->nDelBlk = ctx2.nNew;
      ctx1.pMan->bDelFree = 1;
      ctx2.aNew = 0;
      bcvManifestUploadParsed(p, ctx1.pMan);
      bcvExtraLogManifest(p, 
          "Manifest uploaded by cleanup to remove deleted blocks", ctx1.pMan
      );
    }
  }

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
    pNew->eMethod = SQLITE_BCV_METHOD_GET;
    pNew->xCallback = xCallback;
    pNew->pApp = pApp;
    pNew->pNext = pJob->pPending;
    pNew->pJob = pJob;
    pNew->pCurl = curl_easy_init();
    pJob->pPending = pNew;
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
const char *sqlite3_bcv_request_header(
  sqlite3_bcv_request *pReq, 
  const char *zHdr
){
  int nHdr = bcvStrlen(zHdr);
  int i;
  char *zVal = 0;

  for(i=0; i<pReq->hdr.nData; i+=1+bcvStrlen((char*)&pReq->hdr.aData[i])){
    char *z = (char*)&pReq->hdr.aData[i];
    if( 0==sqlite3_strnicmp(z, zHdr, nHdr) && z[nHdr]==':' ){
      zVal = &z[nHdr+1];
      while( bcv_isspace(zVal[0]) ) zVal++;
      break;
    }
  }
  return zVal;
}
const unsigned char *sqlite3_bcv_request_body(
  sqlite3_bcv_request *pReq, 
  int *pn
){
  *pn = pReq->body.nData;
  return pReq->body.aData;
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
      p->cb.xList(p->pCbApp, SQLITE_OK, (char*)pData);
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
        rc = pMod->mod.xOpen(pMod->pCtx, (const char**)&azField[1], 
            zUser, zSecret, zCont, &pNew->pCont, pzErr
        );
        if( rc!=SQLITE_OK ){
          sqlite3_free(pNew);
        }else{
          *ppCont = pNew;
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
          pJob->cb.xFetch(pApp, SQLITE_ERROR, 0, 0, 0);
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
  if( p->xLog ){
    char *zMsg;
    char *zRetry = 0;
    pReq->iRequestId = ++p->iNextRequestId;
    pReq->iRequestTime = sqlite_timestamp();
    if( pReq->nRetry>0 ){
      zRetry = sqlite3_mprintf(" (nRetry=%d)", pReq->nRetry);
    }
    zMsg = sqlite3_mprintf("r%d_%d [%s] %s%s %s", 
      p->iDispatchId, pReq->iRequestId,
      pReq->pJob->zLogmsg,
      pReq->eMethod==SQLITE_BCV_METHOD_GET ? "GET" :
      pReq->eMethod==SQLITE_BCV_METHOD_PUT ? "PUT" : "DELETE",
      zRetry ? zRetry : "",
      (pReq->zUriLog ? pReq->zUriLog : pReq->zUri)
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
  if( p->xLog ){
    char *zMsg = 0;
    char *zRetry = 0;
    long httpcode = 0;
    const char *zETag = 0;
    const char *zStatus = 0;
    int res = 0;
    curl_easy_getinfo(pReq->pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
    res = sqlite3_bcv_request_status(pReq, &zStatus);
    if( pReq->pJob->bLogEtag && (httpcode/100)==2 ){
      zETag = pReq->pJob->zETag;
    }
    if( bRetry ){
      zRetry = sqlite3_mprintf(" (retry in %dms)", iDelay);
    }
    zMsg = sqlite3_mprintf("r%d_%d [%s] [%lldms] (http=%d) (rc=%d) %s%s%s%s%s",
      p->iDispatchId, pReq->iRequestId, 
      pReq->pJob->zLogmsg,
      sqlite_timestamp() - pReq->iRequestTime,
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
        pJob->cb.xFetch(pApp, pJob->rc,pJob->zETag,pJob->aResult,pJob->nResult);
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


void bcvDispatchFail(
  BcvDispatch *pDisp, 
  BcvContainer *p, 
  int rc, 
  const char *zErr
){
  BcvDispatchJob *pJob;
  BcvDispatchJob *pNext;

  for(pJob=pDisp->pJobList; pJob; pJob=pNext){
    pNext = pJob->pNext;
    if( pJob->pCont==p ){
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
    p->pJobList = pJob;
    pCont->nContRef++;
  }else{
    sqlite3_free(p->zLogmsg);
  }
  p->zLogmsg = 0;
  return pJob;
}

int bcvDispatchLogmsg(BcvDispatch *p, const char *zFmt, ...){
  if( p->xLog!=0 ){
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

int bcvDispatchFetch(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zETag,
  const void *pMd5,
  void *pApp,
  void (*x)(void*, int rc, char *zETag, const u8 *aData, int nData)
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
  void (*x)(void*, int rc, char *zError)
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
  if( rc==SQLITE_OK ){
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
      break;
    case BCV_MESSAGE_VTAB_REPLY:
      bcvBufferMsgBlob(&rc, &buf, pMsg->u.vtab_r.aData, pMsg->u.vtab_r.nData);
      break;
    case BCV_MESSAGE_DETACH:
      bcvBufferMsgString(&rc, &buf, pMsg->u.detach.zName);
      break;
    case BCV_MESSAGE_HELLO:
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello.zContainer);
      bcvBufferMsgString(&rc, &buf, pMsg->u.hello.zDatabase);
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
            break;
          }
          case BCV_MESSAGE_VTAB_REPLY: {
            pMsg->u.vtab_r.aData = bcvMsgGetBlob(&aBody, &pMsg->u.vtab_r.nData);
            break;
          }
          case BCV_MESSAGE_DETACH: {
            pMsg->u.detach.zName = bcvMsgGetString(&aBody);
            break;
          }
          case BCV_MESSAGE_HELLO: {
            pMsg->u.hello.zContainer = bcvMsgGetString(&aBody);
            pMsg->u.hello.zDatabase = bcvMsgGetString(&aBody);
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

/*************************************************************************
**************************************************************************
** Beginning of AES128-OFB encryption/decryption code.
*/

/*
** Lookup tables for the AES algorithm
*/
static const u32 bcvTe0[256] = {
    0xc66363a5U, 0xf87c7c84U, 0xee777799U, 0xf67b7b8dU,
    0xfff2f20dU, 0xd66b6bbdU, 0xde6f6fb1U, 0x91c5c554U,
    0x60303050U, 0x02010103U, 0xce6767a9U, 0x562b2b7dU,
    0xe7fefe19U, 0xb5d7d762U, 0x4dababe6U, 0xec76769aU,
    0x8fcaca45U, 0x1f82829dU, 0x89c9c940U, 0xfa7d7d87U,
    0xeffafa15U, 0xb25959ebU, 0x8e4747c9U, 0xfbf0f00bU,
    0x41adadecU, 0xb3d4d467U, 0x5fa2a2fdU, 0x45afafeaU,
    0x239c9cbfU, 0x53a4a4f7U, 0xe4727296U, 0x9bc0c05bU,
    0x75b7b7c2U, 0xe1fdfd1cU, 0x3d9393aeU, 0x4c26266aU,
    0x6c36365aU, 0x7e3f3f41U, 0xf5f7f702U, 0x83cccc4fU,
    0x6834345cU, 0x51a5a5f4U, 0xd1e5e534U, 0xf9f1f108U,
    0xe2717193U, 0xabd8d873U, 0x62313153U, 0x2a15153fU,
    0x0804040cU, 0x95c7c752U, 0x46232365U, 0x9dc3c35eU,
    0x30181828U, 0x379696a1U, 0x0a05050fU, 0x2f9a9ab5U,
    0x0e070709U, 0x24121236U, 0x1b80809bU, 0xdfe2e23dU,
    0xcdebeb26U, 0x4e272769U, 0x7fb2b2cdU, 0xea75759fU,
    0x1209091bU, 0x1d83839eU, 0x582c2c74U, 0x341a1a2eU,
    0x361b1b2dU, 0xdc6e6eb2U, 0xb45a5aeeU, 0x5ba0a0fbU,
    0xa45252f6U, 0x763b3b4dU, 0xb7d6d661U, 0x7db3b3ceU,
    0x5229297bU, 0xdde3e33eU, 0x5e2f2f71U, 0x13848497U,
    0xa65353f5U, 0xb9d1d168U, 0x00000000U, 0xc1eded2cU,
    0x40202060U, 0xe3fcfc1fU, 0x79b1b1c8U, 0xb65b5bedU,
    0xd46a6abeU, 0x8dcbcb46U, 0x67bebed9U, 0x7239394bU,
    0x944a4adeU, 0x984c4cd4U, 0xb05858e8U, 0x85cfcf4aU,
    0xbbd0d06bU, 0xc5efef2aU, 0x4faaaae5U, 0xedfbfb16U,
    0x864343c5U, 0x9a4d4dd7U, 0x66333355U, 0x11858594U,
    0x8a4545cfU, 0xe9f9f910U, 0x04020206U, 0xfe7f7f81U,
    0xa05050f0U, 0x783c3c44U, 0x259f9fbaU, 0x4ba8a8e3U,
    0xa25151f3U, 0x5da3a3feU, 0x804040c0U, 0x058f8f8aU,
    0x3f9292adU, 0x219d9dbcU, 0x70383848U, 0xf1f5f504U,
    0x63bcbcdfU, 0x77b6b6c1U, 0xafdada75U, 0x42212163U,
    0x20101030U, 0xe5ffff1aU, 0xfdf3f30eU, 0xbfd2d26dU,
    0x81cdcd4cU, 0x180c0c14U, 0x26131335U, 0xc3ecec2fU,
    0xbe5f5fe1U, 0x359797a2U, 0x884444ccU, 0x2e171739U,
    0x93c4c457U, 0x55a7a7f2U, 0xfc7e7e82U, 0x7a3d3d47U,
    0xc86464acU, 0xba5d5de7U, 0x3219192bU, 0xe6737395U,
    0xc06060a0U, 0x19818198U, 0x9e4f4fd1U, 0xa3dcdc7fU,
    0x44222266U, 0x542a2a7eU, 0x3b9090abU, 0x0b888883U,
    0x8c4646caU, 0xc7eeee29U, 0x6bb8b8d3U, 0x2814143cU,
    0xa7dede79U, 0xbc5e5ee2U, 0x160b0b1dU, 0xaddbdb76U,
    0xdbe0e03bU, 0x64323256U, 0x743a3a4eU, 0x140a0a1eU,
    0x924949dbU, 0x0c06060aU, 0x4824246cU, 0xb85c5ce4U,
    0x9fc2c25dU, 0xbdd3d36eU, 0x43acacefU, 0xc46262a6U,
    0x399191a8U, 0x319595a4U, 0xd3e4e437U, 0xf279798bU,
    0xd5e7e732U, 0x8bc8c843U, 0x6e373759U, 0xda6d6db7U,
    0x018d8d8cU, 0xb1d5d564U, 0x9c4e4ed2U, 0x49a9a9e0U,
    0xd86c6cb4U, 0xac5656faU, 0xf3f4f407U, 0xcfeaea25U,
    0xca6565afU, 0xf47a7a8eU, 0x47aeaee9U, 0x10080818U,
    0x6fbabad5U, 0xf0787888U, 0x4a25256fU, 0x5c2e2e72U,
    0x381c1c24U, 0x57a6a6f1U, 0x73b4b4c7U, 0x97c6c651U,
    0xcbe8e823U, 0xa1dddd7cU, 0xe874749cU, 0x3e1f1f21U,
    0x964b4bddU, 0x61bdbddcU, 0x0d8b8b86U, 0x0f8a8a85U,
    0xe0707090U, 0x7c3e3e42U, 0x71b5b5c4U, 0xcc6666aaU,
    0x904848d8U, 0x06030305U, 0xf7f6f601U, 0x1c0e0e12U,
    0xc26161a3U, 0x6a35355fU, 0xae5757f9U, 0x69b9b9d0U,
    0x17868691U, 0x99c1c158U, 0x3a1d1d27U, 0x279e9eb9U,
    0xd9e1e138U, 0xebf8f813U, 0x2b9898b3U, 0x22111133U,
    0xd26969bbU, 0xa9d9d970U, 0x078e8e89U, 0x339494a7U,
    0x2d9b9bb6U, 0x3c1e1e22U, 0x15878792U, 0xc9e9e920U,
    0x87cece49U, 0xaa5555ffU, 0x50282878U, 0xa5dfdf7aU,
    0x038c8c8fU, 0x59a1a1f8U, 0x09898980U, 0x1a0d0d17U,
    0x65bfbfdaU, 0xd7e6e631U, 0x844242c6U, 0xd06868b8U,
    0x824141c3U, 0x299999b0U, 0x5a2d2d77U, 0x1e0f0f11U,
    0x7bb0b0cbU, 0xa85454fcU, 0x6dbbbbd6U, 0x2c16163aU,
};
static const u32 bcvTe1[256] = {
    0xa5c66363U, 0x84f87c7cU, 0x99ee7777U, 0x8df67b7bU,
    0x0dfff2f2U, 0xbdd66b6bU, 0xb1de6f6fU, 0x5491c5c5U,
    0x50603030U, 0x03020101U, 0xa9ce6767U, 0x7d562b2bU,
    0x19e7fefeU, 0x62b5d7d7U, 0xe64dababU, 0x9aec7676U,
    0x458fcacaU, 0x9d1f8282U, 0x4089c9c9U, 0x87fa7d7dU,
    0x15effafaU, 0xebb25959U, 0xc98e4747U, 0x0bfbf0f0U,
    0xec41adadU, 0x67b3d4d4U, 0xfd5fa2a2U, 0xea45afafU,
    0xbf239c9cU, 0xf753a4a4U, 0x96e47272U, 0x5b9bc0c0U,
    0xc275b7b7U, 0x1ce1fdfdU, 0xae3d9393U, 0x6a4c2626U,
    0x5a6c3636U, 0x417e3f3fU, 0x02f5f7f7U, 0x4f83ccccU,
    0x5c683434U, 0xf451a5a5U, 0x34d1e5e5U, 0x08f9f1f1U,
    0x93e27171U, 0x73abd8d8U, 0x53623131U, 0x3f2a1515U,
    0x0c080404U, 0x5295c7c7U, 0x65462323U, 0x5e9dc3c3U,
    0x28301818U, 0xa1379696U, 0x0f0a0505U, 0xb52f9a9aU,
    0x090e0707U, 0x36241212U, 0x9b1b8080U, 0x3ddfe2e2U,
    0x26cdebebU, 0x694e2727U, 0xcd7fb2b2U, 0x9fea7575U,
    0x1b120909U, 0x9e1d8383U, 0x74582c2cU, 0x2e341a1aU,
    0x2d361b1bU, 0xb2dc6e6eU, 0xeeb45a5aU, 0xfb5ba0a0U,
    0xf6a45252U, 0x4d763b3bU, 0x61b7d6d6U, 0xce7db3b3U,
    0x7b522929U, 0x3edde3e3U, 0x715e2f2fU, 0x97138484U,
    0xf5a65353U, 0x68b9d1d1U, 0x00000000U, 0x2cc1ededU,
    0x60402020U, 0x1fe3fcfcU, 0xc879b1b1U, 0xedb65b5bU,
    0xbed46a6aU, 0x468dcbcbU, 0xd967bebeU, 0x4b723939U,
    0xde944a4aU, 0xd4984c4cU, 0xe8b05858U, 0x4a85cfcfU,
    0x6bbbd0d0U, 0x2ac5efefU, 0xe54faaaaU, 0x16edfbfbU,
    0xc5864343U, 0xd79a4d4dU, 0x55663333U, 0x94118585U,
    0xcf8a4545U, 0x10e9f9f9U, 0x06040202U, 0x81fe7f7fU,
    0xf0a05050U, 0x44783c3cU, 0xba259f9fU, 0xe34ba8a8U,
    0xf3a25151U, 0xfe5da3a3U, 0xc0804040U, 0x8a058f8fU,
    0xad3f9292U, 0xbc219d9dU, 0x48703838U, 0x04f1f5f5U,
    0xdf63bcbcU, 0xc177b6b6U, 0x75afdadaU, 0x63422121U,
    0x30201010U, 0x1ae5ffffU, 0x0efdf3f3U, 0x6dbfd2d2U,
    0x4c81cdcdU, 0x14180c0cU, 0x35261313U, 0x2fc3ececU,
    0xe1be5f5fU, 0xa2359797U, 0xcc884444U, 0x392e1717U,
    0x5793c4c4U, 0xf255a7a7U, 0x82fc7e7eU, 0x477a3d3dU,
    0xacc86464U, 0xe7ba5d5dU, 0x2b321919U, 0x95e67373U,
    0xa0c06060U, 0x98198181U, 0xd19e4f4fU, 0x7fa3dcdcU,
    0x66442222U, 0x7e542a2aU, 0xab3b9090U, 0x830b8888U,
    0xca8c4646U, 0x29c7eeeeU, 0xd36bb8b8U, 0x3c281414U,
    0x79a7dedeU, 0xe2bc5e5eU, 0x1d160b0bU, 0x76addbdbU,
    0x3bdbe0e0U, 0x56643232U, 0x4e743a3aU, 0x1e140a0aU,
    0xdb924949U, 0x0a0c0606U, 0x6c482424U, 0xe4b85c5cU,
    0x5d9fc2c2U, 0x6ebdd3d3U, 0xef43acacU, 0xa6c46262U,
    0xa8399191U, 0xa4319595U, 0x37d3e4e4U, 0x8bf27979U,
    0x32d5e7e7U, 0x438bc8c8U, 0x596e3737U, 0xb7da6d6dU,
    0x8c018d8dU, 0x64b1d5d5U, 0xd29c4e4eU, 0xe049a9a9U,
    0xb4d86c6cU, 0xfaac5656U, 0x07f3f4f4U, 0x25cfeaeaU,
    0xafca6565U, 0x8ef47a7aU, 0xe947aeaeU, 0x18100808U,
    0xd56fbabaU, 0x88f07878U, 0x6f4a2525U, 0x725c2e2eU,
    0x24381c1cU, 0xf157a6a6U, 0xc773b4b4U, 0x5197c6c6U,
    0x23cbe8e8U, 0x7ca1ddddU, 0x9ce87474U, 0x213e1f1fU,
    0xdd964b4bU, 0xdc61bdbdU, 0x860d8b8bU, 0x850f8a8aU,
    0x90e07070U, 0x427c3e3eU, 0xc471b5b5U, 0xaacc6666U,
    0xd8904848U, 0x05060303U, 0x01f7f6f6U, 0x121c0e0eU,
    0xa3c26161U, 0x5f6a3535U, 0xf9ae5757U, 0xd069b9b9U,
    0x91178686U, 0x5899c1c1U, 0x273a1d1dU, 0xb9279e9eU,
    0x38d9e1e1U, 0x13ebf8f8U, 0xb32b9898U, 0x33221111U,
    0xbbd26969U, 0x70a9d9d9U, 0x89078e8eU, 0xa7339494U,
    0xb62d9b9bU, 0x223c1e1eU, 0x92158787U, 0x20c9e9e9U,
    0x4987ceceU, 0xffaa5555U, 0x78502828U, 0x7aa5dfdfU,
    0x8f038c8cU, 0xf859a1a1U, 0x80098989U, 0x171a0d0dU,
    0xda65bfbfU, 0x31d7e6e6U, 0xc6844242U, 0xb8d06868U,
    0xc3824141U, 0xb0299999U, 0x775a2d2dU, 0x111e0f0fU,
    0xcb7bb0b0U, 0xfca85454U, 0xd66dbbbbU, 0x3a2c1616U,
};
static const u32 bcvTe2[256] = {
    0x63a5c663U, 0x7c84f87cU, 0x7799ee77U, 0x7b8df67bU,
    0xf20dfff2U, 0x6bbdd66bU, 0x6fb1de6fU, 0xc55491c5U,
    0x30506030U, 0x01030201U, 0x67a9ce67U, 0x2b7d562bU,
    0xfe19e7feU, 0xd762b5d7U, 0xabe64dabU, 0x769aec76U,
    0xca458fcaU, 0x829d1f82U, 0xc94089c9U, 0x7d87fa7dU,
    0xfa15effaU, 0x59ebb259U, 0x47c98e47U, 0xf00bfbf0U,
    0xadec41adU, 0xd467b3d4U, 0xa2fd5fa2U, 0xafea45afU,
    0x9cbf239cU, 0xa4f753a4U, 0x7296e472U, 0xc05b9bc0U,
    0xb7c275b7U, 0xfd1ce1fdU, 0x93ae3d93U, 0x266a4c26U,
    0x365a6c36U, 0x3f417e3fU, 0xf702f5f7U, 0xcc4f83ccU,
    0x345c6834U, 0xa5f451a5U, 0xe534d1e5U, 0xf108f9f1U,
    0x7193e271U, 0xd873abd8U, 0x31536231U, 0x153f2a15U,
    0x040c0804U, 0xc75295c7U, 0x23654623U, 0xc35e9dc3U,
    0x18283018U, 0x96a13796U, 0x050f0a05U, 0x9ab52f9aU,
    0x07090e07U, 0x12362412U, 0x809b1b80U, 0xe23ddfe2U,
    0xeb26cdebU, 0x27694e27U, 0xb2cd7fb2U, 0x759fea75U,
    0x091b1209U, 0x839e1d83U, 0x2c74582cU, 0x1a2e341aU,
    0x1b2d361bU, 0x6eb2dc6eU, 0x5aeeb45aU, 0xa0fb5ba0U,
    0x52f6a452U, 0x3b4d763bU, 0xd661b7d6U, 0xb3ce7db3U,
    0x297b5229U, 0xe33edde3U, 0x2f715e2fU, 0x84971384U,
    0x53f5a653U, 0xd168b9d1U, 0x00000000U, 0xed2cc1edU,
    0x20604020U, 0xfc1fe3fcU, 0xb1c879b1U, 0x5bedb65bU,
    0x6abed46aU, 0xcb468dcbU, 0xbed967beU, 0x394b7239U,
    0x4ade944aU, 0x4cd4984cU, 0x58e8b058U, 0xcf4a85cfU,
    0xd06bbbd0U, 0xef2ac5efU, 0xaae54faaU, 0xfb16edfbU,
    0x43c58643U, 0x4dd79a4dU, 0x33556633U, 0x85941185U,
    0x45cf8a45U, 0xf910e9f9U, 0x02060402U, 0x7f81fe7fU,
    0x50f0a050U, 0x3c44783cU, 0x9fba259fU, 0xa8e34ba8U,
    0x51f3a251U, 0xa3fe5da3U, 0x40c08040U, 0x8f8a058fU,
    0x92ad3f92U, 0x9dbc219dU, 0x38487038U, 0xf504f1f5U,
    0xbcdf63bcU, 0xb6c177b6U, 0xda75afdaU, 0x21634221U,
    0x10302010U, 0xff1ae5ffU, 0xf30efdf3U, 0xd26dbfd2U,
    0xcd4c81cdU, 0x0c14180cU, 0x13352613U, 0xec2fc3ecU,
    0x5fe1be5fU, 0x97a23597U, 0x44cc8844U, 0x17392e17U,
    0xc45793c4U, 0xa7f255a7U, 0x7e82fc7eU, 0x3d477a3dU,
    0x64acc864U, 0x5de7ba5dU, 0x192b3219U, 0x7395e673U,
    0x60a0c060U, 0x81981981U, 0x4fd19e4fU, 0xdc7fa3dcU,
    0x22664422U, 0x2a7e542aU, 0x90ab3b90U, 0x88830b88U,
    0x46ca8c46U, 0xee29c7eeU, 0xb8d36bb8U, 0x143c2814U,
    0xde79a7deU, 0x5ee2bc5eU, 0x0b1d160bU, 0xdb76addbU,
    0xe03bdbe0U, 0x32566432U, 0x3a4e743aU, 0x0a1e140aU,
    0x49db9249U, 0x060a0c06U, 0x246c4824U, 0x5ce4b85cU,
    0xc25d9fc2U, 0xd36ebdd3U, 0xacef43acU, 0x62a6c462U,
    0x91a83991U, 0x95a43195U, 0xe437d3e4U, 0x798bf279U,
    0xe732d5e7U, 0xc8438bc8U, 0x37596e37U, 0x6db7da6dU,
    0x8d8c018dU, 0xd564b1d5U, 0x4ed29c4eU, 0xa9e049a9U,
    0x6cb4d86cU, 0x56faac56U, 0xf407f3f4U, 0xea25cfeaU,
    0x65afca65U, 0x7a8ef47aU, 0xaee947aeU, 0x08181008U,
    0xbad56fbaU, 0x7888f078U, 0x256f4a25U, 0x2e725c2eU,
    0x1c24381cU, 0xa6f157a6U, 0xb4c773b4U, 0xc65197c6U,
    0xe823cbe8U, 0xdd7ca1ddU, 0x749ce874U, 0x1f213e1fU,
    0x4bdd964bU, 0xbddc61bdU, 0x8b860d8bU, 0x8a850f8aU,
    0x7090e070U, 0x3e427c3eU, 0xb5c471b5U, 0x66aacc66U,
    0x48d89048U, 0x03050603U, 0xf601f7f6U, 0x0e121c0eU,
    0x61a3c261U, 0x355f6a35U, 0x57f9ae57U, 0xb9d069b9U,
    0x86911786U, 0xc15899c1U, 0x1d273a1dU, 0x9eb9279eU,
    0xe138d9e1U, 0xf813ebf8U, 0x98b32b98U, 0x11332211U,
    0x69bbd269U, 0xd970a9d9U, 0x8e89078eU, 0x94a73394U,
    0x9bb62d9bU, 0x1e223c1eU, 0x87921587U, 0xe920c9e9U,
    0xce4987ceU, 0x55ffaa55U, 0x28785028U, 0xdf7aa5dfU,
    0x8c8f038cU, 0xa1f859a1U, 0x89800989U, 0x0d171a0dU,
    0xbfda65bfU, 0xe631d7e6U, 0x42c68442U, 0x68b8d068U,
    0x41c38241U, 0x99b02999U, 0x2d775a2dU, 0x0f111e0fU,
    0xb0cb7bb0U, 0x54fca854U, 0xbbd66dbbU, 0x163a2c16U,
};
static const u32 bcvTe3[256] = {
    0x6363a5c6U, 0x7c7c84f8U, 0x777799eeU, 0x7b7b8df6U,
    0xf2f20dffU, 0x6b6bbdd6U, 0x6f6fb1deU, 0xc5c55491U,
    0x30305060U, 0x01010302U, 0x6767a9ceU, 0x2b2b7d56U,
    0xfefe19e7U, 0xd7d762b5U, 0xababe64dU, 0x76769aecU,
    0xcaca458fU, 0x82829d1fU, 0xc9c94089U, 0x7d7d87faU,
    0xfafa15efU, 0x5959ebb2U, 0x4747c98eU, 0xf0f00bfbU,
    0xadadec41U, 0xd4d467b3U, 0xa2a2fd5fU, 0xafafea45U,
    0x9c9cbf23U, 0xa4a4f753U, 0x727296e4U, 0xc0c05b9bU,
    0xb7b7c275U, 0xfdfd1ce1U, 0x9393ae3dU, 0x26266a4cU,
    0x36365a6cU, 0x3f3f417eU, 0xf7f702f5U, 0xcccc4f83U,
    0x34345c68U, 0xa5a5f451U, 0xe5e534d1U, 0xf1f108f9U,
    0x717193e2U, 0xd8d873abU, 0x31315362U, 0x15153f2aU,
    0x04040c08U, 0xc7c75295U, 0x23236546U, 0xc3c35e9dU,
    0x18182830U, 0x9696a137U, 0x05050f0aU, 0x9a9ab52fU,
    0x0707090eU, 0x12123624U, 0x80809b1bU, 0xe2e23ddfU,
    0xebeb26cdU, 0x2727694eU, 0xb2b2cd7fU, 0x75759feaU,
    0x09091b12U, 0x83839e1dU, 0x2c2c7458U, 0x1a1a2e34U,
    0x1b1b2d36U, 0x6e6eb2dcU, 0x5a5aeeb4U, 0xa0a0fb5bU,
    0x5252f6a4U, 0x3b3b4d76U, 0xd6d661b7U, 0xb3b3ce7dU,
    0x29297b52U, 0xe3e33eddU, 0x2f2f715eU, 0x84849713U,
    0x5353f5a6U, 0xd1d168b9U, 0x00000000U, 0xeded2cc1U,
    0x20206040U, 0xfcfc1fe3U, 0xb1b1c879U, 0x5b5bedb6U,
    0x6a6abed4U, 0xcbcb468dU, 0xbebed967U, 0x39394b72U,
    0x4a4ade94U, 0x4c4cd498U, 0x5858e8b0U, 0xcfcf4a85U,
    0xd0d06bbbU, 0xefef2ac5U, 0xaaaae54fU, 0xfbfb16edU,
    0x4343c586U, 0x4d4dd79aU, 0x33335566U, 0x85859411U,
    0x4545cf8aU, 0xf9f910e9U, 0x02020604U, 0x7f7f81feU,
    0x5050f0a0U, 0x3c3c4478U, 0x9f9fba25U, 0xa8a8e34bU,
    0x5151f3a2U, 0xa3a3fe5dU, 0x4040c080U, 0x8f8f8a05U,
    0x9292ad3fU, 0x9d9dbc21U, 0x38384870U, 0xf5f504f1U,
    0xbcbcdf63U, 0xb6b6c177U, 0xdada75afU, 0x21216342U,
    0x10103020U, 0xffff1ae5U, 0xf3f30efdU, 0xd2d26dbfU,
    0xcdcd4c81U, 0x0c0c1418U, 0x13133526U, 0xecec2fc3U,
    0x5f5fe1beU, 0x9797a235U, 0x4444cc88U, 0x1717392eU,
    0xc4c45793U, 0xa7a7f255U, 0x7e7e82fcU, 0x3d3d477aU,
    0x6464acc8U, 0x5d5de7baU, 0x19192b32U, 0x737395e6U,
    0x6060a0c0U, 0x81819819U, 0x4f4fd19eU, 0xdcdc7fa3U,
    0x22226644U, 0x2a2a7e54U, 0x9090ab3bU, 0x8888830bU,
    0x4646ca8cU, 0xeeee29c7U, 0xb8b8d36bU, 0x14143c28U,
    0xdede79a7U, 0x5e5ee2bcU, 0x0b0b1d16U, 0xdbdb76adU,
    0xe0e03bdbU, 0x32325664U, 0x3a3a4e74U, 0x0a0a1e14U,
    0x4949db92U, 0x06060a0cU, 0x24246c48U, 0x5c5ce4b8U,
    0xc2c25d9fU, 0xd3d36ebdU, 0xacacef43U, 0x6262a6c4U,
    0x9191a839U, 0x9595a431U, 0xe4e437d3U, 0x79798bf2U,
    0xe7e732d5U, 0xc8c8438bU, 0x3737596eU, 0x6d6db7daU,
    0x8d8d8c01U, 0xd5d564b1U, 0x4e4ed29cU, 0xa9a9e049U,
    0x6c6cb4d8U, 0x5656faacU, 0xf4f407f3U, 0xeaea25cfU,
    0x6565afcaU, 0x7a7a8ef4U, 0xaeaee947U, 0x08081810U,
    0xbabad56fU, 0x787888f0U, 0x25256f4aU, 0x2e2e725cU,
    0x1c1c2438U, 0xa6a6f157U, 0xb4b4c773U, 0xc6c65197U,
    0xe8e823cbU, 0xdddd7ca1U, 0x74749ce8U, 0x1f1f213eU,
    0x4b4bdd96U, 0xbdbddc61U, 0x8b8b860dU, 0x8a8a850fU,
    0x707090e0U, 0x3e3e427cU, 0xb5b5c471U, 0x6666aaccU,
    0x4848d890U, 0x03030506U, 0xf6f601f7U, 0x0e0e121cU,
    0x6161a3c2U, 0x35355f6aU, 0x5757f9aeU, 0xb9b9d069U,
    0x86869117U, 0xc1c15899U, 0x1d1d273aU, 0x9e9eb927U,
    0xe1e138d9U, 0xf8f813ebU, 0x9898b32bU, 0x11113322U,
    0x6969bbd2U, 0xd9d970a9U, 0x8e8e8907U, 0x9494a733U,
    0x9b9bb62dU, 0x1e1e223cU, 0x87879215U, 0xe9e920c9U,
    0xcece4987U, 0x5555ffaaU, 0x28287850U, 0xdfdf7aa5U,
    0x8c8c8f03U, 0xa1a1f859U, 0x89898009U, 0x0d0d171aU,
    0xbfbfda65U, 0xe6e631d7U, 0x4242c684U, 0x6868b8d0U,
    0x4141c382U, 0x9999b029U, 0x2d2d775aU, 0x0f0f111eU,
    0xb0b0cb7bU, 0x5454fca8U, 0xbbbbd66dU, 0x16163a2cU,
};
static const u32 bcvTe4[256] = {
    0x63636363U, 0x7c7c7c7cU, 0x77777777U, 0x7b7b7b7bU,
    0xf2f2f2f2U, 0x6b6b6b6bU, 0x6f6f6f6fU, 0xc5c5c5c5U,
    0x30303030U, 0x01010101U, 0x67676767U, 0x2b2b2b2bU,
    0xfefefefeU, 0xd7d7d7d7U, 0xababababU, 0x76767676U,
    0xcacacacaU, 0x82828282U, 0xc9c9c9c9U, 0x7d7d7d7dU,
    0xfafafafaU, 0x59595959U, 0x47474747U, 0xf0f0f0f0U,
    0xadadadadU, 0xd4d4d4d4U, 0xa2a2a2a2U, 0xafafafafU,
    0x9c9c9c9cU, 0xa4a4a4a4U, 0x72727272U, 0xc0c0c0c0U,
    0xb7b7b7b7U, 0xfdfdfdfdU, 0x93939393U, 0x26262626U,
    0x36363636U, 0x3f3f3f3fU, 0xf7f7f7f7U, 0xccccccccU,
    0x34343434U, 0xa5a5a5a5U, 0xe5e5e5e5U, 0xf1f1f1f1U,
    0x71717171U, 0xd8d8d8d8U, 0x31313131U, 0x15151515U,
    0x04040404U, 0xc7c7c7c7U, 0x23232323U, 0xc3c3c3c3U,
    0x18181818U, 0x96969696U, 0x05050505U, 0x9a9a9a9aU,
    0x07070707U, 0x12121212U, 0x80808080U, 0xe2e2e2e2U,
    0xebebebebU, 0x27272727U, 0xb2b2b2b2U, 0x75757575U,
    0x09090909U, 0x83838383U, 0x2c2c2c2cU, 0x1a1a1a1aU,
    0x1b1b1b1bU, 0x6e6e6e6eU, 0x5a5a5a5aU, 0xa0a0a0a0U,
    0x52525252U, 0x3b3b3b3bU, 0xd6d6d6d6U, 0xb3b3b3b3U,
    0x29292929U, 0xe3e3e3e3U, 0x2f2f2f2fU, 0x84848484U,
    0x53535353U, 0xd1d1d1d1U, 0x00000000U, 0xededededU,
    0x20202020U, 0xfcfcfcfcU, 0xb1b1b1b1U, 0x5b5b5b5bU,
    0x6a6a6a6aU, 0xcbcbcbcbU, 0xbebebebeU, 0x39393939U,
    0x4a4a4a4aU, 0x4c4c4c4cU, 0x58585858U, 0xcfcfcfcfU,
    0xd0d0d0d0U, 0xefefefefU, 0xaaaaaaaaU, 0xfbfbfbfbU,
    0x43434343U, 0x4d4d4d4dU, 0x33333333U, 0x85858585U,
    0x45454545U, 0xf9f9f9f9U, 0x02020202U, 0x7f7f7f7fU,
    0x50505050U, 0x3c3c3c3cU, 0x9f9f9f9fU, 0xa8a8a8a8U,
    0x51515151U, 0xa3a3a3a3U, 0x40404040U, 0x8f8f8f8fU,
    0x92929292U, 0x9d9d9d9dU, 0x38383838U, 0xf5f5f5f5U,
    0xbcbcbcbcU, 0xb6b6b6b6U, 0xdadadadaU, 0x21212121U,
    0x10101010U, 0xffffffffU, 0xf3f3f3f3U, 0xd2d2d2d2U,
    0xcdcdcdcdU, 0x0c0c0c0cU, 0x13131313U, 0xececececU,
    0x5f5f5f5fU, 0x97979797U, 0x44444444U, 0x17171717U,
    0xc4c4c4c4U, 0xa7a7a7a7U, 0x7e7e7e7eU, 0x3d3d3d3dU,
    0x64646464U, 0x5d5d5d5dU, 0x19191919U, 0x73737373U,
    0x60606060U, 0x81818181U, 0x4f4f4f4fU, 0xdcdcdcdcU,
    0x22222222U, 0x2a2a2a2aU, 0x90909090U, 0x88888888U,
    0x46464646U, 0xeeeeeeeeU, 0xb8b8b8b8U, 0x14141414U,
    0xdedededeU, 0x5e5e5e5eU, 0x0b0b0b0bU, 0xdbdbdbdbU,
    0xe0e0e0e0U, 0x32323232U, 0x3a3a3a3aU, 0x0a0a0a0aU,
    0x49494949U, 0x06060606U, 0x24242424U, 0x5c5c5c5cU,
    0xc2c2c2c2U, 0xd3d3d3d3U, 0xacacacacU, 0x62626262U,
    0x91919191U, 0x95959595U, 0xe4e4e4e4U, 0x79797979U,
    0xe7e7e7e7U, 0xc8c8c8c8U, 0x37373737U, 0x6d6d6d6dU,
    0x8d8d8d8dU, 0xd5d5d5d5U, 0x4e4e4e4eU, 0xa9a9a9a9U,
    0x6c6c6c6cU, 0x56565656U, 0xf4f4f4f4U, 0xeaeaeaeaU,
    0x65656565U, 0x7a7a7a7aU, 0xaeaeaeaeU, 0x08080808U,
    0xbabababaU, 0x78787878U, 0x25252525U, 0x2e2e2e2eU,
    0x1c1c1c1cU, 0xa6a6a6a6U, 0xb4b4b4b4U, 0xc6c6c6c6U,
    0xe8e8e8e8U, 0xddddddddU, 0x74747474U, 0x1f1f1f1fU,
    0x4b4b4b4bU, 0xbdbdbdbdU, 0x8b8b8b8bU, 0x8a8a8a8aU,
    0x70707070U, 0x3e3e3e3eU, 0xb5b5b5b5U, 0x66666666U,
    0x48484848U, 0x03030303U, 0xf6f6f6f6U, 0x0e0e0e0eU,
    0x61616161U, 0x35353535U, 0x57575757U, 0xb9b9b9b9U,
    0x86868686U, 0xc1c1c1c1U, 0x1d1d1d1dU, 0x9e9e9e9eU,
    0xe1e1e1e1U, 0xf8f8f8f8U, 0x98989898U, 0x11111111U,
    0x69696969U, 0xd9d9d9d9U, 0x8e8e8e8eU, 0x94949494U,
    0x9b9b9b9bU, 0x1e1e1e1eU, 0x87878787U, 0xe9e9e9e9U,
    0xcecececeU, 0x55555555U, 0x28282828U, 0xdfdfdfdfU,
    0x8c8c8c8cU, 0xa1a1a1a1U, 0x89898989U, 0x0d0d0d0dU,
    0xbfbfbfbfU, 0xe6e6e6e6U, 0x42424242U, 0x68686868U,
    0x41414141U, 0x99999999U, 0x2d2d2d2dU, 0x0f0f0f0fU,
    0xb0b0b0b0U, 0x54545454U, 0xbbbbbbbbU, 0x16161616U,
};
static const u32 bcvrcon[] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1B000000, 0x36000000,
    /* for 128-bit blocks, Rijndael never uses more than 10 bcvrcon values */
};

#define BCVGETU32(p)       (*(u32*)(p))
#define BCVPUTU32(ct, st)  { *((u32*)(ct)) = (st); }

/*
** Set up the AES key schedule given a 128-bit (16-byte) key in cipherKey[]
*/
static int bcvRijndaelKeySetupEnc128(u32 *rk, const u8 *cipherKey){
  int i = 0;
  u32 temp;

  rk[0] = BCVGETU32(cipherKey     );
  rk[1] = BCVGETU32(cipherKey +  4);
  rk[2] = BCVGETU32(cipherKey +  8);
  rk[3] = BCVGETU32(cipherKey + 12);
  for (;;) {
    temp  = rk[3];
    rk[4] = rk[0] ^
      (bcvTe4[(temp >> 16) & 0xff] & 0xff000000) ^
      (bcvTe4[(temp >>  8) & 0xff] & 0x00ff0000) ^
      (bcvTe4[(temp      ) & 0xff] & 0x0000ff00) ^
      (bcvTe4[(temp >> 24)       ] & 0x000000ff) ^
      bcvrcon[i];
    rk[5] = rk[1] ^ rk[4];
    rk[6] = rk[2] ^ rk[5];
    rk[7] = rk[3] ^ rk[6];
    if (++i == 10) {
      return 10;
    }
    rk += 4;
  }
}

/*
** Encrypt a single block pt[].  Store the output in ct[].
** 10 rounds.  Block size 128 bits.
*/
static void bcvRijndaelEncrypt128(const u32 *rk, const u8 pt[16], u8 ct[16]) {
  u32 s0, s1, s2, s3, t0, t1, t2, t3;

  /*
   * map byte array block to cipher state
   * and add initial round key:
   */
  s0 = BCVGETU32(pt     ) ^ rk[0];
  s1 = BCVGETU32(pt +  4) ^ rk[1];
  s2 = BCVGETU32(pt +  8) ^ rk[2];
  s3 = BCVGETU32(pt + 12) ^ rk[3];
  /* round 1: */
  t0 = bcvTe0[s0>>24]^bcvTe1[(s1>>16)&0xff]^bcvTe2[(s2>> 8)&0xff]^bcvTe3[s3&0xff]^rk[ 4];
  t1 = bcvTe0[s1>>24]^bcvTe1[(s2>>16)&0xff]^bcvTe2[(s3>> 8)&0xff]^bcvTe3[s0&0xff]^rk[ 5];
  t2 = bcvTe0[s2>>24]^bcvTe1[(s3>>16)&0xff]^bcvTe2[(s0>> 8)&0xff]^bcvTe3[s1&0xff]^rk[ 6];
  t3 = bcvTe0[s3>>24]^bcvTe1[(s0>>16)&0xff]^bcvTe2[(s1>> 8)&0xff]^bcvTe3[s2&0xff]^rk[ 7];
  /* round 2: */
  s0 = bcvTe0[t0>>24]^bcvTe1[(t1>>16)&0xff]^bcvTe2[(t2>> 8)&0xff]^bcvTe3[t3&0xff]^rk[ 8];
  s1 = bcvTe0[t1>>24]^bcvTe1[(t2>>16)&0xff]^bcvTe2[(t3>> 8)&0xff]^bcvTe3[t0&0xff]^rk[ 9];
  s2 = bcvTe0[t2>>24]^bcvTe1[(t3>>16)&0xff]^bcvTe2[(t0>> 8)&0xff]^bcvTe3[t1&0xff]^rk[10];
  s3 = bcvTe0[t3>>24]^bcvTe1[(t0>>16)&0xff]^bcvTe2[(t1>> 8)&0xff]^bcvTe3[t2&0xff]^rk[11];
  /* round 3: */
  t0 = bcvTe0[s0>>24]^bcvTe1[(s1>>16)&0xff]^bcvTe2[(s2>> 8)&0xff]^bcvTe3[s3&0xff]^rk[12];
  t1 = bcvTe0[s1>>24]^bcvTe1[(s2>>16)&0xff]^bcvTe2[(s3>> 8)&0xff]^bcvTe3[s0&0xff]^rk[13];
  t2 = bcvTe0[s2>>24]^bcvTe1[(s3>>16)&0xff]^bcvTe2[(s0>> 8)&0xff]^bcvTe3[s1&0xff]^rk[14];
  t3 = bcvTe0[s3>>24]^bcvTe1[(s0>>16)&0xff]^bcvTe2[(s1>> 8)&0xff]^bcvTe3[s2&0xff]^rk[15];
  /* round 4: */
  s0 = bcvTe0[t0>>24]^bcvTe1[(t1>>16)&0xff]^bcvTe2[(t2>> 8)&0xff]^bcvTe3[t3&0xff]^rk[16];
  s1 = bcvTe0[t1>>24]^bcvTe1[(t2>>16)&0xff]^bcvTe2[(t3>> 8)&0xff]^bcvTe3[t0&0xff]^rk[17];
  s2 = bcvTe0[t2>>24]^bcvTe1[(t3>>16)&0xff]^bcvTe2[(t0>> 8)&0xff]^bcvTe3[t1&0xff]^rk[18];
  s3 = bcvTe0[t3>>24]^bcvTe1[(t0>>16)&0xff]^bcvTe2[(t1>> 8)&0xff]^bcvTe3[t2&0xff]^rk[19];
  /* round 5: */
  t0 = bcvTe0[s0>>24]^bcvTe1[(s1>>16)&0xff]^bcvTe2[(s2>> 8)&0xff]^bcvTe3[s3&0xff]^rk[20];
  t1 = bcvTe0[s1>>24]^bcvTe1[(s2>>16)&0xff]^bcvTe2[(s3>> 8)&0xff]^bcvTe3[s0&0xff]^rk[21];
  t2 = bcvTe0[s2>>24]^bcvTe1[(s3>>16)&0xff]^bcvTe2[(s0>> 8)&0xff]^bcvTe3[s1&0xff]^rk[22];
  t3 = bcvTe0[s3>>24]^bcvTe1[(s0>>16)&0xff]^bcvTe2[(s1>> 8)&0xff]^bcvTe3[s2&0xff]^rk[23];
  /* round 6: */
  s0 = bcvTe0[t0>>24]^bcvTe1[(t1>>16)&0xff]^bcvTe2[(t2>> 8)&0xff]^bcvTe3[t3&0xff]^rk[24];
  s1 = bcvTe0[t1>>24]^bcvTe1[(t2>>16)&0xff]^bcvTe2[(t3>> 8)&0xff]^bcvTe3[t0&0xff]^rk[25];
  s2 = bcvTe0[t2>>24]^bcvTe1[(t3>>16)&0xff]^bcvTe2[(t0>> 8)&0xff]^bcvTe3[t1&0xff]^rk[26];
  s3 = bcvTe0[t3>>24]^bcvTe1[(t0>>16)&0xff]^bcvTe2[(t1>> 8)&0xff]^bcvTe3[t2&0xff]^rk[27];
  /* round 7: */
  t0 = bcvTe0[s0>>24]^bcvTe1[(s1>>16)&0xff]^bcvTe2[(s2>> 8)&0xff]^bcvTe3[s3&0xff]^rk[28];
  t1 = bcvTe0[s1>>24]^bcvTe1[(s2>>16)&0xff]^bcvTe2[(s3>> 8)&0xff]^bcvTe3[s0&0xff]^rk[29];
  t2 = bcvTe0[s2>>24]^bcvTe1[(s3>>16)&0xff]^bcvTe2[(s0>> 8)&0xff]^bcvTe3[s1&0xff]^rk[30];
  t3 = bcvTe0[s3>>24]^bcvTe1[(s0>>16)&0xff]^bcvTe2[(s1>> 8)&0xff]^bcvTe3[s2&0xff]^rk[31];
  /* round 8: */
  s0 = bcvTe0[t0>>24]^bcvTe1[(t1>>16)&0xff]^bcvTe2[(t2>> 8)&0xff]^bcvTe3[t3&0xff]^rk[32];
  s1 = bcvTe0[t1>>24]^bcvTe1[(t2>>16)&0xff]^bcvTe2[(t3>> 8)&0xff]^bcvTe3[t0&0xff]^rk[33];
  s2 = bcvTe0[t2>>24]^bcvTe1[(t3>>16)&0xff]^bcvTe2[(t0>> 8)&0xff]^bcvTe3[t1&0xff]^rk[34];
  s3 = bcvTe0[t3>>24]^bcvTe1[(t0>>16)&0xff]^bcvTe2[(t1>> 8)&0xff]^bcvTe3[t2&0xff]^rk[35];
  /* round 9: */
  t0 = bcvTe0[s0>>24]^bcvTe1[(s1>>16)&0xff]^bcvTe2[(s2>> 8)&0xff]^bcvTe3[s3&0xff]^rk[36];
  t1 = bcvTe0[s1>>24]^bcvTe1[(s2>>16)&0xff]^bcvTe2[(s3>> 8)&0xff]^bcvTe3[s0&0xff]^rk[37];
  t2 = bcvTe0[s2>>24]^bcvTe1[(s3>>16)&0xff]^bcvTe2[(s0>> 8)&0xff]^bcvTe3[s1&0xff]^rk[38];
  t3 = bcvTe0[s3>>24]^bcvTe1[(s0>>16)&0xff]^bcvTe2[(s1>> 8)&0xff]^bcvTe3[s2&0xff]^rk[39];
  rk += 40;
  /*
   * apply last round and
   * map cipher state to byte array block:
   */
  s0 =
    (bcvTe4[(t0 >> 24)       ] & 0xff000000) ^
    (bcvTe4[(t1 >> 16) & 0xff] & 0x00ff0000) ^
    (bcvTe4[(t2 >>  8) & 0xff] & 0x0000ff00) ^
    (bcvTe4[(t3      ) & 0xff] & 0x000000ff) ^
    rk[0];
  BCVPUTU32(ct     , s0);
  s1 =
    (bcvTe4[(t1 >> 24)       ] & 0xff000000) ^
    (bcvTe4[(t2 >> 16) & 0xff] & 0x00ff0000) ^
    (bcvTe4[(t3 >>  8) & 0xff] & 0x0000ff00) ^
    (bcvTe4[(t0      ) & 0xff] & 0x000000ff) ^
    rk[1];
  BCVPUTU32(ct +  4, s1);
  s2 =
    (bcvTe4[(t2 >> 24)       ] & 0xff000000) ^
    (bcvTe4[(t3 >> 16) & 0xff] & 0x00ff0000) ^
    (bcvTe4[(t0 >>  8) & 0xff] & 0x0000ff00) ^
    (bcvTe4[(t1      ) & 0xff] & 0x000000ff) ^
    rk[2];
  BCVPUTU32(ct +  8, s2);
  s3 =
    (bcvTe4[(t3 >> 24)       ] & 0xff000000) ^
    (bcvTe4[(t0 >> 16) & 0xff] & 0x00ff0000) ^
    (bcvTe4[(t1 >>  8) & 0xff] & 0x0000ff00) ^
    (bcvTe4[(t2      ) & 0xff] & 0x000000ff) ^
    rk[3];
  BCVPUTU32(ct + 12, s3);
}

/*
** XOR buffer b into buffer a.
*/
static void bcvXorBuffers(u8 *out, u8 *a, u8 *b, int nByte){
  if( sizeof(a)==8
   && (out - (u8*)0)%8==0 
   && (a - (u8*)0)%8==0
   && (b - (u8*)0)%8==0
  ){
    u64 *x = (u64*)out;
    u64 *y = (u64*)a;
    u64 *z = (u64*)b;
    while( nByte>8 ){
      *(x++) = *(y++) ^ *(z++);
      nByte -= 8;
    }
    out = (u8*)x;
    a = (u8*)y;
    b = (u8*)z;
  }
  if( (a - (unsigned char*)0)%4==0
   && (b - (unsigned char*)0)%4==0
  ){
    u32 *x = (u32*)out;
    u32 *y = (u32*)a;
    u32 *z = (u32*)b;
    while( nByte>4 ){
      *(x++) = *(y++) ^ *(z++);
      nByte -= 4;
    }
    out = (u8*)x;
    a = (u8*)y;
    b = (u8*)z;
  }
  while( nByte-- > 0 ){
    *(out++) = *(a++) ^ *(b++);
  }
}

#define BCV_KEY_SCHED_SZ 44

struct BcvEncryptionKey {
  u32 aKeySchedule[BCV_KEY_SCHED_SZ];
  u8 *aMask;                      /* Buffer to assemble mask in */
  int nMask;                      /* Size of buffer nMask in bytes */
  int nRef;                       /* Number of pointers to this key */
};

BcvEncryptionKey *bcvEncryptionKeyNew(const unsigned char *aKey){
  BcvEncryptionKey *pNew;
  pNew = (BcvEncryptionKey*)sqlite3_malloc(sizeof(BcvEncryptionKey));
  if( pNew ){
    memset(pNew, 0, sizeof(BcvEncryptionKey));
    bcvRijndaelKeySetupEnc128(pNew->aKeySchedule, aKey);
    pNew->nRef = 1;
  }
  return pNew;
}

BcvEncryptionKey *bcvEncryptionKeyRef(BcvEncryptionKey *pKey){
  pKey->nRef++;
  return pKey;
}

void bcvEncryptionKeyFree(BcvEncryptionKey *pKey){
  if( pKey ){
    if( pKey->nRef==1 ){
      sqlite3_free(pKey->aMask);
      sqlite3_free(pKey);
    }else{
      pKey->nRef--;
    }
  }
}

/*
** Encrypt buffer aData (size nData bytes) using key pKey. The nonce to
** use is specified by variables iNonce and aNonce, which points to a
** 16-byte (BCV_LOCAL_KEYSIZE byte) buffer.
*/
int bcvEncrypt(
  BcvEncryptionKey *pKey,         /* Key to encrypt with */
  i64 iNonce,                     /* 64 bit integer nonce value */
  u8 *aNonce,                     /* 16 byte nonce value */
  unsigned char *aData, int nData /* Buffer to encrypt */
){
  int ii;
  i64 aLocal[2];
  u8 *aMask;

  assert( (nData % 16)==0 );
  assert( ((aNonce - (u8*)0) % 4)==0 );

  if( nData>pKey->nMask ){
    sqlite3_free(pKey->aMask);
    pKey->nMask = 0;
    pKey->aMask = sqlite3_malloc(nData);
    if( pKey->aMask==0 ) return SQLITE_NOMEM;
    pKey->nMask = nData;
  }
  aMask = pKey->aMask;

 // memcpy(aLocal, aNonce, 16);
  memset(aLocal, 0, sizeof(aLocal));
  aLocal[1] ^= (u64)iNonce;
  bcvRijndaelEncrypt128(pKey->aKeySchedule, (const u8*)aLocal, aMask);
  for(ii=16; ii<nData; ii+=16){
    bcvRijndaelEncrypt128(pKey->aKeySchedule, &aMask[ii-16], &aMask[ii]);
  }

  bcvXorBuffers(aData, aData, aMask, nData);
  return SQLITE_OK;
}

int bcvDecrypt(
  BcvEncryptionKey *pKey, 
  i64 iNonce,
  u8 *aNonce,
  unsigned char *aData, int nData
){
  return bcvEncrypt(pKey, iNonce, aNonce, aData, nData);
}




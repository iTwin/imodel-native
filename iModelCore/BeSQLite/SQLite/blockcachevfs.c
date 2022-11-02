

#if !defined(__WIN32__) && (defined(WIN32) || defined(_WIN32))
# define __WIN32__
#endif

#include "sqlite3.h"
#include "blockcachevfs.h"
#include "bcv_int.h"

#include <string.h>
#include <assert.h>

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

typedef struct BcvFileCache BcvFileCache;
typedef struct BcvKVStore BcvKVStore;
typedef struct BcvfsFile BcvfsFile;
typedef struct BcvMRULink BcvMRULink;
typedef struct BcvProxyFile BcvProxyFile;
typedef struct BcvWrapper BcvWrapper;
typedef struct CacheEntry CacheEntry;
typedef struct Container Container;
typedef struct BcvDbPath BcvDbPath;
typedef struct Mutex Mutex;

#define BCV_POLL_OPTION         "bcv_poll"
#define BCV_AUTH_OPTION         "bcv_auth"
#define BCV_DEFAULT_POLL_OPTION 0

#define BCV_DEFAULT_CACHESIZE   (1024*1024*1024)
#define BCV_DEFAULT_NREQUEST    10

#define BCVFS_MAX_AUTH_RETRIES 10

#define WAL_CHECKPOINTER_LOCK 1

#define BCV_PORTNUMBER_FILE "portnumber.bcv"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifdef __WIN32__
# include <windows.h>
# include <direct.h>
# define BCV_PATH_SEPARATOR "\\"
# define osMkdir(x,y) mkdir(x)
#else
# include <pthread.h>
# define BCV_PATH_SEPARATOR "/"
# define osMkdir(x,y) mkdir(x,y)
#endif

#define BCV_FCNTL_FD 82460214


/*
** Mutex/condition type. This type and its methods encapsulate the 
** platform dependent thread-related things in this file. Methods are:
**
**   bcvfsMutexInit()
**   bcvfsMutexShutdown()
**   bcvfsMutexEnter()
**   bcvfsMutexLeave()
**   bcvfsMutexCondWait()
**   bcvfsMutexCondSignal()
*/
struct Mutex {
  int bHeld;                      /* True if mutex is held */
#ifdef __WIN32__
  CRITICAL_SECTION mut;
  CONDITION_VARIABLE cond;
#else
  pthread_mutex_t mut;
  pthread_cond_t cond;
#endif
};

/*
** An array of structures of this type is maintained by proxy VFS clients
** while reading from the database. See comments above BcvProxyFile for
** details.
*/
struct BcvMRULink {
  int iNext;
  int iPrev;
};

/*
** aMRU:
**   Each time a new cache-file map is obtained from the daemon (either
**   at the start of a read transaction or because a missing block was
**   requested) a new, zeroed, array containing one entry for each block
**   in the database is allocated here. 
**
**   As the read operation progresses, the elements of the array are organized
**   into a doubly-linked list in order from most-recently to least-recently
**   used. Integer values BcvMRULink.iNext and BcvMRULink.iPrev are used
**   in place of pointers - 0 is a null pointer, any value X greater than
**   0 is interpreted as a pointer to aMRU[X-1].
**
** iMRU:
**   aMRU style "pointer" to element the of aMRU representing the most 
**   recently used block. A value of 0 is a null pointer, any value X greater
**   than 0 is interpreted as a pointer to aMRU[X-1].
*/
struct BcvProxyFile {
  BCV_SOCKET_TYPE fdProxy;
  char *zStorage;
  char *zAccount;
  char *zContainer;
  int szBlk;
  BcvMessage *pMap;
  BcvMRULink *aMRU;               /* Array of pMap->u.read_r.nBlk elements */
  int iMRU;                       /* Index of most recently used block */
  char *zAuth;
  BcvEncryptionKey *pKey;         /* Encryption key, if any */
};

struct BcvKVStore {
  char *zETag;
  sqlite3 *db;
};

/*
** File object for this VFS. This VFS handles 3 separate types of files:
**
**   Database files  - (pPath!=0 && pCacheFile!=0)
**   Wal files       - (pPath!=0 && pCacheFile==0)
**   Pass thru files - (pPath==0 && pCacheFile==0)
**
** The final category includes all files that are not cloud database
** files or their associated local wal files.
**
** pFile:
**   All file types - temp files, wal files and database files - store a
**   pointer to the "real" file-handle here.
**
** zFile:
**   Actual file name for BcvfsFile.pFile.
**
** lockMask:
**   Mask of shm-locks held by the connetion. If either an exclusive or
**   shared lock is held on a locking-slot, the corresponding bit is set
**   in this variable. There is no distinction made between shared and
**   exclusive locks, only between locked and unlocked.
**
** bHoldCkpt:
**   If set, do not release the CHECKPOINTER lock in xShmLock when 
**   requested to. This is used as part of uploading databases.
*/
struct BcvfsFile {
  sqlite3_file base;
  sqlite3 **ppDb;                 /* From SQLITE_FCNTL_PDB */
  sqlite3_bcvfs *pFs;             /* VFS that owns this object */
  sqlite3_file *pFile;            /* Open file descriptor */
  char *zLocalPath;               /* Full local path for pFile */
  int bIsMain;                    /* True if opened with MAIN_DB */
  BcvDbPath *pPath;
  sqlite3_file *pCacheFile;

  /* Used in proxy mode only */
  BcvProxyFile p;

  i64 iFileDbId;                  /* Database id for db and wal files */
  Container *pCont;               /* VFS-wide container object */

  Manifest *pMan;                 /* When transaction is open, manifest */
  ManifestDb *pManDb;             /* When transaction is open, manifest-db */
  u32 lockMask;                   /* Mask of shm-locks currently held */
  int bHoldCkpt;                  /* True to not release CHECKPOINTER lock */

  BcvKVStore kv;

  /* Used when fetching a block - see bcvfsFetchBlock(). And by
  ** sqlite3_bcvfs_upload(). */
  CacheEntry *pEntry;
  int rc;

  BcvfsFile *pNextFile;
};

#define BCVFS_PASSTHRU_FILE  0
#define BCVFS_WAL_FILE       1
#define BCVFS_DATABASE_FILE  2
#define BCVFS_NO_FILE        3
#define BCVFS_PROXY_FILE     4

/*
** Macros to enter and leave the VFS mutex.
*/
#define ENTER_VFS_MUTEX (bcvfsMutexEnter(&pFs->mutex))
#define LEAVE_VFS_MUTEX (bcvfsMutexLeave(&pFs->mutex))

#define CHECK_VFS_MUTEX (assert( pFs->mutex.bHeld ))

#define BCVFS_FIRST_LOCAL_ID (i64)(0xFFFFFFFF - 1000000)

/*
** VFS handle.
**
** iNextLocalId:
**   Next id value to assign to a local db created by sqlite3_bcvfs_copy().
**
** pFileList:
**   This is always NULL if the VFS is a proxy VFS. Otherwise, it is the
**   head of a linked list containing all file handles open on database
**   files.
*/
struct sqlite3_bcvfs {
  sqlite3_vfs base;
  int nRef;                       /* Number of connected clients+prefetchers */
  char *zName;                    /* Name of VFS created */
  char *zPortnumber;              /* Where to connect, for a proxy VFS */
  char *zCacheFile;               /* Full path to cache file */
  int nRequest;                   /* SQLITE_BCV_NREQUEST value */
  int nHttpTimeout;               /* SQLITE_BCV_HTTPTIMEOUT value */

  i64 iNextLocalId;
  BcvfsFile *pFileList;

  int nOnDemand;                  /* Current outstanding on-demand requests */

  /* Log callback */
  void *pLogCtx;
  int mLog;
  void(*xLog)(void*, int, const char*);
  int bCurlVerbose;

  /* Authentication callback */
  void *pAuthCtx;
  int(*xAuth)(void*, const char*, const char*, const char*, char**);

  BcvCommon c;                    /* Hash tables etc. */
  Mutex mutex;                    /* Mutex/condition variable object */
};

struct BcvDbPath {
  char *zContainer;
  char *zDatabase;
};


#define BCV_DATABASE_SCHEMA \
  "CREATE TABLE IF NOT EXISTS block(\n"                 \
  "    cachefilepos INTEGER PRIMARY KEY,\n"             \
  "    blockid BLOB,\n"                                 \
  "    container TEXT,\n"                               \
  "    db INTEGER,\n"                                   \
  "    dbpos INTEGER,\n"                                \
  "    dbversion INTEGER,\n"                            \
  "    CHECK (blockid IS NOT NULL OR container IS NOT NULL)" \
  ");" \
  "CREATE TABLE IF NOT EXISTS container(\n"             \
  "    name TEXT PRIMARY KEY,\n"                        \
  "    storage TEXT,\n"                                 \
  "    user TEXT,\n"                                    \
  "    container TEXT,\n"                               \
  "    manifest BLOB,\n"                                \
  "    etag \n"                                         \
  ");" \
  "CREATE TABLE IF NOT EXISTS config("                  \
  "    k TEXT PRIMARY KEY,\n"                           \
  "    v \n"                                            \
  ");" \
  "CREATE TABLE IF NOT EXISTS pinned("                  \
  "    container TEXT,\n"                               \
  "    database TEXT,\n"                                \
  "    PRIMARY KEY(container, database) \n"             \
  ");"


/*
**   bcvfsMutexInit()
**   bcvfsMutexShutdown()
**   bcvfsMutexEnter()
**   bcvfsMutexLeave()
**   bcvfsMutexCondWait()
**   bcvfsMutexCondSignal()
*/

/*
** Initialize a new Mutex object.
*/
static int bcvfsMutexInit(Mutex *p){
  memset(p, 0, sizeof(Mutex));
#ifdef __WIN32__
  InitializeCriticalSection(&p->mut);
  InitializeConditionVariable(&p->cond);
#else
  pthread_mutex_init(&p->mut, 0);
  pthread_cond_init(&p->cond, 0);
#endif
  return SQLITE_OK;
}

/*
** Shutdown a mutex object.
*/
static void bcvfsMutexShutdown(Mutex *p){
#ifdef __WIN32__
  DeleteCriticalSection(&p->mut);
#else
  pthread_mutex_destroy(&p->mut);
  pthread_cond_destroy(&p->cond);
#endif
  memset(p, 0, sizeof(Mutex));
}

/*
** Enter the mutex.
*/
static void bcvfsMutexEnter(Mutex *p){
#ifdef __WIN32__
  EnterCriticalSection(&p->mut);
#else
  pthread_mutex_lock(&p->mut);
#endif
  assert( p->bHeld==0 );
  p->bHeld = 1;
}

/*
** Leave the mutex.
*/
static void bcvfsMutexLeave(Mutex *p){
  assert( p->bHeld );
  p->bHeld = 0;
#ifdef __WIN32__
  LeaveCriticalSection(&p->mut);
#else
  pthread_mutex_unlock(&p->mut);
#endif
}

/*
** The mutex must be held when this function is called. It releases the
** mutex and then blocks, waiting for the condition to by signalled by
** a call to bcvfsMutexCondSignal.
*/
static void bcvfsMutexCondWait(Mutex *p){
  assert( p->bHeld );
  p->bHeld = 0;
#ifdef __WIN32__
  SleepConditionVariableCS(&p->cond, &p->mut, INFINITE);
#else
  pthread_cond_wait(&p->cond, &p->mut);
#endif
  p->bHeld = 1;
}

/*
** Unblock all threads waiting in bcvfsMutexCondWait().
*/
static void bcvfsMutexCondSignal(Mutex *p){
#ifdef __WIN32__
  WakeAllConditionVariable(&p->cond);
#else
  pthread_cond_broadcast(&p->cond);
#endif
}


/*
** Return the type of a file object.
*/
static int bcvfsFileType(BcvfsFile *pFile){
  assert( (pFile->pPath==0 && pFile->pCacheFile==0)
       || (pFile->pPath!=0 && pFile->pCacheFile==0)
       || (pFile->pPath!=0 && pFile->pCacheFile!=0)
  );
  if( pFile->pFile==0 ) return BCVFS_NO_FILE;
  if( pFile->pPath==0 ) return BCVFS_PASSTHRU_FILE;
  if( pFile->pCacheFile==0 ) return BCVFS_WAL_FILE;
  if( pFile->p.zStorage ) return BCVFS_PROXY_FILE;
  return BCVFS_DATABASE_FILE;
}

static int bcvfsIsSafeChar(char c){
  if( c>='0' && c<='9' ) return 1;
  if( c>='a' && c<='z' ) return 1;
  if( c>='A' && c<='Z' ) return 1;
  if( c=='_' || c=='-' || c=='.' ) return 1;
  return 0;
}

static void bcvfsPutText(char **pz, const char *zIn){
  int nIn = bcvStrlen(zIn);
  char *z = *pz;
  memcpy(z, zIn, nIn+1);
  *pz = &z[nIn];
}

static void bcvfsPutFormat(char **pz, const char *zFmt, ...){
  char *z = *pz;
  va_list ap;
  va_start(ap, zFmt);
  sqlite3_vsnprintf(100000, z, zFmt, ap);
  z += strlen(z);
  *pz = z;
  va_end(ap);
}

static void bcvfsPutEscaped(char **pz, const char *zIn){
  const char aHex[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
  };
  char *pOut = *pz;
  const char *pIn;

  for(pIn=zIn; *pIn; pIn++){
    char c = *pIn;
    if( bcvfsIsSafeChar(c) ){
      *pOut++ = c;
    }else{
      *pOut++ = '%';
      *pOut++ = aHex[(c>>4) & 0x0F];
      *pOut++ = aHex[c & 0x0F];
    }
  }

  *pOut = '\0';
  *pz = pOut;
}

/*
** Attempt to decompose the path in zName into its components and allocate
** and return the corresponding BcvDbPath object. There are three forms that
** the path may take:
**
**     /
**     /CONTAINER
**     /CONTAINER/DATABASE
**
** If successful, set (*pp) to point to a new object and return SQLITE_OK.
** Otherwise, if an error occurs, set (*pp) to NULL and return an SQLite
** error code.
*/
static int bcvfsDecodeName(
  sqlite3_bcvfs *pFs, 
  const char *zName, 
  BcvDbPath **pp
){
  int rc = SQLITE_OK;
  int nCopy = bcvStrlen(zName);
  int nAlloc;
  BcvDbPath *pRet = 0;

  nAlloc = sizeof(BcvDbPath) + nCopy+1 + bcvStrlen(pFs->c.zDir) + nCopy*3+1;
  pRet = (BcvDbPath*)bcvMallocRc(&rc, nAlloc);
  if( pRet ){
    char *z = (char*)&pRet[1];
    memcpy(z, zName, nCopy);
    if( *z!='/' && *z!='\\' ){
      rc = SQLITE_ERROR;
    }else{
      z++;
      if( *z ){
        /* not form 1 - "/" */
        pRet->zContainer = z;
        while( *z!='\0' && *z!='/' && *z!='\\' ) z++;
        if( *z!='\0' ){
          /* not form 2 - "/CONTAINER" */
          *z = '\0';
          z++;
          pRet->zDatabase = z;
          while( *z!='\0' && *z!='/' && *z!='\\' ) z++;
          if( *z!='\0' ) rc = SQLITE_ERROR;
        }
        z++;
      }
    }

    if( rc!=SQLITE_OK ){
      sqlite3_free(pRet);
      pRet =0;
    }
  }
  *pp = pRet;
  return rc;
}

static void bcvfsMaskLog(sqlite3_bcvfs *pFs, u32 mask, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  if( pFs->mLog & mask ){
    char *zMsg = sqlite3_vmprintf(zFmt, ap);
    pFs->xLog(pFs->pLogCtx, mask, zMsg);
    sqlite3_free(zMsg);
  }
  va_end(ap);
}

#define bcvfsUploadLog(pFs, zFmt, ...) \
      bcvfsMaskLog(pFs, SQLITE_BCV_LOG_UPLOAD, zFmt, __VA_ARGS__)

#define bcvfsCleanupLog(pFs, zFmt, ...) \
      bcvfsMaskLog(pFs, SQLITE_BCV_LOG_CLEANUP, zFmt, __VA_ARGS__)

#define bcvfsEventLog(pFs, zFmt, ...) \
      bcvfsMaskLog(pFs, SQLITE_BCV_LOG_EVENT, zFmt, __VA_ARGS__)

/*
** Type passed between bcvfsFetchManifest() and callback function 
** bcvfsFetchManifestCb() via the (void*) context pointer. 
*/
typedef struct ManifestCb ManifestCb;
struct ManifestCb {
  int rc;
  char *zErr;
  Manifest *pMan;
};

/*
** Fetch callback used by bcvfsFetchManifest().
*/
static void bcvfsFetchManifestCb(
  void *pCtx, 
  int rc, char *zETag, 
  const u8 *aData, int nData
){
  ManifestCb *pCb = (ManifestCb*)pCtx;
  char *zErr = 0;
  if( rc==SQLITE_OK ){
    rc = bcvManifestParseCopy(aData, nData, zETag, &pCb->pMan, &zErr);
  }else if( zETag ){
    zErr = sqlite3_mprintf("%s", zETag);
  }
  assert( pCb->rc==SQLITE_OK && pCb->zErr==0 );
  pCb->rc = rc;
  pCb->zErr = zErr;
}

/*
** Use the pDisp/pBcv dispatcher/container pair to download the manifest
** file. If successful, deserialize it, store the result in output 
** variable (*ppMan) and return SQLITE_OK.
**
** Or, if an error occurs, set (*ppMan) to NULL and return either an SQLite 
** or HTTPS error code. In this case, if pzErr is not NULL, then (*pzErr)
** may also be set to point to a buffer containing an error message. It
** is the responsibility fo the caller to eventually free this buffer 
** using sqlite3_free().
*/
static int bcvfsFetchManifest(
  BcvDispatch *pDisp,
  BcvContainer *pBcv,
  Manifest **ppMan,
  char **pzErr
){
  ManifestCb cb = {0, 0, 0};
  int rc = bcvDispatchFetch(
      pDisp, pBcv, BCV_MANIFEST_FILE, 0, 0, (void*)&cb, bcvfsFetchManifestCb
  );
  if( rc==SQLITE_OK ){
    rc = bcvDispatchRunAll(pDisp);
  }
  if( cb.pMan==0 && rc==SQLITE_OK ){ 
    rc = cb.rc;
  }
  assert( cb.pMan || rc!=SQLITE_OK );
  *ppMan = cb.pMan;
  *pzErr = cb.zErr;
  return rc;
}

/*
** Type passed between bcvfsFetchManifest() and callback function 
** bcvfsFetchManifestCb() via the (void*) context pointer. 
*/
typedef struct BlockCb BlockCb;
struct BlockCb {
  int rc;
  BcvfsFile *pFile;
  CacheEntry *pEntry;
};

/*
** The first argument points to a NAMEBYTES byte buffer
** containing a block-id. Format the block-id as text and write it to
** buffer aBuf[], which must be at leat BCV_FILESYSTEM_BLOCKID_BYTES
** in size.
*/
void bcvfsBlockidToText(const u8 *pBlk, int nBlk, char *aBuf){
  hex_encode(pBlk, nBlk, aBuf, 1);
  memcpy(&aBuf[nBlk*2], ".bcv", 5);
}

/*
** Fetch callback used by bcvfsFetchBlock().
*/
static void bcvfsFetchBlockCb(
  void *pCtx, 
  int rc, char *zETag, 
  const u8 *aData, int nData
){
  BcvfsFile *pFile = (BcvfsFile*)pCtx;
  if( rc==SQLITE_OK ){
    i64 iOff = (pFile->pEntry->iPos * pFile->pFs->c.szBlk);
    rc = bcvWritefile(pFile->pCacheFile, aData, nData, iOff);
  }else{
    /* TODO: log the error somehow */
  }
  pFile->rc = rc;
}

/*
** Download the block specified by pEntry->aName to cache-file slot
** pEntry->iPos. Return SQLITE_OK if successful, or an error code 
** otherwise.
*/
static int bcvfsFetchBlock(
  BcvfsFile *pFile,
  BcvDispatch *pDisp, 
  BcvContainer *pBcv, 
  int iBlk,
  CacheEntry *pEntry
){
  sqlite3_bcvfs *pFs = pFile->pFs;
  int rc = SQLITE_OK;
  char aName[BCV_MAX_FSNAMEBYTES];

  pFile->pEntry = pEntry;
  pFile->rc = SQLITE_OK;
  bcvfsBlockidToText(pEntry->aName, pEntry->nName, aName);

  ENTER_VFS_MUTEX; {
    pFs->nOnDemand++;
  } LEAVE_VFS_MUTEX;

  rc = bcvDispatchLogmsg(pDisp, 
      "demand %d of %s", iBlk, pFile->pPath->zDatabase
  );
  if( rc==SQLITE_OK ){
    rc = bcvDispatchFetch(pDisp,pBcv,aName,0,0,(void*)pFile,bcvfsFetchBlockCb);
  }
  if( rc==SQLITE_OK ){
    rc = bcvDispatchRunAll(pDisp);
  }
  rc = (rc==SQLITE_OK ? pFile->rc : rc);

  ENTER_VFS_MUTEX; {
    pFs->nOnDemand--;
  } LEAVE_VFS_MUTEX;

  return rc;
}

/*
** Search the manifest passed as the first argument for a database with
** display name zDb. If one is found, return a pointer to it. Otherwise,
** return NULL.
*/
ManifestDb *bcvfsFindDatabase(Manifest *pMan, const char *zDb, int nDb){
  int ii;
  if( nDb<0 ){
    nDb = bcvStrlen(zDb);
  }
  for(ii=0; ii<pMan->nDb; ii++){
    ManifestDb *pDb = &pMan->aDb[ii];
    if( strlen(pDb->zDName)==nDb && memcmp(pDb->zDName, zDb, nDb)==0 ){
      return pDb;
    }
  }
  return 0;
}

/*
** Search the manifest supplied as the first argument for a database with
** database id iDbId. If it is found, return a pointer to the ManifestDb
** object. Otherwise, if there is no such db, return a NULL pointer.
*/
static ManifestDb *bcvfsFindDatabaseById(Manifest *pMan, i64 iDbId){
  int ii;
  for(ii=0; ii<pMan->nDb; ii++){
    ManifestDb *pDb = &pMan->aDb[ii];
    if( pDb->iDbId==iDbId ){
      return pDb;
    }
  }
  return 0;
}

/*
** Return the current MRU array to send to the daemon.
*/
static u32 *bcvfsProxyMRUArray(BcvProxyFile *p, u32 *pn){
  int nRet = 0;
  u32 *aRet = 0;
  BcvMRULink *aMRU = p->aMRU;
  if( aMRU && p->iMRU ){
    aRet = (u32*)&aMRU[p->pMap->u.read_r.nBlk];
    BcvMRULink *pLink = &aMRU[p->iMRU-1];
    u32 *aRet = (u32*)&aMRU[p->pMap->u.read_r.nBlk];

    while( 1 ){
      aRet[nRet++] = pLink - aMRU;
      if( pLink->iNext==0 ) break;
      pLink = &aMRU[pLink->iNext-1];
      assert( nRet<p->pMap->u.read_r.nBlk );
    }
  }

  *pn = (u32)nRet;
  return aRet;
}

/*
** Move block iBlk to the start of the MRU list.
*/
static void bcvfsProxyMRUAdd(BcvProxyFile *p, int iBlk){
  if( p->iMRU!=iBlk+1 ){
    BcvMRULink *aMRU = p->aMRU;

    /* Remove the block from its existing position, if any */
    if( aMRU[iBlk].iNext ){
      aMRU[ aMRU[iBlk].iNext-1 ].iPrev = aMRU[iBlk].iPrev;
    }
    if( aMRU[iBlk].iPrev ){
      aMRU[ aMRU[iBlk].iPrev-1 ].iNext = aMRU[iBlk].iNext;
    }

    /* Add the block to the head of the list */
    aMRU[iBlk].iPrev = 0;
    if( p->iMRU ){
      aMRU[iBlk].iNext = p->iMRU;
      aMRU[p->iMRU-1].iPrev = iBlk+1;
    }
    p->iMRU = iBlk+1;

    {
      u32 dummy = 0;
      bcvfsProxyMRUArray(p, &dummy);
    }
  }
}

static void bcvfsProxyCloseTransactionIf(BcvfsFile *pFile){
  if( pFile->lockMask==0 && pFile->p.pMap ){
    BcvMessage msg;
    memset(&msg, 0, sizeof(BcvMessage));
    msg.eType = BCV_MESSAGE_END;
    msg.u.end.aMru = bcvfsProxyMRUArray(&pFile->p, &msg.u.end.nMru);
    bcvSendMsg(pFile->p.fdProxy, &msg);
    sqlite3_free(pFile->p.pMap);
    sqlite3_free(pFile->p.aMRU);
    pFile->p.pMap = 0;
    pFile->p.aMRU = 0;
  }
}

static void bcvKVStoreFree(BcvKVStore *pKv){
  sqlite3_close(pKv->db);
  sqlite3_free(pKv->zETag);
  memset(pKv, 0, sizeof(BcvKVStore));
}

static void bcvfsCloseFile(sqlite3_file *pFile){
  if( pFile && pFile->pMethods ){
    pFile->pMethods->xClose(pFile);
  }
}

static int bcvfsClose(sqlite3_file *pFd){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  sqlite3_bcvfs *pFs = pFile->pFs;
  if( pFile->pCont ){
    ENTER_VFS_MUTEX; {
      /* Remove the BcvfsFile object from the sqlite3_bcvfs.pFileList list */
      BcvfsFile **pp;
      for(pp=&pFs->pFileList; *pp; pp=&(*pp)->pNextFile){
        if( *pp==pFile ){
          *pp = pFile->pNextFile;
          break;
        }
      }
      pFile->pCont->nClient--;
      bcvManifestDeref(pFile->pMan);
    } LEAVE_VFS_MUTEX;
  }

  ENTER_VFS_MUTEX; {
    pFile->pFs->nRef--;
  } LEAVE_VFS_MUTEX;

  bcv_close_socket(pFile->p.fdProxy);
  sqlite3_free(pFile->p.zStorage);
  sqlite3_free(pFile->p.zAccount);
  sqlite3_free(pFile->p.zContainer);
  sqlite3_free(pFile->p.zAuth);
  bcvEncryptionKeyFree(pFile->p.pKey);

  pFile->lockMask = 0;
  bcvKVStoreFree(&pFile->kv);
  bcvfsProxyCloseTransactionIf(pFile);
  bcvCloseLocal(pFile->pCacheFile);
  bcvfsCloseFile(pFile->pFile);
  sqlite3_free(pFile->zLocalPath);
  sqlite3_free(pFile->pPath);
  return SQLITE_OK;
}

static u32 bcvfsBlockHash(const u8 *pBlk, int nBlk){
  return bcvGetU32(&pBlk[4]);
}


/*
** The cache-entry passed as the second argument is guaranteed to be
** part of the LRU list of the VFS passed as the first argument. Remove
** it from the list.
*/
static void bcvfsLruRemove(BcvCommon *p, CacheEntry *pEntry){
  assert( p->bDaemon || pEntry->nRef==0 );
  assert( pEntry->pLruPrev || pEntry==p->pLruFirst );
  assert( pEntry->pLruNext || pEntry==p->pLruLast );
  assert( pEntry->pLruPrev==0 || pEntry->pLruPrev->pLruNext==pEntry );
  assert( pEntry->pLruNext==0 || pEntry->pLruNext->pLruPrev==pEntry );

  if( pEntry->pLruPrev ){
    pEntry->pLruPrev->pLruNext = pEntry->pLruNext;
  }else{
    p->pLruFirst = pEntry->pLruNext;
  }

  if( pEntry->pLruNext ){
    pEntry->pLruNext->pLruPrev = pEntry->pLruPrev;
  }else{
    p->pLruLast = pEntry->pLruPrev;
  }

  pEntry->pLruNext = pEntry->pLruPrev = 0;
}

/*
** If the cache entry passed as the second argument should be a part
** of the LRU list, remove it. A cache entry "should" be part of said
** list if:
**
**   a) there are no references,
**   b) there are no pins, and
**   c) the block is not dirty.
*/
void bcvfsLruRemoveIf(BcvCommon *p, CacheEntry *pEntry){
  if( (pEntry->nRef==0 || p->bDaemon) && pEntry->bDirty==0 ){
    bcvfsLruRemove(p, pEntry);
  }
}

/*
** The cache-entry passed as the second argument is guaranteed NOT to 
** be part of the LRU list of the VFS passed as the first argument. Add
** it to the list.
*/
void bcvfsLruAdd(BcvCommon *p, CacheEntry *pEntry){
  assert( pEntry->pLruNext==0 && pEntry->pLruPrev==0 );
  assert( p->pLruFirst!=pEntry && p->pLruLast!=pEntry );
  assert( p->bDaemon || pEntry->nRef==0 );
  assert( pEntry->bDirty==0 );

  pEntry->iLruTick = ++p->iLruTick;
  if( p->pLruFirst ){
    pEntry->pLruPrev = p->pLruLast;
    p->pLruLast->pLruNext = pEntry;
    p->pLruLast = pEntry;
  }else{
    p->pLruFirst = p->pLruLast = pEntry;
  }
}

void bcvfsLruAddIf(BcvCommon *p, CacheEntry *pEntry){
  if( (p->bDaemon || pEntry->nRef==0) && pEntry->bDirty==0 ){
    bcvfsLruAdd(p, pEntry);
  }
}

/*
** Search the hash table for a cache entry for the specified block-id.
** If it is found, return a pointer to it.
*/
CacheEntry *bcvfsHashFind(BcvCommon *p, const u8 *pBlk, int nBlk){
  CacheEntry *pRet = 0;
  u32 iHash = bcvfsBlockHash(pBlk, nBlk) % p->nHash;
  pRet = p->aHash[iHash];
  while( pRet && memcmp(pRet->aName, pBlk, nBlk) ){
    pRet = pRet->pHashNext;
  }
  return pRet;
}

/*
** Add the entry passed as the second argument to the hash table. This
** function assumes that the entry is not already in the hash table.
*/
void bcvfsHashAdd(BcvCommon *p, CacheEntry *pEntry){
  u32 iHash = bcvfsBlockHash(pEntry->aName, pEntry->nName) % p->nHash;
  pEntry->pHashNext = p->aHash[iHash];
  p->aHash[iHash] = pEntry;
}

/*
** Remove the entry passed as the second argument from the hash table, if
** it is currently a part of it.
*/
void bcvfsHashRemove(BcvCommon *p, CacheEntry *pEntry){
  CacheEntry **pp;
  u32 iHash = bcvfsBlockHash(pEntry->aName, pEntry->nName) % p->nHash;
  for(pp=&p->aHash[iHash]; *pp; pp=&(*pp)->pHashNext){
    if( *pp==pEntry ){
      *pp = pEntry->pHashNext;
      break;
    }
  }
  pEntry->pHashNext = 0;
}

/*
** Add the cache-entry to the unused list.
*/
void bcvfsUnusedAdd(BcvCommon *p, CacheEntry *pEntry){
  int iPos = pEntry->iPos;
  memset(pEntry, 0, sizeof(CacheEntry));
  pEntry->iPos = iPos;
  pEntry->pHashNext = p->pUnused;
  p->pUnused = pEntry;
}

/*
** Decrement the ref-count on object pEntry. If the final ref-count is
** zero, add pEntry to the LRU list.
*/
void bcvfsEntryUnref(BcvCommon *p, CacheEntry *pEntry){
  assert( pEntry->nRef>0 );
  pEntry->nRef--;
  bcvfsLruAddIf(p, pEntry);
}

/*
** Write an entry into the "block" table for the cache entry
** passed as the second argument.
*/
static int bcvfsWriteBlock(
  sqlite3_bcvfs *p, 
  BcvfsFile *pFile, 
  int iBlk, 
  CacheEntry *pEntry
){
  int rc = SQLITE_OK;
  sqlite3_stmt *pStmt = p->c.pInsertBlock;
  sqlite3_bind_int(pStmt, 1, pEntry->iPos);
  if( pEntry->bDirty ){
    sqlite3_bind_text(pStmt, 3, pFile->pCont->zName, -1, SQLITE_STATIC); 
    sqlite3_bind_int64(pStmt, 4, pFile->iFileDbId);
    sqlite3_bind_int(pStmt, 5, iBlk);
    sqlite3_bind_int(pStmt, 6, 0);
  }else{
    assert( pEntry->aName && pEntry->nName>0 );
    sqlite3_bind_blob(pStmt, 2, pEntry->aName, pEntry->nName, SQLITE_STATIC);
  }
  sqlite3_step(pStmt);
  rc = sqlite3_reset(pStmt);
  sqlite3_clear_bindings(pStmt);
  return rc;
}

CacheEntry *bcvfsAllocCacheEntry(int *pRc, BcvCommon *p){
  CacheEntry *pRet = 0;

  if( p->pUnused ){
    pRet = p->pUnused;
    p->pUnused = pRet->pHashNext;
    pRet->pHashNext = 0;
  }else if( (p->nBlk*p->szBlk)>=p->nMaxCache ){
    for(pRet=p->pLruFirst; pRet; pRet=pRet->pLruNext){
      assert( pRet->bValid );
      assert( pRet->nRef==0 || p->bDaemon );
      if( pRet->nRef==0 ){
        bcvfsLruRemove(p, pRet);
        bcvfsHashRemove(p, pRet);
        pRet->bValid = 0;
        break;
      }
    }
  }
  
  if( pRet==0 ){
    /* Allocate a new structure to represent the next free slot in the
    ** cache file. */
    pRet = (CacheEntry*)bcvMallocRc(pRc, sizeof(CacheEntry));
    if( pRet ){
      pRet->iPos = p->nBlk;
      p->nBlk++;
    }
  }

  assert( pRet==0 || pRet->bDirty==0 );
  return pRet;
}

static int bcvfsCopyBlock(
  BcvfsFile *pFile, 
  i64 szBlk, 
  int iTo, 
  int iFrom
){
  sqlite3_bcvfs *pFs = pFile->pFs;
  const static int nCopy = 32*1024;
  u8 *aBuf;
  int ii;
  int rc = SQLITE_OK;

  int nZero = 0;                  /* Trailing zeroes in copied block */

  assert( (szBlk % nCopy)==0 );
  aBuf = (u8*)sqlite3_malloc(nCopy);
  if( aBuf==0 ) rc = SQLITE_NOMEM;

  for(ii=0; rc==SQLITE_OK && ii<szBlk; ii+=nCopy){
    i64 i1 = (iTo*szBlk) + ii;
    i64 i2 = (iFrom*szBlk) + ii;
    rc = bcvReadfile(pFile->pCacheFile, aBuf, nCopy, i2);
    if( rc==SQLITE_OK ){
      rc = bcvWritefile(pFile->pCacheFile, aBuf, nCopy, i1);

      if( pFs->mLog & SQLITE_BCV_LOG_UPLOAD ){
        int ii;
        for(ii=szBlk; ii>0 && aBuf[ii-1]==0x00; ii--);
        if( ii==0 ){
          nZero += szBlk;
        }else{
          nZero = szBlk-ii;
        }
      }

    }
  }

  bcvfsUploadLog(pFs, 
      "copied block %d of cache file to block %d (%d trailing zeroes)",
      iFrom, iTo, nZero
  );

  sqlite3_free(aBuf);
  return rc;
}

static void bcvfsBlockidToName(const u8 *pBlk, int nBlk, char *aBuf){
  hex_encode(pBlk, nBlk, aBuf, 1);
  memcpy(&aBuf[nBlk*2], ".bcv", 5);
}

/* 
** If the file object does not already have an up to date reference to the
** manifest for the current transaction, obtain one now. 
*/
static int bcvfsGetManifestRef(BcvfsFile *pFile, int errcode){
  int rc = SQLITE_OK;
  if( pFile->pMan==0 ){
    Manifest *pMan = pFile->pCont->pMan;
    pFile->pManDb = bcvfsFindDatabaseById(pMan, pFile->iFileDbId);
    if( pFile->pManDb ){
      pFile->pMan = bcvManifestRef(pMan);
    }
    if( pFile->pMan==0 ){
      rc = errcode;
    }
  }
  return rc;
}

static int bcvfsManifestWrite(
  ManifestDb *pDb,
  int iBlk,
  const u8 *aName,
  int nName
){
  int rc = SQLITE_OK;

  /* Grow the manifest block array and add the new entry */
  while( pDb->nBlkLocalAlloc<=iBlk ){
    int nNew = MAX(pDb->nBlkOrig, iBlk+1)*2;
    u8 *aNew = 0;

    assert( (pDb->nBlkLocalAlloc==0)==(pDb->aBlkOrig==pDb->aBlkLocal) );
    if( pDb->nBlkLocalAlloc==0 ){
      aNew = (u8*)sqlite3_malloc(nNew*nName);
      if( aNew ){
        memcpy(aNew, pDb->aBlkOrig, pDb->nBlkOrig*nName);
      }
    }else{
      aNew = (u8*)sqlite3_realloc(pDb->aBlkLocal, nNew*nName);
    }
    if( aNew ){
      int nZero = (nNew-pDb->nBlkLocal)*nName;
      pDb->nBlkLocalAlloc = nNew;
      pDb->aBlkLocal = aNew;
      memset(&pDb->aBlkLocal[pDb->nBlkLocal*nName], 0, nZero);
    }else{
      rc = SQLITE_NOMEM;
    }
  }
  if( iBlk>=pDb->nBlkLocal ) pDb->nBlkLocal = iBlk + 1;
  memcpy(&pDb->aBlkLocal[iBlk*nName], aName, nName);

  return rc;
}

/*
** BcvDispatch object log callback.
*/
static void bcvfsLogCb(void *pCtx, int bRetry, const char *zMsg){
  sqlite3_bcvfs *p = (sqlite3_bcvfs*)pCtx;
  int flags = SQLITE_BCV_LOG_HTTP;
  if( bRetry ) flags |= SQLITE_BCV_LOG_HTTPRETRY;
  if( flags & p->mLog ){
    p->xLog(p->pLogCtx, flags, zMsg);
  }
}

/*
** Obtain a BCV connection to the cloud container represented by
** pCont. The VFS mutex must be held to call this function.
**
** Each container maintains an array of pairs of BcvDispatch/BcvContainer
** objects (hereafter refered to as "pair of objects"). When a thread
** needs to use such a pair of objects to access cloud storage, it takes the
** VFS mutex and calls this function to obtain exclusive access to a
** pair of objects. It then relinquishes the mutex and accesses cloud
** storage. Once it has finished with the handle, it calls
** bcvfsContainerReleaseBcv() to return the objects to control of the
** container.
**
**   ENTER_VFS_MUTEX;
**     handle = bcvfsContainerGetBcv()
**   LEAVE_VFS_MUTEX;
**
**   // do stuff with handle...
**
**   ENTER_VFS_MUTEX;
**     bcvfsContainerReleaseBcv(handle)
**   LEAVE_VFS_MUTEX;
**
** In single threaded applications, only a single pair of objects is ever
** required. It is only multi-threaded applications, where multiple threads
** may be accessing cloud storage simultaneously, that require multiple
** object pairs.
**
** This means there are a few branches in this function that are not covered
** by the single-threaded coverage tests.
*/
static int bcvfsContainerGetBcv(
  sqlite3_bcvfs *pFs,
  Container *pCont, 
  BcvDispatch **ppDisp, 
  BcvContainer **ppBcv,
  char **pzErr                    /* OUT: Error message (if not NULL) */
){
  int ii;
  BcvWrapper *pWrapper = 0;
  char *zErr = 0;
  int rc = SQLITE_OK;
  char *zAuth = 0;
  int iFree = pCont->nBcv;

  CHECK_VFS_MUTEX;

  for(ii=0; ii<pCont->nBcv; ii++){
    pWrapper = &pCont->aBcv[ii];
    if( pWrapper->pBcv==0 ){
      iFree = ii;
    }
    if( pWrapper->bUsable ){
      assert( pWrapper->pBcv );
      pWrapper->bUsable = 0;
      *ppBcv = pWrapper->pBcv;
      *ppDisp = pWrapper->pDisp;
      return SQLITE_OK;
    }
  }

  if( iFree==pCont->nBcv ){
    int nNew = pCont->nBcv + 4;
    BcvWrapper *aWrap = (BcvWrapper*)sqlite3_realloc(
        pCont->aBcv, nNew*sizeof(BcvWrapper)
    );
    if( aWrap==0 ) return SQLITE_NOMEM;
    memset(&aWrap[pCont->nBcv], 0, 4*sizeof(BcvWrapper));
    pCont->aBcv = aWrap;
    pCont->nBcv = nNew;
  }
  pWrapper = &pCont->aBcv[iFree];

  if( pFs->xAuth ){
    bcvfsEventLog(pFs, "invoking xAuth(%s, %s, %s)", 
        pCont->zStorage, pCont->zAccount, pCont->zContainer
    );
    rc = pFs->xAuth(pFs->pAuthCtx, 
        pCont->zStorage, pCont->zAccount, pCont->zContainer, &zAuth
    );
  }

  if( rc==SQLITE_OK ){
    rc = bcvContainerOpen(pCont->zStorage, pCont->zAccount, 
        zAuth, pCont->zContainer, &pWrapper->pBcv, &zErr
    );
  }
  if( rc==SQLITE_OK ){
    rc = bcvDispatchNew(&pWrapper->pDisp);
    if( rc!=SQLITE_OK ){
      bcvContainerClose(pWrapper->pBcv);
      pWrapper->pBcv = 0;
    }
    if( rc==SQLITE_OK ){
      bcvDispatchVerbose(pWrapper->pDisp, pFs->bCurlVerbose);
      if( (pFs->mLog & (SQLITE_BCV_LOG_HTTP|SQLITE_BCV_LOG_HTTPRETRY) ) ){
        bcvDispatchLog(pWrapper->pDisp, (void*)pFs, bcvfsLogCb);
      }
      bcvDispatchTimeout(pWrapper->pDisp, pFs->nHttpTimeout);
    }
  }
  if( rc==SQLITE_OK ){
    *ppBcv = pWrapper->pBcv;
    *ppDisp = pWrapper->pDisp;
  }

  if( pzErr ){
    *pzErr = zErr;
  }else{
    sqlite3_free(zErr);
  }
  sqlite3_free(zAuth);
  return rc;
}

/*
** Release a BCV connection obtained using bcvfsContainerGetBcv(). The 
** VFS mutex must be held to call this function.
*/
static void bcvfsContainerReleaseBcv(
  sqlite3_bcvfs *pFs,
  Container *pCont, 
  BcvDispatch *pDisp,
  BcvContainer *pBcv,
  int bError                      /* True if error has occurred */
){
  if( pBcv ){
    int ii;
    for(ii=0; ii<pCont->nBcv; ii++){
      BcvWrapper *pWrapper = &pCont->aBcv[ii];
      if( pWrapper->pBcv==pBcv ){
        assert( pWrapper->bUsable==0 );
        if( bError==0 ){
          pWrapper->bUsable = 1;
          return;
        }
        bcvfsEventLog(pFs, "destroying BCV connection %d...", 1);
        memset(pWrapper, 0, sizeof(BcvWrapper));
        break;
      }
      assert( ii<pCont->nBcv-1 );
    }
    bcvDispatchFree(pDisp);
    bcvContainerClose(pBcv);
  }
}

static int bcvfsFindBlockForIO(
  sqlite3_bcvfs *pFs,
  ManifestDb *pManDb,
  int nNameBytes,
  int iBlk,                       /* Block of file to find (0 indexed) */
  CacheEntry **ppEntry            /* OUT: Cache entry for located block */
){
  BcvCommon *pCommon = &pFs->c;
  CacheEntry *pEntry = 0;
  int rc = SQLITE_OK;

  CHECK_VFS_MUTEX;

  if( iBlk<pManDb->nBlkLocal ){
    /* Look up the name of the block in the manifest. Then search the hash
    ** table for it. There are then three possibilities:  
    **
    **   1) The block is found in the cache and can be used directly.
    **   2) An entry is found in the cache, but the block is still being
    **      downloaded by some other client in some other thread.
    **   3) No entry is found in the cache.
    **
    ** This loop usually only runs once (exits via the "break" at the
    ** end of it). The exception is case 2 above.
    */
    while( 1 ){
      u8 *pBlk = &pManDb->aBlkLocal[nNameBytes * iBlk];
      pEntry = bcvfsHashFind(pCommon, pBlk, nNameBytes);

      if( pEntry ){
        if( pEntry->bValid==0 ){
          /* Case 2. Wait a while and then try again. Eventually, either the
          ** block will be fully downloaded or the entry removed from the
          ** hash table.  */
          pFs->nOnDemand++;
          bcvfsMutexCondWait(&pFs->mutex);
          pFs->nOnDemand--;
          continue;
        }
        /* Case 1. The cache entry can be used immediately. If nRef==0, remove
        ** the entry from the LRU list. If nRef!=0, then some other thread is
        ** also using this cache entry - so it won't be on the LRU list. */
        bcvfsLruRemoveIf(pCommon, pEntry);
      }else{
        /* Case 3. The block was not found in the cache. Instead, find a 
        ** slot in the cache file that the block can be downloaded to.  */
        pEntry = bcvfsAllocCacheEntry(&rc, pCommon);

        /* Add the entry to the hash table using the new key */
        if( pEntry ){
          pEntry->nName = nNameBytes;
          memcpy(pEntry->aName, pBlk, nNameBytes);
          bcvfsHashAdd(pCommon, pEntry);
        }
      }

      if( pEntry ) pEntry->nRef++;
      break;
    }
  }else{
    /* The file is being extended. Allocate a new, empty, block. */
    pEntry = bcvfsAllocCacheEntry(&rc, pCommon);
    if( pEntry ){
      pEntry->nRef++;
      pEntry->nName = nNameBytes;
    }
  }

  assert( pEntry==0 || pEntry->bValid || pEntry->nRef==1 );

  *ppEntry = pEntry;
  return rc;
}

static int bcvfsMakeBlockWritable(
  BcvfsFile *pFile, 
  int iBlk,
  CacheEntry **ppEntry
){
  sqlite3_bcvfs *pFs = pFile->pFs;
  BcvCommon *pCommon = &pFs->c;
  int rc = SQLITE_OK;
  CacheEntry *pEntry = *ppEntry;
  CacheEntry *pNew = 0;

  ENTER_VFS_MUTEX; {
    ManifestDb *pManDb = pFile->pManDb;
    if( pEntry->bValid && pEntry->nRef>1 ){
      /* If (pEntry->nRef>1), then some other thread has a pointer to this
      ** cache-entry. In other words, some other thread is currently reading
      ** from this block. This means we cannot safely write to the block. Even
      ** though this thread holds the required SQLite locks to write, the block
      ** might be part of another database as well. So, in this case, make a
      ** copy of the block to write to.  
      **
      ** If (pEntry->nPin>(iBlk>pManDb->nBlkPin)), then the cache-entry is
      ** a part of some pinned database other than this one. Make a copy of
      ** the block to write to in this case too.  */
      pNew = bcvfsAllocCacheEntry(&rc, pCommon);
      pNew->nName = pEntry->nName;
      pNew->nRef = 1;
    }else{
      bcvfsHashRemove(pCommon, pEntry);
      pNew = pEntry;
    }
    assert( pNew || rc!=SQLITE_OK );

    if( rc==SQLITE_OK && pNew!=pEntry ){
      rc = bcvfsCopyBlock(pFile, pCommon->szBlk, pNew->iPos, pEntry->iPos);
    }

    if( pNew ){
      sqlite3_randomness(pNew->nName, pNew->aName);
      rc = bcvfsManifestWrite(pManDb, iBlk, pNew->aName, pNew->nName);
      if( rc==SQLITE_OK ){
        pNew->bDirty = 1;
      }
      bcvfsHashAdd(pCommon, pNew);

      /* Log a message announcing the new dirty block. */
      if( pFs->mLog & SQLITE_BCV_LOG_UPLOAD ){
        char zBlock[BCV_MAX_FSNAMEBYTES];
        bcvfsBlockidToName(pNew->aName, pNew->nName, zBlock);
        bcvfsUploadLog(pFs, "block %d of %s/%s is now dirty block %s"
            " (block %d in cache file)",
            iBlk, pFile->pCont->zName, pManDb->zDName, zBlock,
            pNew->iPos
        );
      }
    }
    if( pEntry!=pNew ){
      bcvfsEntryUnref(pCommon, pEntry);
      pEntry = pNew;
    }
  } LEAVE_VFS_MUTEX;

  *ppEntry = pEntry;
  return rc;
}

static int bcvfsReadWriteDatabase(
  int bWrite,                     /* True for write, false for read */
  BcvfsFile *pFile, 
  void *pBuf, 
  int iAmt, 
  sqlite3_int64 iOfst
){
  sqlite3_bcvfs *pFs = pFile->pFs;
  BcvCommon *pCommon = &pFs->c;
  const int ioerr = bWrite?SQLITE_IOERR_WRITE:SQLITE_IOERR_READ;
  i64 iBlk = 0;
  i64 szBlk = 0;
  int rc = SQLITE_OK;
  int bWriteBlock = 0;            /* True to write entry to blocksdb */
  CacheEntry *pEntry = 0;
  BcvContainer *pBcv = 0;
  BcvDispatch *pDisp = 0;

  /* When it opens a database, SQLite attempts to read the 100 byte 
  ** database header without first taking a read-lock. This is an 
  ** optimization only - the header has to be reread when the first
  ** transaction is opened anyway. So effectively ignore this read. */
  if( iOfst==0 && iAmt==100 ){
    assert( bWrite==0 );
    return SQLITE_IOERR_SHORT_READ;
  }
  assert( (iAmt & (iAmt-1))==0 );

  /* Take the VFS mutex and do three things under its cover:
  **
  **   1) If one is not already held, obtain a reference to the current
  **      container manifest. This pointer may only be dereferenced while
  **      the mutex is held, but holding the reference ensures that we can
  **      use the same manifest for the duration of the transaction.
  **
  **   2) Find the block in the cache. Assuming no error occurs, there 
  **      are two possibilities - the cache block already contains the 
  **      required data (if pEntry->bValid==1), or it needs to be 
  **      downloaded now (if pEntry->bValid==0).
  **
  **   3) If the cache block data needs to be downloaded, also obtain
  **      a pBcv handle to do the download with.
  */
  ENTER_VFS_MUTEX; {
    szBlk = pCommon->szBlk;
    iBlk = iOfst / szBlk;
    rc = bcvfsGetManifestRef(pFile, ioerr);
    if( rc==SQLITE_OK ){
      int nName = NAMEBYTES(pFile->pMan);
      rc = bcvfsFindBlockForIO(pFs, pFile->pManDb, nName, iBlk, &pEntry);
    }
    if( rc==SQLITE_OK && pEntry->bValid==0 && iBlk<pFile->pManDb->nBlkOrig ){
      assert( iBlk<pFile->pManDb->nBlkLocal || bWrite );
      rc = bcvfsContainerGetBcv(pFs, pFile->pCont, &pDisp, &pBcv, 0);
    }
  } LEAVE_VFS_MUTEX;

  /* If pBcv is not NULL, then this thread must download the block from cloud
  ** storage. Note that while it is ok to read pEntry->bValid and other 
  ** fields here, it may not be set following a successful download.
  ** pEntry->bValid may not be set until the VFS mutex is held again.  */
  if( pBcv ){
    rc = bcvfsFetchBlock(pFile, pDisp, pBcv, iBlk, pEntry);
    bWriteBlock = 1;
  }

  if( rc==SQLITE_OK && bWrite && pEntry->bDirty==0 ){
    rc = bcvfsMakeBlockWritable(pFile, iBlk, &pEntry);
    bWriteBlock = 1;
  }

  /* If the block was just downloaded and no error has occurred, write the
  ** entry to blocksdb.bcv. */
  if( rc==SQLITE_OK && bWriteBlock ){
    rc = bcvfsWriteBlock(pFs, pFile, iBlk, pEntry);
  }

  /* Read or write the data from or to the appropriate page of the
  ** cache file block.  
  */
  if( rc==SQLITE_OK ){
    sqlite3_file *pCF = pFile->pCacheFile;
    i64 iOff = (pEntry->iPos * szBlk) + (iOfst % szBlk);
    if( bWrite ){
      rc = pCF->pMethods->xWrite(pCF, pBuf, iAmt, iOff);
    }else{
      rc = pCF->pMethods->xRead(pCF, pBuf, iAmt, iOff);
      if( iOfst==0 ){
        /* Force database to look like a wal database */
        ((u8*)pBuf)[18] = 0x02;
        ((u8*)pBuf)[19] = 0x02;
      }
    }
  }

  /* Under cover of the VFS mutex, release the reference to the cache 
  ** entry used by this call.
  */
  ENTER_VFS_MUTEX; {
    if( pEntry ){
      if( pEntry->bValid==0 ){
        if( rc==SQLITE_OK ){
          pEntry->bValid = 1;
          bcvfsEntryUnref(pCommon, pEntry);
        }else{
          assert( pEntry->nRef==1 );
          bcvfsHashRemove(pCommon, pEntry);
          bcvfsUnusedAdd(pCommon, pEntry);
        }
        bcvfsMutexCondSignal(&pFs->mutex);
      }else{
        bcvfsEntryUnref(pCommon, pEntry);
      }

    }
    bcvfsContainerReleaseBcv(pFs, pFile->pCont, pDisp, pBcv, rc!=SQLITE_OK);
  } LEAVE_VFS_MUTEX; 

  return rc;
}

/*
** Parameter errCode may be an SQLite error code, or an HTTP error code.
** If it is an HTTP error code, convert it to an SQLite one.
*/
static int bcvErrorToSqlite(int errCode){
  int rc = errCode;
  if( rc==HTTP_AUTH_ERROR ){
    rc = SQLITE_IOERR_AUTH;
  }else if( rc>100 ){
    rc = SQLITE_IOERR; 
  }
  return rc;
}

static int bcvfsReadWriteDatabaseWithRetry(
  int bWrite,                     /* True for write, false for read */
  BcvfsFile *pFile, 
  void *pBuf, 
  int iAmt,
  sqlite3_int64 iOfst
){
  int rc = SQLITE_OK;
  int iIter = 0;
  do {
    rc = bcvfsReadWriteDatabase(bWrite, pFile, pBuf, iAmt, iOfst);
  }while( rc==HTTP_AUTH_ERROR && (++iIter<=BCVFS_MAX_AUTH_RETRIES) );
  if( rc!=SQLITE_IOERR_SHORT_READ ){
    rc = bcvErrorToSqlite(rc);
  }
  return rc;
}

static char *bcvInvokeAuth(
  int *pRc,
  sqlite3_bcvfs *pFs,
  const char *zStorage,
  const char *zAccount,
  const char *zCont
){
  int rc = *pRc;
  char *zAuth = 0;
  if( rc==SQLITE_OK && pFs->xAuth ){
    bcvfsEventLog(pFs, "invoking xAuth(%s, %s, %s)", zStorage, zAccount, zCont);
    rc = pFs->xAuth(pFs->pAuthCtx, zStorage, zAccount, zCont, &zAuth);
  }
  *pRc = rc;
  return zAuth;
}

static const char *bcvfsProxyGetAuth(int *pRc, BcvfsFile *pFile){
  char *zRet = 0;
  if( *pRc==SQLITE_OK ){
    if( pFile->p.zAuth==0 ){
      pFile->p.zAuth = bcvInvokeAuth(pRc, 
          pFile->pFs, pFile->p.zStorage, pFile->p.zAccount, pFile->p.zContainer
      );
    }
    zRet = pFile->p.zAuth;
  }
  return zRet;
}


/*
** Send the message pSend on socket s, and wait for a reply. If no error
** occurs and a reply is received, return it encoded as a BcvMessage
** object. It is the responsibility of the caller to eventually free 
** the returned object using sqlite3_free().
**
** This function is a no-op if (*pRc) is other than SQLITE_OK when it is
** called. If an error occurs within it, (*pRc) is set to an SQLite or
** HTTPS error code and NULL is returned.
*/
static BcvMessage *bcvExchangeMessage(
  int *pRc,                       /* IN/OUT: Error code */
  BCV_SOCKET_TYPE s,              /* Socket to communicate on */
  BcvMessage *pSend               /* Message to send */
){
  BcvMessage *pReply = 0;
  int rc = *pRc;
  if( rc==SQLITE_OK ){
    rc = bcvSendMsg(s, pSend);
    if( rc==SQLITE_OK ){
      rc = bcvRecvMsg(s, &pReply);
    }
  }
  *pRc = rc;
  return pReply;
}

static int bcvfsProxyOpenTransaction(BcvfsFile *pFile, int iBlk){
  int rc = SQLITE_OK;

  if( pFile->p.pMap==0 
   || iBlk>=pFile->p.pMap->u.read_r.nBlk
   || pFile->p.pMap->u.read_r.aBlk[iBlk]==0
  ){
    int iIter = 0;
    BcvMessage read;
    BcvMessage *pNew = 0;
    BcvMRULink *aMRU = 0;

    do {
      /* This is either the first iteration of this loop (if rc==SQLITE_OK),
      ** or an iteration following a 403 error from the daemon. In the
      ** latter case, free both the reply message from the previous iteration
      ** and the cached authentication string. This forces the 
      ** bcvfsProxyGetAuth() call below to invoke the authentication callback
      ** to obtain new credentials from the application. */
      assert( (rc==SQLITE_OK && pNew==0) || (rc==HTTP_AUTH_ERROR && pNew) );
      if( rc==HTTP_AUTH_ERROR ){
        sqlite3_free(pNew);
        pNew = 0;
        sqlite3_free(pFile->p.zAuth);
        pFile->p.zAuth = 0;
        rc = SQLITE_OK;
      }
      assert( rc==SQLITE_OK && pNew==0 );

      /* Formulate a message to send to the daemon. */
      memset(&read, 0, sizeof(read));
      read.eType = BCV_MESSAGE_READ;
      read.u.read.iBlk = iBlk;
      read.u.read.zAuth = bcvfsProxyGetAuth(&rc, pFile);
      read.u.read.aMru = bcvfsProxyMRUArray(&pFile->p, &read.u.read.nMru);

      /* Send the message and collect a reply. */
      pNew = bcvExchangeMessage(&rc, pFile->p.fdProxy, &read);
      assert( (rc==SQLITE_OK)==(pNew!=0) );
      if( pNew ){
        rc = pNew->u.read_r.errCode;
      }
      pFile->p.iMRU = 0;
      if( pFile->p.pMap ){
        int nBlk = pFile->p.pMap->u.read_r.nBlk;
        memset(pFile->p.aMRU, 0, (sizeof(BcvMRULink)+sizeof(u32))*nBlk);
        memset(pFile->p.pMap->u.read_r.aBlk, 0, sizeof(u32)*nBlk);
      }
    }while( rc==HTTP_AUTH_ERROR && (++iIter<=BCVFS_MAX_AUTH_RETRIES) );

    if( rc==SQLITE_OK ){
      if( iBlk>=pNew->u.read_r.nBlk || pNew->u.read_r.aBlk[iBlk]==0 ){
        rc = SQLITE_CORRUPT;
      }
    }
    if( rc==SQLITE_OK ){
      int nByte = (sizeof(BcvMRULink)+sizeof(u32))*pNew->u.read_r.nBlk;
      aMRU = (BcvMRULink*)bcvMallocRc(&rc, nByte);
    }
    if( rc==SQLITE_OK ){
      assert( pFile->p.iMRU==0 );
      sqlite3_free(pFile->p.aMRU);
      sqlite3_free(pFile->p.pMap);
      pFile->p.pMap = pNew;
      pFile->p.aMRU = aMRU;
      pNew = 0;
    }
    sqlite3_free(pNew);
  }

  return rc;
}

static void bcvfsProxyDecrypt(
  int *pRc,                       /* IN/OUT: Error code */
  BcvEncryptionKey *pKey,         /* Encryption key, or NULL */
  u8 *aBuf,                       /* Buffer to decrypt */
  int nBuf,                       /* Size of buffer aBuf[] in bytes */
  i64 iCacheOff                   /* Offset within cache file */
){
  if( pKey && *pRc==SQLITE_OK ){
    static const int nChunk = 512;
    int rc = SQLITE_OK;
    int ii;

    assert( (nBuf%nChunk)==0 );
    for(ii=0; ii<nBuf && rc==SQLITE_OK; ii+=nChunk){
      rc = bcvDecrypt(pKey, iCacheOff+ii, 0, &aBuf[ii], nChunk);
    }
    *pRc = rc;
  }
}


/*
** The implementation of xRead() for a proxy database file.
*/
static int bcvfsProxyRead(
  BcvfsFile *pFile,
  void *pBuf, 
  int iAmt, 
  sqlite3_int64 iOfst
){
  int rc = SQLITE_OK;
  int szBlk = pFile->p.szBlk;               /* Block size */
  int iBlk = iOfst / szBlk;                 /* Required block */
  u32 iCache = 0;
  i64 iCacheOffset = 0;

  if( pFile->lockMask==0 ){
    assert( iAmt==100 );
    memset(pBuf, 0, iAmt);
    return SQLITE_OK;
  }

  rc = bcvfsProxyOpenTransaction(pFile, iBlk);

  if( rc==SQLITE_OK ){
    /* Move block iBlk to the front of the MRU list */
    bcvfsProxyMRUAdd(&pFile->p, iBlk);

    iCache = pFile->p.pMap->u.read_r.aBlk[iBlk];
    assert( iCache>0 );
    iCacheOffset = ((i64)szBlk * (iCache-1)) + (iOfst % szBlk);

    rc = pFile->pCacheFile->pMethods->xRead(
        pFile->pCacheFile, pBuf, iAmt, iCacheOffset
    );
    bcvfsProxyDecrypt(&rc, pFile->p.pKey, pBuf, iAmt, iCacheOffset);
    if( iOfst==0 ){
      /* Force database to look like a wal database */
      ((u8*)pBuf)[18] = 0x02;
      ((u8*)pBuf)[19] = 0x02;
    }
  }
  return rc;
}

static int bcvfsNoFileRead(
  sqlite3_file *pFd, 
  void *pBuf, 
  int iAmt, 
  sqlite3_int64 iOfst
){
  unsigned char nofile_db[] = {
    0x53, 0x51, 0x4c, 0x69, 0x74, 0x65, 0x20, 0x66, 0x6f, 0x72, 0x6d, 0x61,
    0x74, 0x20, 0x33, 0x00, 0x02, 0x00, 0x01, 0x01, 0x00, 0x40, 0x20, 0x20,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x2e, 0x5b, 0x30, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00
  };
  int nCopy = MIN(sizeof(nofile_db), iAmt);
  memset(pBuf, 0, iAmt);
  memcpy(pBuf, nofile_db, nCopy);
  return SQLITE_OK;
}


static int bcvfsRead(
  sqlite3_file *pFd, 
  void *pBuf, 
  int iAmt, 
  sqlite3_int64 iOfst
){
  int rc = SQLITE_OK;
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  switch( bcvfsFileType(pFile) ){
    case BCVFS_NO_FILE:
      rc = bcvfsNoFileRead(pFd, pBuf, iAmt, iOfst);
      break;
    case BCVFS_WAL_FILE:
    case BCVFS_PASSTHRU_FILE:
      rc = pFile->pFile->pMethods->xRead(pFile->pFile, pBuf, iAmt, iOfst);
      break;

    case BCVFS_PROXY_FILE:
      rc = bcvfsProxyRead(pFile, pBuf, iAmt, iOfst);
      break;

    default: {
      assert( bcvfsFileType(pFile)==BCVFS_DATABASE_FILE );
      rc = bcvfsReadWriteDatabaseWithRetry(0, pFile, pBuf, iAmt, iOfst);
      break;
    }
  }
  return rc;
}

static int bcvfsWrite(
  sqlite3_file *pFd,
  const void *pBuf, 
  int iAmt, 
  sqlite3_int64 iOfst
){
  int rc = SQLITE_OK;
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  switch( bcvfsFileType(pFile) ){
    case BCVFS_NO_FILE:
      if( pFile->bIsMain && iOfst>0 ){
        rc = SQLITE_READONLY;
      }
      break;
    case BCVFS_WAL_FILE:
    case BCVFS_PASSTHRU_FILE:
      rc = pFile->pFile->pMethods->xWrite(pFile->pFile, pBuf, iAmt, iOfst);
      break;

    default: {
      assert( bcvfsFileType(pFile)==BCVFS_DATABASE_FILE );
      rc = bcvfsReadWriteDatabaseWithRetry(1, pFile, (void*)pBuf, iAmt, iOfst);
      break;
    }
  }
  return rc;
}

/*
** Discard - undirty - all dirty blocks from database pDb between block iFirst
** and the end of the db, inclusive. i.e. if iFirst==0, undirty all blocks
** in the database.
*/
static void bcvfsUndirtyBlocks(
  sqlite3_bcvfs *pFs, 
  int nName, 
  ManifestDb *pDb,
  int iFirst
){
  int iBlk;
  CHECK_VFS_MUTEX;

  for(iBlk=iFirst; iBlk<pDb->nBlkLocal; iBlk++){
    int iOff = iBlk*nName;
    u8 *aBlk = &pDb->aBlkLocal[iOff];
    if( iBlk>=pDb->nBlkOrig || memcmp(aBlk, &pDb->aBlkOrig[iOff], nName) ){
      CacheEntry *pEntry = bcvfsHashFind(&pFs->c, aBlk, nName);
      assert( pEntry && pEntry->bDirty );
      pEntry->bDirty = 0;
      bcvfsLruAddIf(&pFs->c, pEntry);
      bcvfsWriteBlock(pFs, 0, 0, pEntry);
    }
  }
}


static int bcvfsTruncate(sqlite3_file *pFd, sqlite3_int64 size){
  int rc = SQLITE_OK;
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  switch( bcvfsFileType(pFile) ){
    case BCVFS_NO_FILE:
      /* no-op */
      break;
    case BCVFS_WAL_FILE:
    case BCVFS_PASSTHRU_FILE:
      rc = pFile->pFile->pMethods->xTruncate(pFile->pFile, size);
      break;

    default: {
      sqlite3_bcvfs *pFs = pFile->pFs;
      int szBlk = pFile->pMan->szBlk;
      int nNew = (size + szBlk - 1) / szBlk;
      if( nNew<pFile->pManDb->nBlkLocal ){
        ENTER_VFS_MUTEX; {
          bcvfsUndirtyBlocks(pFs, NAMEBYTES(pFile->pMan), pFile->pManDb, nNew);
          pFile->pManDb->nBlkLocal = nNew;
        } LEAVE_VFS_MUTEX;
      }
      assert( pFile->pManDb && pFile->pManDb->nBlkLocalAlloc );
      assert( bcvfsFileType(pFile)==BCVFS_DATABASE_FILE );
      break;
    }
  }
  return rc;
}
static int bcvfsSync(sqlite3_file *pFile, int flags){
  return SQLITE_OK;
}
static int bcvfsFileSize(sqlite3_file *pFd, sqlite3_int64 *pSize){
  int rc = SQLITE_OK;
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  int eType = bcvfsFileType(pFile);
  switch( eType ){
    case BCVFS_NO_FILE:
      *pSize = 512;
      break;

    case BCVFS_WAL_FILE:
    case BCVFS_PASSTHRU_FILE:
      rc = pFile->pFile->pMethods->xFileSize(pFile->pFile, pSize);
      break;

    default: {
      assert( eType==BCVFS_DATABASE_FILE || eType==BCVFS_PROXY_FILE );
      *pSize = ((sqlite3_int64)1 << (16 + 31)) - (1<<16);
      break;
    }
  }
  return rc;
}

/*
** If the file is now completely unlocked, free any attached KV database.
*/
static void bcvfsFileCacheFreeIf(BcvfsFile *pFile){
  if( pFile->lockMask==0 && pFile->kv.db ){
    bcvKVStoreFree(&pFile->kv);
  }
}

static int bcvfsLock(sqlite3_file *pFd, int eLock){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  pFile->lockMask = eLock;
  if( bcvfsFileType(pFile)==BCVFS_NO_FILE ){
    return SQLITE_OK;
  }
  return pFile->pFile->pMethods->xLock(pFile->pFile, eLock);
}
static int bcvfsUnlock(sqlite3_file *pFd, int eLock){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  pFile->lockMask = eLock;
  bcvfsFileCacheFreeIf(pFile);
  if( bcvfsFileType(pFile)==BCVFS_NO_FILE ){
    return SQLITE_OK;
  }
  return pFile->pFile->pMethods->xUnlock(pFile->pFile, eLock);
}
static int bcvfsCheckReservedLock(sqlite3_file *pFd, int *pResOut){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  if( bcvfsFileType(pFile)==BCVFS_NO_FILE ){
    *pResOut = 0;
    return SQLITE_OK;
  }
  return pFile->pFile->pMethods->xCheckReservedLock(pFile->pFile, pResOut);
}

/*
** Install the manifest passed as the third argument into the container
** passed as the second. Also, unless this is a secure container, write an
** entry into the "container" table for this container containing the new
** manifest.
*/
int bcvManifestInstall(
  BcvCommon *p, 
  Container *pCont, 
  Manifest *pMan
){
  sqlite3_stmt *pInsert = p->pInsertCont;
  int rc = SQLITE_OK;
  
  assert( p->szBlk==0 || pMan->szBlk==p->szBlk );

  if( pCont->pKey==0 ){
    int nMan = 0;
    u8 *aMan = bcvManifestCompose(pMan, &nMan);
    if( aMan==0 ){
      rc = SQLITE_NOMEM;
    }else{
      sqlite3_bind_text(pInsert, 1, pCont->zName, -1, SQLITE_STATIC);
      sqlite3_bind_text(pInsert, 2, pCont->zStorage, -1, SQLITE_STATIC);
      sqlite3_bind_text(pInsert, 3, pCont->zAccount, -1, SQLITE_STATIC);
      sqlite3_bind_text(pInsert, 4, pCont->zContainer, -1, SQLITE_STATIC);
      sqlite3_bind_blob(pInsert, 5, aMan, nMan, SQLITE_STATIC);
      sqlite3_bind_text(pInsert, 6, pMan->zETag, -1, SQLITE_STATIC);
      sqlite3_step(pInsert);
      rc = sqlite3_reset(pInsert);
      sqlite3_free(aMan);
    }
  }

  if( rc==SQLITE_OK ){
    bcvManifestDeref(pCont->pMan);
    pCont->pMan = pMan;
    p->szBlk = pMan->szBlk;
  }else{
    bcvManifestDeref(pMan);
  }
  return rc;
}

static u32 bcvfsGetBigU32(const u8 *a){
  return (((u32)a[0]) << 24)
       + (((u32)a[1]) << 16)
       + (((u32)a[2]) <<  8)
       + (((u32)a[3]) <<  0);
}
static u32 bcvfsGetLittleU32(const u8 *a){
  return (((u32)a[3]) << 24)
       + (((u32)a[2]) << 16)
       + (((u32)a[1]) <<  8)
       + (((u32)a[0]) <<  0);
}

typedef struct UploadCtx UploadCtx;
typedef struct UploadCtx1 UploadCtx1;
typedef struct UploadRecord UploadRecord;
struct UploadCtx {
  int rc;                         /* Error code */
  char *zErrMsg;                  /* Error message (from sqlite3_malloc) */
  BcvfsFile *pFile;               /* File being uploaded */
  int iNext;                      /* Next block to upload (if dirty) */
  i64 iDelTime;
  ManifestHash *pHash;
  Manifest *pMan;
  ManifestDb *pManDb;
  UploadRecord *pRecord;

  BcvContainer *pBcv;
  BcvDispatch *pDisp;
};

/*
** A linked list of these structures is accumulated while uploading. Once
** the upload has successfully completed and the new manifest installed, 
** it is used to rename existing cache entries to their new names. And
** to mark them as non-dirty.
*/
struct UploadRecord {
  u8 *aOldName;                   /* Block's temp name */
  u8 *aNewName;                   /* Block's new name (as uploaded) */
  UploadRecord *pNext;            /* Next element in list */
};

struct UploadCtx1 {
  UploadCtx *pCtx;
};

typedef struct UploadManCtx UploadManCtx;
struct UploadManCtx {
  int rc;
  char *zETag;
};

static void bcvfsUploadManDone(void *pArg, int rc, char *zETag){
  UploadManCtx *pCtx = (UploadManCtx*)pArg;
  if( zETag ){
    pCtx->zETag = bcvMprintfRc(&pCtx->rc, "%s", zETag);
  }
  if( pCtx->rc==SQLITE_OK ) pCtx->rc = rc;
}

/*
** Upload manifest pMan to the container represented by pDisp/pBcv. If
** successful, return SQLITE_OK. Or, if an error occurs, return either
** an SQLite error code or an HTTP error code.
*/
static int bcvfsUploadManifest(
  BcvDispatch *pDisp,
  BcvContainer *pBcv,
  Manifest *pMan,
  char **pzErr
){
  int rc = SQLITE_OK;           /* Return code */
  u8 *aMan = 0;                 /* Serialized manifest object */
  int nMan = 0;                 /* Size of aMan[] in bytes */
  UploadManCtx ctx;

  memset(&ctx, 0, sizeof(ctx));
  aMan = bcvManifestCompose(pMan, &nMan);
  if( aMan==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = bcvDispatchPut(pDisp, pBcv, BCV_MANIFEST_FILE, 
        pMan->zETag, aMan, nMan, (void*)&ctx, bcvfsUploadManDone
    );
  }
  if( rc==SQLITE_OK ){
    rc = bcvDispatchRunAll(pDisp);
  }
  if( rc==SQLITE_OK ){
    rc = ctx.rc;
  }
  if( rc==SQLITE_OK ){
    sqlite3_free(pMan->zETag);
    pMan->zETag = ctx.zETag;
  }else{
    *pzErr = ctx.zETag;
  }

  sqlite3_free(aMan);
  return rc;
}

static int bcvfsShmLock(sqlite3_file*,int,int,int);

static const char *bcvfsStateName(int eState){
  const char *azName[] = {
    "NONE",
    "UPLOAD",
    "COPY",
    "POLL",
    "DELETE",
  };

  assert( CONTAINER_STATE_NONE==0 );
  assert( CONTAINER_STATE_UPLOAD==1 );
  assert( CONTAINER_STATE_COPY==2 );
  assert( CONTAINER_STATE_POLL==3 );
  assert( CONTAINER_STATE_DELETE==4 );

  return azName[eState];
}

/*
** The VFS mutex must be held to call this function. It sets the container
** state to eState, waiting for other operations to finish as usual.
*/
static void bcvfsEnterState(sqlite3_bcvfs *pFs, Container *pCont, int eState){
  while( 1 ){
    if( pCont->eState==CONTAINER_STATE_NONE ){
      const char *z = bcvfsStateName(eState);
      pCont->eState = eState;
      bcvfsEventLog(pFs, "container %s enters %s state", pCont->zName, z);
      return;
    }else{
      bcvfsMutexCondWait(&pFs->mutex);
    }
  }
}

static void bcvfsLeaveState(sqlite3_bcvfs *pFs, Container *pCont){
  const char *z = bcvfsStateName(pCont->eState);
  bcvfsEventLog(pFs, "container %s leaves %s state", pCont->zName, z);
  pCont->eState = CONTAINER_STATE_NONE;
  bcvfsMutexCondSignal(&pFs->mutex);
}

static void bcvfsReleaseCheckpointer(BcvfsFile *pFile){
  if( pFile->lockMask & (1 << WAL_CHECKPOINTER_LOCK) ){
    int f = SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_UNLOCK;
    bcvfsShmLock(&pFile->base, WAL_CHECKPOINTER_LOCK, 1, f);
  }
}

static char *bcvfsFindDbName(int *pRc, BcvfsFile *pFile){
  sqlite3 *db = *pFile->ppDb;
  int rc = *pRc;
  sqlite3_stmt *pDList = 0;
  char *zRet = 0;

  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pDList, 0);
  }
  if( rc==SQLITE_OK ){
    int rc2;
    while( SQLITE_ROW==sqlite3_step(pDList) ){
      BcvfsFile *pX = 0;
      const char *zDb = (const char*)sqlite3_column_text(pDList, 1);
      sqlite3_file_control(db, zDb, BCV_FCNTL_FD, (void*)&pX);
      if( pX==pFile ){
        zRet = bcvMprintfRc(&rc, zDb);
      }
    }
    rc2 = sqlite3_finalize(pDList);
    if( rc==SQLITE_OK ) rc = rc2;
  }

  *pRc = rc;
  return zRet;
}

#define WAL_HEADER_SZ       32
#define WAL_FRAME_HEADER_SZ 24

static void bcvfsWalChecksum(
  int bBig,                       /* True for big-endian, false for little */
  u8 *aIn, int nIn,               /* Data to checksum */
  u32 *pCksum1, u32 *pCksum2      /* IN/OUT: Cumulative checksum values */
){
  int ii;
  u32 cksum1 = *pCksum1;
  u32 cksum2 = *pCksum2;
  u32 (*xGet)(const u8*) = (bBig ? bcvfsGetBigU32 : bcvfsGetLittleU32);

  for(ii=0; ii<(nIn/8); ii++){
    u32 x1 = xGet(&aIn[ii*8]);
    u32 x2 = xGet(&aIn[ii*8 + 4]);
    cksum1 += x1 + cksum2;
    cksum2 += x2 + cksum1;
  }

  *pCksum1 = cksum1;
  *pCksum2 = cksum2;
}


/*
** Buffer aIn[] contains an 8-byte checksum - two 32-bit big-endian 
** values. This function compares the two values to arguments cksum1 
** and cksum2, returning 0 if they match or non-zero if they do not.
*/
static int bcvfsCompareCksum(const u8 *aIn, u32 cksum1, u32 cksum2){
  u8 aCksum[8];
  bcvPutU32(&aCksum[0], cksum1);
  bcvPutU32(&aCksum[4], cksum2);
  return memcmp(aCksum, aIn, 8);
}

/*
**
*/
static int bcvfsWalNonEmpty(
  BcvCommon *pCommon,             /* VFS object */
  const char *zPath,              /* Path to wal file */
  int *pbNonEmpty                 /* OUT: True if wal contains transactions */
){
  sqlite3_vfs *pVfs = pCommon->pVfs;
  int rc = SQLITE_OK;
  int flags = 0;

  *pbNonEmpty = 0;
  rc = pVfs->xAccess(pVfs, zPath, SQLITE_ACCESS_EXISTS, &flags);
  if( flags!=0 ){
    i64 sz = 0;
    sqlite3_file *pWal = 0;       /* File descriptor open on wal file */

    pWal = bcvMallocRc(&rc, pVfs->szOsFile);
    if( rc==SQLITE_OK ){
      int f = SQLITE_OPEN_WAL|SQLITE_OPEN_READONLY;
      rc = pVfs->xOpen(pVfs, zPath, pWal, f, &f);
    }
    if( rc==SQLITE_OK ){
      rc = pWal->pMethods->xFileSize(pWal, &sz);
    }
    if( rc==SQLITE_OK && sz>=(WAL_HEADER_SZ+WAL_FRAME_HEADER_SZ+512) ){
      int bBig = 0;             /* 1==big-endian, 0==little-endian checksum */
      u32 pgsz = 0;             /* Page size used by wal file */
      u32 cksum1 = 0;
      u32 cksum2 = 0;
      u8 aHdr[WAL_HEADER_SZ];   /* Contents of wal file header */

      rc = bcvReadfile(pWal, aHdr, WAL_HEADER_SZ, 0);
      if( rc==SQLITE_OK ){
        bBig = bcvfsGetBigU32(&aHdr[0])==0x377f0683;     /* from docs */
        pgsz = bcvfsGetBigU32(&aHdr[8]);

        bcvfsWalChecksum(bBig, aHdr, WAL_HEADER_SZ-8, &cksum1, &cksum2);
        if( 0==bcvfsCompareCksum(&aHdr[24], cksum1, cksum2) ){
          i64 ii;
          i64 nFrame = (sz-WAL_HEADER_SZ) / (pgsz+WAL_FRAME_HEADER_SZ);
          u8 *aBuf = (u8*)bcvMallocRc(&rc, pgsz+WAL_FRAME_HEADER_SZ);

          for(ii=0; rc==SQLITE_OK && ii<nFrame; ii++){
            i64 iOff = ii*(pgsz+WAL_FRAME_HEADER_SZ) + WAL_HEADER_SZ;
            rc = bcvReadfile(pWal, aBuf, pgsz+WAL_FRAME_HEADER_SZ, iOff);
            if( rc==SQLITE_OK ){
              bcvfsWalChecksum(bBig, aBuf, 8, &cksum1, &cksum2);
              bcvfsWalChecksum(
                  bBig, &aBuf[WAL_FRAME_HEADER_SZ], pgsz, &cksum1, &cksum2
              );
              if( bcvfsCompareCksum(&aBuf[16], cksum1, cksum2) ){
                /* Checksum failure */
                break;
              }
              if( bcvfsGetBigU32(&aBuf[4])>0 ){
                /* Complete transaction */
                *pbNonEmpty = 1;
                break;
              }
            }
          }
          sqlite3_free(aBuf);
        }
      }
    }
    if( pWal && pWal->pMethods ){
      pWal->pMethods->xClose(pWal);
    }
    sqlite3_free(pWal);
  }

  return rc;
}


/*
** Append a nul-terminated string to the buffer.
*/
static void bcvBufferAppendString(int *pRc, BcvBuffer *pBuf, const char *zStr){
  int nStr = bcvStrlen(zStr);
  bcvBufferAppendRc(pRc, pBuf, zStr, nStr+1);
  if( *pRc==SQLITE_OK ) pBuf->nData--;
}

/*
** Append a nul-terminated string to the buffer, escaped.
*/
static void bcvBufferAppendEscaped(int *pRc, BcvBuffer *pBuf, const char *zStr){
  const char aHex[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
  };
  const char *pIn;
  for(pIn=zStr; *pIn; pIn++){
    char aStr[4];
    char c = *pIn;
    if( bcvfsIsSafeChar(c) ){
      aStr[0] = c;
      aStr[1] = '\0';
    }else{
      aStr[0] = '%';
      aStr[1] = aHex[(c>>4) & 0x0F];
      aStr[2] = aHex[c & 0x0F];
      aStr[3] = '\0';
    }
    bcvBufferAppendString(pRc, pBuf, aStr);
  }
}

/*
** Return the full local path to the database or wal file identified by
** pPath. The caller is responsible for ensuring that the returned buffer
** is eventually freed using sqlite3_free().
*/
static char *bcvfsLocalPath(int *pRc, sqlite3_bcvfs *pFs, BcvDbPath *pPath){
  BcvBuffer b = {0,0,0};
  bcvBufferAppendString(pRc, &b, pFs->c.zDir);
  bcvBufferAppendString(pRc, &b, BCV_PATH_SEPARATOR);
  bcvBufferAppendEscaped(pRc, &b, pPath->zContainer);
  bcvBufferAppendString(pRc, &b, BCV_PATH_SEPARATOR);
  bcvBufferAppendEscaped(pRc, &b, pPath->zDatabase);
  bcvBufferAppendRc(pRc, &b, "\0\0", 2);
  assert( *pRc==SQLITE_OK || b.aData==0 );
  return (char*)b.aData;
}

/*
** Return the full local path to the database (bWal==0) or wal file (bWal==1)
** associated with the database zDName in container pCont. The caller is
** responsible for ensuring that the returned buffer is eventually freed using
** sqlite3_free().
*/
static char *bcvfsLocalName(
  int *pRc, 
  Container *pCont, 
  const char *zDName,
  int bWal
){
  BcvBuffer b = {0,0,0};
  bcvBufferAppendString(pRc, &b, pCont->zLocalDir);
  bcvBufferAppendString(pRc, &b, BCV_PATH_SEPARATOR);
  bcvBufferAppendEscaped(pRc, &b, zDName);
  if( bWal ){
    bcvBufferAppendString(pRc, &b, "-wal");
  }
  bcvBufferAppendRc(pRc, &b, "\0\0", 2);
  assert( *pRc==SQLITE_OK || b.aData==0 );
  return (char*)b.aData;
}

typedef struct FileWrapper FileWrapper;
struct FileWrapper {
  int bUnmap;
  int bUnlock;
  sqlite3_file *pFd;
  sqlite3_file *pWalFd;
  FileWrapper *pNext;
};

/*
** This function is a no-op if (*pRc) is set to other than SQLITE_OK when
** it is called. Otherwise, it attempts to allocate nIn bytes using
** sqlite3_malloc(). If successful, it populates the new buffer with 
** a copy of buffer pIn and returns a pointer to it. Or, if the allocation
** fails, it sets (*pRc) to SQLITE_NOMEM and returns NULL.
*/
static void *bcvMallocCopy(int *pRc, void *pIn, int nIn){
  void *pRet = bcvMallocRc(pRc, nIn);
  if( pRet ){
    memcpy(pRet, pIn, nIn);
  }
  return pRet;
}

/*
** Check if containter pCont can be updated to use manifest pMan. If so:
**
**   1) Update object pMan with any local modifications (the contents
**      of any ManifestDb.aBlkLocal[] arrays that are not copies of
**      aBlkOrig[]), and
**
**   2) Write pMan to the container table of blocksdb.bcv, and
**
**   3) Set pCont->pMan to pMan.
**
** This function takes ownership of pMan. If it does not install it into
** the container, it calls bcvManifestDeref() to decrement the ref-count.
**
** A manifest can be installed unless it contains changes that conflict 
** with local changes. Specifically, the manifest may not be installed
** if any of the following are true:
**
**   1) A database has been modified or deleted remotely, and either
**        a) the database is pinned locally, or
**        b) the database has been modified locally.
**
**   2) A database has been modified remotely and deleted locally.
**
**   3) A database that has been created remotely has the same display
**      name as one that has been created locally.
**
**   4) One or more blocks in the aBlkOrig[] array used used by a database 
**      created locally are not currently in use by any other database
**      (that was not also created locally).
**
** If any of the above are true one or more databases, the manifest 
** cannot be installed. Checking if the database has been modified locally 
** is done as follows:
**
**   i) Take the WRITER lock on the database. This ensures that the 
**      *-wal file cannot be modified. This lock is not released until
**      all databases have been checked and the new manifest installed.
**
**  ii) Check for dirty blocks.
**
** iii) Check the *-wal file for committed transactions.
*/
int bcvManifestUpdate(
  BcvCommon *pCommon,             /* Hash tables */
  Container *pCont,               /* Container to update manifest of */
  Manifest *pMan,                 /* New manifest object to install */
  char **pzErr                    /* OUT: Error message */
){
  int nName = NAMEBYTES(pMan);
  int rc = SQLITE_OK;
  Manifest *pOld = pCont->pMan;
  int iOld = 0;
  int iNew = 0;
  FileWrapper *pList = 0;
  int nLocal = 0;

  for(iOld=0; rc==SQLITE_OK && iOld<pOld->nDb; iOld++){
    ManifestDb *pOldDb = &pOld->aDb[iOld];
    ManifestDb *pNewDb = 0;
    int bFail = 0;
    while( iNew<pMan->nDb ){
      pNewDb = &pMan->aDb[iNew];
      if( pNewDb->iDbId>=pOldDb->iDbId ) break;
      iNew++;
      pNewDb = 0;
    }

    if( pOldDb->iDbId>=BCVFS_FIRST_LOCAL_ID ){
      /* This branch is taken if pOldDb is a local database. In this case:
      **
      **   1) Check that the new manifest does not contain a database with
      **      the same display name as the local db, and
      **
      **   2) Check that the new manifest contains all blocks required
      **      by the local db (all blocks in aBlkOrig).
      */
      int ii;
      for(ii=0; ii<pMan->nDb; ii++){
        if( strcmp(pMan->aDb[ii].zDName, pOldDb->zDName)==0 ){
          bFail = 1;
        }
      }
      if( bFail==0 ){
        int nName = NAMEBYTES(pMan);
        ManifestHash *pHash = 0;
        rc = bcvMHashBuild(pMan, 0, 0, &pHash);
        if( rc==SQLITE_OK ){
          for(ii=0; ii<pOldDb->nBlkOrig; ii++){
            if( bcvMHashQuery(pHash, &pOldDb->aBlkOrig[ii*nName], nName)==0 ){
              bFail = 1;
            }
          }
        }
        bcvMHashFree(pHash);
      }
      nLocal++;
    }else

    if( pNewDb==0 
     || pNewDb->iDbId>pOldDb->iDbId 
     || pNewDb->iVersion!=pOldDb->iVersion 
    ){
      /* This branch is taken if database pOldDb has been deleted or
      ** modified by this manifest upgrade. Check to see if there have
      ** been any local changes. If there have, the manifest update 
      ** cannot proceed. To check for local changes:
      **
      **   1) Check if there are any modified blocks.
      **   2) Take the WRITER lock on the database.
      **   3) Check the wal file for valid transactions.
      */
      if( pOldDb->nBlkLocal==0 ){
        /* The database has been deleted locally. If it was also deleted
        ** remotely, allow this. Otherwise, if the database was modified
        ** remotely, fail the poll operation. */
        if( pNewDb ) bFail = 1;
      }else
      if( pOldDb->nBlkLocalAlloc>0 ){
        bFail = 1;
      }else{
        const int nAlloc = sizeof(FileWrapper);
        FileWrapper *pWrapper = (FileWrapper*)bcvMallocRc(&rc, nAlloc);
        char *zFile = bcvfsLocalName(&rc, pCont, pOldDb->zDName, 0);
        char *zWal = bcvMprintfRc(&rc, "%s-wal", zFile);

        if( rc==SQLITE_OK ){
          u8 *aMap = 0;
          const sqlite3_io_methods *pMeth = 0;
          sqlite3_file *pFd = 0;
          const int lockflags = SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE;

          rc = bcvOpenLocal(zFile, 0, 0, &pFd);
          pWrapper->pFd = pFd;
          if( rc==SQLITE_OK ){
            pMeth = pFd->pMethods;
            rc = pMeth->xShmMap(pFd, 0, 32*1024, 1, (volatile void**)&aMap);
          }
          if( rc==SQLITE_OK ){
            pWrapper->bUnmap = 1;
            if( pMeth->xShmLock(pFd, 0, 1, lockflags) ){
              bFail = 1;
            }else{
              pWrapper->bUnlock = 1;
              memset(aMap, 0, 24);
              rc = bcvfsWalNonEmpty(pCommon, zWal, &bFail);
            }
          }
        }
        if( pWrapper ){
          pWrapper->pNext = pList;
          pList = pWrapper;
        }
        sqlite3_free(zFile);
        sqlite3_free(zWal);
      }
    }else{
      assert( pNewDb->nBlkLocalAlloc==0 );
      if( pOldDb->nBlkLocalAlloc ){
        pNewDb->aBlkLocal = (u8*)bcvMallocRc(&rc, nName*pOldDb->nBlkLocalAlloc);
        if( rc==SQLITE_OK ){
          pNewDb->nBlkLocal = pOldDb->nBlkLocal;
          pNewDb->nBlkLocalAlloc = pOldDb->nBlkLocalAlloc;
          memcpy(pNewDb->aBlkLocal, pOldDb->aBlkLocal, pOldDb->nBlkLocal*nName);
        }
      }else if( pOldDb->nBlkLocal==0 ){
        pNewDb->nBlkOrig = pNewDb->nBlkLocal = 0;
        pNewDb->aBlkOrig = pNewDb->aBlkLocal = 0;
      }
    }

    if( rc==SQLITE_OK && bFail ){
      assert( rc==SQLITE_OK );
      *pzErr = bcvMprintfRc(&rc, 
          "cannot update manifest - write collision on database %s", 
          pOldDb->zDName
      );
      rc = SQLITE_ERROR;
    }
  }

  /* If there are any local databases, append them onto the end of the
  ** new manifest before installing it. */
  if( nLocal>0 ){
    bcvManifestExpand(&rc, &pMan, nLocal);
    for(iOld=(pOld->nDb-nLocal); rc==SQLITE_OK && iOld<pOld->nDb; iOld++){
      ManifestDb *pOldDb = &pOld->aDb[iOld];
      ManifestDb *pNewDb = &pMan->aDb[pMan->nDb++];

      assert( pOldDb->nBlkOrigAlloc || pOldDb->aBlkOrig==0 );
      pNewDb->aBlkOrig = bcvMallocCopy(&rc, 
          pOldDb->aBlkOrig, nName*pOldDb->nBlkOrig
      );
      pNewDb->nBlkOrigAlloc = pNewDb->nBlkOrig = pOldDb->nBlkOrig;
      if( pOldDb->nBlkLocalAlloc ){
        pNewDb->aBlkLocal = bcvMallocCopy(&rc, 
            pOldDb->aBlkLocal, nName*pOldDb->nBlkLocal
        );
        pNewDb->nBlkLocalAlloc = pOldDb->nBlkLocal;
      }else{
        pNewDb->aBlkLocal = pNewDb->aBlkOrig;
      }
      pNewDb->nBlkLocal = pOldDb->nBlkLocal;

      pNewDb->iDbId = pOldDb->iDbId;
      pNewDb->iVersion = pOldDb->iVersion;
      pNewDb->iParentId = pOldDb->iParentId;
      memcpy(pNewDb->zDName, pOldDb->zDName, BCV_DBNAME_SIZE);
    }
  }

  if( rc==SQLITE_OK ){
    rc = bcvManifestInstall(pCommon, pCont, pMan);
  }else{
    bcvManifestDeref(pMan);
  }

  while( pList ){
    const int lockflags = SQLITE_SHM_UNLOCK|SQLITE_SHM_EXCLUSIVE;
    FileWrapper *pNext = pList->pNext;
    sqlite3_file *pFd = pList->pFd;
    if( pList->bUnmap ){
      if( pList->bUnlock ) pFd->pMethods->xShmLock(pFd, 0, 1, lockflags);
      pFd->pMethods->xShmUnmap(pFd, 0);
    }
    bcvCloseLocal(pFd);
    sqlite3_free(pList);
    pList = pNext;
  }

  return rc;
}

/*
** Search the BcvCommon object passed as the first argument for a container
** object attached with the alias zAlias. If one is found, return a pointer to
** it. Otherwise, return NULL. 
**
** If no container is found and parameter pzErr is not NULL, set *pzErr
** to point to a buffer containing an English language error message.
** It is the responsibility of the caller to free the buffer using
** sqlite3_free().
*/
Container *bcvfsFindContAlias(
  BcvCommon *p,                   /* Search this object */
  const char *zAlias,             /* Name of container to search for */
  char **pzErr                    /* OUT: Error message */
){
  Container *pCont;
  for(pCont=p->pCList; pCont; pCont=pCont->pNext){
    if( strcmp(zAlias, pCont->zName)==0 ){
      break;
    }
  }
  if( pCont==0 && pzErr ){
    *pzErr = sqlite3_mprintf("no such container: %s", zAlias);
  }
  return pCont;
}


static int bcvfsPollContainer(
  sqlite3_bcvfs *pFs, 
  const char *zContainer,
  char **pzErr
){
  int rc = SQLITE_OK;
  int iIter = 0;
  char *zErr = 0;
  do {
    Container *pCont = 0;
    BcvDispatch *pDisp = 0;
    BcvContainer *pBcv = 0;
    Manifest *pMan = 0;

    sqlite3_free(zErr);
    zErr = 0;

    ENTER_VFS_MUTEX; {
      pCont = bcvfsFindContAlias(&pFs->c, zContainer, &zErr);
      if( pCont ){
        bcvfsEnterState(pFs, pCont, CONTAINER_STATE_POLL);
        rc = bcvfsContainerGetBcv(pFs, pCont, &pDisp, &pBcv, 0);
      }else{
        rc = SQLITE_ERROR;
      }
    } LEAVE_VFS_MUTEX;

    /* Download the manifest file for this container */
    if( rc==SQLITE_OK ){
      rc = bcvfsFetchManifest(pDisp, pBcv, &pMan, &zErr);
    }

    assert( (rc==SQLITE_OK)==(pMan!=0) );
    ENTER_VFS_MUTEX; {
      if( rc==SQLITE_OK ){
        rc = bcvManifestUpdate(&pFs->c, pCont, pMan, &zErr);
      }
      if( pCont ){
        bcvfsLeaveState(pFs, pCont);
        bcvfsContainerReleaseBcv(pFs, pCont, pDisp, pBcv, rc!=SQLITE_OK);
      }
    } LEAVE_VFS_MUTEX;
  }while( rc==HTTP_AUTH_ERROR && (++iIter)<BCVFS_MAX_AUTH_RETRIES );

  *pzErr = zErr;
  return rc;
}

/*
** Poll the container for a new manifest.
*/
static int bcvfsPoll(BcvfsFile *pFile, char **pzErr){
  sqlite3 *db = *pFile->ppDb;

  /* Cannot do a poll if there is an open read or write transaction on 
  ** the file.  Return early if this is the case. */
  if( pFile->lockMask || 0==sqlite3_get_autocommit(db) ){
    return SQLITE_BUSY;
  }

  return sqlite3_bcvfs_poll(pFile->pFs, pFile->pPath->zContainer, pzErr);
}


static int bcvfsUploadWithRetry(BcvfsFile *pFile, char **pzErr);

/*
** Handle SQLITE_FCNTL_PRAGMA file-controls.
*/
static int bcvfsPragma(BcvfsFile *pFile, void *pArg){
  sqlite3_bcvfs *pFs = pFile->pFs;
  char **azArg = (char**)pArg;
  const char *zPragma = azArg[1];
  int rc = SQLITE_NOTFOUND;
  char *zErr = 0;
  if( 0==sqlite3_stricmp("bcv_upload", zPragma) ){
    if( pFile->pFs->zPortnumber==0 ){
      pFile->rc = rc = bcvfsUploadWithRetry(pFile, &zErr);
    }else{
      rc = SQLITE_OK;
    }
  }
  if( 0==sqlite3_stricmp("bcv_poll", zPragma) ){
    rc = bcvfsPoll(pFile, &zErr);
  }
  if( 0==sqlite3_stricmp("bcv_blocksize", zPragma) ){
    int szBlk;
    ENTER_VFS_MUTEX; {
      szBlk = pFile->pFs->c.szBlk;
    } LEAVE_VFS_MUTEX;
    zErr = sqlite3_mprintf("%d", szBlk);
    rc = (zErr ? SQLITE_OK : SQLITE_NOMEM);
  }
  if( 0==sqlite3_stricmp("journal_mode", zPragma) ){
    rc = SQLITE_ERROR;
    zErr = sqlite3_mprintf("cannot use \"PRAGMA journal_mode\" with bcvfs");
  }

  rc = bcvErrorToSqlite(rc);
  azArg[0] = zErr;
  return rc;
}

static int bcvfsFileControl(sqlite3_file *pFd, int op, void *pArg){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  int rc = SQLITE_NOTFOUND;

  switch( op ){
    case SQLITE_FCNTL_PRAGMA: {
      rc = bcvfsPragma(pFile, pArg);
      break;
    }
    case SQLITE_FCNTL_PDB: {
      pFile->ppDb = (sqlite3**)pArg;
      break;
    }
    case BCV_FCNTL_FD: {
      *((BcvfsFile**)pArg) = pFile;
      rc = SQLITE_OK;
      break;
    }
  }

  if( rc==SQLITE_NOTFOUND && pFile->pFile ){
    rc = pFile->pFile->pMethods->xFileControl(pFile->pFile, op, pArg);
  }
  return rc;
}
static int bcvfsSectorSize(sqlite3_file *pFd){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  if( bcvfsFileType(pFile)==BCVFS_NO_FILE ) return 512;
  return pFile->pFile->pMethods->xSectorSize(pFile->pFile);
}
static int bcvfsDeviceCharacteristics(sqlite3_file *pFd){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  if( bcvfsFileType(pFile)==BCVFS_NO_FILE ) return 0;
  return pFile->pFile->pMethods->xDeviceCharacteristics(pFile->pFile);
}
static int bcvfsShmMap(
  sqlite3_file *pFd, 
  int iPg, 
  int pgsz, 
  int eMode, 
  void volatile **pp
){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  return pFile->pFile->pMethods->xShmMap(pFile->pFile, iPg, pgsz, eMode, pp);
}

static int bcvfsShmLock(sqlite3_file *pFd, int offset, int n, int flags){
  static const int iCkptLock = 1;
  static const int iReadLock0 = 3;

  BcvfsFile *pFile = (BcvfsFile*)pFd;
  sqlite3_bcvfs *pFs = pFile->pFs;
  int rc = SQLITE_OK;
  u32 mask = (1 << (offset+n)) - (1 << offset);

  /* If the bHoldCkpt flag is set and this call will unlock the CHECKPOINTER
  ** lock, then the lock is not actually released. Specifically, the
  ** bit in BcvfsFile.lockMask remains set and the underlying file is not
  ** unlocked. However, if there are no other locks on the file (there never
  ** are) the manifest reference is still released.  
  **
  ** The assert() verifies that if this is call to unlock CHECKPOINTER, it
  ** does not also unlock any other slots. */
  if( pFile->bHoldCkpt ){
    assert( (mask & (1<<iCkptLock))==0
        || (flags & SQLITE_SHM_UNLOCK)==0
        || (n==1 && offset==iCkptLock)
    );
    if( flags & SQLITE_SHM_UNLOCK ){
      mask &= ~(1<<iCkptLock);
    }
  }

  /* If this is a daemon mode VFS and the lock just obtained was 
  ** a SHARED lock on read-lock slot 0, send a READ message 
  ** requesting block 0 to open a read-transction within the daemon. */
  if( (flags & SQLITE_SHM_LOCK) 
   && pFile->p.fdProxy!=INVALID_SOCKET
   && pFile->lockMask==0 
   && mask==(1<<iReadLock0)
  ){
    rc = bcvfsProxyOpenTransaction(pFile, 0);
  }
  if( rc==SQLITE_OK ){
    rc = pFile->pFile->pMethods->xShmLock(pFile->pFile, offset, n, flags);
  }

  if( rc==SQLITE_OK ){
    assert( (flags & SQLITE_SHM_UNLOCK) || (flags & SQLITE_SHM_LOCK) );
    if( flags & SQLITE_SHM_UNLOCK ){
      pFile->lockMask &= ~mask;
    }

    if( (pFile->lockMask==0) 
     || (pFile->bHoldCkpt && pFile->lockMask==(1<<iCkptLock))
    ){
      ENTER_VFS_MUTEX; {
        bcvManifestDeref(pFile->pMan);
        pFile->pMan = 0;
        pFile->pManDb = 0;
      } LEAVE_VFS_MUTEX;
    }

    if( flags & SQLITE_SHM_LOCK ){
      pFile->lockMask |= mask;
    }

  }
  bcvfsFileCacheFreeIf(pFile);
  bcvfsProxyCloseTransactionIf(pFile);
  return rc;
}

static void bcvfsShmBarrier(sqlite3_file *pFd){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  return pFile->pFile->pMethods->xShmBarrier(pFile->pFile);
}
static int bcvfsShmUnmap(sqlite3_file *pFd, int deleteFlag){
  BcvfsFile *pFile = (BcvfsFile*)pFd;
  return pFile->pFile->pMethods->xShmUnmap(pFile->pFile, deleteFlag);
}

/*
** Create local directory for the specified path, if it does not 
** already exist.
*/
static void bcvfsCreateDir(
  int *pRc, 
  sqlite3_bcvfs *pFs, 
  Container *pCont,
  char **pzErr
){
  BcvBuffer b = {0,0,0};
  struct stat buf;
  const char *zDir = 0;

  bcvBufferAppendString(pRc, &b, pFs->c.zDir);
  bcvBufferAppendString(pRc, &b, BCV_PATH_SEPARATOR);
  bcvBufferAppendEscaped(pRc, &b, pCont->zName);
  zDir = (const char*)b.aData;
  if( *pRc==SQLITE_OK && stat(zDir, &buf)<0 ){
    if( osMkdir(zDir, 0755)<0 && stat(zDir, &buf)<0 ){
      *pRc = SQLITE_CANTOPEN;
      *pzErr = sqlite3_mprintf("failed to create directory: %s", pCont->zName);
    }
  }
  bcvBufferZero(&b);
}


static int bcvParseAddr(const char *zAddr, struct sockaddr_in *pAddr){
  int i;
  int nAddr = strlen(zAddr);
  char *zCopy = (char*)sqlite3_malloc(nAddr+1);
  int iPort;

  memset(pAddr, 0, sizeof(struct sockaddr_in));
  if( zCopy==0 ){
    return SQLITE_NOMEM;
  }
  memcpy(zCopy, zAddr, nAddr+1);

  /* Find the ':' in the address */
  for(i=0; zCopy[i]!=':'; i++){
    if( zCopy[i]==0 ){
      sqlite3_free(zCopy);
      goto parse_error;
    }
  }

  zCopy[i] = '\0';
  iPort = bcvParseInt((const u8*)&zCopy[i+1], -1);
  pAddr->sin_family = AF_INET;
  pAddr->sin_port = htons(iPort);
  pAddr->sin_addr.s_addr = inet_addr(zCopy);
  sqlite3_free(zCopy);
  if( pAddr->sin_addr.s_addr!=INADDR_NONE ){
    return SQLITE_OK;
  }

 parse_error:
  sqlite3_log(SQLITE_CANTOPEN, "failed to parse address: %s", zAddr);
  return SQLITE_CANTOPEN;
}


/*
** Attempt to open a new socket connection to the IP:PORT in string
** zAddr. If successful, set (*pSocket) to point to the new socket handle 
** and return SQLITE_OK. Or, if an error occurs, return an SQLite error code.
** The final value of (*pSocket) is undefined in this case.
*/
static BCV_SOCKET_TYPE bcvConnectSocket(
  int *pRc,
  const char *zAddr,              /* ipv4 address:port to connect to */
  char **pzErr                    /* OUT: Error message (if any) */
){
  int rc = *pRc;
  struct sockaddr_in addr;
  BCV_SOCKET_TYPE s = INVALID_SOCKET;

  if( rc==SQLITE_OK ){
    rc = bcvParseAddr(zAddr, &addr);
  }

  if( rc==SQLITE_OK ){
    s = socket(AF_INET, SOCK_STREAM, 0);
    if( bcv_socket_is_valid(s)==0 ){
      rc = SQLITE_CANTOPEN;
    }else{
      assert( addr.sin_addr.s_addr!=INADDR_NONE );
      if( connect(s, (struct sockaddr*)&addr, sizeof(addr))<0 ){
        rc = SQLITE_CANTOPEN;
        bcv_close_socket(s);
      }
    }
    if( pzErr && rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("failed to connect to daemon: %s", zAddr);
    }
  }

  *pRc = rc;
  return s;
}

/*
** Parameter pFs must be a proxy VFS. This function opens a new socket to
** connect to the daemon to access container zCont, and optionally database
** zDb.
*/
static int bcvfsProxyConnect(
  sqlite3_bcvfs *pFs,             /* Proxy VFS */
  const char *zCont,              /* Container to open */
  const char *zDb,                /* Name of db to open, or NULL */
  BCV_SOCKET_TYPE *pS,            /* OUT: Open socket */
  BcvMessage **ppReply            /* OUT: HELLO_REPLY message */
){
  BCV_SOCKET_TYPE s = INVALID_SOCKET;
  int rc = SQLITE_OK;
  BcvMessage *pReply = 0;

  s = bcvConnectSocket(&rc, pFs->zPortnumber, 0);
  if( rc==SQLITE_OK && zCont ){
    BcvMessage hello;
    memset(&hello, 0, sizeof(BcvMessage));
    hello.eType = BCV_MESSAGE_HELLO;
    hello.u.hello.zContainer = zCont;
    hello.u.hello.zDatabase = zDb;
    pReply = bcvExchangeMessage(&rc, s, &hello);
  }
  if( rc!=SQLITE_OK ){
    assert( pReply==0 );
    bcv_close_socket(s);
    s = INVALID_SOCKET;
  }

  *pS = s;
  *ppReply = pReply;
  return rc;
}

static int bcvfsOpenProxy(
  sqlite3_bcvfs *pFs, 
  BcvDbPath *pPath, 
  BcvfsFile *pFile
){
  int rc = SQLITE_OK;
  BcvMessage *pReply = 0;
  rc = bcvfsProxyConnect(
      pFs, pPath->zContainer, pPath->zDatabase, &pFile->p.fdProxy, &pReply
  );
  if( pReply ){
    if( rc==SQLITE_OK ){
      rc = pReply->u.hello_r.errCode;
    }
    if( rc==SQLITE_OK ){
      pFile->p.zStorage = bcvStrdupRc(&rc, pReply->u.hello_r.zStorage);
      pFile->p.zAccount = bcvStrdupRc(&rc, pReply->u.hello_r.zAccount);
      pFile->p.zContainer = bcvStrdupRc(&rc, pReply->u.hello_r.zContainer);
      pFile->p.szBlk = pReply->u.hello_r.szBlk;
      pFile->iFileDbId = pReply->u.hello_r.iDbId;

      if( pReply->u.hello_r.bEncrypted ){
        BcvMessage msg;
        BcvMessage *pReply2 = 0;
        memset(&msg, 0, sizeof(msg));
        msg.eType = BCV_MESSAGE_PASS;
        msg.u.pass.zAuth = bcvfsProxyGetAuth(&rc, pFile);
        pReply2 = bcvExchangeMessage(&rc, pFile->p.fdProxy, &msg);
        if( pReply2 ){
          rc = pReply2->u.pass_r.errCode;
          if( rc==SQLITE_OK ){
            pFile->p.pKey = bcvEncryptionKeyNew(pReply2->u.pass_r.aKey);
            if( pFile->p.pKey==0 ){
              rc = SQLITE_NOMEM;
            }
          }
          sqlite3_free(pReply2);
        }
      }
    }
    sqlite3_free(pReply);
  }
  return rc;
}

/*
** Open an bcv file handle.
*/
static int bcvfsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  int flags,
  int *pOutFlags
){
  static const sqlite3_io_methods bcvfs_io_methods = {
    2,                            /* iVersion */
    bcvfsClose,                   /* xClose */
    bcvfsRead,                    /* xRead */
    bcvfsWrite,                   /* xWrite */
    bcvfsTruncate,                /* xTruncate */
    bcvfsSync,                    /* xSync */
    bcvfsFileSize,                /* xFileSize */
    bcvfsLock,                    /* xLock */
    bcvfsUnlock,                  /* xUnlock */
    bcvfsCheckReservedLock,       /* xCheckReservedLock */
    bcvfsFileControl,             /* xFileControl */
    bcvfsSectorSize,              /* xSectorSize */
    bcvfsDeviceCharacteristics,   /* xDeviceCharacteristics */
    bcvfsShmMap,                  /* xShmMap */
    bcvfsShmLock,                 /* xShmLock */
    bcvfsShmBarrier,              /* xShmBarrier */
    bcvfsShmUnmap,                /* xShmUnmap */
    0,                            /* xFetch */
    0                             /* xUnfetch */
  };
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  const int bIsMain = !!(flags & SQLITE_OPEN_MAIN_DB);
  BcvfsFile *pRet = (BcvfsFile*)pFile;
  int rc = SQLITE_OK;
  BcvDbPath *pPath = 0;

  memset(pRet, 0, sizeof(BcvfsFile));
  pRet->base.pMethods = &bcvfs_io_methods;
  pRet->pFile = (sqlite3_file*)&pRet[1];
  pRet->pFs = pFs;
  pRet->bIsMain = bIsMain;
  pRet->p.fdProxy = INVALID_SOCKET;
  ENTER_VFS_MUTEX; {
    pRet->pFs->nRef++;
  } LEAVE_VFS_MUTEX;

  assert( (flags & SQLITE_OPEN_SUPER_JOURNAL)==0 );

  if( (flags & SQLITE_OPEN_MAIN_JOURNAL) 
   || (pFs->zPortnumber && (flags & SQLITE_OPEN_WAL))
  ){
    flags &= ~(SQLITE_OPEN_MAIN_JOURNAL|SQLITE_OPEN_WAL);
    flags |= SQLITE_OPEN_TEMP_JOURNAL|SQLITE_OPEN_DELETEONCLOSE;
    zName = 0;
  }

  if( (flags & SQLITE_OPEN_TEMP_DB)
   || (flags & SQLITE_OPEN_TEMP_JOURNAL)
   || (flags & SQLITE_OPEN_SUBJOURNAL)
   || (flags & SQLITE_OPEN_TRANSIENT_DB)
  ){
    /* This is a temp file. Pass it directly through to the underlying VFS. */
    rc = pFs->c.pVfs->xOpen(pFs->c.pVfs, zName, pRet->pFile, flags, pOutFlags);
  }else{

    /* Decode the path passed to this function. */
    rc = bcvfsDecodeName(pFs, zName, &pPath);
    if( rc==SQLITE_OK ){
      if( pFs->zPortnumber ){
        assert( bIsMain );
        rc = bcvfsOpenProxy(pFs, pPath, pRet);
        flags = flags & ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags = flags | SQLITE_OPEN_READONLY;
      }else if( pPath->zContainer ){
        ENTER_VFS_MUTEX; {
          Container *pCont = bcvfsFindContAlias(&pFs->c, pPath->zContainer, 0);
          if( pCont==0 ){
            rc = SQLITE_CANTOPEN;
          }else{
            if( pPath->zDatabase ){
              int nDb = strlen(pPath->zDatabase);
              ManifestDb *pDb;
              if( !bIsMain ) nDb -= 4;
              pDb = bcvfsFindDatabase(pCont->pMan, pPath->zDatabase, nDb);
              if( pDb==0 || pDb->nBlkLocal==0 ){
                rc = SQLITE_CANTOPEN;
              }else{
                pRet->iFileDbId = pDb->iDbId;
              }
            }
            pCont->nClient++;
            pRet->pCont = pCont;
            if( bIsMain ){
              pRet->pNextFile = pFs->pFileList;
              pFs->pFileList = pRet;
            }
          }
        } LEAVE_VFS_MUTEX;
      }
    }

    if( rc==SQLITE_OK ){
      sqlite3_vfs *pVfs = pFs->c.pVfs;
      if( pPath->zDatabase ){
        char *zLocal = bcvfsLocalPath(&rc, pFs, pPath);
        if( rc==SQLITE_OK ){
          int fout = 0;
          int f = (flags & ~SQLITE_OPEN_READONLY);
          f |= SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;
          rc = pVfs->xOpen(pVfs, zLocal, pRet->pFile, f, &fout);
          if( rc==SQLITE_OK && bIsMain ){
            rc = pRet->pFile->pMethods->xWrite(pRet->pFile,(void*)"bcvfs",5,0);
          }
          if( flags & SQLITE_OPEN_READONLY ){
            fout &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
            fout |= SQLITE_OPEN_READONLY;
          }
          *pOutFlags = fout;
          if( rc==SQLITE_OK && bIsMain ){
            rc = bcvOpenLocal(pFs->zCacheFile, 0, 0, &pRet->pCacheFile);
          }
          pRet->pPath = pPath;
          pRet->zLocalPath = zLocal;
          pPath = 0;
        }
      }else{
        pRet->pFile = 0;
      }
    }
    sqlite3_free(pPath);
  }

  if( rc!=SQLITE_OK ){
    bcvfsClose(pFile);
    memset(pFile, 0, sizeof(sqlite3_file));
  }
  return rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static int bcvfsDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  int rc = SQLITE_OK;
  BcvDbPath *pPath = 0;

  rc = bcvfsDecodeName(pFs, zPath, &pPath);
  if( rc==SQLITE_OK && pPath->zDatabase ){
    char *zLocal = bcvfsLocalPath(&rc, pFs, pPath);
    if( rc==SQLITE_OK ){
      rc = pFs->c.pVfs->xDelete(pFs->c.pVfs, zLocal, dirSync);
      sqlite3_free(zLocal);
    }
  }
  sqlite3_free(pPath);
  return rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int bcvfsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int flags, 
  int *pResOut
){
  int rc = SQLITE_OK;
  *pResOut = 0;
  return rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. For this VFS, the output is a simple copy 
** of the input.
*/
static int bcvfsFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int nOut, 
  char *zOut
){
  int nCopy = bcvStrlen(zPath);
  if( nCopy>(nOut-1) ){ 
    return SQLITE_CANTOPEN;
  }
  memcpy(zOut, zPath, nCopy);
  zOut[nCopy] = '\0';
  return SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *bcvfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  return pFs->c.pVfs->xDlOpen(pFs->c.pVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void bcvfsDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  pFs->c.pVfs->xDlError(pFs->c.pVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*bcvfsDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  return pFs->c.pVfs->xDlSym(pFs->c.pVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void bcvfsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  pFs->c.pVfs->xDlClose(pFs->c.pVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static int bcvfsRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  return pFs->c.pVfs->xRandomness(pFs->c.pVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static int bcvfsSleep(sqlite3_vfs *pVfs, int nMicro){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  return pFs->c.pVfs->xSleep(pFs->c.pVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int bcvfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  return pFs->c.pVfs->xCurrentTime(pFs->c.pVfs, pTimeOut);
}

static int bcvfsGetLastError(sqlite3_vfs *pVfs, int a, char *b){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  return pFs->c.pVfs->xGetLastError(pFs->c.pVfs, a, b);
}
static int bcvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  sqlite3_bcvfs *pFs = (sqlite3_bcvfs*)pVfs;
  return pFs->c.pVfs->xCurrentTimeInt64(pFs->c.pVfs, p);
}

static const char *bcvfsStoreString(char **pzCsr, const char *zIn){
  int nByte = strlen(zIn)+1;
  char *zRet = *pzCsr;
  memcpy(zRet, zIn, nByte);
  *pzCsr += nByte;
  return (const char*)zRet;
}

Container *bcvContainerAlloc(
  int *pRc,
  BcvCommon *p,
  const char *zStorage,
  const char *zAccount,
  const char *zContainer,
  const char *zAlias
){
  int nByte = sizeof(Container);;
  int nDir = strlen(p->zDir);
  Container *pCont = 0;
  nByte += strlen(zStorage) + 1;
  nByte += strlen(zAccount) + 1;
  nByte += strlen(zContainer) + 1;
  nByte += strlen(zAlias) + 1;
  nByte += nDir + 1 + strlen(zAlias)*3 + 1;

  pCont = (Container*)bcvMallocRc(pRc, nByte);
  if( pCont ){
    char *pCsr = (char*)&pCont[1];
    pCont->zName = bcvfsStoreString(&pCsr, zAlias);
    pCont->zStorage = bcvfsStoreString(&pCsr, zStorage);
    pCont->zAccount = bcvfsStoreString(&pCsr, zAccount);
    pCont->zContainer = bcvfsStoreString(&pCsr, zContainer);
    pCont->zLocalDir = pCsr;
    memcpy(pCsr, p->zDir, nDir);
    pCsr[nDir] = BCV_PATH_SEPARATOR[0];
    pCsr = &pCsr[nDir+1];
    bcvfsPutEscaped(&pCsr, zAlias);
  }
  return pCont;
}

static sqlite3_stmt *bcvfsPrepare(int *pRc, BcvCommon *p, const char *zSql){
  sqlite3_stmt *pRet = 0;
  if( *pRc==SQLITE_OK ){
    *pRc = sqlite3_prepare_v2(p->bdb, zSql, -1, &pRet, 0);
  }
  return pRet;
}

static void bcvfsFinalize(int *pRc, sqlite3_stmt *p){
  int rc = sqlite3_finalize(p);
  if( *pRc==SQLITE_OK ){
    *pRc = rc;
  }
}

/*
** Usage:
**
**    assert( no_blocks_leaked(pCommon) );
*/
static int no_blocks_leaked(BcvCommon *p){
  if( p->nBlk>0 ){
    CacheEntry *pEntry = 0;
    int nByte = (p->nBlk + 7) / 8;
    int ii;
    u8 *aBit = (u8*)malloc(nByte);
    memset(aBit, 0, nByte);
    int nTotal = 0;

    for(pEntry=p->pUnused; pEntry; pEntry=pEntry->pHashNext){
      int iElem = pEntry->iPos / 8;
      int iBit = pEntry->iPos & 0x07;
      aBit[iElem] = aBit[iElem] | (1 << iBit);
      nTotal++;
    }
    for(ii=0; ii<p->nHash; ii++){
      for(pEntry=p->aHash[ii]; pEntry; pEntry=pEntry->pHashNext){
        int iElem = pEntry->iPos / 8;
        int iBit = pEntry->iPos & 0x07;
        aBit[iElem] = aBit[iElem] | (1 << iBit);
        nTotal++;
      }
    }

    for(ii=0; ii<p->nBlk; ii++){
      int iElem = ii / 8;
      int iBit = ii & 0x07;
      assert( aBit[iElem] & (1 << iBit) );
    }
    assert( nTotal==p->nBlk );

    free(aBit);
  }

  return 1;
}

/*
** The VFS object passed as the only argument has just been created. It
** has not yet been passed to sqlite3_vfs_register() to make it available
** to clients. This function initializes the internal data structures
** based on the contents of the blocksdb.bcv database file ("block" and
** "container" tables).
*/
int bcvfsCacheInit(BcvCommon *p, sqlite3 *db, char *zFullDir){
  int rc = SQLITE_OK;
  const char *zSql1 = "SELECT * FROM container";
  const char *zSql2 = "SELECT * FROM block ORDER BY cachefilepos";
  const char *zSql4 = 
    "WITH ss(i) AS ("
    "  SELECT -1 UNION ALL SELECT i+1 FROM ss WHERE (i+1)<?"
    ") SELECT i FROM ss WHERE i>=0 AND NOT EXISTS ("
    "  SELECT 1 FROM block WHERE cachefilepos=i"
    ")";
  sqlite3_stmt *p1 = 0;
  sqlite3_stmt *p2 = 0;
  sqlite3_stmt *p4 = 0;

  p->bdb = db;
  p->zDir = zFullDir;
  p->nHash = 128;
  p->aHash = (CacheEntry**)bcvMallocRc(&rc, sizeof(CacheEntry*)*p->nHash);
  p->pInsertCont = bcvfsPrepare(&rc, p, BCVFS_INSERT_CONT);
  p->pInsertBlock = bcvfsPrepare(&rc, p, BCVFS_INSERT_BLOCK);
  p->nMaxCache = BCV_DEFAULT_CACHESIZE;

  /* Remove any entries that are not either cloud blocks or dirty blocks */
  bcvExecPrintf(&rc, p->bdb,
      "DELETE FROM block WHERE blockid IS NULL AND container IS NULL"
  );

  /* Load the contents of the "container" table into memory */
  p1 = bcvfsPrepare(&rc, p, zSql1);
  while( rc==SQLITE_OK && sqlite3_step(p1)==SQLITE_ROW ){
    const char *zName = (const char*)sqlite3_column_text(p1, 0);
    const char *zStorage = (const char*)sqlite3_column_text(p1, 1);
    const char *zUser = (const char*)sqlite3_column_text(p1, 2);
    const char *zContainer = (const char*)sqlite3_column_text(p1, 3);
    const u8 *aMan = sqlite3_column_blob(p1, 4);
    int nMan = sqlite3_column_bytes(p1, 4);
    const char *zETag = (const char*)sqlite3_column_text(p1, 5);
    char *zErr = 0;
    Container *pCont;

    pCont = bcvContainerAlloc(&rc, p, zStorage, zUser, zContainer, zName);
    if( pCont==0 ) break;
    pCont->pNext = p->pCList;
    p->pCList = pCont;
    
    rc = bcvManifestParseCopy(aMan, nMan, zETag, &pCont->pMan, &zErr);
    sqlite3_free(zErr);
  }
  bcvfsFinalize(&rc, p1);
  if( rc==SQLITE_OK && p->pCList ){
    p->szBlk = p->pCList->pMan->szBlk;
  }

  /* Load the contents of the "block" table into memory */
  p2 = bcvfsPrepare(&rc, p, zSql2);
  while( rc==SQLITE_OK && sqlite3_step(p2)==SQLITE_ROW ){
    CacheEntry *pNew = (CacheEntry*)bcvMallocRc(&rc, sizeof(CacheEntry));
    if( pNew ){
      const u8 *aName;
      pNew->iPos = sqlite3_column_int(p2, 0);
      aName = sqlite3_column_blob(p2, 1);
      if( aName ){
        pNew->nName = sqlite3_column_bytes(p2, 1);
        memcpy(pNew->aName, aName, pNew->nName);
        pNew->bValid = 1;
        bcvfsLruAdd(p, pNew);
      }else{
        const char *zContainer = (const char*)sqlite3_column_text(p2, 2);
        i64 iDbId = sqlite3_column_int64(p2, 3);
        i64 iDbPos = sqlite3_column_int64(p2, 4);
        Container *pCont = bcvfsFindContAlias(p, zContainer, 0);
        ManifestDb *pManDb = 0;
        if( pCont ){
          int ii;
          for(ii=0; ii<pCont->pMan->nDb; ii++){
            if( pCont->pMan->aDb[ii].iDbId==iDbId ){
              pManDb = &pCont->pMan->aDb[ii];
              break;
            }
          }
        }
        if( pManDb==0 ){
          rc = SQLITE_CORRUPT;
        }else{
          pNew->bValid = 1;
          pNew->bDirty = 1;
          pNew->nName = NAMEBYTES(pCont->pMan);
          sqlite3_randomness(pNew->nName, pNew->aName);
          bcvfsManifestWrite(pManDb, iDbPos, pNew->aName, pNew->nName);
        }
      }
      
      bcvfsHashAdd(p, pNew);
      p->nBlk = pNew->iPos + 1;
    }
  }
  bcvfsFinalize(&rc, p2);

  p4 = bcvfsPrepare(&rc, p, zSql4);
  if( p4 ){
    sqlite3_bind_int(p4, 1, p->nBlk);
  }
  while( rc==SQLITE_OK && sqlite3_step(p4)==SQLITE_ROW ){
    CacheEntry *pNew = (CacheEntry*)bcvMallocRc(&rc, sizeof(CacheEntry));
    if( pNew ){
      pNew->iPos = sqlite3_column_int(p4, 0);
      pNew->pHashNext = p->pUnused;
      p->pUnused = pNew;
    }
  }
  bcvfsFinalize(&rc, p4);
  assert( no_blocks_leaked(p) );
  return rc;
}

/*
** This function opens and initializes the schema of the "blocksdb.bcv"
** database in directory zDir. If successful, the new database handle
** is returned.
**
** If parameter (*pRc) is not SQLITE_OK when this function is invoked, it
** is a no-op. If an error occurs within this function, (*pRc) is set to
** an SQLite error code and NULL returned.
*/
sqlite3 *bcvOpenAndInitDb(int *pRc, const char *zDir, char **pzErr){
  int rc = *pRc;
  sqlite3 *db = 0;

  /* Open and initialize the blocks.db database */
  if( rc==SQLITE_OK ){
    char *zBlocksDb = sqlite3_mprintf(
        "%s" BCV_PATH_SEPARATOR "%s", zDir, BCV_DATABASE_NAME
    );
    if( zBlocksDb==0 ){
      rc = SQLITE_NOMEM;
    }else{
      int f = SQLITE_OPEN_FULLMUTEX|SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;
      rc = sqlite3_open_v2(zBlocksDb, &db, f, 0);
      sqlite3_free(zBlocksDb);
      if( rc==SQLITE_OK ){
        rc = sqlite3_exec(db, 
            "PRAGMA locking_mode = exclusive;"
            "PRAGMA synchronous = OFF;"
            "BEGIN EXCLUSIVE;"
            BCV_DATABASE_SCHEMA
            "COMMIT;", 0, 0, pzErr
        );
      }else if( db && pzErr ){
        *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
        sqlite3_close(db);
      }
    }
  }

  *pRc = rc;
  return db;
}

/*
** Return the full path equivalent to (possibly) local path zDir.
*/
char *bcvGetFullpath(int *pRc, const char *zDir){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  char *zRet = 0;
  zRet = bcvMallocRc(pRc, pVfs->mxPathname+1);
  if( zRet ){
    int rc = pVfs->xFullPathname(pVfs, zDir, pVfs->mxPathname, zRet);
    if( rc ){
      sqlite3_free(zRet);
      zRet = 0;
      *pRc = rc;
    }
  }
  return zRet;
}

/*
** Open text file zFile and read the contents into a buffer obtained from
** sqlite3_malloc(). If successful, append a nul-terminator byte, set
** (*pzText) to point to the new buffer and return SQLITE_OK. Or, if an
** error occurs, return an SQLite error code. The final value of (*pzText)
** is undefined in this case.
*/
static int bcvReadTextFile(const char *zFile, char **pzText){
  int rc = SQLITE_OK;
  sqlite3_int64 sz = 0;
  sqlite3_file *pFile = 0;
  char *zRet = 0;

  rc = bcvOpenLocal(zFile, 0, 1, &pFile);
  if( rc==SQLITE_OK ){
    rc = pFile->pMethods->xFileSize(pFile, &sz);
  }

  if( rc==SQLITE_OK ){
    zRet = sqlite3_malloc(sz+1);
    if( zRet==0 ){
      rc = SQLITE_NOMEM;
    }else{
      zRet[sz] = '\0';
      rc = bcvReadfile(pFile, zRet, sz, 0);
    }
  }

  if( rc!=SQLITE_OK ){
    sqlite3_free(zRet);
    zRet = 0;
  }
  bcvCloseLocal(pFile);
  *pzText = zRet;
  return rc;
}

/*
** Look for a "portnumber.bcv" file. If one is found, read it and return its
** contents in a buffer allocated by sqlite3_malloc(). 
*/
static char *bcvReadPortnumber(int *pRc, const char *zDir){
  int rc = *pRc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  char *zFile = 0;
  char *zRet = 0;
  
  zFile = bcvMprintfRc(&rc, "%s/%s", zDir, BCV_PORTNUMBER_FILE);
  if( zFile ){
    int bExists = 0;
    rc = pVfs->xAccess(pVfs, zFile, SQLITE_ACCESS_EXISTS, &bExists);
    if( rc==SQLITE_OK && bExists ){
      rc = bcvReadTextFile(zFile, &zRet);
    }
  }

  *pRc = rc;
  sqlite3_free(zFile);
  return zRet;
}

/*
** This function is called as part of initializing the BcvCommon object
** within a local VFS. It returns the value that should be used as the
** next local database id - either BCVFS_FIRST_LOCAL_ID or 1 greater
** than the largest existing local database id.
*/
static i64 bcvfsFindNextLocalid(BcvCommon *p){
  Container *pCont;
  i64 iLocal = BCVFS_FIRST_LOCAL_ID;
  for(pCont=p->pCList; pCont; pCont=pCont->pNext){
    Manifest *pMan = pCont->pMan;
    int iDb;
    for(iDb=0; iDb<pMan->nDb; iDb++){
      i64 iId = pMan->aDb[iDb].iDbId;
      if( iId>=iLocal ) iLocal = iId+1;
    }
  }
  return iLocal;
}

/*
** Create a new blockcachevfs VFS in the current process.
*/
int sqlite3_bcvfs_create(
  const char *zDir,               /* Directory to use */
  const char *zName,              /* New VFS name */
  sqlite3_bcvfs **ppFs,           /* OUT: Handle for new object */
  char **pzErr                    /* OUT: Error message */
){
  static sqlite3_vfs bcv_template = {
    3,                            /* iVersion */
    sizeof(BcvfsFile),            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    bcvfsOpen,                    /* xOpen */
    bcvfsDelete,                  /* xDelete */
    bcvfsAccess,                  /* xAccess */
    bcvfsFullPathname,            /* xFullPathname */
    bcvfsDlOpen,                  /* xDlOpen */
    bcvfsDlError,                 /* xDlError */
    bcvfsDlSym,                   /* xDlSym */
    bcvfsDlClose,                 /* xDlClose */
    bcvfsRandomness,              /* xRandomness */
    bcvfsSleep,                   /* xSleep */
    bcvfsCurrentTime,             /* xCurrentTime */
    bcvfsGetLastError,            /* xGetLastError */
    bcvfsCurrentTimeInt64,        /* xCurrentTimeInt64 */
    0,                            /* xSetSystemCall */
    0,                            /* xGetSystemCall */
    0,                            /* xNextSystemCall */
  };
  int rc = SQLITE_OK;
  sqlite3_bcvfs *pFs = 0;
  char *zFullDir = 0;             /* Full path to vfs dir */
  char *zPortnumber = 0;          /* Contents of portnumber.bcv */

  if( pzErr ){
    *pzErr = 0;
  }

  rc = sqlite3_initialize();
  pFs = (sqlite3_bcvfs*)bcvMallocRc(&rc, sizeof(sqlite3_bcvfs));
  if( rc!=SQLITE_OK ) return rc;
  pFs->zName = bcvStrdupRc(&rc, zName);
  memcpy(&pFs->base, &bcv_template, sizeof(bcv_template));
  pFs->base.zName = pFs->zName;
  pFs->c.pVfs = sqlite3_vfs_find(0);
  pFs->base.mxPathname = pFs->c.pVfs->mxPathname;
  pFs->base.szOsFile += pFs->c.pVfs->szOsFile;
  pFs->iNextLocalId = BCVFS_FIRST_LOCAL_ID;

  zFullDir = bcvGetFullpath(&rc, zDir);
  pFs->zCacheFile = bcvMprintfRc(&rc,
      "%s" BCV_PATH_SEPARATOR "%s%c", zFullDir, BCV_CACHEFILE_NAME, '\0'
  );
  zPortnumber = bcvReadPortnumber(&rc, zFullDir);
  bcvfsMutexInit(&pFs->mutex);
  if( zPortnumber ){
    pFs->zPortnumber = zPortnumber;
    pFs->c.zDir = zFullDir;
    zFullDir = 0;
  }else{
    /* Open and initialize the blocks.db database */
    sqlite3 *db = bcvOpenAndInitDb(&rc, zFullDir, 0);
    pFs->nRequest = BCV_DEFAULT_NREQUEST;
    pFs->nHttpTimeout = BCV_DEFAULT_HTTPTIMEOUT;

    if( rc==SQLITE_OK ){
      ENTER_VFS_MUTEX; {
        rc = bcvfsCacheInit(&pFs->c, db, zFullDir);
        zFullDir = 0;
        db = 0;
        if( rc==SQLITE_OK ){
          pFs->iNextLocalId = bcvfsFindNextLocalid(&pFs->c);
        }
      } LEAVE_VFS_MUTEX;
    }

    sqlite3_close(db);
  }

  sqlite3_free(zFullDir);
  if( rc!=SQLITE_OK ){
    sqlite3_bcvfs_destroy(pFs);
    pFs = 0;
  }else{
    sqlite3_vfs_register(&pFs->base, 0);
  }

  *ppFs = pFs;
  return rc;
}

/* 
** Return true in daemon mode, false otherwise.
*/
int sqlite3_bcvfs_isdaemon(sqlite3_bcvfs *pFs){
  return (pFs->zPortnumber!=0);
}

/* 
** Free all BCV handles (or organize for the threads currently using
** them to free them when they are finished with them).
*/
static void bcvfsFreeAllBcv(sqlite3_bcvfs *pFs){
  Container *pCont;
  for(pCont=pFs->c.pCList; pCont; pCont=pCont->pNext){
    int ii;
    for(ii=0; ii<pCont->nBcv; ii++){
      BcvWrapper *pWrapper = &pCont->aBcv[ii];
      if( pWrapper->bUsable ){
        bcvContainerClose(pWrapper->pBcv);
        bcvDispatchFree(pWrapper->pDisp);
      }
    }
    sqlite3_free(pCont->aBcv);
    pCont->aBcv = 0;
    pCont->nBcv = 0;
  }
}

/*
** Configure a VFS object.
**
** This API only works with single-process objects. It always returns
** SQLITE_NOTFOUND if used on a proxy VFS.
*/
int sqlite3_bcvfs_config(sqlite3_bcvfs *pFs, int op, sqlite3_int64 iVal){
  int rc = SQLITE_NOTFOUND;
  if( pFs->zPortnumber==0 ){
    rc = SQLITE_OK;
    ENTER_VFS_MUTEX; {
      switch( op ){
        case SQLITE_BCV_CACHESIZE:
          pFs->c.nMaxCache = iVal;
          break;
        case SQLITE_BCV_NREQUEST:
          pFs->nRequest = iVal;
          break;
        case SQLITE_BCV_HTTPTIMEOUT:
          pFs->nHttpTimeout = iVal;
          break;
        case SQLITE_BCV_CURLVERBOSE:
          pFs->bCurlVerbose = (iVal ? 1 : 0);
          bcvfsFreeAllBcv(pFs);
          break;
        default:
          rc = SQLITE_NOTFOUND;
          break;
      }
    } LEAVE_VFS_MUTEX;
  }
  return rc;
}

/*
** Free the Container object passed as the only argument.
*/
void bcvContainerFree(Container *pCont){
  if( pCont ){
    int ii;
    for(ii=0; ii<pCont->nBcv; ii++){
      assert( pCont->aBcv[ii].pBcv==0 || pCont->aBcv[ii].bUsable );
      bcvContainerClose(pCont->aBcv[ii].pBcv);
      bcvDispatchFree(pCont->aBcv[ii].pDisp);
    }
    bcvManifestDeref(pCont->pMan);
    sqlite3_free(pCont->aBcv);
    bcvEncryptionKeyFree(pCont->pKey);
    sqlite3_free(pCont);
  }
}

void bcvExecPrintf(int *pRc, sqlite3 *db, const char *zFmt, ...){
  if( *pRc==SQLITE_OK ){
    char *zSql = 0;
    va_list ap;
    va_start(ap, zFmt);
    zSql = sqlite3_vmprintf(zFmt, ap);
    va_end(ap);
    if( zSql==0 ){
      *pRc = SQLITE_NOMEM;
    }else{
      *pRc = sqlite3_exec(db, zSql, 0, 0, 0);
    }
    sqlite3_free(zSql);
  }
}

/*
** Remove all references to container pCont from the BcvCommon object
** passed as the first argument. Then free pCont itself.
*/
int bcvContainerDetachAndFree(BcvCommon *p, Container *pCont){
  int rc = SQLITE_OK;
  bcvExecPrintf(&rc, p->bdb,
      "DELETE FROM container WHERE name=%Q;"
      "DELETE FROM pinned WHERE container=%Q;"
      , pCont->zName, pCont->zName
  );
  if( rc==SQLITE_OK ){
    Container **pp;
    for(pp=&p->pCList; (*pp)!=pCont; pp=&(*pp)->pNext);
    *pp = pCont->pNext;
    bcvContainerFree(pCont);
  }
  return rc;
}

/*
** Free the contents of the BcvCommon structure passed as the only argument.
*/
void bcvCommonDestroy(BcvCommon *p){
  int rc;
  CacheEntry *pEntry;

  assert( no_blocks_leaked(p) );
  sqlite3_finalize(p->pInsertBlock);
  sqlite3_finalize(p->pInsertCont);
  rc = sqlite3_close(p->bdb);
  assert( rc==SQLITE_OK );

  while( p->pCList ){
    Container *pCont = p->pCList;
    p->pCList = pCont->pNext;
    assert( pCont->pMan==0 || pCont->pMan->nRef==1 );
    bcvContainerFree(pCont);
  }

  if( p->aHash ){
    int ii;
    for(ii=0; ii<p->nHash; ii++){
      while( (pEntry = p->aHash[ii]) ){
        p->aHash[ii] = pEntry->pHashNext;
        sqlite3_free(pEntry);
      }
    }
  }

  while( (pEntry = p->pUnused) ){
    p->pUnused = pEntry->pHashNext;
    sqlite3_free(pEntry);
  }

  sqlite3_free(p->aHash);
  sqlite3_free(p->zDir);
}

int sqlite3_bcvfs_destroy(sqlite3_bcvfs *pFs){
  int rc = SQLITE_OK;
  if( pFs ){
    ENTER_VFS_MUTEX; {
      if( pFs->nRef>0 ){
        rc = SQLITE_BUSY;
      }
    }; LEAVE_VFS_MUTEX;
    if( rc==SQLITE_OK ){
      sqlite3_vfs_unregister(&pFs->base);
      bcvCommonDestroy(&pFs->c);
      bcvfsMutexShutdown(&pFs->mutex);
      sqlite3_free(pFs->zName);
      sqlite3_free(pFs->zPortnumber);
      sqlite3_free(pFs->zCacheFile);
      sqlite3_free(pFs);
    }
  }
  return rc;
}

int sqlite3_bcvfs_log_callback(
  sqlite3_bcvfs *pFs,
  void *pLogCtx,
  int mLog,
  void(*xLog)(void *pCtx, int mLog, const char *zMsg)
){
  ENTER_VFS_MUTEX; {

    /* Free all BCV handles (or organize for the threads currently using
    ** them to free them when they are finished with them). This is because
    ** they have to be configured with logging callbacks when they are
    ** created, and the logging callback may be about to change. New BCV
    ** handles, configured with the new logging callback, will be created
    ** when they are next required.  */
    bcvfsFreeAllBcv(pFs);

    pFs->pLogCtx = pLogCtx;
    pFs->mLog = mLog;
    pFs->xLog = xLog;
  } LEAVE_VFS_MUTEX;
  return SQLITE_OK;
}

int sqlite3_bcvfs_auth_callback(
  sqlite3_bcvfs *pFs,
  void *pAuthCtx,
  int(*xAuth)(void*, const char*, const char*, const char*, char**)
){
  ENTER_VFS_MUTEX; {
    pFs->pAuthCtx = pAuthCtx;
    pFs->xAuth = xAuth;
  } LEAVE_VFS_MUTEX;
  return SQLITE_OK;
}

/*
** Attach a new container to the VFS.
**
**   1) Allocate a container object, so that it can be used to allocate
**      a BCV handle. The container object is not yet linked into the
**      main list of containers, so there is no chance that some other
**      thread will start using it before the initial manifest has been 
**      downloaded installed.
**
**   2) Fetch the manifest. This is done without the mutex.
**
**   3) Then, holding the mutex: If the manifest was successfully retrieved, 
**      the block-size is Ok, and the container is not already attached,
**      add it to the list of available containers.
**
** While the place-holder container is present:
**
**  * calls to this function specifying the same alias return SQLITE_BUSY.
**
**  * attempts to open a database within a container with the same alias
**    return SQLITE_CANTOPEN.
*/
static int bcvfsAttach(
  sqlite3_bcvfs *pFs,
  const char *zStorage,
  const char *zAccount,
  const char *zContainer,
  const char *zAliasArg,
  int flags,
  char **pzErr                    /* OUT: Error message */
){
  int rc = SQLITE_OK;
  char *zErr = 0;
  const char *zAlias = zAliasArg ? zAliasArg : zContainer;
  BcvDispatch *pDisp = 0;
  Manifest *pMan = 0;
  BcvContainer *pBcv = 0;
  Container *pCont = 0;

  /* Allocate a container object. And a bcv handle to access the remote
  ** cloud container. But do not link the new container into the main
  ** list yet.  */
  pCont = bcvContainerAlloc(
      &rc, &pFs->c, zStorage, zAccount, zContainer, zAlias
  );
  if( rc==SQLITE_OK ){
    ENTER_VFS_MUTEX; {
      rc = bcvfsContainerGetBcv(pFs, pCont, &pDisp, &pBcv, &zErr);
    } LEAVE_VFS_MUTEX;
  }

  /* Download the manifest file for this container */
  if( rc==SQLITE_OK ){
    rc = bcvfsFetchManifest(pDisp, pBcv, &pMan, &zErr);
  }
  assert( pMan==0 || rc==SQLITE_OK );

  ENTER_VFS_MUTEX; {
    BcvCommon *p = &pFs->c;
    bcvfsContainerReleaseBcv(pFs, pCont, pDisp, pBcv, 0);
    if( rc==SQLITE_OK ){
      Container *pExisting = bcvfsFindContAlias(p, zAlias, 0);
      if( pExisting ){
        rc = SQLITE_BUSY;
        if( 0==(flags & SQLITE_BCV_ATTACH_IFNOT) ){
          zErr = sqlite3_mprintf("container already attached: %s", zAlias);
        }
      }

      if( rc==SQLITE_OK && p->szBlk!=0 && pMan->szBlk!=p->szBlk ){
        rc = SQLITE_ERROR;
        zErr = sqlite3_mprintf("block size mismatch in container: %s", zAlias);
      }

      if( rc==SQLITE_OK ){
        rc = bcvManifestInstall(p, pCont, pMan);
        pMan = 0;
      }

      bcvfsCreateDir(&rc, pFs, pCont, &zErr);
      if( rc==SQLITE_OK ){
        pCont->pNext = p->pCList;
        p->pCList = pCont;
      }
    }
  } LEAVE_VFS_MUTEX;

  if( pCont && rc!=SQLITE_OK ){
    bcvContainerFree(pCont);
    bcvManifestDeref(pMan);
  }
  if( pzErr ){
    *pzErr = zErr;
  }else{
    sqlite3_free(zErr);
  }
  if( rc==SQLITE_BUSY && (flags & SQLITE_BCV_ATTACH_IFNOT) ){
    rc = SQLITE_OK;
  }
  return rc;
}

int proxyAttach(
  sqlite3_bcvfs *pFs,
  const char *zStorage,
  const char *zAccount,
  const char *zContainer,
  const char *zAliasArg,
  int flags,
  char **pzErr                    /* OUT: Error message */
){
  BcvMessage msg;
  BcvMessage *pReply = 0;
  int rc = SQLITE_OK;                       /* Error code */
  BCV_SOCKET_TYPE s = INVALID_SOCKET;       /* Socket connection to daemon */
  char *zAuth = 0;                          /* Authorization string */

  s = bcvConnectSocket(&rc, pFs->zPortnumber, pzErr);
  zAuth = bcvInvokeAuth(&rc, pFs, zStorage, zAccount, zContainer);

  memset(&msg, 0, sizeof(msg));
  msg.eType = BCV_MESSAGE_ATTACH;
  msg.u.attach.zStorage = zStorage;
  msg.u.attach.zAccount = zAccount;
  msg.u.attach.zContainer = zContainer;
  msg.u.attach.zAlias = zAliasArg;
  msg.u.attach.zAuth = zAuth;
  msg.u.attach.flags = (u32)flags;

  pReply = bcvExchangeMessage(&rc, s, &msg);
  if( rc==SQLITE_OK ){
    assert( pReply->eType==BCV_MESSAGE_REPLY );
    rc = pReply->u.error_r.errCode;
    if( pzErr && pReply->u.error_r.zErrMsg ){
      assert( rc!=SQLITE_OK );
      *pzErr = sqlite3_mprintf("%s", pReply->u.error_r.zErrMsg);
    }
    if( rc==SQLITE_OK ){
      ENTER_VFS_MUTEX; {
        pFs->c.szBlk = pReply->u.error_r.szBlk;
      } LEAVE_VFS_MUTEX;
    }
  }
  sqlite3_free(pReply);

  bcv_close_socket(s);
  sqlite3_free(zAuth);
  return rc;
}
static int proxyDetach(sqlite3_bcvfs *pFs, const char *zAlias, char **pzErr){
  BcvMessage msg;
  BcvMessage *pReply = 0;
  int rc = SQLITE_OK;                       /* Error code */
  BCV_SOCKET_TYPE s = INVALID_SOCKET;       /* Socket connection to daemon */

  s = bcvConnectSocket(&rc, pFs->zPortnumber, pzErr);

  memset(&msg, 0, sizeof(msg));
  msg.eType = BCV_MESSAGE_DETACH;
  msg.u.detach.zName = zAlias;

  pReply = bcvExchangeMessage(&rc, s, &msg);
  if( rc==SQLITE_OK ){
    assert( pReply->eType==BCV_MESSAGE_REPLY );
    rc = pReply->u.error_r.errCode;
    if( pzErr && pReply->u.error_r.zErrMsg ){
      *pzErr = sqlite3_mprintf("%s", pReply->u.error_r.zErrMsg);
    }
  }
  sqlite3_free(pReply);

  bcv_close_socket(s);
  return rc;
}


int sqlite3_bcvfs_attach(
  sqlite3_bcvfs *pFs,
  const char *zStorage,
  const char *zAccount,
  const char *zCont,
  const char *zAlias,
  int flags,
  char **pzErr                    /* OUT: Error message */
){
  char *zErr = 0;                 /* Error message */
  int rc = SQLITE_OK;             /* Return code */

  if( pFs->zPortnumber ){
    rc = proxyAttach(pFs, zStorage, zAccount, zCont, zAlias, flags, &zErr);
  }else{
    rc = bcvfsAttach(pFs, zStorage, zAccount, zCont, zAlias, flags, &zErr);
  }

  if( pzErr ){
    *pzErr = zErr;
  }else{
    sqlite3_free(zErr);
  }
  return rc;
}

static int bcvfsDetach(sqlite3_bcvfs *pFs, const char *zAlias, char **pzErr){
  BcvCommon *p = &pFs->c;
  int rc = SQLITE_OK;

  ENTER_VFS_MUTEX; {
    Container *pCont = bcvfsFindContAlias(p, zAlias, pzErr);
    if( pCont==0 ){
      rc = SQLITE_ERROR;
    }else{
      if( pCont->nClient>0 ){
        rc = SQLITE_BUSY;
        *pzErr = sqlite3_mprintf("container is still in use: %s", zAlias);
      }else{
        int bDirty = 0;

        /* Check for dirty blocks in the cache */
        sqlite3_stmt *pStmt = bcvfsPrepare(&rc, p,
            "SELECT count(*)>0 FROM block WHERE container=?"
        );
        if( rc==SQLITE_OK ){
          sqlite3_bind_text(pStmt, 1, pCont->zName, -1, SQLITE_STATIC);
          if( SQLITE_ROW==sqlite3_step(pStmt) ){
            bDirty = sqlite3_column_int(pStmt, 0);
          }
          bcvfsFinalize(&rc, pStmt);
        }

        /* If no dirty blocks have been found in the cache, check for
        ** local wal files containing one or more valid transactions */
        if( rc==SQLITE_OK && bDirty==0 ){
          Manifest *pMan = pCont->pMan;
          int ii;
          for(ii=0; rc==SQLITE_OK && bDirty==0 && ii<pMan->nDb; ii++){
            char *zWal = bcvfsLocalName(&rc, pCont, pMan->aDb[ii].zDName, 1);
            if( rc==SQLITE_OK ){
              rc = bcvfsWalNonEmpty(&pFs->c, zWal, &bDirty);
              sqlite3_free(zWal);
            }
          }
        }

        if( rc==SQLITE_OK && bDirty ){
          *pzErr = sqlite3_mprintf("container has local changes: %s", zAlias);
          rc = SQLITE_BUSY;
        }

        if( rc==SQLITE_OK ){
          rc = bcvContainerDetachAndFree(p, pCont);
        }
      }
    }
  } LEAVE_VFS_MUTEX;

  return rc;
}

/*
** Detach container zAlias from the VFS.
**
*/
int sqlite3_bcvfs_detach(sqlite3_bcvfs *pFs, const char *zAlias, char **pzErr){
  if( pFs->zPortnumber ){
    return proxyDetach(pFs, zAlias, pzErr);
  }else{
    return bcvfsDetach(pFs, zAlias, pzErr);
  }
}


/*
** Parameter zFile may be the file name of a block stored in the cloud
** storage container.
*/ 
int bcvfsNameToBlockid(Manifest *p, const char *zName, u8 *aBlk){
  int nName = strlen(zName);
  int nExpect = BCV_FSNAMEBYTES(NAMEBYTES(p));
  if( nName==nExpect-1 && 0==memcmp(".bcv", &zName[nExpect-1-4], 4) ){
    if( hex_decode(zName, nName-4, aBlk)==0 ) return 0;
  }
  return 1;
}

/*
**
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwise, check to see if the wal file for the database with
** database-id iDbId in the container attached as zCont exists and is greater
** than zero bytes in size. If no error occurs, this function returns non-zero
** if the file exists and is >0 bytes in size or zero otherwise. Or, if an
** error does occur, set *pRc to an SQLite error code and return non-zero.
*/
static int bcvfsWalFileExists(
  int *pRc,
  BcvCommon *p,
  const char *zLocalDir,          /* Local directory used by container */
  const char *zDName
){
  const char *zWal = 0;
  int res = 0;
  BcvBuffer b = {0, 0, 0};

  bcvBufferAppendString(pRc, &b, zLocalDir);
  bcvBufferAppendString(pRc, &b, BCV_PATH_SEPARATOR);
  bcvBufferAppendEscaped(pRc, &b, zDName);
  bcvBufferAppendString(pRc, &b, "-wal");
  zWal = (const char*)b.aData;
  if( zWal ){
    *pRc = p->pVfs->xAccess(p->pVfs, zWal, SQLITE_ACCESS_EXISTS, &res);
  }
  bcvBufferZero(&b);
  
  return (*pRc || res);
}

static int proxyCmd(
  sqlite3_bcvfs *pFs,             /* VFS handle */
  const char *zCont,              /* Name (alias) of attached container */
  const char *zDb,                /* Database within zCont to pin or unpin */
  int eCmd,                       /* BCV_CMD_XXX value */
  char **pzErr                    /* OUT: error message */
){
  int rc = SQLITE_OK;                       /* Error code */
  BCV_SOCKET_TYPE s = INVALID_SOCKET;       /* Socket connected to daemon */
  BcvMessage *pReply = 0;                   /* HELLO_REPLY message */

  assert( pzErr );
  rc = bcvfsProxyConnect(pFs, zCont, zDb, &s, &pReply);
  if( pReply ){
    rc = pReply->u.hello_r.errCode;
    if( rc && pzErr ){
      *pzErr = sqlite3_mprintf("%s", pReply->u.hello_r.zErrMsg);
    }
  }
  if( rc==SQLITE_OK ){
    BcvMessage msg;
    BcvMessage *pFinal = 0;       /* reply to CMD message */
    char *zAuth = 0;              /* Authorization string */

    zAuth = bcvInvokeAuth(&rc, pFs, 
        pReply->u.hello_r.zStorage,
        pReply->u.hello_r.zAccount,
        pReply->u.hello_r.zContainer
    );

    memset(&msg, 0, sizeof(msg));
    msg.eType = BCV_MESSAGE_CMD;
    msg.u.cmd.zAuth = zAuth;
    msg.u.cmd.eCmd = eCmd;

    pFinal = bcvExchangeMessage(&rc, s, &msg);
    if( rc==SQLITE_OK ){
      assert( pFinal->eType==BCV_MESSAGE_REPLY );
      rc = pFinal->u.error_r.errCode;
      if( pzErr && pFinal->u.error_r.zErrMsg ){
        assert( rc!=SQLITE_OK );
        *pzErr = sqlite3_mprintf("%s", pFinal->u.error_r.zErrMsg);
      }
    }
    sqlite3_free(zAuth);
    sqlite3_free(pFinal);
  }

  sqlite3_free(pReply);
  bcv_close_socket(s);
  return rc;
}

int sqlite3_bcvfs_poll(sqlite3_bcvfs *pFs, const char *zCont, char **pzErr){
  int rc = SQLITE_OK;
  char *zErr = 0;
  if( pFs->zPortnumber ){
    rc = proxyCmd(pFs, zCont, 0, BCV_CMD_POLL, &zErr);
  }else{
    rc = bcvfsPollContainer(pFs, zCont, &zErr);
  }
  if( pzErr ){
    *pzErr = zErr;
  }else{
    sqlite3_free(zErr);
  }
  return rc;
}

typedef struct UploadDb UploadDb;
typedef struct UploadCtx2 UploadCtx2;

struct UploadDb {
  char *zPath;
  i64 iDbId;
  sqlite3 *db;
  ManifestDb *pDb;
};

typedef struct UploadBlockCtx UploadBlockCtx;
struct UploadBlockCtx {
  UploadCtx2 *pCtx;
};

struct UploadCtx2 {
  sqlite3_bcvfs *pFs;
  sqlite3_file *pCacheFile;
  BcvContainer *pBcv;
  BcvDispatch *pDisp;
  i64 iDelTime;
  UploadRecord *pRecord;

  Manifest *pMan;                 /* Copy of the manifest */
  UploadDb *aDb;                  /* Array of databases to upload */
  int nDb;                        /* Number of entries in aDb[] */

  int iDb;                        /* Current entry in aDb */
  int iBlk;                       /* Last block in aDb[iDb].pDb uploaded */
  ManifestHash *pMHash;           /* Hash of blocks except aDb[iDb].pDb */

  int rc;                         /* Error code */
  char *zErrMsg;                  /* Error message */
};

static void bcvfsUploadOneBlock(UploadCtx2 *pCtx);
static void bcvfsUploadBlockDone(void *pArg, int rc, char *zETag){
  UploadBlockCtx *p = (UploadBlockCtx*)pArg;
  UploadCtx2 *pCtx = p->pCtx;
  if( rc==SQLITE_OK ){
    bcvfsUploadOneBlock(pCtx);
  }else{
    if( pCtx->rc==SQLITE_OK ){
      pCtx->rc = rc;
      if( zETag ) pCtx->zErrMsg = sqlite3_mprintf("%s", zETag);
    }
  }
  sqlite3_free(p);
}


static void bcvfsUploadRecordAdd(
  int *pRc, 
  UploadCtx2 *pCtx, 
  const u8 *aOld,
  const u8 *aNew,
  int nName
){
  int nAlloc = sizeof(UploadRecord) + nName*2;
  UploadRecord *pNew = bcvMallocRc(pRc, nAlloc);
  if( pNew ){
    pNew->aOldName = (u8*)&pNew[1];
    pNew->aNewName = &pNew->aOldName[nName];
    memcpy(pNew->aOldName, aOld, nName);
    memcpy(pNew->aNewName, aNew, nName);
    pNew->pNext = pCtx->pRecord;
    pCtx->pRecord = pNew;
  }
}

/*
** If it is not part of any other database, add block iBlk of the database
** being uploaded to the manifest delete list.
*/
static void bcvfsUploadDeleteBlockIf(UploadCtx2 *pCtx, int iBlk){
  int nName = NAMEBYTES(pCtx->pMan);
  ManifestDb *pDb = pCtx->aDb[pCtx->iDb].pDb;
  u8 *aOrig = &pDb->aBlkOrig[nName*iBlk];

  if( bcvMHashQuery(pCtx->pMHash, aOrig, nName)==0 ){
    sqlite3_bcvfs *pFs = pCtx->pFs;
    Manifest *pMan = pCtx->pMan;
    u8 *pDel = &pMan->aDelBlk[pMan->nDelBlk*GCENTRYBYTES(pMan)];
    assert( pCtx->pMan->bDelFree );
    memcpy(pDel, aOrig, nName);
    bcvPutU64(&pDel[nName], pCtx->iDelTime);
    pMan->nDelBlk++;

    if( pFs->mLog & SQLITE_BCV_LOG_CLEANUP ){
      char zBlock[BCV_MAX_FSNAMEBYTES];
      i64 t1 = pCtx->iDelTime - sqlite_timestamp();
      bcvfsBlockidToName(aOrig, nName, zBlock);
      bcvfsCleanupLog(pFs, 
          "adding %s to delete list (deletetime=%d.%.3ds from now)",
          zBlock, t1/1000, t1%1000
      );
    }
  }
}


static void bcvfsUploadOneBlock(UploadCtx2 *pCtx){
  sqlite3_bcvfs *pFs = pCtx->pFs;
  Manifest *pMan = pCtx->pMan;
  int nName = NAMEBYTES(pMan);
  ManifestDb *pDb = 0;
  int rc = pCtx->rc;

  /* Find the next block to upload */
  if( rc==SQLITE_OK ){
    while( pCtx->iDb<pCtx->nDb ){
      pDb = pCtx->aDb[pCtx->iDb].pDb;
      if( pCtx->iBlk<0 ){
        bcvMHashFree(pCtx->pMHash);
        rc = bcvMHashBuild(pMan, 0, pDb, &pCtx->pMHash);
        if( rc!=SQLITE_OK ) break;
      }
      pCtx->iBlk++;
      if( pCtx->iBlk>=pDb->nBlkLocal ){
        pCtx->iBlk = -1;
        pCtx->iDb++;
      }else if( pCtx->iBlk>=pDb->nBlkOrig ){
        break;
      }else{
        int iOff = pCtx->iBlk * nName;
        if( memcmp(&pDb->aBlkOrig[iOff], &pDb->aBlkLocal[iOff], nName) ) break;
      }
      pDb = 0;
    }
  }

  if( pDb && rc==SQLITE_OK ){
    CacheEntry *pEntry;
    u8 *aName = &pDb->aBlkLocal[nName*pCtx->iBlk];
    int nBuf = pMan->szBlk;
    UploadBlockCtx *p = 0;

    ENTER_VFS_MUTEX; {
      pEntry = bcvfsHashFind(&pFs->c, aName, nName);
    } LEAVE_VFS_MUTEX;
    assert( pEntry );

    p = (UploadBlockCtx*)bcvMallocRc(&rc, sizeof(UploadBlockCtx) + nBuf);
    if( p ){
      int bSkip = 0;              /* True if no upload required */
      u8 aNew[BCV_MAX_NAMEBYTES]; /* New name for block */
      u8 *aBuf = (u8*)&p[1];      /* Buffer to load block data into */

      rc = bcvReadfile(pCtx->pCacheFile, aBuf, nBuf, (i64)nBuf*pEntry->iPos);
      memcpy(aNew, aName, nName);
      if( nName>=BCV_MIN_MD5_NAMEBYTES ){
        u8 *aAlt = 0;
        MD5(aBuf, nBuf, aNew);
        aAlt = bcvMHashQuery(pCtx->pMHash, aNew, MD5_DIGEST_LENGTH);
        if( aAlt ){
          memcpy(aNew, aAlt, nName);
          bSkip = 1;
        }
      }
      bcvfsUploadRecordAdd(&rc, pCtx, aName, aNew, nName);
      memcpy(aName, aNew, nName);
      if( rc==SQLITE_OK && bSkip==0 ){
        char zFile[BCV_MAX_FSNAMEBYTES];
        bcvBlockidToText(pCtx->pMan, aNew, zFile);
        p->pCtx = pCtx;
        rc = bcvDispatchPut(pCtx->pDisp, pCtx->pBcv,
            zFile, 0, aBuf, nBuf, (void*)p, bcvfsUploadBlockDone
        );
        if( rc==SQLITE_OK ){
          rc = pCtx->rc;
        }else{
          sqlite3_free(p);
        }
      }else{
        sqlite3_free(p);
      }
      if( rc==SQLITE_OK && pCtx->iBlk<pDb->nBlkOrig ){
        bcvfsUploadDeleteBlockIf(pCtx, pCtx->iBlk);
      }
    }
  }

  pCtx->rc = rc;
}

/*
** This is called at the end of an upload operation to install the new version
** of the manifest locally. By the time this function is called, the new
** manifest has already been uploaded to cloud storage.  The pattern is:
**
**   ENTER_VFS_MUTEX; {
**     pCont->eState = UPLOAD;
**     pDup = bcvfsManifestDup(pCont->pMan);
**   } LEAVE_VFS_MUTEX;
**
**   // Do stuff to manipulate both cloud storage and the copy of
**   // the manifest pDup. Then serialize and upload pDup to cloud
**   // storage.
**
**   ENTER_VFS_MUTEX; {
**     bcvfsManifestMergeAndInstall(pDup);
**     pCont->eState = NONE;
**   } LEAVE_VFS_MUTEX;
**
** Because Container.eState is set, no other update or poll operation can have
** run while cloud storage and pDup are being manipulated. But the aBlkLocal[]
** array for any database may have been modified (if a client ran a checkpoint
** operation). This function merges any such changes into the manifest before
** installing it.
*/
static int bcvfsManifestMergeAndInstall(
  sqlite3_bcvfs *pFs, 
  Container *pCont, 
  UploadCtx2 *pCtx
){
  Manifest *pMan = pCtx->pMan;    /* New manifest object to update + install */
  int rc = SQLITE_OK;             /* Return code */
  int iDb;                        /* Iterator variable */
  
  assert( pCont->pMan && pCtx->pMan );
  assert( pCont->eState==CONTAINER_STATE_UPLOAD );

  for(iDb=0; iDb<pMan->nDb; iDb++){
    ManifestDb *pNew = &pMan->aDb[iDb];
    ManifestDb *pDb = &pCont->pMan->aDb[iDb];

    if( pDb->nBlkLocalAlloc ){
      int iUp;
      for(iUp=0; iUp<pCtx->nDb; iUp++){
        if( pCtx->aDb[iUp].pDb==pNew ) break;
      }
      if( iUp==pCtx->nDb ){
        /* This branch is taken if pNew has dirty blocks but was not 
        ** uploaded as part of the recently completed UPLOAD operation. */
        int nName = NAMEBYTES(pMan);
        if( pNew->nBlkLocalAlloc ){
          sqlite3_free(pNew->aBlkLocal);
        }
        pNew->aBlkLocal = 0;
        pNew->nBlkLocalAlloc = 0;
        pNew->aBlkLocal = bcvMallocRc(&rc, nName*pDb->nBlkLocalAlloc);
        if( rc==SQLITE_OK ){
          memcpy(pNew->aBlkLocal, pDb->aBlkLocal, nName*pDb->nBlkLocal);
          pNew->nBlkLocalAlloc = pDb->nBlkLocalAlloc;
          pNew->nBlkLocal = pDb->nBlkLocal;
        }
      }
    }
  }

  if( rc==SQLITE_OK ){
    rc = bcvManifestInstall(&pFs->c, pCont, pMan);
    pCtx->pMan = 0;
  }

  return rc;
}

static int bcvfsUpload(
  sqlite3_bcvfs *pFs,             /* VFS handle */
  sqlite3 *dbPragma,
  BcvfsFile *pFilePragma,
  const char *zDbPragma,
  const char *zCont,              /* Container (alias) to upload databases of */
  int (*xBusy)(void*,int),        /* Busy-handler callback */
  void *pBusyArg,                 /* First argument passed to xBusy */
  char **pzErr                    /* OUT: Error message */
){
  int rc = SQLITE_OK;
  if( pzErr ) *pzErr = 0;
  if( pFs->zPortnumber==0 ){
    Container *pCont = 0;
    int iDb = 0;
    int ii = 0;
    UploadCtx2 ctx;
    int nDel = 0;                 /* Max blocks that might be deleted */

    memset(&ctx, 0, sizeof(ctx));
    ctx.pFs = pFs;
    ctx.iDelTime = sqlite_timestamp();

    /* Put the container in UPLOAD state. Then duplicate the manifest file
    ** and determine the set of databases that this call will attempt 
    ** to upload. Populate the aUp[] array with the same. */
    ENTER_VFS_MUTEX; {
      /* Find the container */
      pCont = bcvfsFindContAlias(&pFs->c, zCont, pzErr);
      if( pCont==0 ){
        rc = SQLITE_ERROR;
      }else{
        Manifest *pMan = pCont->pMan;
        int nDb;
        bcvfsEnterState(pFs, pCont, CONTAINER_STATE_UPLOAD);

        nDb = pMan->nDb;
        ctx.aDb = bcvMallocRc(&rc, sizeof(UploadDb) * nDb);
        for(iDb=0; rc==SQLITE_OK && iDb<nDb; iDb++){
          ManifestDb *pDb = &pMan->aDb[iDb];
          if( pDb->iDbId>=BCVFS_FIRST_LOCAL_ID 
           || pDb->nBlkLocalAlloc 
           || pDb->nBlkLocal==0             /* deleted db */
           || bcvfsWalFileExists(&rc, &pFs->c, pCont->zLocalDir, pDb->zDName) 
          ){
            /* This database looks dirty. Add it to the ctx.aDb[] array. */
            if( dbPragma==0 || pDb->iDbId==pFilePragma->iFileDbId ){
              UploadDb *pUp = &ctx.aDb[ctx.nDb++];
              if( pDb->nBlkLocal>0 ){
                pUp->zPath = bcvMprintfRc(&rc, "/%s/%s", zCont, pDb->zDName);
              }
              pUp->iDbId = pDb->iDbId;
              nDel += pDb->nBlkLocal;
            }
          }
        }
      }
    } LEAVE_VFS_MUTEX;

    /* For each database in the ctx.aDb[] array:
    **
    **   1) Open a db handle on the db.
    **   2) Set the bHoldCkpt on the underlying BcvfsFile object.
    **   3) Call sqlite3_wal_checkpoint_v2(TRUNCATE).
    **
    ** Only if this succeeds for all databases will the upload take place.
    ** At that point the CHECKPOINTER lock will still be held on all 
    ** databases that will be uploaded, preventing any client from modifying
    ** the database file itself.  */
    for(iDb=0; rc==SQLITE_OK && iDb<ctx.nDb; iDb++){
      UploadDb *pUp = &ctx.aDb[iDb];

      /* If this is a deleted database */
      if( pUp->zPath==0 ) continue;

      if( rc==SQLITE_OK ){
        if( dbPragma ){
          assert( iDb==0 );
          pUp->db = dbPragma;
        }else{
          const int f = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
          rc = sqlite3_open_v2(pUp->zPath, &pUp->db, f, pFs->zName);
          if( rc==SQLITE_OK ){
            sqlite3_busy_handler(pUp->db, xBusy, pBusyArg);
          }
        }
        if( rc==SQLITE_OK ){
          rc = sqlite3_exec(pUp->db, "SELECT * FROM sqlite_schema", 0, 0, 0);
        }
      }

      if( rc==SQLITE_OK ){
        int nLog = 0;
        int nCkpt = 0;
        BcvfsFile *pFile = 0;
        sqlite3_file_control(pUp->db, "main", BCV_FCNTL_FD, (void*)&pFile);
        pFile->bHoldCkpt = 1;
        rc = sqlite3_wal_checkpoint_v2(
            pUp->db, zDbPragma, SQLITE_CHECKPOINT_TRUNCATE, &nLog, &nCkpt
        );
        pFile->bHoldCkpt = 0;
        assert( rc!=SQLITE_OK || (nLog==nCkpt && nLog==0) );
        if( rc!=SQLITE_OK ){
          ctx.zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(pUp->db));
        }
      }
    }

    ENTER_VFS_MUTEX; {
      if( rc==SQLITE_OK ){
        rc = bcvManifestDup(pCont->pMan, &ctx.pMan);
      }
      if( rc==SQLITE_OK ){
        rc = bcvfsContainerGetBcv(pFs, pCont, &ctx.pDisp, &ctx.pBcv, 0);
      }
    } LEAVE_VFS_MUTEX;
    for(iDb=0; rc==SQLITE_OK && iDb<ctx.nDb; iDb++){
      ctx.aDb[iDb].pDb = bcvfsFindDatabaseById(ctx.pMan, ctx.aDb[iDb].iDbId);
    }

    if( rc==SQLITE_OK && ctx.nDb>0 ){

      /* Extend the Manifest.aDelBlk[] array so that it is guaranteed to
      ** be large enough for all blocks that might be added to it by this
      ** UPLOAD operation.  */
      int nGC = GCENTRYBYTES(ctx.pMan);
      int nNew = ctx.pMan->nDelBlk + nDel + 1;
      u8 *aDel = (u8*)bcvMallocRc(&rc, nGC*nNew);
      if( aDel && ctx.pMan->nDelBlk  ){
        memcpy(aDel, ctx.pMan->aDelBlk, ctx.pMan->nDelBlk*nGC);
      }
      if( ctx.pMan->bDelFree ){
        sqlite3_free(ctx.pMan->aDelBlk);
      }
      ctx.pMan->aDelBlk = aDel;
      ctx.pMan->bDelFree = 1;

      if( rc==SQLITE_OK ){
        rc = bcvOpenLocal(pFs->zCacheFile, 0, 0, &ctx.pCacheFile);
      }
      assert( ctx.pCacheFile || rc!=SQLITE_OK );

      /* Upload all new blocks. */
      ctx.rc = rc;
      ctx.iBlk = -1;
      for(ii=0; ii<pFs->nRequest; ii++){
        bcvfsUploadOneBlock(&ctx);
      }
      rc = bcvDispatchRunAll(ctx.pDisp);
      if( rc==SQLITE_OK ) rc = ctx.rc;

      for(iDb=0; rc==SQLITE_OK && iDb<ctx.nDb; iDb++){
        ManifestDb *pDb = ctx.aDb[iDb].pDb;
        if( pDb->iDbId>=BCVFS_FIRST_LOCAL_ID ){
          pDb->iDbId = ++ctx.pMan->iMaxDbId;
        }
        if( pDb->nBlkLocalAlloc ){
          assert( pDb->nBlkOrigAlloc );
          sqlite3_free(pDb->aBlkOrig);
          pDb->aBlkOrig = pDb->aBlkLocal;
          pDb->nBlkOrig = pDb->nBlkLocal;
          pDb->nBlkOrigAlloc = pDb->nBlkLocalAlloc;
          pDb->nBlkLocalAlloc = 0;
          pDb->iVersion++;
        }
        if( pDb->nBlkLocal==0 ){
          ctx.aDb[iDb].iDbId = -1;
        }
      }

      /* Finally, remove any deleted databases from the manifest. */
      for(iDb=0; rc==SQLITE_OK && iDb<ctx.pMan->nDb; iDb++){
        ManifestDb *pDb = &ctx.pMan->aDb[iDb];
        while( pDb->nBlkLocal==0 ){
          assert( pDb->aBlkOrig && pDb->nBlkOrig && pDb->aBlkLocal==0 );
          ctx.pMan->nDb--;
          if( pDb->nBlkOrigAlloc ) sqlite3_free(pDb->aBlkOrig);
          if( iDb<ctx.pMan->nDb ){
            memmove(pDb, &pDb[1], (ctx.pMan->nDb-iDb)*sizeof(ManifestDb));
          }else{
            break;
          }
        }
      }

      /* Upload the new manifest to cloud storage */
      if( rc==SQLITE_OK ){
        rc = bcvfsUploadManifest(ctx.pDisp, ctx.pBcv, ctx.pMan, &ctx.zErrMsg);
      }

      ENTER_VFS_MUTEX; {
        BcvCommon *pCommon = &pFs->c;
        int nName = ctx.pMan ? NAMEBYTES(ctx.pMan) : 0;
        UploadRecord *pRec;
        UploadRecord *pNext;
        if( rc==SQLITE_OK ){
          rc = bcvfsManifestMergeAndInstall(pFs, pCont, &ctx);
        }

        /* If any local databases were uploaded by this operation, their
        ** database-ids will have changed. Update any open file handles
        ** accordingly. */
        for(iDb=0; rc==SQLITE_OK && iDb<ctx.nDb; iDb++){
          i64 iOrig = ctx.aDb[iDb].iDbId;
          i64 iNew = ctx.aDb[iDb].pDb->iDbId;
          BcvfsFile *p;
          assert( iNew==iOrig || iOrig>=BCVFS_FIRST_LOCAL_ID || iOrig<0 );
          for(p=pFs->pFileList; p; p=p->pNextFile){
            if( p->pCont==pCont && p->iFileDbId==iOrig ){
              p->iFileDbId = iNew;
              if( p->pMan ){
                bcvManifestDeref(p->pMan);
                p->pMan = 0;
                p->pManDb = 0;
                bcvfsGetManifestRef(p, 0);
              }
            }
          }
        }

        for(pRec=ctx.pRecord; pRec; pRec=pNext){
          pNext = pRec->pNext;
          if( rc==SQLITE_OK ){
            CacheEntry *pEntry = bcvfsHashFind(pCommon, pRec->aOldName, nName);
            assert( pEntry && pEntry->bDirty && pEntry->bValid );
            pEntry->bDirty = 0;
            bcvfsHashRemove(pCommon, pEntry);
            memcpy(pEntry->aName, pRec->aNewName, nName);
            bcvfsHashAdd(pCommon, pEntry);
            bcvfsLruAddIf(pCommon, pEntry);
            rc = bcvfsWriteBlock(pFs, 0, 0, pEntry);
          }
          sqlite3_free(pRec);
        }
      } LEAVE_VFS_MUTEX;
    }

    /* Clean up: release all CHECKPOINTER locks and close all database 
    ** handles. Then free memory allocated for aDb[] and the last hash 
    ** table.  */
    for(iDb=0; iDb<ctx.nDb; iDb++){
      sqlite3 *db = ctx.aDb[iDb].db;
      if( db ){
        BcvfsFile *pFile = 0;
        sqlite3_file_control(db, "main", BCV_FCNTL_FD, (void*)&pFile);
        bcvfsReleaseCheckpointer(pFile);
        if( dbPragma==0 ) sqlite3_close(db);
      }
      sqlite3_free(ctx.aDb[iDb].zPath);
    }
    bcvMHashFree(ctx.pMHash);
    bcvCloseLocal(ctx.pCacheFile);
    bcvManifestDeref(ctx.pMan);
    sqlite3_free(ctx.aDb);
    ENTER_VFS_MUTEX; {
      bcvfsContainerReleaseBcv(pFs, pCont, ctx.pDisp, ctx.pBcv, rc!=SQLITE_OK);
      bcvfsLeaveState(pFs, pCont);
    } LEAVE_VFS_MUTEX;

    if( pzErr ){
      *pzErr = ctx.zErrMsg;
    }else{
      sqlite3_free(ctx.zErrMsg);
    }
  }

  return rc;
}

static int bcvfsUploadWithRetry(BcvfsFile *pFile, char **pzErr){
  int iIter = 0;
  int rc = SQLITE_OK;
  char *zErr = 0;
  sqlite3 *db = *pFile->ppDb;
  char *zDb = 0;

  /* Cannot do "PRAGMA bcv_upload" if there is an open read or write
  ** transaction on the file.  Return early if this is the case. */
  if( pFile->lockMask || 0==sqlite3_get_autocommit(db) ){
    return SQLITE_BUSY;
  }
  
  zDb = bcvfsFindDbName(&rc, pFile);
  if( rc==SQLITE_OK ){
    do{
      sqlite3_free(zErr);
      zErr = 0;
      rc = bcvfsUpload(pFile->pFs, 
          db, pFile, zDb, pFile->pCont->zName, 0, 0, &zErr
      );
    }while( rc==HTTP_AUTH_ERROR && (++iIter)<BCVFS_MAX_AUTH_RETRIES );
  }
  sqlite3_free(zDb);
  *pzErr = zErr;
  return rc;
}

int sqlite3_bcvfs_upload(
  sqlite3_bcvfs *pFs,             /* VFS handle */
  const char *zCont,              /* Container (alias) to upload databases of */
  int (*xBusy)(void*,int),        /* Busy-handler callback */
  void *pBusyArg,                 /* First argument passed to xBusy */
  char **pzErr                    /* OUT: Error message */
){
  return bcvfsUpload(pFs, 0, 0, "main", zCont, xBusy, pBusyArg, pzErr);
}

/*
** Make a copy of a database within the local manifest.
*/
int sqlite3_bcvfs_copy(
  sqlite3_bcvfs *pFs,
  const char *zCont,
  const char *zFrom,
  const char *zTo,
  char **pzErr
){
  int rc = SQLITE_OK;
  char *zErr = 0;

  if( pFs->zPortnumber ){ 
    rc = SQLITE_READONLY;
    zErr = sqlite3_mprintf("vfs is readonly");
  }else{
    ENTER_VFS_MUTEX; {
      Container *pCont = 0;
      pCont = bcvfsFindContAlias(&pFs->c, zCont, &zErr);
      if( pCont ){
        Manifest *pMan = 0;       /* Manifest of container pCont */
        ManifestDb *pFrom = 0;    /* Copy from this db */
        ManifestDb *pTo = 0;      /* Copy to this db */
        u8 *aBlkOrig = 0;         /* Deep copy of pFrom->aBlkOrig */
        int nCopy;
  
        bcvfsEnterState(pFs, pCont, CONTAINER_STATE_COPY);
  
        pMan = pCont->pMan;
        pFrom = bcvfsFindDatabase(pMan, zFrom, -1);
        pTo = bcvfsFindDatabase(pMan, zTo, -1);
  
        if( pFrom==0 || pFrom->nBlkLocal==0 ){
          rc = SQLITE_ERROR;
          zErr = sqlite3_mprintf("no such database: %s", zFrom);
        }
        else if( pFrom->iDbId>=BCVFS_FIRST_LOCAL_ID ){
          rc = SQLITE_ERROR;
          zErr = sqlite3_mprintf("cannot copy database %s", zFrom);
        }
        else if( pTo!=0 ){
          rc = SQLITE_ERROR;
          zErr = sqlite3_mprintf("database %s already exists", zTo);
        }
  
        if( rc==SQLITE_OK ){
          rc = bcvManifestDup(pMan, &pMan);
        }
        if( rc==SQLITE_OK ){
          pFrom = bcvfsFindDatabase(pMan, zFrom, -1);
          nCopy = pFrom->nBlkOrig * NAMEBYTES(pMan);
          aBlkOrig = (u8*)bcvMallocRc(&rc, nCopy);
        }
        if( rc==SQLITE_OK ){
          memcpy(aBlkOrig, pFrom->aBlkOrig, nCopy);
          pTo = &pMan->aDb[pMan->nDb++];
          pTo->iDbId = pFs->iNextLocalId++;
          pTo->iParentId = pFrom->iDbId;
          memcpy(pTo->zDName, zTo, bcvStrlen(zTo));
          pTo->iVersion = 1;
          pTo->aBlkLocal = pTo->aBlkOrig = aBlkOrig;
          pTo->nBlkLocal = pTo->nBlkOrig = pFrom->nBlkOrig;
          pTo->nBlkOrigAlloc = pFrom->nBlkOrig;
        }

        if( rc==SQLITE_OK ){
          bcvManifestInstall(&pFs->c, pCont, pMan);
        }
        bcvfsLeaveState(pFs, pCont);
      }else{
        rc = SQLITE_ERROR;
      }
    } LEAVE_VFS_MUTEX;
  }

  if( pzErr ){
    *pzErr = zErr;
  }else{
    sqlite3_free(zErr);
  }
  return rc;
}


/*
** This function does the bulk of the work for sqlite3_bcvfs_delete(). Before
** it is called it has already been verified that:
**
**   * pFs is not a proxy VFS, and
**   * pzErr is non-NULL.
*/
static int bcvfsDeleteDatabase(
  sqlite3_bcvfs *pFs,
  const char *zCont,
  const char *zDb,
  char **pzErr
){
  int rc = SQLITE_OK;
  assert( pzErr );
  assert( pFs->zPortnumber==0 );
  ENTER_VFS_MUTEX; {
    Container *pCont = 0;
    pCont = bcvfsFindContAlias(&pFs->c, zCont, pzErr);
    if( pCont==0 ){
      rc = SQLITE_ERROR;
    }else{
      Manifest *pMan = 0;
      bcvfsEnterState(pFs, pCont, CONTAINER_STATE_DELETE);
      rc = bcvManifestDup(pCont->pMan, &pMan);
      if( rc==SQLITE_OK ){
        ManifestDb *pDb = bcvfsFindDatabase(pMan, zDb, -1);
        if( pDb==0 || pDb->nBlkLocal==0 ){
          *pzErr = sqlite3_mprintf("no such database: %s", zDb);
          rc = SQLITE_ERROR;
        }else{
          int nName = NAMEBYTES(pMan);
          sqlite3_file *pFd = 0;
          char *zFile = bcvfsLocalName(&rc, pCont, zDb, 0);
          char *zWal = bcvfsLocalName(&rc, pCont, zDb, 1);

          /* Open and take an EXCLUSIVE lock on the database file. */
          if( rc==SQLITE_OK ){
            rc = bcvOpenLocal(zFile, 0, 0, &pFd);
          }
          if( rc==SQLITE_OK ){
            rc = pFd->pMethods->xLock(pFd, SQLITE_LOCK_SHARED);
          }
          if( rc==SQLITE_OK ){
            rc = pFd->pMethods->xLock(pFd, SQLITE_LOCK_EXCLUSIVE);
          }
          bcvCloseLocal(pFd);
          pFd = 0;

          /* Delete the wal file, if one is present */
          pFs->c.pVfs->xDelete(pFs->c.pVfs, zWal, 0);

          /* Undirty any dirty blocks */
          bcvfsUndirtyBlocks(pFs, nName, pDb, 0);

          if( rc==SQLITE_OK ){
            if( pDb->nBlkLocalAlloc ){
              sqlite3_free(pDb->aBlkLocal);
              pDb->nBlkLocalAlloc = 0;
            }
            pDb->aBlkLocal = 0;
            pDb->nBlkLocal = 0;
            rc = bcvManifestInstall(&pFs->c, pCont, pMan);
            pMan = 0;
          }else{
            *pzErr = sqlite3_mprintf("%s", sqlite3_errstr(rc));
          }
          sqlite3_free(zFile);
          sqlite3_free(zWal);
        }
      }
      bcvManifestFree(pMan);
      bcvfsLeaveState(pFs, pCont);
    }
  } LEAVE_VFS_MUTEX;

  return rc;
}


/*
** Delete a database from the local manifest.
*/
int sqlite3_bcvfs_delete(
  sqlite3_bcvfs *pFs,
  const char *zCont,
  const char *zDb,
  char **pzErr
){
  int rc = SQLITE_OK;
  char *zErr = 0;
  
  if( pFs->zPortnumber ){ 
    rc = SQLITE_READONLY;
  }else{
    rc = bcvfsDeleteDatabase(pFs, zCont, zDb, &zErr);
  }

  if( pzErr ){
    *pzErr = zErr;
  }else{
    sqlite3_free(zErr);
  }
  return rc;
}

/*
** Assuming the main database attached to database handle db is a bcvfs
** database, this function attempts to obtain and return a bcv handle
** and dispatcher for the container associated with the main database.
** If successful, SQLITE_OK is returned and output variables *ppDisp
** and *ppBcv set to the dispatcher and bcv handle respectively. Or, 
** if an error occurs, an SQLite error code is returned and the two
** output variables zeroed.
**
** If the main database attached to db is not a bcvfs database, then
** SQLITE_OK is returned and the output variables zeroed.
*/
int bcvfsGetBcv(sqlite3 *db, BcvDispatch **ppDisp, BcvContainer **ppBcv){
  int rc = SQLITE_OK;
  BcvfsFile *pFile = 0;
  *ppDisp = 0;
  *ppBcv = 0;
  sqlite3_file_control(db, "main", BCV_FCNTL_FD, (void*)&pFile);
  if( pFile ){
    sqlite3_bcvfs *pFs = pFile->pFs;
    ENTER_VFS_MUTEX; {
      rc = bcvfsContainerGetBcv(pFs, pFile->pCont, ppDisp, ppBcv, 0);
    } LEAVE_VFS_MUTEX;
  }
  return rc;
}

/*
** Release a bcv handle obtained via an earlier call to bcvfsGetBcv().
*/
void bcvfsReleaseBcv(
  sqlite3 *db, 
  BcvDispatch *pDisp, 
  BcvContainer *pBcv, 
  int bError
){
  BcvfsFile *pFile = 0;
  sqlite3_file_control(db, "main", BCV_FCNTL_FD, (void*)&pFile);
  if( pFile ){
    sqlite3_bcvfs *pFs = pFile->pFs;
    ENTER_VFS_MUTEX; {
      bcvfsContainerReleaseBcv(pFs, pFile->pCont, pDisp, pBcv, bError);
    } LEAVE_VFS_MUTEX;
  }
}

struct sqlite3_prefetch {
  sqlite3_bcvfs *pFs;

  Container *pCont;
  Manifest *pMan;
  ManifestDb *pManDb;
  BcvContainer *pBcv;
  BcvDispatch *pDisp;
  sqlite3_file *pCacheFile;

  int iNext;                      /* Next block of file to request */
  int nOutstanding;               /* Current number of outstanding requests */
  int nOnDemand;                  /* Count of on-demand requests (proxy only) */
  int nAuthFailure;

  BcvMessage *pReply;             /* Reply to HELLO message (proxy only) */
  BCV_SOCKET_TYPE fdProxy;        /* Connection to proxy process */
  char *zAuth;                    /* Authorization string (proxy only) */

  int rc;
  char *zErr;
};

typedef struct PrefetchCtx PrefetchCtx;
struct PrefetchCtx {
  sqlite3_prefetch *p;
  CacheEntry *pEntry;
};

/*
** Allocate a new pre-fetch object. 
*/
int sqlite3_bcvfs_prefetch_new(
  sqlite3_bcvfs *pFs,
  const char *zCont,
  const char *zDb,
  sqlite3_prefetch **ppOut
){
  sqlite3_prefetch *pNew = 0;
  int rc = SQLITE_OK;

  pNew = bcvMallocRc(&rc, sizeof(sqlite3_prefetch));
  if( pNew ){
    pNew->fdProxy = INVALID_SOCKET;
    pNew->pFs = pFs;
    ENTER_VFS_MUTEX; {
      pFs->nRef++;
    }; LEAVE_VFS_MUTEX;

    if( pFs->zPortnumber ){
      BcvMessage *pReply = 0;     /* HELLO_REPLY message */
      pNew->rc = bcvfsProxyConnect(
          pFs, zCont, zDb, &pNew->fdProxy, &pReply
      );
      if( pReply ){
        assert( pNew->rc==SQLITE_OK );
        pNew->rc = pReply->u.hello_r.errCode;
        if( pNew->rc ){
          pNew->zErr = sqlite3_mprintf("%s", pReply->u.hello_r.zErrMsg);
        }
      }
      if( pNew->rc ){
        sqlite3_free(pReply);
      }else{
        pNew->pReply = pReply;
      }
    }else{
      ENTER_VFS_MUTEX; {
        Container *pCont = bcvfsFindContAlias(&pFs->c, zCont, &pNew->zErr);
        if( pCont ){
          pNew->rc = bcvfsContainerGetBcv(
              pFs, pCont, &pNew->pDisp, &pNew->pBcv, &pNew->zErr
          );
        }else{
          pNew->rc = SQLITE_ERROR;
        }
        if( pNew->rc==SQLITE_OK ){
          pNew->pCont = pCont;
          pCont->nClient++;
          pNew->pMan = bcvManifestRef(pCont->pMan);
          pNew->pManDb = bcvfsFindDatabase(pNew->pMan, zDb, strlen(zDb));
          if( pNew->pManDb==0 ){
            pNew->zErr = sqlite3_mprintf("no such database: %s", zDb);
            pNew->rc = SQLITE_ERROR;
          }else{
            pNew->rc = bcvOpenLocal(pFs->zCacheFile, 0, 0, &pNew->pCacheFile);
          }
        }
      } LEAVE_VFS_MUTEX;
    }

    rc = pNew->rc;
  }

  *ppOut = pNew;
  return rc;
}

static void bcvfsPrefetchCb(
  void *pArg, 
  int rc, 
  char *zETag, 
  const u8 *aData, 
  int nData
){
  PrefetchCtx *pCtx = (PrefetchCtx*)pArg;
  sqlite3_prefetch *p = pCtx->p;
  CacheEntry *pEntry = pCtx->pEntry;
  sqlite3_bcvfs *pFs = p->pFs;

  p->nOutstanding--;

  if( p->rc==SQLITE_OK && rc!=SQLITE_OK ){
    p->rc = rc;
    p->zErr = sqlite3_mprintf("%s", zETag);
  }

  if( p->rc==SQLITE_OK ){
    i64 iOff = (pEntry->iPos * pFs->c.szBlk);
    p->rc = bcvWritefile(p->pCacheFile, aData, nData, iOff);
  }

  if( p->rc==SQLITE_OK ){
    assert( pEntry->bDirty==0 );
    p->rc = bcvfsWriteBlock(pFs, 0, 0, pEntry);
  }

  ENTER_VFS_MUTEX; {
    if( p->rc==SQLITE_OK ){
      pEntry->bValid = 1;
    }else{
      assert( pEntry->nRef==1 );
      bcvfsHashRemove(&pFs->c, pEntry);
      bcvfsUnusedAdd(&pFs->c, pEntry);
    }
    bcvfsMutexCondSignal(&pFs->mutex);
  } LEAVE_VFS_MUTEX;

  sqlite3_free(pCtx);
}

void bcvfsPrefetchRun(
  sqlite3_prefetch *p,            /* Prefetch handle */
  int nRequest,                   /* Maximum number of outstanding requests */
  int nMs                         /* Timeout in ms */
){
  sqlite3_bcvfs *pFs = p->pFs;
  const int nNameBytes = NAMEBYTES(p->pMan);
  int rc = SQLITE_OK;
  ManifestDb *pManDb = p->pManDb;

  while( rc==SQLITE_OK && p->nOutstanding<nRequest ){
    int iBlk = p->iNext++;
    if( iBlk>=pManDb->nBlkLocal ){
      break;
    }else{
      CacheEntry *pEntry = 0;
      u8 *pBlk = &pManDb->aBlkLocal[nNameBytes * iBlk];

      ENTER_VFS_MUTEX; {
        if( 0==bcvfsHashFind(&pFs->c, pBlk, nNameBytes) ){
          rc = bcvfsFindBlockForIO(pFs, pManDb, nNameBytes, iBlk, &pEntry);
        }
      } LEAVE_VFS_MUTEX;

      if( rc==SQLITE_OK && pEntry ){
        const char *zDb = pManDb->zDName;
        PrefetchCtx *pCtx = 0;
        char zName[BCV_MAX_FSNAMEBYTES];

        rc = bcvDispatchLogmsg(p->pDisp, "prefetch %d of %s", iBlk, zDb);
        bcvfsBlockidToText(pEntry->aName, pEntry->nName, zName);
        pCtx = bcvMallocRc(&rc, sizeof(PrefetchCtx));
        if( rc==SQLITE_OK ){
          pCtx->pEntry = pEntry;
          pCtx->p = p;
          rc = bcvDispatchFetch(
              p->pDisp, p->pBcv, zName, 0, 0, (void*)pCtx, bcvfsPrefetchCb
          );
          p->nOutstanding++;
        }
      }
    }
  }

  if( p->nOutstanding>0 ){
    int nOutstanding = p->nOutstanding;
    i64 tmNow = sqlite_timestamp();
    i64 tmEnd = tmNow + nMs;
    do {
      rc = bcvDispatchRun(p->pDisp, 0, 0, tmEnd-tmNow);
    }while( rc==SQLITE_OK 
         && p->nOutstanding==nOutstanding 
         && (tmNow = sqlite_timestamp())<tmEnd
    );

    if( SQLITE_OK==rc
     && p->rc==HTTP_AUTH_ERROR 
     && p->nAuthFailure<BCVFS_MAX_AUTH_RETRIES 
     && SQLITE_OK==(rc = bcvDispatchRunAll(p->pDisp))
    ){
      sqlite3_free(p->zErr);
      p->zErr = 0;
      p->nAuthFailure++;
      p->iNext = 0;
      ENTER_VFS_MUTEX; {
        bcvfsContainerReleaseBcv(pFs, p->pCont, p->pDisp, p->pBcv, 1);
        p->rc = bcvfsContainerGetBcv(
            pFs, p->pCont, &p->pDisp, &p->pBcv, &p->zErr
        );
      } LEAVE_VFS_MUTEX;
    }
  }

  if( p->rc==SQLITE_OK ) p->rc = rc;
  if( p->rc==SQLITE_OK && p->iNext>=pManDb->nBlkLocal && p->nOutstanding==0 ){
    p->rc = SQLITE_DONE;
  }
}

void bcvfsProxyPrefetchRun(
  sqlite3_prefetch *p,            /* Prefetch handle */
  int nRequest,                   /* Maximum number of outstanding requests */
  int nMs                         /* Timeout in ms */
){
  BcvMessage prefetch;
  BcvMessage *pReply = 0;
  int iIter = 0;
  int bContinue;

  do{
    bContinue = 0;
    if( p->zAuth==0 ){
      p->zAuth = bcvInvokeAuth(&p->rc, p->pFs, 
          p->pReply->u.hello_r.zStorage,
          p->pReply->u.hello_r.zAccount,
          p->pReply->u.hello_r.zContainer
      );
    }

    memset(&prefetch, 0, sizeof(prefetch));
    prefetch.eType = BCV_MESSAGE_PREFETCH;
    prefetch.u.prefetch.zAuth = p->zAuth;
    prefetch.u.prefetch.nRequest = (u32)nRequest;
    prefetch.u.prefetch.nMs = (u32)nMs;

    pReply = bcvExchangeMessage(&p->rc, p->fdProxy, &prefetch);

    if( pReply ){
      assert( pReply->eType==BCV_MESSAGE_PREFETCH_REPLY );
      if( pReply->u.prefetch_r.errCode==HTTP_AUTH_ERROR 
       && (++iIter)<BCVFS_MAX_AUTH_RETRIES 
      ){
        sqlite3_free(p->zAuth);
        p->zAuth = 0;
        bContinue = 1;
      }else{
        p->rc = pReply->u.prefetch_r.errCode;
        if( p->rc ){
          p->zErr = sqlite3_mprintf("%s", pReply->u.prefetch_r.zErrMsg);
        }
        p->nOutstanding = pReply->u.prefetch_r.nOutstanding;
        p->nOnDemand = pReply->u.prefetch_r.nOnDemand;
      }
      sqlite3_free(pReply);
    }
  }while( bContinue );
}

/*
** Do some pre-fetching.
*/
int sqlite3_bcvfs_prefetch_run(
  sqlite3_prefetch *p,            /* Prefetch handle */
  int nRequest,                   /* Maximum number of outstanding requests */
  int nMs                         /* Timeout in ms */
){
  if( p && p->rc==SQLITE_OK ){
    if( p->pFs->zPortnumber==0 ){
      bcvfsPrefetchRun(p, nRequest, nMs);
    }else{
      bcvfsProxyPrefetchRun(p, nRequest, nMs);
    }
  }

  return sqlite3_bcvfs_prefetch_errcode(p);
}

const char *sqlite3_bcvfs_prefetch_errmsg(sqlite3_prefetch *p){
  return p ? p->zErr : "out of memory";
}

int sqlite3_bcvfs_prefetch_errcode(sqlite3_prefetch *p){
  return p ? p->rc : SQLITE_NOMEM;
}

int sqlite3_bcvfs_prefetch_status(
  sqlite3_prefetch *p, 
  int op,
  sqlite3_int64 *piVal
){
  int rc = SQLITE_OK;
  switch( op ){
    case SQLITE_BCVFS_PFS_NOUTSTANDING:
      *piVal = (i64)p->nOutstanding;
      break;

    case SQLITE_BCVFS_PFS_NDEMAND: {
      sqlite3_bcvfs *pFs = p->pFs;
      if( pFs->zPortnumber ){
        *piVal = (i64)p->nOnDemand;
      }else{
        ENTER_VFS_MUTEX; {
          *piVal = (i64)pFs->nOnDemand;
        } LEAVE_VFS_MUTEX;
      }
      break;
    }

    default:
      rc = SQLITE_MISUSE;
      break;
  }

  return rc;
}

void sqlite3_bcvfs_prefetch_destroy(sqlite3_prefetch *p){
  if( p ){
    sqlite3_bcvfs *pFs = p->pFs;
    ENTER_VFS_MUTEX; {
      pFs->nRef--;
    }; LEAVE_VFS_MUTEX;
    bcvManifestDeref(p->pMan);
    if( p->pCont ){
      int bError = p->rc || (p->nOutstanding>0);
      p->pCont->nClient--;
      bcvfsContainerReleaseBcv(p->pFs, p->pCont, p->pDisp, p->pBcv, bError);
    }
    bcvCloseLocal(p->pCacheFile);

    sqlite3_free(p->zErr);
    bcv_close_socket(p->fdProxy);
    sqlite3_free(p->pReply);
    sqlite3_free(p->zAuth);
    sqlite3_free(p);
  }
}

/*
** Revert all local changes to the named container.
**
**   1. Discard any dirty blocks from the cache,
**   2. Truncate any *-wal files in the file-system.
**   3. Install the old manifest (read from the blocksdb file).
*/ 
int sqlite3_bcvfs_revert(sqlite3_bcvfs *pFs, const char *zCont, char **pzErr){
  const char *zSql = "SELECT manifest, etag FROM container WHERE container=?";
  int rc = SQLITE_OK;
  char *zErr = 0;

  if( pFs->zPortnumber ){
    if( pzErr ) *pzErr = 0;
    return SQLITE_OK;
  }

  ENTER_VFS_MUTEX; {
    Container *pCont = bcvfsFindContAlias(&pFs->c, zCont, &zErr);

    if( pCont==0 ){
      rc = SQLITE_ERROR;
    }else{
      FileWrapper *aWrap = 0;
      sqlite3_stmt *pSel = 0;
      int ii;
      Manifest *pMan = pCont->pMan;
      Manifest *pNew = 0;

      /* Load the old manifest from disk */
      aWrap = bcvMallocRc(&rc, sizeof(FileWrapper)*(pMan->nDb+1));
      pSel = bcvfsPrepare(&rc, &pFs->c, zSql);
      if( rc==SQLITE_OK ){
        sqlite3_bind_text(pSel, 1, zCont, -1, SQLITE_STATIC);
        if( sqlite3_step(pSel)==SQLITE_ROW ){
          const u8 *aMan = sqlite3_column_blob(pSel, 0);
          int nMan = sqlite3_column_bytes(pSel, 0);
          const char *zETag = (const char*)sqlite3_column_text(pSel, 1);
          rc = bcvManifestParseCopy(aMan, nMan, zETag, &pNew, &zErr);
        }
        sqlite3_finalize(pSel);
        if( rc==SQLITE_OK && pNew==0 ){
          rc = SQLITE_CORRUPT;
          zErr = sqlite3_mprintf("failed to load old manifest from disk");
        }
      }

      if( rc==SQLITE_OK ){
        assert( pNew );
        int iFirstCopy = -1;
        for(ii=0; ii<pNew->nDb; ii++){
          ManifestDb *pDb = &pNew->aDb[ii];
          if( pDb->iDbId>=BCVFS_FIRST_LOCAL_ID ){
            if( pDb->nBlkOrigAlloc ){
              sqlite3_free(pDb->aBlkOrig);
              if( iFirstCopy<0 ) iFirstCopy = ii;
            }
          }else
          if( pDb->nBlkLocal==0 ){
            pDb->nBlkLocal = pDb->nBlkOrig;
            pDb->aBlkLocal = pDb->aBlkOrig;
          }
        }
        if( iFirstCopy>=0 ){
          pNew->nDb = iFirstCopy;
        }
      }

      /* For each database file that has already been created on disk, take
      ** an exclusive lock on it (so that no clients may access it while
      ** we are reverting changes) and zero the few bytes of the *-shm file.
      */
      for(ii=0; rc==SQLITE_OK && ii<pMan->nDb; ii++){
        ManifestDb *pDb = &pMan->aDb[ii];
        char *zFile = bcvfsLocalName(&rc, pCont, pDb->zDName, 0);
        char *zWal = bcvfsLocalName(&rc, pCont, pDb->zDName, 1);
        u8 *aMap = 0;
        sqlite3_file *pFd = 0;
        const int lockflags = SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE;

        if( rc==SQLITE_OK ){
          int res = 0;
          rc = pFs->c.pVfs->xAccess(
              pFs->c.pVfs, zFile, SQLITE_ACCESS_EXISTS, &res
          );
          if( res ){
            if( rc==SQLITE_OK ){
              rc = bcvOpenLocal(zWal, 1, 0, &pFd);
              aWrap[ii].pWalFd = pFd;
            }

            if( rc==SQLITE_OK ){
              const sqlite3_io_methods *pMeth = 0;
              rc = bcvOpenLocal(zFile, 0, 0, &pFd);
              aWrap[ii].pFd = pFd;
              if( rc==SQLITE_OK ){
                pMeth = pFd->pMethods;
                rc = pMeth->xShmMap(pFd, 0, 32*1024, 1, (volatile void**)&aMap);
              }
              if( rc==SQLITE_OK ){
                aWrap[ii].bUnmap = 1;
                if( pMeth->xShmLock(pFd, 0, 5, lockflags) ){
                  rc = SQLITE_BUSY;
                  zErr = sqlite3_mprintf("database is locked: %s", pDb->zDName);
                }else{
                  aWrap[ii].bUnlock = 1;
                  memset(aMap, 0, 24);
                }
              }
            }
          }
        }
        sqlite3_free(zFile);
        sqlite3_free(zWal);
      }

      /* If no error has occurred so far, discard all dirty blocks associated
      ** with the container and truncate all wal files to zero bytes in 
      ** size. */
      for(ii=0; rc==SQLITE_OK && ii<pMan->nDb; ii++){
        sqlite3_file *pFd = aWrap[ii].pWalFd;
        ManifestDb *pDb = &pMan->aDb[ii];
        bcvfsUndirtyBlocks(pFs, NAMEBYTES(pMan), pDb, 0);
        if( pFd ){
          rc = pFd->pMethods->xTruncate(pFd, 0);
        }
      }

      if( aWrap ){
        for(ii=0; ii<pMan->nDb; ii++){
          sqlite3_file *pFd = aWrap[ii].pFd;
          if( pFd ){
            if( aWrap[ii].bUnmap ){
              if( aWrap[ii].bUnlock ){
                const int lockflags = SQLITE_SHM_UNLOCK|SQLITE_SHM_EXCLUSIVE;
                pFd->pMethods->xShmLock(pFd, 0, 5, lockflags);
              }
              pFd->pMethods->xShmUnmap(pFd, 0);
            }
          }
          bcvCloseLocal(pFd);
          bcvCloseLocal(aWrap[ii].pWalFd);
        }
      }
      sqlite3_free(aWrap);

      if( rc==SQLITE_OK ){
        assert( pNew->nRef==1 );
        bcvManifestDeref(pMan);
        pCont->pMan = pNew;
      }else{
        bcvManifestDeref(pNew);
      }

    }

  }; LEAVE_VFS_MUTEX;

  if( pzErr ){
    *pzErr = zErr;
  }else{
    sqlite3_free(zErr);
  }
  return rc;
}

/*************************************************************************/
/* BEGIN VIRTUAL TABLE CODE */
/*************************************************************************/

#define BCV_DATABASE_VTAB_SCHEMA  \
"CREATE TABLE bcv_database("      \
"  container TEXT,"               \
"  database  TEXT,"               \
"  nblock INTEGER,"               \
"  ncache INTEGER,"               \
"  ndirty INTEGER,"               \
"  walfile BOOLEAN,"              \
"  state TEXT "                   \
")"

#define BCV_CONTAINER_VTAB_SCHEMA \
"CREATE TABLE bcv_container("     \
"  name      TEXT,"               \
"  storage   TEXT,"               \
"  user      TEXT,"               \
"  container TEXT,"               \
"  ncleanup  INTEGER"             \
")"

#define BCV_BLOCK_VTAB_SCHEMA     \
"CREATE TABLE bcv_block("         \
"  container TEXT,"               \
"  database  TEXT,"               \
"  blockno   INTEGER,"            \
"  blockid   BLOB,"               \
"  cache     BOOLEAN,"            \
"  dirty     BOOLEAN"             \
")"

#define BCV_FILE_VTAB_SCHEMA      \
"CREATE TABLE bcv_kv("            \
"  name      TEXT,"               \
"  value     TEXT "               \
")"

/*
** Bits used in the idxNum value
*/
#define BCV_COLUSED_MASK   0x0000FFFF
#define BCV_CONTAINER_MASK 0x00010000
#define BCV_DATABASE_MASK  0x00020000

#define BCV_DATABASE_NCND     (3 << 3)
#define BCV_DATABASE_WALFILE  (1 << 5)

/*
** Used as the vtab object for both bcv_database, bcv_container and
** bcv_block eponymous tables.
*/
typedef struct bcv_database_vtab bcv_database_vtab;
struct bcv_database_vtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3 *db;                    /* Database handle */
  char *zMod;
};

/*
** Cursor types for the bcv_database, bcv_container and bcv_block tables,
** respectively.
*/
typedef struct bcv_database_cursor bcv_database_cursor;
typedef struct bcv_container_cursor bcv_container_cursor;
typedef struct bcv_block_cursor bcv_block_cursor;

typedef struct VtabBlob VtabBlob;
struct VtabBlob {
  const u8 *aData;
  int nData;
  int iData;
};

/*
** pData:
**   If the underlying VFS is a proxy VFS, then the data to return
**   is contained in the pData message object obtained from the daemon
**   process within the xFilter call. The message object contains a
**   blob of data which itself consists of a series of message-format
**   encoded nul-terminated strings and u32s (see functions
**   bcvVtabExtractString and bcvVtabExtractU32). One set of the following 
**   per row visited by the scan:
**
**     1) STRING value of "container" column,
**     2) STRING value of "database" column,
**     3) U32 value of "nblock" column,
**     4) U32 value of "ncache" column,
**     5) U32 value of "pinned" column.
**
**   Virtual table columns "ndirty" and "walfile" are always set to 0
**   for a proxy VFS cursor.
**
** iData:
**   Offset of next row in pData blob.
*/
struct bcv_database_cursor {
  sqlite3_vtab_cursor base;       /* Base class */

  VtabBlob data;
  void *pFreeData;

  /* Values for current row */
  i64 iRow;                       /* Rowid */
  char *zContainer;
  char *zDatabase;
  int nBlock;
  int nCache;
  int nDirty;
  int bWalfile;
  int nPin;
  char *zState;
};

/*
** pData:
**   If the underlying VFS is a proxy VFS, then the data to return
**   is contained in the pData message object obtained from the daemon
**   process within the xFilter call. The message object contains a
**   blob of data which itself consists of a series of message-format
**   encoded nul-terminated strings (see function todo). Four strings
**   for each row to return to the user, as follows:
**
**     1) Value of "name" column,
**     2) Value of "storage" column,
**     3) Value of "user" column,
**     4) Value of "container" column.
**     5) Value of "ncleanup" column.
**
** iData:
**   Offset of next row in pData blob.
*/
struct bcv_container_cursor {
  sqlite3_vtab_cursor base;       /* Base class */

  /* Used by proxy VFS cursors */
  VtabBlob data;
  void *pFreeData;

  /* Values for current row */
  i64 iRow;                       /* Rowid */
  char *zName;                    /* Value for column "name" */
  char *zStorage;                 /* Value for column "storage" */
  char *zUser;                    /* Value for column "user" */
  char *zContainer;               /* Value for column "container" */
  int nCleanup;                   /* Value for column "ncleanup" */
};


/*
** Buffer VtabBlob contains all records that will be returned by the
** current scan, tightly packed. Each record consists of:
**
**   1) STRING:  Value of "container" column
**   2) STRING:  Value of "database" column
**   3) INTEGER: Value of "blockno" column
**   4) BLOB:    Value of "blockid" column. 0 length blob -> SQL NULL.
**   5) INTEGER: Value of "dirty" column.
**   6) INTEGER: Value of "cache" column.
*/
struct bcv_block_cursor {
  sqlite3_vtab_cursor base;       /* Base class */

  VtabBlob data;
  void *pFreeData;

  /* Values for current row */
  i64 iRow;                       /* Rowid value */
  char *zContainer;               /* Container name (local alias) */
  char *zDatabase;                /* Database name */
  int iBlockNo;                   /* Value for "blockno" column */
  u8 *aBlockId;                   /* Value for "blockid" column */
  int nBlockId;                   /* Size of aBlockId in bytes */
  int bDirty;                     /* Value for "dirty" column */
  int bCache;                     /* Value for "cache" column */
};


static int bcvDatabaseVtabConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  bcv_database_vtab *pNew = 0;
  int rc = SQLITE_OK;

  rc = sqlite3_declare_vtab(db, (const char*)pAux);
  pNew = (bcv_database_vtab*)bcvMallocRc(&rc, sizeof(bcv_database_vtab));
  if( pNew ){
    pNew->db = db;
    pNew->zMod = bcvStrdupRc(&rc, argv[0]);
    if( rc!=SQLITE_OK ){
      sqlite3_free(pNew);
      pNew = 0;
    }
  }
  *ppVtab = &pNew->base;
  return rc;
}

/*
** Implementation of xOpen() method for the three read-only virtual 
** tables - bcv_container, bcv_database and bcv_block.
*/
static int bcvReadonlyVtabOpen(sqlite3_vtab_cursor **ppCur, int nByte){
  int rc = SQLITE_OK;
  *ppCur = (sqlite3_vtab_cursor*)bcvMallocRc(&rc, nByte);
  return rc;
}
static int bcvDatabaseVtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCur){
  return bcvReadonlyVtabOpen(ppCur, sizeof(bcv_database_cursor));
}
static int bcvContainerVtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCur){
  return bcvReadonlyVtabOpen(ppCur, sizeof(bcv_container_cursor));
}
static int bcvBlockVtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCur){
  return bcvReadonlyVtabOpen(ppCur, sizeof(bcv_block_cursor));
}

static int bcvDatabaseVtabDisconnect(sqlite3_vtab *pVtab){
  bcv_database_vtab *pTab = (bcv_database_vtab*)pVtab;
  if( pTab ) sqlite3_free(pTab->zMod);
  sqlite3_free(pTab);
  return SQLITE_OK;
}

/*
** Implementation of xClose() for the three read-only vtabs.
*/
static int bcvDatabaseVtabClose(sqlite3_vtab_cursor *cur){
  bcv_database_cursor *pCur = (bcv_database_cursor*)cur;
  sqlite3_free(pCur->pFreeData);
  sqlite3_free(pCur);
  return SQLITE_OK;
}
static int bcvContainerVtabClose(sqlite3_vtab_cursor *cur){
  bcv_container_cursor *pCur = (bcv_container_cursor*)cur;
  sqlite3_free(pCur->pFreeData);
  sqlite3_free(pCur);
  return SQLITE_OK;
}
static int bcvBlockVtabClose(sqlite3_vtab_cursor *cur){
  bcv_block_cursor *pCur = (bcv_block_cursor*)cur;
  sqlite3_free(pCur->pFreeData);
  sqlite3_free(pCur);
  return SQLITE_OK;
}

/*
** Count the number of blocks belonging to database pDb that are present
** in cache pCommon. Return the result. Also, if pnDirty is not NULL,
** set (*pnDirty) to the number of dirty blocks in the cache belonging 
** to database pDb.
*/
int bcvCountCached(
  BcvCommon *pCommon, 
  Manifest *pMan, 
  ManifestDb *pDb, 
  int *pnDirty
){
  const int nName = NAMEBYTES(pMan);
  int jj;
  int nCache = 0;
  int nDirty = 0;
  for(jj=0; jj<pDb->nBlkLocal; jj++){
    u8 *pBlk = &pDb->aBlkLocal[jj*nName];
    CacheEntry *pEntry = bcvfsHashFind(pCommon, pBlk, nName);
    if( pEntry && pEntry->bValid ){
      nCache++;
      if( pEntry->bDirty ) nDirty++;
    }
  }
  if( pnDirty ) *pnDirty = nDirty;
  return nCache;
}

/*
** Obtain a blob containing the data to return for all rows of a query
** on either the "bcv_container" or "bcv_database" table.
*/
u8 *bcvDatabaseVtabData(
  int *pRc,                       /* IN/OUT: error code */
  BcvCommon *pCommon,             /* State to base returned data on */
  const char *zName,              /* "bcv_database" or "bcv_container" */
  const char *zContFilter,        /* Container to query, or NULL */
  const char *zDbFilter,          /* Database to query, or NULL */
  u32 colUsed,                    /* Columns used mask */
  int *pnData                     /* OUT: Size of returned buffer */
){
  int rc = SQLITE_OK;
  BcvBuffer buf = {0,0,0};

  if( bcvStrcmp(zName, "bcv_database")==0 ){
    Container *pCont;
    for(pCont=pCommon->pCList; pCont; pCont=pCont->pNext){
      int ii;
      Manifest *pMan = pCont->pMan;
      if( zContFilter && bcvStrcmp(zContFilter, pCont->zName) ) continue;
      for(ii=0; ii<pMan->nDb; ii++){
        ManifestDb *pDb = &pMan->aDb[ii];
        int nCache = 0;
        int nDirty = 0;
        int bWalfile = 0;
        const char *zState = "";
        if( zDbFilter && bcvStrcmp(zDbFilter, pDb->zDName) ) continue;
        if( colUsed & BCV_DATABASE_NCND ){
          nCache = bcvCountCached(pCommon, pMan, pDb, &nDirty);
        }
        if( colUsed & BCV_DATABASE_WALFILE ){
          bWalfile = bcvfsWalFileExists(
              &rc, pCommon, pCont->zLocalDir, pDb->zDName
          );
        }
        bcvBufferMsgString(&rc, &buf, pCont->zName);
        bcvBufferMsgString(&rc, &buf, pDb->zDName);
        bcvBufferAppendU32(&rc, &buf, pDb->nBlkLocal);
        bcvBufferAppendU32(&rc, &buf, (u32)nCache);
        bcvBufferAppendU32(&rc, &buf, (u32)nDirty);
        bcvBufferAppendU32(&rc, &buf, (u32)bWalfile);
        if( pDb->nBlkLocal==0 ){
          zState = "deleted";
        }else if( pDb->iDbId>=BCVFS_FIRST_LOCAL_ID ){
          zState = "copied";
        }
        bcvBufferMsgString(&rc, &buf, zState);
      }
    }
  }else if( bcvStrcmp(zName, "bcv_container")==0 ){
    Container *pCont;
    for(pCont=pCommon->pCList; pCont; pCont=pCont->pNext){
      if( zContFilter && bcvStrcmp(zContFilter, pCont->zName) ) continue;
      bcvBufferMsgString(&rc, &buf, pCont->zName);
      bcvBufferMsgString(&rc, &buf, pCont->zStorage);
      bcvBufferMsgString(&rc, &buf, pCont->zAccount);
      bcvBufferMsgString(&rc, &buf, pCont->zContainer);
      bcvBufferAppendU32(&rc, &buf, pCont->pMan->nDelBlk);
    }
  }else if( bcvStrcmp(zName, "bcv_block")==0 ){
    Container *pCont;
    for(pCont=pCommon->pCList; pCont; pCont=pCont->pNext){
      int ii;
      Manifest *pMan = pCont->pMan;
      int nName = NAMEBYTES(pMan);
      if( zContFilter && bcvStrcmp(zContFilter, pCont->zName) ) continue;
      for(ii=0; ii<pMan->nDb; ii++){
        ManifestDb *pDb = &pMan->aDb[ii];
        int iBlk;
        if( zDbFilter && bcvStrcmp(zDbFilter, pDb->zDName) ) continue;
        for(iBlk=0; iBlk<pDb->nBlkLocal; iBlk++){
          CacheEntry *pEntry = 0;
          int iOff = nName * iBlk;
          bcvBufferMsgString(&rc, &buf, pCont->zName);
          bcvBufferMsgString(&rc, &buf, pDb->zDName);
          bcvBufferAppendU32(&rc, &buf, (u32)iBlk);
          if( iBlk<pDb->nBlkOrig ){
            bcvBufferMsgBlob(&rc, &buf, &pDb->aBlkOrig[iOff], nName);
          }else{
            bcvBufferMsgBlob(&rc, &buf, 0, 0);
          }
          pEntry = bcvfsHashFind(pCommon, &pDb->aBlkLocal[iOff], nName);
          bcvBufferAppendU32(&rc, &buf, (u32)(pEntry!=0));
          bcvBufferAppendU32(&rc, &buf, (u32)(pEntry!=0 && pEntry->bDirty));
        }
      }
    }
  }

  if( rc!=SQLITE_OK ){
    bcvBufferZero(&buf);
  }
  *pRc = rc;
  *pnData = buf.nData;
  return buf.aData;
}

static u32 bcvVtabExtractU32(VtabBlob *p){
  if( p->iData>=p->nData ){
    return 0;
  }else{
    u32 iRet = bcvGetU32(&p->aData[p->iData]);
    p->iData += 4;
    return iRet;
  }
}

static const char *bcvVtabExtractString(VtabBlob *p){
  const char *zRet = 0;
  u32 nRet = 0;
  if( p->iData>=p->nData ){
    return 0;
  }
  nRet = bcvGetU32(&p->aData[p->iData]);
  p->iData += 4;
  zRet = (const char*)&p->aData[p->iData];
  p->iData += nRet;
  return zRet;
}

static const u8 *bcvVtabExtractBlob(VtabBlob *p, int *pnBlob){
  const u8 *zRet = 0;
  u32 nRet = 0;
  if( p->iData>=p->nData ){
    return 0;
  }
  nRet = bcvGetU32(&p->aData[p->iData]);
  p->iData += 4;
  zRet = (const u8*)&p->aData[p->iData];
  p->iData += nRet;
  *pnBlob = (int)nRet;
  return zRet;
}

static int bcvDatabaseVtabNext(sqlite3_vtab_cursor *cur){
  bcv_database_cursor *pCur = (bcv_database_cursor*)cur;

  pCur->zContainer = (char*)bcvVtabExtractString(&pCur->data);
  pCur->zDatabase = (char*)bcvVtabExtractString(&pCur->data);
  pCur->nBlock = (int)bcvVtabExtractU32(&pCur->data);
  pCur->nCache = (int)bcvVtabExtractU32(&pCur->data);
  pCur->nDirty = (int)bcvVtabExtractU32(&pCur->data);
  pCur->bWalfile = (int)bcvVtabExtractU32(&pCur->data);
  pCur->zState = (char*)bcvVtabExtractString(&pCur->data);

  pCur->iRow++;
  return SQLITE_OK;
}

static int bcvContainerVtabNext(sqlite3_vtab_cursor *cur){
  bcv_container_cursor *pCur = (bcv_container_cursor*)cur;

  pCur->zName = (char*)bcvVtabExtractString(&pCur->data);
  pCur->zStorage = (char*)bcvVtabExtractString(&pCur->data);
  pCur->zUser = (char*)bcvVtabExtractString(&pCur->data);
  pCur->zContainer = (char*)bcvVtabExtractString(&pCur->data);
  pCur->nCleanup = bcvVtabExtractU32(&pCur->data);

  pCur->iRow++;
  return SQLITE_OK;
}

static int bcvBlockVtabNext(sqlite3_vtab_cursor *cur){
  bcv_block_cursor *pCur = (bcv_block_cursor*)cur;

  pCur->zContainer = (char*)bcvVtabExtractString(&pCur->data);
  pCur->zDatabase = (char*)bcvVtabExtractString(&pCur->data);
  pCur->iBlockNo = bcvVtabExtractU32(&pCur->data);
  pCur->aBlockId = (u8*)bcvVtabExtractBlob(&pCur->data, &pCur->nBlockId);
  pCur->bDirty = bcvVtabExtractU32(&pCur->data);
  pCur->bCache = bcvVtabExtractU32(&pCur->data);

  pCur->iRow++;
  return SQLITE_OK;
}

/*
** Return values of columns for the row at which the templatevtab_cursor
** is currently pointing.
*/
static int bcvDatabaseVtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int i                       /* Which column to return */
){
  bcv_database_cursor *pCur = (bcv_database_cursor*)cur;
  switch( i ){
    case 0: /* container */
      sqlite3_result_text(ctx, pCur->zContainer, -1, SQLITE_TRANSIENT);
      break;
    case 1: /* database */
      sqlite3_result_text(ctx, pCur->zDatabase, -1, SQLITE_TRANSIENT);
      break;
    case 2: /* nblock */
      sqlite3_result_int(ctx, pCur->nBlock);
      break;
    case 3: /* ncache */
      sqlite3_result_int(ctx, pCur->nCache);
      break;
    case 4: /* dirty */
      sqlite3_result_int(ctx, pCur->nDirty);
      break;
    case 5: /* dirty */
      sqlite3_result_int(ctx, pCur->bWalfile);
      break;
    case 6: /* state */
      sqlite3_result_text(ctx, pCur->zState, -1, SQLITE_TRANSIENT);
      break;
  }
  return SQLITE_OK;
}

static int bcvContainerVtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int i                       /* Which column to return */
){
  bcv_container_cursor *pCur = (bcv_container_cursor*)cur;
  switch( i ){
    case 0: /* name */
      sqlite3_result_text(ctx, pCur->zName, -1, SQLITE_TRANSIENT);
      break;
    case 1: /* storage */
      sqlite3_result_text(ctx, pCur->zStorage, -1, SQLITE_TRANSIENT);
      break;
    case 2: /* user */
      sqlite3_result_text(ctx, pCur->zUser, -1, SQLITE_TRANSIENT);
      break;
    case 3: /* container */
      sqlite3_result_text(ctx, pCur->zContainer, -1, SQLITE_TRANSIENT);
      break;
    case 4: /* ncleanup */
      sqlite3_result_int(ctx, pCur->nCleanup);
      break;
  }
  return SQLITE_OK;
}

static int bcvBlockVtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int i                       /* Which column to return */
){
  bcv_block_cursor *pCur = (bcv_block_cursor*)cur;
  switch( i ){
    case 0: /* container */
      sqlite3_result_text(ctx, pCur->zContainer, -1, SQLITE_TRANSIENT);
      break;
    case 1: /* database */
      sqlite3_result_text(ctx, pCur->zDatabase, -1, SQLITE_TRANSIENT);
      break;
    case 2: /* blockno */
      sqlite3_result_int(ctx, pCur->iBlockNo);
      break;
    case 3: { /* blockid */
      int nBlockId = pCur->nBlockId;
      if( nBlockId>0 ){
        sqlite3_result_blob(ctx, pCur->aBlockId, nBlockId, SQLITE_TRANSIENT);
      }
      break;
    }
    case 4: /* dirty */
      sqlite3_result_int(ctx, pCur->bDirty);
      break;
    case 5: /* dirty */
      sqlite3_result_int(ctx, pCur->bCache);
      break;
  }
  return SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static int bcvDatabaseVtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bcv_database_cursor *pCur = (bcv_database_cursor*)cur;
  *pRowid = pCur->iRow;
  return SQLITE_OK;
}

static int bcvContainerVtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRow){
  bcv_database_cursor *pCur = (bcv_database_cursor*)cur;
  *pRow = pCur->iRow;
  return SQLITE_OK;
}

static int bcvBlockVtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRow){
  bcv_block_cursor *pCur = (bcv_block_cursor*)cur;
  *pRow = pCur->iRow;
  return SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static int bcvDatabaseVtabEof(sqlite3_vtab_cursor *cur){
  bcv_database_cursor *pCur = (bcv_database_cursor*)cur;
  return (pCur->zContainer==0);
}

static int bcvContainerVtabEof(sqlite3_vtab_cursor *cur){
  bcv_container_cursor *pCur = (bcv_container_cursor*)cur;
  return (pCur->zName==0);
}

static int bcvBlockVtabEof(sqlite3_vtab_cursor *cur){
  bcv_block_cursor *pCur = (bcv_block_cursor*)cur;
  return (pCur->zContainer==0);
}

/*
** pFile is a file opened via a proxy VFS. This function sends a message 
** to the daemon process to fetch the data required for a "SELECT * FROM
** zVtab" query and returns the message.
*/
static BcvMessage *bcvVtabFetchData(
  int *pRc,
  BcvfsFile *pFile, 
  const char *zVtab,
  const char *zCont,
  const char *zDb,
  u32 colUsed
){
  BcvMessage msg;
  BcvMessage *pReply;

  memset(&msg, 0, sizeof(msg));
  msg.eType = BCV_MESSAGE_VTAB;
  msg.u.vtab.zVtab = zVtab;
  msg.u.vtab.zContainer = zCont;
  msg.u.vtab.zDatabase = zDb;
  msg.u.vtab.colUsed = colUsed;

  pReply = bcvExchangeMessage(pRc, pFile->p.fdProxy, &msg);
  assert( (pReply==0)==(*pRc!=SQLITE_OK) );

  return pReply;
}


/*
** xFilter implentations for bcv_database and bcv_block.
*/
static int bcvReadonlyVtabFilter(
  sqlite3_vtab_cursor *cur, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  int rc = SQLITE_OK;
  BcvfsFile *pFile = 0;
  bcv_database_cursor *pCur = (bcv_database_cursor*)cur;
  bcv_database_vtab *pTab = (bcv_database_vtab*)cur->pVtab;
  u32 colUsed = (idxNum & BCV_COLUSED_MASK);
  const char *zCont = 0;
  const char *zDb = 0;
  int idx = 0;

  if( idxNum & BCV_CONTAINER_MASK ){
    zCont = (const char*)sqlite3_value_text(argv[idx]);
    idx++;
  }
  if( idxNum & BCV_DATABASE_MASK ){
    zDb = (const char*)sqlite3_value_text(argv[idx]);
  }

  pCur->iRow = 0;
  sqlite3_free(pCur->pFreeData);
  pCur->pFreeData = 0;
  memset(&pCur->data, 0, sizeof(VtabBlob));

  sqlite3_file_control(pTab->db, "main", BCV_FCNTL_FD, (void*)&pFile);
  if( pFile ){
    if( pFile->pFs->zPortnumber ){
      BcvMessage *pMsg = bcvVtabFetchData(
          &rc, pFile, pTab->zMod, zCont, zDb, colUsed
      );
      if( pMsg ){
        pCur->data.aData = pMsg->u.vtab_r.aData;
        pCur->data.nData = pMsg->u.vtab_r.nData;
        pCur->pFreeData = (void*)pMsg;
      }
    }else{
      pCur->data.aData = bcvDatabaseVtabData(&rc, &pFile->pFs->c, 
          pTab->zMod, zCont, zDb, colUsed, &pCur->data.nData
      );
      pCur->pFreeData = (void*)pCur->data.aData;
    }
    if( rc==SQLITE_OK ){
      return pTab->base.pModule->xNext(cur);
    }
  }
  return rc;
}


static int bcvContainerVtabFilter(
  sqlite3_vtab_cursor *cur, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  int rc = SQLITE_OK;
  BcvfsFile *pFile = 0;
  bcv_container_cursor *pCur = (bcv_container_cursor*)cur;
  bcv_database_vtab *pTab = (bcv_database_vtab*)cur->pVtab;
  u32 colUsed = (idxNum & BCV_COLUSED_MASK);
  const char *zCont = 0;

  if( idxNum & BCV_CONTAINER_MASK ){
    zCont = (const char*)sqlite3_value_text(argv[0]);
  }

  pCur->iRow = 0;
  sqlite3_free(pCur->pFreeData);
  pCur->pFreeData = 0;
  memset(&pCur->data, 0, sizeof(VtabBlob));

  sqlite3_file_control(pTab->db, "main", BCV_FCNTL_FD, (void*)&pFile);
  if( pFile ){
    if( pFile->pFs->zPortnumber ){
      BcvMessage *pMsg = bcvVtabFetchData(
          &rc, pFile, "bcv_container", zCont, 0, colUsed
      );
      if( pMsg ){
        pCur->data.aData = pMsg->u.vtab_r.aData;
        pCur->data.nData = pMsg->u.vtab_r.nData;
        pCur->pFreeData = (void*)pMsg;
      }
    }else{
      pCur->data.aData = bcvDatabaseVtabData(&rc, &pFile->pFs->c, 
          "bcv_container", zCont, 0, colUsed, &pCur->data.nData
      );
      pCur->pFreeData = (void*)pCur->data.aData;
    }
    if( rc==SQLITE_OK ){
      return bcvContainerVtabNext(cur);
    }
  }
  return rc;
}

/*
** SQLite will invoke this method one or more times while planning a query
** that uses the virtual table.  This routine needs to create
** a query plan for each invocation and compute an estimated cost for that
** plan.
*/
static int bcvContainerBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  pIdxInfo->estimatedCost = (double)(10*1000*1000);
  pIdxInfo->estimatedRows = 10*1000*1000;
  pIdxInfo->idxNum = pIdxInfo->colUsed;
  return SQLITE_OK;
}

/*
** SQLite will invoke this method one or more times while planning a query
** that uses the virtual table.  This routine needs to create
** a query plan for each invocation and compute an estimated cost for that
** plan.
**
** This is used by both bcv_container and bcv_block.
*/
static int bcvDatabaseVtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  int ii;
  int iCont = -1;
  int iDb = -1;

  for(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint *pCons = &pIdxInfo->aConstraint[ii];
    if( pCons->usable && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      if( pCons->iColumn==0 ) iCont = ii;
      if( pCons->iColumn==1 ) iDb = ii;
    }
  }

  pIdxInfo->idxNum = (pIdxInfo->colUsed & BCV_COLUSED_MASK);
  if( iCont>=0 ){
    pIdxInfo->aConstraintUsage[iCont].argvIndex = 1;
    pIdxInfo->idxNum += BCV_CONTAINER_MASK;
  }
  if( iDb>=0 ){
    pIdxInfo->aConstraintUsage[iDb].argvIndex = (iCont>=0) ? 2 : 1;
    pIdxInfo->idxNum += BCV_DATABASE_MASK;
  }
  pIdxInfo->estimatedCost = (double)(10*1000*1000);
  pIdxInfo->estimatedRows = 10*1000*1000;

  return SQLITE_OK;
}

/*************************************************************************
** Start of bcv_kv implementation.
*/

typedef struct bcv_kv_vtab bcv_kv_vtab;
typedef struct bcv_kv_csr bcv_kv_csr;

/*
** Virtual table type for eponymous virtual table bcv_kv.
*/
struct bcv_kv_vtab {
  sqlite3_vtab base;              /* Base class */
  sqlite3 *db;                    /* Database handle for client db */
  BcvfsFile *pFile;               /* File object to use to access container */
};

/*
** Virtual cursor type for bcv_kv.
**
** pSelect:
**   Compiled version of "SELECT rowid, name, value FROM kv" on the 
**   kv database. Current row of this statement is the current row of
**   the cursor. pSelect==0 means EOF.
*/
struct bcv_kv_csr {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_stmt *pSelect;
};

/*
** This is both the xCreate and xConnect method for the bcv_kv table. The
** argv[] array contains the following:
**
**   argv[0]   -> module name
**   argv[1]   -> database name
**   argv[2]   -> table name
*/
static int bcvFileVtabConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  BcvfsFile *pFile = 0;
  bcv_kv_vtab *pNew = 0;
  int rc = SQLITE_OK;

  rc = sqlite3_file_control(db, argv[1], BCV_FCNTL_FD, (void*)&pFile);
  assert( pFile || rc!=SQLITE_OK );
  if( pFile ){
    if( pFile->pCont==0 ){
      rc = SQLITE_NOTFOUND;
    }else{
      rc = sqlite3_declare_vtab(db, BCV_FILE_VTAB_SCHEMA);
      pNew = (bcv_kv_vtab*)bcvMallocRc(&rc, sizeof(bcv_kv_vtab));
      if( pNew ){
        pNew->db = db;
        pNew->pFile = pFile;
      }
    }
  }

  *ppVtab = (sqlite3_vtab*)pNew;
  return rc;
}

static int bcvFileVtabDisconnect(sqlite3_vtab *pVtab){
  bcv_kv_vtab *pTab = (bcv_kv_vtab*)pVtab;
  sqlite3_free(pTab);
  return SQLITE_OK;
}

static int bcvFileVtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCur){
  int rc = SQLITE_OK;
  bcv_kv_csr *pNew = 0;
  pNew = (bcv_kv_csr*)bcvMallocRc(&rc, sizeof(bcv_kv_csr));
  *ppCur = &pNew->base;
  return rc;
}

static int bcvFileVtabClose(sqlite3_vtab_cursor *cur){
  bcv_kv_csr *pCur = (bcv_kv_csr*)cur;
  sqlite3_finalize(pCur->pSelect);
  sqlite3_free(pCur);
  return SQLITE_OK;
}

/*
** xBestIndex method for bcv_kv.
*/
static int bcvFileVtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  pIdxInfo->estimatedCost = (double)(10*1000*1000);
  pIdxInfo->estimatedRows = 10*1000*1000;
  return SQLITE_OK;
}

/*
** xNext method for bcv_kv.
*/
static int bcvFileVtabNext(sqlite3_vtab_cursor *cur){
  bcv_kv_csr *pCur = (bcv_kv_csr*)cur;
  int rc;
  assert( pCur->pSelect );
  rc = sqlite3_step(pCur->pSelect);
  if( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCur->pSelect);
    pCur->pSelect = 0;
  }else{
    rc = SQLITE_OK;
  }
  return rc;
}

typedef struct FetchKvCtx FetchKvCtx;
struct FetchKvCtx {
  int rc;
  char *zErr;
  BcvKVStore *pKv;
};

/*
** If successful, return a pointer to a buffer containing the serialized
** version of an empty kv database. Set *pnData to the size of the buffer in
** bytes before returning. It is the responsibility of the caller to eventually
** free the returned buffer using sqlite3_free().
*/
u8 *bcvEmptyKV(int *pRc, int *pnData){
  u8 *aRet = 0;
  *pnData = 0;
  if( *pRc==SQLITE_OK ){
    sqlite3 *db = 0;
    int rc = sqlite3_open(":memory:", &db);
    if( rc==SQLITE_OK ){
      rc = sqlite3_exec(db, 
          "CREATE TABLE kv(i INTEGER PRIMARY KEY, k UNIQUE NOT NULL, v)", 
          0, 0, 0
      );
    }
    if( rc==SQLITE_OK ){
      i64 n = 0;
      aRet = sqlite3_serialize(db, "main", &n, 0);
      if( aRet ){
        *pnData = (int)n;
      }else{
        rc = SQLITE_NOMEM;
      }
    }
    sqlite3_close(db);
    *pRc = rc;
  }
  return aRet;
}

static void bcvfsFetchKvCb(
  void *pApp, 
  int rc, 
  char *zETag, 
  const u8 *aData, 
  int nData
){
  FetchKvCtx *p = (FetchKvCtx*)pApp;
  u8 *aCopy = 0;
  int nCopy = nData;
  if( rc==SQLITE_OK ){
    aCopy = bcvMallocRc(&p->rc, nData);
    if( aCopy ) memcpy(aCopy, aData, nData);
  }else if( rc==HTTP_NOT_FOUND ){
    rc = SQLITE_OK;
    zETag = "*";
    aCopy = bcvEmptyKV(&rc, &nCopy);
  }else{
    p->rc = rc;
    if( zETag ) p->zErr = sqlite3_mprintf("%s", zETag);
  }

  if( aCopy ){
    sqlite3 *db = 0;
    p->rc = sqlite3_open(":memory:", &db);
    if( p->rc==SQLITE_OK ){
      p->pKv->zETag = bcvStrdupRc(&p->rc, zETag);
      if( p->rc==SQLITE_OK ){
        p->rc = sqlite3_deserialize(db, "main", aCopy, nCopy, nCopy, 
            SQLITE_DESERIALIZE_FREEONCLOSE|SQLITE_DESERIALIZE_RESIZEABLE
        );
        if( sqlite3_errcode(db) ) p->rc = sqlite3_errcode(db);
        aCopy = 0;
      }
    }
    p->pKv->db = db;
    if( p->rc!=SQLITE_OK ){
      bcvKVStoreFree(p->pKv);
    }
    sqlite3_free(aCopy);
  }
}

/*
** Set the virtual table error message to the result of printf(zFmt, ...).
*/
static void bcvSetVtabErr(sqlite3_vtab *pVtab, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  sqlite3_free(pVtab->zErrMsg);
  pVtab->zErrMsg = sqlite3_vmprintf(zFmt, ap);
  va_end(ap);
}

static int bcvKVStoreLoad(bcv_kv_vtab *pTab){
  BcvfsFile *pFile = pTab->pFile;
  int rc = SQLITE_OK;

  assert( pFile->lockMask!=0 );
  if( pFile->kv.db==0 ){
    sqlite3_bcvfs *pFs = pFile->pFs;
    BcvContainer *pBcv = 0;
    BcvDispatch *pDisp = 0;
    int iIter = 0;
    FetchKvCtx fkv = {0,0,0};

    do {
      rc = SQLITE_OK;
      sqlite3_free(fkv.zErr);
      memset(&fkv, 0, sizeof(fkv));
      fkv.pKv = &pFile->kv;

      ENTER_VFS_MUTEX; {
        rc = bcvfsContainerGetBcv(pFs, pFile->pCont, &pDisp, &pBcv, 0); 
      } LEAVE_VFS_MUTEX;

      if( rc==SQLITE_OK ){
        rc = bcvDispatchFetch(
            pDisp, pBcv, BCV_KV_FILE, 0, 0, (void*)&fkv, bcvfsFetchKvCb
        );
      }
      if( rc==SQLITE_OK ){
        rc = bcvDispatchRunAll(pDisp);
      }
      if( rc==SQLITE_OK ) rc = fkv.rc;

      ENTER_VFS_MUTEX; {
        bcvfsContainerReleaseBcv(pFs, pFile->pCont, pDisp, pBcv, rc);
      } LEAVE_VFS_MUTEX;
    }while( rc==HTTP_AUTH_ERROR && (++iIter<BCVFS_MAX_AUTH_RETRIES) );

    if( fkv.zErr ){
      bcvSetVtabErr(&pTab->base,
          "download bcv_kv.bcv failed (%d) - %s", rc, fkv.zErr
      );
      sqlite3_free(fkv.zErr);
    }
  }

  return rc;
}

static void bcvfsPutKvCb(void *pApp, int rc, char *zETag){
  FetchKvCtx *p = (FetchKvCtx*)pApp;
  if( rc==SQLITE_OK ){
    char *zCopy = bcvStrdupRc(&p->rc, zETag);
    sqlite3_free(p->pKv->zETag);
    p->pKv->zETag = zCopy;
  }else{
    p->rc = rc;
    if( zETag ) p->zErr = sqlite3_mprintf("%s", zETag);
  }
}

static int bcvKVStorePush(bcv_kv_vtab *pTab){
  BcvfsFile *pFile = pTab->pFile;
  int rc = SQLITE_OK;

  assert( pFile->lockMask!=0 );
  if( pFile->kv.db ){
    BcvKVStore *pKv = &pFile->kv;
    sqlite3_bcvfs *pFs = pFile->pFs;
    BcvContainer *pBcv = 0;
    BcvDispatch *pDisp = 0;
    int iIter = 0;
    FetchKvCtx fkv = {0,0,0};

    do {
      u8 *aData = 0;
      rc = SQLITE_OK;
      sqlite3_free(fkv.zErr);
      memset(&fkv, 0, sizeof(fkv));
      fkv.pKv = pKv;

      ENTER_VFS_MUTEX; {
        rc = bcvfsContainerGetBcv(pFs, pFile->pCont, &pDisp, &pBcv, 0); 
      } LEAVE_VFS_MUTEX;

      if( rc==SQLITE_OK ){
        i64 nData = 0;
        aData = sqlite3_serialize(pKv->db, "main", &nData, 0);
        if( aData==0 ){
          rc = SQLITE_NOMEM;
        }else{
          rc = bcvDispatchPut(pDisp, pBcv, BCV_KV_FILE, 
              pKv->zETag, aData, nData, (void*)&fkv, bcvfsPutKvCb
          );
        }
      }
      if( rc==SQLITE_OK ){
        rc = bcvDispatchRunAll(pDisp);
      }
      if( rc==SQLITE_OK ) rc = fkv.rc;
      sqlite3_free(aData);

      ENTER_VFS_MUTEX; {
        bcvfsContainerReleaseBcv(pFs, pFile->pCont, pDisp, pBcv, rc);
      } LEAVE_VFS_MUTEX;
    }while( rc==HTTP_AUTH_ERROR && (++iIter<BCVFS_MAX_AUTH_RETRIES) );

    if( fkv.zErr ){
      bcvSetVtabErr(&pTab->base,
          "upload bcv_kv.bcv failed (%d) - %s", rc, fkv.zErr
      );
      sqlite3_free(fkv.zErr);
    }
    if( rc==HTTP_BUSY_SNAPSHOT ){
      rc = SQLITE_BUSY_SNAPSHOT;
    }else{
      rc = bcvErrorToSqlite(rc);
    }
  }

  return rc;
}

static int bcvFileVtabFilter(
  sqlite3_vtab_cursor *cur, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  bcv_kv_vtab *pTab = (bcv_kv_vtab*)cur->pVtab;
  bcv_kv_csr *pCur = (bcv_kv_csr*)cur;
  int rc = SQLITE_OK;

  sqlite3_finalize(pCur->pSelect);
  pCur->pSelect = 0;

  assert( pTab->pFile->lockMask!=0 );
  rc = bcvKVStoreLoad(pTab);
  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare(pTab->pFile->kv.db, 
        "SELECT i, k, v FROM kv", -1, &pCur->pSelect, 0
    );
  }
  if( rc==SQLITE_OK ){
    rc = bcvFileVtabNext(cur);
  }
  return bcvErrorToSqlite(rc);
}

static int bcvFileVtabEof(sqlite3_vtab_cursor *cur){
  bcv_kv_csr *pCur = (bcv_kv_csr*)cur;
  return pCur->pSelect==0;
}

static int bcvFileVtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int i                       /* Which column to return */
){
  bcv_kv_csr *pCur = (bcv_kv_csr*)cur;
  sqlite3_value *pVal = 0;
  assert( i==0 || i==1 );
  pVal = sqlite3_column_value(pCur->pSelect, i+1);
  sqlite3_result_value(ctx, pVal);
  return SQLITE_OK;
}

static int bcvFileVtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bcv_kv_csr *pCur = (bcv_kv_csr*)cur;
  *pRowid = sqlite3_column_int64(pCur->pSelect, 0);
  return SQLITE_OK;
}

static sqlite3_stmt *bcvFilePrepare(
  int *pRc,
  bcv_kv_vtab *pTab,
  const char *zSql
){
  sqlite3_stmt *pRet = 0;
  if( *pRc==SQLITE_OK ){
    sqlite3 *db = pTab->pFile->kv.db;
    int rc = sqlite3_prepare(db, zSql, -1, &pRet, 0);
    if( rc!=SQLITE_OK ){
      bcvSetVtabErr(&pTab->base, "%s", sqlite3_errmsg(db));
    }
    *pRc = rc;
  }
  return pRet;
}

/*
** IN/OUT error code wrapper around sqlite3_bind_value().
*/
static void bcvBindValue(
  int *pRc,                       /* IN/OUT: Error code */
  sqlite3_stmt *pStmt,            /* Statement to bind value to */
  int iVar,                       /* Statement variable to bind to */
  sqlite3_value *pVal             /* Value to bind */
){
  if( *pRc==SQLITE_OK ){
    *pRc = sqlite3_bind_value(pStmt, iVar, pVal);
  }
}

/* 
** This function is the implementation of the xUpdate callback used by 
** bcv_kv virtual tables. It is invoked by SQLite each time a row is 
** to be inserted, updated or deleted.
**
** A delete specifies a single argument - the rowid of the row to remove.
** 
** Update and insert operations pass:
**
**   1. The "old" rowid, or NULL.
**   2. The "new" rowid.
**   3. Value for the "name" column of the new row.
**   4. Value for the "content" column of the new row.
*/
static int bcvFileVtabUpdate(
  sqlite3_vtab *tab,
  int nVal,
  sqlite3_value **apVal, 
  sqlite3_int64 *piRowid
){
  bcv_kv_vtab *pTab = (bcv_kv_vtab*)tab;
  int bReplace = sqlite3_vtab_on_conflict(pTab->db)==SQLITE_REPLACE;
  int rc = SQLITE_OK;

  assert( pTab->pFile->kv.db );
  if( rc==SQLITE_OK ){
    sqlite3_stmt *pStmt = 0;

    if( nVal==1 ){
      /* A DELETE */
      pStmt = bcvFilePrepare(&rc, pTab, "DELETE FROM kv WHERE i=?");
      bcvBindValue(&rc, pStmt, 1, apVal[0]);
    }else if( sqlite3_value_type(apVal[0])==SQLITE_NULL ){
      if( bReplace ){
        pStmt = bcvFilePrepare(&rc, pTab, "REPLACE INTO kv VALUES(?,?,?)");
      }else{
        pStmt = bcvFilePrepare(&rc, pTab, "INSERT INTO kv VALUES(?,?,?)");
      }
      bcvBindValue(&rc, pStmt, 1, apVal[1]);
      bcvBindValue(&rc, pStmt, 2, apVal[2]);
      bcvBindValue(&rc, pStmt, 3, apVal[3]);
    }else{
      pStmt = bcvFilePrepare(&rc, pTab, "UPDATE kv set i=?,k=?,v=? WHERE i=?");
      bcvBindValue(&rc, pStmt, 1, apVal[1]);
      bcvBindValue(&rc, pStmt, 2, apVal[2]);
      bcvBindValue(&rc, pStmt, 3, apVal[3]);
      bcvBindValue(&rc, pStmt, 4, apVal[0]);
    }

    if( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      rc = sqlite3_finalize(pStmt);
    }else{
      sqlite3_finalize(pStmt);
    }
  }

  return bcvErrorToSqlite(rc);
}

/*
** bcv_kv.xBegin()
*/
static int bcvFileVtabBegin(sqlite3_vtab *tab){
  bcv_kv_vtab *pTab = (bcv_kv_vtab*)tab;
  int rc = bcvKVStoreLoad(pTab);
  if( rc==SQLITE_OK ){
    rc = sqlite3_exec(pTab->pFile->kv.db, "BEGIN", 0, 0, &pTab->base.zErrMsg);
  }
  return bcvErrorToSqlite(rc);
}

/*
** bcv_kv.xRollback()
*/
static int bcvFileVtabRollback(sqlite3_vtab *tab){
  bcv_kv_vtab *pTab = (bcv_kv_vtab*)tab;
  int rc = SQLITE_OK;
  sqlite3_exec(pTab->pFile->kv.db, "ROLLBACK", 0, 0, 0);
  return rc;
}

/*
** bcv_kv.xSync()
*/
static int bcvFileVtabSync(sqlite3_vtab *tab){
  bcv_kv_vtab *pTab = (bcv_kv_vtab*)tab;
  int rc = SQLITE_OK;
  rc = sqlite3_exec(pTab->pFile->kv.db, "COMMIT", 0, 0, &pTab->base.zErrMsg);
  if( rc==SQLITE_OK ){
    rc = bcvKVStorePush(pTab);
  }
  return rc;
}

/*
** bcv_kv.xCommit()
*/
static int bcvFileVtabCommit(sqlite3_vtab *tab){
  return SQLITE_OK;
}


int sqlite3_bcvfs_register_vtab(sqlite3 *db){
  static sqlite3_module bcv_database = {
    /* iVersion    */ 2,
    /* xCreate     */ 0,
    /* xConnect    */ bcvDatabaseVtabConnect,
    /* xBestIndex  */ bcvDatabaseVtabBestIndex,
    /* xDisconnect */ bcvDatabaseVtabDisconnect,
    /* xDestroy    */ 0,
    /* xOpen       */ bcvDatabaseVtabOpen,
    /* xClose      */ bcvDatabaseVtabClose,
    /* xFilter     */ bcvReadonlyVtabFilter,
    /* xNext       */ bcvDatabaseVtabNext,
    /* xEof        */ bcvDatabaseVtabEof,
    /* xColumn     */ bcvDatabaseVtabColumn,
    /* xRowid      */ bcvDatabaseVtabRowid,
    /* xUpdate     */ 0,
    /* xBegin      */ 0,
    /* xSync       */ 0,
    /* xCommit     */ 0,
    /* xRollback   */ 0,
    /* xFindMethod */ 0,
    /* xRename     */ 0,
    /* xSavepoint  */ 0,
    /* xRelease    */ 0,
    /* xRollbackTo */ 0,
    /* xShadowName */ 0
  };
  static sqlite3_module bcv_container = {
    /* iVersion    */ 2,
    /* xCreate     */ 0,
    /* xConnect    */ bcvDatabaseVtabConnect,
    /* xBestIndex  */ bcvContainerBestIndex,
    /* xDisconnect */ bcvDatabaseVtabDisconnect,
    /* xDestroy    */ 0,
    /* xOpen       */ bcvContainerVtabOpen,
    /* xClose      */ bcvContainerVtabClose,
    /* xFilter     */ bcvContainerVtabFilter,
    /* xNext       */ bcvContainerVtabNext,
    /* xEof        */ bcvContainerVtabEof,
    /* xColumn     */ bcvContainerVtabColumn,
    /* xRowid      */ bcvContainerVtabRowid,
    /* xUpdate     */ 0,
    /* xBegin      */ 0,
    /* xSync       */ 0,
    /* xCommit     */ 0,
    /* xRollback   */ 0,
    /* xFindMethod */ 0,
    /* xRename     */ 0,
    /* xSavepoint  */ 0,
    /* xRelease    */ 0,
    /* xRollbackTo */ 0,
    /* xShadowName */ 0
  };
  static sqlite3_module bcv_block = {
    /* iVersion    */ 2,
    /* xCreate     */ 0,
    /* xConnect    */ bcvDatabaseVtabConnect,
    /* xBestIndex  */ bcvDatabaseVtabBestIndex,
    /* xDisconnect */ bcvDatabaseVtabDisconnect,
    /* xDestroy    */ 0,
    /* xOpen       */ bcvBlockVtabOpen,
    /* xClose      */ bcvBlockVtabClose,
    /* xFilter     */ bcvReadonlyVtabFilter,
    /* xNext       */ bcvBlockVtabNext,
    /* xEof        */ bcvBlockVtabEof,
    /* xColumn     */ bcvBlockVtabColumn,
    /* xRowid      */ bcvBlockVtabRowid,
    /* xUpdate     */ 0,
    /* xBegin      */ 0,
    /* xSync       */ 0,
    /* xCommit     */ 0,
    /* xRollback   */ 0,
    /* xFindMethod */ 0,
    /* xRename     */ 0,
    /* xSavepoint  */ 0,
    /* xRelease    */ 0,
    /* xRollbackTo */ 0,
    /* xShadowName */ 0
  };

  static sqlite3_module bcv_kv = {
    /* iVersion    */ 2,
    /* xCreate     */ 0,
    /* xConnect    */ bcvFileVtabConnect,
    /* xBestIndex  */ bcvFileVtabBestIndex,
    /* xDisconnect */ bcvFileVtabDisconnect,
    /* xDestroy    */ 0,
    /* xOpen       */ bcvFileVtabOpen,
    /* xClose      */ bcvFileVtabClose,
    /* xFilter     */ bcvFileVtabFilter,
    /* xNext       */ bcvFileVtabNext,
    /* xEof        */ bcvFileVtabEof,
    /* xColumn     */ bcvFileVtabColumn,
    /* xRowid      */ bcvFileVtabRowid,
    /* xUpdate     */ bcvFileVtabUpdate,
    /* xBegin      */ bcvFileVtabBegin,
    /* xSync       */ bcvFileVtabSync,
    /* xCommit     */ bcvFileVtabCommit,
    /* xRollback   */ bcvFileVtabRollback,

    /* xFindMethod */ 0,
    /* xRename     */ 0,
    /* xSavepoint  */ 0,
    /* xRelease    */ 0,
    /* xRollbackTo */ 0,
    /* xShadowName */ 0
  };
  int rc = SQLITE_OK;
  rc = sqlite3_create_module(
      db, "bcv_database", &bcv_database, (void*)BCV_DATABASE_VTAB_SCHEMA
  );
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_module(
        db, "bcv_container", &bcv_container, (void*)BCV_CONTAINER_VTAB_SCHEMA
    );
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_module(
        db, "bcv_block", &bcv_block, (void*)BCV_BLOCK_VTAB_SCHEMA
    );
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_module(
        db, "bcv_kv", &bcv_kv, (void*)BCV_FILE_VTAB_SCHEMA
    );
  }
  return rc;
}



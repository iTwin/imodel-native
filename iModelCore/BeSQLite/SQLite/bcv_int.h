/*
** 2020-05-12
**
******************************************************************************
**
** This header file contains declarations for interfaces in bcvutil.c used
** exclusively by blockcachevfsd.c. This file should not be directly
** included by applications.
*/

#if !defined(__WIN32__) && (defined(WIN32) || defined(_WIN32))
# define __WIN32__
#endif

#include "sqlite3.h"

typedef sqlite3_int64 i64;
typedef sqlite3_uint64 u64;
typedef unsigned char u8;
typedef unsigned int u32;

#ifdef __WIN32__
# include <winsock2.h>
# include <windows.h>
# include <ws2tcpip.h>
# define BCV_SOCKET_TYPE SOCKET
#else
# include <unistd.h>
# include <sys/types.h> 
# include <sys/socket.h> 
# include <arpa/inet.h> 
# define BCV_SOCKET_TYPE int
# define O_BINARY 0
# define INVALID_SOCKET -1
#endif

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

#include "bcvutil.h"
#include "bcvmodule.h"
#include "simplexml.h"
#include <curl/curl.h>

#define BCV_MANIFEST_FILE            "manifest.bcv"
#define BCV_KV_FILE                  "bcv_kv.bcv"

/* Number of bytes in the database id that is part of each db header */
#define BCV_DBID_SIZE 16

/* Space in bytes available within manifest file for database display 
** name. This is the maximum length of the utf-8 version of a db name,
** including a nul-terminator.  */
#define BCV_DBNAME_SIZE 128

/* Size of manifiest header with no databases. */
#define BCV_MANIFEST_HEADER_BYTES 20

/* Extra bytes in manifest header for each db */
#define BCV_MANIFEST_DBHEADER_BYTES (                   \
    3*4+BCV_DBID_SIZE+BCV_DBNAME_SIZE   \
)

/* Current version of manifest file format */
#define BCV_MANIFEST_VERSION 4

/* Block identifiers must be between 12 and 32 bytes in size. Between
** 96 and 256 bits of content-hash/random-values to guarantee uniqueness.  */
#define BCV_MIN_NAMEBYTES            12
#define BCV_MAX_NAMEBYTES            32

/* Minimum required name-size to use md5 hashing. */
#define BCV_MIN_MD5_NAMEBYTES        24

#define BCV_FSNAMEBYTES(namebytes) (((namebytes)*2)+4+1)
#define BCV_MAX_FSNAMEBYTES BCV_FSNAMEBYTES(BCV_MAX_NAMEBYTES)

#define NAMEBYTES(pManifest)    ((pManifest)->nNamebytes)
#define GCENTRYBYTES(pManifest) (NAMEBYTES(pManifest) + 8)

#define BCV_CACHEFILE_NAME "cachefile.bcv\0"
#define BCV_DATABASE_NAME  "blocksdb.bcv"

#define BCV_DEFAULT_NAMEBYTES        16
#define BCV_DEFAULT_BLOCKSIZE        (4*1024*1024)
#define BCV_DEFAULT_HTTPTIMEOUT 600  

/* Size of local encryption keys in bytes. */
#define BCV_LOCAL_KEYSIZE        16

#define HTTP_AUTH_ERROR          403
#define HTTP_PRECONDITION_FAILED 304
#define HTTP_NOT_FOUND           404
#define HTTP_BUSY_SNAPSHOT       412        /* 'if-match' PUT test failed */

#define MIN(a,b)  ((a)<(b) ? (a) : (b))
#define MAX(a,b)  ((a)>(b) ? (a) : (b))

typedef unsigned int u32;
typedef unsigned char u8;
typedef sqlite3_uint64 u64;
typedef sqlite3_int64 i64;

typedef struct ManifestHash ManifestHash;
typedef struct Manifest Manifest;
typedef struct ManifestDb ManifestDb;
typedef struct BcvCommon BcvCommon;
typedef struct Container Container;
typedef struct CacheEntry CacheEntry;
typedef struct BcvDispatch BcvDispatch;
typedef struct BcvContainer BcvContainer;
typedef struct BcvWrapper BcvWrapper;
typedef struct BcvBuffer BcvBuffer;
typedef struct BcvEncryptionKey BcvEncryptionKey;


struct Manifest {
  int nRef;                       /* Number of pointers to this object */
  int nDb;                        /* Number of databases */
  int szBlk;                      /* Block size */
  int nNamebytes;                 /* Bytes in each block name */
  u32 iMaxDbId;                   /* Largest dbid ever assigned */
  ManifestDb *aDb;                /* Array of nDb databases */

  int bDelFree;                   /* If true, aDelBlk must be freed */
  int nDelBlk;                    /* Number of blocks in delete array */
  u8 *aDelBlk;                    /* Array of delete blocks */

  char *zETag;                    /* e-tag value for this manifest */
  u8 *pFree;                      /* Blob of data to free */
};

/*
** aBlkOrig, nBlkOrig, bBlkOrigFree:
**   aBlkOrig[] contains the block array for the database as it existed
**   when the manifest file was downloaded. nBlkArray is the number of
**   blocks it contains. If the manifest is serialized and uploaded, this is
**   the block array that will form part of the new manifest. If bBlkOrigFree
**   is non-zero, then aBlkOrig[] must be freed using sqlite3_free when this
**   object is deallocated.
**
** aBlkLocal, nBlkLocal, nBlkLocalAlloc:
*/
struct ManifestDb {
  i64 iDbId;                      /* Database id */
  i64 iParentId;                  /* Id of parent database, if any */
  char zDName[BCV_DBNAME_SIZE];   /* Database display name */
  int iVersion;                   /* Database version */

  /* Original block array (as it exists or existed in cloud storage). */
  u8 *aBlkOrig;
  int nBlkOrig;
  int nBlkOrigAlloc;

  /* Current block array */
  u8 *aBlkLocal;
  int nBlkLocal;
  int nBlkLocalAlloc;
};

#define BCVFS_INSERT_BLOCK    "REPLACE INTO block VALUES(?,?,?,?,?,?)"
#define BCVFS_INSERT_CONT     "REPLACE INTO container VALUES(?,?,?,?,?,?)"

/*
** bDaemon:
**   This is true if the object is part of a daemon process, false for
**   a local read/write VFS.
** 
** pInsertBlock
**   INSERT statement used for writing to the "block" table:
**      REPLACE INTO block VALUES(?,?,?,?,?,?,?,?);
**
** pInsertCont
**   INSERT statement used for writing to the "manifest" table:
**      REPLACE INTO manifest VALUES(?,?,?,?,?,?);
**
** aHash, nHash:
**   aHash[] is a hash table array nHash entries in size containing
**   CacheEntry structs, indexed by the block name of the block that
**   the corresponding cache slot contains. The hash table contains
**   any entry for each cache slot that (a) contains a valid block, or
**   (b) is currently the target slot for an ongoing download of a new 
**   block.
**
** pUnused:
**   Singly-linked list of CacheEntry structs for cache-file slots that
**   do not contain a valid block and have no thread currently downloading
**   one. This comes about if an error occurs while downloading a block.
**   List is linked using CacheEntry.pHashNext. A CacheEntry struct may
**   be in the hash table or in this list, but not both.
**
** nBlk:
**   Size of cache file in blocks. This value should always be equal to 
**   the number of CacheEntry structs in the hash table and pUnused list
**   combined. See function assert_no_cache_leaks().
*/
struct BcvCommon {
  int bDaemon;                    /* True for daemon, false for local */
  sqlite3_vfs *pVfs;
  char *zDir;                     /* Full path for directory to use */
  i64 szBlk;                      /* Block size for this cache file */
  i64 nMaxCache;                  /* SQLITE_BCV_CACHESIZE value */
  Container *pCList;              /* List of all "attached" containers */
  CacheEntry **aHash;
  CacheEntry *pUnused;
  CacheEntry *pLruFirst;
  CacheEntry *pLruLast;
  i64 iLruTick;
  int nHash;                      /* Number of hash buckets */
  int nBlk;                       /* Number of blocks in cache file */

  /* To access blocksdb.bcv */
  sqlite3 *bdb;                   /* Open database handle */
  sqlite3_stmt *pInsertBlock;     /* REPLACE INTO blocks... */
  sqlite3_stmt *pInsertCont;      /* REPLACE INTO container... */
};

/*
** nClient:
**   Number of database handles open on databases within this container
**   using this VFS module.
**
** zLocalDir:
**   Full path to the local directory used for this container. The last
**   character of the path is not a directory separator.
*/
struct Container {
  const char *zName;
  const char *zStorage;           /* Storage module */
  const char *zAccount;           /* Account name (if any) */
  const char *zContainer;         /* Name of container in cloud */
  const char *zLocalDir;          /* Local directory used by container */

  Manifest *pMan;                 /* Current container manifest */
  int nClient;                    /* Current number of clients */
  Container *pNext;               /* Next container on same VFS */

  int nBcv;
  BcvWrapper *aBcv;
  int eState;                     /* CONTAINER_STATE_* constant */

  u8 aKey[BCV_LOCAL_KEYSIZE];     /* Encryption key to use */
  BcvEncryptionKey *pKey;         /* Compiled encryption key to use */
  int iEnc;                       /* Encryption id (or 0 for no encryption) */
};

struct BcvWrapper {
  BcvContainer *pBcv;
  BcvDispatch *pDisp;
  int bUsable;
};

#define CONTAINER_STATE_NONE          0
#define CONTAINER_STATE_UPLOAD        1
#define CONTAINER_STATE_COPY          2
#define CONTAINER_STATE_POLL          3
#define CONTAINER_STATE_DELETE        4

/*
** Each blocks in the cache-file is represented by an instance of the 
** following type.
**
** iEnc:
**   For an in-process VFS, or an insecure container in a daemon process,
**   this field is set to 0. Otherwise, it is an integer that identifies
**   to the daemon which encryption key has been used to encrypt it. This
**   is necessary in case the same container is attached twice with different
**   encryption schemes, or if somebody copies a cloud storage container -
**   creating a separate container containing the same block ids.
**   
*/
struct CacheEntry {
  int iPos;                       /* Position of block in cache file */
  int nRef;                       /* Current number of client refs */
  int iEnc;                       /* Encryption index, if any */
  i64 iLruTick;                   /* Logic clock when last added to LRU list */
  CacheEntry *pHashNext;          /* Hash collision list */
  CacheEntry *pLruPrev;           /* Previous entry in LRU list */
  CacheEntry *pLruNext;           /* Next entry in LRU list */
  u8 bValid;                      /* True if cache entry is valid */
  u8 bDirty;                      /* True if cache entry is a dirty block */
  u8 nName;                       /* Size of aName[] in bytes */
  u8 aName[BCV_MAX_NAMEBYTES];    /* Block identifier */
};

struct BcvBuffer {
  u8 *aData;
  int nData;
  int nAlloc;
};


/*
** A ManifestHash object is a hash-table containing all block-ids from all
** databases from a single Manifest object (block-ids on the delete-list
** are not part of the hash table).
*/
struct ManifestHash {
  int nHash;
  u8 **aHash;
};

int bcvMHashBuild(Manifest*, int, ManifestDb*, ManifestHash**);
u8 *bcvMHashQuery(ManifestHash *pHash, u8 *aBlk, int nBlk);
void bcvMHashFree(ManifestHash *pHash);


void bcvBufferAppendU32(int *pRc, BcvBuffer *p, u32 iVal);
void bcvBufferMsgString(int *pRc, BcvBuffer *p, const char *zStr);
void bcvBufferMsgBlob(int *pRc, BcvBuffer *p, const u8 *aData, int nData);
void bcvBufferZero(BcvBuffer *p);
void bcvBufferAppendRc(int *pRc, BcvBuffer *p, const void *a, int n);

int bcvStrcmp(const char *zLeft, const char *zRight);
int bcvStrlen(const char *z);

char *bcvMprintf(const char *zFmt, ...);
char *bcvMprintfRc(int *pRc, const char *zFmt, ...);
void *bcvMalloc(i64 nByte);
void *bcvRealloc(void *a, i64 nByte);
void *bcvMallocZero(i64 nByte);
u8 *bcvMemdup(int nIn, const u8 *aIn);
char *bcvStrdup(const char *zIn);

void *bcvMallocRc(int *pRc, i64 nByte);
char *bcvStrdupRc(int *pRc, const char *z);

i64 sqlite_timestamp(void);

int bcv_isdigit(char c);
int bcv_isspace(char c);
int bcvParseInt(const u8 *z, int n);

u32 bcvGetU16(const u8 *a);
void bcvPutU32(u8 *a, u32 iVal);
u32 bcvGetU32(const u8 *a);
void bcvPutU64(u8 *a, u64 iVal);
u64 bcvGetU64(const u8 *a);

int bcvManifestParse(u8 *a, int n, char *zETag, Manifest **ppOut, char **pz);
int bcvManifestParseCopy(
  const u8 *a, int n, 
  const char *zETag, 
  Manifest **ppOut, 
  char **pz
);

u8 *bcvManifestCompose(Manifest *p, int *pnOut);
void bcvManifestFree(Manifest *p);
void bcvManifestDeref(Manifest *p);
Manifest *bcvManifestRef(Manifest *p);
int bcvManifestDup(Manifest *p, Manifest **ppNew);
void bcvManifestExpand(int*, Manifest**, int);

void bcvBlockidToText(Manifest *p, const u8 *pBlk, char *aBuf);

int bcvWritefile(sqlite3_file *pFd, const u8 *aData, int nData, i64 iOff);
int bcvOpenLocal(const char *, int bWal, int bReadonly, sqlite3_file **ppFd);
void bcvCloseLocal(sqlite3_file *pFd);
int bcvReadfile(sqlite3_file *pFd, void *aBuf, int iAmt, sqlite3_int64 iOff);

sqlite3 *bcvOpenAndInitDb(int *pRc, const char *zDir, char **pzErr);
char *bcvGetFullpath(int *pRc, const char *zDir);
int bcvfsCacheInit(BcvCommon *p, sqlite3 *db, char *zFullDir);
Container *bcvContainerAlloc(
  int*, BcvCommon*, 
  const char *zStorage, 
  const char *zAccount, 
  const char *zContainer, 
  const char *zAlias
);
Container *bcvfsFindContAlias(BcvCommon*, const char*, char **pzErr);
void bcvCommonDestroy(BcvCommon *p);
void bcvContainerFree(Container *pCont);
int bcvContainerDetachAndFree(BcvCommon *p, Container *pCont);
int bcvManifestInstall(BcvCommon *p, Container *pCont, Manifest *pMan);
int bcvCountCached(BcvCommon*, Manifest*, ManifestDb*, int*);
ManifestDb *bcvfsFindDatabase(Manifest *pMan, const char *zDb, int nDb);
CacheEntry *bcvfsHashFind(BcvCommon *p, const u8 *pBlk, int nBlk);
CacheEntry *bcvfsAllocCacheEntry(int *pRc, BcvCommon *p);
void bcvfsHashAdd(BcvCommon *p, CacheEntry *pEntry);
void bcvfsHashRemove(BcvCommon *p, CacheEntry *pEntry);
void bcvfsLruRemoveIf(BcvCommon *p, CacheEntry *pEntry);
void bcvfsEntryUnref(BcvCommon *p, CacheEntry *pEntry);
void bcvfsBlockidToText(const u8 *pBlk, int nBlk, char *aBuf);
ManifestDb *bcvManifestDbidToDb(Manifest *p, i64 iDbId);
void bcvfsLruAdd(BcvCommon *p, CacheEntry *pEntry);
u8 *bcvDatabaseVtabData(
    int*, BcvCommon*, const char*, const char*, const char*, u32, int*
);
int bcvManifestUpdate(BcvCommon*, Container*, Manifest*, char**);
void bcvExecPrintf(int *pRc, sqlite3 *db, const char *zFmt, ...);
void bcvfsUnusedAdd(BcvCommon *p, CacheEntry *pEntry);
void bcvfsLruAddIf(BcvCommon *p, CacheEntry *pEntry);
int bcvfsNameToBlockid(Manifest *p, const char *zName, u8 *aBlk);
u8 *bcvEmptyKV(int *pRc, int *pnData);

/*
** Below here should be eventually moved back to blockcachevfsd. 
*/
void fatal_oom_error(void);

void hex_encode(const unsigned char *aIn, int nIn, char *aBuf, int bUpper);
int hex_decode(const char *aIn, int nIn, u8 *aBuf);

int bcvInstallBuiltinModules(void);

/************************************************************************/

struct BcvContainer {
  sqlite3_bcv_container *pCont;
  sqlite3_bcv_module *pMod;
  int nContRef;
};

int bcvDispatchNew(BcvDispatch**);
int bcvDispatchRun(BcvDispatch*, struct curl_waitfd *aFd, int nFd, int ms);
int bcvDispatchRunAll(BcvDispatch*);
void bcvDispatchFree(BcvDispatch*);
void bcvDispatchVerbose(BcvDispatch*, int);
void bcvDispatchTimeout(BcvDispatch*, int);
void bcvDispatchLog(BcvDispatch*, void*, void (*xLog)(void*,int,const char*));

int bcvDispatchFetch(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zETag,
  const void *pMd5,
  void *pApp,
  void (*x)(void*, int rc, char *zETag, const u8 *aData, int nData)
);

int bcvDispatchPut(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zIfMatch,
  const u8 *aData, int nData,
  void *pApp,
  void (*x)(void*, int rc, char *zETag)
);

int bcvDispatchDelete(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zIfMatch,
  void *pApp,
  void (*x)(void*, int rc, char *zErr)
);

int bcvDispatchCreate(
  BcvDispatch *p,
  BcvContainer *pCont,
  void *pApp,
  void (*x)(void*, int rc, char *zError)
);

int bcvDispatchDestroy(
  BcvDispatch *p,
  BcvContainer *pCont,
  void *pApp,
  void (*x)(void*, int rc, char *zError)
);

int bcvDispatchList(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zPrefix,
  void *pApp,
  void (*x)(void*, int rc, char *zError)
);

int bcvDispatchLogmsg(BcvDispatch *p, const char *zFmt, ...);

int bcvCreateModuleUnsafe(const char*, sqlite3_bcv_module*, void*);

int bcvContainerOpen(
  const char *zMod,
  const char *zUser,
  const char *zAuth,
  const char *zCont,
  BcvContainer **ppCont,          /* OUT: New container object (if no error */
  char **pzErr                    /* OUT: error message (if any) */
);
void bcvContainerClose(BcvContainer*);

void bcvDispatchFail(BcvDispatch*, BcvContainer*, int, const char*);

int bcv_socket_is_valid(BCV_SOCKET_TYPE s);
void bcv_close_socket(BCV_SOCKET_TYPE s);
void bcv_socket_init();

void sqlite3_bcv_test_socket_api(
  int (*xRecv)(BCV_SOCKET_TYPE, void *, int),
  int (*xSend)(BCV_SOCKET_TYPE, void *, int),
  int (**pxRecv)(BCV_SOCKET_TYPE, void *, int),
  int (**pxSend)(BCV_SOCKET_TYPE, void *, int)
);

typedef struct BcvMessage BcvMessage;

typedef struct BcvAttachMsg BcvAttachMsg;
typedef struct BcvCmdMsg BcvCmdMsg;
typedef struct BcvDetachMsg BcvDetachMsg;
typedef struct BcvEndMsg BcvEndMsg;
typedef struct BcvHelloMsg BcvHelloMsg;
typedef struct BcvPassMsg BcvPassMsg;
typedef struct BcvReadMsg BcvReadMsg;
typedef struct BcvVtabMsg BcvVtabMsg;
typedef struct BcvReply BcvReply;
typedef struct BcvHelloReply BcvHelloReply;
typedef struct BcvPassReply BcvPassReply;
typedef struct BcvReadReply BcvReadReply;
typedef struct BcvVtabReply BcvVtabReply;
typedef struct BcvPrefetchMsg BcvPrefetchMsg;
typedef struct BcvPrefetchReply BcvPrefetchReply;

struct BcvHelloMsg {
  const char *zContainer;
  const char *zDatabase;
};

struct BcvHelloReply {
  u32 errCode;                    /* Error code */
  const char *zErrMsg;            /* Error message */

  const char *zStorage;           /* Storage system in use for db */
  const char *zAccount;           /* Account uses by db */
  const char *zContainer;         /* Cloud container name (not alias) for db */
  i64 iDbId;                      /* Database id */
  u32 szBlk;                      /* Block size in bytes */
  u32 bEncrypted;                 /* True for encrypted databases */
};

struct BcvPassMsg {
  const char *zAuth;
};

struct BcvPassReply {
  u32 errCode;                    /* Error code */
  const char *zErrMsg;            /* Error message */
  u8 *aKey;                       /* BCV_LOCAL_KEYSIZE bytes of key data */
};

struct BcvAttachMsg {
  const char *zStorage;
  const char *zAccount;
  const char *zContainer;
  const char *zAlias;
  const char *zAuth;
  u32 flags;                      /* sqlite3_bcvfs_attach() flags param */
};

struct BcvReply {
  u32 errCode;                    /* Error code */
  int szBlk;                      /* Daemon block size, if available */
  const char *zErrMsg;            /* Error message */
};

struct BcvVtabMsg {
  const char *zVtab;
  const char *zContainer;
  const char *zDatabase;
  u32 colUsed;
};

struct BcvVtabReply {
  int nData;
  const u8 *aData;
};

struct BcvDetachMsg {
  const char *zName;
};

struct BcvReadMsg {
  u32 iBlk;                       /* Requested block */
  const char *zAuth;              /* Authentication string */
  u32 nMru;                       /* Number of entries in aMru[] */
  u32 *aMru;                      /* Most recently used array */
};

struct BcvReadReply {
  u32 errCode;                    /* Error code */
  const char *zErrMsg;            /* Error message */
  u32 nBlk;                       /* Number of entries in aBlk[] */
  u32 *aBlk;                      /* Block -> cache entry map */
};

struct BcvEndMsg {
  u32 nMru;                       /* Number of entries in aMru[] */
  u32 *aMru;                      /* Most recently used array */
};

struct BcvCmdMsg { 
  const char *zAuth;
  u32 eCmd;
};

struct BcvPrefetchMsg {
  const char *zAuth;
  u32 nRequest;
  u32 nMs;
};
struct BcvPrefetchReply {
  u32 errCode;                    /* Error code */
  const char *zErrMsg;            /* Error message */
  u32 nOnDemand;
  u32 nOutstanding;
};

/*
** Candidate values for BcvCmdMsg.eCmd
*/
#define BCV_CMD_POLL   2

struct BcvMessage {
  int eType;                      /* BCV_MESSAGE_* value */
  union {
    BcvAttachMsg attach;          /* BCV_MESSAGE_ATTACH */
    BcvDetachMsg detach;          /* BCV_MESSAGE_DETACH */
    BcvHelloMsg hello;            /* BCV_MESSAGE_HELLO */
    BcvHelloReply hello_r;        /* BCV_MESSAGE_HELLO_REPLY */
    BcvReply error_r;             /* BCV_MESSAGE_REPLY */
    BcvVtabMsg vtab;              /* BCV_MESSAGE_VTAB */
    BcvVtabReply vtab_r;          /* BCV_MESSAGE_VTAB_REPLY */
    BcvReadMsg read;              /* BCV_MESSAGE_READ */
    BcvReadReply read_r;          /* BCV_MESSAGE_READ_REPLY */
    BcvEndMsg end;                /* BCV_MESSAGE_END */
    BcvCmdMsg cmd;                /* BCV_MESSAGE_CMD */
    BcvPassMsg pass;              /* BCV_MESSAGE_PASS */
    BcvPassReply pass_r;          /* BCV_MESSAGE_PASS_REPLY */
    BcvPrefetchMsg prefetch;      /* BCV_MESSAGE_PREFETCH */
    BcvPrefetchReply prefetch_r;  /* BCV_MESSAGE_PREFETCH_REPLY */
  } u;
};


int bcvRecvMsg(BCV_SOCKET_TYPE fd, BcvMessage **ppMsg);
int bcvSendMsg(BCV_SOCKET_TYPE fd, BcvMessage *pMsg);

/* 
** Valid values for BcvMessage.
*/
#define BCV_MESSAGE_HELLO          0x01      /* c->d   BcvHelloMsg */
#define BCV_MESSAGE_HELLO_REPLY    0x02      /* d->c   BcvHelloReply */
#define BCV_MESSAGE_REPLY          0x03      /* d->c   BcvReply */
#define BCV_MESSAGE_ATTACH         0x04      /* d->c   BcvAttachMsg */
#define BCV_MESSAGE_VTAB           0x05      /* c->d   BcvVtabMsg */
#define BCV_MESSAGE_VTAB_REPLY     0x06      /* d->c   BcvVtabMsg */
#define BCV_MESSAGE_DETACH         0x07      /* c->d   BcvDetachMsg */
#define BCV_MESSAGE_READ           0x08      /* c->d   BcvReadMsg */
#define BCV_MESSAGE_READ_REPLY     0x09      /* d->c   BcvReadReply */
#define BCV_MESSAGE_END            0x0A      /* c->d   BcvEndMsg */
#define BCV_MESSAGE_CMD            0x0B      /* c->d   BcvCmdMsg */
#define BCV_MESSAGE_PASS           0x0C      /* c->d   BcvPassMsg */
#define BCV_MESSAGE_PASS_REPLY     0x0D      /* d->c   BcvPassReply */
#define BCV_MESSAGE_PREFETCH       0x0E      /* c->d   BcvPrefetchMsg */
#define BCV_MESSAGE_PREFETCH_REPLY 0x0F      /* d->c   BcvPrefetchReply */

BcvEncryptionKey *bcvEncryptionKeyNew(const u8 *aKey);
void bcvEncryptionKeyFree(BcvEncryptionKey*);
BcvEncryptionKey *bcvEncryptionKeyRef(BcvEncryptionKey*);

int bcvDecrypt(BcvEncryptionKey*, sqlite3_int64, u8*, u8*, int);
int bcvEncrypt(BcvEncryptionKey*, sqlite3_int64, u8*, u8*, int);


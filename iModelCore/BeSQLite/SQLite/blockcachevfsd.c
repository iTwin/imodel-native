/*
** 2019 October 31
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains the code for the blockcachevfs daemon - blockcachevfsd.
**
** Usage:
**
**   blockcachevfsd upload   ?SWITCHES? DBFILE
**   blockcachevfsd download ?SWITCHES? DBFILE
*/

/*
** MANIFEST FILE FORMAT VERSION 1
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
**     Then, for each database:
**         Database version:       32-bit integer.
**         Database offset:        32-bit integer.
**         Number of blocks in db: 32-bit integer.
**         Database unique id:     16 bytes
**         Database display name:  128 byte utf-8. (zero padded)
**
** Part 2: Delete Block list.
**
**   An array of block ids. Each block id is a 128-bit blob consisting of:
**
**     Random block id value:  name-size byte value
**   
**   And a 64-bit timestamp, identifying when the block should be deleted.
**   The 64-bit timestamp is GMT time in SQLite's xCurrentTimeInt64() format.
**
** Part 3: Database Block lists.
**
**   There is a database block list for each database identified in the
**   manifest header. Each block list begins at the offset identified in
**   the manifest header. There is no "number-of-entries" field, the number
**   of entries in each block list is defined by the block size and size
**   of the db file as specified in the manifest header. The block list is
**   a list of name-size byte block ids as specified in section 2.
*/

/*
** UPLOADING A NEW MANIFEST FILE
*/

/*
** GARBAGE COLLECTION NOTES
**
** In this context, "garbage collection" means "how and when are blocks that
** are not in use by any database deleted from the cloud storage?".
**
** There are two types of blocks that can be deleted:
**
**     1. Those on the delete-list in the manifest file. These may be safely
**        deleted at any time - the associated timestamps allow any node 
**        to decide exactly when.
**
**     2. Those that are present in the container, but are (a) not used 
**        by any database, (b) not on the delete-list and (c) have not just
**        been uploaded by a node that is about to update the manifest
**        to include them.
**
** Case 2 is the tricky one of course. These files can be created if a
** node uploads them and then fails before either updating the manifest
** or deleting them (because it cannot commit due to a write conflict).
** This should be fairly uncommon.
**
** These blocks are never deleted directly - they must first be added to
** the delete-list in the manifest file and then deleted. A garbage-collector 
** node attempting to clean up case 2 blocks uses the following approach:
**
**     1. Download a manifest file.
**
**     2. Download a list of all blocks, along with their mtime 
**        timestamps.
**
**     3. Add each block with an mtime more than T seconds old that 
**        is not part of the manifest file (either as a database block
**        or the delete-list) to the delete-list in the manifest.
**
**     4. Upload the new manifest file, using an If-Match header as
**        described above.
**
** T should be set to a very large value - perhaps as much as 4 hours.
** Since these blocks may only be created by process, network or node
** failure, there should not be too many of them.
*/

/*
** CLIENT/DAEMON PROTOCOL
**
** Database clients connect to this this daemon via a localhost socket
** using a custom wire protocol. All messages begin with a 5 byte header:
**
**     Message type (1 byte)
**     Message size in bytes (4 byte big-endian integer)
**
** The message size field value includes the 5 byte header.
**
** Login message:
**
**   Sent by client as soon as it connects. Format is:
**
**     Message type (1 byte):  0x4C (ascii 'L')
**     Message size (4 byte big-endian integer): Message size in bytes
**     Database id (variable length, nul-term): $container/$database
**
**   Reply is:
**
**     Message type (1 byte):  0x6C (ascii 'l')
**     Message size (4 byte big-endian integer): Message size in bytes (5)
**     Block size (4 byte big-endian integer)
**
** Block request message:
**
**   Sent by client to request a block 
**
**     Message type (1 byte):  0x4C (ascii 'R')
**     Message size (4 byte big-endian integer): Message size in bytes
**     Block number (4 byte big-endian integer): Blocks are numbered from 0
**     Bitmask array indicating used blocks.
**
**   Reply is:
**
**     Message type (1 byte):  0x4C (ascii 'r')
**     Message size (4 byte big-endian integer): Message size in bytes
**     Array of block numbers (4 byte big-endian integers): One for each
**         block in the database.
**
** Transaction done message:
**
**   Sent by client to request a block 
**
**     Message type (1 byte):  0x4C (ascii 'D')
**     Message size (4 byte big-endian integer): Message size in bytes
**     Bitmask array indicating used blocks.
**
** Write-Request message:
**     Message type (1 byte): 'Q'
**     Message size (4 byte big-endian integer): Message size in bytes
**     Array of 32-bit big-endian integers - the required blocks.
**
** Write-Request Reply message:
**     Message type (1 byte): 'q'
**     Message size (4 byte big-endian integer): Message size in bytes
**     Array of 32-bit big-endian integers - the block offsets within
**     the cache file of each requested block.
**
** Alternatively, the daemon may reply to any client message with an 'error'
** message. Of the following form:
**
**     Message type (1 byte):  0x6C (ascii 'e')
**     Message size (4 byte big-endian integer): Message size in bytes (5)
**     Error message: utf-8 text.
*/


/*
** Contents of blocksdb.bcv database.
*/
#define BCV_DATABASE_SCHEMA \
  "CREATE TABLE IF NOT EXISTS block("  \
      "cachefilepos INTEGER PRIMARY KEY, blockid BLOB, dbpos INTEGER," \
      "container TEXT, db TEXT, dbversion INTEGER" \
  ");" \
  "CREATE TABLE IF NOT EXISTS manifest(" \
      "account, container, manifest BLOB, etag, " \
      "PRIMARY KEY(account, container)" \
  ");"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
// #include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include "bcv_socket.h"

#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md5.h>


#if defined(__WIN32__)
#define strerror_r strerror_s
#define sleep Sleep
#endif

#ifdef __WIN32__
# define osMkdir(x,y) mkdir(x)
#else
# define osMkdir(x,y) mkdir(x,y)
# define O_BINARY 0
#endif

#include "sqlite3.h"
#include "simplexml.h"

/* Default values of various command line parameters. */
#define BCV_DEFAULT_NAMEBYTES        16
#define BCV_DEFAULT_CACHEFILE_SIZE   (1024*1024*1024)
#define BCV_DEFAULT_NWRITE           10
#define BCV_DEFAULT_NDELETE          10
#define BCV_DEFAULT_POLLTIME         10
#define BCV_DEFAULT_DELETETIME       3600
#define BCV_DEFAULT_GCTIME           3600
#define BCV_DEFAULT_RETRYTIME        10
#define BCV_DEFAULT_BLOCKSIZE        (4*1024*1024)

/* Current version of manifest file format */
#define BCV_MANIFEST_VERSION 3

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

#define BCV_DATEBUFFER_SIZE          128

#define BCV_AZURE_VERSION_HDR  "x-ms-version:2015-02-21"
#define BCV_AZURE_BLOBTYPE_HDR "x-ms-blob-type:BlockBlob"
#define BCV_AZURE_MIMETYPE_HDR "Content-Type:application/octet-stream"

#define BCV_MANIFEST_FILE            "manifest.bcv"

#define BCV_HTTP_OK       200
#define BCV_HTTP_NOTFOUND 404

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

#define BCV_CACHEFILE_NAME "cachefile.bcv"
#define BCV_DATABASE_NAME  "blocksdb.bcv"

/*
** Message protocol message types.
*/
#define BCV_MESSAGE_LOGIN          'L'
#define BCV_MESSAGE_LOGIN_REPLY    'l'

#define BCV_MESSAGE_REQUEST        'R'
#define BCV_MESSAGE_REQUEST_REPLY  'r'
#define BCV_MESSAGE_DONE           'D'

#define BCV_MESSAGE_WREQUEST       'Q'
#define BCV_MESSAGE_WREQUEST_REPLY 'q'
#define BCV_MESSAGE_WDONE          'C'

#define BCV_MESSAGE_UPLOAD         'U'
#define BCV_MESSAGE_UPLOAD_REPLY   'u'

#define BCV_MESSAGE_QUERY          'Z'
#define BCV_MESSAGE_QUERY_REPLY    'z'

#define BCV_MESSAGE_ERROR_REPLY    'e'


#define MIN(a,b)  ((a)<(b) ? (a) : (b))
#define MAX(a,b)  ((a)>(b) ? (a) : (b))

typedef unsigned int u32;
typedef unsigned char u8;
typedef sqlite3_uint64 u64;
typedef sqlite3_int64 i64;

typedef struct CommandSwitches CommandSwitches;
struct CommandSwitches {
  char *zContainer;               /* -container ARG */
  char *zAccessKey;               /* -accesskey ARG */
  char *zAccount;                 /* -account ARG */
  char *zDirectory;               /* -directory ARG */
  char *zEmulator;                /* Emulator URI */
  int iPort;                      /* -port ARG */
  int bVerbose;                   /* True if -verbose is specified */
  int bAutoexit;                  /* True if -autoexit is specified */
  i64 szCache;                    /* Size of cache file in bytes */
  i64 szBlk;                      /* Block size */
  int nNamebytes;                 /* -namebytes INT (used by "create") */
  int nPollTime;
  int nDeleteTime;
  int nGCTime;
  int nRetryTime;

  int mLog;                       /* Mask of daemon events to log */
  int bNodelete;                  /* Disable deleting blocks in daemon */
  int bClobber;                   /* Have "create" clobber existing manifest */
  int bReadyMessage;              /* Daemon outputs "ready" message */
  int bPersistent;                /* Have the cache persist through restarts */

  int nWrite;                     /* Integer value of -nwrite option */
  int nDelete;                    /* Integer value of -deletes option */
};

#define BCV_LOG_MESSAGE 0x01
#define BCV_LOG_EVENT   0x02
#define BCV_LOG_POLL    0x04
#define BCV_LOG_UPLOAD  0x08
#define BCV_LOG_CACHE   0x10

typedef struct AccessPoint AccessPoint;
struct AccessPoint {
  const char *zAccount;
  const char *zAccessKey;
  const char *zEmulator;
  char *zSlashAccountSlash;
  u8 *aKey;                       /* zAccessKey decoded from base64 */
  int nKey;                       /* Size of buffer aKey[] in bytes */
};

typedef struct CurlRequest CurlRequest;
struct CurlRequest {
  CURL *pCurl;
  struct curl_slist *pList;
  int bVerbose;
  char *zStatus;
  char *zETag;
};

typedef struct MemoryDownload MemoryDownload;
struct MemoryDownload {
  CurlRequest *pCurl;
  long iRes;                      /* HTTP response code */
  int nByte;                      /* Bytes of content in a[] */
  int nAlloc;                     /* Number of bytes allocated a[] */
  u8 *a;                          /* Buffer containing download */
};

/*
** Structure used as context for a curl CURLOPT_READFUNCTION and
** CURLOPT_SEEKFUNCTION when uploading a blob that is present in main
** memory (usually a manifest file).
*/
typedef struct MemoryUpload MemoryUpload;
struct MemoryUpload {
  int iOff;                       /* Current offset within a[] */
  int nByte;                      /* Bytes of content in a[] */
  u8 *a;                          /* Buffer containing data */
};

/*
** Structure used as context for a curl CURLOPT_READFUNCTION and
** CURLOPT_SEEKFUNCTION when uploading a blob that is a part of a
** file on disk.
*/
typedef struct FileUpload FileUpload;
struct FileUpload {
  i64 iStart;                     /* Offset of part of file to upload */
  int iOff;                       /* Current offset in file */
  int nByte;                      /* Total bytes of data to read from disk */
  int szUpload;                   /* Total bytes of data to upload */
  int fd;                         /* File descriptor open on file */
};

typedef struct FileDownload FileDownload;
struct FileDownload {
  int fd;                         /* File descriptor to write data to */
  i64 iStart;                     /* Starting offset in file fd */
  int iOff;                       /* Current offset in block */
};

typedef struct ObjectName ObjectName;
struct ObjectName {
  char *zName;
  ObjectName *pNext;
};

typedef struct ClistParse ClistParse;
struct ClistParse {
  int bExists;
  const char *zExists;
  ObjectName *pList;
};

typedef struct FilesParse FilesParse;
typedef struct FilesParseEntry FilesParseEntry;

struct FilesParseEntry {
  char *zName;
  char *zModified;
  FilesParseEntry *pNext;
};
struct FilesParse {
  FilesParseEntry *pList;
  char *zNextMarker;
};

typedef struct Manifest Manifest;
typedef struct ManifestDb ManifestDb;
struct Manifest {
  int nRef;                       /* Number of pointers to this object */
  int nDb;                        /* Number of databases */
  int szBlk;                      /* Block size */
  int nNamebytes;                 /* Bytes in each block name */
  ManifestDb *aDb;                /* Array of nDb databases */

  int bDelFree;                   /* If true, aDelBlk must be freed */
  int nDelBlk;                    /* Number of blocks in delete array */
  u8 *aDelBlk;                    /* Array of delete blocks */

  char *zETag;                    /* e-tag value for this manifest */
  u8 *pFree;                      /* Blob of data to free */
};

/*
** A ManifestHash object is a hash-table containing all block-ids from all
** databases from a single Manifest object (block-ids on the delete-list
** are not part of the hash table).
*/
typedef struct ManifestHash ManifestHash;
struct ManifestHash {
  int nHash;                      /* Number of hash buckets */
  u8 *aHash[0];                   /* Array of nHash hash buckets */
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
  u8 aId[BCV_DBID_SIZE];          /* Database id */
  char zDName[BCV_DBNAME_SIZE];   /* Database display name */
  int iVersion;                   /* Database version */

  /* Original block array (as it exists or existed in cloud storage. */
  u8 *aBlkOrig;
  int nBlkOrig;
  int bBlkOrigFree;

  /* Current block array */
  u8 *aBlkLocal;
  int nBlkLocal;
  int nBlkLocalAlloc;
};

/*
** A printf() style function that outputs a message on stderr, then calls
** exit(-1). This is used for command line errors and other fatal conditions
*/
static void fatal_error(const char *zFmt, ...){
  char *zMsg;
  va_list ap;
  va_start(ap, zFmt);
  zMsg = sqlite3_vmprintf(zFmt, ap);
  fprintf(stderr, "FATAL: %s\n", zMsg);
  sqlite3_free(zMsg);
  va_end(ap);
  exit(-1);
}

static void fatal_oom_error(void){
  fprintf(stderr, "FATAL: out-of-memory error\n");
  exit(-1);
}

static void fatal_sql_error(sqlite3 *db, const char *zMsg){
  fprintf(stderr, "FATAL: SQL error in %s (rc=%d) (errmsg=%s)\n", 
      zMsg, sqlite3_errcode(db), sqlite3_errmsg(db)
  );
  exit(-1);
}

/*
** Wrapper around sqlite3_mprintf() that calls fatal_oom_error() if an 
** OOM occurs.
*/
static char *bcvMprintf(const char *zFmt, ...){
  char *zRet;
  va_list ap;
  va_start(ap, zFmt);
  zRet = sqlite3_vmprintf(zFmt, ap);
  va_end(ap);
  if( zRet==0 ) fatal_oom_error();
  return zRet;
}

static void *bcvMalloc(int nByte){
  void *p = sqlite3_malloc(nByte);
  assert( nByte!=0 );
  if( p==0 ) fatal_oom_error();
  return p;
}

static void *bcvRealloc(void *a, int nByte){
  void *p = sqlite3_realloc(a, nByte);
  assert( nByte!=0 );
  if( p==0 ) fatal_oom_error();
  return p;
}

static void *bcvMallocZero(int nByte){
  void *p = bcvMalloc(nByte);
  memset(p, 0, nByte);
  return p;
}

static char *bcvStrdup(const char *zIn){
  char *zNew = 0;
  if( zIn ){
    int nIn = strlen(zIn);
    zNew = (char*)bcvMalloc(nIn+1);
    memcpy(zNew, zIn, nIn+1);
  }
  return zNew;
}

/*
** Return a duplicate of the nIn byte buffer at aIn[]. It is the 
** responsibility of the caller to eventually free the returned buffer
** by passing a pointer to it to sqlite3_free().
*/
static u8 *bcvMemdup(int nIn, const u8 *aIn){
  u8 *aRet = bcvMalloc(nIn+1);
  memcpy(aRet, aIn, nIn);
  return aRet;
}

static int bcvStrcmp(const char *zLeft, const char *zRight){
  if( zLeft==zRight ){
    return 0;
  }else if( zLeft==0 ){
    return -1;
  }else if( zRight==0 ){
    return +1;
  }
  return strcmp(zLeft, zRight);
}

static void fatal_system_error(const char *zApi, int iError){
  char *zBuf = bcvMalloc(1000);
  strerror_r(iError, zBuf, 1000);
  zBuf[999] = '\0';
  fatal_error("%s - %s", zApi, zBuf);
}

static i64 sqlite_timestamp(void){
  i64 iRet = 0;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  pVfs->xCurrentTimeInt64(pVfs, &iRet);
  return iRet;
}

static i64 compute_timestamp(
  int iYear,                      /* Year */
  int iMonth,                     /* Month (jan=1, dec=12) */
  int iDay,                       /* Day of month */
  int iHour,                      /* Hour of day */
  int iMin,                       /* Minute of hour */
  int iSecond                     /* Hour of day */
){
  i64 iJD;                        /* Return value */
  int Y, M, D, A, B, X1, X2;

  assert( iMonth>=1 && iMonth<=12 );
  assert( iDay>=1 && iDay<=31 );
  assert( iHour>=0 && iHour<=23 );
  assert( iMin>=0 && iMin<=59 );
  assert( iSecond>=0 && iSecond<=59 );

  Y = iYear;
  M = iMonth;
  D = iDay;

  if( M<=2 ){
    Y--;
    M += 12;
  }
  A = Y/100;
  B = 2 - A + (A/4);
  X1 = 36525*(Y+4716)/100;
  X2 = 306001*(M+1)/10000;
  iJD = (sqlite3_int64)((X1 + X2 + D + B - 1524.5 ) * 86400000);

  iJD += iHour*3600000 + iMin*60000 + iSecond*1000;
  return iJD;
}

static int bcv_isdigit(char c){
  return c>='0' && c<='9';
}
static int bcv_isspace(char c){
  return (c==' ' || c=='\n' || c=='\r' || c=='\t');
}

static int parse_number(const char *zNum, int *piVal){
  int i = 0;
  int iVal = 0;
  for(i=0; zNum[i]; i++){
    char c = zNum[i];
    if( !bcv_isdigit(c) ) break;
    iVal = iVal*10 + (c-'0');
  }
  *piVal = iVal;
  return i;
}
  

/*
** This function parses a GMT timestamp in the following format:
**
**    "Sun, 06 Nov 1994 08:49:37 GMT"
**
** and returns the equivalent time in SQLite integer timestamp format - the
** julian day multiplied by 86400000 (the number of ms in a day).
*/
static i64 parse_timestamp(const char *zTime){
  const char *azMonth[] = {
    0, "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  int iDay, iMonth, iYear;
  int iHour, iMin, iSec;
  int iCur;

  for(iCur=0; zTime[iCur] && !bcv_isdigit(zTime[iCur]); iCur++);
  iCur += parse_number(&zTime[iCur], &iDay);
  if( zTime[iCur++]!=' ' ) goto parse_failed;
  for(iMonth=1; iMonth<=12; iMonth++){
    if( memcmp(&zTime[iCur], azMonth[iMonth], 3)==0 ) break;
  }
  if( iMonth>12 ) goto parse_failed;
  iCur+=3;
  if( zTime[iCur++]!=' ' ) goto parse_failed;
  iCur += parse_number(&zTime[iCur], &iYear);
  if( zTime[iCur++]!=' ' ) goto parse_failed;
  iCur += parse_number(&zTime[iCur], &iHour);
  if( zTime[iCur++]!=':' ) goto parse_failed;
  iCur += parse_number(&zTime[iCur], &iMin);
  if( zTime[iCur++]!=':' ) goto parse_failed;
  iCur += parse_number(&zTime[iCur], &iSec);

  return compute_timestamp(iYear, iMonth, iDay, iHour, iMin, iSec);

 parse_failed:
  fatal_error("failed to parse timestamp: %s", zTime);
  return 0;
}

/* 
** Primitives to read and write 32-bit big-endian integers. 
*/
static u32 bcvGetU16(const u8 *a){
  return ((u32)a[0] << 8) + ((u32)a[1] << 0);
}
static void bcvPutU32(u8 *a, u32 iVal){
  a[0] = (iVal>>24) & 0xFF;
  a[1] = (iVal>>16) & 0xFF;
  a[2] = (iVal>> 8) & 0xFF;
  a[3] = (iVal>> 0) & 0xFF;
}
static u32 bcvGetU32(const u8 *a){
  return ((u32)a[0] << 24) + ((u32)a[1] << 16) 
       + ((u32)a[2] << 8) + ((u32)a[3] << 0);
}
static void bcvPutU64(u8 *a, u64 iVal){
  a[0] = (iVal>>56) & 0xFF;
  a[1] = (iVal>>48) & 0xFF;
  a[2] = (iVal>>40) & 0xFF;
  a[3] = (iVal>>32) & 0xFF;
  a[4] = (iVal>>24) & 0xFF;
  a[5] = (iVal>>16) & 0xFF;
  a[6] = (iVal>> 8) & 0xFF;
  a[7] = (iVal>> 0) & 0xFF;
}

static u64 bcvGetU64(const u8 *a){
  return ((u64)a[0] << 56)
       + ((u64)a[1] << 48)
       + ((u64)a[2] << 40)
       + ((u64)a[3] << 32)
       + ((u64)a[4] << 24)
       + ((u64)a[5] << 16)
       + ((u64)a[6] <<  8)
       + ((u64)a[7] <<  0);
}

static size_t curlHeaderFunction(char *buf, size_t sz, size_t n, void *pUser){
  CurlRequest *p = (CurlRequest*)pUser;
  int nByte = (sz * n);
  if( p->zStatus==0 ){
    p->zStatus = bcvMprintf("%.*s", nByte-1, buf);
  }else if( nByte>6 && sqlite3_strnicmp(buf, "ETag: ", 6)==0 ){
    p->zETag = bcvMprintf("%.*s", nByte-8, &buf[6]);
  }
  return (size_t)nByte;
}

static void curlRequestInit(CurlRequest *p, int bVerbose){
  memset(p, 0, sizeof(CurlRequest));
  p->pCurl = curl_easy_init();
  if( bVerbose ){
    p->bVerbose = 1;
    curl_easy_setopt(p->pCurl, CURLOPT_VERBOSE, 1L);
  }
  curl_easy_setopt(p->pCurl, CURLOPT_HEADERFUNCTION, curlHeaderFunction);
  curl_easy_setopt(p->pCurl, CURLOPT_HEADERDATA, (void*)p);
}

static void curlRequestReset(CurlRequest *p){
  curl_easy_reset(p->pCurl);
  curl_slist_free_all(p->pList);
  p->pList = 0;
  if( p->bVerbose ){
    curl_easy_setopt(p->pCurl, CURLOPT_VERBOSE, 1L);
  }
  curl_easy_setopt(p->pCurl, CURLOPT_HEADERFUNCTION, curlHeaderFunction);
  curl_easy_setopt(p->pCurl, CURLOPT_HEADERDATA, (void*)p);
  sqlite3_free(p->zStatus);
  sqlite3_free(p->zETag);
  p->zStatus = 0;
  p->zETag = 0;
}

static void curlRequestFinalize(CurlRequest *p){
  curl_easy_cleanup(p->pCurl);
  curl_slist_free_all(p->pList);
  sqlite3_free(p->zStatus);
  sqlite3_free(p->zETag);
  memset(p, 0, sizeof(CurlRequest));
}

static const char aBase64Encode[] = 
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "abcdefghijklmnopqrstuvwxyz"
                       "0123456789+/";
static u8 aBase64Decode[256];

static void base64_init(void){
  int i;
  memset(aBase64Decode, 0x80, sizeof(aBase64Decode));
  for(i=0; i<sizeof(aBase64Encode); i++){
    aBase64Decode[(int)aBase64Encode[i]] = (char)i;
  }
  aBase64Decode[(int)'='] = 0;
}

/*
** The first argument passed to this function is a blob of data encoded using 
** base64 encoding, with no embedded spaces or newlines. This function
** decodes the base64 and returns a pointer to a buffer containing the
** same binary data. It is the responsibility of the caller to
** free the returned buffer using sqlite3_free(). Before returning (*pnOut)
** is set to the number of bytes in the returned buffer.
*/
static u8 *base64_decode(const char *zBase64, int *pnOut){
  unsigned char *aOut = 0;
  const unsigned char *zIn = (const unsigned char*)zBase64;
  int nIn = strlen((const char*)zIn);
  if( nIn>0 && (nIn%4)==0 ){
    int iOut = 0;
    int i;

    /* Figure out the length in bytes of the output */
    int nOut = (nIn/4) * 3;
    if( zBase64[nIn-1]=='=' ) nOut--;
    if( zBase64[nIn-2]=='=' ) nOut--;

    /* Allocate space for the output */
    aOut = (u8*)bcvMalloc(nOut);

    for(i=0; i<nIn; i+=4){

      if( aBase64Decode[ zIn[i+0] ]==0x80 
       || aBase64Decode[ zIn[i+1] ]==0x80 
       || aBase64Decode[ zIn[i+2] ]==0x80 
       || aBase64Decode[ zIn[i+3] ]==0x80 
      ){
        fatal_error("cannot decode base64 data - illegal character");
      }else{
        u32 v = (aBase64Decode[ zIn[i+0] ] << 18) +
                (aBase64Decode[ zIn[i+1] ] << 12) +
                (aBase64Decode[ zIn[i+2] ] <<  6) +
                (aBase64Decode[ zIn[i+3] ] <<  0);
        aOut[iOut++] = (v>>16) & 0xFF;
        if( iOut<nOut ) aOut[iOut++] = (v>>8) & 0xFF;
        if( iOut<nOut ) aOut[iOut++] = (v>>0) & 0xFF;
      }
    }

    assert( iOut==nOut );
    *pnOut = iOut;
  }else{
    fatal_error("cannot decode base64 data - length is %d bytes", nIn);
  }

  return aOut;
}

/*
** Encode the binary data in buffer aIn[] (size nIn bytes) as a
** nul-terminated base64 string. Return a pointer to the buffer. It is
** the responsibility of the caller to eventually free the returned 
** buffer using sqlite3_free().
*/
static char *base64_encode(const unsigned char *aIn, int nIn){
  int i;
  int nOut = ((nIn + 2) / 3) * 4;
  char *zOut = sqlite3_malloc(nOut+1);
  int iOut = 0;

  for(i=0; i<nIn; i+=3){
    u32 v = aIn[i] << 16;
    if( i+1<nIn ) v |= aIn[i+1] << 8;
    if( i+2<nIn ) v |= aIn[i+2] << 0;
    zOut[iOut++] = aBase64Encode[(v >> 18) & 0x3F];
    zOut[iOut++] = aBase64Encode[(v >> 12) & 0x3F];
    zOut[iOut++] = aBase64Encode[(v >>  6) & 0x3F];
    zOut[iOut++] = aBase64Encode[(v >>  0) & 0x3F];
  }
  assert( iOut==nOut );

  zOut[iOut] = '\0';
  if( (nIn % 3)==1 ){
    assert( zOut[iOut-2]=='A' && zOut[iOut-1]=='A' );
    zOut[iOut-2] = '=';
    zOut[iOut-1] = '=';
  }
  if( (nIn % 3)==2 ){
    assert( zOut[iOut-1]=='A' );
    zOut[iOut-1] = '=';
  }

  return zOut;
}

/*
** Hex encode the blob passed via the only two arguments to this function
** into buffer aBuf[].
*/
static void hex_encode(const unsigned char *aIn, int nIn, char *aBuf){
  static const char aDigit[] = "0123456789ABCDEF";
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
static int hex_decode(const char *aIn, int nIn, u8 *aBuf){
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
    if( a>=sizeof(aCharMap) || aCharMap[a]<0 ) return 1;
    if( b>=sizeof(aCharMap) || aCharMap[b]<0 ) return 1;
    aBuf[i/2] = (aCharMap[a]<<4) + aCharMap[b];
  }

  return 0;
}

typedef struct SelectOption SelectOption;
struct SelectOption {
  const char *zOpt;
  u32 mask;
  int eVal;
};

static int select_option(
  const char *zOpt, 
  SelectOption *aOpt,
  u32 mask,
  int bUnknownFatal               /* If true, unknown option is a fatal error */
){
  int nOpt = strlen(zOpt);
  int i;
  int nMatch = 0;
  int iRet = -1;
  char *zMsg = 0;
  int nCandidate = 0;

  for(i=0; aOpt[i].zOpt; i++){
    if( aOpt[i].mask & mask ){
      int n = strlen(aOpt[i].zOpt);
      if( nOpt && n>=nOpt && 0==memcmp(zOpt, aOpt[i].zOpt, nOpt) ){
        nMatch++;
        iRet = i;
      }
      nCandidate++;
    }
  }

  if( nMatch==1 ) return iRet;
  if( nMatch==0 ){
    int iCand = 0;
    if( bUnknownFatal==0 && zOpt[0]!='-' ) return -1;
    zMsg = sqlite3_mprintf("unknown option \"%s\". Expected ", zOpt);
    for(i=0; aOpt[i].zOpt; i++){
      if( aOpt[i].mask & mask ){
        iCand++;
        if( iCand<nCandidate ){
          zMsg = sqlite3_mprintf("%z%s%s", zMsg, (i==0?"":", "), aOpt[i].zOpt);
        }else{
          zMsg = sqlite3_mprintf("%z or %s", zMsg, aOpt[i].zOpt);
        }
      }
    }
  }else{
    int iCand = 0;
    int bFirst = 1;
    zMsg = sqlite3_mprintf("ambiguous option \"%s\" - could be ", zOpt);
    for(i=0; aOpt[i].zOpt; i++){
      if( aOpt[i].mask & mask ){
        int n = strlen(aOpt[i].zOpt);
        if( n>=nOpt && 0==memcmp(zOpt, aOpt[i].zOpt, nOpt) ){
          iCand++;
          if( iCand==nMatch ){
            zMsg = sqlite3_mprintf("%z or %s", zMsg, aOpt[i].zOpt);
          }else{
            zMsg = sqlite3_mprintf("%z%s%s",zMsg,(bFirst?"":", "),aOpt[i].zOpt);
            bFirst = 0;
          }
        }
      }
    }
  }

  fatal_error("%s", zMsg);
  return 0;
}

#define COMMANDLINE_CONTAINER    0x0001
#define COMMANDLINE_ACCESSKEY    0x0002
#define COMMANDLINE_ACCOUNT      0x0004
#define COMMANDLINE_DIRECTORY    0x0008
#define COMMANDLINE_PORT         0x0010
#define COMMANDLINE_VERBOSE      0x0020
#define COMMANDLINE_CACHESIZE    0x0040
#define COMMANDLINE_AUTOEXIT     0x0080
#define COMMANDLINE_BLOCKSIZE    0x0100

#define COMMANDLINE_POLLTIME     0x0200
#define COMMANDLINE_DELETETIME   0x0400
#define COMMANDLINE_GCTIME       0x0800
#define COMMANDLINE_RETRYTIME    0x1000
 
#define COMMANDLINE_LOG          0x2000

#define COMMANDLINE_NODELETE     0x4000
#define COMMANDLINE_EMULATOR     0x8000
#define COMMANDLINE_CLOBBER     0x10000
#define COMMANDLINE_NWRITE      0x20000
#define COMMANDLINE_NDELETE     0x40000
#define COMMANDLINE_READYMSG    0x80000
#define COMMANDLINE_NAMEBYTES  0x100000
#define COMMANDLINE_PERSISTENT 0x200000


/*
** Parse the nul-terminated string indicated by argument zSize as a size
** parameter. A size parameter consists of any number of digits optionally
** followed by 'K', 'M' or 'G'. If successful, the size in bytes is returned.
** Otherwise, a fatal error is raised and the function does not return.
*/
static i64 parse_size(const char *zSize){
  i64 ret = 0;
  int i;

  for(i=0; zSize[i]; i++){
    if( zSize[i]<'0' || zSize[i]>'9' ) break;
    ret = ret*10 + (zSize[i]-'0');
  }

  switch( zSize[i] ){
    case 'k': case 'K':
      ret = ret * 1024;
      i++;
      break;
    case 'm': case 'M':
      ret = ret * 1024*1024;
      i++;
      break;
    case 'g': case 'G':
      ret = ret * 1024*1024*1024;
      i++;
      break;
  }

  if( zSize[i] ){
    fatal_error("parse error in size: %s", zSize);
  }

  return ret;
}

static int parse_seconds(const char *zSeconds){
  i64 ret = 0;
  int i;

  for(i=0; zSeconds[i]; i++){
    if( zSeconds[i]<'0' || zSeconds[i]>'9' ){
      fatal_error("parse error in seconds: %s", zSeconds);
    }
    ret = ret*10 + (zSeconds[i]-'0');
    if( ret>0x7fffffff ){
      fatal_error(
          "parse error in seconds (max value is %d): %s", 0x7fffffff, zSeconds
      );
    }
  }

  return (int)ret;
}

static int parse_integer(const char *zInt){
  i64 ret = 0;
  int i;

  for(i=0; zInt[i]; i++){
    if( zInt[i]<'0' || zInt[i]>'9' ){
      fatal_error("parse error in integer: %s", zInt);
    }
    ret = ret*10 + (zInt[i]-'0');
    if( ret>0x7fffffff ){
      fatal_error(
          "parse error in seconds (max value is %d): %s", 0x7fffffff, zInt
      );
    }
  }

  return (int)ret;
}

#define COMMANDLINE_ALLMASK 0xFFFFFFFF

static void parse_more_switches(
  CommandSwitches *pCmd, 
  const char **azArg, 
  int nArg, 
  int *piFail,
  u32 mask
){
  int i;
  SelectOption aSwitch[] = {
    { "-file",       COMMANDLINE_ALLMASK    , 0},
    { "-delay",      COMMANDLINE_ALLMASK    , -1},
    { "-accesskey",  COMMANDLINE_ACCESSKEY  , COMMANDLINE_ACCESSKEY },
    { "-container",  COMMANDLINE_CONTAINER  , COMMANDLINE_CONTAINER },
    { "-account",    COMMANDLINE_ACCOUNT    , COMMANDLINE_ACCOUNT },
    { "-directory",  COMMANDLINE_DIRECTORY  , COMMANDLINE_DIRECTORY },
    { "-port",       COMMANDLINE_PORT       , COMMANDLINE_PORT },
    { "-verbose",    COMMANDLINE_VERBOSE    , COMMANDLINE_VERBOSE },
    { "-cachesize",  COMMANDLINE_CACHESIZE  , COMMANDLINE_CACHESIZE },
    { "-autoexit",   COMMANDLINE_AUTOEXIT   , COMMANDLINE_AUTOEXIT },
    { "-blocksize",  COMMANDLINE_BLOCKSIZE  , COMMANDLINE_BLOCKSIZE },
    { "-polltime",   COMMANDLINE_POLLTIME   , COMMANDLINE_POLLTIME },
    { "-deletetime", COMMANDLINE_DELETETIME , COMMANDLINE_DELETETIME },
    { "-gctime",     COMMANDLINE_GCTIME     , COMMANDLINE_GCTIME },
    { "-retrytime",  COMMANDLINE_RETRYTIME  , COMMANDLINE_RETRYTIME },
    { "-log",        COMMANDLINE_LOG        , COMMANDLINE_LOG },
    { "-nodelete",   COMMANDLINE_NODELETE   , COMMANDLINE_NODELETE },
    { "-emulator",   COMMANDLINE_EMULATOR   , COMMANDLINE_EMULATOR },
    { "-clobber",    COMMANDLINE_CLOBBER    , COMMANDLINE_CLOBBER  },
    { "-nwrite",     COMMANDLINE_NWRITE     , COMMANDLINE_NWRITE  },
    { "-ndelete",    COMMANDLINE_NDELETE    , COMMANDLINE_NDELETE  },
    { "-readymessage",COMMANDLINE_READYMSG  , COMMANDLINE_READYMSG  },
    { "-namebytes",  COMMANDLINE_NAMEBYTES  , COMMANDLINE_NAMEBYTES  },
    { "-persistent", COMMANDLINE_PERSISTENT , COMMANDLINE_PERSISTENT  },
    { 0, 0 }
  };

  if( piFail ){
    *piFail = nArg;
  }

  for(i=0; i<nArg; i++){
    int iVal = select_option(azArg[i], aSwitch, mask, (piFail ? 0 : 1));
    if( iVal<0 ){
      *piFail = i;
      return;
    }

    switch( aSwitch[iVal].eVal){
      case -1:
      case 0:
      case COMMANDLINE_CONTAINER:
      case COMMANDLINE_ACCESSKEY:
      case COMMANDLINE_ACCOUNT:
      case COMMANDLINE_DIRECTORY:
      case COMMANDLINE_PORT:
      case COMMANDLINE_CACHESIZE:
      case COMMANDLINE_BLOCKSIZE:
      case COMMANDLINE_POLLTIME:
      case COMMANDLINE_DELETETIME:
      case COMMANDLINE_GCTIME:
      case COMMANDLINE_RETRYTIME:
      case COMMANDLINE_LOG:
      case COMMANDLINE_EMULATOR:
      case COMMANDLINE_NWRITE:
      case COMMANDLINE_NDELETE:
      case COMMANDLINE_NAMEBYTES:
        i++;
        if( i==nArg ){
          fatal_error("option requires an argument: %s\n", azArg[i-1]);
        }
        break;
      default:
        break;
    }

    switch( aSwitch[iVal].eVal ){
      case -1: {
        int n = parse_seconds(azArg[i]);
        sleep((unsigned int)n);
        break;
      }
      case 0: {
        const char *zFile = azArg[i];
        const char *azParse[256];
        int nParse;
        int fd;
        struct stat buf;          /* Used to fstat() database file */
        char *aBuf;
        size_t nRead;
        char *p;

        /* Read the contents of file into a buffer obtained from
        ** sqlite3_malloc(). Append a nul-terminator to the buffer. */
        fd = open(zFile, O_RDONLY);
        if( fd<0 ){
          fatal_error("failed to open file %s", azArg[i]);
        }
        fstat(fd, &buf);
        aBuf = bcvMallocZero(buf.st_size+1);
        nRead = read(fd, aBuf, buf.st_size);
        close(fd);
        if( nRead!=buf.st_size ){
          fatal_error("error reading file %s", azArg[i]);
        }

        memset(azParse, 0, sizeof(azParse));
        nParse = 0;
        p = aBuf;
        while( nParse<sizeof(azParse)/sizeof(azParse[0]) ){
          while( bcv_isspace(*p) ){
            *p = '\0';
            p++;
          }
          if( *p=='\0' ) break;
          azParse[nParse++] = p;
          while( !bcv_isspace(*p) ) p++;
        }
        parse_more_switches(pCmd, azParse, nParse, 0, mask);
        sqlite3_free(aBuf);
        break;
      }
      case COMMANDLINE_CONTAINER:
        pCmd->zContainer = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_ACCESSKEY:
        pCmd->zAccessKey = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_ACCOUNT:
        pCmd->zAccount = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_DIRECTORY:
        pCmd->zDirectory = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_PORT:
        pCmd->iPort = atoi(azArg[i]);
        break;
      case COMMANDLINE_CACHESIZE:
        pCmd->szCache = parse_size(azArg[i]);
        break;
      case COMMANDLINE_BLOCKSIZE:
        pCmd->szBlk = parse_size(azArg[i]);
        break;
      case COMMANDLINE_VERBOSE:
        pCmd->bVerbose = 1;
        break;
      case COMMANDLINE_AUTOEXIT:
        pCmd->bAutoexit = 1;
        break;
      case COMMANDLINE_PERSISTENT:
        pCmd->bPersistent = 1;
        break;
      case COMMANDLINE_POLLTIME:
        pCmd->nPollTime = parse_seconds(azArg[i]);
        break;
      case COMMANDLINE_DELETETIME:
        pCmd->nDeleteTime = parse_seconds(azArg[i]);
        break;
      case COMMANDLINE_GCTIME:
        pCmd->nGCTime = parse_seconds(azArg[i]);
        break;
      case COMMANDLINE_RETRYTIME:
        pCmd->nRetryTime = parse_seconds(azArg[i]);
        break;
      case COMMANDLINE_NAMEBYTES:
        pCmd->nNamebytes = parse_integer(azArg[i]);
        if( pCmd->nNamebytes>BCV_MAX_NAMEBYTES 
         || pCmd->nNamebytes<BCV_MIN_NAMEBYTES
        ){
          fatal_error("value for -namebytes out of range "
              "(should be %d <= value <= %d)",
              BCV_MIN_NAMEBYTES, BCV_MAX_NAMEBYTES
          );
        }
        break;
      case COMMANDLINE_LOG: {
        int ii;
        const char *zArg = azArg[i];
        for(ii=0; zArg[ii]; ii++){
          switch( zArg[ii] ){
            case 'm':
              pCmd->mLog |= BCV_LOG_MESSAGE;
              break;
            case 'e':
              pCmd->mLog |= BCV_LOG_EVENT;
              break;
            case 'p':
              pCmd->mLog |= BCV_LOG_POLL;
              break;
            case 'u':
              pCmd->mLog |= BCV_LOG_UPLOAD;
              break;
            case 'c':
              pCmd->mLog |= BCV_LOG_CACHE;
              break;
            default:
              fatal_error(
                  "unexpected -log flag '%c', expected 'm' or 'e'", zArg[ii]
              );

          }
        }
        break;
      case COMMANDLINE_NODELETE:
        pCmd->bNodelete = 1;
        break;
      case COMMANDLINE_EMULATOR:
        /* The following credentials are the only ones accepted by the 
        ** official Azure storage emulator app. They're published as part of
        ** Azure documentation. So the access-key being included here need not
        ** alarm anybody - it's not a secret.  */
        pCmd->zEmulator = bcvStrdup(azArg[i]);
        pCmd->zAccount = bcvStrdup("devstoreaccount1");
        pCmd->zAccessKey = bcvStrdup(
            "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuF"
            "q2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="
        );
        break;
      }
      case COMMANDLINE_CLOBBER:
        pCmd->bClobber = 1;
        break;
      case COMMANDLINE_READYMSG:
        pCmd->bReadyMessage = 1;
        break;
      case COMMANDLINE_NWRITE:
        pCmd->nWrite = parse_integer(azArg[i]);
        break;
      case COMMANDLINE_NDELETE:
        pCmd->nDelete = parse_integer(azArg[i]);
        break;
      default:
        break;
    }
  }
}

static void parse_switches(
  CommandSwitches *pCmd, 
  const char **azArg, 
  int nArg, 
  int *piFail,
  u32 mask
){
  memset(pCmd, 0, sizeof(CommandSwitches));
  return parse_more_switches(pCmd, azArg, nArg, piFail, mask);
}

static void free_parse_switches(CommandSwitches *pCmd){
  sqlite3_free(pCmd->zContainer);
  sqlite3_free(pCmd->zAccessKey);
  sqlite3_free(pCmd->zAccount);
  sqlite3_free(pCmd->zDirectory);
  sqlite3_free(pCmd->zEmulator);
  memset(pCmd, 0, sizeof(CommandSwitches));
}

static void missing_required_switch(const char *zSwitch){
  fprintf(stderr, "missing required switch: %s\n", zSwitch);
  exit(-1);
}

static void azure_date_header(char *zBuffer){
  static const char *nameOfDay[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };

  static const char *nameOfMonth[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  time_t t = time(NULL);
  struct tm tm = *gmtime(&t);

  sprintf(zBuffer, "x-ms-date:%s, %02d %s %d %02d:%02d:%02d GMT",
      nameOfDay[tm.tm_wday],          // Day of week name
      tm.tm_mday,                     // Day
      nameOfMonth[tm.tm_mon],         // Month name
      tm.tm_year + 1900,              // Year
      tm.tm_hour,                     // Hour
      tm.tm_min,                      // Minute
      tm.tm_sec);                     // Second
}

static char *azure_auth_header(AccessPoint *pA, const char *zStringToSign){
  unsigned char aDigest[SHA256_DIGEST_LENGTH];
  u32 nDigest = SHA256_DIGEST_LENGTH;
  HMAC_CTX *pHmac;
  char *zSig = 0;
  char *zAuth = 0;
  
  pHmac = HMAC_CTX_new();
  HMAC_CTX_reset(pHmac);
  HMAC_Init_ex(pHmac, pA->aKey, pA->nKey, EVP_sha256(), NULL);
  HMAC_Update(pHmac, (const u8*)zStringToSign, strlen(zStringToSign));
  HMAC_Final(pHmac, aDigest, &nDigest);
  HMAC_CTX_free(pHmac);

  zSig = base64_encode(aDigest, (int)nDigest);
  zAuth = sqlite3_mprintf("Authorization: SharedKey %s:%s", pA->zAccount, zSig);

  sqlite3_free(zSig);
  return zAuth;
}

static void bcvAccessPointNew(
  AccessPoint *p, 
  const char *zAccount, 
  const char *zAccessKey,
  const char *zEmulator
){
  memset(p, 0, sizeof(AccessPoint));
  p->zAccount = zAccount;
  p->zAccessKey = zAccessKey;
  p->zEmulator = zEmulator;
  p->aKey = base64_decode(zAccessKey, &p->nKey);
  if( zEmulator ){
    p->zSlashAccountSlash = bcvMprintf("/%s/%s/", zAccount, zAccount);
  }else{
    p->zSlashAccountSlash = bcvMprintf("/%s/", zAccount);
  }
}

static void bcvAccessPointFree(AccessPoint *p){
  sqlite3_free(p->zSlashAccountSlash);
  sqlite3_free(p->aKey);
}

#define STR_(x) #x
#define STR(x) STR_(x)

static char *azure_base_uri(AccessPoint *p){
  char *zRet;
  if( p->zEmulator ){
    zRet = bcvMprintf("http://%s/%s/", p->zEmulator, p->zAccount);
  }else{
    zRet = bcvMprintf("https://%s.blob.core.windows.net/", p->zAccount);
  }
  return zRet;
}

size_t uploadReadFunction(char *pBuffer, size_t n1, size_t n2, void *pFd){
  int fd = *(int*)pFd;
  size_t nRead;
  nRead = read(fd, pBuffer, n1*n2);
  return nRead;
}
size_t uploadSeekFunction(void *pFd, curl_off_t offset, int origin){
  int fd = *(int*)pFd;
  lseek(fd, offset, origin);
  return CURL_SEEKFUNC_OK;
}

/*
** The following two functions are used as CURLOPT_READFUNCTION and
** CURLOPT_SEEKFUNCTION callbacks when uploading a blob that is in 
** main memory (a manifest file). The context pointer points to an
** instance of type MemoryUpload.
*/
static size_t memoryUploadRead(char *pBuffer, size_t n1, size_t n2, void *pCtx){
  int nReq;
  MemoryUpload *p = (MemoryUpload*)pCtx;
  nReq = MIN(n1*n2, p->nByte-p->iOff);
  memcpy(pBuffer, &p->a[p->iOff], nReq);
  p->iOff += nReq;
  return nReq;
}
static size_t memoryUploadSeek(void *pCtx, curl_off_t offset, int origin){
  MemoryUpload *p = (MemoryUpload*)pCtx;
  p->iOff = (int)offset;
  return CURL_SEEKFUNC_OK;
}

/*
** Configure the CurlRequest object indicated by the first argument to 
** download file zFile from container zContainer, using the specified 
** account and shared access-key.
**
** This function does not configure a function to deal with the fetched
** data. Doing so before the request is sent is the responsibility of
** the caller.
*/
static void curlFetchBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zBase = 0;                /* Base URI for all requests */
  char *zRes = 0;                 /* Canonicalized resource */
  char *zAuth = 0;                /* Authorization header */
  char *zStringToSign = 0;        /* String to sign for auth header */
  char zDate[BCV_DATEBUFFER_SIZE];

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s/%s", zBase, zContainer, zFile);
  zRes = bcvMprintf("%s%s/%s", pA->zSlashAccountSlash, zContainer, zFile);

  zStringToSign = bcvMprintf(
      "GET\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s", 
      zDate, BCV_AZURE_VERSION_HDR, zRes
  );
  zAuth = azure_auth_header(pA, zStringToSign);

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zRes);
  sqlite3_free(zAuth);
}

static void curlPutBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile,
  const char *zETag,              /* If-Match tag (or NULL) */
  int nByte                       /* Size of file in bytes */
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zBase = 0;                /* Base URI for all requests */
  char *zAuth = 0;                /* Authorization header */
  char *zStringToSign = 0;        /* String to sign for auth header */
  char zDate[BCV_DATEBUFFER_SIZE];

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s/%s", zBase, zContainer, zFile);

  zStringToSign = bcvMprintf(
      "PUT\n\n\n%d\n\n%s\n\n\n%s\n\n\n\n%s\n%s\n%s\n%s%s/%s", 
      nByte, 
      "application/octet-stream", 
      zETag ? zETag : "",
      BCV_AZURE_BLOBTYPE_HDR, 
      zDate, BCV_AZURE_VERSION_HDR,
      pA->zSlashAccountSlash,
      zContainer, zFile
  );
  zAuth = azure_auth_header(pA, zStringToSign);

  if( zETag ){
    char *zHdr = bcvMprintf("If-Match:%s", zETag);
    p->pList = curl_slist_append(p->pList, zHdr);
    sqlite3_free(zHdr);
  }

  p->pList = curl_slist_append(p->pList, BCV_AZURE_BLOBTYPE_HDR);
  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_MIMETYPE_HDR);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)nByte);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
}

static void curlCreateContainer(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zBase = 0;                /* Base URI for all requests */
  char *zAuth = 0;                /* Authorization header */
  char *zStringToSign = 0;        /* String to sign for auth header */
  char zDate[BCV_DATEBUFFER_SIZE];

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s?restype=container", zBase, zContainer);

  zStringToSign = bcvMprintf(
      "PUT\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s\nrestype:container", 
      zDate, BCV_AZURE_VERSION_HDR,
      pA->zSlashAccountSlash,
      zContainer
  );
  zAuth = azure_auth_header(pA, zStringToSign);

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
}

static size_t bdIgnore(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  return nSize*nMember;
}

static void curlDeleteBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zBase = 0;                /* Base URI for all requests */
  char *zAuth = 0;                /* Authorization header */
  char *zStringToSign = 0;        /* String to sign for auth header */
  char zDate[BCV_DATEBUFFER_SIZE];

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s/%s", zBase, zContainer, zFile);

  zStringToSign = bcvMprintf(
      "DELETE\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s/%s", 
      zDate, BCV_AZURE_VERSION_HDR,
      pA->zSlashAccountSlash,
      zContainer, zFile
  );
  zAuth = azure_auth_header(pA, zStringToSign);

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);
  curl_easy_setopt(p->pCurl, CURLOPT_WRITEFUNCTION, bdIgnore);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
}

static void curlDestroyContainer(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zBase = 0;                /* Base URI for all requests */
  char *zAuth = 0;                /* Authorization header */
  char *zStringToSign = 0;        /* String to sign for auth header */
  char zDate[BCV_DATEBUFFER_SIZE];

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s?restype=container", zBase, zContainer);

  zStringToSign = bcvMprintf(
      "DELETE\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s\nrestype:container", 
      zDate, BCV_AZURE_VERSION_HDR,
      pA->zSlashAccountSlash,
      zContainer
  );
  zAuth = azure_auth_header(pA, zStringToSign);

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
}

static void curlListFiles(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zNextMarker
){
  static const int nMaxResults = 200;
  char *zUrl = 0;                 /* URL to fetch */
  char *zBase = 0;                /* Base URI for all requests */
  char *zRes = 0;                 /* Canonicalized resource */
  char *zAuth = 0;                /* Authorization header */
  char *zStringToSign = 0;        /* String to sign for auth header */
  char zDate[BCV_DATEBUFFER_SIZE];/* HTTP date header */

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s?restype=container&comp=list&maxresults=%d"
      "&marker=%s", 
      zBase, zContainer, nMaxResults, zNextMarker
  );
  zRes = bcvMprintf("%s%s", pA->zSlashAccountSlash, zContainer);

  zStringToSign = bcvMprintf(
      "GET\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s\ncomp:list"
      "\nmarker:%s\nmaxresults:%d\nrestype:container",
      zDate, BCV_AZURE_VERSION_HDR, zRes, zNextMarker, nMaxResults
  );
  zAuth = azure_auth_header(pA, zStringToSign);

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zRes);
  sqlite3_free(zAuth);
}

static void curlListContainers(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zPrefix
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zBase = 0;                /* Base URI for all requests */
  char *zRes = 0;                 /* Canonicalized resource */
  char *zAuth = 0;                /* Authorization header */
  char *zStringToSign = 0;        /* String to sign for auth header */
  char zDate[BCV_DATEBUFFER_SIZE];/* HTTP date header */

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s?comp=list%s%s", 
      zBase, zPrefix?"&prefix=":"", zPrefix?zPrefix:""
  );
  zRes = bcvMprintf("%s", pA->zSlashAccountSlash);

  zStringToSign = bcvMprintf(
      "GET\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s\ncomp:list%s%s",
      zDate, BCV_AZURE_VERSION_HDR, zRes,
      zPrefix ? "\nprefix:" : "",
      zPrefix ? zPrefix : ""
  );
  zAuth = azure_auth_header(pA, zStringToSign);

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zRes);
  sqlite3_free(zAuth);
}

static void curlFatalIfNot(
  CurlRequest *p, 
  CURLcode res, 
  int code, 
  const char *zPre
){
  long httpcode;
  if( res!=CURLE_OK ){
    fatal_error("%s: %s\n", zPre, curl_easy_strerror(res));
  }
  curl_easy_getinfo(p->pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
  if( httpcode!=code ){
    fatal_error("%s (%d) - %s", zPre, httpcode, p->zStatus);
  }
}

static size_t memoryDownloadWrite(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  MemoryDownload *p = (MemoryDownload*)pCtx;
  size_t sz = (nSize * nMember);

  if( p->a==0 ){
    curl_easy_getinfo(p->pCurl->pCurl, CURLINFO_RESPONSE_CODE, &p->iRes);
    p->nAlloc = MAX(1024, sz+1);
    p->a = bcvMalloc( p->nAlloc );
  }else{
    while( p->nAlloc<(p->nByte+sz+1) ){
      p->nAlloc = p->nAlloc*2;
    }
    p->a = bcvRealloc(p->a, p->nAlloc);
  }

  memcpy(&p->a[p->nByte], pData, sz);
  p->nByte += sz;
  p->a[p->nByte] = '\0';

  return sz;
}

static size_t fileDownloadWrite(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  FileDownload *p = (FileDownload*)pCtx;
  size_t sz = (nSize * nMember);
  size_t res;

  lseek(p->fd, p->iStart+p->iOff, SEEK_SET);
  res = write(p->fd, pData, sz);
  p->iOff += sz;

  return res;
}

static void *clistNameHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_CONTENT ){
    ClistParse *p = simpleXmlGetUserData(parser);
    int n = strlen(szValue);
    if( p->zExists==0 ){
      ObjectName *pName;
      pName = (ObjectName*)bcvMalloc(sizeof(ObjectName) + n + 1);
      pName->zName = (char*)&pName[1];
      memcpy(pName->zName, szValue, n+1);
      pName->pNext = p->pList;
      p->pList = pName;
    }else if( 0==bcvStrcmp(szValue, p->zExists) ){
      p->bExists = 1;
    }
  }
  return clistNameHandler;
}
static void *clistContainerHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_SUBTAG ){
    if( 0==bcvStrcmp("Name", szName) ){
      return clistNameHandler;
    }
    return 0;
  }
  return clistContainerHandler;
}
static void *clistTagHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_SUBTAG && 0==bcvStrcmp("Container", szName) ){
    return clistContainerHandler;
  }
  return clistTagHandler;
}

static void bcvListContainers(
  CurlRequest *pCurl, 
  ClistParse *pParse,
  AccessPoint *pA,
  const char *zContainer
){
  MemoryDownload md;
  CURLcode res;

  curlListContainers(pCurl, pA, zContainer);

  memset(&md, 0, sizeof(MemoryDownload));
  md.pCurl = pCurl;
  curl_easy_setopt(pCurl->pCurl, CURLOPT_WRITEFUNCTION, memoryDownloadWrite);
  curl_easy_setopt(pCurl->pCurl, CURLOPT_WRITEDATA, (void*)&md);

  res = curl_easy_perform(pCurl->pCurl);
  if( res!=CURLE_OK ){
    fatal_error("clist failed: %s\n", curl_easy_strerror(res));
  }

  if( md.iRes==200 ){
    SimpleXmlParser x;
    x = simpleXmlCreateParser((const char*)md.a, md.nByte);
    simpleXmlPushUserData(x, (void*)pParse);
    simpleXmlParse(x, clistTagHandler);
    simpleXmlDestroyParser(x);
  }else{
    fatal_error("clist failed - %d %s", md.iRes, (char*)md.a);
  }
  sqlite3_free(md.a);
}

static void *bcvParseFilesModifiedHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_CONTENT ){
    FilesParse *p = simpleXmlGetUserData(parser);
    int n = strlen(szValue);
    char *zBuf = (char*)bcvMalloc(n+1);
    memcpy(zBuf, szValue, n+1);
    p->pList->zModified = zBuf;
  }
  return bcvParseFilesModifiedHandler;
}

static void *bcvParseFilesNameHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_CONTENT ){
    FilesParse *p = simpleXmlGetUserData(parser);
    int n = strlen(szValue);
    FilesParseEntry *pNew;
    pNew = (FilesParseEntry*)bcvMallocZero(sizeof(FilesParseEntry) + n + 1);
    pNew->zName = (char*)&pNew[1];
    memcpy(pNew->zName, szValue, n+1);
    pNew->pNext = p->pList;
    p->pList = pNew;
  }
  return bcvParseFilesNameHandler;
}

static void *bcvParseFilesMarkerHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_CONTENT ){
    FilesParse *p = simpleXmlGetUserData(parser);
    if( p->zNextMarker==0 ){
      p->zNextMarker = bcvStrdup(szValue);
    }
  }
  return bcvParseFilesMarkerHandler;
}

static void *bcvParseFilesTagHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_SUBTAG ){
    if( 0==bcvStrcmp("Name", szName) ){
      return bcvParseFilesNameHandler;
    }else
    if( 0==bcvStrcmp("Last-Modified", szName) ){
      return bcvParseFilesModifiedHandler;
    }else
    if( 0==bcvStrcmp("NextMarker", szName) ){
      return bcvParseFilesMarkerHandler;
    }
  }
  return bcvParseFilesTagHandler;
}

static void bcvParseFiles(
  const u8 *aBuf, int nBuf,
  FilesParse *pParse
){
  SimpleXmlParser x;
  sqlite3_free(pParse->zNextMarker);
  pParse->zNextMarker = 0;
  x = simpleXmlCreateParser((const char*)aBuf, nBuf);
  simpleXmlPushUserData(x, (void*)pParse);
  simpleXmlParse(x, bcvParseFilesTagHandler);
  simpleXmlDestroyParser(x);
}

static void bcvParseFilesClear(FilesParse *pParse){
  FilesParseEntry *p;
  FilesParseEntry *pNext;
  for(p=pParse->pList; p; p=pNext){
    pNext = p->pNext;
    sqlite3_free(p->zModified);
    sqlite3_free(p);
  }
  sqlite3_free(pParse->zNextMarker);
  memset(pParse, 0, sizeof(FilesParse));
}


static int manifestBlockSize(const u8 *a, int n){
  return (int)bcvGetU32(&a[4]);
}
static int manifestDatabaseCount(const u8 *a, int n){
  return (int)bcvGetU32(&a[8]);
}
static int manifestGCBlkCount(const u8 *a, int n){
  return (int)bcvGetU32(&a[12]);
}
static int manifestNamebytes(const u8 *a, int n){
  return (int)bcvGetU32(&a[16]);
}

static char *manifestDatabaseId(const u8 *a, int n, int iDb){
  int iOff = BCV_MANIFEST_HEADER_BYTES 
           + iDb*BCV_MANIFEST_DBHEADER_BYTES
           + 12;
  return (char*)&a[iOff];
}
static char *manifestDatabaseName(const u8 *a, int n, int iDb){
  int iOff = BCV_MANIFEST_HEADER_BYTES 
           + iDb*BCV_MANIFEST_DBHEADER_BYTES
           + 12 + BCV_DBID_SIZE; 
  return (char*)&a[iOff];
}
static int manifestBlockCount(const u8 *a, int n, int iDb){
  return (int)bcvGetU32(&a[
    BCV_MANIFEST_HEADER_BYTES + iDb*BCV_MANIFEST_DBHEADER_BYTES + 8
  ]);
}
static int manifestDatabaseVersion(const u8 *a, int n, int iDb){
  return bcvGetU32(
      &a[BCV_MANIFEST_HEADER_BYTES + iDb*BCV_MANIFEST_DBHEADER_BYTES]
  );
}

static u8 *manifestBlockArray(u8 *a, int n, int iDb){
  int iOff = (int)bcvGetU32(&a[
      BCV_MANIFEST_HEADER_BYTES + iDb*BCV_MANIFEST_DBHEADER_BYTES + 4
  ]);
  return &a[iOff];
}
static u8 *manifestGCBlkArray(u8 *a, int n){
  int nDb = manifestDatabaseCount(a, n);
  return &a[BCV_MANIFEST_HEADER_BYTES + nDb*BCV_MANIFEST_DBHEADER_BYTES];
}

/*
** Parse the manifest in buffer a[], size n bytes, and return a Manifest
** structure with ref-count set to 1 reflecting the results.
*/
static Manifest *bcvManifestParse(u8 *a, int n, char *zETag){
  Manifest *pNew;
  int nDb;
  int i;
  u32 iVersion;

  iVersion = bcvGetU32(a);
  if( iVersion!=BCV_MANIFEST_VERSION ){
    fatal_error("bad manifest version - expected %d, found %d.",
        BCV_MANIFEST_VERSION, iVersion
    );
  }
  
  nDb = manifestDatabaseCount(a, n);
  pNew = (Manifest*)bcvMallocZero(
      sizeof(Manifest) + (nDb+1)*sizeof(ManifestDb)
  );

  pNew->aDb = (ManifestDb*)&pNew[1];
  pNew->nDb = nDb;
  pNew->szBlk = manifestBlockSize(a, n);
  pNew->nNamebytes = manifestNamebytes(a, n);
  pNew->nDelBlk = manifestGCBlkCount(a, n);
  pNew->aDelBlk = manifestGCBlkArray(a, n);
  pNew->pFree = a;
  pNew->zETag = zETag;
  pNew->nRef = 1;

  for(i=0; i<nDb; i++){
    ManifestDb *pDb = &pNew->aDb[i];
    memcpy(pDb->aId, manifestDatabaseId(a, n, i), BCV_DBID_SIZE);
    memcpy(pDb->zDName, manifestDatabaseName(a, n, i), BCV_DBNAME_SIZE);
    pDb->iVersion = manifestDatabaseVersion(a, n, i);
    pDb->aBlkLocal = pDb->aBlkOrig = manifestBlockArray(a, n, i);
    pDb->nBlkLocal = pDb->nBlkOrig = manifestBlockCount(a, n, i);
    if( i>0 && memcmp(pDb[-1].aId, pDb[0].aId, BCV_DBID_SIZE)>=0 ){
      assert( 0 );
      fatal_error("corrupt manifest file - ids out of order");
    }
  }

  if( pNew->nNamebytes<BCV_MIN_NAMEBYTES 
   || pNew->nNamebytes>BCV_MAX_NAMEBYTES
  ){
    fatal_error("corrupt manifest file - namebytes value out of range (%d)",
        pNew->nNamebytes
    );
  }

  return pNew;
}

static u8 *bcvManifestCompose(Manifest *p, int *pnOut){
  int nByte;                      /* Bytes required for new manifest */
  u8 *aNew;                       /* New serialized manifest */
  int i;
  int iOff;
  int iGCOff;

  /* Calculate the size of the serialized manifest */
  nByte = BCV_MANIFEST_HEADER_BYTES 
        + p->nDb * BCV_MANIFEST_DBHEADER_BYTES
        + p->nDelBlk * GCENTRYBYTES(p);
  for(i=0; i<p->nDb; i++){
    nByte += p->aDb[i].nBlkOrig * NAMEBYTES(p);
  }

  aNew = (u8*)bcvMalloc(nByte);

  /* Serialize the manifest header */
  bcvPutU32(&aNew[0], BCV_MANIFEST_VERSION);/* Manifest version */
  bcvPutU32(&aNew[4], p->szBlk);            /* Block size */
  bcvPutU32(&aNew[8], p->nDb);              /* Database count */
  bcvPutU32(&aNew[12], p->nDelBlk);         /* GC block count */
  bcvPutU32(&aNew[16], p->nNamebytes);      /* Bytes of data in block names */
  assert( p->nNamebytes>=BCV_MIN_NAMEBYTES );
  assert( p->nNamebytes<=BCV_MAX_NAMEBYTES );

  /* Copy in the GC block array, if any */
  iGCOff = BCV_MANIFEST_HEADER_BYTES + p->nDb * BCV_MANIFEST_DBHEADER_BYTES;
  if( p->nDelBlk>0 ){
    memcpy(&aNew[iGCOff], p->aDelBlk, p->nDelBlk*GCENTRYBYTES(p));
  }

  /* Serialize each of the database headers. And copy the block arrays. */
  iOff = iGCOff + p->nDelBlk * GCENTRYBYTES(p);
  for(i=0; i<p->nDb; i++){
    ManifestDb *pDb = &p->aDb[i];
    u8 *aHdr = &aNew[BCV_MANIFEST_HEADER_BYTES + i*BCV_MANIFEST_DBHEADER_BYTES];
    bcvPutU32(&aHdr[0], pDb->iVersion);
    bcvPutU32(&aHdr[4], iOff);
    bcvPutU32(&aHdr[8], pDb->nBlkOrig);
    memcpy(&aHdr[12], pDb->aId, BCV_DBID_SIZE);
    memcpy(&aHdr[12+BCV_DBID_SIZE], pDb->zDName, BCV_DBNAME_SIZE);
    memcpy(&aNew[iOff], pDb->aBlkOrig, pDb->nBlkOrig*NAMEBYTES(p));
    iOff += pDb->nBlkOrig * NAMEBYTES(p);
  }

  assert( iOff==nByte );
  *pnOut = nByte;
  return aNew;
}

static void bcvManifestFree(Manifest *p){
  if( p ){
    int i;
    for(i=0; i<p->nDb; i++){
      ManifestDb *pDb = &p->aDb[i];
      assert( (pDb->nBlkLocalAlloc==0)==(pDb->aBlkLocal==pDb->aBlkOrig) );
      if( pDb->bBlkOrigFree ) sqlite3_free(pDb->aBlkOrig);
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

static void bcvManifestDeref(Manifest *p){
  if( p ){
    p->nRef--;
    assert( p->nRef>=0 );
    if( p->nRef==0 ){
      bcvManifestFree(p);
    }
  }
}

static Manifest *bcvManifestRef(Manifest *p){
  p->nRef++;
  return p;
}

/*
** Return a copy of the Manifest object passed as the only argument.
*/
static Manifest *bcvManifestDup(Manifest *p){
  Manifest *pNew;
  int i;
  int nByte;

  nByte = sizeof(Manifest) + (p->nDb+1) * sizeof(ManifestDb);

  pNew = (Manifest*)bcvMallocZero(nByte);
  pNew->nDb = p->nDb;
  pNew->szBlk = p->szBlk;
  pNew->nNamebytes = p->nNamebytes;
  pNew->aDb = (ManifestDb*)&pNew[1];
  for(i=0; i<pNew->nDb; i++){
    ManifestDb *pNewDb = &pNew->aDb[i];
    ManifestDb *pOldDb = &p->aDb[i];

    memcpy(pNewDb->aId, pOldDb->aId, BCV_DBID_SIZE);
    memcpy(pNewDb->zDName, pOldDb->zDName, BCV_DBNAME_SIZE);
    assert( pOldDb->nBlkLocal>=pOldDb->nBlkOrig );

    assert( (!pOldDb->nBlkLocalAlloc)==(pOldDb->aBlkLocal==pOldDb->aBlkOrig));
    pNewDb->nBlkOrig = pOldDb->nBlkOrig;
    if( pOldDb->nBlkOrig>0 ){
      int nByte = pOldDb->nBlkOrig*NAMEBYTES(p);
      pNewDb->aBlkOrig = (u8*)bcvMalloc(nByte);
      memcpy(pNewDb->aBlkOrig, pOldDb->aBlkOrig, nByte);
      pNewDb->nBlkOrig = pOldDb->nBlkOrig;
      pNewDb->bBlkOrigFree = 1;
    }
    if( pOldDb->nBlkLocalAlloc ){
      int nByte = pOldDb->nBlkLocal*NAMEBYTES(p);
      pNewDb->aBlkLocal = (u8*)bcvMalloc(nByte);
      memcpy(pNewDb->aBlkLocal, pOldDb->aBlkLocal, nByte);
      pNewDb->nBlkLocal = pNewDb->nBlkLocalAlloc = pOldDb->nBlkLocal;
    }else{
      pNewDb->nBlkLocal = pNewDb->nBlkOrig;
      pNewDb->aBlkLocal = pNewDb->aBlkOrig;
    }
    pNewDb->iVersion = pOldDb->iVersion;
  }

  if( p->nDelBlk ){
    nByte = p->nDelBlk * GCENTRYBYTES(p);
    pNew->aDelBlk = (u8*)bcvMalloc(nByte);
    memcpy(pNew->aDelBlk, p->aDelBlk, nByte);
    pNew->nDelBlk = p->nDelBlk;
    pNew->bDelFree = 1;
  }

  pNew->zETag = bcvStrdup(p->zETag);
  pNew->nRef = 1;
  return pNew;
}

/*
** Synchronous function to request manifest file from the specified
** container.
*/
static u8 *bcvGetManifest(
  CurlRequest *pCurl, 
  AccessPoint *pA,
  const char *zContainer,
  int *pnManifest,
  char **pzETag
){
  MemoryDownload md;
  CURLcode res;

  /* Download any existing manifest file. If there is no existing manifest
  ** file, create a new one containing zero databases. */
  curlFetchBlob(pCurl, pA, zContainer, BCV_MANIFEST_FILE);
  memset(&md, 0, sizeof(MemoryDownload));
  md.pCurl = pCurl;
  curl_easy_setopt(pCurl->pCurl, CURLOPT_WRITEFUNCTION, memoryDownloadWrite);
  curl_easy_setopt(pCurl->pCurl, CURLOPT_WRITEDATA, (void*)&md);
  res = curl_easy_perform(pCurl->pCurl);
  curlFatalIfNot(pCurl, res, 200, "download manifest failed");
  *pzETag = pCurl->zETag;
  pCurl->zETag = 0;
  curlRequestReset(pCurl);

  *pnManifest = md.nByte;
  return md.a;
}

static Manifest *bcvManifestGetParsed(
  CurlRequest *pCurl, 
  AccessPoint *pA,
  const char *zContainer,
  int *pnByte
){
  int n;
  u8 *a;
  char *zETag = 0;
  a = bcvGetManifest(pCurl, pA, zContainer, &n, &zETag);
  if( pnByte ) *pnByte = n;
  return bcvManifestParse(a, n, zETag);
}

static void bcvManifestPutParsed(
  CurlRequest *pCurl,
  Manifest *p,
  AccessPoint *pA,
  const char *zContainer
){
  MemoryUpload mu;
  u8 *aMan = 0;
  int nMan = 0;
  CURLcode res;
  char *zIfMatch = 0;

  assert( pCurl->pList==0 );
  assert( p->nDb==0 || p->zETag );

  aMan = bcvManifestCompose(p, &nMan);
  curlPutBlob(pCurl, pA, zContainer, BCV_MANIFEST_FILE, p->zETag, nMan);
  memset(&mu, 0, sizeof(MemoryUpload));
  mu.a = aMan;
  mu.nByte = nMan;
  curl_easy_setopt(pCurl->pCurl, CURLOPT_READFUNCTION, memoryUploadRead);
  curl_easy_setopt(pCurl->pCurl, CURLOPT_READDATA, (void*)&mu);
  curl_easy_setopt(pCurl->pCurl, CURLOPT_SEEKFUNCTION, memoryUploadSeek);
  curl_easy_setopt(pCurl->pCurl, CURLOPT_SEEKDATA, (void*)&mu);

  res = curl_easy_perform(pCurl->pCurl);
  curlFatalIfNot(pCurl, res, 201, "manifest upload failed");

  curlRequestReset(pCurl);
  sqlite3_free(aMan);
  sqlite3_free(zIfMatch);
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
static void bcvBlockidToText(Manifest *p, const u8 *pBlk, char *aBuf){
  hex_encode(pBlk, NAMEBYTES(p), aBuf);
  memcpy(&aBuf[NAMEBYTES(p)*2], ".bcv", 5);
}

static int bcvMHashBucket(ManifestHash *pHash, u8 *pBlk){
  return (int)(bcvGetU32(pBlk) % pHash->nHash);
}

/*
** Build and return a ManifestHash object for the manifest passed as
** the only argument. Exclude blocks from database iExclude from the hash.
*/
static ManifestHash *bcvMHashBuild(Manifest *pMan, int iExclude){
  ManifestHash *pHash = 0;
  if( NAMEBYTES(pMan)>=BCV_MIN_MD5_NAMEBYTES ){
    int nBlock = 0;               /* Number of blocks in manifest */
    int nHash = 1;                /* Number of hash buckets */
    int nByte;                    /* Bytes of space to allocate */
    int i;

    for(i=0; i<pMan->nDb; i++){
      if( i!=iExclude ){
        nBlock += pMan->aDb[i].nBlkOrig;
      }
    }

    while( nHash<nBlock ) nHash = nHash*2;
    nHash = nHash*2;
    nByte = sizeof(ManifestHash)+sizeof(u8*)*nHash;
    pHash = (ManifestHash*)bcvMallocZero(nByte);
    pHash->nHash = nHash;

    for(i=0; i<pMan->nDb; i++){
      if( i!=iExclude ){
        int iBlk;
        ManifestDb *pDb = &pMan->aDb[i];
        for(iBlk=0; iBlk<pDb->nBlkOrig; iBlk++){
          u8 *pBlk = &pDb->aBlkOrig[iBlk*NAMEBYTES(pMan)];
          int iHash = bcvMHashBucket(pHash, pBlk);
          while( pHash->aHash[iHash] ) iHash = (iHash + 1) % pHash->nHash;
          pHash->aHash[iHash] = pBlk;
        }
      }
    }
  }

  return pHash;
}

/*
** Free a hash object allocated by bcvMHashBuild().
*/
static void bcvMHashFree(ManifestHash *pHash){
  sqlite3_free(pHash);
}

/*
** Search the hash table for a block-id that matches the nPrefix byte 
** prefix aPrefix[]. Return a pointer to the first such block-id found,
** or NULL if there is no such block id in the hash table.
*/
static u8 *bcvMHashMatch(ManifestHash *pHash, u8 *aPrefix, int nPrefix){
  if( pHash ){
    int iHash = bcvMHashBucket(pHash, aPrefix);
    while( pHash->aHash[iHash] ){
      if( 0==memcmp(pHash->aHash[iHash], aPrefix, nPrefix) ){
        return pHash->aHash[iHash];
      }
      iHash = (iHash + 1) % pHash->nHash;
    }
  }
  return 0;
}

/*
** Command: $argv[0] upload ?SWITCHES? DBFILE ?NAME?
**
** Upload a database file to cloud storage. The container and manifest
** file must already exist.
*/
static int main_upload(int argc, char **argv){
  CURLcode res;
  CurlRequest curl;
  ClistParse clist;
  int nDbfile;
  CommandSwitches cmd;
  int fd;                         /* fd open on database file */
  int i;
  struct stat buf;                /* Used to fstat() database file */
  int nBlock;                     /* Number of blocks */
  AccessPoint ap;
  Manifest *p = 0;
  ManifestDb *pDb = 0;
  ManifestHash *pMHash = 0;
  int iFail = 0;
  int *ptr;
  const char *zDbfile;
  const char *zName;
  MemoryUpload mu;                /* For uploading block files */
  u8 aId[BCV_DBID_SIZE];          /* New database id */

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s upload ?SWITCHES? DBFILE ?NAME?\n", argv[0]);
    exit(1);
  }

  ptr = &iFail;
  while( iFail<argc-4 ){
    parse_switches(&cmd, (const char**)&argv[2], argc-3, ptr,
        COMMANDLINE_CONTAINER | COMMANDLINE_ACCESSKEY | COMMANDLINE_EMULATOR |
        COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE 
    );
    ptr = 0;
  }
  assert( iFail==argc-4 || iFail==argc-3 );
  if( iFail==argc-4 ){
    zDbfile = (const char*)argv[argc-2];
    zName = (const char*)argv[argc-1];
  }else{
    zDbfile = (const char*)argv[argc-1];
    zName = &zDbfile[strlen(zDbfile)];
    while( zName>zDbfile && zName[-1]!='/' ) zName--;
  }
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);
  curlRequestInit(&curl, cmd.bVerbose);

  /* Check that the database name is not too long for the manifest format */
  nDbfile = strlen(zDbfile);
  if( nDbfile>=BCV_DBNAME_SIZE ){
    fatal_error("database name \"%s\" is too long (max = %d bytes)",
        zDbfile, BCV_DBNAME_SIZE-1
    );
  }

  /* Check if the named container exists. Error out if it does not. */
  memset(&clist, 0, sizeof(ClistParse));
  clist.zExists = cmd.zContainer;
  bcvListContainers(&curl, &clist, &ap,cmd.zContainer);
  if( clist.bExists==0 ){
    fatal_error("no such container: %s", cmd.zContainer);
  }
  curlRequestReset(&curl);

  /* Download the manifest file. */
  p = bcvManifestGetParsed(&curl, &ap, cmd.zContainer, 0);

  /* Check that the db name is not already in the manifest. */
  i = bcvManifestNameToIndex(p, zName);
  if( i>=0 ) fatal_error("duplicate database name - \"%s\"", zName);

  /* Open the database file to be uploaded. */
  fd = open(zDbfile, O_RDONLY|O_BINARY);
  if( fd<0 ){
    fatal_error("failed to open file \"%s\" - %s\n", zDbfile, strerror(errno));
  }
  fstat(fd, &buf);
  nBlock = ((u64)buf.st_size + p->szBlk-1) / p->szBlk;

  /* Extend manifest to include new database */
  sqlite3_randomness(BCV_DBID_SIZE, aId);
  for(i=0; i<p->nDb; i++){
    if( memcmp(aId, p->aDb[i].aId, BCV_DBID_SIZE)<0 ) break;
  }
  pDb = &p->aDb[i];
  if( i<p->nDb ) memmove(&pDb[1], pDb, (p->nDb-i)*sizeof(ManifestDb));
  p->nDb++;
  pDb->iVersion = 1;
  memset(pDb->zDName, 0, BCV_DBNAME_SIZE);
  memcpy(pDb->aId, aId, BCV_DBID_SIZE);
  memcpy(pDb->zDName, zName, strlen(zName));
  if( nBlock>0 ){
    assert( pDb->nBlkLocalAlloc==0 );
    pDb->nBlkLocal = pDb->nBlkLocalAlloc = nBlock;
    pDb->aBlkLocal = (u8*)bcvMalloc(nBlock * NAMEBYTES(p));
  }

  memset(&mu, 0, sizeof(MemoryUpload));
  mu.a = (u8*)bcvMalloc(p->szBlk);

  pMHash = bcvMHashBuild(p, -1);
  for(i=0; i<nBlock; i++){
    ssize_t rres;
    char aBuf[BCV_MAX_FSNAMEBYTES];
    u8 *aBlk = &pDb->aBlkLocal[i * NAMEBYTES(p)];

    mu.nByte = (i==nBlock-1 ? buf.st_size % p->szBlk : p->szBlk);
    mu.iOff = 0;
    rres = read(fd, mu.a, mu.nByte);
    if( rres!=mu.nByte ){
      fatal_error("failed to read %d bytes from offset %lld of %s (fd=%d)", mu.nByte, 
          (i64)i*p->szBlk, zDbfile, fd
      );
    }

    if( NAMEBYTES(p)>=BCV_MIN_MD5_NAMEBYTES ){
      u8 *pMatch;
      assert( MD5_DIGEST_LENGTH==16 );
      MD5(mu.a, mu.nByte, aBlk);
      pMatch = bcvMHashMatch(pMHash, aBlk, MD5_DIGEST_LENGTH);
      if( pMatch ){
        memcpy(aBlk, pMatch, NAMEBYTES(p));
        continue;
      }else{
        sqlite3_randomness(
            NAMEBYTES(p) - MD5_DIGEST_LENGTH, &aBlk[MD5_DIGEST_LENGTH]
        );
      }
    }else{
      sqlite3_randomness(NAMEBYTES(p), aBlk);
    }

    bcvBlockidToText(p, aBlk, aBuf);
    curlPutBlob(&curl, &ap, cmd.zContainer, aBuf, 0, mu.nByte);

    curl_easy_setopt(curl.pCurl, CURLOPT_READFUNCTION, memoryUploadRead);
    curl_easy_setopt(curl.pCurl, CURLOPT_READDATA, (void*)&mu);
    curl_easy_setopt(curl.pCurl, CURLOPT_SEEKFUNCTION, memoryUploadSeek);
    curl_easy_setopt(curl.pCurl, CURLOPT_SEEKDATA, (void*)&mu);
    res = curl_easy_perform(curl.pCurl);
    curlFatalIfNot(&curl, res, 201, "block upload failed");
    curlRequestReset(&curl);
    fprintf(stdout, ".");
    fflush(stdout);
  }
  bcvMHashFree(pMHash);
  sqlite3_free(mu.a);
  assert( pDb->bBlkOrigFree==0 );
  pDb->aBlkOrig = pDb->aBlkLocal;
  pDb->nBlkOrig = pDb->nBlkLocal;
  pDb->bBlkOrigFree = 1;
  pDb->nBlkLocalAlloc = 0;
  bcvManifestPutParsed(&curl, p, &ap, cmd.zContainer);

  curlRequestFinalize(&curl);
  close(fd);
  bcvAccessPointFree(&ap);
  free_parse_switches(&cmd);
  bcvManifestFree(p);
  return 0;
}

/*
** Command: $argv[0] download ?SWITCHES? DBFILE
**
** Download a database file from cloud storage.
*/
static int main_download(int argc, char **argv){
  CommandSwitches cmd;
  CURLcode res;
  AccessPoint ap;
  CurlRequest curl;
  const char *zFile;

  int iDb;                        /* Database index in manifest */
  int i;
  FileDownload fd;

  Manifest *p;
  ManifestDb *pDb;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s download ?SWITCHES? DBFILE\n", argv[0]);
    exit(1);
  }
  zFile = argv[argc-1];
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0,
    COMMANDLINE_CONTAINER | COMMANDLINE_ACCESSKEY | COMMANDLINE_EMULATOR |
    COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE
  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);
  curlRequestInit(&curl, cmd.bVerbose);

  /* Grab the manifest file */
  p = bcvManifestGetParsed(&curl, &ap, cmd.zContainer, 0);

  /* Search manifest for the requested file. Error out if it is not there. */
  iDb = bcvManifestNameToIndex(p, zFile);
  if( iDb<0 ) fatal_error("database not found: %s", zFile);
  pDb = &p->aDb[iDb];

  /* Open the local file (to download to) */
  memset(&fd, 0, sizeof(FileDownload));
  fd.fd = open(zFile, O_CREAT|O_RDWR|O_BINARY, 0644);
  if( fd.fd<0 ){
    fatal_error("cannot open local file for writing: %s", zFile);
  }

  for(i=0; i<pDb->nBlkLocal; i++){
    long iRes;
    char aBuf[BCV_MAX_FSNAMEBYTES];
    bcvBlockidToText(p, &pDb->aBlkLocal[NAMEBYTES(p)*i], aBuf);
    fd.iStart = (i64)i*p->szBlk;
    fd.iOff = 0;
    curlFetchBlob(&curl, &ap, cmd.zContainer, aBuf);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEFUNCTION, fileDownloadWrite);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEDATA, (void*)&fd);
    res = curl_easy_perform(curl.pCurl);
    if( res!=CURLE_OK ){
      fatal_error("Download failed: %s (%s)", aBuf, curl_easy_strerror(res));
    }
    curl_easy_getinfo(curl.pCurl, CURLINFO_RESPONSE_CODE, &iRes);
    if( iRes!=200 ){
      fatal_error("Download failed: %s (http code=%d)", aBuf, iRes);
    }
    curlRequestReset(&curl);
  }

  curlRequestFinalize(&curl);
  bcvAccessPointFree(&ap);
  bcvManifestFree(p);
  return 0;
}

static void bcvDeleteBlocks(Manifest *pMan, int iDb){
  ManifestDb *pDb = &pMan->aDb[iDb];
  u8 *aGC = 0;
  i64 iTime = 0;
  u8 aTime[8];
  int i;

  iTime = sqlite_timestamp();
  bcvPutU64(aTime, iTime);

  assert( pDb->nBlkOrig==pDb->nBlkLocal );
  aGC = (u8*)bcvMalloc((pMan->nDelBlk+pDb->nBlkOrig) * GCENTRYBYTES(pMan));
  if( pMan->nDelBlk ){
    memcpy(aGC, pMan->aDelBlk, pMan->nDelBlk * GCENTRYBYTES(pMan));
  }
  for(i=0; i<pDb->nBlkOrig; i++){
    memcpy(&aGC[(pMan->nDelBlk+i)*GCENTRYBYTES(pMan)], 
           &pDb->aBlkOrig[i*NAMEBYTES(pMan)],
           NAMEBYTES(pMan)
    );
    memcpy(&aGC[(pMan->nDelBlk+i)*GCENTRYBYTES(pMan)+16], aTime, 8);
  }
  pMan->nDelBlk += pDb->nBlkOrig;
  pMan->aDelBlk = aGC;
  pMan->bDelFree = 1;
}

/*
** Command: $argv[0] copy ?SWITCHES? DB1 DB2
**
** Make a copy of a database within cloud storage.
*/
static int main_copy(int argc, char **argv){
  CurlRequest curl;
  const char *zDbFrom, *zDbTo;
  int nDbTo;
  int iFrom, iTo;
  ManifestDb *pFrom, *pTo;
  CommandSwitches cmd;
  Manifest *p = 0;
  AccessPoint ap;

  /* Parse command line */
  if( argc<4 ){
    fprintf(stderr, "Usage: %s copy ?SWITCHES? DBFROM DBTO\n", argv[0]);
    exit(1);
  }
  zDbFrom = argv[argc-2];
  zDbTo = argv[argc-1];
  nDbTo = strlen(zDbTo);
  parse_switches(&cmd, (const char**)&argv[2], argc-4, 0,
    COMMANDLINE_CONTAINER | COMMANDLINE_ACCESSKEY | 
    COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_EMULATOR
  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);
  curlRequestInit(&curl, cmd.bVerbose);

  /* Download the manifest file. */
  p = bcvManifestGetParsed(&curl, &ap, cmd.zContainer, 0);

  iTo = bcvManifestNameToIndex(p, zDbTo);
  if( iTo<0 ){
    u8 aId[BCV_DBID_SIZE];
    sqlite3_randomness(BCV_DBID_SIZE, aId);
    for(iTo=0; iTo<p->nDb; iTo++){
      if( memcmp(aId, p->aDb[iTo].aId, BCV_DBID_SIZE)<0 ) break;
    }
    if( iTo<p->nDb ){
      memmove(&p->aDb[iTo+1], &p->aDb[iTo], sizeof(ManifestDb)*(p->nDb-iTo));
    }
    p->nDb++;
    pTo = &p->aDb[iTo];
    memset(pTo, 0, sizeof(ManifestDb));
    memcpy(pTo->aId, aId, BCV_DBID_SIZE);
    memcpy(pTo->zDName, zDbTo, nDbTo);
  }else{
    bcvDeleteBlocks(p, iTo);
    pTo = &p->aDb[iTo];
    pTo->aBlkOrig = pTo->aBlkLocal = 0;
    pTo->nBlkOrig = pTo->nBlkLocal = 0;
  }

  /* Check that the from-db name is in the manifest. */
  iFrom = bcvManifestNameToIndex(p, zDbFrom);
  if( iFrom<0 ) fatal_error("no such database: %s", zDbFrom);
  pFrom = &p->aDb[iFrom];

  pTo->nBlkLocal = pTo->nBlkOrig = pFrom->nBlkOrig;
  pTo->aBlkOrig = (u8*)bcvMallocZero(pTo->nBlkOrig*NAMEBYTES(p));
  pTo->aBlkLocal = pTo->aBlkOrig;
  memcpy(pTo->aBlkOrig, pFrom->aBlkOrig, pTo->nBlkOrig*NAMEBYTES(p));
  pTo->iVersion++;

  bcvManifestPutParsed(&curl, p, &ap, cmd.zContainer);

  curlRequestFinalize(&curl);
  bcvAccessPointFree(&ap);
  bcvManifestFree(p);
  return 0;
}

/*
** Command: $argv[0] clist ?SWITCHES?
**
** List all containers in the specified account.
*/
static int main_clist(int argc, char **argv){
  MemoryDownload md;
  CommandSwitches cmd;
  CURLcode res;
  CurlRequest curl;
  AccessPoint ap;

  /* Parse command line */
  parse_switches(&cmd, (const char**)&argv[2], argc-2, 0,
    COMMANDLINE_ACCESSKEY | COMMANDLINE_ACCOUNT 
    | COMMANDLINE_VERBOSE | COMMANDLINE_EMULATOR
  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);

  curlRequestInit(&curl, cmd.bVerbose);
  curlListContainers(&curl, &ap, cmd.zContainer);

  memset(&md, 0, sizeof(MemoryDownload));
  md.pCurl = &curl;
  curl_easy_setopt(curl.pCurl, CURLOPT_WRITEFUNCTION, memoryDownloadWrite);
  curl_easy_setopt(curl.pCurl, CURLOPT_WRITEDATA, (void*)&md);

  res = curl_easy_perform(curl.pCurl);
  if( res!=CURLE_OK ){
    fprintf(stderr, "clist failed: %s\n", curl_easy_strerror(res));
    exit(-1);
  }

  if( md.iRes==200 ){
    SimpleXmlParser x;
    int nContainer = 0;
    ObjectName *p = 0;
    ObjectName *pNext = 0;
    ClistParse clist;
    memset(&clist, 0, sizeof(ClistParse));
    x = simpleXmlCreateParser((char*)md.a, md.nByte);
    simpleXmlPushUserData(x, (void*)&clist);
    simpleXmlParse(x, clistTagHandler);
    simpleXmlDestroyParser(x);
    for(p=clist.pList; p; p=pNext){
      printf("%s\n", p->zName);
      nContainer++;
      pNext = p->pNext;
      sqlite3_free(p);
    }
    printf("%d total\n", nContainer);
  }else{
    fatal_error("clist failed - %d %s", md.iRes, (char*)md.a);
  }

  sqlite3_free(md.a);
  curlRequestFinalize(&curl);
  free_parse_switches(&cmd);
  bcvAccessPointFree(&ap);
  return 0;
}

static int main_files(int argc, char **argv){
  CommandSwitches cmd;
  CurlRequest curl;
  AccessPoint ap;
  MemoryDownload md;
  CURLcode res;
  FilesParse files;
  FilesParseEntry *p;

  /* Parse command line */
  parse_switches(&cmd, (const char**)&argv[2], argc-2, 0,
    COMMANDLINE_CONTAINER | COMMANDLINE_ACCESSKEY | 
    COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_EMULATOR

  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);

  curlRequestInit(&curl, cmd.bVerbose);

  memset(&files, 0, sizeof(files));
  do{
    memset(&md, 0, sizeof(MemoryDownload));
    md.pCurl = &curl;

    curlListFiles(&curl, &ap, cmd.zContainer, files.zNextMarker);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEFUNCTION, memoryDownloadWrite);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEDATA, (void*)&md);

    res = curl_easy_perform(curl.pCurl);
    curlFatalIfNot(&curl, res, 200, "list files failed");

    bcvParseFiles(md.a, md.nByte , &files);
    curlRequestReset(&curl);
  }while( files.zNextMarker );

  for(p=files.pList; p; p=p->pNext){
    i64 iTm = parse_timestamp(p->zModified);
    printf("%s \"%s\" (%lld)\n", p->zName, p->zModified, iTm);
  }

  sqlite3_free(md.a);
  bcvParseFilesClear(&files);
  curlRequestFinalize(&curl);
  bcvAccessPointFree(&ap);
  return 0;
}

static int main_list(int argc, char **argv){
  CommandSwitches cmd;
  CurlRequest curl;
  AccessPoint ap;
  Manifest *p;
  int i;

  /* Parse command line */
  parse_switches(&cmd, (const char**)&argv[2], argc-2, 0,
    COMMANDLINE_CONTAINER | COMMANDLINE_ACCESSKEY | 
    COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_EMULATOR

  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);

  curlRequestInit(&curl, cmd.bVerbose);
  p = bcvManifestGetParsed(&curl, &ap, cmd.zContainer, 0);

  for(i=0; i<p->nDb; i++){
    ManifestDb *pDb = &p->aDb[i];
    printf("%s %d blocks\n", pDb->zDName, pDb->nBlkLocal);
  }

  curlRequestFinalize(&curl);
  bcvAccessPointFree(&ap);
  bcvManifestFree(p);
  return 0;
}

static int main_manifest(int argc, char **argv){
  CommandSwitches cmd;
  CurlRequest curl;
  AccessPoint ap;
  Manifest *p;
  int i, j;

  /* Parse command line */
  parse_switches(&cmd, (const char**)&argv[2], argc-2, 0,
    COMMANDLINE_CONTAINER | COMMANDLINE_ACCESSKEY | 
    COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_EMULATOR
  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);

  curlRequestInit(&curl, cmd.bVerbose);
  p = bcvManifestGetParsed(&curl, &ap, cmd.zContainer, 0);

  printf("Manifest version: %d\n", BCV_MANIFEST_VERSION);
  printf("Block size: %d\n", p->szBlk);
  printf("Database count: %d\n", p->nDb);

  printf("Delete Block list: (%d blocks)\n", p->nDelBlk);
  for(i=0; i<p->nDelBlk; i++){
    u8 *aEntry = &p->aDelBlk[i*GCENTRYBYTES(p)];
    i64 t;
    char aBuf[BCV_MAX_FSNAMEBYTES];
    bcvBlockidToText(p, aEntry, aBuf);
    t = bcvGetU64(&aEntry[NAMEBYTES(p)]);
    printf("    %s (t=%lld)\n", aBuf, t);
  }

  for(i=0; i<p->nDb; i++){
    char zId[BCV_DBID_SIZE*2+1];
    ManifestDb *pDb = &p->aDb[i];
    hex_encode(pDb->aId, BCV_DBID_SIZE, zId);
    printf("Database %d id: %s\n", i, zId);
    printf("Database %d name: %s\n", i, pDb->zDName);
    printf("Database %d version: %d\n", i, pDb->iVersion);
    printf("Database %d block list: (%d blocks)\n", i, pDb->nBlkLocal);
    for(j=0; j<pDb->nBlkLocal; j++){
      char aBuf[BCV_MAX_FSNAMEBYTES];
      bcvBlockidToText(p, &pDb->aBlkLocal[j*NAMEBYTES(p)], aBuf);
      printf("    %s\n", aBuf);
    }
  }

  curlRequestFinalize(&curl);
  bcvAccessPointFree(&ap);
  bcvManifestFree(p);
  return 0;
}

static int main_delete(int argc, char **argv){
  CurlRequest curl;
  const char *zDbfile;
  CommandSwitches cmd;
  Manifest *p = 0;
  ManifestDb *pDb = 0;
  int iDb;
  AccessPoint ap;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s upload ?SWITCHES? DB\n", argv[0]);
    exit(1);
  }
  zDbfile = argv[argc-1];
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0,
    COMMANDLINE_CONTAINER | COMMANDLINE_ACCESSKEY | 
    COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_EMULATOR
  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);
  curlRequestInit(&curl, cmd.bVerbose);


  /* Download the manifest file. */
  p = bcvManifestGetParsed(&curl, &ap, cmd.zContainer, 0);

  /* Check that the db name is in the manifest. */
  iDb = bcvManifestNameToIndex(p, zDbfile);
  if( iDb<0 ) fatal_error("no such database: %s", zDbfile);
  pDb = &p->aDb[iDb];

  /* Move blocks to the delete list */
  bcvDeleteBlocks(p, iDb);

  assert( pDb->bBlkOrigFree==0 && pDb->nBlkLocalAlloc==0 );
  if( iDb<p->nDb-1 ){
    memmove(pDb, &pDb[1], (p->nDb-iDb-1)*sizeof(ManifestDb));
  }
  p->nDb--;

  bcvManifestPutParsed(&curl, p, &ap, cmd.zContainer);

  curlRequestFinalize(&curl);
  bcvAccessPointFree(&ap);
  bcvManifestFree(p);
  return 0;
}

static int main_create(int argc, char **argv){
  CurlRequest curl;
  const char *zCont;              /* Name of new container */
  CommandSwitches cmd;
  CURLcode res;
  Manifest man;
  AccessPoint ap;
  int bSkipCreate = 0;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s create ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  zCont = argv[argc-1];
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0,
    COMMANDLINE_ACCESSKEY | COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_BLOCKSIZE | COMMANDLINE_EMULATOR | COMMANDLINE_CLOBBER |
    COMMANDLINE_NAMEBYTES
  ); 
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  if( cmd.szBlk<=0 ) cmd.szBlk = BCV_DEFAULT_BLOCKSIZE;
  if( cmd.nNamebytes<=0 ) cmd.nNamebytes = BCV_DEFAULT_NAMEBYTES;
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);
  curlRequestInit(&curl, cmd.bVerbose);

  if( cmd.bClobber ){
    ClistParse clist;
    memset(&clist, 0, sizeof(ClistParse));
    clist.zExists = zCont;
    bcvListContainers(&curl, &clist, &ap,cmd.zContainer);
    bSkipCreate = clist.bExists;
    curlRequestReset(&curl);
  }

  if( bSkipCreate==0 ){
    curlCreateContainer(&curl, &ap, zCont);
    res = curl_easy_perform(curl.pCurl);
    curlFatalIfNot(&curl, res, 201, "create container failed");
    curlRequestReset(&curl);
  }

  memset(&man, 0, sizeof(man));
  man.szBlk = cmd.szBlk;
  man.nNamebytes = cmd.nNamebytes;
  bcvManifestPutParsed(&curl, &man, &ap, zCont);

  free_parse_switches(&cmd);
  bcvAccessPointFree(&ap);
  curlRequestFinalize(&curl);
  return 0;
}

static int main_destroy(int argc, char **argv){
  CurlRequest curl;
  const char *zCont;              /* Name of new container */
  CommandSwitches cmd;
  CURLcode res;
  AccessPoint ap;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s destroy ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  zCont = argv[argc-1];
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0,
    COMMANDLINE_ACCESSKEY | COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_EMULATOR
  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, cmd.zAccount, cmd.zAccessKey, cmd.zEmulator);
  curlRequestInit(&curl, cmd.bVerbose);

  curlDestroyContainer(&curl, &ap, zCont);
  res = curl_easy_perform(curl.pCurl);
  curlFatalIfNot(&curl, res, 202, "destroy container failed");

  bcvAccessPointFree(&ap);
  curlRequestFinalize(&curl);
  return 0;
}

/**************************************************************************
***************************************************************************
** START OF DAEMON CODE
***************************************************************************
**************************************************************************/

typedef struct DMessage DMessage;
typedef struct DaemonCtx DaemonCtx;
typedef struct DClient DClient;
typedef struct DCacheEntry DCacheEntry;
typedef struct DCheckpoint DCheckpoint;
typedef struct DUpload DUpload;
typedef struct DGarbageCollect DGarbageCollect;
typedef struct DContainer DContainer;

struct DMessage {
  int eType;                      /* Message type */

  /* Fields used by BCV_MESSAGE_LOGIN ('L') messages */
  char *zContainer; 
  char *zDb;

  /* Integer field used by the following messages:
  **   LOGIN_REPLY: manifest block-size in bytes.
  **   REQUEST: requested block number.
  **   INTERNAL: true if upload ok, false otherwise.
  */
  int iVal;

  /* Blob of data used by the following messages:
  **   REQUEST_REPLY:
  **   WREQUEST:
  **   WREQUEST_REPLY:
  **   DONE:
  **   REQUEST:
  */
  int nBlobByte;
  u8 *aBlob;
};

typedef struct DDelete DDelete;
struct DDelete {
  i64 iDelBefore;                 /* Delete blocks that expire at this time */
  int nReq;                       /* Number of outstanding http requests */
  int iDel;                       /* Next block in delete list to consider */
  Manifest *pMan;                 /* Copy of manifest */
  DContainer *pContainer;         /* Container being deleted from */
  DaemonCtx *pCtx;                /* Main application object */
  int bErr;                       /* True if error has occurred */
};

typedef struct DCollect DCollect;
struct DCollect {
  Manifest *pMan;                 /* Copy of manifest */
  DContainer *pContainer;         /* Container being garbage collected */
  DaemonCtx *pCtx;                /* Main application object */

  int nDelete;
  int nDeleteAlloc;
  u8 *aDelete;

  i64 iDeleteTime;
  u8 **aHash;
  int nHash;
};



/*
** An instance of the following is used for each outstanding HTTPS request
** managed by a daemon process. A daemon makes the following HTTP requests:
**
**   * Download manifest.
**   * Upload manifest.
**
**   * Download block file.
**   * Upload block file.
**   * Delete block file.
**
**   * List files in container.
**
** aBuf, nBuf, nBufAlloc:
**   These are used when uploading or downloading a manifest file and when
**   reading the list of files in a container. In all cases aBuf points to
**   an allocation obtained from sqlite3_malloc(). nBuf is the current
**   valid size of the data in the buffer. For downloads (of either the
**   manifest or the XML returned by Azure for a list-files operation),
**   nBufAlloc is set to the current allocated size of the buffer.
** 
** pMan:
**   When uploading a manifest file, the deserialized manifest. If the
**   upload is successful, this manifest file is installed into the 
**   DContainer indicated by pContainer.
*/
typedef struct DCurlRequest DCurlRequest;
struct DCurlRequest {
  CurlRequest req;
  DCurlRequest *pNext;            /* Next in list of outstanding requests */
  DaemonCtx *pDaemonCtx;          /* Daemon context object */
  void (*xDone)(DCurlRequest*, CURLMsg*);   /* Done callback */

  u8 *aBuf;                       /* Data to upload */
  int nBuf;                       /* Size of aBuf[] in bytes */
  int nBufAlloc;                  /* Allocated size at aBuf */

  Manifest *pMan;                 /* For uploads only - the manifest object */
  DContainer *pContainer;         /* Container manifest belongs to */

  sqlite3_int64 iOff;             /* Write offset for blob requests */
  DCacheEntry *pEntry;            /* Cache entry being populated */

  DUpload *pUpload;               /* Upload this request is a part of */
  DDelete *pDelete;               /* Delete this request is a part of */
  DCollect *pCollect;             /* GC this request is a part of */
};

struct DContainer {
  char *zName;
  Manifest *pManifest;            /* Current manifest object */
  int eOp;                        /* Current operation type (if any) */
  i64 iPollTime;                  /* Time at which to poll for new manifest */
  i64 iDeleteTime;                /* Time at which to delete old blocks */
  i64 iGCTime;                    /* Time at which to GC container */
  DCheckpoint *pCheckpointList;   /* List of ongoing upload operations */
  DContainer *pNext;
  DClient *pAttachClient;         /* Client waiting on bcv_attach reply */
};

/*
** Values for DContainer.eOp
*/
#define CONTAINER_OP_NONE    0
#define CONTAINER_OP_UPLOAD  1
#define CONTAINER_OP_POLL    2
#define CONTAINER_OP_DELETE  3
#define CONTAINER_OP_COLLECT 4

/*
** pWaiting:
**   Linked list of clients waiting for this blob to finish downloading.
*/   
struct DCacheEntry {
  int iPos;                       /* Position of block in cache file */
  int nRef;                       /* Current number of client refs */
  DCacheEntry *pLruPrev;          /* Previous entry in LRU list */
  DCacheEntry *pLruNext;          /* Next entry in LRU list */
  DCacheEntry *pHashNext;         /* Hash collision list */ 

  DClient *pWaiting;              /* Clients waiting on this blob */
  u8 bPending;                    /* True while this is downloading */
  u8 bDirty;                      /* True if this entry is dirty */

  u8 nName;                       /* Size of aName[] in bytes */
  u8 aName[0];                    /* 128-bit block identifier */
};

struct DClient {
  int iId;                        /* Client id */
  BCV_SOCKET_TYPE fd;             /* Localhost socket for this client */
  u8 aId[BCV_DBID_SIZE];          /* Id of accessed database */
  DContainer *pContainer;         /* Database container */
  DClient *pNext;                 /* Next client belonging to this daemon */

  /* Populated while a transaction is opened only */
  Manifest *pManifest;            /* Manifest in use */
  int iDb;                        /* Index of database within pManifest */
  DCacheEntry **apEntry;          /* Array of entries client knows about */

  DClient *pNextWaiting;          /* Next client waiting on same blob */
  int bWReq;                      /* True if waiting on blob for WREQUEST */
  int iWReqBlk;                   /* Block number if bWReq is true */
};

struct DaemonCtx {
  BCV_SOCKET_TYPE fdListen;       /* Socket fd waiting on new connections */
  int iListenPort;                /* Port fdListen is listening on */
  CommandSwitches cmd;            /* Command line options */
  CURLM *pMulti;                  /* The curl multi-handle */
  int iNextId;                    /* Next client id */
  DClient *pClientList;           /* List of connected clients */
  DContainer *pContainerList;     /* List of accessed containers */
  i64 szBlk;                      /* Block size for this daemon */
  AccessPoint ap;                 /* Access point for daemon */

  sqlite3 *db;                    /* Database to record dirty blocks in */
  sqlite3_stmt *pInsert;          /* INSERT INTO dirty VALUES(...) */
  sqlite3_stmt *pDelete;          /* DELETE FROM dirty WHERE ... */
  sqlite3_stmt *pDeletePos;       /* DELETE FROM dirty WHERE ... */
  sqlite3_stmt *pWriteMan;        /* REPLACE INTO manifest VALUES(..) */
  sqlite3_stmt *pReadMan;         /* SELECT manifest FROM manifest ... */

  /* Active requests */
  DCurlRequest *pRequestList;

  /* Cache structures */
  int nCacheMax;                  /* Maximum desired number of cache entries */
  int nCacheUsedEntry;            /* Size of aCacheUsed[] array */
  int nCacheEntry;                /* Current number of cache entries */
  int nHash;                      /* Size of aHash[] array (power-of-2) */
  u32 *aCacheUsed;                /* Bitmask indicating free cache slots */
  DCacheEntry **aHash;            /* Hash table containing all cached blocks */
  DCacheEntry *pLruFirst;         /* Start of LRU list (next to expel) */
  DCacheEntry *pLruLast;          /* Last entry in LRU list */
  sqlite3_file *pCacheFile;       /* File containing cached blocks */
};

/*
** An instance of the following structure exists for each external thread
** running "PRAGMA wal_checkpoint" using a daemon=1 VFS.
*/
struct DCheckpoint {
  char *zContainer;
  char *zDb;
  u8 aId[BCV_DBID_SIZE];
  i64 iRetryTime;                 /* Time to retry upload */
  DCheckpoint *pNext;             /* Next checkpoint belonging to container */
  DClient *pWaiting;              /* List of "PRAGMA bcv_upload" clients */
};

/*
** An instance of the following is allocated for each garbage-collection
** operation currently being undertaken.
*/
struct DGarbageCollect {
  char *zContainer;               /* Container being cleaned out */
  DGarbageCollect *pNext;         /* Next GC being run by this daemon */
};

/*
** This structure represents an upload operation in progress.
*/
struct DUpload {
  DaemonCtx *pCtx;                /* Daemon context */
  Manifest *pManifest;            /* Deep copy of manifest being updated */
  ManifestHash *pMHash;           /* Hash table of blocks in pManifest */
  int nGCOrig;                    /* Blocks in aDelBlk[] array before upload */
  int iDb;                        /* Index of db within pManifest->aDb[] */
  DContainer *pContainer;         /* Container blobs are uploaded to */
  DClient *pClient;               /* Internal client to reply to when done */
  sqlite3_stmt *pRead;            /* Reads dirty blocks from block db */
  int bDone;                      /* All uploads have been started */
  int bError;                     /* An error has occurred */
  int nRef;                       /* Number of outstanding HTTP requests */
};

static void daemon_usage(char *argv0){
  fatal_error("Usage: %s daemon ?SWITCHES? CONTAINER [CONTAINER...]", argv0);
}

static void daemon_msg_log(const char *zFmt, ...){
  char *zMsg;
  va_list ap;
  va_start(ap, zFmt);
  zMsg = sqlite3_vmprintf(zFmt, ap);
  fprintf(stdout, "INFO(m): %s\n", zMsg);
  fflush(stdout);
  sqlite3_free(zMsg);
  va_end(ap);
}

static void daemon_event_log(DaemonCtx *p, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  if( p->cmd.mLog & BCV_LOG_EVENT ){
    char *zMsg = sqlite3_vmprintf(zFmt, ap);
    fprintf(stdout, "INFO(e): %s\n", zMsg);
    fflush(stdout);
    sqlite3_free(zMsg);
  }
  va_end(ap);
}

static void daemon_vlog(DaemonCtx *p, int flags, const char *zFmt, va_list ap){
  if( p->cmd.mLog & flags ){
    char *zMsg = sqlite3_vmprintf(zFmt, ap);
    fprintf(stdout, "INFO(%s%s%s%s%s): %s\n", 
        (flags & BCV_LOG_POLL ? "p" : ""),
        (flags & BCV_LOG_EVENT ? "e" : ""),
        (flags & BCV_LOG_MESSAGE ? "m" : ""),
        (flags & BCV_LOG_UPLOAD ? "u" : ""),
        (flags & BCV_LOG_CACHE ? "c" : ""),
        zMsg
    );
    fflush(stdout);
    sqlite3_free(zMsg);
  }
}

static void daemon_log(DaemonCtx *p, int flags, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  daemon_vlog(p, flags, zFmt, ap);
  va_end(ap);
}

static void daemon_error(const char *zFmt, ...){
  char *zMsg;
  va_list ap;
  va_start(ap, zFmt);
  zMsg = sqlite3_vmprintf(zFmt, ap);
  fprintf(stdout, "ERROR: %s\n", zMsg);
  fflush(stdout);
  sqlite3_free(zMsg);
  va_end(ap);
}

/*
** Start listening on a localhost port. Return the port number. 
*/
static int bcvDaemonListen(DaemonCtx *p){
  int nAttempt = p->cmd.iPort ? 1 : 1000;
  int i;
  struct sockaddr_in addr;
  int tr = 1;

  p->fdListen = socket(AF_INET, SOCK_STREAM, 0);
  if( bcv_socket_is_valid(p->fdListen)==0 ){
    fatal_system_error("listen()", errno);
  }
#ifndef __WIN32__
  setsockopt(p->fdListen, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int));
#endif

  for(i=0; i<nAttempt; i++){
    int iPort = p->cmd.iPort ? p->cmd.iPort : 22002+i;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(iPort);

    if( bind(p->fdListen, (struct sockaddr*)&addr, sizeof(addr))==0 ){
      if( listen(p->fdListen, 16)<0 ){
        fatal_system_error("listen()", errno);
      }else{
        daemon_event_log(p, "listening on localhost port %d", iPort);
        p->iListenPort = iPort;
        return iPort;
      }
    }
  }

  fatal_error("failed to bind to localhost port - tried %d to %d", 
      22002, 22002+nAttempt-1
  );
  return -1;
}

static void bcvDaemonClientClearRefs(DClient *pClient){
  if( pClient->apEntry ){
    int i;
    for(i=0; pClient->apEntry[i]; i++){
      pClient->apEntry[i]->nRef--;
    }
    sqlite3_free(pClient->apEntry);
    pClient->apEntry = 0;
  }
}

/*
** This is called by the daemon main loop when the client represented
** by pClient disconnects.
*/
static void bcvDaemonCloseConnection(DaemonCtx *p, DClient *pClient){
  DCurlRequest *pReq;
  DClient **pp;
  daemon_event_log(p, "client %d has disconnected", pClient->iId);
  bcv_close_socket(pClient->fd);

  /* Remove from client list */
  for(pp=&p->pClientList; *pp; pp=&(*pp)->pNext){
    if( *pp==pClient ){
      *pp = pClient->pNext;
      break;
    }
  }

  /* Remove from the waiting list of any ongoing downloads */
  for(pReq=p->pRequestList; pReq; pReq=pReq->pNext){
    if( pReq->pEntry ){
      for(pp=&pReq->pEntry->pWaiting; *pp; pp=&(*pp)->pNextWaiting){
        if( *pp==pClient ){
          *pp = pClient->pNextWaiting;
          break;
        }
      }
    }
  }

  bcvManifestDeref(pClient->pManifest);
  bcvDaemonClientClearRefs(pClient);
  sqlite3_free(pClient);
}

/*
** This is called by the daemon main loop whenever there is a new client
** connection.
*/
static void bcvDaemonNewConnection(DaemonCtx *p){
  struct sockaddr_in addr;        /* Address of new client */
  socklen_t len = sizeof(addr);   /* Size of addr in bytes */
  DClient *pNew;

  pNew = (DClient*)bcvMallocZero(sizeof(DClient));
  pNew->fd = accept(p->fdListen, (struct sockaddr*)&addr, &len);
  if( pNew->fd<0 ){
    fatal_system_error("accept()", errno);
  }
  pNew->iId = p->iNextId++;
  pNew->pNext = p->pClientList;
  p->pClientList = pNew;

  daemon_event_log(p, "new connection! (id=%d)", pNew->iId);
}

/*
** Find and return a pointer to the container named zCont. Or, if there
** is no such container, return NULL.
*/
static DContainer *bdFindContainer(DaemonCtx *pCtx, const char *zCont){
  DContainer *pCont;
  for(pCont=pCtx->pContainerList; pCont; pCont=pCont->pNext){
    if( 0==bcvStrcmp(pCont->zName, zCont) ) break;
  }
  return pCont;
}

static int bcvDaemonFindDb(DContainer *pCont, const u8 *aId){
  Manifest *p = pCont->pManifest;
  int ii;
  for(ii=0; ii<p->nDb; ii++){
    if( memcmp(aId, p->aDb[ii].aId, BCV_DBID_SIZE)==0 ){
      return ii;
    }
  }
  return -1;
}

/*
** Emit a log message explaining that parsing the message body failed.
*/
static void bdParseError(int eType, u8 *aBody, int nBody){
  static const char hex[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };
  char aBuf[80];
  int ii;

  daemon_error("parse error: etype=%d ('%c') size=%d", eType, eType, nBody);
  for(ii=0; ii<nBody; ii+=16){
    int iByte;
    memset(aBuf, 0, sizeof(aBuf));
    for(iByte=ii; iByte<ii+16 && iByte<nBody; iByte++){
      int iOff = (iByte-ii)*3;
      aBuf[iOff+0] = hex[ (aBody[iByte]>>4) & 0xF ];
      aBuf[iOff+1] = hex[ (aBody[iByte]) & 0xF ];
      aBuf[iOff+2] = ' ';
    }

    daemon_error("parse error: %s", aBuf);
  }
}

static int bcvDaemonParseMessage(
  u8 eType, 
  u8 *aBody, 
  int nBody,
  DMessage *pMsg
){
  int rc = 0;
  pMsg->eType = (int)eType;
  switch( eType ){
    case BCV_MESSAGE_LOGIN:
      if( aBody[nBody-1]!=0x00 ){
        rc = 1;
      }else{
        int ii;
        for(ii=0; ii<nBody; ii++){
          if( aBody[ii]=='/' ) break;
        }
        if( ii==nBody || ii==nBody-1 ){
          rc = 1;
        }else{
          pMsg->zContainer = bcvMprintf("%.*s", ii, (char*)aBody);
          pMsg->zDb = bcvMprintf("%s", (char*)&aBody[ii+1]);
        }
      }
      break;

    case BCV_MESSAGE_REQUEST:
      pMsg->iVal = (int)bcvGetU32(aBody);
      pMsg->nBlobByte = nBody-sizeof(u32);
      pMsg->aBlob = &aBody[sizeof(u32)];
      break;

    case BCV_MESSAGE_WDONE:
      break;

    case BCV_MESSAGE_UPLOAD:
      pMsg->iVal = (int)bcvGetU32(aBody);
      break;

    case BCV_MESSAGE_QUERY_REPLY:
    case BCV_MESSAGE_DONE:
    case BCV_MESSAGE_QUERY:
    case BCV_MESSAGE_WREQUEST:
      pMsg->nBlobByte = nBody;
      pMsg->aBlob = aBody;
      break;

    default:
      rc = 1;
      break;
  }

  if( rc ){
    bdParseError(eType, aBody, nBody);
  }
  return rc;
}

static void bcvDaemonParseFree(DMessage *pMsg){
  sqlite3_free(pMsg->zDb);
  sqlite3_free(pMsg->zContainer);
  memset(pMsg, 0, sizeof(DMessage));
}

static void bcvDaemonLogMessage(DaemonCtx *p, DClient *pClient, DMessage *pMsg){
  if( p->cmd.mLog & BCV_LOG_MESSAGE ){
    int iId = pClient->iId;
    switch( pMsg->eType ){
      case BCV_MESSAGE_LOGIN:
        daemon_msg_log("recv %d: LOGIN %s/%s", iId, pMsg->zContainer,pMsg->zDb);
        break;
      case BCV_MESSAGE_LOGIN_REPLY:
        daemon_msg_log("send %d: LOGIN_REPLY blocksize=%d", iId, pMsg->iVal);
        break;
      case BCV_MESSAGE_REQUEST_REPLY: {
        const char *zSep = "";
        char *aBuf = 0;
        int i;
        for(i=0; i<pMsg->nBlobByte; i+=sizeof(u32)){
          aBuf = bcvMprintf("%z%s%d", aBuf, zSep, bcvGetU32(&pMsg->aBlob[i]));
          zSep = ",";
        }
        daemon_msg_log("send %d: REQUEST_REPLY %z", iId, aBuf);
        break;
      }
      case BCV_MESSAGE_REQUEST:
      case BCV_MESSAGE_DONE: {
        const char *zSep = "";
        char *aBuf = 0;
        int i;
        for(i=0; i<pMsg->nBlobByte; i++){
          aBuf = bcvMprintf("%z%s%02X", aBuf, zSep, pMsg->aBlob[i]);
          zSep = " ";
        }
        if( pMsg->eType==BCV_MESSAGE_DONE ){
          daemon_msg_log("recv %d: DONE %z", iId, aBuf);
        }else{
          daemon_msg_log("recv %d: REQUEST block=%d %z", iId, pMsg->iVal, aBuf);
        }
        break;
      }
      case BCV_MESSAGE_UPLOAD_REPLY:
        daemon_msg_log("send %d: INTERNAL_REPLY", iId);
        break;
      case BCV_MESSAGE_ERROR_REPLY:
        daemon_msg_log("send %d: ERROR_REPLY \"%.*s\"", 
            iId, pMsg->nBlobByte, (char*)pMsg->aBlob
        );
        break;
      case BCV_MESSAGE_WDONE:
        daemon_msg_log("recv %d: WDONE", iId);
        break;
      case BCV_MESSAGE_UPLOAD:
        daemon_msg_log("recv %d: UPLOAD upload=%d", iId, pMsg->iVal);
        break;

      case BCV_MESSAGE_QUERY:
        daemon_msg_log("recv %d: QUERY \"%.*s\"", 
            iId, pMsg->nBlobByte, (char*)pMsg->aBlob
        );
        break;
      case BCV_MESSAGE_QUERY_REPLY:
        daemon_msg_log("send %d: QUERY_REPLY (%d bytes of data...)",
            iId, pMsg->nBlobByte
        );
        break;

      case BCV_MESSAGE_WREQUEST:
      case BCV_MESSAGE_WREQUEST_REPLY: {
        const char *zSep = "";
        char *zList = 0;
        int i;
        for(i=0; i<pMsg->nBlobByte; i+=4){
          int iBlk = (int)bcvGetU32(&pMsg->aBlob[i]);
          zList = bcvMprintf("%z%d%s", zList, iBlk, zSep);
          zSep = ", ";
        }
        daemon_msg_log("%s %d: %s (%z)", 
            pMsg->eType==BCV_MESSAGE_WREQUEST ? "recv" : "send", iId,
            pMsg->eType==BCV_MESSAGE_WREQUEST ? "WREQUEST" : "WREQUEST_REPLY",
            zList
        );
        break;
      }
      default:
        assert( 0 );
        break;
    }
  }
}

static int bdMessageSend(BCV_SOCKET_TYPE s, DMessage *pMsg){
  u8 *aMsg = 0;
  int nMsg = 0;
  int res;
  switch( pMsg->eType ){
    case BCV_MESSAGE_LOGIN_REPLY: {
      nMsg = 9;
      aMsg = (u8*)bcvMalloc(nMsg);
      bcvPutU32(&aMsg[5], (u32)pMsg->iVal);
      break;
    }
    case BCV_MESSAGE_ERROR_REPLY:
    case BCV_MESSAGE_QUERY_REPLY:
    case BCV_MESSAGE_QUERY:
    case BCV_MESSAGE_WREQUEST_REPLY: 
    case BCV_MESSAGE_REQUEST_REPLY: {
      nMsg = 5 + pMsg->nBlobByte;
      aMsg = (u8*)bcvMalloc(nMsg);
      memcpy(&aMsg[5], pMsg->aBlob, pMsg->nBlobByte);
      break;
    }
    case BCV_MESSAGE_UPLOAD_REPLY:
      nMsg = 5;
      aMsg = (u8*)bcvMalloc(nMsg);
      break;
    default:
      assert( 0 );
  }

  aMsg[0] = pMsg->eType;
  bcvPutU32(&aMsg[1], nMsg);
  res = send(s, aMsg, nMsg, 0); 
  sqlite3_free(aMsg);
  return (res!=nMsg);
}

static void bcvDaemonMessageSend(
  DaemonCtx *p, 
  DClient *pClient, 
  DMessage *pMsg
){
  int rc;
  bcvDaemonLogMessage(p, pClient, pMsg);
  rc = bdMessageSend(pClient->fd, pMsg);
  if( rc ){
    daemon_error(
        "failed to write send message to client %d - closing connection",
        pClient->iId
    );
    bcvDaemonCloseConnection(p, pClient);
  }
}

static char *bdIdToHex(u8 *aId, int nId){
  static char aBuf[100];
  hex_encode(aId, nId, aBuf);
  return aBuf;
}

static int bdCacheHashBucket(DaemonCtx *p, const u8 *pBlk){
  return (bcvGetU32(pBlk) & (p->nHash-1));
}

/*
** Remove the cache-entry object passed as the second argument from
** the hash table belonging to daemon p. This function segfaults
** if the cache-entry is not actually part of the hash table.
*/
static void bdCacheHashRemove(DaemonCtx *p, DCacheEntry *pEntry){
  int iHash = bdCacheHashBucket(p, pEntry->aName);
  DCacheEntry **pp;
  for(pp=&p->aHash[iHash]; *pp!=pEntry; pp=&(*pp)->pHashNext) assert( *pp );
  *pp = pEntry->pHashNext;
  pEntry->pHashNext = 0;
}

/*
** Add the entry passed as the second argument to the hash table 
** belonging to daemon p.
*/
static void bdCacheHashAdd(DaemonCtx *p, DCacheEntry *pEntry){
  int iHash = bdCacheHashBucket(p, pEntry->aName);
  pEntry->pHashNext = p->aHash[iHash];
  p->aHash[iHash] = pEntry;
}

/*
** Logging wrapper around bdCacheHashAdd() and bdCacheHashRemove().
** Messages are enabled using "-log c".
*/
static void bdCacheHashRemoveX(DaemonCtx *p, DCacheEntry *pEntry, int iLine){
  assert( pEntry->nName>=BCV_MIN_NAMEBYTES );
  daemon_log(p, BCV_LOG_CACHE, "removing %s from cache (line=%d)",
      bdIdToHex(pEntry->aName, pEntry->nName), iLine
  );
  bdCacheHashRemove(p, pEntry);
}
static void bdCacheHashAddX(DaemonCtx *p, DCacheEntry *pEntry, int iLine){
  assert( pEntry->nName>=BCV_MIN_NAMEBYTES );
  daemon_log(p, BCV_LOG_CACHE, "adding %s to cache (line=%d)",
      bdIdToHex(pEntry->aName, pEntry->nName), iLine
  );
  bdCacheHashAdd(p, pEntry);
}
#define bdCacheHashRemove(x,y) bdCacheHashRemoveX(x,y,__LINE__)
#define bdCacheHashAdd(x,y) bdCacheHashAddX(x,y,__LINE__)

/*
** Search the hash table that is part of DaemonCtx p for a block with the
** NAMEBYTES byte id passed as the second argument. Return
** NULL if not found.
*/
static DCacheEntry *bdCacheHashFind(DaemonCtx *p, const u8 *pBlk, int nBlk){
  int iHash;
  DCacheEntry *pEntry;

  iHash = bdCacheHashBucket(p, pBlk);
  assert( iHash>=0 );
  for(pEntry=p->aHash[iHash]; pEntry; pEntry=pEntry->pHashNext){
    if( pEntry->nName==nBlk && memcmp(pEntry->aName, pBlk, nBlk)==0 ){
      break;
    }
  }

  return pEntry;
}

/*
** Obtain a new CurlRequest handle.
*/
static DCurlRequest *bdCurlRequest(DaemonCtx *p){
  DCurlRequest *pNew = (DCurlRequest*)bcvMallocZero(sizeof(DCurlRequest));
  curlRequestInit(&pNew->req, p->cmd.bVerbose);
  return pNew;
}

/*
** Release a CurlRequest obtained from bdCurlRequest().
*/
static void bcvDaemonCurlRelease(DaemonCtx *p, DCurlRequest *pCurl){
  if( pCurl ){
    curlRequestFinalize(&pCurl->req);
    bcvManifestDeref(pCurl->pMan);
    sqlite3_free(pCurl->aBuf);
    sqlite3_free(pCurl);
  }
}

/*
** libcurl WRITEFUNCTION callback used for bdCurlMemoryDownload() downloads.
*/
static size_t bdCurlMemoryDownloadCb(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  DCurlRequest *pCurl = (DCurlRequest*)pCtx;
  int nData = nSize*nMember;

  while( pCurl->nBuf+nData>pCurl->nBufAlloc ){
    int nAlloc = pCurl->nBufAlloc ? pCurl->nBufAlloc*2 : 1024;
    pCurl->aBuf = (u8*)bcvRealloc(pCurl->aBuf, nAlloc);
    pCurl->nBufAlloc = nAlloc;
  }

  memcpy(&pCurl->aBuf[pCurl->nBuf], pData, nData);
  pCurl->nBuf += nData;

  return nData;
}

/*
** Configure the handle passed as the only argument to download to the
** DCurlRequest.aBuf/nBuf/nBufAlloc buffer.
*/
static void bdCurlMemoryDownload(DCurlRequest *pCurl){
  CURL *pHdl = pCurl->req.pCurl;
  curl_easy_setopt(pHdl, CURLOPT_WRITEFUNCTION, bdCurlMemoryDownloadCb);
  curl_easy_setopt(pHdl, CURLOPT_WRITEDATA, (void*)pCurl);
}

/*
** Append entry pEntry to the end of the LRU list (i.e. so that it is the 
** most-recently-used). It must not be part of the LRU list when this
** function is called.
*/
static void bcvDaemonAddToLRU(DaemonCtx *p, DCacheEntry *pEntry){
  assert( pEntry->pLruPrev==0 && pEntry->pLruNext==0 );
  assert( p->pLruFirst!=pEntry && p->pLruLast!=pEntry );

  assert( (p->pLruFirst==0)==(p->pLruLast==0) );
  if( p->pLruLast ){
    p->pLruLast->pLruNext = pEntry;
    pEntry->pLruPrev = p->pLruLast;
    p->pLruLast = pEntry;
  }else{
    p->pLruFirst = pEntry;
    p->pLruLast = pEntry;
  }
}

/*
** Remove entry pEntry from the LRU list.
*/
static void bcvDaemonRemoveFromLRU(DaemonCtx *p, DCacheEntry *pEntry){
  assert( p->pLruFirst && p->pLruLast );
  assert( pEntry==p->pLruLast || pEntry->pLruNext );
  assert( pEntry==p->pLruFirst || pEntry->pLruPrev );

  if( pEntry->pLruNext ){
    pEntry->pLruNext->pLruPrev = pEntry->pLruPrev;
  }else{
    p->pLruLast = pEntry->pLruPrev;
  }
  if( pEntry->pLruPrev ){
    pEntry->pLruPrev->pLruNext = pEntry->pLruNext;
  }else{
    p->pLruFirst = pEntry->pLruNext;
  }
  pEntry->pLruNext = 0;
  pEntry->pLruPrev = 0;

  assert( (p->pLruFirst==0)==(p->pLruLast==0) );
}

static sqlite3_stmt *bdGetInsert(DaemonCtx *p){
  if( p->pInsert==0 ){
    const char *zInsert = 
      "REPLACE INTO block"
      "(cachefilepos, blockid, dbpos, container, db, dbversion) "
      "VALUES(?, ?, ?, ?, ?, ?)";
    int rc = sqlite3_prepare_v2(p->db, zInsert, -1, &p->pInsert, 0);
    if( rc!=SQLITE_OK ){
      fatal_error("failed to prepare write-to-blocks stmt (rc=%d) (err=%s)", 
          rc, sqlite3_errmsg(p->db)
      );
    }
  }

  return p->pInsert;
}

static sqlite3_stmt *bdGetDeletePos(DaemonCtx *p){
  if( p->pDeletePos==0 ){
    const char *zDeletePos = "DELETE FROM block WHERE cachefilepos = ?";
    int rc = sqlite3_prepare_v2(p->db, zDeletePos, -1, &p->pDeletePos, 0);
    if( rc!=SQLITE_OK ){
      fatal_error("failed to prepare delete-from-blocks stmt (rc=%d) (err=%s)", 
          rc, sqlite3_errmsg(p->db)
      );
    }
  }

  return p->pDeletePos;
}

/*
** Add an entry to the "block" table in the db.
*/
static int bdWriteDirtyEntry(
  DaemonCtx *p,
  Manifest *pMan,
  int iCacheFilePos,
  int iDbPos,
  const char *zContainer,
  const u8 *aId,                  /* Database id */
  int iDbversion
){
  sqlite3_stmt *pInsert = bdGetInsert(p);
  sqlite3_bind_int(pInsert, 1, iCacheFilePos);
  sqlite3_bind_null(pInsert, 2);
  sqlite3_bind_int(pInsert, 3, iDbPos);
  sqlite3_bind_text(pInsert, 4, zContainer, -1, SQLITE_TRANSIENT);
  sqlite3_bind_blob(pInsert, 5, aId, BCV_DBID_SIZE, SQLITE_TRANSIENT);
  sqlite3_bind_int(pInsert, 6, iDbversion);
  sqlite3_step(pInsert);
  return sqlite3_reset(pInsert);
}

/*
** Write to the "block" table of the blocks database. This function is
** a no-op unless the "-persistent" option was passed to the daemon when
** it was started.
*/
static void bdWriteCleanEntry(
  DaemonCtx *p,
  int iCacheFilePos,
  const u8 *aName,
  int nName
){
  if( p->cmd.bPersistent ){
    sqlite3_stmt *pStmt = 0;
    if( aName ){
      pStmt = bdGetInsert(p);
      sqlite3_bind_blob(pStmt, 2, aName, nName, SQLITE_TRANSIENT);
      sqlite3_bind_null(pStmt, 3);
      sqlite3_bind_null(pStmt, 4);
      sqlite3_bind_null(pStmt, 5);
      sqlite3_bind_null(pStmt, 6);
    }else{
      pStmt = bdGetDeletePos(p);
    }
    sqlite3_bind_int(pStmt, 1, iCacheFilePos);
    sqlite3_step(pStmt);
    if( sqlite3_reset(pStmt)!=SQLITE_OK ){
      fatal_sql_error(p->db, "bdWriteCleanEntry()");
    }
  }
}

/*
** An entry is about to be added to the cache. Ensure there is a free
** slot for it.
*/
static void bcvDaemonFreeSlots(DaemonCtx *p){
  while( p->nCacheEntry>=p->nCacheMax ){
    DCacheEntry *pEntry;
    for(pEntry=p->pLruFirst; pEntry; pEntry=pEntry->pLruNext){
      if( pEntry->nRef==0 && pEntry->bPending==0 && pEntry->bDirty==0 ) break;
    }
    if( pEntry==0 ) break;

    /* Remove entry from LRU list */
    bcvDaemonRemoveFromLRU(p, pEntry);

    /* Remove entry from hash table. */
    bdWriteCleanEntry(p, pEntry->iPos, 0, 0);
    bdCacheHashRemove(p, pEntry);

    /* Mark slot as free and decrease cache entry count */
    p->aCacheUsed[pEntry->iPos / 32] &= ~(1 << (pEntry->iPos%32));
    p->nCacheEntry--;

    /* Free the entry structure */
    sqlite3_free(pEntry);
  }

  if( p->nCacheEntry>=(32*p->nCacheUsedEntry) ){
    int nEntry = p->nCacheUsedEntry;
    p->aCacheUsed = (u32*)bcvRealloc(p->aCacheUsed, (nEntry+1)*sizeof(u32));
    p->aCacheUsed[nEntry] = 0;
    p->nCacheUsedEntry += 1;
  }
}

/*
** Search for a free slot within the cache-file belonging to daemon p.
** Return the slot number, or -1 if one cannot be located. If a free slot
** is located, mark it as used before returning.
*/
static int bcvDaemonCacheSlot(DaemonCtx *p){
  int i;
  int j = 0;
  for(i=0; i<p->nCacheUsedEntry; i++){
    if( p->aCacheUsed[i]!=0xFFFFFFFF ){
      for(j=0; j<32; j++){
        if( (p->aCacheUsed[i] & (1<<j))==0 ){
          p->aCacheUsed[i] |= (1<<j);
          return i*32 + j;
        }
      }
    }
  }
  return -1;
}

static size_t bcvDaemonBlobCb(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  int nByte = nSize*nMember;
  DCurlRequest *pReq = (DCurlRequest*)pCtx;
  DaemonCtx *p = pReq->pDaemonCtx;
  sqlite3_file *pFile = p->pCacheFile;
  int rc;

  rc = pFile->pMethods->xWrite(pFile, pData, nByte, pReq->iOff);
  if( rc!=SQLITE_OK ){
    return 0;
  }
  pReq->iOff += nByte;
  return nByte;
}

static void bdCurlRun(
  DaemonCtx *p,
  DCurlRequest *pCurl, 
  void (*xDone)(DCurlRequest*, CURLMsg*)
){
  pCurl->pDaemonCtx = p;
  pCurl->xDone = xDone;
  pCurl->pNext = p->pRequestList;
  p->pRequestList = pCurl;
  curl_multi_add_handle(p->pMulti, pCurl->req.pCurl);
}

static void bcvDaemonProcessUsed(
  DaemonCtx *p, 
  DClient *pClient, 
  u8 *aUsed, 
  int nUsedByte
){
  int ii;
  ManifestDb *pDb = &pClient->pManifest->aDb[pClient->iDb];
  assert( pDb );
  assert( pDb->nBlkOrig<=pDb->nBlkLocal );
  assert( (pDb->nBlkLocalAlloc==0)==(pDb->aBlkOrig==pDb->aBlkLocal) );
  for(ii=0; ii<nUsedByte; ii++){
    u32 val = aUsed[ii];
    if( val ){
      int nNamebytes = NAMEBYTES(pClient->pManifest);
      int jj;
      for(jj=0; jj<8; jj++){
        int iBlk = ii*8 + jj;
        if( (val & (1<<jj)) && iBlk<pDb->nBlkOrig ){
          u8 *pBlk = &pDb->aBlkLocal[nNamebytes * ii];
          DCacheEntry *pEntry = bdCacheHashFind(p, pBlk, nNamebytes);
          if( pEntry ){
            bcvDaemonRemoveFromLRU(p, pEntry);
            bcvDaemonAddToLRU(p, pEntry);
          }
        }
      }
    }
  }
}

static void bcvDaemonRequestReply(DaemonCtx *p, DClient *pClient){
  int ii;
  int iEntry = 0;
  ManifestDb *pDb = &pClient->pManifest->aDb[pClient->iDb];
  int nNamebytes = NAMEBYTES(pClient->pManifest);
  DMessage reply;
  memset(&reply, 0, sizeof(DMessage));
  reply.eType = BCV_MESSAGE_REQUEST_REPLY;
  reply.nBlobByte = pDb->nBlkLocal * sizeof(u32);
  reply.aBlob = (u8*)bcvMallocZero(reply.nBlobByte);

  assert( pClient->apEntry==0 );
  pClient->apEntry = (DCacheEntry**)bcvMallocZero(
      (1+pDb->nBlkLocal)*sizeof(DCacheEntry*)
  );

  for(ii=0; ii<pDb->nBlkLocal; ii++){
    u8 *pBlk = &pDb->aBlkLocal[nNamebytes * ii];
    DCacheEntry *pEntry = bdCacheHashFind(p, pBlk, nNamebytes);
    if( pEntry ){
      bcvPutU32(&reply.aBlob[ii*sizeof(u32)], pEntry->iPos+1);
      pClient->apEntry[iEntry++] = pEntry;
      pEntry->nRef++;
    }
  }
  assert( pClient->apEntry[0] );

  bcvDaemonMessageSend(p, pClient, &reply);
  sqlite3_free(reply.aBlob);
}

/*
** This function does one of two things with cache entry (*ppEntry):
**
**   (a) copies the block-id in aNewid[] into (*ppEntry)->aName and
**       moves (*ppEntry) to the corresponding bucket within the
**       hash table, OR
**
**   (b) makes a copy of the entry's cache-file data to a new cache-file
**       block and allocates a new DCacheEntry block, with the name
**       set to aName[] to return via (*ppEntry).
**
** Option (b) is only required if there is currently a reference to 
** the cache-entry ((*ppEntry)->nRef>0) and the block is referenced more
** than once from within the current manifest.
**
** An SQLite error code is returned if an error occurs, or SQLITE_OK
** otherwise.
*/
static int bdRenameEntry(
  DaemonCtx *p,                   /* Daemon object */
  u8 *aNewid,                     /* New id */
  DCacheEntry **ppEntry           /* IN/OUT: Entry to rename */
){
  DCacheEntry *pEntry = *ppEntry;
  int rc = SQLITE_OK;

  if( pEntry->nRef ){
    sqlite3_file *pCache = p->pCacheFile;
    sqlite3_io_methods const *pMeth = pCache->pMethods;
    u8 *aBuf;
    int rc;
    DCacheEntry *pNew = bcvMallocZero(sizeof(DCacheEntry) + pEntry->nName);
    bcvDaemonFreeSlots(p);
    pNew->iPos = bcvDaemonCacheSlot(p);
    pNew->bDirty = 1;
    bcvDaemonAddToLRU(p, pNew);

    aBuf = bcvMalloc(p->szBlk);
    rc = pMeth->xRead(pCache, aBuf, p->szBlk, (i64)p->szBlk*pEntry->iPos);
    if( rc==SQLITE_OK ){
      rc = pMeth->xWrite(pCache, aBuf, p->szBlk, (i64)p->szBlk*pNew->iPos);
    }
    sqlite3_free(aBuf);

  }else{
    bdCacheHashRemove(p, pEntry);
  }

  memcpy(pEntry->aName, aNewid, pEntry->nName);
  bdCacheHashAdd(p, pEntry);
  return rc;
}

static void bdManifestWriteBlock(
  ManifestDb *pDb, 
  int iBlk, 
  const u8 *aName, 
  int nName
){
  /* Grow the manifest block array and add the new entry */
  while( pDb->nBlkLocalAlloc<=iBlk ){
    int nZero;
    assert( (pDb->nBlkLocalAlloc==0)==(pDb->aBlkOrig==pDb->aBlkLocal) );
    if( pDb->nBlkLocalAlloc==0 ){
      pDb->nBlkLocalAlloc = pDb->nBlkLocal*2;
      pDb->aBlkLocal = (u8*)bcvMallocZero(pDb->nBlkLocalAlloc*nName);
      memcpy(pDb->aBlkLocal, pDb->aBlkOrig, pDb->nBlkLocal*nName);
    }else{
      int nNew = pDb->nBlkLocalAlloc*2;
      pDb->nBlkLocalAlloc = nNew;
      pDb->aBlkLocal = (u8*)bcvRealloc(pDb->aBlkLocal, nNew*nName);
    }
    nZero = (pDb->nBlkLocalAlloc-pDb->nBlkLocal)*nName;
    memset(&pDb->aBlkLocal[pDb->nBlkLocal*nName], 0, nZero);
  }
  if( iBlk>=pDb->nBlkLocal ) pDb->nBlkLocal = iBlk + 1;
  memcpy(&pDb->aBlkLocal[iBlk*nName], aName, nName);
}

#if 0
static void bdLogLocalX(DaemonCtx *pCtx, Manifest *pMan, int iLine){
  int iDb;
  int nNamebytes = NAMEBYTES(pMan);
  for(iDb=0; iDb<pMan->nDb; iDb++){
    ManifestDb *pDb = &pMan->aDb[iDb];
    int i;
    for(i=0; i<pDb->nBlkLocal; i++){
      daemon_log(pCtx, BCV_LOG_CACHE,
          "%d: (%p) (%s) local block %d is: %s", iLine, (void*)pMan,
          pDb->zDName, i, bdIdToHex(&pDb->aBlkLocal[nNamebytes*i], nNamebytes)
      );
    }
  }
}
#define bdLogLocal(x,y) bdLogLocalX(x,y,__LINE__)
#endif

static void bdWRequestReply(DaemonCtx *p, DClient *pClient){
  Manifest *pMan = pClient->pContainer->pManifest;
  int nNamebytes = NAMEBYTES(pMan);
  ManifestDb *pDb;
  int iDb;
  DMessage reply;
  u8 aBuf[4];
  u8 *pBlk;
  DCacheEntry *pEntry;
  int rc = SQLITE_OK;

  assert( pClient->apEntry==0 );
  assert( pClient->pManifest==0 );
  assert( pClient->bWReq );
  pClient->bWReq = 0;

  iDb = bcvDaemonFindDb(pClient->pContainer, pClient->aId);
  if( iDb<0 ){
    bcvDaemonCloseConnection(p, pClient);
    return;
  }
  pDb = &pMan->aDb[iDb];

  pBlk = &pDb->aBlkLocal[nNamebytes * pClient->iWReqBlk];
  pEntry = bdCacheHashFind(p, pBlk, nNamebytes);
  assert( pEntry && pEntry->nName==nNamebytes );
  bcvPutU32(aBuf, pEntry->iPos+1);

  if( pEntry->bDirty==0 ){
    /* If the cache-entry is not already marked dirty (this is the case unless
    ** it was just appended to the file), then move it to a new, randomly
    ** generated block-id at this point. Write this new block-id into 
    ** the aBlkLocal[] array. Because it is about to be modified, the
    ** cache-entry can not continue to use its existing name - this could
    ** cause problems if the block is shared (or about to become shared). */
    u8 aNewid[BCV_MAX_NAMEBYTES];
    pEntry->bDirty = 1;
    sqlite3_randomness(nNamebytes, aNewid);
    rc = bdRenameEntry(p, aNewid, &pEntry);
    if( rc==SQLITE_OK ){
      rc = bdWriteDirtyEntry(
          p, pMan, pEntry->iPos, pClient->iWReqBlk, 
          pClient->pContainer->zName, pDb->aId, pDb->iVersion
      );
    }
    if( rc==SQLITE_OK ){
      bdManifestWriteBlock(pDb, pClient->iWReqBlk, aNewid, nNamebytes);
    }

    daemon_log(p, BCV_LOG_UPLOAD, 
        "marking %s as dirty", bdIdToHex(aNewid, NAMEBYTES(pMan))
    );
  }

  memset(&reply, 0, sizeof(DMessage));
  if( rc==SQLITE_OK ){
    reply.eType = BCV_MESSAGE_WREQUEST_REPLY;
    reply.nBlobByte = sizeof(aBuf);
    reply.aBlob = aBuf;
  }else{
    reply.eType = BCV_MESSAGE_ERROR_REPLY;
    reply.aBlob = (u8*)bcvMprintf("SQL error: %s", sqlite3_errmsg(p->db));
    reply.nBlobByte = strlen((char*)reply.aBlob);
  }

  bcvDaemonMessageSend(p, pClient, &reply);
}

/*
** The client passed as the second argument is waiting for a reply to
** either a REQUEST or WREQUEST message. This function is called to send
** the reply.
*/
static void bdRequestReply(DaemonCtx *p, DClient *pClient){
  if( pClient->bWReq==0 ){
    bcvDaemonRequestReply(p, pClient);
  }else{
    bdWRequestReply(p, pClient);
  }
}

static void bcvDaemonBlobDone(
  DCurlRequest *pCurl, 
  CURLMsg *pMsg 
){
  assert( pMsg->msg==CURLMSG_DONE );
  daemon_event_log(pCurl->pDaemonCtx, "blob done iOff=%d", (int)pCurl->iOff);
  if( pMsg->data.result==CURLE_OK ){
    DClient *pClient;
    DCacheEntry *pEntry = pCurl->pEntry;
    DaemonCtx *p = pCurl->pDaemonCtx;

    /* Send a REQUEST_REPLY message to each waiting client */
    for(pClient=pEntry->pWaiting; pClient; pClient=pClient->pNextWaiting){
      bdRequestReply(p, pClient);
    }
    pEntry->pWaiting = 0;
    pEntry->bPending = 0;

    bdWriteCleanEntry(p, pEntry->iPos, pEntry->aName, pEntry->nName);
  }
}

static int bcvDaemonDeleteBlockEntries(
  DaemonCtx *p,
  const char *zContainer,
  const u8 *aId
){
  int rc = SQLITE_OK;
  if( p->pDelete==0 ){
    rc = sqlite3_prepare_v2(p->db, 
        "DELETE FROM block WHERE container=? AND db=?", -1, &p->pDelete, 0
    );
  }

  if( rc==SQLITE_OK ){
    sqlite3_bind_text(p->pDelete, 1, zContainer, -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(p->pDelete, 2, aId, BCV_DBID_SIZE, SQLITE_TRANSIENT);
    sqlite3_step(p->pDelete);
    rc = sqlite3_reset(p->pDelete);
  }

  return rc;
}

static const char *bdDisplayName(DContainer *pContainer, const u8 *aId){
  int iDb = bcvDaemonFindDb(pContainer, aId);
  if( iDb<0 ) return "?unknown?";
  return pContainer->pManifest->aDb[iDb].zDName;
}

static void bcvDaemonNewBlock(
  DaemonCtx *p, 
  DClient *pClient, 
  int iBlk,
  DCacheEntry **ppEntry
){
  Manifest *pMan = pClient->pContainer->pManifest;
  int nNamebytes = NAMEBYTES(pMan);
  DCacheEntry *pNew = 0;
  u8 aName[BCV_MAX_NAMEBYTES];
  ManifestDb *pDb = &pMan->aDb[pClient->iDb];

  assert( iBlk==pDb->nBlkLocal );
  assert( pClient->pManifest==0 );
  assert( pDb->nBlkLocalAlloc==0 || pDb->nBlkLocalAlloc>=pDb->nBlkLocal );

  /* Allocate a new cache-entry object */
  sqlite3_randomness(nNamebytes, aName);
  pNew = (DCacheEntry*)bcvMallocZero(sizeof(DCacheEntry) + nNamebytes);
  pNew->nName = nNamebytes;
  memcpy(pNew->aName, aName, nNamebytes);

  /* Find a free cache-slot */
  bcvDaemonFreeSlots(p);
  pNew->iPos = bcvDaemonCacheSlot(p);
  assert( pNew->iPos>=0 );
  pNew->bDirty = 1;

  /* Grow the manifest block array and add the new entry */
  bdManifestWriteBlock(pDb, iBlk, aName, nNamebytes);

  /* Write a database entry for the new, dirty, block */
  bdWriteDirtyEntry(p, pMan, pNew->iPos, iBlk, 
      pClient->pContainer->zName, pClient->aId, pDb->iVersion
  );

  /* Add new cache-entry to the hash and LRU data structures */
  bcvDaemonAddToLRU(p, pNew);
  bdCacheHashAdd(p, pNew);
  p->nCacheEntry++;
  *ppEntry = pNew;

  daemon_event_log(p, "added block %d to database %s/%s", iBlk,
      pClient->pContainer->zName, 
      bdDisplayName(pClient->pContainer, pClient->aId)
  );
}

/*
** This function handles both REQUEST and WREQUEST messages.
*/
static void bdHandleClientRequest(
  DaemonCtx *p,                   /* Daemon object */
  DClient *pClient,               /* Client from which message was received */
  DMessage *pMsg                  /* REQUEST or WREQUEST message */
){
  DCacheEntry *pEntry = 0;
  u8 *pBlk;
  Manifest *pMan;
  ManifestDb *pDb;
  int nNamebytes;
  int iBlk;                       /* Index of requested block */
  int bWReq;                      /* True for WREQUEST message */

  assert( pMsg->eType==BCV_MESSAGE_REQUEST||pMsg->eType==BCV_MESSAGE_WREQUEST );
  if( pMsg->eType==BCV_MESSAGE_REQUEST ){
    bWReq = 0;
    iBlk = pMsg->iVal;
  }else{
    assert( pMsg->nBlobByte==4 );
    bWReq = 1;
    iBlk = bcvGetU32(pMsg->aBlob);
  }

  if( pClient->pManifest==0 ){
    int iDb = bcvDaemonFindDb(pClient->pContainer, pClient->aId);
    if( iDb<0 ){
      bcvDaemonCloseConnection(p, pClient);
      return;
    }
    pClient->iDb = iDb;
    if( bWReq ){
      pMan = pClient->pContainer->pManifest;
    }else{
      pClient->pManifest = bcvManifestRef(pClient->pContainer->pManifest);
      pMan = pClient->pManifest;
    }
  }else{
    pMan = pClient->pManifest;
  }
  pDb = &pMan->aDb[pClient->iDb];
  nNamebytes = NAMEBYTES(pMan);

  assert( bWReq==0 || pClient->apEntry==0 );
  if( bWReq==0 ){
    bcvDaemonClientClearRefs(pClient);
    bcvDaemonProcessUsed(p, pClient, pMsg->aBlob, pMsg->nBlobByte);
  }

  if( iBlk>=pDb->nBlkLocal ){
    assert( bWReq );
    bcvDaemonNewBlock(p, pClient, iBlk, &pEntry);
  }else{
    pBlk = &pDb->aBlkLocal[NAMEBYTES(pMan)*iBlk];
    pEntry = bdCacheHashFind(p, pBlk, nNamebytes);
  }
  
  pClient->bWReq = bWReq;
  pClient->iWReqBlk = iBlk;

  if( pEntry ){
    if( pEntry->bPending ){
      /* This blob is currently downloading */
      daemon_event_log(p,
          "client %d waiting on existing download", pClient->iId
      );
      pClient->pNextWaiting = pEntry->pWaiting;
      pEntry->pWaiting = pClient;
    }else{
      bdRequestReply(p, pClient);
    }
  }else{
    char aBuf[BCV_MAX_FSNAMEBYTES];
    DCurlRequest *pCurl = 0;

    /* Allocate a new cache-entry object */
    pEntry = (DCacheEntry*)bcvMallocZero(sizeof(DCacheEntry) + nNamebytes);
    pEntry->nName = nNamebytes;
    memcpy(pEntry->aName, pBlk, nNamebytes);

    /* Find a free cache-slot */
    bcvDaemonFreeSlots(p);
    pEntry->iPos = bcvDaemonCacheSlot(p);
    pEntry->bPending = 1;
    assert( pEntry->iPos>=0 );

    /* Add the DCacheEntry to the hash table */
    bdCacheHashAdd(p, pEntry);
    pClient->pNextWaiting = 0;
    pEntry->pWaiting = pClient;
    p->nCacheEntry++;

    /* Add the DCacheEntry to the end of the LRU list */
    bcvDaemonAddToLRU(p, pEntry);

    /* Configure a DCurlRequest to download the blob and write it into
    ** a free slot in the cache file. */
    pCurl = bdCurlRequest(p);
    bcvBlockidToText(pMan, pBlk, aBuf);
    curlFetchBlob(&pCurl->req, &p->ap, pClient->pContainer->zName, aBuf);
    curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEFUNCTION, bcvDaemonBlobCb);
    curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEDATA, (void*)pCurl);
    pCurl->iOff = (i64)pEntry->iPos * (i64)p->szBlk;
    pCurl->pEntry = pEntry;

    bdCurlRun(p, pCurl, bcvDaemonBlobDone);
  }
}

typedef struct BlockcacheFile BlockcacheFile;
struct BlockcacheFile {
  sqlite3_file base;
  int fdDaemon;
};

/*
** This function is the main() routine for an "upload thread". Whenever 
** the main daemon thread needs to upload a new version of a database, 
** it launches an upload thread. Which does the following:
**
**   1. Opens a database connection to the db being uploaded,
**
**   2. Runs a checkpoint on the db in a special mode such that
**      the CHECKPOINTER lock is not released when the checkpoint
**      is concluded. It is not released until the database connection
**      is closed.
**
**   3. Sends a message (via the localhost tcp/ip tsocket opened by the
**      database connection) to the main thread, indicating if the
**      checkpoint was both successful and complete (i.e. whether or
**      not all frames in the wal file were checkpointed). If it was,
**      then the main thread may safely upload the new version of
**      the database. Because this thread is holding the CHECKPOINTER
**      lock, no other database client can modify the database blocks
**      while the upload is ongoing.
**
**   4. Waits on a reply from the main thread, then exits.
**
** This task - taking the CHECKPOINTER lock and ensuring the database
** is fully checkpointed - could be done in the main thread. But using
** a second thread is convenient, as it can use a busy-timeout to wait
** on any required locks held by other database clients.
*/
#ifndef __WIN32__
static void *bcvUploadThread(void *pCtx){
#else
static void  bcvUploadThread(void *pCtx){
#endif
  DCheckpoint *p = (DCheckpoint*)pCtx;
  sqlite3 *db = 0;
  int rc = 0;
  int bBusy = 0;
  int nFrame = 0;
  int nCkpt = 0;
  int bUpload = 0;
  u8 aMsg[9];
  sqlite3_file *pFile;
  BCV_SOCKET_TYPE fdDaemon;
  int res;

  char *zFile = bcvMprintf("file:%s/%s?daemon=1", p->zContainer, p->zDb);

  rc = sqlite3_open_v2(zFile, &db, 
    SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI, "blockcachevfs"
  );
  if( rc==SQLITE_OK ){
    sqlite3_stmt *pStmt = 0;
    sqlite3_busy_timeout(db, 2000);
    rc = sqlite3_exec(db, "SELECT * FROM sqlite_master", 0, 0, 0);
    if( rc==SQLITE_OK ){
      rc = sqlite3_prepare_v2(db, 
          "PRAGMA wal_checkpoint = TRUNCATE", -1, &pStmt, 0
      );
    }
    if( rc==SQLITE_OK ){
      sqlite3_step(pStmt);
      bBusy = sqlite3_column_int(pStmt, 0);
      nFrame = sqlite3_column_int(pStmt, 1);
      nCkpt = sqlite3_column_int(pStmt, 2);
      rc = sqlite3_finalize(pStmt);
    }
  }

  bUpload = (rc==SQLITE_OK && bBusy==0 && nFrame==nCkpt);
  aMsg[0] = BCV_MESSAGE_UPLOAD;
  bcvPutU32(&aMsg[1], 9);
  bcvPutU32(&aMsg[5], bUpload);

  sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, (void*)&pFile);
  fdDaemon = ((BlockcacheFile*)pFile)->fdDaemon;

  res = send(fdDaemon, aMsg, sizeof(aMsg), 0);
  if( res==sizeof(aMsg) && bUpload ){
    res = recv(fdDaemon, aMsg, 5, 0);
  }

  sqlite3_free(zFile);
  sqlite3_close(db);
#ifndef __WIN32__
  return 0;
#endif
}

static void bdLaunchUpload(DContainer *pCont, DCheckpoint *pCkpt){
#ifdef __WIN32__
  uintptr_t ret;
  ret = _beginthread(bcvUploadThread, 0, (void*)pCkpt);
  if( ret<0 ){
    fatal_system_error("_beginthread()", ret);
  }
#else
  int ret;
  pthread_t tid;
  memset(&tid, 0, sizeof(tid));
  ret = pthread_create(&tid, NULL, bcvUploadThread, (void*)pCkpt);
  if( ret!=0 ){
    fatal_system_error("pthread_create()", ret);
  }
  pthread_detach(tid);
#endif

  assert( pCont->eOp==CONTAINER_OP_NONE );
  pCont->eOp = CONTAINER_OP_UPLOAD;
}

static DCheckpoint *bdFindOrCreateCheckpoint(DaemonCtx *p, DClient *pClient){
  DContainer *pCont = pClient->pContainer;
  const char *zCont = pCont->zName;
  DCheckpoint *pNew;
  int nByte;
  int nCont = strlen(zCont);
  const char *zDb = bdDisplayName(pCont, pClient->aId);
  int nDb = strlen(zDb);

  assert( pClient->pManifest==0 );
  assert( pClient->apEntry==0 );

  /* See if there is already an ongoing upload on this database. If so,
  ** return early without launching a checkpoint thread.  */
  for(pNew=pCont->pCheckpointList; pNew; pNew=pNew->pNext){
    if( memcmp(pClient->aId, pNew->aId, BCV_DBID_SIZE)==0 ) break;
  }
  if( pNew==0 ){
    nByte = sizeof(DCheckpoint) + nCont + nDb + 2;
    pNew = (DCheckpoint*)bcvMallocZero(nByte);
    pNew->zContainer = (char*)&pNew[1];
    pNew->zDb = &pNew->zContainer[nCont+1];
    memcpy(pNew->aId, pClient->aId, BCV_DBID_SIZE);
    memcpy(pNew->zContainer, zCont, nCont);
    memcpy(pNew->zDb, zDb, nDb);
    pNew->pNext = pCont->pCheckpointList;
    pNew->iRetryTime = sqlite_timestamp();
    pCont->pCheckpointList = pNew;

    daemon_log(p, BCV_LOG_UPLOAD, "created DCheckpoint object for %s/%s (%s)",
        pNew->zContainer, pNew->zDb, bdIdToHex(pNew->aId, BCV_DBID_SIZE)
    );
  }

  return pNew;
}

static void bdUploadBlockDone(DCurlRequest*, CURLMsg*);

static int bcvDaemonCachefileSize(DaemonCtx *p, i64 *psz){
  int rc = p->pCacheFile->pMethods->xFileSize(p->pCacheFile, psz);
  return rc;
}

/*
** libcurl xRead and xSeek callbacks for in-memory upload requests.
*/
static size_t bdMemoryRead(char *pBuffer, size_t n1, size_t n2, void *pCtx){
  DCurlRequest *p = (DCurlRequest*)pCtx;
  size_t nReq = MIN(n1*n2, p->nBuf-p->iOff);
  memcpy(pBuffer, &p->aBuf[p->iOff], nReq);
  p->iOff += nReq;
  return nReq;
}
static size_t bdMemorySeek(void *pCtx, curl_off_t offset, int origin){
  DCurlRequest *p = (DCurlRequest*)pCtx;
  assert( origin==SEEK_SET );
  p->iOff = offset;
  return CURL_SEEKFUNC_OK;
}

static int bdUploadBlock(DUpload *p){
  int rc = SQLITE_OK;
  assert( p->bDone==0 );
  if( sqlite3_step(p->pRead)==SQLITE_ROW ){
    const char *zCont = p->pContainer->zName;
    Manifest *pMan = p->pManifest;
    int nNamebytes = NAMEBYTES(pMan);
    char zFile[BCV_MAX_FSNAMEBYTES];   /* Filename to upload to */
    u8 *aPermid;                       /* Permanent block-id to use */
    u8 *aLocalid;                      /* Id of block in aBlkLocal[] */
    DaemonCtx *pCtx = p->pCtx;
    ManifestDb *pDb = &pMan->aDb[p->iDb];
    int iDbPos = sqlite3_column_int(p->pRead, 1);
    DCurlRequest *pCurl = bdCurlRequest(pCtx);
    i64 szFile = 0;                    /* Size of cache-file in bytes */

    assert( pDb->aBlkLocal!=pDb->aBlkOrig && pDb->nBlkLocalAlloc );
    aLocalid = &pDb->aBlkLocal[iDbPos*nNamebytes];
    aPermid = &pDb->aBlkOrig[iDbPos*nNamebytes];

    /* If this block clobbers one that is part of the aBlkOrig[] array,
    ** add the aBlkOrig[] entry to the delete-list of the manifest file.
    ** The aDelBlk[] allocation is guaranteed to be large enough. Except -
    ** do not free the block if it is also being used by some other 
    ** database.  */
    if( iDbPos<pDb->nBlkOrig ){
      const char *zLog = 0;
      if( 0==bcvMHashMatch(p->pMHash, aPermid, nNamebytes) ){
        zLog = "adding block %s to delete-list";
        u8 *aGC = &pMan->aDelBlk[pMan->nDelBlk*GCENTRYBYTES(pMan)];
        memcpy(aGC, aPermid, nNamebytes);
        pMan->nDelBlk++;
      }else{
        zLog = "not adding block %s to delete-list (still in use)";
      }
      if( pCtx->cmd.mLog & BCV_LOG_UPLOAD ){
        bcvBlockidToText(pMan, aPermid, zFile);
        daemon_log(pCtx, BCV_LOG_UPLOAD, zLog, zFile);
      }
    }

    rc = bcvDaemonCachefileSize(pCtx, &szFile);
    if( rc==SQLITE_OK ){
      u8 *pMatch = 0;
      sqlite3_file *pCache = pCtx->pCacheFile;
      i64 iOff;

      pCurl->pEntry = bdCacheHashFind(pCtx, aLocalid, nNamebytes);
      if( ((1+pCurl->pEntry->iPos)*pCtx->szBlk)>szFile ){
        pCurl->nBuf = szFile % pCtx->szBlk;
      }else{
        pCurl->nBuf = pCtx->szBlk;
      }

      pCurl->aBuf = bcvMalloc(pCurl->nBuf);
      iOff = pCurl->pEntry->iPos*pCtx->szBlk;
      rc = pCache->pMethods->xRead(pCache, pCurl->aBuf, pCurl->nBuf, iOff);
      assert( rc==SQLITE_OK );

      /* Generate a new, permanent, block-id to upload the block to. */
      sqlite3_randomness(nNamebytes, aPermid);
      if( p->pMHash ){
        assert( MD5_DIGEST_LENGTH==16 );
        MD5(pCurl->aBuf, pCurl->nBuf, aPermid);
        pMatch = bcvMHashMatch(p->pMHash, aPermid, MD5_DIGEST_LENGTH);
        if( pMatch ){
          memcpy(aPermid, pMatch, NAMEBYTES(pMan));
          bcvDaemonCurlRelease(pCtx, pCurl);
        }
      }

      if( pMatch==0 ){
        bcvBlockidToText(pMan, aPermid, zFile);
        pCurl->pUpload = p;
        curlPutBlob(&pCurl->req, &pCtx->ap, zCont, zFile, 0, pCurl->nBuf);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READFUNCTION, bdMemoryRead);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READDATA, (void*)pCurl);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKFUNCTION, bdMemorySeek);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKDATA, (void*)pCurl);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEFUNCTION, bdIgnore);
        p->nRef++;
        bdCurlRun(pCtx, pCurl, bdUploadBlockDone);
      }

      if( pCtx->cmd.mLog & BCV_LOG_UPLOAD ){
        bcvBlockidToText(pMan, aPermid, zFile);
        daemon_log(pCtx, BCV_LOG_UPLOAD, pMatch ? 
            "reusing block %s in db %s/%s" :
            "uploading new block %s for db %s/%s",
            zFile, p->pContainer->zName, 
            bdDisplayName(p->pContainer, p->pClient->aId)
        );
      }
    }
  }else{
    rc = sqlite3_reset(p->pRead);
    p->bDone = 1;
  }
  return rc;
}

/*
** Arguments pReq and pMsg are as passed to a bdCurlRun() callback. The
** expected http response code for a successful operation is c1. If it is
** not zero, then c2 is a second http response code that is also not
** considered an error. For example, when deleting a block blob the expected
** code for success is 202 (Accepted), but 404 (Not Found) is not considered
** an error.
**
** If there has been an error, either at the libcurl level or in the sense
** that the http response code is not as expected, log an error message and
** return non-zero. If there has been no error, this function returns zero.
*/
static int bdIsHttpError(DCurlRequest *pReq, CURLMsg *pMsg, int c1, int c2){
  if( pMsg->data.result!=CURLE_OK ){
    daemon_error("error in curl request - %s", 
        curl_easy_strerror(pMsg->data.result)
    );
    return 1;
  }else{
    long httpcode;
    curl_easy_getinfo(pReq->req.pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
    if( httpcode!=c1 && httpcode!=c2 ){
      char *zExtra = 0;
      char *zUrl = 0;
      if( c2 ) zExtra = bcvMprintf(" or %d", c2);
      daemon_error(
          "error in http request - response code is %d, expected %d%s",
          (int)httpcode, c1, zExtra?zExtra:""
      );
      daemon_error("http status: %s", pReq->req.zStatus);
      curl_easy_getinfo(pReq->req.pCurl, CURLINFO_EFFECTIVE_URL, &zUrl);
      daemon_error("http uri: %s", zUrl);
      sqlite3_free(zExtra);
      return 1;
    }
  }
  return 0;
}

static void bcvDaemonUploadFree(DUpload *pUpload){
  bcvManifestDeref(pUpload->pManifest);
  bcvMHashFree(pUpload->pMHash);
  sqlite3_finalize(pUpload->pRead);
  sqlite3_free(pUpload);
}

static void bdRemoveDCheckpoint(
  DaemonCtx *p,
  DContainer *pCont,
  const u8 *aId
){
  DCheckpoint **pp;
  for(pp=&pCont->pCheckpointList; *pp; pp=&(*pp)->pNext){
    if( memcmp(aId, (*pp)->aId, BCV_DBID_SIZE)==0 ){
      DCheckpoint *pFree = *pp;
      DClient *pClient;
      daemon_log(p, BCV_LOG_UPLOAD, "deleting DCheckpoint for %s/%s (%s)",
          pFree->zContainer, pFree->zDb, bdIdToHex(pFree->aId, BCV_DBID_SIZE)
      );
      *pp = pFree->pNext;

      /* Send replies to any "PRAGMA bcv_upload" clients */
      for(pClient=pFree->pWaiting; pClient; pClient=pClient->pNextWaiting){
        DMessage reply;
        memset(&reply, 0, sizeof(DMessage));
        reply.eType = BCV_MESSAGE_QUERY_REPLY;
        reply.aBlob = (u8*)"ok";
        reply.nBlobByte = 2;
        bcvDaemonMessageSend(p, pClient, &reply);
      }

      sqlite3_free(pFree);
      return;
    }
  }
  assert( 0 );
}

/*
** Use manifest pMan for container pCont.
**
** In practice, this means:
**
**   * Decrementing the ref count on pCont->pManifest,
**   * Setting pCont->iDeleteTime to the earliest time when an entry on
**     the GC list of the new manifest needs to be deleted,
**   * Setting pCont->iPollTime to the time to next poll the manifest,
**   * Setting pCont->pManifest to point to the new object.
*/
static void bdManifestInstall(
  DaemonCtx *p,                   /* Main application object */
  DContainer *pCont,              /* Container to update manifest of */
  Manifest *pMan,                 /* Manifest object to install */
  u8 *aSerial,                    /* Buffer containing serialized manifest */
  int nSerial                     /* Size of buffer aSerialized[] in bytes */
){
  int i;

  if( aSerial ){
    int rc;
    if( p->pWriteMan==0 ){
      rc = sqlite3_prepare_v2(p->db, 
          "REPLACE INTO manifest VALUES(?, ?, ?, ?)", -1, &p->pWriteMan, 0
      );
      if( rc!=SQLITE_OK ){
        fatal_error("failed to prepare write-to-manifest stmt (rc=%d)", rc);
      }
    }

    sqlite3_bind_text(p->pWriteMan, 1, p->cmd.zAccount, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(p->pWriteMan, 2, pCont->zName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(p->pWriteMan, 3, aSerial, nSerial, SQLITE_TRANSIENT);
    sqlite3_bind_text(p->pWriteMan, 4, pMan->zETag, -1, SQLITE_TRANSIENT);
    sqlite3_step(p->pWriteMan);
    rc = sqlite3_reset(p->pWriteMan);
    if( rc!=SQLITE_OK ){
      fatal_error("failed to write to manifest table (%d)", rc);
    }
  }

  bcvManifestDeref(pCont->pManifest);
  pCont->iDeleteTime = 0;
  if( p->cmd.bNodelete==0 ){
    for(i=0; i<pMan->nDelBlk; i++){
      int iOff = i*GCENTRYBYTES(pMan) + NAMEBYTES(pMan);
      i64 iTime = (i64)bcvGetU64(&pMan->aDelBlk[iOff]);
      if( pCont->iDeleteTime==0 || iTime<pCont->iDeleteTime ){
        pCont->iDeleteTime = MAX(iTime, 1);
      }
    }
  }

  pCont->iPollTime = sqlite_timestamp() + p->cmd.nPollTime+1000;
  pCont->pManifest = pMan;
}

static void bdPollManifest(DaemonCtx *p, DContainer *pCont);

static void bdFinishUpload(DCurlRequest *p, CURLMsg *pMsg, int bFail){
  DaemonCtx *pCtx = p->pDaemonCtx;
  int rc;
  DMessage reply;
  DUpload *pUpload = p->pUpload;

  daemon_log(pCtx, BCV_LOG_UPLOAD, "manifest upload done (%s)",
    bFail ? "failed... download new manifest then retry upload" : "success"
  );

  if( bFail==0 ){
    int nNamebytes = NAMEBYTES(pUpload->pManifest);
    ManifestDb *pDb = &pUpload->pManifest->aDb[pUpload->iDb];

    /* Move all cache entries to their permanent names. And clear the
    ** bDirty flags. */
    while( SQLITE_ROW==sqlite3_step(pUpload->pRead) ){
      int iDbPos = sqlite3_column_int(pUpload->pRead, 1);
      const u8 *aOldid = &pDb->aBlkLocal[iDbPos*nNamebytes];
      const u8 *aNewid = &pDb->aBlkOrig[iDbPos*nNamebytes];
      DCacheEntry *pEntry;

      pEntry = bdCacheHashFind(pCtx, aOldid, nNamebytes);
      assert( pEntry && pEntry->bDirty );
      pEntry->bDirty = 0;

      bdCacheHashRemove(pCtx, pEntry);
      memcpy(pEntry->aName, aNewid, nNamebytes);
      bdCacheHashAdd(pCtx, pEntry);

      bdWriteCleanEntry(pCtx, pEntry->iPos, pEntry->aName, pEntry->nName);
    }
    rc = sqlite3_finalize(pUpload->pRead);
    pUpload->pRead = 0;

    if( rc==SQLITE_OK ){
      const char *zCont = pUpload->pContainer->zName;
      rc = bcvDaemonDeleteBlockEntries(pCtx, zCont, pUpload->pClient->aId);
    }
    assert( rc==SQLITE_OK );

    assert( pDb->nBlkLocalAlloc>0 );
    sqlite3_free(pDb->aBlkLocal);
    pDb->nBlkLocalAlloc = 0;
    pDb->nBlkLocal = pDb->nBlkOrig;
    pDb->aBlkLocal = pDb->aBlkOrig;

    /* Remove the DCheckpoint object from the list. */
    bdRemoveDCheckpoint(pCtx, pUpload->pContainer, pUpload->pClient->aId);
  }else{
    bdPollManifest(pCtx, pUpload->pContainer);
  }

  /* Reply to the internal client */
  memset(&reply, 0, sizeof(DMessage));
  reply.eType = BCV_MESSAGE_UPLOAD_REPLY;
  bcvDaemonMessageSend(pCtx, pUpload->pClient, &reply);

  /* Free whatever is left of the DUpload object */
  bcvDaemonUploadFree(pUpload);
}

static void bdDeleteFree(DDelete *p){
  bcvManifestDeref(p->pMan);
  sqlite3_free(p);
}

static void bdCollectFree(DCollect *p){
  bcvManifestDeref(p->pMan);
  sqlite3_free(p->aDelete);
  sqlite3_free(p);
}

/*
** This is the final callback for all "upload-manifest" operations. A daemon
** may have uploaded a new manifest because:
**
**   1) A new version of a database was just uploaded.
**   2) Blocks from the aDelBlk[] list have been deleted.
**   3) Stray blocks found in cloud storage have been added to aDelBlk[].
**
** If the upload succeeded, then the manifest object just uploaded is 
** installed as the new manifest for the associated container.
*/
static void bdUploadManifestDone(DCurlRequest *p, CURLMsg *pMsg){
  Manifest *pMan = p->pMan;
  DaemonCtx *pCtx = p->pDaemonCtx;
  int bFail;

  bFail = bdIsHttpError(p, pMsg, 201, 0);
  daemon_event_log(pCtx, 
      "done uploading manifest if-match=%s new-etag=%s reason=%s bFail=%d", 
      pMan->zETag, (bFail ? "n/a" : p->req.zETag), 
      p->pUpload ? "upload" : (p->pDelete ? "delete" : "collect"),
      bFail
  );

  if( bFail==0 ){
    int i;
    Manifest *pOld = p->pContainer->pManifest;

    for(i=0; i<pMan->nDb; i++){
      if( p->pUpload==0 || i!=p->pUpload->iDb ){
        ManifestDb *pn = &pMan->aDb[i];
        ManifestDb *po = &pOld->aDb[i];
        if( po->nBlkLocalAlloc ){
          if( pn->nBlkLocalAlloc ){
            sqlite3_free(pn->aBlkLocal);
          }
          pn->nBlkLocalAlloc = pn->nBlkLocal = po->nBlkLocal;
          pn->aBlkLocal = (u8*)bcvMalloc(pn->nBlkLocal * NAMEBYTES(pMan));
          memcpy(pn->aBlkLocal, po->aBlkLocal, pn->nBlkLocal * NAMEBYTES(pMan));
        }
      }
    }

    /* Update the zETag field on the Manifest just uploaded. Then replace
    ** the main manifest for this container with the new one.  */
    sqlite3_free(pMan->zETag);
    pMan->zETag = p->req.zETag;
    p->req.zETag = 0;
    bdManifestInstall(pCtx, p->pContainer, pMan, p->aBuf, p->nBuf);
    p->pMan = 0;
  }

  assert( (p->pContainer->eOp==CONTAINER_OP_UPLOAD && p->pUpload)
       || (p->pContainer->eOp==CONTAINER_OP_DELETE && p->pDelete)
       || (p->pContainer->eOp==CONTAINER_OP_COLLECT && p->pCollect)
  );
  p->pContainer->eOp = CONTAINER_OP_NONE;
  
  if( p->pUpload ){
    bdFinishUpload(p, pMsg, bFail);
  }

  if( p->pDelete ){
    /* If the operation could not be completed, schedule it to be retried
    ** in nRetryTime seconds.  */
    if( bFail ){
      DContainer *pCont = p->pDelete->pContainer;
      pCont->iDeleteTime = sqlite_timestamp() + pCtx->cmd.nRetryTime*1000;
    }
    bdDeleteFree(p->pDelete);
    p->pDelete = 0;
  }

  if( p->pCollect ){
    DContainer *pCont = p->pCollect->pContainer;
    if( bFail ){
      pCont->iGCTime = sqlite_timestamp() + pCtx->cmd.nRetryTime*1000;
    }else{
      pCont->iGCTime = sqlite_timestamp() + pCtx->cmd.nGCTime*1000;
    }
    bdCollectFree(p->pCollect);
    p->pCollect = 0;
  }
}

static DCurlRequest *bdUploadManifestRequest(
  DaemonCtx *p, 
  int logflags,
  DContainer *pContainer,
  Manifest *pMan
){
  DCurlRequest *pCurl = bdCurlRequest(p);
  pCurl->pContainer = pContainer;
  pCurl->aBuf = bcvManifestCompose(pMan, &pCurl->nBuf);
  pCurl->pMan = pMan;
  bcvManifestRef(pMan);

  curlPutBlob(&pCurl->req, &p->ap, 
      pContainer->zName, BCV_MANIFEST_FILE, pMan->zETag, pCurl->nBuf
  );
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READFUNCTION, bdMemoryRead);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READDATA, (void*)pCurl);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKFUNCTION, bdMemorySeek);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKDATA, (void*)pCurl);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEFUNCTION, bdIgnore);

  daemon_log(p, logflags, "uploading manifest if-match=%s", pMan->zETag);
  return pCurl;
}

static void bdUploadBlockDone(DCurlRequest *pCurl, CURLMsg *pMsg){
  int rc = SQLITE_OK;
  DUpload *p = pCurl->pUpload;
  Manifest *pMan = p->pManifest;
  int bFail;
  
  assert( p->pContainer->eOp==CONTAINER_OP_UPLOAD );
  p->nRef--;
  bFail = bdIsHttpError(pCurl, pMsg, 201, 0);
  daemon_log(p->pCtx, BCV_LOG_UPLOAD, 
      "done uploading blob for %s/%s (bFail=%d)", p->pContainer->zName, 
      bdDisplayName(p->pContainer, p->pClient->aId), bFail
  );
  if( bFail ) p->bError = 1;

  /* If there are still blocks to upload, kick off another HTTP request */
  while( p->bDone==0 && p->bError==0 && p->nRef<p->pCtx->cmd.nWrite ){
    rc = bdUploadBlock(p);
    assert( rc==SQLITE_OK );
  }

  /* If all blocks have been uploaded successfully, upload the new manifest.
  ** The database version has already been incremented.  */
  if( p->nRef==0 ){
    if( p->bError ){
      DMessage reply;
      memset(&reply, 0, sizeof(DMessage));
      reply.eType = BCV_MESSAGE_UPLOAD_REPLY;
      bcvDaemonMessageSend(p->pCtx, p->pClient, &reply);
      p->pContainer->eOp = CONTAINER_OP_NONE;
      bcvDaemonUploadFree(p);
    }else{
      int i;
      DCurlRequest *pReq;
      DaemonCtx *pCtx = pCurl->pDaemonCtx;
      ManifestDb *pDb = &pMan->aDb[p->iDb];

      /* Fill in the timestamps for all blocks in the delete-list added by
      ** this upload.  */
      i64 tm = sqlite_timestamp() + pCtx->cmd.nDeleteTime*1000;
      for(i=p->nGCOrig; i<pMan->nDelBlk; i++){
        u8 *aGC = &pMan->aDelBlk[i*GCENTRYBYTES(pMan)];
        bcvPutU64(&aGC[NAMEBYTES(pMan)], (u64)tm);
      }

      /* Set the ManifestDb.nBlkOrig value, which was not set earlier */
      pDb->nBlkOrig = pDb->nBlkLocal;

      pReq = bdUploadManifestRequest(pCtx, BCV_LOG_UPLOAD, p->pContainer, pMan);
      pReq->pUpload = p;
      bdCurlRun(pCtx, pReq, bdUploadManifestDone);
    }
  }
}

/*
** Process a BCV_MESSAGE_UPLOAD message received from a client (always
** an upload thread in this process).
*/
static void bdClientUpload(
  DaemonCtx *p, 
  DClient *pClient,
  DMessage *pMsg
){
  int bReschedule = 0;
  int bSendReply = 0;
  DContainer *pCont = pClient->pContainer;
  u8 *aId = pClient->aId;
  int iDb = bcvDaemonFindDb(pCont, aId);

  daemon_log(p, BCV_LOG_UPLOAD, 
      "processing UPLOAD message for %s/%s (bUpload=%d)",
      pCont->zName, bdDisplayName(pCont, aId), pMsg->iVal
  );
  assert( pMsg->eType==BCV_MESSAGE_UPLOAD );
  assert( pCont->eOp==CONTAINER_OP_UPLOAD );

  if( pCont->pManifest->aDb[iDb].nBlkLocalAlloc==0 ){
    /* There are no local changes to upload. Or they have already been
    ** uploaded. Remove the DCheckpoint object so that the upload is not 
    ** retried and reply to the upload thread.  */
    bdRemoveDCheckpoint(p, pCont, aId);
    bSendReply = 1;
  }else if( pMsg->iVal==0 ){
    /* The checkpoint thread could not ensure that the entire wal is 
    ** checkpointed (presumably due to read locks). Reply to the checkpoint
    ** thread. The whole upload will be retried again in -retrytime 
    ** seconds. */
    bSendReply = 1;
    bReschedule = 1;
  }else{
    int rc;
    DUpload *pNew = (DUpload*)bcvMallocZero(sizeof(DUpload));
    Manifest *pMan = 0;
    ManifestDb *pDb = 0;

    pNew->pClient = pClient;
    pNew->pCtx = p;
    pNew->pContainer = pClient->pContainer;
    pNew->iDb = iDb;

    /* Create a query to iterate through the set of dirty blocks for
    ** this database. */
    rc = sqlite3_prepare_v2(p->db, 
        "SELECT cachefilepos, dbpos FROM block WHERE container=? AND db=?",
        -1, &pNew->pRead, 0
    );

    if( rc==SQLITE_OK ){
      int iBlk;
      DCacheEntry *pEntry = 0;
      int nByte;                  /* Bytes of space to allocate */
      const char *zCont = pNew->pContainer->zName;
      sqlite3_bind_text(pNew->pRead, 1, zCont, -1, SQLITE_TRANSIENT);
      sqlite3_bind_blob(pNew->pRead, 2, aId, BCV_DBID_SIZE, SQLITE_TRANSIENT);

      pMan = pNew->pManifest = bcvManifestDup(pClient->pContainer->pManifest);
      pNew->pMHash = bcvMHashBuild(pMan, iDb);
      pDb = &pMan->aDb[iDb];
      pDb->iVersion++;

      /* Check if the first block of the db is in the cache. If it is, read
      ** the file-size field from the database header. Use this to reduce
      ** the value of pDb->nBlkLocal if possible. */
      pEntry = bdCacheHashFind(p, pDb->aBlkLocal, NAMEBYTES(pMan));
      if( pEntry ){
        sqlite3_file *pCache = p->pCacheFile;
        u8 a[32];
        rc = pCache->pMethods->xRead(pCache, a, 32, p->szBlk*pEntry->iPos);
        if( rc==SQLITE_OK ){
          u32 pgsz = bcvGetU16(&a[16]);
          int nPagePerBlock = (p->szBlk / (pgsz==1 ? 65536 : pgsz));
          int nBlk;
          nBlk = (bcvGetU32(&a[28]) + nPagePerBlock - 1) / nPagePerBlock;
          if( nBlk<pDb->nBlkLocal ){
            pDb->nBlkLocal = nBlk;
          }
        }
      }

      /* Ensure the aBlkOrig[] array is large enough to update with the
      ** new block names (possibly based on a hash of the block content) as
      ** blocks are uploaded. pDb->nBlkOrig is not updated until the 
      ** manifest file is about to be uploaded, it is required until then
      ** to tell which blocks from aBlkOrig[] should be added to the
      ** delete-list.  */
      assert( pDb->bBlkOrigFree );          /* because bcvManifestDup() */
      if( pDb->nBlkOrig<pDb->nBlkLocal ){
        nByte = pDb->nBlkLocal*NAMEBYTES(pMan);
        pDb->aBlkOrig = (u8*)bcvRealloc(pDb->aBlkOrig, nByte);
      }

      /* Ensure that the Manifest.aDelBlk[] array is large enough to
      ** accommodate the block-ids that will be added to it.  */
      assert( pMan->aDelBlk==0 || pMan->bDelFree );
      assert( pDb->nBlkLocalAlloc>0 );
      nByte = (pMan->nDelBlk+pDb->nBlkOrig) * GCENTRYBYTES(pMan);
      pMan->aDelBlk = (u8*)bcvRealloc(pMan->aDelBlk, nByte);
      pMan->bDelFree = 1;
      pNew->nGCOrig = pMan->nDelBlk;

      /* If blocks are being truncated away, add them to the delete list now. */
      for(iBlk=pDb->nBlkLocal; iBlk<pDb->nBlkOrig; iBlk++){
        u8 *aGC = &pMan->aDelBlk[pMan->nDelBlk*GCENTRYBYTES(pMan)];
        memcpy(aGC, &pDb->aBlkOrig[NAMEBYTES(pMan)*iBlk], NAMEBYTES(pMan));
        pMan->nDelBlk++;
      }
    }

    /* Kick off some block uploads. Either the number configured using
    ** the -nwrite command line option, or one for each block to be
    ** uploaded, whichever is smaller.  */
    while( rc==SQLITE_OK && pNew->bDone==0 && pNew->nRef<p->cmd.nWrite ){
      rc = bdUploadBlock(pNew);
    }

    if( rc!=SQLITE_OK || pNew->nRef==0 ){
      sqlite3_finalize(pNew->pRead);
      pNew->pRead = 0;
      if( pNew->nRef==0 ){
        sqlite3_free(pNew);
      }
      bSendReply = 1;
      if( rc!=SQLITE_OK ){
        bReschedule = 1;
      }else{
        bdRemoveDCheckpoint(p, pCont, aId);
      }
    }
  }

  if( bReschedule ){
    DCheckpoint *pCkpt;
    daemon_log(p, BCV_LOG_UPLOAD, 
        "rescheduling upload of %s/%s for %d seconds from now",
        pCont->zName, bdDisplayName(pCont, aId), p->cmd.nRetryTime
    );
    for(pCkpt=pCont->pCheckpointList; pCkpt; pCkpt=pCkpt->pNext){
      if( 0==memcmp(pClient->aId, pCkpt->aId, BCV_DBID_SIZE) ){
        pCkpt->iRetryTime = sqlite_timestamp() + (i64)p->cmd.nRetryTime*1000;
        break;
      }
    }
    assert( pCkpt );
  }
  if( bSendReply ){
    DMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_UPLOAD_REPLY;
    bcvDaemonMessageSend(p, pClient, &reply);
    assert( pCont->eOp==CONTAINER_OP_UPLOAD );
    pCont->eOp = CONTAINER_OP_NONE;
  }
}

static void bdQueryAppendRes(DMessage *pMsg, int *pnAlloc, const char *zStr){
  int nStr = strlen(zStr);
  int nAlloc = *pnAlloc;
  while( pMsg->nBlobByte+nStr+1>nAlloc ){
    nAlloc = nAlloc ? nAlloc*2 : 1024;
    pMsg->aBlob = (u8*)bcvRealloc(pMsg->aBlob, nAlloc);
    *pnAlloc = nAlloc;
  }
  memcpy(&pMsg->aBlob[pMsg->nBlobByte], zStr, nStr+1);
  pMsg->nBlobByte += nStr+1;
}

static void bdVtabQuery(
  DaemonCtx *p, 
  DClient *pClient, 
  DMessage *pMsg
){
  DMessage reply;
  int nAlloc = 0;
  int szBlk = p->pContainerList->pManifest->szBlk;

  memset(&reply, 0, sizeof(reply));
  reply.eType = BCV_MESSAGE_QUERY_REPLY;

  if( pMsg->nBlobByte==12 && memcmp("bcv_manifest", pMsg->aBlob, 12)==0 ){
    DContainer *pCont;
    for(pCont=p->pContainerList; pCont; pCont=pCont->pNext){
      int iDb, iBlk;
      Manifest *pMan = pCont->pManifest;
      for(iDb=0; iDb<pMan->nDb; iDb++){
        ManifestDb *pDb = &pMan->aDb[iDb];
        char zDbid[BCV_DBID_SIZE*2+1];
        hex_encode(pDb->aId, BCV_DBID_SIZE, zDbid);
        for(iBlk=0; iBlk<pDb->nBlkOrig; iBlk++){
          char zBlkid[BCV_MAX_FSNAMEBYTES];
          char aOff[64];
          u8 *aBlk = &pDb->aBlkOrig[iBlk*NAMEBYTES(pMan)];
          sqlite3_snprintf(sizeof(aOff), aOff, "%lld", (i64)iBlk*szBlk);
          hex_encode(aBlk, NAMEBYTES(pMan), zBlkid);
          bdQueryAppendRes(&reply, &nAlloc, pCont->zName);
          bdQueryAppendRes(&reply, &nAlloc, zDbid);
          bdQueryAppendRes(&reply, &nAlloc, pDb->zDName);
          bdQueryAppendRes(&reply, &nAlloc, aOff);
          bdQueryAppendRes(&reply, &nAlloc, zBlkid);
        }
      }

      for(iBlk=0; iBlk<pMan->nDelBlk; iBlk++){
        u8 *aBlk = &pMan->aDelBlk[iBlk*GCENTRYBYTES(pMan)];
        u8 *aTime = &aBlk[NAMEBYTES(pMan)];
        char zBlkid[BCV_MAX_FSNAMEBYTES];
        char aOff[64];
        sqlite3_snprintf(sizeof(aOff), aOff, "%lld", (i64)bcvGetU64(aTime));
        hex_encode(aBlk, NAMEBYTES(pMan), zBlkid);
        bdQueryAppendRes(&reply, &nAlloc, pCont->zName);
        bdQueryAppendRes(&reply, &nAlloc, "");
        bdQueryAppendRes(&reply, &nAlloc, "");
        bdQueryAppendRes(&reply, &nAlloc, aOff);
        bdQueryAppendRes(&reply, &nAlloc, zBlkid);
      }
    }
  }else
  if( pMsg->nBlobByte==9 && memcmp("bcv_cache", pMsg->aBlob, 9)==0 ){
    int i;
    for(i=0; i<p->nHash; i++){
      DCacheEntry *pEntry;
      for(pEntry = p->aHash[i]; pEntry; pEntry=pEntry->pHashNext){
        char zBlkid[BCV_MAX_FSNAMEBYTES];
        char aOff[64];
        hex_encode(pEntry->aName, pEntry->nName, zBlkid);
        sqlite3_snprintf(sizeof(aOff), aOff, "%lld", (i64)szBlk*pEntry->iPos);
        bdQueryAppendRes(&reply, &nAlloc, zBlkid);
        bdQueryAppendRes(&reply, &nAlloc, aOff);
        bdQueryAppendRes(&reply, &nAlloc, pEntry->bPending ? "1" : "0");
        bdQueryAppendRes(&reply, &nAlloc, pEntry->bDirty ? "1" : "0");
      }
    }
  }

  bcvDaemonMessageSend(p, pClient, &reply);
  sqlite3_free(reply.aBlob);
}

/*
** Create directory zName, if it does not already exist.
*/
static void bcvDaemonMkdir(const char *zName){
  struct stat buf;
  if( stat(zName, &buf)<0 ){
    if( errno==ENOENT ){
      if( osMkdir(zName, 0755)<0 ){
        fatal_system_error("mkdir", errno);
      }
    }else{
      fatal_system_error("stat", errno);
    }
  }
}

static void bdCloseLocalFile(sqlite3_file *pFile){
  if( pFile ){
    if( pFile->pMethods ) pFile->pMethods->xClose(pFile);
    sqlite3_free(pFile);
  }
}

/*
** Unlink the local database, wal and shm files for database zDb in 
** container zCont.
*/
static void bdUnlinkLocalFile(const char *zCont, const char *zDb){
  char *zFile = bcvMprintf("%s/%s", zCont, zDb);
  char *zWal = bcvMprintf("%s/%s-wal", zCont, zDb);
  char *zShm = bcvMprintf("%s/%s-shm", zCont, zDb);

  unlink(zFile);
  unlink(zWal);
  unlink(zShm);

  sqlite3_free(zFile);
  sqlite3_free(zWal);
  sqlite3_free(zShm);
}

static int bdOpenLocalFile(
  const char *zCont, 
  const char *zDb, 
  int bReadonly,
  sqlite3_file **ppFile
){
  int f = SQLITE_OPEN_MAIN_DB;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  sqlite3_file *pNew;
  int rc;
  char *zPath = bcvMprintf("%s%s%s", zCont, zCont?"/":"", zDb);
  int nPath = strlen(zPath);
  char *zOpen;

  if( bReadonly ){
    f |= SQLITE_OPEN_READONLY;
  }else{
    f |= SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;
  }

  pNew = (sqlite3_file*)bcvMallocZero(pVfs->szOsFile + nPath + 2);
  zOpen = &((char*)pNew)[pVfs->szOsFile];
  memcpy(zOpen, zPath, nPath);
  sqlite3_free(zPath);

  rc = pVfs->xOpen(pVfs, zOpen, pNew, f, &f);
  if( rc!=SQLITE_OK ){
    sqlite3_free(pNew);
    pNew = 0;
  }

  *ppFile = pNew;
  return rc;
}


/*
** This is called when a new manifest is downloaded that contains 
** modifications to database pDb, part of container pCont (deleting the db
** altogether counts as a modification for the purposes of this function). It
** checks if there are any local changes to database pDb. Non-zero is returned
** if there have been local changes made to the database, or zero otherwise.
** Additionally, if there have been no changes made, the first 16 bytes of
** the *-shm file are zeroed in order to force local database clients to flush
** their page-caches. 
**
** The procedure to check for local changes is:
**
**   1) Check that the db file itself has not been modified by a 
**      checkpoint (this can be done just by checking pDb->nBlkLocalAlloc -
**      return non-zero if it is not zero).
**
**   2) Take an exclusive xShmLock(WRITER) lock on the db file. If
**      the lock fails then return non-zero.
**
**   3) Check that the wal file has been completely checkpointed. Do this
**      by verifying that:
**      
**        + the two copies of the wal-index header are identical, and
**        + the mxFrame value is equal to the WalCkptInfo.nBackfill value.
**
**      If either of the above are false, return non-zero.
**
**   4) Write zeroes to the first 136 bytes of the *-shm file.
**
**   5) Release the xShmLock(WRITER) lock.
**
** If there have been local changes to the database, this is considered an
** error and a daemon_error() error message is output before returning.
**
** If an error occurs while attempting to open the database file or map
** its *-shm file, it is considered a fatal error and the daemon process
** exits. The function does not return in this case.
*/
static int bdHasLocalChanges(DContainer *pCont, ManifestDb *pDb){
  const char *zReason = "local database changes";
  if( pDb->nBlkLocalAlloc==0 ){
    const sqlite3_io_methods *pMeth = 0;
    u8 *aMap = 0;
    sqlite3_file *pFile = 0;
    int rc;
    
    zReason = 0;
    
    /* Open the database and map the *-shm file */
    rc = bdOpenLocalFile(pCont->zName, pDb->zDName, 0, &pFile);
    if( rc==SQLITE_OK ){
      pMeth = pFile->pMethods;
      rc = pMeth->xShmMap(pFile, 0, 32*1024, 1, (volatile void **)&aMap);
    }
    if( rc!=SQLITE_OK ){
      fatal_error("failed to open and map *-shm for database %s/%s (rc=%d)",
          pCont->zName, pDb->zDName, rc
      );
    }
    assert( pFile && aMap );

    /* Take the WRITER lock */
    if( pMeth->xShmLock(pFile, 0, 1, SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE) ){
      zReason = "cannot get WRITE lock";
    }

    /* Check if the wal file has been modified */
    if( zReason==0 ){
      if( memcmp(&aMap[0], &aMap[48], 48) || memcmp(&aMap[16], &aMap[96], 4) ){
        zReason = "wal file has been modified";
      }
    }

    /* Zero the first few bytes of the *-shm file */
    if( zReason==0 && rc==SQLITE_OK ){
      memset(aMap, 0, 136);
    }

    pMeth->xShmLock(pFile, 0, 1, SQLITE_SHM_UNLOCK|SQLITE_SHM_EXCLUSIVE);
    pMeth->xShmUnmap(pFile, 0);
    bdCloseLocalFile(pFile);
  }

  if( zReason ){
    daemon_error("database %s/%s has been modified locally (%s)",
        pCont->zName, pDb->zDName, zReason
    );
  }
  return (zReason!=0);
}


/*
** Process a "PRAGMA bcv_detach = zCont" (or equivalent) command.
** A container can be detached iff:
**
**   a) there are no connected clients, and
**   b) there are no local changes.
**
** To detach a container:
**
**   a) the entry is removed from the "manifest" table, and
**   b) the database files, and any *-wal and *-shm files, are unlinked.
**   c) the in-memory object (type DContainer) is freed, and
*/
static void bdDelContainer(
  DaemonCtx *pCtx,                /* Daemon context */
  DClient *pClient,               /* Client to reply to */
  const char *zCont               /* New container to attach */
){
  DContainer *pCont;
  DMessage reply;
  char *zRet = 0;

  pCont = bdFindContainer(pCtx, zCont);
  if( pCont==0 || pCont->pAttachClient ){
    zRet = bcvMprintf("no such container: %s", zCont);
  }else{
    Manifest *pMan = pCont->pManifest;
    DClient *pClient;
    int ii;

    /* First check for any active clients */
    for(pClient=pCtx->pClientList; pClient; pClient=pClient->pNext){
      if( pClient->pContainer==pCont ){
        zRet = bcvMprintf(
            "cannot detach container (active clients): %s", zCont
        );
        break;
      }
    }

    /* If there are no active clients, check for local changes. */
    for(ii=0; zRet==0 && ii<pMan->nDb; ii++){
      if( bdHasLocalChanges(pCont, &pMan->aDb[ii]) ){
        zRet = bcvMprintf("cannot detach container (local changes): %s", zCont);
        break;
      }
    }

    if( zRet==0 ){
      DContainer **pp;
      char *z = bcvMprintf("DELETE FROM manifest WHERE container = %Q", zCont);
      int rc = sqlite3_exec(pCtx->db, z, 0, 0, 0);
      if( rc!=SQLITE_OK ) fatal_sql_error(pCtx->db, "bdDelContainer()");
      sqlite3_free(z);

      for(ii=0; zRet==0 && ii<pMan->nDb; ii++){
        bdUnlinkLocalFile(zCont, pMan->aDb[ii].zDName);
      }

      for(pp=&pCtx->pContainerList; *pp!=pCont; pp=&((*pp)->pNext));
      *pp = pCont->pNext;
      *pp = pCont->pNext;
      bcvManifestDeref(pCont->pManifest);
      sqlite3_free(pCont);
    }
  }

  memset(&reply, 0, sizeof(DMessage));
  reply.eType = BCV_MESSAGE_QUERY_REPLY;
  if( zRet ){
    reply.aBlob = (u8*)zRet;
  }else{
    reply.aBlob = (u8*)"ok";
  }
  reply.nBlobByte = strlen((const char*)reply.aBlob);
  bcvDaemonMessageSend(pCtx, pClient, &reply);
  sqlite3_free(zRet);
}


/*
** Process a "PRAGMA bcv_attach = zCont" (or equivalent) command.
*/
static void bdAddContainer(
  DaemonCtx *pCtx,                /* Daemon context */
  DClient *pClient,               /* Client to reply to */
  const char *zCont               /* New container to attach */
){
  /* Check that there is not already a container of this name */
  if( bdFindContainer(pCtx, zCont) ){
    DMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_QUERY_REPLY;
    reply.aBlob = (u8*)bcvMprintf("already attached: %s", zCont);
    reply.nBlobByte = strlen((char*)reply.aBlob);
    bcvDaemonMessageSend(pCtx, pClient, &reply);
    sqlite3_free(reply.aBlob);
  }else{
    Manifest *pMan;
    int nCont = strlen(zCont);
    DContainer *pNew;

    /* Allocate and populate the new container object. Then link it into
    ** the daemon context object. */
    pNew = (DContainer*)bcvMallocZero(sizeof(DContainer)+nCont+1);
    pNew->zName = (char*)&pNew[1];
    memcpy(pNew->zName, zCont, nCont);
    pNew->iPollTime = sqlite_timestamp();
    pNew->iGCTime = pNew->iPollTime + pCtx->cmd.nGCTime*1000;
    pNew->pNext = pCtx->pContainerList;
    pCtx->pContainerList = pNew;

    /* If required, create a directory for the new container */
    bcvDaemonMkdir(zCont);

    /* Create and install an empty manifest. */
    pMan = bcvMallocZero(sizeof(Manifest));
    pMan->szBlk = pCtx->szBlk;
    pMan->nNamebytes = BCV_DEFAULT_NAMEBYTES;
    pMan->nRef = 1;
    bdManifestInstall(pCtx, pNew, pMan, 0, 0);

    /* Reply to the client after the manifest has been polled. If an error
    ** occurs, the container will be automatically detached at that point.  */
    pNew->pAttachClient = pClient;
  }
}

static void bdPragmaQuery(
  DaemonCtx *p,                   /* Daemon object */
  DClient *pClient,               /* Client that sent QUERY message */
  const char *zName,              /* Name of pragma */
  const char *zArg                /* Argument passed to pragma */
){
  if( sqlite3_stricmp("bcv_upload", zName)==0 ){
    DCheckpoint *pCkpt = bdFindOrCreateCheckpoint(p, pClient);
    pClient->pNextWaiting = pCkpt->pWaiting;
    pCkpt->pWaiting = pClient;
  }else
  if( sqlite3_stricmp("bcv_attach", zName)==0 ){
    bdAddContainer(p, pClient, zArg);
  }else if( sqlite3_stricmp("bcv_detach", zName)==0 ){
    bdDelContainer(p, pClient, zArg);
  }else{
    DMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_QUERY_REPLY;
    reply.aBlob = (u8*)bcvMprintf("no such pragma: %s", zName);
    reply.nBlobByte = strlen((char*)reply.aBlob);
    bcvDaemonMessageSend(p, pClient, &reply);
  }
}

static void bdHandleClientQuery(
  DaemonCtx *p, 
  DClient *pClient, 
  DMessage *pMsg
){
  int bPragma = 0;
  int i;

  /* Figure out if this is a virtual-table or PRAGMA request. It is a PRAGMA
  ** request if the string argument contains an '=' character.  */
  for(i=0; i<pMsg->nBlobByte; i++){
    if( pMsg->aBlob[i]=='=' ){
      bPragma = 1;
      break;
    }
  }

  if( bPragma ){
    char *z1 = bcvMprintf("%.*s", i, (char*)pMsg->aBlob);
    char *z2 = bcvMprintf("%.*s", pMsg->nBlobByte-i-1,(char*)&pMsg->aBlob[i+1]);
    bdPragmaQuery(p, pClient, z1, z2);
    sqlite3_free(z1);
    sqlite3_free(z2);
  }else{
    bdVtabQuery(p, pClient, pMsg);
  }
}

/*
** Return 0 if a message is successfully read, or non-zero if an error
** occurs.
*/
static int bdClientMessage(
  BCV_SOCKET_TYPE s,              /* Read data from this socket */
  DMessage *pMsg,                 /* Object to populate */
  u8 **paFree                     /* OUT: Buffer for caller to free */
){
  u8 aMsg[5];
  ssize_t nRead = 0;
  int nBody = 0;
  u8 *aBody = 0;

  nRead = recv(s, aMsg, 5, 0);
  if( nRead!=5 ) return 1;

  nBody = (int)bcvGetU32(&aMsg[1]) - 5;
  if( nBody>0 ){
    aBody = (u8*)bcvMalloc(nBody);
    nRead = recv(s, aBody, nBody, 0);
  }else{
    nRead = 0;
  }
  memset(pMsg, 0, sizeof(DMessage));
  if( nRead!=nBody || bcvDaemonParseMessage(aMsg[0], aBody, nBody, pMsg) ){
    sqlite3_free(aBody);
    return 1;
  }
  *paFree = aBody;
  return 0;
}

/*
** This is called by the daemon main loop whenever there is a message to read
** from the socket associated with client pClient.
*/
static void bcvDaemonClientMessage(DaemonCtx *p, DClient *pClient){
  DMessage msg;
  u8 *aBody = 0;

  if( bdClientMessage(pClient->fd, &msg, &aBody) ){
    bcvDaemonCloseConnection(p, pClient);
  }else{
    bcvDaemonLogMessage(p, pClient, &msg);
    switch( msg.eType ){
      case BCV_MESSAGE_LOGIN: {
        DContainer *pCont;
        for(pCont=p->pContainerList; pCont; pCont=pCont->pNext){
          if( 0==bcvStrcmp(pCont->zName, msg.zContainer) ) break;
        }
        hex_decode(msg.zDb, BCV_DBID_SIZE*2, pClient->aId);
        if( pCont==0 || bcvDaemonFindDb(pCont, pClient->aId)<0 ){
          daemon_error("no such container/db: %s/%s", msg.zContainer,msg.zDb);
          bcvDaemonCloseConnection(p, pClient);
        }else{
          DMessage reply;
          pClient->pContainer = pCont;
          memset(&reply, 0, sizeof(DMessage));
          reply.eType = BCV_MESSAGE_LOGIN_REPLY;
          reply.iVal = pCont->pManifest->szBlk;
          bcvDaemonMessageSend(p, pClient, &reply);
        }
        break;
      }
      case BCV_MESSAGE_REQUEST: {
        bdHandleClientRequest(p, pClient, &msg);
        break;
      }
      case BCV_MESSAGE_DONE: {
        bcvDaemonClientClearRefs(pClient);
        bcvDaemonProcessUsed(p, pClient, msg.aBlob, msg.nBlobByte);
        bcvManifestDeref(pClient->pManifest);
        pClient->pManifest = 0;
        pClient->iDb = 0;
        break;
      }
      case BCV_MESSAGE_WDONE: {
        bdFindOrCreateCheckpoint(p, pClient);
        break;
      }
      case BCV_MESSAGE_UPLOAD: {
        bdClientUpload(p, pClient, &msg);
        break;
      }
      case BCV_MESSAGE_WREQUEST: {
        bdHandleClientRequest(p, pClient, &msg);
        break;
      }
      case BCV_MESSAGE_QUERY: {
        bdHandleClientQuery(p, pClient, &msg);
        break;
      }
      default:
        assert( 0 );
    }

    sqlite3_free(aBody);
    bcvDaemonParseFree(&msg);
  }
}

/*
** Database pDb has been removed from the manifest.
*/
static void bdDeleteDatabase(
  DaemonCtx *pCtx, 
  DContainer *pCont, 
  ManifestDb *pDb
){
  bdUnlinkLocalFile(pCont->zName, pDb->zDName);
}

static void bdPopulateDatabase(
  sqlite3_file *pFile,
  int iPort,                      /* Port that daemon is listening on */
  const char *zCont,              /* Container name */
  const char *zDb,                /* Database (file) name */
  const u8 *aId                   /* Database id */
){
  char zId[BCV_DBID_SIZE*2+1];
  char *zStr;
  int rc;

  /* Create the file contents string in zStr */
  hex_encode(aId, BCV_DBID_SIZE, zId);
  zStr = bcvMprintf("blockvfs:1:%s/%s:%d\n", zCont, zId, iPort);

  /* Write the file contents */
  rc = pFile->pMethods->xWrite(pFile, zStr, strlen(zStr), 0);
  if( rc!=SQLITE_OK ){
    fatal_error("failed to write to %s/%s (%d)", zCont, zDb, rc);
  }

  sqlite3_free(zStr);
}

/*
** Database pDb has been added to the manifest.
*/
static void bdCreateDatabase(
  DaemonCtx *pCtx, 
  DContainer *pCont, 
  ManifestDb *pDb
){
  const char *zCont = pCont->zName;
  int rc;                       /* VFS method return code */
  sqlite3_file *pFile = 0;      /* Database file */

  bdUnlinkLocalFile(zCont, pDb->zDName);
  rc = bdOpenLocalFile(zCont, pDb->zDName, 0, &pFile);
  if( rc!=SQLITE_OK ){
    fatal_error("failed to open %s/%s (%d)", zCont, pDb->zDName, rc);
  }
  bdPopulateDatabase(pFile, pCtx->iListenPort, zCont, pDb->zDName, pDb->aId);
  bdCloseLocalFile(pFile);
}

static void bdPollManifestDone(DCurlRequest *p, CURLMsg *pMsg){
  DaemonCtx *pCtx = p->pDaemonCtx;
  char *zETag = p->req.zETag;
  DContainer *pCont = p->pContainer;
  int bNew;
  int bFail;
  int bInstall = 1;
  
  bFail = bdIsHttpError(p, pMsg, 200, 0);
  bNew = bcvStrcmp(pCont->pManifest->zETag, zETag);
  daemon_log(pCtx, BCV_LOG_POLL, 
      "done polling manifest for container %s (fail=%d) (bNew=%d) (etag=%s)", 
      pCont->zName, bFail, bNew, zETag
  );
  if( bNew && bFail==0 ){
    int iNew;                     /* Index into pNew->aDb[] */
    int iOld;                     /* Index into pOld->aDb[] */
    Manifest *pNew = bcvManifestParse(p->aBuf, p->nBuf, zETag);
    Manifest *pOld = pCont->pManifest;

    p->aBuf = 0;
    p->nBuf = 0;
    p->req.zETag = 0;

    daemon_event_log(pCtx,
        "updated manifest for container \"%s\" (old=%s new=%s)", 
        pCont->zName, pOld->zETag, zETag
    );

    /* First pass - to find deleted and modified databases */
    iNew = iOld = 0;
    while( iOld<pOld->nDb ){
      int cmp;
      ManifestDb *pOldDb = &pOld->aDb[iOld];
      ManifestDb *pNewDb = iNew<pNew->nDb ? &pNew->aDb[iNew] : 0;

      if( pNewDb==0 ) cmp = -1;
      else cmp = memcmp(pOldDb->aId, pNewDb->aId, BCV_DBID_SIZE);

      if( cmp<0 ){
        /* pOldDb has been deleted */
        if( bdHasLocalChanges(pCont, pOldDb) ){
          bInstall = 0;
          break;
        }
        bdDeleteDatabase(pCtx, pCont, pOldDb);
        iOld++;
      }else if( cmp>0 ){
        /* pNewDb has been added. Do nothing this pass. */
        iNew++;
      }else{
        if( pOldDb->iVersion!=pNewDb->iVersion ){
          /* If the version numbers don't match then the database has been
          ** modified remotely. If it has also been modified locally, the
          ** system is in an error state.  */
          if( bdHasLocalChanges(pCont, pOldDb) ){
            bInstall = 0;
            break;
          }
        }else if( pOldDb->nBlkLocalAlloc ){
          /* If the db versions are identical but there is a locally allocated
          ** block array, then the database has been modified locally, but not
          ** remotely. In this case copy the local block array from the current
          ** manifest. */
          int nByte = pOldDb->nBlkLocal*NAMEBYTES(pNew);
          assert( pNewDb->nBlkLocalAlloc==0 );
          assert( pOldDb->nBlkLocal>=pNewDb->nBlkLocal );
          pNewDb->aBlkLocal = (u8*)bcvMallocZero(nByte);
          pNewDb->nBlkLocalAlloc = pNewDb->nBlkLocal = pOldDb->nBlkLocal;
          memcpy(pNewDb->aBlkLocal, pOldDb->aBlkLocal, nByte);
        }
        iNew++;
        iOld++;
      }
    }

    /* Second pass - to find new databases. We have to search for new
    ** databases in a second pass to avoid the case where one database
    ** is deleted and another with the same display-name added. */
    if( bInstall ){
      iNew = iOld = 0;
      while( iNew<pNew->nDb ){
        int cmp;
        ManifestDb *pNewDb = &pNew->aDb[iNew];
        ManifestDb *pOldDb = iOld<pOld->nDb ? &pOld->aDb[iOld] : 0;

        if( pOldDb==0 ) cmp = 1;
        else cmp = memcmp(pOldDb->aId, pNewDb->aId, BCV_DBID_SIZE);

        if( cmp<0 ){
          /* pOldDb has been deleted. No-op. */
          iOld++;
        }else if( cmp>0 ){
          /* pNewDb has been added. */
          bdCreateDatabase(pCtx, pCont, pNewDb);
          iNew++;
        }else{
          iOld++;
          iNew++;
        }
      }
    }

    if( bInstall ){
      bdManifestInstall(pCtx, pCont, pNew, p->aBuf, p->nBuf);
    }else{
      daemon_error("cannot use new manifest due to local changes");
      bcvManifestDeref(pNew);
    }
  }

  if( pCont->pAttachClient ){
    DClient *pClient = pCont->pAttachClient;
    DMessage reply;
    pCont->pAttachClient = 0;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_QUERY_REPLY;
    if( bFail || (bNew && bInstall==0) ){
      DContainer **pp;
      if( bFail ){
        reply.aBlob = (u8*)"failed to load manifest";
      }else{
        reply.aBlob = (u8*)"cannot attach due to local changes";
      }

      /* Detach the container. */
      for(pp=&pCtx->pContainerList; *pp!=pCont; pp=&((*pp)->pNext));
      *pp = pCont->pNext;
      bcvManifestDeref(pCont->pManifest);
      sqlite3_free(pCont);
      pCont = 0;
    }else{
      reply.aBlob = (u8*)"ok";
    }

    reply.nBlobByte = strlen((char*)reply.aBlob);
    bcvDaemonMessageSend(pCtx, pClient, &reply);
  }

  if( pCont ){
    assert( pCont->eOp==CONTAINER_OP_POLL );
    pCont->eOp = CONTAINER_OP_NONE;
    pCont->iPollTime = sqlite_timestamp() + p->pDaemonCtx->cmd.nPollTime*1000;
  }
}

/*
** This is called when it is time to poll cloud storage for a new manifest
** file for container pCont.
*/
static void bdPollManifest(DaemonCtx *p, DContainer *pCont){
  const char *zCont = pCont->zName;
  DCurlRequest *pCurl;

  pCurl = bdCurlRequest(p);
  curlFetchBlob(&pCurl->req, &p->ap, zCont, BCV_MANIFEST_FILE);
  bdCurlMemoryDownload(pCurl);

  pCurl->pContainer = pCont;
  assert( pCont->eOp==CONTAINER_OP_NONE );
  pCont->eOp = CONTAINER_OP_POLL;
  bdCurlRun(p, pCurl, bdPollManifestDone);
  daemon_log(p, BCV_LOG_POLL, "polling manifest for container %s", zCont);
}

static void bdDeleteOneBlock(DDelete *p);

static void bdDeleteOneBlockDone(DCurlRequest *pReq, CURLMsg *pMsg){
  DDelete *p = pReq->pDelete;
  int bErr;

  assert( p->pContainer->eOp==CONTAINER_OP_DELETE );
  p->nReq--;
  bErr = bdIsHttpError(pReq, pMsg, 202, 404);
  if( bErr ) p->bErr = 1;
  if( p->bErr==0 ){
    bdDeleteOneBlock(p);
  }

  if( p->nReq==0 ){
    DaemonCtx *pCtx = pReq->pDaemonCtx;
    if( p->bErr ){
      /* Retry the delete operation in nRetryTime seconds. */
      bdDeleteFree(p);
      p->pContainer->iDeleteTime = sqlite_timestamp()+pCtx->cmd.nRetryTime*1000;
      p->pContainer->eOp = CONTAINER_OP_NONE;
    }else{
      Manifest *pMan = p->pMan;
      int iOut = 0;
      int i;
      DCurlRequest *pUp;

      /* Manifest object DDelete.pMan is a copy of the manifest for
      ** the container that old objects have just been deleted from. The
      ** 8-byte timestamp field for each aDelBlk[] entry deleted has been
      ** set to 0. The following block edits the aDelBlk[] array so that
      ** the deleted entries are no longer present.  */
      for(i=0; i<pMan->nDelBlk; i++){
        u8 *aBlk = &pMan->aDelBlk[i*GCENTRYBYTES(pMan)];
        if( bcvGetU64(&aBlk[NAMEBYTES(pMan)]) ){
          u8 *aBlkOut = &pMan->aDelBlk[iOut*GCENTRYBYTES(pMan)];
          if( aBlk!=aBlkOut ){
            memcpy(aBlkOut, aBlk, GCENTRYBYTES(pMan));
          }
          iOut++;
        }
      }
      pMan->nDelBlk = iOut;

      /* Upload the newly edited manifest */
      pUp = bdUploadManifestRequest(pCtx, BCV_LOG_EVENT, p->pContainer, pMan);
      pUp->pDelete = p;
      bdCurlRun(pReq->pDaemonCtx, pUp, bdUploadManifestDone);
    }
  }
}

static void bdDeleteOneBlock(DDelete *p){
  Manifest *pMan = p->pMan;
  assert( p->pContainer->eOp==CONTAINER_OP_DELETE );
  while( p->iDel<pMan->nDelBlk ){
    u8 *aBlk = &pMan->aDelBlk[p->iDel * GCENTRYBYTES(pMan)];
    i64 iTm = (i64)bcvGetU64(&aBlk[NAMEBYTES(pMan)]);
    p->iDel++;

    if( iTm<=p->iDelBefore ){
      char zFile[BCV_MAX_FSNAMEBYTES];
      DCurlRequest *pReq = bdCurlRequest(p->pCtx);
      bcvBlockidToText(pMan, aBlk, zFile);
      curlDeleteBlob(&pReq->req, &p->pCtx->ap, p->pContainer->zName, zFile);
      p->nReq++;
      pReq->pDelete = p;
      bdCurlRun(p->pCtx, pReq, bdDeleteOneBlockDone);
      bcvPutU64(&aBlk[NAMEBYTES(pMan)], 0);
      daemon_event_log(p->pCtx, "deleting block %s", zFile);
      break;
    }
  }
}

/*
** This is called when it is time to delete one or more blocks on the 
** delete-list of container pCont.
*/
static void bdDeleteBlocks(DaemonCtx *p, DContainer *pCont){
  DDelete *pNew;
  int i;

  daemon_event_log(p, "deleting blocks from container %s", pCont->zName);
  assert( pCont->eOp==CONTAINER_OP_NONE );
  pCont->eOp = CONTAINER_OP_DELETE;

  pNew = (DDelete*)bcvMallocZero(sizeof(DDelete));
  pNew->pMan = bcvManifestDup(pCont->pManifest);
  pNew->pContainer = pCont;
  pNew->pCtx = p;
  pNew->iDelBefore = sqlite_timestamp();

  for(i=0; i<p->cmd.nDelete; i++){
    bdDeleteOneBlock(pNew);
  }
  assert( pNew->nReq>0 );
}

static int bdCollectHashKey(DCollect *p, u8 *aBlk){
  return bcvGetU32(aBlk) % p->nHash;
}

static void bdCollectHashAdd(DCollect *p, u8 *aBlk){
  int iKey = bdCollectHashKey(p, aBlk);
  while( 1 ){
    if( p->aHash[iKey]==0 ){
      p->aHash[iKey] = aBlk;
      break;
    }
    iKey = (iKey+1) % p->nHash;
  }
}

static int bdCollectHashContains(DCollect *p, u8 *aBlk){
  int iKey = bdCollectHashKey(p, aBlk);
  assert( iKey>=0 && iKey<p->nHash );
  while( p->aHash[iKey] ){
    if( 0==memcmp(p->aHash[iKey], aBlk, NAMEBYTES(p->pMan)) ) return 1;
    iKey = (iKey+1) % p->nHash;
  }
  return 0;
}

static int bdFilenameToBlockid(Manifest *p, const char *zName, u8 *aBlk){
  int nName = strlen(zName);
  int nExpect = BCV_FSNAMEBYTES(NAMEBYTES(p));
  if( nName==nExpect-1 && 0==memcmp(".bcv", &zName[nExpect-1-4], 4) ){
    if( hex_decode(zName, nName-4, aBlk) ) return 0;
    return 1;
  }
  return 0;
}

static void bdListFilesDone(DCurlRequest *pReq, CURLMsg *pMsg){
  DCollect *p = pReq->pCollect;
  DContainer *pCont = p->pContainer;
  DaemonCtx *pCtx = p->pCtx;
  assert( pCont->eOp==CONTAINER_OP_COLLECT );
  if( bdIsHttpError(pReq, pMsg, 200, 0) ){
    pCont->iGCTime = sqlite_timestamp() + pCtx->cmd.nGCTime*1000;
    bdCollectFree(p);
    pCont->eOp = CONTAINER_OP_NONE;
  }else{
    Manifest *pMan = p->pMan;
    FilesParse files;
    FilesParseEntry *pEntry;

    memset(&files, 0, sizeof(files));
    bcvParseFiles(pReq->aBuf, pReq->nBuf, &files);
    for(pEntry=files.pList; pEntry; pEntry=pEntry->pNext){
      u8 aBlk[BCV_MAX_NAMEBYTES];
      if( bdFilenameToBlockid(p->pMan, pEntry->zName, aBlk)
       && 0==bdCollectHashContains(p, aBlk)
      ){
        u8 *aEntry;
        if( p->nDelete==p->nDeleteAlloc ){
          int nByte;
          p->nDeleteAlloc += 20;
          nByte = p->nDeleteAlloc * GCENTRYBYTES(pMan);
          p->aDelete = (u8*)bcvRealloc(p->aDelete, nByte);
        }

        aEntry = &p->aDelete[p->nDelete*GCENTRYBYTES(pMan)];
        memcpy(aEntry, aBlk, NAMEBYTES(p->pMan));
        bcvPutU64(&aEntry[NAMEBYTES(p->pMan)], p->iDeleteTime);
        p->nDelete++;
        daemon_event_log(pCtx, "collecting block %s", pEntry->zName);
      }
    }

    if( files.zNextMarker ){
      const char *zCont = p->pContainer->zName;
      DCurlRequest *pCurl = bdCurlRequest(pCtx);
      curlListFiles(&pCurl->req, &pCtx->ap, zCont, files.zNextMarker);
      pCurl->pCollect = p;
      bdCurlMemoryDownload(pCurl);
      bdCurlRun(pCtx, pCurl, bdListFilesDone);
    }else if( p->aDelete ){
      Manifest *pMan = p->pMan;
      DCurlRequest *pReq;
      int nByte = (p->nDelete+pMan->nDelBlk) * GCENTRYBYTES(pMan);

      assert( pMan->bDelFree || pMan->aDelBlk==0 );
      pMan->aDelBlk = (u8*)bcvRealloc(pMan->aDelBlk, nByte);
      pMan->bDelFree = 1;
      memcpy(&pMan->aDelBlk[pMan->nDelBlk*GCENTRYBYTES(pMan)],
             p->aDelete, p->nDelete * GCENTRYBYTES(pMan)
      );
      pMan->nDelBlk += p->nDelete;

      pReq = bdUploadManifestRequest(pCtx, BCV_LOG_EVENT, p->pContainer, pMan);
      pReq->pCollect = p;
      bdCurlRun(pCtx, pReq, bdUploadManifestDone);
    }else{
      p->pContainer->iGCTime = sqlite_timestamp() + pCtx->cmd.nGCTime*1000;
      bdCollectFree(p);
      pCont->eOp = CONTAINER_OP_NONE;
    }

    bcvParseFilesClear(&files);
  }
}

static void bdStartGC(DaemonCtx *pCtx, DContainer *pCont){
  DCurlRequest *pCurl;
  DCollect *p;
  Manifest *pMan = pCont->pManifest;
  int nBlk;
  int iDb, iBlk;                  /* Iterator variables */
  int nHash;

  daemon_event_log(pCtx, "garbage collecting from container %s", pCont->zName);

  /* Count the number of blocks in the manifest file. And determine the
  ** size of the aHash[] array to use. */
  nBlk = pMan->nDelBlk;
  for(iDb=0; iDb<pMan->nDb; iDb++){
    nBlk += pMan->aDb[iDb].nBlkLocal;
  }
  nHash = 2;
  while( nHash<nBlk ) nHash = nHash*2;
  nHash = nHash*2;

  /* Allocate and populate the hash table */
  p = (DCollect*)bcvMallocZero(sizeof(DCollect) + nHash*sizeof(u8*));
  p->aHash = (u8**)&p[1];
  p->nHash = nHash;
  pMan = p->pMan = bcvManifestDup(pMan);
  p->pContainer = pCont;
  p->pCtx = pCtx;
  for(iBlk=0; iBlk<pMan->nDelBlk; iBlk++){
    bdCollectHashAdd(p, &pMan->aDelBlk[iBlk * GCENTRYBYTES(pMan)]);
  }
  for(iDb=0; iDb<pMan->nDb; iDb++){
    ManifestDb *pDb = &pMan->aDb[iDb];
    for(iBlk=0; iBlk<pDb->nBlkOrig; iBlk++){
      bdCollectHashAdd(p, &pDb->aBlkOrig[iBlk * NAMEBYTES(pMan)]);
    }
  }

  /* Set the containers iGCTime member to zero to indicate garbage 
  ** collection is underway.  */
  pCont->iGCTime = 0;

  pCurl = bdCurlRequest(pCtx);
  curlListFiles(&pCurl->req, &pCtx->ap, pCont->zName, 0);
  pCurl->pCollect = p;
  bdCurlMemoryDownload(pCurl);
  bdCurlRun(pCtx, pCurl, bdListFilesDone);

  assert( pCont->eOp==CONTAINER_OP_NONE );
  pCont->eOp = CONTAINER_OP_COLLECT;
}

static void bdMainloop(DaemonCtx *p){
  int bClient = 0;

  while( 1 ){
    int nDummy;
    DClient *pClient;
    DClient *pNextClient;
    CURLMcode mc;
    int nFd = 1;
    int i;
    int nReady = 0;
    struct curl_waitfd *aWait;
    struct CURLMsg *pMsg;
    i64 iTime;                    /* Current timestamp */
    i64 iTimeout;                 /* Time-out for curl_multi_wait() */
    DContainer *pCont;            /* For iterating through containers */
    DCheckpoint *pCkpt;           /* For iterating through checkpoints */
    int bBusy = 0;

    /* Work on http requests. Issue xDone callbacks if any are finished. */
    mc = curl_multi_perform(p->pMulti, &nDummy);
    if( mc!=CURLM_OK ){
      fatal_error("curl_multi_perform() failed - %d", mc);
    }
    while( (pMsg = curl_multi_info_read(p->pMulti, &nDummy)) ){
      DCurlRequest *pCurl;
      DCurlRequest **pp;
      for(pCurl=p->pRequestList; pCurl; pCurl=pCurl->pNext){
        if( pMsg->easy_handle==pCurl->req.pCurl ){
          pCurl->xDone(pCurl, pMsg);
          break;
        }
      }
      assert( pCurl );
      for(pp=&p->pRequestList; *pp!=pCurl; pp=&(*pp)->pNext);
      *pp = pCurl->pNext;
      curl_multi_remove_handle(p->pMulti, pCurl->req.pCurl);
      bcvDaemonCurlRelease(p, pCurl);
    }

    for(pClient=p->pClientList; pClient; pClient=pClient->pNext) nFd++;

    aWait = bcvMallocZero(sizeof(struct curl_waitfd) * nFd);
    aWait[0].fd = p->fdListen;
    aWait[0].events = CURL_WAIT_POLLIN;

    i = 1;
    for(pClient=p->pClientList; pClient; pClient=pClient->pNext){
      aWait[i].fd = pClient->fd;
      aWait[i].events = CURL_WAIT_POLLIN;
      i++;
    }

    /* Figure out the timeout value to use for curl_multi_wait(). Events
    ** that use time-outs are:
    **
    **   + polling for new manifest files,
    **   + deleting old blocks from a GC list,
    **   + deleting stray blocks from a GC container,
    **   + retrying a checkpoint impeded by client locks.
    */
    iTime = sqlite_timestamp();
    iTimeout = 60*1000;
    for(pCont=p->pContainerList; pCont; pCont=pCont->pNext){
      if( pCont->eOp==CONTAINER_OP_NONE ){
        if( pCont->iPollTime && (pCont->iPollTime - iTime) < iTimeout ){
          iTimeout = pCont->iPollTime - iTime;
          iTimeout = MAX(iTimeout, 1);
        }
        if( pCont->iDeleteTime && (pCont->iDeleteTime - iTime) < iTimeout ){
          iTimeout = pCont->iDeleteTime - iTime;
          iTimeout = MAX(iTimeout, 1);
        }
        if( pCont->iGCTime && (pCont->iGCTime - iTime) < iTimeout ){
          iTimeout = pCont->iGCTime - iTime;
          iTimeout = MAX(iTimeout, 1);
        }
        for(pCkpt=pCont->pCheckpointList; pCkpt; pCkpt=pCkpt->pNext){
          if( pCkpt->iRetryTime && (pCkpt->iRetryTime - iTime) < iTimeout ){
            iTimeout = pCkpt->iRetryTime - iTime;
            iTimeout = MAX(iTimeout, 1);
          }
        }
      }
      if( pCont->pCheckpointList || pCont->eOp!=CONTAINER_OP_NONE ) bBusy = 1;
    }

    /* Wait for something to happen */
    mc = curl_multi_wait(p->pMulti, aWait, nFd, iTimeout, &nReady);
    if( mc!=CURLM_OK ){
      fatal_error("curl_multi_wait() failed - %d", mc);
    }

    /* Check for client messages */
    i = 1;
    for(pClient=p->pClientList; pClient; pClient=pNextClient){
      pNextClient = pClient->pNext;
      if( aWait[i].revents ){
        bcvDaemonClientMessage(p, pClient);
      }
      i++;
    }

    /* Check for new connections */
    if( aWait[0].revents!=0 ){
      bcvDaemonNewConnection(p);
      bClient = 1;
    }

    /* Check if it is time to bail out (due to -autoexit option) */ 
    sqlite3_free(aWait);
    if( p->cmd.bAutoexit && bClient && p->pClientList==0 && bBusy==0 ){
      break;
    }

    /* For each container, check if it is time to:
    **
    **   (a) poll for a new manifest,
    **   (b) attempt an upload,
    **   (c) delete one or more blocks from the delete-list, or
    **   (d) find stray block files to add to the delete-list (garbage
    **       collection).
    */
    iTime = sqlite_timestamp();
    for(pCont=p->pContainerList; pCont; pCont=pCont->pNext){
      if( pCont->eOp==CONTAINER_OP_NONE ){
        for(pCkpt=pCont->pCheckpointList; pCkpt; pCkpt=pCkpt->pNext){
          if( pCkpt->iRetryTime && iTime>=pCkpt->iRetryTime ){
            bdLaunchUpload(pCont, pCkpt);
            daemon_log(p, BCV_LOG_UPLOAD, 
                "created upload thread for %s/%s", pCont->zName, pCkpt->zDb
            );
            break;
          }
        }
        if( pCkpt==0 ){
          if( pCont->iPollTime && iTime>=pCont->iPollTime ){
            bdPollManifest(p, pCont);
          }
          else if( pCont->iDeleteTime && iTime>=pCont->iDeleteTime ){
            bdDeleteBlocks(p, pCont);
          }
          else if( pCont->iGCTime && iTime>=pCont->iGCTime ){
            bdStartGC(p, pCont);
          }
        }
      }
    }
  }
}

/*
** Free the DaemonCtx object and all ancillary resources. This is called
** right before the process exits - if there are any outstanding allocations 
** not freed by this routine it indicates that the daemon is leaking memory.
*/
static void bdCleanup(DaemonCtx *p){
  int rc;
  DCacheEntry *pEntry;
  DCacheEntry *pNextEntry;
  DContainer *pCont;
  DContainer *pNextCont;

  DCurlRequest *pReq;
  DCurlRequest *pNextReq;

  assert( p->pClientList==0 );

  bcvAccessPointFree(&p->ap);

  /* Close and delete the cache-file handle */
  if( p->pCacheFile && p->pCacheFile->pMethods ){
    p->pCacheFile->pMethods->xClose(p->pCacheFile);
  }
  sqlite3_free(p->pCacheFile);
  p->pCacheFile = 0;

  /* Close the database handle */
  sqlite3_finalize(p->pInsert);
  sqlite3_finalize(p->pDelete);
  sqlite3_finalize(p->pDeletePos);
  sqlite3_finalize(p->pWriteMan);
  sqlite3_finalize(p->pReadMan);
  rc = sqlite3_close(p->db);
  assert( rc==SQLITE_OK );

  /* Delete all DCacheEntry structures */
  for(pEntry=p->pLruFirst; pEntry; pEntry=pNextEntry){
    pNextEntry = pEntry->pLruNext;
    sqlite3_free(pEntry);
  }

  /* Delete all DContainer structures */
  for(pCont=p->pContainerList; pCont; pCont=pNextCont){
    pNextCont = pCont->pNext;
    bcvManifestDeref(pCont->pManifest);
    sqlite3_free(pCont);
  }

  for(pReq=p->pRequestList; pReq; pReq=pNextReq){
    pNextReq = pReq->pNext;
    curlRequestFinalize(&pReq->req);
    sqlite3_free(pReq);
  }

  if( p->pMulti ){
    curl_multi_cleanup(p->pMulti);
  }

  free_parse_switches(&p->cmd);
  sqlite3_free(p->aHash);
  sqlite3_free(p->aCacheUsed);
  sqlite3_free(p);
}

static void bdOpenDatabaseFile(
  sqlite3_vfs *pVfs,              /* VFS for accessing file-system */
  int iPort,                      /* Port that daemon is listening on */
  const char *zCont,              /* Container name */
  const char *zDb,                /* Database (file) name */
  const u8 *aId                   /* Database id */
){
  /* If zDb is zero bytes in length, this function is a no-op */
  if( zDb[0] ){
    int rc;                       /* VFS method return code */
    sqlite3_file *pFile = 0;      /* Database file */
    volatile void *aMap = 0;      /* shm for database file */

    rc = bdOpenLocalFile(zCont, zDb, 0, &pFile);
    if( rc!=SQLITE_OK ){
      fatal_error("failed to open %s/%s (%d)", zCont, zDb, rc);
    }
    bdPopulateDatabase(pFile, iPort, zCont, zDb, aId);

    /* It may be that the current process is being started because an
    ** earlier daemon process was shut down abruptly, leaving existing
    ** clients that may have open read or write transactions, or ongoing
    ** checkpoint operations. Obtain and then release EXCLUSIVE locks on
    ** the reader, writer and checkpointer slots for this file to ensure
    ** that all such operations have concluded before proceeding. */
    rc = pFile->pMethods->xShmMap(pFile, 0, 32*1024, 1, &aMap);
    if( rc!=SQLITE_OK ){
      fatal_error("cannot map shm for file %s/%s", zCont, zDb);
    }
    rc = pFile->pMethods->xShmLock(pFile, 0, SQLITE_SHM_NLOCK, 
        SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE
    );
    if( rc!=SQLITE_OK ){
      fatal_error("cannot lock database file %s/%s", zCont, zDb);
    }
    memset((void*)aMap, 0, 16);
    pFile->pMethods->xShmLock(pFile, 0, SQLITE_SHM_NLOCK,
        SQLITE_SHM_UNLOCK|SQLITE_SHM_EXCLUSIVE
    );
    pFile->pMethods->xShmUnmap(pFile, 0);
    bdCloseLocalFile(pFile);
  }
}

static void bdContainerIter(
  DaemonCtx *pCtx,                /* Daemon context object */
  int nCont,                      /* Number of entries in azCont[] */
  const char **azCont,            /* Container names passed on command line */
  sqlite3_stmt **ppStmt           /* OUT: statement to iterate containers */
){
  char *zSql = 0;
  int ii;
  int rc;
  char *zList = bcvMprintf("SELECT NULL");
  for(ii=0; ii<nCont; ii++){
    zList = bcvMprintf("%z UNION ALL SELECT %Q", zList, azCont[ii]);
  }
  zSql = bcvMprintf(
      "WITH cmd(c) AS (%z) "
      "SELECT container, manifest, etag FROM manifest "
      "UNION ALL "
      "SELECT c, NULL, NULL FROM cmd WHERE c IS NOT NULL AND c NOT IN ("
      "  SELECT container FROM manifest"
      ")", zList
  );

  rc = sqlite3_prepare_v2(pCtx->db, zSql, -1, ppStmt, 0);
  if( rc!=SQLITE_OK ){
    fatal_sql_error(pCtx->db, "bdContainerIter()");
  }
  sqlite3_free(zSql);
}

static int bdWrongAccount(void *pData, int n, char **azVal, char **azCol){
  DaemonCtx *pCtx = (DaemonCtx*)pData;
  fatal_error(
      "daemon restarted with unexpected account name "
      "- expected \"%s\", have \"%s\"", azVal[0], pCtx->cmd.zAccount
  );
  return 0;
}

/*
** Check that the account name used by the daemon represented by pCtx
** is the same as the one in the blocksdb.bcv database just opened. If
** not, call fatal_error() to exit.
*/
static void bdCheckAccountName(DaemonCtx *pCtx){
  int rc;
  char *zSql = bcvMprintf(
      "SELECT account FROM manifest WHERE account!=%Q", pCtx->cmd.zAccount
  );
  rc = sqlite3_exec(pCtx->db, zSql, bdWrongAccount, (void*)pCtx, 0);
  if( rc!=SQLITE_OK ){
    fatal_sql_error(pCtx->db, "bdCheckAccountName()");
  }
  sqlite3_free(zSql);
}

/*
** Populate the portnumber.bcv file used by the [attach] and [detach]
** commands to locate the daemon's listening port.
*/
static void bdWritePortNumber(DaemonCtx *pCtx){
  char aBuf[64];
  sqlite3_file *pFile = 0;
  int rc;

  sqlite3_snprintf(sizeof(aBuf)-1, aBuf, "%d", pCtx->iListenPort);

  rc = bdOpenLocalFile(0, "portnumber.bcv", 0, &pFile);
  if( rc==SQLITE_OK ){
    rc = pFile->pMethods->xTruncate(pFile, 0);
  }
  if( rc==SQLITE_OK ){
    rc = pFile->pMethods->xWrite(pFile, (void*)aBuf, strlen(aBuf), 0);
  }
  if( rc!=SQLITE_OK ){
    fatal_error("failed to create and populate file: portnumber.bcv");
  }
  bdCloseLocalFile(pFile);
}

/*
** Command: $argv[0] daemon ?SWITCHES? CONTAINER1 [CONTAINER2...]
**
** List all containers in the specified account.
*/
static int main_daemon(int argc, char **argv){
  CommandSwitches *pCmd;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  sqlite3_file *pFile = 0;
  CurlRequest curl;               /* Used to grab intitial manifests */
  DaemonCtx *p = 0;
  int iCont;
  const char **azArg = (const char**)&argv[2];
  int nArg = argc-2;
  int rc;
  int f;
  int iPort;                      /* Port that daemon listens on */
  i64 nUsed;
  i64 iGCTime;                    /* Time for first GC attempt */
  sqlite3_stmt *pReader = 0;      /* For reading "block" table */
  sqlite3_stmt *pIter = 0;        /* For reading "manifest" table */

  pFile = (sqlite3_file*)bcvMallocZero(pVfs->szOsFile);
  p = (DaemonCtx*)bcvMallocZero(sizeof(DaemonCtx));
  p->pMulti = curl_multi_init();
  pCmd = &p->cmd;

  /* Parse command line */
  if( argc<=2 ) daemon_usage(argv[0]);
  parse_switches(pCmd, azArg, nArg, &iCont,
    COMMANDLINE_ACCESSKEY | COMMANDLINE_ACCOUNT | COMMANDLINE_VERBOSE |
    COMMANDLINE_PORT | COMMANDLINE_CACHESIZE | COMMANDLINE_DIRECTORY |
    COMMANDLINE_AUTOEXIT | COMMANDLINE_POLLTIME | COMMANDLINE_DELETETIME |
    COMMANDLINE_GCTIME | COMMANDLINE_RETRYTIME | COMMANDLINE_LOG |
    COMMANDLINE_NODELETE | COMMANDLINE_EMULATOR | COMMANDLINE_NWRITE |
    COMMANDLINE_NDELETE | COMMANDLINE_READYMSG | COMMANDLINE_PERSISTENT
  );
  if( pCmd->zAccount==0 )   missing_required_switch("-account");
  if( pCmd->zAccessKey==0 ) missing_required_switch("-accesskey");
  if( pCmd->zDirectory==0 ) missing_required_switch("-directory");
  if( pCmd->szCache<=0 ) pCmd->szCache = BCV_DEFAULT_CACHEFILE_SIZE;

  if( pCmd->nWrite<=0 ) pCmd->nWrite = pCmd->zEmulator?1:BCV_DEFAULT_NWRITE;
  if( pCmd->nDelete<=0 ) pCmd->nDelete = pCmd->zEmulator?1:BCV_DEFAULT_NDELETE;
  if( pCmd->nPollTime<=0 ) pCmd->nPollTime = BCV_DEFAULT_POLLTIME;
  if( pCmd->nDeleteTime<=0 ) pCmd->nDeleteTime = BCV_DEFAULT_DELETETIME;
  if( pCmd->nGCTime<=0 ) pCmd->nGCTime = BCV_DEFAULT_GCTIME;
  if( pCmd->nRetryTime<=0 ) pCmd->nRetryTime = BCV_DEFAULT_RETRYTIME;

  if( iCont>=nArg ) daemon_usage(argv[0]);
  bcvAccessPointNew(&p->ap,pCmd->zAccount,pCmd->zAccessKey,pCmd->zEmulator);

  /* Open the listen socket */
  iPort = bcvDaemonListen(p);

  /* Change to the daemon -directory dir. */
  rc = chdir(pCmd->zDirectory);
  if( rc!=0 ){
    fatal_system_error("chdir()", errno);
  }

  /* Open the SQLite database used to store the dirty block list. Ensure
  ** we have an EXCLUSIVE lock on this db. This guarantees no other daemon
  ** process is also using this directory.  */
  rc = sqlite3_open(BCV_DATABASE_NAME, &p->db);
  if( rc!=SQLITE_OK ){
    fatal_error("failed to open database %s", BCV_DATABASE_NAME);
  }
  rc = sqlite3_exec(p->db,
    "PRAGMA locking_mode = EXCLUSIVE;"
    "BEGIN EXCLUSIVE;"
    BCV_DATABASE_SCHEMA
    "COMMIT;", 0, 0, 0
  );
  if( rc!=SQLITE_OK ){
    fatal_error("failed to lock database %s", BCV_DATABASE_NAME);
  }

  bdCheckAccountName(p);
  bdWritePortNumber(p);

  curlRequestInit(&curl, pCmd->bVerbose);
  iGCTime = sqlite_timestamp() + pCmd->nGCTime*1000;
  bdContainerIter(p, nArg-iCont, &azArg[iCont], &pIter);
  while( SQLITE_ROW==sqlite3_step(pIter) ){
    const char *zCont = (const char*)sqlite3_column_text(pIter, 0);
    int nCont = sqlite3_column_bytes(pIter, 0);
    const void *aMan = sqlite3_column_blob(pIter, 1);
    int nMan = sqlite3_column_bytes(pIter, 1);
    const char *zETag = (const char*)sqlite3_column_text(pIter, 2);
    
    Manifest *pMan;
    int iDb;

    DContainer *pNew = (DContainer*)bcvMallocZero(sizeof(DContainer)+nCont+1);
    pNew->zName = (char*)&pNew[1];
    pNew->iGCTime = iGCTime;
    memcpy(pNew->zName, zCont, nCont);
    if( nMan>0 ){
      pMan = bcvManifestParse(bcvMemdup(nMan, aMan), nMan, bcvStrdup(zETag));
      bdManifestInstall(p, pNew, pMan, 0, 0);
      pNew->iPollTime = sqlite_timestamp();
    }else{
      pMan = bcvManifestGetParsed(&curl, &p->ap, zCont, &nMan);
      bdManifestInstall(p, pNew, pMan, pMan->pFree, nMan);
      curlRequestReset(&curl);
    }

    bcvDaemonMkdir(zCont);
    for(iDb=0; iDb<pNew->pManifest->nDb; iDb++){
      ManifestDb *pDb = &pNew->pManifest->aDb[iDb];
      bdOpenDatabaseFile(pVfs, iPort, zCont, pDb->zDName, pDb->aId);
    }

    pNew->pNext = p->pContainerList;
    p->pContainerList = pNew;
  }
  curlRequestFinalize(&curl);
  sqlite3_finalize(pIter);

  /* Allocate space for the hash table and cache bitmap */
  p->szBlk = p->pContainerList->pManifest->szBlk;
  p->nCacheMax = (int)(pCmd->szCache / p->szBlk);
  p->nHash = 2;
  while( p->nHash<p->nCacheMax ) p->nHash = p->nHash*2;
  p->aHash = (DCacheEntry**)bcvMallocZero(p->nHash * sizeof(DCacheEntry*));
  p->nCacheUsedEntry = (p->nCacheMax/32)+1;
  p->aCacheUsed = (u32*)bcvMallocZero(p->nCacheUsedEntry * sizeof(u32));

  /* If the -persistent flag was not passed, delete all read-only entries
  ** from the database.  */
  if( pCmd->bPersistent==0 ){
    rc = sqlite3_exec(p->db, 
        "DELETE FROM block WHERE blockid IS NOT NULL", 0, 0, 0
    );
    if( rc!=SQLITE_OK ){
      fatal_error("error in SQL: %s", sqlite3_errmsg(p->db));
    }
  }
  
  /* If there are any entries for dirty blocks in the blocks table, add 
  ** them to the hash table now. */
  rc = sqlite3_prepare_v2(p->db, 
      "SELECT cachefilepos, blockid, dbpos, container, db FROM block",
      -1, &pReader, 0
  );
  if( rc!=SQLITE_OK ){
    fatal_error("error in SQL: %s", sqlite3_errmsg(p->db));
  }
  while( sqlite3_step(pReader)==SQLITE_ROW ){
    int iCacheFilePos = sqlite3_column_int(pReader, 0);
    int nName = 0;
    int bDirty = 0;
    u8 aRnd[BCV_MAX_NAMEBYTES];
    const u8 *aName = 0;

    if( sqlite3_column_type(pReader, 1)==SQLITE_NULL ){
      int iDbPos = sqlite3_column_int(pReader, 2);
      const char *zContainer = (const char*)sqlite3_column_text(pReader, 3);
      const u8 *aId = (const u8*)sqlite3_column_blob(pReader, 4);
      DContainer *pCont = bdFindContainer(p, zContainer);

      if( pCont ){
        Manifest *pMan = pCont->pManifest;
        int iDb = bcvDaemonFindDb(pCont, aId);
        if( iDb>=0 ){
          nName = NAMEBYTES(pMan);
          bDirty = 1;
          aName = aRnd;
          sqlite3_randomness(nName, aRnd);
          bdManifestWriteBlock(&pMan->aDb[iDb], iDbPos, aRnd, nName);
        }
      }
    }else{
      nName = sqlite3_column_bytes(pReader, 1);
      aName = (const u8*)sqlite3_column_blob(pReader, 1);
    }

    /* If aName points to a buffer (not NULL), create a cache entry. Mark
    ** as dirty if bDirty is true.  */
    if( aName ){
      DCacheEntry *pEntry;
      pEntry = (DCacheEntry*)bcvMallocZero(sizeof(DCacheEntry)+nName);
      memcpy(pEntry->aName, aName, nName);
      pEntry->nName = nName;
      pEntry->iPos = iCacheFilePos;
      pEntry->bDirty = bDirty;
      bdCacheHashAdd(p, pEntry);
      bcvDaemonAddToLRU(p, pEntry);
      p->nCacheEntry++;
      p->aCacheUsed[pEntry->iPos / 32] |= (1 << (pEntry->iPos%32));
    }
  }
  rc = sqlite3_finalize(pReader);
  pReader = 0;
  daemon_event_log(p, "added %d dirty blocks to hash", p->nCacheEntry);

  /* Open the cache file */
  f = SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;
  rc = pVfs->xOpen(pVfs, BCV_CACHEFILE_NAME, pFile, f, &f);
  if( rc!=SQLITE_OK ){
    fatal_error("failed to open cachefile: %s", BCV_CACHEFILE_NAME);
  }
  p->pCacheFile = pFile;

  /* If the -readymessage option was passed, output the message "ready\n"
  ** on stdout to indicate that initialization has finished and database
  ** clients may expect to access databases successfully. This is used
  ** by test scripts.  */
  if( p->cmd.bReadyMessage ){
    fprintf(stdout, "ready\n");
    fflush(stdout);
  }

  /* Run the daemon main loop. This only exits if (a) the -autoexit option
  ** was passed and (b) the number of connected database clients drops to
  ** zero.  */
  bdMainloop(p);

  /* The main loop has exited. Attempt to clean up all memory allocations,
  ** then output an error message if this reveals that this process has 
  ** leaked memory.  */
  bdCleanup(p);
  sqlite3_reset_auto_extension();
  nUsed = sqlite3_memory_used();
  if( nUsed>0 ){
    daemon_error("daemon leaked %d bytes of memory", (int)nUsed);
    return 1;
  }

  return 0;
}


/*
** Command: $argv[0] [attach|detach] DIR CONTAINER
**
** Send a message to a running daemon process to attach or detach a 
** container. 
*/
static int main_attach(int argc, char **argv, int bAttach){
  sqlite3_file *pFile = 0;        /* File handle for portnumber.bcv */
  char aPort[64];                 /* Contents of portnumber.bcv */
  int iPort = 0;                  /* Port number to connect to */
  char *zDir = 0;                 /* Directory argument */
  char *zCont = 0;                /* Container argument */
  BCV_SOCKET_TYPE s;              /* Socket connection to daemon */
  struct sockaddr_in addr;        /* 127.0.0.1:iPort */
  DMessage msg;
  DMessage reply;
  int rc;
  u8 *aFree = 0;

  if( argc<4 ){
    fprintf(stderr, "Usage: %s %s ?SWITCHES? DBFROM DBTO\n", 
        argv[0], bAttach ? "attach" : "detach"
    );
    return 1;
  }
  zDir = argv[2];
  zCont = argv[3];

  memset(aPort, 0, sizeof(aPort));
  rc = bdOpenLocalFile(zDir, "portnumber.bcv", 1, &pFile);
  if( rc==SQLITE_OK ){
    i64 nByte;
    rc = pFile->pMethods->xFileSize(pFile, &nByte);
    if( rc==SQLITE_OK ){
      if( nByte>=sizeof(aPort) ){
        rc = SQLITE_ERROR;
      }else{
        rc = pFile->pMethods->xRead(pFile, aPort, nByte, 0);
      }
    }
  }
  if( rc==SQLITE_OK ){
    int ii;
    for(ii=0; aPort[ii]; ii++){
      iPort = iPort*10 + (aPort[ii] - '0');
    }
  }
  bdCloseLocalFile(pFile);
  if( rc!=SQLITE_OK ){
    fatal_error("failed to read port number from %s/portnumber.bcv", zDir);
  }

  s = socket(AF_INET, SOCK_STREAM, 0);
  if( bcv_socket_is_valid(s)==0 ){
    fatal_system_error("socket()", errno);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(iPort);
  ((u8*)&addr.sin_addr)[0] = 0x7F;
  ((u8*)&addr.sin_addr)[1] = 0x00;
  ((u8*)&addr.sin_addr)[2] = 0x00;
  ((u8*)&addr.sin_addr)[3] = 0x01;
  if( connect(s, (struct sockaddr*)&addr, sizeof(addr))<0 ){
  printf("port is %d\n", iPort);
    fatal_system_error("connect()", errno);
  }

  msg.eType = BCV_MESSAGE_QUERY;
  msg.aBlob = (u8*)bcvMprintf("bcv_%s=%s", bAttach ?"attach":"detach", zCont);
  msg.nBlobByte = strlen((char*)msg.aBlob);
  if( bdMessageSend(s, &msg) ){
    fatal_error("failed to send message to daemon");
  }
  if( bdClientMessage(s, &reply, &aFree) ){
    fatal_error("failed to receive reply from daemon");
  }
  sqlite3_free(aFree);

  printf("%.*s\n", reply.nBlobByte, (char*)reply.aBlob);
  return 0;
}

void bcvRestoreDefaultVfs(void);

int main(int argc, char **argv){
  SelectOption aCmd[] = {
    { "upload",   1 , 0},
    { "download", 1 , 1},
    { "clist",    1 , 2},
    { "list",     1 , 3},
    { "manifest", 1 , 4},
    { "delete",   1 , 5},
    { "destroy",  1 , 6},
    { "create",   1 , 7},
    { "daemon",   1 , 8},
    { "files",    1 , 9},
    { "copy",     1 , 10},
    { "attach",   1 , 11},
    { "detach",   1 , 12},
    { 0, 0 }
  };
  int rc;
  int iCmd;

  bcv_socket_init();
  curl_global_init(CURL_GLOBAL_ALL);
  base64_init();
  OpenSSL_add_all_algorithms();

  sqlite3_initialize();
  bcvRestoreDefaultVfs();

  iCmd = select_option(argc>=2 ? argv[1] : "", aCmd, 0xFFFFFFFF, 1);
  switch( aCmd[iCmd].eVal ){
    case 0:
      rc = main_upload(argc, argv);
      break;
    case 1:
      rc = main_download(argc, argv);
      break;
    case 2:
      rc = main_clist(argc, argv);
      break;
    case 3:
      rc = main_list(argc, argv);
      break;
    case 4:
      rc = main_manifest(argc, argv);
      break;
    case 5:
      rc = main_delete(argc, argv);
      break;
    case 6:
      rc = main_destroy(argc, argv);
      break;
    case 7:
      rc = main_create(argc, argv);
      break;
    case 8:
      rc = main_daemon(argc, argv);
      break;
    case 9:
      rc = main_files(argc, argv);
      break;
    case 10:
      rc = main_copy(argc, argv);
      break;
    case 11:
      rc = main_attach(argc, argv, 1);
      break;
    case 12:
      rc = main_attach(argc, argv, 0);
      break;
  }

  return rc;
}

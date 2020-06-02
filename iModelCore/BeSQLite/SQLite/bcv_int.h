/*
** 2020-05-12
**
******************************************************************************
**
** This header file contains declarations for interfaces in bcvutil.c used
** exclusively by blockcachevfsd.c. This file should not be directly
** included by applications.
*/

#define BCV_STORAGE_AZURE  0
#define BCV_STORAGE_GOOGLE 1

#include "sqlite3.h"
#include "bcvutil.h"
#include "blockcachevfs.h"
#include "simplexml.h"
#include <curl/curl.h>

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

#define BCV_CACHEFILE_NAME "cachefile.bcv\0"
#define BCV_DATABASE_NAME  "blocksdb.bcv"

#define MIN(a,b)  ((a)<(b) ? (a) : (b))
#define MAX(a,b)  ((a)>(b) ? (a) : (b))


#define BCV_DEFAULT_NAMEBYTES        16
#define BCV_DEFAULT_BLOCKSIZE        (4*1024*1024)

typedef unsigned int u32;
typedef unsigned char u8;
typedef sqlite3_uint64 u64;
typedef sqlite3_int64 i64;

#define BCV_HTTP_GET           1
#define BCV_HTTP_PUT           2
#define BCV_HTTP_DELETE        3


typedef struct CurlRequest CurlRequest;
struct CurlRequest {
  CURL *pCurl;
  struct curl_slist *pList;
  int bVerbose;
  int bGen;                       /* True if have seen x-goog-metageneration */
  char *zStatus;
  char *zETag;

  char *zHttpUrl;                 /* URL used by this curl request */
  int eHttpMethod;                /* Method used by this curl request */
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

typedef struct AccessPoint AccessPoint;
struct AccessPoint {
  int bDoNotUse;                  /* This is a template, not for actual use */
  int eStorage;                   /* BCV_STORAGE_XXX constant */
  const char *zAccount;
  const char *zAccessKey;
  const char *zEmulator;
  char *zSas;                     /* Shared access signature */
  char *zSlashAccountSlash;
  u8 *aKey;                       /* zAccessKey decoded from base64 */
  int nKey;                       /* Size of buffer aKey[] in bytes */
};

int bcvAccessPointInit(AccessPoint*,int,const char*,const char*,char**);
void bcvAccessPointFree(AccessPoint *p);
void bcvAccessPointCopy(AccessPoint *pTo, AccessPoint *pFrom);

int bcvStrcmp(const char *zLeft, const char *zRight);
int bcvStrlen(const char *z);

char *bcvBase64Encode(const unsigned char *aIn, int nIn);

char *bcvMprintf(const char *zFmt, ...);
void *bcvMalloc(int nByte);
void *bcvRealloc(void *a, int nByte);
void *bcvMallocZero(int nByte);
u8 *bcvMemdup(int nIn, const u8 *aIn);
char *bcvStrdup(const char *zIn);

i64 sqlite_timestamp(void);

int bcv_isdigit(char c);
int bcv_isspace(char c);
int parse_number(const char *zNum, int *piVal);

u32 bcvGetU16(const u8 *a);
void bcvPutU32(u8 *a, u32 iVal);
u32 bcvGetU32(const u8 *a);
void bcvPutU64(u8 *a, u64 iVal);
u64 bcvGetU64(const u8 *a);

size_t curlHeaderFunction(char *buf, size_t sz, size_t n, void *pUser);
void curlRequestInit(CurlRequest *p, int bVerbose);
void curlRequestReset(CurlRequest *p);
void curlRequestFinalize(CurlRequest *p);
void curlFetchBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
);
void curlPutBlob(
  CurlRequest *p,
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile,
  const char *zETag,
  int nByte
);
void curlCreateContainer(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer
);
void curlDestroyContainer(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer
);
size_t bdIgnore(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
);
void curlDeleteBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
);

int bcvManifestParse(u8 *a, int n, char *zETag, Manifest **ppOut, char **pz);

u8 *bcvManifestCompose(Manifest *p, int *pnOut);
void bcvManifestFree(Manifest *p);
void bcvManifestDeref(Manifest *p);
Manifest *bcvManifestRef(Manifest *p);

int bcvManifestNameToIndex(Manifest *p, const char *zDb);
ManifestHash *bcvMHashBuild(Manifest *pMan, int iExclude);
u8 *bcvMHashMatch(ManifestHash *pHash, u8 *aPrefix, int nPrefix);
void bcvMHashFree(ManifestHash *pHash);
int bcvMHashBucket(ManifestHash *pHash, u8 *pBlk);

size_t memoryUploadRead(char *pBuffer, size_t n1, size_t n2, void *pCtx);
size_t memoryUploadSeek(void *pCtx, curl_off_t offset, int origin);

void bcvBlockidToText(Manifest *p, const u8 *pBlk, char *aBuf);

size_t fileDownloadWrite(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
);
size_t memoryDownloadWrite(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
);

void bcvParseFiles(
  int eStorage,
  const u8 *aBuf, int nBuf,
  FilesParse *pParse
);
void bcvParseFilesClear(FilesParse *pParse);

void curlListFiles(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zNextMarker
);

/*
** Below here should be eventually moved back to blockcachevfsd. 
*/
void fatal_oom_error(void);

void azure_date_header(char *zBuffer);
char *azure_auth_header(AccessPoint *pA, const char *zStringToSign);
char *azure_base_uri(AccessPoint *p);

void hex_encode(const unsigned char *aIn, int nIn, char *aBuf);
int hex_decode(const char *aIn, int nIn, u8 *aBuf);


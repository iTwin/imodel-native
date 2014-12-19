
/*
** Copyright (c) 2010-2011 Hipp, Wyrick & Company, Inc.
** 6200 Maple Cove Lane, Charlotte, NC 28269 USA
** +1.704.948.4565
**
** All rights reserved.
**
*/
#ifdef SQLITE_ENABLE_ZIPVFS

#ifndef SQLITE_AMALGAMATION
# include <sqlite3.h>
# include "sqliteInt.h"
#endif

/*
** Defines SQLITE_OPEN_URI and SQLITE_FCNTL_OVERWRITE were added to 
** sqlite3.h for version 3.7.7 and 3.7.9, respectively. If they are not
** defined, define them here so that zipvfs can be built with older
** versions of SQLite.
**
** Also, if SQLITE_OPEN_URI is not defined, do not attempt to link sqlite
** library symbol sqlite3_uri_parameter().
*/
#ifndef SQLITE_OPEN_URI
# define SQLITE_OPEN_URI 0x00000040
# define sqlite3_uri_parameter(x,y) 0
#endif
#ifndef SQLITE_FCNTL_OVERWRITE
# define SQLITE_FCNTL_OVERWRITE 11
#endif
#ifndef SQLITE_FCNTL_PRAGMA
# define SQLITE_FCNTL_PRAGMA 14
#endif

#ifndef _ZIPVFS_H
# include "zipvfs.h"
#endif

/*
** This file contains the implementation of the zipvfs VFS extension. VFS
** instances are created by calls to zipvfs_create_vfs() specifying callback
** functions to compress and decompress buffers of data. VFS instances created
** this way have the following features:
**
** 1. If a file is opened with the SQLITE_OPEN_MAIN_DB flag, then the file
**    is expected to be a zipvfs database file. When SQLite writes a page
**    of data to a zipvfs database file, the page image is compressed before
**    it is stored in the file. When SQLite reads a page of data from a
**    zipvfs database file, the compressed data is read from the file and
**    decompressed before being returned. 
**
**    Zipvfs therefore provides a compressed database format for SQLite,
**    at the cost of the extra complexity and inefficiency that comes from
**    storing variable sized pages instead of fixed size pages. The 
**    file-format used to organize the collection of variable sized pages
**    is described below.
**
**    Zipvfs database files are written to via an SQLite pager module. As a
**    result write operations are robust and transactional. When using a 
**    zipvfs database file, the user does not risk database corruption or
**    data loss in the event of a power failure even when running with
**    "PRAGMA journal_mode=OFF" (since zipvfs databases have their own
**    persistent journal file created by the pager module to preven 
**    corruption).
**
** 2. If a file is opened with the SQLITE_OPEN_MAIN_JOURNAL flag set, then
**    a temp file is opened and returned instead. This temp file may be used
**    by SQLite for explicit transaction and savepoint rollbacks, but if an
**    application or power failure occurs during a write operation, it is
**    the journal file created by the zipvfs pager module that is rolled back
**    to restore the database by the next client.
**
**    The xAccess() method of this VFS (function zipvfsAccess) is rigged so 
**    that SQLite never detects a hot-journal in the file-system. If there is
**    a file that ends in "-journal" in the file-system, it is handled by the
**    zipvfs pager module, not by SQLite.
*/

/*
** FILE FORMAT:
**
**   The file is broken into three sections, the header, the page-map and the
**   data area. 
**
**   The first 100 bytes of the file are not used by this module, though many
**   of the bytes in this range are initialized by this module.  The first 100
**   bytes are mainly used by the underlying pager module layer for storing
**   information such as the database change counter.  This module initializes
**   the first 92 bytes to a copy of the first 92 bytes in the database file
**   except that the first 16 bytes are changed from "SQLite version 3" to
**   to "ZV-name" where "name" is the name assigned to this module by
**   the first parameter to zipvfs_create_vfs(). The "name", which is
**   guaranteed to be 13 bytes or less in size, is padded with 0x00 bytes to
**   make up the full 16 bytes. The last 8 bytes of the 100-byte header, and 
**   some other bytes in the interior of the header, are set by the underlying
**   pager layer.
**
**   The second 100 bytes of space are reserved for the ZIPVFS header. The
**   ZIPVFS header contains a serialization of the ZipvfsHdr structure
**   (see below).
**
**   The page-map begins at byte offset 200 of the file. It contains an 8-byte
**   entry for each page in the users database. The entry consists of the
**   following:
**
**     * Offset of compressed page image in file.         5 bytes (40 bits). 
**     * Size of compressed page image in file.           17 bits.
**     * Number of unused bytes (if any) in slot.         7 bits.
**
**   The data-area begins at some point in the file after the end of the 
**   page-map. An exact offset is stored in the file header (struct ZipvfsHdr).
**   The data-area contains the compressed page images. Each page is stored as
**   follows:
**
**     * The page number (31 bits)
**     * The slot payload size (17 bits)
**     * The compressed page image,
**     * Free space (if any),
**
**   When SQLite overwrites a database page, the compressed image may change
**   size. As a result, Zipvfs may not (usually does not) write the new
**   compressed image back to the same point in the file as the old. The
**   part of the file then occupied by the old compressed image becomes a
**   "free-slot". It may be reused later.
**
**   Free slots are organized using a b-tree that allows for the "best fit"
**   slot to be located efficiently, even for large free lists. Btree nodes
**   are themselves stored within free-slots. See a separate block comment
**   further down in this source file for details.
**
**   The data-area may also contain a "gap", defined by two offsets stored
**   in the header. A gap is a chunk of completely unused space. Neither
**   compressed images or free-slots are found within a gap. A gap is created
**   and manipulated by incremental-compact options (see file-control
**   ZIPVFS_CTRL_COMPACT).
**
**
** READING:
**
**   Reading a page from the file is simple. First lookup the entry for
**   the requested page in the page map. Then read the blob of data from
**   the data area and decompress it.
**
**   If there is no write transaction active and the pager is running
**   in rollback mode, data can be read directly from the file, bypassing
**   the pager module.
**
**
** WRITING:
**
**   The upper layer writes to the file a page at a time. If the upper 
**   layer is overwriting an existing page, then the slot that currently
**   contains the compressed page image is added to the free slot b-tree.
**
**   This module then compresses the page image and writes it into the 
**   file. There are two ways in which this can be done:
**
**     1. The new page may be inserted into a free slot, or
**     2. The new page may be appended to the end of the data area.
**
**   If there are less than N free slots in the database, then option 1
**   is only taken if an exact fit can be found - a free-slot of the exact
**   size required by the compressed page. If there are greater than or
**   equal to N free slots, then option 1 is taken if there exists any
**   free-slot large enough to accommodate the compressed page. The
**   btree mechanism described further down in this file is used to locate
**   the best fit efficiently.
**
**   The value of N is configurable using the ZIPVFS_CTRL_MAXFREE 
**   file-control. 
*/


#define ZIPVFS_DEFAULT_MAXFREE 100
#define ZIPVFS_DEFAULT_MAXFRAG 200

/*
** The maximum number of bytes allowed in the UTF-8 rendering of a zipvfs
** VFS name (the string passed as the first argumen to zipvfs_create_vfs()).
** This is based on fitting the string "ZV-XXX", where XXX is a zipvfs VFS
** name, in the first 16 bytes of the database file.
*/
#define ZIPVFS_MAX_NAME 13

typedef struct zipvfs_file zipvfs_file;
typedef struct ZipvfsHdr ZipvfsHdr;
typedef struct ZipvfsVfs ZipvfsVfs;

/*
** A structure that contains the same data as the zipvfs file header. 
** Serialization and deserialization is done using the zipvfsReadHeader() 
** and zipvfsWriteHeader() functions.
**
** This version of zipvfs can read and write files with parameter iVersion
** set to values between 0 and 2, as follows:
**
**     0: Rollback journal, pending-byte page is the same size as the 
**        default page-size on the current system.
**     1: Rollback journal, pending-byte page is 64K in size.
**     2: WAL mode file, pending-byte page is 64K in size.
**
** All new zipvfs files created by the current software version have 
** iVersion set to either 1 or 2. Unless it can first be upgraded to 
** version 1, WAL mode may not be used with a version 0 zipvfs file.
*/
struct ZipvfsHdr {
  i64 iFreeSlot;                  /* Byte offset of first free-slot in file */
  i64 iDataStart;                 /* Byte offset of first byte in data-area */
  i64 iDataEnd;                   /* Offset of first byte following data-area */
  i64 iGapStart;                  /* Offset of first byte in gap region */
  i64 iGapEnd;                    /* Offset of first byte after gap region */
  i64 iSize;                      /* Size of user database in bytes */
  int pgsz;                       /* User db page size */
  i64 nFreeSlot;                  /* Number of free-slots in file */
  i64 nFreeByte;                  /* Total size of all free-slots */
  i64 nFreeFragment;              /* Total size of all padding in used slots */
  int iVersion;                   /* Zipvfs file version */
}; 

/*
** File handles opened using this VFS are structures of the following type.
**
** inWriteTrans:
**   Set to 0 when no transaction or a read transaction is open. Set to 1
**   after a write-transaction has started (when SQLite requests a reserved
**   lock). Set to 2 after SQLite requests an exclusive lock on the file.
*/
struct zipvfs_file {
  sqlite3_file base;              /* Base class */
  ZipvfsHdr hdr;                  /* Cache of current file header */
  ZipvfsMethods methods;          /* Compress/uncompress etc. callbacks */
  Pager *pPager;                  /* Pager handle for this file */
  DbPage *pPage1;                 /* Page 1 of file pPager */
  int nPgsz;                      /* Page-size used by pPager */
  int nBlocksize;                 /* Page-size configured by URI parameter */
  int inWriteTrans;               /* Non-zero when in a write transaction */
  int nMaxFree;                   /* Configured with ZIPVFS_CTRL_MAXFREE */
  int nMaxFrag;                   /* Configured using ZIPVFS_CTRL_MAXFRAG */
  ZipvfsVfs *pZipVfs;             /* Associated VFS */
  int eErrorCode;                 /* If non-zero, fd is in error mode */
  u8 *aCompressionBuffer;         /* sqlite3_malloc'd compression buffer */
  int nCompressionBuffer;         /* size of aCompressionBuffer in bytes */
  int openFlags;                  /* Flags passed into xOpen() */
  const char *zFilename;          /* Filename passed into xOpen() */
  const char *zJournal;           /* For main dbs, the journal file */
  const char *zWal;               /* For main dbs, the WAL file */
  u8 doIntegrityCheck;            /* Use expensive corruption detection */
  u8 bAutoDetect;                 /* True to detect compression type */
  u8 bGetMethodsDone;             /* True if methods.xCompressClose is req. */
  zipvfs_file *pNext;             /* List of all main files for this VFS */
  int bOvwrPage1;                 /* Set after page 1 is written by vacuum */
  u32 iOvwrNext;                  /* Next page vacuum may write */
  int nAutoCheckpoint;            /* In WAL mode, the auto-checkpoint limit */
#ifdef SQLITE_TEST
  int bCreateVersion0;            /* If true, create version 0 files */
  ZipvfsWriteCb xWrite;           /* Write callback (if any) */
#endif
};

/*
** A call to zipvfs_create_vfs allocates a structure of the following type.
** After ZipvfsVfs.base has been populated with the various VFS methods,
** the newly allocated structure is registered with sqlite using
** sqlite3_vfs_register().
*/
struct ZipvfsVfs {
  sqlite3_vfs base;
  zipvfs_file *pFirst;            /* Head of list of all open MAIN_DB files */
  void *pGetMethodsCtx;
  int (*xGetMethods)(void *, const char *, const char *, ZipvfsMethods *);
  int (*xLegacyOpen)(void*, const char *zFile, void**);
  ZipvfsMethods dflt;
};

/* Number of bytes reserved before the file-header. This module never
** reads or writes to the first ZIPVFS_FILEHEADER_RESERVED bytes of the
** file. This is because the pager module uses some of the fields 
** internally (i.e. change-counter).  */
#define ZIPVFS_FILEHEADER_RESERVED   100

/* Number of bytes for file-header. */
#define ZIPVFS_FILEHEADER_SIZE       100

/* Initial amount of space reserved for the page map. */
#define ZIPVFS_PAGEMAP_INIT_SIZE     (1*256)

/* Minimum payload size for a slot. This is dictated by the format of
** internal nodes used as part of the free-slot b-tree. A slot must have
** a large enough payload to store an interior b-tree node that contains
** two entries.  */
#define ZIPVFS_MIN_PAYLOAD            (6 + 5 + 2*13)

/* Size of a slot header */
#define ZIPVFS_SLOT_HDRSIZE           6

/* Maximum allowable payload. Dictated by the 17-bit field in the 
 * slot header used to store payload size. */
#define ZIPVFS_MAX_PAYLOAD            ((1 << 17) - 1)

#define ZIPVFS_LOCKING_PAGE_SIZE      65536


/*
** Method declarations for zipvfs_file.
*/
static int zipvfsClose(sqlite3_file*);
static int zipvfsRead(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
static int zipvfsWrite(sqlite3_file*, const void*,int iAmt,sqlite3_int64 iOfst);
static int zipvfsTruncate(sqlite3_file*, sqlite3_int64 size);
static int zipvfsSync(sqlite3_file*, int flags);
static int zipvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static int zipvfsLock(sqlite3_file*, int);
static int zipvfsUnlock(sqlite3_file*, int);
static int zipvfsCheckReservedLock(sqlite3_file *, int *);
static int zipvfsFileControl(sqlite3_file*, int op, void *pArg);
static int zipvfsSectorSize(sqlite3_file*);
static int zipvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for zipvfs_vfs.
*/
static int zipvfsOpen(sqlite3_vfs*, const char *, sqlite3_file*, int , int *);
static int zipvfsDelete(sqlite3_vfs*, const char *, int);
static int zipvfsAccess(sqlite3_vfs*, const char *, int, int *);
static int zipvfsFullPathname(sqlite3_vfs*, const char *, int, char *);
static void *zipvfsDlOpen(sqlite3_vfs*, const char *);
static void zipvfsDlError(sqlite3_vfs*, int nByte, char *);
static void (*zipvfsDlSym(sqlite3_vfs *, void *, const char *))(void);
static void zipvfsDlClose(sqlite3_vfs*, void*);
static int zipvfsRandomness(sqlite3_vfs*, int nByte, char *);
static int zipvfsSleep(sqlite3_vfs*, int);
static int zipvfsCurrentTime(sqlite3_vfs*, double*);
static int zipvfsGetLastError(sqlite3_vfs *, int, char *);
static int zipvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);
static int zipvfsShmMap(sqlite3_file*, int, int, int, void volatile**);
static int zipvfsShmLock(sqlite3_file*, int, int, int);
static void zipvfsShmBarrier(sqlite3_file*);
static int zipvfsShmUnmap(sqlite3_file*, int);


static sqlite3_io_methods zipvfs_io_methods = {
  2,                              /* iVersion */
  zipvfsClose,                    /* xClose */
  zipvfsRead,                     /* xRead */
  zipvfsWrite,                    /* xWrite */
  zipvfsTruncate,                 /* xTruncate */
  zipvfsSync,                     /* xSync */
  zipvfsFileSize,                 /* xFileSize */
  zipvfsLock,                     /* xLock */
  zipvfsUnlock,                   /* xUnlock */
  zipvfsCheckReservedLock,        /* xCheckReservedLock */
  zipvfsFileControl,              /* xFileControl */
  zipvfsSectorSize,               /* xSectorSize */
  zipvfsDeviceCharacteristics,    /* xDeviceCharacteristics */
  zipvfsShmMap,                   /* xShmMap */
  zipvfsShmLock,                  /* xShmLock */
  zipvfsShmBarrier,               /* xShmBarrier */
  zipvfsShmUnmap                  /* xShmUnmap */
};


/*
** Functions used to access the free-list b-tree data structure.
*/
static void zipvfsFreelistAdd(zipvfs_file *, i64, int, int *);
static void zipvfsFreelistRemove(zipvfs_file *, i64, int, int *);
static void zipvfsFreelistBestfit(zipvfs_file*,int,int,i64*,int*,int*);
static int zipvfsFreelistTest(zipvfs_file *, i64, int, int *, int *);
static void zipvfsFreelistIntegrity(zipvfs_file *, int *);
#ifdef SQLITE_TEST
static void zipvfsFreelistIterate(zipvfs_file *, ZipvfsStructureCb *, int *);
#endif

/*
** Given a pointer to a ZipvfsVfs structure, return a pointer to the 
** underlying "real" VFS.
*/
#define SYSVFS(pVfs) ((sqlite3_vfs *)((pVfs)->pAppData))

/*
** Given a pointer to a zipvfs_file structure that is not a main database
** file (implying zipvfs_file.pPager==0), return a pointer to the underlying 
** "real" file-handle.
*/
#define SYSFILE(pFile) ((sqlite3_file *)&((zipvfs_file *)pFile)[1])

#ifndef MIN
# define MIN(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef MAX
# define MAX(x,y) (((x)>(y))?(x):(y))
#endif

/*
** For tracing behavior, compile with SQLITE_DEBUG and set the
** variable zipvfsTraceEnable to true.
*/
#ifdef SQLITE_DEBUG
static FILE *zipvfsTraceFile = 0;
static void zipvfsTrace(const char *zFormat, ...){
  va_list ap;
  char zOut[300];
  va_start(ap, zFormat);
  sqlite3_vsnprintf(sizeof(zOut), zOut, zFormat, ap);
  va_end(ap);
  fprintf(zipvfsTraceFile, "ZIPVFS: %s\n", zOut);
  fflush(zipvfsTraceFile);
}
# define ZIPVFSTRACE(X) if(zipvfsTraceFile)zipvfsTrace X
#else
# define ZIPVFSTRACE(X)
#endif

/*
** The file handle passed as the first argument is a MAIN_DB file. Add it
** to the linked list headed at ZipvfsVfs.pFirst. 
**
** Before linking the file into the list, populate the zipvfs_file.zJournal 
** member variable with a pointer to the buffer containing the name of the
** corresponding rollback journal owned by the upper pager layer (the one
** that calls this modules functions). To find this pointer, we perform some
** quite unbecoming pointer arithmetic. See comments above
** zipvfsIsJournalFile() for the reason.
*/
static void zipvfsAddFileToList(zipvfs_file *pFd){
  int nJournal;
  const char *z;
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
  assert( pFd->pNext==0 );
  assert( pFd->zFilename );

  /* Questionable pointer arithmetic starts here. This works by assuming that
  ** the buffer containing the journal name is allocated immediately following
  ** the buffer that contains the database filename. */
  z = &pFd->zFilename[strlen(pFd->zFilename)];
  assert( *z==0 );
  if( (pFd->openFlags & SQLITE_OPEN_URI) ){
    int odd = 0;
    while( 1 ){
      if( z[0]==0 ){
        odd = 1 - odd;
        if( odd && z[1]==0 ) break;
      }
      z++;
    }
    pFd->zJournal = &z[2];
  }else{
    while( !*z ) z++;
    pFd->zJournal = z;
  }
  pFd->zWal = &pFd->zJournal[strlen(pFd->zJournal)+1];
  nJournal = strlen(pFd->zJournal);

  /* If SQLite is using 8.3 filenames, then the WAL file name does not
  ** immediately follow the journal file name. Instead, the journal file-name
  ** ends, there are some non-nul bytes, then another nul, then the WAL
  ** file name.  */
  if( memcmp(&pFd->zJournal[nJournal-7], "journal", 7) ){
    pFd->zWal += strlen(pFd->zWal) + 1;
  }

  /* Check that the journal and wal file names end in "nal" and "wal". */
  assert( 0==memcmp("wal", &pFd->zWal[strlen(pFd->zWal)-3], 3) );
  assert( 0==memcmp("nal", &pFd->zJournal[strlen(pFd->zJournal)-3], 3) );

  pFd->pNext = pFd->pZipVfs->pFirst;
  pFd->pZipVfs->pFirst = pFd;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
}

/*
** The file-handle passed as an argument is a MAIN_DB handle that is
** being closed. Remove it from the linked-list at ZipvfsVfs.pFirst
** and return.
*/
static void zipvfsRemoveFileFromList(zipvfs_file *pFd){
  if( pFd->zJournal ){
    zipvfs_file **pp;
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
    for(pp=&pFd->pZipVfs->pFirst; (*pp)!=pFd; pp=&((*pp)->pNext));
    *pp = pFd->pNext;
    pFd->pNext = 0;
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
  }
}

/*
** Argument zName is a copy of a pointer to a file-name buffer that has 
** been passed to either zipvfsOpen(), zipvfsAccess() or zipvfsDelete().
** This function returns true if the file-name corresponds to a journal
** file belonging to a Zipvfs database. If it is, the calling function
** modifies it's behaviour - xOpen() opens a temporary file instead of
** a real file, xDelete() becomes a noop and xAccess(SQLITE_ACCESS_EXISTS)
** always returns 0 (file does not exist).
**
** This function relies on the fact that each time the upper level pager
** opens, deletes, or calls xAccess() on a journal file, it passes the
** Pager.zJournal pointer to the VFS function. So, to determine if this
** is such a call, check if the zName pointer passed to this function is 
** the same as any of the zipvfs_file.zJournal pointers in the list of
** MAIN_DB file-handles headed at ZipvfsVfs.pFirst.
*/
static int zipvfsIsJournalFile(sqlite3_vfs *pVfs, const char *zName){
  ZipvfsVfs *pZipVfs = (ZipvfsVfs *)pVfs;
  int nName;                      /* Length of zName in bytes */
  zipvfs_file *pIter;             /* Iterate through open MAIN_DB files */
  int ret = 0;                    /* Return value */

  nName = sqlite3Strlen30(zName);
  if( nName>3 && memcmp("al", &zName[nName-2], 2)==0 ){
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
    for(pIter=pZipVfs->pFirst; !ret && pIter; pIter=pIter->pNext){
      if( pIter->pPager ){
        if( pIter->zJournal==zName || pIter->zWal==zName ) ret = 1;
      }
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
  }

  return ret;
}

/*
** If argument rc is SQLITE_NOMEM, return SQLITE_IOERR_NOMEM. Otherwise 
** return a copy of rc. The idea is that IO methods like zipvfsRead()
** exit as follows:
**
**   return zipvfsError(rc);
*/
static int zipvfsError(int rc){
  return (rc==SQLITE_NOMEM ? SQLITE_IOERR_NOMEM : rc);
}

/*
** Instead of using SQLITE_CORRUPT or SQLITE_CORRUPT_BKPT, code in this file
** should use the constant ZIPVFS_CORRUPT. This ensures that whenever 
** corruption occurs, a message is sent to the log.
*/
static int zipvfsCorrupt(lineno){
  sqlite3_log(SQLITE_CORRUPT, 
      "zipvfs database is corrupt. Line %d of [%.10s]",
      lineno, 20+sqlite3_sourceid()
  );
  return SQLITE_IOERR;
}
#define ZIPVFS_CORRUPT zipvfsCorrupt(__LINE__)

/*
** This function is called when exiting the following methods:
**
**   xWrite
**   xSync 
**   xTruncate
**
** The second parameter is the error code that the method will return. If 
** the file handle is a database (not a journal or some other temporary
** file) and the error code is not SQLITE_OK, then the file handle
** enters the "error state". Once the file handle is in the error state
** the only way out is to completely unlock the file. While the file is
** in the error state the following methods always fail:
**
**   xWrite
**   xSync
**   xTruncate
**   xRead
**
** This is because after an IO or malloc error is returned to SQLite, it
** will try to roll back the transaction based on the contents of the
** journal file. This module will interpret the writes as page updates and
** attempt to write them into the database file, just as it would if the
** writes were part of a transaction. This is normally fine (if a bit odd).
** However, if an error has occured, then parts of the file may be corrupt,
** and the in-memory cache of the file header (pZip->hdr) may be invalid.
** Recovery must be performed by rolling back the journal file associated
** with pZip->pPager, not the users journal file.
*/
static int zipvfsWriteError(zipvfs_file *pZip, int rc){
  assert( rc!=SQLITE_BUSY );
  rc = zipvfsError(rc);
  if( pZip->pPager && rc!=SQLITE_OK ){
    pZip->eErrorCode = SQLITE_IOERR;
  }
  return rc;
}

/*
** Close a zipvfs-file.
*/
static int zipvfsClose(sqlite3_file *pFile){
  zipvfs_file *pZip = (zipvfs_file *)pFile;
  zipvfsRemoveFileFromList(pZip);
  if( pZip->pPager ){
#ifdef SQLITE_TEST
    if( pZip->xWrite.xDestruct ) pZip->xWrite.xDestruct(pZip->xWrite.pCtx);
#endif
    if( pZip->methods.xCompressClose ){
      pZip->methods.xCompressClose(pZip->methods.pCtx);
    }
    sqlite3_free(pZip->aCompressionBuffer);
    sqlite3PagerClose(pZip->pPager);
  }else{
    sqlite3_file *pSys = SYSFILE(pFile);
    if( pSys->pMethods ) pSys->pMethods->xClose(pSys);
  }
  pZip->base.pMethods = 0;
  return SQLITE_OK;
}

/*
** A wrapper around sqlite3_malloc(). The *pRc value is the status of the
** calling function.  If *pRc is initially anything other than SQLITE_OK,
** then the caller has already encountered one or more errors, so this routine
** should be a no-op and return NULL.  If *pRC is initially SQLITE_OK then
** attempt the memory allocation and set set *pRc to SQLITE_IOERR_NOMEM
** before returning if the memory allocation fails.
*/
static u8 *zipvfsMalloc(int nByte, int *pRc){
  u8 *z = 0;
  if( *pRc==SQLITE_OK ){
    z = sqlite3_malloc(nByte);
    if( !z ) *pRc = SQLITE_IOERR_NOMEM;
  }
  return z;
}

/*
** Free an allocation made using zipvfsMalloc().
*/
static void zipvfsFree(u8 *z){
  sqlite3_free(z);
}

/*
** When a page is loaded from a zipvfs database file (via a call made by
** the upper layer to the zipvfsRead() function), the compressed version is
** read from the file, then decompressed into the user-supplied buffer using 
** the xUncompress callback. The first time this function is called it 
** allocates the buffer into which the compressed version of the page is read. 
** The buffer is retained and returned by subsequent calls.
**
** The same buffer is used when compressing (storing) pages.
**
** If an out-of-memory condition is encountered, *pRc is set to 
** SQLITE_IOERR_NOMEM and NULL is returned.
*/
static u8* zipvfsCompressionBuffer(
  zipvfs_file *pZip,              /* Zip file handle */
  int *pRc                        /* IN/OUT: Error code */
){
  if( pZip->aCompressionBuffer==0 ){
    pZip->nCompressionBuffer =
            pZip->methods.xCompressBound(pZip->methods.pCtx, pZip->hdr.pgsz);
    pZip->aCompressionBuffer = zipvfsMalloc(pZip->nCompressionBuffer, pRc);
  }

  assert( *pRc!=SQLITE_OK
       || pZip->nCompressionBuffer==
           pZip->methods.xCompressBound(pZip->methods.pCtx, pZip->hdr.pgsz)
  );
  assert( *pRc!=SQLITE_OK || pZip->aCompressionBuffer );
  return pZip->aCompressionBuffer;
}

/*
** This function is a wrapper around sqlite3PagerAcquire(). It remaps
** the page-space so that it is contiguous - all pages including the
** pending-byte page may be used.
**
** This wrapper must be used by all code in this file apart from the
** call to load page 1 in zipvfsLock().
*/
static int zipvfsPagerAcquire(zipvfs_file *pZip, u32 iPg, DbPage **ppPg){ 
  assert( pZip->nPgsz );
  if( iPg>=1+(PENDING_BYTE/pZip->nPgsz) ){
    iPg += (pZip->hdr.iVersion ? (ZIPVFS_LOCKING_PAGE_SIZE / pZip->nPgsz) : 1);
  }
  return sqlite3PagerAcquire(pZip->pPager, iPg, ppPg, 0);
}

/*
** This function is a wrapper around sqlite3PagerTruncateImage(). It 
** does the same remapping of the page-space as zipvfsPagerAcquire().
**
** This wrapper must be used by all code in this file.
*/
static void zipvfsPagerTruncateImage(zipvfs_file *pZip, u32 iPg){
  assert( pZip->nPgsz );
  if( iPg>=1+(PENDING_BYTE/pZip->nPgsz) ){
    iPg += (pZip->hdr.iVersion ? (ZIPVFS_LOCKING_PAGE_SIZE / pZip->nPgsz) : 1);
  }
  sqlite3PagerTruncateImage(pZip->pPager, iPg);
}

/*
** Load raw data from the zipvfs file. This file just loads bytes of data
** from the disk, it does not do any decompression or interpretation of
** the data read.
*/
static void zipvfsLoadData(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOffset,                    /* Offset of data to load from file */
  int nByte,                      /* Number of bytes to load from file */
  u8 *aBuf,                       /* OUT: Write data here */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int rc = SQLITE_OK;           /* Value to set *pRc to before returning */
    if( pZip->inWriteTrans>1 || pZip->hdr.iVersion==2 ){
      int nRem = nByte;           /* Number of bytes still to load */
      while( nRem>0 ){
        u32 iPg;                  /* Page number of page file to load */
        DbPage *pPg;              /* Pager page to copy data from */
        u8 *zPgData;              /* Pointer to data of page pPg */
        int nCopy;                /* Number of bytes to copy from pPg */
        int iPgData;              /* Offset in zPgData to copy from */
  
        iPg = ((iOffset+nByte-nRem) / pZip->nPgsz) + 1;
        rc = zipvfsPagerAcquire(pZip, iPg, &pPg);
        if( rc!=SQLITE_OK ) break;
        zPgData = (u8 *)sqlite3PagerGetData(pPg);
  
        iPgData = ((nRem==nByte) ? (iOffset%pZip->nPgsz) : 0);
        nCopy = pZip->nPgsz - iPgData;
        if( nCopy>nRem ) nCopy = nRem;
  
        memcpy(&aBuf[nByte-nRem], &zPgData[iPgData], nCopy);
        sqlite3PagerUnref(pPg);
        nRem = nRem - nCopy;
      }
    }else{
      /* Read data directly from the file. This prevents the pager from
      ** caching the compressed data. Data will be cached in uncompressed
      ** form by the upper layer. 
      **
      ** The trick here is that the data must be read from the remapped
      ** file address space. See zipvfsPagerAcquire() for more details.
      */
      sqlite3_file *fd = sqlite3PagerFile(pZip->pPager);

      if( iOffset<PENDING_BYTE ){
        int amt = MIN(nByte, PENDING_BYTE-iOffset);
        rc = fd->pMethods->xRead(fd, aBuf, amt, iOffset);
      }
      if( rc==SQLITE_OK && (iOffset+nByte)>PENDING_BYTE ){
        int nDone;                /* Bytes already read */
        i64 iOff;                 /* File offset to read from */
        if( iOffset<PENDING_BYTE ){
          nDone = PENDING_BYTE-iOffset;
          iOff = PENDING_BYTE;
        }else{
          nDone = 0;
          iOff = iOffset;
        }
        iOff += (pZip->hdr.iVersion ? ZIPVFS_LOCKING_PAGE_SIZE : pZip->nPgsz);

        rc = fd->pMethods->xRead(fd, &aBuf[nDone], nByte-nDone, iOff);
      }
    }
    *pRc = rc;
  }
}


/*
** This function is used to uncompress a buffer of compressed data, then copy
** a subset of the uncompressed data into an output buffer.
**
** The compressed (input) data is stored in buffer aCompressed, size 
** nCompressed bytes. Once this has been uncompressed, nBuf bytes of the
** uncompressed data, starting at offset iPgOff, are copied into the output
** buffer aBuf. 
**
** Usually, but not always, iPgOff will be zero and nBuf will be
** the page-size of the user database. Unless the file is corrupt, the total
** size of the uncompressed data is the same as the page-size of the user
** database.
*/
static void zipvfsDecompress(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  u8 *aBuf,                       /* Output buffer. */
  int nBuf,                       /* Size of output buffer */
  int iPgOff,                     /* Offset in uncompressed page to read from */
  u8 *aCompressed,                /* Input buffer containing compressed data */
  int nCompressed,                /* Size of buffer aCompressed */
  int *pRc                        /* IN/OUT: Error code */
){
  assert( iPgOff==0 || nBuf<pZip->hdr.pgsz );
  if( *pRc==SQLITE_OK ){
    int rc;                       /* xUncompress return code */
    int nOut = pZip->hdr.pgsz;    /* Size of output buffer */
    char *aOut = (char *)aBuf;    /* Pointer to output buffer */

    if( nBuf<pZip->hdr.pgsz ){
      aOut = (char *)zipvfsMalloc(pZip->hdr.pgsz, pRc);
      if( !aOut ) return;
    }

    rc = pZip->methods.xUncompress(pZip->methods.pCtx, 
                        aOut, &nOut,(char*)aCompressed,nCompressed);
    if( rc!=SQLITE_OK || nOut!=pZip->hdr.pgsz ){
      *pRc = ZIPVFS_CORRUPT;
    }

    if( aOut!=(char *)aBuf ){
      assert( nBuf<nOut );
      memcpy(aBuf, &aOut[iPgOff], nBuf);
      zipvfsFree((u8 *)aOut);
    }
  }
}

/*
** Load and uncompress a page record from the slot at offset iOff,
** compressed data size nByte. Parameter iOff is a slot offset, as stored
** in the page-map, so the nByte bytes of compressed data actually begins
** at file offset (iOff+4).
**
** Once the page record has been loaded and uncompressed, copy nOutput bytes
** of uncompressed data into the output buffer aOutput, starting at offset
** iPgOff of the uncompressed data. Usually, but not always, iPgOff will be 
** zero and nOutput will be the page-size of the user database.
*/
static void zipvfsLoadAndUncompress(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  u8 *aOutput,                    /* OUT: Write decompressed data here */
  int nOutput,                    /* Size of output buffer */
  int iPgOff,                     /* Offset in uncompressed page to read from */
  int nByte,                      /* Number of bytes of compressed data */
  i64 iOff,                       /* Offset to load compressed data from */
  int *pRc                        /* IN/OUT: Error code */
){
  u8 *z;                          /* Buffer to load compressed page in to */

  /* Allocate the decompression buffer */
  z = zipvfsCompressionBuffer(pZip, pRc);
    
  /* Load the compressed data into the decompression buffer */
  zipvfsLoadData(pZip, iOff+ZIPVFS_SLOT_HDRSIZE, nByte, z, pRc);
    
  /* Decompress data into the output buffer supplied by the caller */
  zipvfsDecompress(pZip, aOutput, nOutput, iPgOff, z, nByte, pRc);
}

/*
** Store raw data in the zipvfs file.
**
** This function does not compress or decompress any data. It simply writes
** nBuf bytes of data from buffer zBuf into the zipvfs database file,
** starting at offset iOffset.
*/
static void zipvfsStoreData(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOffset,                    /* Offset to write to */
  const u8 *zBuf,                 /* Data to write (or 0, to clear) */
  int nBuf,                       /* Number of bytes of data to write/clear */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int nRem = nBuf;              /* Number of bytes still to store */
    while( nRem>0 ){
      u32 iPg;                    /* Page number of page file to load */
      DbPage *pPg = 0;            /* Pager page to copy data to */
      u8 *zPgData;                /* Pointer to data of page pPg */
      int nCopy;                  /* Number of bytes to copy to pPg */
      int iPgData;                /* Offset in zPgData to copy to */

      iPg = ((iOffset+nBuf-nRem) / pZip->nPgsz) + 1;
      if( SQLITE_OK!=(*pRc = zipvfsPagerAcquire(pZip, iPg, &pPg))
       || SQLITE_OK!=(*pRc = sqlite3PagerWrite(pPg))
      ){
        sqlite3PagerUnref(pPg);
        return;
      }
      zPgData = (u8 *)sqlite3PagerGetData(pPg);

      iPgData = ((nRem==nBuf) ? (iOffset%pZip->nPgsz) : 0);
      nCopy = pZip->nPgsz - iPgData;
      if( nCopy>nRem ) nCopy = nRem;
      if( zBuf ){
        memcpy(&zPgData[iPgData], &zBuf[nBuf-nRem], nCopy);
      }else{
        memset(&zPgData[iPgData], 0, nCopy);
      }
      sqlite3PagerUnref(pPg);
      nRem = nRem - nCopy;
    }
  }
}

/*
** Write 16-bit, 32-bit, 40-bit, or 56-bit unsigned integers into
** a memory buffer in a big-endian format.
*/
static void zipvfsPutU16(u8 *aBuf, u16 iVal){
  aBuf[0] = (iVal >> 8) & 0xFF;
  aBuf[1] = (iVal >> 0) & 0xFF;
}
static void zipvfsPutU32(u8 *aBuf, u32 iVal){
  aBuf[0] = (iVal >> 24) & 0xFF;
  aBuf[1] = (iVal >> 16) & 0xFF;
  aBuf[2] = (iVal >>  8) & 0xFF;
  aBuf[3] = (iVal >>  0) & 0xFF;
}
static void zipvfsPutU40(u8 *aBuf, i64 iVal){
  aBuf[0] = (iVal >> 32) & 0xFF;
  aBuf[1] = (iVal >> 24) & 0xFF;
  aBuf[2] = (iVal >> 16) & 0xFF;
  aBuf[3] = (iVal >>  8) & 0xFF;
  aBuf[4] = (iVal >>  0) & 0xFF;
}
static void zipvfsPutU64(u8 *aBuf, i64 iVal){
  aBuf[0] = (iVal >> 56) & 0xFF;
  aBuf[1] = (iVal >> 48) & 0xFF;
  aBuf[2] = (iVal >> 40) & 0xFF;
  aBuf[3] = (iVal >> 32) & 0xFF;
  aBuf[4] = (iVal >> 24) & 0xFF;
  aBuf[5] = (iVal >> 16) & 0xFF;
  aBuf[6] = (iVal >>  8) & 0xFF;
  aBuf[7] = (iVal >>  0) & 0xFF;
}

/*
** Read 16-bit, 32-bit, 40-bit, or 56-bit big-endian integers from
** a memory buffer and return their values.
*/
static u16 zipvfsGetU16(u8 *aBuf){
  return ((u16)aBuf[0] << 8)
       + (u16)aBuf[1];
}
static i64 zipvfsGetU32(u8 *aBuf){
  return (((i64)aBuf[0]) << 24)
       + (((i64)aBuf[1]) << 16)
       + (((i64)aBuf[2]) <<  8)
       + (((i64)aBuf[3]));
}
static i64 zipvfsGetU40(u8 *aBuf){
  return (((i64)aBuf[0]) << 32)
       + (((i64)aBuf[1]) << 24)
       + (((i64)aBuf[2]) << 16)
       + (((i64)aBuf[3]) <<  8)
       + (((i64)aBuf[4]));
}
static i64 zipvfsGetU64(u8 *aBuf){
  return (((i64)aBuf[0]) << 56)
       + (((i64)aBuf[1]) << 48)
       + (((i64)aBuf[2]) << 40)
       + (((i64)aBuf[3]) << 32)
       + (((i64)aBuf[4]) << 24)
       + (((i64)aBuf[5]) << 16)
       + (((i64)aBuf[6]) <<  8)
       + (((i64)aBuf[7]) <<  0);
}


/*
** Write a 5-byte big-endian integer to the file at offset iOffset.
*/
static void zipvfsWriteU40(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOffset,                    /* Offset in file to write */
  i64 iVal,                       /* Value to write to file */
  int *pRc                        /* IN/OUT: Error code */
){
  u8 a40[5];                      /* Serialization buffer */
  zipvfsPutU40(a40, iVal);
  zipvfsStoreData(pZip, iOffset, a40, 5, pRc);
}

/*
** Return the byte offset in the file of the page-map entry that corresponds
** to user database page iPg.
**
** The result of this function is only valid if (iPg>0).
*/
static i64 zipvfsPgmapEntry(u32 iPg){
  return (iPg-1) * 8 + ZIPVFS_FILEHEADER_SIZE + ZIPVFS_FILEHEADER_RESERVED;
}


/*
** Read a 6-byte slot-header from offset iOffset of the zipvfs file. A slot
** header consists of two integer fields:
**
**   31 bits: Page number of current content.
**   17 bits: Slot payload size.
*/
static void zipvfsReadSlotHdr(
  zipvfs_file *pZip,              /* Zipvfs file to read from */
  i64 iOffset,                    /* Offset to read slot header from */
  u32 *piPg,                      /* OUT: Page number from slot header */
  int *pnPayload,                 /* OUT: Page size from slot header */
  int *pRc                        /* IN/OUT: Error code */
){
  u8 aHdr[6] = {0,0,0,0,0,0};
  zipvfsLoadData(pZip, iOffset, 6, aHdr, pRc);
  
  if( *pRc==SQLITE_OK ){
    int nPayload;
    if( piPg ){
      *piPg = (((u32)aHdr[0] << 23) & 0x7F800000) +
              (((u32)aHdr[1] << 15) & 0x007F8000) +
              (((u32)aHdr[2] <<  7) & 0x00007F80) +
              (((u32)aHdr[3] >>  1) & 0x0000007F);
    }
  
    nPayload = (((int)aHdr[3] << 16) & 0x00010000) +
               (((int)aHdr[4] <<  8) & 0x0000FF00) +
               (((int)aHdr[5] <<  0) & 0x000000FF);
  
    if( nPayload<ZIPVFS_MIN_PAYLOAD ){
      *pRc = ZIPVFS_CORRUPT;
    }
    *pnPayload = nPayload;
  }
}

/*
** Find the location of the compressed blob for a page. This function does
** not actually load the compressed blob, it merely returns the size and 
** offset of the blob in the file. If the pnPadding argument is not NULL, it
** also reports on the number of bytes of free space (padding) following the
** blob.
*/
static void zipvfsFindPage(
  zipvfs_file *pZip,         /* The open database file */
  u32 iPage,                 /* The page sought */
  i64 *piOffset,             /* Write the byte offset to the record here */
  int *pnByte,               /* Write the size of the record here */
  int *pnPadding,            /* Write the number of extra padding bytes here */
  int *pRc                   /* Result code.  */
){
  if( *pRc==SQLITE_OK ){
    i64 iOff;                /* Offset of page-map entry for page iPage */
    u32 iMapPg;              /* Page of zipvfs file containing entry */
    DbPage *pPg;             /* Pointer to zipvfs file containing entry */
    u8 *aEntry;              /* Pointer to buffer containing entry */

    /* Figure out the offset of the page-map entry within the zipvfs file,
    ** and the page number of the zipvfs file page containing said entry.
    */
    iOff = zipvfsPgmapEntry(iPage);
    iMapPg = (iOff / pZip->nPgsz) + 1;

    /* Load the required zipvfs page into memory. Set aEntry to point at the
    ** eight-byte page-map entry.
    */
    *pRc = zipvfsPagerAcquire(pZip, iMapPg, &pPg);
    if( *pRc!=SQLITE_OK ) return;
    aEntry = sqlite3PagerGetData(pPg);
    aEntry = &aEntry[iOff - (iMapPg-1)*pZip->nPgsz];

    /* Read the offset, size and amount of padding for the page from the
    ** page-map entry. If the padding is reported as 255 bytes, then the
    ** actual amount of padding must be read from within the slot itself.
    */
    *piOffset = zipvfsGetU40(aEntry);
    *pnByte = ((int)aEntry[5]<<9)+((int)aEntry[6]<<1)+((aEntry[7]&0x80)?1:0);

    if( pnPadding ){
      if( (aEntry[7]&0x7F)==0x7F ){
        int nPayload;
        zipvfsReadSlotHdr(pZip, *piOffset, 0, &nPayload, pRc);
        *pnPadding = nPayload - *pnByte;
      }else{
        *pnPadding = (aEntry[7] & 0x7F);
      }
    }

    /* Release the reference to the zipvfs page acquired earlier. */
    sqlite3PagerUnref(pPg);
  }
}

/*
** Write a compressed page record into a specified offset of the zipvfs 
** file. Also write the associated page-map entry.
**
** The compressed record is passed via buffer zBuf, size nBuf bytes. The
** page-number associated with the compressed record is iPg. The record
** should be stored with nPadding bytes of padding.
**
** When this function is called, the file offset to write to is stored in
** *piOffset. Before returning, *piOffset is set to the offset of the first
** byte immediately following those written.
*/
static void zipvfsWritePageTo(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  u32 iPg,                        /* Page number being written */
  i64 *piOffset,                  /* IN/OUT: Offset to write page record to */
  u8 *zBuf, int nBuf,             /* Compressed page data */
  int nPadding,                   /* Bytes of padding following page data */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    u8 aEntry[8];                 /* Serialized page map entry */
    i64 iOffset = *piOffset;      /* Byte offset of new record */
    int nSlot = nBuf+nPadding;    /* Slot payload size. */

    assert( nBuf>0 && nPadding>=0 );
    assert( (nPadding+nBuf)<=ZIPVFS_MAX_PAYLOAD );
    assert( (nPadding+nBuf)>=ZIPVFS_MIN_PAYLOAD );

    /* Create a serialized page-map entry. The format is:
    **
    **     40 bits: slot offset.
    **     17 bits: record size.
    **     7 bits: padding size (0x7F means 127 bytes or more).
    */
    aEntry[0] = ((iOffset >> 32) & 0xFF);
    aEntry[1] = ((iOffset >> 24) & 0xFF);
    aEntry[2] = ((iOffset >> 16) & 0xFF);
    aEntry[3] = ((iOffset >>  8) & 0xFF);
    aEntry[4] = ((iOffset >>  0) & 0xFF);
    aEntry[5] = (nBuf >> 9) & 0xFF;
    aEntry[6] = (nBuf >> 1) & 0xFF;
    aEntry[7] = (nBuf << 7) & 0x80;
    if( nPadding<127 ){
      aEntry[7] |= (u8)nPadding;
    }else{
      aEntry[7] |= 0x7F;
    }

    /* Write the page-map entry to the file. */
    zipvfsStoreData(pZip, zipvfsPgmapEntry(iPg), aEntry, 8, pRc);

    /* Write the slot header: 
    **
    **     31 bits: Page number.
    **     17 bits: Slot size.
    */
    assert( PAGER_MAX_PGNO<=0x7FFFFFFF );
    assert( iPg<=0x7FFFFFFF );
    aEntry[0] = (iPg >> 23) & 0xFF;
    aEntry[1] = (iPg >> 15) & 0xFF;
    aEntry[2] = (iPg >>  7) & 0xFF;
    aEntry[3] = ((iPg <<  1) & 0xFE) | ((nSlot >> 16) & 0x01);
    aEntry[4] = ((nSlot >> 8) & 0xFF);
    aEntry[5] = ((nSlot >> 0) & 0xFF);

    zipvfsStoreData(pZip, iOffset, aEntry, ZIPVFS_SLOT_HDRSIZE, pRc);
    zipvfsStoreData(pZip, iOffset+ZIPVFS_SLOT_HDRSIZE, zBuf, nBuf, pRc);

    /* Update *piOffset before returning. */
    *piOffset = iOffset + ZIPVFS_SLOT_HDRSIZE + nBuf + nPadding;
  }
}

/*
** Write a compressed page into the file. Update the page-map accordingly.
**
** The compressed page data is stored in buffer zBuf, length nBuf bytes.
** The page number is as specified by parameter iPg. If the appendOnly
** parameter is non-zero, then the compressed page is always appended to
** the end of the data-area. Otherwise, it may be written either to the
** end of the data area or to an existing free-slot.
**
** The difference between this function and zipvfsWritePageTo() is that
** this function determines where in the file to write the page to whereas
** that information has to be supplied explicitly to zipvfsWritePageTo().
** If the page is written to a free-slot, then this function removes the
** associated entry from the free-slot btree before doing so. If there is
** an existing version of the page already in the database, then it is
** removed (and the empty space added to the free-list b-tree) by this
** function.
*/
static void zipvfsWritePage(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  int appendOnly,                 /* Non-zero to force page to end of file */
  u32 iPg,                        /* Page number being written */
  u8 *zBuf, int nBuf,             /* Compressed page data */
  int *pRc                        /* IN/OUT: Error code */
){
  i64 iSlotOff = 0;
  int nSlotSize = 0;

  /* Find any existing slot for this page and add it to the free-list. */
  if( pZip->hdr.iSize>(iPg-1)*pZip->hdr.pgsz ){
    i64 iOff = 0;                 /* Offset of existing page record */
    int nByte = 0;                /* Size in bytes of existing page record */
    int nPadding = 0;             /* Bytes of padding in exising page record */

    zipvfsFindPage(pZip, iPg, &iOff, &nByte, &nPadding, pRc);
    assert( *pRc || iOff || (iOff==0 && nByte==0 && nPadding==0) );
    zipvfsFreelistAdd(pZip, iOff, nByte + nPadding, pRc);
    pZip->hdr.nFreeFragment -= nPadding;
  }

  /* If the appendOnly flag is not set, try to find a suitable free-slot to
  ** reuse for the new record.  */
  if( !appendOnly ){
    int bExact = (pZip->hdr.nFreeSlot<pZip->nMaxFree);
    zipvfsFreelistBestfit(pZip, bExact, nBuf, &iSlotOff, &nSlotSize, pRc);
    assert( bExact==0 || *pRc || nSlotSize==nBuf || nSlotSize==0 );
  }

  /* If no existing free-slot could be found (or the appendOnly flag is set),
  ** append the slot to the end of the data-area.  */
  if( iSlotOff==0 ){
    iSlotOff = pZip->hdr.iDataEnd;
    nSlotSize = nBuf;
    if( nSlotSize<ZIPVFS_MIN_PAYLOAD ){
      nSlotSize = ZIPVFS_MIN_PAYLOAD;
    }
  }

#ifdef SQLITE_DEBUG
  if( !appendOnly ){
    ZIPVFSTRACE(("write page[%d] to (%lld,%d,%d)", iPg, iSlotOff, nBuf,
                 nSlotSize - nBuf));
  }
#endif

  /* Write the page to the location determined above. Update the end-of-data
  ** and fragmented-bytes header fields.  */
  zipvfsWritePageTo(pZip, iPg, &iSlotOff, zBuf, nBuf, nSlotSize-nBuf, pRc);
  if( iSlotOff>pZip->hdr.iDataEnd ){
    pZip->hdr.iDataEnd = iSlotOff;
  }
  pZip->hdr.nFreeFragment += nSlotSize-nBuf;
}

/*
** Compress a page of data.
**
** The input data is initially stored in buffer zInput. Write the compressed
** version to buffer zOut, and set *pnOut to the number of bytes written. If
** the compressed version is actually larger than the input (pZip->hdr.pgsz
** bytes), then instead store a copy of the input in buffer zOut and set
** *pnOut to pZip->hdr.pgsz.
*/
static void zipvfsCompress(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  u8 *zInput,                     /* Compress this data. ZipvfsHdr.pgsz bytes */
  u8 *zOut,                       /* Output buffer */
  int *pnOut,                     /* OUT: Output buffer size */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    pZip->methods.xCompress(pZip->methods.pCtx,
                 (char*)zOut, pnOut, (char*)zInput, pZip->hdr.pgsz);
    if( *pnOut>ZIPVFS_MAX_PAYLOAD ){
      *pRc = SQLITE_ERROR;
    }
  }
}

/*
** This function checks to see if the structure of the zipvfs database 
** file is internally consistent (similar to "PRAGMA integrity_check"
** for SQLite). If a problem is encountered, *pRc is set to SQLITE_CORRUPT
** before returning. No further explanation of the problem is provided.
**
** This function is a no-op unless the zipvfs_file.doIntegrityCheck flag
** is set. It is clear by default. This allows calls like the following:
**
**   zipvfsIntegrityCheck(pZip, &rc);
**
** to be added to the code before and after write operations in order to
** detect bugs causing corruption as early as possible. This is useful in
** debugging, but much too slow to turn on for general use. To set or clear
** the doIntegrityCheck variable, use the ZIPVFS_CTRL_DETECT_CORRUPTION 
** file-control method.
**
** The integrity-check may also be run on demand using the 
** ZIPVFS_CTRL_INTEGRITY_CHECK file-control method.
*/
static void zipvfsIntegrityCheck(zipvfs_file *pZip, int *pRc){
  if( pZip->doIntegrityCheck ){
    i64 iRec;                     /* Offset when iterating through data-area */
    u32 iPg;                      /* User database page number */
    i64 nFreeSlot = 0;            /* Number of free-slots in the file */
    i64 nFreeByte = 0;            /* Total size of free-slots */
    i64 nFreeFrag = 0;            /* Total size of padding on used slots */
    u8 *aBuf;                     /* Buffer to test uncompressing pages to */
    int pgsz = pZip->hdr.pgsz;    /* User database page size */

    zipvfsFreelistIntegrity(pZip, pRc); 
    aBuf = zipvfsMalloc(pgsz, pRc);

    iRec = pZip->hdr.iDataStart;
    while( *pRc==SQLITE_OK && iRec<pZip->hdr.iDataEnd ){
      int nByte;                  /* Size of compressed page in page-map */
      int nPayload;               /* Size of slot payload */
      int nPadding = 0;           /* Bytes of space following page image */
      i64 iOff;                   /* Offset of slot holding page iPg */

      if( iRec==pZip->hdr.iGapStart ){
        iRec = pZip->hdr.iGapEnd;
      }
        
      zipvfsReadSlotHdr(pZip, iRec, &iPg, &nPayload, pRc);
      zipvfsFindPage(pZip, iPg, &iOff, &nByte, &nPadding, pRc);

      if( iOff!=iRec ){
        /* A free slot. Make sure it is present in the free-list, and that 
        ** the payload-size stored in the free-list matches the payload-size 
        ** stored as part of the free-slot header.  */
        int ok;                   /* True if entry in free-list is correct */

        nFreeSlot++;
        nFreeByte += nPayload;
        ok = zipvfsFreelistTest(pZip, iRec, nPayload, 0, pRc);
        if( *pRc==SQLITE_OK && ok==0 ) *pRc = ZIPVFS_CORRUPT;
      }else{
        if( *pRc==SQLITE_OK && nByte+nPadding!=nPayload ){
          *pRc = ZIPVFS_CORRUPT;
        }
        nFreeFrag += nPadding;

        /* Check that the xUncompress() routine can uncompress this record. */
        if( *pRc==SQLITE_OK ){
          zipvfsLoadAndUncompress(pZip, aBuf, pgsz, 0, nByte, iRec, pRc);
        }
      }

      iRec += ZIPVFS_SLOT_HDRSIZE + nPayload;
    }

    /* Check that the number of free slots and bytes in the file-header agree
    ** with the figures calculated by traversing the entire file above.
    */
    if( *pRc==SQLITE_OK && nFreeSlot!=pZip->hdr.nFreeSlot ){
      *pRc = ZIPVFS_CORRUPT;
    }
    if( *pRc==SQLITE_OK && nFreeByte!=pZip->hdr.nFreeByte ){
      *pRc = ZIPVFS_CORRUPT;
    }
    if( *pRc==SQLITE_OK && nFreeFrag!=pZip->hdr.nFreeFragment ){
      *pRc = ZIPVFS_CORRUPT;
    }
  
    for(iPg=1; *pRc==SQLITE_OK && iPg<=(pZip->hdr.iSize/pgsz); iPg++){
      u32 iPg2;
      i64 iOff;                   /* Offset of page record */
      int n;                      /* Size of compressed page image */
      int nPayload;               /* Size of slot payload */
      int nPadding = 0;           /* Bytes of space following page image */
      if( iPg==1+(PENDING_BYTE/pgsz) ) continue;

      zipvfsFindPage(pZip, iPg, &iOff, &n, &nPadding, pRc);

      if( iOff!=0 ){
        zipvfsReadSlotHdr(pZip, iOff, &iPg2, &nPayload, pRc);
        if( *pRc==SQLITE_OK && iPg!=iPg2 ) *pRc = ZIPVFS_CORRUPT;
        if( *pRc==SQLITE_OK && n+nPadding!=nPayload ) *pRc = ZIPVFS_CORRUPT;
      }
    }

    /* Unless the database is zero bytes in size, check that the block
    ** of 52 bytes beginning at byte offset 40 of page 1 of the users
    ** database is replicated in the zipvfs database.
    */
    if( pZip->hdr.iSize ){
      i64 iOff;                   /* Offset of page record for page 1*/
      int nByte;                  /* Size of compressed page image */
      zipvfsFindPage(pZip, 1, &iOff, &nByte, 0, pRc);
      if( *pRc==SQLITE_OK && iOff ){
        u8 *aPage1 = sqlite3PagerGetData(pZip->pPage1);
        zipvfsLoadAndUncompress(pZip, aBuf, pgsz, 0, nByte, iOff, pRc);
        if( *pRc==SQLITE_OK && memcmp(&aPage1[40], &aBuf[40], 52) ){
          *pRc = ZIPVFS_CORRUPT;
        }
      }
    }

    zipvfsFree(aBuf);
  }
}

/*
** Read data from a zipvfs-file.
*/
static int zipvfsRead(
  sqlite3_file *pFile,            /* The file to read from */
  void *zBuf,                     /* Write uncompressed data here */
  int iAmt,                       /* Amount to read (uncompressed) */
  sqlite_int64 iOfst              /* Begin reading from here (uncompressed) */
){
  /* pZip->eErrorCode is never set here. If an error occurs while writing 
  ** to a zipvfs file, the upper layer always tries to roll back either a
  ** statement or main journal. This causes an error in either zipvfsWrite
  ** or zipvfsTruncate (since zipvfs_file.eErrorCode is already set),
  ** putting the upper layer pager into PAGER_ERROR state. Once in the
  ** error state, the pager does not read from the database file until
  ** it has been unlocked and relocked, by which time zipvfs_file.eErrorCode
  ** has been cleared. 
  **
  ** But if it were set, due to a change in SQLite or a bug in the reasoning
  ** above, initializing rc this way ensures that an error is returned to
  ** the user. See comments above zipvfsWriteError() for details.
  */
  zipvfs_file *pZip = (zipvfs_file *)pFile;
  int rc = pZip->eErrorCode;

  if( pZip->pPager ){

#if !defined(SQLITE_DIRECT_OVERFLOW_READ)
    /* SQLITE_DIRECT_OVERFLOW_READ can do a short read - a read of less
    ** than a complete sector.  So the follow assertions do not apply. */
    assert( ((iAmt & (iAmt-1))==0 && (iOfst & (iAmt-1))==0) /* page read */
         || (iAmt==16 && iOfst==24)                         /* change counter */
         || (iAmt==100 && iOfst==0)                         /* file-header */
    );
#endif
    assert( iAmt==100 || pZip->pPage1 );
    assert( pZip->pPage1==0 || pZip->hdr.iSize==0 || pZip->hdr.pgsz>0 );
    assert( pZip->hdr.pgsz>=512 || pZip->hdr.iSize==0 );

    if( iAmt==100 ){
      memset(zBuf, 0, iAmt);
    }else if( iOfst>=pZip->hdr.iSize ){
      /* Coverage: This case can only be hit during a VACUUM. */
      memset(zBuf, 0, iAmt);
      rc = SQLITE_IOERR_SHORT_READ;
    }else{
      u32 iPg;                    /* User database page number */
      i64 iOff = 0;               /* Offset of compressed record in file */
      int nByte;                  /* Size of compressed record in file */
      int iPgOff;                 /* Offset within page to copy from */

      iPg = (iOfst/pZip->hdr.pgsz)+1;
      iPgOff = iOfst % pZip->hdr.pgsz;
#if !defined(SQLITE_DIRECT_OVERFLOW_READ)
      assert( iPg==1 || (pZip->hdr.pgsz==iAmt && iPgOff==0) );
#endif
      
      /* Load and decompress the record for the requested page. */
      zipvfsIntegrityCheck(pZip, &rc);
      zipvfsFindPage(pZip, iPg, &iOff, &nByte, 0, &rc);
      if( iOff ){
        ZIPVFSTRACE(("read page[%d] from (%lld,%d)", iPg, iOfst, nByte));
        zipvfsLoadAndUncompress(pZip, zBuf, iAmt, iPgOff, nByte, iOff, &rc);
  
        /* If the amount of data requested is greater than the size of a page,
        ** then the zipvfsLoadAndUncompress() function populates the first
        ** pZip->hdr.pgsz bytes of buffer zBuf with the uncompressed contents
        ** of a single page. In this case zero the remainder of the buffer and
        ** return SQLITE_IOERR_SHORT_READ.
        */
        if( rc==SQLITE_OK && iAmt>pZip->hdr.pgsz ){
          assert( iPg==1 );
          assert( iPgOff==0 );
          memset(&(((u8*)zBuf)[pZip->hdr.pgsz]), 0, iAmt-pZip->hdr.pgsz);
          rc = SQLITE_IOERR_SHORT_READ;
        }
      }else{
        /* SQLite is trying to read a page that has never been written.
        ** This is not an error, just unusual. Zero the output buffer. */
        memset(zBuf, 0, iAmt);
      }
    }

  }else{
    const sqlite3_io_methods *pMethods = SYSFILE(pFile)->pMethods;
    assert( rc==SQLITE_OK );
    rc = pMethods->xRead(SYSFILE(pFile), zBuf, iAmt, iOfst);
  }

  return zipvfsError(rc);
}

/*
** Write data to a zipvfs-file.
*/
static int zipvfsWrite(
  sqlite3_file *pFile,            /* The file to write to */
  const void *zBuf,               /* Buffer of data to write */
  int iAmt,                       /* Number of bytes to write */
  sqlite_int64 iOfst              /* File offset to write to */
){
  zipvfs_file *pZip = (zipvfs_file *)pFile;
  int rc = SQLITE_OK;             /* Return code */

  if( pZip->eErrorCode ){
    rc = pZip->eErrorCode;
  }else if( pZip->pPager ){
    u8 *z;                        /* Compression buffer */
    int n;                        /* Size of compression buffer */
    u32 iPg;                      /* User db page number */

    /* The amount of data written is always the page size - a power of two 
    ** between 512 and 65536. Also, the offset is always to the beginning of 
    ** a page.  */
    assert( (iAmt&(iAmt-1))==0 && iAmt>=512 && iAmt<=65536 );
    assert( (iOfst&(iAmt-1))==0 );

    if( pZip->inWriteTrans<2 ) return SQLITE_IOERR;
    if( /* pZip->hdr.iSize>0 && */ iOfst==0 ){
      int nPgsz = (int)zipvfsGetU16(&((u8 *)zBuf)[16]);
      if( nPgsz==1 ) nPgsz = 65536;
      if( iAmt!=nPgsz ){
        return SQLITE_IOERR;
      }
    }
    pZip->hdr.pgsz = iAmt;
    iPg = (iOfst / pZip->hdr.pgsz) + 1;

    /* Detect the upper layer attempting to rollback an FCNTL_OVERWRITE
    ** transaction. See comments in zipvfsFileControl() for further details.
    */
    if( pZip->iOvwrNext ){
      if( (iPg==1 && pZip->bOvwrPage1==1)
       || (iPg!=1 && iPg!=pZip->iOvwrNext && iPg!=pZip->iOvwrNext+1) ){
        rc = SQLITE_IOERR;
      }else{
        if( iPg==1 ){
          pZip->bOvwrPage1 = 1;
        }else{
          pZip->iOvwrNext++;
        }
      }
    }

    zipvfsIntegrityCheck(pZip, &rc);

    /* Allocate the compression buffer, if it has not already been allocated. */
    z = zipvfsCompressionBuffer(pZip, &rc);
    n = pZip->nCompressionBuffer;

    /* Check if we need to shift one or more pages from the start of the data
    ** area to the end in order to expand the page-map.  */
    while( rc==SQLITE_OK && zipvfsPgmapEntry(iPg+1)>pZip->hdr.iDataStart ){
      int nByte = 0;              /* Number of bytes to extend page-map by */
   
      if( pZip->hdr.iDataStart==pZip->hdr.iGapStart ){
        nByte = pZip->hdr.iGapEnd - pZip->hdr.iGapStart;
        if( nByte>128 ){
          nByte = 128;
          pZip->hdr.iGapStart += nByte;
        }else{
          pZip->hdr.iGapStart = 0;
          pZip->hdr.iGapEnd = 0;
        }
      }else{
        /* Read the page number of the first record in the data area */
        u32 iFirstPg = 0;           /* Page number of first page in data-area */
        i64 iOff = 0;               /* Offset of page according to page-map */
        int nData = 0;              /* Size of page record to move */

        zipvfsReadSlotHdr(pZip, pZip->hdr.iDataStart, &iFirstPg, &nByte, &rc);
        zipvfsFindPage(pZip, iFirstPg, &iOff, &nData, 0, &rc);
        if( iOff==pZip->hdr.iDataStart ){
          ZIPVFSTRACE(("moving page[%d] from (%lld,%d,%d) to (%lld,%d,0)",
                        iFirstPg, iOff, nData, nByte-nData, 
                        pZip->hdr.iDataEnd, nData));

          /* Slot is not yet free. Free it. */
          /* Load the data for the first page into the compression buffer. */
          zipvfsLoadData(pZip, iOff+6, nData, z, &rc);
  
          /* Write the page record to the end of the file. */
          zipvfsWritePage(pZip, 1, iFirstPg, z, nData, &rc);
        }
        zipvfsFreelistRemove(pZip, pZip->hdr.iDataStart, nByte, &rc);
        nByte += 6;
      }

      zipvfsStoreData(pZip, pZip->hdr.iDataStart, 0, nByte, &rc);
      pZip->hdr.iDataStart += nByte;
      zipvfsIntegrityCheck(pZip, &rc);
    }

    /* If writing page 1, copy 16 through 91 from the user database page 1
    ** into the start of the zipvfs db file.  Also store the database file
    ** format name as a zero-padded string in bytes 0 through 15. */
    if( iPg==1 ){
      u8 aPg1[92];
      memset(aPg1, 0, 16);
      sqlite3_snprintf(17, (char*)aPg1, "ZV-%s", pZip->methods.zHdr);
      memcpy(&aPg1[16], &((u8*)zBuf)[16], 76);
      zipvfsStoreData(pZip, 0, aPg1, 92, &rc);
    }

    /* Compress the new page and write it into the file */
    zipvfsCompress(pZip, (u8 *)zBuf, z, &n, &rc);
    zipvfsWritePage(pZip, 0, iPg, z, n, &rc);
    zipvfsIntegrityCheck(pZip, &rc);

#ifdef SQLITE_TEST
    if( rc==SQLITE_OK && pZip->xWrite.x ){
      pZip->xWrite.x(pZip->xWrite.pCtx, iPg, n);
    }
#endif

    if( iOfst+iAmt>pZip->hdr.iSize ){
      pZip->hdr.iSize = iOfst+iAmt;
    }
  }else{
    rc = SYSFILE(pFile)->pMethods->xWrite(SYSFILE(pFile), zBuf, iAmt, iOfst);
  }

  return zipvfsWriteError(pZip, rc);
}

/*
** Truncate a zipvfs-file.
*/
static int zipvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  zipvfs_file *pZip = (zipvfs_file *)pFile;
  int rc = SQLITE_OK;             /* Return code */

  if( pZip->eErrorCode ){
    rc = pZip->eErrorCode;
  }else if( pZip->pPager ){
    assert( size<pZip->hdr.iSize );
    if( size%pZip->hdr.pgsz ){
      rc = SQLITE_IOERR_TRUNCATE;
    }else{
      u32 iPg;
      u32 iFree = (size/pZip->hdr.pgsz)+1;
      u32 nPage = (pZip->hdr.iSize / pZip->hdr.pgsz);
      for(iPg=iFree; iPg<=nPage; iPg++){
        i64 iOff;
        int nByte;
        int nPadding;
        zipvfsFindPage(pZip, iPg, &iOff, &nByte, &nPadding, &rc);
        zipvfsFreelistAdd(pZip, iOff, nByte+nPadding, &rc);
      }
      zipvfsStoreData(pZip, zipvfsPgmapEntry(iFree), 0, 
          zipvfsPgmapEntry(nPage+1)-zipvfsPgmapEntry(iFree), &rc
      );
      pZip->hdr.iSize = size;
    }
  }else{
    rc = SYSFILE(pFile)->pMethods->xTruncate(SYSFILE(pFile), size);
  }
  return zipvfsWriteError(pZip, rc);
}

/*
** This function is called after the database file is locked. It sets
** member variable pZip->nPgsz to the page-size of the underlying pager-file
** used to store the zipvfs database.
**
** If zero is returned, then the underlying pager is using the page size
** pZip->nPgsz. In this case the read transaction can proceed. If non-zero
** is returned, then the pager should be unlocked, sqlite3PagerSetPagesize()
** used to set the page-size to pZip->nPgsz, and then relocked. This function
** should then be called a second time to check that the page-size really
** has been changed.
**
** It is not possible to simply change the page-size and continue, as 
** page 1 is currently in memory.
*/
static int zipvfsGetPgsz(zipvfs_file *pZip, int *pRc){
  u32 pgsz = 0;

  if( *pRc==SQLITE_OK ){
    int nFramesize;               /* Page size in WAL file (or zero if n/a) */

    assert( sqlite3PagerRefcount(pZip->pPager)==1 );
    nFramesize = sqlite3PagerWalFramesize(pZip->pPager);
    (void)sqlite3PagerSetPagesize(pZip->pPager, &pgsz, -1);

    if( nFramesize ){
      /* There exists a WAL file that already contains one or more frames.
      ** The page size must be set to the same size as the frames already
      ** in the WAL in this case.
      */
      pZip->nPgsz = nFramesize;
    }else if( pZip->nBlocksize && pZip->hdr.iVersion>0 ){
      /* A URI parameter specifies the page-size to use.  Use it only
      ** if the file format version is greater than 0 */
      pZip->nPgsz = pZip->nBlocksize;
    }else{
      /* If this is a version 0 database or if no block_size query
      ** parameter is provided, then the page-size is determined by
      ** the preferences of the pager.
      */
      pZip->nPgsz = pgsz;
    }
  }

  return( pZip->nPgsz!=pgsz );
}

/*
** Read the zipvfs file header from disk and populate pZip->hdr with
** the values read. This function is a no-op if *pRc is not SQLITE_OK
** when it is called. It sets *pRc to an error code before returning if
** an error occurs.
**
** Normally, this function returns 0. Except, if this is the first time
** the header has been read since the file was opened and the header
** appears to indicate that this is an uncompressed SQLite database, and
** not a Zipvfs db, then return 1.
*/
static int zipvfsReadHeader(zipvfs_file *pZip, int *pRc){
  if( *pRc==SQLITE_OK ){
    u8 *aHdr;

    assert( pZip->pPage1 );
    aHdr = sqlite3PagerGetData(pZip->pPage1);

    if( pZip->bGetMethodsDone==0 ){
      char zSpace[ZIPVFS_MAX_NAME+1];
      const char *zHdr = 0;
      ZipvfsVfs *pVfs = pZip->pZipVfs;

      if( memcmp(aHdr, "SQLite format 3", 16)==0 ){
        pZip->bGetMethodsDone = 1;
        return 1;
      }else{
        int rc;
        if( memcmp(aHdr, "ZV-", 3)==0 ){
          memcpy(zSpace, &aHdr[3], ZIPVFS_MAX_NAME);
          zSpace[ZIPVFS_MAX_NAME] = '\0';
          zHdr = zSpace;
        }
        rc = pVfs->xGetMethods(
            pVfs->pGetMethodsCtx, pZip->zFilename, zHdr, &pZip->methods
        );
        if( rc!=SQLITE_OK ){
          *pRc = rc;
          return 0;
        }
        if( pZip->methods.xCompress==0 ){
          return 1;
        }
      }
      pZip->bGetMethodsDone = 1;
    }

    aHdr += ZIPVFS_FILEHEADER_RESERVED;
    pZip->hdr.iFreeSlot = zipvfsGetU64(&aHdr[0]);
    pZip->hdr.iDataStart = zipvfsGetU64(&aHdr[8]);
    pZip->hdr.iDataEnd = zipvfsGetU64(&aHdr[16]);
    pZip->hdr.iGapStart = zipvfsGetU64(&aHdr[24]);
    pZip->hdr.iGapEnd = zipvfsGetU64(&aHdr[32]);
    pZip->hdr.iSize = zipvfsGetU64(&aHdr[40]);
    pZip->hdr.nFreeSlot = zipvfsGetU64(&aHdr[48]);
    pZip->hdr.nFreeByte = zipvfsGetU64(&aHdr[56]);
    pZip->hdr.nFreeFragment = zipvfsGetU64(&aHdr[64]);
    pZip->hdr.pgsz = (int)zipvfsGetU32(&aHdr[72]);
    pZip->hdr.iVersion = (int)zipvfsGetU32(&aHdr[76]);

    if( pZip->hdr.iDataEnd==0 ){
      pZip->hdr.iDataStart = ZIPVFS_FILEHEADER_RESERVED + 
          ZIPVFS_FILEHEADER_SIZE + ZIPVFS_PAGEMAP_INIT_SIZE;
      pZip->hdr.iDataEnd = pZip->hdr.iDataStart;
    }

    if( pZip->hdr.iSize>0 && (
          pZip->hdr.pgsz<512 
       || pZip->hdr.pgsz>65536 
       || pZip->hdr.pgsz&(pZip->hdr.pgsz-1)
    )){
      /* If the users database is not zero bytes in size, the page-size
      ** header field must be set to something sensible. Otherwise, the
      ** database is corrupt. 
      */
      *pRc = ZIPVFS_CORRUPT;
    }else if( pZip->hdr.iVersion>2 ){
      sqlite3_log(
          SQLITE_CANTOPEN, "cannot read zipvfs version: %d", pZip->hdr.iVersion
      );
      *pRc = SQLITE_CANTOPEN;
    }else if( pZip->hdr.iVersion==0 && pZip->hdr.iDataEnd<PENDING_BYTE 
#ifdef SQLITE_TEST
        && pZip->bCreateVersion0==0
#endif
    ){
      /* If the zipvfs file is version 0, but it is smaller than PENDING_BYTE
      ** bytes in size, automatically upgrade it to version 1. For small
      ** files, there is no difference in the two formats.  */
      pZip->hdr.iVersion = 1;
    }
  }

  return 0;
}

/*
** Update the zipvfs file header with the values now contained in pZip->hdr.
** This function just writes data to the underlying pager sub-system, it
** does not make sure that the new header actually makes it to disk.
*/
static void zipvfsWriteHeader(zipvfs_file *pZip, int *pRc){
  if( *pRc==SQLITE_OK ){
    assert( pZip->pPage1 );
    *pRc = sqlite3PagerWrite(pZip->pPage1);
    if( *pRc==SQLITE_OK ){
      u8 *aHdr = sqlite3PagerGetData(pZip->pPage1);
      aHdr += ZIPVFS_FILEHEADER_RESERVED;
      zipvfsPutU64(&aHdr[0], pZip->hdr.iFreeSlot);
      zipvfsPutU64(&aHdr[8], pZip->hdr.iDataStart);
      zipvfsPutU64(&aHdr[16], pZip->hdr.iDataEnd);
      zipvfsPutU64(&aHdr[24], pZip->hdr.iGapStart);
      zipvfsPutU64(&aHdr[32], pZip->hdr.iGapEnd);
      zipvfsPutU64(&aHdr[40], pZip->hdr.iSize);
      zipvfsPutU64(&aHdr[48], pZip->hdr.nFreeSlot);
      zipvfsPutU64(&aHdr[56], pZip->hdr.nFreeByte);
      zipvfsPutU64(&aHdr[64], pZip->hdr.nFreeFragment);
      zipvfsPutU32(&aHdr[72], (u32)pZip->hdr.pgsz);
      zipvfsPutU32(&aHdr[76], (u32)pZip->hdr.iVersion);
    }
  }
}

/*
** Update the contents of the header with the values currently stored
** in pZip->hdr and commit the current transaction open on the zipvfs 
** file to disk. 
*/
static void zipvfsCommitTransaction(zipvfs_file *pZip, int *pRc){
  if( *pRc==SQLITE_OK && pZip->iOvwrNext ){
    if( pZip->bOvwrPage1==0 ){
      *pRc = SQLITE_IOERR;
    }else{
      zipvfsPagerTruncateImage(pZip, (pZip->hdr.iDataEnd/pZip->nPgsz)+1);
    }
  }
  zipvfsWriteHeader(pZip, pRc);
  if( *pRc==SQLITE_OK ){
    Pager *p = pZip->pPager;
    int rc = sqlite3PagerCommitPhaseOne(p, 0, 0);
    if( rc==SQLITE_OK ){
      rc = sqlite3PagerCommitPhaseTwo(p);
      pZip->inWriteTrans = 0;
      pZip->iOvwrNext = 0;
      pZip->bOvwrPage1 = 0;
    }
    *pRc = rc;
  }
}

/*
** Sync a zipvfs-file. The zipvfsDoSync() function is invoked when SQLite
** invokes the SQLITE_FCNTL_SYNC_OMITTED file-control.  SQLite issues that
** file-control in rollback mode instead of syncing the database file
** as part of transaction commit when PRAGMA synchronous=OFF.
**
** The zipvfsSync() function is the traditional xSync method that is
** invoked when PRAGMA synchronous=NORMAL or =FULL.
*/
static int zipvfsDoSync(zipvfs_file *pZip){
  int rc = pZip->eErrorCode;
  zipvfsCommitTransaction(pZip, &rc);
  return zipvfsWriteError(pZip, rc);
}
static int zipvfsSync(sqlite3_file *pFile, int flags){
  zipvfs_file *pZip = (zipvfs_file*)pFile;
  int rc;
  if( pZip->pPager ){
    rc = pZip->eErrorCode;
    if( rc==SQLITE_OK && pZip->inWriteTrans<2 ){
      /* This may occur if the zipvfs file is written to as part of a 
      ** multi-file transaction. If this file is successfully synced
      ** to disk, and then some other error occurs, the upper layer may
      ** try to roll back the transaction by writing the contents of its
      ** journal to the database file. Return an error in this case.
      ** The zipvfsWrite() function contains the same logic.
      **
      ** This condition only occur if the upper layer has already encountered
      ** some other error. For this reason, the SQLite user will never see
      ** this error code. So it doesn't matter whether it is strictly
      ** accurate or not, so long as it is an error.
      */
      rc = SQLITE_IOERR;
    }else{
      zipvfsCommitTransaction(pZip, &rc);
      rc = zipvfsWriteError(pZip, rc);
    }
  }else{
    rc = SYSFILE(pFile)->pMethods->xSync(SYSFILE(pFile), flags);
  }
  return rc;
}

/*
** Return the current file-size of a zipvfs-file. If the file is a main
** database file, the size returned is the size of the users uncompressed 
** database file.
*/
static int zipvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  int rc;
  zipvfs_file *pZip = (zipvfs_file *)pFile;
  if( pZip->pPager ){
    assert( pZip->pPage1 );
    *pSize = pZip->hdr.iSize;
    rc = SQLITE_OK;
  }else{
    rc = SYSFILE(pFile)->pMethods->xFileSize(SYSFILE(pFile), pSize);
  }
  return rc;
}

/*
** Make sure that there is at least a SHARED, RESERVED or EXCLUSIVE lock 
** (depending on parameter eLock) on the database file. If this function
** is called to take a SHARED lock, and the iVersion field in the zipvfs
** file header is "2", and bAllowWal is true, the pager opens the file in
** WAL mode. If bAllowWal is false, the file is always opened in rollback
** mode.
**
** SQLITE_OK is returned if the lock is successfully obtained, or an
** SQLite error code otherwise.
*/
static int zipvfsLockFile(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  int eLock,                      /* SHARED, RESERVED or EXCLUSIVE */
  int bAllowWal                   /* True to open WAL file if iVersion==2 */
){
  int rc = SQLITE_OK;

  assert( pZip->pPager );
  assert( eLock!=SQLITE_LOCK_SHARED || pZip->pPage1==0 );
  
  while( rc==SQLITE_OK && pZip->pPage1==0 ){
    assert( sqlite3PagerRefcount(pZip->pPager)==0 );
    rc = sqlite3PagerSharedLock(pZip->pPager);
    if( rc==SQLITE_OK ){
      rc = sqlite3PagerAcquire(pZip->pPager, 1, &pZip->pPage1, 0);
      if( zipvfsReadHeader(pZip, &rc) ){
        sqlite3_vfs *pVfs;      /* Underlying "real" VFS */
        sqlite3_file *pFd;      /* Real file handle to open */
        int dummy;              /* Used for xOpen() output flags */
        pFd = SYSFILE((sqlite3_file *)pZip);
        pVfs = SYSVFS((sqlite3_vfs *)pZip->pZipVfs);
        assert( rc==SQLITE_OK );
        assert( pZip->aCompressionBuffer==0 );
        sqlite3PagerClose(pZip->pPager);
        pZip->pPager = 0;
        rc = pVfs->xOpen(pVfs, pZip->zFilename, pFd, pZip->openFlags, &dummy);
        if( rc!=SQLITE_OK ) return rc;
        return zipvfsLock((sqlite3_file *)pZip, eLock);
      }

      if( rc==SQLITE_OK && pZip->hdr.iVersion==2 && bAllowWal ){
        int bOpen = 0;
        rc = sqlite3PagerOpenWal(pZip->pPager, &bOpen);
        if( rc==SQLITE_OK && bOpen==0 ){
          sqlite3PagerUnref(pZip->pPage1);
          pZip->pPage1 = 0;
        }
      }
      
      if( rc==SQLITE_OK && pZip->pPage1 && zipvfsGetPgsz(pZip, &rc) ){
        u32 pgsz = pZip->nPgsz;
        sqlite3PagerUnref(pZip->pPage1);
        rc = sqlite3PagerSetPagesize(pZip->pPager, &pgsz, -1);
        pZip->pPage1 = 0;
      }
    }
  }

  if( rc==SQLITE_OK && eLock>=SQLITE_LOCK_RESERVED && !pZip->inWriteTrans ){
    rc = sqlite3PagerBegin(pZip->pPager, 0, 0);
    if( rc==SQLITE_OK ) pZip->inWriteTrans = 1;
  }

  if( rc==SQLITE_OK && eLock==SQLITE_LOCK_EXCLUSIVE ){
    assert( pZip->inWriteTrans );
    rc = sqlite3PagerExclusiveLock(pZip->pPager);
    if( rc==SQLITE_OK ) pZip->inWriteTrans = 2;
  }

  return rc;
}

/*
** Lock a zipvfs-file. The following summarizes the actions taken when
** upgrading to each type of file-lock. The key here is that this function
** must grab the equivalent lock on the underlying pager file. Otherwise,
** other VFS methods might need to upgrade locks on the underlying pager,
** causing them to return SQLITE_BUSY. This kind of thing confuses the upper
** layer.
**
** SQLITE_LOCK_SHARED:
**     Open a read transaction on the underlying pager file.
**
** SQLITE_LOCK_RESERVED:
**     Open a write transaction on the underlying pager file.
**
** SQLITE_LOCK_EXCLUSIVE:
**     Open a write transaction on the underlying pager file if one is
**     not already open. Then use sqlite3PagerExclusiveLock() to take an
**     exclusive lock on the underlying file.
*/
static int zipvfsLock(sqlite3_file *pFile, int eLock){
  int rc = SQLITE_OK;
  zipvfs_file *pZip = (zipvfs_file *)pFile;

  assert( eLock==SQLITE_LOCK_SHARED 
       || eLock==SQLITE_LOCK_RESERVED
       || eLock==SQLITE_LOCK_EXCLUSIVE
  );
  if( pZip->pPager ){
    rc = zipvfsLockFile(pZip, eLock, 1);
  }else{
    rc = SYSFILE(pFile)->pMethods->xLock(SYSFILE(pFile), eLock);
  }

  return rc;
}

/*
** Unlock a zipvfs-file. If a write-transaction is open when this happens,
** attempt to roll it back.
*/
static int zipvfsUnlock(sqlite3_file *pFile, int eLock){
  zipvfs_file *pZip = (zipvfs_file *)pFile;

  if( pZip->pPager ){
    /* If eLock is SQLITE_LOCK_SHARED, call sqlite3PagerRollback() to release
    ** any write-locks and roll back any active write transaction. If eLock
    ** is SQLITE_LOCK_NONE, do that then release the reference to pZip->pPage1
    ** to drop the read-lock on the file.  
    */
    assert( eLock==SQLITE_LOCK_NONE || eLock==SQLITE_LOCK_SHARED );
    sqlite3PagerRollback(pZip->pPager);
    pZip->inWriteTrans = 0;
    pZip->iOvwrNext = 0;
    pZip->bOvwrPage1 = 0;
    if( eLock==SQLITE_LOCK_NONE ){
      sqlite3PagerUnref(pZip->pPage1);
      pZip->pPage1 = 0;
      pZip->eErrorCode = SQLITE_OK;

      /* Run auto-checkpoint, if required */
      if( pZip->nAutoCheckpoint>0
       && sqlite3PagerWalCallback(pZip->pPager)>=pZip->nAutoCheckpoint 
      ){
        sqlite3PagerCheckpoint(pZip->pPager, SQLITE_CHECKPOINT_PASSIVE, 0, 0);
      }
    }
  }else{
    sqlite3_file *pSys = SYSFILE(pFile);
    if( pSys->pMethods ) pSys->pMethods->xUnlock(pSys, eLock);
  }

  return SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a zipvfs-file.
**
** This method is never called. Usually SQLite calls xCheckReservedLock()
** after taking a shared-lock on the database file and detecting (via
** xAccess) a potential hot-journal in the file-system. Since this never
** happens using zipvfs, this method is never called.
*/
static int zipvfsCheckReservedLock(sqlite3_file *pFile, int *pRes){
  *pRes = 0;
  return SQLITE_OK;
}

/*
** A call to this function either partially or completely compacts the
** database file. Compaction is achieved by scanning the file from beginning
** to end, moving each page record as far as possible towards the start
** of the file to eliminate fragmentation and free-slots.
**
** If nMax is less than or equal to zero, then the entire file is compacted.
** In this case it is also truncated as appropriate before this function 
** returns.
**
** If nMax is greater than zero, then it is a rough limit to the number of
** bytes that will be read from the data area and rewritten in a more compact
** form. If this value is less than the total size of the data-area, then
** the file is left with a "gap region" in the middle of it. The next call
** to zipvfsCompactFile (by this or any other database client) begins 
** compacting the file at the end of the gap region. If the end of the file
** is reached, it is truncated before returning. Otherwise not.
*/
static int zipvfsCompactFile(zipvfs_file *pZip, i64 nMax){
  int rc;                         /* Return Code */
  if( pZip->pPage1 ){
    /* It is an error to try to compact the file while there is an open read
    ** or write transaction. Return MISUSE if this is attempted. */
    rc = SQLITE_MISUSE;
  }else{

    /* If this is the first operation on the file since it was opened, it
    ** may be that this is an ordinary SQLite database file, not a zipvfs
    ** file at all. If so, the code below will cause problems - starting
    ** with the call to zipvfsLock(SQLITE_LOCK_EXCLUSIVE), which the default
    ** SQLite VFS implementations consider a misuse (since you cannot jump 
    ** straight from no lock at all to an exclusive lock). So return early
    ** if this turns out to be the case.
    */
    assert( pZip->pPager );
    rc = zipvfsLock((sqlite3_file *)pZip, SQLITE_LOCK_SHARED);
    if( rc!=SQLITE_OK || pZip->pPager==0 ){
      zipvfsUnlock((sqlite3_file *)pZip, SQLITE_LOCK_NONE);
      return (rc ? rc : SQLITE_NOTFOUND);
    }

    /* Take a write lock on the database file. This call also reads the
    ** header of the zipvfs file. If there are no free-slots or fragments in
    ** the file, then there is no need to scan the file. In this case unlock
    ** the file and return. Otherwise, proceed with the compaction. */
    rc = zipvfsLock((sqlite3_file *)pZip, SQLITE_LOCK_EXCLUSIVE);
    if( pZip->hdr.nFreeByte>0 
     || pZip->hdr.nFreeFragment>0 
     || pZip->hdr.iGapStart 
    ){
      i64 iRead;                  /* Read offset */
      i64 iWrite;                 /* Write offset */
      i64 iEnd;                   /* Stop reading after this point */
      u8 *z;                      /* Buffer */

      /* Figure out where to start reading and writing. */
      if( pZip->hdr.iGapStart==0 ){
        iRead = iWrite = pZip->hdr.iDataStart;
      }else{
        iRead = pZip->hdr.iGapEnd;
        iWrite = pZip->hdr.iGapStart;
      }

      /* Figure out where to stop reading. */
      if( nMax<=0 || (iRead + nMax)>pZip->hdr.iDataEnd ){
        iEnd = pZip->hdr.iDataEnd;
      }else{
        iEnd = iRead + nMax;
      }

      /* Grab the compression buffer. This buffer is guaranteed to be large 
      ** enough to hold the largest possible compressed page image.  */
      z = zipvfsCompressionBuffer(pZip, &rc);

      while( rc==SQLITE_OK && iRead<iEnd ){
        u32 iPg;                  /* Page number of next page in file */
        i64 iOff;                 /* Record offset according to page-map */
        int nByte;                /* Number of bytes in compressed page image */
        int nPayload;             /* Slot payload size */
        
        zipvfsIntegrityCheck(pZip, &rc);
        zipvfsReadSlotHdr(pZip, iRead, &iPg, &nPayload, &rc);
        zipvfsFindPage(pZip, iPg, &iOff, &nByte, 0, &rc);

        if( iOff==iRead ){
          /* A page. Move it towards the start of the file. */
          int nPad = 0;
          if( nByte<ZIPVFS_MIN_PAYLOAD ){
            nPad = ZIPVFS_MIN_PAYLOAD - nByte;
          }
          zipvfsLoadData(pZip, iOff+ZIPVFS_SLOT_HDRSIZE, nByte, z, &rc);
          zipvfsWritePageTo(pZip, iPg, &iWrite, z, nByte, nPad, &rc);
          pZip->hdr.nFreeFragment -= (nPayload-nByte-nPad);
        }else{
          /* A free-slot. Remove it from the free-slot btree. */
          zipvfsFreelistRemove(pZip, iRead, nPayload, &rc);
        }
        iRead += (ZIPVFS_SLOT_HDRSIZE + nPayload);
        pZip->hdr.iGapStart = iWrite;
        pZip->hdr.iGapEnd = iRead;
      }
  
      if( rc==SQLITE_OK && iEnd==pZip->hdr.iDataEnd ){
        /* The file has been completely compacted. */
        pZip->hdr.iDataEnd = iWrite;
        pZip->hdr.iGapStart = 0;
        pZip->hdr.iGapEnd = 0;
        if( pZip->hdr.iFreeSlot || pZip->hdr.nFreeByte 
         || pZip->hdr.nFreeSlot || pZip->hdr.nFreeFragment<0
        ){
          rc = ZIPVFS_CORRUPT;
        }else{
          zipvfsPagerTruncateImage(pZip, (iWrite/pZip->nPgsz)+1);
        }
      }

      zipvfsCommitTransaction(pZip, &rc);
    }

    zipvfsIntegrityCheck(pZip, &rc);
    zipvfsUnlock((sqlite3_file *)pZip, SQLITE_LOCK_NONE);
  }

  return rc;
}

/*
** The following two functions are used by the xFileControl() method to lock
** and unlock the zipvfs file for file-controls that require a read-lock.
** The idiom is as follows:
**
**     int doUnlock = 0;
**     ...
**     if( lock-required ) zipvfsFCLock(pZip, &doUnlock);
**     ...
**     zipvfsFCUnlock(pZip, doUnlock);
**
** When called, zipvfsFCLock() checks if there is already a read transaction
** open on the file. If so, it sets doUnlock to 0 and returns without doing
** anything. Otherwise, if a lock is required, it takes the shared-lock and
** sets doUnlock to 1 before returning.
**
** zipvfsFCUnlock() unlocks the file if argument doUnlock is non-zero, or is
** a no-op if it is zero.
**
** This way, the file is unlocked only if it was locked by the current
** file-control, not if it was already locked or no such lock was required.
**
** If this is the first time the file has been locked since it was opened,
** it may be discovered that this is actually an ordinary, uncompressed,
** SQLite database file. In that case the lock is still obtained (and must
** be dropped by a call to zipvfsFCUnlock()), but the return value is
** SQLITE_NOTFOUND.
*/
static int zipvfsFCLock(zipvfs_file *pZip, int *pDoUnlock){
  int rc = SQLITE_OK;
  assert( pZip->pPager );
  if( pZip->pPage1 ){
    *pDoUnlock = 0;
  }else{
    *pDoUnlock = 1;
    rc = zipvfsLock((sqlite3_file *)pZip, SQLITE_LOCK_SHARED);
    if( rc==SQLITE_OK && pZip->pPager==0 ){
      rc = SQLITE_NOTFOUND;
    }
  }
  return rc;
}
static void zipvfsFCUnlock(zipvfs_file *pZip, int doUnlock){
  if( doUnlock ){
    zipvfsUnlock((sqlite3_file *)pZip, SQLITE_LOCK_NONE);
  }
}

/*
** The following two functions implement the ZIPVFS_CTRL_APPEND_FREESLOT and
** ZIPVFS_CTRL_REMOVE_FREESLOT test file-controls, respectively.
*/
#ifdef SQLITE_TEST
static int zipvfsTestAppendFreeSlot(
  zipvfs_file *pZip,              /* Zip file handle */
  int sz                          /* Size of free-slot to append to file */
){
  int rc;                         /* Return code */
  i64 iOff;                       /* Point to allocate new slot at */
  u8 aSlotHdr[6];

  assert( pZip->pPage1==0 );
  assert( sz>=ZIPVFS_MIN_PAYLOAD );
  assert( sz<=(1<<16) );

  rc = zipvfsLock((sqlite3_file *)pZip, SQLITE_LOCK_EXCLUSIVE);
  iOff = pZip->hdr.iDataEnd;
  
  /* Store a slot-header. The 'page-number' field is set to 0, but the
  ** slot-size is correct. */
  aSlotHdr[0] = 0x00;
  aSlotHdr[1] = 0x00;
  zipvfsPutU32(&aSlotHdr[2], sz);
  zipvfsStoreData(pZip, iOff, aSlotHdr, ZIPVFS_SLOT_HDRSIZE, &rc);

  pZip->hdr.iDataEnd = iOff + ZIPVFS_SLOT_HDRSIZE + sz;
  zipvfsStoreData(pZip, iOff+ZIPVFS_SLOT_HDRSIZE, 0, sz, &rc);

  zipvfsFreelistAdd(pZip, iOff, sz, &rc);
  zipvfsCommitTransaction(pZip, &rc);

  zipvfsUnlock((sqlite3_file *)pZip, SQLITE_LOCK_NONE);
  return rc;
}
static int zipvfsTestRemoveFreeSlot(
  zipvfs_file *pZip,              /* Zip file handle */
  int sz                          /* Size of free-slot to append to file */
){
  int rc;                         /* Return code */
  i64 iOff;                       /* Offset of slot removed from list */
  int nByte;                      /* Payload size of slot removed from list */
  assert( pZip->pPage1==0 );
  assert( sz>=ZIPVFS_MIN_PAYLOAD );
  assert( sz<=(1<<16) );

  rc = zipvfsLock((sqlite3_file *)pZip, SQLITE_LOCK_EXCLUSIVE);
  zipvfsFreelistBestfit(pZip, 1, sz, &iOff, &nByte, &rc);
  zipvfsCommitTransaction(pZip, &rc);

  zipvfsUnlock((sqlite3_file *)pZip, SQLITE_LOCK_NONE);
  return rc;
}
#endif

/*
** Change the journal mode used by pager pZip->pPager to access the 
** database file to eMode.
*/
static int zipvfsJournalMode(
  zipvfs_file *pZip,              /* Zip file handle */
  int eMode                       /* IN/OUT: Requested/actual journal mode */
){
  int rc = SQLITE_OK;             /* Return code */

  assert( eMode==PAGER_JOURNALMODE_DELETE
       || eMode==PAGER_JOURNALMODE_PERSIST
       || eMode==PAGER_JOURNALMODE_OFF
       || eMode==PAGER_JOURNALMODE_TRUNCATE
       || eMode==PAGER_JOURNALMODE_MEMORY
       || eMode==PAGER_JOURNALMODE_WAL
  );

  /* Any attempt to change the journal mode while a read or write 
  ** transaction is open is an error.  */
  if( pZip->pPage1 ) rc = SQLITE_ERROR;

  /* If the database has not yet been read, open it and read the header 
  ** now. This is so that a "PRAGMA zipvfs_journal_mode" statement that
  ** switches between rollback and WAL mode is not incorrectly assumed
  ** to be a no-op by the code below.  */
  if( pZip->hdr.pgsz==0 ){
    rc = zipvfsLockFile(pZip, SQLITE_LOCK_SHARED, 1);
    zipvfsUnlock((sqlite3_file *)pZip, SQLITE_LOCK_NONE);
  }

  if( rc==SQLITE_OK ){
    int bWal = (pZip->hdr.iVersion==2);

    /* When switching between rollback modes, all that is required is to
    ** call sqlite3PagerSetJournalMode(). Special handling is required to
    ** switch between rollback and WAL modes.  */
    if( (eMode==PAGER_JOURNALMODE_WAL) != bWal ){

      /* If switching out of WAL mode, checkpoint, close and delete the
      ** WAL file. Then change the journal mode (so that the transaction
      ** in the next block is written using a rollback transaction, not
      ** a new WAL file.  */
      if( bWal ){
        rc = sqlite3PagerCloseWal(pZip->pPager);
      }

      /* Make sure a rollback-mode write transaction is open */
      if( rc==SQLITE_OK ){
        rc = zipvfsLockFile(pZip, SQLITE_LOCK_EXCLUSIVE, 0);
      }

      /* Modify the iVersion field in the zipvfs header */
      if( rc==SQLITE_OK ){
        pZip->hdr.iVersion = (bWal ? 1 : 2);
        zipvfsCommitTransaction(pZip, &rc);
      }
      zipvfsUnlock((sqlite3_file *)pZip, SQLITE_LOCK_NONE);
    }

    /* If successful, update the pager journal mode. */
    if( rc==SQLITE_OK ) sqlite3PagerSetJournalMode(pZip->pPager, eMode);
  }

  return rc;
}


/*
** The argument points to a nul-terminated string. If the string is the
** name of a journal mode (e.g. "delete", "wal", "truncate" etc.), then
** return the associate PAGER_JOURNALMODE_XXX constant. Otherwise
** return -1.
*/
static int zipvfsStringToJournalMode(const char *z){
  static struct JournalMode {
    const char *zName;
    int eMode;
  } aMode[] = {
    {"delete",   PAGER_JOURNALMODE_DELETE }, 
    {"persist",  PAGER_JOURNALMODE_PERSIST }, 
    {"off",      PAGER_JOURNALMODE_OFF }, 
    {"truncate", PAGER_JOURNALMODE_TRUNCATE }, 
    {"memory",   PAGER_JOURNALMODE_MEMORY }, 
    {"wal",      PAGER_JOURNALMODE_WAL }, 
    {0,          -1 }
  };
  int i;

  for(i=0; aMode[i].zName && sqlite3StrICmp(z, aMode[i].zName); i++);
  return aMode[i].eMode;
}

/*
** This function handles SQLITE_FCNTL_PRAGMA file-controls.
*/
static int zipvfsHandlePragma(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  const char *zPragma,            /* Name of pragma */
  const char *zPragmaArg,         /* Pragma argument, or NULL */
  char **pzResult                 /* OUT: sqlite3_malloc'd result string */
){
  int rc = SQLITE_NOTFOUND;

  /*
  ** PRAGMA journal_mode = [delete|persist|truncate|memory|off|wal]
  **
  **   The purpose of this is to prevent the upper layer from switching
  **   into WAL mode. Unless zipvfs is in pass-through mode, the upper layer
  **   may not use journal_mode=WAL.
  */
  if( sqlite3_stricmp("journal_mode", zPragma)==0 ){
    if( zPragmaArg && 0==sqlite3_stricmp("wal", zPragmaArg) ){
      *pzResult = sqlite3_mprintf(
          "journal_mode=wal may not be used with zipvfs"
      );
      rc = SQLITE_ERROR;
    }

  /*
  ** PRAGMA locking_mode = [normal|exclusive]
  **
  **   Prevent attempts to set locking_mode=EXCLUSIVE.
  */
  }else if( sqlite3_stricmp("locking_mode", zPragma)==0 ){
    if( zPragmaArg && 0==sqlite3_stricmp("exclusive", zPragmaArg) ){
      *pzResult = sqlite3_mprintf(
          "locking_mode=exclusive may not be used with zipvfs"
      );
      rc = SQLITE_ERROR;
    }

  /*
  ** PRAGMA zipvfs_journal_mode = [delete|persist|truncate|memory|off|wal]
  */
  }else if( sqlite3_stricmp("zipvfs_journal_mode", zPragma)==0 ){
    rc = SQLITE_OK;

    if( zPragmaArg ){
      int eMode = zipvfsStringToJournalMode(zPragmaArg);
      if( eMode!=-1 ){
        rc = zipvfsJournalMode(pZip, eMode);
      }
    }

    if( rc==SQLITE_OK ){
      int eRet = sqlite3PagerGetJournalMode(pZip->pPager);
      *pzResult = sqlite3_mprintf("%s", sqlite3JournalModename(eRet));
      if( *pzResult==0 ) rc = SQLITE_NOMEM;
    }

  /*
  ** PRAGMA wal_autocheckpoint = N
  */
  }else if( sqlite3_stricmp("wal_autocheckpoint", zPragma)==0 ){
    rc = SQLITE_OK;
    if( zPragmaArg ){
      pZip->nAutoCheckpoint = sqlite3Atoi(zPragmaArg);
    }
    *pzResult = sqlite3_mprintf("%d", pZip->nAutoCheckpoint);

  /*
  ** PRAGMA wal_checkpoint = [passive|full|restart]
  */
  }else if( sqlite3_stricmp("wal_checkpoint", zPragma)==0 ){
    int eMode = SQLITE_CHECKPOINT_PASSIVE;
    if( zPragmaArg ){
      if( sqlite3_stricmp(zPragmaArg, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }else if( sqlite3StrICmp(zPragmaArg, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }
    }
    rc = sqlite3PagerCheckpoint(pZip->pPager, eMode, 0, 0);
    *pzResult = sqlite3_mprintf("%d", rc==SQLITE_BUSY);
    if( rc==SQLITE_BUSY ) rc = SQLITE_OK;
  }

  return rc;
}

/*
** File control method. For custom operations on a zipvfs-file.
*/
static int zipvfsFileControl(sqlite3_file *pFile, int op, void *pArg){
  int rc = SQLITE_OK;
  int doUnlock = 0;
  zipvfs_file *pZip = (zipvfs_file *)pFile;

  if( pZip->pPager ){
    switch( op ){
      case SQLITE_FCNTL_SYNC_OMITTED:
        rc = zipvfsDoSync(pZip);
        break;

      case SQLITE_FCNTL_OVERWRITE:
        if( pZip->hdr.pgsz ){
          int nExpectedPg;        /* Expected number of pages in file */
          int iVersion = pZip->hdr.iVersion;

          assert( pZip->pPage1 && pZip->inWriteTrans );
          nExpectedPg = MAX(
              (*((i64 *)pArg) / pZip->hdr.pgsz),
              (ZIPVFS_PAGEMAP_INIT_SIZE / 8)
          );
          memset(&pZip->hdr, 0, sizeof(ZipvfsHdr));
          pZip->hdr.iDataStart += ZIPVFS_FILEHEADER_RESERVED;
          pZip->hdr.iDataStart += ZIPVFS_FILEHEADER_SIZE;
          pZip->hdr.iDataStart += (nExpectedPg * 8);
          pZip->hdr.iDataEnd = pZip->hdr.iDataStart;
          pZip->hdr.iVersion = (iVersion ? iVersion : 1);
          zipvfsStoreData(pZip, 
              ZIPVFS_FILEHEADER_RESERVED+ZIPVFS_FILEHEADER_SIZE, 0,
              nExpectedPg*8, &rc
          );
          if( rc==SQLITE_OK ){
            /* Usually, when the upper layer tries to rollback a transaction,
            ** either explicitly or as a result of an error condition (say
            ** an OOM), the start of the abandoned user level transaction and
            ** the subsequent rollback appear to zipvfs as a single committed
            ** transaction. If the upper level has modified half the pages
            ** in the database before the rollback starts, it overwrites those
            ** same pages with the original data and assumes that the database
            ** is now in its original state. However, that won't work once
            ** SQLITE_FCNTL_OVERWRITE has been invoked, since once committed
            ** the database would consist only of those pages that had been
            ** modified and rolled back. It would be corrupted.
            **
            ** So, if such a rollback is attempted, we have to ensure that
            ** it is performed by rolling back the journal belonging to the
            ** lower level pager. To accomplish this, the zipvfs_file object
            ** enters the error state if it detects the upper layer attempting
            ** a rollback. Since FCNTL_OVERWRITE is only used during a
            ** VACUUM, a rollback is easy to detect: It is the only reason
            ** a single page would be written more than once. And since,
            ** except for page 1 and the PENDING_BYTE page, pages are always
            ** written in order, this can be easily detected within
            ** the zipvfsWrite().
            **
            ** Related to the above is that the zipvfsDeviceCharacteristics()
            ** method may not return a mask with the IOCAP_UNDELETABLE_WHEN_OPEN
            ** bit set. If it did, and a zipvfs_file object enters the error
            ** state for the reasons described above, then SQLite would not
            ** close the journal file-handle when unlocking the zipvfs_file
            ** (and causing it to exit the error state). The next time SQLite
            ** accessed the database, it would try again to rollback the 
            ** contents of the upper level journal, and this time it would
            ** succeed. Likely corrupting the database file. However, if
            ** IOCAP_UNDELETABLE_WHEN_OPEN is not set, then SQLite must close
            ** the journal file when unlocking the zipvfs_file.
            */
            pZip->iOvwrNext = 2;
            assert( pZip->bOvwrPage1==0 );
          }
          rc = zipvfsWriteError(pZip, rc);
        }
        break;

      case ZIPVFS_CTRL_COMPACT: {
        i64 iParam = 0;
        if( pArg ) iParam = *(i64 *)pArg;
        rc = zipvfsCompactFile(pZip, iParam);
        if( pArg ){ 
          i64 iRemain = 0;
          if( pZip->hdr.iGapStart ){
            iRemain = pZip->hdr.iDataEnd - pZip->hdr.iGapEnd;
          }
          *(i64 *)pArg = iRemain;
        }
        break;
      }

      case ZIPVFS_CTRL_OFFSET_AND_SIZE: {
        sqlite3_int64 *x = (sqlite3_int64*)pArg;
        int nByte;
        rc = zipvfsFCLock(pZip, &doUnlock);
        if( rc==SQLITE_OK ){
          zipvfsFindPage(pZip, (int)x[0], &x[0], &nByte, 0, &rc);
          x[1] = nByte;
        }
        break;
      }

      case ZIPVFS_CTRL_MAXFREE:
        pZip->nMaxFree = *(int *)pArg;
        break;

      case ZIPVFS_CTRL_MAXFRAG:
        pZip->nMaxFrag = *(int *)pArg;
        break;

      case ZIPVFS_CTRL_CACHESIZE:
        sqlite3PagerSetCachesize(pZip->pPager, *(int *)pArg);
        break;

      case ZIPVFS_CTRL_STAT: {
        int nSlot;                /* Number of slots in database file */
        ZipvfsStat *pStat = (ZipvfsStat *)pArg;
        rc = zipvfsFCLock(pZip, &doUnlock);

        if( pZip->hdr.pgsz ){
          nSlot = pZip->hdr.nFreeSlot + (pZip->hdr.iSize / pZip->hdr.pgsz);
          pStat->nFreeSlot = pZip->hdr.nFreeSlot;

          pStat->nFileByte = pZip->hdr.iDataEnd;
          pStat->nFreeByte = pZip->hdr.nFreeByte;
          pStat->nFragByte = pZip->hdr.nFreeFragment;
          pStat->nGapByte = pZip->hdr.iGapEnd - pZip->hdr.iGapStart;

          pStat->nContentByte = pStat->nFileByte - pZip->hdr.iDataStart;
          pStat->nContentByte -= pStat->nFreeByte;
          pStat->nContentByte -= pStat->nFragByte;
          pStat->nContentByte -= pStat->nGapByte;
          pStat->nContentByte -= ZIPVFS_SLOT_HDRSIZE * nSlot;
        }else{
          memset(pStat, 0, sizeof(ZipvfsStat));
        }

        break;
      }

      case ZIPVFS_CTRL_INTEGRITY_CHECK: {
        int doIntegrityCheck = pZip->doIntegrityCheck;
        rc = zipvfsFCLock(pZip, &doUnlock);
        pZip->doIntegrityCheck = 1;
        zipvfsIntegrityCheck(pZip, &rc);
        pZip->doIntegrityCheck = doIntegrityCheck;
        break;
      }

      case ZIPVFS_CTRL_LOCKING_MODE: {
        int iArg = *(int *)pArg;
        rc = zipvfsFCLock(pZip, &doUnlock);
        if( rc==SQLITE_OK ){
          if( iArg!=PAGER_LOCKINGMODE_NORMAL
              && iArg!=PAGER_LOCKINGMODE_EXCLUSIVE
            ){
            iArg = PAGER_LOCKINGMODE_QUERY;
          }
          *(int *)pArg = sqlite3PagerLockingMode(pZip->pPager, iArg);
        }
        break;
      }

      case SQLITE_FCNTL_PRAGMA: {
        char **azArg = (char **)pArg;

        if( pZip->bGetMethodsDone==0 ){
          rc = zipvfsLockFile(pZip, SQLITE_LOCK_SHARED, 1);
          if( rc!=SQLITE_OK ) break;
          assert( pZip->bGetMethodsDone || pZip->pPager==0 );
          zipvfsUnlock(pFile, SQLITE_LOCK_NONE);
          if( pZip->pPager==0 ){
            return zipvfsFileControl(pFile, op, pArg);
          }
        }

        rc = zipvfsHandlePragma(pZip, azArg[1], azArg[2], &azArg[0]);
        break;
      }

#ifdef SQLITE_TEST
      case ZIPVFS_CTRL_DETECT_CORRUPTION:
        pZip->doIntegrityCheck = (u8)*(int *)pArg;
        break;

      case ZIPVFS_CTRL_STRUCTURE: 
        rc = zipvfsFCLock(pZip, &doUnlock);
        if( rc==SQLITE_OK ){
          ZipvfsStructureCb *p = (ZipvfsStructureCb *)pArg;
          u32 iPg;

          /* Invoke the callback for each page in the database */
          for(iPg=1; iPg<(pZip->hdr.iSize/pZip->hdr.pgsz); iPg++){
            i64 iOffset = 0;
            int nByte;
            int nPadding;
            zipvfsFindPage(pZip, iPg, &iOffset, &nByte, &nPadding, &rc);
            if( iOffset ){
              assert( rc==SQLITE_OK );
              p->x(p, iPg, iOffset, nByte, nPadding, 0);
            }
          }

          /* Invoke the callback for each free-slot in the file */
          zipvfsFreelistIterate(pZip, p, &rc);
        }
        break;

      case ZIPVFS_CTRL_WRITE_HOOK:
        if( pZip->xWrite.xDestruct ) pZip->xWrite.xDestruct(pZip->xWrite.pCtx);
        pZip->xWrite.x = ((ZipvfsWriteCb *)pArg)->x;
        pZip->xWrite.pCtx = ((ZipvfsWriteCb *)pArg)->pCtx;
        pZip->xWrite.xDestruct = ((ZipvfsWriteCb *)pArg)->xDestruct;
        break;

      case ZIPVFS_CTRL_APPEND_FREESLOT: {
        int sz = *(int *)pArg;
        rc = zipvfsTestAppendFreeSlot(pZip, sz);
        break;
      }
      case ZIPVFS_CTRL_REMOVE_FREESLOT: {
        int sz = *(int *)pArg;
        rc = zipvfsTestRemoveFreeSlot(pZip, sz);
        break;
      }

      case ZIPVFS_CTRL_CREATE_VERSION_0: {
        int flag = *(int *)pArg;
        pZip->bCreateVersion0 = flag;
        break;
      }
#endif  /* ifdef SQLITE_TEST */

#ifdef SQLITE_DEBUG
      case ZIPVFS_CTRL_TRACE:
        zipvfsTraceFile = (FILE*)pArg;
        break;
#endif

      case 0xCA093FA0:   /* SQLITE_FCNTL_DB_UNCHANGED - internal testing only */
      case SQLITE_FCNTL_SIZE_HINT: {
        /* Don't let this filter down to the low-level VFS */
        break;
      }
      default: {
        /* Any other unrecognized file-control is passed through to the
        ** low-level VFS */
        sqlite3_file *pSysFile = sqlite3PagerFile(pZip->pPager);
        const sqlite3_io_methods *pMethods = pSysFile->pMethods;
        if( pMethods ){
          rc = pMethods->xFileControl(pSysFile, op, pArg);
          if( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
            *(char**)pArg = sqlite3_mprintf("zipvfs/%z", *(char**)pArg);
          }
        }
      }
    }
  }else{
    rc = SYSFILE(pFile)->pMethods->xFileControl(SYSFILE(pFile), op, pArg);
  }

  zipvfsFCUnlock(pZip, doUnlock);
  return (rc==SQLITE_IOERR_NOMEM ? SQLITE_NOMEM : rc);
}

/*
** Return the sector-size in bytes for a zipvfs-file.
*/
static int zipvfsSectorSize(sqlite3_file *pFile){
  zipvfs_file *pZip = (zipvfs_file*)pFile;
  sqlite3_file *pSys;
  if( pZip->pPager ){
    pSys = sqlite3PagerFile(pZip->pPager);
  }else{
    pSys = SYSFILE(pFile);
  }
  return pSys->pMethods->xSectorSize(pSys);
}

/*
** Return the device characteristic flags supported by a zipvfs-file.
*/
static int zipvfsDeviceCharacteristics(sqlite3_file *pFile){
  int mask;
  zipvfs_file *pZip = (zipvfs_file*)pFile;
  sqlite3_file *pSys;
  if( pZip->pPager ){
    pSys = sqlite3PagerFile(pZip->pPager);
  }else{
    pSys = SYSFILE(pFile);
  }
  mask = pSys->pMethods ? pSys->pMethods->xDeviceCharacteristics(pSys) : 0;

  /* Never return a mask with the IOCAP_UNDELETABLE_WHEN_OPEN bit set. Doing
  ** so causes problems in recovering from the error state 
  ** (zipvfs_file.eErrorCode!=SQLITE_OK). In the error state, it is not safe
  ** to keep writing to the database file for any reason, including rolling
  ** back the contents of the upper level pagers journal file. To recover
  ** from the error state, the lower level pager journal must be rolled
  ** back.
  **
  ** To guarantee this happens, Zipvfs returns an IOERR for all database file
  ** reads and writes after entering the error state. Eventually, the upper
  ** layer gives up and unlocks the zipvfs file (which causes it to exit
  ** the error state). The next time data is read from teh zipvfs file, a
  ** new read transaction is opened on the lower level pager and the lower
  ** level journal (if any) is rolled back. Successful recovery.
  **
  ** This works since, normally, the upper level must close its journal
  ** file handle when unlocking the zipvfs file. Since this journal file
  ** is a temporary, closing it destroys the contents. However, if 
  ** IOCAP_UNDELETABLE_WHEN_OPEN is set, then the journal file is not closed
  ** when the zipvfs file is unlocked, and may be rolled back later on,
  ** corrupting the database.
  */
  return mask & ~SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN;
}

/*
** Busy handler function registered with the pager module used to write
** to the database file. 
**
** Currently, this function fails immediately (causing the pager to return
** SQLITE_BUSY to the caller). It would be better if this invoked the 
** busy-handler function registered with the user-level database handle.
*/
static int zipvfsBusyHandler(void *p){
  return 0;
}

/*
** Function registered as the "reinit" callback with the pager module used
** to write to the file. Since there is no user data associated with each
** page, this callback is a no-op.
*/
static void zipvfsReinitPage(DbPage *pPage){ 
  return;
}

/*
** Argument zName is the file name passed to the zipvfs xOpen() method to
** open a new file. This function checks if zName contains a URI option
** "auto_detect". Set the zipvfs_file.bAutoDetect variable accordingly.
*/
static void zipvfsSetAutoDetect(zipvfs_file *pZip, const char *zName){
  const char *zAutoDetect;        /* Value of auto_detect option */

  assert( pZip->bAutoDetect==0 );
  zAutoDetect = sqlite3_uri_parameter(zName, "auto_detect");
  if( !zAutoDetect || sqlite3Atoi(zAutoDetect) ){
    pZip->bAutoDetect = 1;
  }
}

/*
** Argument zName is the file name passed to the zipvfs xOpen() method to
** open a new file. This function checks if zName contains a URI option
** "block_size". If so, and if it is set to an integer power of two between
** 512 and 65536, set zipvfs_file.nBlocksize to this value.
*/
static void zipvfsSetBlocksize(zipvfs_file *pZip, const char *zName){
  const char *zBlocksize;         /* Value of block_size option */

  assert( pZip->nBlocksize==0 );
  zBlocksize = sqlite3_uri_parameter(zName, "block_size");
  if( zBlocksize ){
    int nBlocksize = sqlite3Atoi(zBlocksize);
    if( (nBlocksize & (nBlocksize-1))==0
     && (nBlocksize >= 512)
     && (nBlocksize <= SQLITE_MAX_PAGE_SIZE)
    ){
      pZip->nBlocksize = nBlocksize;
    }
  }
}

/*
** Open a zipvfs file handle.
*/
static int zipvfsOpen(
  sqlite3_vfs *pVfs,              /* Pointer to ZipvfsVfs object */
  const char *zName,              /* Name of file to open */
  sqlite3_file *pFile,            /* OUT: Write file handle here */
  int flags,                      /* xOpen() flags */
  int *pOutFlags                  /* OUT: Actual flags used with xOpen() */
){
  int rc = SQLITE_OK;             /* Return code */
  zipvfs_file *pZipFd = (zipvfs_file *)pFile;

  memset(pZipFd, 0, sizeof(*pZipFd));
  pZipFd->zFilename = zName;
  pZipFd->openFlags = flags;
  pZipFd->nAutoCheckpoint = SQLITE_DEFAULT_WAL_AUTOCHECKPOINT;
  if( flags&SQLITE_OPEN_MAIN_DB ){
    /* This is a main database file.  Open a pager object that uses the 
    ** underlying "real" VFS for IO. By using a pager object, we can
    ** safely write to the database file without risking corruption in
    ** the event of a crash or power failure. 
    */
    zipvfsSetAutoDetect(pZipFd, zName);
    zipvfsSetBlocksize(pZipFd, zName);
    pZipFd->pZipVfs = (ZipvfsVfs *)pVfs;
    if( rc==SQLITE_OK ){
      rc = sqlite3PagerOpen(
          SYSVFS(pVfs), &pZipFd->pPager, zName, 0, 0,
          SQLITE_OPEN_MAIN_DB|(flags&0x78047),
          zipvfsReinitPage
      );
    }
    if( rc==SQLITE_OK ){
      sqlite3PagerSetBusyhandler(pZipFd->pPager, zipvfsBusyHandler, 0);
      pZipFd->nMaxFree = ZIPVFS_DEFAULT_MAXFREE;
      pZipFd->nMaxFrag = ZIPVFS_DEFAULT_MAXFRAG;
      if( sqlite3_uri_parameter(zName, "excl")!=0 ){
        sqlite3PagerLockingMode(pZipFd->pPager, PAGER_LOCKINGMODE_EXCLUSIVE);
      }

      if( sqlite3PagerIsreadonly(pZipFd->pPager) ){
        flags &= ~SQLITE_OPEN_READWRITE;
        flags |= SQLITE_OPEN_READONLY;
      }
      *pOutFlags = flags;
    }

    /* Add the new file to the global list */
    if( rc==SQLITE_OK ) zipvfsAddFileToList(pZipFd);

  }else{
    /* Otherwise, if this is not a main-db file, then use the underlying
    ** VFS to open a file-handle directly. Except, if the caller is trying
    ** to open a main-journal file, open a temp file handle instead. We
    ** cannot have the upper layer using a real persistent journal file, as
    ** it would have the same name as the journal created by the pager
    ** sub-system in order to safely write to the associated zipvfs database 
    ** file.
    */
    sqlite3_file *pRealFd = SYSFILE(pZipFd);
    if( zipvfsIsJournalFile(pVfs, zName) ){
      assert( flags & SQLITE_OPEN_MAIN_JOURNAL );
      assert( !pOutFlags );
      flags = SQLITE_OPEN_TEMP_JOURNAL | SQLITE_OPEN_DELETEONCLOSE
            | SQLITE_OPEN_READWRITE    | SQLITE_OPEN_CREATE;
      zName = 0;
    }
    rc = SYSVFS(pVfs)->xOpen(SYSVFS(pVfs), zName, pRealFd, flags, pOutFlags);
  }

  if( rc==SQLITE_OK ){
    pZipFd->base.pMethods = &zipvfs_io_methods;
  }
 
  return rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static int zipvfsDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
  int rc = SQLITE_OK;
  if( !zipvfsIsJournalFile(pVfs, zPath) ){
    rc = SYSVFS(pVfs)->xDelete(SYSVFS(pVfs), zPath, dirSync);
  }
  return rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int zipvfsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int flags, 
  int *pRes
){
  int rc;                         /* Return code */

  rc = SYSVFS(pVfs)->xAccess(SYSVFS(pVfs), zPath, flags, pRes);
  if( rc==SQLITE_OK && flags==SQLITE_ACCESS_EXISTS && *pRes ){
    if( zipvfsIsJournalFile(pVfs, zPath) ) *pRes = 0;
  }
  return rc;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (pVfs->mxPathname+1) bytes.
*/
static int zipvfsFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int nOut, 
  char *zOut
){
  return SYSVFS(pVfs)->xFullPathname(SYSVFS(pVfs), zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *zipvfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  return SYSVFS(pVfs)->xDlOpen(SYSVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void zipvfsDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  SYSVFS(pVfs)->xDlError(SYSVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*zipvfsDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  return SYSVFS(pVfs)->xDlSym(SYSVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void zipvfsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  SYSVFS(pVfs)->xDlClose(SYSVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static int zipvfsRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
  return SYSVFS(pVfs)->xRandomness(SYSVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static int zipvfsSleep(sqlite3_vfs *pVfs, int nMicro){
  return SYSVFS(pVfs)->xSleep(SYSVFS(pVfs), nMicro);
}

/*
** Return details of the most recent OS-level error.
*/
static int zipvfsGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
  return SYSVFS(pVfs)->xGetLastError(SYSVFS(pVfs), nBuf, zBuf);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int zipvfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeout){
  return SYSVFS(pVfs)->xCurrentTime(SYSVFS(pVfs), pTimeout);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int zipvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeout){
  return SYSVFS(pVfs)->xCurrentTimeInt64(SYSVFS(pVfs), pTimeout);
}

/*
** Create a new Zipvfs VFS.
*/
static int zipvfsCreateVfs(
  char const *zName,              /* Name of new VFS */
  char const *zParent,            /* Underlying VFS to use */
  void *pCtx,                     /* Context pointer for xAutoDetect */
  int (*xAutoDetect)(void*,const char *zFile,const char *zHdr, ZipvfsMethods*),
  ZipvfsVfs **ppVfs               /* OUT: A pointer to the new VFS */
){
  sqlite3_vfs *pSys;              /* Underlying VFS (VFS zParent) */
  ZipvfsVfs *pNew;                /* New VFS object (VFS zName) */
  char *zCopy;                    /* Copy of zName */
  int nName;                      /* Number of bytes in zName */
  int rc;

  pSys = sqlite3_vfs_find(zParent);
  if( !pSys ) return SQLITE_ERROR;

  nName = sqlite3Strlen30(zName);
  if( nName>ZIPVFS_MAX_NAME ){
    return SQLITE_ERROR;
  }
  pNew = (ZipvfsVfs *)sqlite3_malloc(sizeof(ZipvfsVfs) + nName + 1);
  if( !pNew ) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(ZipvfsVfs));
  zCopy = (char *)&pNew[1];
  memcpy(zCopy, zName, nName+1);

  pNew->base.iVersion = 2;
  pNew->base.mxPathname = pSys->mxPathname;
  pNew->base.szOsFile = sizeof(zipvfs_file) + pSys->szOsFile;
  pNew->base.zName = (const char *)zCopy;
  pNew->base.pAppData = (void *)pSys;
  pNew->base.xOpen = zipvfsOpen;
  pNew->base.xDelete = zipvfsDelete;
  pNew->base.xAccess = zipvfsAccess;
  pNew->base.xFullPathname = zipvfsFullPathname;
  pNew->base.xDlOpen = zipvfsDlOpen;
  pNew->base.xDlError = zipvfsDlError;
  pNew->base.xDlSym = zipvfsDlSym;
  pNew->base.xDlClose = zipvfsDlClose;
  pNew->base.xRandomness = zipvfsRandomness;
  pNew->base.xSleep = zipvfsSleep;
  pNew->base.xCurrentTime = zipvfsCurrentTime;
  pNew->base.xGetLastError = zipvfsGetLastError;
  pNew->base.xCurrentTimeInt64 = zipvfsCurrentTimeInt64;

  pNew->xGetMethods = xAutoDetect;
  pNew->pGetMethodsCtx = pCtx;

  /* Coverage: This call never fails. The only way sqlite3_vfs_register()
  ** can fail is if a call to sqlite3_initialize() fails. But SQLite must
  ** already be initialized at this point, as the calls to vfs_find() and
  ** sqlite3_malloc() above have succeeded. */
  rc = sqlite3_vfs_register(&pNew->base, 0);
  if( NEVER(rc!=SQLITE_OK) ) sqlite3_free(pNew);
  if( ppVfs ) *ppVfs = pNew;
  return rc;
}
int zipvfs_create_vfs_v3(
  char const *zName,
  char const *zParent,
  void *pCtx,
  int (*xAutoDetect)(void*,const char *zFile,const char *zHdr, ZipvfsMethods*)
){
  return zipvfsCreateVfs(zName, zParent, pCtx, xAutoDetect, 0);
}

/*
** This function is used as the xAutoDetect callback for files that use VFS's
** created with either zipvfs_create_vfs() or zipvfs_create_vfs_v2().
*/
static int zipvfsLegacyAutodetect(
  void *pCtx,                     /* Pointer to ZipvfsVfs structure */
  const char *zFilename,          /* Database file name */
  const char *zHdr,               /* 13-byte header read from database */
  ZipvfsMethods *pOut             /* OUT: Compression methods to use */
){
  ZipvfsVfs *p;                   /* The VFS that owns the methods to use */
  const char *zAutoDetect;        /* Value of auto_detect option */
  int rc = SQLITE_OK;             /* Return Code */

  zAutoDetect = sqlite3_uri_parameter(zFilename, "auto_detect");
  if( zHdr && (!zAutoDetect || sqlite3Atoi(zAutoDetect)) ){
    p = (ZipvfsVfs *)sqlite3_vfs_find(zHdr);
    if( !p 
     || p->base.xOpen!=zipvfsOpen 
     || p->xGetMethods!=zipvfsLegacyAutodetect
    ){
      rc = SQLITE_CANTOPEN;
    }
  }else{
    /* A new file. Or auto_detect=0 was specified. Use the default methods. */
    p = (ZipvfsVfs *)pCtx;
  }

  if( rc==SQLITE_OK ){
    memcpy(pOut, &p->dflt, sizeof(ZipvfsMethods));
    if( p->xLegacyOpen ){
      rc = p->xLegacyOpen(p->dflt.pCtx, zFilename, &pOut->pCtx);
    }
  }
  return rc;
}

/*
** Create and register a new zipvfs VFS.
**
** zName:               Name of the new VFS to be created
** zParent:             Name of an existing VFS that the new VFS will
**                      be layered on top of.
** pCtx:                Pointer passed through as the first argument to
**                      the following three compression routines.
** xCompressBound:      Function that returns the required size of the output
**                      buffer for xCompress()
** xCompress:           Compression function
** xUncompress:         Decompression function
*/
int zipvfs_create_vfs_v2(
  char const *zName,
  char const *zParent,
  void *pCtx,
  int (*xCompressBound)(void *, int),
  int (*xCompress)(void *, char *aDest, int *pnDest, char *aSrc, int nSrc),
  int (*xUncompress)(void *, char *aDest, int *pnDest, char *aSrc, int nSrc),
  int (*xLegacyOpen)(void*,const char*,void**),
  int (*xCompressClose)(void*)
){
  ZipvfsVfs *pNew;                /* New VFS object (VFS zName) */
  int rc;                         /* Return Code */

  rc = zipvfsCreateVfs(zName, zParent, 0, zipvfsLegacyAutodetect, &pNew);
  if( rc==SQLITE_OK ){
    assert( pNew );
    pNew->pGetMethodsCtx = (void *)pNew;
    pNew->dflt.zHdr = pNew->base.zName;
    pNew->dflt.pCtx = pCtx;
    pNew->dflt.xCompressBound = xCompressBound;
    pNew->dflt.xCompress = xCompress;
    pNew->dflt.xUncompress = xUncompress;
    pNew->dflt.xCompressClose = xCompressClose;
    pNew->xLegacyOpen = xLegacyOpen;
  }

  return rc;
}
int zipvfs_create_vfs(
  char const *zName,
  char const *zParent,
  void *pCtx,
  int (*xCompressBound)(void *, int),
  int (*xCompress)(void *, char *aDest, int *pnDest, char *aSrc, int nSrc),
  int (*xUncompress)(void *, char *aDest, int *pnDest, char *aSrc, int nSrc)
){
  return zipvfs_create_vfs_v2(
      zName, zParent, pCtx, xCompressBound, xCompress, xUncompress, 0, 0
  );
}

/*
** Destroy a zipvfs that was previously created by a call to 
** zipvfs_create_vfs().
**
** VFS objects are not reference counted. If a VFS object is destroyed
** before all database handles that use it have been closed, the results 
** are undefined and probably undesirable.
*/
void zipvfs_destroy_vfs(const char *zName){
  sqlite3_vfs *p;
  p = sqlite3_vfs_find(zName);
  if( p ){
    sqlite3_vfs_unregister(p);
    sqlite3_free(p);
  }
}

/*
** Return a pointer to a static buffer containing an English language
** interpretation of the error code passed as the only argument. The error
** code must be one returned by either zipvfs_create_vfs(), or a call to
** sqlite3_file_control() with one of the ZIPVFS_CTRL_*** verbs.
*/
const char *zipvfs_errmsg(int rc){
  return sqlite3ErrStr(rc);
}


/*************************************************************************
** Below this point is the b-tree code to manage the free-slot list.
** The API used by the main zipvfs code to access the b-tree is declared
** above (near the top of this file). All functions in the API begin with 
** "zipvfsFreelist":
**
**     zipvfsFreelistAdd
**         Add a new entry to the free-slot list.
**
**     zipvfsFreelistRemove
**         Remove a specific entry from the free-slot list (identified by its
**         size and offset within the file).
**
**     zipvfsFreelistBestfit
**         Find the entry within the free-slot list that is the best available
**         fit for a specified payload. Depending on the results of the search,
**         possibly remove the free-slot list entry.
**
**     zipvfsFreelistTest
**         Test to see if the free-slot list contains a slot that matches
**         a specific offset/size supplied by the caller. This is used by the
**         integrity-check procedure.
**
**     zipvfsFreelistIterate
**         Iterate through the free-list, invoking a callback for each entry.
**         This function is used only as part of the test file-control 
**         ZIPVFS_CTRL_STRUCTURE. It is omitted if SQLITE_TEST is not defined
**         at build time.
**
**     zipvfsFreelistIntegrity
**         Check that the free-list b-tree is internally consistent. This is
**         used as part of the ZIPVFS_CTRL_INTEGRITY_CHECK integrity-check
**         procedure.
**
** Functions used internally by the b-tree code start with "zipvfsFl". 
** Normally we would put code like this in a separate C file, but that is
** inconvenient in this case because of the way the zipvfs code is 
** packaged, distributed and deployed.
*/

/*
** B-Tree node format:
**
** All slots in the database, free or otherwise, begin with a slot-header.
** For b-tree (or other free) slots, the page-number field is meaningless.
** The slot payload size is used though. Following the slot header is a
** 4 byte b-tree node header:
**
**   Height:             2 bytes. 
**     Height of the sub-tree headed by this node. Leaf nodes have this
**     value set to 1, the parents of leaf nodes have 2, etc.
**
**   Number of entries:  2 bytes. 
**     Number of entries currently stored in node.
**
** Each key in the b-tree is an 8-byte integer. The integer is broken down
** into three components describing the free-slot that the key represents:
**
**   * a 17-bit payload size field (most significant)
**   * a 40-bit offset (next most significant)
**   * a 1-bit flag set to true if the slot is itself part of 
**     the b-tree structure (least significant).
**
** Following the header, a leaf b-tree node contains N entries of 8 bytes 
** each (just the key value), where N is the value stored in the "Number 
** of entries" field of the b-tree node header. 
**
** For internal nodes (those with height>1), the b-tree node header is 
** followed by a 5 byte offset (offset of the right-child) and N 13-byte 
** entries, where each entry is a key value is followed by a 5 byte child 
** slot offset.
**
** For both internal and leaf nodes, keys are stored in ascending order.
** 
** For each entry on an internal node, all keys stored in the sub-tree
** headed by the child slot are less than the key stored as part of the
** entry itself. If the slot is not the left-most on its node, then all
** keys stored in the sub-tree are greater than the key associated with
** the key in the previous entry. All keys stored in the sub-tree headed
** by the right-child are greater than all keys stored on the internal
** node page.
**
** The minimum payload size for an internal node is (6 + 5 + 2*13)==37 
** bytes (the Page Number field of the header is not part of the payload). 
** To ensure that this is always true, the zipvfs module never allocates a 
** slot with a payload smaller than ZIPVFS_MIN_PAYLOAD bytes.
*/

typedef struct ZipvfsFlPath ZipvfsFlPath;
typedef struct ZipvfsFlSlot ZipvfsFlSlot;

#define ZIPVFS_KEYISUSED_MASK 0x00000001
#define ZIPVFS_KEY_MASK ((((u64)0xFFFFFFFF) << 32) + (u64)0xFFFFFFFE)

/*
** An object of this type represents a path from the root node down to an
** entry somewhere within the b-tree (either on an interior or leaf node).
*/
struct ZipvfsFlPath {
  int nSlot;                      /* Size of array aSlot */
  ZipvfsFlSlot *aSlot;            /* Slots. aSlot[0] is always the root node */

  /* The following are used only during InsertKey operations. */
  i64 iRootOff;                   /* Offset to new root slot */
  int nRootPayload;               /* Size of new root slot */
};

/*
** A single node that is part of a ZipvfsFlPath object.
*/
struct ZipvfsFlSlot {
  i64 iOff;                       /* Offset of this slot in file */
  int iEntry;                     /* Entry on this page */
  u8 *aPayload;                   /* Slot payload */
  int nPayload;                   /* Size of buffer aPayload[] in bytes */

  /* The following are used only during InsertKey operations. */
  i64 iSiblingOff;                /* Offset of new sibling slot */
  int nSiblingPayload;            /* Size of new sibling slot */
};

/*
** A b-tree node header consists of three 16-bit big-endian unsigned integer
** values. The payload, height and number-of-entries fields specifically.
** See above for an in-depth description of the b-tree node format.
**
** This function decodes a b-tree node header and writes the decode values
** to output variables *pnPayload, *piHeight and *pnEntry.
*/
static void zipvfsFlNodeReadHeader(
  u8 *aHdr,                       /* Pointer to buffer containing node header */
  int *piHeight,                  /* OUT: Height of node in b-tree */
  int *pnEntry                    /* OUT: Number of entries on b-tree node */
){
  if( piHeight ) *piHeight = (int)zipvfsGetU16(&aHdr[0]);
  if( pnEntry ) *pnEntry = (int)zipvfsGetU16(&aHdr[2]);
}

/*
** Return the offset within the payload of node entry iEntry. The first
** entry on a node is entry 0, the second entry 1, etc. iHeight is the
** height of the node in the tree (1 for leaf nodes, 2 for their parents, 
** etc.).
**
** Leaf nodes begin with a 4 byte header. Followed by tightly packed 
** 8-byte entries. Interior nodes begin with a 4 byte header followed by a
** 5 byte right-child offset followed by tightly packed 13 byte entries.
*/
static int zipvfsFlNodeOffset(int iHeight, int iEntry){
  return 4 + ((iHeight>1)?5:0) + iEntry*((iHeight>1)?13:8);
}

/*
** Return the size of each entry stored on b-tree nodes with height iHeight.
** This is 8 bytes for leaf nodes (iHeight==1) and 13 bytes for interior
** nodes (iHeight>1).
*/
static int zipvfsFlEntrySize(int iHeight){
  return 8 + ((iHeight>1)?5:0);
}

/*
** Load the contents of a b-tree node into a malloced buffer. Decode header
** fields at the same time.
*/
static void zipvfsFlLoadSlot(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOff,                       /* Offset of node to load */
  int *pnPayload,                 /* OUT: Node payload size */
  int *piHeight,                  /* OUT: Height field of node header */
  int *pnEntry,                   /* OUT: Number of entries stored on node */
  u8 **paPayload,                 /* OUT: Malloced buffer containing payload */
  int *pRc                        /* IN/OUT: Error code */
){
  int nPayload = 0;               /* Node payload size */
  u8 *aPayload;

  assert( paPayload );
  zipvfsReadSlotHdr(pZip, iOff, 0, &nPayload, pRc); 
  aPayload = zipvfsMalloc(nPayload, pRc);
  zipvfsLoadData(pZip, iOff+ZIPVFS_SLOT_HDRSIZE, nPayload, aPayload, pRc);
  
  *pnPayload = nPayload;
  *paPayload = aPayload;

  if( *pRc==SQLITE_OK ){
    int nEntry;                   /* Number of entries on node */
    int iHeight;                  /* Height of slot in tree */
    assert( nPayload>=ZIPVFS_MIN_PAYLOAD );
    zipvfsFlNodeReadHeader(aPayload, &iHeight, &nEntry);
    if( nEntry<1 || iHeight<1 
     || zipvfsFlNodeOffset(iHeight, nEntry)>nPayload 
    ){
      *pRc = ZIPVFS_CORRUPT;
    }
    if( pnEntry ) *pnEntry = nEntry;
    if( piHeight ) *piHeight = iHeight;
  }
}

/*
** Search for the key iKey in the free-list b-tree. If the key is present,
** set the contents of path *pPath to point to it and return non-zero. 
**
** If the key is not present, set pPath either to point to a leaf page entry
** that key iKey should be inserted immediately before (or to point to a
** non-existant entry to the right of the right-most entry, if the key should
** be inserted at the extreme right of a leaf page. In this case return zero.
**
** If parameter iStop is greater than 1, then stop descending the tree at 
** the level where nodes have the "height" header-field set to iStop. This
** is to allow sub-trees to be inserted back into the tree as part of b-tree
** rebalancing.
**
** If an error occurs, set *pRc to an error code and return 0. If *pRc is
** other than SQLITE_OK when this function is called, return without doing
** anything.
*/
static int zipvfsFlSeek(
  zipvfs_file *pZip,              /* Zip file handle */
  i64 iKey,                       /* Key value to search the b-tree for */
  ZipvfsFlPath *pPath,            /* OUT: Results of a successful search */
  int iStop,                      /* Stop at this height (1 for leaves) */
  int *pRc                        /* IN/OUT: Error code */
){
  i64 iOff;                       /* Offset of current b-tree node */
  int nAlloc = 0;                 /* Allocated size of pPath->aSlot[] */

  assert( pPath->nSlot==0 && pPath->aSlot==0 );
  assert( iStop>0 );
  assert( *pRc || (pZip->hdr.iFreeSlot && pZip->hdr.nFreeSlot) );
  assert( iKey==(iKey & ZIPVFS_KEY_MASK) );

  iOff = pZip->hdr.iFreeSlot;
  while( *pRc==SQLITE_OK ){
    int iHeight;                  /* Height header field of this node */
    int nEntry;                   /* Number of entries stored on node */
    ZipvfsFlSlot *pSlot;          /* Load node into this slot. */


    /* Extend the pPath->aSlot[] array, if necessary, by 8 additional
    ** slots and zero out those 8 new slots. */
    if( nAlloc==pPath->nSlot ){
      ZipvfsFlSlot *aNew;

      if( nAlloc>=64 ){
        /* The b-tree cannot reasonably have a height greater than 64, since
        ** each node stores at least one key and has a fanout of at least 2.
        ** A tree of height 64 would contain at least 2^64 leaf nodes, 
        ** suggesting the file contains more than 2^64 free slots. Not 
        ** possible. The b-tree structure must be corrupt. */
        *pRc = ZIPVFS_CORRUPT;
        return 0;
      }

      nAlloc += 8;
      aNew = sqlite3_realloc(pPath->aSlot, nAlloc * sizeof(ZipvfsFlSlot));
      if( !aNew ){
        *pRc = SQLITE_NOMEM;
        return 0;
      }
      assert( (nAlloc-pPath->nSlot)==8 );
      memset(&aNew[pPath->nSlot], 0, 8*sizeof(ZipvfsFlSlot));
      pPath->aSlot = aNew;
    }

    /* Load the slot payload into memory */
    pSlot = &pPath->aSlot[pPath->nSlot++];
    pSlot->iOff = iOff;
    zipvfsFlLoadSlot(
        pZip, iOff, &pSlot->nPayload, &iHeight, &nEntry, &pSlot->aPayload, pRc
    );
    if( *pRc ) return 0;

    /* Search for the smallest key on the slot that is equal to or greater
    ** than key iKey. TODO: Should do a binary search here. And use memcmp()
    ** to compare serialized keys instead of deserializing each key into an 
    ** i64 variable.  */
    for(pSlot->iEntry=0; pSlot->iEntry<nEntry; pSlot->iEntry++){
      i64 iThisKey;
      int iKeyOff;
     
      iKeyOff = zipvfsFlNodeOffset(iHeight, pSlot->iEntry);
      iThisKey = zipvfsGetU64(&pSlot->aPayload[iKeyOff]) & ZIPVFS_KEY_MASK;
      if( iThisKey==iKey ) return 1;
      if( iThisKey>iKey ) break;
    }

    if( iHeight==iStop ) break;
    if( pSlot->iEntry==nEntry ){
      iOff = zipvfsGetU40(&pSlot->aPayload[4]);
    }else{
      int iEntryOff = zipvfsFlNodeOffset(iHeight, pSlot->iEntry);
      iOff = zipvfsGetU40(&pSlot->aPayload[iEntryOff + 8]);
    }
  }

  return 0;
}

/*
** A b-tree key is a 7-byte blob of data. The first (most significant) two 
** bytes store the size of a free slot. The remaining five bytes store its
** slot offset in the file. This function is used to split a key, stored
** in a 64-bit integer variable, into its two components.
*/
static void zipvfsFlDecodeKey(
  i64 iKey,                       /* 7 byte key value to split */
  int *pnPayload,                 /* OUT: Payload size key component */
  i64 *piOffset,                  /* OUT: Offset key component */
  int *pbUsed                     /* OUT: True if slot is part of b-tree */
){
  static const i64 offset_mask = ((((i64)0xFF) << 32) + (i64)0xFFFFFFFF);

  *piOffset = (iKey >> 1) & offset_mask;
  *pnPayload = (iKey >> 41);
  if( pbUsed ) *pbUsed = (iKey & 0x01);
}

/*
** This function encodes a size and offset into a 7-byte key. The key value 
** is returned stored in a 64-bit integer variable.
*/
static i64 zipvfsFlEncodeKey(
  int nPayload,                   /* Payload size key component */
  i64 iOffset                     /* Offset key component */
){
  return ((i64)nPayload << 41) + (iOffset << 1);
}

/*
** Copy a single b-tree entry to destination buffer aDest. If the iHeight
** parameter is greater than one, the entry is followed by a 5 byte offset
** value, as it is on interior nodes. Otherwise, if iHeight==1, the entry
** consists of just the 8-byte key and flag combination.
**
** If parameter iEntry is less than iNewEntry, then it is the index of an
** entry to copy from b-tree node payload aPayload[].
**
** If parameter iEntry is equal to iNewEntry, then instead of copying an
** entry from within aPayload[], the entry is formed from key iNewKey 
** and, if iHeight>1, offset iNewOff.
**
** If parameter iEntry is greater than iNewEntry, then the (iEntry-1)th
** entry on aPayload[] is copied to the destination buffer.
**
** This function is used as a helper function by zipvfsFlInsertEntry() when
** distributing entries after splitting a node.
*/
static void zipvfsFlCopyEntry(
  u8 *aDest,                      /* Copy key value to this buffer */
  int iHeight,                    /* Height of nodes to copy keys between */
  int iEntry,                     /* Entry to copy. */
  u8 *aPayload,                   /* Payload to copy entries from */
  int iNewEntry,                  /* Index of "new" entry */
  i64 iNewKey,                    /* New key value */
  i64 iNewOff                     /* New offset value */
){
  assert( (iHeight==1 && iNewOff==0) || (iHeight>1 && iNewOff>0) );
  if( iEntry==iNewEntry ){
    zipvfsPutU64(aDest, iNewKey);
    if( iHeight>1 ) zipvfsPutU40(&aDest[8], iNewOff);
  }else{
    int i = zipvfsFlNodeOffset(iHeight, iEntry - (iEntry>iNewEntry));
    memcpy(aDest, &aPayload[i], 8 + 5*(iHeight>1));
  }
}

/*
** Insert a new key at the point in the tree indicated by pPath. If new
** nodes will need to be allocated as part of this insertion (i.e. if one
** or more of the nodes in the path need to be split), they should already
** have been allocated by zipvfsFlAllocate().
*/
static void zipvfsFlInsertEntry(
  zipvfs_file *pZip,              /* Zip file handle */
  i64 iKey,                       /* Key value to insert */
  i64 iOff,                       /* Offset to insert */
  ZipvfsFlPath *pPath,            /* Point to insert new entry at */
  int *pRc                        /* IN/OUT: Error code */
){
  int iSlot;
  i64 iInsertKey = iKey;          /* Key value to insert */
  i64 iInsertOff = iOff;          /* Offset to accompany inserted key */
  int iHeight = 0;                /* Height of node in tree */

  for(iSlot=pPath->nSlot-1; *pRc==SQLITE_OK && iSlot>=0; iSlot--){
    ZipvfsFlSlot *pSlot = &pPath->aSlot[iSlot];
    u8 *aPayload = pSlot->aPayload;
    int nPayload = pSlot->nPayload;
    int iEntry = pSlot->iEntry;   /* Insert new entry here */
    int nEntry;                   /* Number of entries in this node */
    sqlite3_int64 iSlotOff;       /* Write pSlot here after modifying it */

    iSlotOff = pSlot->iOff + ZIPVFS_SLOT_HDRSIZE;
    zipvfsFlNodeReadHeader(aPayload, &iHeight, &nEntry);

    assert(
        (!pSlot->iSiblingOff && zipvfsFlNodeOffset(iHeight, nEntry+1)<=nPayload)
     || (pSlot->iSiblingOff && zipvfsFlNodeOffset(iHeight, nEntry+1)>nPayload)
    );

    if( pSlot->iSiblingOff==0 ){
      /* There is space on this node. Insert the new entry. */
      int iSrc = zipvfsFlNodeOffset(iHeight, iEntry);
      int iDest = zipvfsFlNodeOffset(iHeight, iEntry+1);
      int nByte = zipvfsFlNodeOffset(iHeight, nEntry) - iSrc;

      memmove(&aPayload[iDest], &aPayload[iSrc], nByte);

      /* Write the new entry into the node. */
      zipvfsPutU64(&aPayload[iSrc], iInsertKey);
      if( iHeight>1 ) zipvfsPutU40(&aPayload[iSrc+8], iInsertOff);

      /* Increase the nEntry field in the header by one. */
      zipvfsPutU16(&aPayload[2], nEntry+1);

      /* Write the node back to the database and return. */
      zipvfsStoreData(pZip, iSlotOff, aPayload, nPayload, pRc);
      return;
    }else{
      /* There is insufficient space on this node. Split the node into
      ** two siblings and add the new key to one of them. The next iteration 
      ** of the loop will insert the pointer to the new sibling node into the 
      ** parent. */
      i64 iNewOff;                /* Offset of new sibling node */
      int nNewPayload;            /* Payload size for new sibling node */
      u8 *aNewPayload;            /* Pointer to malloc'd buffer */

      int nNewEntry;              /* Number of entries for new sibling */
      int nOldEntry;              /* Number of entries for old sibling */
      int nNewAvail;              /* New sibling space available for entries */
      int i;                      /* Iterator variable */
      i64 iNewKey;                /* Key to insert into parent */

      iNewOff = pSlot->iSiblingOff;
      nNewPayload = pSlot->nSiblingPayload;

      /* Allocate space to create the new node in. */
      aNewPayload = zipvfsMalloc(nNewPayload, pRc);
      if( *pRc ) return;

      /* The new node will be the left-hand sibling of the existing node. 
      ** This is easier, as we do not have to modify the entry in the parent
      ** that contains the pointer to the existing node. And it means the
      ** new node will not be the right-child of its parent.
      **
      ** The new entry (iInsertKey/iInsertOff) and the entries on the existing 
      ** node need to be distributed between the two siblings. One entry is
      ** added to the parent to point to the new sibling. In total there are
      ** (nEntry+1) entries to add. Distributed as follows:
      */
      nNewAvail = nNewPayload - ZIPVFS_SLOT_HDRSIZE - 5 * (iHeight>1);
      nNewEntry = MIN((nEntry+1)/2, nNewAvail/zipvfsFlEntrySize(iHeight));
      nOldEntry = nEntry - nNewEntry;

      /* Set iNewKey to the key value that will be inserted into the parent
      ** (the divider entry). If required, set the right-child pointer on
      ** the new sibling page.
      */
      zipvfsFlCopyEntry(aNewPayload, iHeight, 
          nNewEntry, aPayload, iEntry, iInsertKey, iInsertOff
      );
      iNewKey = zipvfsGetU64(aNewPayload);
      if( iHeight>1 ){
        for(i=8; i<13; i++) aNewPayload[i-4] = aNewPayload[i];
      }

      /* Populate the new sibling node header and entries. */
      zipvfsPutU16(&aNewPayload[0], iHeight);
      zipvfsPutU16(&aNewPayload[2], nNewEntry);
      for(i=0; i<nNewEntry; i++){
        zipvfsFlCopyEntry(&aNewPayload[zipvfsFlNodeOffset(iHeight, i)], 
            iHeight, i, aPayload, iEntry, iInsertKey, iInsertOff
        );
      }

      /* Update the old sibling node */
      zipvfsPutU16(&aPayload[2], nOldEntry);
      for(i=0; i<nOldEntry; i++){
        zipvfsFlCopyEntry(&aPayload[zipvfsFlNodeOffset(iHeight, i)], iHeight, 
            i+nNewEntry+1, aPayload, iEntry, iInsertKey, iInsertOff
        );
      }

      /* Write both siblings to the file. */
      zipvfsStoreData(pZip, iNewOff+ZIPVFS_SLOT_HDRSIZE, aNewPayload,
          zipvfsFlNodeOffset(iHeight, nNewEntry), pRc
      );
      zipvfsStoreData(pZip, iSlotOff, aPayload,
          zipvfsFlNodeOffset(iHeight, nOldEntry), pRc
      );
      zipvfsFree(aNewPayload);

      iInsertOff = iNewOff;
      iInsertKey = iNewKey;
    }
  }

  if( *pRc==SQLITE_OK ){
    /* If control flows to here, then the root node has been split into two
    ** siblings. Create a new root node to be the parent of the new sibling
    ** and the old root node. The old root becomes the right-child of the
    ** new root. The only cell on the new root is (iInsertKey/iInsertOff).
    */
    u8 aRoot[2 + 2 + 5 + 8 + 5];

    assert( iInsertOff>0 );
    assert( (sizeof(aRoot)-4)<ZIPVFS_MIN_PAYLOAD );

    zipvfsPutU16(&aRoot[0], iHeight+1);               /* Height */
    zipvfsPutU16(&aRoot[2], 1);                       /* nEntry */
    zipvfsPutU40(&aRoot[4], pZip->hdr.iFreeSlot);     /* Right child ptr */
    zipvfsPutU64(&aRoot[9], iInsertKey);              /* Key value */
    zipvfsPutU40(&aRoot[17], iInsertOff);             /* Left child ptr */

    zipvfsStoreData(pZip, 
        pPath->iRootOff+ZIPVFS_SLOT_HDRSIZE, aRoot, sizeof(aRoot), pRc);
    pZip->hdr.iFreeSlot = pPath->iRootOff;
  }
}

/* Functions zipvfsFlAllocFromOff() and zipvfsFlAllocFromNode() are mutually
** recursive. The following forward declaration makes this possible.
*/
static int zipvfsFlAllocFromOff(zipvfs_file*,ZipvfsFlPath*,i64,int*,i64*,int*);

/*
** This function is called to select (allocate) a currently unused free-slot
** for use as a b-tree node. Argument aPayload contains the payload of an
** existing b-tree node to search for an eligible free-slot (one with the
** flag byte cleared - indicating that it is not currently used as a b-tree
** node).
**
** If the iFirstEntry argument is 0, then all entries in aPayload are 
** considered, as are all entries that lie on child nodes of the node stored
** in aPayload[]. If one is found, then the flag must be set to show the
** slot is now used in three places:
**
**   * Within the aPayload[] buffer itself, and
**
**   * Within the database file, assuming that the node that corresponds to
**     aPayload[] is stored at slot-offset iFree within the db file.
**
**   * If the entry is discovered within a descendent node of aPayload[],
**     within any buffers cached by path pPath. Each node cached by pPath
**     can be identified by the ZipvfsFlSlot.iOff variable.
**
** If iFirstEntry is not 0, then only any entries in aPayload[] with indexes
** smaller than iFirstEntry are ignored. As are entries stored on their 
** child nodes, if any. If iFirstEntry is non-zero, then entries stored within
** the sub-tree headed by the right-child of aPayload[] (if any) are not
** considered.
*/
static int zipvfsFlAllocFromNode(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  ZipvfsFlPath *pPath,            /* Path object to update if required */
  i64 iFree,                      /* Offset to write node content back to */
  int iFirstEntry,                /* Start at this entry */
  u8 *aPayload,                   /* Pointer to buffer containing b-tree node */
  int nPayload,                   /* Size of buffer aPayload[] in bytes */
  int *pnSize,                    /* OUT: Payload size of allocated node */
  i64 *piOff,                     /* OUT: Offset of allocated node */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int iHeight;                  /* Height of this node (aPayload) */
    int nEntry;                   /* Number of entries on node (aPayload) */
    int i;                        /* Iterator variable */

    /* Decode node header */
    zipvfsFlNodeReadHeader(aPayload, &iHeight, &nEntry);

    if( iHeight>1 ){
      /* Search the right-child */
      if( iFirstEntry==0 ){
        i64 iRight = zipvfsGetU40(&aPayload[4]);
        if( zipvfsFlAllocFromOff(pZip, pPath, iRight, pnSize, piOff, pRc) ){
          return 1;
        }
      }

      /* Search the other children */
      for(i=iFirstEntry; i<nEntry; i++){
        i64 iChild = zipvfsGetU40(&aPayload[zipvfsFlNodeOffset(iHeight, i)+8]);
        if( zipvfsFlAllocFromOff(pZip, pPath, iChild, pnSize, piOff, pRc) ){
          return 1;
        }
      }
    }

    /* Search the entries on this page. */
    for(i=iFirstEntry; i<nEntry; i++){
      u8 *aEntry = &aPayload[zipvfsFlNodeOffset(iHeight, i)];
      i64 iKey = zipvfsGetU64(aEntry);
      if( (iKey & ZIPVFS_KEYISUSED_MASK)==0 ){
        zipvfsFlDecodeKey(iKey, pnSize, piOff, 0);
        zipvfsPutU64(aEntry, iKey | ZIPVFS_KEYISUSED_MASK);
        if( iFree ){
          zipvfsStoreData(pZip, 
              iFree + ZIPVFS_SLOT_HDRSIZE + (aEntry-aPayload),
              aEntry, 8, pRc);
        }
        return 1;
      }
    }
  }

  return 0;
}

/*
** Load the b-tree node stored at slot offset iFree, then try to allocate
** a new b-tree node from it using zipvfsFlAllocFromNode().
**
** If slot iFree is cached somewhere within path pPath, do not load the
** slot from disk. Instead, pass the buffer that is part of the path object
** to zipvfsFlAllocFromNode(). This ensures that the cached nodes stay
** up to date.
**
** If a slot can be allocated, return non-zero and set *pnPayload and
** *piOff to describe the allocated slot. If no slot can be allocated 
** (because the in-use flag is already set for all entries on the node
** at iFree and its descendants), return 0.
**
** If *pRc is already other than SQLITE_OK when this function is called,
** it is a no-op. If an error occurs within this function, set *pRc to an 
** error code before returning.
*/
static int zipvfsFlAllocFromOff(
  zipvfs_file *pZip,              /* Zip file handle */
  ZipvfsFlPath *pPath,            /* Path object to update if required */
  i64 iFree,                      /* Offset to root of tree to scan */
  int *pnPayload,                 /* OUT: Payload size of allocated node */
  i64 *piOff,                     /* OUT: Offset of allocated node */
  int *pRc                        /* IN/OUT: Error code */
){
  int ret;                        /* Return value */
  u8 *aPayload = 0;               /* Pointer to payload buffer */
  int nPayload;                   /* Number of bytes in buffer aPayload[] */
  u8 *aFree = 0;                  /* Buffer to free before returning */

  if( pPath ){
    int i;
    for(i=0; i<pPath->nSlot; i++){
      if( pPath->aSlot[i].iOff==iFree ){
        aPayload = pPath->aSlot[i].aPayload;
        nPayload = pPath->aSlot[i].nPayload;
        break;
      }
    }
  }
  if( aPayload==0 ){
    zipvfsFlLoadSlot(pZip, iFree, &nPayload, 0, 0, &aPayload, pRc);
    aFree = aPayload;
  }

  ret = zipvfsFlAllocFromNode(
      pZip, pPath, iFree, 0, aPayload, nPayload, pnPayload, piOff, pRc
  );
  zipvfsFree(aFree);
  return ret;
}

/*
** Allocate a new free-slot for use as a b-tree node. The slot may
** be allocated from one of three places, in order of preference:
**
**   * If *piNewKey is non-zero, then it contains a key value representing
**     a newly freed slot that may be used as part of the b-tree. If this
**     slot is used, *piNewKey is set to zero before returning.
**
**   * If aNode is not NULL, then it contains a b-tree node. A new slot
**     may be allocated from any entry on the b-tree node with an index
**     greater than iNodeEntry, or from any descendent of such an entry
**     (if aNode is not a leaf node). Descendents of the right-child of
**     aNode (if any) are not considered.
**
**   * From within the free-slot b-tree headed by the node at offset
**     pZip->hdr.iFreeSlot.
**
** It is guaranteed that there is at least one available free-slot. If this
** function fails to find one and no other error occurs, *pRc is set to
** SQLITE_CORRUPT before returning.
*/
static void zipvfsFlAllocOne(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  ZipvfsFlPath *pPath,            /* Path object to update if required */
  i64 *piNewKey,                  /* IN/OUT: New free slot available for use */
  int iNodeEntry,                 /* First entry on aNode[] to consider */
  u8 *aNode,                      /* Node image (or NULL) */
  int nNode,                      /* Bytes of data in buffer aNode[] */
  int *pnPayload,                 /* OUT: Payload size of allocated node */
  i64 *piOff,                     /* OUT: Offset of allocated node */
  int *pRc                        /* IN/OUT: Error code */
){
  if( piNewKey && *piNewKey ){
    zipvfsFlDecodeKey(*piNewKey, pnPayload, piOff, 0);
    *piNewKey = 0;
  }else{
    int ret;
    if( aNode ){ 
      ret = zipvfsFlAllocFromNode(
          pZip, 0, 0, iNodeEntry, aNode, nNode, pnPayload, piOff, pRc
      );
      if( ret ) return;
    }
    ret = zipvfsFlAllocFromOff(
        pZip, pPath, pZip->hdr.iFreeSlot, pnPayload, piOff, pRc
    );
    if( ret==0 && *pRc==SQLITE_OK ) *pRc = ZIPVFS_CORRUPT;
  }
}

/*
** The caller is about to insert a new entry into the b-tree, at the point
** indicated by pPath. This function determines whether or not any extra
** nodes will be required by the insert operation, and if so, allocates
** them. The offsets of the allocated nodes are stored within the path
** object itself.
*/
static void zipvfsFlAllocate(
  zipvfs_file *pZip,              /* Zip file handle */
  i64 iNewKey,                    /* New key being inserted (or 0 if none) */
  int iFirstEntry,                /* First entry on aNode to consider */
  u8 *aNode,                      /* Disconnected node to allocate from */
  int nNode,                      /* Bytes in buffer aNode[] */
  ZipvfsFlPath *pPath,            /* Path new key is to be inserted at */
  int *pRc                        /* IN/OUT: Error code */
){
  int i;                          /* Iterator variable */

  for(i=pPath->nSlot; i>0 && *pRc==SQLITE_OK; i--){
    ZipvfsFlSlot *pSlot = &pPath->aSlot[i-1];
    int iHeight;                  /* Height header field for pSlot */
    int nEntry;                   /* Number of entries stored on node pSlot */

    zipvfsFlNodeReadHeader(pSlot->aPayload, &iHeight, &nEntry);
    if( zipvfsFlNodeOffset(iHeight, nEntry+1)<=pSlot->nPayload ) return;

    zipvfsFlAllocOne(pZip, pPath, &iNewKey, iFirstEntry, aNode, nNode,
      &pSlot->nSiblingPayload, &pSlot->iSiblingOff, pRc
    );
  }

  /* This insert is going to need a new root node. Allocate a slot. */
  zipvfsFlAllocOne(pZip, pPath, &iNewKey, iFirstEntry, aNode, nNode,
      &pPath->nRootPayload, &pPath->iRootOff, pRc
  );
}


/*
** Free a path object allocated by zipvfsFlSeek(). This must be called after
** every zipvfsFlSeek() call, even if it returns an error.
*/
static void zipvfsFlFreePath(ZipvfsFlPath *pPath){
  int i;
  for(i=0; i<pPath->nSlot; i++){
    sqlite3_free(pPath->aSlot[i].aPayload);
  }
  sqlite3_free(pPath->aSlot);
  pPath->nSlot = 0;
  pPath->aSlot = 0;
}

/*
** Check to see if the free-slot b-tree contains a key corresponding to a
** free slot with payload nByte bytes at slot-offset iOff. If so, return
** non-zero. Otherwise, if no such slot exists, return zero.
**
** If a slot is found, *pisUsed is set to true if the slot is part of the
** b-tree structure, or false otherwise.
**
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. If an error occurs during this function *pRc is set to an error
** code and zero returned.
*/
static int zipvfsFreelistTest(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOff,                       /* Offset part of b-tree key to test for */
  int nByte,                      /* Payload size part of b-tree key */
  int *pisUsed,                   /* OUT: True if slot is part of b-tree */
  int *pRc                        /* IN/OUT: Error code */
){
  ZipvfsFlPath path = {0, 0, 0, 0};
  int ret = zipvfsFlSeek(pZip, zipvfsFlEncodeKey(nByte, iOff), &path, 1, pRc);
  if( *pRc==SQLITE_OK && ret==1 && pisUsed ){
    u8 *aEntry;
    int iHeight;
    ZipvfsFlSlot *pSlot = &path.aSlot[path.nSlot-1];
    zipvfsFlNodeReadHeader(pSlot->aPayload, &iHeight, 0);
    aEntry = &pSlot->aPayload[zipvfsFlNodeOffset(iHeight, pSlot->iEntry)];
    *pisUsed = (zipvfsGetU32(&aEntry[4]) & ZIPVFS_KEYISUSED_MASK);
  }
  zipvfsFlFreePath(&path);
  return ret;
}

/*
** Add an entry to the free-slot list structure. This function also updates
** the ZipvfsHdr.nFreeSlot, iFreeSlot and nFreeByte header fields.
**
** If iOff==0, this function is a no-op.
**
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. If an error occurs during this function *pRc is set to an error
** code before returning.
*/
static void zipvfsFreelistAdd(
  zipvfs_file *pZip,              /* Zip file handle */
  i64 iOff,                       /* Offset of slot to add to FSL */
  int nByte,                      /* Payload size of slot to add to FSL */
  int *pRc                        /* IN/OUT: Error code */
){
  i64 iKey;                       /* Key value to add to B-Tree */

  if( iOff==0 ) return;
  zipvfsFreelistIntegrity(pZip, pRc);
  pZip->hdr.nFreeSlot++;
  pZip->hdr.nFreeByte += nByte;

  iKey = zipvfsFlEncodeKey(nByte, iOff);

  if( pZip->hdr.iFreeSlot==0 ){
    /* There are currently no free-slots in the list. The newly freed slot
    ** becomes the root of the tree (with zero children). This is the only
    ** case in which a node may have zero children. 
    */
    u8 aRoot[12];                  /* New image for this free-slot */

    /* Write the page image into buffer aRoot[] */
    zipvfsPutU16(&aRoot[0], 1);             /* height=1 (leaf node) */
    zipvfsPutU16(&aRoot[2], 1);             /* Number of entries */
    zipvfsPutU64(&aRoot[4], iKey | ZIPVFS_KEYISUSED_MASK);

    /* Store the contents of aRoot[] in the file */
    zipvfsStoreData(pZip, iOff+6, aRoot, sizeof(aRoot), pRc);

    /* Update the file-header fields */
    pZip->hdr.iFreeSlot = iOff;

  }else{
    /* Seek to the leaf node that the key should be stored on and store
    ** it there. */
    ZipvfsFlPath path = {0, 0, 0, 0};
    zipvfsFlSeek(pZip, iKey, &path, 1, pRc);
    zipvfsFlAllocate(pZip, iKey, 0, 0, 0, &path, pRc);
    if( *pRc==SQLITE_OK ){
      iKey |= (path.aSlot[path.nSlot-1].iSiblingOff!=0);
      zipvfsFlInsertEntry(pZip, iKey, 0, &path, pRc);
    }
    zipvfsFlFreePath(&path);
  }
  zipvfsFreelistIntegrity(pZip, pRc);
}

/*
** If there exists a key in the b-tree corresponding to offset iOff and
** payloadsize nPayload, clear the "in-use" flag associated with the entry.
**
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. If an error occurs during this function *pRc is set to an error
** code before returning.
*/
static void zipvfsFlMarkAsUnused(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOff,                       /* Offset part of key to search for */
  int nPayload,                   /* Payload-size part of key to search for */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int ret;                      /* Return value from zipvfsFlSeek() */
    ZipvfsFlPath path = {0,0,0,0};/* Path to key */

    ret = zipvfsFlSeek(pZip, zipvfsFlEncodeKey(nPayload, iOff), &path, 1, pRc);
    assert( *pRc==SQLITE_OK || ret==0 );
    if( ret ){
      ZipvfsFlSlot *pSlot;        /* Slot containing key nPayload/iOff */
      int iHeight;                /* Height field from node header of pSlot */
      u8 aEntry[8];               /* New serialized key value to write */

      zipvfsPutU64(aEntry, zipvfsFlEncodeKey(nPayload, iOff));
      pSlot = &path.aSlot[path.nSlot-1];
      zipvfsFlNodeReadHeader(pSlot->aPayload, &iHeight, 0);
      zipvfsStoreData(pZip, 
          pSlot->iOff + ZIPVFS_SLOT_HDRSIZE + 
          zipvfsFlNodeOffset(iHeight, pSlot->iEntry),
          aEntry, 8, pRc
      );
    }
    zipvfsFlFreePath(&path);
  }
}

/*
** Path pPath points to a key stored on a leaf node of the free-slot b-tree.
** Remove this key from the b-tree.
**
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. If an error occurs during this function *pRc is set to an error
** code before returning.
*/
static void zipvfsFlDeleteKey(
  zipvfs_file *pZip,              /* Zip file handle */
  ZipvfsFlPath *pPath,            /* Path to key to delete */
  int *pRc                        /* Return code */
){
  int iHeight, nEntry;            /* Slot header values */
  ZipvfsFlSlot *pSlot = &pPath->aSlot[pPath->nSlot-1];
  int nPayload = pSlot->nPayload;

  zipvfsFlNodeReadHeader(pSlot->aPayload, &iHeight, &nEntry);
  if( nEntry==1 ){
    assert( pSlot->iEntry==0 );
    if( pPath->nSlot==1 ){
      /* Root node is now empty. Replace it with it's only child if it is
      ** not a leaf, or set the header field to zero if it is. This is how
      ** the height of the tree is reduced.  */
      if( iHeight==1 ){
        pZip->hdr.iFreeSlot = 0;
      }else{
        pZip->hdr.iFreeSlot = zipvfsGetU40(&pSlot->aPayload[4]);
        zipvfsFlMarkAsUnused(pZip, pSlot->iOff, nPayload, pRc);
      }
    }else{
      /* After removing this entry, the slot will be completely empty. 
      ** If this is a leaf node, we do the following:
      **
      **   1. Find a sibling node. Doesn't matter if the sibling is to the
      **      left or the right. 
      **
      **   2. If there is space for an extra entry on the sibling, copy the
      **      key in the divider cell down into the sibling and delete it from
      **      the parent. Mark the empty slot as "unused" in the b-tree.
      **      To delete the parent cell, this function is called recursively.
      **
      **   3. If there is no space for an extra entry on the sibling, copy
      **      the divider cell down onto this slot. And copy the largest (or
      **      smallest) key out of the sibling to become the new divider.
      */
      ZipvfsFlSlot *pParent = &pPath->aSlot[pPath->nSlot-2];
      int iSib;                   /* Index of sibling in pParent */
      i64 iSibOff;                /* Offset of sibling in file */
      int nParentEntry;           /* Number of entries on parent node */
      int nSiblingPayload;        /* Size of sibling payload */
      int nSiblingEntry;          /* Number of entries on sibling node */
      u8 *aSiblingPayload;        /* Malloc'd buffer for sibling payload */
      u8 *aDiv;                   /* Pointer to divider cell */
      u8 aInsert[14];             /* Buffer large enough for one entry */

      zipvfsFlNodeReadHeader(pParent->aPayload, 0, &nParentEntry);
      if( pParent->iEntry==nParentEntry ){  /* Left-hand sibling */
        pParent->iEntry--;
        iSib = pParent->iEntry;
      }else{                                /* Right-hand sibling */
        iSib = pParent->iEntry + 1;
      }

      aDiv = &pParent->aPayload[zipvfsFlNodeOffset(2, pParent->iEntry)];
      memcpy(aInsert, aDiv, 8);

      if( iSib==nParentEntry ){
        iSibOff = zipvfsGetU40(&pParent->aPayload[4]);
      }else{
        int iPtrOff = zipvfsFlNodeOffset(iHeight+1, iSib) + 8;
        iSibOff = zipvfsGetU40(&pParent->aPayload[iPtrOff]);
      }
      zipvfsFlLoadSlot(pZip, iSibOff, 
          &nSiblingPayload, 0, &nSiblingEntry, &aSiblingPayload, pRc
      );
      if( *pRc ){
        zipvfsFree(aSiblingPayload);
        return;
      }

      if( zipvfsFlNodeOffset(iHeight, nSiblingEntry+1)<=nSiblingPayload ){
        /* There is space on the sibling for an extra entry. Take option (2) 
        ** from the comment above - move the divider entry down to the 
        ** sibling and remove node pSlot from the tree. i.e.
        **
        **         -----CD---               -----D----
        **             /  \         ->          /
        **       ----AB    ------         ---ABC
        **             \         \             \\
        **              x         y             xy
        **
        ** The entry to insert into the sibling page is stored in aInsert[]. 
        ** If the siblings are leaves, this is just the 7 byte key followed 
        ** by the flag byte. If the siblings are not leaves, then the entry 
        ** consists of the 7 byte key followed by the flag byte followed by a 
        ** 5-byte pointer to a child node. 
        **
        ** If the sibling is the left-hand sibling of pSlot (all keys are
        ** smaller than the divider cell), then the child-node associated
        ** with the new entry is the old right-child of the sibling. The
        ** old right-child of pSlot becomes the new right-child of the 
        ** sibling.
        **
        ** If the sibling is the right-hand sibling of pSlot, then the
        ** child node associated with the new entry is the old right-child
        ** of pSlot.
        */

        if( iSib>pParent->iEntry ){
          /* Sibling is the right-hand sibling. Insert the new entry as the 
          ** left-most in the slot (iEntry==0). If the siblings are not leaves,
          ** copy the right-child pointer of pSlot into the entry before
          ** inserting it.
          */

          if( iHeight>1 ){
            memcpy(&aInsert[8], &pSlot->aPayload[4], 5);
          }

          memmove(
              &aSiblingPayload[zipvfsFlNodeOffset(iHeight, 1)],
              &aSiblingPayload[zipvfsFlNodeOffset(iHeight, 0)],
              nSiblingEntry * zipvfsFlEntrySize(iHeight)
          );
          memcpy(
              &aSiblingPayload[zipvfsFlNodeOffset(iHeight, 0)], 
              aInsert, 
              zipvfsFlEntrySize(iHeight)
          );

        }else{
          /* Sibling is left-hand sibling. Insert the new entry as the 
          ** right-most in the slot. If the siblings are not leaves, then
          ** the right-child pointer of the entry is the old right-child
          ** pointer of the sibling itself. The right-child pointer of the
          ** sibling is replaced by the right-child pointer of pSlot.
          */
          if( iHeight>1 ){
            memcpy(&aInsert[8], &aSiblingPayload[4], 5);
            memcpy(&aSiblingPayload[4], &pSlot->aPayload[4], 5);
          }
          memcpy(
              &aSiblingPayload[zipvfsFlNodeOffset(iHeight, nSiblingEntry)], 
              aInsert, 
              zipvfsFlEntrySize(iHeight)
          );

          assert( iSib==nParentEntry-1 );
          memcpy(&pParent->aPayload[4], &aDiv[8], 5);
        }
        
        /* Increment the number of entries on the sibling page and write
        ** it back to the file. */ 
        zipvfsPutU16(&aSiblingPayload[2], nSiblingEntry+1);
        zipvfsStoreData(pZip, iSibOff + ZIPVFS_SLOT_HDRSIZE, 
            aSiblingPayload, nSiblingPayload, pRc);

        /* Delete cell pParent->iEntry. */
        zipvfsFree(pSlot->aPayload);
        pPath->nSlot--;
        zipvfsFlDeleteKey(pZip, pPath, pRc);

        /* Mark pSlot as unused. By the time control gets to this point, it
        ** is guaranteed that pPath will not be accessed again. So there is
        ** no need to update it. Just write the new flag value directly to
        ** the file.  */
        zipvfsFlMarkAsUnused(pZip, pSlot->iOff, nPayload, pRc);

      }else{
        /* The sibling page is full. Move the divider entry down to pSlot.
        ** Then take an entry from the sibling page to become the new
        ** divider. */
        if( iSib>pParent->iEntry ){       
          /* Right-hand sibling */
          if( iHeight>1 ){
            memcpy(&aInsert[8], &pSlot->aPayload[4], 5);
            memcpy(&pSlot->aPayload[4], 
                &aSiblingPayload[zipvfsFlNodeOffset(iHeight, 0)+8], 5);
          }

          memcpy(aDiv, &aSiblingPayload[zipvfsFlNodeOffset(iHeight, 0)], 8);
          memmove(
              &aSiblingPayload[zipvfsFlNodeOffset(iHeight, 0)],
              &aSiblingPayload[zipvfsFlNodeOffset(iHeight, 1)],
              (nSiblingEntry-1) * zipvfsFlEntrySize(iHeight)
          );
        }else{                            
          /* Left-hand sibling */
          if( iHeight>1 ){
            memcpy(&aInsert[8], &aSiblingPayload[4], 5);
            memcpy(
                &aSiblingPayload[4], 
                &aSiblingPayload[zipvfsFlNodeOffset(iHeight, nSiblingEntry-1)+8]
                ,5
            );
          }
          memcpy(aDiv, 
              &aSiblingPayload[zipvfsFlNodeOffset(iHeight, nSiblingEntry-1)]
              , 8
          );
        }
        zipvfsPutU16(&aSiblingPayload[2], nSiblingEntry-1);

        memcpy(
            &pSlot->aPayload[zipvfsFlNodeOffset(iHeight, 0)], 
            aInsert, zipvfsFlEntrySize(iHeight)
        );

        /* Write pSlot, the sibling and the parent page back to the file */
        zipvfsStoreData(pZip, pSlot->iOff+ZIPVFS_SLOT_HDRSIZE, 
            pSlot->aPayload, nPayload, pRc);
        zipvfsStoreData(pZip, iSibOff+ZIPVFS_SLOT_HDRSIZE, 
            aSiblingPayload, nSiblingPayload, pRc);
        zipvfsStoreData(pZip, pParent->iOff+ZIPVFS_SLOT_HDRSIZE, 
            pParent->aPayload, pParent->nPayload, pRc);
      }

      zipvfsFree(aSiblingPayload);
    }
  }else{
    int iTo = zipvfsFlNodeOffset(iHeight, pSlot->iEntry);
    int iFrom = zipvfsFlNodeOffset(iHeight, pSlot->iEntry+1);
    int nByte = zipvfsFlNodeOffset(iHeight, nEntry) - iFrom;

    /* Update slot content */
    if( nByte>0 ){
      memmove(&pSlot->aPayload[iTo], &pSlot->aPayload[iFrom], nByte);
    }
    zipvfsPutU16(&pSlot->aPayload[2], nEntry-1);

    /* Write the slot back to the file. */
    zipvfsStoreData(pZip, pSlot->iOff+ZIPVFS_SLOT_HDRSIZE, 
        pSlot->aPayload, nPayload, pRc);
  }
}

/*
** This function is called when a slot that is part of the b-tree is being
** removed from the free-list. The key corresponding to the slot has already
** been removed from the tree.
**
** This function modifies the tree so that the node content currently 
** stored on the slot at offset iOff is stored on one or more other 
** free-slots.
*/
static void zipvfsFlReplaceSlot(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOff,                       /* Offset of slot */
  int *pRc                        /* IN/OUT: Error code */
){
  int nPayload;
  int iHeight;
  int nEntry;
  u8 *aPayload = 0;

  zipvfsFlLoadSlot(pZip, iOff, &nPayload, &iHeight, &nEntry, &aPayload, pRc);
  if( *pRc==SQLITE_OK ){
    i64 iNewOff = 0;              /* Slot offset of new slot */
    int nSize = 0;                /* Payload size of new slot */
    u8 *aNew;                     /* Malloc'd buffer for new slot */
    int nNew;                     /* Number of entries on new slot */
    i64 iKey;                     /* First key on the slot at iOff. */
    ZipvfsFlPath path = {0,0,0,0};/* Path to iKey */
    int i;

    /* Seek to the first key that lies on the node at iOff. */
    iKey = zipvfsGetU64(&aPayload[zipvfsFlNodeOffset(iHeight, 0)]);
    zipvfsFlSeek(pZip, iKey & ZIPVFS_KEY_MASK, &path, 1, pRc);

    if( *pRc || path.aSlot[path.nSlot-1].iOff!=iOff ){
      zipvfsFlFreePath(&path);
      goto replace_done;
    }

    /* Allocate a new node. Then copy as many as possible of the entries
    ** stored on the original node to it, starting with the left-most.
    ** If the node is not a leaf node, then set the right-child of this
    ** new node to be the as the right-child of the old node - even if
    ** less than all of the entries were copied over.
    */
    zipvfsFlAllocOne(pZip, &path, 0, 0, 0, 0, &nSize, &iNewOff, pRc);

    /* Modify the pointer in the parent to point to the new node. */
    if( path.nSlot==1 ){
      pZip->hdr.iFreeSlot = iNewOff;
    }else{
      i64 iPtrOff;
      int nParentEntry;
      ZipvfsFlSlot *pParent = &path.aSlot[path.nSlot-2];
      zipvfsFlNodeReadHeader(pParent->aPayload, 0, &nParentEntry);
      if( nParentEntry==pParent->iEntry ){
        iPtrOff = pParent->iOff + ZIPVFS_SLOT_HDRSIZE + 4;
      }else{
        iPtrOff = pParent->iOff + ZIPVFS_SLOT_HDRSIZE 
          + zipvfsFlNodeOffset(2, pParent->iEntry) + 8;
      }
      zipvfsWriteU40(pZip, iPtrOff, iNewOff, pRc);
    }

    /* Copy data from the path to aPayload. This is because the path's
    ** cache was updated by zipvfsFlAllocOne(), but aPayload[] was not. */
    memcpy(aPayload, path.aSlot[path.nSlot-1].aPayload, nPayload);
    zipvfsFlFreePath(&path);
    aNew = zipvfsMalloc(nSize, pRc);
    if( *pRc ) goto replace_done;

    /* Figure out how many entries fit on the new node. */
    nNew = (nSize-zipvfsFlNodeOffset(iHeight, 0)) / zipvfsFlEntrySize(iHeight);
    if( nNew>nEntry ) nNew = nEntry;

    /* Populate a buffer with the new node image */
    memset(aNew, 0, nSize);                 /* Avoid valgrind warnings. */
    zipvfsPutU16(&aNew[0], iHeight);
    zipvfsPutU16(&aNew[2], nNew);
    if( iHeight>1 ){
      memcpy(&aNew[4], &aPayload[4], 5);
    }
    memcpy(&aNew[zipvfsFlNodeOffset(iHeight, 0)],
           &aPayload[zipvfsFlNodeOffset(iHeight, 0)],
           nNew * zipvfsFlEntrySize(iHeight)
    );

    /* Write the new node image back to the file. */
    zipvfsStoreData(pZip, iNewOff+ZIPVFS_SLOT_HDRSIZE, aNew, nSize, pRc);
    zipvfsFree(aNew);

    for(i=nNew; i<nEntry; i++){
      i64 iInsertKey;
      i64 iInsertOff = 0;
      u8 *aEntry;
      ZipvfsFlPath inspath = {0,0,0,0};

      aEntry = &aPayload[zipvfsFlNodeOffset(iHeight, i)];
      iInsertKey = zipvfsGetU64(aEntry);
      if( iHeight>1 ) iInsertOff = zipvfsGetU40(&aEntry[8]);

      zipvfsFlSeek(pZip, iInsertKey & ZIPVFS_KEY_MASK, &inspath, iHeight, pRc);
      zipvfsFlAllocate(pZip, 0, i, aPayload, nPayload, &inspath, pRc);

      /* Before inserting it, reread the key from aPayload[]. This is because
      ** the call to zipvfsFlAllocate() above may have allocated the free-slot
      ** the key refers to for use in the b-tree (and set the LSB of the key
      ** with aPayload[] at the same time). */
      assert( zipvfsGetU64(aEntry)==iInsertKey
           || zipvfsGetU64(aEntry)==(iInsertKey|ZIPVFS_KEYISUSED_MASK) );
      iInsertKey = zipvfsGetU64(aEntry);
      zipvfsFlInsertEntry(pZip, iInsertKey, iInsertOff, &inspath, pRc);
      zipvfsFlFreePath(&inspath);
    }
  }

 replace_done:
  zipvfsFree(aPayload);
}

/*
** This function does the work for functions zipvfsFreelistBestfit() and
** zipvfsFreelistRemove(). It extracts a single slot from the free-slot
** b-tree structure.
**
** If iExtractOff is zero, then this function operates as 
** zipvfsFreelistBestFit(), attempting to extract a slot as close as possible
** (but not smaller than) nByte bytes. Or, if bExact is true, a slot with
** a payload size of exactly nByte bytes.
**
** If iExtractOff is not zero, then this function will extract a slot with
** payload size nByte bytes and slot-offset iExtractOff only.
**
** If a slot is successfully extracted from the b-tree structure, *piOff
** and *pnSize are set to its offset and payload size before returning. 
** Otherwise, if no slot can be extracted, they are both set to 0 before
** returning.
*/
static void zipvfsFlExtractEntry(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  int bExact,                     /* True if an exact fit is required */
  int nByte,                      /* Required payload space */
  i64 iExtractOff,                /* Extract the slot at this offset */
  i64 *piOff,                     /* OUT: Slot offset, or 0 if no slot found */
  int *pnSize,                    /* OUT: Slot payload size */
  int *pRc                        /* IN/OUT: Error code */
){
  ZipvfsFlPath path = {0,0,0,0};  /* Path to entry to extract */
  int i;                          /* Iterator variable */
  i64 iSeek;                      /* Key value to search for */
  int bUsed = 0;                  /* Flag value for entry being removed */
  ZipvfsFlSlot *pLeaf;            /* Leaf node to remove entry from */

  zipvfsFreelistIntegrity(pZip, pRc);
  *piOff = 0;
  *pnSize = 0;
  if( pZip->hdr.iFreeSlot==0 ) return;
  iSeek = zipvfsFlEncodeKey(nByte, iExtractOff ? (iExtractOff-1) : 0);

  /* It is guaranteed that iSeek does not exist in the b-tree (since there
  ** are no keys corresponding to offset==0). This call therefore returns a
  ** path that reaches all the way to the position on a leaf node that the
  ** key nByte/0 would be inserted at.
  */
  zipvfsFlSeek(pZip, iSeek, &path, 1, pRc);
  if( *pRc ){
    zipvfsFlFreePath(&path);
    return;
  }

  pLeaf = &path.aSlot[path.nSlot-1];
  for(i=path.nSlot-1; i>=0; i--){
    ZipvfsFlSlot *pSlot = &path.aSlot[i];
    int iHeight;                  /* Value of "height" field node pSlot */
    int nEntry;                   /* Number of entries stored on pSlot */

    zipvfsFlNodeReadHeader(pSlot->aPayload, &iHeight, &nEntry);
    if( nEntry>pSlot->iEntry ){
      int iEntryOff = zipvfsFlNodeOffset(iHeight, pSlot->iEntry);
      i64 iFound;                 /* Key value for found slot */
      i64 iFoundOff;              /* Offset of free-slot to reuse */
      int nFoundSize;             /* Payload size of free-slot to reuse */

      iFound = zipvfsGetU64(&pSlot->aPayload[iEntryOff]);
      zipvfsFlDecodeKey(iFound, &nFoundSize, &iFoundOff, &bUsed);
      assert( nFoundSize>=nByte );
      if( iExtractOff && iFoundOff!=iExtractOff ) continue;
      if( nFoundSize>(nByte+pZip->nMaxFrag) || (bExact && nFoundSize!=nByte) ){
        /* There is no suitable slot. */
        zipvfsFlFreePath(&path);
        return;
      }

      /* Found a suitable free-slot to reuse */
      *piOff = iFoundOff;
      *pnSize = nFoundSize;

      /* If the slot is not on a leaf node, exchange it with one that is. */
      if( pSlot!=pLeaf ){
        u8 *aEntry = &pLeaf->aPayload[zipvfsFlNodeOffset(1, pLeaf->iEntry-1)];
        memcpy(&pSlot->aPayload[iEntryOff], aEntry, 8);
        zipvfsStoreData(pZip, pSlot->iOff + ZIPVFS_SLOT_HDRSIZE + iEntryOff, 
            aEntry, 8, pRc);
        pLeaf->iEntry--;
      }
      break;
    }
  }

  if( i>=0 ){
    zipvfsFlDeleteKey(pZip, &path, pRc);
    if( pZip->hdr.iFreeSlot && bUsed ){
      zipvfsFlReplaceSlot(pZip, *piOff, pRc);
    }
    pZip->hdr.nFreeSlot--;
    pZip->hdr.nFreeByte -= *pnSize;
  }
  zipvfsFreelistIntegrity(pZip, pRc);
  zipvfsFlFreePath(&path);
}

/*
** Search the free-slot b-tree for a free-slot that can be used to store a
** record with payload size nByte. If such a free-slot can be found, remove
** it from the free-slot b-tree and write its offset and size to *piOff
** and *pnSize, respectively, before returning.
**
** If the bExact argument is non-zero, then only a free-slot with a payload
** size of exactly nByte bytes will be considered a match. If it is non-zero,
** then this function endeavours to find the smallest free-slot with a
** payload larger than nByte bytes and smaller than nByte+N bytes, where N
** is the value configured by ZIPVFS_CTRL_MAXFRAG.
*/
static void zipvfsFreelistBestfit(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  int bExact,                     /* True if an exact fit is required */
  int nByte,                      /* Required payload space */
  i64 *piOff,                     /* OUT: Slot offset, or 0 if no slot found */
  int *pnSize,                    /* OUT: Slot payload size */
  int *pRc                        /* IN/OUT: Error code */
){
  zipvfsFlExtractEntry(pZip, bExact, nByte, 0, piOff, pnSize, pRc);
}

/*
** Remove a specific entry from the free-slot list structure. The error code
** is set to SQLITE_CORRUPT if no entry exists with offset iOff and payload
** size nByte.
*/
static void zipvfsFreelistRemove(
  zipvfs_file *pZip,              /* Zip file handle */
  i64 iOff,                       /* Offset of slot to remove from FSL */
  int nByte,                      /* Payload size of slot to remove from FSL */
  int *pRc                        /* IN/OUT: Error code */
){
  i64 iOffOut = 0;
  int nSize = 0;
  zipvfsFlExtractEntry(pZip, 1, nByte, iOff, &iOffOut, &nSize, pRc);
  if( *pRc==SQLITE_OK && iOffOut!=iOff ){
    *pRc = ZIPVFS_CORRUPT;
  }
  assert( *pRc || nSize==nByte );
}

/*
** Check that the sub-tree of the free-list b-tree headed by the node with
** slot offset iOff is internally consistent. Also check that all keys in 
** the sub-tree are greater than iGreaterThan. If parameter iLessThan is
** non-zero, also check that all keys in the sub-tree are smaller than 
** iLessThan.
**
** If any of these checks fail, set *pRc to SQLITE_CORRUPT before returning.
** Or, if some other error occurs, set *pRc to an appropriate error code.
** If *pRc is already other than SQLITE_OK when this function is called, it
** is a no-op.
**
** If no error occurs, increment the value stored in *pnNode by the total
** number of nodes in the sub-tree, including the root node. Similarly,
** increment *pnFlag by the number of entries in the sub-tree that have
** the in-use flag set.
*/
static void zipvfsFlIntegrity(
  zipvfs_file *pZip,              /* Zipvfs file handle */
  i64 iOff,                       /* Slot-offset of sub-tree root */
  i64 iGreaterThan,               /* All keys must be greater than this value */
  i64 iLessThan,                  /* All keys must be less than this value */
  int *pnNode,                    /* OUT: Incr by # of b-tree nodes traversed */
  int *pnFlag,                    /* OUT: Incr by # of set flags seen */
  int *pRc                        /* IN/OUT: Error code */
){
  int nPayload;                   /* Payload size for this node */
  int iHeight;                    /* Value of "height" field for this node */
  int nEntry;                     /* Number of entries stored on this node */
  u8 *aPayload;                   /* Malloc'd buffer containing node payload */

  assert( pnNode && pnFlag );

  zipvfsFlLoadSlot(pZip, iOff, &nPayload, &iHeight, &nEntry, &aPayload, pRc);
  if( *pRc==SQLITE_OK ){
    int i;
    i64 iPrev;
    int isUsed = 0;
    int ret;

    /* Check that the b-tree entry that corresponds to this slot is present
    ** and the flag is set to mark it as used. */
    (*pnNode)++;
    ret = zipvfsFreelistTest(pZip, iOff, nPayload, &isUsed, pRc);
    if( *pRc==SQLITE_OK && (ret==0 || isUsed==0) ){
      *pRc = ZIPVFS_CORRUPT;
    }

    /* Check that all the keys on this page are smaller than iLessThan. 
    ** And that they are stored in ascending order within this page. */
    iPrev = iGreaterThan;
    for(i=0; i<nEntry && *pRc==SQLITE_OK; i++){
      int nKeyPayload;            /* Payload component of iKey */
      i64 iKeyOff;                /* Offset component of iKey */
      int bUsed;                  /* Used-flag component of iKey */
      i64 iKey = zipvfsGetU64(&aPayload[zipvfsFlNodeOffset(iHeight, i)]);

      zipvfsFlDecodeKey(iKey, &nKeyPayload, &iKeyOff, &bUsed);
      if( bUsed ) (*pnFlag)++;

      if( nKeyPayload<ZIPVFS_MIN_PAYLOAD ) *pRc = ZIPVFS_CORRUPT;
      if( *pRc==SQLITE_OK && iPrev!=0 && iKey<=iPrev ){
        *pRc = ZIPVFS_CORRUPT;
      }
      if( *pRc==SQLITE_OK && iLessThan!=0 && iKey>=iLessThan ){
        *pRc = ZIPVFS_CORRUPT;
      }
      

      if( iHeight>1 ){
        i64 iChild = zipvfsGetU40(&aPayload[zipvfsFlNodeOffset(iHeight, i)+8]);
        zipvfsFlIntegrity(pZip, iChild, iPrev, iKey, pnNode, pnFlag, pRc);
      }
      iPrev = iKey;
    }

    if( iHeight>1 ){
      i64 iChild = zipvfsGetU40(&aPayload[4]);
      zipvfsFlIntegrity(pZip, iChild, iPrev, iLessThan, pnNode, pnFlag, pRc);
    }
  }
  zipvfsFree(aPayload);
}

/*
** Unless the zipvfs_file.doIntegrityCheck flag is true, this function is
** a no-op. If it is true, then check the that the free-list b-tree is
** internally consistent. If not, set *pRc to SQLITE_CORRUPT before returning.
*/
static void zipvfsFreelistIntegrity(
  zipvfs_file *pZip,              /* Zip file handle */
  int *pRc                        /* IN/OUT: Error code */
){
  if( pZip->hdr.iFreeSlot && pZip->doIntegrityCheck && *pRc==SQLITE_OK ){
    int nNode = 0;
    int nFlag = 0;
    zipvfsFlIntegrity(pZip, pZip->hdr.iFreeSlot, 0, 0, &nNode, &nFlag, pRc);
    if( *pRc==SQLITE_OK && nNode!=nFlag ){
      *pRc = ZIPVFS_CORRUPT;
    }
  }
}

/*
** The xShmMap, xShmLock, xShmBarrier and xShmUnmap functions. These are 
** only used in pass-through mode.
*/
static int zipvfsShmMap(
  sqlite3_file *pFile, 
  int a, int b, int c, void volatile **d
){
  const sqlite3_io_methods *pMethods = SYSFILE(pFile)->pMethods;
  assert( ((zipvfs_file *)pFile)->pPager==0 );
  return pMethods->xShmMap(SYSFILE(pFile), a, b, c, d);
}
static int zipvfsShmLock(sqlite3_file *pFile, int a, int b, int c){
  const sqlite3_io_methods *pMethods = SYSFILE(pFile)->pMethods;
  assert( ((zipvfs_file *)pFile)->pPager==0 );
  return pMethods->xShmLock(SYSFILE(pFile), a, b, c);
}
static void zipvfsShmBarrier(sqlite3_file *pFile){
  const sqlite3_io_methods *pMethods = SYSFILE(pFile)->pMethods;
  assert( ((zipvfs_file *)pFile)->pPager==0 );
  pMethods->xShmBarrier(SYSFILE(pFile));
}
static int zipvfsShmUnmap(sqlite3_file *pFile, int a){
  const sqlite3_io_methods *pMethods = SYSFILE(pFile)->pMethods;
  assert( ((zipvfs_file *)pFile)->pPager==0 );
  return pMethods->xShmUnmap(SYSFILE(pFile), a);
}

#if defined(SQLITE_TEST)

/*
** Append the output of a printf() style formatting to an existing string.
*/
static void zipvfsAppendf(
  int *pRc,                       /* IN/OUT: Error code */
  char **pz,                      /* IN/OUT: Pointer to string buffer */
  const char *zFormat,            /* Printf format string to append */
  ...                             /* Arguments for printf format string */
){
  if( *pRc==SQLITE_OK ){
    va_list ap;
    char *z;
    va_start(ap, zFormat);
    z = sqlite3_vmprintf(zFormat, ap);
    if( z && *pz ){
      char *z2 = sqlite3_mprintf("%s%s", *pz, z);
      sqlite3_free(z);
      z = z2;
    }
    if( z==0 ) *pRc = SQLITE_NOMEM;
    sqlite3_free(*pz);
    *pz = z;
  }
}

/*
** If *pRc is initially SQLITE_OK and no error occurs within this function,
** return a string containing a human-readable representation of the contents
** of the b-tree node stored in buffer aPayload. The memory for the returned
** string is obtained from sqlite3_malloc(). It is the responsibility of the
** caller to eventually call sqlite3_free() to free the buffer.
**
** If *pRc is other than SQLITE_OK when this function is called, NULL is
** returned. Otherwise, if an OOM condition is encountered, NULL is returned
** and *pRc set to SQLITE_IOERR_NOMEM.
*/
static char *zipvfsFlPrintNode(u8 *aPayload, int nPayload, int *pRc){
  int i;
  int iHeight;
  int nEntry;
  int rc = *pRc;
  char *z = 0;

  if( rc!=SQLITE_OK ) return 0;
  zipvfsFlNodeReadHeader(aPayload, &iHeight, &nEntry);
  zipvfsAppendf(&rc, &z, "payload=%d, ", (int)nPayload);
  zipvfsAppendf(&rc, &z, "height=%d, ", (int)iHeight);
  zipvfsAppendf(&rc, &z, "entries=%d", (int)nEntry);
  if( iHeight>1 ){
    zipvfsAppendf(&rc, &z, ", right-child=%d", (int)zipvfsGetU40(&aPayload[4]));
  }
  zipvfsAppendf(&rc, &z, " (");

  for(i=0; i<nEntry; i++){
    int iEntryOff = zipvfsFlNodeOffset(iHeight, i);
    int i1;
    i64 i2;
    int i3;
    i64 i4 = 0;

    zipvfsFlDecodeKey(zipvfsGetU64(&aPayload[iEntryOff]), &i1, &i2, &i3);
    if( iHeight>1 ) i4 = zipvfsGetU40(&aPayload[iEntryOff+8]);

    if( i ) zipvfsAppendf(&rc, &z, " ");
    zipvfsAppendf(&rc, &z, "%d/%d/%d+%d", i1, (int)i2, i3, (int)i4);
  }
  zipvfsAppendf(&rc, &z, ")");

  *pRc = rc;
  return z;
}

/*
** If *pRc is initially SQLITE_OK and no error occurs within this function,
** return a string containing a human-readable representation of the contents
** of the b-tree node stored in the slot at slot offset iOff. Memory for the
** string is obtained from sqlite3_malloc(). It is the responsibility of the
** caller to eventually call sqlite3_free() to free the buffer.
**
** If *pRc is other than SQLITE_OK when this function is called, NULL is
** returned. Otherwise, if an OOM condition is encountered, NULL is returned
** and *pRc set to SQLITE_IOERR_NOMEM.
*/
static char *zipvfsFlPrintSlot(zipvfs_file *pZip, i64 iOff, int *pRc){
  u8 *aPayload;                   /* Slot payload buffer */
  int nPayload;
  char *z;                        /* String to return */

  zipvfsFlLoadSlot(pZip, iOff, &nPayload, 0, 0, &aPayload, pRc);
  z = zipvfsFlPrintNode(aPayload, nPayload, pRc);
  zipvfsFree(aPayload);
  return z;
}

/*
** Iterate through the entries stored in the sub-tree of the free-list b-tree
** headed by the node stored in the free-slot at slot offset iOff. Invoke
** the callback specified by structure *p for each entry.
**
** This function is used by the ZIPVFS_CTRL_STRUCTURE test file-control.
*/
static void zipvfsFreelistFlIter(
  zipvfs_file *pZip, 
  ZipvfsStructureCb *p,
  i64 iOff,
  int *pRc
){
  u8 *aPayload;
  int nPayload;
  int iHeight;
  int nEntry;
  int i;

  zipvfsFlLoadSlot(pZip, iOff, &nPayload, &iHeight, &nEntry, &aPayload, pRc);

  if( *pRc==SQLITE_OK ){
    for(i=0; i<nEntry; i++){
      i64 iOff;                   /* Slot offset */
      int nByte;                  /* Slot payload size */
      int bUsed;                  /* True if slot is part of b-tree */
      char *z = 0;

      u8 *a = &aPayload[zipvfsFlNodeOffset(iHeight, i)];
      if( iHeight>1 ){
        zipvfsFreelistFlIter(pZip, p, zipvfsGetU40(&a[8]), pRc);
      }
      zipvfsFlDecodeKey(zipvfsGetU64(a), &nByte, &iOff, &bUsed);

      if( bUsed ){
        z = zipvfsFlPrintSlot(pZip, iOff, pRc);
      }
      p->x(p, 0, iOff, nByte, bUsed, z);
      sqlite3_free(z);
    }
    if( iHeight>1 ){
      zipvfsFreelistFlIter(pZip, p, zipvfsGetU40(&aPayload[4]), pRc);
    }
  }

  zipvfsFree(aPayload);
}

/*
** Iterate through the entries stored in the sub-tree of the free-list b-tree
** headed by the node stored in the free-slot at slot offset iOff. Invoke
** the callback specified by structure *p for each entry.
**
** This function is used by the ZIPVFS_CTRL_STRUCTURE test file-control.
*/
static void zipvfsFreelistIterate(
  zipvfs_file *pZip, 
  ZipvfsStructureCb *p,
  int *pRc
){
  if( pZip->hdr.iFreeSlot ){
    zipvfsFreelistFlIter(pZip, p, pZip->hdr.iFreeSlot, pRc);
  }
}
#endif /* defined(SQLITE_TEST) */

#endif /* defined(SQLITE_ENABLE_ZIPVFS) */

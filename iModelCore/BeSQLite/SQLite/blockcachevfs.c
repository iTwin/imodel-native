/*
** 2019-11-25
**
******************************************************************************
**
** Implementation of the client VFS for the blockcachevfs system.
*/
#include "bcv_socket.h"
#include "sqlite3.h"
#include <string.h>
#include <assert.h>
#include <stdio.h> 

#include "blockcachevfs.h"

typedef unsigned char u8;
typedef unsigned int u32;

typedef struct BlockcacheFile BlockcacheFile;
typedef struct BlockcacheMessage BlockcacheMessage;

/* 
** Each file opened using this VFS is represented by an instance of this
** structure. If the opened file is a database file that uses blockcachevfs,
** then fdDaemon is set to 0 or greater and the other fields of this 
** structure populated. Otherwise, calls are passed through to the underlying
** VFS. In this case, the "real" file is stored immediately after this
** structure in memory. It can be accessed using the ORIGFILE() macro.
**
** fdDaemon:
**   For a blockcachevfs database file, the fd of a localhost tcl socket
**   connected to the daemon process. Or -1 for other files.
**
** bDaemon:
**   True if the URI parameter "daemon=1" was specified when opening the
**   file. This is used by the blockcachevfs daemon when it opens a 
**   database file itself in order to run a checkpoint.
**
** szBlk:
**   Block size in bytes for the blocks in pBlkFile.
**
** pBlkFile:
**   File descriptor opened on block cache file.
**
** lockMask:
**   Mask of xShmLock() locks currently held. No distinction is made between
**   EXCLUSIVE and SHARED locks. If a lock is held on a locking-slot, then
**   the corresponding bit of lockMask is set.
**
** bSendCkptOnClose:
**   True to send a CKPT_DONE message to the daemon when this file is 
**   closed (i.e. from within the xClose() method).
**
** aBlkArray/nBlkByte:
**   If it is not NULL, aBlkArray[] is an array nBlkByte bytes in size. It
**   contains one 4-byte entry (a 32-bit big-endian unsigned integer) for
**   each block in the database indicating where in pBlkFile, if anywhere,
**   the data for the block is stored. Blocks within pBlkFile are numbered
**   starting from 1 - a value of 0 indicates that the data for the block
**   in question is not currently accessible.
**
**   aBlkArray is only non-NULL (and nBlkByte non-zero) within a transaction or
**   when the client is doing a checkpoint.
**
** aUsed/nUsed:
**   aUsed[] is an array nUsed bytes in size. It contains a bitmask 
**   indicating which blocks have been used by the current transaction. The
**   least-significant-bit in the first byte of the array corresponds to
**   the first block in the database. The most-significant-bit in the
**   second byte of the database corresponds to the 16th block of the
**   database. And so on.
**
**   aUsed is only non-NULL (and nUsed non-zero) within a transaction.
**
** aWritten/nWritten:
**   aWritten/nWritten are a bitmask in the same format as aUsed/nUsed. They
**   are only non-NULL/non-zero within a checkpoint. Bits are set for
**   each block of the database that has been written by the current
**   checkpoint operation.
*/
struct BlockcacheFile {
  sqlite3_file base;              /* IO methods */
  BCV_SOCKET_TYPE fdDaemon;       /* Localhost connection to daemon (or -1) */
  int bDaemon;                    /* True if daemon=1 was specified */
  char *zAccount;                 /* Account name used by connected daemon */
  char *zContainer;               /* Container name */
  sqlite3_int64 szBlk;            /* Block size */
  char *pFree;                    /* Free when closing file */
  sqlite3_file *pBlkFile;         /* File descriptor open on block-cache file */

  u32 lockMask;                   /* Mask of current shm-locks */
  int bSendCkptOnClose;           /* True to send CKPT_DONE from xClose() */
  u8 *aBlkArray;                  /* Block map */
  int nBlkByte;                   /* Size of block map in bytes */
  u8 *aUsed;                      /* Bitmask indicating blocks used */
  int nUsed;                      /* Size of aUsed in (u8) */

  int iPrevBlk;                   /* Within checkpoints, prev. block written */
  int iPrevLoc;                   /* Location of iPrevBlk withing cache-file */

  sqlite3_int64 nMsg;             /* Message exchanges with daemon */
  sqlite3_int64 msMsgWait;        /* Total us spent waiting */
};

/* Valid message types */
#define BCV_MESSAGE_LOGIN          'L'
#define BCV_MESSAGE_LOGIN_REPLY    'l'
#define BCV_MESSAGE_REQUEST        'R'
#define BCV_MESSAGE_REQUEST_REPLY  'r'
#define BCV_MESSAGE_DONE           'D'
#define BCV_MESSAGE_ERROR_REPLY    'e'

#define BCV_MESSAGE_WREQUEST       'Q'
#define BCV_MESSAGE_WREQUEST_REPLY 'q'
#define BCV_MESSAGE_WDONE          'C'

#define BCV_MESSAGE_QUERY          'Z'
#define BCV_MESSAGE_QUERY_REPLY    'z'

#define BCV_CACHEFILE_NAME "cachefile.bcv"

#define BCV_SAS_PARAM "bcv_sas"
#define BCV_PORTNUMBER_FILE "portnumber.bcv"

#define HTTP_AUTH_ERROR 403

/* 
** The following structure represents the deserialized version of a message
** passed between the daemon and the current process.
**
** iVal:
**   Used by LOGIN_REPLY (the block-size for the daemon), and REQUEST (the
**   requested block).
**     
** aData/nData:
**   Used by messages that carry a payload - LOGIN, REQUEST, REQUEST_REPLY, 
**   DONE and WRITTEN. aData[] is an array nData bytes in size.
**
*/
struct BlockcacheMessage {
  int eType;                      /* BCV_MESSAGE_* value */
  int iVal;
  int nData;
  u8 *aData;
  void *pFree;                    /* Use sqlite3_free() on this */
};

/* Access to a lower-level VFS. Argument is the blockcachevfs object */
#define ORIGVFS(p) ((sqlite3_vfs*)((p)->pAppData))

/* Access to underlying file-handle */
#define ORIGFILE(p) ((sqlite3_file*)(&((BlockcacheFile*)p)[1]))


/*
** Methods for BlockcacheFile
*/
static int bcvClose(sqlite3_file*);
static int bcvRead(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
static int bcvWrite(sqlite3_file*,const void*,int iAmt, sqlite3_int64 iOfst);
static int bcvTruncate(sqlite3_file*, sqlite3_int64 size);
static int bcvSync(sqlite3_file*, int flags);
static int bcvFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static int bcvLock(sqlite3_file*, int);
static int bcvUnlock(sqlite3_file*, int);
static int bcvCheckReservedLock(sqlite3_file*, int *pResOut);
static int bcvFileControl(sqlite3_file*, int op, void *pArg);
static int bcvSectorSize(sqlite3_file*);
static int bcvDeviceCharacteristics(sqlite3_file*);
static int bcvShmMap(sqlite3_file*, int iPg, int pgsz, int, void volatile**);
static int bcvShmLock(sqlite3_file*, int offset, int n, int flags);
static void bcvShmBarrier(sqlite3_file*);
static int bcvShmUnmap(sqlite3_file*, int deleteFlag);
static int bcvFetch(sqlite3_file*, sqlite3_int64 iOfst, int iAmt, void **pp);
static int bcvUnfetch(sqlite3_file*, sqlite3_int64 iOfst, void *p);

/*
** Methods for VFS "blockcachevfs".
*/
static int bcvOpen(sqlite3_vfs*, const char *, sqlite3_file*, int , int *);
static int bcvDelete(sqlite3_vfs*, const char *zName, int syncDir);
static int bcvAccess(sqlite3_vfs*, const char *zName, int flags, int *);
static int bcvFullPathname(sqlite3_vfs*, const char *zName, int, char *zOut);
static void *bcvDlOpen(sqlite3_vfs*, const char *zFilename);
static void bcvDlError(sqlite3_vfs*, int nByte, char *zErrMsg);
static void (*bcvDlSym(sqlite3_vfs *pVfs, void *p, const char*zSym))(void);
static void bcvDlClose(sqlite3_vfs*, void*);
static int bcvRandomness(sqlite3_vfs*, int nByte, char *zOut);
static int bcvSleep(sqlite3_vfs*, int microseconds);
static int bcvCurrentTime(sqlite3_vfs*, double*);
static int bcvGetLastError(sqlite3_vfs*, int, char *);
static int bcvCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

struct bcv_global_t {
  void *pSasCtx;                  /* First argument for xSas */
  int(*xSas)(                     /* SAS token factory */
      void *pCtx, 
      const char *zStorage, 
      const char *zAccount, 
      const char *zContainer,
      char **pzSasToken,
      int *pbReadonly
  );
};

static struct bcv_global_t bcv_global = {
  0,
  0
};

static sqlite3_vfs bcv_vfs = {
  2,                              /* iVersion */
  0,                              /* szOsFile (set when registered) */
  1024,                           /* mxPathname */
  0,                              /* pNext */
  "blockcachevfs",                /* zName */
  0,                              /* pAppData (set when registered) */ 
  bcvOpen,                        /* xOpen */
  bcvDelete,                      /* xDelete */
  bcvAccess,                      /* xAccess */
  bcvFullPathname,                /* xFullPathname */
  bcvDlOpen,                      /* xDlOpen */
  bcvDlError,                     /* xDlError */
  bcvDlSym,                       /* xDlSym */
  bcvDlClose,                     /* xDlClose */
  bcvRandomness,                  /* xRandomness */
  bcvSleep,                       /* xSleep */
  bcvCurrentTime,                 /* xCurrentTime */
  bcvGetLastError,                /* xGetLastError */
  bcvCurrentTimeInt64             /* xCurrentTimeInt64 */
};

static const sqlite3_io_methods bcv_io_methods = {
  3,                              /* iVersion */
  bcvClose,                       /* xClose */
  bcvRead,                        /* xRead */
  bcvWrite,                       /* xWrite */
  bcvTruncate,                    /* xTruncate */
  bcvSync,                        /* xSync */
  bcvFileSize,                    /* xFileSize */
  bcvLock,                        /* xLock */
  bcvUnlock,                      /* xUnlock */
  bcvCheckReservedLock,           /* xCheckReservedLock */
  bcvFileControl,                 /* xFileControl */
  bcvSectorSize,                  /* xSectorSize */
  bcvDeviceCharacteristics,       /* xDeviceCharacteristics */
  bcvShmMap,                      /* xShmMap */
  bcvShmLock,                     /* xShmLock */
  bcvShmBarrier,                  /* xShmBarrier */
  bcvShmUnmap,                    /* xShmUnmap */
  bcvFetch,                       /* xFetch */
  bcvUnfetch                      /* xUnfetch */
};


int bcv_socket_is_valid(BCV_SOCKET_TYPE s){
#ifdef __WIN32__
  return (s!=INVALID_SOCKET);
#else
  return (s>=0);
#endif
}

void bcv_close_socket(BCV_SOCKET_TYPE s){
#ifdef __WIN32__
  shutdown(s, SD_BOTH);
  closesocket(s);
#else
  shutdown(s, SHUT_RDWR);
  close(s);
#endif
}

void bcv_socket_init(){
#ifdef __WIN32__
  WORD v;
  WSADATA wsa;
  v = MAKEWORD(2,2);
  WSAStartup(v, &wsa);
#endif
}

/*
** VFS and IO methods use "return BCV_RETURN(rc);" instead of "return rc;"
** to make it easier to figure out where errors are coming from when using
** a debugger.
*/
#define BCV_RETURN(rc) bcv_return(rc)
static int bcv_return(int rc){
  return rc;
}

/* 
** Primitives to read and write 32-bit big-endian integers. 
*/
static void bcvPutU32(u8 *a, u32 iVal){
  a[0] = (iVal>>24) & 0xFF;
  a[1] = (iVal>>16) & 0xFF;
  a[2] = (iVal>> 8) & 0xFF;
  a[3] = (iVal>> 0) & 0xFF;
}
static u32 bcvGetU32(u8 *a){
  return (a[0] << 24) + (a[1] << 16) + (a[2] << 8) + (a[3] << 0);
}

/*
** Send the message in the second argument to the daemon. 
*/
static int bcvSendMessage(BCV_SOCKET_TYPE fd, BlockcacheMessage *pMsg){
  int rc = SQLITE_OK;
  u8 *aMsg = 0;
  int nMsg = 5;
  int res;

  assert( pMsg->eType==BCV_MESSAGE_LOGIN 
       || pMsg->eType==BCV_MESSAGE_REQUEST 
       || pMsg->eType==BCV_MESSAGE_DONE 
       || pMsg->eType==BCV_MESSAGE_WDONE 
       || pMsg->eType==BCV_MESSAGE_WREQUEST 
       || pMsg->eType==BCV_MESSAGE_QUERY 
  );

  nMsg += pMsg->nData;
  if( pMsg->eType==BCV_MESSAGE_REQUEST 
   || pMsg->eType==BCV_MESSAGE_QUERY 
   || pMsg->eType==BCV_MESSAGE_LOGIN 
  ){
    nMsg += sizeof(u32);
  }
  aMsg = sqlite3_malloc(nMsg);
  if( aMsg==0 ) return SQLITE_NOMEM;

  aMsg[0] = (u8)pMsg->eType;
  bcvPutU32(&aMsg[1], nMsg);
  if( pMsg->eType==BCV_MESSAGE_REQUEST 
   || pMsg->eType==BCV_MESSAGE_QUERY 
   || pMsg->eType==BCV_MESSAGE_LOGIN 
  ){
    bcvPutU32(&aMsg[5], pMsg->iVal);
  }
  if( pMsg->nData ){
    memcpy(&aMsg[nMsg-pMsg->nData], pMsg->aData, pMsg->nData);
  }

  res = send(fd, aMsg, nMsg, 0);
  if( res!=nMsg ){
    rc = SQLITE_IOERR;
  }

  sqlite3_free(aMsg);
  return rc;
}


/*
** Read a message from the daemon process into the structure supplied
** as the second argument. Return SQLITE_OK if successful, or an SQLite
** error code if an error has occured.
*/
static int bcvReceiveMessage(BCV_SOCKET_TYPE fd, BlockcacheMessage *pMsg){
  int rc = SQLITE_OK;
  int nRead;
  u8 aMsg[5];

  memset(pMsg, 0, sizeof(BlockcacheMessage));
  nRead = recv(fd, aMsg, 5, 0);
  if( nRead!=5 ){
    rc = SQLITE_IOERR;
  }else{
    int nBody = (int)bcvGetU32(&aMsg[1]) - 5;
    u8 *aBody = 0;

    pMsg->eType = (int)aMsg[0];
    if( nBody>0 ){
      aBody = sqlite3_malloc(nBody+1);
      if( aBody==0 ){
        rc = SQLITE_IOERR_NOMEM;
      }else{
        aBody[nBody] = 0x00;
        nRead = (int)recv(fd, aBody, nBody, 0);
        if( nRead!=nBody ){
          rc = SQLITE_IOERR;
        }
      }
    }

    if( rc==SQLITE_OK ){
      switch( aMsg[0] ){
        case BCV_MESSAGE_ERROR_REPLY:
        case BCV_MESSAGE_LOGIN_REPLY:
        case BCV_MESSAGE_QUERY_REPLY: 
          pMsg->iVal = (int)bcvGetU32(&aBody[0]);
          pMsg->nData = nBody-4;
          pMsg->aData = &aBody[4];
          pMsg->pFree = (void*)aBody;
          aBody = 0;
          break;

        case BCV_MESSAGE_WREQUEST_REPLY: 
        case BCV_MESSAGE_REQUEST_REPLY: {
          pMsg->nData = nBody;
          pMsg->aData = aBody;
          pMsg->pFree = (void*)aBody;
          aBody = 0;
          break;
        }
#if 0
        case BCV_MESSAGE_ERROR_REPLY: {
          sqlite3_log(SQLITE_IOERR, "daemon error: %.*s", nBody, aBody);
          sqlite3_free(aBody);
          return SQLITE_IOERR;
        }
#endif
        default:
          rc = SQLITE_IOERR;
          break;
      }
    }

    sqlite3_free(aBody);
  }

  if( rc!=SQLITE_OK ){
    sqlite3_log(rc, "error receiving message from blockcachevfsd");
  }

  return rc;
}

/*
**
*/
static int bcvExchangeMessage(
  BlockcacheFile *p, 
  BlockcacheMessage *pSend,
  BlockcacheMessage *pReply
){
  int rc = SQLITE_OK;
  int ii;
  sqlite3_vfs *pVfs = ORIGVFS(&bcv_vfs);
  sqlite3_int64 tm1;
  sqlite3_int64 tm2;

  pVfs->xCurrentTimeInt64(pVfs, &tm1);
  for(ii=0; ii<5 && rc==SQLITE_OK; ii++){
    rc = bcvSendMessage(p->fdDaemon, pSend);
    if( rc==SQLITE_OK ){
      rc = bcvReceiveMessage(p->fdDaemon, pReply);
    }
    if( rc==SQLITE_OK 
     && pReply->eType==BCV_MESSAGE_ERROR_REPLY 
     && pReply->iVal==HTTP_AUTH_ERROR 
     && bcv_global.xSas
    ){
      BlockcacheMessage query;
      BlockcacheMessage reply;
      char *zToken = 0;
      int bReadonly = 0;
      rc = bcv_global.xSas(
          bcv_global.pSasCtx, "azure", 
          p->zAccount, p->zContainer, &zToken, &bReadonly
      );
      if( zToken ){
        memset(&query, 0, sizeof(BlockcacheMessage));
        query.aData = (u8*)sqlite3_mprintf("bcv_%s=%s?%s", 
            bReadonly?"readonly":"attach", p->zContainer, zToken
        );
        query.nData = strlen((const char*)query.aData);
        query.eType = BCV_MESSAGE_QUERY;
        rc = bcvSendMessage(p->fdDaemon, &query);
        if( rc==SQLITE_OK ){
          rc = bcvReceiveMessage(p->fdDaemon, &reply);
        }
        if( rc==SQLITE_OK ){
          sqlite3_free(reply.pFree);
        }
      }else if( rc==SQLITE_OK ){
        rc = SQLITE_IOERR;
      }
    }else{
      break;
    }
  }
  pVfs->xCurrentTimeInt64(pVfs, &tm2);
  p->msMsgWait += (tm2-tm1);
  p->nMsg++;

  if( rc==SQLITE_OK && pReply->eType==BCV_MESSAGE_ERROR_REPLY ){
    sqlite3_log(SQLITE_ERROR, "error from blockcachevfsd: rc=%d msg=%.*s",
        pReply->iVal, pReply->nData, pReply->aData
    );
    sqlite3_free(pReply->pFree);
    rc = SQLITE_IOERR;
  }

  return rc;
}
  

/*
** Close a BlockcacheFile.
*/
static int bcvClose(sqlite3_file *pFile){
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  sqlite3_file *pOrig = ORIGFILE(pFile);
  if( p->bSendCkptOnClose ){
    BlockcacheMessage ckpt;
    memset(&ckpt, 0, sizeof(ckpt));
    ckpt.eType = BCV_MESSAGE_WDONE;
    bcvSendMessage(p->fdDaemon, &ckpt);
  }
  if( pOrig->pMethods ){
    pOrig->pMethods->xClose(pOrig);
  }
  if( p->pBlkFile && p->pBlkFile->pMethods ){
    p->pBlkFile->pMethods->xClose(p->pBlkFile);
  }
  sqlite3_free(p->pFree);
  p->pFree = 0;
  sqlite3_free(p->aUsed);
  sqlite3_free(p->aBlkArray);
  sqlite3_free(p->zContainer);
  sqlite3_free(p->zAccount);
  if( p->fdDaemon ){
    bcv_close_socket(p->fdDaemon);
  }
  return SQLITE_OK;
}

/*
** Request block iBlk for the database file from the daemon. The daemon,
** assuming no error occurs, will reply with a new block map containing
** an entry for the requested block that is installed in 
** BlockcacheFile.aBlkArray/nBlkByte, from where the caller may read the
** new location of block iBlk. Return SQLITE_OK if no error occurs,
** or an SQLite error code otherwise.
*/
static int bcvRequestBlock(BlockcacheFile *p, int iBlk){
  int rc = SQLITE_OK;
  BlockcacheMessage req;
  BlockcacheMessage reply;

  memset(&req, 0, sizeof(req));
  req.eType = BCV_MESSAGE_REQUEST;
  req.iVal = iBlk;
  if( p->aUsed ){
    req.nData = p->nUsed;
    req.aData = p->aUsed;
  }
  rc = bcvExchangeMessage(p, &req, &reply);

  if( rc==SQLITE_OK ){
    int nUsedReq;
    sqlite3_free(p->aBlkArray);
    p->aBlkArray = reply.aData;
    p->nBlkByte = reply.nData;
    nUsedReq = ((p->nBlkByte / sizeof(u32))+7)/8;
    if( nUsedReq>p->nUsed ){
      u8 *aNew = (u8*)sqlite3_realloc(p->aUsed, nUsedReq);
      if( aNew ){
        memset(&aNew[p->nUsed], 0, (nUsedReq-p->nUsed));
        p->aUsed = aNew;
        p->nUsed = nUsedReq;
      }else{
        rc = SQLITE_NOMEM;
      }
    }
  }

  /* Either an error occured or the requested block must now be available */
  assert( rc!=SQLITE_OK 
      || (p->nBlkByte>=(iBlk+1)*4 && bcvGetU32(&p->aBlkArray[iBlk*4])!=0) 
  );
  return rc;
}

/*
** Locate block iBlk of the database file within the cache-file, making
** a request to the daemon process if necessary. If successful, return
** SQLITE_OK and set *piMap to the cache-file block number. Otherwise,
** return an SQLite error code. The final value of *piMap is undefined
** in this case.
**
** Block numbers within the database file are numbered starting from
** 0. Block numbers within the cache-file start at 1.
*/
static int bcvFindBlock(BlockcacheFile *p, int iBlk, int *piMap){
  int rc = SQLITE_OK;
  int iMap = 0;

  if( p->aBlkArray ){
    if( iBlk*sizeof(u32)<p->nBlkByte ){
      iMap = (int)bcvGetU32(&p->aBlkArray[iBlk*sizeof(u32)]);
    }
  }
  if( iMap==0 ){
    rc = bcvRequestBlock(p, iBlk);
    if( rc==SQLITE_OK && iBlk*sizeof(u32)<p->nBlkByte ){
      iMap = (int)bcvGetU32(&p->aBlkArray[iBlk*sizeof(u32)]);
    }
  }

  *piMap = iMap;
  return rc;
}

/*
** Read data from a BlockcacheFile.
*/
static int bcvRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  int iAmt, 
  sqlite_int64 iOfst
){
  int rc = SQLITE_OK;
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  sqlite3_file *pOrig = ORIGFILE(pFile);
  if( p->fdDaemon ){
    int iBlk = (iOfst / p->szBlk);
    int iMap = 0;

    if( iOfst==0 && iAmt==100 ){
      /* Ignore this read. */
      return SQLITE_IOERR_SHORT_READ;
    }
    assert( (iAmt & (iAmt-1))==0 );
    assert( p->iPrevBlk<0 );

    rc = bcvFindBlock(p, iBlk, &iMap);
    if( rc==SQLITE_OK ){
      sqlite3_int64 iOff = (iMap-1)*p->szBlk + (iOfst % p->szBlk);
      if( iMap<=0 ) return BCV_RETURN(SQLITE_IOERR);
      rc = p->pBlkFile->pMethods->xRead(p->pBlkFile, zBuf, iAmt, iOff);
      p->aUsed[(iBlk)/8] |= 1<<((iBlk)%8);
    }

    /* Ensure this database is a wal database */
    if( rc==SQLITE_OK && iOfst==0 ){
      ((u8*)zBuf)[18] = 0x02;
      ((u8*)zBuf)[19] = 0x02;
    }
  }else{
    rc = pOrig->pMethods->xRead(pOrig, zBuf, iAmt, iOfst);
  }
  return BCV_RETURN(rc);
}

/*
** Write data to an mem-file.
*/
static int bcvWrite(
  sqlite3_file *pFile,
  const void *z,
  int iAmt,
  sqlite_int64 iOfst
){
  int rc = SQLITE_OK;
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  sqlite3_file *pOrig = ORIGFILE(pFile);
  if( p->fdDaemon ){
    int iBlk = (iOfst / p->szBlk);
    int iMap = 0;

    if( iBlk!=p->iPrevBlk ){
      BlockcacheMessage wreq;
      BlockcacheMessage reply;
      u8 aBuf[4];

      memset(&wreq, 0, sizeof(wreq));
      memset(&reply, 0, sizeof(reply));
      bcvPutU32(aBuf, iBlk);
      wreq.eType = BCV_MESSAGE_WREQUEST;
      wreq.aData = aBuf;
      wreq.nData = sizeof(aBuf);
      rc = bcvExchangeMessage(p, &wreq, &reply);
      if( rc==SQLITE_OK ){
        assert( reply.nData==wreq.nData );
        p->iPrevBlk = iBlk;
        p->iPrevLoc = bcvGetU32(reply.aData);
        sqlite3_free(reply.pFree);
      }
    }
    iMap = p->iPrevLoc;

    if( rc==SQLITE_OK ){
      sqlite3_int64 iOff = (iMap-1)*p->szBlk + (iOfst % p->szBlk);
      rc = p->pBlkFile->pMethods->xWrite(p->pBlkFile, z, iAmt, iOff);
    }
  }else{
    rc = pOrig->pMethods->xWrite(pOrig, z, iAmt, iOfst);
  }

  return BCV_RETURN(rc);
}

/*
** Truncate an mem-file.
*/
static int bcvTruncate(sqlite3_file *pFile, sqlite_int64 size){
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  int rc = SQLITE_OK;
  if( 0==p->fdDaemon ){
    sqlite3_file *pOrig = ORIGFILE(pFile);
    rc = pOrig->pMethods->xTruncate(pOrig, size);
  }
  return rc;
}

/*
** Sync an mem-file.
*/
static int bcvSync(sqlite3_file *pFile, int flags){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xSync(pOrig, flags);
}

/*
** Return the current file-size of an mem-file.
*/
static int bcvFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  int rc = SQLITE_OK;
  if( p->fdDaemon ){
    if( p->lockMask && (p->lockMask & (1<<1))==0 && p->aBlkArray==0 ){
      rc = bcvRequestBlock(p, 0);
    }
    if( p->aBlkArray==0 ){
      *pSize = ((sqlite3_int64)1 << (16 + 31)) - (1<<16);
    }else{
      *pSize = (p->nBlkByte / sizeof(u32)) * p->szBlk;
    }
    assert( *pSize!=0 );
  }else{
    sqlite3_file *pOrig = ORIGFILE(pFile);
    rc = pOrig->pMethods->xFileSize(pOrig, pSize);
  }
  return BCV_RETURN(rc);
}

/*
** Lock an mem-file.
*/
static int bcvLock(sqlite3_file *pFile, int eLock){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xLock(pOrig, eLock);
}

/*
** Unlock an mem-file.
*/
static int bcvUnlock(sqlite3_file *pFile, int eLock){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xUnlock(pOrig, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an mem-file.
*/
static int bcvCheckReservedLock(sqlite3_file *pFile, int *pResOut){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xCheckReservedLock(pOrig, pResOut);
}

/*
** File control method. For custom operations on an mem-file.
*/
static int bcvFileControl(sqlite3_file *pFile, int op, void *pArg){
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  sqlite3_file *pOrig = ORIGFILE(pFile);
  int rc;

  if( op==SQLITE_FCNTL_BCV_N_MSG ){
    sqlite3_int64 *pi = (sqlite3_int64*)pArg;
    *pi = p->nMsg;
    rc = SQLITE_OK;
  }else if( op==SQLITE_FCNTL_BCV_MS_MSG ){
    sqlite3_int64 *pi = (sqlite3_int64*)pArg;
    *pi = p->msMsgWait;
    rc = SQLITE_OK;
  }else{
    rc = pOrig->pMethods->xFileControl(pOrig, op, pArg);
    if( rc==SQLITE_OK && op==SQLITE_FCNTL_VFSNAME ){
      char *z = *(char**)pArg;
      z = sqlite3_mprintf("blockcachevfs/%z", z);
      *(char**)pArg = z;
    }
    else if( p->bDaemon==0 && op==SQLITE_FCNTL_CKPT_DONE ){
      assert( p->fdDaemon );
      if( rc==SQLITE_OK || rc==SQLITE_NOTFOUND ){
        if( p->lockMask==0 ){
          p->bSendCkptOnClose = 1;
        }
      }
      p->iPrevBlk = -1;
    }
    else if( rc==SQLITE_NOTFOUND && op==SQLITE_FCNTL_PRAGMA ){
      char **aPragma = (char**)pArg;
      char *zName = aPragma[1];
      if( memcmp("bcv_", zName, 4)==0 ){
        char *zMsg;
        BlockcacheMessage msg;
        BlockcacheMessage reply;
        memset(&msg, 0, sizeof(msg));
        memset(&reply, 0, sizeof(reply));
        
        zMsg = sqlite3_mprintf("%s=%s", zName, aPragma[2] ? aPragma[2] : "");
        if( zMsg==0 ) return SQLITE_NOMEM;
  
        msg.eType = BCV_MESSAGE_QUERY;
        msg.nData = strlen(zMsg);
        msg.aData = (u8*)zMsg;
  
        rc = bcvExchangeMessage(p, &msg, &reply);
        if( rc==SQLITE_OK ){
          assert( reply.eType==BCV_MESSAGE_QUERY_REPLY );
          aPragma[0] = sqlite3_mprintf("%.*s", reply.nData, reply.aData);
          sqlite3_free(reply.pFree);
          rc = reply.iVal;
        }
        sqlite3_free(zMsg);
      }
    }
  }

  return rc;
}

/*
** Return the sector-size in bytes for an mem-file.
*/
static int bcvSectorSize(sqlite3_file *pFile){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xSectorSize(pOrig);
}

/*
** Return the device characteristic flags supported by an mem-file.
*/
static int bcvDeviceCharacteristics(sqlite3_file *pFile){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xDeviceCharacteristics(pOrig);
}

/* 
** Create a shared memory file mapping 
*/
static int bcvShmMap(
  sqlite3_file *pFile,
  int iPg,
  int pgsz,
  int bExtend,
  void volatile **pp
){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xShmMap(pOrig, iPg, pgsz, bExtend, pp);
}

/* 
** Perform locking on a shared-memory segment 
*/
static int bcvShmLock(sqlite3_file *pFile, int offset, int n, int flags){
  int rc = SQLITE_OK;
  int rc2;
  int bSendCkpt = 0;
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  sqlite3_file *pOrig = ORIGFILE(pFile);
  u32 mask = (1 << (offset+n)) - (1 << offset);

  if( flags & SQLITE_SHM_UNLOCK ){
    if( p->lockMask==(1<<1) && (flags & SQLITE_SHM_EXCLUSIVE) ){
      if( p->bDaemon ) return SQLITE_OK;
      bSendCkpt = 1;
    }
    p->lockMask &= ~mask;
    if( p->lockMask==0 && p->aBlkArray ){
      BlockcacheMessage done;
      memset(&done, 0, sizeof(done));
      done.eType = BCV_MESSAGE_DONE;
      done.nData = p->nUsed;
      done.aData = p->aUsed;
      rc = bcvSendMessage(p->fdDaemon, &done);
      sqlite3_free(p->aBlkArray);
      sqlite3_free(p->aUsed);
      p->aBlkArray = 0;
      p->nBlkByte = 0;
      p->aUsed = 0;
      p->nUsed = 0;
    }
  }
  rc2 = pOrig->pMethods->xShmLock(pOrig, offset, n, flags);
  if( rc2==SQLITE_OK && (flags & SQLITE_SHM_LOCK) ){
    p->lockMask |= mask;
  }
  if( bSendCkpt && rc==SQLITE_OK && rc2==SQLITE_OK ){
    BlockcacheMessage ckpt;
    memset(&ckpt, 0, sizeof(ckpt));
    ckpt.eType = BCV_MESSAGE_WDONE;
    rc = bcvSendMessage(p->fdDaemon, &ckpt);
    p->iPrevBlk = -1;
  }
  return rc==SQLITE_OK ? rc2 : rc;
}

/* 
** Memory barrier operation on shared memory 
*/
static void bcvShmBarrier(sqlite3_file *pFile){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  pOrig->pMethods->xShmBarrier(pOrig);
}

/* 
** Unmap a shared memory segment 
*/
static int bcvShmUnmap(sqlite3_file *pFile, int deleteFlag){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xShmUnmap(pOrig, deleteFlag);
}

/*
** Fetch a page of a memory-mapped file
*/
static int bcvFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  int iAmt,
  void **pp
){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xFetch(pOrig, iOfst, iAmt, pp);
}

/* 
** Release a memory-mapped page 
*/
static int bcvUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *pPage){
  sqlite3_file *pOrig = ORIGFILE(pFile);
  return pOrig->pMethods->xUnfetch(pOrig, iOfst, pPage);
}

static int bcvParseInt(const u8 *z, int n){
  int ii;
  int iPort = 0;
  for(ii=0; z[ii]>='0' && z[ii]<='9' && (ii<n || n<0); ii++){
    iPort = iPort*10 + (z[ii]-'0');
  }
  return iPort;
}

/*
** Attempt to open a new socket connection to port iPort on localhost.
** If successful, set (*pSocket) to point to the new socket handle and
** return SQLITE_OK. Or, if an error occurs, return an SQLite error
** code. The final value of (*pSocket) is undefined in this case.
*/
static int bcvConnectSocket(int iPort, BCV_SOCKET_TYPE *pSocket){
  int rc = SQLITE_OK;
  BCV_SOCKET_TYPE s;
  s = socket(AF_INET, SOCK_STREAM, 0);
  if( bcv_socket_is_valid(s)==0 ){
    rc = SQLITE_CANTOPEN;
  }else{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(iPort);

    /* This should really be:
    **
    **   inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr); 
    **
    ** But since mingw does not have inet_pton, and we only ever use 
    ** it on the ipv4 localhost address, just hard code the value. */
    ((u8*)&addr.sin_addr)[0] = 0x7F;
    ((u8*)&addr.sin_addr)[1] = 0x00;
    ((u8*)&addr.sin_addr)[2] = 0x00;
    ((u8*)&addr.sin_addr)[3] = 0x01;

    if( connect(s, (struct sockaddr*)&addr, sizeof(addr))<0 ){
      sqlite3_log(SQLITE_CANTOPEN, 
          "blockcachevfs: failed to connect to 127.0.0.1:%d", iPort
      );
      rc = SQLITE_CANTOPEN;
      bcv_close_socket(s);
    }else{
      *pSocket = s;
    }
  }

  return rc;
}

/* 
** Connect to the daemon process. Leave the tcp socket in p->fdDaemon.
*/
static int bcvConnectDaemon(BlockcacheFile *p){
  sqlite3_file *pOrig = ORIGFILE(p);
  sqlite3_int64 sz;
  int rc;

  rc = pOrig->pMethods->xFileSize(pOrig, &sz);
  if( rc==SQLITE_OK ){
    u8 *aBuf = (u8*)sqlite3_malloc(sz);
    if( aBuf==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }else{
      rc = pOrig->pMethods->xRead(pOrig, aBuf, sz, 0);
    }

    if( rc==SQLITE_OK ){
      /* Assuming it is a valid blockvfs database file, buffer aBuf[] now
      ** contains something of the form:
      **
      **    blockvfs:1:$container/$database:$port\n
      **
      ** Parse this up.  */
      int iPort = 0;
      char *zDb = (char*)&aBuf[11];
      int nDb = 0;
      for(nDb=0; nDb<(sz-11); nDb++){
        if( zDb[nDb]==':' ) break;
      }

      iPort = bcvParseInt(&aBuf[nDb+12], sz-nDb-12);
      if( iPort==0 ){
        rc = SQLITE_CANTOPEN;
      }else{
        rc = bcvConnectSocket(iPort, &p->fdDaemon);
        if( rc==SQLITE_OK ){
          int nMsg = 5 + nDb + 1;
          u8 *aMsg = sqlite3_malloc(nMsg);
          if( aMsg==0 ){
            rc = SQLITE_IOERR_NOMEM;
          }else{
            BlockcacheMessage reply;
            BlockcacheMessage login;
            memset(&reply, 0, sizeof(reply));

            memset(&login, 0, sizeof(login));
            login.eType = BCV_MESSAGE_LOGIN;
            login.aData = (u8*)sqlite3_mprintf("%.*s", nDb, zDb);;
            login.nData = strlen((const char*)login.aData);

            rc = bcvExchangeMessage(p, &login, &reply);
            if( rc==SQLITE_OK ){
              int i;
              const char *zReply = (const char*)reply.aData;
              assert( p->zContainer==0 );
              for(i=0; i<reply.nData && zReply[i]!='/'; i++);
              p->zContainer = sqlite3_mprintf("%.*s", i, zReply);
              p->zAccount = sqlite3_mprintf(
                  "%.*s", reply.nData-i-1, &zReply[i+1]
              );
              p->szBlk = reply.iVal;
              if( p->zContainer==0 || p->zAccount==0 ){
                rc = SQLITE_NOMEM;
              }
            }

            sqlite3_free(login.aData);
            sqlite3_free(aMsg);
            sqlite3_free(reply.pFree);
          }
        }
      }
    }
    sqlite3_free(aBuf);
  }

  return rc;
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
  int f = SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB;
  sqlite3_int64 sz = 0;
  sqlite3_file *pFile = 0;
  sqlite3_vfs *pVfs = (sqlite3_vfs*)bcv_vfs.pAppData;
  char *zRet = 0;

  pFile = (sqlite3_file*)sqlite3_malloc(pVfs->szOsFile);
  if( pFile==0 ){
    return SQLITE_NOMEM;
  }
  memset(pFile, 0, pVfs->szOsFile);

  rc = pVfs->xOpen(pVfs, zFile, pFile, f, &f);
  if( rc==SQLITE_OK ){
    rc = pFile->pMethods->xFileSize(pFile, &sz);
  }

  if( rc==SQLITE_OK ){
    zRet = sqlite3_malloc(sz+1);
    if( zRet==0 ){
      rc = SQLITE_NOMEM;
    }else{
      zRet[sz] = '\0';
      rc = pFile->pMethods->xRead(pFile, zRet, sz, 0);
    }
  }

  if( rc!=SQLITE_OK ){
    sqlite3_free(zRet);
    zRet = 0;
  }
  if( pFile && pFile->pMethods ){
    pFile->pMethods->xClose(pFile);
  }
  sqlite3_free(pFile);

  *pzText = zRet;
  return rc;
}

int bcvMakeFilename(char **pzFile, char **ppFree){
  int rc = SQLITE_NOMEM;
  *ppFree = 0;
  if( *pzFile ){
    char *zFile = *pzFile;
    int nFile = strlen(zFile);
    char *pFree;
    *ppFree = pFree = sqlite3_malloc(nFile + 4 + 2);
    if( pFree==0 ){
      *pzFile = 0;
      rc = SQLITE_NOMEM;
    }else{
      memset(pFree, 0, nFile+4+2);
      memcpy(&pFree[4], zFile, nFile);
      *pzFile = &pFree[4];
    }
    sqlite3_free(zFile);
  }
  return rc;
}

static int bcvAutoAttach(
  const char *zFile,              /* File-name to open */
  const char *zSas,               /* URI bcv_sas= parameter */
  int bReadflag                   /* True if SQLITE_OPEN_READONLY */
){
  int rc = SQLITE_OK;
  int i2;
  int i1;
  char *zPortfile = 0;
  char *zContainer = 0;
  char *zPort = 0;
  char *zFree = 0;
  char *pFree = 0;
  int bReadonly = bReadflag;
  const char *zSasToken = zSas;
  BCV_SOCKET_TYPE fd;

  for(i2=strlen(zFile); i2>0 && zFile[i2]!='/'; i2--);
  if( i2==0 ) return SQLITE_ERROR;
  for(i1=i2-1; i1>0 && zFile[i1]!='/'; i1--);
  if( zFile[i1]!='/' ) return SQLITE_ERROR;

  zPortfile = sqlite3_mprintf("%.*s%s", i1+1, zFile, BCV_PORTNUMBER_FILE);
  bcvMakeFilename(&zPortfile, &pFree);
  zContainer = sqlite3_mprintf("%.*s", i2-i1-1, &zFile[i1+1]);
  if( zPortfile==0 || zContainer==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = bcvReadTextFile(zPortfile, &zPort);
    if( rc==SQLITE_OK ){
      int iPort = bcvParseInt((const u8*)zPort, -1);
      rc = bcvConnectSocket(iPort, &fd);
    }
  }

  if( rc==SQLITE_OK ){
    if( zSas[0]=='1' && zSas[1]=='\0' ){
      /* If the SAS token is just "1", then request the account name from the
      ** daemon process, then use the callback to obtain an SAS token.  */
      if( bcv_global.xSas==0 ){
        rc = SQLITE_CANTOPEN;
      }else{
        BlockcacheMessage msg;
        BlockcacheMessage reply;
  
        memset(&msg, 0, sizeof(msg));
        memset(&reply, 0, sizeof(reply));
  
        msg.eType = BCV_MESSAGE_QUERY;
        msg.aData = (u8*)"bcv_account=";
        msg.nData = 12;
        rc = bcvSendMessage(fd, &msg);
        if( rc==SQLITE_OK ){
          rc = bcvReceiveMessage(fd, &reply);
        }
        if( rc==SQLITE_OK ){
          rc = bcv_global.xSas(
              bcv_global.pSasCtx, "azure", 
              (const char*)reply.aData, zContainer, &zFree, &bReadonly
          );
          zSasToken = zFree;
        }
        sqlite3_free(reply.pFree);
      }
    }
  
    if( rc==SQLITE_OK ){
      BlockcacheMessage msg;
      BlockcacheMessage reply;
      char *zMsg;
  
      memset(&msg, 0, sizeof(msg));
      memset(&reply, 0, sizeof(reply));
        
      zMsg = sqlite3_mprintf("bcv_%s=%s?%s", 
          bReadonly?"readonly":"attach", zContainer, zSasToken
      );
      if( zMsg==0 ){
        rc = SQLITE_NOMEM;
      }else{
        msg.eType = BCV_MESSAGE_QUERY;
        msg.nData = strlen(zMsg);
        msg.aData = (u8*)zMsg;
        rc = bcvSendMessage(fd, &msg);
        if( rc==SQLITE_OK ){
          rc = bcvReceiveMessage(fd, &reply);
        }
        sqlite3_free(zMsg);
        if( rc==SQLITE_OK && reply.iVal ){
          rc = SQLITE_CANTOPEN;
        }
        sqlite3_free(reply.pFree);
      }
    }
    bcv_close_socket(fd);
  }

  sqlite3_free(zPort);
  sqlite3_free(zContainer);
  sqlite3_free(zFree);
  sqlite3_free(pFree);
  return rc;
}

/*
** Open an bcv file handle.
*/
static int bcvOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  int flags,
  int *pOutFlags
){
  int rc = SQLITE_OK;
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  BlockcacheFile *p = (BlockcacheFile*)pFile;
  sqlite3_file *pOrig = ORIGFILE(p);

  memset(p, 0, sizeof(BlockcacheFile));
  p->iPrevBlk = -1;
  p->base.pMethods = &bcv_io_methods;

  if( flags & SQLITE_OPEN_URI ){
    const char *zSas = sqlite3_uri_parameter(zName, BCV_SAS_PARAM);
    if( zSas ){
      rc = bcvAutoAttach(zName, zSas, (flags & SQLITE_OPEN_READONLY)!=0);
      flags &= ~SQLITE_OPEN_CREATE;
    }
  }

  if( rc==SQLITE_OK ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pOrig, flags, pOutFlags);
  }
  if( rc==SQLITE_OK && (flags & SQLITE_OPEN_MAIN_DB) ){
    char aHdr[11];
    rc = pOrig->pMethods->xRead(pOrig, aHdr, 11, 0);
    if( rc==SQLITE_OK ){
      if( memcmp(aHdr, "blockvfs:1:", 11)==0 ){
        p->bDaemon = sqlite3_uri_boolean(zName, "daemon", 0);
        rc = bcvConnectDaemon(p);
        if( rc==SQLITE_OK ){
          int nName = strlen(zName);
          int f = SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_READWRITE;
          char *zNew = 0;
          char *pFree = 0;
          int ii;
          int nSlash = 0;
    
          for(ii=nName-1; ii>0; ii--){
            if( zName[ii]=='/' || zName[ii]=='\\' ){
              if( (++nSlash)==2 ) break;
            }
          }
          zNew = sqlite3_mprintf("%.*s%s", ii+1, zName, BCV_CACHEFILE_NAME);
          bcvMakeFilename(&zNew, &pFree);
          if( zNew==0 ){
            rc = SQLITE_IOERR_NOMEM;
          }else{
            p->pBlkFile = (sqlite3_file*)(((u8*)pOrig) + pOrigVfs->szOsFile);
            rc = pOrigVfs->xOpen(pOrigVfs, zNew, p->pBlkFile, f, &f);
            p->pFree = pFree;
          }
        }
      }
    }else if( rc==SQLITE_IOERR_SHORT_READ ){
      rc = SQLITE_OK;
    }

    if( rc!=SQLITE_OK ){
      bcvClose(pFile);
    }
  }

  return rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static int bcvDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xDelete(pOrigVfs, zPath, dirSync);
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int bcvAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int flags, 
  int *pResOut
){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xAccess(pOrigVfs, zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static int bcvFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int nOut, 
  char *zOut
){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xFullPathname(pOrigVfs, zPath, nOut, zOut);
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *bcvDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xDlOpen(pOrigVfs, zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void bcvDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  pOrigVfs->xDlError(pOrigVfs, nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*bcvDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xDlSym(pOrigVfs, p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void bcvDlClose(sqlite3_vfs *pVfs, void *pHandle){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  pOrigVfs->xDlClose(pOrigVfs, pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static int bcvRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xRandomness(pOrigVfs, nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static int bcvSleep(sqlite3_vfs *pVfs, int nMicro){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xSleep(pOrigVfs, nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int bcvCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xCurrentTime(pOrigVfs, pTimeOut);
}

static int bcvGetLastError(sqlite3_vfs *pVfs, int a, char *b){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xGetLastError(pOrigVfs, a, b);
}
static int bcvCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  sqlite3_vfs *pOrigVfs = ORIGVFS(pVfs);
  return pOrigVfs->xCurrentTimeInt64(pOrigVfs, p);
}

/*************************************************************************/
/* BEGIN VIRTUAL TABLE CODE */
/*************************************************************************/

#define BCV_MANIFEST_COLS "container,dbid,dbname,offset,blockid"
#define BCV_CACHE_COLS    "blockid,offset,pending,dirty"

typedef struct bcv_vtab bcv_vtab;
struct bcv_vtab {
  sqlite3_vtab base;
  const char *zTopic;
  int nCol;
  sqlite3 *db;
};

typedef struct bcv_cursor bcv_cursor;
struct bcv_cursor {
  sqlite3_vtab_cursor base;       /* Base class */
  sqlite3_int64 iRowid;           /* Current rowid */
  char **azVal;                   /* Array of text values for current row */
  BlockcacheMessage data;         /* Data from daemon */
  int iDataOff;                   /* Offset within data.aData, -ve==EOF */
};

static int bcvVtabConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  const char *zTopic = (const char*)pAux;
  bcv_vtab *pNew = 0;
  int nCol = 0;
  int rc;

  if( sqlite3_stricmp(zTopic, "bcv_manifest")==0 ){
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(" BCV_MANIFEST_COLS ")");
    nCol = 5;
  }else
  if( sqlite3_stricmp(zTopic, "bcv_cache")==0 ){
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(" BCV_CACHE_COLS ")");
    nCol = 4;
  }else{
    rc = SQLITE_ERROR;
  }

  if( rc==SQLITE_OK ){
    pNew = (bcv_vtab*)sqlite3_malloc(sizeof(*pNew));
    if( pNew==0 ){
      rc = SQLITE_NOMEM;
    }else{
      memset(pNew, 0, sizeof(*pNew));
      pNew->nCol = nCol;
      pNew->db = db;
      pNew->zTopic = zTopic;
    }
  }

  *ppVtab = (sqlite3_vtab*)pNew;
  return rc;
}

static int bcvVtabDisconnect(sqlite3_vtab *pVtab){
  bcv_vtab *p = (bcv_vtab*)pVtab;
  sqlite3_free(p);
  return SQLITE_OK;
}

static int bcvVtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  bcv_vtab *pTab = (bcv_vtab*)p;
  bcv_cursor *pCur;
  int nByte = sizeof(bcv_cursor) + pTab->nCol * sizeof(char*);
  pCur = (bcv_cursor*)sqlite3_malloc(nByte);
  if( pCur==0 ) return SQLITE_NOMEM;
  memset(pCur, 0, nByte);
  pCur->azVal = (char**)&pCur[1];
  *ppCursor = &pCur->base;
  return SQLITE_OK;
}

static int bcvVtabClose(sqlite3_vtab_cursor *cur){
  bcv_cursor *pCur = (bcv_cursor*)cur;
  sqlite3_free(pCur->data.pFree);
  sqlite3_free(pCur);
  return SQLITE_OK;
}

static int bcvVtabNext(sqlite3_vtab_cursor *cur){
  bcv_vtab *pTab = (bcv_vtab*)cur->pVtab;
  bcv_cursor *pCur = (bcv_cursor*)cur;
  int i;
  pCur->iRowid++;
  for(i=0; i<pTab->nCol; i++){
    if( pCur->iDataOff>=pCur->data.nData ){
      pCur->iDataOff = -1;
      break;
    }
    pCur->azVal[i] = (char*)&pCur->data.aData[pCur->iDataOff];
    pCur->iDataOff += strlen(pCur->azVal[i]) + 1;
  }
  return SQLITE_OK;
}

/*
** Return values of columns for the row at which the templatevtab_cursor
** is currently pointing.
*/
static int bcvVtabColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  int i                       /* Which column to return */
){
  bcv_vtab *pTab = (bcv_vtab*)cur->pVtab;
  bcv_cursor *pCur = (bcv_cursor*)cur;
  assert( i>=0 && i<pTab->nCol );
  sqlite3_result_text(ctx, pCur->azVal[i], -1, SQLITE_TRANSIENT);
  return SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static int bcvVtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  bcv_cursor *pCur = (bcv_cursor*)cur;
  *pRowid = pCur->iRowid;
  return SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static int bcvVtabEof(sqlite3_vtab_cursor *cur){
  bcv_cursor *pCur = (bcv_cursor*)cur;
  return pCur->iDataOff<0;
}

/*
** This method is called to "rewind" the templatevtab_cursor object back
** to the first row of output.  This method is always called at least
** once prior to any call to templatevtabColumn() or templatevtabRowid() or 
** templatevtabEof().
*/
static int bcvVtabFilter(
  sqlite3_vtab_cursor *cur, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  int rc;
  bcv_cursor *pCur = (bcv_cursor*)cur;
  bcv_vtab *pTab = (bcv_vtab*)cur->pVtab;
  sqlite3_file *pFile;
  BlockcacheFile *p;
  BlockcacheMessage msg;

  sqlite3_file_control(
      pTab->db, "main", SQLITE_FCNTL_FILE_POINTER, (void*)&pFile
  );
  if( pFile->pMethods!=&bcv_io_methods ) return SQLITE_ERROR;
  p = (BlockcacheFile*)pFile;

  sqlite3_free(pCur->data.aData);
  memset(&pCur->data, 0, sizeof(BlockcacheMessage));
  pCur->iRowid = 0;
  pCur->iDataOff = 0;
  memset(&msg, 0, sizeof(msg));
  msg.eType = BCV_MESSAGE_QUERY;
  msg.aData = (u8*)pTab->zTopic;
  msg.nData = strlen(pTab->zTopic);
  rc = bcvSendMessage(p->fdDaemon, &msg);
  if( rc==SQLITE_OK ){
    rc = bcvReceiveMessage(p->fdDaemon, &pCur->data);
  }
  if( rc==SQLITE_OK && !bcvVtabEof(cur) ){
    rc = bcvVtabNext(cur);
  }
  return rc;
}

/*
** SQLite will invoke this method one or more times while planning a query
** that uses the virtual table.  This routine needs to create
** a query plan for each invocation and compute an estimated cost for that
** plan.
*/
static int bcvVtabBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  pIdxInfo->estimatedCost = (double)(10*1000*1000);
  pIdxInfo->estimatedRows = 10*1000*1000;
  return SQLITE_OK;
}

static int bcvRegisterVtab(
  sqlite3 *db, 
  const char **pzErrMsg, 
  const struct sqlite3_api_routines *pThunk
){
  static sqlite3_module bcv_manifest = {
    /* iVersion    */ 2,
    /* xCreate     */ 0,
    /* xConnect    */ bcvVtabConnect,
    /* xBestIndex  */ bcvVtabBestIndex,
    /* xDisconnect */ bcvVtabDisconnect,
    /* xDestroy    */ 0,
    /* xOpen       */ bcvVtabOpen,
    /* xClose      */ bcvVtabClose,
    /* xFilter     */ bcvVtabFilter,
    /* xNext       */ bcvVtabNext,
    /* xEof        */ bcvVtabEof,
    /* xColumn     */ bcvVtabColumn,
    /* xRowid      */ bcvVtabRowid,
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
  const char *azMod[] = {"bcv_manifest", "bcv_cache", 0};
  int i;

  for(i=0; azMod[i]; i++){
    sqlite3_create_module(db, azMod[i], &bcv_manifest, (void*)azMod[i]);
  }
  return SQLITE_OK;
}

const char *sqlite3_bcv_register(int bDefault){
  if( bcv_vfs.pAppData==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
    bcv_vfs.szOsFile = sizeof(BlockcacheFile) + pVfs->szOsFile*2;
    bcv_vfs.pAppData = (void*)pVfs;
    sqlite3_vfs_register(&bcv_vfs, bDefault);
    sqlite3_auto_extension((void(*)(void))bcvRegisterVtab);
    bcv_socket_init();
  }
  return bcv_vfs.zName;
}

int sqlite3_bcv_sas_callback(
  void *pSasCtx,
  int(*xSas)(
      void *pCtx, 
      const char *zStorage, 
      const char *zAccount, 
      const char *zContainer,
      char **pzSasToken,
      int *pbReadonly
  )
){
  bcv_global.pSasCtx = pSasCtx;
  bcv_global.xSas = xSas;
  return SQLITE_OK;
}

int sqlite3_bcv_init(const char *unused){
  sqlite3_bcv_register(1);
  return SQLITE_OK;
}

static int bcvQueryDaemon(
  const char *zDir,
  const char *zQuery,
  int flags,
  BlockcacheMessage *pReply,
  char **pzErr
){
  char *zPort = 0;
  char *zPortfile = 0;
  char *pFree = 0;
  int rc = SQLITE_OK;
  int bOpen = 0;
  BCV_SOCKET_TYPE fd;
  BlockcacheMessage msg;

  memset(&fd, 0, sizeof(fd));
  memset(&msg, 0, sizeof(msg));
  memset(pReply, 0, sizeof(BlockcacheMessage));

  zPortfile = sqlite3_mprintf("%s/%s", zDir, BCV_PORTNUMBER_FILE);
  bcvMakeFilename(&zPortfile, &pFree);
  if( zPortfile==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = bcvReadTextFile(zPortfile, &zPort);
    if( rc!=SQLITE_OK && rc!=SQLITE_NOMEM ){
      rc = SQLITE_BCV_CANTCONNECT1;
      *pzErr = sqlite3_mprintf("failed to open daemon directory: %s", zDir);
    }
  }

  if( rc==SQLITE_OK ){
    int iPort = bcvParseInt((const u8*)zPort, -1);
    rc = bcvConnectSocket(iPort, &fd);
    bOpen = (rc==SQLITE_OK);
    if( rc!=SQLITE_OK ){
      rc = SQLITE_BCV_CANTCONNECT2;
      *pzErr = sqlite3_mprintf("failed to connect to daemon: %s", zDir);
    }
  }
  sqlite3_free(zPort);
  sqlite3_free(pFree);

  if( rc==SQLITE_OK ){
    msg.eType = BCV_MESSAGE_QUERY;
    msg.iVal = flags;
    msg.aData = (u8*)zQuery;
    msg.nData = strlen(zQuery);
    rc = bcvSendMessage(fd, &msg);
    if( rc==SQLITE_OK ){
      rc = bcvReceiveMessage(fd, pReply);
    }
    if( rc!=SQLITE_OK && rc!=SQLITE_NOMEM ){
      rc = SQLITE_BCV_CANTCONNECT3;
      *pzErr = sqlite3_mprintf("error communicating with daemon: %s", zDir);
    }
  }

  if( bOpen ) bcv_close_socket(fd);
  return rc;
}

/*
** Send a message to a local daemon process to attach a new container.
*/
int sqlite3_bcv_attach(
  const char *zDir,               /* Directory of daemon process to contact */
  const char *zContainer,         /* Container to attach */
  const char *zSas,               /* SAS token (if any) */
  int flags,                      /* SQLITE_BCVATTACH_X flags */
  char **pzErr                    /* OUT: error message (if any) */
){
  int rc = SQLITE_OK;
  BlockcacheMessage reply;
  char *zQuery = 0;

  memset(&reply, 0, sizeof(reply));

  if( pzErr ) *pzErr = 0;
  zQuery = sqlite3_mprintf("bcv_%s=%s%s%s", 
      ((flags & SQLITE_BCVATTACH_READONLY) ? "readonly" : "attach"), 
      zContainer, (zSas ? "?" : ""), (zSas ? zSas : "")
  );
  if( zQuery==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = bcvQueryDaemon(zDir, zQuery, flags, &reply, pzErr);
    sqlite3_free(zQuery);
  }

  if( rc==SQLITE_OK && reply.iVal!=SQLITE_OK ){
    rc = reply.iVal;
    if( reply.aData ) *pzErr = sqlite3_mprintf("%s", (char*)reply.aData);
  }

  sqlite3_free(reply.pFree);
  return rc;
}

/*
** Send a message to a local daemon process to detach a container.
*/
int sqlite3_bcv_detach(
  const char *zDir,               /* Directory of daemon process to contact */
  const char *zContainer,         /* Container to detach */
  char **pzErr                    /* OUT: error message (if any) */
){
  int rc = SQLITE_OK;
  BlockcacheMessage reply;
  char *zQuery = 0;

  memset(&reply, 0, sizeof(reply));

  if( pzErr ) *pzErr = 0;
  zQuery = sqlite3_mprintf("bcv_detach=%s", zContainer);
  if( zQuery==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = bcvQueryDaemon(zDir, zQuery, 0, &reply, pzErr);
    sqlite3_free(zQuery);
  }

  if( rc==SQLITE_OK && reply.iVal!=SQLITE_OK ){
    rc = reply.iVal;
    if( reply.aData ) *pzErr = sqlite3_mprintf("%s", (char*)reply.aData);
  }

  sqlite3_free(reply.pFree);
  return rc;
}


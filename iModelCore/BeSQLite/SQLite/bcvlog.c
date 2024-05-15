/*
** 2023-04-01
**
******************************************************************************
**
*/

#include "bcv_int.h"
#include "blockcachevfs.h"

#include <assert.h>
#include <string.h>

/*
** Each log entry is stored in memory as an instance of the following
** structure.
*/
typedef struct BcvLogHttp BcvLogHttp;
struct BcvLogHttp {
  i64 iLogId;                     /* Logging id */
  i64 iRequestTime;               /* Request timestamp */
  i64 iReplyTime;                 /* Reply timestamp */

  int eMethod;                    /* SQLITE_BCV_METHOD_ constant */
  int nRetry;                     /* # attempts before this one */
  char *zFile;                    /* File tail of of logged URI */
  char *zClientId;                /* Client ID */
  char *zMsg;                     /* Log message */
  int httpcode;                   /* HTTP reply code from server */

  BcvLogHttp *pNext;
  BcvLogHttp *pPrev;
};

/*
** Logging object used by daemon and local VFS.
*/
struct BcvLog {
  i64 iTimeout;
  i64 iMaxEntry;

  i64 iNextLogId;
  sqlite3_mutex *mutex;

  int nEntry;                     /* Number of entries in pFirst/pLast list */
  BcvLogHttp *pFirst;
  BcvLogHttp *pLast;
};

/*
** Defaults for the two sqlite3_bcv_config() parameters.
*/
#define BCVLOG_DEFAULT_TIMEOUT  3600
#define BCVLOG_DEFAULT_MAXENTRY -1

/*
** Allocate and return pointer to a new logging object.
*/
BcvLog *bcvLogNew(){
  BcvLog *p = sqlite3_malloc(sizeof(BcvLog));
  if( p ){
    memset(p, 0, sizeof(BcvLog));
    p->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    if( p->mutex==0 && sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN) ){
      /* OOM in sqlite3_mutex_alloc() */
      sqlite3_free(p);
      p = 0;
    }else{
      p->iTimeout = BCVLOG_DEFAULT_TIMEOUT;
      p->iMaxEntry = BCVLOG_DEFAULT_MAXENTRY;
    }
  }
  return p;
}

/*
** Delete a logging object.
*/
void bcvLogDelete(BcvLog *pLog){
  if( pLog ){
    while( pLog->pFirst ){
      BcvLogHttp *pNext = pLog->pFirst->pNext;
      sqlite3_free(pLog->pFirst);
      pLog->pFirst = pNext;
    }
    sqlite3_mutex_free(pLog->mutex);
    sqlite3_free(pLog);
  }
}

/*
** Return the number of bytes in the URI passed as the only argument,
** up to but not including the first '?' character.
*/
static int bcvLogUriLength(const char *zUri){
  int nByte;
  for(nByte=0; zUri[nByte] && zUri[nByte]!='?'; nByte++);
  return nByte;
}


/*
** The logging-object mutex must be held to call this function. It
** frees any log entries that should be freed according to the configured
** values of the SQLITE_BCV_HTTPLOG_TIMEOUT and SQLITE_BCV_HTTPLOG_NENTRY
** parameters.
*/
static void bcvLogEnforceLimits(BcvLog *pLog){
  if( pLog->iTimeout>0 || pLog->iMaxEntry>0 ){
    i64 iLimit = sqlite_timestamp() - (pLog->iTimeout*1000);
    while( (pLog->pFirst && pLog->pFirst->iRequestTime<iLimit)
        || (pLog->iMaxEntry>=0 && pLog->nEntry>pLog->iMaxEntry)
    ){
      BcvLogHttp *pDel = pLog->pFirst;
      pLog->pFirst = pDel->pNext;
      sqlite3_free(pDel);
      pLog->nEntry--;
    }
    if( pLog->pFirst==0 ) pLog->pLast = 0;
  }
}

/*
** Log an HTTP request.
*/
int bcvLogRequest(
  BcvLog *pLog, 
  const char *zClientId,          /* Id of client that made this request */
  const char *zLogMsg,            /* Log message */
  int eMethod,                    /* SQLITE_BCV_METHOD_* constant */
  int nRetry,                     /* 0 for first attempt, 1 for second... */
  const char *zUri,               /* Logging URI of request */
  i64 *piLogId                    /* OUT: Id to pass to bcvLogReply() */
){
  int rc = SQLITE_OK;
  if( pLog ){
    BcvLogHttp *pNew = 0;
    int nByte = sizeof(BcvLogHttp);
    int nUri = bcvLogUriLength(zUri);
    int nClient = bcvStrlen(zClientId);
    int nLogMsg = bcvStrlen(zLogMsg);

    nByte += nUri+1 + nClient+1 + nLogMsg+1;
    pNew = bcvMallocRc(&rc, nByte);
    if( pNew ){
      i64 iRequestTime = 0;

      pNew->eMethod = eMethod;
      pNew->nRetry = nRetry;
      pNew->zFile = (char*)&pNew[1];
      if( nUri ) memcpy(pNew->zFile, zUri, nUri);
      pNew->zClientId = &pNew->zFile[nUri+1];
      if( nClient ) memcpy(pNew->zClientId, zClientId, nClient);
      pNew->zMsg = &pNew->zClientId[nClient+1];
      if( nLogMsg ) memcpy(pNew->zMsg, zLogMsg, nLogMsg);
      iRequestTime = pNew->iRequestTime = sqlite_timestamp();

      sqlite3_mutex_enter(pLog->mutex);
      *piLogId = pNew->iLogId = pLog->iNextLogId++;
      pNew->pPrev = pLog->pLast;
      if( pLog->pLast ){
        assert( pLog->pLast->pNext==0 );
        pLog->pLast->pNext = pNew;
      }else{
        assert( pLog->pFirst==0 );
        pLog->pFirst = pNew;
      }
      pLog->pLast = pNew;
      pLog->nEntry++;
      bcvLogEnforceLimits(pLog);
      sqlite3_mutex_leave(pLog->mutex);
    }
  }

  return rc;
}

/*
** Log an HTTP reply.
*/
int bcvLogReply(
  BcvLog *pLog,
  i64 iLogId,                     /* Id from earlier bcvLogRequest() call */
  int httpcode,                   /* HTTP reponse code */
  i64 *piMs                       /* OUT: Total ms for this request */
){
  int rc = SQLITE_OK;
  if( pLog ){
    BcvLogHttp *pHttp = 0;
    i64 iReplyTime = sqlite_timestamp();

    sqlite3_mutex_enter(pLog->mutex);
    for(pHttp=pLog->pLast; pHttp; pHttp=pHttp->pPrev){
      if( pHttp->iLogId==iLogId ) break;
    }
    if( pHttp ){
      pHttp->iReplyTime = iReplyTime;
      pHttp->httpcode = httpcode;
      *piMs = pHttp->iReplyTime - pHttp->iRequestTime;
    }
    sqlite3_mutex_leave(pLog->mutex);
  }

  return rc;
}

/*
** Return a string corresponding to the SQLITE_BCV_METHOD_XXX constant
** passed as the only argment. e.g. SQLITE_BCV_METHOD_GET -> "GET".
*/
static const char *bcvLogMethodString(int eMethod){
  const char *azRet[] = {
    0, "GET", "PUT", "DELETE", "HEAD"
  };
  assert( SQLITE_BCV_METHOD_GET==1 );
  assert( SQLITE_BCV_METHOD_PUT==2 );
  assert( SQLITE_BCV_METHOD_DELETE==3 );
  assert( SQLITE_BCV_METHOD_HEAD==4 );

  assert( eMethod>=1 && eMethod<sizeof(azRet)/sizeof(azRet[0]) );
  return azRet[eMethod];
}

/*
** Populate the buffer passed as the second argument with a blob containing
** the current http log contents. Return SQLITE_OK if successful, or an
** SQLite error code (e.g. SQLITE_NOMEM) otherwise.
*/
int bcvLogGetData(BcvLog *pLog, BcvBuffer *pBuf){
  int rc = SQLITE_OK;
  if( pLog ){
    BcvLogHttp *p;
    sqlite3_mutex_enter(pLog->mutex);
    bcvLogEnforceLimits(pLog);
    for(p=pLog->pFirst; p; p=p->pNext){
      bcvBufferAppendU64(&rc, pBuf, p->iLogId);
      bcvBufferAppendU64(&rc, pBuf, p->iRequestTime);
      bcvBufferAppendU64(&rc, pBuf, p->iReplyTime);
      bcvBufferMsgString(&rc, pBuf, bcvLogMethodString(p->eMethod));
      bcvBufferMsgString(&rc, pBuf, p->zClientId);
      bcvBufferMsgString(&rc, pBuf, p->zMsg);
      bcvBufferMsgString(&rc, pBuf, p->zFile);
      bcvBufferAppendU32(&rc, pBuf, p->httpcode);
    }
    sqlite3_mutex_leave(pLog->mutex);
  }
  return rc;
}

/*
** Parameter op must be either SQLITE_BCV_HTTPLOG_TIMEOUT or
** SQLITE_BCV_HTTPLOG_NENTRY. This function sets the value of the corresponding
** parameter to iVal.
*/
void bcvLogConfig(BcvLog *pLog, int op, i64 iVal){
  assert( 
      op==SQLITE_BCV_HTTPLOG_TIMEOUT
   || op==SQLITE_BCV_HTTPLOG_NENTRY
  );
  
  if( pLog ){
    sqlite3_mutex_enter(pLog->mutex);
    if( op==SQLITE_BCV_HTTPLOG_TIMEOUT ){
      pLog->iTimeout = iVal;
    }else{
      pLog->iMaxEntry = iVal;
    }
    bcvLogEnforceLimits(pLog);
    sqlite3_mutex_leave(pLog->mutex);
  }
}

/*
** Parameter iTime is a julian-day value multiplied by 86400000, as returned 
** by an SQLite VFS xCurrentTimeInt64() method. This function formats the
** time value as an ISO-8601 time string and writes the results into buffer
** zBuf. The caller is responsible for ensuring that zBuf is large enough.
** Example time string:
**
**     "2015-09-20 17:57:22.432"
*/
void bcvTimeToString(i64 iTime, char *zBuf){
  int iDay, iMonth, iYear;
  int iSec, iMin, iHour;
  int Z, A, B, C, D, E, X1;
  int s;

  Z = (int)((iTime + 43200000)/86400000);
  A = (int)((Z - 1867216.25)/36524.25);
  A = Z + 1 + A - (A/4);
  B = A + 1524;
  C = (int)((B - 122.1)/365.25);
  D = (36525*(C&32767))/100;
  E = (int)((B-D)/30.6001);
  X1 = (int)(30.6001*E);
  iDay = B - D - X1;
  iMonth = E<14 ? E-1 : E-13;
  iYear = iMonth>2 ? C - 4716 : C - 4715;

  s = (int)((iTime + 43200000) % 86400000);
  iSec = s/1000.0;
  s = (int)iSec;
  iSec -= s;
  iHour = s/3600;
  s -= iHour*3600;
  iMin = s/60;
  iSec += s - iMin*60;

  sqlite3_snprintf(64, zBuf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
      iYear, iMonth, iDay, iHour, iMin, iSec, (iTime % 1000)
  );
}




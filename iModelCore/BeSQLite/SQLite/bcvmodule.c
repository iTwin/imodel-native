/*
** 2020-06-17
**
******************************************************************************
** This file contains the implementation of the various BCV modules:
**
**   "azure"
**   "google"
**   "aws"
*/

#include "bcv_int.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#define AZURE_DATEHEADER_SIZE     128
#define AZURE_VERSION_HDR         "x-ms-version:2020-10-02"
#define AZURE_MIMETYPE_HDR        "Content-Type:application/octet-stream"
#define AZURE_BLOBTYPE_HDR        "x-ms-blob-type:BlockBlob"
#define AZURE_EXPECT_HDR          "Expect:"


/*
** Type cast to (sqlite3_bcv_container*) for "azure" cloud modules.
**
** zContainer: 
**   Name of Azure container this object is used to create/delete/access.
**
** zSlashAccountSlash:
**
** aKey/nKey:
**   If bSas==0, then aKey points to a buffer containing the binary version
**   of the base64 access-key stored in zKey. nKey is the size of the buffer
**   in bytes. If bSas!=0, then aKey and nKey are both 0.
*/
typedef struct BcvAzure BcvAzure;
struct BcvAzure {
  int bSas;                       /* True to use SAS tokens */
  char *zAccount;                 /* Account name e.g. "devstoreaccount1" */
  char *zContainer;               /* Name of container */
  char *zSlashAccountSlash;
  char *zKey;                     /* SAS. Or base64 access-key. */
  int nMaxResults;
  char *zBase;                    /* Base URI */
  u8 *aKey;                       /* Decoded version of zKey (if bSas==0) */
  int nKey;                       /* Size of aKey[] in bytes */
};

/*
** The first argument passed to this function is a nul-terminated string
** containing data encoded using base64 encoding, with no embedded spaces or
** newlines. This function decodes the base64 and writes the results into
** a buffer obtained by calling sqlite3_malloc(). If successful, (*paOut)
** is set to point to the new buffer, (*pnOut) is set to its size in bytes,
** and SQLITE_OK returned. In this case it is the responsibility of the caller
** to free the returned buffer using sqlite3_free(). 
**
** If an error occurs, an SQLite error code is returned and both (*paOut)
** and (*pnOut) set to zero. In this case, *pzErr may be set to point
** to a new buffer containing an English language error message. It is the 
** responsibility of the caller to eventually free any such buffer using
** sqlite3_free().
*/
static void bcvBase64DecodeRc(
  int *pRc,
  const char *zBase64, 
  u8 **paOut, int *pnOut,
  char **pzErr
){
  static u8 aBase64Decode[256] = {
    0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x3E, 0x80, 0x80, 0x80, 0x3F, 
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 
    0x3C, 0x3D, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 
    0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 
    0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 
    0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
  };

  /* Initialize output parameters */
  *paOut = 0;
  *pnOut = 0;
  *pzErr = 0;

  if( *pRc==SQLITE_OK ){
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
      aOut = (u8*)bcvMallocRc(pRc, nOut);
      if( aOut ){
        for(i=0; i<nIn; i+=4){
          if( aBase64Decode[ zIn[i+0] ]==0x80 
           || aBase64Decode[ zIn[i+1] ]==0x80 
           || aBase64Decode[ zIn[i+2] ]==0x80 
           || aBase64Decode[ zIn[i+3] ]==0x80 
          ){
            *pzErr = sqlite3_mprintf(
                "cannot decode base64 data - illegal character"
            );
            sqlite3_free(aOut);
            *pRc = SQLITE_ERROR;
            return;
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
        *paOut = aOut;
      }
    }else{
      *pzErr = sqlite3_mprintf(
          "cannot decode base64 data - length is %d bytes", nIn
      );
      *pRc = SQLITE_ERROR;
    }
  }
}


static void bcvModuleGenericCb(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  void *pApp
){
  int rc;
  const char *zStatus = 0;
  rc = sqlite3_bcv_request_status(pReq, &zStatus);
  if( rc!=SQLITE_OK ){
    sqlite3_bcv_job_error(pCtx, rc, zStatus);
  }
}

static void bcvModuleFetchCb(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  void *pApp
){
  int rc;
  const char *zStatus = 0;

  rc = sqlite3_bcv_request_status(pReq, &zStatus);
  if( rc!=SQLITE_OK ){
    sqlite3_bcv_job_error(pCtx, rc, zStatus);
  }else{
    const char *zETagHdr = (const char*)pApp;
    int nBody;
    const u8 *aBody;
    const char *zETag;
    aBody = sqlite3_bcv_request_body(pReq, &nBody);
    sqlite3_bcv_job_result(pCtx, aBody, nBody);
    zETag = sqlite3_bcv_request_header(pReq, zETagHdr);
    sqlite3_bcv_job_etag(pCtx, zETag);
  }
}

static void bcvModulePutCb(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  void *pApp
){
  int rc;
  const char *zStatus = 0;
  rc = sqlite3_bcv_request_status(pReq, &zStatus);
  if( rc ){
    sqlite3_bcv_job_error(pCtx, rc, zStatus);
  }else{
    const char *zETagHdr = (const char*)pApp;
    const char *zETag = sqlite3_bcv_request_header(pReq, zETagHdr);
    sqlite3_bcv_job_etag(pCtx, zETag);
  }
}

typedef struct ListFilesCtx ListFilesCtx;
struct ListFilesCtx {
  sqlite3_bcv_job *pCtx;
  const char *zNameTag;
  char *zNextMarker;
  int rc;
};

static void *bcvModuleListNameHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_CONTENT ){
    ListFilesCtx *p = (ListFilesCtx*)simpleXmlGetUserData(parser);
    sqlite3_bcv_job_result(p->pCtx, (const u8*)szValue, -1);
  }
  return bcvModuleListNameHandler;
}

static void *bcvModuleListMarkerHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_CONTENT ){
    ListFilesCtx *p = (ListFilesCtx*)simpleXmlGetUserData(parser);
    if( p->zNextMarker==0 ){
      p->zNextMarker = bcvStrdupRc(&p->rc, szValue);
    }
  }
  return bcvModuleListMarkerHandler;
}

static void *bcvModuleListTagHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_SUBTAG ){
    ListFilesCtx *p = (ListFilesCtx*)simpleXmlGetUserData(parser);
    if( 0==bcvStrcmp(p->zNameTag, szName) ){
      return bcvModuleListNameHandler;
    }else
    if( 0==bcvStrcmp("NextMarker", szName) ){
      return bcvModuleListMarkerHandler;
    }
  }
  return bcvModuleListTagHandler;
}

static void bcvModuleListCb(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  sqlite3_bcv_container *pCont,
  const char *zNameTag,
  void (*xNext)(sqlite3_bcv_container*,sqlite3_bcv_job*,const char*)
){
  int rc;
  const char *zStatus = 0;
  rc = sqlite3_bcv_request_status(pReq, &zStatus);
  if( rc!=SQLITE_OK ){
    sqlite3_bcv_job_error(pCtx, rc, zStatus);
  }else{
    ListFilesCtx lfc;
    SimpleXmlParser x;
    const u8 *aData;
    int nData;

    memset(&lfc, 0, sizeof(lfc));
    lfc.pCtx = pCtx;
    lfc.zNameTag = zNameTag;

    aData = sqlite3_bcv_request_body(pReq, &nData);
    x = simpleXmlCreateParser((const char*)aData, nData);
    simpleXmlPushUserData(x, (void*)&lfc);
    simpleXmlParse(x, bcvModuleListTagHandler);
    simpleXmlDestroyParser(x);

    if( lfc.rc ){
      sqlite3_bcv_job_error(pCtx, lfc.rc, 0);
    }else if( lfc.zNextMarker ){
      xNext(pCont, pCtx, lfc.zNextMarker);
    }

    sqlite3_free(lfc.zNextMarker);
  }
}


/*
** Cross-platform version of gmtime_r().   
*/
#ifdef __WIN32__
static void osGmtime(const time_t *t, struct tm *r){
  /* On windows, gmtime() is threadsafe as it uses TLS */
  struct tm *r2 = gmtime(t);
  *r = *r2;
}
#else
static void osGmtime(const time_t *t, struct tm *r){
  gmtime_r(t, r);
}
#endif

/*
** Argument zBuffer must point to a buffer at least AZURE_DATEHEADER_SIZE
** bytes in size. This function populates the buffer with an 
** Azure "x-ms-date:" header specifying the current GMT date and time.
*/
static void bcvAzureDateHeader(char *zBuffer){
  static const char *nameOfDay[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char *nameOfMonth[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  time_t t = time(NULL);
  struct tm tm;

  memset(&tm, 0, sizeof(tm));
  osGmtime(&t, &tm);

  sprintf(zBuffer, "x-ms-date:%s, %02d %s %d %02d:%02d:%02d GMT",
      nameOfDay[tm.tm_wday],          // Day of week name
      tm.tm_mday,                     // Day
      nameOfMonth[tm.tm_mon],         // Month name
      tm.tm_year + 1900,              // Year
      tm.tm_hour,                     // Hour
      tm.tm_min,                      // Minute
      tm.tm_sec);                     // Second
}

static void bcvAzureClose(sqlite3_bcv_container *pCont){
  if( pCont ){
    BcvAzure *p = (BcvAzure*)pCont;

    assert( p->bSas==0 || p->zAccount==0 );
    assert( p->bSas==0 || p->zSlashAccountSlash==0 );
    assert( p->bSas==0 || p->zContainer==0 );

    sqlite3_free(p->zAccount);
    sqlite3_free(p->zKey);
    sqlite3_free(p->zSlashAccountSlash);
    sqlite3_free(p->zContainer);
    sqlite3_free(p->zBase);
    sqlite3_free(p->aKey);
    sqlite3_free(p);
  }
}

/*
** xOpen for the "azure" module. Parameters are:
**
**   emulator - set to a "hostname:portnumber" combination to use an emulator
**       (either azurite or the legacy Microsoft emulator).
**
**   sas - must be set to either 0 (the default) or 1. If set to 1, use SAS
**       tokens for authorization instead of an accesskey.
**
**   customuri - must be set to either 0 (the default) or 1. If set to 1, use
**       the account name as the base URI
*/
static int bcvAzureOpen(
  void *pCtx,                     /* Context pointer (not used) */
  const char **azParam,           /* Parameters specified with system name */
  const char *zUser,              /* Azure user name */
  const char *zSecret,            /* Authorization (accesskey or SAS) */
  const char *zContainer,         /* Container name */
  sqlite3_bcv_container **pp,     /* OUT: New container handle */
  char **pzErrmsg                 /* OUT: Error message (if any) */
){
  BcvAzure *pNew = 0;
  int bSas = 0;                   /* True if sas=1 is present */
  int bCustomUri = 0;             /* True if customuri=1 is present */
  const char *zEmulator = 0;      /* Value of emulator= option, if any */
  int i;
  int rc = SQLITE_OK;
  int nMaxResults = 200;

  /* Search the azParam[] array for recognized parameter names (currently
  ** "emulator" and "sas"). Set stack variables bSas and zEmulator as
  ** appropriate. Return an error if an unrecognized parameter or illegal
  ** parameter value is encountered.  */
  for(i=0; azParam[i]; i+=2){
    const char *zKey = azParam[i];
    const char *zVal = azParam[i+1];
    if( 0==sqlite3_stricmp(zKey, "maxresults") ){
      nMaxResults = bcvParseInt((const u8*)zVal, -1);
    }else 
    if( 0==sqlite3_stricmp(zKey, "emulator") ){
      zEmulator = zVal;
    }else 
    if( 0==sqlite3_stricmp(zKey, "sas") ){
      if( zVal[1]!='\0' || (zVal[0]!='0' && zVal[0]!='1') ){
        *pzErrmsg = sqlite3_mprintf(
            "bad argument to sas parameter \"%s\" - must be 0 or 1", zVal
        );
        return SQLITE_ERROR;
      }
      bSas = zVal[0]=='1';
    }else if( 0==sqlite3_stricmp(zKey, "customuri") ){
      if( zVal[1]!='\0' || (zVal[0]!='0' && zVal[0]!='1') ){
        *pzErrmsg = sqlite3_mprintf(
            "bad argument to customuri parameter \"%s\" - must be 0 or 1", zVal
        );
        return SQLITE_ERROR;
      }
      bCustomUri = zVal[0]=='1';
    }else{
      *pzErrmsg = sqlite3_mprintf(
          "bad parameter \"%s\" - must be \"emulator\" or \"sas\"", zKey
      );
      return SQLITE_ERROR;
    }
  }

  /* If using SAS tokens for authentication, discard any leading '?' from
  ** the authentication information. Some MS tools and APIs return the SAS
  ** token with a leading '?' and some do not.  */
  if( bSas && zSecret && zSecret[0]=='?' ) zSecret++;

  /* If using the emulator, there is a default account name and access key. */
  if( zEmulator ){
    if( zUser==0 ){
      zUser = "devstoreaccount1";
    }
    if( bSas==0 && zSecret==0 ){
      zSecret = "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuF"
        "q2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==";
    }
  }

  /* customuri=1 requires sas=1 */
  if( bCustomUri && bSas==0 ){
    *pzErrmsg = sqlite3_mprintf("customuri=1 requires sas=1");
    return SQLITE_ERROR;
  }

  /* Unless the "customuri=1" option was specified, a container name is
  ** required.  */
  if( bCustomUri==0 && zContainer==0 ){
    *pzErrmsg = sqlite3_mprintf("container name may not be NULL");
    return SQLITE_ERROR;
  }

  /* Account name is always required */
  if( zUser==0 ){
    *pzErrmsg = sqlite3_mprintf("account name may not be NULL");
    return SQLITE_ERROR;
  }

  pNew = (BcvAzure*)bcvMallocRc(&rc, sizeof(BcvAzure));
  if( pNew ){
    pNew->bSas = bSas;
    pNew->zKey = bcvStrdupRc(&rc, zSecret);;
    pNew->nMaxResults = nMaxResults;

    /* If not using SAS, preserve a copy of the account and container names.
    ** They are required to sign requests. */
    if( bSas==0 ){
      pNew->zAccount = bcvStrdupRc(&rc, zUser);
      pNew->zContainer = bcvStrdupRc(&rc, zContainer);
      if( zEmulator ){
        pNew->zSlashAccountSlash = bcvMprintfRc(&rc, "/%s/%s/", zUser, zUser);
      }else{
        pNew->zSlashAccountSlash = bcvMprintfRc(&rc, "/%s/", zUser);
      }
      bcvBase64DecodeRc(&rc, zSecret, &pNew->aKey, &pNew->nKey, pzErrmsg);
    }

    /* Figure out the base URI */
    if( bCustomUri ){
      int bCName = zContainer && zContainer[0]!='\0';
      pNew->zBase = bcvMprintfRc(&rc, "%s%s%s", zUser,
        (bCName ? "/" : ""), (bCName ? zContainer : "")
      );
    }else if( zEmulator ){
      pNew->zBase = bcvMprintfRc(&rc, 
          "http://%s/%s/%s", zEmulator, zUser, zContainer
      );
    }else{
      pNew->zBase = bcvMprintfRc(&rc,
          "https://%s.blob.core.windows.net/%s", zUser, zContainer
      );
    }
  }

  if( rc!=SQLITE_OK ){
    bcvAzureClose((sqlite3_bcv_container*)pNew);
    pNew = 0;
  }

  *pp = (sqlite3_bcv_container*)pNew;
  return rc;
}

/*
** Set the URI of request pReq based on the printf() style format string
** and its arguments passed as the third and subsequent parameters.
*/
static void bcvUriPrintf(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  const char *zFmt, ...           /* printf() format string + args */
){
  char *zRet;
  va_list ap;
  va_start(ap, zFmt);
  zRet = sqlite3_vmprintf(zFmt, ap);
  va_end(ap);
  if( zRet==0 ){
    sqlite3_bcv_job_error(pCtx, SQLITE_NOMEM, 0);
  }else{
    sqlite3_bcv_request_set_uri(pReq, zRet);
    sqlite3_free(zRet);
  }
}

/*
** Like bcvUriPrintf(), but for sqlite3_bcv_request_set_log().
*/
static void bcvUriLogPrintf(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  const char *zFmt, ...           /* printf() format string + args */
){
  char *zRet;
  va_list ap;
  va_start(ap, zFmt);
  zRet = sqlite3_vmprintf(zFmt, ap);
  va_end(ap);
  if( zRet==0 ){
    sqlite3_bcv_job_error(pCtx, SQLITE_NOMEM, 0);
  }else{
    sqlite3_bcv_request_set_log(pReq, zRet);
    sqlite3_free(zRet);
  }
}

static void bcvHdrPrintf(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  const char *zFmt, ...           /* printf() format string + args */
){
  char *zRet;
  va_list ap;
  va_start(ap, zFmt);
  zRet = sqlite3_vmprintf(zFmt, ap);
  va_end(ap);
  if( zRet==0 ){
    sqlite3_bcv_job_error(pCtx, SQLITE_NOMEM, 0);
  }else{
    sqlite3_bcv_request_set_header(pReq, zRet);
    sqlite3_free(zRet);
  }
}

/*
** Encode the binary data in buffer aIn[] (size nIn bytes) as a
** nul-terminated base64 string. Return a pointer to the buffer. It is
** the responsibility of the caller to eventually free the returned 
** buffer using sqlite3_free().
*/
char *bcvBase64Encode(const unsigned char *aIn, int nIn){
  static const char aBase64Encode[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
  int nOut = ((nIn + 2) / 3) * 4;
  char *zOut = sqlite3_malloc(nOut+1);

  if( zOut ){
    int i;
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
  }

  return zOut;
}


static void bcvAzureAuthHeader(
  BcvAzure *p,
  sqlite3_bcv_job *pCtx,
  sqlite3_bcv_request *pReq, 
  const char *zFmt, ...           /* printf() format string + args */
){
  unsigned char aDigest[SHA256_DIGEST_LENGTH];
  u32 nDigest = SHA256_DIGEST_LENGTH;
  HMAC_CTX *pHmac;
  char *zSig = 0;
  char *zAuth = 0;
  char *zStringToSign = 0;

  va_list ap;
  va_start(ap, zFmt);
  zStringToSign = sqlite3_vmprintf(zFmt, ap);
  if( zStringToSign ){
    pHmac = HMAC_CTX_new();
    HMAC_CTX_reset(pHmac);
    HMAC_Init_ex(pHmac, p->aKey, p->nKey, EVP_sha256(), NULL);
    HMAC_Update(pHmac, (const u8*)zStringToSign, strlen(zStringToSign));
    HMAC_Final(pHmac, aDigest, &nDigest);
    HMAC_CTX_free(pHmac);
    zSig = bcvBase64Encode(aDigest, (int)nDigest);
    zAuth = sqlite3_mprintf("Authorization: SharedKey %s:%s", p->zAccount,zSig);
  }
  sqlite3_bcv_request_set_header(pReq, zAuth);
  if( zStringToSign==0 || zAuth==0 || zSig==0 ){
    sqlite3_bcv_job_error(pCtx, SQLITE_NOMEM, 0);
  }
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
  sqlite3_free(zSig);
}

struct AzureHeadCtx {
  BcvAzure *p;
  u8 aMd5[MD5_DIGEST_LENGTH];
  char *zFile;
};
typedef struct AzureHeadCtx AzureHeadCtx;

static void bcvAzureHeadFetch(
    BcvAzure*,int,sqlite3_bcv_job*,const char*,const void*
);

static void bcvAzureHeadCb(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  void *p
){
  AzureHeadCtx *pHead = (AzureHeadCtx*)p;
  int rc;
  const char *zStatus = 0;
  int bOmit = 0;

  rc = sqlite3_bcv_request_status(pReq, &zStatus);
  if( rc!=SQLITE_OK ){
    sqlite3_bcv_job_error(pCtx, rc, zStatus);
    bOmit = 1;
  }else{
    const char *zBase64 = sqlite3_bcv_request_header(pReq, "content-md5");
    if( zBase64 ){
      char *zErr = 0;
      u8 *aMd5 = 0;
      int nMd5 = 0;
      bcvBase64DecodeRc(&rc, zBase64, &aMd5, &nMd5, &zErr);
      if( rc!=SQLITE_OK ){
        sqlite3_bcv_job_error(pCtx, rc, zErr);
        bOmit = 1;
      }else if( nMd5==MD5_DIGEST_LENGTH && memcmp(pHead->aMd5, aMd5, nMd5)==0 ){
        sqlite3_bcv_job_error(pCtx, HTTP_PRECONDITION_FAILED, 0);
        bOmit = 1;
      }

      sqlite3_free(zErr);
      sqlite3_free(aMd5);
    }
  }

  if( bOmit==0 ){
    bcvAzureHeadFetch(pHead->p, 0, pCtx, pHead->zFile, 0);
  }
  sqlite3_free(pHead);
}

/*
** Issue either a HEAD or GET request to cloud storage using the azure
** handle passed as the first argument.
*/
static void bcvAzureHeadFetch(
  BcvAzure *p,
  int bHead,                      /* True for HEAD, false for GET */
  sqlite3_bcv_job *pCtx,
  const char *zFile,
  const void *zETag
){
  sqlite3_bcv_request *pReq = 0;
  char zDate[AZURE_DATEHEADER_SIZE];
  char *zIfNoneMatch = 0;

  if( bHead==0 ){
    pReq = sqlite3_bcv_job_request(pCtx, (void*)"ETag", bcvModuleFetchCb);
  }else{
    AzureHeadCtx *pHead;
    int nFile = strlen(zFile);
    int rc = SQLITE_OK;
    pHead = (AzureHeadCtx*)bcvMallocRc(&rc, sizeof(AzureHeadCtx) + nFile + 1);
    if( pHead==0 ){
      sqlite3_bcv_job_error(pCtx, SQLITE_NOMEM, 0);
    }else{
      pReq = sqlite3_bcv_job_request(pCtx, pHead, bcvAzureHeadCb);
      if( pReq==0 ){
        sqlite3_free(pHead);
      }else{
        pHead->p = p;
        memcpy(pHead->aMd5, zETag, MD5_DIGEST_LENGTH);
        pHead->zFile = (char*)&pHead[1];
        memcpy(pHead->zFile, zFile, nFile);
        sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_HEAD);
      }
    }
  }
  if( pReq==0 ) return;

  bcvAzureDateHeader(zDate);
  sqlite3_bcv_request_set_header(pReq, zDate);
  sqlite3_bcv_request_set_header(pReq, AZURE_VERSION_HDR);
  if( bHead==0 && zETag ){
    zIfNoneMatch = sqlite3_mprintf("If-None-Match:%s", zETag);
    sqlite3_bcv_request_set_header(pReq, zIfNoneMatch);
  }
  if( p->bSas ){
    bcvUriPrintf(pCtx, pReq, "%s/%s?%s", p->zBase, zFile, p->zKey);
    bcvUriLogPrintf(pCtx, pReq, "%s/%s?...", p->zBase, zFile);
  }else{
    bcvUriPrintf(pCtx, pReq, "%s/%s", p->zBase, zFile);
    bcvAzureAuthHeader(p, pCtx, pReq,
        "%s\n\n\n\n\n\n\n\n\n%s\n\n\n%s\n%s\n%s%s/%s", 
        (bHead==0 ? "GET" : "HEAD"),
        (bHead==0 ? zETag : 0),
        zDate, AZURE_VERSION_HDR, 
        p->zSlashAccountSlash, p->zContainer, zFile
    );
  }

  sqlite3_free(zIfNoneMatch);
}

static void bcvAzureFetch(
  sqlite3_bcv_container *pCont,
  sqlite3_bcv_job *pCtx, 
  const char *zFile,
  int flags,
  const void *zETag, int nETag
){
  BcvAzure *p = (BcvAzure*)pCont;
  if( flags & BCV_MODULE_CONDITION_MD5 ){
    bcvAzureHeadFetch(p, 1, pCtx, zFile, zETag);
  }else{
    assert( (flags & BCV_MODULE_CONDITION_ETAG) || zETag==0 );
    bcvAzureHeadFetch(p, 0, pCtx, zFile, zETag);
  }
}

static void bcvAzurePut(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx,
  const char *zFile, 
  const unsigned char *aData, int nData,
  const char *zETag
){
  BcvAzure *p = (BcvAzure*)pCont;
  sqlite3_bcv_request *pReq = 0;
  char zDate[AZURE_DATEHEADER_SIZE];
  const char *zIfNone = 0;

  pReq = sqlite3_bcv_job_request(pCtx, (void*)"ETag", bcvModulePutCb);
  if( pReq==0 ) return;

  bcvAzureDateHeader(zDate);
  sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_PUT);
  sqlite3_bcv_request_set_header(pReq, AZURE_BLOBTYPE_HDR);
  sqlite3_bcv_request_set_header(pReq, AZURE_MIMETYPE_HDR);
  sqlite3_bcv_request_set_header(pReq, zDate);
  sqlite3_bcv_request_set_header(pReq, AZURE_VERSION_HDR);
  sqlite3_bcv_request_set_header(pReq, AZURE_EXPECT_HDR);
  if( zETag ){
    int rc = SQLITE_OK;
    char *zHdr = 0;
    if( zETag[0]=='*' && zETag[1]=='\0' ){
      zHdr = bcvMprintfRc(&rc, "If-None-Match:*");
      zIfNone = zETag;
      zETag = 0;
    }else{
      zHdr = bcvMprintfRc(&rc, "If-Match:%s", zETag);
    }
    if( rc==SQLITE_OK ){
      sqlite3_bcv_request_set_header(pReq, zHdr);
    }else{
      sqlite3_bcv_job_error(pCtx, SQLITE_NOMEM, 0);
    }
    sqlite3_free(zHdr);
  }
  sqlite3_bcv_request_set_body(pReq, aData, nData);

  if( p->bSas ){
    bcvUriPrintf(pCtx, pReq, "%s/%s?%s", p->zBase, zFile, p->zKey);
    bcvUriLogPrintf(pCtx, pReq, "%s/%s?...", p->zBase, zFile);
  }else{
    bcvUriPrintf(pCtx, pReq, "%s/%s", p->zBase, zFile);
    bcvAzureAuthHeader(p, pCtx, pReq,
        "PUT\n\n\n%d\n\n%s\n\n\n%s\n%s\n\n\n%s\n%s\n%s\n%s%s/%s", 
        nData, 
        "application/octet-stream", 
        zETag ? zETag : "",
        zIfNone ? zIfNone : "",
        AZURE_BLOBTYPE_HDR, 
        zDate, AZURE_VERSION_HDR,
        p->zSlashAccountSlash,
        p->zContainer, zFile
    );
  }
}

static void bcvAzureDelete(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx, 
  const char *zFile,
  const char *zIfMatch
){
  BcvAzure *p = (BcvAzure*)pCont;
  sqlite3_bcv_request *pReq = 0;
  char zDate[AZURE_DATEHEADER_SIZE];

  pReq = sqlite3_bcv_job_request(pCtx, 0, bcvModuleGenericCb);
  if( pReq==0 ) return;

  bcvAzureDateHeader(zDate);
  sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_DELETE);
  sqlite3_bcv_request_set_header(pReq, zDate);
  sqlite3_bcv_request_set_header(pReq, AZURE_VERSION_HDR);
  if( zIfMatch ) bcvHdrPrintf(pCtx, pReq, "If-Match:%s", zIfMatch);

  if( p->bSas ){
    bcvUriPrintf(pCtx, pReq, "%s/%s?%s", p->zBase, zFile, p->zKey);
    bcvUriLogPrintf(pCtx, pReq, "%s/%s?...", p->zBase, zFile);
  }else{
    bcvUriPrintf(pCtx, pReq, "%s/%s", p->zBase, zFile);
    bcvAzureAuthHeader(p, pCtx, pReq,
        "DELETE\n\n\n\n\n\n\n\n%s\n\n\n\n%s\n%s\n%s%s/%s", 
        zIfMatch ? zIfMatch : "",
        zDate, AZURE_VERSION_HDR,
        p->zSlashAccountSlash,
        p->zContainer, zFile
    );
  }
}

static void bcvAzureListCb(sqlite3_bcv_job*, sqlite3_bcv_request*, void*);
static void bcvAzureListFiles(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx, 
  const char *zMarker
){
  BcvAzure *p = (BcvAzure*)pCont;
  sqlite3_bcv_request *pReq = 0;
  char zDate[AZURE_DATEHEADER_SIZE];
  const char *zPrefix = sqlite3_bcv_job_prefix(pCtx);

  pReq = sqlite3_bcv_job_request(pCtx, (void*)p, bcvAzureListCb);
  if( pReq==0 ) return;

  bcvAzureDateHeader(zDate);
  sqlite3_bcv_request_set_header(pReq, zDate);
  sqlite3_bcv_request_set_header(pReq, AZURE_VERSION_HDR);

  bcvUriPrintf(pCtx, pReq, 
      "%s?restype=container&comp=list&maxresults=%d&marker=%s&prefix=%s%s%s",
      p->zBase, p->nMaxResults, zMarker, (zPrefix ? zPrefix : ""),
      (p->bSas ? "&" : ""), (p->bSas ? p->zKey : "")
  );
  if( p->bSas==0 ){
    bcvAzureAuthHeader(p, pCtx, pReq,
        "GET\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s\ncomp:list"
        "\nmarker:%s\nmaxresults:%d\nprefix:%s\nrestype:container",
        zDate, AZURE_VERSION_HDR, 
        p->zSlashAccountSlash, p->zContainer, 
        zMarker, p->nMaxResults, (zPrefix ? zPrefix : "")
    );
  }else{
    bcvUriLogPrintf(pCtx, pReq, 
        "%s?restype=container&comp=list&maxresults=%d&marker=%s&...",
        p->zBase, p->nMaxResults, zMarker
    );
  }
}

static void bcvAzureListCb(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  void *pApp
){
  sqlite3_bcv_container *pCont = (sqlite3_bcv_container*)pApp;
  bcvModuleListCb(pCtx, pReq, pCont, "Name", bcvAzureListFiles);
}

static void bcvAzureList(
  sqlite3_bcv_container *pCont,
  sqlite3_bcv_job *pCtx
){
  bcvAzureListFiles(pCont, pCtx, "");
}

static void bcvAzureCreate(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx
){
  BcvAzure *p = (BcvAzure*)pCont;
  sqlite3_bcv_request *pReq = 0;
  char zDate[AZURE_DATEHEADER_SIZE];

  pReq = sqlite3_bcv_job_request(pCtx, 0, bcvModuleGenericCb);
  if( pReq==0 ) return;

  bcvAzureDateHeader(zDate);
  sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_PUT);
  sqlite3_bcv_request_set_header(pReq, zDate);
  sqlite3_bcv_request_set_header(pReq, AZURE_VERSION_HDR);

  if( p->bSas ){
    bcvUriPrintf(pCtx, pReq, "%s?restype=container&%s", p->zBase, p->zKey);
    bcvUriLogPrintf(pCtx, pReq, "%s?restype=container&...", p->zBase);
  }else{
    bcvUriPrintf(pCtx, pReq, "%s?restype=container", p->zBase);
    bcvAzureAuthHeader(p, pCtx, pReq,
        "PUT\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s\nrestype:container", 
        zDate, AZURE_VERSION_HDR, p->zSlashAccountSlash, p->zContainer
    );
  }
}

static void bcvAzureDestroy(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx
){
  BcvAzure *p = (BcvAzure*)pCont;
  sqlite3_bcv_request *pReq = 0;
  char zDate[AZURE_DATEHEADER_SIZE];

  pReq = sqlite3_bcv_job_request(pCtx, 0, bcvModuleGenericCb);
  if( pReq==0 ) return;

  bcvAzureDateHeader(zDate);
  sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_DELETE);
  sqlite3_bcv_request_set_header(pReq, zDate);
  sqlite3_bcv_request_set_header(pReq, AZURE_VERSION_HDR);

  if( p->bSas ){
    bcvUriPrintf(pCtx, pReq, "%s?restype=container&%s", p->zBase, p->zKey);
    bcvUriLogPrintf(pCtx, pReq, "%s?restype=container&...", p->zBase);
  }else{
    bcvUriPrintf(pCtx, pReq, "%s?restype=container", p->zBase);
    bcvAzureAuthHeader(p, pCtx, pReq,
        "DELETE\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s\nrestype:container", 
        zDate, AZURE_VERSION_HDR, p->zSlashAccountSlash, p->zContainer
    );
  }
}

typedef struct BcvGoogle BcvGoogle;
struct BcvGoogle {
  const char *zUser;
  const char *zSecret;
  const char *zCont;
  const char *zBase;
};

static void bcvGoogleClose(sqlite3_bcv_container *pCont){
  sqlite3_free(pCont);
}

static int bcvGoogleOpen(
  void *pCtx,                     /* Context pointer (not used) */
  const char **azParam,           /* Parameters specified with system name */
  const char *zUser,              /* Google project name */
  const char *zSecret,            /* Authorization (access token) */
  const char *zContainer,         /* Container name */
  sqlite3_bcv_container **pp,     /* OUT: New container handle */
  char **pzErrmsg                 /* OUT: Error message (if any) */
){
  BcvGoogle *pNew;
  int rc = SQLITE_OK;

  int nUser = bcvStrlen(zUser);
  int nSecret = bcvStrlen(zSecret);
  int nContainer = bcvStrlen(zContainer);
  int nBase = bcvStrlen("https://storage.googleapis.com/") + nContainer;
  int nByte = sizeof(BcvGoogle) + nUser + nSecret + nContainer + nBase + 4;

  pNew = (BcvGoogle*)bcvMallocRc(&rc, nByte);
  if( pNew ){
    char *zCsr = (char*)&pNew[1];
    memset(pNew, 0, nByte);
    memcpy(zCsr, zUser, nUser);
    pNew->zUser = zCsr;
    zCsr += nUser+1;
    memcpy(zCsr, zSecret, nSecret);
    pNew->zSecret = zCsr;
    zCsr += nSecret+1;
    memcpy(zCsr, zContainer, nContainer);
    pNew->zCont = zCsr;
    zCsr += nContainer+1;
    sqlite3_snprintf(
        nBase+1, zCsr, "https://storage.googleapis.com/%s", zContainer
    );
    pNew->zBase = zCsr;
  }
  *pp = (sqlite3_bcv_container*)pNew;
  return rc;
}

static void bcvGoogleFetch(
  sqlite3_bcv_container *pCont,
  sqlite3_bcv_job *pCtx, 
  const char *zFile,
  int flags,
  const void *zETag, int nETag
){
  BcvGoogle *p = (BcvGoogle*)pCont;
  sqlite3_bcv_request *pReq = 0;

  pReq = sqlite3_bcv_job_request(pCtx, 
      (void*)"x-goog-generation", bcvModuleFetchCb
  );
  bcvUriPrintf(pCtx, pReq, "%s/%s", p->zBase, zFile);
  bcvHdrPrintf(pCtx, pReq, "Authorization: Bearer %s", p->zSecret);
  if( flags & BCV_MODULE_CONDITION_ETAG ){
    bcvHdrPrintf(pCtx, pReq, "If-None-Match: %s", zETag);
  }
  sqlite3_bcv_request_set_header(pReq, AZURE_MIMETYPE_HDR);
}

static void bcvGooglePut(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx,
  const char *zFile, 
  const unsigned char *aData, int nData,
  const char *zETag
){
  BcvGoogle *p = (BcvGoogle*)pCont;
  sqlite3_bcv_request *pReq = 0;

  pReq = sqlite3_bcv_job_request(pCtx,
      (void*)"x-goog-generation", bcvModulePutCb
  );
  sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_PUT);
  bcvUriPrintf(pCtx, pReq, "%s/%s", p->zBase, zFile);
  bcvHdrPrintf(pCtx, pReq, "Authorization: Bearer %s", p->zSecret);
  sqlite3_bcv_request_set_header(pReq, AZURE_MIMETYPE_HDR);
  if( zETag ){
    bcvHdrPrintf(pCtx, pReq, "x-goog-if-generation-match: %s", zETag);
  }
  sqlite3_bcv_request_set_body(pReq, aData, nData);
}
 
static void bcvGoogleDelete(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx, 
  const char *zFile,
  const char *zETag
){
  BcvGoogle *p = (BcvGoogle*)pCont;
  sqlite3_bcv_request *pReq = 0;

  pReq = sqlite3_bcv_job_request(pCtx, 0, bcvModuleGenericCb);
  sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_DELETE);
  bcvUriPrintf(pCtx, pReq, "%s/%s", p->zBase, zFile);
  bcvHdrPrintf(pCtx, pReq, "Authorization: Bearer %s", p->zSecret);
  sqlite3_bcv_request_set_header(pReq, AZURE_MIMETYPE_HDR);
  if( zETag ){
    bcvHdrPrintf(pCtx, pReq, "x-goog-if-generation-match: %s", zETag);
  }
}

static void bcvGoogleListCb(sqlite3_bcv_job*, sqlite3_bcv_request*, void*);
static void bcvGoogleListFiles(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx, 
  const char *zMarker
){
  const int nMax = 200;
  BcvGoogle *p = (BcvGoogle*)pCont;
  sqlite3_bcv_request *pReq = 0;

  pReq = sqlite3_bcv_job_request(pCtx, (void*)p, bcvGoogleListCb);
  bcvUriPrintf(pCtx, pReq, "%s?max-keys=%d%s%s", p->zBase, nMax,
      zMarker ? "&marker=" : "", zMarker ? zMarker : ""
  );
  bcvHdrPrintf(pCtx, pReq, "Authorization: Bearer %s", p->zSecret);
}

static void bcvGoogleListCb(
  sqlite3_bcv_job *pCtx, 
  sqlite3_bcv_request *pReq, 
  void *pApp
){
  sqlite3_bcv_container *pCont = (sqlite3_bcv_container*)pApp;
  bcvModuleListCb(pCtx, pReq, pCont, "Key", bcvGoogleListFiles);
}

static void bcvGoogleList(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx
){
  bcvGoogleListFiles(pCont, pCtx, 0);
}

static void bcvGoogleCreate(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx
){
  BcvGoogle *p = (BcvGoogle*)pCont;
  sqlite3_bcv_request *pReq = 0;

  pReq = sqlite3_bcv_job_request(pCtx, 0, bcvModuleGenericCb);
  sqlite3_bcv_request_set_method(pReq, SQLITE_BCV_METHOD_PUT);
  sqlite3_bcv_request_set_uri(pReq, p->zBase);
  bcvHdrPrintf(pCtx, pReq, "Authorization: Bearer %s", p->zSecret);
  bcvHdrPrintf(pCtx, pReq, "x-goog-project-id: %s", p->zUser);
}

static void bcvGoogleDestroy(
  sqlite3_bcv_container *pCont, 
  sqlite3_bcv_job *pCtx
){
}

int bcvInstallBuiltinModules(void){
  static sqlite3_bcv_module mAzure = {
    bcvAzureOpen,
    bcvAzureClose,
    bcvAzureFetch,
    bcvAzurePut,
    bcvAzureDelete,
    bcvAzureList,
    bcvAzureCreate,
    bcvAzureDestroy
  };
  static sqlite3_bcv_module mGoogle = {
    bcvGoogleOpen,
    bcvGoogleClose,
    bcvGoogleFetch,
    bcvGooglePut,
    bcvGoogleDelete,
    bcvGoogleList,
    bcvGoogleCreate,
    bcvGoogleDestroy
  };

  int rc;
  rc = bcvCreateModuleUnsafe("azure", &mAzure, 0);
  if( rc==SQLITE_OK ){
    rc = bcvCreateModuleUnsafe("google", &mGoogle, 0);
  }
  return rc;
}


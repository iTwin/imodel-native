/*
** 2020-05-12
**
******************************************************************************
**
*/

#include "bcvutil.h"
#include "bcv_int.h"
#include "bcv_socket.h"
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
#define O_BINARY 0
#include <unistd.h>
#endif

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#ifdef __WIN32__
# define BCV_DEFAULT_VFS "win32"
#else
# define BCV_DEFAULT_VFS "unix"
#endif

typedef int(*bcv_progress_callback)(void*, sqlite3_int64, sqlite3_int64);

struct sqlite3_bcv {
  AccessPoint ap;                 /* Address of cloud storage */

  /* Copies of the three strings passed to sqlite3_bcv_open() */
  char *zCont;                    /* Container name */
  char *zUser;                    /* User name */
  char *zKey;                     /* Authentication key */

  char *zErrmsg;                  /* Error message from most recent API call */
  int errCode;                    /* Error code from most recent API call */

  int bVerbose;                   /* True to use verbose curl handles */
  void *pProgressCtx;
  bcv_progress_callback xProgress;
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

void *bcvMalloc(int nByte){
  void *p = sqlite3_malloc(nByte);
  assert( nByte!=0 );
  if( p==0 ) fatal_oom_error();
  return p;
}

void *bcvRealloc(void *a, int nByte){
  void *p = sqlite3_realloc(a, nByte);
  assert( nByte!=0 );
  if( p==0 ) fatal_oom_error();
  return p;
}

void *bcvMallocZero(int nByte){
  void *p = bcvMalloc(nByte);
  memset(p, 0, nByte);
  return p;
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

int parse_number(const char *zNum, int *piVal){
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

/*
** Hex encode the blob passed via the only two arguments to this function
** into buffer aBuf[].
*/
void hex_encode(const unsigned char *aIn, int nIn, char *aBuf){
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
    if( a>=sizeof(aCharMap) || aCharMap[a]<0 ) return 1;
    if( b>=sizeof(aCharMap) || aCharMap[b]<0 ) return 1;
    aBuf[i/2] = (aCharMap[a]<<4) + aCharMap[b];
  }

  return 0;
}

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
static int bcvBase64Decode(
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
  unsigned char *aOut = 0;
  const unsigned char *zIn = (const unsigned char*)zBase64;
  int nIn = strlen((const char*)zIn);

  /* Initialize output parameters */
  *paOut = 0;
  *pnOut = 0;
  *pzErr = 0;
 
  if( nIn>0 && (nIn%4)==0 ){
    int iOut = 0;
    int i;

    /* Figure out the length in bytes of the output */
    int nOut = (nIn/4) * 3;
    if( zBase64[nIn-1]=='=' ) nOut--;
    if( zBase64[nIn-2]=='=' ) nOut--;

    /* Allocate space for the output */
    aOut = (u8*)sqlite3_malloc(nOut);
    if( aOut==0 ){
      return SQLITE_NOMEM;
    }else{
      for(i=0; i<nIn; i+=4){
        if( aBase64Decode[ zIn[i+0] ]==0x80 
            || aBase64Decode[ zIn[i+1] ]==0x80 
            || aBase64Decode[ zIn[i+2] ]==0x80 
            || aBase64Decode[ zIn[i+3] ]==0x80 
        ){
          *pzErr = bcvMprintf("cannot decode base64 data - illegal character");
          sqlite3_free(aOut);
          return SQLITE_ERROR;
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
    *pzErr = bcvMprintf("cannot decode base64 data - length is %d bytes", nIn);
    return SQLITE_ERROR;
  }

  return SQLITE_OK;
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

int bcvAccessPointInit(
  AccessPoint *p,                 /* AccessPoint object to populate */
  int eType,                      /* SQLITE_BCV_* constant */
  const char *zUser,              /* Cloud storage user name */
  const char *zKey,               /* Key or SAS used for cloud storage auth. */
  char **pzErr
){
  int rc = SQLITE_OK;
  memset(p, 0, sizeof(AccessPoint));

  switch( eType ){
    case SQLITE_BCV_AZURE:
      p->zAccount = zUser;
      p->zAccessKey = zKey;
      p->eStorage = BCV_STORAGE_AZURE;
      break;
    case SQLITE_BCV_AZURE_EMU:
      p->zEmulator = zUser;
      p->zAccount = "devstoreaccount1";
      p->zAccessKey = 
        "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuF"
        "q2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==";
      p->eStorage = BCV_STORAGE_AZURE;
      break;
    case SQLITE_BCV_AZURE_SAS:
      p->zAccount = zUser;
      p->zSas = bcvStrdup(zKey);
      p->eStorage = BCV_STORAGE_AZURE;
      break;
    case SQLITE_BCV_AZURE_EMU_SAS:
      p->zEmulator = zUser;
      p->zAccount = "devstoreaccount1";
      p->zSas = bcvStrdup(zKey);
      p->eStorage = BCV_STORAGE_AZURE;
      break;
    default:
      p->eStorage = BCV_STORAGE_GOOGLE;
      p->zAccount = zUser;
      p->zSas = bcvStrdup(zKey);
      assert( eType==SQLITE_BCV_GOOGLE );
      break;
  }

  if( p->eStorage==BCV_STORAGE_AZURE ){
    if( p->zAccessKey ){
      rc = bcvBase64Decode(p->zAccessKey, &p->aKey, &p->nKey, pzErr);
    }
    if( rc==SQLITE_OK ){
      if( p->zEmulator ){
        p->zSlashAccountSlash = bcvMprintf("/%s/%s/", p->zAccount, p->zAccount);
      }else{
        p->zSlashAccountSlash = bcvMprintf("/%s/", p->zAccount);
      }
      if( p->zSlashAccountSlash==0 ){
        rc = SQLITE_NOMEM;
      }
    }
  }

  return rc;
}

size_t curlHeaderFunction(char *buf, size_t sz, size_t n, void *pUser){
  CurlRequest *p = (CurlRequest*)pUser;
  int nByte = (sz * n);
  if( nByte>5 && sqlite3_strnicmp(buf, "http/", 5)==0 ){
    int nCopy = nByte;
    while( nCopy>0 && bcv_isspace(buf[nCopy-1]) ) nCopy--;
    sqlite3_free(p->zStatus);
    p->zStatus = bcvMprintf("%.*s", nCopy, buf);
  }else if( p->bGen==0 && nByte>6 && sqlite3_strnicmp(buf, "ETag: ", 6)==0 ){
    p->zETag = bcvMprintf("%.*s", nByte-8, &buf[6]);
  }else if( nByte>19 && sqlite3_strnicmp(buf, "x-goog-generation: ", 19)==0 ){
    sqlite3_free(p->zETag);
    p->zETag = bcvMprintf("%.*s", nByte-21, &buf[19]);
    p->bGen = 1;
  }
  return (size_t)nByte;
}

void curlRequestInit(CurlRequest *p, int bVerbose){
  memset(p, 0, sizeof(CurlRequest));
  p->pCurl = curl_easy_init();
  if( bVerbose ){
    p->bVerbose = 1;
    curl_easy_setopt(p->pCurl, CURLOPT_VERBOSE, 1L);
  }
  curl_easy_setopt(p->pCurl, CURLOPT_HEADERFUNCTION, curlHeaderFunction);
  curl_easy_setopt(p->pCurl, CURLOPT_HEADERDATA, (void*)p);
}

void curlRequestReset(CurlRequest *p){
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
  sqlite3_free(p->zHttpUrl);
  p->zStatus = 0;
  p->zETag = 0;
  p->zHttpUrl = 0;
}

void curlRequestFinalize(CurlRequest *p){
  curl_easy_cleanup(p->pCurl);
  curl_slist_free_all(p->pList);
  sqlite3_free(p->zStatus);
  sqlite3_free(p->zETag);
  sqlite3_free(p->zHttpUrl);
  memset(p, 0, sizeof(CurlRequest));
}

void azure_date_header(char *zBuffer){
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

char *azure_auth_header(AccessPoint *pA, const char *zStringToSign){
  unsigned char aDigest[SHA256_DIGEST_LENGTH];
  u32 nDigest = SHA256_DIGEST_LENGTH;
  HMAC_CTX *pHmac;
  char *zSig = 0;
  char *zAuth = 0;

  assert( pA->bDoNotUse==0 );
  
  pHmac = HMAC_CTX_new();
  HMAC_CTX_reset(pHmac);
  HMAC_Init_ex(pHmac, pA->aKey, pA->nKey, EVP_sha256(), NULL);
  HMAC_Update(pHmac, (const u8*)zStringToSign, strlen(zStringToSign));
  HMAC_Final(pHmac, aDigest, &nDigest);
  HMAC_CTX_free(pHmac);

  zSig = bcvBase64Encode(aDigest, (int)nDigest);
  zAuth = sqlite3_mprintf("Authorization: SharedKey %s:%s", pA->zAccount, zSig);

  sqlite3_free(zSig);
  return zAuth;
}

char *azure_base_uri(AccessPoint *p){
  char *zRet;
  if( p->zEmulator ){
    zRet = bcvMprintf("http://%s/%s/", p->zEmulator, p->zAccount);
  }else{
    zRet = bcvMprintf("https://%s.blob.core.windows.net/", p->zAccount);
  }
  return zRet;
}


/*
** Version of curlFetchBlob() for Azure storage.
*/
static void curlAzureFetchBlob(
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
  char *zDebugUrl = 0;

  assert( p->zHttpUrl==0 );

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s/%s", zBase, zContainer, zFile);
  zRes = bcvMprintf("%s%s/%s", pA->zSlashAccountSlash, zContainer, zFile);

  assert( pA->zAccessKey || pA->zSas );
  if( pA->zAccessKey ){
    zStringToSign = bcvMprintf(
        "GET\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s", 
        zDate, BCV_AZURE_VERSION_HDR, zRes
    );
    zAuth = azure_auth_header(pA, zStringToSign);
    p->pList = curl_slist_append(p->pList, zAuth);
    zDebugUrl = zUrl;
  }else{
    zDebugUrl = bcvMprintf("%s?<sas>", zUrl);
    zUrl = bcvMprintf("%z?%s", zUrl, pA->zSas);
  }
  p->zHttpUrl = zDebugUrl;
  p->eHttpMethod = BCV_HTTP_GET;

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zStringToSign);
  sqlite3_free(zRes);
  sqlite3_free(zAuth);
  if( zUrl!=zDebugUrl ) sqlite3_free(zUrl);
}

static void curlAzurePutBlob(
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
  char *zDebugUrl = 0;

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s/%s", zBase, zContainer, zFile);

  if( pA->zAccessKey ){
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
    p->pList = curl_slist_append(p->pList, zAuth);
    zDebugUrl = zUrl;
  }else{
    zDebugUrl = bcvMprintf("%s?<sas> %s%s%s", zUrl, 
        zETag?"(if-match=":"", zETag?zETag:"", zETag?")":""
    );
    zUrl = bcvMprintf("%z?%s", zUrl, pA->zSas);
  }
  assert( p->zHttpUrl==0 );
  p->zHttpUrl = zDebugUrl;
  p->eHttpMethod = BCV_HTTP_PUT;

  if( zETag ){
    char *zHdr = bcvMprintf("If-Match:%s", zETag);
    p->pList = curl_slist_append(p->pList, zHdr);
    sqlite3_free(zHdr);
  }

  p->pList = curl_slist_append(p->pList, BCV_AZURE_BLOBTYPE_HDR);
  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_MIMETYPE_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)nByte);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
  if( zUrl!=zDebugUrl ) sqlite3_free(zUrl);
}

/*
** Version of curlFetchBlob() for Google storage.
*/
static void curlGoogleFetchBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zAuth = 0;                /* Authorization header */

  zUrl = bcvMprintf("https://storage.googleapis.com/%s/%s", zContainer, zFile);

  zAuth = bcvMprintf("Authorization: Bearer %s", pA->zSas);
  p->pList = curl_slist_append(p->pList, zAuth);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_MIMETYPE_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  assert( p->zHttpUrl==0 );
  p->zHttpUrl = zUrl;
  p->eHttpMethod = BCV_HTTP_GET;
  sqlite3_free(zAuth);
}

static void curlGooglePutBlob(
  CurlRequest *p,
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile,
  const char *zETag,              /* If-Match tag (or NULL) */
  int nByte                       /* Size of file in bytes */
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zAuth = 0;                /* Authorization header */

  zUrl = bcvMprintf("https://storage.googleapis.com/%s/%s", zContainer, zFile);

  zAuth = bcvMprintf("Authorization: Bearer %s", pA->zSas);
  p->pList = curl_slist_append(p->pList, zAuth);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_MIMETYPE_HDR);
  if( zETag ){
    char *zIf = bcvMprintf("x-goog-if-generation-match: %s", zETag);
    p->pList = curl_slist_append(p->pList, zIf);
    sqlite3_free(zIf);
  }

  curl_easy_setopt(p->pCurl, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)nByte);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  p->zHttpUrl = zUrl;
  p->eHttpMethod = BCV_HTTP_PUT;
  sqlite3_free(zAuth);
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
void curlFetchBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
){
  switch( pA->eStorage ){
    case BCV_STORAGE_AZURE:
      curlAzureFetchBlob(p, pA, zContainer, zFile);
      break;
    case BCV_STORAGE_GOOGLE:
      curlGoogleFetchBlob(p, pA, zContainer, zFile);
      break;
  }
}

void curlPutBlob(
  CurlRequest *p,
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile,
  const char *zETag,              /* If-Match tag (or NULL) */
  int nByte                       /* Size of file in bytes */
){
  switch( pA->eStorage ){
    case BCV_STORAGE_AZURE:
      curlAzurePutBlob(p, pA, zContainer, zFile, zETag, nByte);
      break;
    case BCV_STORAGE_GOOGLE:
      curlGooglePutBlob(p, pA, zContainer, zFile, zETag, nByte);
      break;
  }
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
int bcvManifestParse(
  u8 *a, int n,                   /* Buffer to parse */
  char *zETag,                    /* etag value for new manifest */
  Manifest **ppOut,               /* OUT: new parsed manifest object */
  char **pzErr                    /* OUT: error message */
){
  int rc = SQLITE_OK;             /* return code */
  Manifest *pNew;
  int nDb;
  int i;
  u32 iVersion;

  /* Zero output parameteers */
  *ppOut = 0;
  *pzErr = 0;

  iVersion = bcvGetU32(a);
  if( iVersion!=BCV_MANIFEST_VERSION ){
    *pzErr = bcvMprintf(
        "bad manifest version - expected %d, found %d.",
        BCV_MANIFEST_VERSION, iVersion
    );
    rc = SQLITE_ERROR;
  }else{
    nDb = manifestDatabaseCount(a, n);
    pNew = (Manifest*)bcvMallocZero(
        sizeof(Manifest) + (nDb+1)*sizeof(ManifestDb)
    );
    if( pNew==0 ){
      rc = SQLITE_NOMEM;
    }else{

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
          *pzErr = bcvMprintf("corrupt manifest file - ids out of order");
          rc = SQLITE_ERROR;
          break;
        }
      }

      if( rc==SQLITE_OK && (
          pNew->nNamebytes<BCV_MIN_NAMEBYTES
       || pNew->nNamebytes>BCV_MAX_NAMEBYTES
      )){
        *pzErr = bcvMprintf(
            "corrupt manifest file - namebytes value out of range (%d)",
            pNew->nNamebytes
        );
        rc = SQLITE_ERROR;
      }
    }
  }

  if( rc==SQLITE_OK ){
    *ppOut = pNew;
  }else{
    bcvManifestFree(pNew);
    *ppOut = 0;
  }
  return rc;
}

u8 *bcvManifestCompose(Manifest *p, int *pnOut){
  int i;
  int nByte;                      /* Bytes required for new manifest */
  u8 *aNew;                       /* New serialized manifest */

  /* Calculate the size of the serialized manifest */
  nByte = BCV_MANIFEST_HEADER_BYTES 
        + p->nDb * BCV_MANIFEST_DBHEADER_BYTES
        + p->nDelBlk * GCENTRYBYTES(p);
  for(i=0; i<p->nDb; i++){
    nByte += p->aDb[i].nBlkOrig * NAMEBYTES(p);
  }

  aNew = (u8*)bcvMalloc(nByte);
  if( aNew==0 ){
    *pnOut = 0;
  }else{
    int iOff;
    int iGCOff;

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
      u8 *aHdr = &aNew[BCV_MANIFEST_HEADER_BYTES+i*BCV_MANIFEST_DBHEADER_BYTES];
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
  }
  return aNew;
}

void bcvManifestFree(Manifest *p){
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


int curlErrorIfNot2XX(
  sqlite3_bcv *p,
  CurlRequest *pCurl, 
  CURLcode res, 
  const char *zPre
){
  long code;
  if( res!=CURLE_OK ){
    bcvApiError(p, SQLITE_ERROR, "%s: %s\n", zPre, curl_easy_strerror(res));
  }
  curl_easy_getinfo(pCurl->pCurl, CURLINFO_RESPONSE_CODE, &code);
  if( (code / 100)!=2 ){
    bcvApiError(p, code, "%s (%d) - %s", zPre, code, pCurl->zStatus);
  }
  return p->errCode;
}


int bcvManifestFetchParsed(sqlite3_bcv *p, Manifest **ppOut){
  CurlRequest curl;
  MemoryDownload md;
  CURLcode res;

  curlRequestInit(&curl, p->bVerbose);
  curlFetchBlob(&curl, &p->ap, p->zCont, BCV_MANIFEST_FILE);
  memset(&md, 0, sizeof(MemoryDownload));
  md.pCurl = &curl;
  curl_easy_setopt(curl.pCurl, CURLOPT_WRITEFUNCTION, memoryDownloadWrite);
  curl_easy_setopt(curl.pCurl, CURLOPT_WRITEDATA, (void*)&md);
  res = curl_easy_perform(curl.pCurl);

  if( SQLITE_OK==curlErrorIfNot2XX(p, &curl, res, "download manifest failed") ){
    p->errCode = bcvManifestParse(
        md.a, md.nByte, curl.zETag, ppOut, &p->zErrmsg
    );
    curl.zETag = 0;
  }

  curlRequestReset(&curl);
  return p->errCode;
}

int bcvManifestUploadParsed(sqlite3_bcv *p, Manifest *pMan){
  u8 *aMan = 0;
  int nMan = 0;

  assert( p->errCode==SQLITE_OK );
  assert( pMan->nDb==0 || pMan->zETag );

  aMan = bcvManifestCompose(pMan, &nMan);
  if( aMan==0 ){
    p->errCode = SQLITE_NOMEM;
  }else{
    CURLcode res;
    MemoryUpload mu;
    CurlRequest curl;

    curlRequestInit(&curl, p->bVerbose);
    curlPutBlob(&curl, &p->ap, p->zCont, BCV_MANIFEST_FILE, pMan->zETag, nMan);
    memset(&mu, 0, sizeof(MemoryUpload));
    mu.a = aMan;
    mu.nByte = nMan;
    curl_easy_setopt(curl.pCurl, CURLOPT_READFUNCTION, memoryUploadRead);
    curl_easy_setopt(curl.pCurl, CURLOPT_READDATA, (void*)&mu);
    curl_easy_setopt(curl.pCurl, CURLOPT_SEEKFUNCTION, memoryUploadSeek);
    curl_easy_setopt(curl.pCurl, CURLOPT_SEEKDATA, (void*)&mu);

    res = curl_easy_perform(curl.pCurl);
    curlErrorIfNot2XX(p, &curl, res, "upload manifest failed");
    curlRequestReset(&curl);
    sqlite3_free(aMan);
  }

  return p->errCode;
}

void bcvAccessPointFree(AccessPoint *p){
  sqlite3_free(p->zSlashAccountSlash);
  sqlite3_free(p->aKey);
  sqlite3_free(p->zSas);
}

void bcvAccessPointCopy(AccessPoint *pTo, AccessPoint *pFrom){
  memset(pTo, 0, sizeof(AccessPoint));
  pTo->zAccount = pFrom->zAccount;
  pTo->zAccessKey = pFrom->zAccessKey;
  pTo->zEmulator = pFrom->zEmulator;

  pTo->zSas = bcvStrdup(pFrom->zSas);
  pTo->zSlashAccountSlash = bcvStrdup(pFrom->zSlashAccountSlash);
  pTo->eStorage = pFrom->eStorage;

  if( pFrom->aKey ){
    pTo->aKey = bcvMemdup(pFrom->nKey, pFrom->aKey);
    pTo->nKey = pFrom->nKey;
  }
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
size_t memoryUploadRead(char *pBuffer, size_t n1, size_t n2, void *pCtx){
  int nReq;
  MemoryUpload *p = (MemoryUpload*)pCtx;
  nReq = MIN(n1*n2, p->nByte-p->iOff);
  memcpy(pBuffer, &p->a[p->iOff], nReq);
  p->iOff += nReq;
  return nReq;
}
size_t memoryUploadSeek(void *pCtx, curl_off_t offset, int origin){
  MemoryUpload *p = (MemoryUpload*)pCtx;
  p->iOff = (int)offset;
  return CURL_SEEKFUNC_OK;
}

static void curlAzureCreateContainer(
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

  if( pA->zAccessKey ){
    zStringToSign = bcvMprintf(
        "PUT\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s\nrestype:container", 
        zDate, BCV_AZURE_VERSION_HDR,
        pA->zSlashAccountSlash,
        zContainer
    );
    zAuth = azure_auth_header(pA, zStringToSign);
    p->pList = curl_slist_append(p->pList, zAuth);
  }else{
    zUrl = bcvMprintf("%z&%s", zUrl, pA->zSas);
  }

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
}

static void curlGoogleCreateContainer(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zAuth = 0;                /* Authorization header */
  char *zProject = 0;             /* Project id header */

  zUrl = bcvMprintf("https://storage.googleapis.com/%s", zContainer);
  zAuth = bcvMprintf("Authorization: Bearer %s", pA->zSas);
  zProject = bcvMprintf("x-goog-project-id: %s", pA->zAccount);

  p->pList = curl_slist_append(p->pList, zAuth);
  p->pList = curl_slist_append(p->pList, zProject);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_MIMETYPE_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zUrl);
  sqlite3_free(zProject);
  sqlite3_free(zAuth);
}

void curlCreateContainer(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer
){
  switch( pA->eStorage ){
    case BCV_STORAGE_AZURE:
      curlAzureCreateContainer(p, pA, zContainer);
      break;
    case BCV_STORAGE_GOOGLE:
      curlGoogleCreateContainer(p, pA, zContainer);
      break;
  }
}

size_t bdIgnore(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  return nSize*nMember;
}

static void curlAzureDeleteBlob(
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
  char *zDebugUrl = 0;

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s/%s", zBase, zContainer, zFile);

  if( pA->zAccessKey ){
    zStringToSign = bcvMprintf(
        "DELETE\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s/%s", 
        zDate, BCV_AZURE_VERSION_HDR,
        pA->zSlashAccountSlash,
        zContainer, zFile
    );
    zAuth = azure_auth_header(pA, zStringToSign);
    p->pList = curl_slist_append(p->pList, zAuth);
    zDebugUrl = zUrl;
  }else{
    zDebugUrl = bcvMprintf("%s?<sas>", zUrl);
    zUrl = bcvMprintf("%z?%s", zUrl, pA->zSas);
  }
  assert( p->zHttpUrl==0 );
  p->zHttpUrl = zDebugUrl;
  p->eHttpMethod = BCV_HTTP_DELETE;

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);
  curl_easy_setopt(p->pCurl, CURLOPT_WRITEFUNCTION, bdIgnore);

  sqlite3_free(zBase);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
  if( zUrl!=zDebugUrl ) sqlite3_free(zUrl);
}

static void curlGoogleDeleteBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zAuth = 0;                /* Authorization header */

  zUrl = bcvMprintf("https://storage.googleapis.com/%s/%s", zContainer, zFile);

  zAuth = bcvMprintf("Authorization: Bearer %s", pA->zSas);
  p->pList = curl_slist_append(p->pList, zAuth);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_MIMETYPE_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);
  curl_easy_setopt(p->pCurl, CURLOPT_WRITEFUNCTION, bdIgnore);

  assert( p->zHttpUrl==0 );
  p->zHttpUrl = zUrl;
  p->eHttpMethod = BCV_HTTP_DELETE;
  sqlite3_free(zAuth);
}

void curlDeleteBlob(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zFile
){
  switch( pA->eStorage ){
    case BCV_STORAGE_AZURE:
      curlAzureDeleteBlob(p, pA, zContainer, zFile);
      break;
    case BCV_STORAGE_GOOGLE:
      curlGoogleDeleteBlob(p, pA, zContainer, zFile);
      break;
  }
}

void curlDestroyContainer(
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

  if( pA->zAccessKey ){
    zStringToSign = bcvMprintf(
        "DELETE\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s%s\nrestype:container", 
        zDate, BCV_AZURE_VERSION_HDR,
        pA->zSlashAccountSlash,
        zContainer
    );
    zAuth = azure_auth_header(pA, zStringToSign);
    p->pList = curl_slist_append(p->pList, zAuth);
  }else{
    zUrl = bcvMprintf("%z&%s", zUrl, pA->zSas);
  }

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
  curl_easy_setopt(p->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)0);
  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zUrl);
  sqlite3_free(zStringToSign);
  sqlite3_free(zAuth);
}

static void curlAzureListFiles(
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
  char *zDebugUrl = 0;

  azure_date_header(zDate);
  zBase = azure_base_uri(pA);
  zUrl = bcvMprintf("%s%s?restype=container&comp=list&maxresults=%d"
      "&marker=%s", 
      zBase, zContainer, nMaxResults, zNextMarker
  );
  zRes = bcvMprintf("%s%s", pA->zSlashAccountSlash, zContainer);

  if( pA->zAccessKey ){
    zStringToSign = bcvMprintf(
        "GET\n\n\n\n\n\n\n\n\n\n\n\n%s\n%s\n%s\ncomp:list"
        "\nmarker:%s\nmaxresults:%d\nrestype:container",
        zDate, BCV_AZURE_VERSION_HDR, zRes, zNextMarker, nMaxResults
    );
    zAuth = azure_auth_header(pA, zStringToSign);
    p->pList = curl_slist_append(p->pList, zAuth);
  }else{
    zDebugUrl = bcvMprintf("%s&<sas>", zUrl);
    zUrl = bcvMprintf("%z&%s", zUrl, pA->zSas);
  }
  assert( p->zHttpUrl==0 );
  p->eHttpMethod = BCV_HTTP_GET;
  p->zHttpUrl = zDebugUrl;

  p->pList = curl_slist_append(p->pList, zDate);
  p->pList = curl_slist_append(p->pList, BCV_AZURE_VERSION_HDR);

  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  sqlite3_free(zBase);
  sqlite3_free(zStringToSign);
  sqlite3_free(zRes);
  sqlite3_free(zAuth);
  if( zUrl!=zDebugUrl ) sqlite3_free(zUrl);
}

static void curlGoogleListFiles(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zNextMarker
){
  char *zUrl = 0;                 /* URL to fetch */
  char *zAuth = 0;                /* Authorization header */
  int nMaxResults = 200;

  zUrl = bcvMprintf("https://storage.googleapis.com/%s?max-keys=%d%s%s",
      zContainer, nMaxResults, 
      zNextMarker?"&marker=":"", zNextMarker?zNextMarker:""
  );

  zAuth = bcvMprintf("Authorization: Bearer %s", pA->zSas);
  p->pList = curl_slist_append(p->pList, zAuth);

  curl_easy_setopt(p->pCurl, CURLOPT_URL, zUrl);
  curl_easy_setopt(p->pCurl, CURLOPT_HTTPHEADER, p->pList);

  assert( p->zHttpUrl==0 );
  p->eHttpMethod = BCV_HTTP_GET;
  p->zHttpUrl = zUrl;
  sqlite3_free(zAuth);
}

void curlListFiles(
  CurlRequest *p, 
  AccessPoint *pA,
  const char *zContainer,
  const char *zNextMarker
){
  switch( pA->eStorage ){
    case BCV_STORAGE_AZURE:
      curlAzureListFiles(p, pA, zContainer, zNextMarker);
      break;
    case BCV_STORAGE_GOOGLE:
      curlGoogleListFiles(p, pA, zContainer, zNextMarker);
      break;
  }
}

size_t memoryDownloadWrite(
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

typedef struct FileDownload FileDownload;
struct FileDownload {
  sqlite3_file *pFd;              /* File descriptor to write data to */
  i64 iStart;                     /* Starting offset in file fd */
  int iOff;                       /* Current offset in block */
};

size_t fileDownloadWrite(
  char *pData, 
  size_t nSize, 
  size_t nMember, 
  void *pCtx
){
  FileDownload *p = (FileDownload*)pCtx;
  size_t sz = (nSize * nMember);
  if( p->pFd->pMethods->xWrite(p->pFd, pData, sz, p->iStart+p->iOff) ){
    return 0;
  }
  p->iOff += sz;
  return sz;
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

static void *bcvAzureFilesModifiedHandler(
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
  return bcvAzureFilesModifiedHandler;
}

static void *bcvAzureFilesNameHandler(
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
  return bcvAzureFilesNameHandler;
}

static void *bcvAzureFilesMarkerHandler(
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
  return bcvAzureFilesMarkerHandler;
}

static void *bcvAzureFilesTagHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_SUBTAG ){
    if( 0==bcvStrcmp("Name", szName) ){
      return bcvAzureFilesNameHandler;
    }else
    if( 0==bcvStrcmp("Last-Modified", szName) ){
      return bcvAzureFilesModifiedHandler;
    }else
    if( 0==bcvStrcmp("NextMarker", szName) ){
      return bcvAzureFilesMarkerHandler;
    }
  }
  return bcvAzureFilesTagHandler;
}

static void *bcvGoogleFilesTagHandler(
    SimpleXmlParser parser,
    SimpleXmlEvent event,
    const char* szName,
    const char* szAttribute,
    const char* szValue
){
  if( event==ADD_SUBTAG ){
    if( 0==bcvStrcmp("Key", szName) ){
      return bcvAzureFilesNameHandler;
    }else
    if( 0==bcvStrcmp("LastModified", szName) ){
      return bcvAzureFilesModifiedHandler;
    }
    if( 0==bcvStrcmp("NextMarker", szName) ){
      return bcvAzureFilesMarkerHandler;
    }
  }
  return bcvGoogleFilesTagHandler;
}

void bcvParseFiles(
  int eStorage,
  const u8 *aBuf, int nBuf,
  FilesParse *pParse
){
  SimpleXmlParser x;
  assert( eStorage==BCV_STORAGE_AZURE || eStorage==BCV_STORAGE_GOOGLE );
  sqlite3_free(pParse->zNextMarker);
  pParse->zNextMarker = 0;
  x = simpleXmlCreateParser((const char*)aBuf, nBuf);
  simpleXmlPushUserData(x, (void*)pParse);
  if( eStorage==BCV_STORAGE_AZURE ){
    simpleXmlParse(x, bcvAzureFilesTagHandler);
  }else{
    simpleXmlParse(x, bcvGoogleFilesTagHandler);
  }
  simpleXmlDestroyParser(x);
}

void bcvParseFilesClear(FilesParse *pParse){
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


int bcvManifestNameToIndex(Manifest *p, const char *zDb){
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
  hex_encode(pBlk, NAMEBYTES(p), aBuf);
  memcpy(&aBuf[NAMEBYTES(p)*2], ".bcv", 5);
}

int bcvMHashBucket(ManifestHash *pHash, u8 *pBlk){
  return (int)(bcvGetU32(pBlk) % pHash->nHash);
}

/*
** Build and return a ManifestHash object for the manifest passed as
** the only argument. Exclude blocks from database iExclude from the hash.
*/
ManifestHash *bcvMHashBuild(Manifest *pMan, int iExclude){
  ManifestHash *pHash = 0;
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

  return pHash;
}

/*
** Free a hash object allocated by bcvMHashBuild().
*/
void bcvMHashFree(ManifestHash *pHash){
  sqlite3_free(pHash);
}

/*
** Search the hash table for a block-id that matches the nPrefix byte 
** prefix aPrefix[]. Return a pointer to the first such block-id found,
** or NULL if there is no such block id in the hash table.
*/
u8 *bcvMHashMatch(ManifestHash *pHash, u8 *aPrefix, int nPrefix){
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


int sqlite3_bcv_open(
  int eType,                      /* SQLITE_BCV_* constant */
  const char *zUser,              /* Cloud storage user name */
  const char *zKey,               /* Key or SAS used for cloud storage auth. */
  const char *zCont,              /* Cloud storage container/bucket */
  sqlite3_bcv **ppOut             /* OUT: New object */
){
  sqlite3_bcv *pNew;
  int rc = SQLITE_OK;
  int nCont = bcvStrlen(zCont);
  int nUser = bcvStrlen(zUser);
  int nKey = bcvStrlen(zKey);
  int nByte;

  /* Check arguments are valid. Return SQLITE_MISUSE if they are not */
  if( eType!=SQLITE_BCV_AZURE
   && eType!=SQLITE_BCV_AZURE_EMU
   && eType!=SQLITE_BCV_AZURE_SAS
   && eType!=SQLITE_BCV_AZURE_EMU_SAS
   && eType!=SQLITE_BCV_GOOGLE
  ){
    return SQLITE_MISUSE;
  }
  if( zCont==0 ) return SQLITE_MISUSE;
  if( eType!=SQLITE_BCV_AZURE_EMU && zKey==0 ) return SQLITE_MISUSE;
  if( eType!=SQLITE_BCV_GOOGLE && zUser==0 ) return SQLITE_MISUSE;

  nByte = nCont + nUser + nKey + 3 + sizeof(sqlite3_bcv);
  pNew = sqlite3_malloc(nByte);
  if( pNew==0 ){
    rc = SQLITE_NOMEM;
  }else{
    memset(pNew, 0, nByte);
    pNew->zCont = (char*)&pNew[1];
    pNew->zUser = &pNew->zCont[nCont+1];
    pNew->zKey = &pNew->zUser[nUser+1];
    memcpy(pNew->zCont, zCont, nCont);
    pNew->zCont[nCont] = '\0';
    memcpy(pNew->zUser, zUser, nUser);
    pNew->zUser[nUser] = '\0';
    memcpy(pNew->zKey, zKey, nKey);
    pNew->zKey[nKey] = '\0';
    rc = bcvAccessPointInit(&pNew->ap, eType, zUser, zKey, &pNew->zErrmsg);
  }

  *ppOut = pNew;
  return rc;
}

void sqlite3_bcv_close(sqlite3_bcv *p){
  bcvAccessPointFree(&p->ap);
  sqlite3_free(p->zErrmsg);
  sqlite3_free(p);
}

int sqlite3_bcv_errcode(sqlite3_bcv *p){
  return p->errCode;
}

const char *sqlite3_bcv_errmsg(sqlite3_bcv *p){
  return p->zErrmsg;
}

int sqlite3_bcv_config(sqlite3_bcv *p, int eOp, ...){
  int rc = SQLITE_OK;
  va_list ap;
  va_start(ap, eOp);

  switch( eOp ){
    case SQLITE_BCVCONFIG_VERBOSE:
      p->bVerbose = va_arg(ap, int);
      break;
    case SQLITE_BCVCONFIG_PROGRESS:
      p->pProgressCtx = va_arg(ap, void*);
      p->xProgress = va_arg(ap, bcv_progress_callback);
      break;
    default:
      rc = SQLITE_MISUSE;
      break;
  }

  va_end(ap);
  return rc;
}

static int bcvOpenLocal(const char *zFile, int bRO, sqlite3_file **ppFd){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(BCV_DEFAULT_VFS);
  sqlite3_file *pFd = 0;
  int nFile = strlen(zFile);
  char *zOpen = 0;
  int nByte;
  int flags = SQLITE_OPEN_MAIN_DB;
  int rc;

  if( bRO ){
    flags |= SQLITE_OPEN_READONLY;
  }else{
    flags |= SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;
  }

  *ppFd = 0;
  nByte = pVfs->szOsFile + 4 + nFile + 2;
  pFd = (sqlite3_file*)sqlite3_malloc(nByte);
  if( pFd==0 ){
    return SQLITE_NOMEM;
  }
  memset(pFd, 0, nByte);
  zOpen = &((char*)pFd)[pVfs->szOsFile + 4];
  memcpy(zOpen, zFile, nFile);

  rc = pVfs->xOpen(pVfs, zOpen, pFd, flags, &flags);
  if( rc==SQLITE_OK && bRO==0 && (flags & SQLITE_OPEN_READONLY) ){
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

static void bcvCloseLocal(sqlite3_file *pFd){
  if( pFd ){
    pFd->pMethods->xClose(pFd);
    sqlite3_free(pFd);
  }
}

/*
** Upload a database to cloud storage.
*/
int sqlite3_bcv_upload(
  sqlite3_bcv *p,
  const char *zLocal,
  const char *zRemote
){
  CURLcode res;
  CurlRequest curl;
  int fd = -1;                    /* fd open on database file */
  int i;
  int nBlock;                     /* Number of blocks */
  Manifest *pMan = 0;
  ManifestDb *pDb = 0;
  ManifestHash *pMHash = 0;
  MemoryUpload mu;                /* For uploading block files */
  u8 aId[BCV_DBID_SIZE];          /* New database id */
  int nRemote;                    /* strlen(zRemote) */
  sqlite3_file *pFd = 0;
  sqlite3_int64 szFile = 0;

  bcvApiErrorClear(p);
  curlRequestInit(&curl, p->bVerbose);
  memset(&mu, 0, sizeof(MemoryUpload));

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

  /* Check that the db name is not already in the manifest. */
  i = bcvManifestNameToIndex(pMan, zRemote);
  if( i>=0 ){
    bcvApiError(p, SQLITE_ERROR, "duplicate database name - \"%s\"", zRemote);
    goto update_out;
  }

  /* Open the database file to be uploaded. */
  if( bcvOpenLocal(zLocal, 1, &pFd) ){
    bcvApiError(p, SQLITE_IOERR, 
        "failed to open file \"%s\" for reading", zLocal
    );
    goto update_out;
  }

  fd = open(zLocal, O_RDONLY|O_BINARY);
  if( fd<0 ){
    bcvApiError(p, SQLITE_IOERR, 
        "failed to open file \"%s\" - %s\n", zLocal
    );
    goto update_out;
  }
  if( SQLITE_OK!=pFd->pMethods->xFileSize(pFd, &szFile) ){
    bcvApiError(p, SQLITE_IOERR, 
        "failed to read size of file \"%s\" - %s\n", zLocal
    );
    goto update_out;
  }
  nBlock = ((u64)szFile + pMan->szBlk-1) / pMan->szBlk;

  /* Extend manifest to include new database */
  sqlite3_randomness(BCV_DBID_SIZE, aId);
  for(i=0; i<pMan->nDb; i++){
    if( memcmp(aId, pMan->aDb[i].aId, BCV_DBID_SIZE)<0 ) break;
  }
  pDb = &pMan->aDb[i];
  if( i<pMan->nDb ) memmove(&pDb[1], pDb, (pMan->nDb-i)*sizeof(ManifestDb));
  pMan->nDb++;
  pDb->iVersion = 1;
  memset(pDb->zDName, 0, BCV_DBNAME_SIZE);
  memcpy(pDb->aId, aId, BCV_DBID_SIZE);
  memcpy(pDb->zDName, zRemote, nRemote);
  if( nBlock>0 ){
    assert( pDb->nBlkLocalAlloc==0 );
    pDb->nBlkLocal = pDb->nBlkLocalAlloc = nBlock;
    pDb->aBlkLocal = (u8*)bcvMalloc(nBlock * NAMEBYTES(pMan));
  }

  mu.a = (u8*)bcvMalloc(pMan->szBlk);

  pMHash = bcvMHashBuild(pMan, -1);
  for(i=0; i<nBlock; i++){
    int rres;
    char aBuf[BCV_MAX_FSNAMEBYTES];
    u8 *aBlk = &pDb->aBlkLocal[i * NAMEBYTES(pMan)];

    mu.nByte = (i==nBlock-1 ? szFile % pMan->szBlk : pMan->szBlk);
    mu.iOff = 0;
    rres = (int)read(fd, mu.a, mu.nByte);
    if( rres!=mu.nByte ){
      bcvApiError(p, SQLITE_IOERR, 
          "failed to read %d bytes from offset %lld of %s (fd=%d)", 
          mu.nByte, (i64)i*pMan->szBlk, zLocal, fd
      );
      goto update_out;
    }

    if( NAMEBYTES(pMan)>=BCV_MIN_MD5_NAMEBYTES ){
      u8 *pMatch;
      assert( MD5_DIGEST_LENGTH==16 );
      MD5(mu.a, mu.nByte, aBlk);
      pMatch = bcvMHashMatch(pMHash, aBlk, MD5_DIGEST_LENGTH);
      if( pMatch ){
        memcpy(aBlk, pMatch, NAMEBYTES(pMan));
        continue;
      }else{
        sqlite3_randomness(
            NAMEBYTES(pMan) - MD5_DIGEST_LENGTH, &aBlk[MD5_DIGEST_LENGTH]
        );
      }
    }else{
      sqlite3_randomness(NAMEBYTES(pMan), aBlk);
    }

    bcvBlockidToText(pMan, aBlk, aBuf);
    curlPutBlob(&curl, &p->ap, p->zCont, aBuf, 0, mu.nByte);

    curl_easy_setopt(curl.pCurl, CURLOPT_READFUNCTION, memoryUploadRead);
    curl_easy_setopt(curl.pCurl, CURLOPT_READDATA, (void*)&mu);
    curl_easy_setopt(curl.pCurl, CURLOPT_SEEKFUNCTION, memoryUploadSeek);
    curl_easy_setopt(curl.pCurl, CURLOPT_SEEKDATA, (void*)&mu);
    res = curl_easy_perform(curl.pCurl);
    if( curlErrorIfNot2XX(p, &curl, res, "upload block failed") ){
      goto update_out;
    }
    if( p->xProgress ){
      if( p->xProgress(p->pProgressCtx, (i*pMan->szBlk)+mu.nByte, szFile) ){
        bcvApiError(p, SQLITE_ABORT, "abort requested by progress callback");
        goto update_out;
      }
    }
    curlRequestReset(&curl);
  }
  assert( pDb->bBlkOrigFree==0 );
  pDb->aBlkOrig = pDb->aBlkLocal;
  pDb->nBlkOrig = pDb->nBlkLocal;
  pDb->bBlkOrigFree = 1;
  pDb->nBlkLocalAlloc = 0;
  bcvManifestUploadParsed(p, pMan);

 update_out:
  bcvCloseLocal(pFd);
  bcvManifestFree(pMan);
  bcvMHashFree(pMHash);
  sqlite3_free(mu.a);
  curlRequestFinalize(&curl);
  if( fd>=0 ) close(fd);
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

  pMH = bcvMHashBuild(pMan, iDb);
  assert( pDb->nBlkOrig==pDb->nBlkLocal );
  aGC = (u8*)bcvMalloc((pMan->nDelBlk+pDb->nBlkOrig) * GCENTRYBYTES(pMan));
  if( pMan->nDelBlk ){
    memcpy(aGC, pMan->aDelBlk, pMan->nDelBlk * GCENTRYBYTES(pMan));
  }

  for(i=0; i<pDb->nBlkOrig; i++){
    u8 *pDel = &pDb->aBlkOrig[i*nName];
    if( 0==bcvMHashMatch(pMH, pDel, nName) ){
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
  CurlRequest curl;
  int nTo;
  int iFrom, iTo;
  ManifestDb *pFrom, *pTo;
  Manifest *pMan = 0;

  bcvApiErrorClear(p);
  curlRequestInit(&curl, p->bVerbose);

  /* Download the manifest file. */
  if( bcvManifestFetchParsed(p, &pMan) ){
    return p->errCode;
  }
  nTo = bcvStrlen(zTo);

  iTo = bcvManifestNameToIndex(pMan, zTo);
  if( iTo<0 ){
    u8 aId[BCV_DBID_SIZE];
    sqlite3_randomness(BCV_DBID_SIZE, aId);
    for(iTo=0; iTo<pMan->nDb; iTo++){
      if( memcmp(aId, pMan->aDb[iTo].aId, BCV_DBID_SIZE)<0 ) break;
    }
    if( iTo<pMan->nDb ){
      int nByte = sizeof(ManifestDb)*(pMan->nDb-iTo);
      memmove(&pMan->aDb[iTo+1], &pMan->aDb[iTo], nByte);
    }
    pMan->nDb++;
    pTo = &pMan->aDb[iTo];
    memset(pTo, 0, sizeof(ManifestDb));
    memcpy(pTo->aId, aId, BCV_DBID_SIZE);
    memcpy(pTo->zDName, zTo, nTo);
  }else{
    bcvDeleteBlocks(pMan, iTo);
    pTo = &pMan->aDb[iTo];
    pTo->aBlkOrig = pTo->aBlkLocal = 0;
    pTo->nBlkOrig = pTo->nBlkLocal = 0;
  }

  /* Check that the from-db name is in the manifest. */
  iFrom = bcvManifestNameToIndex(pMan, zFrom);
  if( iFrom<0 ){
    bcvApiError(p, SQLITE_ERROR, "no such database: %s", zFrom);
  }else{
    pFrom = &pMan->aDb[iFrom];
    pTo->nBlkLocal = pTo->nBlkOrig = pFrom->nBlkOrig;
    pTo->aBlkOrig = (u8*)bcvMallocZero(pTo->nBlkOrig*NAMEBYTES(pMan));
    pTo->aBlkLocal = pTo->aBlkOrig;
    memcpy(pTo->aBlkOrig, pFrom->aBlkOrig, pTo->nBlkOrig*NAMEBYTES(pMan));
    pTo->iVersion++;
    bcvManifestUploadParsed(p, pMan);
  }

  bcvManifestFree(pMan);
  return p->errCode;
}

/*
** Download a database from cloud storage.
*/
int sqlite3_bcv_download(
  sqlite3_bcv *p,
  const char *zRemote,
  const char *zLocal
){
  CURLcode res;
  CurlRequest curl;
  int iDb;                        /* Database index in manifest */
  int i;
  FileDownload fd;
  Manifest *pMan = 0;
  ManifestDb *pDb = 0;
  int rc;
  i64 szFile;

  bcvApiErrorClear(p);
  curlRequestInit(&curl, p->bVerbose);
  memset(&fd, 0, sizeof(FileDownload));

  /* Grab the manifest file */
  if( bcvManifestFetchParsed(p, &pMan) ){
    return p->errCode;
  }

  /* Search manifest for the requested file. Error out if it is not there. */
  iDb = bcvManifestNameToIndex(pMan, zRemote);
  if( iDb<0 ){
    bcvApiError(p, SQLITE_ERROR, "database not found: %s", zRemote);
    goto download_out;
  }
  pDb = &pMan->aDb[iDb];

  /* Open the local file (to download to) */
  memset(&fd, 0, sizeof(FileDownload));
  rc = bcvOpenLocal(zLocal, 0, &fd.pFd);
  if( rc!=SQLITE_OK ){
    bcvApiError(
        p, SQLITE_ERROR, "cannot open local file for writing: %s", zLocal
    );
    goto download_out;
  }

  szFile = pMan->szBlk * pDb->nBlkLocal;
  for(i=0; p->errCode==SQLITE_OK && i<pDb->nBlkLocal; i++){
    char aBuf[BCV_MAX_FSNAMEBYTES];
    bcvBlockidToText(pMan, &pDb->aBlkLocal[NAMEBYTES(pMan)*i], aBuf);
    fd.iStart = (i64)i*pMan->szBlk;
    fd.iOff = 0;
    curlFetchBlob(&curl, &p->ap, p->zCont, aBuf);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEFUNCTION, fileDownloadWrite);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEDATA, (void*)&fd);
    res = curl_easy_perform(curl.pCurl);
    curlErrorIfNot2XX(p, &curl, res, "download block failed");
    curlRequestReset(&curl);

    if( p->xProgress ){
      if( p->xProgress(p->pProgressCtx, (i64)(i+1)*pMan->szBlk, szFile) ){
        bcvApiError(p, SQLITE_ABORT, "abort requested by progress callback");
        break;
      }
    }
  }
  bcvCloseLocal(fd.pFd);

download_out:
  curlRequestFinalize(&curl);
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

  /* Download the manifest file. */
  if( bcvManifestFetchParsed(p, &pMan) ){
    return p->errCode;
  }

  /* Check that the db name is in the manifest. */
  iDb = bcvManifestNameToIndex(pMan, zDelete);
  if( iDb<0 ){
    bcvApiError(p, SQLITE_ERROR, "database not found: %s", zDelete);
  }else{
    ManifestDb *pDb = &pMan->aDb[iDb];

    /* Move blocks to the delete list */
    bcvDeleteBlocks(pMan, iDb);

    assert( pDb->bBlkOrigFree==0 && pDb->nBlkLocalAlloc==0 );
    if( iDb<pMan->nDb-1 ){
      memmove(pDb, &pDb[1], (pMan->nDb-iDb-1)*sizeof(ManifestDb));
    }
    pMan->nDb--;

    bcvManifestUploadParsed(p, pMan);
  }

  bcvManifestFree(pMan);
  return 0;
}

/*
** Delete the entire cloud storage container or bucket.
*/
int sqlite3_bcv_destroy(sqlite3_bcv *p){
  CURLcode res;
  CurlRequest curl;

  curlRequestInit(&curl, p->bVerbose);

  curlDestroyContainer(&curl, &p->ap, p->zCont);
  res = curl_easy_perform(curl.pCurl);
  curlErrorIfNot2XX(p, &curl, res, "destroy container failed");
  curlRequestFinalize(&curl);

  return p->errCode;
}

/*
** Create a new cloud storage container and manifest file. If the container
** already exists, it is not an error, but any existing manifest file is
** simply clobbered.
*/
int sqlite3_bcv_create(sqlite3_bcv *p, int szName, int szBlock){
  Manifest man;

  if( szName<=0 ) szName = BCV_DEFAULT_NAMEBYTES;
  if( szBlock<=0 ) szBlock = BCV_DEFAULT_BLOCKSIZE;

  if( szName<BCV_MIN_NAMEBYTES || szName>BCV_MAX_NAMEBYTES ){
    return bcvApiError(p, SQLITE_RANGE, "parameter szName out of range");
  }

  bcvApiErrorClear(p);
  memset(&man, 0, sizeof(man));
  man.szBlk = szBlock;
  man.nNamebytes = szName;

  if( bcvManifestUploadParsed(p, &man) ){
    CurlRequest curl;
    CURLcode res;

    bcvApiErrorClear(p);
    curlRequestInit(&curl, p->bVerbose);
    curlCreateContainer(&curl, &p->ap, p->zCont);
    res = curl_easy_perform(curl.pCurl);
    curlErrorIfNot2XX(p, &curl, res, "create container failed");

    if( p->errCode==SQLITE_OK ){
      bcvManifestUploadParsed(p, &man);
    }
  }

  return p->errCode;
}


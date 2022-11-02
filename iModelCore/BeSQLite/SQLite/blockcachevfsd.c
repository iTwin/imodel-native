/*
** 2019 October 31
**
*************************************************************************
**
** This file contains the code for the blockcachevfs daemon - blockcachevfsd.
*/


#include "bcv_int.h"
#include "blockcachevfs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __WIN32__
# include <windows.h>
# define strerror_r(errno,buf,len) strerror_s(buf,len,errno)
#endif
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#ifdef __WIN32__
# define BCV_PATH_SEPARATOR "\\"
# define osMkdir(x,y) mkdir(x)
# define signal(x,y)
#else
# define BCV_PATH_SEPARATOR "/"
# define osMkdir(x,y) mkdir(x,y)
# include <signal.h>
#endif

#include "sqlite3.h"
#include "simplexml.h"

/* Default values of various command line parameters. */
#define BCV_DEFAULT_CACHEFILE_SIZE   (1024*1024*1024)

#define LARGEST_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))

typedef struct CommandSwitches CommandSwitches;
struct CommandSwitches {
  /* Used by almost all commands: */
  char *zContainer;               /* -container ARG */
  char *zModule;                  /* -module ARG */
  char *zUser;                    /* -user ARG */
  char *zSecret;                  /* -auth ARG */

  /* Used by 'create' command only: */
  i64 szBlk;                      /* -blocksize ARG */
  int nNamebytes;                 /* -namebytes ARG */

  /* Used by 'attach': */
  int bReadonly;                  /* -readonly is present */
  int bSecure;                    /* -secure is present */
  char *zAlias;                   /* -alias ARG */

  /* Used by 'upload', 'download' and 'daemon' */
  int nRequest;                   /* -nrequest ARG */

  /* Used by daemon only: */
  char *zAddr;                    /* -addr ARG */
  int iPort;                      /* -port ARG */
  int bNoTimestamps;              /* True if -notimestamps is specified */
  int bAutoexit;                  /* True if -autoexit is specified */
  i64 szCache;                    /* Size of cache file in bytes */
  int mLog;                       /* Mask of daemon events to log */
  int bReadyMessage;              /* Daemon outputs "ready" message */
  int nHttpTimeout;               /* Value of --httptimeout option */
};

#define BCV_LOG_MESSAGE   0x01
#define BCV_LOG_HTTP      0x20
#define BCV_LOG_HTTPRETRY 0x04
#define BCV_LOG_EVENT     0x02
#define BCV_LOG_VERBOSE   0x40
#define BCV_LOG_SCHEDULE  0x80

#define BCV_LOG_POLL      0x04
#define BCV_LOG_UPLOAD    0x08
#define BCV_LOG_CACHE     0x10


static void fatal_sql_error(sqlite3 *db, const char *zMsg){
  fprintf(stderr, "FATAL: SQL error in %s (rc=%d) (errmsg=%s)\n", 
      zMsg, sqlite3_errcode(db), sqlite3_errmsg(db)
  );
  abort();
}


typedef struct SelectOption SelectOption;
struct SelectOption {
  const char *zOpt;
  u32 mask;
  int eVal;
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
  abort();
}

void fatal_system_error(const char *zApi, int iError){
  char *zBuf = bcvMalloc(1000);
  strerror_r(iError, zBuf, 1000);
  zBuf[999] = '\0';
  fatal_error("%s - %s", zApi, zBuf);
}

static void *bdMalloc(int n){
  void *pRet = sqlite3_malloc(n);
  if( pRet==0 ){
    fatal_oom_error();
  }
  return pRet;
}

static void *bdMallocZero(int n){
  void *pRet = bdMalloc(n);
  memset(pRet, 0, n);
  return pRet;
}

static void *bdBufferDup(const void *aIn, int nIn){
  void *pRet = bdMalloc(nIn);
  memcpy(pRet, aIn, nIn);
  return pRet;
}

static char *bdStrdup(const char *zIn){
  char *zRet = 0;
  if( zIn ){
    int nIn = bcvStrlen(zIn);
    zRet = bdMalloc(nIn+1);
    memcpy(zRet, zIn, nIn+1);
  }
  return zRet;
}

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
          "parse error in integer (max value is %d): %s", 0x7fffffff, zInt
      );
    }
  }

  return (int)ret;
}


#define COMMANDLINE_CONTAINER    1
#define COMMANDLINE_DIRECTORY    4
#define COMMANDLINE_PORT         5
#define COMMANDLINE_CACHESIZE    7
#define COMMANDLINE_AUTOEXIT     8
#define COMMANDLINE_BLOCKSIZE    9
#define COMMANDLINE_POLLTIME     10
#define COMMANDLINE_DELETETIME   11
#define COMMANDLINE_GCTIME       12
#define COMMANDLINE_RETRYTIME    13
#define COMMANDLINE_LOG          14
#define COMMANDLINE_NODELETE     15
#define COMMANDLINE_CLOBBER      17
#define COMMANDLINE_NWRITE       18
#define COMMANDLINE_NDELETE      19
#define COMMANDLINE_READYMSG     20
#define COMMANDLINE_NAMEBYTES    21
#define COMMANDLINE_PERSISTENT   22
#define COMMANDLINE_VTAB         23
#define COMMANDLINE_NOTIMESTAMPS 27
#define COMMANDLINE_MODULE       28
#define COMMANDLINE_USER         29
#define COMMANDLINE_SECRET       30
#define COMMANDLINE_NREQUEST     31
#define COMMANDLINE_READONLY     32
#define COMMANDLINE_ALIAS        33
#define COMMANDLINE_POLL         34
#define COMMANDLINE_LAZY         35
#define COMMANDLINE_ADDR         36
#define COMMANDLINE_SECURE       37
#define COMMANDLINE_NONONCE      38
#define COMMANDLINE_AUTODETACH   39
#define COMMANDLINE_HTTPTIMEOUT  40

#define COMMAND_ALL              0xFFFFFFFF
#define COMMAND_DAEMON           0x40000000
#define COMMAND_CREATE           0x20000000      /* -blocksize, -namebytes */
#define COMMAND_CMDLINE          0x08000000      /* -module, -user, -auth */
#define COMMAND_CONTAINER        0x04000000
#define COMMAND_NREQUEST         0x02000000      /* -nrequest */

#define COMMAND_AUTH_CONTAINER (COMMAND_CMDLINE | COMMAND_CONTAINER)
#define COMMAND_UPDOWN         (COMMAND_AUTH_CONTAINER | COMMAND_NREQUEST)
#define COMMAND_CMDLINE_DAEMON (COMMAND_CMDLINE | COMMAND_DAEMON)


static void parse_more_switches(
  CommandSwitches *pCmd, 
  const char **azArg, 
  int nArg, 
  int *piFail,
  u32 mask
){
  int i;
  SelectOption aSwitch[] = {
    { "-file",           COMMAND_ALL    , 0},
    { "-delay",          COMMAND_ALL    , -1},
    { "-container",      COMMAND_CONTAINER      , COMMANDLINE_CONTAINER    },
    { "-port",           COMMAND_DAEMON         , COMMANDLINE_PORT         },
    { "-addr",           COMMAND_DAEMON         , COMMANDLINE_ADDR         },
    { "-cachesize",      COMMAND_DAEMON         , COMMANDLINE_CACHESIZE    },
    { "-autoexit",       COMMAND_DAEMON         , COMMANDLINE_AUTOEXIT     },
    { "-blocksize",      COMMAND_CREATE         , COMMANDLINE_BLOCKSIZE    },
    { "-log",            COMMAND_CMDLINE_DAEMON , COMMANDLINE_LOG          },
    { "-readymessage",   COMMAND_DAEMON         , COMMANDLINE_READYMSG     },
    { "-namebytes",      COMMAND_CREATE         , COMMANDLINE_NAMEBYTES    },
    { "-notimestamps",   COMMAND_DAEMON         , COMMANDLINE_NOTIMESTAMPS },
    { "-nrequest",       COMMAND_NREQUEST       , COMMANDLINE_NREQUEST     },
    { "-module",         COMMAND_CMDLINE        , COMMANDLINE_MODULE       },
    { "-user",           COMMAND_CMDLINE        , COMMANDLINE_USER         },
    { "-authentication", COMMAND_CMDLINE        , COMMANDLINE_SECRET,      },
    { "-httptimeout",    COMMAND_DAEMON         , COMMANDLINE_HTTPTIMEOUT  },
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
      case COMMANDLINE_DIRECTORY:
      case COMMANDLINE_PORT:
      case COMMANDLINE_ADDR:
      case COMMANDLINE_CACHESIZE:
      case COMMANDLINE_BLOCKSIZE:
      case COMMANDLINE_POLLTIME:
      case COMMANDLINE_DELETETIME:
      case COMMANDLINE_GCTIME:
      case COMMANDLINE_RETRYTIME:
      case COMMANDLINE_LOG:
      case COMMANDLINE_NWRITE:
      case COMMANDLINE_NDELETE:
      case COMMANDLINE_NAMEBYTES:
      case COMMANDLINE_MODULE:
      case COMMANDLINE_USER:
      case COMMANDLINE_SECRET:
      case COMMANDLINE_NREQUEST:
      case COMMANDLINE_ALIAS:
      case COMMANDLINE_AUTODETACH:
      case COMMANDLINE_HTTPTIMEOUT:
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
        sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
        int n = parse_seconds(azArg[i]);
        pVfs->xSleep(pVfs, 1000000*n);
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
        fd = open(zFile, O_RDONLY|O_BINARY);
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

        memset((char**)azParse, 0, sizeof(azParse));
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
      case COMMANDLINE_MODULE:
        sqlite3_free(pCmd->zModule);
        pCmd->zModule = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_USER:
        sqlite3_free(pCmd->zUser);
        pCmd->zUser = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_SECRET:
        sqlite3_free(pCmd->zSecret);
        pCmd->zSecret = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_ALIAS:
        sqlite3_free(pCmd->zAlias);
        pCmd->zAlias = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_ADDR:
        sqlite3_free(pCmd->zAddr);
        pCmd->zAddr = bcvStrdup(azArg[i]);
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
      case COMMANDLINE_NOTIMESTAMPS:
        pCmd->bNoTimestamps = 1;
        break;
      case COMMANDLINE_AUTOEXIT:
        pCmd->bAutoexit = 1;
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
            case 'h':
              pCmd->mLog |= BCV_LOG_HTTP;
              break;
            case 'r':
              pCmd->mLog |= BCV_LOG_HTTPRETRY;
              break;
            case 'v':
              pCmd->mLog |= BCV_LOG_VERBOSE;
              break;
            case 's':
              pCmd->mLog |= BCV_LOG_SCHEDULE;
              break;
            default:
              /* no-op */
              break;
          }
        }
        break;
      }
      case COMMANDLINE_READYMSG:
        pCmd->bReadyMessage = 1;
        break;
      case COMMANDLINE_NREQUEST:
        pCmd->nRequest = parse_integer(azArg[i]);
        break;
      case COMMANDLINE_READONLY:
        pCmd->bReadonly = 1;
        break;
      case COMMANDLINE_SECURE:
        pCmd->bSecure = 1;
        break;
      case COMMANDLINE_HTTPTIMEOUT:
        pCmd->nHttpTimeout = parse_seconds(azArg[i]);
        break;
      default:
        break;
    }
  }
}

static void parse_switches(
  CommandSwitches *pCmd, 
  char **azArg, 
  int nArg, 
  int *piFail,
  u32 mask
){
  memset(pCmd, 0, sizeof(CommandSwitches));
  parse_more_switches(pCmd, (const char**)azArg, nArg, piFail, mask);
}

static void free_parse_switches(CommandSwitches *pCmd){
  sqlite3_free(pCmd->zContainer);
  sqlite3_free(pCmd->zModule);
  sqlite3_free(pCmd->zUser);
  sqlite3_free(pCmd->zSecret);
  sqlite3_free(pCmd->zAddr);
  memset(pCmd, 0, sizeof(CommandSwitches));
}

static void missing_required_switch(const char *zSwitch){
  fatal_error("missing required switch: %s\n", zSwitch);
}

static void bcvHttpLog(void *pUnused, int bRetry, const char *zMsg){
  if( bRetry || pUnused!=0 ){
    fprintf(stdout, "INFO(h): %s\n", zMsg);
    fflush(stdout);
  }
}

static void bcvOpenHandle(CommandSwitches *pCmd, sqlite3_bcv **ppBcv){
  int rc;
  if( pCmd->zModule==0 ) missing_required_switch("-module");
  rc = sqlite3_bcv_open(
      pCmd->zModule, pCmd->zUser, pCmd->zSecret, pCmd->zContainer, ppBcv
  );
  if( rc!=SQLITE_OK ){
    fatal_error("error (%d): %s\n", rc, sqlite3_bcv_errmsg(*ppBcv));
  }else{
    if( pCmd->mLog & BCV_LOG_VERBOSE ){
      sqlite3_bcv_config(*ppBcv, SQLITE_BCVCONFIG_VERBOSE, (int)1);
    }
    if( pCmd->mLog & (BCV_LOG_HTTP|BCV_LOG_HTTPRETRY) ){
      void *pCtx = (void*)(pCmd->mLog & BCV_LOG_HTTP ? 1 : 0);
      sqlite3_bcv_config(*ppBcv, SQLITE_BCVCONFIG_LOG, pCtx, bcvHttpLog);
    }
    sqlite3_bcv_config(*ppBcv, SQLITE_BCVCONFIG_NREQUEST, pCmd->nRequest);
  }
}

static void bcvOpenContainer(
  CommandSwitches *pCmd, 
  BcvContainer **ppCont,
  BcvDispatch **ppDisp
){
  int rc = SQLITE_OK;
  char *zErr = 0;
  if( pCmd->zModule==0 ) missing_required_switch("-module");
  rc = bcvContainerOpen(
      pCmd->zModule, pCmd->zUser, pCmd->zSecret, pCmd->zContainer, ppCont, &zErr
  );
  if( rc!=SQLITE_OK ){
    fatal_error("error (%d): %s\n", rc, zErr);
  }
  rc = bcvDispatchNew(ppDisp);
  if( rc!=SQLITE_OK ){
    assert( rc==SQLITE_NOMEM );
    fatal_error("error (%d): out of memory\n", rc);
  }
  bcvDispatchVerbose(*ppDisp, (pCmd->mLog & BCV_LOG_VERBOSE) ? 1 : 0);
  if( pCmd->mLog & (BCV_LOG_HTTP|BCV_LOG_HTTPRETRY) ){
    void *pCtx = (void*)(pCmd->mLog & BCV_LOG_HTTP ? 1 : 0);
    bcvDispatchLog(*ppDisp, pCtx, bcvHttpLog);
  }
}

static int xProgress(void *pCtx, sqlite3_int64 nDone, sqlite3_int64 nTotal){
  fprintf(stdout, ".");
  fflush(stdout);
  return 0;
}

/*
** Command: $argv[0] upload ?SWITCHES? DBFILE ?NAME?
**
** Upload a database file to cloud storage. The container and manifest
** file must already exist.
*/
static int main_upload(int argc, char **argv){
  CommandSwitches cmd;
  int iFail = 0;
  int *ptr;
  const char *zDbfile;
  const char *zName;
  sqlite3_bcv *pBcv;
  int rc;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s upload ?SWITCHES? DBFILE ?NAME?\n", argv[0]);
    exit(1);
  }
  ptr = &iFail;
  while( iFail<argc-4 ){
    parse_switches(&cmd, &argv[2], argc-3, ptr, COMMAND_UPDOWN);
    ptr = 0;
  }
  assert( iFail==argc-4 || iFail==argc-3 );
  if( iFail==argc-4 ){
    zDbfile = (const char*)argv[argc-2];
    zName = (const char*)argv[argc-1];
  }else{
    zDbfile = (const char*)argv[argc-1];
    zName = &zDbfile[strlen(zDbfile)];
    while( zName>zDbfile && zName[-1]!='/' && zName[-1]!='\\' ) zName--;
  }
  if( cmd.zContainer==0 ) missing_required_switch("-container");

  bcvOpenHandle(&cmd, &pBcv);
  sqlite3_bcv_config(pBcv, SQLITE_BCVCONFIG_PROGRESS, (void*)0, xProgress);

  rc = sqlite3_bcv_upload(pBcv, zDbfile, zName);
  if( rc ){
    fprintf(stderr, "error (%d): %s\n", rc, sqlite3_bcv_errmsg(pBcv));
  }else{
    fprintf(stdout, "\n");
  }
  sqlite3_bcv_close(pBcv);
  free_parse_switches(&cmd);

  return rc;
}

/*
** Command: $argv[0] download ?SWITCHES? DBNAME ?LOCALFILE?
**
** Download a database file from cloud storage.
*/
static int main_download(int argc, char **argv){
  CommandSwitches cmd;
  int iFail = 0;
  int *ptr = 0;
  const char *zRemote = 0;
  const char *zLocal = 0;
  sqlite3_bcv *pBcv = 0;
  int rc;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, 
        "Usage: %s download ?SWITCHES? DBNAME ?LOCALFILE?\n", argv[0]
    );
    exit(1);
  }
  ptr = &iFail;
  while( iFail<argc-4 ){
    parse_switches(&cmd, &argv[2], argc-3, ptr, COMMAND_UPDOWN);
    ptr = 0;
  }
  if( iFail==argc-4 ){
    zRemote = (const char*)argv[argc-2];
    zLocal = (const char*)argv[argc-1];
  }else{
    zLocal = zRemote = (const char*)argv[argc-1];
  }
  if( cmd.zContainer==0 ) missing_required_switch("-container");

  bcvOpenHandle(&cmd, &pBcv);
  sqlite3_bcv_config(pBcv, SQLITE_BCVCONFIG_PROGRESS, (void*)0, xProgress);

  rc = sqlite3_bcv_download(pBcv, zRemote, zLocal);
  if( rc ){
    fprintf(stderr, "error (%d): %s\n", rc, sqlite3_bcv_errmsg(pBcv));
  }else{
    fprintf(stdout, "\n");
  }
  sqlite3_bcv_close(pBcv);
  free_parse_switches(&cmd);
  return rc;
}

/*
** Command: $argv[0] copy ?SWITCHES? DB1 DB2
**
** Make a copy of a database within cloud storage.
*/
static int main_copy(int argc, char **argv){
  const char *zDbFrom, *zDbTo;
  CommandSwitches cmd;
  int rc;
  sqlite3_bcv *pBcv = 0;

  /* Parse command line */
  if( argc<4 ){
    fprintf(stderr, "Usage: %s copy ?SWITCHES? DBFROM DBTO\n", argv[0]);
    exit(1);
  }
  zDbFrom = argv[argc-2];
  zDbTo = argv[argc-1];
  parse_switches(&cmd, &argv[2], argc-4, 0, COMMAND_AUTH_CONTAINER);

  bcvOpenHandle(&cmd, &pBcv);
  rc = sqlite3_bcv_copy(pBcv, zDbFrom, zDbTo);
  if( rc ){
    fprintf(stderr, "error (%d): %s\n", rc, sqlite3_bcv_errmsg(pBcv));
  }
  sqlite3_bcv_close(pBcv);
  free_parse_switches(&cmd);

  return rc;
}

static void filesCb(void *pApp, int rc, char *z){
  if( rc==SQLITE_OK ){
    if( z ) printf("%s\n", z);
  }else{
    fatal_error("error (%d) - %s", rc, z);
  }
}

/*
** Command: $argv[0] files ?SWITCHES? CONTAINER
*/
static int main_files(int argc, char **argv){
  CommandSwitches cmd;
  BcvContainer *pCont = 0;
  BcvDispatch *pDisp = 0;
  int rc;

  /* Parse command line */
  if( argc<3 ){
    fprintf(stderr, "Usage: %s files ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  parse_switches(&cmd, &argv[2], argc-3, 0, COMMAND_CMDLINE);
  cmd.zContainer = bdStrdup(argv[argc-1]);

  bcvOpenContainer(&cmd, &pCont, &pDisp);
  bcvDispatchList(pDisp, pCont, 0, 0, filesCb);
  rc = bcvDispatchRunAll(pDisp);
  if( rc!=SQLITE_OK ) fatal_error("error %d", rc);

  bcvDispatchFree(pDisp);
  bcvContainerClose(pCont);
  free_parse_switches(&cmd);
  return 0;
}

static void bcvFetchManifestCb(
  void *pApp, 
  int rc, 
  char *zETag, 
  const u8 *aData, 
  int nData
){
  Manifest **ppMan = (Manifest**)pApp;
  char *zErr = 0;
  int prc;

  if( rc!=SQLITE_OK ){
    fatal_error("error %d - %s", rc, zETag);
  }
  prc = bcvManifestParseCopy(aData, nData, zETag, ppMan, &zErr);
  if( prc ){
    fatal_error("error parsing manifest (%d) - %s", prc, zErr);
  }
}

static Manifest *bcvFetchManifest(CommandSwitches *pCmd){
  BcvContainer *pCont = 0;        /* Connection to cloud storage */
  BcvDispatch *pDisp = 0;         /* Dispatcher used to manage pCont */
  Manifest *pMan = 0;
  int rc;

  bcvOpenContainer(pCmd, &pCont, &pDisp);
  bcvDispatchFetch(pDisp, pCont,BCV_MANIFEST_FILE,0,0,&pMan,bcvFetchManifestCb);
  rc = bcvDispatchRunAll(pDisp);
  if( rc!=SQLITE_OK ) fatal_error("error %d", rc);
  bcvDispatchFree(pDisp);
  bcvContainerClose(pCont);
  return pMan;
}


static int main_list(int argc, char **argv){
  CommandSwitches cmd;
  Manifest *p;
  int i;

  /* Parse command line */
  if( argc<3 ){
    fprintf(stderr, "Usage: %s manifest ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  parse_switches(&cmd, &argv[2], argc-3, 0, COMMAND_CMDLINE);
  cmd.zContainer = argv[argc-1];

  p = bcvFetchManifest(&cmd);
  for(i=0; i<p->nDb; i++){
    ManifestDb *pDb = &p->aDb[i];
    printf("%s %d blocks\n", pDb->zDName, pDb->nBlkLocal);
  }
  bcvManifestFree(p);
  return 0;
}

static int bdOpenLocalFile(
  const char *zCont, 
  const char *zDb, 
  int bWal,
  int bReadonly,
  sqlite3_file **ppFile
){
  char *zPath;
  int rc;
  zPath = bcvMprintf("%s%s%s%s", zCont, zCont?"/":"", zDb, bWal?"-wal":"");
  rc = bcvOpenLocal(zPath, bWal, bReadonly, ppFile);
  sqlite3_free(zPath);
  return rc;
}


/*
** Command: $argv[0] manifest ?SWITCHES? CONTAINER
*/
static int main_manifest(int argc, char **argv){
  CommandSwitches cmd;
  Manifest *p = 0;
  int i, j;

  /* Parse command line */
  if( argc<3 ){
    fprintf(stderr, "Usage: %s manifest ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  parse_switches(&cmd, &argv[2], argc-3, 0, COMMAND_CMDLINE);
  cmd.zContainer = argv[argc-1];
  p = bcvFetchManifest(&cmd);

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
    ManifestDb *pDb = &p->aDb[i];
    printf("Database %d id: %lld\n", i, pDb->iDbId);
    printf("Database %d name: %s\n", i, pDb->zDName);
    printf("Database %d version: %d\n", i, pDb->iVersion);
    printf("Database %d block list: (%d blocks)\n", i, pDb->nBlkLocal);
    for(j=0; j<pDb->nBlkLocal; j++){
      char aBuf[BCV_MAX_FSNAMEBYTES];
      bcvBlockidToText(p, &pDb->aBlkLocal[j*NAMEBYTES(p)], aBuf);
      printf("    %s\n", aBuf);
    }
  }

  bcvManifestFree(p);
  return 0;
}

static int main_delete(int argc, char **argv){
  const char *zDbfile;
  CommandSwitches cmd;
  sqlite3_bcv *pBcv = 0;
  int rc;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s delete ?SWITCHES? DB\n", argv[0]);
    exit(1);
  }
  zDbfile = argv[argc-1];
  parse_switches(&cmd, &argv[2], argc-3, 0, COMMAND_AUTH_CONTAINER);
  if( cmd.zContainer==0 ) missing_required_switch("-container");

  bcvOpenHandle(&cmd, &pBcv);
  rc = sqlite3_bcv_delete(pBcv, zDbfile);
  if( rc ){
    fprintf(stderr, "error (%d): %s\n", rc, sqlite3_bcv_errmsg(pBcv));
  }
  sqlite3_bcv_close(pBcv);
  free_parse_switches(&cmd);

  return rc;
}

static int main_create(int argc, char **argv){
  const char *zCont;              /* Name of new container */
  CommandSwitches cmd;
  sqlite3_bcv *pBcv = 0;
  int rc;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s create ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  zCont = argv[argc-1];
  parse_switches(&cmd, &argv[2], argc-3, 0, COMMAND_CREATE | COMMAND_CMDLINE); 

  cmd.zContainer = (char*)zCont;
  bcvOpenHandle(&cmd, &pBcv);
  cmd.zContainer = 0;
  rc = sqlite3_bcv_create(pBcv, cmd.nNamebytes, cmd.szBlk);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "error (%d): %s\n", rc, sqlite3_bcv_errmsg(pBcv));
  }
  sqlite3_bcv_close(pBcv);
  free_parse_switches(&cmd);

  return rc;
}

static int main_destroy(int argc, char **argv){
  const char *zCont;              /* Name of new container */
  CommandSwitches cmd;
  sqlite3_bcv *pBcv = 0;
  int rc;

  /* Parse command line */
  if( argc<=2 ){
    fprintf(stderr, "Usage: %s destroy ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  zCont = argv[argc-1];
  parse_switches(&cmd, &argv[2], argc-3, 0, COMMAND_CMDLINE);
  cmd.zContainer = (char*)zCont;
  bcvOpenHandle(&cmd, &pBcv);
  cmd.zContainer = 0;
  rc = sqlite3_bcv_destroy(pBcv);
  if( rc ){
    fprintf(stderr, "error (%d): %s\n", rc, sqlite3_bcv_errmsg(pBcv));
  }
  sqlite3_bcv_close(pBcv);
  free_parse_switches(&cmd);
  return rc;
}

/**************************************************************************
***************************************************************************
** START OF DAEMON CODE
***************************************************************************
**************************************************************************/

#define BCV_MAX_RESERVE 10

typedef struct DaemonCtx DaemonCtx;
typedef struct DClient DClient;
typedef struct FetchCtx FetchCtx;
typedef struct DPrefetch DPrefetch;

struct DPrefetch {
  int nOutstanding;
  int iNext;
  int bReply;
  int errCode;
  char *zErrMsg;
};

/*
** pMan, pManDb:
**   Non-NULL only when a transaction is open. The manifest and specific
**   datbase within it on which the transaction is open.
*/
struct DClient {
  int nRef;                       /* Ref count on this structure */
  DaemonCtx *pCtx;
  DClient *pNext;                 /* Next client belonging to this daemon */
  int iClientId;                  /* Client id */
  BCV_SOCKET_TYPE fd;             /* Localhost socket for this client */

  Container *pCont;               /* Container this client reads from */
  i64 iDbId;                      /* Id of accessed database */
  Manifest *pMan;
  ManifestDb *pManDb;
  CacheEntry **apRef;

  DPrefetch prefetch;             /* If this is a prefetch client */

  BcvContainer *pBcv;             /* BCV handle, if any */
  char *zAuth;                    /* Authentication string used by pBcv */
  BcvMessage *pMsg;               /* Client message currently being processed */

  int pgsz;                       /* Page-size of connected database */
};

struct DaemonCtx {
  BCV_SOCKET_TYPE fdListen;       /* Socket fd waiting on new connections */
  int iListenPort;                /* Port fdListen is listening on */
  CommandSwitches cmd;            /* Command line options */

  BcvCommon c;                    /* Hash tables and so on */
  BcvDispatch *pDisp;             /* Dispatch object used by this daemon */

  DClient *pClientList;           /* List of connected clients */
  int iNextId;                    /* Next client or encryption id */

  sqlite3_file *pCacheFile;
};

struct FetchCtx {
  DClient *pClient;               /* Client object */
  DaemonCtx *pCtx;
  CacheEntry *pEntry;             /* Cache entry to populate (if any) */
  Container *pCont;
  BcvEncryptionKey *pKey;
};

static void daemon_usage(char *argv0){
  fatal_error("Usage: %s daemon ?SWITCHES? CONTAINER [CONTAINER...]", argv0);
}

static void daemon_log_timestamp(DaemonCtx *p, char *zBuf){
  if( p->cmd.bNoTimestamps ){
    zBuf[0] = '\0';
  }else{
    sqlite3_int64 ms = sqlite_timestamp();
    int Z, A, B, C, D, E, X1;
    int s;

    int day, month, year;
    int sec, min, hour;

    Z = (int)((ms + 43200000)/86400000);
    A = (int)((Z - 1867216.25)/36524.25);
    A = Z + 1 + A - (A/4);
    B = A + 1524;
    C = (int)((B - 122.1)/365.25);
    D = (36525*(C&32767))/100;
    E = (int)((B-D)/30.6001);
    X1 = (int)(30.6001*E);

    day =  B - D - X1;
    month = E<14 ? E-1 : E-13;
    year = month>2 ? C - 4716 : C - 4715;

    s = (int)((ms + 43200000) % 86400000)/1000;
    hour = s/3600;
    s -= hour*3600;
    min = s/60;
    sec = s - min*60;

    sqlite3_snprintf(128, zBuf,
        " [%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.3d]", 
        year, month, day, hour, min, sec, ms%1000
    );
  }
}

static void daemon_vlog(DaemonCtx *p, int flags, const char *zFmt, va_list ap){
  if( p->cmd.mLog & flags ){
    char zTime[128];
    char *zMsg = sqlite3_vmprintf(zFmt, ap);
    daemon_log_timestamp(p, zTime);
    fprintf(stdout, "INFO(%s%s%s%s%s%s%s)%s: %s\n",
        (flags & BCV_LOG_POLL ? "p" : ""),
        (flags & BCV_LOG_EVENT ? "e" : ""),
        (flags & BCV_LOG_MESSAGE ? "m" : ""),
        (flags & BCV_LOG_UPLOAD ? "u" : ""),
        (flags & BCV_LOG_CACHE ? "c" : ""),
        (flags & BCV_LOG_HTTPRETRY ? "r" : ""),
        (flags & BCV_LOG_HTTP ? "h" : ""),
        (flags & BCV_LOG_SCHEDULE ? "s" : ""),
        zTime, zMsg);
    fflush(stdout);
    sqlite3_free(zMsg);
  }
}

static void daemon_event_log(DaemonCtx *p, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  daemon_vlog(p, BCV_LOG_EVENT, zFmt, ap);
  va_end(ap);
}

static void daemon_msg_log(DaemonCtx *p, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  daemon_vlog(p, BCV_LOG_MESSAGE, zFmt, ap);
  va_end(ap);
}

static void daemon_log(DaemonCtx *p, int flags, const char *zFmt, ...){
  va_list ap;
  va_start(ap, zFmt);
  daemon_vlog(p, flags, zFmt, ap);
  va_end(ap);
}

static void bdHttpLog(void *pApp, int bRetry, const char *zMsg){
  DaemonCtx *p = (DaemonCtx*)pApp;
  int flags = BCV_LOG_HTTP | (bRetry ? BCV_LOG_HTTPRETRY : 0);
  daemon_log(p, flags, "%s", zMsg);
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
**
** If successful, the port number is left in DaemonCtx.iListenPort. If an
** error occurs, this function does not return. Instead, it outputs a
** message to stderr and exits.
*/
static void bdListen(DaemonCtx *p){
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
    if( p->cmd.zAddr ){
      addr.sin_addr.s_addr = inet_addr(p->cmd.zAddr);
      if( addr.sin_addr.s_addr==INADDR_NONE ){
        fatal_error("invalid IP address: %s", p->cmd.zAddr);
      }
    }else{
      addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    addr.sin_port = htons(iPort);

    if( bind(p->fdListen, (struct sockaddr*)&addr, sizeof(addr))==0 ){
      if( listen(p->fdListen, 16)<0 ){
        fatal_system_error("listen()", errno);
      }else{
        p->iListenPort = iPort;
        return;
      }
    }
  }

  fatal_error("failed to bind to localhost port - tried %d to %d", 
      22002, 22002+nAttempt-1
  );
}

/*
** This is called by the daemon main loop whenever there is a new client
** connection.
*/
static void bdNewConnection(DaemonCtx *p){
  struct sockaddr_in addr;        /* Address of new client */
  socklen_t len = sizeof(addr);   /* Size of addr in bytes */
  DClient *pNew;                  /* New client object */

  pNew = (DClient*)bcvMallocZero(sizeof(DClient));
  pNew->fd = accept(p->fdListen, (struct sockaddr*)&addr, &len);
  if( pNew->fd<0 ){
    fatal_system_error("accept()", errno);
  }
  pNew->iClientId = ++p->iNextId;
  pNew->pNext = p->pClientList;
  pNew->pCtx = p;
  pNew->nRef = 1;
  p->pClientList = pNew;

  if( p->cmd.mLog & BCV_LOG_MESSAGE ){
    daemon_msg_log(p, "%d->D: connect!", pNew->iClientId);
  }
}

/*
** Return a string representation of the array of unsigned integers passed
** via the two arguments. e.g. "1,2,3,4,5". It is the responsibility of the
** caller to eventually free the returned buffer using sqlite3_free().
*/
static char *bdArrayToString(u32 *aInt, int nInt){
  char *zRet = 0;
  int ii;
  for(ii=0; ii<nInt; ii++){
    zRet = bcvMprintf("%z%s%d", zRet, zRet?",":"", (int)aInt[ii]);
  }
  return zRet;
}

static void bdLogRecvMsg(DaemonCtx *p, DClient *pClient, BcvMessage *pMsg){
  if( p->cmd.mLog & BCV_LOG_MESSAGE ){
    switch( pMsg->eType ){
      case BCV_MESSAGE_ATTACH:
        daemon_msg_log(p, "%d->D: ATTACH"
            "(\"%s\", \"%s\", \"%s\", \"%s\", %d bytes of auth-data, 0x%02.x)",
            pClient->iClientId,
            pMsg->u.attach.zStorage,
            pMsg->u.attach.zAccount,
            pMsg->u.attach.zContainer,
            pMsg->u.attach.zAlias,
            bcvStrlen(pMsg->u.attach.zAuth),
            pMsg->u.attach.flags
        );
        break;

      case BCV_MESSAGE_DETACH:
        daemon_msg_log(p, "%d->D: "
            "DETACH(\"%s\")", pClient->iClientId, pMsg->u.detach.zName
        );
        break;

      case BCV_MESSAGE_VTAB:
        daemon_msg_log(p, "%d->D: "
            "VTAB(\"%s\", \"%s\", \"%s\")", pClient->iClientId, 
            pMsg->u.vtab.zVtab,
            pMsg->u.vtab.zContainer,
            pMsg->u.vtab.zDatabase
        );
        break;

      case BCV_MESSAGE_READ: {
        char *zArray = bdArrayToString(pMsg->u.read.aMru, pMsg->u.read.nMru);
        daemon_msg_log(p, "%d->D: READ(%d, [%s])", 
            pClient->iClientId, (int)pMsg->u.read.iBlk, zArray
        );
        sqlite3_free(zArray);
        break;
      }

      case BCV_MESSAGE_END: {
        char *zArray = bdArrayToString(pMsg->u.end.aMru, pMsg->u.end.nMru);
        daemon_msg_log(p, "%d->D: END([%s])", pClient->iClientId, zArray);
        sqlite3_free(zArray);
        break;
      }

      case BCV_MESSAGE_HELLO:
        daemon_msg_log(p, "%d->D: "
            "HELLO(\"%s\",\"%s\")", pClient->iClientId, 
            pMsg->u.hello.zContainer, pMsg->u.hello.zDatabase
        );
        break;

      case BCV_MESSAGE_CMD: {
        int eCmd = pMsg->u.cmd.eCmd;
        daemon_msg_log(p, "%d->D: CMD(%s)", pClient->iClientId, 
          eCmd==BCV_CMD_POLL ? "poll" : "???"
        );
        break;
      }

      case BCV_MESSAGE_PASS:
        daemon_msg_log(p, "%d->D: "
            "PASS(%d bytes of auth data)", pClient->iClientId, 
            bcvStrlen(pMsg->u.pass.zAuth)
        );
        break;

      case BCV_MESSAGE_PREFETCH:
        daemon_msg_log(p, "%d->D: "
            "PREFETCH(%d bytes of auth data, %d, %d)", pClient->iClientId, 
            bcvStrlen(pMsg->u.prefetch.zAuth), pMsg->u.prefetch.nRequest,
            pMsg->u.prefetch.nMs
        );
        break;
      default:
        assert( 0 );
    }
  }
}

static void bdLogSendMsg(DaemonCtx *p, DClient *pClient, BcvMessage *pMsg){
  if( p->cmd.mLog & BCV_LOG_MESSAGE ){
    switch( pMsg->eType ){
      case BCV_MESSAGE_REPLY:
        daemon_msg_log(p, "D->%d: REPLY(%d, \"%s\")",
            pClient->iClientId,
            pMsg->u.error_r.errCode,
            pMsg->u.error_r.zErrMsg
        );
        break;
      case BCV_MESSAGE_VTAB_REPLY:
        daemon_msg_log(p, "D->%d: VTAB_REPLY(%d bytes...)", 
            pClient->iClientId, pMsg->u.vtab_r.nData
        );
        break;
      case BCV_MESSAGE_HELLO_REPLY:
        daemon_msg_log(p, 
            "D->%d: HELLO_REPLY(%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,%d)",
            pClient->iClientId, 
            pMsg->u.hello_r.errCode,
            pMsg->u.hello_r.zErrMsg,
            pMsg->u.hello_r.zStorage,
            pMsg->u.hello_r.zAccount,
            pMsg->u.hello_r.zContainer,
            pMsg->u.hello_r.szBlk,
            pMsg->u.hello_r.bEncrypted
        );
        break;
      case BCV_MESSAGE_READ_REPLY: {
        char *z = bdArrayToString(pMsg->u.read_r.aBlk, pMsg->u.read_r.nBlk);
        daemon_msg_log(p, "D->%d: READ_REPLY(%d, \"%s\", [%s])",
            pClient->iClientId,
            pMsg->u.read_r.errCode,
            pMsg->u.read_r.zErrMsg,
            z
        );
        sqlite3_free(z);
        break;
      }

      case BCV_MESSAGE_PASS_REPLY: {
        daemon_msg_log(p, "D->%d: PASS_REPLY(%d, \"%s\", 16 bytes of key)",
            pClient->iClientId,
            pMsg->u.pass_r.errCode,
            pMsg->u.pass_r.zErrMsg
        );
        break;
      }

      case BCV_MESSAGE_PREFETCH_REPLY: {
        daemon_msg_log(p, "D->%d: PREFETCH_REPLY(%d, \"%s\", %d, %d)",
            pClient->iClientId,
            pMsg->u.prefetch_r.errCode,
            pMsg->u.prefetch_r.zErrMsg,
            pMsg->u.prefetch_r.nOutstanding,
            pMsg->u.prefetch_r.nOnDemand
        );
        break;
      }
      default:
        assert( 0 );
        break;
    }
  }
}

static void bdReleaseEntryRefs(DClient *pClient){
  int nRef = pClient->pManDb->nBlkLocal;
  int ii;
  for(ii=0; ii<nRef; ii++){
    CacheEntry *pEntry = pClient->apRef[ii];
    if( pEntry ){
      assert( pEntry->nRef>0 );
      pEntry->nRef--;
    }
  }
  memset(pClient->apRef, 0, sizeof(CacheEntry*) * nRef);
}

static void bdClientDecrRefcount(DClient *pClient){
  pClient->nRef--;
  if( pClient->nRef==0 ){
    sqlite3_free(pClient);
  }
}

static void bdDisconnectClient(
  DaemonCtx *p, 
  DClient *pClient, 
  const char *zWhy
){
  DClient **pp;
  Container *pCont = pClient->pCont;

  daemon_msg_log(p, "%d->D: disconnect.. (%s)", pClient->iClientId, zWhy);

  /* Decrement the ref-count on a container, if this client was associated
  ** with one.  */
  if( pCont ){
    assert( pCont->nClient>0 );
    pCont->nClient--;
  }

  /* Remove the client from the linked list at DaemonCtx.pClientList */
  for(pp=&p->pClientList; *pp!=pClient; pp=&(*pp)->pNext);
  *pp = pClient->pNext;

  /* Close the socket connection */
  bcv_close_socket(pClient->fd);

  if( pClient->apRef ){
    bdReleaseEntryRefs(pClient);
    sqlite3_free(pClient->apRef);
  }
  bcvManifestDeref(pClient->pMan);

  bcvContainerClose(pClient->pBcv);
  sqlite3_free(pClient->zAuth);
  sqlite3_free(pClient->prefetch.zErrMsg);

  /* Free the client structure itself */
  bdClientDecrRefcount(pClient);
}

static void bdSendMsg(DaemonCtx *p, DClient *pClient, BcvMessage *pMsg){
  int rc;
  bdLogSendMsg(p, pClient, pMsg);
  rc = bcvSendMsg(pClient->fd, pMsg);
  if( rc!=SQLITE_OK ){
    bdDisconnectClient(p, pClient, "send() failure");
  }
}

static DClient *bdFetchClient(FetchCtx *pDLCtx){
  return pDLCtx->pClient->fd==INVALID_SOCKET ? 0 : pDLCtx->pClient;
}

static void bdBlockDownloadFree(FetchCtx *p){
  bdClientDecrRefcount(p->pClient);
  bcvEncryptionKeyFree(p->pKey);
  sqlite3_free(p);
}

/*
** Create a directory for container zCont.
*/
static void bcvCreateDir(int *pRc, BcvCommon *p, const char *zCont){
  struct stat buf;
  char *zDir = bcvMprintfRc(pRc, 
      "%s" BCV_PATH_SEPARATOR "%s", p->zDir, zCont
  );
  if( *pRc==SQLITE_OK && stat(zDir, &buf)<0 ){
    if( osMkdir(zDir, 0755)<0 && stat(zDir, &buf)<0 ){
      *pRc = SQLITE_CANTOPEN;
    }
  }
  sqlite3_free(zDir);
}

static void bdAttachCb(
  void *pCtx, 
  int errCode, 
  char *zETag, 
  const u8 *aData, 
  int nData
){
  BcvMessage reply;
  FetchCtx *pDLCtx = (FetchCtx*)pCtx;
  DClient *pClient = bdFetchClient(pDLCtx);
  BcvCommon *pCommon = &pDLCtx->pCtx->c;
  BcvMessage *pMsg = 0;
  Container *pCont = 0;
  int rc = errCode;
  Manifest *pMan = 0;
  char *zErr = 0;
  const char *zName = 0;

  /* If the client has already disconnected, do nothing. */
  bdBlockDownloadFree(pDLCtx);
  if( pClient==0 ) return;

  /* Check if there is already a container of the same name attached. 
  ** If there is, it is an error.  */
  pMsg = pClient->pMsg;
  zName = pMsg->u.attach.zAlias?pMsg->u.attach.zAlias:pMsg->u.attach.zContainer;
  if( rc==SQLITE_OK && bcvfsFindContAlias(pCommon, zName, 0) ){
    if( pMsg->u.attach.flags & SQLITE_BCV_ATTACH_IFNOT ) goto attach_cb_out;
    zErr = sqlite3_mprintf("container already attached: %s", zName);
    rc = SQLITE_ERROR;
  }

  /* Create a directory to use */
  bcvCreateDir(&rc, pCommon, zName);

  /* Parse the manifest */
  if( rc==SQLITE_OK ){
    rc = bcvManifestParseCopy(aData, nData, zETag, &pMan, &zErr);
  }

  if( rc==SQLITE_OK ){
    if( pCommon->szBlk==0 ){
      pCommon->szBlk = pMan->szBlk;
    }else if( pCommon->szBlk!=pMan->szBlk ){
      zErr = sqlite3_mprintf("block size mismatch for container: %s", zName);
      rc = SQLITE_ERROR;
    }
  }

  /* Allocate the container object */
  pCont = bcvContainerAlloc(&rc, pCommon,
      pMsg->u.attach.zStorage, pMsg->u.attach.zAccount,
      pMsg->u.attach.zContainer, zName
  );

  if( rc==SQLITE_OK && (pMsg->u.attach.flags & SQLITE_BCV_ATTACH_SECURE) ){
    sqlite3_randomness(BCV_LOCAL_KEYSIZE, pCont->aKey);
    pCont->pKey = bcvEncryptionKeyNew(pCont->aKey);
    if( pCont->pKey==0 ) fatal_oom_error();
    pCont->iEnc = ++pClient->pCtx->iNextId;
  }

  /* Parse the manifest and install it into the container. */
  if( rc==SQLITE_OK ){
    rc = bcvManifestInstall(pCommon, pCont, pMan);
  }

  /* If successful, link the container into the list */
  if( rc==SQLITE_OK ){
    pCont->pMan = pMan;
    pCont->pNext = pCommon->pCList;
    pCommon->pCList = pCont;
  }else{
    bcvManifestFree(pMan);
    bcvContainerFree(pCont);
  }

 attach_cb_out:
  memset(&reply, 0, sizeof(reply));
  reply.eType = BCV_MESSAGE_REPLY;
  reply.u.error_r.errCode = rc;
  reply.u.error_r.szBlk = pCommon->szBlk;
  if( rc!=SQLITE_OK ){
    reply.u.error_r.zErrMsg = zErr ? zErr : zETag;
  }

  bdSendMsg(pClient->pCtx, pClient, &reply);
  bcvContainerClose(pClient->pBcv);
  pClient->pBcv = 0;
  pClient->pMsg = 0;
  sqlite3_free(pMsg);
  sqlite3_free(zErr);
}

static void bdDispatchFetch(
  BcvDispatch *p,
  BcvContainer *pCont,
  const char *zFile,
  const char *zETag,
  const void *pMd5,
  FetchCtx *pCtx,
  void (*x)(void*, int rc, char *zETag, const u8 *aData, int nData)
){
  int rc = bcvDispatchFetch(p, pCont, zFile, zETag, pMd5, (void*)pCtx, x);
  if( rc!=SQLITE_OK ) fatal_oom_error();
}

static FetchCtx *bdFetchCtx(
  DClient *pClient,
  CacheEntry *pEntry
){
  FetchCtx *p = (FetchCtx*)bdMallocZero(sizeof(FetchCtx));
  p->pClient = pClient;
  p->pEntry = pEntry;
  p->pCtx = pClient->pCtx;
  p->pCont = pClient->pCont;
  if( p->pCont && p->pCont->pKey ){
    p->pKey = bcvEncryptionKeyRef(p->pCont->pKey);
  }
  pClient->nRef++;
  return p;
}

/*
** Handle an ATTACH message from a client.
**
** This routine just kicks off a network request for the manifest file of
** the specified container - the callback is bdAttachCb. It does not check
** that the container is not already attached, as this has to be checked
** again after the network request is answered anyway.
*/
static void bdHandleAttach(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  int rc = SQLITE_OK;;
  BcvContainer *pBcv = 0;
  char *zErr = 0;

  assert( pClient->pBcv==0 );
  assert( pClient->pMsg==0 );

  rc = bcvContainerOpen(
      pMsg->u.attach.zStorage, pMsg->u.attach.zAccount,
      pMsg->u.attach.zAuth, pMsg->u.attach.zContainer, 
      &pBcv, &zErr
  );
  if( rc==SQLITE_OK ){
    FetchCtx *pFetch = bdFetchCtx(pClient, 0);
    pClient->pMsg = pMsg;
    pClient->pBcv = pBcv;
    bdDispatchFetch(p->pDisp, pBcv, BCV_MANIFEST_FILE, 0,0, pFetch, bdAttachCb);
  }else{
    BcvMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_REPLY;
    reply.u.error_r.errCode = rc;
    reply.u.error_r.zErrMsg = zErr;
    bdSendMsg(p, pClient, &reply);
    bcvContainerClose(pBcv);
    sqlite3_free(pMsg);
    pClient->pMsg = 0;
    pClient->pBcv = 0;
  }
  sqlite3_free(zErr);
}

/*
** Handle an DETACH message from a client.
*/
static void bdHandleDetach(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  BcvMessage reply;
  Container *pCont = 0;
  char *zErr = 0;
  int rc = SQLITE_OK;

  pCont = bcvfsFindContAlias(&p->c, pMsg->u.detach.zName, &zErr);
  if( !pCont ){
    rc = SQLITE_ERROR;
  }else if( pCont->nClient>0 ){
    rc = SQLITE_ERROR;
    zErr = sqlite3_mprintf("container is busy: %s", pMsg->u.detach.zName);
  }else{
    rc = bcvContainerDetachAndFree(&p->c, pCont);
    if( rc!=SQLITE_OK ) fatal_sql_error(p->c.bdb, "bcvHandleDetach");
  }

  memset(&reply, 0, sizeof(reply));
  reply.eType = BCV_MESSAGE_REPLY;
  reply.u.error_r.errCode = rc;
  reply.u.error_r.zErrMsg = zErr;
  bdSendMsg(p, pClient, &reply);
  sqlite3_free(zErr);
  sqlite3_free(pMsg);
}

/*
** Handle a message of type BCV_MESSAGE_VTAB from client pClient.
*/
static void bdHandleVtab(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  const char *zTab = pMsg->u.vtab.zVtab;
  const char *zCont = pMsg->u.vtab.zContainer;
  const char *zDb = pMsg->u.vtab.zDatabase;
  const u32 colUsed = pMsg->u.vtab.colUsed;

  int rc = SQLITE_OK;
  BcvMessage reply;
  u8 *aData = 0;
  int nData = 0;

  memset(&reply, 0, sizeof(reply));
  reply.eType = BCV_MESSAGE_VTAB_REPLY;

  aData = bcvDatabaseVtabData(&rc, &p->c, zTab, zCont, zDb, colUsed, &nData);
  reply.u.vtab_r.aData = aData;
  reply.u.vtab_r.nData = nData;
  bdSendMsg(p, pClient, &reply);

  sqlite3_free(aData);
  sqlite3_free(pMsg);
}

/*
** Handle a message of type BCV_MESSAGE_HELLO from client pClient.
*/
static void bdHandleHello(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  const char *zCont = pMsg->u.hello.zContainer;
  const char *zDb = pMsg->u.hello.zDatabase;
  Container *pCont = 0;
  ManifestDb *pDb = 0;
  BcvMessage reply;
  char *zErr = 0;

  pCont = bcvfsFindContAlias(&p->c, zCont, &zErr);
  if( pCont && zDb ){
    pDb = bcvfsFindDatabase(pCont->pMan, zDb, -1);
    if( pDb==0 ){
      zErr = bcvMprintf("no such database: /%s/%s", zCont, zDb);
      pCont = 0;
    }
  }

  memset(&reply, 0, sizeof(BcvMessage));
  reply.eType = BCV_MESSAGE_HELLO_REPLY;
  if( pCont==0 ){
    reply.u.hello_r.errCode = SQLITE_CANTOPEN;
    reply.u.hello_r.zErrMsg = zErr;
  }else{
    pClient->pCont = pCont;
    if( pDb ) pClient->iDbId = pDb->iDbId;
    pCont->nClient++;
    reply.u.hello_r.szBlk = pCont->pMan->szBlk;
    reply.u.hello_r.zStorage = pCont->zStorage;
    reply.u.hello_r.zAccount = pCont->zAccount;
    reply.u.hello_r.zContainer = pCont->zContainer;
    reply.u.hello_r.iDbId = pClient->iDbId;
    reply.u.hello_r.szBlk = pCont->pMan->szBlk;
    reply.u.hello_r.bEncrypted = (pCont->pKey!=0);
  }

  bdSendMsg(p, pClient, &reply);
  sqlite3_free(pMsg);
  sqlite3_free(zErr);
}

/*
** Send a message of type READ_REPLY to client pClient. The message
** contains error code rc and error message zErr.
*/
static void bdReadReplyError(
  DClient *pClient,
  int rc,
  const char *zErr
){
  BcvMessage reply;
  memset(&reply, 0, sizeof(reply));
  reply.eType = BCV_MESSAGE_READ_REPLY;
  reply.u.read_r.errCode = rc;
  reply.u.read_r.zErrMsg = zErr;
  bdSendMsg(pClient->pCtx, pClient, &reply);
}

static void bdHandleReadMsg(DaemonCtx *p, DClient *pClient);

static void bdWriteBlock(
  BcvCommon *pCommon, 
  CacheEntry *pEntry
){
  int rc = SQLITE_OK;
  sqlite3_stmt *pStmt = pCommon->pInsertBlock;
  sqlite3_bind_int(pStmt, 1, pEntry->iPos);
  sqlite3_bind_blob(pStmt, 2, pEntry->aName, pEntry->nName, SQLITE_STATIC);
  sqlite3_step(pStmt);
  rc = sqlite3_reset(pStmt);
  if( rc ) fatal_sql_error(pCommon->bdb, "bdWriteBlock()");
  sqlite3_clear_bindings(pStmt);
}

static int bdWriteFile(
  sqlite3_file *pFd, 
  BcvEncryptionKey *pKey,
  const u8 *aData, 
  int nData, 
  i64 iOff
){
  u8 aBuf[512];
  int rc = SQLITE_OK;
  int i;

  assert( (nData % sizeof(aBuf))==0 );
  for(i=0; i<nData && rc==SQLITE_OK; i+=sizeof(aBuf)){
    const u8 *aWrite = &aData[i];
    if( pKey ){
      memcpy(aBuf, aWrite, sizeof(aBuf));
      bcvEncrypt(pKey, iOff+i, 0, aBuf, sizeof(aBuf));
      aWrite = aBuf;
    }
    rc = pFd->pMethods->xWrite(pFd, aWrite, sizeof(aBuf), iOff+i);
  }
  return rc;
}

static void bdBlockDownloadCb(
  void *pCtx, 
  int errCode, 
  char *zETag, 
  const u8 *aData, 
  int nData
){
  FetchCtx *pDLCtx = (FetchCtx*)pCtx;
  DClient *pClient = bdFetchClient(pDLCtx);
  DaemonCtx *p = pDLCtx->pCtx;
  CacheEntry *pEntry = pDLCtx->pEntry;
  int rc = errCode;

  if( rc==SQLITE_OK ){
    if( p->cmd.mLog & BCV_LOG_EVENT ){
      char zName[BCV_MAX_FSNAMEBYTES];
      bcvBlockidToText(pClient->pMan, pEntry->aName, zName);
      daemon_event_log(p, "writing %s to cache slot %d", zName, pEntry->iPos);
    }
    i64 iOff = (pEntry->iPos * p->c.szBlk);
    rc = bdWriteFile(p->pCacheFile, pDLCtx->pKey, aData, nData, iOff);
  }

  if( rc==SQLITE_OK ){
    pEntry->bValid = 1;
    bcvfsLruAdd(&p->c, pEntry);
    if( pDLCtx->pKey==0 ) bdWriteBlock(&p->c, pEntry);
  }else{
    /* An error has occurred. Unless it has already disconnected (pClient==0),
    ** send an error reply to the client that initiated this request. Then
    ** mark the cache entry as unused.  */
    if( pClient ){
      bdReadReplyError(pClient, errCode, zETag);
      sqlite3_free(pClient->pMsg);
      pClient->pMsg = 0;
    }
    bcvfsHashRemove(&p->c, pEntry);
    bcvfsUnusedAdd(&p->c, pEntry);
  }

  bdBlockDownloadFree(pDLCtx);
}

/*
** Ensure that the BCV handle at pClient->pBcv is one configured to use
** authentication string zAuth. Then return a copy of it.
*/
static BcvContainer *bdUpdateBCV(DClient *pClient, const char *zAuth){
  if( pClient->pBcv==0 || bcvStrcmp(zAuth, pClient->zAuth) ){
    char *zErr = 0;
    int rc = SQLITE_OK;

    bcvContainerClose(pClient->pBcv);
    sqlite3_free(pClient->zAuth);
    pClient->pBcv = 0;
    pClient->zAuth = 0;

    rc = bcvContainerOpen(
        pClient->pCont->zStorage, pClient->pCont->zAccount,
        zAuth, pClient->pCont->zContainer, &pClient->pBcv, &zErr
    );
    if( rc!=SQLITE_OK ){
      fatal_error("failed to create BCV: %s", zErr);
    }
    pClient->zAuth = bdStrdup(zAuth);
  }
  return pClient->pBcv;
}

static i64 bdFindMinTick(DaemonCtx *pCtx){
  BcvCommon *p = &pCtx->c;
  i64 iRet = 0;
  CacheEntry *pEntry = 0;
  int nAllow = ((p->nMaxCache+p->szBlk-1) / p->szBlk);
  int nReserve = MIN(nAllow/5, BCV_MAX_RESERVE);
  int nIgnore = p->nBlk - (nAllow - nReserve);

  for(pEntry=p->pUnused; pEntry; pEntry=pEntry->pHashNext){
    nIgnore--;
  }
  for(pEntry=p->pLruFirst; pEntry && nIgnore>0; pEntry=pEntry->pLruNext){
    iRet = pEntry->iLruTick;
    nIgnore--;
  }

  return iRet;
}

/*
**
*/
static CacheEntry *bdHashFind(
  BcvCommon *pCommon,             /* Hash tables to search */
  const u8 *aName, int nName,     /* Name of block to search for */
  int iEnc                        /* Required encryption index */
){
  CacheEntry *pEntry = bcvfsHashFind(pCommon, aName, nName);
  if( pEntry && pEntry->iEnc!=iEnc ){
    while( (pEntry = pEntry->pHashNext) ){
      if( pEntry->iEnc==iEnc 
       && pEntry->nName==nName 
       && memcpy(pEntry->aName, aName, nName)
      ){
        break;
      }
    }
  }
  return pEntry;
}

static void bdHandleReadMsg(DaemonCtx *p, DClient *pClient){
  const int iEnc = pClient->pCont->iEnc;
  int iBlk = pClient->pMsg->u.read.iBlk;
  int nName = NAMEBYTES(pClient->pMan);
  u8 *aName = &pClient->pManDb->aBlkLocal[nName * iBlk];
  CacheEntry *pEntry = 0;

  /* Check if the required block is already in the cache. There
  ** are three possibilities: 
  **
  **   1) The block is not in the cache and is not currently being
  **      downloaded. In this case start the download.
  **
  **   2) The block is in the cache but is not ready to use - it
  **      is still being downloaded. In this case just wait.
  **
  **   3) The block is in the cache and ready to use. The READ
  **      message can be answered immediately in this case.
  */
  pEntry = bdHashFind(&p->c, aName, nName, iEnc);
  if( pEntry==0 ){
    /* Case (1) - download the block */
    BcvContainer *pBcv = pClient->pBcv;
    int rc = SQLITE_OK;
    char zName[BCV_MAX_FSNAMEBYTES];
    CacheEntry *pEntry = 0;
    FetchCtx *pDLCtx = 0;

    pEntry = bcvfsAllocCacheEntry(&rc, &p->c);
    if( rc!=SQLITE_OK ) fatal_oom_error();
    memcpy(pEntry->aName, aName, nName);
    pEntry->nName = nName;
    pEntry->iEnc = iEnc;
    bcvfsHashAdd(&p->c, pEntry);

    pDLCtx = bdFetchCtx(pClient, pEntry);
    bcvfsBlockidToText(aName, nName, zName);
    bdDispatchFetch(p->pDisp, pBcv, zName, 0, 0, pDLCtx, bdBlockDownloadCb);

  }else if( pEntry->bValid==0 ){
    /* Case (2) - wait */
  }else{
    /* Case (3) - answer the READ message now */
    BcvMessage reply;
    ManifestDb *pDb = pClient->pManDb;
    int ii;
    i64 iMinTick = bdFindMinTick(p);

    /* Shift pEntry to the end of the LRU list */
    bcvfsLruRemoveIf(&p->c, pEntry);
    bcvfsLruAddIf(&p->c, pEntry);

    memset(&reply, 0, sizeof(BcvMessage));
    reply.eType = BCV_MESSAGE_READ_REPLY;
    reply.u.read_r.aBlk = (u32*)bdMallocZero(pDb->nBlkLocal * sizeof(u32));
    reply.u.read_r.nBlk = pDb->nBlkLocal;
    memset(pClient->apRef, 0, pDb->nBlkLocal * sizeof(CacheEntry*));
    for(ii=0; ii<pDb->nBlkLocal; ii++){
      const u8 *aName = &pDb->aBlkLocal[ii*nName];
      CacheEntry *pEntry = bdHashFind(&p->c, aName, nName, iEnc);
      if( pEntry && pEntry->bValid && pEntry->iLruTick>iMinTick ){
        reply.u.read_r.aBlk[ii] = pEntry->iPos + 1;
        pEntry->nRef++;
        pClient->apRef[ii] = pEntry;
      }
    }

    bdSendMsg(p, pClient, &reply);
    sqlite3_free(pClient->pMsg);
    sqlite3_free(reply.u.read_r.aBlk);
    pClient->pMsg = 0;
  }
}

static void log_db_blocks(DaemonCtx *pCtx, Manifest *pMan, int iDb){
  ManifestDb *pDb = &pMan->aDb[iDb];
  int ii;
  for(ii=0; ii<pDb->nBlkLocal; ii++){
    CacheEntry *pEntry = 0;
    u8 *aName = &pDb->aBlkLocal[NAMEBYTES(pMan) * ii];
    char zName[BCV_MAX_FSNAMEBYTES];
    bcvBlockidToText(pMan, aName, zName);
    pEntry = bcvfsHashFind(&pCtx->c, aName, NAMEBYTES(pMan));
    daemon_event_log(pCtx, "block %d of \"%s\" is %s (cache-file-pos=%d)",
        ii, pDb->zDName, zName, pEntry?pEntry->iPos:-1
    );
  }
}

/*
** This callback is invoked when a GET request for a manifest file made
** by bdHandlePollCmdMsg() is ready.
*/
static void bdPollCb(
  void *pCtx,                     /* Pointer to requesting DClient */
  int errCode,                    /* Error code */
  char *zETag,                    /* E-Tag (rc==SQLITE_OK) or error message */
  const u8 *aData, int nData      /* Serialized manifest */
){
  FetchCtx *pDLCtx = (FetchCtx*)pCtx;
  DClient *pClient = bdFetchClient(pDLCtx);
  Container *pCont = pDLCtx->pCont;
  DaemonCtx *p = pDLCtx->pCtx;
  char *zErr = 0;                 /* Error message (from sqlite3_malloc) */
  int rc = errCode;               /* Error code */
  Manifest *pMan = 0;             /* Parsed manifest object */

  if( rc!=SQLITE_OK ){
    zErr = bdStrdup(zETag);
  }else{
    rc = bcvManifestParseCopy(aData, nData, zETag, &pMan, &zErr);
  }

  if( rc==SQLITE_OK ){
    rc = bcvManifestUpdate(&p->c, pCont, pMan, &zErr);
  }
  pCont->eState = CONTAINER_STATE_NONE;


  if( pClient ){
    BcvMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_REPLY;
    reply.u.error_r.errCode = rc;
    reply.u.error_r.zErrMsg = zErr;
    sqlite3_free(pClient->pMsg);
    pClient->pMsg = 0;
    bdSendMsg(pClient->pCtx, pClient, &reply); 
  }

  sqlite3_free(zErr);
  bdBlockDownloadFree(pDLCtx);
}

/*
** The client passed as the second argument is currently processing a
** message of type BCV_MESSAGE_CMD, subtype BCV_CMD_POLL. If possible,
** get on with handling this message.
*/
static void bdHandlePollCmdMsg(DaemonCtx *p, DClient *pClient){
  Container *pCont = pClient->pCont;
  assert( pCont && pClient->iDbId==0 && pClient->pMsg );

  /* If the container is currently in state NONE, proceed with the poll.
  ** Otherwise, do nothing - this function will be called again at some
  ** point when the container state is NONE.  */
  if( pCont->eState==CONTAINER_STATE_NONE ){
    FetchCtx *pDLCtx = bdFetchCtx(pClient, 0);
    BcvDispatch *pDisp = p->pDisp;
    BcvContainer *pBcv = bdUpdateBCV(pClient, pClient->pMsg->u.cmd.zAuth);
    bdDispatchFetch(pDisp, pBcv, BCV_MANIFEST_FILE, 0, 0, pDLCtx, bdPollCb);
    pCont->eState = CONTAINER_STATE_POLL;
  }
}

/*
** Array aMru[] (size nMru) is an MRU array just received as part of
** a READ or END message. Shuffle entries within the LRU list accordingly.
*/
static void bdProcessMRUList(
  DaemonCtx *p, 
  DClient *pClient,
  u32 *aMru,
  int nMru
){
  CacheEntry **apRef = pClient->apRef;
  int ii;
  for(ii=nMru-1; ii>=0; ii--){
    CacheEntry *pEntry = apRef[ aMru[ii] ];
    assert( (int)aMru[ii]<pClient->pManDb->nBlkLocal && pEntry );
    assert( pEntry->bValid && pEntry->bDirty==0 );

    bcvfsLruRemoveIf(&p->c, pEntry);
    bcvfsLruAddIf(&p->c, pEntry);
  }
}

/*
** Handle a message of type BCV_MESSAGE_READ from client pClient.
*/
static void bdHandleRead(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  int rc = SQLITE_OK;
  char *zErr = 0;

  /* Take a reference to the container manifest if this is the first
  ** READ message of the transaction. Or, if this is the second or 
  ** subsequent request of the transaction, release all currently held 
  ** references.  */
  if( pClient->pMan==0 ){
    Manifest *pMan = pClient->pCont->pMan;
    ManifestDb *pDb = bcvManifestDbidToDb(pMan, pClient->iDbId);
    if( pDb==0 ){
      zErr = sqlite3_mprintf("database has been deleted");
      rc = SQLITE_ERROR;
    }else{
      int nByte = pDb->nBlkLocal * sizeof(CacheEntry*);
      bcvManifestRef(pMan);
      pClient->pMan = pMan;
      pClient->pManDb = pDb;
      pClient->apRef = (CacheEntry**)bdMallocZero(nByte);
    }
  }else{
    bdProcessMRUList(p, pClient, pMsg->u.read.aMru, pMsg->u.read.nMru);
    bdReleaseEntryRefs(pClient);
  }

  bdUpdateBCV(pClient, pMsg->u.read.zAuth);

  /* If no error has occurred, try to handle the READ message now. 
  ** Otherwise, return an error to the client.  */
  if( rc==SQLITE_OK ){
    pClient->pMsg = pMsg;
  }else{
    bdReadReplyError(pClient, SQLITE_ERROR, 0);
    sqlite3_free(pMsg);
  }
}

/*
** Handle a message of type BCV_MESSAGE_END from client pClient.
*/
static void bdHandleEnd(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  bdProcessMRUList(p, pClient, pMsg->u.end.aMru, pMsg->u.end.nMru);
  bdReleaseEntryRefs(pClient);
  sqlite3_free(pClient->apRef);
  pClient->apRef = 0;
  bcvManifestDeref(pClient->pMan);
  pClient->pMan = 0;
  pClient->pManDb = 0;
  sqlite3_free(pMsg);
}

static void bdPassCb(
  void *pCtx, 
  int errCode, 
  char *zETag, 
  const u8 *aData, 
  int nData
){
  BcvMessage reply;
  FetchCtx *pDLCtx = (FetchCtx*)pCtx;
  DClient *pClient = bdFetchClient(pDLCtx);
  DaemonCtx *p = pDLCtx->pCtx;

  bdBlockDownloadFree(pDLCtx);

  memset(&reply, 0, sizeof(reply));
  reply.eType = BCV_MESSAGE_PASS_REPLY;
  if( errCode==SQLITE_OK ){
    reply.u.pass_r.aKey = pClient->pCont->aKey;
  }else{
    reply.u.pass_r.errCode = errCode;
    reply.u.pass_r.zErrMsg = zETag;
  }

  bdSendMsg(p, pClient, &reply);
}

/*
** Handle a PASS message from a client.
*/
static void bdHandlePass(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  BcvContainer *pBcv = 0;
  FetchCtx *pDLCtx = bdFetchCtx(pClient, 0);
 
  pBcv = bdUpdateBCV(pClient, pMsg->u.pass.zAuth);
  bdDispatchFetch(p->pDisp, pBcv, BCV_MANIFEST_FILE, 0, 0, pDLCtx, bdPassCb);
  sqlite3_free(pMsg);
}

static void bdSendPrefetchReply(DClient *pClient){
  BcvMessage msg;
  DClient *pIter = 0;
  int nOnDemand = 0;

  int rc = pClient->prefetch.errCode;
  if( rc==SQLITE_OK 
   && pClient->prefetch.nOutstanding==0 
   && pClient->prefetch.iNext>=pClient->pManDb->nBlkLocal 
  ){
    rc = SQLITE_DONE;
  }

  for(pIter=pClient->pCtx->pClientList; pIter; pIter=pIter->pNext){
    if( pIter->pMsg && pIter->pMsg->eType==BCV_MESSAGE_READ ){
      nOnDemand++;
    }
  }

  memset(&msg, 0, sizeof(msg));
  msg.eType = BCV_MESSAGE_PREFETCH_REPLY;
  msg.u.prefetch_r.errCode = rc;
  msg.u.prefetch_r.zErrMsg = pClient->prefetch.zErrMsg;
  msg.u.prefetch_r.nOutstanding = pClient->prefetch.nOutstanding;
  msg.u.prefetch_r.nOnDemand = nOnDemand;
  bdSendMsg(pClient->pCtx, pClient, &msg);

  pClient->prefetch.bReply = 0;
  pClient->prefetch.errCode = SQLITE_OK;
  sqlite3_free(pClient->prefetch.zErrMsg);
  pClient->prefetch.zErrMsg = 0;
  if( rc==HTTP_AUTH_ERROR ){
    pClient->prefetch.iNext = 0;
  }
}

/*
** This callback is invoked to announce that a block requested by 
** bdHandlePrefetch() had finished downloading. Or that an error has occurred
** while attempting the same.
*/
static void bdPrefetchCb(
  void *pCtx, 
  int errCode, 
  char *zETag, 
  const u8 *aData, 
  int nData
){
  FetchCtx *pDLCtx = (FetchCtx*)pCtx;
  DClient *pClient = bdFetchClient(pDLCtx);
  DaemonCtx *p = pDLCtx->pCtx;
  CacheEntry *pEntry = pDLCtx->pEntry;
  int rc = errCode;

  pClient->prefetch.nOutstanding--;

  if( rc==SQLITE_OK ){
    i64 iOff = (pEntry->iPos * p->c.szBlk);
    rc = bdWriteFile(p->pCacheFile, pDLCtx->pKey, aData, nData, iOff);
  }

  if( rc==SQLITE_OK ){
    pEntry->bValid = 1;
    bcvfsLruAdd(&p->c, pEntry);
    if( pDLCtx->pKey==0 ) bdWriteBlock(&p->c, pEntry);
  }else{
    bcvfsHashRemove(&p->c, pEntry);
    bcvfsUnusedAdd(&p->c, pEntry);
  }

  bdBlockDownloadFree(pDLCtx);

  if( rc!=SQLITE_OK && pClient->prefetch.errCode==SQLITE_OK ){
    pClient->prefetch.errCode = rc;
    pClient->prefetch.zErrMsg = sqlite3_mprintf("%s", zETag);
  }

  if( pClient && pClient->prefetch.bReply ){
    bdSendPrefetchReply(pClient);
    pClient->prefetch.bReply = 0;
  }
}


static void bdHandlePrefetch(
  DaemonCtx *p, 
  DClient *pClient, 
  BcvMessage *pMsg
){
  BcvContainer *pBcv = 0;
  int rc = SQLITE_OK;

  pClient->prefetch.bReply = 1;

  pBcv = bdUpdateBCV(pClient, pMsg->u.pass.zAuth);
  if( pClient->pManDb==0 ){
    Manifest *pMan = pClient->pCont->pMan;
    ManifestDb *pDb = bcvManifestDbidToDb(pMan, pClient->iDbId);
    if( pDb ){
      bcvManifestRef(pMan);
      pClient->pMan = pMan;
      pClient->pManDb = pDb;
    }else{
      /* Error - database no longer exists */
      assert( 0 );
    }
  }

  while( pClient->prefetch.nOutstanding<pMsg->u.prefetch.nRequest
      && pClient->prefetch.iNext<pClient->pManDb->nBlkLocal
  ){
    int nName = NAMEBYTES(pClient->pMan);
    int iBlk = pClient->prefetch.iNext++;
    const u8 *aName = &pClient->pManDb->aBlkLocal[iBlk * nName];
    CacheEntry *pEntry = 0;

    pEntry = bdHashFind(&p->c, aName, nName, pClient->pCont->iEnc);
    if( pEntry==0 ){
      char zName[BCV_MAX_FSNAMEBYTES];
      FetchCtx *pDL = 0;

      pEntry = bcvfsAllocCacheEntry(&rc, &p->c);
      if( rc!=SQLITE_OK ) fatal_oom_error();
      pEntry->iEnc = pClient->pCont->iEnc;
      memcpy(pEntry->aName, aName, nName);
      pEntry->nName = nName;
      bcvfsHashAdd(&p->c, pEntry);
      assert( pEntry->bValid==0 );

      pClient->prefetch.nOutstanding++;
      pDL = bdFetchCtx(pClient, pEntry);
      bcvfsBlockidToText(pEntry->aName, nName, zName);
      bdDispatchFetch(p->pDisp, pBcv, zName, 0, 0, pDL, bdPrefetchCb);
    }
  }

  if( pClient->prefetch.nOutstanding==0 && pClient->prefetch.bReply ){
    bdSendPrefetchReply(pClient);
  }

  sqlite3_free(pMsg);
}

/*
** This is called by the daemon main loop whenever there is input detected
** on a socket associated with client pClient.
*/
static void bdHandleClientMessage(
  DaemonCtx *p, 
  DClient *pClient, 
  int *pbLogin                    /* Set to true if message is a LOGIN */
){
  BcvMessage *pMsg = 0;
  int rc = SQLITE_OK;

  assert( pClient->pMsg==0 );
  rc = bcvRecvMsg(pClient->fd, &pMsg);
  if( rc!=SQLITE_OK ){
    bdDisconnectClient(p, pClient, "client hangup");
  }else{
    bdLogRecvMsg(p, pClient, pMsg);
    switch( pMsg->eType ){
      case BCV_MESSAGE_ATTACH:
        bdHandleAttach(p, pClient, pMsg);
        break;
      case BCV_MESSAGE_DETACH:
        bdHandleDetach(p, pClient, pMsg);
        break;
      case BCV_MESSAGE_VTAB:
        bdHandleVtab(p, pClient, pMsg);
        *pbLogin = 1;
        break;
      case BCV_MESSAGE_HELLO:
        bdHandleHello(p, pClient, pMsg);
        break;
      case BCV_MESSAGE_READ:
        bdHandleRead(p, pClient, pMsg);
        *pbLogin = 1;
        break;
      case BCV_MESSAGE_END:
        bdHandleEnd(p, pClient, pMsg);
        break;
      case BCV_MESSAGE_CMD:
        pClient->pMsg = pMsg;
        break;
      case BCV_MESSAGE_PASS:
        bdHandlePass(p, pClient, pMsg);
        break;
      case BCV_MESSAGE_PREFETCH:
        bdHandlePrefetch(p, pClient, pMsg);
        break;
      default:
        fatal_error("unexpected message type: %d", pMsg->eType);
    }
  }

}


static void bdMainloop(DaemonCtx *p){
  int bClient = 0;

  while( 1 ){
    DClient *pNextClient;
    DClient *pClient;
    int nFd = 1;
    int i;
    struct curl_waitfd *aWait;    /* Array of file descriptors to select() on */
    int rc;

    /* Assemble the set of file-descriptors to wait for events on. The
    ** listening socket fd, and one socket fd for each connected client. */
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

    /* Run the dispatcher. This will execute callbacks for any completed
    ** network requests.  */
    rc = bcvDispatchRun(p->pDisp, aWait, nFd, 10000);
    if( rc!=SQLITE_OK ){
      fatal_error("bcvDispatchRun() failed - %d", rc);
    }

    /* Check for client messages */
    i = 1;
    for(pClient=p->pClientList; pClient; pClient=pNextClient){
      pNextClient = pClient->pNext;
      if( aWait[i].revents ){
        bdHandleClientMessage(p, pClient, &bClient);
      }
      i++;
    }

    /* Check for new connections */
    if( aWait[0].revents!=0 ){
      bdNewConnection(p);
    }

    /* Check if any PIN, POLL or READ clients can continue. */
    for(pClient=p->pClientList; pClient; pClient=pNextClient){
      BcvMessage *pMsg = pClient->pMsg;
      pNextClient = pClient->pNext;
      if( pMsg ){
        assert( pMsg->eType==BCV_MESSAGE_READ
             || pMsg->eType==BCV_MESSAGE_CMD
             || pMsg->eType==BCV_MESSAGE_ATTACH
        );
        if( pMsg->eType==BCV_MESSAGE_READ ){
          bdHandleReadMsg(p, pClient);
        }else if( pMsg->eType==BCV_MESSAGE_CMD ){
          assert( pMsg->u.cmd.eCmd==BCV_CMD_POLL );
          bdHandlePollCmdMsg(p, pClient);
        }
      }
    }

    /* Check if it is time to bail out (due to -autoexit option) */ 
    sqlite3_free(aWait);
    if( p->cmd.bAutoexit && bClient && p->pClientList==0 ){
      break;
    }
  }
}

/*
** Populate the portnumber.bcv file used by the [attach] and [detach]
** commands to locate the daemon's listening port.
*/
static void bdWritePortNumber(DaemonCtx *pCtx, const char *zDir){
  char *aBuf = 0;
  sqlite3_file *pFile = 0;
  int rc;
  const char *zAddr = pCtx->cmd.zAddr;
  char *zFile = bcvMprintf("%s/portnumber.bcv", zDir);

  if( zAddr==0 ) zAddr = "127.0.0.1";
  aBuf = bcvMprintf("%s:%d", zAddr, pCtx->iListenPort);

  rc = bdOpenLocalFile(0, zFile, 0, 0, &pFile);
  if( rc==SQLITE_OK ){
    rc = pFile->pMethods->xTruncate(pFile, 0);
  }
  if( rc==SQLITE_OK ){
    rc = pFile->pMethods->xWrite(pFile, (void*)aBuf, strlen(aBuf), 0);
  }
  if( rc!=SQLITE_OK ){
    fatal_error("failed to create and populate file: portnumber.bcv");
  }
  sqlite3_free(aBuf);
  sqlite3_free(zFile);
  bcvCloseLocal(pFile);
}

static void bdCleanup(DaemonCtx *p){
  bcvCommonDestroy(&p->c);
  bcvCloseLocal(p->pCacheFile);
  bcvDispatchFree(p->pDisp);
  sqlite3_free(p);
}

/*
** Command: $argv[0] daemon ?SWITCHES? DIRECTORY
*/
static int main_daemon(int argc, char **argv){
  CommandSwitches *pCmd;
  DaemonCtx *p = 0;
  const char *zDir = 0;           /* DIRECTORY argument */
  char *zFullDir = 0;             /* Full-path to zDir (from sqlite3_malloc) */
  sqlite3 *db = 0;                /* blocksdb.bcv */
  int rc = SQLITE_OK;
  i64 nUsed;
  char *zErr = 0;

  p = (DaemonCtx*)bcvMallocZero(sizeof(DaemonCtx));
  pCmd = &p->cmd;

  /* Parse command line */
  if( argc<3 ) daemon_usage(argv[0]);
  zDir = argv[argc-1];
  parse_switches(pCmd, &argv[2], argc-3, 0, COMMAND_DAEMON|COMMAND_NREQUEST);
  if( pCmd->szCache<=0 ) pCmd->szCache = BCV_DEFAULT_CACHEFILE_SIZE;
  if( pCmd->nHttpTimeout<=0 ) pCmd->nHttpTimeout = BCV_DEFAULT_HTTPTIMEOUT;

  /* Open the listen socket */
  bdListen(p);

  /* Open and initialize blocksdb.bcv */
  zFullDir = bcvGetFullpath(&rc, zDir);
  db = bcvOpenAndInitDb(&rc, zFullDir, &zErr);
  if( rc!=SQLITE_OK ){
    fatal_error("failed to open and lock directory: %s (%s)", zDir, zErr);
  }

  p->c.pVfs = sqlite3_vfs_find(0);
  p->c.bDaemon = 1;
  rc = bcvfsCacheInit(&p->c, db, zFullDir);
  p->c.nMaxCache = pCmd->szCache;
  if( rc!=SQLITE_OK ){
    fatal_error("failed to initialize in directory: %s", zDir);
  }else{
    char *zCacheFile = sqlite3_mprintf(
        "%s" BCV_PATH_SEPARATOR "%s", zFullDir, BCV_CACHEFILE_NAME
    );
    rc = bcvOpenLocal(zCacheFile, 0, 0, &p->pCacheFile);
    if( rc!=SQLITE_OK ){
      fatal_error("failed to open cachefile in directory: %s", zDir);
    }
    sqlite3_free(zCacheFile);
  }
  

  /* Write the "portnumber.bcv" file clients read to find the listening port */
  bdWritePortNumber(p, zFullDir);

  /* Allocate a dispatcher object for the daemon to use */
  rc = bcvDispatchNew(&p->pDisp);
  if( rc!=SQLITE_OK ) fatal_oom_error();
  bcvDispatchVerbose(p->pDisp, (pCmd->mLog & BCV_LOG_VERBOSE) ? 1 : 0);
  bcvDispatchTimeout(p->pDisp, pCmd->nHttpTimeout);
  if( pCmd->mLog & (BCV_LOG_HTTP|BCV_LOG_HTTPRETRY) ){
    bcvDispatchLog(p->pDisp, p, bdHttpLog);
  }

  /* If the -readymessage option was passed, output the message "ready\n"
  ** on stdout to indicate that initialization has finished and database
  ** clients may expect to access databases successfully. This is used
  ** by test scripts.  */
  if( p->cmd.bReadyMessage ){
    fprintf(stdout, "ready\n");
    fflush(stdout);
  }
  daemon_event_log(p, "listening on localhost port %d", p->iListenPort);

  /* Run the daemon main loop. This only exits if (a) the -autoexit option
  ** was passed and (b) the number of connected database clients drops to
  ** zero.  */
  bdMainloop(p);

  /* The main loop has exited. Attempt to clean up all memory allocations,
  ** then output an error message if this reveals that this process has 
  ** leaked memory.  */
  bdCleanup(p);
  sqlite3_bcv_shutdown();
  sqlite3_reset_auto_extension();
  nUsed = sqlite3_memory_used();
  if( nUsed>0 ){
    daemon_error("daemon leaked %d bytes of memory", (int)nUsed);
    return 1;
  }

  return 0;
}

#define STRINGVALUE2(x) #x
#define STRINGVALUE(x) STRINGVALUE2(x)

static void bcvCustomInit(void){
#ifdef SQLITE_BCV_CUSTOM_INIT
  int rc = SQLITE_BCV_CUSTOM_INIT();
  if( rc!=SQLITE_OK ){
    const char *zFunc = STRINGVALUE(SQLITE_BCV_CUSTOM_INIT);
    fatal_error("custom init function %s() failed (rc=%d)", zFunc, rc);
  }
#endif
}

void bcvRestoreDefaultVfs(void);

int main(int argc, char **argv){
  SelectOption aCmd[] = {
    { "upload",   1 , 0},
    { "download", 1 , 1},
    { "list",     1 , 3},
    { "manifest", 1 , 4},
    { "delete",   1 , 5},
    { "destroy",  1 , 6},
    { "create",   1 , 7},
    { "daemon",   1 , 8},
    { "files",    1 , 9},
    { "copy",     1 , 10},
    { 0, 0 }
  };
  int rc;
  int iCmd;

  bcv_socket_init();
  curl_global_init(CURL_GLOBAL_ALL);
  OpenSSL_add_all_algorithms();

  signal(SIGPIPE, SIG_IGN);

  sqlite3_initialize();
  bcvCustomInit();

  iCmd = select_option(argc>=2 ? argv[1] : "", aCmd, 0xFFFFFFFF, 1);
  switch( aCmd[iCmd].eVal ){
    case 0:
      rc = main_upload(argc, argv);
      break;
    case 1:
      rc = main_download(argc, argv);
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
  }

  return rc;
}

/*
** 2019 October 31
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
**     Flags word (4 bytes)
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

#include "bcv_socket.h"
#include "bcv_int.h"
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
# define osMkdir(x,y) mkdir(x)
#else
# define osMkdir(x,y) mkdir(x,y)
# define O_BINARY 0
#endif

#define HTTP_AUTH_ERROR 403

#include "sqlite3.h"
#include "simplexml.h"

/* Default values of various command line parameters. */
#define BCV_DEFAULT_CACHEFILE_SIZE   (1024*1024*1024)
#define BCV_DEFAULT_NWRITE           10
#define BCV_DEFAULT_NDELETE          10
#define BCV_DEFAULT_POLLTIME         10
#define BCV_DEFAULT_DELETETIME       3600
#define BCV_DEFAULT_GCTIME           3600
#define BCV_DEFAULT_RETRYTIME        10

/*
** Message protocol message types.
*/
#define BCV_MESSAGE_LOGIN          'L'      /* client -> daemon */
#define BCV_MESSAGE_LOGIN_REPLY    'l'      /* daemon -> client */

#define BCV_MESSAGE_REQUEST        'R'      /* client -> daemon */
#define BCV_MESSAGE_REQUEST_REPLY  'r'      /* daemon -> client */
#define BCV_MESSAGE_DONE           'D'      /* client -> daemon */

#define BCV_MESSAGE_WREQUEST       'Q'      /* client -> daemon */
#define BCV_MESSAGE_WREQUEST_REPLY 'q'
#define BCV_MESSAGE_WDONE          'C'      /* client -> daemon */

#define BCV_MESSAGE_UPLOAD         'U'      /* internal client -> daemon */
#define BCV_MESSAGE_UPLOAD_REPLY   'u'      /* daemon -> internal client */

#define BCV_MESSAGE_QUERY          'Z'      /* client -> daemon */
#define BCV_MESSAGE_QUERY_REPLY    'z'      /* daemon -> client */

#define BCV_MESSAGE_ERROR_REPLY    'e'


typedef struct CommandSwitches CommandSwitches;
struct CommandSwitches {
  char *zContainer;               /* -container ARG */
  char *zAccessKey;               /* -accesskey ARG */
  char *zAccount;                 /* -account ARG */
  char *zProject;                 /* -project ARG */
  char *zSas;                     /* -sas ARG */
  char *zDirectory;               /* -directory ARG */
  char *zEmulator;                /* Emulator URI */
  int eStorage;
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
  int bVtab;                      /* True to enable bcv_* virtual-tables */

  int nWrite;                     /* Integer value of -nwrite option */
  int nDelete;                    /* Integer value of -deletes option */
};

#define BCV_LOG_MESSAGE 0x01
#define BCV_LOG_HTTP    0x20
#define BCV_LOG_EVENT   0x02

#define BCV_LOG_POLL    0x04
#define BCV_LOG_UPLOAD  0x08
#define BCV_LOG_CACHE   0x10


static void fatal_sql_error(sqlite3 *db, const char *zMsg){
  fprintf(stderr, "FATAL: SQL error in %s (rc=%d) (errmsg=%s)\n", 
      zMsg, sqlite3_errcode(db), sqlite3_errmsg(db)
  );
  exit(-1);
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
  exit(-1);
}

static void curlFatalIfNot2XX(
  CurlRequest *p, 
  CURLcode res, 
  const char *zPre
){
  long httpcode;
  if( res!=CURLE_OK ){
    fatal_error("%s: %s\n", zPre, curl_easy_strerror(res));
  }
  curl_easy_getinfo(p->pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
  if( (httpcode / 100)!=2 ){
    fatal_error("%s (%d) - %s", zPre, httpcode, p->zStatus);
  }
}


void fatal_system_error(const char *zApi, int iError){
  char *zBuf = bcvMalloc(1000);
  strerror_r(iError, zBuf, 1000);
  zBuf[999] = '\0';
  fatal_error("%s - %s", zApi, zBuf);
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
          "parse error in seconds (max value is %d): %s", 0x7fffffff, zInt
      );
    }
  }

  return (int)ret;
}


#define COMMANDLINE_CONTAINER    1
#define COMMANDLINE_ACCESSKEY    2
#define COMMANDLINE_ACCOUNT      3
#define COMMANDLINE_DIRECTORY    4
#define COMMANDLINE_PORT         5
#define COMMANDLINE_VERBOSE      6
#define COMMANDLINE_CACHESIZE    7
#define COMMANDLINE_AUTOEXIT     8
#define COMMANDLINE_BLOCKSIZE    9

#define COMMANDLINE_POLLTIME     10
#define COMMANDLINE_DELETETIME   11
#define COMMANDLINE_GCTIME       12
#define COMMANDLINE_RETRYTIME    13
 
#define COMMANDLINE_LOG          14

#define COMMANDLINE_NODELETE     15
#define COMMANDLINE_EMULATOR     16
#define COMMANDLINE_CLOBBER      17
#define COMMANDLINE_NWRITE       18
#define COMMANDLINE_NDELETE      19
#define COMMANDLINE_READYMSG     20
#define COMMANDLINE_NAMEBYTES    21
#define COMMANDLINE_PERSISTENT   22
#define COMMANDLINE_VTAB         23
#define COMMANDLINE_SAS          24

#define COMMANDLINE_STORAGE      25
#define COMMANDLINE_PROJECT      26

#define COMMAND_ALL              0xFFFFFFFF
#define COMMAND_DAEMON           0x40000000
#define COMMAND_CREATE           0x20000000
#define COMMAND_AUTH             0x10000000

#define COMMAND_AUTH_DAEMON (COMMAND_AUTH | COMMAND_DAEMON)

static void parse_more_switches(
  CommandSwitches *pCmd, 
  const char **azArg, 
  int nArg, 
  int *piFail,
  u32 mask
){
  int i;
  SelectOption aSwitch[] = {
    { "-file",       COMMAND_ALL    , 0},
    { "-delay",      COMMAND_ALL    , -1},

    { "-accesskey",  COMMAND_AUTH_DAEMON    , COMMANDLINE_ACCESSKEY },
    { "-account",    COMMAND_AUTH_DAEMON    , COMMANDLINE_ACCOUNT },
    { "-project",    COMMAND_AUTH_DAEMON    , COMMANDLINE_PROJECT },
    { "-emulator",   COMMAND_AUTH_DAEMON    , COMMANDLINE_EMULATOR },
    { "-sas",        COMMAND_AUTH           , COMMANDLINE_SAS },

    { "-container",  COMMANDLINE_CONTAINER  , COMMANDLINE_CONTAINER },
    { "-directory",  COMMANDLINE_DIRECTORY  , COMMANDLINE_DIRECTORY },
    { "-port",       COMMAND_DAEMON         , COMMANDLINE_PORT },
    { "-verbose",    COMMAND_ALL            , COMMANDLINE_VERBOSE },
    { "-cachesize",  COMMAND_DAEMON         , COMMANDLINE_CACHESIZE },
    { "-autoexit",   COMMAND_DAEMON         , COMMANDLINE_AUTOEXIT  },
    { "-blocksize",  COMMAND_CREATE         , COMMANDLINE_BLOCKSIZE },
    { "-polltime",   COMMAND_DAEMON         , COMMANDLINE_POLLTIME  },
    { "-deletetime", COMMAND_DAEMON         , COMMANDLINE_DELETETIME},
    { "-gctime",     COMMAND_DAEMON         , COMMANDLINE_GCTIME    },
    { "-retrytime",  COMMAND_DAEMON         , COMMANDLINE_RETRYTIME },
    { "-log",        COMMAND_DAEMON         , COMMANDLINE_LOG       },
    { "-nodelete",   COMMAND_DAEMON         , COMMANDLINE_NODELETE  },
    { "-clobber",    COMMAND_CREATE         , COMMANDLINE_CLOBBER  },
    { "-nwrite",     COMMAND_DAEMON         , COMMANDLINE_NWRITE    },
    { "-ndelete",    COMMAND_DAEMON         , COMMANDLINE_NDELETE   },
    { "-readymessage",COMMAND_DAEMON        , COMMANDLINE_READYMSG },
    { "-namebytes",  COMMAND_CREATE         , COMMANDLINE_NAMEBYTES  },
    { "-persistent", COMMAND_DAEMON         , COMMANDLINE_PERSISTENT},
    { "-vtab",       COMMAND_DAEMON         , COMMANDLINE_VTAB,     },
    { "-storage",    COMMAND_ALL            , COMMANDLINE_STORAGE,     },
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
      case COMMANDLINE_PROJECT:
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
      case COMMANDLINE_SAS:
      case COMMANDLINE_STORAGE:
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
      case COMMANDLINE_ACCESSKEY:
        sqlite3_free(pCmd->zAccessKey);
        if( azArg[i][0]=='\0' ){
          pCmd->zAccessKey = 0;
        }else{
          pCmd->zAccessKey = bcvStrdup(azArg[i]);
        }
        break;
      case COMMANDLINE_ACCOUNT:
        pCmd->zAccount = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_PROJECT:
        pCmd->zProject = bcvStrdup(azArg[i]);
        break;
      case COMMANDLINE_SAS:
        pCmd->zSas = bcvStrdup(azArg[i]);
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
      case COMMANDLINE_VTAB:
        pCmd->bVtab = 1;
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
            case 'h':
              pCmd->mLog |= BCV_LOG_HTTP;
              break;
            default:
              fatal_error(
                  "unexpected -log flag '%c', expected 'm' or 'e'", zArg[ii]
              );

          }
        }
        break;
      }
      case COMMANDLINE_NODELETE:
        pCmd->bNodelete = 1;
        break;
      case COMMANDLINE_EMULATOR:
        /* The following credentials are the only ones accepted by the 
        ** official Azure storage emulator app. They're published as part of
        ** Azure documentation. So the access-key being included here need not
        ** alarm anybody - it's not a secret.  */
        sqlite3_free(pCmd->zEmulator);
        sqlite3_free(pCmd->zAccount);
        sqlite3_free(pCmd->zAccessKey);
        pCmd->zEmulator = bcvStrdup(azArg[i]);
        pCmd->zAccount = bcvStrdup("devstoreaccount1");
        pCmd->zAccessKey = bcvStrdup(
            "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuF"
            "q2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="
        );
        break;
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
      case COMMANDLINE_STORAGE: {
        int iSel;
        SelectOption aOpt[] = {
          { "azure", 0x01,  BCV_STORAGE_AZURE },
          { "google", 0x01, BCV_STORAGE_GOOGLE },
          { 0, 0, 0 }
        };
        iSel = select_option(azArg[i], aOpt, 0x01, 1);
        pCmd->eStorage = aOpt[iSel].eVal;
        break;
      }
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
  parse_more_switches(pCmd, azArg, nArg, piFail, mask);
}

static void free_parse_switches(CommandSwitches *pCmd){
  sqlite3_free(pCmd->zContainer);
  sqlite3_free(pCmd->zAccessKey);
  sqlite3_free(pCmd->zAccount);
  sqlite3_free(pCmd->zDirectory);
  sqlite3_free(pCmd->zEmulator);
  sqlite3_free(pCmd->zProject);
  memset(pCmd, 0, sizeof(CommandSwitches));
}

static void missing_required_switch(const char *zSwitch){
  fatal_error("missing required switch: %s\n", zSwitch);
}

static void bcvAccessPointNew(
  AccessPoint *p, 
  CommandSwitches *pCmd,
  int bTemplate
){
  int eType = SQLITE_BCV_AZURE;
  const char *zUser = pCmd->zAccount;
  const char *zKey = pCmd->zAccessKey;
  char *zErr = 0;

  assert( pCmd->eStorage==BCV_STORAGE_AZURE
       || pCmd->eStorage==BCV_STORAGE_GOOGLE
  );
  if( pCmd->eStorage==BCV_STORAGE_AZURE ){
    if( pCmd->zAccount==0 ){
      missing_required_switch("-account");
    }
    if( pCmd->zProject ){
      fatal_error("cannot use -project with azure storage");
    }
    if( bTemplate==0 && pCmd->zAccessKey==0 && pCmd->zSas==0 ){
      missing_required_switch("-accesskey or -sas");
    }
  }else{
    if( pCmd->zAccount ){
      fatal_error("cannot use -account with google storage");
    }
    if( pCmd->zAccessKey ){
      fatal_error("cannot use -accesskey with google storage");
    }
    if( pCmd->zEmulator ){
      fatal_error("cannot use -emulator with google storage");
    }
    if( bTemplate==0 && pCmd->zSas==0 ){
      fatal_error("missing required switch for google storage: -sas");
    }
  }

  if( pCmd->eStorage==BCV_STORAGE_AZURE ){
    if( pCmd->zEmulator ){
      zUser = pCmd->zEmulator;
      if( pCmd->zAccessKey==0 ){
        eType = SQLITE_BCV_AZURE_EMU_SAS;
        zKey = pCmd->zSas;
      }else{
        eType = SQLITE_BCV_AZURE_EMU;
      }
    }
    else if( pCmd->zSas ){
      eType = SQLITE_BCV_AZURE_SAS;
      zKey = pCmd->zSas;
    }
  }else{
    zUser = pCmd->zProject;
    zKey = pCmd->zSas;
  }

  if( bcvAccessPointInit(p, eType, zUser, zKey, &zErr) ){
    fatal_error("error configuring accesspoint: %s", zErr);
  }
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
  curlFatalIfNot2XX(pCurl, res, "download manifest failed");
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
  char *zErr = 0;
  Manifest *pRet = 0;
  int rc = 0;
  int n = 0;
  u8 *a = 0;
  char *zETag = 0;
  a = bcvGetManifest(pCurl, pA, zContainer, &n, &zETag);
  if( pnByte ) *pnByte = n;
  rc = bcvManifestParse(a, n, zETag, &pRet, &zErr);
  if( rc!=SQLITE_OK ){
    fatal_error("error parsing manifest: %s", zErr);
  }
  return pRet;
}

static void bcvOpenHandle(CommandSwitches *pCmd, sqlite3_bcv **ppBcv){
  int eType = SQLITE_BCV_AZURE;
  const char *zUser = pCmd->zAccount;
  const char *zKey = pCmd->zAccessKey;
  int rc;

  assert( pCmd->eStorage==BCV_STORAGE_AZURE
       || pCmd->eStorage==BCV_STORAGE_GOOGLE
  );
  if( pCmd->eStorage==BCV_STORAGE_AZURE ){
    if( pCmd->zAccount==0 ){
      missing_required_switch("-account");
    }
    if( pCmd->zProject ){
      fatal_error("cannot use -project with azure storage");
    }
    if( pCmd->zAccessKey==0 && pCmd->zSas==0 ){
      missing_required_switch("-accesskey or -sas");
    }
  }else{
    if( pCmd->zAccount ){
      fatal_error("cannot use -account with google storage");
    }
    if( pCmd->zAccessKey ){
      fatal_error("cannot use -accesskey with google storage");
    }
    if( pCmd->zEmulator ){
      fatal_error("cannot use -emulator with google storage");
    }
    if( pCmd->zSas==0 ){
      fatal_error("missing required switch for google storage: -sas");
    }
  }

  if( pCmd->eStorage==BCV_STORAGE_AZURE ){
    if( pCmd->zEmulator ){
      zUser = pCmd->zEmulator;
      if( pCmd->zSas ){
        eType = SQLITE_BCV_AZURE_EMU_SAS;
        zKey = pCmd->zSas;
      }else{
        eType = SQLITE_BCV_AZURE_EMU;
      }
    }
    else if( pCmd->zSas ){
      eType = SQLITE_BCV_AZURE_SAS;
      zKey = pCmd->zSas;
    }
  }else{
    zUser = pCmd->zProject;
    zKey = pCmd->zSas;
    eType = SQLITE_BCV_GOOGLE;
  }

  rc = sqlite3_bcv_open(eType, zUser, zKey, pCmd->zContainer, ppBcv);
  if( rc!=SQLITE_OK ){
    fatal_error("error (%d): %s\n", rc, sqlite3_bcv_errmsg(*ppBcv));
  }else{
    sqlite3_bcv_config(*ppBcv, SQLITE_BCVCONFIG_VERBOSE, (int)pCmd->bVerbose);
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
    parse_switches(&cmd, (const char**)&argv[2], argc-3, ptr,
        COMMANDLINE_CONTAINER | COMMAND_AUTH
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
    parse_switches(&cmd, (const char**)&argv[2], argc-3, ptr,
        COMMANDLINE_CONTAINER | COMMAND_AUTH
    );
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
  parse_switches(&cmd, (const char**)&argv[2], argc-4, 0,
    COMMANDLINE_CONTAINER | COMMAND_AUTH
  );

  bcvOpenHandle(&cmd, &pBcv);
  rc = sqlite3_bcv_copy(pBcv, zDbFrom, zDbTo);
  if( rc ){
    fprintf(stderr, "error (%d): %s\n", rc, sqlite3_bcv_errmsg(pBcv));
  }
  sqlite3_bcv_close(pBcv);
  free_parse_switches(&cmd);

  return rc;
}

/*
** Command: $argv[0] files ?SWITCHES? CONTAINER
*/
static int main_files(int argc, char **argv){
  CommandSwitches cmd;
  CurlRequest curl;
  AccessPoint ap;
  MemoryDownload md;
  CURLcode res;
  FilesParse files;
  FilesParseEntry *p;

  /* Parse command line */
  if( argc<3 ){
    fprintf(stderr, "Usage: %s files ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0, COMMAND_AUTH);
  cmd.zContainer = argv[argc-1];
  bcvAccessPointNew(&ap, &cmd, 0);
  curlRequestInit(&curl, cmd.bVerbose);

  memset(&files, 0, sizeof(files));
  do{
    memset(&md, 0, sizeof(MemoryDownload));
    md.pCurl = &curl;

    curlListFiles(&curl, &ap, cmd.zContainer, files.zNextMarker);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEFUNCTION, memoryDownloadWrite);
    curl_easy_setopt(curl.pCurl, CURLOPT_WRITEDATA, (void*)&md);

    res = curl_easy_perform(curl.pCurl);
    curlFatalIfNot2XX(&curl, res, "list files failed");

    bcvParseFiles(cmd.eStorage, md.a, md.nByte, &files);
    curlRequestReset(&curl);
  }while( files.zNextMarker );

  for(p=files.pList; p; p=p->pNext){
    printf("%s\n", p->zName);
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
    COMMANDLINE_CONTAINER | COMMAND_AUTH
  );
  if( cmd.zAccount==0 )   missing_required_switch("-account");
  if( cmd.zContainer==0 ) missing_required_switch("-container");
  if( cmd.zAccessKey==0 ) missing_required_switch("-accesskey");
  bcvAccessPointNew(&ap, &cmd, 0);

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

/*
** Command: $argv[0] manifest ?SWITCHES? CONTAINER
*/
static int main_manifest(int argc, char **argv){
  CommandSwitches cmd;
  CurlRequest curl;
  AccessPoint ap;
  Manifest *p;
  int i, j;

  /* Parse command line */
  if( argc<3 ){
    fprintf(stderr, "Usage: %s manifest ?SWITCHES? CONTAINER\n", argv[0]);
    exit(1);
  }
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0, COMMAND_AUTH);
  cmd.zContainer = argv[argc-1];
  bcvAccessPointNew(&ap, &cmd, 0);

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
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0,
    COMMANDLINE_CONTAINER | COMMAND_AUTH
  );
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
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0,
      COMMAND_CREATE | COMMAND_AUTH
  ); 

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
  parse_switches(&cmd, (const char**)&argv[2], argc-3, 0, COMMAND_AUTH);
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
  int nTotal;                     /* Total number of blocks seen in directory */

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

  int iReq;                       /* Id used for logging */
};

/*
** The pAttachClient, pPollClient and pNextPollClient lists are clients
** that require replies to bcv_attach or bcv_readonly messages. Possibly
** with the SQLITE_BCVATTACH_POLL flag set.
**
** pAttachClient clients are waiting on the initial download of the manifest
** for a reply. If there is a problem with the manifest file, an error is
** returned and the container detached automatically. pPollClient clients are
** waiting on the result of a currently outstanding poll operation.
** pNextPollClient need to wait until the current outstanding
** poll is finished, then for the conclusion of the next one. See functions:
**
**     bdAddContainer()
**     bdDownloadBlockDone()
**
** for implementation details.
*/
struct DContainer {
  char *zName;
  int bReadonly;                  /* True for a read-only container */
  Manifest *pManifest;            /* Current manifest object */
  int eOp;                        /* Current operation type (if any) */
  i64 iPollTime;                  /* Time at which to poll for new manifest */
  i64 iDeleteTime;                /* Time at which to delete old blocks */
  i64 iGCTime;                    /* Time at which to GC container */
  AccessPoint ap;                 /* Access point for this container */
  DCheckpoint *pCheckpointList;   /* List of ongoing upload operations */
  DContainer *pNext;
  int iSasShmZero;                /* Countdown to *-shm zeroing */

  int bAutoDetach;
  DClient *pPollClient;
  DClient *pNextPollClient;
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
  AccessPoint ap_template;        /* Access point for daemon */

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
    fprintf(stdout, "INFO(%s%s%s%s%s%s): %s\n", 
        (flags & BCV_LOG_POLL ? "p" : ""),
        (flags & BCV_LOG_EVENT ? "e" : ""),
        (flags & BCV_LOG_MESSAGE ? "m" : ""),
        (flags & BCV_LOG_UPLOAD ? "u" : ""),
        (flags & BCV_LOG_CACHE ? "c" : ""),
        (flags & BCV_LOG_HTTP ? "h" : ""),
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
static int bdListen(DaemonCtx *p){
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

static void bdClientClearRefs(DClient *pClient){
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
static void bdCloseConnection(DaemonCtx *p, DClient *pClient){
  DCurlRequest *pReq;
  DClient **pp;
  bcv_close_socket(pClient->fd);

  if( p->cmd.mLog & BCV_LOG_MESSAGE ){
    daemon_msg_log("%d->D: disconnect...", pClient->iId);
  }

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
  bdClientClearRefs(pClient);
  sqlite3_free(pClient);
}

/*
** This is called by the daemon main loop whenever there is a new client
** connection.
*/
static void bdNewConnection(DaemonCtx *p){
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

  if( p->cmd.mLog & BCV_LOG_MESSAGE ){
    daemon_msg_log("%d->D: connect!", pNew->iId);
  }
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

static int bdFindDb(DContainer *pCont, const u8 *aId){
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

static int bdParseMessage(
  u8 eType, 
  u8 *aBody, 
  int nBody,
  DMessage *pMsg
){
  int rc = 0;
  pMsg->eType = (int)eType;
  switch( eType ){
    case BCV_MESSAGE_REQUEST:
    case BCV_MESSAGE_LOGIN:
      pMsg->iVal = (int)bcvGetU32(aBody);
      pMsg->nBlobByte = nBody-sizeof(u32);
      pMsg->aBlob = &aBody[sizeof(u32)];
      break;

    case BCV_MESSAGE_WDONE:
      break;

    case BCV_MESSAGE_UPLOAD:
      pMsg->iVal = (int)bcvGetU32(aBody);
      break;

    case BCV_MESSAGE_QUERY:
      pMsg->iVal = (int)bcvGetU32(aBody);
      pMsg->nBlobByte = nBody-4;
      pMsg->aBlob = &aBody[4];
      break;

    case BCV_MESSAGE_DONE:
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

static void bdParseFree(DMessage *pMsg){
  memset(pMsg, 0, sizeof(DMessage));
}

static void bdLogMessage(DaemonCtx *p, DClient *pClient, DMessage *pMsg){
  if( p->cmd.mLog & BCV_LOG_MESSAGE ){
    int iId = pClient->iId;
    int eType = pMsg->eType;
    switch( eType ){
      case BCV_MESSAGE_LOGIN_REPLY:
      case BCV_MESSAGE_LOGIN: {
        const char *zType = (eType==BCV_MESSAGE_LOGIN?"LOGIN":"LOGIN_REPLY");
        int n = pMsg->nBlobByte;
        char *z = (char*)pMsg->aBlob;
        daemon_msg_log("%d->D: %s (flags=%d) %.*s",iId,zType,pMsg->iVal,n,z);
        break;
      }
      case BCV_MESSAGE_REQUEST_REPLY: {
        const char *zSep = "";
        char *aBuf = 0;
        int i;
        for(i=0; i<pMsg->nBlobByte; i+=sizeof(u32)){
          aBuf = bcvMprintf("%z%s%d", aBuf, zSep, bcvGetU32(&pMsg->aBlob[i]));
          zSep = ",";
        }
        daemon_msg_log("D->%d: REQUEST_REPLY %z", iId, aBuf);
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
          daemon_msg_log("%d->D: DONE %z", iId, aBuf);
        }else{
          daemon_msg_log("%d->D: REQUEST block=%d %z", iId, pMsg->iVal, aBuf);
        }
        break;
      }
      case BCV_MESSAGE_UPLOAD_REPLY:
        daemon_msg_log("D->%d: UPLOAD_REPLY", iId);
        break;
      case BCV_MESSAGE_ERROR_REPLY:
        daemon_msg_log("D->send %d: ERROR_REPLY (rc=%d) \"%.*s\"", 
            iId, pMsg->iVal, pMsg->nBlobByte, (char*)pMsg->aBlob
        );
        break;
      case BCV_MESSAGE_WDONE:
        daemon_msg_log("%d->D: WDONE", iId);
        break;
      case BCV_MESSAGE_UPLOAD:
        daemon_msg_log("%d->D: UPLOAD upload=%d", iId, pMsg->iVal);
        break;

      case BCV_MESSAGE_QUERY: {
        int n = pMsg->nBlobByte;
        char *z = (char*)pMsg->aBlob;
        int bSas = 0;
        if( (n>11 && sqlite3_strnicmp(z, "bcv_attach=", 11)==0)
         || (n>13 && sqlite3_strnicmp(z, "bcv_readonly=", 13)==0)
        ){
          int i;
          for(i=0; i<n && z[i]!='?'; i++);
          if( i<n ){
            bSas = 1;
            n = i;
          }
        }
        daemon_msg_log("%d->D: QUERY \"%.*s%s\"", iId, n, z, bSas?"?<sas>":"");
        break;
      }
      case BCV_MESSAGE_QUERY_REPLY:
        daemon_msg_log("D->%d: QUERY_REPLY rc=%d (%d bytes of data...)",
            iId, pMsg->iVal, pMsg->nBlobByte
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
        if( pMsg->eType==BCV_MESSAGE_WREQUEST ){
          daemon_msg_log("%d->D: WREQUEST (%z)", iId, zList);
        }else{
          daemon_msg_log("D->%d: WREQUEST_REPLY (%z)", iId, zList);
        }
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
      nMsg = 9 + pMsg->nBlobByte;
      aMsg = (u8*)bcvMallocZero(nMsg);
      bcvPutU32(&aMsg[5], (u32)pMsg->iVal);
      memcpy(&aMsg[9], pMsg->aBlob, pMsg->nBlobByte);
      break;
    }
    case BCV_MESSAGE_ERROR_REPLY:
    case BCV_MESSAGE_QUERY_REPLY:
      nMsg = 5 + 4 + pMsg->nBlobByte;
      aMsg = (u8*)bcvMalloc(nMsg);
      bcvPutU32(&aMsg[5], pMsg->iVal);
      memcpy(&aMsg[9], pMsg->aBlob, pMsg->nBlobByte);
      break;

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

static void bdClientMessageSend(
  DaemonCtx *p, 
  DClient *pClient, 
  DMessage *pMsg
){
  int rc;
  bdLogMessage(p, pClient, pMsg);
  rc = bdMessageSend(pClient->fd, pMsg);
  if( rc ){
    daemon_error(
        "failed to write send message to client %d - closing connection",
        pClient->iId
    );
    bdCloseConnection(p, pClient);
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
#ifndef NDEBUG
  DCacheEntry *pCsr;
  for(pCsr=p->aHash[iHash]; pCsr; pCsr=pCsr->pHashNext) assert( pEntry!=pCsr );
#endif
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
    assert( pEntry->pHashNext!=pEntry );
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
static void bdCurlRelease(DaemonCtx *p, DCurlRequest *pCurl){
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
static void bdAddToLRU(DaemonCtx *p, DCacheEntry *pEntry){
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
static void bdRemoveFromLRU(DaemonCtx *p, DCacheEntry *pEntry){
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
static void bdFreeSlots(DaemonCtx *p){
  while( p->nCacheEntry>=p->nCacheMax ){
    DCacheEntry *pEntry;
    for(pEntry=p->pLruFirst; pEntry; pEntry=pEntry->pLruNext){
      if( pEntry->nRef==0 && pEntry->bPending==0 && pEntry->bDirty==0 ) break;
    }
    if( pEntry==0 ) break;

    /* Remove entry from LRU list */
    bdRemoveFromLRU(p, pEntry);

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
static int bdCacheSlot(DaemonCtx *p){
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

static size_t bdBlobCb(
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

static int bdHttpCode(DCurlRequest *pReq, CURLMsg *pMsg){
  long httpcode = -1;
  if( pMsg->data.result==CURLE_OK ){
    curl_easy_getinfo(pReq->req.pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
  }
  return (int)httpcode;
}
static const char *bdRequestError(DCurlRequest *pReq, CURLMsg *pMsg){
  if( pMsg->data.result!=CURLE_OK ){
    return curl_easy_strerror(pMsg->data.result);
  }
  return pReq->req.zStatus;
}

static void bdCurlLogRequest(DaemonCtx *p, DCurlRequest *pCurl){
  static int iReq = 0;
  if( 0!=(p->cmd.mLog & BCV_LOG_HTTP) ){
    const char *zMethod;
    switch( pCurl->req.eHttpMethod ){
      case BCV_HTTP_GET:    zMethod = "GET"; break;
      case BCV_HTTP_PUT:    zMethod = "PUT"; break;
      case BCV_HTTP_DELETE: zMethod = "DELETE"; break;
      default: zMethod = "?unknown?"; break;
    }
    pCurl->iReq = iReq++;
    daemon_log(p, BCV_LOG_HTTP, "r%d %s %s", 
        pCurl->iReq, zMethod, pCurl->req.zHttpUrl
    );
  }
}

static void bdCurlRun(
  DaemonCtx *p,
  DCurlRequest *pCurl, 
  DContainer *pContainer,
  void (*xDone)(DCurlRequest*, CURLMsg*)
){
  assert( pContainer && pCurl->pContainer==0 );
  pCurl->pDaemonCtx = p;
  pCurl->xDone = xDone;
  pCurl->pContainer = pContainer;
  pCurl->pNext = p->pRequestList;
  p->pRequestList = pCurl;
  bdCurlLogRequest(p, pCurl);
  curl_multi_add_handle(p->pMulti, pCurl->req.pCurl);
}

static void bdProcessUsed(
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
            bdRemoveFromLRU(p, pEntry);
            bdAddToLRU(p, pEntry);
          }
        }
      }
    }
  }
}

static void bdReadRequestReply(DaemonCtx *p, DClient *pClient){
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

  bdClientMessageSend(p, pClient, &reply);
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
    bdFreeSlots(p);
    pNew->iPos = bdCacheSlot(p);
    pNew->bDirty = 1;
    bdAddToLRU(p, pNew);

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

  iDb = bdFindDb(pClient->pContainer, pClient->aId);
  if( iDb<0 ){
    bdCloseConnection(p, pClient);
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
    reply.iVal = 1;
    reply.aBlob = (u8*)bcvMprintf("SQL error: %s", sqlite3_errmsg(p->db));
    reply.nBlobByte = strlen((char*)reply.aBlob);
  }

  bdClientMessageSend(p, pClient, &reply);
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
static int bdIsHttpError(DCurlRequest *pReq, CURLMsg *pMsg, int c2){
  if( pMsg->data.result!=CURLE_OK ){
    daemon_error("error in curl request - %s", 
        curl_easy_strerror(pMsg->data.result)
    );
    return 1;
  }else{
    int httpcode = bdHttpCode(pReq, pMsg);
    if( (httpcode / 100)!=2 && httpcode!=c2 ){
      char *zExtra = 0;
      char *zUrl = 0;
      if( c2 ) zExtra = bcvMprintf(" or %d", c2);
      daemon_error(
          "error in http request - response code is %d, expected 2XX%s",
          (int)httpcode, zExtra?zExtra:""
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

/*
** The client passed as the second argument is waiting for a reply to
** either a REQUEST or WREQUEST message. This function is called to send
** the reply.
*/
static void bdRequestReply(DaemonCtx *p, DClient *pClient){
  if( pClient->bWReq==0 ){
    bdReadRequestReply(p, pClient);
  }else{
    bdWRequestReply(p, pClient);
  }
}

static void bdDownloadBlockDone(
  DCurlRequest *pCurl, 
  CURLMsg *pMsg 
){
  DClient *pClient;
  DCacheEntry *pEntry = pCurl->pEntry;
  DaemonCtx *p = pCurl->pDaemonCtx;

  assert( pMsg->msg==CURLMSG_DONE );
  if( bdIsHttpError(pCurl, pMsg, 0) ){
    if( pEntry->pWaiting ){
      DMessage msg;
      memset(&msg, 0, sizeof(msg));
      msg.eType = BCV_MESSAGE_ERROR_REPLY;
      msg.iVal = bdHttpCode(pCurl, pMsg);
      msg.aBlob = (u8*)bcvMprintf(
          "failed to download block file (httpcode=%d)", msg.iVal
      );
      msg.nBlobByte = strlen((const char*)msg.aBlob);
      for(pClient=pEntry->pWaiting; pClient; pClient=pClient->pNextWaiting){
        bdClientMessageSend(p, pClient, &msg);
      }
      sqlite3_free(msg.aBlob);
    }

    /* Remove the DCacheEntry object from the hash table, mark the slot it
    ** was to use as free, and delete it.  */
    bdCacheHashRemove(p, pEntry);
    bdRemoveFromLRU(p, pEntry);
    p->aCacheUsed[pEntry->iPos / 32] &= ~(1 << (pEntry->iPos%32));
    p->nCacheEntry--;
    sqlite3_free(pEntry);

  }else{
    /* Success - send a REQUEST_REPLY message to each waiting client */
    for(pClient=pEntry->pWaiting; pClient; pClient=pClient->pNextWaiting){
      bdRequestReply(p, pClient);
    }
    bdWriteCleanEntry(p, pEntry->iPos, pEntry->aName, pEntry->nName);
    pEntry->pWaiting = 0;
    pEntry->bPending = 0;
  }

}

static int bdDeleteBlockEntries(
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
  int iDb = bdFindDb(pContainer, aId);
  if( iDb<0 ) return "?unknown?";
  return pContainer->pManifest->aDb[iDb].zDName;
}

static void bdNewBlock(
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
  bdFreeSlots(p);
  pNew->iPos = bdCacheSlot(p);
  assert( pNew->iPos>=0 );
  pNew->bDirty = 1;

  /* Grow the manifest block array and add the new entry */
  bdManifestWriteBlock(pDb, iBlk, aName, nNamebytes);

  /* Write a database entry for the new, dirty, block */
  bdWriteDirtyEntry(p, pMan, pNew->iPos, iBlk, 
      pClient->pContainer->zName, pClient->aId, pDb->iVersion
  );

  /* Add new cache-entry to the hash and LRU data structures */
  bdAddToLRU(p, pNew);
  bdCacheHashAdd(p, pNew);
  p->nCacheEntry++;
  *ppEntry = pNew;
}

/*
** Return true if the container requires an SAS.
*/
static int bdRequireSas(DContainer *p){
  return (p->ap.zAccessKey==0 && p->ap.zSas==0 );
}

/*
** This function handles both REQUEST and WREQUEST messages.
*/
static void bdHandleClientRequest(
  DaemonCtx *p,                   /* Daemon object */
  DClient *pClient,               /* Client from which message was received */
  DMessage *pMsg                  /* REQUEST or WREQUEST message */
){
  DContainer *pCont = pClient->pContainer;
  DCacheEntry *pEntry = 0;
  u8 *pBlk;
  Manifest *pMan;
  ManifestDb *pDb;
  int nNamebytes;
  int iBlk;                       /* Index of requested block */
  int bWReq;                      /* True for WREQUEST message */

  assert( pCont );
  assert( pMsg->eType==BCV_MESSAGE_REQUEST||pMsg->eType==BCV_MESSAGE_WREQUEST );
  if( bdRequireSas(pCont) ){
    DMessage msg;
    memset(&msg, 0, sizeof(DMessage));
    msg.eType = BCV_MESSAGE_ERROR_REPLY;
    msg.iVal = HTTP_AUTH_ERROR;
    msg.aBlob = (u8*)bcvMprintf(
        "daemon has no SAS token for container \"%s\"", pCont->zName
    );
    bdMessageSend(pClient->fd, &msg);
    sqlite3_free(msg.aBlob);
    return;
  }

  if( pMsg->eType==BCV_MESSAGE_REQUEST ){
    bWReq = 0;
    iBlk = pMsg->iVal;
  }else{
    assert( pMsg->nBlobByte==4 );
    bWReq = 1;
    iBlk = bcvGetU32(pMsg->aBlob);
  }

  if( pClient->pManifest==0 ){
    int iDb = bdFindDb(pCont, pClient->aId);
    if( iDb<0 ){
      bdCloseConnection(p, pClient);
      return;
    }
    pClient->iDb = iDb;
    if( bWReq ){
      pMan = pCont->pManifest;
    }else{
      pClient->pManifest = bcvManifestRef(pCont->pManifest);
      pMan = pClient->pManifest;
    }
  }else{
    pMan = pClient->pManifest;
  }
  pDb = &pMan->aDb[pClient->iDb];
  nNamebytes = NAMEBYTES(pMan);

  assert( bWReq==0 || pClient->apEntry==0 );
  if( bWReq==0 ){
    bdClientClearRefs(pClient);
    bdProcessUsed(p, pClient, pMsg->aBlob, pMsg->nBlobByte);
  }

  if( iBlk>=pDb->nBlkLocal ){
    assert( bWReq );
    bdNewBlock(p, pClient, iBlk, &pEntry);
  }else{
    pBlk = &pDb->aBlkLocal[NAMEBYTES(pMan)*iBlk];
    pEntry = bdCacheHashFind(p, pBlk, nNamebytes);
  }
  
  pClient->bWReq = bWReq;
  pClient->iWReqBlk = iBlk;

  if( pEntry ){
    if( pEntry->bPending ){
      /* This blob is currently downloading */
      pClient->pNextWaiting = pEntry->pWaiting;
      pEntry->pWaiting = pClient;
    }else{
      bdRequestReply(p, pClient);
    }
  }else{
    char aBuf[BCV_MAX_FSNAMEBYTES];
    DCurlRequest *pCurl = 0;
    DContainer *pCont = pClient->pContainer;

    /* Allocate a new cache-entry object */
    pEntry = (DCacheEntry*)bcvMallocZero(sizeof(DCacheEntry) + nNamebytes);
    pEntry->nName = nNamebytes;
    memcpy(pEntry->aName, pBlk, nNamebytes);

    /* Find a free cache-slot */
    bdFreeSlots(p);
    pEntry->iPos = bdCacheSlot(p);
    pEntry->bPending = 1;
    assert( pEntry->iPos>=0 );

    /* Add the DCacheEntry to the hash table */
    bdCacheHashAdd(p, pEntry);
    pClient->pNextWaiting = 0;
    pEntry->pWaiting = pClient;
    p->nCacheEntry++;

    /* Add the DCacheEntry to the end of the LRU list */
    bdAddToLRU(p, pEntry);

    /* Configure a DCurlRequest to download the blob and write it into
    ** a free slot in the cache file. */
    pCurl = bdCurlRequest(p);
    bcvBlockidToText(pMan, pBlk, aBuf);
    curlFetchBlob(&pCurl->req, &pCont->ap, pCont->zName, aBuf);
    curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEFUNCTION, bdBlobCb);
    curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEDATA, (void*)pCurl);
    pCurl->iOff = (i64)pEntry->iPos * (i64)p->szBlk;
    pCurl->pEntry = pEntry;

    bdCurlRun(p, pCurl, pCont, bdDownloadBlockDone);
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

static void bdLaunchUpload(DaemonCtx *p, DContainer *pCont, DCheckpoint *pCkpt){
  daemon_event_log(p, "UPLOAD operation on container %s (db=%s) starting", 
      pCont->zName, pCkpt->zDb
  );

  {
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
  }

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

static int bdCachefileSize(DaemonCtx *p, i64 *psz){
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
    DContainer *pCont = p->pContainer;
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

    rc = bdCachefileSize(pCtx, &szFile);
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
      assert( p->pMHash );
      if( nNamebytes>=BCV_MIN_MD5_NAMEBYTES ){
        assert( MD5_DIGEST_LENGTH==16 );
        MD5(pCurl->aBuf, pCurl->nBuf, aPermid);
        pMatch = bcvMHashMatch(p->pMHash, aPermid, MD5_DIGEST_LENGTH);
        if( pMatch ){
          memcpy(aPermid, pMatch, NAMEBYTES(pMan));
          bdCurlRelease(pCtx, pCurl);
        }
      }

      if( pMatch==0 ){
        bcvBlockidToText(pMan, aPermid, zFile);
        pCurl->pUpload = p;
        curlPutBlob(&pCurl->req, &pCont->ap, pCont->zName, zFile,0,pCurl->nBuf);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READFUNCTION, bdMemoryRead);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READDATA, (void*)pCurl);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKFUNCTION, bdMemorySeek);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKDATA, (void*)pCurl);
        curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEFUNCTION, bdIgnore);
        p->nRef++;
        bdCurlRun(pCtx, pCurl, pCont, bdUploadBlockDone);
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

static void bdUploadFree(DUpload *pUpload){
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
        bdClientMessageSend(p, pClient, &reply);
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
  for(i=0; i<pMan->nDelBlk; i++){
    int iOff = i*GCENTRYBYTES(pMan) + NAMEBYTES(pMan);
    i64 iTime = (i64)bcvGetU64(&pMan->aDelBlk[iOff]);
    if( pCont->iDeleteTime==0 || iTime<pCont->iDeleteTime ){
      pCont->iDeleteTime = MAX(iTime, 1);
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
      rc = bdDeleteBlockEntries(pCtx, zCont, pUpload->pClient->aId);
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
  bdClientMessageSend(pCtx, pUpload->pClient, &reply);

  daemon_event_log(pCtx, "UPLOAD operation on container %s finished - %s",
      pUpload->pContainer->zName, bFail ? "failed" : "ok"
  );

  /* Free whatever is left of the DUpload object */
  bdUploadFree(pUpload);
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

static void bdLogGCFinished(DaemonCtx *pCtx, DCollect *p){
  daemon_event_log(pCtx,
    "GARBAGE COLLECTION operation on container %s finished "
    "- %d of %d blocks scheduled for deletion",
    p->pContainer->zName,
    p->nDelete, p->nTotal
  );
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

  bFail = bdIsHttpError(p, pMsg, 0);
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
    DContainer *pCont = p->pDelete->pContainer;
    daemon_event_log(pCtx, "DELETE operation on container %s finished - %s",
        pCont->zName, bFail ? "failed" : "ok"
    );

    /* If the operation could not be completed, schedule it to be retried
    ** in nRetryTime seconds.  */
    if( bFail ){
      pCont->iDeleteTime = sqlite_timestamp() + pCtx->cmd.nRetryTime*1000;
    }
    bdDeleteFree(p->pDelete);
    p->pDelete = 0;
  }

  if( p->pCollect ){
    DContainer *pCont = p->pCollect->pContainer;
    bdLogGCFinished(pCtx, p->pCollect);
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
  DContainer *pContainer,
  Manifest *pMan
){
  DCurlRequest *pCurl = bdCurlRequest(p);
  pCurl->aBuf = bcvManifestCompose(pMan, &pCurl->nBuf);
  pCurl->pMan = pMan;
  bcvManifestRef(pMan);

  curlPutBlob(&pCurl->req, &pContainer->ap, 
      pContainer->zName, BCV_MANIFEST_FILE, pMan->zETag, pCurl->nBuf
  );
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READFUNCTION, bdMemoryRead);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_READDATA, (void*)pCurl);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKFUNCTION, bdMemorySeek);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_SEEKDATA, (void*)pCurl);
  curl_easy_setopt(pCurl->req.pCurl, CURLOPT_WRITEFUNCTION, bdIgnore);

  return pCurl;
}

static void bdUploadBlockDone(DCurlRequest *pCurl, CURLMsg *pMsg){
  int rc = SQLITE_OK;
  DUpload *p = pCurl->pUpload;
  Manifest *pMan = p->pManifest;
  int bFail;
  
  assert( p->pContainer->eOp==CONTAINER_OP_UPLOAD );
  p->nRef--;
  bFail = bdIsHttpError(pCurl, pMsg, 0);
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
      /* Reply to the internal client */
      DMessage reply;
      memset(&reply, 0, sizeof(DMessage));
      reply.eType = BCV_MESSAGE_UPLOAD_REPLY;
      bdClientMessageSend(p->pCtx, p->pClient, &reply);
      p->pContainer->eOp = CONTAINER_OP_NONE;
      daemon_event_log(p->pCtx,
          "UPLOAD operation on container %s finished - failed",
          p->pContainer->zName
      );
      bdUploadFree(p);
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

      pReq = bdUploadManifestRequest(pCtx, p->pContainer, pMan);
      pReq->pUpload = p;
      bdCurlRun(pCtx, pReq, p->pContainer, bdUploadManifestDone);
    }
  }
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
  int iDb = bdFindDb(pCont, aId);
  const char *zLogMessage = "";

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
    zLogMessage = "no blocks to upload";
  }else if( pMsg->iVal==0 ){
    /* The checkpoint thread could not ensure that the entire wal is 
    ** checkpointed (presumably due to read locks). Reply to the checkpoint
    ** thread. The whole upload will be retried again in -retrytime 
    ** seconds. */
    bSendReply = 1;
    bReschedule = 1;
    zLogMessage = "failed to lock local file";
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
        u8 *aBlock = &pDb->aBlkOrig[NAMEBYTES(pMan)*iBlk];
        if( 0==bcvMHashMatch(pNew->pMHash, aBlock, NAMEBYTES(pMan)) ){
          u8 *aGC = &pMan->aDelBlk[pMan->nDelBlk*GCENTRYBYTES(pMan)];
          memcpy(aGC, aBlock, NAMEBYTES(pMan));
          pMan->nDelBlk++;
        }
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
        zLogMessage = "failed - will retry";
      }else{
        bdRemoveDCheckpoint(p, pCont, aId);
        zLogMessage = "no blocks to upload";
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
    bdClientMessageSend(p, pClient, &reply);

    assert( pCont->eOp==CONTAINER_OP_UPLOAD );
    pCont->eOp = CONTAINER_OP_NONE;
    daemon_event_log(p, 
        "UPLOAD operation on container %s finished - %s",
        zLogMessage
    );
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

  if( p->cmd.bVtab ){
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
  }

  bdClientMessageSend(p, pClient, &reply);
  sqlite3_free(reply.aBlob);
}

/*
** Create directory zName, if it does not already exist.
*/
static void bdMkdir(const char *zName){
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

  pNew = (sqlite3_file*)bcvMallocZero(pVfs->szOsFile + 4 + nPath + 2);
  zOpen = &((char*)pNew)[pVfs->szOsFile + 4];
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
**   4) If argument bZeroShm is true, write zeroes to the first 136 bytes of
**      the *-shm file.
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
static int bdHasLocalChanges(DContainer *pCont, ManifestDb *pDb, int bZeroShm){
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
    if( bZeroShm && zReason==0 && rc==SQLITE_OK ){
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

static int bdZeroShm(DaemonCtx *p, DContainer *pCont, ManifestDb *pDb){
  const sqlite3_io_methods *pMeth = 0;
  u8 *aMap = 0;
  sqlite3_file *pFile = 0;
  int rc = SQLITE_OK;

  /* Open the database and map the *-shm file */
  rc = bdOpenLocalFile(pCont->zName, pDb->zDName, 0, &pFile);
  if( rc==SQLITE_OK ){
    pMeth = pFile->pMethods;
    rc = pMeth->xShmMap(pFile, 0, 32*1024, 1, (volatile void **)&aMap);
  }

  if( rc==SQLITE_OK ){
    assert( pFile && aMap );
  
    /* Take the WRITER lock */
    if( pMeth->xShmLock(pFile, 0, 1, SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE) ){
      rc = SQLITE_BUSY;
    }else{
      /* Zero the first few bytes of the *-shm file */
      memset(aMap, 0, 136);
    }
  }

  pMeth->xShmLock(pFile, 0, 1, SQLITE_SHM_UNLOCK|SQLITE_SHM_EXCLUSIVE);
  pMeth->xShmUnmap(pFile, 0);
  bdCloseLocalFile(pFile);
  return rc;
}

static void bdContainerFree(DContainer *pCont){
  bcvManifestDeref(pCont->pManifest);
  bcvAccessPointFree(&pCont->ap);
  sqlite3_free(pCont);
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
  if( pCont==0 || pCont->bAutoDetach ){
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
      if( bdHasLocalChanges(pCont, &pMan->aDb[ii], 0) ){
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
      bdContainerFree(pCont);
    }
  }

  memset(&reply, 0, sizeof(DMessage));
  reply.eType = BCV_MESSAGE_QUERY_REPLY;
  if( zRet ){
    reply.aBlob = (u8*)zRet;
    reply.iVal = SQLITE_ERROR;
  }else{
    reply.aBlob = (u8*)"ok";
  }
  reply.nBlobByte = strlen((const char*)reply.aBlob);
  bdClientMessageSend(pCtx, pClient, &reply);
  sqlite3_free(zRet);
}

static void bdSendQueryReply(
  DaemonCtx *pCtx, 
  DClient *pClient, 
  int iVal,
  const char *zMsg
){
  DMessage reply;
  memset(&reply, 0, sizeof(reply));
  reply.eType = BCV_MESSAGE_QUERY_REPLY;
  reply.iVal = iVal;
  reply.aBlob = (u8*)zMsg;
  reply.nBlobByte = strlen(zMsg);
  bdClientMessageSend(pCtx, pClient, &reply);
}

/*
** Process a "PRAGMA bcv_attach = zCont" or "PRAGMA bcv_readonly = zCont"
** command (or equivalent).
*/
static void bdAddContainer(
  DaemonCtx *pCtx,                /* Daemon context */
  DClient *pClient,               /* Client to reply to */
  int flags,
  const char *zCont,              /* New container to attach */
  int bReadonly                   /* 1 -> bcv_readonly, 0 -> bcv_attach */
){
  int ii;
  const char *zSas = 0;
  char *zCopy = bcvStrdup(zCont);
  DContainer *pCont = 0;

  /* Check if the argument includes an SAS. */
  for(ii=0; zCopy[ii]; ii++){
    if( zCopy[ii]=='?' ){
      zCopy[ii] = '\0';
      zSas = &zCopy[ii+1];
      break;
    }
  }

  pCont = bdFindContainer(pCtx, zCopy);
  if( pCont ){
    if( pCont->bAutoDetach ){
      /* Some other client has already sent a bcv_attach or bcv_readonly
      ** message to this daemon to attach this container and is waiting
      ** for the manifest file to download. This client will wait on the 
      ** same reply from the cloud storage system. If this is a bcv_readonly
      ** command, ignore the SAS token in case the other client provided
      ** a read/write SAS token.  */
      if( flags & SQLITE_BCVATTACH_POLL ){
        pClient->pNextWaiting = pCont->pNextPollClient;
        pCont->pNextPollClient = pClient;
      }else{
        pClient->pNextWaiting = pCont->pPollClient;
        pCont->pPollClient = pClient;
      }
      if( bReadonly ){
        zSas = 0;
      }
    }else{
      Manifest *pMan = pCont->pManifest;
      ii = pMan->nDb;
      if( bReadonly && zSas ){
        /* In this case, check if there are any local changes to the 
        ** containers database. If there have been, then it is an error
        ** to try to install a read-only SAS token. */
        for(ii=0; ii<pMan->nDb; ii++){
          if( bdHasLocalChanges(pCont, &pMan->aDb[ii], 0) ){
            char *zMsg = sqlite3_mprintf("require r/w SAS token - "
                "local changes to container %s", zCopy
            );
            bdSendQueryReply(pCtx, pClient, SQLITE_ERROR, zMsg);
            sqlite3_free(zMsg);
            zSas = 0;
            break;
          }
        }
      }
      if( ii==pMan->nDb ){
        if( flags & SQLITE_BCVATTACH_POLL ){
          pClient->pNextWaiting = pCont->pNextPollClient;
          pCont->pNextPollClient = pClient;
          pCont->iPollTime = sqlite_timestamp();
        }else{
          bdSendQueryReply(pCtx, pClient, SQLITE_OK, "ok");
        }
      }
    }
  }else{
    Manifest *pMan;
    int nCont = strlen(zCopy);

    if( zSas==0 && pCtx->ap_template.zAccessKey==0 ){
      bdSendQueryReply(
          pCtx, pClient, SQLITE_ERROR, "attach requires an sas token"
      );
      sqlite3_free(zCopy);
      return;
    }

    /* Allocate and populate the new container object. Then link it into
    ** the daemon context object. */
    pCont = (DContainer*)bcvMallocZero(sizeof(DContainer)+nCont+1);
    pCont->zName = (char*)&pCont[1];
    memcpy(pCont->zName, zCopy, nCont);
    pCont->iPollTime = sqlite_timestamp();
    pCont->iGCTime = pCont->iPollTime + pCtx->cmd.nGCTime*1000;
    pCont->pNext = pCtx->pContainerList;
    pCtx->pContainerList = pCont;
    bcvAccessPointCopy(&pCont->ap, &pCtx->ap_template);

    /* If required, create a directory for the new container */
    bdMkdir(zCopy);

    /* Create and install an empty manifest. */
    pMan = bcvMallocZero(sizeof(Manifest));
    pMan->szBlk = pCtx->szBlk;
    pMan->nNamebytes = BCV_DEFAULT_NAMEBYTES;
    pMan->nRef = 1;
    bdManifestInstall(pCtx, pCont, pMan, 0, 0);

    /* Reply to the client after the manifest has been polled. If an error
    ** occurs, the container will be automatically detached at that point.  */
    pCont->pPollClient = pClient;
    pCont->bAutoDetach = 1;
  }

  if( zSas ){
    /* TODO: Handle the case where a container already as read-write SAS,
    ** but argument bReadonly is true. Or when bReadonly is true but there
    ** are already local changes to one or more database files */
    sqlite3_free(pCont->ap.zSas);
    pCont->ap.zSas = bcvStrdup(zSas);
    pCont->bReadonly = bReadonly;
  }
  sqlite3_free(zCopy);
}

/*
** Handle a PRAGMA query delivered via a BCV_MESSAGE_QUERY message.
*/
static void bdPragmaQuery(
  DaemonCtx *p,                   /* Daemon object */
  DClient *pClient,               /* Client that sent QUERY message */
  int flags,
  const char *zName,              /* Name of pragma */
  const char *zArg                /* Argument passed to pragma */
){
  if( sqlite3_stricmp("bcv_upload", zName)==0 ){
    DCheckpoint *pCkpt = bdFindOrCreateCheckpoint(p, pClient);
    pClient->pNextWaiting = pCkpt->pWaiting;
    pCkpt->pWaiting = pClient;
  }else
  if( sqlite3_stricmp("bcv_account", zName)==0 ){
    DMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_QUERY_REPLY;
    reply.aBlob = (u8*)p->cmd.zAccount;
    reply.nBlobByte = bcvStrlen((char*)reply.aBlob);
    bdClientMessageSend(p, pClient, &reply);
  }else
  if( sqlite3_stricmp("bcv_attach", zName)==0 ){
    bdAddContainer(p, pClient, flags, zArg, 0);
  }else
  if( sqlite3_stricmp("bcv_readonly", zName)==0 ){
    bdAddContainer(p, pClient, flags, zArg, 1);
  }else
  if( sqlite3_stricmp("bcv_detach", zName)==0 ){
    bdDelContainer(p, pClient, zArg);
  }else{
    DMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_QUERY_REPLY;
    reply.aBlob = (u8*)bcvMprintf("no such pragma: %s", zName);
    reply.nBlobByte = strlen((char*)reply.aBlob);
    bdClientMessageSend(p, pClient, &reply);
    sqlite3_free(reply.aBlob);
  }
}

/*
** Handle a BCV_MESSAGE_QUERY message.
*/
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
    bdPragmaQuery(p, pClient, pMsg->iVal, z1, z2);
    sqlite3_free(z1);
    sqlite3_free(z2);
  }else{
    bdVtabQuery(p, pClient, pMsg);
  }
}

/*
** Handle a BCV_MESSAGE_LOGIN message.
*/
static void bdHandleClientLogin(
  DaemonCtx *p, 
  DClient *pClient, 
  DMessage *pMsg
){
  char *zCont = 0;
  char *zDb = 0;
  int i;
  DContainer *pCont;

  for(i=0; i<pMsg->nBlobByte && pMsg->aBlob[i]!='/'; i++);
  if( i==pMsg->nBlobByte ){
    daemon_error("error parsing login message");
    bdCloseConnection(p, pClient);
    return;
  }
  zCont = bcvMprintf("%.*s", i, (const char*)pMsg->aBlob);
  zDb = bcvMprintf("%.*s", pMsg->nBlobByte-i-1,(const char*)&pMsg->aBlob[i+1]);

  for(pCont=p->pContainerList; pCont; pCont=pCont->pNext){
    if( 0==bcvStrcmp(pCont->zName, zCont) ) break;
  }
  hex_decode(zDb, BCV_DBID_SIZE*2, pClient->aId);
  if( pCont==0 || bdFindDb(pCont, pClient->aId)<0 ){
    daemon_error("no such container/db: %s/%s", zCont, zDb);
    bdCloseConnection(p, pClient);
  }else{
    DMessage reply;
    char *zReply = bcvMprintf("%s/%s", pCont->zName, p->cmd.zAccount);
    pClient->pContainer = pCont;
    memset(&reply, 0, sizeof(DMessage));
    reply.eType = BCV_MESSAGE_LOGIN_REPLY;
    reply.iVal = pCont->pManifest->szBlk;
    reply.nBlobByte = bcvStrlen(zReply);
    reply.aBlob = (u8*)zReply;
    bdClientMessageSend(p, pClient, &reply);
    sqlite3_free(zReply);
  }

  sqlite3_free(zCont);
  sqlite3_free(zDb);
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
  int nRead = 0;
  int nBody = 0;
  u8 *aBody = 0;

  nRead = (int)recv(s, aMsg, 5, 0);
  if( nRead!=5 ) return 1;

  nBody = (int)bcvGetU32(&aMsg[1]) - 5;
  if( nBody>0 ){
    aBody = (u8*)bcvMalloc(nBody);
    nRead = (int)recv(s, aBody, nBody, 0);
  }else{
    nRead = 0;
  }
  memset(pMsg, 0, sizeof(DMessage));
  if( nRead!=nBody || bdParseMessage(aMsg[0], aBody, nBody, pMsg) ){
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
static void bdHandleClientMessage(
  DaemonCtx *p, 
  DClient *pClient, 
  int *pbLogin                    /* Set to true if message is a LOGIN */
){
  DMessage msg;
  u8 *aBody = 0;

  if( bdClientMessage(pClient->fd, &msg, &aBody) ){
    bdCloseConnection(p, pClient);
  }else{
    bdLogMessage(p, pClient, &msg);
    switch( msg.eType ){
      case BCV_MESSAGE_LOGIN: {
        bdHandleClientLogin(p, pClient, &msg);
        *pbLogin = 1;
        break;
      }
      case BCV_MESSAGE_REQUEST: {
        bdHandleClientRequest(p, pClient, &msg);
        break;
      }
      case BCV_MESSAGE_DONE: {
        bdClientClearRefs(pClient);
        bdProcessUsed(p, pClient, msg.aBlob, msg.nBlobByte);
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
    bdParseFree(&msg);
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

/*
** The daemon has hit a fatal write-conflict error. Database iOld of
** the current manifest of container pCont has been modified locally, but
** has also been modified within the cloud. The system cannot currently
** handle this scenario, so it considers it a fatal error. This function:
**
**   + Sends an error message to all connected clients and closes the
**     localhost sockets,
**   + outputs an error message, and
**   + calls exit(1).
*/
static void bdFatalConflict(DaemonCtx *pCtx, DContainer *pCont, int iOld){
  DClient *pClient;
  DMessage msg;

  memset(&msg, 0, sizeof(DMessage));
  msg.eType = BCV_MESSAGE_ERROR_REPLY;
  msg.iVal = 1;
  msg.aBlob = (u8*)bcvMprintf(
      "fatal write conflict in container %s (db %s)", pCont->zName,
      pCont->pManifest->aDb[iOld].zDName
  );
  msg.nBlobByte = strlen((char*)msg.aBlob);
  for(pClient=pCtx->pClientList; pClient; pClient=pClient->pNext){
    bdClientMessageSend(pCtx, pClient, &msg);
    bdCloseConnection(pCtx, pClient);
  }

  daemon_error("exiting - %s", (char*)msg.aBlob);
  exit(1);
}

/*
** If the hash-table DaemonCtx.aHash[] has not been allocated, allocate it
** now. This can't be done until the first manifest has been obtained from
** the cloud and the system knows the block-size.
*/
static void bdAllocateHash(DaemonCtx *p){
  if( p->szBlk==0 && p->pContainerList ){
    p->szBlk = p->pContainerList->pManifest->szBlk;
    p->nCacheMax = (int)(p->cmd.szCache / p->szBlk);
    p->nHash = 2;
    while( p->nHash<p->nCacheMax ) p->nHash = p->nHash*2;
    p->aHash = (DCacheEntry**)bcvMallocZero(p->nHash * sizeof(DCacheEntry*));
    p->nCacheUsedEntry = (p->nCacheMax/32)+1;
    p->aCacheUsed = (u32*)bcvMallocZero(p->nCacheUsedEntry * sizeof(u32));
  }
}


/*
** The callback for when a new manifest file has been downloaded from 
** cloud storage.
*/
static void bdDownloadManifestDone(DCurlRequest *p, CURLMsg *pMsg){
  DaemonCtx *pCtx = p->pDaemonCtx;     /* Daemon context */
  char *zETag = p->req.zETag;          /* e-tag value for new manifest */
  DContainer *pCont = p->pContainer;   /* Associated container */
  int bNew;                            /* True if new manifest */
  int bFail;                           /* True if HTTP error */
  int bInstall = 1;                    /* True to use this manifest */
  
  bFail = bdIsHttpError(p, pMsg, 0);
  bNew = bcvStrcmp(pCont->pManifest->zETag, zETag);
  if( bNew && bFail==0 ){
    int iNew;                     /* Index into pNew->aDb[] */
    int iOld;                     /* Index into pOld->aDb[] */
    Manifest *pNew = 0;
    Manifest *pOld = pCont->pManifest;
    int rc;
    char *zErr = 0;

    rc = bcvManifestParse(p->aBuf, p->nBuf, zETag, &pNew, &zErr);
    if( rc!=SQLITE_OK ){
      fatal_error("error parsing manifest: %s", zErr);
    }

    p->aBuf = 0;
    p->nBuf = 0;
    p->req.zETag = 0;

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
        if( bdHasLocalChanges(pCont, pOldDb, 1) ){
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
          if( bdHasLocalChanges(pCont, pOldDb, 1) ){
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
      assert( iOld<pOld->nDb );
      bcvManifestDeref(pNew);
      daemon_error("cannot use new manifest due to local changes");
      if( pCont->bAutoDetach==0 ){
        bdFatalConflict(pCtx, pCont, iOld);
      }
    }
  }

  /* Send QUERY_REPLY messages to any clients waiting for them. Messages must
  ** be sent to all clients in the pPollClient list, and in the pNextPollClient
  ** list as well if an error has occurred. Additionally, if an error has
  ** occurred and the bAutoDetach flag is set, detach the container.  
  */
  if( bFail || (bNew && bInstall==0) ){
    /* An error has occurred. Add the pNextPollClient list to the end of
    ** the pPollClient list so that those clients get messages too. */
    DClient **ppC;
    for(ppC=&pCont->pPollClient; *ppC; ppC=&((*ppC)->pNextWaiting));
    *ppC = pCont->pNextPollClient;
    pCont->pNextPollClient = 0;
  }
  if( pCont->pPollClient ){
    DClient *pClient;
    DMessage reply;
    memset(&reply, 0, sizeof(reply));
    reply.eType = BCV_MESSAGE_QUERY_REPLY;

    if( bFail ){
      int httpcode = bdHttpCode(p, pMsg);
      reply.iVal = (httpcode>=400) ? httpcode : SQLITE_ERROR;
      reply.aBlob = (u8*)bcvMprintf(
          "failed to load manifest for %s: %s",
          pCont->zName, bdRequestError(p, pMsg)
      );
    }else if( bNew && bInstall==0 ){
      reply.aBlob = (u8*)bcvMprintf("cannot attach due to local changes");
      reply.iVal = SQLITE_ERROR;
    }else{
      bdAllocateHash(pCtx);
      reply.aBlob = (u8*)bcvMprintf("ok");
    }
    reply.nBlobByte = bcvStrlen((const char*)reply.aBlob);
    
    for(pClient=pCont->pPollClient; pClient; pClient=pClient->pNextWaiting){
      bdClientMessageSend(pCtx, pClient, &reply);
    }
    pCont->pPollClient = 0;
    sqlite3_free(reply.aBlob);
  }

  daemon_event_log(pCtx,
      "POLL MANIFEST operation on container %s finished - %s",
      pCont->zName,
      bFail ? "failed" : 
      (bNew && bInstall==0) ? "cannot install due to local changes" : "ok"
  );

  if( pCont->bAutoDetach && (bFail || (bNew && bInstall==0)) ){
    /* Detach the container. */
    DContainer **pp;
    for(pp=&pCtx->pContainerList; *pp!=pCont; pp=&((*pp)->pNext));
    *pp = pCont->pNext;
    bdContainerFree(pCont);
    pCont = 0;
  }
  if( pCont ){
    pCont->bAutoDetach = 0;
    assert( pCont->eOp==CONTAINER_OP_POLL );
    pCont->eOp = CONTAINER_OP_NONE;
    pCont->iPollTime = sqlite_timestamp();
    if( pCont->pNextPollClient==0 ){
      pCont->iPollTime += p->pDaemonCtx->cmd.nPollTime*1000;
    }
  }
}

static void bdCurlLogReply(DaemonCtx *p, DCurlRequest *pCurl, CURLMsg *pMsg){
  if( 0!=(p->cmd.mLog & BCV_LOG_HTTP) ){
    int bEtag = pCurl->req.zETag && (
        (pCurl->xDone==bdDownloadManifestDone) 
     || (pCurl->xDone==bdUploadManifestDone) 
    );
    daemon_log(p, BCV_LOG_HTTP, "r%d (%d) %s %s%s%s", pCurl->iReq,
        bdHttpCode(pCurl, pMsg), bdRequestError(pCurl, pMsg),
        bEtag?"(etag=":"", bEtag?pCurl->req.zETag:"", bEtag?")":""
    );
  }
}

/*
** This is called when it is time to poll cloud storage for a new manifest
** file for container pCont.
*/
static void bdPollManifest(DaemonCtx *p, DContainer *pCont){
  assert( pCont->eOp==CONTAINER_OP_NONE );

  daemon_event_log(p,
      "POLL MANIFEST operation on container %s starting",
      pCont->zName
  );

  if( bdRequireSas(pCont) ){
    pCont->iSasShmZero--;
    if( pCont->iSasShmZero==0 ){
      int ii;
      pCont->iSasShmZero = 100;
      for(ii=0; ii<pCont->pManifest->nDb; ii++){
        int rc = bdZeroShm(p, pCont, &pCont->pManifest->aDb[ii]);
        if( rc!=SQLITE_OK ){
          pCont->iSasShmZero = 1;
        }
      }
    }
    pCont->iPollTime = sqlite_timestamp() + p->cmd.nPollTime*1000;
    daemon_event_log(p,
        "POLL MANIFEST operation on container %s finished - "
        "failed: container requires an SAS",
        pCont->zName
    );
  }else{
    const char *zCont = pCont->zName;
    DCurlRequest *pCurl;
    pCont->iSasShmZero = 2;
    pCurl = bdCurlRequest(p);
    curlFetchBlob(&pCurl->req, &pCont->ap, zCont, BCV_MANIFEST_FILE);
    bdCurlMemoryDownload(pCurl);

    assert( pCont->eOp==CONTAINER_OP_NONE );
    pCont->eOp = CONTAINER_OP_POLL;
    bdCurlRun(p, pCurl, pCont, bdDownloadManifestDone);
    daemon_log(p, BCV_LOG_POLL, "polling manifest for container %s", zCont);
    if( pCont->pPollClient==0 ){
      pCont->pPollClient = pCont->pNextPollClient;
      pCont->pNextPollClient = 0;
    }
  }
}

static void bdDeleteOneBlock(DDelete *p);

static void bdDeleteBlockDone(DCurlRequest *pReq, CURLMsg *pMsg){
  DDelete *p = pReq->pDelete;
  int bErr;

  assert( p->pContainer->eOp==CONTAINER_OP_DELETE );
  p->nReq--;
  bErr = bdIsHttpError(pReq, pMsg, 404);
  if( bErr ) p->bErr = 1;
  if( p->bErr==0 ){
    bdDeleteOneBlock(p);
  }

  if( p->nReq==0 ){
    DaemonCtx *pCtx = pReq->pDaemonCtx;
    if( p->bErr ){
      DContainer *pCont = p->pContainer;
      /* Retry the delete operation in nRetryTime seconds. */
      bdDeleteFree(p);
      pCont->iDeleteTime = sqlite_timestamp()+pCtx->cmd.nRetryTime*1000;
      pCont->eOp = CONTAINER_OP_NONE;
      daemon_event_log(pCtx,
          "DELETE operation on container %s finished - failed", 
          pCont->zName
      );
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
      pUp = bdUploadManifestRequest(pCtx, p->pContainer, pMan);
      pUp->pDelete = p;
      bdCurlRun(pReq->pDaemonCtx, pUp, p->pContainer, bdUploadManifestDone);
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
      DContainer *pCont = p->pContainer;
      char zFile[BCV_MAX_FSNAMEBYTES];
      DCurlRequest *pReq = bdCurlRequest(p->pCtx);
      bcvBlockidToText(pMan, aBlk, zFile);
      curlDeleteBlob(&pReq->req, &pCont->ap, pCont->zName, zFile);
      p->nReq++;
      pReq->pDelete = p;
      bdCurlRun(p->pCtx, pReq, pCont, bdDeleteBlockDone);
      bcvPutU64(&aBlk[NAMEBYTES(pMan)], 0);
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

  daemon_event_log(p,
      "DELETE operation on container %s starting",
      pCont->zName
  );

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
  if( bdIsHttpError(pReq, pMsg, 0) ){
    pCont->iGCTime = sqlite_timestamp() + pCtx->cmd.nGCTime*1000;
    bdCollectFree(p);
    pCont->eOp = CONTAINER_OP_NONE;
    daemon_event_log(pCtx,
        "GARBAGE COLLECTION operation on container %s finished - failed",
        pCont->zName
    );
  }else{
    Manifest *pMan = p->pMan;
    FilesParse files;
    FilesParseEntry *pEntry;

    memset(&files, 0, sizeof(files));
    bcvParseFiles(pCtx->cmd.eStorage, pReq->aBuf, pReq->nBuf, &files);
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
      }
    }

    if( files.zNextMarker ){
      DCurlRequest *pCurl = bdCurlRequest(pCtx);
      curlListFiles(&pCurl->req, &pCont->ap, pCont->zName, files.zNextMarker);
      pCurl->pCollect = p;
      bdCurlMemoryDownload(pCurl);
      bdCurlRun(pCtx, pCurl, pCont, bdListFilesDone);
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

      pReq = bdUploadManifestRequest(pCtx, pCont, pMan);
      pReq->pCollect = p;
      bdCurlRun(pCtx, pReq, pCont, bdUploadManifestDone);
    }else{
      bdLogGCFinished(pCtx, p);
      pCont->iGCTime = sqlite_timestamp() + pCtx->cmd.nGCTime*1000;
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

  daemon_event_log(pCtx,
      "GARBAGE COLLECTION operation on container %s starting",
      pCont->zName
  );

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
  curlListFiles(&pCurl->req, &pCont->ap, pCont->zName, 0);
  pCurl->pCollect = p;
  bdCurlMemoryDownload(pCurl);
  bdCurlRun(pCtx, pCurl, pCont, bdListFilesDone);

  assert( pCont->eOp==CONTAINER_OP_NONE );
  pCont->eOp = CONTAINER_OP_COLLECT;
}

/*
** Argument pMsg contains the response to remote request pReq. If pReq
** used an SAS token to authenticate, and authentication failed (HTTP error
** 403), discard the SAS token. 
*/
static void bdHandleSasTimeout(DCurlRequest *pReq, CURLMsg *pMsg){
  DContainer *pCont = pReq->pContainer;
  if( pCont->ap.zSas && pMsg->data.result==CURLE_OK ){
    DaemonCtx *pCtx = pReq->pDaemonCtx;
    long httpcode;
    curl_easy_getinfo(pReq->req.pCurl, CURLINFO_RESPONSE_CODE, &httpcode);
    if( httpcode==HTTP_AUTH_ERROR ){
      DCheckpoint *pCkpt;
      sqlite3_free(pCont->ap.zSas);
      pCont->ap.zSas = 0;

      /* Send replies to any outstanding "PRAGMA bcv_upload" messages. */
      for(pCkpt=pCont->pCheckpointList; pCkpt; pCkpt=pCkpt->pNext){
        DClient *pClient;
        DClient *pNext;
        for(pClient=pCkpt->pWaiting; pClient; pClient=pNext){
          DMessage msg;
          memset(&msg, 0, sizeof(DMessage));
          msg.eType = BCV_MESSAGE_ERROR_REPLY;
          msg.iVal = HTTP_AUTH_ERROR;
          msg.aBlob = (u8*)"SAS token failure";
          msg.nBlobByte = strlen((const char*)msg.aBlob);
          bdClientMessageSend(pCtx, pClient, &msg);
          pNext = pClient->pNextWaiting;
          pClient->pNextWaiting = 0;
        }
        pCkpt->pWaiting = 0;
      }
    }
  }
}

/*
** Figure out the next scheduled operation, if any, on the container passed 
** as the first argument. Return one of the following constants:
**
**     CONTAINER_OP_NONE (there is no scheduled operation)
**     CONTAINER_OP_POLL
**     CONTAINER_OP_DELETE
**     CONTAINER_OP_COLLECT
**     CONTAINER_OP_UPLOAD
**
** Unless CONTAINER_OP_NONE is returned, set (*piTime) to the time that
** the operation is scheduled to before returning. If the scheduled 
** operation is a CONTAINER_OP_UPLOAD and ppCkpt is not NULL, also
** set *ppCkpt to point to the associated DCheckpoint object.
*/
static int bdContainerOp(
  DaemonCtx *p,
  DContainer *pCont,
  sqlite3_int64 *piTime, 
  DCheckpoint **ppCkpt
){
  sqlite3_int64 iTm = 0;
  int eOp = CONTAINER_OP_NONE;

  if( ppCkpt ) *ppCkpt = 0;
  *piTime = 0;
  if( pCont->eOp==CONTAINER_OP_NONE ){
    iTm = pCont->iPollTime;
    eOp = CONTAINER_OP_POLL;

    if( pCont->bReadonly==0 && bdRequireSas(pCont)==0 ){
      DCheckpoint *pCkpt;
      if( p->cmd.bNodelete==0 ){
        if( pCont->iDeleteTime && pCont->iDeleteTime<iTm ){
          iTm = pCont->iDeleteTime;
          eOp = CONTAINER_OP_DELETE;
        }
        if( pCont->iGCTime && pCont->iGCTime<iTm ){
          iTm = pCont->iGCTime;
          eOp = CONTAINER_OP_COLLECT;
        }
      }
      for(pCkpt=pCont->pCheckpointList; pCkpt; pCkpt=pCkpt->pNext){
        if( pCkpt->iRetryTime<iTm ){
          iTm = pCkpt->iRetryTime;
          eOp = CONTAINER_OP_UPLOAD;
          if( ppCkpt ) *ppCkpt = pCkpt;
        }
      }
    }
  }

  *piTime = iTm;
  return eOp;
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
    int bBusy = 0;                /* True if there are pending container ops */

    /* Work on http requests. Issue xDone callbacks if any are finished. */
    mc = curl_multi_perform(p->pMulti, &nDummy);
    if( mc!=CURLM_OK ){
      fatal_error("curl_multi_perform() failed - %d", mc);
    }
    while( (pMsg = curl_multi_info_read(p->pMulti, &nDummy)) ){
      DCurlRequest *pCurl;
      DCurlRequest **pp;
      for(pCurl=p->pRequestList; pCurl; pCurl=pCurl->pNext){
        if( pMsg->easy_handle==pCurl->req.pCurl ) break;
      }
      assert( pCurl );
      bdCurlLogReply(p, pCurl, pMsg);
      bdHandleSasTimeout(pCurl, pMsg);
      pCurl->xDone(pCurl, pMsg);
      
      for(pp=&p->pRequestList; *pp!=pCurl; pp=&(*pp)->pNext);
      *pp = pCurl->pNext;
      curl_multi_remove_handle(p->pMulti, pCurl->req.pCurl);
      bdCurlRelease(p, pCurl);
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
      i64 iTm;
      bdContainerOp(p, pCont, &iTm, 0);
      if( iTm-iTime<iTimeout ){
        iTimeout = iTm - iTime;
        iTimeout = MAX(iTimeout, 1);
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
        bdHandleClientMessage(p, pClient, &bClient);
      }
      i++;
    }

    /* Check for new connections */
    if( aWait[0].revents!=0 ){
      bdNewConnection(p);
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
      int eOp = 0;
      i64 iTm = 0;
      DCheckpoint *pCkpt = 0;

      eOp = bdContainerOp(p, pCont, &iTm, &pCkpt);
      if( iTm<=iTime ){
        switch( eOp ){
          case CONTAINER_OP_POLL:
            bdPollManifest(p, pCont);
            break;
          case CONTAINER_OP_DELETE:
            bdDeleteBlocks(p, pCont);
            break;
          case CONTAINER_OP_COLLECT:
            bdStartGC(p, pCont);
            break;
          case CONTAINER_OP_UPLOAD:
            bdLaunchUpload(p, pCont, pCkpt);
            break;
          default:
            assert( eOp==CONTAINER_OP_NONE );
            break;
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

  bcvAccessPointFree(&p->ap_template);

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
    bdContainerFree(pCont);
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
** Command: $argv[0] daemon ?SWITCHES? [CONTAINER1...]
**
** List all containers in the specified account.
*/
static int main_daemon(int argc, char **argv){
  CommandSwitches *pCmd;
  CurlRequest curl;               /* Used to grab intitial manifests */
  DaemonCtx *p = 0;
  int iCont;
  const char **azArg = (const char**)&argv[2];
  int nArg = argc-2;
  int rc;
  int iPort;                      /* Port that daemon listens on */
  i64 nUsed;
  i64 iGCTime;                    /* Time for first GC attempt */
  sqlite3_stmt *pReader = 0;      /* For reading "block" table */
  sqlite3_stmt *pIter = 0;        /* For reading "manifest" table */

  p = (DaemonCtx*)bcvMallocZero(sizeof(DaemonCtx));
  p->pMulti = curl_multi_init();
  pCmd = &p->cmd;

  /* Parse command line */
  if( argc<=2 ) daemon_usage(argv[0]);
  parse_switches(pCmd, azArg, nArg, &iCont,
    COMMANDLINE_DIRECTORY | COMMAND_DAEMON
  );
  if( pCmd->zDirectory==0 ) missing_required_switch("-directory");
  if( pCmd->szCache<=0 ) pCmd->szCache = BCV_DEFAULT_CACHEFILE_SIZE;

  if( pCmd->nWrite<=0 ) pCmd->nWrite = pCmd->zEmulator?1:BCV_DEFAULT_NWRITE;
  if( pCmd->nDelete<=0 ) pCmd->nDelete = pCmd->zEmulator?1:BCV_DEFAULT_NDELETE;
  if( pCmd->nPollTime<=0 ) pCmd->nPollTime = BCV_DEFAULT_POLLTIME;
  if( pCmd->nDeleteTime<=0 ) pCmd->nDeleteTime = BCV_DEFAULT_DELETETIME;
  if( pCmd->nGCTime<=0 ) pCmd->nGCTime = BCV_DEFAULT_GCTIME;
  if( pCmd->nRetryTime<=0 ) pCmd->nRetryTime = BCV_DEFAULT_RETRYTIME;

  if( iCont<nArg && pCmd->zAccessKey==0 ){
    fatal_error("Specifying containers on command line requires -accesskey");
  }
  bcvAccessPointNew(&p->ap_template, &p->cmd, 1);

  /* Open the listen socket */
  iPort = bdListen(p);

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
    bcvAccessPointCopy(&pNew->ap, &p->ap_template);
    if( nMan>0 ){
      u8 *aDup = bcvMemdup(nMan, aMan);
      char *zErr = 0;
      rc = bcvManifestParse(aDup, nMan, bcvStrdup(zETag), &pMan, &zErr);
      if( rc!=SQLITE_OK ){
        fatal_error("corrupt manifest in db: %s", zErr);
      }
      bdManifestInstall(p, pNew, pMan, 0, 0);
      pNew->iPollTime = sqlite_timestamp(); /* poll for new manifest ASAP */
    }else{
      pMan = bcvManifestGetParsed(&curl, &pNew->ap, zCont, &nMan);
      bdManifestInstall(p, pNew, pMan, pMan->pFree, nMan);
      curlRequestReset(&curl);
    }

    bdMkdir(zCont);
    for(iDb=0; iDb<pNew->pManifest->nDb; iDb++){
      ManifestDb *pDb = &pNew->pManifest->aDb[iDb];
      bdOpenDatabaseFile(iPort, zCont, pDb->zDName, pDb->aId);
    }

    pNew->pNext = p->pContainerList;
    p->pContainerList = pNew;
  }
  curlRequestFinalize(&curl);
  sqlite3_finalize(pIter);

  /* Allocate space for the hash table and cache bitmap */
  bdAllocateHash(p);

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
        int iDb = bdFindDb(pCont, aId);
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
      bdAddToLRU(p, pEntry);
      p->nCacheEntry++;
      p->aCacheUsed[pEntry->iPos / 32] |= (1 << (pEntry->iPos%32));
    }
  }
  rc = sqlite3_finalize(pReader);
  pReader = 0;

  /* Open the cache file */
  rc = bdOpenLocalFile(".", BCV_CACHEFILE_NAME, 0, &p->pCacheFile);
  if( rc!=SQLITE_OK ){
    fatal_error("failed to open cachefile: %s", BCV_CACHEFILE_NAME);
  }

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
  char *zDir = 0;                 /* Directory argument */
  char *zCont = 0;                /* Container argument */
  int rc = SQLITE_OK;
  char *zErr = 0;

  if( argc<4 ){
    fprintf(stderr, "Usage: %s %s ?SWITCHES? DIRECTORY CONTAINER\n", 
        argv[0], bAttach ? "attach" : "detach"
    );
    return 1;
  }
  zDir = argv[2];
  zCont = argv[3];

  if( bAttach ){
    rc = sqlite3_bcv_attach(zDir, zCont, 0, 0, &zErr);
  }else{
    rc = sqlite3_bcv_detach(zDir, zCont, &zErr);
  }
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "%s\n", (zErr ? zErr : ""));
  }
  sqlite3_free(zErr);
  return rc;
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
    { "attach",   1 , 11},
    { "detach",   1 , 12},
    { 0, 0 }
  };
  int rc;
  int iCmd;
  sqlite3_vfs *pVfs;

  bcv_socket_init();
  curl_global_init(CURL_GLOBAL_ALL);
  OpenSSL_add_all_algorithms();

  sqlite3_initialize();
  pVfs = sqlite3_vfs_find("blockcachevfs");
  sqlite3_vfs_register((sqlite3_vfs*)pVfs->pAppData, 1);

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
    case 11:
      rc = main_attach(argc, argv, 1);
      break;
    case 12:
      rc = main_attach(argc, argv, 0);
      break;
  }

  return rc;
}

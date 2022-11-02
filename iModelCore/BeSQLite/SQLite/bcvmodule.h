/*
** 2020-07-08
**
******************************************************************************
**
** This file contains declarations for the public interface used to create
** new cloud storage modules. The source code comments within this file
** that document these interfaces should be read in conjunction with the
** documentation in file www/module.wiki of the source code. Or:
**
**     https://sqlite.org/blockcachevfs/doc/trunk/www/module.wiki
*/

#ifndef _BCVMODULE_H
#define _BCVMODULE_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sqlite3_bcv_module sqlite3_bcv_module;
typedef struct sqlite3_bcv_container sqlite3_bcv_container;
typedef struct sqlite3_bcv_request sqlite3_bcv_request;
typedef struct sqlite3_bcv_job sqlite3_bcv_job;

/*
** Structure containing a pointer to each method required by a cloud 
** storage module implementation.
*/
struct sqlite3_bcv_module {
  int (*xOpen)(
    void *pCtx,
    const char **azParam,
    const char *zUser, const char *zAuth, 
    const char *zContainer, sqlite3_bcv_container **pp,
    char **pzErrmsg
  );
  void (*xClose)(sqlite3_bcv_container*);

  void (*xFetch)(
      sqlite3_bcv_container*, sqlite3_bcv_job*, 
      const char *zFile,
      int flags, const void *zCond, int nCond
  );
  void (*xPut)(
    sqlite3_bcv_container*, sqlite3_bcv_job*, 
    const char *zFile, 
    const unsigned char *zData, int nData,
    const char *zETag
  );
  void (*xDelete)(
    sqlite3_bcv_container*, sqlite3_bcv_job*, 
    const char *z, const char *zETag
  );
  void (*xList)(sqlite3_bcv_container*, sqlite3_bcv_job*);
  void (*xCreate)(sqlite3_bcv_container*, sqlite3_bcv_job*);
  void (*xDestroy)(sqlite3_bcv_container*, sqlite3_bcv_job*);
};

/*
** Candidate flags for third argument of xFetch() method.
*/
#define BCV_MODULE_CONDITION_MD5  0x01
#define BCV_MODULE_CONDITION_ETAG 0x02

/*
** Register a new cloud-storage module. SQLITE_OK is returned if the module 
** is successfully registered, or an SQLite error code (e.g. SQLITE_NOMEM) 
** if an error occurs.
*/
int sqlite3_bcv_create_module(
  const char *zName,              /* Name of module (e.g. "azure") */
  sqlite3_bcv_module *pMod,       /* Method implementations for new module */
  void *pArg                      /* First arg to pass to pMod->xOpen() */
);

/*
** Release all global resources allocated for both built-in cloud storage
** modules and any modules added using sqlite3_bcv_create_module(). The
** caller must ensure that there are no outstanding sqlite3_bcv handles,
** users of databases that use BCV VFSs, or any other callers of BCV
** API functions when this is called, otherwise the effects are undefined.
*/
void sqlite3_bcv_shutdown();

/*
** Create a new HTTP(S) request that will be issued when the current
** cloud module method invocation or HTTP(S) callback returns. 
**
** The callback function specified by the third argument to this function 
** is invoked when a reply to the HTTP(S) request is received, or when a
** network or other error occurs. When it is invoked, the callback function
** is passed a copy of the job handle, a copy of the request handle returned
** by this call, and a copy of the second (void*) argument passed to this
** call.
*/
sqlite3_bcv_request *sqlite3_bcv_job_request(sqlite3_bcv_job*, void*,
  void (*xCallback)(sqlite3_bcv_job*, sqlite3_bcv_request*, void*)
);

/*
** Set the method of an HTTP(S) request allocated by sqlite3_bcv_job_request()
** within the same cloud module method call or HTTP(S) reply callback.
** The second argument must be one of SQLITE_BCV_METHOD_GET, _PUT, _DELETE
** or _HEAD. If this function is not called on a request handle, the
** request uses the default method - GET.
*/
void sqlite3_bcv_request_set_method(sqlite3_bcv_request*, int eMethod);

/*
** Values that may be passed to sqlite3_bcv_request_set_method():
*/
#define SQLITE_BCV_METHOD_GET     1
#define SQLITE_BCV_METHOD_PUT     2
#define SQLITE_BCV_METHOD_DELETE  3
#define SQLITE_BCV_METHOD_HEAD    4

/*
** Set the URI of an HTTP(S) request allocated by sqlite3_bcv_job_request()
** within the same cloud module method call or HTTP(S) reply callback.
** The second argument must be a nul-terminated string containing a full
** http: or https: URI.
**
** This function must be called on all request handles allocated by
** sqlite3_bcv_job_request(). The effects of returning from the cloud module
** method call or HTTP(S) reply callback without having called this function
** on a new request handle are undefined (and may include the entire process
** exiting via abort()).
*/
void sqlite3_bcv_request_set_uri(sqlite3_bcv_request*, const char *zUri);

/*
** Calling this function is optional. If it is not called, and logging is
** enabled, the URI as passed to sqlite3_bcv_request_set_uri() may be
** output. Or, if this function is called, then the text passed to this
** function is output instead. This may be useful if the URI contains
** sensitive information, such as an SAS token.
*/
void sqlite3_bcv_request_set_log(sqlite3_bcv_request*, const char *zLog);

/*
** Specify an HTTP(S) header to include in the request. The argument
** should point to a nul-terminated string containing the HTTP(S) header
** in the usual "Name: Value" format. The string should not include any
** trailing carraige return or newline characters. For example:
**
**     "Content-Type: application/octet-stream"
**
** This function takes a copy of the header if necessary. The cloud storage
** module implementation does not have to ensure that it remains valid after
** the call to this function.
**
** This function may only be called on a request handle from within the
** same cloud module method invocation or HTTP(S) reply callback in which
** it was allocated using sqlite3_bcv_job_request(). The results of calling
** it at any other time are undefined.
*/
void sqlite3_bcv_request_set_header(sqlite3_bcv_request*, const char *zHdr);

/*
** Set the body of an HTTP(S) request. This is only meaningful for PUT
** requests. Parameter aData must point to a buffer nData bytes in size
** containing the data to use as the request body. This function takes a
** copy of the buffer if necessary, the cloud storage module implementation
** does not have to ensure that it remains valid after the call to
** this function.
**
** This function may only be called on a request handle from within the
** same cloud module method invocation or HTTP(S) reply callback in which
** it was allocated using sqlite3_bcv_job_request(). The results of calling
** it at any other time are undefined.
*/
void sqlite3_bcv_request_set_body(
    sqlite3_bcv_request*, const unsigned char *aData, int nData
);

/*
** This function may be called on a request handle within its HTTP(S) 
** reply callback to determine success or failure of the request. If
** the request was successful (a reply with HTTP(S) status 2** was
** received), SQLITE_OK is returned. Otherwise, if an error occurs,
** an error code, which may be either a non-extended SQLite error code
** or an HTTP error status (a value between 400 and 599, inclusive).
**
** If parameter pzStatus is not NULL, (*pzStatus) may be set to point to
** a buffer containing a nul-terminated English language status message.
**
** This function may only be called on a request handle from within its
** HTTP(S) reply callback (the callback specified as part of the
** sqlite3_bcv_job_request() call that created it). The results of calling 
** it at any other time are undefined.
*/
int sqlite3_bcv_request_status(sqlite3_bcv_request*, const char **pzStatus);

/*
** This function may be called on a request handle within its HTTP(S) 
** reply callback to extract an HTTP(S) header from the reply. Parameter
** zHdr must point to a nul-terminated string containing the name of the
** requested header (e.g. "Content-Type"). If the HTTP(S) reply did not
** contain the requested header, NULL is returned. Otherwise, a pointer
** to a nul-terminated string containing the header value is returned (e.g.
** "application/octet-stream").
**
** Header names are matched in a case-independent fashion.
**
** This function may only be called on a request handle from within its
** HTTP(S) reply callback (the callback specified as part of the
** sqlite3_bcv_job_request() call that created it). The results of calling 
** it at any other time are undefined.
*/
const char *sqlite3_bcv_request_header(sqlite3_bcv_request*, const char *zHdr);

/*
** This function may be called on a request handle within its HTTP(S) 
** reply callback to obtain access to data contained in the body of the
** HTTP(S) reply. A pointer to a buffer containing the entire reply body
** is returned, and output variable (*pn) set to the size of the reply
** body in bytes.
**
** This function may only be called on a request handle from within its
** HTTP(S) reply callback (the callback specified as part of the
** sqlite3_bcv_job_request() call that created it). The results of calling 
** it at any other time are undefined.
*/
const unsigned char *sqlite3_bcv_request_body(sqlite3_bcv_request*, int *pn);

/*
** This function is used by xFetch() and xList() jobs to return data to BCV.
** The data to return is in a buffer n bytes in size indicated by parameter
** p.
**
** xFetch() jobs must call this function once, passing a buffer containing
** the contents of the downloaded object.
**
** xList() jobs call this function once for each entry in the returned list.
** In this case, the buffer should contain the name of the entry with no
** nul-terminator. Or, if a negative value is passed for parameter n, the
** entry name is assumed to consist of all bytes up until the first
** nul-terminator in the buffer.
*/
void sqlite3_bcv_job_result(sqlite3_bcv_job*, const unsigned char *p, int n);

/*
** This function is used by xFetch() and xPut() jobs to return the unique
** version id ("e-tag") for the object just downloaded or uploaded. Argument
** zETag must point to a nul-terminated string containing the version id
** value.
*/
void sqlite3_bcv_job_etag(sqlite3_bcv_job*, const char *zETag);

/*
** This function may be used by any job type to return an error to BCV.
** Parameter eCode must be passed either a non-extended SQLite error code,
** or an HTTP(S) status indicating an error (a value between 400 and 599,
** inclusive). Parameter zErr may either be NULL, or else point to a buffer
** containing a nul-terminated English language error message.
*/
void sqlite3_bcv_job_error(sqlite3_bcv_job*, int eCode, const char *zErr);

/*
** Return the prefix, if any, for list operations.
*/
const char *sqlite3_bcv_job_prefix(sqlite3_bcv_job*);

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
#endif /* ifndef _BCVMODULE_H */

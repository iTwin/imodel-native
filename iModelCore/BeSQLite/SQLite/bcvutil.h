/*
** 2020-05-12
**
******************************************************************************
**
** This file contains declarations for the public interface to code in 
** bcvutil.c. This API allows applications to upload, download, duplicate and
** delete CBS cloud storage databases. It is also possible to create
** or destroy CBS manifests and cloud storage containers/buckets.
**
** In general API users proceed as follows:
**
**   1. A (sqlite3_bcv*) handle is created using sqlite3_bcv_open().
**
**   2. The handle is configured in various ways using zero or more
**      calls to sqlite3_bcv_config().
**
**   3. Operations are performed on cloud storage using one or more
**      calls to the following APIs:
**
**        sqlite3_bcv_create()    // create a new container/bucket
**        sqlite3_bcv_destroy()   // delete an entire container/bucket
**        sqlite3_bcv_upload()    // upload a database to cloud storage
**        sqlite3_bcv_download()  // download a database from cloud storage
**        sqlite3_bcv_copy()      // duplicate a database within cloud storage
**        sqlite3_bcv_delete()    // delete a single database from cloud storage
**        sqlite3_bcv_cleanup()   // delete unused blocks from cloud storage
**
**   4. The (sqlite3_bcv*) handle is deleted, freeing all allocated resources,
**      by calling sqlite3_bcv_close().
**
** The calls made in steps 2 and 3 above may be interwoven - a handle may
** be configured or reconfigured at any time, not only immediately after it
** is created.
**
** If an error occurs, the sqlite3_bcv_errmsg() API may in some cases be used
** to obtain an English language error message.
**
** Calls that return an integer error code may return either a non-extended
** SQLite error code (rc < 400) or an HTTP error code (rc >= 400) if an 
** HTTP error occurs.
*/

#ifndef _BCVUTIL_H
#define _BCVUTIL_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sqlite3_bcv sqlite3_bcv;

/*
** Allocate a new sqlite3_bcv handle. 
**
** The first argument is passed the name of the module to use. This may
** be either a simple module name (e.g. "google") or a module name with
** URI style parameters (e.g. "azure?emulator=127.0.0.1:10000&sas=1").
**
** The second parameter is passed the user or account name that will be
** used to access the cloud storage container. The third parameter, zAuth,
** must be passed a nul-terminated string containing the authentication
** information to use when accessing the remote resource. Refer to 
** documentation for the specific cloud storage module in use for exactly
** how this value, and the user name value, are used.
**
** The fourth parameter, zContainer, is passed the name of the remote
** container to operate on.
**
** If successful, SQLITE_OK is returned and output variable (*ppOut) set
** to contain a pointer to the new handle. Or, if an error occurs, an
** SQLite error code is returned. In this case (*ppOut) is still set to
** point to a new handle, but passing it to any function except
** sqlite3_bcv_close() and sqlite3_bcv_errmsg() is a no-op that returns
** a copy of the error code returned by sqlite3_bcv_open(). An application
** may use sqlite3_bcv_errmsg() to obtain an English language error message
** before closing the handle in this case.
*/
int sqlite3_bcv_open(
  const char *zModule,
  const char *zUser,              /* Cloud storage user name */
  const char *zAuth,              /* Key or SAS used for cloud storage auth. */
  const char *zContainer,         /* Cloud storage container/bucket */
  sqlite3_bcv **ppOut             /* OUT: New object */
);

/*
** Configure the bcv handle passed as the first argument.
*/
int sqlite3_bcv_config(sqlite3_bcv*, int eOp, ...);

/*
** SQLITE_BCVCONFIG_VERBOSE:
**   A single trailing int is required by this configuration option. If
**   non-zero, the handle is configured to use CURLOPT_VERBOSE with all
**   http/https requests. If zero, it is configured to not use 
**   CURLOPT_VERBOSE. The default is not to use CURLOPT_VERBOSE.
**
** SQLITE_BCVCONFIG_PROGRESS:
**   Configure a callback to be issued after each block is 
**   uploaded/downloaded within sqlite3_bcv_upload()/download() API calls.
**   This configuration option requires two trailing arguments, a (void*)
**   and a function with the following signature:
**
**     int xProgress(void *pCtx, sqlite3_int64 nDone, sqlite3_int64 nTotal);
**
**   Within a call to sqlite3_bcv_upload(), the callback is invoked after
**   each block has been uploaded. The three arguments are a copy of the
**   (void*) passed to sqlite3_bcv_config(), the number of bytes uploaded
**   by the current call so far, and the total number of bytes that will
**   be uploaded, assuming the call is successful, before returning.
**   For a sqlite3_bcv_download() call, the callback is invoked after each
**   block has been downloaded. The three arguments are similar to those 
**   for _upload() - a copy of the (void*), the number of bytes downloaded 
**   so far, and the total bytes that will be downloaded.
**
**   If the progress callback returns 0, the operation continues. If it
**   returns a non-zero value, the operation is abandoned and error code
**   SQLITE_ABORT returned to the caller.
**
** SQLITE_BCVCONFIG_LOG:
**   Configure a callback to be invoked each time an HTTP(S) request is
**   sent, and each time a reply is received. The trailing arguments for
**   this option are a (void*) and a function with the following signature:
**
**     void xLog(void *pCtx, const char*);
**
**   Each time the callback is invoked, a copy of the first trailing 
**   argument passed to the sqlite3_bcv_config() call is passed as its
**   first parameter. The second parameter points to a nul-terminated 
**   string containing a human-readable message suitable for outputing
**   to a log file.
**
** SQLITE_BCVCONFIG_LOGLEVEL:
**   Set the verbosity level of the debugging messages output to
**   the SQLITE_BCVCONFIG_LOG callback. The default (and recommended)
**   level is 0. Higher values produce more logging output.
**
** SQLITE_BCVCONFIG_NREQUEST:
**   This option requires a single trailing argument of type int. It
**   sets the maximum number of simultaneous HTTPS requests for an
**   upload or download operation. Values are clipped to between 1
**   and 32. The default value is 1.
**
** SQLITE_BCVCONFIG_TESTNOKV:
**   Internal option used for testing only.
**
** SQLITE_BCVCONFIG_HTTPTIMEOUT:
**   Sets the number of seconds before an HTTPS request is deemed
**   to have timed out. Default is 600.
*/
#define SQLITE_BCVCONFIG_VERBOSE     1      /* (int) */
#define SQLITE_BCVCONFIG_PROGRESS    2      /* (void*,xProgress) */
#define SQLITE_BCVCONFIG_LOG         3      /* (void*,xLog) */
#define SQLITE_BCVCONFIG_NREQUEST    4      /* (int) */
#define SQLITE_BCVCONFIG_LOGLEVEL    5      /* (int) */
#define SQLITE_BCVCONFIG_TESTNOKV    6      /* (int) */
#define SQLITE_BCVCONFIG_HTTPTIMEOUT 7      /* (int) */

/*
** Delete an sqlite3_bcv handle obtained via an earlier call to
** sqlite3_bcv_open().
*/
void sqlite3_bcv_close(sqlite3_bcv*);

/*
** Return a copy of the error code returned by the most recent call to
** sqlite_bcv_create(), destroy(), upload(), download(), copy(), delete()
** or open() on the handle passed as the only argument.
*/
int sqlite3_bcv_errcode(sqlite3_bcv*);

/*
** If the most recent call to sqlite3_bcv_create(), destroy(), upload(),
** download(), copy(), delete() or open() encountered an error, return an
** English language error message describing the error that occurred. Or, if
** the most recent API call did not return an error, return NULL.
*/
const char *sqlite3_bcv_errmsg(sqlite3_bcv *p);

/*
** Upload a database to the cloud storage container. Argument zLocal 
** identifies the database to upload from the local file-system. Argument
** zRemote is the name the database will take within cloud storage.
**
** If the operation is successful, SQLITE_OK is returned. Or, if
** an HTTPS request made to the cloud storage system fails, an HTTP 
** response code is returned. Or, if some other error occurs (for example, 
** a failed memory allocation), a non-extended SQLite error code is 
** returned. All HTTP response codes that may be returned are greater
** than or equal to 400. All non-extended SQLite error codes are positive
** integers smaller than 256.
*/ 
int sqlite3_bcv_upload(
  sqlite3_bcv *p,
  const char *zLocal,
  const char *zRemote
);

/*
** Download a database from the cloud storage container. Parameter zRemote
** is the name of the database within cloud storage, and zLocal is the
** local path at which the downloaded databse will be written.
**
** If the operation is successful, SQLITE_OK is returned. Or, if
** an HTTPS request made to the cloud storage system fails, an HTTP 
** response code is returned. Or, if some other error occurs (for example, 
** a failed memory allocation), a non-extended SQLite error code is 
** returned. All HTTP response codes that may be returned are greater
** than or equal to 400. All non-extended SQLite error codes are positive
** integers smaller than 256.
*/
int sqlite3_bcv_download(
  sqlite3_bcv *p,
  const char *zRemote,
  const char *zLocal
);

/*
** Create a copy of an existing cloud storage database within its container.
**
** If the operation is successful, SQLITE_OK is returned. Or, if
** an HTTPS request made to the cloud storage system fails, an HTTP 
** response code is returned. Or, if some other error occurs (for example, 
** a failed memory allocation), a non-extended SQLite error code is 
** returned. All HTTP response codes that may be returned are greater
** than or equal to 400. All non-extended SQLite error codes are positive
** integers smaller than 256.
*/
int sqlite3_bcv_copy(
  sqlite3_bcv *p,
  const char *zFrom,
  const char *zTo
);

/*
** Delete a database from cloud storage.
**
** If the operation is successful, SQLITE_OK is returned. Or, if
** an HTTPS request made to the cloud storage system fails, an HTTP 
** response code is returned. Or, if some other error occurs (for example, 
** a failed memory allocation), a non-extended SQLite error code is 
** returned. All HTTP response codes that may be returned are greater
** than or equal to 400. All non-extended SQLite error codes are positive
** integers smaller than 256.
*/
int sqlite3_bcv_delete(
  sqlite3_bcv *p,
  const char *zDelete
);

/*
** Create a new cloud storage container and manifest file. If the container
** already exists, it is not an error, but any existing manifest file is
** simply clobbered.
**
** If the operation is successful, SQLITE_OK is returned. Or, if
** an HTTPS request made to the cloud storage system fails, an HTTP 
** response code is returned. Or, if some other error occurs (for example, 
** a failed memory allocation), a non-extended SQLite error code is 
** returned. All HTTP response codes that may be returned are greater
** than or equal to 400. All non-extended SQLite error codes are positive
** integers smaller than 256.
*/
int sqlite3_bcv_create(sqlite3_bcv *p, int szName, int szBlock);

/*
** Delete the entire cloud storage container or bucket.
**
** If the operation is successful, SQLITE_OK is returned. Or, if
** an HTTPS request made to the cloud storage system fails, an HTTP 
** response code is returned. Or, if some other error occurs (for example, 
** a failed memory allocation), a non-extended SQLite error code is 
** returned. All HTTP response codes that may be returned are greater
** than or equal to 400. All non-extended SQLite error codes are positive
** integers smaller than 256.
*/
int sqlite3_bcv_destroy(sqlite3_bcv *p);

/*
** When a remote database is written, a subset of its blocks are replaced
** by new versions, sometimes leaving the originals unused by any database.
** In this case, they are not deleted immediately. Instead, they are 
** marked as unused, ready for deletion at some later time. Calling this 
** routine deletes all blocks in the named remote container that were
** marked as unused nSecond seconds or more ago. Calling this routine with 
** the nSecond parameter set to 0 or less deletes all unused blocks.
**
** If the cloud storage container is being written simultaneously by some
** other client anywhere on the network, this function may return HTTPS
** error 412 (pre-condition failed).
**
** If the operation is successful, SQLITE_OK is returned. Or, if
** an HTTPS request made to the cloud storage system fails, an HTTP 
** response code is returned. Or, if some other error occurs (for example, 
** a failed memory allocation), a non-extended SQLite error code is 
** returned. All HTTP response codes that may be returned are greater
** than or equal to 400. All non-extended SQLite error codes are positive
** integers smaller than 256.
*/
int sqlite3_bcv_cleanup(sqlite3_bcv *p, int nSecond);

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
#endif /* ifndef _BCVUTIL_H */


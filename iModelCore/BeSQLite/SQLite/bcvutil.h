/*
** 2020-05-12
**
******************************************************************************
**
** This file contains declarations for the public interface to code in 
** bcvutil.c. This API allows you to upload, download, duplicate and
** delete BCV cloud storage databases. It is also possible to create
** or destroy BCV manifests and cloud storage containers/buckets.
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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sqlite3_bcv sqlite3_bcv;

/*
** Allocate a new sqlite3_bcv handle. 
**
** The interpretation of arguments zUser and zKey depend on the type
** of cloud storage specified by argument eType. As follows:
**
** SQLITE_BCV_AZURE:
**   Argument zUser is passed the user name (e.g. "devstoreaccount1") and
**   zKey the account access key.
**
** SQLITE_BCV_AZURE_SAS:
**   Argument zUser is passed the user name (e.g. "devstoreaccount1") and
**   zKey a shared-access-signature token providing sufficient permissions
**   for the operations that the sqlite3_bcv handle will be used for.
**
** SQLITE_BCV_AZURE_EMU:
**   In this case argument zUser should be passed the address and port number
**   on which the Azure emulator can be contacted (e.g. "127.0.0.1:10000").
**   Argument zKey is ignored (as the account and access key for the emulator
**   are well-known).
**
** SQLITE_BCV_AZURE_EMU_SAS:
**   Argument zUser is as for SQLITE_BCV_AZURE_EMU. Argument zKey must be an
**   SAS token that can be used with account "devstoreaccount1" to access
**   emulator resources.
**
** SQLITE_BCV_GOOGLE:
**   Argument zUser is passed the name of the project that owns (or will own)
**   the named bucket. It is only required if the handle will be used to
**   create the bucket, not if it already exists, in which case it may be NULL.
**   zKey must be passed a Google Cloud access token providing sufficient
**   access for the operations that will be requested on the sqlite3_bcv
**   handle.
*/
int sqlite3_bcv_open(
  int eType,                      /* SQLITE_BCV_* constant */
  const char *zUser,              /* Cloud storage user name */
  const char *zKey,               /* Key or SAS used for cloud storage auth. */
  const char *zContainer,         /* Cloud storage container/bucket */
  sqlite3_bcv **ppOut             /* OUT: New object */
);

/*
** Candidate values for the first argument to sqlite3_bcv_new().
*/
#define SQLITE_BCV_AZURE         1
#define SQLITE_BCV_AZURE_SAS     2
#define SQLITE_BCV_AZURE_EMU     3
#define SQLITE_BCV_AZURE_EMU_SAS 4
#define SQLITE_BCV_GOOGLE        5

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
*/
#define SQLITE_BCVCONFIG_VERBOSE 1          /* (int) */
#define SQLITE_BCVCONFIG_PROGRESS 2         /* (void*,xProgress) */

/*
** Delete an sqlite3_bcv handle obtained via an earlier call to
** sqlite3_bcv_new().
*/
void sqlite3_bcv_close(sqlite3_bcv*);

/*
** Return a copy of the error code returned by the most recent call to
** sqlite_bcv_create(), destroy(), upload(), download(), copy or delete()
** on the handle passed as the only argument. Or SQLITE_OK if no such
** call has ever been made on the handle.
*/
int sqlite3_bcv_errcode(sqlite3_bcv*);

/*
** If the most recent call to sqlite3_bcv_upload(), download(), copy() or
** delete() encountered an error, return an English language error message
** describing the error that occurred. Or, if the most recent API call did
** not return an error, or if no API calls have been made on the handle
** since it was created, return NULL.
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


#ifdef __cplusplus
} /* end of the extern "C" block */
#endif


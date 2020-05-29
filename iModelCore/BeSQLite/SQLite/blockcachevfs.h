/*
** 2020-04-01
**
******************************************************************************
**
** Public interface to code in blockcachevfs.c.
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
** Register the blockcachevfs VFS with SQLite. If the only argument is 
** non-zero, the blockcachevfs VFS is made the new default VFS. Return
** a pointer to a buffer containing the name of the new VFS if successful,
** or NULL otherwise. The buffer belongs to this module - it must not be
** freed or overwritten by the caller.
**
** The second and subsequent calls, if any, to this function are no-ops
** that return a pointer to the name buffer. Once the VFS has been 
** registered it cannot be reregistered or unregistered.
*/
const char *sqlite3_bcv_register(int bDefault);

/*
** Register a global SAS token callback, replacing the current SAS token
** callback, if any. To unregister the SAS token callback completely,
** pass a NULL pointer as the second argument to this routine.
**
** This interface is not threadsafe. It must not be called when any other
** threads may be using the blockcachevfs VFS or any API in this file.
**
** If successful, SQLITE_OK is returned. If an error occurs, an SQLite
** error code. It is not currently possible for an error to occur.
**
** Once registered, the callback is invoked under two circumstances -
** when a database is opened with the URI parameter "bcv_sas=1", and when
** a connected daemon process returns an error to the client indicating
** that the current SAS token has expired. The blockcachevfs daemon process
** assumes that an SAS token has expired if any operation using it returns
** HTTP code 403 ("The server understood the request, but is refusing to
** authorize it").
**
** When invoked, the first argument passed to the callback is a copy of
** the first argument passed to this function. The second argument is
** the name of the cloud storage system for which an SAS token is requested,
** this is currently always "azure". The third argument is the name of
** the cloud storage account (e.g. "devstoreaccount1") and the fourth
** the name of the container for which an SAS token is requested. The 
** callback function must attempt to generate the required SAS token and 
** store it (along with a nul-terminator) into a buffer obtained from 
** sqlite3_malloc() or compatible. If successful, it should set output
** variable (*pzSasToken) to point to the new buffer, and (*pbReadonly)
** to 1 or 0, depending on whether or not the SAS token provides read-only
** (*pbReadonly=1) or read-write (*pbReadonly=0) access. SQLITE_OK should
** be returned in this case, and the caller will take responsibility
** for eventually freeing the (*pzSasToken) buffer. Or, if an error occurs,
** the callback should return an SQLite error code. No assumptions are
** made about the final values of (*pzSasToken) and (*pbReadonly) in this
** case.
*/
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
);

/*
** Send a message to a local daemon process to attach a new container.
**
** Parameter zDir must be passed a path identifying the directory used by
** a running daemon process (i.e. a path identifying the same directory
** as the -directory option passed to the daemon did). zContainer is the
** name of the cloud storage container or bucket to attach, and zSas
** points to a buffer containing an Azure SAS token or Google Cloud Access
** token, if any, for the daemon process to use.
**
** Parameter flags contains a combination of SQLITE_BCVATTACH_X flags. See
** comments above the definition of the flags themselves for a description
** of the effect of each.
**
** If successful, SQLITE_OK (0) is returned and output parameter (*pzErr) 
** is set to NULL. A call to this function is considered to be successful 
** even if the specified container is already attached. If an error occurs,
** the output parameter (*pzErr) may be set to point to a buffer containing 
** an English language error message. It is the responsibility of the 
** caller to eventually free such an error message buffer using 
** sqlite3_free(). In this case, the error code returned may either be
** an HTTP error status (a value greater or equal to 400), an SQLite error
** code (a value greater than zero but smaller than 400), or one of the
** SQLITE_BCV_CANTCONNECT[123] error codes defined below.
**
** If an error occurs within the local process, the error code is one of:
**
**   SQLITE_MISUSE - indicating that either the zDir or zContainer parameter
**     was passed a NULL value,
**
**   SQLITE_NOMEM - indicating that a memory allocation failed, or
**
**   SQLITE_BCV_CANTCONNECT[123] - indicating that there was a problem 
**     connecting to the daemon process. See below for details.
**
** If the daemon process is successfully contacted, but an error occurs
** within one of the requests made to cloud storage, then an HTTP error
** code is returned. Or, if some other error occurs within the daemon 
** process, an SQLite error code is returned. This will usually be 
** SQLITE_NOMEM or SQLITE_IOERR, indicating that an allocation or attempt
** to read or write to the local file-system failed, but may also be
** another SQLite error code, indicating some other error has occurred.
*/
int sqlite3_bcv_attach(
  const char *zDir,               /* Directory of daemon process to contact */
  const char *zContainer,         /* Container to attach */
  const char *zSas,               /* SAS token (if any) */
  int flags,                      /* SQLITE_BCVATTACH_X flags */
  char **pzErr                    /* OUT: error message (if any) */
);

/*
** The following values may be returned by sqlite3_bcv_attach() or
** sqlite3_bcv_detach(). They should be interpreted as follows:
**
**   SQLITE_BCV_CANTCONNECT1:
**     The specified directory does not exist, or is not populated by
**     the files expected of a daemon process's working directory.
**
**   SQLITE_BCV_CANTCONNECT2:
**     Establishing the localhost socket connection to the daemon process
**     failed. 
**
**   SQLITE_BCV_CANTCONNECT2:
**     The connection to the daemon process was established, but an error
**     occured at the socket level while exchanging messages with it.
*/
#define SQLITE_BCV_CANTCONNECT1   (-1)
#define SQLITE_BCV_CANTCONNECT2   (-2)
#define SQLITE_BCV_CANTCONNECT3   (-3)

/*
** Candidate flags for the fourth argument to sqlite3_bcv_attach().
**
** SQLITE_BCVATTACH_READONLY:
**   If set, the container is attached in read-only mode. If clear, it
**   is attached in read/write mode. 
**
** SQLITE_BCVATTACH_POLL:
**   If this flag is clear and the specified container is already attached 
**   to the daemon process when sqlite3_bcv_attach() is called, the call 
**   returns SQLITE_OK without delaying. Or, if the specified container is in
**   the process of being attached but the initial download of the manifest 
**   file is not complete, the call waits until the manifest had been 
**   downloaded before returning. 
**
**   Setting this flag changes the behaviour so that if the specified
**   container has already been attached when sqlite3_bcv_attach() is called,
**   it causes the daemon process to immediately poll cloud storage for an
**   updated manifest file. The sqlite3_bcv_attach() call does not return
**   until after the poll operation has finished. Similarly, if the daemon
**   is waiting for the initial download of the manifest file when
**   sqlite3_bcv_attach() is invoked, it is caused to poll cloud storage
**   immediately after the initial download is finished. Again, the call to
**   sqlite3_bcv_attach() does not return until after the poll operation
**   is finished.
*/
#define SQLITE_BCVATTACH_READONLY 0x0001
#define SQLITE_BCVATTACH_POLL     0x0002

/*
** Send a message to a local daemon process to detach a container.
**
** If successful, SQLITE_OK is returned and output parameter (*pzErr) is
** set to NULL. Of, if an error occurs, output parameter (*pzErr) may be set
** to point to a buffer containing an English language error message. In this
** case, it is the responsibility of the caller to eventually free the error 
** message buffer using sqlite3_free().
**
** If an error occurs, sqlite3_bcv_detach() returns an error code that should
** be interpreted as for sqlite3_bcv_attach().
*/
int sqlite3_bcv_detach(
  const char *zDir,               /* Directory of daemon process to contact */
  const char *zContainer,         /* Container to detach */
  char **pzErr                    /* OUT: error message (if any) */
);

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif


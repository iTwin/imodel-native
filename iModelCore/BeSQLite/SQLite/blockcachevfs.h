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

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif


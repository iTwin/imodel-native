/*
** 2020-04-01
**
******************************************************************************
** Public interface used by Cloud Backed SQLite (CBS) database clients.
*/

/*
** OVERVIEW
**
** This module provides a VFS that provides access to databases stored in
** Cloud-Backed-SQLite format in remote cloud containers. The VFS may operate
** in two modes:
**
**   1) Daemonless mode. This mode provides read/write access to remote
**      databases.
**
**   2) Daemon mode. This mode provides read-only access to remote databases,
**      but allows database clients from multiple processes to share a
**      single cache directory. This mode requires that the daemon process
**      be started before any VFS objects are created.
**
** In general, the same API is used to interface with a daemon or daemonless
** mode VFS. Please refer to the website documentation for more information.
**
** APPLICATION PROGRAMMING
**
** Creating and destroying VFS objects:
**
**   The application creates a Cloud Backed SQLite VFS object using the
**   sqlite3_bcvfs_create() API. Parameters to this call specify the name of
**   the VFS to create, and the path to a directory that the VFS will use to
**   store its various files. The directory should either be completely empty,
**   or a directory used previously by a Cloud Backed SQLite VFS, or a directory
**   that has a running CBS daemon using it. If the directory has a daemon
*    running in it, then the new VFS automatically connects to
**   that daemon and runs in daemon mode. Otherwise, if there is no daemon,
**   then the created VFS object runs in daemonless mode.
**
**   If the specified directory does not have a running CBS daemon, but was
**   used previously, then the new VFS object is initialized based on the state
**   of the directory - meaning that all containers that were attached when 
**   the previous VFS was shutdown are automatically reattached.
**
**   As well as creating a VFS object that may be used by database clients,
**   a successful call to sqlite3_bcvfs_create() returns an opaque handle
**   (type sqlite3_bcvfs*) that may be used to configure the VFS, attach
**   and detach containers, and other operations described below.
**
**   Once an application has finished with it and all database clients have
**   disconnected, the application may destroy the CBS VFS object
**   and reclaim all associated resources using sqlite3_bcvfs_destroy().
**
**   An application may create any number of VFS objects, but usually one
**   is sufficient. A single VFS object may be used to access databases
**   stored in multiple cloud containers across multiple providers.
**
** Configuring VFS objects:
**
**   Once a VFS object has been created, it must be configured. A VFS object
**   may be configured or reconfigured at any time, but most applications
**   will find it sufficient to configure the object once immediately after
**   it has been created.
**
**   Almost all applications will need to configure the VFS object with
**   an authentication callback using sqlite3_bcvfs_auth_callback(). This
**   callback is the only way the authentication data (e.g. an Azure SAS 
**   string or Google Cloud access token) may be provided to cloud storage
**   modules that require it.
**
**   Application logging (i.e. for debugging) is configure using
**   sqlite3_bcvfs_log_callback().
**
**   Various integer parameters may also be configured using
**   sqlite3_bcvfs_config(). Notably, the SQLITE_BCV_CACHESIZE parameter
**   may be usd with sqlite3_bcvfs_config() to configure the maximum size of
**   the local block cache in bytes.
**
** Attaching and detaching containers:
**
**   Before databases may be accessed, the cloud container that they reside
**   in must be "attached" to the VFS. Containers are attached using the
**   sqlite3_bcvfs_attach() API function. To attach a container, the
**   application specifies cloud storage system (e.g. "azure" or "google"),
**   the user-name used to connect to cloud storage, the name of the cloud
**   storage container as used by the remote system, and an alias that will
**   be used to refer to the container locally.
**
**   Unused containers may be detached from a VFS using sqlite3_bcvfs_detach().
**
** Opening and accessing databases:
**
**   Once one or more containers have been attached to the VFS, databases
**   contained within them may be opened and accessed by SQLite database
**   clients. This is done by specifying a path consisting of the container
**   alias (local name) and the database name, as follows:
**
**      /ALIAS/DATABASE
**
**   and using either an SQLite URI filename or the sqlite3_open_v[23]()
**   interface to specify the VFS name.
**
**   For example, with all error handling omitted, to create a new VFS
**   that uses directory "dirname" to store its files, then to use it
**   to access database "mydb.db" in container "mycont" belonging to 
**   Azure user "myuser":
**
**     sqlite3_bcvfs *pFs;
**     sqlite3 *db;
**
**     sqlite3_bcvfs_create("dirname", "myvfs", &pFs);
**     // ... configure VFS ...
**     sqlite3_bcvfs_attach(pFs, "azure", "myuser", "mycont", "myalias", 0, 0);
**     sqlite3_open_v2("/myalias/mydb.db", &db, SQLITE_OPEN_READWRITE, "myvfs");
**
**   Assuming URI filenames are enabled (SQLITE_CONFIG_URI), the following
**   may be used in place of the sqlite3_open_v2() call above:
**
**     sqlite3_open("file:/myalias/mydb.db?vfs=myvfs", &db);
**
** Poll and Upload operations:
**
**   The sqlite3_bcvfs_poll() and sqlite3_bcvfs_upload() and
**   APIs are used to perform "poll" and "upload" operations on a specified 
**   attached container. Also available are the "PRAGMA bcv_poll"
**   and "PRAGMA bcv_upload" SQL commands.
**
**   Poll operations: When a container is first attached, its "manifest" file
**   is downloaded from cloud storage. The manifest file contains the list
**   of databases in the container and the list of immutable blocks that
**   each database consists of. By default, this manifest is used until the
**   container is detached, only being updated according to local "upload"
**   operations (see below). A "poll" operation downloads a new copy of the
**   manifest from cloud storage, making any changes to the list of
**   available databases and their contents visible to local clients.
**
**   Upload operations: After a database has been modified locally, using the
**   usual SQLite interfaces (i.e. SQL statements), the changes may be uploaded
**   to cloud storage. An "upload" operation uploads all locally modified
**   databases that are part of the specified container.
**
**   When databases modifications are uploaded, or databases are deleted
**   from cloud storage containers altogether, old, unused blocks are left 
**   in cloud storage. The system allows these blocks to linger in case 
**   other clients are still using them instead of deleting them immediately.
**   To actually delete old, unused, blocks from cloud storage, see the
**   sqlite3_bcv_cleanup() API (in bcvutil.h).
**
** VIRTUAL TABLE INTERFACE
**
**   The sqlite3_bcvfs_register_vtab() API is used to register three read-only 
**   eponymous virtual tables, "bcv_database", "bcv_container" and "bcv_block",
**   and one read-write table, "bcv_kv". If the main database of the database
**   handle that these are registered with is open on a CBS database,
**   the three read-only tables provide information regarding the current state 
**   of the VFS object. The "bcv_kv" table provides read-write access to a
**   key-value store stored in the cloud container that may be useful for
**   advisory locks or other communication between remote database clients.
**
**   Virtual tables bcv_database and bcv_container are available to both
**   daemon and daemonless mode VFS clients, but bcv_kv is only available
**   in daemonless mode.
**
** The bcv_container table:
**
**   The "bcv_container" table contains one row for each container attached
**   to the VFS. It has the equivalent of the following schema:
**
**     CREATE TABLE bcv_container(
**       name      TEXT,          -- local name (alias) of container 
**       storage   TEXT,          -- cloud storage system (e.g. "azure")
**       user      TEXT,          -- cloud storage username
**       container TEXT,          -- container name in cloud storage
**       ncleanup  INTEGER        -- number of blocks eligible for cleanup
**     )
**
**   The "ncleanup" column usually contains the number of blocks that will 
**   be deleted from cloud storage if a cleanup operation (see the 
**   sqlite3_bcv_cleanup() API)is run on the container. However, if
**   errors or client crashes have occurred while uploading changes to
**   cloud storage, then there may be extra unused blocks left in the
**   cloud storage container. In this case those blocks will be deleted
**   by the next cleanup operation, but are not included in the value
**   stored in the "ncleanup" column of the "bcv_container" table.
**
** The bcv_database table:
**
**   The "bcv_database" table contains one row for each database in each
**   attached container. It has the equivalent of the following schema:
**
**     CREATE TABLE bcv_database(
**       container TEXT,          -- local name (alias) of container
**       database  TEXT,          -- name of database
**       nblock INTEGER,          -- total number of blocks in database
**       ncache INTEGER,          -- number of blocks in cache
**       ndirty INTEGER,          -- number of dirty blocks in cache
**       walfile BOOLEAN,         -- true if transactions in local wal file
**       state TEXT               -- state of database (see below)
**     )
**
**   The "state" column usually contains an empty string. There are two
**   exceptions:
** 
**      * If the database has been created using sqlite3_bcvfs_copy() but
**        not yet uploaded, then this column contains the text 'copied'.
**
**      * If the database has been deleted using sqlite3_bcvfs_delete() but
**        the delete operation has not yet uploaded, then this column contains 
**        the text 'deleted'.
**
** The bcv_block table:
**
**   The "bcv_block" table contains one row for each block in each database 
**   in each attached container. It has the equivalent of the following 
**   schema:
**
**     CREATE TABLE bcv_block(
**       container TEXT,          -- local name (alias) of container
**       database  TEXT,          -- name of database
**       blockno   INTEGER,       -- block number within database
**       blockid   BLOB,          -- block id (or NULL)
**       cache     BOOLEAN,       -- true if block is in cache
**       dirty     BOOLEAN        -- true if block is dirty
**     ) 
**
**   The first three columns, "container", "database" and "blockno", identify
**   a block within a specific database. Blocks are numbered starting from
**   zero.
**
**   The "blockid" column usually contains a unique block identifier. Or, if
**   the block in question has been appended to the database since it was
**   last uploaded, this column contains a NULL value. The boolean "cache"
**   column is true if the block is currently cached locally, and "dirty"
**   is true if the block is dirty and a new version needs to be uploaded.
**
** The bcv_kv table:
**
**   The schema of the "bcv_kv" table is:
**
**     CREATE TABLE bcv_kv(
**       name TEXT PRIMARY KEY,   -- Key value 
**       value                    -- Associated payload value
**     )
**
**   The contents of the bcv_kv table is stored as a separate file (an
**   SQLite database) in the cloud container - "bcv_kv.bcv". The table
**   is empty when the container is first created.
**
**   The bcv_kv table implements transaction-like properties. As follows:
**
**     * The first time in a database transaction that the bcv_kv table 
**       is read or written, the file is downloaded from cloud storage and
**       cached for the duration of the transaction. All user queries -
**       read and write - are executed against this cached version of the 
**       bcv_kv table. This ensures that each transaction sees a consistent 
**       version of the table contents.
**
**     * When a read/write transaction is committed, a new version of the
**       bcv_kv table data is uploaded to cloud storage. At this point,
**       if it is found that the file has been modified within cloud storage
**       since it was downloaded at the beginning of the transaction (because
**       some other client wrote to it), the commit fails and the transaction
**       is rolled back. The extended error code in this case is 
**       SQLITE_BUSY_SNAPSHOT.
**
** Virtual tables without a database:
**
**   Sometimes, it may be convenient to access the virtual table interface
**   without having to identify and open a cloud container database. To
**   support this, paths of the forms:
**
**      /
**      /CONTAINER
**
**   may be opened with bcvfs VFSs, where CONTAINER is the local name (alias)
**   of an attached container. The databases opened by either of these two
**   forms of path are empty. Attempting to create any new tables within them
**   fails. However, if sqlite3_bcvfs_register_vtab() is called on the
**   database handle, they may be used to access the bcvfs virtual table
**   interface.
**
**   If the path opened is simply "/", then only the "bcv_database" and
**   "bcv_container" tables are available. If the path is of the form
**   "/CONTAINER", then all three virtual tables are available.
*/

#include "sqlite3.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** Opaque Cloud Backed SQLite VFS handle type.
*/
typedef struct sqlite3_bcvfs sqlite3_bcvfs;

/*
** Create a new Cloud Backed SQLite VFS in the current process. Parameter zDir
** must point to a nul-terminated buffer containing the path (absolute
** or relative) of the directory to use for the various files created
** by Cloud Backed SQLite. Parameter zName should point to a buffer containing 
** the name of the new VFS.
**
** Unless there is a daemon process running in the directory, this routine
** requires exclusive access to the directory. If it is already in use by
** some other Cloud Backed SQLite VFS, in this or another process, this routine 
** fails with SQLITE_BUSY.
*/
int sqlite3_bcvfs_create(
  const char *zDir,               /* Directory to use */
  const char *zName,              /* New VFS name */
  sqlite3_bcvfs **ppFs,           /* OUT: Handle for new object */
  char **pzErr                    /* OUT: Error message */
);

/*
** Configure a VFS object created by a call to sqlite3_bcv_new().
*/
int sqlite3_bcvfs_config(sqlite3_bcvfs *pFs, int op, sqlite3_int64 iVal);

/*
** SQLITE_BCV_CACHESIZE:
**   Argument sets the maximum size of the cachefile in bytes. This option
**   is only available for daemonless mode VFSs.
**
** SQLITE_BCV_NREQUEST:
**   Argument sets the maximum number of simultaneous HTTPS requests
**   used when writing to cloud storage (e.g. to upload database changes).
**   This option is only available for daemonless mode VFSs.
**
** SQLITE_BCV_HTTPTIMEOUT:
**   Argument sets the number of seconds after which to assume an HTTPS
**   request has been lost. Default value is 600.
**
** SQLITE_BCV_CURLVERBOSE:
**   The argument enables 
*/
#define SQLITE_BCV_CACHESIZE   1
#define SQLITE_BCV_NREQUEST    2
#define SQLITE_BCV_HTTPTIMEOUT 3
#define SQLITE_BCV_CURLVERBOSE 4

/*
** Return true if this VFS is connected to a daemon process. Or false if
** it accesses cloud storage directly (daemonless mode).
*/
int sqlite3_bcvfs_isdaemon(sqlite3_bcvfs*);

/*
** Configure an authentication callback.
**
** An authentication callback is required by both daemon and daemonless 
** mode VFS.
*/
int sqlite3_bcvfs_auth_callback(
  sqlite3_bcvfs *pFs,
  void *pAuthCtx,
  int(*xAuth)(
      void *pCtx, 
      const char *zStorage,
      const char *zAccount,
      const char *zContainer,
      char **pzAuthToken
  )
);

/*
** Configure a logging callback.
*/
int sqlite3_bcvfs_log_callback(
  sqlite3_bcvfs *pFs,
  void *pLogCtx,
  int mask,                       /* mask of SQLITE_BCV_LOG_XXX flags */
  void(*xLog)(void *pCtx, int mLog, const char *zMsg)
);

/*
** Flags for the 3rd argument to sqlite3_bcvfs_log_callback().
*/
#define SQLITE_BCV_LOG_HTTP      0x0001
#define SQLITE_BCV_LOG_UPLOAD    0x0002
#define SQLITE_BCV_LOG_CLEANUP   0x0004
#define SQLITE_BCV_LOG_EVENT     0x0008
#define SQLITE_BCV_LOG_HTTPRETRY 0x0010

/*
** Attach the specified container to the VFS. Parameters zStorage, zAccount
** and zContainer identify the cloud storage container to attach. zAlias is the
** alias to use for the attached container. If parameter zAlias is NULL, the
** alias is the same as the container name (zContainer).
**
** This API is available for both both daemon and daemonless mode VFS.
*/
int sqlite3_bcvfs_attach(
  sqlite3_bcvfs *pFs,
  const char *zStorage,
  const char *zAccount,
  const char *zContainer,
  const char *zAlias,
  int flags,                      /* Mask of SQLITE_BCV_ATTACH_XXX flags */
  char **pzErr                    /* OUT: Error message */
);

/*
** Flags for the 6th argument to sqlite3_bcvfs_attach().
**
** SQLITE_BCV_ATTACH_SECURE:
**   Specifying this flag has no effect when using a single-process VFS
**   (one created by sqlite3_bcvfs_create()). When used with a proxy
**   VFS connected to a daemon process, it requests that the daemon
**   attach the database in secure mode - encrypting blocks stored in
**   the cachefile and sharing the key only with clients that have
**   proven that they have the credentials required to access the cloud
**   storage container.
**
** SQLITE_BCV_ATTACH_IFNOT:
**   Usually, if an attempt is made to attach a container with the same
**   name (alias) as one that is already attached, it is an error. However,
**   if this flag is specified, then the operation instead returns SQLITE_OK
**   immediately.
*/
#define SQLITE_BCV_ATTACH_SECURE 0x0001
#define SQLITE_BCV_ATTACH_IFNOT  0x0002

/*
** Detach a container. A container may be detached only if both of the
** following are true:
**
**   1) There are no clients connected to databases within the container.
**   2) There are no local changes that have not been pushed to the cloud
**      to any databases within the container.
**
** If the container is successfully detached, SQLITE_OK is returned. If 
** either of the two conditions above are not met, SQLITE_BUSY is returned
** and the container remains attached.
**
** This API is available for both both daemon and daemonless mode VFS.
*/
int sqlite3_bcvfs_detach(sqlite3_bcvfs *pFs, const char *zAlias, char **pzErr);


/*
** Poll the named container for a new manifest.
**
** This API is available for both both daemon and daemonless mode VFS.
*/
int sqlite3_bcvfs_poll(sqlite3_bcvfs *pFs, const char *zCont, char **pzErr);


/*
** Attempt to upload all locally modified databases in container zCont to
** cloud storage.
**
** Uploading a modified database to cloud storage involves running the
** equivalent of "PRAGMA wal_checkpoint = full" on the database. If it
** is not NULL, then the xBusy parameter specifies a callback function
** that may be invoked in the same way as an SQLite busy-handler function
** to wait on any read or write clients blocking the checkpoint. If this
** callback function is invoked, a copy of parameter pBusyArg is passed
** as the first argument. The second argument is passed the number of times
** that the busy-handler has already been invoked by the current call to
** sqlite3_bcvfs_upload().
**
** If all locally modified databases are succesfully uploaded, SQLITE_OK
** is returned. If parameter pzErr is not NULL, *pzErr is set to NULL in
** this case. 
**
** Otherwise, if an error occurs, either an SQLite or HTTPS error code 
** is returned.
**
** If this function returns a value other than SQLITE_OK and parameter pzErr
** is not NULL, then (*pzErr) may be set to point to point to a buffer
** containing an English language error message. In this case it is the
** responsibility of the caller to free the buffer using sqlite3_free().
** If it is not set to point to a buffer containing an error message,
** (*pzErr) is set to NULL.
**
** This API always returns SQLITE_OK without doing anything for daemonless
** mode VFS (as there are never any local changes to upload). This is true
** even if the specified container does not exist.
*/
int sqlite3_bcvfs_upload(
  sqlite3_bcvfs *pFs,             /* VFS handle */
  const char *zCont,              /* Container (alias) to upload databases of */
  int (*xBusy)(void*,int),        /* Busy-handler callback */
  void *pBusyArg,                 /* First argument passed to xBusy */
  char **pzErr                    /* OUT: Error message */
);

/*
** This function is used to create a copy of a database within container
** zCont, which must be the name (alias) of an attached container. The
** database to make a copy of is identified by parameter zFrom, the name
** of the new copy will be zTo.
**
** If successful, this function returns SQLITE_OK. Otherwise, it returns
** an SQLite error code. If parameter pzErr is not NULL, then (*pzErr)
** may be set to a buffer containing an English language error message.
** It is the responsibility of the caller to ensure that this buffer
** is eventually freed using sqlite3_free().
**
** It is an error if database zFrom does not exist within the attached
** container, or if database zTo already exists.
**
** The copy operation creates the new database locally only. It is not
** uploaded to cloud storage until the next call to sqlite3_bcvfs_upload()
** on the container. Until that time it is a "local-only" database. A
** local-only database may be read and written locally in the same way
** as any other database. There are the following eccentricities:
**
**   * a local-only database may not be copied using sqlite3_bcvfs_copy().
**
**   * if another client creates a database in the container using the
**     same name as the local-only database, then this is handled in the
**     same way as a write collision - subsequent attempts to update the
**     local manifest using sqlite3_bcvfs_poll() (or "PRAGMA bcv_poll") 
**     will fail.
**
**   * if another client manipulates the parent database in such a way
**     as to make blocks that the local-only database depends on eligible
**     for deletion, this also counts as a write collision.
**
**   * if the database being copied is dirty (has changes that have yet
**     to be uploaded to the cloud), these are not included in the copy.
**     The copy is based on the uploaded version of the database, not
**     the local version.
*/
int sqlite3_bcvfs_copy(
  sqlite3_bcvfs *pFs,
  const char *zCont,              /* Name (alias) of attached container */
  const char *zFrom,
  const char *zTo,
  char **pzErr                    /* OUT: error message */
);

/*
** Delete database zDb from container zCont, which must be the name (alias)
** of an attached container. 
**
** This operation deletes the named database locally only. The database 
** is not deleted from cloud storage until the next call to 
** sqlite3_bcvfs_upload(). If there are local changes to the deleted 
** database, they are lost.
**
** If there are local connections holding the database open, this operation
** may fail with SQLITE_BUSY.
*/
int sqlite3_bcvfs_delete(
  sqlite3_bcvfs *pFs,
  const char *zCont,              /* Name (alias) of attached container */
  const char *zDb,
  char **pzErr                    /* OUT: error message */
);

/*
** Free a VFS created by a call to sqlite3_bcvfs_new(). This call will fail
** and return SQLITE_BUSY if there are still one or more database clients
** using the VFS.
*/
int sqlite3_bcvfs_destroy(sqlite3_bcvfs *pFs);

/*
** Register the eponymous vtab modules "bcv_database" and "bcv_container" 
** with the database handle passed as the only argument.
**
** This API is available for both both daemon and daemonless mode VFS.
*/
int sqlite3_bcvfs_register_vtab(sqlite3*);

typedef struct sqlite3_prefetch sqlite3_prefetch;

/*
 ** Create a new prefetch object. A prefetch object can be used to manage
 ** prefetching blocks belonging to database zDb in container zCont.
 **
 ** If successful, (*ppOut) is set to point to the new prefetch object
 ** and SQLITE_OK is returned. Otherwise, if an error occurs, an
 ** SQLite error code is returned and (*ppOut) is set to NULL.
 */
int sqlite3_bcvfs_prefetch_new(
  sqlite3_bcvfs *pFs,
  const char *zCont,
  const char *zDb,
  sqlite3_prefetch **ppOut
);

/*
** Do some prefetching. Calling this function issues HTTP requests
** for blocks of the database file that are not currently in the
** local cache until either (a) there are in total nRequest such
** requests outstanding, or (b) there are requests outstanding
** for all blocks not currently in the cache.
**
** This function does not return until either one of the outstanding
** HTTP requests has completed or the timeout of nMs ms has expired.
** It returns SQLITE_DONE if all blocks of the specified file have
** been fetched, SQLITE_OK if no error has occurred but the entire
** file has not yet been cached, or an SQLite or HTTP error code
** if an error has occured.
**
** Once an error has occurred, all subsequent calls to
** sqlite3_bcvfs_prefetch_run() immediately return the same error code.
**
** IMPORTANT: If there are outstanding requests and no error has
** occurred, applications must call sqlite3_bcvfs_prefetch_run()
** or sqlite3_bcvfs_prefetch_destroy() in a timely manner. This is
** because those outstanding requests will not be handled (or abandoned)
** until one of those two is called, and there may be other threads
** also waiting on the same requests.
*/
int sqlite3_bcvfs_prefetch_run(
  sqlite3_prefetch*,              /* Prefetch handle */
  int nRequest,                   /* Maximum number of outstanding requests */
  int nMs                         /* Timeout in ms */
);

/*
** If sqlite3_bcvfs_prefetch_run() has encountered an error - returned
** some value other than SQLITE_OK or SQLITE_DONE - this function returns
** a pointer to a buffer containing an English language error message.
*/
const char *sqlite3_bcvfs_prefetch_errmsg(sqlite3_prefetch*);
int sqlite3_bcvfs_prefetch_errcode(sqlite3_prefetch*);

/*
** Query a prefetch object for various quantities. The specific value
** queried for is determined by the second parameter, which must be
** one of the SQLITE_BCVFS_PFS_* values. If successful, SQLITE_OK is
** returned and output parameter (*piVal) set to the queried value.
** Otherwise, an SQLite error code is returned and the final value
** of (*piVal) is undefined.
*/
int sqlite3_bcvfs_prefetch_status(
  sqlite3_prefetch*, 
  int op, 
  sqlite3_int64 *piVal
);

/*
** Candidates for the second argument to sqlite3_bcvfs_prefetch_status().
**
** SQLITE_BCVFS_PFS_NOUTSTANDING:
**   The total number of outstanding requests managed by the
**   prefetch object.
**
** SQLITE_BCVFS_PFS_NDEMAND:
**   The number of clients waiting on on-demand requests currently outstanding
**   when the most recent call to prefetch_run() returned.
*/
#define SQLITE_BCVFS_PFS_NOUTSTANDING 1
#define SQLITE_BCVFS_PFS_NDEMAND      2

/*
** Free a prefetch object created using sqlite3_bcvfs_prefetch_new().
**
** If there are outstanding HTTP requests managed by the prefetch
** object when this function is called, they are abandoned.
*/
void sqlite3_bcvfs_prefetch_destroy(sqlite3_prefetch*);

/*
** Revert all changes made to the container identified by the second
** argument since the last successful call to sqlite3_bcvfs_upload(),
** including database writes, sqlite3_bcvfs_copy() and sqlite3_bcvfs_delete()
** operations.
**
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code
** is returned and output variable (*pzErr) may be set to point to a buffer
** containing an English language error message. In this case it is the
** responsibility of the caller to eventually free the buffer using
** sqlite3_free(). This function performs no network IO, so there is no
** chance of an HTTP error code being returned.
**
** This function fails with SQLITE_BUSY if there exist one or more clients
** with open read or write transactions on databases within the named 
** container.
**
** If the VFS passed as the first argument is a proxy VFS, then this
** function is a no-op that always returns SQLITE_OK. This is true even if
** zCont is not the name of an attached container.
*/
int sqlite3_bcvfs_revert(sqlite3_bcvfs*, const char *zCont, char **pzErr);


#ifdef __cplusplus
} /* end of the extern "C" block */
#endif


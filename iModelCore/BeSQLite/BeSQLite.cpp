/*--------------------------------------------------------------------------------------+
|
|     $Source: BeSQLite.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define ZLIB_INTERNAL

#include <BeSQLite/BeSQLite.h>
#include "SQLite/sqlite3.h"
#include <Bentley/BeFileName.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeThread.h>
#include <Logging/bentleylogging.h>
#include "SQLite/zipvfs.h"
#include "snappy/snappy.h"
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <string>
#include <BeSQLite/DownloadAdmin.h>
#include "BeSQLiteProfileManager.h"
#include <prg.h>

#include "seven/Types.h"
#include "seven/Lzma2Enc.h"
#include "seven/Lzma2Dec.h"
#undef min
#undef max

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"BeSQLite"))

#define DELETE_AND_CLEAR(ptr) {if(ptr){delete (ptr); ptr=NULL;}}
#define RUNONCE_CHECK(var)    {if(var) return; var=true;}

// this is used to quiet compiler warnings for variables only used in BeAsserts
#define UNUSED_VARIABLE(x) (void)(x)

using namespace std;
USING_NAMESPACE_BENTLEY_SQLITE

static Utf8CP loadZlibVfs();
static Utf8CP loadSnappyVfs();

#if defined (NDEBUG)
    #define HPOS_CheckSQLiteOperationAllowed(queryDb)
#else
    #define HPOS_CheckSQLiteOperationAllowed(queryDb) HighPriorityOperationSequencer::CheckSQLiteOperationAllowed(queryDb)
#endif

BEGIN_BENTLEY_SQLITE_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/12
//=======================================================================================
struct CachedPropertyKey
    {
    Utf8String  m_namespace;
    Utf8String  m_name;
    uint64_t    m_id;
    uint64_t    m_subId;

    void Init (Utf8CP nameSpace, Utf8CP name, uint64_t id, uint64_t subId)  {m_id = id;m_subId = subId;m_namespace.assign(nameSpace);m_name.assign(name);}
    CachedPropertyKey(){}
    CachedPropertyKey(Utf8CP nameSpace, Utf8CP name, uint64_t id, uint64_t subId) {Init(nameSpace, name, id, subId);}
    CachedPropertyKey(PropertySpecCR spec, uint64_t id, uint64_t subId) {Init(spec.GetNamespace(), spec.GetName(), id, subId);}
    bool operator< (CachedPropertyKey const& other) const
        {
        int val = m_namespace.CompareTo(other.m_namespace);
        if (val == 0)
            {
            val = m_name.CompareTo(other.m_name);
            if (val == 0)
                return (m_id != other.m_id) ? (m_id < other.m_id) : (m_subId < other.m_subId);
            }
        return val<0;
        }
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/12
//=======================================================================================
struct CachedPropertyValue
    {
    bool          m_dirty;
    bool          m_compressed;
    Utf8String    m_strVal;
    bvector<Byte> m_value;

    CachedPropertyValue() {m_dirty=m_compressed=false;}
    void ChangeValue(Utf8CP strVal, uint32_t valSize, Byte const* value, bool dirty, bool compressed)
        {
        m_dirty = dirty;
        m_compressed = compressed;
        if (strVal)
            m_strVal.assign(strVal);
        if (value)
            m_value.assign (value, value+valSize);
        }
    uint32_t GetSize() const {return (uint32_t) m_value.size();}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/12
//=======================================================================================
struct CachedProperyMap : bmap<CachedPropertyKey, CachedPropertyValue>
    {
    bool Delete(PropertySpecCR spec, uint64_t id, uint64_t subId) {return 0 != erase(CachedPropertyKey(spec, id, subId));}
    CachedPropertyValue* Find(PropertySpecCR spec, uint64_t id, uint64_t subId)
        {
        iterator it = find (CachedPropertyKey(spec, id, subId));
        return (end() == it) ? NULL : &it->second;
        }
    };

//=======================================================================================
// Cached "repository local values"
// @bsiclass                                                    Keith.Bentley   12/12
//=======================================================================================
struct CachedRlValue
    {
private:
    Utf8CP m_name;
    bool m_isUnset;
    bool m_dirty;
    int64_t m_value;

public:
    explicit CachedRlValue(Utf8CP name)
        : m_name (name)
        {
        BeAssert (!Utf8String::IsNullOrEmpty (name));
        Reset ();
        }

    Utf8CP GetName () const { return m_name; }

    int64_t GetValue () const
        {
        BeAssert (!m_isUnset);
        return m_value;
        }

    void ChangeValue (int64_t value, bool initializing = false)
        {
        m_isUnset = false;
        m_dirty = !initializing;
        m_value = value;
        }

    int64_t Increment ()
        {
        BeAssert (!m_isUnset);
        m_dirty = true;
        m_value++;
        return m_value;
        }

    bool IsUnset () const { return m_isUnset; }

    bool IsDirty () const {
        BeAssert (!m_isUnset);
        return m_dirty;
        }

    void SetIsNotDirty () {
        BeAssert (!m_isUnset);
        m_dirty = false;
        }

    void Reset ()
        {
        m_isUnset = true;
        m_dirty = false;
        m_value = -1LL;
        }
    };

//=======================================================================================
// Cache for RepositoryLocalValues
// @bsiclass                                                    Krischan.Eberle     07/14
//=======================================================================================
struct RlvCache : NonCopyableClass
    {
public:
    typedef bvector<CachedRlValue>::iterator iterator;

private:
    bvector<CachedRlValue> m_cache;

public:
    DbResult RegisterRlv (size_t& index, Utf8CP name)
        {
        size_t existingIndex = 0;
        if (TryGetIndex (existingIndex, name))
            return BE_SQLITE_ERROR;

        m_cache.push_back (CachedRlValue (name));
        index = m_cache.size () - 1;
        return BE_SQLITE_OK;
        }

    bool TryGetIndex (size_t& index, Utf8CP name) const
        {
        const size_t size = m_cache.size ();
        for (size_t i = 0; i < size; i++)
            {
            if (strcmp (name, m_cache[i].GetName ()) == 0)
                {
                index = i;
                return true;
                }
            }

        return false;
        }

    CachedRlValue& operator[] (size_t index)
        {
        BeAssert (index < m_cache.size ());
        return m_cache[index];
        }

    size_t Size () const { return m_cache.size (); }

    iterator begin () { return m_cache.begin (); }
    iterator end () { return m_cache.end (); }

    void Clear ()
        {
        for (CachedRlValue& val : m_cache)
            {
            val.Reset ();
            }
        }
    };

END_BENTLEY_SQLITE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeGuid::Create()
    {
    sqlite3_randomness (sizeof(BeGuid), m_guid.u);
    m_guid.b[6] = (m_guid.b[6] & 0x0f) | 0x40;   // see http://en.wikipedia.org/wiki/Universally_unique_identifier, use Version 4 (random)
    m_guid.b[8] = (m_guid.b[8] & 0x3f) | 0x80;   // set 2-bit flags to 0b10
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from apr_guid.c
* @bsimethod                                                    Sam.Wilson      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeGuid::ToString () const
    {
    return Utf8PrintfString ("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    m_guid.b[0], m_guid.b[1], m_guid.b[2], m_guid.b[3], m_guid.b[4], m_guid.b[5], m_guid.b[6], m_guid.b[7],
                    m_guid.b[8], m_guid.b[9], m_guid.b[10], m_guid.b[11], m_guid.b[12], m_guid.b[13], m_guid.b[14], m_guid.b[15]);
    }

/*---------------------------------------------------------------------------------**//**
* Copied from apr_guid.c
* convert a pair of hex digits to an integer value [0,255]
* @bsimethod                                                    Sam.Wilson      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static unsigned char parse_hexpair(Utf8CP s)
    {
    int result = s[0] - '0';
    if (result > 48)
        result = (result - 39) << 4;
    else if (result > 16)
        result = (result - 7) << 4;
    else
        result = result << 4;

    int temp = s[1] - '0';
    if (temp > 48)
        result |= temp - 39;
    else if (temp > 16)
        result |= temp - 7;
    else
        result |= temp;

    return (unsigned char)result;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from apr_guid.c
* @bsimethod                                                    Sam.Wilson      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeGuid::FromString (Utf8CP uuid_str)
    {
    for (int i = 0; i < 36; ++i)
        {
        char c = uuid_str[i];
        if (!isxdigit(c) && !(c == '-' && (i == 8 || i == 13 || i == 18 || i == 23)))
            return ERROR;       /* ### need a better value */
        }

    if (uuid_str[36] != '\0')
        return ERROR; /* ### need a better value */

    m_guid.b[0] = parse_hexpair(&uuid_str[0]);
    m_guid.b[1] = parse_hexpair(&uuid_str[2]);
    m_guid.b[2] = parse_hexpair(&uuid_str[4]);
    m_guid.b[3] = parse_hexpair(&uuid_str[6]);
    m_guid.b[4] = parse_hexpair(&uuid_str[9]);
    m_guid.b[5] = parse_hexpair(&uuid_str[11]);
    m_guid.b[6] = parse_hexpair(&uuid_str[14]);
    m_guid.b[7] = parse_hexpair(&uuid_str[16]);
    m_guid.b[8] = parse_hexpair(&uuid_str[19]);
    m_guid.b[9] = parse_hexpair(&uuid_str[21]);

    for (int i = 6; i--;)
        m_guid.b[10 + i] = parse_hexpair(&uuid_str[i*2+24]);

    return SUCCESS;
    }

void        BeLuid::CreateRandom() {sqlite3_randomness (sizeof(BeLuid), &m_luid.u);}
void        Statement::Finalize() {if(m_stmt){sqlite3_finalize(m_stmt);m_stmt=NULL;}}
DbResult    Statement::Step() { return m_stmt ? (DbResult) sqlite3_step(m_stmt) : BE_SQLITE_ERROR;}
DbResult    Statement::Reset() {return (DbResult)sqlite3_reset(m_stmt);}
DbResult    Statement::ClearBindings() { return m_stmt ? (DbResult)sqlite3_clear_bindings(m_stmt): BE_SQLITE_ERROR;}
DbResult    Statement::BindInt (int col, int val)           { return (DbResult)sqlite3_bind_int(m_stmt, col, val);}
DbResult    Statement::BindInt64 (int col, int64_t val)       { return (DbResult)sqlite3_bind_int64(m_stmt, col, val);}
DbResult    Statement::BindDouble (int col, double val)     { return (DbResult)sqlite3_bind_double(m_stmt, col, val);}
DbResult    Statement::BindText (int col, Utf8CP val, BindMakeCopy makeCopy, int nBytes) { return (DbResult)sqlite3_bind_text(m_stmt, col, val, nBytes, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);}
DbResult    Statement::BindZeroBlob (int col, int size) { return (DbResult)sqlite3_bind_zeroblob(m_stmt, col, size);}
DbResult    Statement::BindBlob (int col, void const* val, int size, BindMakeCopy makeCopy) { return (DbResult)sqlite3_bind_blob(m_stmt, col, val, size, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);}
DbResult    Statement::BindGuid (int col, BeGuidCR guid) { return (DbResult)sqlite3_bind_blob(m_stmt, col, &guid, sizeof(guid), SQLITE_TRANSIENT);}
DbResult    Statement::BindLuid (int col, BeLuid luid) { return (DbResult)sqlite3_bind_int64(m_stmt, col, luid.GetValue());}
DbResult    Statement::BindNull (int col) { return (DbResult)sqlite3_bind_null(m_stmt, col);}
DbResult    Statement::BindVirtualSet (int col, VirtualSet const& intSet) {return BindInt64(col, (int64_t) &intSet);}
DbValueType Statement::GetColumnType(int col)    {return (DbValueType) sqlite3_column_type(m_stmt, col);}
int         Statement::GetColumnCount ()         {return sqlite3_column_count (m_stmt);}
int         Statement::GetColumnBytes (int col)  {return sqlite3_column_bytes (m_stmt, col);}
int         Statement::GetColumnBytes16 (int col){return sqlite3_column_bytes16 (m_stmt, col);}
Utf8CP      Statement::GetColumnName (int col)   {return sqlite3_column_name (m_stmt, col);}
void const* Statement::GetValueBlob (int col)    {return sqlite3_column_blob (m_stmt, col);}
Utf8CP      Statement::GetValueText (int col)    {return (Utf8CP) sqlite3_column_text(m_stmt, col);}
int         Statement::GetValueInt (int col)     {return sqlite3_column_int (m_stmt, col);}
int64_t     Statement::GetValueInt64 (int col)   {return sqlite3_column_int64 (m_stmt, col);}
double      Statement::GetValueDouble (int col)  {return sqlite3_column_double(m_stmt, col);}
void        Statement::GetValueGuid (int col, BeGuidR guid) {memcpy (&guid, GetValueBlob(col), sizeof(guid));}
int         Statement::GetParameterIndex(Utf8CP name) { return sqlite3_bind_parameter_index(m_stmt, name);}
Utf8CP      Statement::GetSql () const           {return sqlite3_sql(m_stmt); }
DbResult    Statement::Prepare (BeSQLiteDbCR db, Utf8CP sql) {return Prepare(*db.m_dbFile, sql);}

DbValueType DbValue::GetValueType() const             {return (DbValueType) sqlite3_value_type(m_val);}
int         DbValue::GetValueBytes() const            {return sqlite3_value_bytes(m_val);}
void const* DbValue::GetValueBlob() const             {return sqlite3_value_blob(m_val);}
Utf8CP      DbValue::GetValueText() const             {return (Utf8CP) sqlite3_value_text(m_val);}
int         DbValue::GetValueInt() const              {return sqlite3_value_int(m_val);}
int64_t     DbValue::GetValueInt64() const            {return sqlite3_value_int64(m_val);}
double      DbValue::GetValueDouble() const           {return sqlite3_value_double(m_val);}
BeLuid      DbValue::GetValueLuid() const             {return BeLuid ((uint64_t) GetValueInt64());}
void        DbValue::GetValueGuid(BeGuidR guid) const {memcpy (&guid, GetValueBlob(), sizeof(guid));}

SqlDbP   Db::GetSqlDb() const {return m_dbFile->m_sqlDb;}
bool     Db::IsReadonly() const {return m_dbFile->m_flags.m_readonly;}
BeDbGuid Db::GetDbGuid() const {return m_dbFile->m_dbGuid;}
int32_t  Db::GetCurrentSavepointDepth() const {return (int32_t) m_dbFile->m_txns.size();}
Utf8String Db::GetLastError (DbResult* lastResult) const { return IsDbOpen() ? m_dbFile->GetLastError(lastResult) : "Not opened"; }
BeRepositoryId Db::GetRepositoryId() const {return m_dbFile->m_repositoryId;}

int64_t  Db::GetLastInsertRowId() const {return sqlite3_last_insert_rowid(GetSqlDb());}
int      Db::GetModifiedRowCount() const {return sqlite3_changes(GetSqlDb());}
void     ZipFromBlob::Finish() {m_blobIO.Close();}
void     SnappyFromBlob::Finish() {m_blobIO.Close();}
void     Db::SetAllowImplictTransactions (bool val) {m_dbFile->m_allowImplicitTxns=val;}

static Utf8CP s_versionfmt = "{\"major\":%d,\"minor\":%d,\"sub1\":%d,\"sub2\":%d}";

Utf8String SchemaVersion::ToJson() const {return ToString (s_versionfmt);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaVersion::FromJson(Utf8CP val)
    {
    FromString (val, s_versionfmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Statement::TryPrepare (BeSQLiteDbCR db, Utf8CP sql)
    {
    return DoPrepare (db.GetSqlDb (), sql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Statement::DoPrepare (SqlDbP db, Utf8CP sql)
    {
    HPOS_CheckSQLiteOperationAllowed(db);
    return (nullptr != m_stmt) ? BE_SQLITE_MISUSE : (DbResult) sqlite3_prepare_v2 (db, sql, -1, &m_stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Statement::Prepare (DbFile const& dbFile, Utf8CP sql)
    {
    if (!dbFile.CheckImplicitTxn())
        {
        BeAssert (false);
        return BE_SQLITE_ERROR_NoTxnActive;
        }

    DbResult rc = DoPrepare (dbFile.m_sqlDb, sql);
    if (rc != BE_SQLITE_OK)
        {
        Utf8String lastError = dbFile.GetLastError (nullptr); // keep on separate line for debugging
        LOG.errorv("Error \"%s\" preparing SQL: %s", lastError.c_str(), sql);
        BeAssert (false);
        }

    return rc;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void Statement::DumpResults()
    {
    printf("Dumping SQL: %s\n", GetSql());

    bool firstTime = true;
    while (BE_SQLITE_ROW == Step())
        {
        if (firstTime)
            {
            firstTime = false;
            Utf8String header;
            for (int i = 0; i < GetColumnCount(); ++i)
                {
                if (i > 0)
                    header.append(", ");

                header.append(GetColumnName(i));
                }
            printf("%s\n", header.c_str());
            }

        Utf8String values;
        for (int i = 0; i < GetColumnCount(); ++i)
            {
            values.append(i > 0 ? ", " : "  ");
            values.append(GetValueText(i));
            }
        printf("%s\n", values.c_str());
        }

    Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
SqlPrintfString::SqlPrintfString(Utf8CP fmt, ...)
    {
    va_list vl;
    va_start(vl,fmt);
    m_str = sqlite3_vmprintf (fmt, vl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
SqlPrintfString::~SqlPrintfString() {sqlite3_free (m_str);}

#ifdef _MSC_VER
    #pragma warning (disable:4355)
#endif // _MSC_VER

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP const DbFile::BE_REPOSITORYID_NAME = "be_repositoryId";

static int besqliteBusyHandler(void* retry, int count) {return ((BusyRetry const*) retry)->_OnBusy(count);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbFile::DbFile (SqlDbP sqlDb, BusyRetry* retry, BeSQLiteTxnMode defaultTxnMode) : m_sqlDb(sqlDb), m_cachedProps(nullptr), m_rlvCache(new RlvCache()), 
            m_defaultTxn(*this, "default", defaultTxnMode), m_statements(10), m_rtreeMatch("rTreeAccept", 1)
    {
    m_inCommit = false;
    m_allowImplicitTxns = m_settingsTableCreated = m_settingsDirty = false;
    m_dataVersion = 0;

    memset (&m_flags,       0, sizeof(m_flags));
    memset (&m_dummyBool,   0, sizeof(m_dummyBool));
    memset (&m_dummyInt32,  0, sizeof(m_dummyInt32));
    memset (&m_dummyInt64,  0, sizeof(m_dummyInt64));
    memset (&m_dummyDouble, 0, sizeof(m_dummyDouble));

    if (retry)
        {
        m_retry = retry;
        sqlite3_busy_handler(sqlDb, besqliteBusyHandler, m_retry.get());
        }

    if (BE_SQLITE_OK != m_rlvCache->RegisterRlv (m_repositoryIdRlvIndex, BE_REPOSITORYID_NAME))
        {
        BeAssert (false && "Could not register RepositoryLocalValue for 'be_repositoryId'.");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP getStartTxnSql(BeSQLiteTxnMode mode)
    {
    switch (mode)
        {
        case BeSQLiteTxnMode::Immediate: return "BEGIN IMMEDIATE";
        case BeSQLiteTxnMode::Exclusive: return "BEGIN EXCLUSIVE";
        }
    return  "BEGIN";
    }

static int savepointCommitHook (void* arg) {return ((DbFile*) arg)->OnCommit();}
static void savepointRollbackHook (void* arg) {((DbFile*) arg)->OnCommit();}

/*---------------------------------------------------------------------------------**//**
* Ensure that all commits and rollbacks are done using BeSQLite api, not through SQL directly
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
int DbFile::OnCommit()
    {
    if (m_inCommit || m_txns.empty())
        return  0;

    BeAssert (0);     // !!! USE BeSQLite API !!!

#ifdef __clang__
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wnull-dereference"
#endif

    *((int*)0) = 100; // force a crash - this must be illegal.

#ifdef __clang__
   #pragma clang diagnostic pop
#endif

    return  1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::StartSavepoint (Savepoint& txn, BeSQLiteTxnMode txnMode)
    {
    BeAssert (nullptr != m_sqlDb);

    if (txn.IsActive())
        return BE_SQLITE_ERROR;

    // we need to save the cached properties/rlvs in case nested txn is cancelled
    SaveCachedProperties(true);
    SaveCachedRlvs(true);

    void* old = sqlite3_commit_hook (m_sqlDb, savepointCommitHook, this);
    BeAssert(old == NULL || old == this);

    old = sqlite3_rollback_hook (m_sqlDb, savepointRollbackHook, this);
    BeAssert(old == NULL || old == this);

    DbResult rc = (DbResult) sqlite3_exec (m_sqlDb,
               (0 == m_txns.size()) ? getStartTxnSql(txnMode) : SqlPrintfString("SAVEPOINT \"%s\"", txn.GetName()).GetUtf8CP(), NULL, NULL, NULL);
    
    if (BE_SQLITE_OK != rc)
        {
        txn.m_depth = -1;
        return rc;
        }

    if (txn.m_db && 0 == m_txns.size())
        {
        Statement stmt;
        stmt.TryPrepare (*txn.m_db, "PRAGMA data_version");
        rc = stmt.Step();
        BeAssert (BE_SQLITE_ROW == rc);

        UInt64 newDataVersion = stmt.GetValueInt64(0);
        if (0 != m_dataVersion && newDataVersion != m_dataVersion) // don't call this on the first Savepoint
            txn.m_db->_OnDbChangedByOtherConnection();

        m_dataVersion = newDataVersion;
        }

    m_txns.push_back(&txn);
    txn.m_depth = (int32_t) m_txns.size() - 1;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::StopSavepoint (Savepoint& txn, bool isCommit)
    {
    if (!txn.IsActive())     // not active? forget it.
        return BE_SQLITE_ERROR;

    // make sure the txn is really active for this DbFile
    DbTxnIter thisPos = m_txns.begin() + txn.m_depth;
    if (*thisPos != &txn)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    SaveCachedProperties(isCommit);
    SaveCachedRlvs(isCommit);

    m_inCommit = true;

    // we can't commit or rollback if any active changetrack has changes.
    for (auto const& tracker : m_trackers)
        {
        if (tracker.second->HasChanges())
            return  BE_SQLITE_ERROR_ChangeTrackError;
        }

    // attempt the commit/release or rollback
    DbResult rc;
    if (0 == txn.GetDepth())
        {
        rc = (DbResult) sqlite3_exec (m_sqlDb, (isCommit ? "COMMIT" : "ROLLBACK"), NULL, NULL, NULL);
        }
    else
        {
        Utf8String sql;
        if (!isCommit)   // to cancel a nested transaction, we need to roll it back and then release it.
            sql.append (SqlPrintfString ("ROLLBACK TO \"%s\";", txn.GetName()));

        sql.append (SqlPrintfString ("RELEASE \"%s\"", txn.GetName()));
        rc = (DbResult) sqlite3_exec (m_sqlDb, sql.c_str(), NULL, NULL, NULL);
        }

    m_inCommit = false;

    if (rc != BE_SQLITE_OK)
        return rc;

    // deactivate this and all nested lower txns.
    for (DbTxnIter it=thisPos; it != m_txns.end(); ++it)
        (*it)->_OnDeactivate(isCommit);

    m_txns.erase (thisPos, m_txns.end());
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
Db::Db () : m_embeddedFiles(*this), m_appData(*this)
    {
    m_statements=0;
    }

Db::~Db() {DoCloseDb();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Savepoint::Savepoint(BeSQLiteDbR db, Utf8CP name, bool beginTxn, BeSQLiteTxnMode txnMode) : m_dbFile(db.m_dbFile.get()), m_name(name), m_txnMode(txnMode)
    {
    m_db = &db;
    m_depth = -1;

    if (beginTxn)
        Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Savepoint::_Begin(BeSQLiteTxnMode mode)  {return m_dbFile ? m_dbFile->StartSavepoint(*this, mode) : BE_SQLITE_ERROR;}
DbResult Savepoint::_Cancel() {return m_dbFile ? m_dbFile->StopSavepoint(*this, false) : BE_SQLITE_ERROR;}
DbResult Savepoint::_Commit() {return m_dbFile ? m_dbFile->StopSavepoint(*this, true) : BE_SQLITE_ERROR;}
DbResult Savepoint::Begin(BeSQLiteTxnMode mode)  {return _Begin(mode);}
DbResult Savepoint::Commit() {return _Commit();}
DbResult Savepoint::Cancel() {return _Cancel();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Savepoint* Db::GetSavepoint (int32_t depth) const
    {
    return (depth<0 || depth>=GetCurrentSavepointDepth()) ? NULL : m_dbFile->m_txns[depth];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ExecuteSql (Utf8CP sql, int (*callback)(void*,int,CharP*,CharP*),void* arg,CharP* errmsg)
    {
    if (!m_dbFile->CheckImplicitTxn())
        {
        BeAssert (false);
        return  BE_SQLITE_ERROR_NoTxnActive;
        }

    DbResult rc = (DbResult) sqlite3_exec (GetSqlDb(), sql, callback, arg, errmsg);
    if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
        {
        Utf8String lastError = GetLastError(); // keep on separate line for debuggging
        LOG.errorv("Error \"%s\" SQL: %s", lastError.c_str(), sql);
        BeAssert(false);  // If you EXPECT failures to be non-exceptional, call TryExecuteSql
        }
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::TryExecuteSql (Utf8CP sql, int (*callback)(void*,int,CharP*,CharP*),void* arg,CharP* errmsg)
    {
    return (DbResult) sqlite3_exec (GetSqlDb(), sql, callback, arg, errmsg);
    }

static Utf8CP getTempPrefix(bool temp) {return temp ? TEMP_TABLE_Prefix : "";}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::CreatePropertyTable (Utf8CP tablename, Utf8CP ddl, bool temp)
    {
    if (temp)
        {
        if (m_settingsTableCreated)
            return  BE_SQLITE_OK;

        m_settingsTableCreated = true;
        }

    return (DbResult) sqlite3_exec (m_sqlDb, SqlPrintfString("CREATE TABLE %s%s (%s)",getTempPrefix(temp),tablename, ddl), NULL, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
CachedProperyMap& DbFile::GetCachedPropMap() const
    {
    if (NULL == m_cachedProps)
        m_cachedProps = new CachedProperyMap();

    CachedProperyMap& map =*((CachedProperyMap*) m_cachedProps);
    return  map;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
CachedPropertyValue& DbFile::GetCachedProperty (PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    return GetCachedPropMap()[CachedPropertyKey(spec.GetNamespace(), spec.GetName(), id, subId)];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
CachedPropertyValue* DbFile::FindCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    return GetCachedPropMap().Find(spec, id, subId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DeleteCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId)
    {
    if (NULL != m_cachedProps)
        GetCachedPropMap().erase(CachedPropertyKey(spec.GetNamespace(), spec.GetName(), id, subId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::SaveCachedProperties(bool isCommit)
    {
    if (NULL == m_cachedProps)
        return;

    if (!isCommit)
        {
        DeleteCachedPropertyMap(); // only delete cached property map on cancel, not commit.
        return;
        }

    CachedProperyMap& map = GetCachedPropMap();
    for (CachedProperyMap::iterator it=map.begin(); it!=map.end(); ++it)
        {
        CachedPropertyKey const& key = it->first;
        CachedPropertyValue& val = it->second;
        if (val.m_dirty)
            {
            PropertySpec spec (key.m_name.c_str(), key.m_namespace.c_str(), PropertySpec::TXN_MODE_Normal, val.m_compressed ? PropertySpec::COMPRESS_PROPERTY_Yes : PropertySpec::COMPRESS_PROPERTY_No);
            SaveProperty (spec, val.m_strVal.length()>0 ? val.m_strVal.c_str() : NULL, val.m_value.size()>0 ? val.m_value.data() : NULL, (uint32_t) val.m_value.size(),
                          key.m_id, key.m_subId);
            val.m_dirty=false;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
RlvCache& DbFile::GetRlvCache () const
    {
    BeAssert (m_rlvCache != nullptr);
    return *m_rlvCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::SaveCachedRlvs(bool isCommit) const
    {
    if (nullptr == m_rlvCache)
        return;

    if (!isCommit)
        {
        ClearRlvCache ();
        return; // only clear cached RLVs on cancel, not commit
        }

    CachedStatementPtr stmt;
    RlvCache& cache = GetRlvCache();
    for (CachedRlValue& rlv : cache)
        {
        if (!rlv.IsUnset () && rlv.IsDirty ())
            {
            if (!stmt.IsValid())
                {
                DbResult rc = m_statements.GetPreparedStatement(stmt, *this, "INSERT OR REPLACE INTO " BEDB_TABLE_Local " (Name,Val) VALUES(?,?)");
                if (rc != BE_SQLITE_OK)
                    { BeAssert(false); }
                }

            stmt->BindText (1, rlv.GetName (), Statement::MAKE_COPY_No);

            int64_t int64Val = rlv.GetValue ();
            stmt->BindBlob (2, &int64Val, (int) sizeof (int64Val), Statement::MAKE_COPY_No);
            DbResult rc = stmt->Step ();
            if (BE_SQLITE_DONE != rc)
                { BeAssert(false); }
            stmt->Reset();
            rlv.SetIsNotDirty ();
            }
        }
    }

#define PROPERTY_TABLE_DDL "Namespace CHAR NOT NULL,Name CHAR NOT NULL,Id INT NOT NULL,SubId INT NOT NULL,TxnMode Int NOT NULL,StrData CHAR,RawSize INT,Data BLOB,PRIMARY KEY(Namespace,Name,Id,SubId)"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::SaveProperty (PropertySpecCR spec, Utf8CP stringData, void const* value, uint32_t size, uint64_t id, uint64_t subId)
    {
    if (NULL == value && NULL == stringData && !spec.SaveIfNull())
        {
        DeleteProperty(spec, id);
        return  BE_SQLITE_OK;
        }

    if (spec.IsCached())
        {
        GetCachedProperty(spec, id, subId).ChangeValue(stringData, size, (Byte*)value, true, spec.IsCompress());
        return  BE_SQLITE_OK;
        }

    DbResult rc;
    if (spec.IsSetting())
        {
        rc = CreatePropertyTable(BEDB_TABLE_Property, PROPERTY_TABLE_DDL, true);
        if (BE_SQLITE_OK != rc)
            return rc;
        }

    CachedStatementPtr stmt;
    rc = m_statements.GetPreparedStatement(stmt, *this, SqlPrintfString("INSERT OR REPLACE INTO %s" BEDB_TABLE_Property " (Namespace,Name,Id,SubId,TxnMode,RawSize,Data,StrData) VALUES(?,?,?,?,?,?,?,?)",
                                     getTempPrefix(spec.IsSetting())));
    if (BE_SQLITE_OK != rc)
        return  rc;

    if (spec.IsSetting())
        m_settingsDirty = true;

    stmt->BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
    stmt->BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
    stmt->BindInt64 (3, id);
    stmt->BindInt64 (4, subId);
    stmt->BindInt (5, spec.IsSetting());
    stmt->BindText(8, stringData, Statement::MAKE_COPY_No);

    bool doCompress = spec.IsCompress();
    bvector<Byte> compressed; // use bvector just so destructor will free make sure this is outside if!
    if (NULL != value)
        {
        if (size <= 100) // too small to be worth trying
            doCompress = false;

        if (doCompress)
            {
            unsigned long compressedSize= (uint32_t) (size*1.01) + 12;
            compressed.reserve (compressedSize);
            if (Z_OK != compress2 (compressed.data(), &compressedSize, (ByteCP) value, size, DefaultCompressionLevel) || (compressedSize >= size))
                doCompress = false;
            else
                {
                stmt->BindInt(6, size);     // this is the uncompressed size, when compressed
                stmt->BindBlob (7, compressed.data(), compressedSize, Statement::MAKE_COPY_No);
                }
            }

        if (!doCompress) // dont use "else" here, value of compress can change if zip fails!
            {
            stmt->BindNull(6);
            stmt->BindBlob (7, value, size, Statement::MAKE_COPY_No);
            }
        }

    rc = stmt->Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbFile::UseSettingsTable(PropertySpecCR spec) const {return m_settingsDirty && spec.IsSetting();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryPropertySize (uint32_t& size, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        {
        HasProperty (spec, id, subId); // make sure its cached

        CachedPropertyValue* cachedProp = FindCachedProperty(spec, id, subId);
        if (NULL == cachedProp)
            return BE_SQLITE_ERROR;

        size = cachedProp->GetSize();
        return  BE_SQLITE_ROW;
        }

    bool useSettingsTable = UseSettingsTable(spec);
    Statement stmt;
    DbResult rc = stmt.Prepare (*this, SqlPrintfString("SELECT RawSize,length(Data) FROM %s" BEDB_TABLE_Property " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?",getTempPrefix(useSettingsTable)));
    if (BE_SQLITE_OK == rc)
        {
        stmt.BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
        stmt.BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
        stmt.BindInt64(3, id);
        stmt.BindInt64(4, subId);
        rc = stmt.Step();
        }

    if (rc != BE_SQLITE_ROW)
        {
        size = 0;
        return useSettingsTable ? QueryPropertySize (size, PropertySpec(spec, PropertySpec::TXN_MODE_Normal), id) : BE_SQLITE_ERROR;
        }

    size = stmt.GetValueInt(0);
    if (0 == size)
        size = stmt.GetValueInt(1);

    return  BE_SQLITE_ROW;
    }

#define FROM_PROPERTY_TABLE_SQL " FROM %s" BEDB_TABLE_Property " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryCachedProperty (Utf8String* strval, void** value, uint32_t size, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    BeAssert (spec.IsCached());
    CachedPropertyValue* cachedProp = FindCachedProperty(spec, id, subId);
    if (NULL == cachedProp)
        {
        CachedStatementPtr stmt;
        DbResult rc = m_statements.GetPreparedStatement(stmt, *this, SqlPrintfString("SELECT RawSize,Data,StrData" FROM_PROPERTY_TABLE_SQL, getTempPrefix(false)));
        if (BE_SQLITE_OK == rc)
            {
            stmt->BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
            stmt->BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
            stmt->BindInt64(3, id);
            stmt->BindInt64(4, subId);
            rc = stmt->Step();
            }

        if (rc != BE_SQLITE_ROW)
            return BE_SQLITE_ERROR;

        cachedProp = &GetCachedProperty(spec, id, subId);

        Utf8CP strVal = stmt->GetValueText(2);
        if (strVal)
            cachedProp->m_strVal = strVal;

        uint32_t compressedBytes = stmt->GetValueInt(0);
        uint32_t blobsize = stmt->GetColumnBytes(1);

        uint32_t bytes = (0==compressedBytes) ? blobsize : compressedBytes;

        cachedProp->m_value.resize(bytes);

        void const* blobdata = stmt->GetValueBlob(1);

        if (compressedBytes > 0)
            {
            unsigned long actuallyRead = size;
            uncompress ((Byte*)cachedProp->m_value.data(), &actuallyRead, (ByteCP) blobdata, blobsize);
            if (actuallyRead != size)
                return BE_SQLITE_MISMATCH;
            }
        else
            {
            if (blobsize < size)
                return BE_SQLITE_MISMATCH;

            memcpy ((Byte*)cachedProp->m_value.data(), blobdata, bytes);
            }
        }

    if (strval)
        *strval = cachedProp->m_strVal;

    if (size>cachedProp->GetSize())
        size = cachedProp->GetSize();

    if (value)
        memcpy (*value, cachedProp->m_value.data(), size);

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryProperty (void* value, uint32_t size, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        return QueryCachedProperty(NULL, &value, size, spec, id, subId);

    bool useSettingsTable = UseSettingsTable(spec);
    CachedStatementPtr stmt;
    DbResult rc = m_statements.GetPreparedStatement(stmt, *this, SqlPrintfString("SELECT RawSize,Data" FROM_PROPERTY_TABLE_SQL, getTempPrefix(useSettingsTable)));
    if (BE_SQLITE_OK == rc)
        {
        stmt->BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
        stmt->BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
        stmt->BindInt64(3, id);
        stmt->BindInt64(4, subId);
        rc = stmt->Step();
        }

    if (rc != BE_SQLITE_ROW)
        return useSettingsTable ? QueryProperty(value, size, PropertySpec(spec, PropertySpec::TXN_MODE_Normal), id) : BE_SQLITE_ERROR;

    uint32_t compressedBytes = stmt->GetValueInt(0);
    uint32_t blobsize = stmt->GetColumnBytes(1);

    uint32_t bytes = (0==compressedBytes) ? blobsize : compressedBytes;
    if (size>bytes)
        size=bytes;

    void const* blobdata = stmt->GetValueBlob(1);

    if (compressedBytes > 0)
        {
        unsigned long actuallyRead = size;
        uncompress ((Byte*)value, &actuallyRead, (ByteCP) blobdata, blobsize);
        if (actuallyRead != size)
            return BE_SQLITE_MISMATCH;
        }
    else
        {
        if (blobsize < size)
            return BE_SQLITE_MISMATCH;

        memcpy (value, blobdata, size);
        }

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryProperty (Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        return QueryCachedProperty(&value, NULL, 0, spec, id, subId);

    bool useSettingsTable = UseSettingsTable(spec);
    CachedStatementPtr stmt;
    DbResult rc = m_statements.GetPreparedStatement(stmt, *this, SqlPrintfString("SELECT StrData" FROM_PROPERTY_TABLE_SQL, getTempPrefix(useSettingsTable)));
    if (BE_SQLITE_OK == rc)
        {
        stmt->BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
        stmt->BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
        stmt->BindInt64(3, id);
        stmt->BindInt64(4, subId);
        rc = stmt->Step();
        }

    if (rc != BE_SQLITE_ROW)
        {
        value.clear();
        return  useSettingsTable ? QueryProperty(value, PropertySpec(spec, PropertySpec::TXN_MODE_Normal), id) : BE_SQLITE_ERROR;
        }

    value.AssignOrClear (stmt->GetValueText(0));
    return  BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbFile::HasProperty (PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        return BE_SQLITE_ROW==QueryCachedProperty (NULL, NULL, 0, spec, id, subId);

    bool useSettingsTable = UseSettingsTable(spec);
    Statement stmt;
    DbResult rc = stmt.Prepare (*this, SqlPrintfString("SELECT 1" FROM_PROPERTY_TABLE_SQL, getTempPrefix(useSettingsTable)));
    if (BE_SQLITE_OK == rc)
        {
        stmt.BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
        stmt.BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
        stmt.BindInt64(3, id);
        stmt.BindInt64(4, subId);
        rc = stmt.Step();
        }

    if (rc == BE_SQLITE_ROW)
        return  true;

    return  useSettingsTable ? HasProperty (PropertySpec(spec, PropertySpec::TXN_MODE_Normal), id) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::DeleteProperty (PropertySpecCR spec, uint64_t id, uint64_t subId)
    {
    if (spec.IsCached())
        DeleteCachedProperty (spec, id, subId);

    bool useSettingsTable = UseSettingsTable(spec);
    Statement stmt;
    DbResult rc = stmt.Prepare (*this, SqlPrintfString("DELETE" FROM_PROPERTY_TABLE_SQL, getTempPrefix(useSettingsTable)));
    if (BE_SQLITE_OK == rc)
        {
        stmt.BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
        stmt.BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
        stmt.BindInt64(3, id);
        stmt.BindInt64(4, subId);
        rc = stmt.Step();
        }

    // if it's a setting, delete it from both tables.
    return useSettingsTable ? DeleteProperty (PropertySpec(spec, PropertySpec::TXN_MODE_Normal), id, subId) : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::DeleteProperties (PropertySpecCR spec, uint64_t* id)
    {
    bool useSettingsTable = UseSettingsTable(spec);
    Utf8String sql (SqlPrintfString("DELETE FROM %s" BEDB_TABLE_Property " WHERE Namespace=? AND Name=? ", getTempPrefix(useSettingsTable)));
    if (id)
        sql.append ("AND Id=?");

    Statement stmt;
    DbResult rc = stmt.Prepare (*this, sql.c_str());
    if (BE_SQLITE_OK == rc)
        {
        stmt.BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
        stmt.BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
        if (id)
            stmt.BindInt64(3, *id);

        rc = stmt.Step();
        }

    // if it's a setting, delete it from both tables.
    return useSettingsTable ? DeleteProperties (PropertySpec(spec, PropertySpec::TXN_MODE_Normal), id) : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::SaveSettings ()
    {
    if (!m_settingsDirty)
        return;

    // we don't want changes to settings in the persistent properties table to be undoable.
    for (auto tracker : m_trackers)
        tracker.second->_OnSettingsSave();

    m_settingsDirty = false;
    DbResult rc = (DbResult) sqlite3_exec (m_sqlDb,"INSERT OR REPLACE INTO " BEDB_TABLE_Property " (Namespace,Name,Id,SubId,TxnMode,RawSize,Data,StrData) "
                  "SELECT Namespace,Name,Id,SubId,TxnMode,RawSize,Data,StrData FROM " TEMP_TABLE_Prefix BEDB_TABLE_Property, NULL, NULL, NULL);
    BeAssert (BE_SQLITE_OK == rc);

    rc = (DbResult) sqlite3_exec (m_sqlDb,"DELETE FROM " TEMP_TABLE_Prefix BEDB_TABLE_Property, NULL, NULL, NULL);
    BeAssert (BE_SQLITE_OK == rc);

    for (auto tracker : m_trackers)
        tracker.second->_OnSettingsSaved();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::SaveSettings ()
    {
    _OnSaveSettings();
    m_dbFile->SaveSettings();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveBeDbGuid()
    {
    if (!m_dbFile->m_dbGuid.IsValid())
        m_dbFile->m_dbGuid.Create();

    DbResult rc = SaveProperty (Properties::DbGuid(), (void*) &m_dbFile->m_dbGuid, sizeof(m_dbFile->m_dbGuid));
    if (BE_SQLITE_OK != rc)
        {
        LOG.warningv ("Could not save GUID. Error: %s - %s", InterpretDbResult (rc), GetLastError ().c_str());
        BeAssert (false);
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveRepositoryId()
    {
    if (!m_dbFile->m_repositoryId.IsValid())
        m_dbFile->m_repositoryId = BeRepositoryId(0);

    return SaveRepositoryLocalValue (m_dbFile->m_repositoryIdRlvIndex, m_dbFile->m_repositoryId.GetValue ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
DbResult Db::SaveBeSQLiteBuild()
    {
    static Utf8CP BUILD_VERSION = REL_V "." MAJ_V "." MIN_V "." SUBMIN_V;
    
    return SavePropertyString(Properties::BeSQLiteBuild(), BUILD_VERSION);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::ChangeDbGuid(BeDbGuid id)
    {
    m_dbFile->m_dbGuid = id;
    SaveBeDbGuid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ChangeRepositoryId(BeRepositoryId id)
    {
    if (IsReadonly ())
        return BE_SQLITE_READONLY;

    m_dbFile->m_repositoryId = id;

    //instead of updating the repo id in the be_local table, simply empty it
    //and insert the new repo id via the next call. Whether the one or the other option
    //is faster, would need to be tested, but changing a repo id is a singular operation.
    //So it shouldn't matter.

    DbResult stat = DeleteRepositoryLocalValues();
    if (stat != BE_SQLITE_OK)
        {
        AbandonChanges();
        return stat;
        }

    stat = SaveRepositoryId();
    if (stat != BE_SQLITE_OK)
        {
        AbandonChanges();
        return stat;
        }

    stat =_OnRepositoryIdChanged (id);
    if (stat != BE_SQLITE_OK)
        {
        AbandonChanges();
        return stat;
        }

    stat = SaveChanges();
    if (stat != BE_SQLITE_OK)
        return stat;

    m_dbFile->m_repositoryId = id;

    LOG.infov ("Changed repository id to %d for '%s'.", id.GetValue(), GetDbFileName());
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::GetNextRepositoryBasedId(BeRepositoryBasedId& value, Utf8CP tableName, Utf8CP colName)
    {
    if (value.IsValid())
        value.UseNext();
    else
        {
        Statement stmt;
        stmt.Prepare (*this, SqlPrintfString ("SELECT max(%s) FROM %s WHERE %s<?", colName, tableName, colName));

        BeRepositoryBasedId lastId(m_dbFile->m_repositoryId.GetNextRepositoryId(), 0);
        stmt.BindInt64 (1, lastId.GetValue());

        DbResult result = stmt.Step();
        BeAssert (result == BE_SQLITE_ROW);
        if (result != BE_SQLITE_ROW)
            return  result;

        int64_t currMax = stmt.GetValueInt64(0);

        BeRepositoryBasedId firstId (m_dbFile->m_repositoryId, 1);
        value = ((currMax < firstId.GetValue()) ? firstId : BeRepositoryBasedId (currMax+1));
        }
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::AddAggregateFunction(AggregateFunction& func) const
    {
    const int stat = m_dbFile->AddAggregateFunction(func);
    if (stat != 0)
        return stat;

    return _OnAddFunction(func);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::AddScalarFunction(ScalarFunction& func) const
    {
    const int stat = m_dbFile->AddScalarFunction(func);
    if (stat != 0)
        return stat;

    return _OnAddFunction(func);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::RemoveFunction(DbFunction& func) const
    {
    _OnRemoveFunction(func);
    return m_dbFile->RemoveFunction(func);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::SaveMyProjectGuid (BeProjectGuid projectGuid)
    {
    if (!projectGuid.IsValid())
        projectGuid.Create();

    SaveProperty (Properties::GuidOfMyProject(), (void*) &projectGuid, sizeof(projectGuid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeProjectGuid Db::QueryMyProjectGuid() const
    {
    BeProjectGuid projectGuid(false);
    QueryProperty (&projectGuid, sizeof(projectGuid), Properties::GuidOfMyProject());
    return  projectGuid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
Db::OpenParams::OpenParams (OpenMode openMode, StartDefaultTransaction defaultTxn, BusyRetry* retry)
    : m_openMode (openMode), m_startDefaultTxn (defaultTxn), m_skipSchemaCheck (false), m_rawSQLite(false), m_busyRetry(retry)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::CreateNewDb (Utf8CP dbName, BeDbGuid dbGuid, CreateParams const& params)
    {
    if (IsDbOpen())
        return BE_SQLITE_ERROR_AlreadyOpen;

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    // We are only really supporting MAX_PATH paths on Windows right now; it would take some effort to update BeFilename and inject //?/ in various places to really support much better.
    // It should also be noted that while sqlite3_open_v2 may succeed with a path close to MAX_PATH, Savepoint::Begin (or any other writable operation) can fail when sqlite goes to open the journal file (since its name is longer).
    // Check up-front to ensure that the DB name itself, as well as its journal filename, are under MAX_PATH. Note that the MAX_PATH limit is based on UTF-16.

    static const size_t JOURNAL_SUFFIX_LENGTH = 8; // "-journal"
    WString dbNameW(dbName, BentleyCharEncoding::Utf8);

    if ((dbNameW.size() + 1 + JOURNAL_SUFFIX_LENGTH) > MAX_PATH)
        return BE_SQLITE_CANTOPEN;
#endif

    OpenMode openMode = OPEN_Create;
    if (dbName && (0 != strcmp (dbName, BEDB_MemoryDb)))
        {
        WString dbNameW (dbName, BentleyCharEncoding::Utf8); // string conversion
        if (BeFileName::DoesPathExist (dbNameW.c_str()))
            {
            if (params.m_failIfDbExists)
                return BE_SQLITE_ERROR_FileExists;

            openMode = OPEN_ReadWrite;
            }
        }

    Utf8CP vfs = 0;
    switch (params.m_compressedDb)
        {
        case CreateParams::CompressDb_Zlib:
            vfs = loadZlibVfs();
            break;
        case CreateParams::CompressDb_Snappy:
            vfs = loadSnappyVfs();
            break;
        }

    SqlDbP sqlDb;
    DbResult rc = (DbResult) sqlite3_open_v2 (dbName, &sqlDb, openMode, vfs);
    if (BE_SQLITE_OK != rc)
        return  rc;

    sqlite3_extended_result_codes (sqlDb, 1); // turn on extended error codes
    m_dbFile = new DbFile(sqlDb, params.m_busyRetry, (BeSQLiteTxnMode)params.m_startDefaultTxn);

    m_dbFile->m_defaultTxn.Begin();
    m_dbFile->m_dbGuid = dbGuid;

    ExecuteSql (SqlPrintfString("PRAGMA page_size=%d;PRAGMA encoding=\"%s\";PRAGMA user_version=%d;PRAGMA application_id=%lld;PRAGMA locking_mode=\"%s\"", params.m_pagesize,
                              params.m_encoding==ENCODING_Utf8 ? "UTF-8" : "UTF-16le", BeSQLite::DbUserVersion, params.m_applicationId,
                              params.m_startDefaultTxn==DefaultTxn_Exclusive ? "EXCLUSIVE" : "NORMAL"));

    rc = m_dbFile->CreatePropertyTable(BEDB_TABLE_Property, PROPERTY_TABLE_DDL, false);
    if (BE_SQLITE_OK != rc)
        return  rc;

    // this table purposely has no primary key so it won't be tracked / merged. It is meant to hold values that are
    // local to the repository and never in a changeset.
    rc = CreateTable (BEDB_TABLE_Local, "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,Val BLOB");
    if (BE_SQLITE_OK != rc)
        return  rc;

    rc = SaveBeDbGuid();
    if (BE_SQLITE_OK != rc)
        return  rc;

    rc = SaveRepositoryId();
    if (BE_SQLITE_OK != rc)
        return  rc;

    rc = EmbeddedFiles ().CreateTable ();
    if (rc != BE_SQLITE_OK)
        return rc;

    if (params.m_expirationDate != DateTime())
        SetExpirationDate (params.m_expirationDate);

    rc = BeSQLiteProfileManager::AssignProfileVersion (*this);
    if (rc != BE_SQLITE_OK)
        return rc;

    rc = _OnDbCreated (params);
    if (BE_SQLITE_OK != rc)
        return  rc;

    if (DefaultTxn_No == params.m_startDefaultTxn)
        m_dbFile->m_defaultTxn.Commit();

    LOG.infov ("Created file '%s'.", GetDbFileName ());
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::AttachDb (Utf8CP filename, Utf8CP alias)
    {
    Savepoint* txn = GetSavepoint(0);
    bool wasActive = (txn!= NULL) && (BE_SQLITE_OK == txn->Commit());

    DbResult rc = (DbResult) sqlite3_exec (GetSqlDb(), SqlPrintfString("ATTACH \"%s\" AS %s", filename, alias), NULL, NULL, NULL);

    if (rc != BE_SQLITE_OK)
        {
        BeAssert (false);
        Utf8String lastError = GetLastError (nullptr); // keep on separate line for debuggging
        LOG.errorv("AttachDb failed: \"%s\" filename:[%s], alias:[%s]", lastError.c_str(), filename, alias);
        }

    if (wasActive)
        txn->Begin();

    return  rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DetachDb (Utf8CP alias)
    {
    Savepoint* txn = GetSavepoint(0);
    bool wasActive = (txn!= NULL) && (BE_SQLITE_OK == txn->Commit());

    DbResult rc = (DbResult) sqlite3_exec (GetSqlDb(), SqlPrintfString("DETACH %s", alias), NULL, NULL, NULL);

    if (wasActive)
        txn->Begin();

    return  rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::RegisterRepositoryLocalValue (size_t& rlvIndex, Utf8CP rlvName)
    {
    if (m_dbFile == nullptr)
        {
        LOG.fatal ("Cannot call RegisterRepositoryLocalValue until Db has been opened/created.");
        return BE_SQLITE_ERROR;
        }

    return m_dbFile->GetRlvCache ().RegisterRlv (rlvIndex, rlvName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
bool Db::TryGetRepositoryLocalValueIndex (size_t& rlvIndex, Utf8CP rlvName) const
    {
    if (m_dbFile == nullptr)
        {
        LOG.fatal ("Cannot call TryGetRepositoryLocalValueIndex until Db has been opened/created.");
        BeAssert (false && "Cannot call TryGetRepositoryLocalValueIndex until Db has been opened/created.");
        return false;
        }

    return m_dbFile->GetRlvCache ().TryGetIndex (rlvIndex, rlvName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::SaveRepositoryLocalValue (size_t rlvIndex, int64_t value)
    {
    auto& rlvCache = m_dbFile->GetRlvCache ();
    if (rlvIndex >= rlvCache.Size ())
        return BE_SQLITE_NOTFOUND;

    rlvCache[rlvIndex].ChangeValue (value);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::QueryRepositoryLocalValue (int64_t& value, size_t rlvIndex) const
    {
    auto& rlvCache = m_dbFile->GetRlvCache ();
    if (rlvIndex >= rlvCache.Size ())
        return BE_SQLITE_NOTFOUND;

    CachedRlValue* cachedRlv = nullptr;
    if (!TryQueryRepositoryLocalValue (cachedRlv, rlvIndex))
        return BE_SQLITE_ERROR;

    value = cachedRlv->GetValue ();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::IncrementRepositoryLocalValue (int64_t& newValue, size_t rlvIndex) const
    {
    CachedRlValue* cachedRlv = nullptr;
    if (!TryQueryRepositoryLocalValue (cachedRlv, rlvIndex))
        return BE_SQLITE_ERROR;

    BeAssert (cachedRlv->GetValue () < BeRepositoryBasedId (GetRepositoryId (), std::numeric_limits<uint32_t>::max ()).GetValue () && "Maximum BeRepositoryBasedId reached.");
    newValue = cachedRlv->Increment ();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
bool Db::TryQueryRepositoryLocalValue (CachedRlValue*& value, size_t rlvIndex) const
    {
    auto& rlvCache = m_dbFile->GetRlvCache ();
    if (rlvIndex >= rlvCache.Size ())
        return false;

    CachedRlValue& cachedRlv = rlvCache[rlvIndex];
    if (cachedRlv.IsUnset ())
        {
        Statement stmt;
        stmt.Prepare (*this, "SELECT Val FROM " BEDB_TABLE_Local " WHERE Name=?");
        stmt.BindText (1, cachedRlv.GetName (), Statement::MAKE_COPY_No);

        const DbResult rc = stmt.Step ();
        if (rc != BE_SQLITE_ROW)
            return false;

        BeAssert (!stmt.IsColumnNull (0));
        void const* blob = stmt.GetValueBlob (0);
        int64_t val = -1LL;
        memcpy (&val, blob, sizeof(val));
        BeAssert (stmt.GetColumnBytes (0) <= (int) sizeof (int64_t));
        cachedRlv.ChangeValue (val, true);
        }

    BeAssert (!cachedRlv.IsUnset ());
    value = &cachedRlv;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::DeleteRepositoryLocalValues ()
    {
    m_dbFile->ClearRlvCache ();
    return TruncateTable (BEDB_TABLE_Local);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::CreateTable (Utf8CP tableName, Utf8CP ddl)
    {
    DbResult rc = TryExecuteSql(SqlPrintfString("CREATE TABLE %s (%s)", tableName, ddl));
    return  rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DropTable (Utf8CP tableName)
    {
    DbResult rc = TryExecuteSql(SqlPrintfString("DROP TABLE %s", tableName));
    BeAssert (rc == BE_SQLITE_OK);
    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 12/12
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::TruncateTable (Utf8CP tableName)
    {
    DbResult rc = TryExecuteSql (SqlPrintfString ("DELETE FROM %s", tableName));
    BeAssert (rc == BE_SQLITE_OK);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::TableExists(Utf8CP tableName) const
    {
    if (!IsDbOpen ())
        {
        BeAssert (false);
        return false;
        }

    Statement statement;
    return BE_SQLITE_OK == statement.TryPrepare (*this, SqlPrintfString ("SELECT NULL FROM %s", tableName));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2012
//--------------+------------------------------------------------------------------------
void Db::FlushPageCache ()
    {
    sqlite3_db_release_memory(m_dbFile->m_sqlDb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::ColumnExists(Utf8CP tableName, Utf8CP columnName) const
    {
    if (!IsDbOpen ())
        {
        LOG.error ("Db::ColumnExists can only be called on open database connection.");
        BeAssert (IsDbOpen ());
        return false;
        }

    Statement sql;
    return BE_SQLITE_OK == sql.TryPrepare(*this, SqlPrintfString ("SELECT [%s] FROM [%s]", columnName, tableName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::GetColumns(bvector<Utf8String>& columns, Utf8CP tableName) const
    {
    if (!IsDbOpen ())
        {
        BeAssert (false);
        return false;
        }

    Statement statement;
    DbResult status =  statement.TryPrepare (*this, SqlPrintfString ("SELECT * FROM [%s] LIMIT 0", tableName));
    if (status != BE_SQLITE_OK)
        return false;

    columns.clear();
    for(int nColumn = 0; nColumn < statement.GetColumnCount(); nColumn++)
        {
        columns.push_back(statement.GetColumnName(nColumn));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::RenameTable (Utf8CP tableName, Utf8CP newTableName)
    {
    if (!IsDbOpen ())
        {
        BeAssert (false);
        return false;
        }

    return BE_SQLITE_OK == ExecuteSql (SqlPrintfString ("ALTER TABLE [%s] RENAME TO [%s]", tableName, newTableName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DeleteCachedPropertyMap()
    {
    if (NULL != m_cachedProps)
        {
        delete (CachedProperyMap*) m_cachedProps;
        m_cachedProps = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::ClearRlvCache () const
    {
    GetRlvCache ().Clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbFile::~DbFile()
    {
    if (NULL == m_sqlDb)
        return;

    for (auto tracker : m_trackers)
        delete tracker.second;

    m_trackers.clear();

    if (0 != m_txns.size())
        {
        m_txns[0]->Commit();        // Commit writes out cached RLVs, etc.  Note: Commit creates new cached statements.
        m_txns.clear();
        }

    m_statements.Empty();           // No more statements will be prepared and cached.
    DeleteCachedPropertyMap();
    if (m_rlvCache != nullptr)
        {
        delete m_rlvCache;
        m_rlvCache = nullptr;
        }

    BeAssert (m_txns.empty());
    BeAssert (m_statements.IsEmpty());
    BeAssert (NULL == m_cachedProps);

    DbResult rc =  (DbResult) sqlite3_close(m_sqlDb);

    if (BE_SQLITE_OK != rc)
        {
        sqlite3_stmt* stmt = NULL;
        while (NULL != (stmt = sqlite3_next_stmt(m_sqlDb, stmt)))
            LOG.errorv("Statement not closed: '%s'", sqlite3_sql(stmt));

        LOG.errorv("Cannot close database '%s'", sqlite3_db_filename(m_sqlDb, "main"));
        BeAssert (false);
        }

    m_sqlDb = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::IsDbOpen() const
    {
    return  (m_dbFile.IsValid() && NULL != m_dbFile->m_sqlDb);
    }

struct CleanupCaller
    {
    BeSQLiteDbR m_db;
    CleanupCaller (BeSQLiteDbR db) : m_db(db) {}
    void CallHandler (DbAppData& handler) const {handler._OnCleanup (m_db);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::DoCloseDb ()
    {
    if (!IsDbOpen())
        return;

    m_appData.m_entries.CallAll (CleanupCaller(*this));
    m_appData.m_entries.m_list.clear();

    DELETE_AND_CLEAR (m_statements);
    m_dbFile = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::CloseDb()
    {
    _OnDbClose();
    DoCloseDb();
    }

Db::DbAppDataList::DbAppDataList(Db& db ) : m_db(db) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Db::DbAppDataList::Add (DbAppData::Key const& key, DbAppData* obj)
    {
    return m_entries.AddAppData (key, obj, m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Db::DbAppDataList::Drop(DbAppData::Key const& key)
    {
    return m_entries.DropAppData (key, m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DbAppData* Db::DbAppDataList::Find(DbAppData::Key const& key) const
    {
    return m_entries.FindAppData (key);
    }

Db::DbAppDataList& Db::AppData() const {return const_cast<DbAppDataList&>(m_appData);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ShareDbFileFrom (Db& from)
    {
    if (IsDbOpen() || !from.IsDbOpen())
        return  BE_SQLITE_ERROR;

    m_dbFile = from.m_dbFile;
    _OnDbShared (from);
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::SetLimit (int id, int newVal) {return sqlite3_limit (m_dbFile->m_sqlDb, id, newVal);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP Db::GetDbFileName() const
    {
    return  (m_dbFile.IsValid() && m_dbFile->m_sqlDb) ? sqlite3_db_filename(m_dbFile->m_sqlDb, "main") : NULL;
    }

//  See http://www.sqlite.org/fileformat.html for format of header. 
#define DBFILE_PAGESIZE_OFFSET  16
#define DBFILE_PAGECOUNT_OFFSET 28

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult isValidDbFile(Utf8CP filename, Utf8CP& vfs)
    {
    WString filenameW (filename, BentleyCharEncoding::Utf8);
    BeFile tester;
    BeFileStatus status = tester.Open (filenameW.c_str(), BeFileAccess::Read);
    switch (status)
        {
        case BeFileStatus::Success:
            break;
        case BeFileStatus::FileNotFoundError:
            return  BE_SQLITE_ERROR_FileNotFound;
        default:
            return  BE_SQLITE_ERROR;
        }

    uint32_t bytesRead;
    uint8_t header[100] = "";
    tester.Read (header, &bytesRead, sizeof(header));
    tester.Close ();

    DbResult result = BE_SQLITE_NOTADB;
    Utf8CP ident = (Utf8CP)header;
    if (0 == strcmp (ident, SQLITE_FORMAT_SIGNATURE))
        result = BE_SQLITE_OK;
    else if (NULL != BeSQLiteLib::GetDownloadAdmin() && (0 == strcmp (ident, DOWNLOAD_FORMAT_SIGNATURE)))
        {
        vfs = BeSQLiteLib::GetDownloadAdmin()->GetVfs();
        result = BE_SQLITE_OK;
        }
    else if (0 == strcmp (ident, SQLZLIB_FORMAT_SIGNATURE))
        {
        vfs = loadZlibVfs();
        result = BE_SQLITE_OK;
        }
    else if (0 == strcmp (ident, SQLSNAPPY_FORMAT_SIGNATURE))
        {
        vfs = loadSnappyVfs();
        result = BE_SQLITE_OK;
        }

    if (BE_SQLITE_OK != result)
        return result;

    //  Extract big endian values
    uint32_t pageSize = (header[DBFILE_PAGESIZE_OFFSET+0] << 8) + header[DBFILE_PAGESIZE_OFFSET+1];  
    uint64_t pageCount = (header[DBFILE_PAGECOUNT_OFFSET+0] << 24) + (header[DBFILE_PAGECOUNT_OFFSET+1] << 16) + (header[DBFILE_PAGECOUNT_OFFSET+2] << 8) + header[DBFILE_PAGECOUNT_OFFSET+3]; 
    uint64_t requiredMin = pageSize*pageCount;
    uint64_t filesize;
    BeFileName::GetFileSize(filesize, filenameW.c_str());
    if (filesize<requiredMin)
        {
        LOG.errorv("isValidDbFile: file %s is truncated", filename);
        return BE_SQLITE_CORRUPT_VTAB;
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DoOpenDb (Utf8CP dbName, OpenParams const& params)
    {
    if (IsDbOpen())
        return BE_SQLITE_ERROR_AlreadyOpen;

    Utf8CP vfs = 0;
    DbResult rc = isValidDbFile(dbName, vfs);
    if (BE_SQLITE_OK != rc)
        return  rc;

    SqlDbP sqlDb;
    rc = (DbResult) sqlite3_open_v2 (dbName, &sqlDb, params.m_openMode, vfs);
    if (BE_SQLITE_OK != rc)
        return  rc;

    m_dbFile = new DbFile(sqlDb, params.m_busyRetry, (BeSQLiteTxnMode) params.m_startDefaultTxn);
    m_dbFile->m_flags.m_readonly = (params.m_openMode & OPEN_Readonly)==OPEN_Readonly;
    sqlite3_extended_result_codes (sqlDb, 1); // turn on extended error codes

    if (m_dbFile->m_defaultTxn.GetTxnMode() == BeSQLiteTxnMode::Exclusive)
        {
        rc = TryExecuteSql ("PRAGMA locking_mode=\"EXCLUSIVE\"");
        BeAssert (rc == BE_SQLITE_OK);
        }

    rc = m_dbFile->m_defaultTxn.Begin();
    if (BE_SQLITE_OK != rc)
        return  rc;

    if (!params.m_rawSQLite)
        {
        // make sure that the be_Prop table exists and has the right columns before continuing
        Statement statement;
        rc = statement.TryPrepare (*this, "SELECT Namespace,Name,Id,SubId,TxnMode,RawSize,Data,StrData FROM " BEDB_TABLE_Property);
        if (BE_SQLITE_OK != rc)
            return rc == BE_SQLITE_ERROR ? BE_SQLITE_ERROR_NoPropertyTable : rc;

        rc = _OnDbOpened();
        }

    if (!params.m_startDefaultTxn)
        m_dbFile->m_defaultTxn.Commit();

    return  rc;
    }

/*---------------------------------------------------------------------------------**//**
* Called before the schema upgrade process starts. Make sure the Db is writeable.
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::OpenParams::_ReopenForSchemaUpgrade(Db& db) const
    {
    if (!IsReadonly())
        return true;

    Utf8String filename(db.GetDbFileName());
    db.CloseDb();

    m_openMode = OPEN_ReadWrite;
    m_skipSchemaCheck = true;

    return db.OpenBeSQLiteDb (filename.c_str(), *this) == BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::OpenBeSQLiteDb (Utf8CP dbName, OpenParams const& params)
    {
    DbResult rc = DoOpenDb(dbName, params);
    if (rc != BE_SQLITE_OK)
        {
        DoCloseDb();
        return rc;
        }

    return params.m_skipSchemaCheck ? BE_SQLITE_OK  : _VerifySchemaVersion(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::QueryExpirationDate (DateTime& xdate) const
    {
    if (!IsDbOpen())
        return BE_SQLITE_ERROR;

    BeAssert (TableExists(BEDB_TABLE_Property) && "call GetExpirationDate only on Dbs that have a be_Prop table.");

    Utf8String xdateStr;
    auto rc = QueryProperty (xdateStr, Properties::ExpirationDate());
    if (BE_SQLITE_ROW != rc)
        return  BE_SQLITE_NOTFOUND;   // expiration date is optional

    if (DateTime::FromString (xdate, xdateStr.c_str()) != BSISUCCESS)
        {
        BeDataAssert (false && "invalid value stored for expiration date property");
        return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* DEPRECATED function - remove in Graphite06
* @bsimethod                                    Sam.Wilson                      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::GetExpirationDate (DateTime& xdate) const
    {
    DbResult result = QueryExpirationDate(xdate);
    if (BE_SQLITE_ROW == result)
        result = BE_SQLITE_OK;        // preserve legacy return value for now...
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveExpirationDate (DateTime const& xdate)
    {
    if (!xdate.IsValid())
        return BE_SQLITE_ERROR;

    if (xdate.GetInfo().GetKind() != DateTime::Kind::Utc)
        return BE_SQLITE_ERROR;

    Utf8String xdateString(xdate.ToString());
    if (xdateString.empty())
        {
        BeAssert(false); // a valid DateTime generated an empty string?
        return BE_SQLITE_ERROR;
        }
    
    return SavePropertyString (Properties::ExpirationDate(), xdateString);
    }

/*---------------------------------------------------------------------------------**//**
* DEPRECATED function - remove in Graphite06
* @bsimethod                                    Sam.Wilson                      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SetExpirationDate (DateTime const& xdate)
    {
    return SaveExpirationDate(xdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::IsExpired() const
    {
    DateTime xdate;
    if (GetExpirationDate (xdate) != BE_SQLITE_OK)
        return false;

    return DateTime::Compare (DateTime::GetCurrentTimeUtc(), xdate) != DateTime::CompareResult::EarlierThan;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveChanges()
    {
    Savepoint* txn = GetSavepoint(0);
    return txn ? txn->Save() : BE_SQLITE_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::AbandonChanges()
    {
    Savepoint* txn = GetSavepoint(0);
    if (NULL == txn)
        return  BE_SQLITE_ERROR;

    txn->Cancel();
    return txn->Begin();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Keith.Bentley                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::_OnDbCreated(CreateParams const& params) {return BE_SQLITE_OK;}
DbResult Db::_OnDbOpened() {return QueryDbIds();}
DbResult Db::_OnDbShared(Db& from) {return QueryDbIds();}

void Db::_OnDbChangedByOtherConnection()
    {
    m_dbFile->DeleteCachedPropertyMap();
    m_dbFile->ClearRlvCache();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::CheckProfileVersion (bool& fileIsAutoUpgradable, SchemaVersion const& expectedProfileVersion, SchemaVersion const& actualProfileVersion,
                                  SchemaVersion const& minimumUpgradableProfileVersion, bool openModeIsReadonly,
                                  Utf8CP profileName)
    {
    fileIsAutoUpgradable = false;

    if (Utf8String::IsNullOrEmpty(profileName))
        {
        BeAssert (!Utf8String::IsNullOrEmpty(profileName) && "Db::CheckProfileVersion expects profileName to neither be null or empty.");
        return BE_SQLITE_INTERNAL;
        }

    if (minimumUpgradableProfileVersion.GetSub1() != 0 || minimumUpgradableProfileVersion.GetSub2() != 0)
        {
        BeAssert (false && "Db::CheckProfileVersion expects minimumUpgradableProfileVersion's Sub1 and Sub2 digits to be 0.");
        return BE_SQLITE_INTERNAL;
        }

    //If major/minor of file version is older than minimum version to which auto-upgrades can be done, file is too old
    if (actualProfileVersion.CompareTo (minimumUpgradableProfileVersion, SchemaVersion::VERSION_MajorMinor) < 0)
        {
        BeAssert (minimumUpgradableProfileVersion.CompareTo (expectedProfileVersion) <= 0 && "Minimum auto-upgradable profile version must be less or equal expected profile version.");

        LOG.errorv ("Cannot open file: The file's %s profile is too old to be auto-upgraded.", profileName);
        return BE_SQLITE_ERROR_ProfileTooOld;
        }

    //If major and minor of actual profile version is newer than expected, file cannot be opened.
    if (actualProfileVersion.CompareTo (expectedProfileVersion, SchemaVersion::VERSION_MajorMinor) > 0)
        {
        LOG.errorv ("Cannot open file: The file's %s profile is too new. Please upgrade your Bentley product to the latest version.", profileName);
        return BE_SQLITE_ERROR_ProfileTooNew;
        }

    //at this point, actual major / minor is between or equal minimum and expected version.
    BeAssert (actualProfileVersion.CompareTo(minimumUpgradableProfileVersion, SchemaVersion::VERSION_MajorMinor) >= 0 &&
            actualProfileVersion.CompareTo(expectedProfileVersion, SchemaVersion::VERSION_MajorMinor) <= 0 && "Logical error in Db::CheckProfileVersion");

    //If file is older than expected version (but newer or equal than minimum auto-upgrade version), file is auto-upgradable
    if (actualProfileVersion.CompareTo (expectedProfileVersion, SchemaVersion::VERSION_All) < 0)
        {
        fileIsAutoUpgradable = true;

        if (actualProfileVersion.CompareTo (expectedProfileVersion, SchemaVersion::VERSION_MajorMinor) < 0)
            {
            LOG.debugv ("File's %s profile is too old, but auto-upgradable.", profileName);
            return BE_SQLITE_ERROR_ProfileTooOld;
            }
        else
            {
            LOG.debugv ("File's %s profile is older than expected, but is compatible with the version of this software. It also is auto-upgradable.", profileName);
            return BE_SQLITE_OK;
            }
        }

    //at this point actual version's major and minor are equal to expected, and sub1 and sub2 are equal or greater than expected
    BeAssert (actualProfileVersion.CompareTo(expectedProfileVersion, SchemaVersion::VERSION_MajorMinor) == 0 &&
             (actualProfileVersion.GetSub1 () >= expectedProfileVersion.GetSub1() || actualProfileVersion.GetSub2() >= expectedProfileVersion.GetSub2()) &&
             "Logical error in Db::CheckProfileVersion");

    //If sub1 of actual profile version is greater than expected file can be opened readonly
    if (actualProfileVersion.GetSub1 () > expectedProfileVersion.GetSub1 ())
        {
        if (openModeIsReadonly)
            {
            LOG.warningv ("File's %s profile is newer than expected, but the file can be opened read-only. Please consider to upgrade your Bentley product to the latest version.", profileName);
            return BE_SQLITE_OK;
            }

        LOG.errorv ("File's %s profile is newer than expected. The file can only be opened read-only. Re-open the file in read-only mode. Please consider to upgrade your Bentley product to the latest version.", profileName);
        return BE_SQLITE_ERROR_ProfileTooNewForReadWrite;
        }

    BeAssert (actualProfileVersion.GetSub1() == expectedProfileVersion.GetSub1() && "Logical error in Db::CheckProfileVersion");

    if (actualProfileVersion.GetSub2() > expectedProfileVersion.GetSub2 ())
        {
        LOG.warningv ("File's %s profile is newer than expected, but the file is backwards compatible with the version of this software. Please consider to upgrade your Bentley product to the latest version.", profileName);
        return BE_SQLITE_OK;
        }

    BeAssert (actualProfileVersion.CompareTo (expectedProfileVersion) == 0 && "Logical error in Db::CheckProfileVersion");

    LOG.debugv ("File's %s profile is up-to-date.", profileName);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Krischan.Eberle                   11/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::UpgradeBeSQLiteProfile ()
    {
    return BeSQLiteProfileManager::UpgradeProfile (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::QueryDbIds()
    {
    DbResult rc = QueryProperty (&m_dbFile->m_dbGuid, sizeof(m_dbFile->m_dbGuid), Properties::DbGuid());
    if (BE_SQLITE_ROW != rc)
        return  BE_SQLITE_ERROR_NoPropertyTable;

    int64_t repoId;
    rc = QueryRepositoryLocalValue (repoId, m_dbFile->m_repositoryIdRlvIndex);
    if (BE_SQLITE_OK != rc)
        return BE_SQLITE_ERROR_NoPropertyTable;

    m_dbFile->m_repositoryId = BeRepositoryId ((int32_t) repoId);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFunction::Context::SetResultBlob(void const* value, int length, CopyData doCopy) {sqlite3_result_blob((sqlite3_context*) this, value, length, (sqlite3_destructor_type) doCopy);}
void DbFunction::Context::SetResultDouble(double val){sqlite3_result_double((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultError(Utf8CP val, int len){sqlite3_result_error((sqlite3_context*) this, val, len);}
void DbFunction::Context::SetResultError_toobig(){sqlite3_result_error_toobig((sqlite3_context*) this);}
void DbFunction::Context::SetResultError_nomem(){sqlite3_result_error_nomem((sqlite3_context*) this);}
void DbFunction::Context::SetResultError_code(int val){sqlite3_result_error_code((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultInt(int val){sqlite3_result_int((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultInt64(int64_t val){sqlite3_result_int64((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultNull(){sqlite3_result_null((sqlite3_context*) this);}
void DbFunction::Context::SetResultText(Utf8CP val, int length, CopyData doCopy){sqlite3_result_text((sqlite3_context*) this, val, length,(sqlite3_destructor_type) doCopy);}
void DbFunction::Context::SetResultZeroblob(int length){sqlite3_result_zeroblob((sqlite3_context*)this, length);}
void DbFunction::Context::SetResultValue(DbValue val){sqlite3_result_value((sqlite3_context*)this, val.m_val);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void aggregateStep(sqlite3_context* context, int nArgs, sqlite3_value** args) {((AggregateFunction*) sqlite3_user_data(context))->GetAggregate()->_StepAggregate((DbFunction::Context*) context, nArgs, (DbValue*) args); }
static void aggregateFinal(sqlite3_context* context) {((AggregateFunction*)sqlite3_user_data(context))->GetAggregate()->_FinishAggregate((DbFunction::Context*) context);}
static void scalarFunc(sqlite3_context* context, int nArgs, sqlite3_value** args) {((ScalarFunction*)sqlite3_user_data(context))->GetScalar()->_ComputeScalar((DbFunction::Context*) context, nArgs, (DbValue*)args);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
int DbFile::AddAggregateFunction(AggregateFunction& function) const
    {
    return sqlite3_create_function_v2(m_sqlDb, function.GetName(), function.GetNumArgs(), SQLITE_UTF8 | SQLITE_DETERMINISTIC, &function, NULL, aggregateStep, aggregateFinal, NULL);
    }

int DbFile::AddScalarFunction(ScalarFunction& function) const
    {
    return sqlite3_create_function_v2 (m_sqlDb, function.GetName(), function.GetNumArgs(), SQLITE_UTF8 | SQLITE_DETERMINISTIC, &function, scalarFunc, NULL, NULL, NULL);
    }

int DbFile::RemoveFunction(DbFunction& function) const
    {
    return sqlite3_create_function_v2 (m_sqlDb, function.GetName(), function.GetNumArgs(), SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, NULL, NULL, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int filterCaller (void* tracker, Utf8CP tableName) {return ((ChangeTracker*) tracker)->_FilterTable(tableName);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeTracker::CreateSession ()
    {
    if (m_session)
        return  BE_SQLITE_OK;

    if (NULL == m_dbFile)
        {
        BeAssert (false);
        return  BE_SQLITE_ERROR;
        }

    DbResult result = (DbResult) sqlite3session_create(m_dbFile->GetSqlDb(), "main", &m_session);
    BeAssert (BE_SQLITE_OK == result);

    if (BE_SQLITE_OK == result)
        sqlite3session_table_filter (m_session, filterCaller, this); // set up auto-attach for all tables

    m_isTracking = true; // new sessions are on by default
    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeTracker::EndTracking ()
    {
    if (m_session)
        {
        sqlite3session_delete (m_session);
        m_session = 0;
        }
    m_isTracking = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeTracker::EnableTracking (bool yesNo)
    {
    CreateSession();
    if (m_isTracking == yesNo)
        return;

    sqlite3session_enable(m_session, yesNo);
    m_isTracking = yesNo;
    }

void ChangeTracker::SetIndirectChanges (bool yesNo) {sqlite3session_indirect(m_session, yesNo);}
bool ChangeTracker::HasChanges() {return m_session && 0 == sqlite3session_isempty(m_session);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::FromChangeTrack(ChangeTracker& session)
    {
    BeAssert(!IsValid());
    return (DbResult) sqlite3session_changeset (session.GetSqlSession(), &m_size, &m_changeset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::PatchSetFromChangeTrack(ChangeTracker& session)
    {
    BeAssert(!IsValid());
    return (DbResult) sqlite3session_patchset(session.GetSqlSession(), &m_size, &m_changeset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::FromData (int size, void const* data, bool invert)
    {
    if (IsValid())
        {
        BeAssert (false);
        return  BE_SQLITE_ERROR;
        }

    if (invert)
        return (DbResult) sqlite3changeset_invert (size, data, &m_size, &m_changeset);

    m_size = size;
    m_changeset = sqlite3_malloc (size);
    memcpy(m_changeset, data, size);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeSet::Free()
    {
    sqlite3_free (m_changeset);
    m_changeset = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
static int conflictCallback (void *pCtx, int cause, SqlChangesetIterP iter) {return ((ChangeSet*) pCtx)->_OnConflict ((ChangeSet::ConflictCause) cause, Changes::Change(iter, true));}
static int filterTableCallback (void *pCtx, Utf8CP tableName) {return ((ChangeSet*) pCtx)->_FilterTable (tableName);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::ApplyChanges (BeSQLiteDbR db)
    {
    return (DbResult) sqlite3changeset_apply(db.GetSqlDb(), m_size, m_changeset, filterTableCallback, conflictCallback, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Finalize() const
    {
    if (m_iter)
        {
        sqlite3changeset_finalize (m_iter);
        m_iter=0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Change Changes::begin() const
    {
    Finalize();

    if (m_changeset.IsValid())
        sqlite3changeset_start(&m_iter, m_changeset.GetSize(), (void*) m_changeset.GetData());

    if (0 == m_iter)
        return  Change (0, false);

    DbResult result = (DbResult) sqlite3changeset_next(m_iter);
    return  Change (m_iter, result==BE_SQLITE_ROW);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::~Changes () {Finalize();}
DbResult Changes::Change::GetOperation (Utf8CP* tableName, int* nCols, DbOpcode* opcode, int* indirect)const {return (DbResult) sqlite3changeset_op(m_iter, tableName, nCols, (int*) opcode, indirect);}
DbResult Changes::Change::GetPrimaryKeyColumns (Byte** cols, int* nCols) const {return (DbResult) sqlite3changeset_pk(m_iter, cols, nCols);}
DbValue  Changes::Change::GetOldValue (int colNum) const
    {
    SqlValueP val=0;
    int rc=sqlite3changeset_old (m_iter, colNum, &val);
    if (rc!=BE_SQLITE_OK)
        { BeAssert(false); }
    return DbValue(val);
    }

DbValue  Changes::Change::GetNewValue (int colNum) const
    {
    SqlValueP val=0;
    int rc=sqlite3changeset_new (m_iter, colNum, &val);
    if (rc!=BE_SQLITE_OK)
        { BeAssert(false); }

    return DbValue(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Change& Changes::Change::operator++()
    {
    m_isValid = (BE_SQLITE_ROW == (DbResult) sqlite3changeset_next(m_iter));
    return  *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::AddChangeTracker (ChangeTracker& tracker)
    {
    if (tracker.GetDbFile() != NULL)
        {
        BeAssert (false);
        return  BE_SQLITE_ERROR;
        }

    if (!m_dbFile->m_trackers.Insert (tracker.GetName(), &tracker).second)
        return  BE_SQLITE_ERROR;

    tracker.SetDbFile(m_dbFile.get());
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DropChangeTracker (Utf8CP name)
    {
    auto tracker = m_dbFile->m_trackers.find (name);
    if (tracker == m_dbFile->m_trackers.end())
        return  BE_SQLITE_ERROR;

    delete tracker->second;
    m_dbFile->m_trackers.erase (tracker);
    return  BE_SQLITE_OK;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker* Db::FindChangeTracker (Utf8CP name)
    {
    auto tracker = m_dbFile->m_trackers.find (name);
    return (tracker == m_dbFile->m_trackers.end()) ? NULL : tracker->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String toHex(Byte* buf, size_t nb, size_t w)
    {
    Utf8String hd;
    size_t i, n = std::min(nb, w);
    unsigned char c;
    static Utf8Char hxdg[] = "0123456789abcdef";

    if (!buf)
        return "<null>";

    for (i=0; i<n; ++i)
        {
        if (i && i%4==0)
            hd.append(" ");

        c = buf[i];
        hd.append(1, hxdg[c>>4]);
        hd.append(1, hxdg[0xf&c]);
        }

    for (   ; i<w; ++i)
        {
        if (i && i%4==0)
            hd.append(" ");

        hd.append(" ");
        hd.append(" ");
        }

    return hd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String toAsc(Byte* buf, size_t nb, size_t w)
    {
    Utf8String ad;
    size_t i, n = std::min(nb, w);
    for (i=0; i<n; ++i)
        if (isprint ((int)buf[i]))
            ad.append(1, (Utf8Char)buf[i]);
        else
            ad.append (".");

    ad.append((w-i), ' ');
    return ad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String hexDump(Byte* bytes, size_t nbytes)
    {
    Utf8String output;
    size_t const nperline = 32;

    Byte* nextBytes = bytes;
    Byte* bytesX    = bytes + nbytes;

    size_t nlines = nbytes/nperline;
    for (size_t i=0; i<nlines; ++i)
        {
        output.append("\n");
        output.append(toHex(nextBytes, nperline, nperline));
        output.append(" |");
        output.append(toAsc(nextBytes, nperline, nperline));
        output.append(" |");
        nextBytes += nperline;
        }

    int nrem = static_cast<int>(bytesX - nextBytes);
    if (0 != nrem)
        {
        output.append("\n");
        output.append(toHex(nextBytes, nrem, nperline));
        output.append(" |");
        output.append(toAsc(nextBytes, nrem, nperline));
        output.append(" |");
        }

    output.append("\n");

    return output;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbValue::Format(int detailLevel) const
    {
    if (!IsValid())
        return "<<INVALID>>";

    switch (GetValueType())
        {
        case DbValueType::IntegerVal:
            return Utf8PrintfString("%" PRId64, GetValueInt64());

        case DbValueType::FloatVal:
            return Utf8PrintfString("%lg", GetValueDouble());

        case DbValueType::TextVal:
            return Utf8PrintfString("\"%s\"", GetValueText());

        case DbValueType::BlobVal:
            if (detailLevel < 1)
                return "...";
            return Utf8PrintfString(hexDump((Byte*)GetValueBlob(), GetValueBytes()).c_str());

        case DbValueType::NullVal:
            return "NULL";
        }

    BeAssert(false);
    return "?";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Change::DumpColumns(int startCol, int endCol, Changes::Change::ValueStage stage, bvector<Utf8String> const& columns, int detailLevel) const
    {
    Byte* pcols;
    int npcols;
    GetPrimaryKeyColumns(&pcols, &npcols);

    int nprinted = 0;
    for (int i=startCol; i <= endCol; ++i)
        {
        if (std::find(pcols, pcols+npcols, (Byte)i) != pcols+npcols)    // we print the old value of the (unchanging) primary key columns separately
            continue;

        auto v = GetValue(i, stage);
        if (!v.IsValid() || v.IsNull())
            continue;

        if (nprinted != 0)
            printf(" ");

        printf("[%s]", columns[i].c_str());

        if (v.IsValid())
            printf("%s", v.Format(detailLevel).c_str());

        ++nprinted;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Changes::Change::FormatPrimarykeyColumns(bool isInsert, int detailLevel) const
    {
    Byte* pcols;
    int npcols;
    GetPrimaryKeyColumns(&pcols, &npcols);

    Utf8String pcolstr;
    for (int i=0; i<npcols; ++i)
        {
        if (pcols[i] == 0)
            continue;

        auto pkv = GetValue(i, isInsert? Changes::Change::VALUE_New: Changes::Change::VALUE_Old);
        BeAssert(pkv.IsValid());
        
        if (pkv.IsNull())   // WIP_CHANGES -- why do we get this??
            continue;

        if (!pcolstr.empty())
            pcolstr.append(", ");

        pcolstr.append(pkv.Format(detailLevel));
        }
    return pcolstr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Change::Dump(Db const& db, bool isPatchSet, bset<Utf8String>& tablesSeen, int detailLevel) const
    {
    Utf8CP tableName;
    int nCols,indirect;
    DbOpcode opcode;
    DbResult rc = GetOperation(&tableName, &nCols, &opcode, &indirect);
    if (rc!=BE_SQLITE_OK)
        { BeAssert(false); }

    if (tablesSeen.find(tableName) == tablesSeen.end())
        {
        printf("\n\tTable: %s\n", tableName);
        tablesSeen.insert (tableName);
        }

    printf("[%s] ", FormatPrimarykeyColumns((BE_SQLITEOP_INSERT==opcode), detailLevel).c_str());

    bvector<Utf8String> columnNames;
    db.GetColumns(columnNames, tableName);

    if (nCols > (int)columnNames.size())
        {
        printf("*** CORRUPT ***\n");
        return;
        }

    switch (opcode)
        {
        case BE_SQLITEOP_DELETE:
            printf("DELETE ");
            if (detailLevel > 0)
                printf("\n");
            DumpColumns(0, nCols-1, VALUE_Old, columnNames, detailLevel);
            break;
        case BE_SQLITEOP_INSERT:
            printf("INSERT ");
            if (detailLevel > 0)
                printf("\n");
            DumpColumns(0, nCols-1, VALUE_New, columnNames, detailLevel);
            break;
        case BE_SQLITEOP_UPDATE:
            printf("UPDATE ");
            if (detailLevel > 0)
                printf("\n");
            if (!isPatchSet)
                {
                printf("old: ");
                DumpColumns(0, nCols-1, VALUE_Old, columnNames, detailLevel);
                printf("\nnew: ");
                }
            DumpColumns(0, nCols-1, VALUE_New, columnNames, detailLevel);
            break;

        default:
            BeAssert(false);
        }
    printf("\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeSet::Dump(Db const& db, bool isPatchSet, int detailLevel) const
    {
    bset<Utf8String> tablesSeen;

    printf("\nChangeSet:\n");
    Changes changes(*const_cast<ChangeSet*>(this));
    for (auto& change : changes)
        {
        change.Dump(db, isPatchSet, tablesSeen, detailLevel);
        }
    }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ChangeSet::InterpretConflictCause (ChangeSet::ConflictCause cause)
    {
    switch (cause)
        {
        case ChangeSet::CONFLICT_CAUSE_Data: return ("data");
        case ChangeSet::CONFLICT_CAUSE_NotFound: return ("not found");
        case ChangeSet::CONFLICT_CAUSE_Conflict: return ("conflict");
        case ChangeSet::CONFLICT_CAUSE_Constraint: return ("constraint");
        case ChangeSet::CONFLICT_CAUSE_ForeignKey: return ("foreign key");
        }
    BeAssert(false);
    return ("?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbFile::GetLastError (DbResult* lastResult) const
    {
    DbResult ALLOW_NULL_OUTPUT (status, lastResult);
    if (NULL == m_sqlDb)
        {
        status = BE_SQLITE_ERROR;
        return "Not opened";
        }

    status = (DbResult) sqlite3_errcode(m_sqlDb);
    Utf8String msg = (Utf8CP)sqlite3_errmsg(m_sqlDb);
    msg.append(" (").append(Db::InterpretDbResult(status)).append(")");
    return msg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP Db::InterpretDbResult (DbResult result)
    {
    switch (result)
        {
        case BE_SQLITE_OK:                  return "BE_SQLITE_OK";
        case BE_SQLITE_ERROR:               return "BE_SQLITE_ERROR";
        case BE_SQLITE_INTERNAL:            return "BE_SQLITE_INTERNAL";
        case BE_SQLITE_PERM:                return "BE_SQLITE_PERM";
        case BE_SQLITE_ABORT:               return "BE_SQLITE_ABORT";
        case BE_SQLITE_BUSY:                return "BE_SQLITE_BUSY";
        case BE_SQLITE_LOCKED:              return "BE_SQLITE_LOCKED";
        case BE_SQLITE_NOMEM:               return "BE_SQLITE_NOMEM";
        case BE_SQLITE_READONLY:            return "BE_SQLITE_READONLY";
        case BE_SQLITE_INTERRUPT:           return "BE_SQLITE_INTERRUPT";
        case BE_SQLITE_IOERR:               return "BE_SQLITE_IOERR";
        case BE_SQLITE_CORRUPT:             return "BE_SQLITE_CORRUPT";
        case BE_SQLITE_NOTFOUND:            return "BE_SQLITE_NOTFOUND";
        case BE_SQLITE_FULL:                return "BE_SQLITE_FULL";
        case BE_SQLITE_CANTOPEN:            return "BE_SQLITE_CANTOPEN";
        case BE_SQLITE_PROTOCOL:            return "BE_SQLITE_PROTOCOL";
        case BE_SQLITE_EMPTY:               return "BE_SQLITE_EMPTY";
        case BE_SQLITE_SCHEMA:              return "BE_SQLITE_SCHEMA";
        case BE_SQLITE_TOOBIG:              return "BE_SQLITE_TOOBIG";
        case BE_SQLITE_CONSTRAINT_BASE:     return "BE_SQLITE_CONSTRAINT";
        case BE_SQLITE_MISMATCH:            return "BE_SQLITE_MISMATCH";
        case BE_SQLITE_MISUSE:              return "BE_SQLITE_MISUSE";
        case BE_SQLITE_NOLFS:               return "BE_SQLITE_NOLFS";
        case BE_SQLITE_AUTH:                return "BE_SQLITE_AUTH";
        case BE_SQLITE_FORMAT:              return "BE_SQLITE_FORMAT";
        case BE_SQLITE_RANGE:               return "BE_SQLITE_RANGE";
        case BE_SQLITE_NOTADB:              return "BE_SQLITE_NOTADB";
        case BE_SQLITE_ROW:                 return "BE_SQLITE_ROW";
        case BE_SQLITE_DONE:                return "BE_SQLITE_DONE";
        case BE_SQLITE_IOERR_READ:          return "BE_SQLITE_IOERR_READ";
        case BE_SQLITE_IOERR_SHORT_READ:    return "BE_SQLITE_IOERR_SHORT_READ";
        case BE_SQLITE_IOERR_WRITE:         return "BE_SQLITE_IOERR_WRITE";
        case BE_SQLITE_IOERR_FSYNC:         return "BE_SQLITE_IOERR_FSYNC";
        case BE_SQLITE_IOERR_DIR_FSYNC:     return "BE_SQLITE_IOERR_DIR_FSYNC";
        case BE_SQLITE_IOERR_TRUNCATE:      return "BE_SQLITE_IOERR_TRUNCATE";
        case BE_SQLITE_IOERR_FSTAT:         return "BE_SQLITE_IOERR_FSTAT";
        case BE_SQLITE_IOERR_UNLOCK:        return "BE_SQLITE_IOERR_UNLOCK";
        case BE_SQLITE_IOERR_RDLOCK:        return "BE_SQLITE_IOERR_RDLOCK";
        case BE_SQLITE_IOERR_DELETE:        return "BE_SQLITE_IOERR_DELETE";
        case BE_SQLITE_IOERR_BLOCKED:       return "BE_SQLITE_IOERR_BLOCKED";
        case BE_SQLITE_IOERR_NOMEM:         return "BE_SQLITE_IOERR_NOMEM";
        case BE_SQLITE_IOERR_ACCESS:        return "BE_SQLITE_IOERR_ACCESS";
        case BE_SQLITE_IOERR_CHECKRESERVEDLOCK: return "BE_SQLITE_IOERR_CHECKRESERVEDLOCK";
        case BE_SQLITE_IOERR_LOCK:          return "BE_SQLITE_IOERR_LOCK";
        case BE_SQLITE_IOERR_CLOSE:         return "BE_SQLITE_IOERR_CLOSE";
        case BE_SQLITE_IOERR_DIR_CLOSE:     return "BE_SQLITE_IOERR_DIR_CLOSE";
        case BE_SQLITE_IOERR_SHMOPEN:       return "BE_SQLITE_IOERR_SHMOPEN";
        case BE_SQLITE_IOERR_SHMSIZE:       return "BE_SQLITE_IOERR_SHMSIZE";
        case BE_SQLITE_IOERR_SHMLOCK:       return "BE_SQLITE_IOERR_SHMLOCK";
        case BE_SQLITE_LOCKED_SHAREDCACHE:  return "BE_SQLITE_LOCKED_SHAREDCACHE";
        case BE_SQLITE_BUSY_RECOVERY:       return "BE_SQLITE_BUSY_RECOVERY";
        case BE_SQLITE_CANTOPEN_NOTEMPDIR:  return "BE_SQLITE_CANTOPEN_NOTEMPDIR";
        case BE_SQLITE_CANTOPEN_ISDIR:      return "BE_SQLITE_CANTOPEN_ISDIR";
        case BE_SQLITE_CANTOPEN_FULLPATH:   return "BE_SQLITE_CANTOPEN_FULLPATH";
        case BE_SQLITE_CORRUPT_VTAB:        return "BE_SQLITE_CORRUPT_VTAB";
        case BE_SQLITE_ERROR_FileExists:    return "BE_SQLITE_ERROR_FileExists";
        case BE_SQLITE_ERROR_AlreadyOpen:   return "BE_SQLITE_ERROR_AlreadyOpen";
        case BE_SQLITE_ERROR_NoPropertyTable:return "BE_SQLITE_ERROR_NoPropertyTable";
        case BE_SQLITE_ERROR_FileNotFound:   return "BE_SQLITE_ERROR_FileNotFound";
        case BE_SQLITE_ERROR_NoTxnActive:    return "BE_SQLITE_ERROR_NoTxnActive";
        case BE_SQLITE_READONLY_RECOVERY:    return "BE_SQLITE_READONLY_RECOVERY";
        case BE_SQLITE_READONLY_CANTLOCK:    return "BE_SQLITE_READONLY_CANTLOCK";
        case BE_SQLITE_READONLY_ROLLBACK:    return "BE_SQLITE_READONLY_ROLLBACK";
        case BE_SQLITE_ABORT_ROLLBACK:       return "BE_SQLITE_ABORT_ROLLBACK";
        case BE_SQLITE_CONSTRAINT_CHECK:      return "BE_SQLITE_CONSTRAINT_CHECK";
        case BE_SQLITE_CONSTRAINT_COMMITHOOK: return "BE_SQLITE_CONSTRAINT_COMMITHOOK";
        case BE_SQLITE_CONSTRAINT_FOREIGNKEY: return "BE_SQLITE_CONSTRAINT_FOREIGNKEY";
        case BE_SQLITE_CONSTRAINT_FUNCTION:   return "BE_SQLITE_CONSTRAINT_FUNCTION";
        case BE_SQLITE_CONSTRAINT_NOTNULL:    return "BE_SQLITE_CONSTRAINT_NOTNULL";
        case BE_SQLITE_CONSTRAINT_PRIMARYKEY: return "BE_SQLITE_CONSTRAINT_PRIMARYKEY";
        case BE_SQLITE_CONSTRAINT_TRIGGER:    return "BE_SQLITE_CONSTRAINT_TRIGGER";
        case BE_SQLITE_CONSTRAINT_UNIQUE:     return "BE_SQLITE_CONSTRAINT_UNIQUE";
        case BE_SQLITE_CONSTRAINT_VTAB:       return "BE_SQLITE_CONSTRAINT_VTAB";
        }

    return "<unkown result code>";
    }

bool HighPriorityOperationSequencer::s_isHighPriorityRequired;
int HighPriorityOperationSequencer::s_inHighPriorityOperation;
intptr_t HighPriorityOperationSequencer::s_rangeTreeThreadId;
SqlDbP HighPriorityOperationSequencer::s_queryDb;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void HighPriorityOperationSequencer::StartHighPriorityRequired()
    {
    BeAssert (!s_isHighPriorityRequired);
    s_isHighPriorityRequired = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void HighPriorityOperationSequencer::EndHighPriorityRequired()
    {
    BeAssert (s_isHighPriorityRequired);
    s_isHighPriorityRequired = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void HighPriorityOperationSequencer::StartRangeTreeOperation(BeSQLite::Db const& queryDb)
    {
    BeAssert (0 == s_rangeTreeThreadId);
    s_queryDb = queryDb.GetSqlDb();
    s_rangeTreeThreadId = BeThreadUtilities::GetCurrentThreadId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void HighPriorityOperationSequencer::EndRangeTreeOperation()
    {
    BeAssert (0 != s_rangeTreeThreadId);
    s_rangeTreeThreadId = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void HighPriorityOperationSequencer::StartHighPriorityOperation()
    {
    s_inHighPriorityOperation++;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void HighPriorityOperationSequencer::EndHighPriorityOperation()
    {
    BeAssert (s_inHighPriorityOperation > 0);
    s_inHighPriorityOperation--;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
bool HighPriorityOperationSequencer::IsHighPriorityOperationActive()
    {
    return s_inHighPriorityOperation != 0;
    }

//---------------------------------------------------------------------------------------
// The only purpose of this method is to assert in a debug build with an operation is not allowed
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
void HighPriorityOperationSequencer::CheckSQLiteOperationAllowed(SqlDbP queryDb)
    {
#if !defined (NDEBUG)
    if (!s_isHighPriorityRequired || s_inHighPriorityOperation)
        return;

    if (BeThreadUtilities::GetCurrentThreadId() == s_rangeTreeThreadId)
        return;

    if (queryDb != s_queryDb)
        return;

    BeAssert(false);// && "detected an sql operation that may block dynamic update");
#endif
    }

HighPriorityOperationBlock::HighPriorityOperationBlock () { HighPriorityOperationSequencer::StartHighPriorityOperation (); }
HighPriorityOperationBlock::~HighPriorityOperationBlock () { HighPriorityOperationSequencer::EndHighPriorityOperation (); }
RangeTreeOperationBlock::RangeTreeOperationBlock (BeSQLite::Db const&queryDb) { HighPriorityOperationSequencer::StartRangeTreeOperation (queryDb); }
RangeTreeOperationBlock::~RangeTreeOperationBlock () { HighPriorityOperationSequencer::EndRangeTreeOperation (); }
HighPriorityRequired::HighPriorityRequired () { HighPriorityOperationSequencer::StartHighPriorityRequired(); }
HighPriorityRequired::~HighPriorityRequired () { HighPriorityOperationSequencer::EndHighPriorityRequired(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BlobIO::Open (BeSQLiteDbR db, Utf8CP tableName, Utf8CP columnName, uint64_t row, bool writable, Utf8CP dbName)
    {
    return (DbResult) sqlite3_blob_open(db.GetSqlDb(), dbName ? dbName : "main", tableName, columnName, row, writable, &m_blob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BlobIO::Close ()
    {
    SqlDbBlobP blob = m_blob;
    m_blob  = 0;
    return blob ? (DbResult) sqlite3_blob_close(blob) : BE_SQLITE_OK;
    }

DbResult BlobIO::ReOpen (uint64_t row) {return (DbResult) sqlite3_blob_reopen(m_blob, row);}
DbResult BlobIO::Read (void* data, int numBytes, int offset) {return (DbResult) sqlite3_blob_read(m_blob, data, numBytes, offset);}
DbResult BlobIO::Write (const void* data, int numBytes, int offset) {return (DbResult) sqlite3_blob_write(m_blob, data, numBytes, offset);}
int BlobIO::GetNumBytes() const {return sqlite3_blob_bytes (m_blob);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipFromBlob::ZipFromBlob (int32_t blobBufferSize, int32_t zipBufferSize)
    {
    m_stream.zalloc = (alloc_func) NULL;
    m_stream.zfree  = (free_func) NULL;
    m_stream.opaque = (voidpf) NULL;
    inflateInit (&m_stream);

    m_blobBufferSize = blobBufferSize;
    m_unzippedBufferSize = zipBufferSize;
    m_blobData = (Byte*) malloc (blobBufferSize);
    m_unzippedData = (Byte*) malloc (zipBufferSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipFromBlob::~ZipFromBlob()
    {
    Finish();
    inflateEnd (&m_stream);
    free (m_blobData);
    free (m_unzippedData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors ZipFromBlob::ReadFromBlob()
    {
    if (0 == m_blobBytesLeft)
        return  ZIP_ERROR_BLOB_READ_ERROR;

    int32_t sizeToRead = (int32_t) ((m_blobBytesLeft > m_blobBufferSize) ? m_blobBufferSize : m_blobBytesLeft);

    if (BE_SQLITE_OK != m_blobIO.Read(m_blobData, sizeToRead, m_blobOffset))
        {
        m_blobBytesLeft = 0;
        BeAssert (0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    m_blobOffset     += sizeToRead;
    m_blobBytesLeft  -= sizeToRead;
    m_stream.avail_in = sizeToRead;
    m_stream.next_in  = m_blobData;
    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors ZipFromBlob::Init (BeSQLiteDbCR db, Utf8CP tableName, Utf8CP column, int64_t rowId)
    {
    m_blobOffset = 0;
    m_unzippedAvail = 0;

    if (NULL != m_blobIO.GetBlobP())
        {
        if (BE_SQLITE_OK != m_blobIO.ReOpen(rowId))
            {
            m_blobBytesLeft = 0;
            BeAssert (0);
            return  ZIP_ERROR_BLOB_READ_ERROR;
            }
        }
    else if (BE_SQLITE_OK != m_blobIO.Open ((BeSQLiteDbR) db, tableName, column, rowId, 0))
        {
        m_blobBytesLeft = 0;
        BeAssert (0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    inflateReset (&m_stream);
    m_blobBytesLeft = m_blobIO.GetNumBytes();
    return ReadFromBlob();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors ZipFromBlob::ZipRead (Byte* data, uint32_t bufSize, uint32_t& bytesActuallyRead)
    {
    // point output pointer to their buffer
    m_stream.next_out  = (Byte*) data;
    m_stream.avail_out = bufSize;
    bytesActuallyRead = 0;

    // while there's still room in their output buffer, keep inflating
    while (0 != m_stream.avail_out)
        {
        // if there's no more zipped input, read from the blob
        if (0 == m_stream.avail_in)
            {
            // get more data from the blob
            if (ZIP_SUCCESS != ReadFromBlob ())
                {
                bytesActuallyRead = bufSize - m_stream.avail_out;
                return ZIP_ERROR_END_OF_DATA;
                }
            }

        // inflate into their buffer
        int err = inflate (&m_stream, Z_NO_FLUSH);
        if (Z_OK != err)
            {
            // partial read
            bytesActuallyRead = bufSize - m_stream.avail_out;

            /* if we read any, return SUCCESS, otherwise END_OF_DATA */
            if (Z_STREAM_END == err)
                {
                BeAssert(0 < bytesActuallyRead);
                return (0 < bytesActuallyRead) ? ZIP_SUCCESS : ZIP_ERROR_END_OF_DATA;
                }

            /* some other type of error */
            BeAssert(0);
            return ZIP_ERROR_COMPRESSION_ERROR;
            }
        }

    // The caller's read buffer is now full.
    // Report how much was processed, and return SUCCESS to indicate that there is more waiting to be read.
    bytesActuallyRead = bufSize;
    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors ZipFromBlob::Read (Byte* data, uint32_t bufSize, uint32_t& bytesActuallyRead)
    {
    bytesActuallyRead = bufSize;
    while (m_unzippedAvail < bufSize)
        {
        if (m_unzippedAvail > 0)
            {
            memcpy (data, m_unzippedCurr,  m_unzippedAvail);
            bufSize -= m_unzippedAvail;
            data    += m_unzippedAvail;
            }

        m_unzippedCurr = m_unzippedData;
        ZipErrors status = ZipRead (m_unzippedCurr, m_unzippedBufferSize, m_unzippedAvail);
        if (ZIP_SUCCESS != status)
            {
            bytesActuallyRead = 0;
            return  status;
            }
        }

    memcpy (data, m_unzippedCurr, bufSize);
    m_unzippedCurr  += bufSize;
    m_unzippedAvail -= bufSize;
    return  ZIP_SUCCESS;
    }

#define SNAPPY_UNCOMPRESSED_BUFFER_SIZE (34*1024)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromBlob::SnappyFromBlob ()
    {
    m_uncompressed = NULL;
    m_uncompressAvail = 0;

    enum {blobBufferSize = 34*1024, uncompressedSize=SNAPPY_UNCOMPRESSED_BUFFER_SIZE};
    m_blobBufferSize = blobBufferSize;
    m_blobData = (Byte*) malloc (blobBufferSize);
    m_uncompressed = (Byte*) malloc (uncompressedSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromBlob::~SnappyFromBlob()
    {
    Finish();
    free (m_blobData);
    free (m_uncompressed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromBlob::ReadNextChunk()
    {
    BeAssert (0==m_uncompressAvail);

    if (0 == m_blobBytesLeft)
        return  ZIP_ERROR_BLOB_READ_ERROR;

    uint16_t chunkSize;
    if (BE_SQLITE_OK != m_blobIO.Read(&chunkSize, 2, m_blobOffset))
        {
        m_blobBytesLeft = 0;
        BeAssert (0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    BeAssert (chunkSize <= m_blobBytesLeft);

    m_blobOffset += 2;
    m_blobBytesLeft -= 2;
    chunkSize -= 2;

    if (BE_SQLITE_OK != m_blobIO.Read(m_blobData, chunkSize, m_blobOffset))
        {
        m_blobBytesLeft = 0;
        BeAssert (0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    m_blobOffset     += chunkSize;
    m_blobBytesLeft  -= chunkSize;

    size_t uncompressSize;
    bool bstat = snappy::GetUncompressedLength((char const*) m_blobData, chunkSize, &uncompressSize);
    BeAssert (uncompressSize<=SNAPPY_UNCOMPRESSED_BUFFER_SIZE && uncompressSize>0);
    BeAssert (bstat);

    bstat = snappy::RawUncompress ((char const*) m_blobData, chunkSize, (char*) m_uncompressed);
    BeAssert (bstat);

    m_uncompressCurr = m_uncompressed;
    m_uncompressAvail = (uint16_t) uncompressSize;
    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromBlob::Init (BeSQLiteDbCR db, Utf8CP tableName, Utf8CP column, int64_t rowId)
    {
    m_blobOffset = 0;
    m_uncompressAvail = 0;

    if (NULL != m_blobIO.GetBlobP())
        {
        if (BE_SQLITE_OK != m_blobIO.ReOpen(rowId))
            {
            m_blobBytesLeft = 0;
            BeAssert (0);
            return  ZIP_ERROR_BLOB_READ_ERROR;
            }
        }
    else if (BE_SQLITE_OK != m_blobIO.Open ((BeSQLiteDbR) db, tableName, column, rowId, 0))
        {
        m_blobBytesLeft = 0;
        BeAssert (0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    m_blobBytesLeft = m_blobIO.GetNumBytes();
    return ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromBlob::_Read (Byte* data, uint32_t bufSize, uint32_t& bytesActuallyRead)
    {
    // point output pointer to their buffer
    bytesActuallyRead = 0;

    // while there's still room in their output buffer, keep inflating
    while (0 != bufSize)
        {
        // if there's no more uncompressed data, read from the blob
        if (0 >= m_uncompressAvail)
            {
            // get more data from the blob
            if (ZIP_SUCCESS != ReadNextChunk ())
                return ZIP_ERROR_COMPRESSION_ERROR;
            }

        uint32_t readSize = (bufSize > m_uncompressAvail) ? m_uncompressAvail : bufSize;

        memcpy (data, m_uncompressCurr, readSize);
        data += readSize;
        m_uncompressCurr += readSize;
        bytesActuallyRead += readSize;
        bufSize -= readSize;
        m_uncompressAvail -= readSize;
        }

    return  ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2012
//--------------+------------------------------------------------------------------------
uint32_t SnappyFromMemory::GetUncompressedBufferSize()
    {
    return SNAPPY_UNCOMPRESSED_BUFFER_SIZE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromMemory::SnappyFromMemory (void*uncompressedBuffer, uint32_t uncompressedBufferSize)
    {
    BeAssert (SNAPPY_UNCOMPRESSED_BUFFER_SIZE == uncompressedBufferSize);

    m_uncompressed = (Byte*)uncompressedBuffer;
    m_uncompressAvail = 0;
    m_uncompressSize = uncompressedBufferSize;

    m_blobData = NULL;
    m_blobOffset = 0;
    m_blobBytesLeft = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyFromMemory::Init (void*blobBuffer, uint32_t blobBufferSize)
    {
    m_uncompressAvail = 0;

    m_blobData = (Byte*) blobBuffer;
    m_blobOffset = 0;
    m_blobBytesLeft = blobBufferSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromMemory::TransferFromBlob (void* data, uint32_t numBytes, int offset)
    {
    //  Verify that it does not go beyond the end of the buffer
    memcpy(data, m_blobData + offset, numBytes);
    return ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromMemory::ReadNextChunk()
    {
    BeAssert (0==m_uncompressAvail);

    if (0 == m_blobBytesLeft)
        return  ZIP_ERROR_BLOB_READ_ERROR;

    uint16_t chunkSize;
    if (ZIP_SUCCESS != TransferFromBlob(&chunkSize, 2, m_blobOffset))
        {
        m_blobBytesLeft = 0;
        BeAssert (0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    BeAssert (chunkSize <= m_blobBytesLeft);

    m_blobOffset += 2;
    m_blobBytesLeft -= 2;
    chunkSize -= 2;

    Byte*   currentData = m_blobData + m_blobOffset;

    m_blobOffset     += chunkSize;
    m_blobBytesLeft  -= chunkSize;

    size_t uncompressSize;
    bool bstat = snappy::GetUncompressedLength((char const*) currentData, chunkSize, &uncompressSize);
    BeAssert (uncompressSize<=32*1024 && uncompressSize>0);
    BeAssert (bstat);

    bstat = snappy::RawUncompress ((char const*) currentData, chunkSize, (char*) m_uncompressed);
    BeAssert (bstat);

    m_uncompressCurr = m_uncompressed;
    m_uncompressAvail = (uint16_t) uncompressSize;
    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/12
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromMemory::_Read (Byte* data, uint32_t bufSize, uint32_t& bytesActuallyRead)
    {
    // point output pointer to their buffer
    bytesActuallyRead = 0;

    // while there's still room in their output buffer, keep inflating
    while (0 != bufSize)
        {
        // if there's no more uncompressed data, read from the blob
        if (0 >= m_uncompressAvail)
            {
            // get more data from the blob
            if (ZIP_SUCCESS != ReadNextChunk ())
                return ZIP_ERROR_COMPRESSION_ERROR;
            }

        uint32_t readSize = (bufSize > m_uncompressAvail) ? m_uncompressAvail : bufSize;

        memcpy (data, m_uncompressCurr, readSize);
        data += readSize;
        m_uncompressCurr += readSize;
        bytesActuallyRead += readSize;
        bufSize -= readSize;
        m_uncompressAvail -= readSize;
        }

    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ZipToBlob::UseChunk (uint32_t chunkNumber)
    {
    if (m_chunks.size() <= chunkNumber)
        m_chunks.push_back(new ZippedChunk(m_chunkSize));

    ZippedChunk* chunk = m_chunks[chunkNumber];
    chunk->m_bytesUsed = 0;
    m_stream.next_out  = chunk->m_data;
    m_stream.avail_out = m_chunkSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipToBlob::ZipToBlob (int bufsize, int compressionLevel)
    {
    m_currChunk = 0;
    m_unzippedSize = 0;
    m_chunkSize = bufsize;
    m_stream.zalloc = (alloc_func) NULL;
    m_stream.zfree  = (free_func) NULL;
    m_stream.opaque = (voidpf) NULL;

    deflateInit(&m_stream, compressionLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ZipToBlob::FreeMemory()
    {
    FOR_EACH (ZippedChunk* chunk, m_chunks)
        delete chunk;

    m_chunks.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipToBlob::~ZipToBlob ()
    {
    deflateEnd (&m_stream);
    FreeMemory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ZipToBlob::GetCompressedSize()
    {
    uint32_t total = 0;
    for (uint32_t i=0; i<=m_currChunk; ++i)
        total += m_chunks[i]->m_bytesUsed;
    return  total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ZipToBlob::Init()
    {
    m_unzippedSize = 0;

    UseChunk (m_currChunk = 0);
    deflateReset(&m_stream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ZipToBlob::ResetCompressionLevel(int level)
    {
    Init();
    deflateParams (&m_stream, level, Z_DEFAULT_STRATEGY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors ZipToBlob::Write (ByteCP data, uint32_t size)
    {
    m_stream.next_in  = (Byte*) data;
    m_stream.avail_in = size;
    m_unzippedSize += size;

    // while there's still data in their buffer, continue compressing
    while (m_stream.avail_in != 0)
        {
        // is our output buffer is full?
        if (m_stream.avail_out == 0)
            {
            m_chunks[m_currChunk]->m_bytesUsed = m_chunkSize;
            UseChunk(++m_currChunk);
            }

        // compress input buffer into internal buffer
        if (Z_OK != deflate (&m_stream, Z_NO_FLUSH))
            {
            BeAssert(0);
            return  ZIP_ERROR_COMPRESSION_ERROR;
            }
        }

    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors ZipToBlob::Finish()
    {
    while (true)
        {
        if (0 == m_stream.avail_out)
            {
            m_chunks[m_currChunk]->m_bytesUsed = m_chunkSize;
            UseChunk(++m_currChunk);
            }

        int status = deflate (&m_stream, Z_FINISH);

        switch (status)
            {
            case Z_STREAM_END:
                m_chunks[m_currChunk]->m_bytesUsed = (m_chunkSize-m_stream.avail_out);
                return ZIP_SUCCESS;

            case Z_OK:
                break;

            default:
                BeAssert (false);
                return  ZIP_ERROR_COMPRESSION_ERROR;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ZipToBlob::SaveToRow (BeSQLiteDbR db, Utf8CP tableName, Utf8CP column, int64_t rowId)
    {
    BlobIO blobIO;
    StatusInt status = blobIO.Open (db, tableName, column, rowId, true);
    if (BE_SQLITE_OK != status)
        {
        BeAssert (0);
        return  ERROR;
        }

    int offset = 0;
    for (uint32_t i=0; i<= m_currChunk; ++i)
        {
        status = blobIO.Write(m_chunks[i]->m_data, m_chunks[i]->m_bytesUsed, offset);
        if (BE_SQLITE_OK != status)
            {
            BeAssert (0);
            break;
            }
        offset += m_chunks[i]->m_bytesUsed;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyToBlob::SnappyToBlob ()
    {
    enum {bufsize = 32*1024};
    m_rawBuf = (Byte*) malloc (bufsize);
    m_rawSize = bufsize;
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyToBlob::Init()
    {
    m_unsnappedSize = m_rawCurr = 0;
    m_rawAvail = m_rawSize;
    m_currChunk = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyToBlob::~SnappyToBlob()
    {
    FOR_EACH (SnappyChunk* chunk, m_chunks)
        delete chunk;

    m_chunks.clear();
    free(m_rawBuf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyToBlob::Write (ByteCP data, uint32_t inSize)
    {
    m_unsnappedSize += inSize;
    while (inSize>0)
        {
        uint32_t thisSize = (inSize>m_rawAvail) ? m_rawAvail : inSize;
        memcpy (m_rawBuf+m_rawCurr, data, thisSize);

        inSize -= thisSize;
        data   += thisSize;

        m_rawCurr  += thisSize;
        m_rawAvail -= thisSize;

        if (0 >= m_rawAvail)
            Finish();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SnappyToBlob::GetCompressedSize()
    {
    Finish();
    uint32_t total = 0;

    for (uint32_t i=0; i<m_currChunk; ++i)
        total += m_chunks[i]->GetChunkSize();

    return  total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyToBlob::Finish()
    {
    if (0 == m_rawCurr)
        return;

    if (m_chunks.size() <= m_currChunk)
        m_chunks.push_back(new SnappyChunk((uint32_t) snappy::MaxCompressedLength(m_rawSize)));

    unsigned int compressedBytes;
    snappy::RawCompress ((char const*) m_rawBuf, m_rawCurr, (char*) &m_chunks[m_currChunk]->m_data[1], &compressedBytes);
    BeAssert ((compressedBytes+2) < 64*1024);
    m_chunks[m_currChunk]->m_data[0] = (uint16_t) compressedBytes + 2; // add 2 because compressed data starts 2 bytes into buffer

    ++m_currChunk;
    m_rawCurr = 0;
    m_rawAvail = m_rawSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SnappyToBlob::SaveToRow (BeSQLiteDbR db, Utf8CP tableName, Utf8CP column, int64_t rowId)
    {
    Finish();

    BlobIO blobIO;
    StatusInt status = blobIO.Open (db, tableName, column, rowId, true);
    if (BE_SQLITE_OK != status)
        {
        BeAssert (0);
        return  ERROR;
        }

    int offset = 0;
    for (uint32_t i=0; i< m_currChunk; ++i)
        {
        status = blobIO.Write(m_chunks[i]->m_data, m_chunks[i]->GetChunkSize(), offset);
        if (BE_SQLITE_OK != status)
            {
            BeAssert (0);
            break;
            }
        offset += m_chunks[i]->GetChunkSize();
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeDbMutex::BeDbMutex(MutexType mutexType) {m_mux = sqlite3_mutex_alloc (mutexType);}
BeDbMutex::~BeDbMutex()   {sqlite3_mutex_free((sqlite3_mutex*)m_mux);}
void BeDbMutex::Enter() {sqlite3_mutex_enter((sqlite3_mutex*)m_mux);}
void BeDbMutex::Leave() {sqlite3_mutex_leave((sqlite3_mutex*)m_mux);}

#ifndef NDEBUG
bool BeDbMutex::IsHeld() {return 0!=sqlite3_mutex_held((sqlite3_mutex*)m_mux);}
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatement::CachedStatement (Utf8CP sql)
    {
    m_refCount = 0;
    size_t len = strlen(sql) + 1;
    m_sql = (Utf8P) sqlite3_malloc((int)len);
    memcpy ((char*)m_sql, sql, len);
    }

CachedStatement::~CachedStatement () {sqlite3_free((void*)m_sql);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t CachedStatement::Release()
    {
    if (0 == --m_refCount)
        {
        delete this;
        return  0;
        }

    // SharedStatements are always held in a StatementCache, so the ref count will be 1 if no
    // one else is pointing to this instance. That means that the statement is no longer in use and
    // we should reset it so sqlite won't keep it in the list of active vdbe's. Also, clear its bindings so
    // the next user won't accidentally inherit them.
    if (1== m_refCount)
        {
        Reset();
        ClearBindings();
        }

    return  m_refCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void StatementCache::Empty()
    {
    BeDbMutexHolder _v_v(m_mutex);
    m_entries.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void StatementCache::Dump()
    {
    for (bvector<CachedStatementPtr>::iterator it=m_entries.begin(); it<m_entries.end(); ++it)
        {
        printf ("%s\n", (*it)->GetSQL());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CachedStatementPtr>::iterator StatementCache::FindEntry(Utf8CP sql) const
    {
    for (bvector<CachedStatementPtr>::iterator it=m_entries.begin(), end=m_entries.end(); it<end; ++it)
        {
        if (0==strcmp((*it)->GetSQL(), sql))
            {
            if (1 < (*it)->GetRefCount()) // this statement is currently in use, we can't share it
                continue;

            return  it;
            }
        }

    return  m_entries.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatement& StatementCache::AddStatement (Utf8CP sql) const
    {
    BeDbMutexHolder _v_v(m_mutex);

    if (m_entries.size() >= m_entries.capacity()) // if cache is full, remove oldest entry
        m_entries.erase (m_entries.begin());

    CachedStatement* newEntry = new CachedStatement(sql);
    m_entries.push_back (newEntry);
    return  *newEntry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult StatementCache::GetPreparedStatement(CachedStatementPtr& stmt, DbFile const& dbFile, Utf8CP sqlString) const
    {
    if (!dbFile.CheckImplicitTxn())
        {
        BeAssert (false);
        return  BE_SQLITE_ERROR_NoTxnActive;
        }

    stmt = FindStatement (sqlString);
    if (stmt.IsValid())
        {
        //  We normally test for this in DoPrepare, but cached statements bypass that.
        HPOS_CheckSQLiteOperationAllowed(dbFile.GetSqlDb());
        return  BE_SQLITE_OK;
        }

    stmt = &AddStatement (sqlString);
    return stmt->Prepare (dbFile, sqlString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatement* StatementCache::FindStatement (Utf8CP sql) const
    {
    BeDbMutexHolder _v_v(m_mutex);

    bvector<CachedStatementPtr>::iterator entry = FindEntry (sql);
    return (entry != m_entries.end()) ? entry->get(): NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
StatementCache& Db::GetStatementCache() const
    {
    if (NULL == m_statements)
        m_statements = new StatementCache(20);

    return  *m_statements;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::GetCachedStatement (CachedStatementPtr& stmt, Utf8CP sqlString) const
    {
    return GetStatementCache().GetPreparedStatement(stmt, *m_dbFile.get(), sqlString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void NamedParams::Bind(Statement& stmt) const
    {
    for (auto param : m_strParams)
        stmt.BindText (stmt.GetParameterIndex(param.m_name.c_str()), param.m_value, Statement::MAKE_COPY_No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::DumpSqlResults (Utf8CP sql)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(*this, sql))
        return;

    stmt.DumpResults();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbTableIterator& DbTableIterator::operator=(DbTableIterator const& rhs)
    {
    m_stmt = 0;
    m_whereClause = rhs.m_whereClause;
    m_db = rhs.m_db;
    return  *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbTableIterator::DbTableIterator(BeSQLiteDbCR db, Utf8CP whereClause) : m_db((BeSQLiteDbP)&db), m_params(NULL)
    {
    if (NULL == whereClause)
        return;

    m_whereClause.assign(whereClause);
    m_whereClause.Trim();
    if (0 == strncmp(m_whereClause.c_str(), "WHERE ", 6))
        m_whereClause.erase(0,6);
    else if (0 == strncmp(m_whereClause.c_str(), "AND ", 4))
        m_whereClause.erase(0,4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbTableIterator::MakeSqlString(Utf8CP sql, bool hasWhere) const
    {
    Utf8String sqlString(sql);

    if (!m_whereClause.empty())
        {
        if (0 == strncmp(m_whereClause.c_str(), "ORDER BY", 8))
            sqlString.append (" ");
        else
            sqlString.append(hasWhere ? " AND " : " WHERE ");

        sqlString.append(m_whereClause);
        }

    return sqlString;
    }

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct PropertyBlobInStream : ILzmaInputStream
{
private:
    Db&                 m_db;
    BeRepositoryBasedId m_id;
    Byte*               m_buffer;
    uint32_t            m_bufferSize;
    uint32_t            m_nBytesInBuffer;
    uint32_t            m_bufferOffset;
    uint32_t            m_nextChunk;

public:
    PropertyBlobInStream(Db&db, BeRepositoryBasedId id) : m_db(db), m_id(id), m_buffer(NULL), m_bufferSize(0), m_nBytesInBuffer(0), m_bufferOffset(0), m_nextChunk(0) {}
    ~PropertyBlobInStream() { free (m_buffer); }

    //  The LZMA2 multithreading ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    ZipErrors _Read (void* data, uint32_t size, uint32_t& actuallyRead) override
        {
        for (actuallyRead = 0; actuallyRead < size; )
            {
            if (m_nBytesInBuffer == m_bufferOffset)
                {
                m_bufferOffset = 0;
                DbResult rc = m_db.QueryPropertySize (m_nBytesInBuffer, Properties::EmbeddedFileBlob(), m_id.GetValue(), m_nextChunk);
                if (rc != BE_SQLITE_ROW)
                    return ZIP_ERROR_BAD_DATA;

                if (m_nBytesInBuffer > m_bufferSize)
                    {
                    m_bufferSize = m_nBytesInBuffer;
                    m_buffer = (Byte*)realloc(m_buffer, m_bufferSize);
                    }

                rc = m_db.QueryProperty (m_buffer, m_nBytesInBuffer, Properties::EmbeddedFileBlob(), m_id.GetValue(), m_nextChunk++);
                if (rc != BE_SQLITE_ROW)
                    return ZIP_ERROR_BAD_DATA;
                }

            uint32_t bytesToTransfer = std::min(m_nBytesInBuffer - m_bufferOffset, size - actuallyRead);
            memcpy ((Byte*)data + actuallyRead, m_buffer + m_bufferOffset, bytesToTransfer);
            actuallyRead += bytesToTransfer;
            m_bufferOffset += bytesToTransfer;
            }

        return ZIP_SUCCESS;
        }

    //  This is not needed since PropertyBlobInStream is only used for decompressing.  It is the LZMA compression code that needs it.
    uint64_t _GetSize() override {BeAssert (false && "PropertyBlobInStream::_GetSize not implemented");  return 0;}
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct  PropertyBlobOutStream : ILzmaOutputStream
{
private:
    Db&                 m_db;
    BeRepositoryBasedId m_id;
    bvector<Byte>       m_buffer;
    uint32_t            m_chunkSize;
    uint32_t            m_bufferOffset;
    uint32_t            m_nextChunk;
    bool                m_alwaysFlush;

public:
    DbResult Flush()
        {
        if (0 == m_bufferOffset)
            return BE_SQLITE_OK;

        DbResult rc = m_db.SaveProperty (Properties::EmbeddedFileBlob(), m_buffer.data(), m_bufferOffset, m_id.GetValue(), m_nextChunk++);
        if (BE_SQLITE_OK != rc)
            LOG.errorv("PropertyBlobOutStream::Flush is returning %d", rc);
        m_bufferOffset = 0;
        return rc;
        }

    PropertyBlobOutStream(Db&db, BeRepositoryBasedId id, uint64_t chunkSize) : m_db(db), m_id(id), m_chunkSize(static_cast <uint32_t> (chunkSize)), m_bufferOffset(0), m_nextChunk(0), m_alwaysFlush(false)
        {
        m_buffer.resize(m_chunkSize);
        }

    //  Setting m_alwaysFlush to true force PropertyBlobOutStream to create a new embedded blob for each write. Since LZMA2 calls
    //  _Write whenever it finishes processing a a block this creates a one-to-one mapping from input blocks to embedded blobs.
    //  That makes it possible to read and expand any given block, randomly accessing the blocks.
    void _SetAlwaysFlush(bool flushOnEveryWrite) override
        {
        m_alwaysFlush = flushOnEveryWrite;
        Flush();
        }

    ~PropertyBlobOutStream()
        {
        //  We should always flush and check that status, so m_bufferOffset should be 0.
        BeAssert(0 == m_bufferOffset);
        Flush();
        }

    ZipErrors _Write (void const* data, uint32_t size, uint32_t& bytesWritten) override
        {
        bytesWritten = 0;
        while (bytesWritten < size)
            {
            uint32_t bytesToTransfer = size;  //  Assume chunk size applies to input.
            if (!m_alwaysFlush)
                //  Chunk size applies to output.
                bytesToTransfer = std::min(m_chunkSize - m_bufferOffset, size - bytesWritten);

            if (bytesToTransfer > 0)
                memcpy(m_buffer.data() + m_bufferOffset, (CharCP)data + bytesWritten, bytesToTransfer);

            m_bufferOffset += bytesToTransfer;
            if (m_alwaysFlush)
                {
                if (BE_SQLITE_OK != Flush())
                    return ZIP_ERROR_WRITE_ERROR;
                }
            else if (m_bufferOffset == m_chunkSize)
                {
                if (BE_SQLITE_OK != Flush())
                    return ZIP_ERROR_WRITE_ERROR;
                }
            bytesWritten += bytesToTransfer;
            }

        return ZIP_SUCCESS;
        }
};

#define EMBEDDED_LZMA_MARKER   "EmLzma"
//=======================================================================================
// The first EmbeddedFileBlob for an embedded file starts with this structure.  If
// m_compressionType is NO_COMPRESSION then the image is not compressed.
// @bsiclass                                                    John.Gooding    03/2013
//=======================================================================================
struct   EmbeddedLzmaHeader
{
private:
    uint16_t m_sizeOfHeader;
    char    m_idString [10];
    uint16_t m_formatVersionNumber;
    uint16_t m_compressionType;

public:
    static const int formatVersionNumber = 0x10;
    enum CompressionType
        {
        NO_COMPRESSION  =       0,
        LZMA2           =       2
        };

    EmbeddedLzmaHeader (CompressionType compressionType)
        {
        CharCP idString = EMBEDDED_LZMA_MARKER;
        BeAssert ((strlen (idString)+ 1) <= sizeof(m_idString));
        memset (this, 0, sizeof (*this));
        m_sizeOfHeader = (uint16_t)sizeof (EmbeddedLzmaHeader);
        strcpy (m_idString, idString);
        m_compressionType = compressionType;
        m_formatVersionNumber = formatVersionNumber;
        }

    int GetVersion () {return m_formatVersionNumber;}
    bool IsLzma2 () {return LZMA2 == m_compressionType;}
    bool IsUncompressed () {return NO_COMPRESSION == m_compressionType;}
    bool IsValid ()
        {
        if (m_sizeOfHeader != sizeof(EmbeddedLzmaHeader))
            return false;

        if (strcmp(m_idString, EMBEDDED_LZMA_MARKER))
            return false;

        if (formatVersionNumber != m_formatVersionNumber)
            return false;

        return m_compressionType == LZMA2 || m_compressionType == NO_COMPRESSION;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbEmbeddedFileTable& Db::EmbeddedFiles() {return m_embeddedFiles;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void removeFileBlobs (Db& db, BeRepositoryBasedId id)
    {
    Statement stmt;
    DbResult rc = stmt.Prepare (db, "DELETE FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=? AND Id=?");
    stmt.BindText(1, Properties::EmbeddedFileBlob().GetNamespace(), Statement::MAKE_COPY_No);
    stmt.BindText(2, Properties::EmbeddedFileBlob().GetName(), Statement::MAKE_COPY_No);
    stmt.BindId (3, id);
    rc = stmt.Step();
    }

//---------------------------------------------------------------------------------------
//  This is based on LZMA2_DIC_SIZE_FROM_PROP.  This is the size of the dictionary
//  they allocate when decompressing.  It makes sense for us to use same
//  boundaries as the LZMA library.
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static uint32_t getDictionarySize (uint32_t dictionarySize)
    {
    for (unsigned i = 0; i < 40; i++)
        {
        uint32_t computed = (2 | (i & 1)) << (i / 2 + 11);
        if (dictionarySize <= computed)
            return computed;
        }

    return dictionarySize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
static DbResult compressAndEmbedFileImage (Db& db, uint32_t& chunkSize, BeRepositoryBasedId id, void const* data, uint32_t const size, bool supportRandomAccess)
    {
    if (supportRandomAccess)
        chunkSize = getDictionarySize (chunkSize);

    BeFileLzmaInFromMemory inStream (data, size);
    PropertyBlobOutStream outStream (db, id, chunkSize);

    uint32_t dictionarySize = std::min (size, chunkSize);

    LzmaEncoder encoder (dictionarySize);

    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t bytesWritten;
    outStream._Write (&header, sizeof (header), bytesWritten);
    if (bytesWritten != sizeof (header))
        return BE_SQLITE_IOERR;

    if (encoder.Compress (outStream, inStream, NULL, false) != ZIP_SUCCESS)
        return BE_SQLITE_IOERR;

    DbResult rc = outStream.Flush ();

    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2015
//---------------------------------------------------------------------------------------
static DbResult translateZipErrorToSQLiteError(ZipErrors zipError)
    {
    switch (zipError)
        {
        case ZIP_SUCCESS:
            return BE_SQLITE_OK;
        case ZIP_ERROR_READ_ERROR:
            return BE_SQLITE_IOERR_READ;
        case ZIP_ERROR_WRITE_ERROR:
            return BE_SQLITE_IOERR_WRITE;
        case ZIP_ERROR_ABORTED:
            return BE_SQLITE_ABORT;
        case ZIP_ERROR_END_OF_DATA:
            return BE_SQLITE_IOERR_SHORT_READ;
        }

    return BE_SQLITE_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult compressAndEmbedFile (Db& db, uint64_t& filesize, uint32_t& chunkSize, BeRepositoryBasedId id, Utf8CP filespec, bool supportRandomAccess)
    {
    if (supportRandomAccess)
        chunkSize = getDictionarySize (chunkSize);

    BeFileLzmaInStream inStream;
    PropertyBlobOutStream outStream (db, id, chunkSize);

    WString filespecW (filespec, true);
    BeFileName filename (filespecW.c_str ());

    if (inStream.OpenInputFile (filename) != BSISUCCESS)
        return BE_SQLITE_ERROR_FileNotFound;

    filesize = inStream._GetSize ();

    //  If SQLite has a reserved, pending, or exclusive lock on this file we will get a SharingViolation trying to read the 512 bytes starting at 1meg.  
    //  Try reading them here rather than forcing the user to wait until one million bytes have been processed.
    BeFile& inputFile = inStream.GetBeFile();
    if (inputFile.SetPointer(SQLITE_PENDING_BYTE, BeFileSeekOrigin::Begin) == BeFileStatus::Success)
        {
        uint8_t buffer[512];
        if (inputFile.Read(buffer, nullptr, sizeof (buffer)) == BeFileStatus::SharingViolationError)
            {
            LOG.errorv("embedFile got a SharingViolationError at 1meg. Apparently SQLite has this file locked.");
            return BE_SQLITE_BUSY;
            }
        } 

    inputFile.SetPointer(0, BeFileSeekOrigin::Begin);
    uint32_t dictionarySize = static_cast <uint32_t> (std::min (filesize, (uint64_t) chunkSize));

    LzmaEncoder encoder (dictionarySize);
    if (supportRandomAccess)
        //  Forces LZMA to process input in chunks that correspond to block size.
        encoder.SetBlockSize (dictionarySize);

    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t bytesWritten;
    outStream._Write (&header, sizeof (header), bytesWritten);
    if (bytesWritten != sizeof (header))
        return BE_SQLITE_IOERR;

    ZipErrors compressResult = encoder.Compress (outStream, inStream, NULL, supportRandomAccess);
    if (compressResult != ZIP_SUCCESS)
        {
        LOG.errorv("LzmaEncoder::Compress returned %d", compressResult);

        return translateZipErrorToSQLiteError(compressResult);
        }

    if (inStream.GetBytesRead() != filesize)
        {
        LOG.errorv("LzmaEncoder::Compress succeeded but read the wrong number of bytes: expected %lld, actual %lld", filesize, inStream.GetBytesRead());
        return BE_SQLITE_IOERR;
        }

    DbResult rc = outStream.Flush ();

    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
static DbResult embedFileImageWithoutCompressing (Db& db, void const*data, uint64_t size, uint32_t chunkSize, BeRepositoryBasedId id)
    {
    int32_t nChunks=0;
    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::NO_COMPRESSION);
    DbResult rc = db.SaveProperty (Properties::EmbeddedFileBlob(), &header, sizeof(header), id.GetValue(), nChunks++);
    if (rc != BE_SQLITE_OK)
        return  rc;

    for (int64_t nLeft=(int64_t)size; nLeft>0; nLeft-=chunkSize)
        {
        uint32_t thisSize = (uint32_t) std::min ((int64_t) chunkSize, nLeft);

        //  This relies on EmbeddedFileBlob returning PropertySpec::COMPRESS_PROPERTY_No to prevent trying
        //  to compress the blobs
        rc = db.SaveProperty (Properties::EmbeddedFileBlob(), data, thisSize, id.GetValue(), nChunks++);
        if (rc != BE_SQLITE_OK)
            return  rc;

        data  = (char*) data + thisSize;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
static DbResult embedFileWithoutCompressing (Db& db, uint64_t&filesize, uint32_t chunkSize, BeRepositoryBasedId id, Utf8CP filespec)
    {
    WString filespecW (filespec, true);
    BeFileName filename (filespecW.c_str ());
    BeFile      inputFile;

    if (inputFile.Open(filename, BeFileAccess::Read) != BeFileStatus::Success)
        return BE_SQLITE_ERROR_FileNotFound;

    int32_t nChunks=0;
    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::NO_COMPRESSION);
    DbResult rc = db.SaveProperty (Properties::EmbeddedFileBlob(), &header, (uint32_t)sizeof(header), id.GetValue(), nChunks++);
    if (rc != BE_SQLITE_OK)
        return  rc;

    filename.GetFileSize(filesize);
    bvector<Byte>    buffer;
    buffer.resize(chunkSize);

    while(true)
        {
        uint32_t bytesRead;
        inputFile.Read(&buffer[0], &bytesRead, chunkSize);
        if (0 == bytesRead)
            break;

        rc = db.SaveProperty (Properties::EmbeddedFileBlob(), &buffer[0], bytesRead, id.GetValue(), nChunks++);
        if (rc != BE_SQLITE_OK)
            return  rc;
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult addEmbedFile (Db& db, Utf8CP name, Utf8CP type, Utf8CP description, BeRepositoryBasedId id, uint64_t fileSize, DateTime const* lastModified, uint32_t chunkSize)
    {
    const bool hasLastModifiedColumn = DbEmbeddedFileTable::HasLastModifiedColumn (db);

    Statement stmt;
    stmt.Prepare (db, hasLastModifiedColumn ?
                  "INSERT INTO " BEDB_TABLE_EmbeddedFile " (Name,Descr,Type,Id,Size,Chunk,LastModified) VALUES(?,?,?,?,?,?,?)" :
                  "INSERT INTO " BEDB_TABLE_EmbeddedFile " (Name,Descr,Type,Id,Size,Chunk) VALUES(?,?,?,?,?,?)");
    stmt.BindText (1, name, Statement::MAKE_COPY_No);
    stmt.BindText (2, description, Statement::MAKE_COPY_No);
    stmt.BindText (3, type, Statement::MAKE_COPY_No);
    stmt.BindId (4, id);
    stmt.BindInt64 (5, fileSize);
    stmt.BindInt (6, chunkSize);
    if (hasLastModifiedColumn && lastModified != nullptr && lastModified->IsValid ())
        {
        if (lastModified->GetInfo ().GetKind () == DateTime::Kind::Local)
            LOG.warningv ("LastModified date of file '%s' to embed must not be in local time. Only DateTime::Kind::Utc or DateTime::Kind::Unspecified is supported. LastModified will be ignored.",
                          name);
        else
            {
            double lastModifiedJulianDay = 0.0;
            lastModified->ToJulianDay (lastModifiedJulianDay);
            stmt.BindDouble (7, lastModifiedJulianDay);
            }
        }

    DbResult rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeRepositoryBasedId DbEmbeddedFileTable::GetNextEmbedFileId () const
    {
    if (BE_SQLITE_OK != BeSQLiteProfileManager::UpgradeProfile (m_db))
        return BeRepositoryBasedId ();

    BeAssert (m_db.TableExists (BEDB_TABLE_EmbeddedFile));
    BeRepositoryBasedId id;
    m_db.GetNextRepositoryBasedId (id, BEDB_TABLE_EmbeddedFile, "Id");
    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2015
//---------------------------------------------------------------------------------------
BeRepositoryBasedId DbEmbeddedFileTable::ImportDbFile (DbResult& stat, Utf8CP name, Utf8CP filespec, Utf8CP type, Utf8CP description, DateTime const* lastModified, uint32_t chunkSize, bool supportRandomAccess)
    {
    Utf8CP vfs = 0;
    stat = isValidDbFile(filespec, vfs);
    //  We aren't going to use the VFS to read it so it better not require a VFS.
    if (BE_SQLITE_OK != stat || nullptr != vfs)
        {
        LOG.errorv("ImportDbFile: isValidDbFile reported error %d", stat);
        return BeRepositoryBasedId();
        }

    return Import(&stat, name, filespec, type, description, lastModified, chunkSize, supportRandomAccess);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
BeRepositoryBasedId DbEmbeddedFileTable::ImportWithoutCompressing (DbResult* stat, Utf8CP name, Utf8CP filespec, Utf8CP type, Utf8CP description, DateTime const* lastModified, uint32_t chunkSize)
    {
    BeAssert (m_db.IsTransactionActive ());

    // make sure name is unique before continuing
    BeRepositoryBasedId fileId = QueryFile (name);
    if (fileId.IsValid ())
        {
        if (stat != nullptr)
            *stat = BE_SQLITE_CONSTRAINT_BASE;

        return BeRepositoryBasedId ();
        }

    BeRepositoryBasedId newId = GetNextEmbedFileId ();

    uint64_t fileSize;
    DbResult rc = embedFileWithoutCompressing (m_db, fileSize, chunkSize, newId, filespec);
    if (BE_SQLITE_OK == rc)
        rc = addEmbedFile (m_db, name, type, description, newId, fileSize, lastModified, chunkSize);

    if (stat != nullptr)
        *stat = rc;

    if (rc == BE_SQLITE_OK)
        return newId;
    else
        return BeRepositoryBasedId ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   10/14
//+---------------+---------------+---------------+---------------+---------------+------
BeRepositoryBasedId DbEmbeddedFileTable::Import (DbResult* stat, Utf8CP name, Utf8CP filespec, Utf8CP type, Utf8CP description, DateTime const* lastModified, uint32_t chunkSize, bool supportRandomAccess)
    {
    BeAssert (m_db.IsTransactionActive ());

    // make sure name is unique before continuing
    BeRepositoryBasedId fileId = QueryFile (name);
    if (fileId.IsValid ())
        {
        if (stat != nullptr)
            *stat = BE_SQLITE_CONSTRAINT_BASE;

        return BeRepositoryBasedId ();
        }

    BeRepositoryBasedId newId = GetNextEmbedFileId ();

    uint64_t fileSize;
    DbResult rc = compressAndEmbedFile (m_db, fileSize, chunkSize, newId, filespec, supportRandomAccess);
    if (BE_SQLITE_OK == rc)
        rc = addEmbedFile (m_db, name, type, description, newId, fileSize, lastModified, chunkSize);

    if (stat != nullptr)
        *stat = rc;

    if (rc == BE_SQLITE_OK)
        return newId;
    else
        return BeRepositoryBasedId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::AddEntry (Utf8CP name, Utf8CP type, Utf8CP description, DateTime const* lastModified)
    {
    return addEmbedFile (m_db, name, type, description, GetNextEmbedFileId (), 0, lastModified, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult updateEmbedFile (Db& db, BeRepositoryBasedId id, uint64_t fileSize, DateTime const* lastModified, uint32_t chunkSize)
    {
    const bool updateLastModified = DbEmbeddedFileTable::HasLastModifiedColumn (db) && lastModified != nullptr && lastModified->IsValid ();

    Statement stmt;
    Utf8CP sql = updateLastModified ? "UPDATE " BEDB_TABLE_EmbeddedFile " SET Size=?,Chunk=?,LastModified=? WHERE Id=?" : "UPDATE " BEDB_TABLE_EmbeddedFile " SET Size=?,Chunk=? WHERE Id=?";
    stmt.Prepare (db, sql);

    int parameterIndex = 1;
    stmt.BindInt64 (parameterIndex, fileSize);

    parameterIndex++;
    stmt.BindInt (parameterIndex, chunkSize);

    if (updateLastModified)
        {
        parameterIndex++;
        double lastModifiedJd = 0.0;
        lastModified->ToJulianDay (lastModifiedJd);
        stmt.BindDouble (parameterIndex, lastModifiedJd);
        }

    parameterIndex++;
    stmt.BindId (parameterIndex, id);

    DbResult rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Replace (Utf8CP name, Utf8CP filespec, uint32_t chunkSize, DateTime const* lastModified)
    {
    BeAssert (m_db.IsTransactionActive());
    BeRepositoryBasedId id = QueryFile (name);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    bool isLzma2 = true;
    {
    PropertyBlobInStream    inStream(m_db, id);
    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t actuallyRead;
    inStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        return BE_SQLITE_MISMATCH;
    isLzma2 = header.IsLzma2();
    }

    removeFileBlobs (m_db, id);

    if (isLzma2)
        {
        uint64_t fileSize;
        DbResult rc = compressAndEmbedFile (m_db, fileSize, chunkSize, id, filespec, false);
        return (BE_SQLITE_OK != rc) ? rc : updateEmbedFile (m_db, id, fileSize, lastModified, chunkSize);
        }

    uint64_t fileSize;
    DbResult rc = embedFileWithoutCompressing(m_db, fileSize, chunkSize, id, filespec);
    return (BE_SQLITE_OK != rc) ? rc : updateEmbedFile (m_db, id, fileSize, lastModified, chunkSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Save (void const* data, uint64_t size, Utf8CP name, bool compress, uint32_t chunkSize)
    {
    BeAssert (m_db.IsTransactionActive());
    BeRepositoryBasedId id = QueryFile (name);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    removeFileBlobs (m_db, id);

    if (compress)
        {
        compressAndEmbedFileImage(m_db, chunkSize, id, data, (uint32_t)size, true);
        return updateEmbedFile (m_db, id, size, nullptr, chunkSize);
        }

    /*DbResult rc =*/ embedFileImageWithoutCompressing(m_db, data, size, chunkSize, id);
    return updateEmbedFile (m_db, id, size, nullptr, chunkSize);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Export (Utf8CP filespec, Utf8CP name, ICompressProgressTracker* progress)
    {
    BeAssert (m_db.IsTransactionActive());
    uint64_t expectedFileSize;
    uint32_t chunkSize;
    BeRepositoryBasedId id = QueryFile (name, &expectedFileSize, &chunkSize);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    PropertyBlobInStream    inStream(m_db, id);
    BeFileLzmaOutStream     outStream;

    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t actuallyRead;
    inStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        return BE_SQLITE_MISMATCH;

    WString filespecW (filespec, true);
    BeFileName outName(filespecW.c_str());

    if (outStream.CreateOutputFile(outName, false) != BeFileStatus::Success)
        return  BE_SQLITE_ERROR_FileExists;

    if (header.IsLzma2())
        {
        LzmaDecoder decoder;
        ZipErrors result = decoder.Uncompress(outStream, inStream, true, progress);
        if (ZIP_SUCCESS != result)
            {
            outName.BeDeleteFile();
            return  translateZipErrorToSQLiteError(result);
            }

        uint64_t  sizeOfNewFile;
        outName.GetFileSize(sizeOfNewFile);
        if (expectedFileSize != sizeOfNewFile)
            {
            LOG.errorv("DbEmbeddedFileTable::Export: exported file is not the expected size");
            outName.BeDeleteFile();
            return BE_SQLITE_IOERR_READ;
            }

        BeAssert(expectedFileSize == outStream.GetBytesWritten());

        return BE_SQLITE_OK;
        }

    bvector<Byte>   buffer;
    buffer.resize(20000);
    uint64_t totalRead = 0;
    while(true)
        {
        inStream._Read(&buffer[0], (uint32_t)buffer.size(), actuallyRead);
        if (0 == actuallyRead)
            break;
        totalRead += actuallyRead;
        uint32_t bytesWritten = 0;
        outStream._Write(&buffer[0], actuallyRead, bytesWritten);
        if (bytesWritten != actuallyRead)
            return BE_SQLITE_IOERR_WRITE;
        }

    //  Verify that the file size matches.
    if (totalRead != expectedFileSize)
        return BE_SQLITE_IOERR_READ;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2015
//---------------------------------------------------------------------------------------
DbResult DbEmbeddedFileTable::ExportDbFile (Utf8CP localFileName, Utf8CP name, ICompressProgressTracker* progress)
    {
    DbResult status = Export(localFileName, name, progress);
    if (BE_SQLITE_OK != status)
        return status;

    Utf8CP  vfs;
    status = isValidDbFile(localFileName, vfs);
    if (BE_SQLITE_OK != status)
        LOG.errorv("ExportDbFile: validateDbFile returned %d", status);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Read (bvector<Byte>& callerBuffer, Utf8CP name)
    {
    BeAssert (m_db.IsTransactionActive());
    uint64_t actualSize;
    uint32_t chunkSize;
    BeRepositoryBasedId id = QueryFile (name, &actualSize, &chunkSize);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    PropertyBlobInStream    inStream(m_db, id);
    LzmaOutToBvectorStream  outStream (callerBuffer);
    callerBuffer.resize(0);
    outStream.Reserve((size_t)actualSize);

    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t actuallyRead;
    inStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        return BE_SQLITE_MISMATCH;

    if (header.IsLzma2())
        {
        LzmaDecoder decoder;
        ZipErrors result = decoder.Uncompress(outStream, inStream, true, nullptr);

        BeAssert(callerBuffer.size() == actualSize);

        return  ZIP_SUCCESS == result ? BE_SQLITE_OK : BE_SQLITE_IOERR_READ;
        }

    char    tempBuffer[2000];
    while(true)
        {
        inStream._Read(tempBuffer, (uint32_t)sizeof(tempBuffer), actuallyRead);
        if (0 == actuallyRead)
            break;

        uint32_t bytesWritten = 0;
        outStream._Write(&tempBuffer[0], actuallyRead, bytesWritten);
        if (bytesWritten != actuallyRead)
            return BE_SQLITE_IOERR_WRITE;
        }

    BeAssert(callerBuffer.size() == actualSize);
    //  Verify that the file size matches.
    if (callerBuffer.size() != actualSize)
        return BE_SQLITE_IOERR_READ;

    return BE_SQLITE_OK;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeRepositoryBasedId DbEmbeddedFileTable::QueryFile (Utf8CP name, uint64_t* size, uint32_t* chunkSize, Utf8StringP descr, Utf8StringP typestr, DateTime* lastModified)
    {
    // embedded file table is created on demand, so may not exist yet
    if (!m_db.TableExists(BEDB_TABLE_EmbeddedFile))
        return BeRepositoryBasedId(); // invalid id indicates an error

    Utf8CP sql = DbEmbeddedFileTable::HasLastModifiedColumn (m_db) ?
        "SELECT Descr,Type,Id,Size,Chunk,LastModified FROM " BEDB_TABLE_EmbeddedFile " WHERE Name=?" :
        "SELECT Descr,Type,Id,Size,Chunk, NULL AS LastModified FROM " BEDB_TABLE_EmbeddedFile" WHERE Name=?";

    Statement stmt;
    stmt.Prepare (m_db, sql);
    stmt.BindText (1, name, Statement::MAKE_COPY_No);

    DbResult rc = stmt.Step();
    if (rc != BE_SQLITE_ROW)
        return BeRepositoryBasedId(); // invalid id indicates an error

    if (descr)
        descr->AssignOrClear (stmt.GetValueText(0));
    if (typestr)
        typestr->AssignOrClear (stmt.GetValueText(1));
    if (size)
        *size = stmt.GetValueInt64(3);
    if (chunkSize)
        *chunkSize = stmt.GetValueInt(4);
    if (lastModified != nullptr)
        {
        if (stmt.IsColumnNull (5))
            *lastModified = DateTime ();
        else
            {
            double lastModifiedJulianDay = stmt.GetValueDouble (5);
            DateTime::FromJulianDay (*lastModified, lastModifiedJulianDay, DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime));
            }
        }

    return BeRepositoryBasedId (stmt.GetValueInt64(2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Remove (Utf8CP name)
    {
    BeAssert (m_db.IsTransactionActive());
    BeRepositoryBasedId id = QueryFile (name);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    // this isn't really necessary for files created after we added the trigger, but do it for older dbs before we had the trigger.
    removeFileBlobs (m_db, id);

    Statement stmt;
    stmt.Prepare (m_db, "DELETE FROM " BEDB_TABLE_EmbeddedFile " WHERE Id=?");
    stmt.BindId(1, id);

    DbResult rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::CreateTable() const
    {
    if (m_db.TableExists (BEDB_TABLE_EmbeddedFile))
        return BE_SQLITE_OK;

    auto rc = m_db.ExecuteSql ("CREATE TABLE " BEDB_TABLE_EmbeddedFile
        " (Id INTEGER PRIMARY KEY,Name CHAR NOT NULL COLLATE NOCASE UNIQUE,"
        "Descr CHAR, Type CHAR, Size INT, Chunk INT, LastModified TIMESTAMP)");

    if (BE_SQLITE_OK != rc)
        return rc;

    return m_db.ExecuteSql ("CREATE TRIGGER delete_embeddedFiles AFTER DELETE ON " BEDB_TABLE_EmbeddedFile
            " BEGIN DELETE FROM " BEDB_TABLE_Property
            " WHERE Namespace=\"" BEDB_PROPSPEC_NAMESPACE "\" AND NAME=\"" BEDB_PROPSPEC_EMBEDBLOB_NAME "\" AND Id=OLD.Id; END");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+----
//static
bool DbEmbeddedFileTable::HasLastModifiedColumn (BeSQLiteDbR db)
    {
    const SchemaVersion firstVersionWithLastModifiedColumn (3, 1, 0, 1);

    SchemaVersion actualVersion (0,0,0,0);
    if (BE_SQLITE_OK != BeSQLiteProfileManager::ReadProfileVersion (actualVersion, db))
        {
        BeAssert (false && "Could not read BeSQLite profile version.");
        return false;
        }

    return actualVersion.CompareTo (firstVersionWithLastModifiedColumn) >= 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbEmbeddedFileTable::Iterator::Entry DbEmbeddedFileTable::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        if (!m_db->TableExists(BEDB_TABLE_EmbeddedFile))
            return Entry (nullptr, false);

        Utf8String sqlString = MakeSqlString (DbEmbeddedFileTable::HasLastModifiedColumn (*m_db) ?
                                              "SELECT Name,Descr,Type,Id,Size,Chunk,LastModified FROM " BEDB_TABLE_EmbeddedFile :
                                              "SELECT Name,Descr,Type,Id,Size,Chunk, NULL AS LastModified FROM " BEDB_TABLE_EmbeddedFile);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        if (m_params)
            m_params->Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

Utf8CP DbEmbeddedFileTable::Iterator::Entry::GetNameUtf8() const {return m_sql->GetValueText(0);}
Utf8CP DbEmbeddedFileTable::Iterator::Entry::GetDescriptionUtf8() const {return m_sql->GetValueText(1);}
Utf8CP DbEmbeddedFileTable::Iterator::Entry::GetTypeUtf8() const {return m_sql->GetValueText(2);}
uint64_t DbEmbeddedFileTable::Iterator::Entry::GetFileSize() const {return m_sql->GetValueInt64(4);}
uint32_t DbEmbeddedFileTable::Iterator::Entry::GetChunkSize() const {return m_sql->GetValueInt(5);}
DateTime DbEmbeddedFileTable::Iterator::Entry::GetLastModified () const
    {
    if (m_sql->IsColumnNull (6))
        return DateTime ();

    DateTime lastModified;
    DateTime::FromJulianDay (lastModified, m_sql->GetValueDouble (6), DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime));
    return std::move (lastModified);
    }

BeRepositoryBasedId DbEmbeddedFileTable::Iterator::Entry::GetId() const {return BeRepositoryBasedId(m_sql->GetValueInt64(3));}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2012
//---------------------------------------------------------------------------------------
size_t DbEmbeddedFileTable::Iterator::QueryCount () const
    {
    // embedded file table is created on demand, so may not exist yet
    if (!m_db->TableExists (BEDB_TABLE_EmbeddedFile))
        return 0;

    Utf8String sqlString = "SELECT COUNT(*) FROM " BEDB_TABLE_EmbeddedFile;
    if (!m_whereClause.empty())
        {
        sqlString += " WHERE ";
        sqlString += m_whereClause;
        }

    Statement statement;
    statement.Prepare (*m_db, sqlString.c_str());

    if (m_params)
        m_params->Bind(statement);

    return ((BE_SQLITE_ROW != statement.Step()) ? 0 : statement.GetValueInt (0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
static int rTreeMatch(RTreeMatch::QueryInfo* info)
    {
    AggregateFunction* agg = (AggregateFunction*) info->m_context;
    return ((RTreeMatch*) agg->GetAggregate())->_TestRange(*info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::InitRTreeMatch() const
    {
    if (m_flags.m_rtreeMatchValid)
        return;
    m_flags.m_rtreeMatchValid = true;

    int stat = sqlite3_rtree_query_callback(m_sqlDb, "rTreeMatch", (int(*)(sqlite3_rtree_query_info*)) rTreeMatch, &m_rtreeMatch, NULL);
    BeAssert(BE_SQLITE_OK == stat);

    stat = AddAggregateFunction (m_rtreeMatch);
    BeAssert(BE_SQLITE_OK == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::SetRTreeMatch(RTreeMatch* tester) const
    {
    BeAssert (m_flags.m_rtreeMatchValid);
    BeAssert (NULL == m_rtreeMatch.GetAggregate () || NULL == tester);
    m_rtreeMatch.SetAggregate(tester);
    }

RTreeMatch::RTreeMatch(BeSQLiteDbR db) : m_db(db){db.m_dbFile->InitRTreeMatch();}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RTreeMatch::StepRTree(Statement& stmt)
    {
    m_db.m_dbFile->SetRTreeMatch(this);
    DbResult rc = stmt.Step();
    m_db.m_dbFile->SetRTreeMatch(NULL);
    return rc;
    }

//=======================================================================================
// support for zlib-compressed databases
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
static int zfsZlibBound(void *pCtx, int nByte){return compressBound(nByte);}
static int zfsZlibCompress(void *pCtx, char *aDest, int *pnDest, char *aSrc, int nSrc)
    {
    uLongf n = *pnDest;
    int rc = compress((Bytef*)aDest, &n, (Bytef*)aSrc, nSrc);
    *pnDest = n;
    return (rc==Z_OK ? SQLITE_OK : SQLITE_ERROR);
    }
static int zfsZlibUncompress(void *pCtx, char *aDest, int *pnDest, char *aSrc, int nSrc)
    {
    uLongf n = *pnDest;
    int rc = uncompress((Bytef*)aDest, &n, (Bytef*)aSrc, nSrc);
    *pnDest = n;
    return (rc==Z_OK ? SQLITE_OK : SQLITE_ERROR);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP loadZlibVfs()
    {
    static Utf8CP s_vfsZlib = "zlib";
    if (0 == sqlite3_vfs_find(s_vfsZlib))
        zipvfs_create_vfs(s_vfsZlib, 0, 0, zfsZlibBound, zfsZlibCompress, zfsZlibUncompress);

    return  s_vfsZlib;
    }

//=======================================================================================
// support for snappy-compressed databases
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
static int zfsSnappyBound(void *pCtx, int nByte){return (int) snappy::MaxCompressedLength(nByte);}
static int zfsSnappyCompress(void *pCtx, char *aDest, int *pnDest, char *aSrc, int nSrc)
    {
    snappy::RawCompress (aSrc, nSrc, aDest, (unsigned int*) pnDest);
    return SQLITE_OK;
    }

static int zfsSnappyUncompress(void *pCtx, char *aDest, int *pnDest, char *aSrc, int nSrc)
    {
    size_t outSize;
    snappy::GetUncompressedLength(aSrc, nSrc, &outSize);
    *pnDest = (int) outSize;
    bool rc = snappy::RawUncompress (aSrc, nSrc, aDest);
    return (rc ? SQLITE_OK : SQLITE_ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP loadSnappyVfs()
    {
    static Utf8CP s_vfsSnappy = "snappy";
    if (0 == sqlite3_vfs_find(s_vfsSnappy))
        zipvfs_create_vfs(s_vfsSnappy, 0, 0, zfsSnappyBound, zfsSnappyCompress, zfsSnappyUncompress);

    return s_vfsSnappy;
    }

static IDownloadAdmin* s_downloadAdmin = NULL;
void BeSQLiteLib::SetDownloadAdmin(IDownloadAdmin& dlAdmin) {if (NULL == s_downloadAdmin) s_downloadAdmin = &dlAdmin;}
IDownloadAdmin* BeSQLiteLib::GetDownloadAdmin() {return s_downloadAdmin;}

/*---------------------------------------------------------------------------------**//**
* implementation of SQL "IsInSet" function. Returns 1 if the value is contained in the set.
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void isInVirtualSet (sqlite3_context* ctx, int nArgs, sqlite3_value** args)
    {
    if (2 > nArgs)
        {
        sqlite3_result_error(ctx, "Not enough arguments to InVirtualSet", -1);
        return;
        }

    // the first argument must be the set to test against.
    VirtualSet const* vSet = (VirtualSet const*) sqlite3_value_int64(args[0]);
    if (NULL==vSet)
        {
        sqlite3_result_error(ctx, "Illegal VirtualSet", -1);
        return;
        }

    // skip the first argument - we used it above.
    sqlite3_result_int(ctx, vSet->_IsInSet(nArgs-1, (DbValue const*) args+1));
    }

//---------------------------------------------------------------------------------------
// Direct sqlite callback when we override the LOWER and UPPER scalar functions to delegate to BeSQLiteLib::ILanguageSupport.
// @bsimethod                                                   Jeff.Marker     01/2014
//---------------------------------------------------------------------------------------
static void caseCallback(sqlite3_context* context, int numArgs, sqlite3_value** args)
    {
    // Largely a copy of icuCaseFunc16 in ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.

    BeSQLiteLib::ILanguageSupport* languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (NULL == languageSupport)
        { BeAssert(false); return; }

    if (1 != numArgs)
        { BeAssert(false); return; }

    Utf16CP source = (Utf16CP)sqlite3_value_text16(args[0]);
    if (NULL == source)
        { BeAssert(false); return; }

    int sourceSize = (size_t)sqlite3_value_bytes16(args[0]);
    int resultSize = (2 * sourceSize) * sizeof(uint16_t);

    Utf16P result = (Utf16P)sqlite3_malloc((int)resultSize);
    if (NULL == result)
        { BeAssert(false); return; }

    if (0 != sqlite3_user_data(context))
        languageSupport->_Upper(source, sourceSize / sizeof(uint16_t), result, resultSize / sizeof(uint16_t));
    else
        languageSupport->_Lower(source, sourceSize / sizeof(uint16_t), result, resultSize / sizeof(uint16_t));

    sqlite3_result_text16(context, result, -1, sqlite3_free);
    }

//---------------------------------------------------------------------------------------
// Copied from utf8.h in ICU.
// Supports our LIKE operator implementation.
// @bsimethod                                                   Jeff.Marker     02/2014
//---------------------------------------------------------------------------------------
#define U8_NEXT_UNSAFE(s, i, c) \
    { \
    (c) = (uint8_t)(s)[(i)++]; \
    if ((c) >= 0x80) { \
        if ((c)<0xe0) { \
                (c) = (((c)& 0x1f) << 6) | ((s)[(i)++] & 0x3f); \
            } else if ((c)<0xf0) { \
                /* no need for (c&0xf) because the upper bits are truncated after <<12 in the cast to (UChar) */ \
                (c) = (unsigned char)(((c) << 12) | (((s)[i] & 0x3f) << 6) | ((s)[(i)+1] & 0x3f)); \
                (i) += 2; \
            } else { \
                (c) = (((c)& 7) << 18) | (((s)[i] & 0x3f) << 12) | (((s)[(i)+1] & 0x3f) << 6) | ((s)[(i)+2] & 0x3f); \
                (i) += 3; \
            } \
        } \
    }

//---------------------------------------------------------------------------------------
// Copied from utf8.h in ICU.
// Supports our LIKE operator implementation.
// @bsimethod                                                   Jeff.Marker     02/2014
//---------------------------------------------------------------------------------------
#define U8_COUNT_TRAIL_BYTES_UNSAFE(leadByte) (((leadByte)>=0xc0)+((leadByte)>=0xe0)+((leadByte)>=0xf0))

//---------------------------------------------------------------------------------------
// Copied from utf8.h in ICU.
// Supports our LIKE operator implementation.
// @bsimethod                                                   Jeff.Marker     02/2014
//---------------------------------------------------------------------------------------
#define U8_FWD_1_UNSAFE(s, i) \
    { \
    (i) += 1 + U8_COUNT_TRAIL_BYTES_UNSAFE((uint8_t)(s)[i]); \
    }

//---------------------------------------------------------------------------------------
// Actual comparison logic for our custom LIKE operator.
// @see likeCallback
// @bsimethod                                                   Jeff.Marker     02/2014
//---------------------------------------------------------------------------------------
static int likeCompare(unsigned char const* patternString, unsigned char const* matchString, uint32_t escapeChar, BeSQLiteLib::ILanguageSupport* languageSupport)
    {
    // Largely a copy of icuLikeCompare in ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.
    // See also patternCompare in sqlite3/src/func.c... though that also supports globs and can use other nice internal utility functions that we can't, so copying is limited.

    static const uint32_t MATCH_ONE = (uint32_t)'_';
    static const uint32_t MATCH_ALL = (uint32_t)'%';

    int iPattern = 0; // Current byte index in patternString
    int iMatch = 0; // Current byte index in matchString
    bool wasPreviousCharEscape = 0; // True if the previous character was escapeChar

    while (0 != patternString[iPattern])
        {
        // Read (and consume) the next character from the input pattern.
        uint32_t currPatternChar;
        U8_NEXT_UNSAFE(patternString, iPattern, currPatternChar);
        BeAssert(0 != currPatternChar);

        // There are now 4 possibilities:
        //  1. currPatternChar is an unescaped match-all character "%"
        //  2. currPatternChar is an unescaped match-one character "_"
        //  3. currPatternChar is an unescaped escape character
        //  4. currPatternChar is to be handled as an ordinary character

        if (!wasPreviousCharEscape && (MATCH_ALL == currPatternChar))
            {
            // Case 1.
            uint8_t peekPatternChar;

            // Skip any MATCH_ALL or MATCH_ONE characters that follow a MATCH_ALL. For each MATCH_ONE, skip one character in the test string.
            while ((MATCH_ALL == (peekPatternChar = patternString[iPattern])) || (MATCH_ONE == peekPatternChar))
                {
                if (MATCH_ONE == peekPatternChar)
                    {
                    if (0 == matchString[iMatch])
                        return 0;

                    U8_FWD_1_UNSAFE(matchString, iMatch);
                    }

                ++iPattern;
                }

            if (0 == patternString[iPattern])
                return 1;

            while (0 != matchString[iMatch])
                {
                if (likeCompare(&patternString[iPattern], &matchString[iMatch], escapeChar, languageSupport))
                    return 1;

                U8_FWD_1_UNSAFE(matchString, iMatch);
                }

            return 0;
            }
        else if (!wasPreviousCharEscape && (MATCH_ONE == currPatternChar))
            {
            // Case 2.
            if (0 == matchString[iMatch])
                return 0;

            U8_FWD_1_UNSAFE(matchString, iMatch);
            }
        else if (!wasPreviousCharEscape && (currPatternChar == escapeChar))
            {
            // Case 3.
            wasPreviousCharEscape = true;
            }
        else{
            // Case 4.
            uint32_t currMatchChar;
            U8_NEXT_UNSAFE(matchString, iMatch, currMatchChar);

            currMatchChar = languageSupport->_FoldCase(currMatchChar);
            currPatternChar = languageSupport->_FoldCase(currPatternChar);

            if (currMatchChar != currPatternChar)
                return 0;

            wasPreviousCharEscape = false;
            }
        }

    return (0 == matchString[iMatch]);
    }

//---------------------------------------------------------------------------------------
// Direct sqlite callback when we override the LIKE operator to delegate to BeSQLiteLib::ILanguageSupport.
// @bsimethod                                                   Jeff.Marker     02/2014
//---------------------------------------------------------------------------------------
static void likeCallback(sqlite3_context* context, int numArgs, sqlite3_value** args)
    {
    // Largely a copy of icuLikeFunc in sqlite3/ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.
    // See also likeFunc in sqlite3/src/func.c... though that can use other nice internal utility functions that we can't, so copying is limited.

    auto languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (NULL == languageSupport)
        { BeAssert(false); return; }

    if ((numArgs < 2) || (numArgs > 3))
        { BeAssert(false); return; }

    auto patternString = sqlite3_value_text(args[0]);
    auto matchString = sqlite3_value_text(args[1]);

    if ((NULL == patternString) || (NULL == matchString))
        return;

    // Limit the length of the LIKE or GLOB pattern to avoid problems of deep recursion and N*N behavior in likeCompare.
    auto maxPatternLen = sqlite3_limit(sqlite3_context_db_handle(context), SQLITE_LIMIT_LIKE_PATTERN_LENGTH, -1);
    if (sqlite3_value_bytes(args[0]) > maxPatternLen)
        { BeAssert(false); return; }

    uint32_t escapeChar = 0;
    if (3 == numArgs)
        {
        // The escape character string must consist of a single UTF-8 character. Otherwise, return an error.
        auto escapeCharStr = sqlite3_value_text(args[2]);
        if (NULL == escapeCharStr)
            { BeAssert(false); return; }

        auto escapeCharNumBytes = sqlite3_value_bytes(args[2]);
        int iNextChar = 0;
        U8_NEXT_UNSAFE(escapeCharStr, iNextChar, escapeChar);

        if (iNextChar != escapeCharNumBytes)
            { BeAssert(false); return; }
        }

    sqlite3_result_int(context, likeCompare(patternString, matchString, escapeChar, languageSupport));
    }

//---------------------------------------------------------------------------------------
// Direct sqlite callback when we add custom collations to delegate to BeSQLiteLib::ILanguageSupport.
// @bsimethod                                                   Jeff.Marker     01/2014
//---------------------------------------------------------------------------------------
static int collateCallback(void* userData, int lhsSize, void const* lhs, int rhsSize, void const* rhs)
    {
    // Largely a copy of icuCollationColl in ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.

    BeSQLiteLib::ILanguageSupport* languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (NULL == languageSupport)
        { BeAssert(false); return 0; }

    return languageSupport->_Collate((Utf16CP)lhs, lhsSize / sizeof(uint16_t), (Utf16CP)rhs, rhsSize / sizeof(uint16_t), userData);
    }

//---------------------------------------------------------------------------------------
// Registers overrides and additions to be able to delegate language-aware string processing to BeSQLiteLib::ILanguageSupport.
// @bsimethod                                                   Jeff.Marker     01/2014
//---------------------------------------------------------------------------------------
static void initLanguageSupportOnDb(sqlite3* db)
    {
    BeSQLiteLib::ILanguageSupport* languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (NULL == languageSupport)
        return;

    int rc;
    UNUSED_VARIABLE(rc);

    // The ICU sample from the sqlite folks overrides scalar functions for both SQLITE_UTF8 and SQLITE_UTF16, but only provides SQLITE_UTF8 versions for operators and collations...
    // I'm not sure why, but following their example until proven otherwise.

    rc = sqlite3_create_function_v2(db, "lower", 1, SQLITE_UTF8, (void*)0, caseCallback, NULL, NULL, NULL);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "lower", 1, SQLITE_UTF16, (void*)0, caseCallback, NULL, NULL, NULL);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "upper", 1, SQLITE_UTF8, (void*)1, caseCallback, NULL, NULL, NULL);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "upper", 1, SQLITE_UTF16, (void*)1, caseCallback, NULL, NULL, NULL);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "like", 2, SQLITE_UTF8, (void*)0, likeCallback, NULL, NULL, NULL);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "like", 3, SQLITE_UTF8, (void*)0, likeCallback, NULL, NULL, NULL);
    BeAssert(BE_SQLITE_OK == rc);

    bvector<BeSQLiteLib::ILanguageSupport::CollationEntry> collationEntries;
    BeSQLiteLib::ILanguageSupport::CollationUserDataFreeFunc collatorFreeFunc = NULL;
    languageSupport->_InitCollation(collationEntries, collatorFreeFunc);

    for (auto const& collationEntry : collationEntries)
        {
        rc = sqlite3_create_collation_v2(db, collationEntry.m_name.c_str(), SQLITE_UTF16, collationEntry.m_collator, collateCallback, collatorFreeFunc);
        BeAssert(BE_SQLITE_OK == rc);
        }
    }

extern "C" int sqlite3_closure_init(sqlite3* db, char** pzErrMsg, struct sqlite3_api_routines const* pApi);

/*---------------------------------------------------------------------------------**//**
* this function is called for every new database connection.
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int besqlite_db_init(sqlite3* db, char** pzErrMsg, struct sqlite3_api_routines const* pApi)
    {
    // install the "closure" virtual table
    int rc = sqlite3_closure_init (db, pzErrMsg, pApi);
    UNUSED_VARIABLE(rc);
    BeAssert (BE_SQLITE_OK == rc);

    // and the "InVirtualSet" SQL function. It requires at least two arguments: the address of the VirtualSet and the value(s) to test
    rc = sqlite3_create_function_v2 (db, "InVirtualSet", -1, SQLITE_UTF8, NULL, &isInVirtualSet, NULL, NULL, NULL);
    BeAssert(BE_SQLITE_OK == rc);

    // Register language-aware callbacks if necessary.
    initLanguageSupportOnDb(db);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void logCallback (void *pArg, int iErrCode, Utf8CP zMsg) {LOG.infov ("SQLite error %d [%s]", iErrCode, zMsg);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BeSQLiteLib::Initialize(BeFileNameCR tempDir, LogErrors logErrors)
    {
    static bool s_done = false;
    if (s_done)
        return BE_SQLITE_OK;
    s_done = true;

    if (logErrors == LogErrors::Yes)
        sqlite3_config (SQLITE_CONFIG_LOG, logCallback, NULL);

    sqlite3_initialize();
    sqlite3_auto_extension ((void(*)(void))&besqlite_db_init);

    Utf8String tempDirUtf8 = tempDir.GetNameUtf8 ();
    if (!tempDir.DoesPathExist ())
        {
        LOG.errorv ("BeSQLiteLib::Initialize failed: Temporary directory '%s' does not exist", tempDirUtf8.c_str ());
        BeAssert (false && "Error in BeSQLiteLib::Initialize: Temp dir does not exist!");
        return BE_SQLITE_CANTOPEN_NOTEMPDIR;
        }

    sqlite3_temp_directory = (char*) sqlite3_malloc ((int) (tempDirUtf8.size ()) + 1);
    strcpy (sqlite3_temp_directory, tempDirUtf8.c_str ());
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
int BeSQLiteLib::CloseSqlDb(void* p) {return sqlite3_close((sqlite3*) p);}

// Functions needed for 7z.  These are for a C API that simulates C++.
static void *allocFor7z (void *p, size_t size)   {return malloc (size);}
static void freeFor7z(void *p, void *address)    {free (address);}
static SRes readFor7z(void *p, void *buf, size_t *size);
static size_t writeFor7z (void *p, const void *buf, size_t size);
static SRes progressFor7z (void *p, UInt64 inSize, UInt64 outSize);

//=======================================================================================
// Provides an implementation of the 7z C struct ISeqInStream.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct SeqInStreamImpl : ::ISeqInStream
{
private:
    ILzmaInputStream& m_stream;

public:
    SeqInStreamImpl (ILzmaInputStream& in) : m_stream(in) { Read = readFor7z; }
    ILzmaInputStream& GetStream () { return m_stream; }
    SRes ReadData (void *buf, size_t *size) { return Read (this, buf, size); }
    uint64_t GetSize () { return m_stream._GetSize(); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BeFileLzmaInFromMemory::BeFileLzmaInFromMemory(void const*data, uint32_t size) : m_mutex(BeDbMutex::FastMutex), m_data(data), m_size(size), m_offset(0) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
ZipErrors BeFileLzmaInFromMemory::_Read (void* data, uint32_t size, uint32_t& actuallyRead)
    {
    BeDbMutexHolder   __holder(m_mutex);

    actuallyRead = std::min(size, m_size-m_offset);
    if (0 == actuallyRead)
        //  The file-based implementation does not treat EOF as an error.
        return ZIP_SUCCESS;

    memcpy(data, (char*)m_data + m_offset, actuallyRead);
    m_offset += actuallyRead;

    return ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
StatusInt BeFileLzmaInStream::OpenInputFile(BeFileNameCR fileName)
    {
    if (m_file.IsOpen())
        return BSIERROR;

    m_bytesRead = 0;
    BeFileStatus    result = m_file.Open(fileName.GetName(), BeFileAccess::Read);
    if (BeFileStatus::Success != result)
        return (StatusInt)result;

    BeFileName::GetFileSize(m_fileSize, fileName.GetName());
    m_file.SetPointer(0, BeFileSeekOrigin::Begin);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
ZipErrors BeFileLzmaInStream::_Read (void* data, uint32_t size, uint32_t& actuallyRead)
    {
    BeFileStatus result = m_file.Read(data, &actuallyRead, size);
    m_bytesRead += actuallyRead;

    if (BeFileStatus::Success != result)
        {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
        HRESULT hr = ::GetLastError();
#endif
        LOG.errorv("BeFileLzmaInStream::_Read result = %d, m_bytesRead = %lld, filesize = %lld", result, m_bytesRead, m_fileSize);
#if defined (BENTLEYCONFIG_OS_WINDOWS)
        LOG.errorv("    HRESULT = %d", hr);
#endif
        }

    return BeFileStatus::Success != result ? ZIP_ERROR_READ_ERROR : ZIP_SUCCESS;
    }

//=======================================================================================
// Provides an implementation of the 7z C struct ISeqOutStream.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct SeqOutStreamImpl : ::ISeqOutStream
{
private:
    ILzmaOutputStream&  m_stream;

public:
    ILzmaOutputStream& GetStream () { return m_stream; }
    SeqOutStreamImpl (ILzmaOutputStream& outStream) : m_stream(outStream) { Write = writeFor7z; }
    size_t WriteData (const void *buf, size_t size) { return Write(this, buf, size); }
    void SetAlwaysFlush(bool flushOnEveryWrite) { m_stream._SetAlwaysFlush(flushOnEveryWrite); }
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct          MemoryLzmaOutStream : ILzmaOutputStream
{
private:
    bvector<Byte>& m_out;

public:
    //  UInt32 GetOffset() { return m_offset; }
    MemoryLzmaOutStream (bvector<Byte>& out) : m_out (out) { m_out.resize(0); }

    ZipErrors _Write (void const* data, uint32_t writeSize, uint32_t&bytesWritten) override
        {
        size_t oldSize = m_out.size();
        m_out.resize (oldSize + writeSize);
        memcpy (&m_out[oldSize], data, writeSize);
        bytesWritten = writeSize;
        return ZIP_SUCCESS;
        }

    void _SetAlwaysFlush(bool flushOnEveryWrite) override {}
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct          MemoryLzmaInStream : ILzmaInputStream
{
private:
    uint32_t    m_offset;
    uint32_t    m_limit;
    void const* m_data;

    uint32_t    m_headerOffset;
    uint32_t    m_headerLimit;
    void const* m_headerData;

public:
    uint32_t GetOffset() { return m_offset; }
    MemoryLzmaInStream(void const*data, uint32_t size) : m_offset (0), m_limit (size), m_data (data), m_headerData(nullptr), m_headerOffset(0), m_headerLimit(0) {}
    void SetHeaderData(void*headerData, uint32_t headerSize) { m_headerData = headerData; m_headerLimit = headerSize; }
    //  The LZMA2 multithreading ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    virtual ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) override
        {
        uint32_t readFromHeader = 0;
        if (m_headerData)
            {
            readFromHeader = std::min(m_headerLimit-m_headerOffset, size);
            memcpy (data, (Byte*)m_headerData + m_headerOffset, readFromHeader);
            if ((m_headerOffset += readFromHeader) >=  m_headerLimit)
                m_headerData = nullptr; //  don't use it again
            BeAssert(size >= readFromHeader);
            size -= readFromHeader;
            data = (Byte*)data + readFromHeader;
            }

        actuallyRead = size;
        if (size > (m_limit - m_offset))
            actuallyRead = m_limit - m_offset;
        memcpy (data, (Byte*)m_data + m_offset, actuallyRead);
        m_offset += actuallyRead;
        actuallyRead += readFromHeader;
        return ZIP_SUCCESS;
        }

    virtual uint64_t _GetSize() override { return m_limit + m_headerLimit; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
BeFileStatus BeFileLzmaOutStream::CreateOutputFile (BeFileNameCR fileName, bool createAlways)
    {
    if (m_file.IsOpen ())
        return BeFileStatus::UnknownError;

    m_bytesWritten = 0;
    BeFileStatus    result = m_file.Create(fileName.GetName(), createAlways);
    if (BeFileStatus::Success != result)
        return result;

    m_file.SetPointer(0, BeFileSeekOrigin::Begin);

    return BeFileStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
void BeFileLzmaOutStream::_SetAlwaysFlush(bool flushOnEveryWrite) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors BeFileLzmaOutStream::_Write (void const* data, uint32_t size, uint32_t& bytesWritten)
    {
    //  The LZMA2 multi-threading support ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    BeFileStatus result = m_file.Write(&bytesWritten, data, size);
    //  We check m_bytesWritten at the end to verify that we have processed exactly the expected number of bytes.
    m_bytesWritten += bytesWritten;

    if (bytesWritten != size)
        LOG.errorv("BeFileLzmaOutStream::_Write %u requested, %u written", size, bytesWritten);
    if (BeFileStatus::Success != result)
        return ZIP_ERROR_WRITE_ERROR;

    return ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaOutToBvectorStream::_Write (void const* data, uint32_t size, uint32_t& bytesWritten)
    {
    //  The LZMA2 multi-threading support ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    auto currSize = m_buffer.size();
    m_buffer.resize (currSize+size);
    memcpy (&m_buffer[currSize], data, size);
    bytesWritten = size;
    return ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
void LzmaOutToBvectorStream::Reserve(size_t min)
    {
    if (min < m_buffer.capacity())
        return;

    m_buffer.reserve(min);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
void LzmaOutToBvectorStream::_SetAlwaysFlush(bool flushOnEveryWrite) {}

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct          ICompressProgressImpl : ::ICompressProgress
{
    ICompressProgressTracker*   m_tracker;
    ICompressProgressImpl (ICompressProgressTracker* tracker) : m_tracker (tracker) {Progress = progressFor7z;}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static SRes readFor7z(void *p, void *buf, size_t *size)
    {
    SeqInStreamImpl* pImpl = static_cast <SeqInStreamImpl*>(p);

    uint32_t bytesRead = 0;
    ZipErrors zipError = pImpl->GetStream()._Read(buf, (uint32_t)*size, bytesRead);
    *size = bytesRead;

    return ZIP_SUCCESS == zipError ? SZ_OK : SZ_ERROR_READ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static size_t writeFor7z (void *p, const void *buf, size_t size)
    {
    SeqOutStreamImpl* pImpl = static_cast<SeqOutStreamImpl*>(p);
    uint32_t bytesWritten;
    pImpl->GetStream()._Write(buf, (uint32_t)size, bytesWritten);

    return bytesWritten;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static SRes progressFor7z (void *p, UInt64 inSize, UInt64 outSize)
    {
    ICompressProgressImpl* impl = (ICompressProgressImpl*)p;

    BeAssert (impl->m_tracker);  //  SetICompressProgress does not use a ICompressProgressImpl with m_tracker == NULL;

    if (impl->m_tracker->_Progress (inSize, (int64_t)outSize) != ZIP_SUCCESS)
        return SZ_ERROR_PROGRESS;

    return SZ_OK;
    }

#define DECODE_INPUT_BUFFER_SIZE (32 * 1024)
#define ENCODE_INPUT_BUFFER_SIZE (32 * 1024)
#define DECODE_OUTPUT_BUFFER_SIZE (32 * 1024)
#define DGNDB_LZMA_MARKER   "LzmaDgnDb"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static ZipErrors translate7ZipError (SRes code, bool readingFromFile)
    {
    switch (code)
        {
        case SZ_OK:
            return ZIP_SUCCESS;
        case SZ_ERROR_DATA:
        case SZ_ERROR_ARCHIVE:
        case SZ_ERROR_NO_ARCHIVE:
            return ZIP_ERROR_BAD_DATA;

        case SZ_ERROR_INPUT_EOF:
            return ZIP_ERROR_END_OF_DATA;

        case SZ_ERROR_READ:
            if (readingFromFile)
                return ZIP_ERROR_READ_ERROR;

            return ZIP_ERROR_BLOB_READ_ERROR;

        case SZ_ERROR_OUTPUT_EOF:
        case SZ_ERROR_WRITE:
            return ZIP_ERROR_WRITE_ERROR;

        case SZ_ERROR_PROGRESS:
            return ZIP_ERROR_ABORTED;  //  ICompressProgress returned something other than ZIP_SUCCESS
        }

    BeAssert (ZIP_SUCCESS == code);
    return ZIP_ERROR_UNKNOWN;
    }

//=======================================================================================
//  A compressed DgnDb file starts with LzmaDgnDbHeader.  This is not the same as an
//  EmbeddedLzmaHeader.  A EmbeddedLzmaHeader is used for any type of embedded file.
//  If a compressed DgnDb file is stored as an embedded file, it also gets a
//  EmbeddedLzmaHeader as part of the embedded stream.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct          LzmaDgnDbHeader
{
private:
    uint16_t        m_sizeOfHeader;
    char            m_idString [10];
    uint16_t        m_formatVersionNumber;
    uint16_t        m_compressionType;
    uint64_t        m_sourceSize;

public:
    static const int formatVersionNumber = 0x10;
    enum CompressionType
        {
        LZMA2   = 2
        };

    LzmaDgnDbHeader (CharCP idString, CompressionType compressionType, uint64_t sourceSize)
        {
        BeAssert ((strlen (idString)+ 1) <= sizeof(m_idString));
        memset (this, 0, sizeof (*this));
        m_sizeOfHeader = (uint16_t)sizeof (LzmaDgnDbHeader);
        strcpy (m_idString, idString);
        m_compressionType = compressionType;
        m_formatVersionNumber = formatVersionNumber;
        m_sourceSize = sourceSize;
        }

    LzmaDgnDbHeader ()
        {
        memset (this, 0, sizeof (*this));
        }

    int GetVersion () { return m_formatVersionNumber; }
    bool IsLzma2 () { return true; }
    bool IsValid ()
        {
        if (strcmp(m_idString, DGNDB_LZMA_MARKER))
            return false;

        if (formatVersionNumber != m_formatVersionNumber)
            return false;

        return m_compressionType == LZMA2;
        }
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/13
//=======================================================================================
struct SevenZImpl
{
private:
    ::ISzAlloc m_szAlloc;
    ::ICompressProgress* m_compressProgress;
    SeqInStreamImpl m_inStream;
    SeqOutStreamImpl m_outStream;

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    SevenZImpl (ILzmaOutputStream&outStream, ILzmaInputStream&inStream) : m_outStream(outStream), m_inStream(inStream)
        {
        m_szAlloc.Alloc = allocFor7z;
        m_szAlloc.Free = freeFor7z;
        m_compressProgress = NULL;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    void SetICompressProgress (::ICompressProgressImpl& compressProgress)
        {
        m_compressProgress = NULL == compressProgress.m_tracker ? NULL : &compressProgress;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    ZipErrors DoDecode(CLzmaDec& decodeState, uint64_t unpackSize)
        {
        int thereIsSize = (unpackSize != (uint64_t)(int64_t)-1);
        Byte inBuf[DECODE_INPUT_BUFFER_SIZE];
        Byte outBuf[DECODE_OUTPUT_BUFFER_SIZE];

        size_t inPos = 0, inSize = 0, outPos = 0;
        uint64_t totalRead = 0, totalWritten = 0;
        LzmaDec_Init(&decodeState);
        for (;;)
            {
            if (inPos == inSize)
                {
                inSize = DECODE_INPUT_BUFFER_SIZE;
                m_inStream.ReadData(inBuf, &inSize);
                totalRead += inSize;
                inPos = 0;
                }

            SRes res;
            SizeT inProcessed = inSize - inPos;
            SizeT outProcessed = DECODE_OUTPUT_BUFFER_SIZE - outPos;
            ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
            ELzmaStatus status;
            if (thereIsSize && outProcessed > unpackSize)
                {
                outProcessed = (SizeT)unpackSize;
                finishMode = LZMA_FINISH_END;
                }

            res = LzmaDec_DecodeToBuf(&decodeState, outBuf + outPos, &outProcessed, inBuf + inPos, &inProcessed, finishMode, &status);
            inPos += inProcessed;
            outPos += outProcessed;
            unpackSize -= outProcessed;

            if (m_outStream.WriteData(outBuf, outPos) != outPos)
                return ZIP_ERROR_WRITE_ERROR;

            totalWritten += outPos;
            if (NULL != m_compressProgress)
                {
                res = m_compressProgress->Progress(m_compressProgress, totalRead, totalWritten);
                }

            outPos = 0;

            if (res != SZ_OK || thereIsSize && unpackSize == 0)
                return translate7ZipError(res, false);

            if (inProcessed == 0 && outProcessed == 0)
                {
                if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
                    return ZIP_ERROR_BAD_DATA;

                return translate7ZipError(res, false);
                }
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    ZipErrors DoDecode2(CLzma2Dec& decodeState)
        {
        Byte inBuf[DECODE_INPUT_BUFFER_SIZE];
        ScopedArray<Byte> scopedOutputBuf(DECODE_OUTPUT_BUFFER_SIZE);
        Byte* outBuf = scopedOutputBuf.GetData();

        uint64_t totalRead = 0;
        uint64_t totalWritten = 0;

        size_t inPos = 0, inSize = 0, outPos = 0;
        Lzma2Dec_Init(&decodeState);
        for (;;)
            {
            if (inPos == inSize)
                {
                inSize = DECODE_INPUT_BUFFER_SIZE;
                //  We ignore the read error here. It probably means end-of-stream.  We count on LZMA detecting that it
                //  has hit the end of the input data without hitting the end of the stream.  As a failsafe Export verifies that the output file is the expected size.
                m_inStream.ReadData(inBuf, &inSize);
                totalRead += inSize;
                inPos = 0;
                }

            SRes res;
            SizeT inProcessed = inSize - inPos;
            SizeT outProcessed = DECODE_OUTPUT_BUFFER_SIZE - outPos;
            ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
            ELzmaStatus status;

            res = Lzma2Dec_DecodeToBuf(&decodeState, outBuf + outPos, &outProcessed, inBuf + inPos, &inProcessed, finishMode, &status);
            inPos += inProcessed;
            outPos += outProcessed;

            if (m_outStream.WriteData(outBuf, outPos) != outPos)
                return ZIP_ERROR_WRITE_ERROR;

            totalWritten += outPos;
            outPos = 0;

            if (NULL != m_compressProgress && SZ_OK == res)
                res = m_compressProgress->Progress(m_compressProgress, totalRead, totalWritten);

            if (res != SZ_OK)
                return translate7ZipError(res, false);

            if (inProcessed == 0 && outProcessed == 0)
                {
                if (status != LZMA_STATUS_FINISHED_WITH_MARK)
                    return ZIP_ERROR_END_OF_DATA;

                return translate7ZipError(res, false);
                }
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    ZipErrors DoInitAndDecode2()
        {
        ZipErrors res = ZIP_SUCCESS;

        CLzma2Dec decodeState;
        Lzma2Dec_Construct(&decodeState);

        //  1 byte describing the properties and 8 bytes of uncompressed size
        Byte header;

        size_t  readSize = sizeof (header);
        if (m_inStream.ReadData(&header, &readSize) != BSISUCCESS)
            return ZIP_ERROR_BAD_DATA;

        Lzma2Dec_Allocate(&decodeState, header, &m_szAlloc);

        res = DoDecode2(decodeState);
        Lzma2Dec_Free(&decodeState, &m_szAlloc);

        return res;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    ZipErrors Uncompress (bool isLzma2)
        {
        BeAssert(isLzma2);
        return DoInitAndDecode2 ();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    ZipErrors Compress1 (::CLzmaEncProps*encProps)
        {
        ::CLzmaEncHandle enc = LzmaEnc_Create(&m_szAlloc);
        if (enc == 0)
            return ZIP_ERROR_UNKNOWN;

        int res = LzmaEnc_SetProps(enc, encProps);
        if (SZ_OK != res)
            return translate7ZipError(res, true);

        Byte header[LZMA_PROPS_SIZE + 8];
        size_t headerSize = LZMA_PROPS_SIZE;

        res = LzmaEnc_WriteProperties(enc, header, &headerSize);
        if (SZ_OK != res)
            return translate7ZipError(res, true);

        uint64_t sourceSize = m_inStream.GetSize();
        for (unsigned i = 0; i < 8; i++)
            header[headerSize++] = (Byte)(sourceSize >> (8 * i));

        if (m_outStream.WriteData (header, headerSize) != headerSize)
            return ZIP_ERROR_WRITE_ERROR;

        //  Compress progress may be NULL.
        res = LzmaEnc_Encode(enc, &m_outStream, &m_inStream, m_compressProgress, &m_szAlloc, &m_szAlloc);
        if (SZ_OK != res)
            return translate7ZipError(res, true);

        LzmaEnc_Destroy(enc, &m_szAlloc, &m_szAlloc);

        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    ZipErrors Compress2 (::CLzma2EncProps*encProps, bool supportRandomAccess)
        {
        ::CLzma2EncHandle enc = Lzma2Enc_Create(&m_szAlloc, &m_szAlloc);
        if (enc == 0)
            return ZIP_ERROR_UNKNOWN;

        int res = Lzma2Enc_SetProps(enc, encProps);
        if (SZ_OK != res)
            return ZIP_ERROR_UNKNOWN;

        Byte header;
        size_t headerSize = sizeof (header);
        header = Lzma2Enc_WriteProperties(enc);

        if (m_outStream.WriteData (&header, headerSize) != headerSize)
            return ZIP_ERROR_WRITE_ERROR;

        if (supportRandomAccess)
            //  Setting alwaysFlush to true forces PropertyBlobOutStream to create a new embedded blob for each write. Since LZMA2 calls
            //  _Write whenever it finishes processing a a block this creates a one-to-one mapping from input blocks to embedded blobs.
            //  That makes it possible to read and expand any given block, randomly accessing the blocks.
            m_outStream.SetAlwaysFlush(true);

        res = Lzma2Enc_Encode(enc, &m_outStream, &m_inStream, m_compressProgress);
        Lzma2Enc_Destroy(enc);

        if (SZ_OK != res)
            return translate7ZipError(res, true);

        return ZIP_SUCCESS;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::UncompressDgnDb (Utf8CP targetFile, Utf8CP sourceFile, ICompressProgressTracker* tracker)
    {
    WString targetFileW (targetFile, BentleyCharEncoding::Utf8);
    WString sourceFileW (sourceFile, BentleyCharEncoding::Utf8);

    return UncompressDgnDb (BeFileName(targetFileW.c_str()), BeFileName (sourceFileW.c_str()), tracker);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::UncompressDgnDb (BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* tracker)
    {
    BeFileLzmaInStream  inStream;
    if (inStream.OpenInputFile(sourceFile) != BSISUCCESS)
        return ZIP_ERROR_CANNOT_OPEN_INPUT;

    BeFileLzmaOutStream outStream;
    if (outStream.CreateOutputFile(targetFile) != BeFileStatus::Success)
        return ZIP_ERROR_CANNOT_OPEN_OUTPUT;

    //  Advance past header
    LzmaDgnDbHeader     lzmaDgndbHeader;

    uint32_t readSize = sizeof (lzmaDgndbHeader);
    inStream._Read(&lzmaDgndbHeader, readSize, readSize);
    if (readSize != sizeof (lzmaDgndbHeader) || !lzmaDgndbHeader.IsValid ())
        return ZIP_ERROR_BAD_DATA;

    BeAssert(lzmaDgndbHeader.IsLzma2());
    return Uncompress(outStream, inStream, true, tracker);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::UncompressDgnDbBlob (bvector<Byte>&out, uint32_t expectedSize, void const*inputBuffer, uint32_t inputSize, Byte*header, uint32_t headerSize)
    {
    //  Set up the input stream
    MemoryLzmaInStream  inStream(inputBuffer, inputSize);
    if (headerSize != sizeof (EmbeddedLzmaHeader) + 1)
        return ZIP_ERROR_BAD_DATA;

    EmbeddedLzmaHeader  dgndbHeader(EmbeddedLzmaHeader::LZMA2);
    memcpy(&dgndbHeader, header, sizeof(EmbeddedLzmaHeader));

    if (!dgndbHeader.IsValid())
        return ZIP_ERROR_BAD_DATA;

    inStream.SetHeaderData(header + sizeof(EmbeddedLzmaHeader), 1);

    //  Set up the output stream
    MemoryLzmaOutStream outStream(out);

    return Uncompress(outStream, inStream, true, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::Uncompress (bvector<Byte>&out, void const*inputBuffer, uint32_t inputSize)
    {
    MemoryLzmaInStream  inStream(inputBuffer, inputSize);
    MemoryLzmaOutStream outStream(out);

    return Uncompress(outStream, inStream, true, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::Uncompress (ILzmaOutputStream& out, ILzmaInputStream& in, bool isLzma2, ICompressProgressTracker* tracker)
    {
    SevenZImpl  sevenZ (out, in);

    ICompressProgressImpl progressImpl (tracker);
    sevenZ.SetICompressProgress (progressImpl);

    return sevenZ.Uncompress(isLzma2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
LzmaEncoder::LzmaEncoder (uint32_t dictionarySize) : m_enc2Props (NULL)
    {
    uint32_t level = 7;
    dictionarySize = getDictionarySize(dictionarySize);

    m_enc2Props = new CLzma2EncProps ();
    Lzma2EncProps_Init(m_enc2Props);
    m_enc2Props->lzmaProps.dictSize = (int)dictionarySize;
    m_enc2Props->lzmaProps.level = level;
    m_enc2Props->numTotalThreads = 8;
    Lzma2EncProps_Normalize(m_enc2Props);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
void LzmaEncoder::SetBlockSize(uint32_t blockSize)
    {
    m_enc2Props->blockSize = blockSize < m_enc2Props->lzmaProps.dictSize ? m_enc2Props->lzmaProps.dictSize : blockSize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
LzmaEncoder::~LzmaEncoder ()
    {
    delete m_enc2Props;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::CompressDgnDb (Utf8CP targetFile, Utf8CP sourceFile, ICompressProgressTracker* tracker, bool supportRandomAccess)
    {
    WString targetFileW (targetFile, BentleyCharEncoding::Utf8);
    WString sourceFileW (sourceFile, BentleyCharEncoding::Utf8);

    return CompressDgnDb (BeFileName(targetFileW.c_str()), BeFileName (sourceFileW.c_str()), tracker, supportRandomAccess);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::CompressDgnDb (BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* tracker, bool supportRandomAccess)
    {
    if (!BeFileName::DoesPathExist(sourceFile.GetName()))
        return ZIP_ERROR_FILE_DOES_NOT_EXIST;

    BeFileLzmaInStream  inStream;
    if (BSISUCCESS != inStream.OpenInputFile(sourceFile))
        return ZIP_ERROR_CANNOT_OPEN_INPUT;

    BeFileLzmaOutStream outStream;
    if (BeFileStatus::Success != outStream.CreateOutputFile(targetFile))
        return ZIP_ERROR_CANNOT_OPEN_OUTPUT;

    SevenZImpl  sevenZ (outStream, inStream);

    ICompressProgressImpl progressImpl (tracker);
    sevenZ.SetICompressProgress (progressImpl);

    LzmaDgnDbHeader     lzmaDgndbHeader (DGNDB_LZMA_MARKER, LzmaDgnDbHeader::LZMA2, inStream._GetSize());
    uint32_t bytesWritten;
    if (outStream._Write (&lzmaDgndbHeader, sizeof (lzmaDgndbHeader), bytesWritten) != ZIP_SUCCESS)
        return ZIP_ERROR_WRITE_ERROR;

    return sevenZ.Compress2(m_enc2Props, supportRandomAccess);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::Compress (ILzmaOutputStream& outStream, ILzmaInputStream& inStream, ICompressProgressTracker* tracker, bool supportRandomAccess)
    {
    SevenZImpl  sevenZ (outStream, inStream);

    ICompressProgressImpl progressImpl (tracker);
    sevenZ.SetICompressProgress (progressImpl);

    return sevenZ.Compress2(m_enc2Props, supportRandomAccess);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::Compress(bvector<Byte>& out, void const *input, uint32_t sizeInput, ICompressProgressTracker* tracker, bool supportRandomAccess)
    {
    MemoryLzmaInStream  inStream(input, sizeInput);
    MemoryLzmaOutStream outStream(out);

    ZipErrors result = Compress(outStream, inStream, tracker, supportRandomAccess);
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2013
//---------------------------------------------------------------------------------------
DbResult Db::SaveCreationDate()
    {
    return SavePropertyString(Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToUtf8String());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2013
//---------------------------------------------------------------------------------------
DbResult Db::QueryCreationDate(DateTime& creationDate) const
    {
    Utf8String date;
    DbResult rc = QueryProperty(date, Properties::CreationDate());
    if (BE_SQLITE_ROW != rc)
        return rc;

    return (BSISUCCESS == DateTime::FromString(creationDate, date.c_str())) ? BE_SQLITE_ROW : BE_SQLITE_MISMATCH;
    }

BeSQLiteLib::ILanguageSupport* BeSQLiteLib::s_languageSupport;
void BeSQLiteLib::SetLanguageSupport(ILanguageSupport* value) {s_languageSupport = value;}
BeSQLiteLib::ILanguageSupport* BeSQLiteLib::GetLanguageSupport() { return s_languageSupport; }

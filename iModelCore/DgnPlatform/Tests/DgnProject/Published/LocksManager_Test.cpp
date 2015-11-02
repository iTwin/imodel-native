/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/LocksManager_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define SERVER_Table "Locks"
#define SERVER_LockId "Id"
#define SERVER_LockType "Type"
#define SERVER_BcId "Owner"
#define SERVER_Exclusive "Exclusive"

//#define DUMP_SERVER 1

// ###TODO move down to LocksManager.h
typedef bset<DgnLock, DgnLock::IdentityComparator> DgnLockSet;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksServer
{
    Db m_db;
    bool m_offline;

    LocksServer();

    bool QueryLocksHeld(LockRequestCR reqs, BeBriefcaseId bc);
    LockStatus AcquireLocks(LockRequestCR reqs, BeBriefcaseId bc);
    LockStatus RelinquishLocks(BeBriefcaseId bc);

    bool AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId requestor);
    LockStatus QueryLocks(DgnLockSet& locks, BeBriefcaseId bc);
    void Reset();
    void Dump();

    LockLevel QueryLevel(LockableId id, BeBriefcaseId bc);
    int32_t QueryLockCount(BeBriefcaseId bc);

    void SetOffline(bool offline) { m_offline = offline; }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LockRequestVirtualSet : VirtualSet
{
    LockRequestCR m_request;

    LockRequestVirtualSet(LockRequestCR request) : m_request(request) { }

    virtual bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        if (3 != nVals)
            {
            BeAssert(false);
            return false;
            }

        LockableId id(static_cast<LockableType>(vals[0].GetValueInt()), BeInt64Id(vals[1].GetValueUInt64()));
        DgnLock lock(id, 0 == vals[2].GetValueInt() ? LockLevel::Shared : LockLevel::Exclusive);
        return _IsLockInRequest(lock);
        }

    virtual bool _IsLockInRequest(DgnLockCR lock) const = 0;
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct InRequestVirtualSet : LockRequestVirtualSet
{
    InRequestVirtualSet(LockRequestCR request) : LockRequestVirtualSet(request) { }

    virtual bool _IsLockInRequest(DgnLockCR lock) const override
        {
        return m_request.Contains(lock);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct RequestConflictsVirtualSet : LockRequestVirtualSet
{
    RequestConflictsVirtualSet(LockRequestCR request) : LockRequestVirtualSet(request) { }

    virtual bool _IsLockInRequest(DgnLockCR lock) const override
        {
        // input: an entry for a lock ID in our request, from the server's db
        // output: true if the input lock conflicts with our request
        DgnLockCP found = m_request.Find(lock.GetLockableId());
        BeAssert(nullptr != found);
        return nullptr != found && (found->IsExclusive() || lock.IsExclusive());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LocksServer::LocksServer() : m_offline(false)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    DbResult result = m_db.CreateNewDb(BEDB_MemoryDb);
    if (BE_SQLITE_OK == result)
        {
        result = m_db.CreateTable(SERVER_Table,
                    SERVER_LockId " INTEGER,"
                    SERVER_LockType " INTEGER,"
                    SERVER_BcId " INTEGER,"
                    SERVER_Exclusive " INTEGER,"
                    "PRIMARY KEY(" SERVER_LockId "," SERVER_LockType "," SERVER_BcId ")");
        }

    BeAssert(BE_SQLITE_OK == result);
    m_offline = (BE_SQLITE_OK != result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void bindBcId(Statement& stmt, int index, BeBriefcaseId id)
    {
    stmt.BindInt(index, static_cast<int32_t>(id.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocksServer::RelinquishLocks(BeBriefcaseId bc)
    {
    if (m_offline)
        return LockStatus::ServerUnavailable;

    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_BcId "=?");
    bindBcId(stmt, 1, bc);
    if (BE_SQLITE_DONE != stmt.Step())
        BeAssert(false);

    return LockStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocksServer::QueryLocksHeld(LockRequestCR reqs, BeBriefcaseId bc)
    {
    if (m_offline)
        return false;

    Statement stmt;
    stmt.Prepare(m_db, "SELECT count(*) FROM " SERVER_Table
                 " WHERE " SERVER_BcId "=?"
                 " AND InVirtualSet(@vset," SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive ")");

    InRequestVirtualSet vset(reqs);
    bindBcId(stmt, 1, bc);
    stmt.BindVirtualSet(2, vset);

    if (BE_SQLITE_ROW != stmt.Step())
        return false;

    return stmt.GetValueInt(0) == reqs.Size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocksServer::QueryLocks(DgnLockSet& locks, BeBriefcaseId bc)
    {
    if (m_offline)
        return LockStatus::ServerUnavailable;

    locks.clear();
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive " FROM " SERVER_Table " WHERE " SERVER_BcId "=?");
    bindBcId(stmt, 1, bc);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        LockableId id(static_cast<LockableType>(stmt.GetValueInt(0)), stmt.GetValueId<BeInt64Id>(1));
        locks.insert(DgnLock(id, static_cast<LockLevel>(stmt.GetValueInt(2))));
        }

    return LockStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocksServer::AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId bc)
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT count(*) FROM " SERVER_Table
                   " WHERE " SERVER_BcId " != ?"
                   " AND NOT InVirtualSet(@vset," SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive ")");

    RequestConflictsVirtualSet vset(reqs);
    bindBcId(stmt, 1, bc);
    stmt.BindVirtualSet(2, vset);

    auto rc = stmt.Step();
    BeAssert(BE_SQLITE_ROW == rc);
    return BE_SQLITE_ROW == rc && 0 == stmt.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockLevel LocksServer::QueryLevel(LockableId id, BeBriefcaseId bc)
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Exclusive " FROM " SERVER_Table
                       " WHERE " SERVER_LockType "=? AND " SERVER_LockId "=? AND " SERVER_BcId "=?");
    stmt.BindInt(1, static_cast<int32_t>(id.GetType()));
    stmt.BindId(2, id.GetId());
    bindBcId(stmt, 3, bc);
    auto rc = stmt.Step();
    if (BE_SQLITE_ROW == rc)
        return 0 != stmt.GetValueInt(0) ? LockLevel::Exclusive : LockLevel::Shared;

    BeAssert(BE_SQLITE_DONE == rc);
    return LockLevel::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LocksServer::Reset()
    {
    auto rc = m_db.ExecuteSql("DELETE FROM " SERVER_Table);
    BeAssert(BE_SQLITE_OK == rc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocksServer::AcquireLocks(LockRequestCR reqs, BeBriefcaseId bc)
    {
    if (!AreLocksAvailable(reqs, bc))
        return LockStatus::AlreadyHeld;

    for (auto const& req : reqs)
        {
        Statement select;
        select.Prepare(m_db, "SELECT " SERVER_Exclusive ",rowid FROM " SERVER_Table
                       " WHERE " SERVER_LockType "=? AND " SERVER_LockId "=? AND " SERVER_BcId "=?");
        select.BindInt(1, static_cast<int32_t>(req.GetType()));
        select.BindId(2, req.GetId());
        bindBcId(select, 3, bc);

        switch (select.Step())
            {
            case BE_SQLITE_DONE:
                {
                Statement stmt;
                stmt.Prepare(m_db, "INSERT INTO " SERVER_Table " (" SERVER_LockType "," SERVER_LockId "," SERVER_BcId "," SERVER_Exclusive ") VALUES(?,?,?,?)");

                stmt.BindInt(1, static_cast<int32_t>(req.GetType()));
                stmt.BindId(2, req.GetId());
                bindBcId(stmt, 3, bc);
                stmt.BindInt(4, req.IsExclusive() ? 1 : 0);

                if (BE_SQLITE_DONE != stmt.Step())
                    BeAssert(false);

                break;
                }
            case BE_SQLITE_ROW:
                {
                auto isExclusive = select.GetValueInt(0) != 0;
                if (!isExclusive && req.IsExclusive())
                    {
                    Statement stmt;
                    stmt.Prepare(m_db, "UPDATE " SERVER_Table " SET " SERVER_Exclusive "=1 WHERE rowid=?");
                    stmt.BindInt(1, select.GetValueInt(1));
                    if (BE_SQLITE_DONE != stmt.Step())
                        BeAssert(false);
                    }

                break;
                }
            default:
                BeAssert(false);
                break;
            }
        }

    return LockStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t LocksServer::QueryLockCount(BeBriefcaseId bc)
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT count(*) FROM " SERVER_Table " WHERE " SERVER_BcId "=?");
    bindBcId(stmt, 1, bc);
    auto rc = stmt.Step();
    BeAssert(BE_SQLITE_ROW == rc);
    return BE_SQLITE_ROW == rc ? stmt.GetValueInt(0) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LocksServer::Dump()
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT * FROM " SERVER_Table);
    printf(">>>> Dumping server >>>>\n");
    stmt.DumpResults();
    printf("<<<<<<<<<<<<<<<<<<<<<<<<\n");
    }

/*---------------------------------------------------------------------------------**//**
* Queries server on every request
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct StatelessLocksManager : ILocksManager
{
    LocksServer& m_server;

    StatelessLocksManager(DgnDbR db, LocksServer& server) : ILocksManager(db), m_server(server) { }

    BeBriefcaseId GetId() const { return GetDgnDb().GetBriefcaseId(); }

    virtual bool _QueryLocksHeld(LockRequestR locks, bool localOnly) override
        {
        return localOnly ? false : m_server.QueryLocksHeld(locks, GetId());
        }
    virtual LockStatus _AcquireLocks(LockRequestR locks) override
        {
        return m_server.AcquireLocks(locks, GetId());
        }
    virtual LockStatus _RelinquishLocks() override
        {
        return m_server.RelinquishLocks(GetId());
        }
    virtual void _OnElementInserted(DgnElementId id) override { }
    virtual void _OnModelInserted(DgnModelId id) override { }
};

#define LOCAL_Table "Locks"
#define LOCAL_Type "Type"
#define LOCAL_Id "Id"
#define LOCAL_Level "Level"
#define LOCAL_Values "(" LOCAL_Type "," LOCAL_Id "," LOCAL_Level ")"

#define STMT_SelectExisting "SELECT " LOCAL_Level ",rowid FROM " LOCAL_Table " WHERE " LOCAL_Type "=? AND " LOCAL_Id "=?"
#define STMT_InsertNew "INSERT INTO " LOCAL_Table " " LOCAL_Values " VALUES (?,?,?)"
#define STMT_UpdateLevel "UPDATE " LOCAL_Table " SET " LOCAL_Level "=? WHERE rowid=?"
#define STMT_SelectInSet "SELECT " LOCAL_Type "," LOCAL_Id " FROM " LOCAL_Table " WHERE InVirtualSet(@vset," LOCAL_Type "," LOCAL_Id "," LOCAL_Level ")"
#define STMT_SelectElementInModel " SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?"
#define STMT_InsertOrReplace "INSERT OR REPLACE INTO " LOCAL_Table " " LOCAL_Values " VALUES(?,?,?)"

/*---------------------------------------------------------------------------------**//**
* Stores a local sqlite db containing locks obtained by a briefcase.
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalLocksManager : ILocksManager
{
    enum class DbState { New, Ready, Invalid };

    LocksServer& m_server;
    Db m_db;
    DbState m_dbState;

    LocalLocksManager(DgnDbR db, LocksServer& server);

    BeBriefcaseId GetId() const { return GetDgnDb().GetBriefcaseId(); }

    virtual bool _QueryLocksHeld(LockRequestR locks, bool localOnly) override;
    virtual LockStatus _AcquireLocks(LockRequestR locks) override;
    virtual LockStatus _RelinquishLocks() override;
    virtual void _OnElementInserted(DgnElementId id) override
        {
        if (Validate())
            {
            Insert(LockableId(id), LockLevel::Exclusive);
            Save();
            }
        }
    virtual void _OnModelInserted(DgnModelId id) override
        {
        if (Validate())
            {
            Insert(LockableId(id), LockLevel::Exclusive);
            Save();
            }
        }

    void Insert(LockableId id, LockLevel level, bool overwrite=false);
    template<typename T> void Insert(T const& locks, bool checkExisting);
    DbResult Save() { return m_db.SaveChanges(); }

    void Cull(LockRequestR locks);

    bool Validate();

    template<typename T> LockLevel QueryLevel(T const& obj)
        {
        LockableId id = LocksManagerTest::MakeLockableId(obj);
        return QueryLevel(id);
        }
    LockLevel QueryLevel(LockableId id);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LocalLocksManager::LocalLocksManager(DgnDbR db, LocksServer& server) : ILocksManager(db), m_server(server), m_dbState(DbState::New)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalLocksManager::Validate()
    {
    switch (m_dbState)
        {
        case DbState::Ready:    return true;
        case DbState::Invalid: return false;
        }

    // ###TODO? Look for an existing locks db and don't throw away existing if found?
    BeFileName filename = GetLockTableFileName();
    filename.BeDeleteFile();

    DbResult result = m_db.CreateNewDb(filename);
    if (BE_SQLITE_OK == result)
        {
        result = m_db.CreateTable(LOCAL_Table,
                                    LOCAL_Type " INTEGER,"
                                    LOCAL_Id " INTEGER,"
                                    LOCAL_Level " INTEGER,"
                                    "PRIMARY KEY(" LOCAL_Type "," LOCAL_Id ")");
        }

    m_dbState = BE_SQLITE_OK == result ? DbState::Ready : DbState::Invalid;
    BeAssert(DbState::Ready == m_dbState);

    // Request a list of current locks from server
    DgnLockSet locks;
    if (LockStatus::Success == m_server.QueryLocks(locks, GetId()))
        Insert(locks, false);

    return DbState::Ready == m_dbState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void bindEnum(Statement& stmt, int32_t index, T val)
    {
    stmt.BindInt(index, static_cast<int32_t>(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalLocksManager::Insert(LockableId id, LockLevel level, bool overwrite)
    {
    CachedStatementPtr stmt = m_db.GetCachedStatement(overwrite ? STMT_InsertOrReplace : STMT_InsertNew);
    bindEnum(*stmt, 1, id.GetType());
    stmt->BindId(2, id.GetId());
    bindEnum(*stmt, 3, level);

    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ModelElementLocks : DbTableIterator
{
    BeInt64Id m_modelId;

    ModelElementLocks(BeInt64Id modelId, DgnDbCR db) : DbTableIterator((DbCR)db), m_modelId(modelId) { }

    struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
    {
        Entry(StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) { }

        LockableType GetType() const { return LockableType::Element; }
        LockLevel GetLevel() const { return LockLevel::Exclusive; }
        BeInt64Id GetId() const { return m_sql->GetValueId<BeInt64Id>(0); }
        bool IsExclusive() const { return true; }

        Entry const& operator*() const { return *this; }
    };

    typedef Entry const_iterator;
    typedef const_iterator iterator;
    const_iterator begin() const;
    const_iterator end() const { return Entry(nullptr, false); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelElementLocks::const_iterator ModelElementLocks::begin() const
    {
    if (!m_stmt.IsValid())
        {
        m_db->GetCachedStatement(m_stmt, STMT_SelectElementInModel);
        m_stmt->BindId(1, m_modelId);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockLevel LocalLocksManager::QueryLevel(LockableId id)
    {
    CachedStatementPtr select = m_db.GetCachedStatement(STMT_SelectExisting);
    bindEnum(*select, 1, id.GetType());
    select->BindId(2, id.GetId());
    return BE_SQLITE_ROW == select->Step() ? static_cast<LockLevel>(select->GetValueInt(0)) : LockLevel::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void LocalLocksManager::Insert(T const& locks, bool checkExisting)
    {
    CachedStatementPtr select = checkExisting ? m_db.GetCachedStatement(STMT_SelectExisting) : nullptr,
                       insert = m_db.GetCachedStatement(STMT_InsertNew),
                       update = checkExisting ? m_db.GetCachedStatement(STMT_UpdateLevel) : nullptr;

    // If we obtain exclusive lock on a model, we want to record an exclusive lock on all its elements
    // Likewise, an exclusive lock on the db should record an exclusive lock on everything in it
    // Doing that within the loop could be problematic - so record such locks during iteration and then post-process
    bool dbExclusivelyLocked = false;
    bset<BeInt64Id> exclusivelyLockedModels;
    for (auto const& lock : locks)
        {
        auto step = BE_SQLITE_DONE;
        if (checkExisting)
            {
            bindEnum(*select, 1, lock.GetType());
            select->BindId(2, lock.GetId());
            step = select->Step();
            }

        switch (step)
            {
            case BE_SQLITE_DONE:
                {
                bindEnum(*insert, 1, lock.GetType());
                insert->BindId(2, lock.GetId());
                bindEnum(*insert, 3, lock.GetLevel());

                insert->Step();
                insert->Reset();
                break;
                }
            case BE_SQLITE_ROW:
                {
                if (static_cast<LockLevel>(select->GetValueInt(0)) > lock.GetLevel())
                    {
                    update->BindInt(1, select->GetValueInt(1));
                    update->Step();
                    update->Reset();
                    }
                break;
                }
            default:
                BeAssert(false);
                break;
            }

        if (select.IsValid())
            select->Reset();

        if (lock.IsExclusive())
            {
            switch (lock.GetType())
                {
                case LockableType::Db:
                    dbExclusivelyLocked = true;
                    exclusivelyLockedModels.clear();
                    for (auto const& model : GetDgnDb().Models().MakeIterator())
                        exclusivelyLockedModels.insert(model.GetModelId());

                    break;
                case LockableType::Model:
                    if (!dbExclusivelyLocked)
                        exclusivelyLockedModels.insert(lock.GetId());

                    break;
                }
            }
        }

    // Now process any exclusive locks
    select = nullptr;
    insert = nullptr;
    update = nullptr;

    if (dbExclusivelyLocked)
        {
        for (auto const& model : exclusivelyLockedModels)
            Insert(LockableId(LockableType::Model, model), LockLevel::Exclusive, true);
        }

    if (!exclusivelyLockedModels.empty())
        {
        for (auto const& model : exclusivelyLockedModels)
            {
            ModelElementLocks elemLocks(model, GetDgnDb());
            Insert(elemLocks, true);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalLocksManager::Cull(LockRequestR locks)
    {
    struct VSet : VirtualSet
    {
        LockRequestCR m_locks;
        VSet(LockRequestCR locks) : m_locks(locks) { }
        virtual bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            BeAssert(3 == nVals);
            LockableId id(static_cast<LockableType>(vals[0].GetValueInt()), BeInt64Id(vals[1].GetValueUInt64()));
            return m_locks.Contains(DgnLock(id, static_cast<LockLevel>(vals[2].GetValueInt())));
            }
    };

    VSet vset(locks);
    CachedStatementPtr stmt = m_db.GetCachedStatement(STMT_SelectInSet);
    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        LockableId id(static_cast<LockableType>(stmt->GetValueInt(0)), BeInt64Id(stmt->GetValueUInt64(1)));
        locks.Remove(id);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocalLocksManager::_RelinquishLocks()
    {
    if (!Validate())
        return LockStatus::SyncError;
    else if (BE_SQLITE_OK == m_db.ExecuteSql("DELETE FROM " LOCAL_Table) && BE_SQLITE_OK == Save())
        return m_server.RelinquishLocks(GetId());
    else
        return LockStatus::SyncError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalLocksManager::_QueryLocksHeld(LockRequestR locks, bool localOnly)
    {
    if (!Validate())
        return false;

    Cull(locks);
    if (localOnly || locks.Empty())
        return locks.Empty();
    else
        return m_server.QueryLocksHeld(locks, GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocalLocksManager::_AcquireLocks(LockRequestR locks)
    {
    if (!Validate())
        return LockStatus::SyncError;

    Cull(locks);
    if (locks.Empty())
        return LockStatus::Success;

    auto status = m_server.AcquireLocks(locks, GetId());
    if (LockStatus::Success != status)
        return status;

    Insert(locks, true);
    Save();
    return LockStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksManagerTest : public ::testing::Test, DgnPlatformLib::Host::LocksAdmin
{
    mutable LocksServer m_server;
    ScopedDgnHost m_host;
    DgnDbPtr m_db;
    DgnModelId m_modelId;
    DgnElementId m_elemId;
    bool m_createStateless;

    LocksManagerTest() : m_createStateless(false)
        {
        m_host.SetLocksAdmin(this);
        }

    virtual ILocksManagerPtr _CreateLocksManager(DgnDbR db) const override
        {
        if (m_createStateless)
            return new StatelessLocksManager(db, m_server);
        else
            return new LocalLocksManager(db, m_server);
        }

    void SetupDb(WCharCP testFile, BeBriefcaseId bcId)
        {
        WCharCP baseFile = L"3dMetricGeneral.idgndb";
        BeFileName outFileName;
        ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFile, __FILE__));
        m_db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(m_db.IsValid());

        m_modelId = DgnModel::DictionaryId();
        m_elemId = DgnCategory::QueryFirstCategoryId(*m_db);

        m_db->ChangeBriefcaseId(bcId);
        }

    static LockableId MakeLockableId(DgnElementCR el) { return LockableId(el.GetElementId()); }
    static LockableId MakeLockableId(DgnModelCR model) { return LockableId(model.GetModelId()); }
    static LockableId MakeLockableId(DgnDbCR db) { return LockableId(db); }

    void ExpectLevel(LockableId id, BeBriefcaseId bc, LockLevel expLevel)
        {
        EXPECT_EQ(expLevel, m_server.QueryLevel(id, bc));
        }
    template<typename T> void ExpectLevel(T const& obj, BeBriefcaseId bc, LockLevel level)
        {
        ExpectLevel(MakeLockableId(obj), bc, level);
        }

    template<typename T> LockStatus Acquire(T const& obj, BeBriefcaseId bc, LockLevel level)
        {
#ifdef DUMP_SERVER
        printf("Before acquiring locks...\n");
        m_server.Dump();
#endif

        LockRequest req;
        req.Insert(obj, level);
        LockStatus status = m_db->Locks().AcquireLocks(req);

#ifdef DUMP_SERVER
        printf("After acquiring locks...\n");
        m_server.Dump();
#endif

        return status;
        }

    template<typename T> void ExpectAcquire(T const& obj, BeBriefcaseId bc, LockLevel level)
        {
        EXPECT_EQ(LockStatus::Success, Acquire(obj, bc, level));
        }

    template<typename T> void ExpectDenied(T const& obj, BeBriefcaseId bc, LockLevel level)
        {
        EXPECT_EQ(LockStatus::AlreadyHeld, Acquire(obj, bc, level));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockBriefcaseLocksTest : LocksManagerTest
{
    // Some pretend briefcase IDs
    BeBriefcaseId m_bcA;
    BeBriefcaseId m_bcB;

    MockBriefcaseLocksTest() : m_bcA(1), m_bcB(2) { }

    void Test_SingleBriefcase(bool useStatelessManager);
};

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MockBriefcaseLocksTest::Test_SingleBriefcase(bool useStateless)
    {
    m_createStateless = useStateless;
    SetupDb(L"SingleBriefcase.dgndb", m_bcA);

    DgnDbCR db = *m_db;
    DgnModelPtr pModel = db.Models().GetModel(m_modelId);
    ASSERT_TRUE(pModel.IsValid());
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    ASSERT_TRUE(cpEl.IsValid());

    DgnModelCR model = *pModel;
    DgnElementCR el = *cpEl;
    BeBriefcaseId bc = m_bcA;

    ExpectAcquire(model, bc, LockLevel::Shared);
    ExpectLevel(model, bc, LockLevel::Shared);
    ExpectLevel(el, bc, LockLevel::None);
    ExpectLevel(db, bc, LockLevel::None);

    ExpectAcquire(model, bc, LockLevel::Exclusive);
    ExpectLevel(model, bc, LockLevel::Exclusive);
    ExpectLevel(db, bc, LockLevel::Shared);
    ExpectLevel(el, bc, LockLevel::None);

    ExpectAcquire(db, bc, LockLevel::Exclusive);
    ExpectLevel(db, bc, LockLevel::Exclusive);
    ExpectLevel(model, bc, LockLevel::Exclusive);
    ExpectLevel(el, bc, LockLevel::None);

    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());
    m_server.Reset();

    ExpectAcquire(el, bc, LockLevel::Shared);
    ExpectLevel(el, bc, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, bc, LockLevel::Shared);
    ExpectLevel(db, bc, LockLevel::Shared);

    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());
    m_server.Reset();
    
    ExpectAcquire(model, bc, LockLevel::Exclusive);
    ExpectLevel(model, bc, LockLevel::Exclusive);
    ExpectLevel(db, bc, LockLevel::Shared);

    ExpectAcquire(model, bc, LockLevel::Shared);
    ExpectLevel(model, bc, LockLevel::Exclusive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MockBriefcaseLocksTest, SingleBriefcase_Stateless) { Test_SingleBriefcase(true); }
TEST_F(MockBriefcaseLocksTest, SingleBriefcase_Stateful) { Test_SingleBriefcase(false); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MockBriefcaseLocksTest, ExclusiveLocks)
    {
    BeBriefcaseId bc = m_bcA;
    SetupDb(L"ExclusiveLocks.dgndb", bc);

    DgnDbCR db = *m_db;
    DgnModelPtr model = db.Models().GetModel(m_modelId);
    DgnElementCPtr el = db.Elements().GetElement(m_elemId);

    auto& locks = static_cast<LocalLocksManager&>(db.Locks());

    // An exclusive model lock results in exclusive locks on all of its elements
    ExpectAcquire(*model, bc, LockLevel::Exclusive);
    EXPECT_EQ(locks.QueryLevel(*model), LockLevel::Exclusive);
    EXPECT_EQ(locks.QueryLevel(*el), LockLevel::Exclusive);
    EXPECT_EQ(locks.QueryLevel(db), LockLevel::Shared);

    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());
    EXPECT_EQ(locks.QueryLevel(*model), LockLevel::None);
    EXPECT_EQ(locks.QueryLevel(*el), LockLevel::None);
    EXPECT_EQ(locks.QueryLevel(db), LockLevel::None);

    // An exclusive db lock results in exclusive locks on all models and elements
    ExpectAcquire(db, bc, LockLevel::Exclusive);
    EXPECT_EQ(locks.QueryLevel(*model), LockLevel::Exclusive);
    EXPECT_EQ(locks.QueryLevel(*el), LockLevel::Exclusive);
    EXPECT_EQ(locks.QueryLevel(db), LockLevel::Exclusive);
    }


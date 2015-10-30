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

    virtual bool _QueryLocksHeld(LockRequestCR locks, bool localOnly) override
        {
        return localOnly ? false : m_server.QueryLocksHeld(locks, GetId());
        }
    virtual LockStatus _AcquireLocks(LockRequestCR locks) override
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

//#define DUMP_SERVER 1

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
    bool m_useLocksApi;

    LocksManagerTest() : m_useLocksApi(false)
        {
        m_host.SetLocksAdmin(this);
        }

    virtual ILocksManagerPtr _CreateLocksManager(DgnDbR db) const override
        {
        return new StatelessLocksManager(db, m_server);
        }

    void SetupDb(WCharCP testFile)
        {
        WCharCP baseFile = L"3dMetricGeneral.idgndb";
        BeFileName outFileName;
        ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFile, __FILE__));
        m_db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(m_db.IsValid());

        m_modelId = DgnModel::DictionaryId();
        m_elemId = DgnCategory::QueryFirstCategoryId(*m_db);
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
        LockStatus status = m_useLocksApi ? m_db->Locks().AcquireLocks(req) : m_server.AcquireLocks(req, bc);

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
};

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MockBriefcaseLocksTest, SingleBriefcase)
    {
    SetupDb(L"SingleBriefcase.dgndb");

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

    m_server.Reset();

    ExpectAcquire(el, bc, LockLevel::Shared);
    ExpectLevel(el, bc, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, bc, LockLevel::Shared);
    ExpectLevel(db, bc, LockLevel::Shared);

    m_server.Reset();
    
    ExpectAcquire(model, bc, LockLevel::Exclusive);
    ExpectLevel(model, bc, LockLevel::Exclusive);
    ExpectLevel(db, bc, LockLevel::Shared);

    ExpectAcquire(model, bc, LockLevel::Shared);
    ExpectLevel(model, bc, LockLevel::Exclusive);
    }


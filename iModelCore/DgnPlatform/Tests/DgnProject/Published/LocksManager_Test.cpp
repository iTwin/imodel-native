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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpectLocksEqual(DgnLockCR a, DgnLockCR b)
    {
    EXPECT_EQ(a.GetLevel(), b.GetLevel());
    EXPECT_EQ(a.GetType(), b.GetType());
    EXPECT_EQ(a.GetId(), b.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpectLockSetsEqual(DgnLockSet const& a, DgnLockSet const& b)
    {
    EXPECT_EQ(a.size(), b.size());
    if (a.size() != b.size())
        return;

    auto iterA = a.begin(), iterB = b.begin();
    while (iterA != a.end())
        ExpectLocksEqual(*iterA++, *iterB++);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpectRequestsEqual(LockRequestCR a, LockRequestCR b)
    {
    EXPECT_EQ(a.GetOptions(), b.GetOptions());
    ExpectLockSetsEqual(a.GetLockSet(), b.GetLockSet());
    }

/*---------------------------------------------------------------------------------**//**
* Mock server, since we currently lack a real server.
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksServer : ILocksServer
{
private:
    typedef LockRequest::ResponseOptions ResponseOptions;

    Db m_db;
    bool m_offline;

    virtual bool _QueryLocksHeld(LockRequestCR reqs, DgnDbR db) override;
    virtual LockRequest::Response _AcquireLocks(LockRequestCR reqs, DgnDbR db) override;
    virtual LockStatus _RelinquishLocks(DgnDbR db) override;
    virtual LockStatus _QueryLockLevel(LockLevel& level, LockableId lockId, DgnDbR db) override;
    virtual LockStatus _QueryLocks(DgnLockSet& locks, DgnDbR db) override;
    virtual LockStatus _QueryOwnership(DgnLockOwnershipR ownership, LockableId lockId) override;

    bool AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId requestor);
    void GetDeniedLocks(DgnLockSet& locks, LockRequestCR reqs, BeBriefcaseId bcId);
    void Reset();

    int32_t QueryLockCount(BeBriefcaseId bc);

    void SetOffline(bool offline) { m_offline = offline; }
public:
    LocksServer();

    void Dump(Utf8CP descr=nullptr);
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
LockStatus LocksServer::_RelinquishLocks(DgnDbR db)
    {
    if (m_offline)
        return LockStatus::ServerUnavailable;

    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_BcId "=?");
    bindBcId(stmt, 1, db.GetBriefcaseId());
    if (BE_SQLITE_DONE != stmt.Step())
        BeAssert(false);

    return LockStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocksServer::_QueryLocksHeld(LockRequestCR inputReqs, DgnDbR db)
    {
    if (m_offline)
        return false;

    // Simulating serialization and deserialization of server request...
    Json::Value reqJson;
    inputReqs.ToJson(reqJson);
    LockRequest reqs;
    EXPECT_TRUE(reqs.FromJson(reqJson));
    ExpectRequestsEqual(inputReqs, reqs);

    Statement stmt;
    stmt.Prepare(m_db, "SELECT count(*) FROM " SERVER_Table
                 " WHERE " SERVER_BcId "=?"
                 " AND InVirtualSet(@vset," SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive ")");

    InRequestVirtualSet vset(reqs);
    bindBcId(stmt, 1, db.GetBriefcaseId());
    stmt.BindVirtualSet(2, vset);

    if (BE_SQLITE_ROW != stmt.Step())
        return false;

    return stmt.GetValueInt(0) == reqs.Size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocksServer::_QueryOwnership(DgnLockOwnershipR ownership, LockableId inputLockId)
    {
    ownership.Reset();
    if (m_offline)
        return LockStatus::ServerUnavailable;

    // Simulating serialization and deserialization of server request...
    Json::Value lockIdJson;
    inputLockId.ToJson(lockIdJson);
    LockableId lockId;
    EXPECT_TRUE(lockId.FromJson(lockIdJson));
    EXPECT_EQ(lockId, inputLockId);

    Statement stmt;
    stmt.Prepare(m_db, "SELECT" SERVER_Exclusive "," SERVER_BcId " FROM " SERVER_Table " WHERE " SERVER_LockType "=? AND " SERVER_LockId "=?");
    stmt.BindInt(1, static_cast<int32_t>(lockId.GetType()));
    stmt.BindId(2, lockId.GetId());

    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto owner = stmt.GetValueId<BeBriefcaseId>(1);
        bool exclusive = 0 != stmt.GetValueInt(0);
        if (exclusive)
            {
            EXPECT_EQ(LockLevel::None, ownership.GetLockLevel());
            ownership.SetExclusiveOwner(owner);
            }
        else
            {
            EXPECT_NE(LockLevel::Exclusive, ownership.GetLockLevel());
            ownership.AddSharedOwner(owner);
            }
        }

    return LockStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocksServer::_QueryLocks(DgnLockSet& locks, DgnDbR db)
    {
    if (m_offline)
        return LockStatus::ServerUnavailable;

    locks.clear();
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive " FROM " SERVER_Table " WHERE " SERVER_BcId "=?");
    bindBcId(stmt, 1, db.GetBriefcaseId());
    while (BE_SQLITE_ROW == stmt.Step())
        {
        LockableId id(static_cast<LockableType>(stmt.GetValueInt(0)), stmt.GetValueId<BeInt64Id>(1));
        locks.insert(DgnLock(id, 0 != stmt.GetValueInt(2) ? LockLevel::Exclusive : LockLevel::Shared));
        }

    return LockStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocksServer::AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId bcId)
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT count(*) FROM " SERVER_Table
                   " WHERE " SERVER_BcId " != ?"
                   " AND InVirtualSet(@vset," SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive ")");

    RequestConflictsVirtualSet vset(reqs);
    bindBcId(stmt, 1, bcId);
    stmt.BindVirtualSet(2, vset);

    auto rc = stmt.Step();
    BeAssert(BE_SQLITE_ROW == rc);
    return BE_SQLITE_ROW == rc && 0 == stmt.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LocksServer::GetDeniedLocks(DgnLockSet& locks, LockRequestCR reqs, BeBriefcaseId bcId)
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive
                       " FROM " SERVER_Table " WHERE " SERVER_BcId " != ?" " AND InVirtualSet(@vset," SERVER_LockType "," SERVER_LockId "," SERVER_Exclusive ")");

    RequestConflictsVirtualSet vset(reqs);
    bindBcId(stmt, 1, bcId);
    stmt.BindVirtualSet(2, vset);

    while (BE_SQLITE_ROW == stmt.Step())
        {
        LockableId id(static_cast<LockableType>(stmt.GetValueInt(0)), stmt.GetValueId<BeInt64Id>(1));
        auto level = (0 != stmt.GetValueInt(2)) ? LockLevel::Exclusive : LockLevel::Shared;
        DgnLock lock(id, level);
        auto inserted = locks.insert(lock);
        if (LockLevel::Exclusive == level)
            EXPECT_TRUE(inserted.second);   // If the server has granted an exclusive lock, it should not have any shared locks for same object.
        else if (!inserted.second)
            EXPECT_EQ(LockLevel::Shared, inserted.first->GetLevel()); // If more than one lock exists for the same object, they should all be shared locks.
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocksServer::_QueryLockLevel(LockLevel& level, LockableId id, DgnDbR db)
    {
    level = LockLevel::None;

    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Exclusive " FROM " SERVER_Table
                       " WHERE " SERVER_LockType "=? AND " SERVER_LockId "=? AND " SERVER_BcId "=?");
    stmt.BindInt(1, static_cast<int32_t>(id.GetType()));
    stmt.BindId(2, id.GetId());
    bindBcId(stmt, 3, db.GetBriefcaseId());
    auto rc = stmt.Step();
    if (BE_SQLITE_ROW == rc)
        {
        level = 0 != stmt.GetValueInt(0) ? LockLevel::Exclusive : LockLevel::Shared;
        return LockStatus::Success;
        }

    BeAssert(BE_SQLITE_DONE == rc);
    return LockStatus::Success;
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
LockRequest::Response LocksServer::_AcquireLocks(LockRequestCR inputReqs, DgnDbR db)
    {
    // Simulating serialization and deserialization of server request...
    Json::Value reqJson;
    inputReqs.ToJson(reqJson);
    LockRequest reqs;
    EXPECT_TRUE(reqs.FromJson(reqJson));
    ExpectRequestsEqual(inputReqs, reqs);

    if (!AreLocksAvailable(reqs, db.GetBriefcaseId()))
        {
        LockRequest::Response response(LockStatus::AlreadyHeld);
        if (ResponseOptions::None != (ResponseOptions::DeniedLocks & reqs.GetOptions()))
            GetDeniedLocks(response.GetDeniedLocks(), reqs, db.GetBriefcaseId());

        return response;
        }

    for (auto const& req : reqs)
        {
        Statement select;
        select.Prepare(m_db, "SELECT " SERVER_Exclusive ",rowid FROM " SERVER_Table
                       " WHERE " SERVER_LockType "=? AND " SERVER_LockId "=? AND " SERVER_BcId "=?");
        select.BindInt(1, static_cast<int32_t>(req.GetType()));
        select.BindId(2, req.GetId());
        bindBcId(select, 3, db.GetBriefcaseId());

        switch (select.Step())
            {
            case BE_SQLITE_DONE:
                {
                Statement stmt;
                stmt.Prepare(m_db, "INSERT INTO " SERVER_Table " (" SERVER_LockType "," SERVER_LockId "," SERVER_BcId "," SERVER_Exclusive ") VALUES(?,?,?,?)");

                stmt.BindInt(1, static_cast<int32_t>(req.GetType()));
                stmt.BindId(2, req.GetId());
                bindBcId(stmt, 3, db.GetBriefcaseId());
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

    return LockRequest::Response(LockStatus::Success);
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
void LocksServer::Dump(Utf8CP descr)
    {
#ifdef DUMP_SERVER
    Statement stmt;
    stmt.Prepare(m_db, "SELECT * FROM " SERVER_Table);
    printf(">>>> %s >>>>\n", nullptr != descr ? descr : "Dumping Server");
    stmt.DumpResults();
    printf("<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksManagerTest : public ::testing::Test, DgnPlatformLib::Host::LocksAdmin
{
    mutable LocksServer m_server;
    ScopedDgnHost m_host;
    DgnModelId m_modelId;
    DgnElementId m_elemId;

    LocksManagerTest()
        {
        m_host.SetLocksAdmin(this);
        BackDoor::ILocksManager::SetLockingEnabled(true);
        }

    ~LocksManagerTest()
        {
        BackDoor::ILocksManager::SetLockingEnabled(false);
        }

    virtual ILocksServerP _GetLocksServer(DgnDbR) const override { return &m_server; }

    DgnDbPtr SetupDb(WCharCP testFile, BeBriefcaseId bcId, WCharCP baseFile=L"3dMetricGeneral.idgndb")
        {
        BeFileName outFileName;
        EXPECT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFile, __FILE__));
        auto db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());
        if (!db.IsValid())
            return nullptr;

        TestDataManager::MustBeBriefcase(db, Db::OpenMode::ReadWrite); // sets a fake briefcase id and reopens w/ txn manager support turned on

        m_modelId = DgnModel::DictionaryId();
        m_elemId = DgnCategory::QueryFirstCategoryId(*db);

        db->ChangeBriefcaseId(bcId);
        return db;
        }

    static LockableId MakeLockableId(DgnElementCR el) { return LockableId(el.GetElementId()); }
    static LockableId MakeLockableId(DgnModelCR model) { return LockableId(model.GetModelId()); }
    static LockableId MakeLockableId(DgnDbCR db) { return LockableId(db); }

    static Utf8String MakeLockableName(LockableId lid)
        {
        Utf8CP typeName = nullptr;
        switch (lid.GetType())
            {
            case LockableType::Db: typeName = "DgnDb"; break;
            case LockableType::Model: typeName = "Model"; break;
            default: typeName = "Element"; break;
            }

        Utf8Char buf[0x20];
        BeStringUtilities::FormatUInt64(buf, _countof(buf), lid.GetId().GetValue(), HexFormatOptions());
        Utf8String name(typeName);
        name.append(1, ' ');
        name.append(buf);

        return name;
        }

    void ExpectLevel(LockableId id, LockLevel expLevel, DgnDbR db)
        {
        LockLevel actualLevel;
        EXPECT_EQ(LockStatus::Success, db.Locks().QueryLockLevel(actualLevel, id));
        EXPECT_EQ(expLevel, actualLevel) << MakeLockableName(id).c_str();
        }
    template<typename T> void ExpectLevel(T const& obj, LockLevel level)
        {
        ExpectLevel(MakeLockableId(obj), level, ExtractDgnDb(obj));
        }

    template<typename T> static DgnDbR ExtractDgnDb(T const& obj) { return obj.GetDgnDb(); }

    template<typename T> LockRequest::Response AcquireResponse(T const& obj, LockLevel level)
        {
        m_server.Dump("Before acquiring locks");

        LockRequest req(LockRequest::ResponseOptions::DeniedLocks);
        req.Insert(obj, level);
        DgnDbR db = ExtractDgnDb(obj);

        auto reponse = db.Locks().AcquireLocks(req);

        m_server.Dump("After acquiring locks");

        return response;
        }

    template<typename T> LockStatus Acquire(T const& obj, LockLevel level)
        {
        return AcquireResponse(obj, level).GetStatus();
        }

    template<typename T> void ExpectAcquire(T const& obj, LockLevel level)
        {
        EXPECT_EQ(LockStatus::Success, Acquire(obj, level));
        }

    template<typename T> void ExpectDenied(T const& obj, LockLevel level)
        {
        EXPECT_EQ(LockStatus::AlreadyHeld, Acquire(obj, level));
        }

    template<typename T> void ExpectInDeniedSet(T const& lockedObj, LockLevel level, LockRequest::Response const& response)
        {
        // Test that locked obj is in response
        auto lockableId = MakeLockableId(lockedObj);
        DgnLock lock(lockableId, level);
        auto found = response.GetDeniedLocks().find(lock);
        EXPECT_FALSE(found == response.GetDeniedLocks().end());
        if (response.GetDeniedLocks().end() != found)
            EXPECT_EQ(found->GetLevel(), level);

        // Test that response matches direct ownership query
        DgnLockOwnership ownership;
        EXPECT_EQ(LockStatus::Success, T_HOST.GetLocksAdmin()._GetLocksServer()->QueryOwnership(ownership, lockableId));
        EXPECT_EQ(level, ownership.GetLockLevel());
        auto owningBcId = ExtractDgnDb(lockedObj).GetBriefcaseId();
        switch (level)
            {
            case LockLevel::Exclusive:
                EXPECT_EQ(owningBcId, ownership.GetExclusiveOwner());
                break;
            case LockLevel::Shared:
                EXPECT_TRUE(ownership.GetSharedOwners().end() != ownership.GetSharedOwners().find(owningBcId));
                break;
            }
        }

    DgnModelPtr CreateModel(Utf8CP name, DgnDbR db)
        {
        DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
        PhysicalModelPtr model = new PhysicalModel(PhysicalModel::CreateParams(db, classId, DgnModel::CreateModelCode(name)));
        auto status = model->Insert();
        EXPECT_EQ(DgnDbStatus::Success, status);
        return DgnDbStatus::Success == status ? model : nullptr;
        }

    DgnElementCPtr CreateElement(DgnModelR model)
        {
        auto elem = model.Is3d() ? Create3dElement(model) : Create2dElement(model);
        auto persistentElem = elem->Insert();
        return persistentElem;
        }

    DgnElementPtr Create3dElement(DgnModelR model)
        {
        DgnDbR db = model.GetDgnDb();
        DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
        DgnCategoryId catId = DgnCategory::QueryHighestCategoryId(db);
        return PhysicalElement::Create(PhysicalElement::CreateParams(db, model.GetModelId(), classId, catId, Placement3d()));
        }

    DgnElementPtr Create2dElement(DgnModelR model)
        {
        DgnDbR db = model.GetDgnDb();
        return DrawingElement::Create(DrawingElement::CreateParams(db, model.GetModelId(), DrawingElement::QueryClassId(db), DgnCategory::QueryHighestCategoryId(db)));
        }
};

/*---------------------------------------------------------------------------------**//**
* gcc errs if defined inside the class: explicit specialization in non-namespace scope 
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<> DgnDbR LocksManagerTest::ExtractDgnDb(DgnDb const& obj) { return const_cast<DgnDbR>(obj); }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SingleBriefcaseLocksTest : LocksManagerTest
{
    DEFINE_T_SUPER(LocksManagerTest);

    // Our DgnDb is masquerading as a briefcase...it has no actual master copy.
    BeBriefcaseId m_bcId;
    DgnDbPtr m_db;

    SingleBriefcaseLocksTest() : m_bcId(1) { }
    ~SingleBriefcaseLocksTest() {if (m_db.IsValid()) m_db->CloseDb();}

    DgnModelPtr CreateModel(Utf8CP name) { return T_Super::CreateModel(name, *m_db); }
};

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, AcquireLocks)
    {
    m_db = SetupDb(L"AcquireLocks.dgndb", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr pModel = db.Models().GetModel(m_modelId);
    ASSERT_TRUE(pModel.IsValid());
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    ASSERT_TRUE(cpEl.IsValid());

    DgnModelCR model = *pModel;
    DgnElementCR el = *cpEl;

    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared); // a shared lock on a model => shared lock on DB

    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);

    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);

    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());

    ExpectAcquire(el, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());
    
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Exclusive);

    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());

    // An exclusive model lock results in exclusive locks on all of its elements
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);

    // An exclusive db lock results in exclusive locks on all models and elements
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    
    // If we obtain an exclusive lock on a model, exclusive locks on its elements should be retained after refresh
    EXPECT_EQ(LockStatus::Success, db.Locks().RelinquishLocks());
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(LockStatus::Success, db.Locks().RefreshLocks());
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* Newly-created models and elements are implicitly exclusively locked by that briefcase.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, LocallyCreatedObjects)
    {
    m_db = SetupDb(L"LocallyCreatedObjects.dgndb", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr model = db.Models().GetModel(m_modelId);
    DgnElementCPtr elem = db.Elements().GetElement(m_elemId);

    // Create a new model
    DgnModelPtr newModel = CreateModel("NewModel");

    m_server.Dump("Created Model");

    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);


    // Our exclusive locks are only known locally, because they refer to elements not known to the server
    // If we refresh our local locks from server, we will need to re-obtain them.
    // Server *will* know about shared locks on locally-created models
    EXPECT_EQ(LockStatus::Success, db.Locks().RefreshLocks());
    ExpectLevel(*newModel, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared);

    // Create a new element in our new model
    DgnElementCPtr newElem = CreateElement(*newModel);

    m_server.Dump("Created Element");

    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Shared);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    EXPECT_EQ(LockStatus::Success, db.Locks().RefreshLocks());
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Shared);
    ExpectLevel(*newElem, LockLevel::None);

    ExpectAcquire(*newElem, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*newModel, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);

    ExpectAcquire(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(LockStatus::Success, db.Locks().RefreshLocks());
    ExpectAcquire(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    m_server.Dump("Finished");
    }

/*---------------------------------------------------------------------------------**//**
* Simulate two briefcases operating against the same master DgnDb.
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DoubleBriefcaseTest : LocksManagerTest
{
    DEFINE_T_SUPER(LocksManagerTest);

    DgnDbPtr m_dbA;
    DgnDbPtr m_dbB;

    DgnModelId m_modelIds[2];
    DgnElementId m_elemIds[4];

    ~DoubleBriefcaseTest() 
        {
        if (m_dbA.IsValid())
            m_dbA->SaveChanges();
        if (m_dbB.IsValid())
            m_dbB->SaveChanges();
        }

    static WCharCP SeedFileName() { return L"ElementsSymbologyByLeveldgn.i.idgndb"; }

    DgnModelId Model3dId() { return m_modelIds[0]; }
    DgnModelId Model2dId() { return m_modelIds[1]; }

    DgnElementId Elem3dId1() { return m_elemIds[0]; } 
    DgnElementId Elem3dId2() { return m_elemIds[1]; } 
    DgnElementId Elem2dId1() { return m_elemIds[2]; } 
    DgnElementId Elem2dId2() { return m_elemIds[3]; } 

    DgnModelPtr GetModel(DgnDbR db, bool twoD) { return db.Models().GetModel(twoD ? Model2dId() : Model3dId()); }
    DgnElementCPtr GetElement(DgnDbR db, DgnElementId id) { return db.Elements().GetElement(id); }

    bool LookupElementIds(DgnElementId* ids, DgnModelR model)
        {
        model.FillModel();
        uint32_t nElemsFound = 0;
        for (auto const& elem : model)
            {
            ids[nElemsFound++] = elem.first;
            if (2 == nElemsFound)
                return true;
            }

        return false;
        }

    void SetupDbs()
        {
        m_dbA = SetupDb(L"DbA.dgndb", BeBriefcaseId(1), SeedFileName());
        m_dbB = SetupDb(L"DbB.dgndb", BeBriefcaseId(2), SeedFileName());

        ASSERT_TRUE(m_dbA.IsValid());
        ASSERT_TRUE(m_dbB.IsValid());

        // Model + element IDs may vary each time we convert the v8 file...need to look them up.
        DgnModelId model2d, model3d;
        for (auto const& entry : m_dbA->Models().MakeIterator())
            {
            auto model = m_dbA->Models().GetModel(entry.GetModelId());
            if (model.IsValid() && !model->IsDictionaryModel())
                {
                if (!model3d.IsValid() && LookupElementIds(m_elemIds, *model))
                    {
                    model3d = entry.GetModelId();
                    }
                else if (!model2d.IsValid() && LookupElementIds(m_elemIds+2, *model))
                    {
                    model2d = entry.GetModelId();
                    break;
                    }
                }
            }

        ASSERT_TRUE(model2d.IsValid() && model3d.IsValid());
        m_modelIds[0] = model3d;
        m_modelIds[1] = model2d;
        }

    void RelinquishAll()
        {
        EXPECT_EQ(LockStatus::Success, m_dbA->Locks().RelinquishLocks());
        EXPECT_EQ(LockStatus::Success, m_dbB->Locks().RelinquishLocks());
        }
};

/*---------------------------------------------------------------------------------**//**
* Acquisition of locks may result in contention with another briefcase.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, Contention)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;
    DgnModelPtr modelA2d = GetModel(dbA, true);
    DgnModelPtr modelB2d = GetModel(dbB, true);
    DgnElementCPtr elemA2d2 = GetElement(dbA, Elem2dId2());
    DgnElementCPtr elemB2d2 = GetElement(dbB, Elem2dId2());

    // Both briefcases can hold shared locks
    ExpectAcquire(*modelA2d, LockLevel::Shared);
    ExpectAcquire(*elemB2d2, LockLevel::Exclusive);
    ExpectLevel(*modelA2d, LockLevel::Shared);
    ExpectLevel(dbA, LockLevel::Shared);
    ExpectLevel(*elemA2d2, LockLevel::None);
    ExpectLevel(*modelB2d, LockLevel::Shared);
    ExpectLevel(*elemB2d2, LockLevel::Exclusive);
    ExpectLevel(dbB, LockLevel::Shared);

    // No briefcase can exclusively lock a model if a shared lock already held on it by another briefcase
    ExpectDenied(*modelA2d, LockLevel::Exclusive);
    ExpectDenied(*elemA2d2, LockLevel::Exclusive);

    // One briefcase can acquire exclusive lock on a model
    DgnModelPtr modelA3d = GetModel(dbA, false);
    DgnModelPtr modelB3d = GetModel(dbB, false);
    ExpectAcquire(*modelA3d, LockLevel::Exclusive);
    ExpectDenied(*modelB3d, LockLevel::Shared);
    ExpectDenied(*modelB3d, LockLevel::Exclusive);

    // One briefcase cannot obtain an exclusive lock on an element in a model exclusively locked by another briefcase
    DgnElementCPtr elemA3d2 = GetElement(dbA, Elem3dId2());
    DgnElementCPtr elemB3d2 = GetElement(dbB, Elem3dId2());
    ExpectDenied(*elemB3d2, LockLevel::Exclusive);
    ExpectLevel(*modelA3d, LockLevel::Exclusive);
    ExpectLevel(*elemA3d2, LockLevel::Exclusive);
    ExpectLevel(*modelB3d, LockLevel::None);
    ExpectLevel(*elemB3d2, LockLevel::None);

    // No briefcase can lock the db unless no shared locks exist
    ExpectDenied(dbA, LockLevel::Exclusive);
    ExpectDenied(dbB, LockLevel::Exclusive);

    // If one briefcase relinquishes its locks they become available to others
    EXPECT_EQ(LockStatus::Success, dbA.Locks().RelinquishLocks());
    ExpectAcquire(dbB, LockLevel::Exclusive);
    ExpectLevel(dbA, LockLevel::None);
    ExpectLevel(*modelA3d, LockLevel::None);
    ExpectLevel(*modelA2d, LockLevel::None);
    ExpectLevel(*elemA3d2, LockLevel::None);
    ExpectLevel(dbB, LockLevel::Exclusive);
    ExpectLevel(*modelB3d, LockLevel::Exclusive);
    ExpectLevel(*elemB3d2, LockLevel::Exclusive);

    // B's exclusive Db lock prevents A from acquiring any locks, shared or otherwise
    ExpectDenied(dbA, LockLevel::Shared);
    ExpectDenied(*modelA3d, LockLevel::Shared);
    ExpectDenied(*elemA3d2, LockLevel::Shared);
    ExpectDenied(*elemA2d2, LockLevel::Shared);
    ExpectDenied(dbA, LockLevel::Exclusive);
    ExpectDenied(*modelA3d, LockLevel::Exclusive);
    ExpectDenied(*elemA3d2, LockLevel::Exclusive);
    ExpectDenied(*elemA2d2, LockLevel::Exclusive);

    // B's exclusive locks imply shared ownership
    ExpectAcquire(dbB, LockLevel::Shared);
    ExpectAcquire(*modelB3d, LockLevel::Shared);
    ExpectAcquire(*elemB3d2, LockLevel::Shared);
    ExpectAcquire(*elemB2d2, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* Operations on elements and models automatically acquire locks.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, AutomaticLocking)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;

    // Creating a new model requires shared ownership of Db, implies exclusive ownership of new model
    DgnModelPtr newModelA = CreateModel("New Model A", dbA);
    ASSERT_TRUE(newModelA.IsValid());
    ExpectLevel(dbA, LockLevel::Shared);
    ExpectLevel(*newModelA, LockLevel::Exclusive);

    // Creating a new element requires shared ownership of model, implies exclusive ownership of new element
    DgnModelPtr modelB3d = GetModel(dbB, false);
    DgnElementCPtr newElemB3d = CreateElement(*modelB3d);
    ASSERT_TRUE(newElemB3d.IsValid());
    ExpectLevel(dbB, LockLevel::Shared);
    ExpectLevel(*modelB3d, LockLevel::Shared);
    ExpectLevel(*newElemB3d, LockLevel::Exclusive);

    // Deleting a model requires exclusive ownership
    DgnModelPtr modelA2d = GetModel(dbA, true);
    ExpectLevel(*modelA2d, LockLevel::None);
    EXPECT_EQ(DgnDbStatus::Success, modelA2d->Delete());
    ExpectLevel(*modelA2d, LockLevel::Exclusive);
    DgnModelPtr modelA3d = GetModel(dbA, false);
    ExpectLevel(*modelA3d, LockLevel::None);
    EXPECT_EQ(DgnDbStatus::LockNotHeld, modelA3d->Delete()); // because B has a shared lock on it
    ExpectLevel(*modelA3d, LockLevel::None);

    // Adding an element to a model requires shared ownership
    DgnModelPtr modelB2d = GetModel(dbB, true);
    ExpectLevel(*modelB2d, LockLevel::None);
    DgnElementCPtr newElemB2d = CreateElement(*modelB2d);
    EXPECT_TRUE(newElemB2d.IsNull());   // no return status to check...
    ExpectLevel(*modelB2d, LockLevel::None);

    // At this point, ownership is:
    //  Object      A   B
    //  Model2d     Ex  No      // deleted
    //    Elem1     Ex  No      // deleted
    //    Elem2     Ex  No      // deleted
    //  Model3d     Sh  Sh
    //    Elem1     No  No
    //    Elem2     No  No

    // Modifying an element requires shared ownership of model and exclusive ownership of element
    DgnElementCPtr cpElemB2d = GetElement(dbB, Elem2dId1());
    EXPECT_EQ(DgnDbStatus::LockNotHeld, cpElemB2d->Delete());
    DgnElementPtr pElemB2d = cpElemB2d->CopyForEdit();
    DgnDbStatus status;
    EXPECT_TRUE(pElemB2d->Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::LockNotHeld, status);
    ExpectLevel(*cpElemB2d, LockLevel::None);
    ExpectLevel(*modelB2d, LockLevel::None);

    DgnElementCPtr cpElemB3d = GetElement(dbB, Elem3dId1());
    ExpectLevel(*cpElemB3d, LockLevel::None);
    EXPECT_EQ(DgnDbStatus::Success, cpElemB3d->Delete());
    ExpectLevel(*cpElemB3d, LockLevel::Exclusive);

    DgnElementPtr pElemB3d2 = GetElement(dbB, Elem3dId2())->CopyForEdit();
    ExpectLevel(*pElemB3d2, LockLevel::None);
    EXPECT_TRUE(pElemB3d2->Update(&status).IsValid());
    ExpectLevel(*pElemB3d2, LockLevel::Exclusive);
    }

/*---------------------------------------------------------------------------------**//**
* Locks are not required for dynamic operations, as all changes will be reverted when
* the operation completes.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, Dynamics)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;

    // Briefcase A exclusively locks model
    auto modelA = GetModel(dbA, true);
    auto elemA = GetElement(dbA, Elem2dId1());
    ExpectAcquire(*modelA, LockLevel::Exclusive);
    ExpectAcquire(*elemA, LockLevel::Exclusive);

    // Briefcase B cannot obtain locks during normal transactions
    auto modelB = GetModel(dbB, true);
    auto elemB = GetElement(dbB, Elem2dId1());
    ExpectDenied(*modelB, LockLevel::Shared);
    ExpectDenied(*elemB, LockLevel::Exclusive);

    // Briefcase B starts a dynamic operation
    auto& txns = dbB.Txns();
    txns.BeginDynamicOperation();
        // within dynamics, Briefcase A can make temporary changes without acquiring locks
        // delete an element within the model
        EXPECT_EQ(DgnDbStatus::Success, elemB->Delete());
        ExpectLevel(*elemB, LockLevel::None);
        ExpectLevel(*modelB, LockLevel::None);
        EXPECT_EQ(DgnDbStatus::Success, modelB->Delete());
        ExpectLevel(*modelB, LockLevel::None);
    txns.EndDynamicOperation();

    ExpectLevel(*modelB, LockLevel::None);
    ExpectLevel(*modelA, LockLevel::Exclusive);
    ExpectLevel(*elemB, LockLevel::None);
    ExpectLevel(*elemA, LockLevel::Exclusive);
    }

/*---------------------------------------------------------------------------------**//**
* Test that:
*   - Reponse from AcquireLocks() includes set of denied locks (if option specified in request)
*   - QueryOwnership response matches that set of denied locks
*   - ReformulateRequest produces a set of locks which are not in denied set
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, ReformulateRequest)
    {
    SetupDbs();
    DgnDbR dbA = *m_dbA;
    DgnDbR dbB = *m_dbB;
    DgnModelPtr modelA2d = GetModel(dbA, true);
    DgnModelPtr modelB2d = GetModel(dbB, true);
    DgnElementCPtr elemA2d2 = GetElement(dbA, Elem2dId2());
    DgnElementCPtr elemB2d2 = GetElement(dbB, Elem2dId2());

    // Model: Locked exclusively
        {
        ExpectAcquire(*modelA2d, LockLevel::Exclusive);
        auto response = AcquireResponse(*modelB2d, LockLevel::Shared);
        EXPECT_EQ(LockStatus::AlreadyHeld, response.GetStatus());
        EXPECT_EQ(1, response.GetDeniedLocks().size());

        ExpectInDeniedSet(*modelA2d, LockLevel::Exclusive, response);
        // Reformulate request => remove request for model
        LockRequest request;
        request.Insert(*modelB2, LockLevel::Shared);
        dbB.Locks().ReformulateRequest(request, response.GetDeniedLocks());
        EXPECT_TRUE(request.IsEmpty());
        }

    RelinquishAll();

    // Model: Locked shared by both
        {
        ExpectAcquire(*modelA2d, LockLevel::Shared);
        ExpectAcquire(*modelB2d, LockLevel::Shared);
        // Try to lock exclusively
        auto response = AcquireResponse(*modelB2d, LockLevel::Exclusive);
        EXPECT_EQ(LockStatus::AlreadyHeld, response.GetStatus());
        EXPECT_EQ(1, response.GetDeniedLocks().size());
        ExpectInDeniedSet(*modelA2d, LockLevel::Shared, response);

        // Expect two shared owners
        DgnLockOwnership ownership;
        auto modelLockId = MakeLockableId(*modelA2d);
        EXPECT_EQ(LockStatus::Success, T_HOST.GetLocksAdmin()._GetLocksServer()->QueryOwnership(ownership, modelLockId));
        EXPECT_EQ(LockLevel::Shared, ownership.GetLockLevel());
        auto const& sharedOwners = ownership.GetSharedOwners();
        EXPECT_EQ(2, sharedOwners.size());
        EXPECT_TRUE(sharedOwners.find(dbA.GetBriefcaseId()) != sharedOwners.end());
        EXPECT_TRUE(sharedOwners.find(dbB.GetBriefcaseId()) != sharedOwners.end());

        // Reformulate request => demote exclusive to shared
        EXPECT_EQ(1, request.Size());
        EXPECT_EQ(LockLevel::Shared, request.GetLockLevel(modelLockId));
        }

    RelinquishAll();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractLocksTest : SingleBriefcaseLocksTest
{
    DgnDbStatus ExtractLocks(LockRequestR req)
        {
        if (BE_SQLITE_OK != m_db->SaveChanges())
            return DgnDbStatus::WriteError;

        DgnRevisionPtr rev = m_db->Revisions().StartCreateRevision();
        if (rev.IsNull())
            return DgnDbStatus::BadRequest;

        ChangeStreamFileReader stream(rev->GetChangeStreamFile());
        DgnDbStatus status = req.FromChangeSet(stream, *m_db);

        m_db->Revisions().AbandonCreateRevision();

        return status;
        }
};

/*---------------------------------------------------------------------------------**//**
* Test functions which query the set of locks which are required by the changes actually
* made in the briefcase. (Excluding locks obtained but not actually used).
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtractLocksTest, UsedLocks)
    {
    struct AssertScope
    {
        AssertScope() { BeTest::SetFailOnAssert(false); }
        ~AssertScope() { BeTest::SetFailOnAssert(true); }
    };

    struct UndoScope
    {
        DgnDbR m_db;
        TxnManager::TxnId m_id;
        UndoScope(DgnDbR db) : m_db(db), m_id(db.Txns().GetCurrentTxnId()) { }
        ~UndoScope() { m_db.Txns().ReverseTo(m_id); }
    };

    AssertScope V_V_V_Asserts;
    m_db = SetupDb(L"UsedLocks.dgndb", m_bcId);

    DgnDbR db = *m_db;
    DgnModelR model = *db.Models().GetModel(m_modelId);
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);

    LockRequest req;
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Modify an elem (it's a DgnCategory...)
        {
        UndoScope V_V_V_Undo(db);
        auto pEl = cpEl->CopyForEdit();
        DgnElement::Code newCode = DgnCategory::CreateCategoryCode("RenamedCategory");
        EXPECT_EQ(DgnDbStatus::Success, pEl->SetCode(newCode));
        cpEl = pEl->Update();
        ASSERT_TRUE(cpEl.IsValid());

        EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
        EXPECT_FALSE(req.IsEmpty());
        EXPECT_EQ(3, req.Size());
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(model)));
        EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*pEl)));
        EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));
        }

    // Change reversed on exit above scope
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Create a new model
    DgnModelPtr newModel = CreateModel("NewModel");
    DgnElementCPtr newElem = CreateElement(*newModel);

    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_EQ(3, req.Size());
    EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*newModel)));
    EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*newElem)));
    EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));

    // Delete the new element
    EXPECT_EQ(DgnDbStatus::Success, newElem->Delete());
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_EQ(2, req.Size());
    EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*newModel)));
    EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));

    // Delete the new model
    EXPECT_EQ(DgnDbStatus::Success, newModel->Delete());
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());
    }


/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/BriefcaseManager_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnMaterial.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* There used to be two separate interfaces: one for locks, another for codes.
* We combined them.
* Minimal changes made to this mock implementation - mostly just performs lock+code
* operations independently.
* That mostly works because we generally also test locks+codes independently.
* A real implementation would need to take care that if an operation involving both
* locks+codes fails, the entire operation fails (i.e., does not succeed in acquiring locks
* but fail in acquiring codes).
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryManager : IRepositoryManager
{
    typedef IBriefcaseManager::ResponseOptions Options;
    typedef IBriefcaseManager::RequestPurpose Purpose;
private:
    Db m_db;

    // impl
    Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override;
    RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    RepositoryStatus _Relinquish(Resources which, DgnDbR db) override;
    RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override;
    RepositoryStatus _QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override;

    DbResult CreateLocksTable();
    DbResult CreateCodesTable();

    // locks
    bool AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId requestor);
    void GetDeniedLocks(DgnLockInfoSet& locks, LockRequestCR reqs, BeBriefcaseId bcId);
    void GetUnavailableLocks(DgnLockSet& locks, BeBriefcaseId bcId);
    int32_t QueryLockCount(BeBriefcaseId bc);
    void Relinquish(DgnLockSet const&, DgnDbR);
    void Reduce(DgnLockSet const&, DgnDbR);
    void _AcquireLocks(Response&, LockRequestCR, DgnDbR, Options opts);
    RepositoryStatus _RelinquishLocks(DgnDbR);
    RepositoryStatus _DemoteLocks(DgnLockSet const&, DgnDbR);
    RepositoryStatus _QueryLocks(DgnLockSet&, DgnDbR);
    RepositoryStatus _QueryLockState(DgnLockInfoR, LockableId);
    RepositoryStatus _QueryLockStates(DgnLockInfoSet&, LockableIdSet const&);

    // codes
    enum class CodeState : uint8_t { Available, Reserved, Discarded, Used };

    RepositoryStatus ValidateRelease(DgnCodeInfoSet& toMarkDiscarded, Statement& stmt, BeBriefcaseId bcId);
    RepositoryStatus ValidateRelease(DgnCodeInfoSet&, DgnCodeSet const&, DgnDbR);
    RepositoryStatus ValidateRelinquish(DgnCodeInfoSet&, DgnDbR);
    void MarkDiscarded(DgnCodeInfoSet const& discarded);
    void MarkRevision(DgnCodeSet const& codes, bool discarded, Utf8StringCR revId);
    void _ReserveCodes(Response& response, DgnCodeSet const& req, DgnDbR db, Options opts, bool queryOnly);
    RepositoryStatus _ReleaseCodes(DgnCodeSet const&, DgnDbR);
    RepositoryStatus _RelinquishCodes(DgnDbR);
    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&);
    void GetUnavailableCodes(DgnCodeSet& codes, BeBriefcaseId bcId);

public:
    RepositoryManager();

    // Simulates what the real server does with codes when a revision is pushed.
    void OnFinishRevision(DgnRevision const& rev, DgnDbCR dgndb)
        {
        DgnCodeSet assignedCodes, discardedCodes;
        rev.ExtractCodes(assignedCodes, discardedCodes, dgndb);

        MarkRevision(assignedCodes, false, rev.GetId());
        MarkRevision(discardedCodes, true, rev.GetId());
        }

    void MarkUsed(DgnCode const& code, Utf8StringCR revision)
        {
        DgnCodeSet codes;
        codes.insert(code);
        MarkRevision(codes, false, revision);
        }

    RepositoryStatus _QueryCodes(DgnCodeSet&, DgnDbR);
};

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
    ExpectLockSetsEqual(a.GetLockSet(), b.GetLockSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LockVirtualSet : VirtualSet
{
    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        if (3 != nVals)
            {
            BeAssert(false);
            return false;
            }

        LockableId id(static_cast<LockableType>(vals[0].GetValueInt()), BeInt64Id(vals[1].GetValueUInt64()));
        DgnLock lock(id, 0 == vals[2].GetValueInt() ? LockLevel::Shared : LockLevel::Exclusive);
        return _IsLockInSet(lock);
        }

    virtual bool _IsLockInSet(DgnLockCR lock) const = 0;
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LockRequestVirtualSet : LockVirtualSet
{
    LockRequestCR m_request;

    LockRequestVirtualSet(LockRequestCR request) : m_request(request) { }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct InRequestVirtualSet : LockRequestVirtualSet
{
    InRequestVirtualSet(LockRequestCR request) : LockRequestVirtualSet(request) { }

    bool _IsLockInSet(DgnLockCR lock) const override
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

    bool _IsLockInSet(DgnLockCR lock) const override
        {
        // input: an entry for a lock ID in our request, from the server's db
        // output: true if the input lock conflicts with our request
        DgnLockCP found = m_request.Find(lock.GetLockableId());
        return nullptr != found && (found->IsExclusive() || lock.IsExclusive());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LockSetVirtualSet : LockVirtualSet
{
    DgnLockSet const& m_set;

    LockSetVirtualSet(DgnLockSet const& set) : m_set(set) { }

    bool _IsLockInSet(DgnLockCR lock) const override
        {
        return m_set.end() != m_set.find(lock);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LockLevelVirtualSet : LockSetVirtualSet
{
    LockLevel m_level;

    LockLevelVirtualSet(DgnLockSet const& set, LockLevel level) : LockSetVirtualSet(set), m_level(level) { }

    bool _IsLockInSet(DgnLockCR lock) const override
        {
        auto found = m_set.find(lock);
        return m_set.end() != found && found->GetLevel() == m_level;
        }
};

#define TABLE_Locks "Locks"
#define LOCK_Id "Id"
#define LOCK_Type "Type"
#define LOCK_BcId "Owner"
#define LOCK_Exclusive "Exclusive"

#define TABLE_Codes "Codes"
#define CODE_CodeSpec "CodeSpec"
#define CODE_Scope "Scope"
#define CODE_Value "Name"
#define CODE_State "State"
#define CODE_Revision "Revision"
#define CODE_Briefcase "Briefcase"

#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryManager::RepositoryManager()
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

#if 1
    DbResult result = m_db.CreateNewDb(BEDB_MemoryDb); // use MemoryDb for performance
#else
    BeFileName testFixtureName(TEST_FIXTURE_NAME, BentleyCharEncoding::Utf8);
    BeFileName testName(TEST_NAME, BentleyCharEncoding::Utf8);
    BeFileName serverDb;
    BeTest::GetHost().GetOutputRoot(serverDb);
    serverDb.AppendToPath(testFixtureName);
    BeFileName::CreateNewDirectory(serverDb);
    serverDb.AppendToPath(testName);
    serverDb.AppendExtension(L"bim.server");
    DbResult result = m_db.CreateNewDb(serverDb); // use file db for debugging
#endif

    if (BE_SQLITE_OK == result)
        {
        result = CreateLocksTable();
        if (BE_SQLITE_OK == result)
            result = CreateCodesTable();
        }

    BeAssert(BE_SQLITE_OK == result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RepositoryManager::CreateLocksTable()
    {
    return m_db.CreateTable(TABLE_Locks,
            LOCK_Id " INTEGER,"
            LOCK_Type " INTEGER,"
            LOCK_BcId " INTEGER,"
            LOCK_Exclusive " INTEGER,"
            "PRIMARY KEY(" LOCK_Id "," LOCK_Type "," LOCK_BcId ")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RepositoryManager::CreateCodesTable()
    {
    return m_db.CreateTable(TABLE_Codes,
            CODE_CodeSpec " INTEGER NOT NULL,"
            CODE_Scope " TEXT NOT NULL,"
            CODE_Value " TEXT NOT NULL,"
            CODE_State " INTEGER,"
            CODE_Revision " TEXT,"
            CODE_Briefcase " INTEGER,"
            "PRIMARY KEY(" CODE_CodeSpec "," CODE_Scope "," CODE_Value ")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void bindBcId(Statement& stmt, int index, BeBriefcaseId id)
    {
    stmt.BindInt(index, static_cast<int32_t>(id.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryManager::Response RepositoryManager::_ProcessRequest(Request const& req, DgnDbR db, bool queryOnly)
    {
    Response response(queryOnly ? Purpose::Query : Purpose::Acquire, req.Options());
    _ReserveCodes(response, req.Codes(), db, req.Options(), queryOnly);
    auto prevStatus = response.Result();
    if (queryOnly)
        {
        if (!AreLocksAvailable(req.Locks(), db.GetBriefcaseId()))
            {
            response.SetResult(RepositoryStatus::LockAlreadyHeld);
            if (Options::None != (Options::LockState & req.Options()))
                GetDeniedLocks(response.LockStates(), req.Locks(), db.GetBriefcaseId());
            }
        }
    else
        {
        _AcquireLocks(response, req.Locks(), db, req.Options());
        }

    if (RepositoryStatus::Success == response.Result())
        response.SetResult(prevStatus);

    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
    {
    auto stat = _DemoteLocks(locks, db);
    if (RepositoryStatus::Success == stat)
        stat = _ReleaseCodes(codes, db);

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_Relinquish(Resources which, DgnDbR db)
    {
    auto stat = RepositoryStatus::Success;
    if (Resources::Locks == (Resources::Locks & which))
        stat = _RelinquishLocks(db);

    if (RepositoryStatus::Success == stat && Resources::Codes == (Resources::Codes & which))
        stat = _RelinquishCodes(db);

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
    {
    auto stat = _QueryLockStates(lockStates, locks);
    if (RepositoryStatus::Success == stat)
        stat = _QueryCodeStates(codeStates, codes);

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
    {
    auto stat = _QueryLocks(locks, db);
    if (RepositoryStatus::Success == stat)
        stat = _QueryCodes(codes, db);

    GetUnavailableLocks(unavailableLocks, db.GetBriefcaseId());
    GetUnavailableCodes(unavailableCodes, db.GetBriefcaseId());

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::GetUnavailableLocks(DgnLockSet& locks, BeBriefcaseId bcId)
    {
    // ###TODO: This should also include locks which were released in a revision not yet pulled by this briefcase...
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " LOCK_Type "," LOCK_Id "," LOCK_Exclusive " FROM " TABLE_Locks " WHERE " LOCK_BcId " != ?");
    bindBcId(stmt, 1, bcId);

    while (BE_SQLITE_ROW == stmt.Step())
        {
        LockableId id(static_cast<LockableType>(stmt.GetValueInt(0)), stmt.GetValueId<BeInt64Id>(1));
        auto level = (0 != stmt.GetValueInt(2)) ? LockLevel::Exclusive : LockLevel::Shared;
        locks.insert(DgnLock(id, level));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::GetUnavailableCodes(DgnCodeSet& codes, BeBriefcaseId bcId)
    {
    // ###TODO: This should also include codes which became discarded or used in a revision not yet pulled by this briefcase...
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " CODE_CodeSpec "," CODE_Scope "," CODE_Value " FROM " TABLE_Codes
                       " WHERE " CODE_State " = 1 AND " CODE_Briefcase " != ?");
    bindBcId(stmt, 1, bcId);

    while (BE_SQLITE_ROW == stmt.Step())
        codes.insert(DgnCode::From(stmt.GetValueId<CodeSpecId>(0), stmt.GetValueText(1), stmt.GetValueText(2)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryManager::AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId bcId)
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT count(*) FROM " TABLE_Locks
                   " WHERE " LOCK_BcId " != ?"
                   " AND InVirtualSet(@vset," LOCK_Type "," LOCK_Id "," LOCK_Exclusive ")");

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
void RepositoryManager::GetDeniedLocks(DgnLockInfoSet& states, LockRequestCR reqs, BeBriefcaseId bcId)
    {
    // NB: Interface has evolved repeatedly...should probably rewrite this function...
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " LOCK_Type "," LOCK_Id "," LOCK_Exclusive
                       " FROM " TABLE_Locks " WHERE " LOCK_BcId " != ?" " AND InVirtualSet(@vset," LOCK_Type "," LOCK_Id "," LOCK_Exclusive ")");

    RequestConflictsVirtualSet vset(reqs);
    bindBcId(stmt, 1, bcId);
    stmt.BindVirtualSet(2, vset);

    LockableIdSet deniedLocks;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        LockableId id(static_cast<LockableType>(stmt.GetValueInt(0)), stmt.GetValueId<BeInt64Id>(1));
        auto level = (0 != stmt.GetValueInt(2)) ? LockLevel::Exclusive : LockLevel::Shared;
        auto inserted = deniedLocks.insert(id);
        if (LockLevel::Exclusive == level)
            EXPECT_TRUE(inserted.second);   // If the server has granted an exclusive lock, it should not have any shared locks for same object.
        }

    if (!deniedLocks.empty())
        QueryLockStates(states, deniedLocks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t RepositoryManager::QueryLockCount(BeBriefcaseId bc)
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT count(*) FROM " TABLE_Locks " WHERE " LOCK_BcId "=?");
    bindBcId(stmt, 1, bc);
    auto rc = stmt.Step();
    BeAssert(BE_SQLITE_ROW == rc);
    return BE_SQLITE_ROW == rc ? stmt.GetValueInt(0) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::_AcquireLocks(Response& response, LockRequestCR inputReqs, DgnDbR db, Options opts)
    {
    // Simulating serialization and deserialization of server request...
    Json::Value reqJson;
    inputReqs.ToJson(reqJson);
    LockRequest reqs;
    EXPECT_TRUE(reqs.FromJson(reqJson));
    ExpectRequestsEqual(inputReqs, reqs);

    if (!AreLocksAvailable(reqs, db.GetBriefcaseId()))
        {
        response.SetResult(RepositoryStatus::LockAlreadyHeld);
        if (Options::None != (Options::LockState & opts))
            GetDeniedLocks(response.LockStates(), reqs, db.GetBriefcaseId());

        return;
        }

    for (auto const& req : reqs)
        {
        Statement select;
        select.Prepare(m_db, "SELECT " LOCK_Exclusive ",rowid FROM " TABLE_Locks
                       " WHERE " LOCK_Type "=? AND " LOCK_Id "=? AND " LOCK_BcId "=?");
        select.BindInt(1, static_cast<int32_t>(req.GetType()));
        select.BindId(2, req.GetId());
        bindBcId(select, 3, db.GetBriefcaseId());

        switch (select.Step())
            {
            case BE_SQLITE_DONE:
                {
                Statement stmt;
                stmt.Prepare(m_db, "INSERT INTO " TABLE_Locks " (" LOCK_Type "," LOCK_Id "," LOCK_BcId "," LOCK_Exclusive ") VALUES(?,?,?,?)");

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
                    stmt.Prepare(m_db, "UPDATE " TABLE_Locks " SET " LOCK_Exclusive "=1 WHERE rowid=?");
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

    response.SetResult(RepositoryStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_RelinquishLocks(DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " TABLE_Locks " WHERE " LOCK_BcId "=?");
    bindBcId(stmt, 1, db.GetBriefcaseId());
    if (BE_SQLITE_DONE != stmt.Step())
        BeAssert(false);

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_DemoteLocks(DgnLockSet const& locks, DgnDbR db)
    {
    Relinquish(locks, db);
    Reduce(locks, db);

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::Relinquish(DgnLockSet const& locks, DgnDbR db)
    {
    LockLevelVirtualSet vset(locks, LockLevel::None);
    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " TABLE_Locks " WHERE " LOCK_BcId "=?"
                 " AND InVirtualSet(@vset," LOCK_Type "," LOCK_Id "," LOCK_Exclusive ")");

    bindBcId(stmt, 1, db.GetBriefcaseId());
    stmt.BindVirtualSet(2, vset);
    if (BE_SQLITE_DONE != stmt.Step())
        BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::Reduce(DgnLockSet const& locks, DgnDbR db)
    {
    LockLevelVirtualSet vset(locks, LockLevel::Shared);
    Statement stmt;
    stmt.Prepare(m_db, "UPDATE " TABLE_Locks " SET " LOCK_Exclusive "=0 WHERE " LOCK_BcId "=?"
                 " AND InVirtualSet(@vset," LOCK_Type "," LOCK_Id "," LOCK_Exclusive ")");

    bindBcId(stmt, 1, db.GetBriefcaseId());
    stmt.BindVirtualSet(2, vset);
    if (BE_SQLITE_DONE != stmt.Step())
        BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_QueryLocks(DgnLockSet& locks, DgnDbR db)
    {
    locks.clear();
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " LOCK_Type "," LOCK_Id "," LOCK_Exclusive " FROM " TABLE_Locks " WHERE " LOCK_BcId "=?");
    bindBcId(stmt, 1, db.GetBriefcaseId());
    while (BE_SQLITE_ROW == stmt.Step())
        {
        LockableId id(static_cast<LockableType>(stmt.GetValueInt(0)), stmt.GetValueId<BeInt64Id>(1));
        locks.insert(DgnLock(id, 0 != stmt.GetValueInt(2) ? LockLevel::Exclusive : LockLevel::Shared));
        }

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_QueryLockState(DgnLockInfoR state, LockableId inputLockId)
    {
    // NB: Currently not tracking revision IDs in mock server for locks...
    state.Reset();
    DgnLockOwnershipR ownership = state.GetOwnership();

    // Simulating serialization and deserialization of server request...
    Json::Value lockIdJson;
    inputLockId.ToJson(lockIdJson);
    LockableId lockId;
    EXPECT_TRUE(lockId.FromJson(lockIdJson));
    EXPECT_EQ(lockId, inputLockId);

    Statement stmt;
    stmt.Prepare(m_db, "SELECT " LOCK_Exclusive "," LOCK_BcId " FROM " TABLE_Locks " WHERE " LOCK_Type "=? AND " LOCK_Id "=?");
    stmt.BindInt(1, static_cast<int32_t>(lockId.GetType()));
    stmt.BindId(2, lockId.GetId());

    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto owner = BeBriefcaseId(static_cast<uint32_t>(stmt.GetValueInt(1)));
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

    if (state.IsOwned())
        state.SetTracked();

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_QueryLockStates(DgnLockInfoSet& states, LockableIdSet const& ids)
    {
    // NB: Previously could not batch requests like this...forward to old one-at-a-time impl
    for (auto const& id : ids)
        {
        DgnLockInfo& info = *states.insert(DgnLockInfo(id)).first;
        auto status = _QueryLockState(info, id);
        if (RepositoryStatus::Success != status)
            return status;
        }

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::ValidateRelease(DgnCodeInfoSet& discarded, Statement& stmt, BeBriefcaseId bcId)
    {
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (static_cast<int>(CodeState::Reserved) != stmt.GetValueInt(4) || bcId.GetValue() != stmt.GetValueInt(3))
            return RepositoryStatus::CodeNotReserved;

        if (!stmt.IsColumnNull(5))
            {
            DgnCodeInfo info(DgnCode::From(stmt.GetValueId<CodeSpecId>(0), stmt.GetValueText(1), stmt.GetValueText(2)));
            info.SetDiscarded(stmt.GetValueText(5));
            discarded.insert(info);
            }
        }

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::MarkDiscarded(DgnCodeInfoSet const& discarded)
    {
    Statement stmt;
    stmt.Prepare(m_db, "INSERT INTO " TABLE_Codes "(" CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_State "," CODE_Revision ") VALUES (?,?,?,2,?)");

    for (DgnCodeInfo const& info : discarded)
        {
        DgnCodeCR code = info.GetCode();
        stmt.BindId(1, code.GetCodeSpecId());
        stmt.BindText(2, code.GetScopeString(), Statement::MakeCopy::No);
        stmt.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        stmt.BindText(4, info.GetRevisionId(), Statement::MakeCopy::No);

        stmt.Step();
        stmt.Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct VirtualCodeSet : VirtualSet
{
    DgnCodeSet const& m_codes;

    VirtualCodeSet(DgnCodeSet const& codes) : m_codes(codes) { }

    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        return m_codes.end() != std::find_if(m_codes.begin(), m_codes.end(), [&](DgnCode const& arg)
            {
            return arg.GetCodeSpecId().GetValueUnchecked() == vals[0].GetValueUInt64()
                && arg.GetScopeString().Equals(vals[1].GetValueText())
                && arg.GetValue().Equals(vals[2].GetValueText());
            });
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::_ReserveCodes(Response& response, DgnCodeSet const& req, DgnDbR db, Options opts, bool queryOnly)
    {
    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_State "," CODE_Revision "," CODE_Briefcase
                    "   FROM " TABLE_Codes " WHERE InVirtualSet(@vset, " CODE_CodeSpec "," CODE_Scope "," CODE_Value ")");

    DgnCodeInfoSet discarded;

    RepositoryStatus status = RepositoryStatus::Success;
    stmt.BindVirtualSet(1, vset);
    bool wantInfos = Options::CodeState == (opts & Options::CodeState);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCode code = DgnCode::From(stmt.GetValueId<CodeSpecId>(0), stmt.GetValueText(1), stmt.GetValueText(2));
        switch (static_cast<CodeState>(stmt.GetValueInt(3)))
            {
            case CodeState::Reserved:
                {
                BeBriefcaseId owner(static_cast<uint32_t>(stmt.GetValueInt(5)));
                if (owner.GetValue() != db.GetBriefcaseId().GetValue())
                    {
                    status = RepositoryStatus::CodeUnavailable;
                    if (wantInfos)
                        {
                        DgnCodeInfo info(code);
                        info.SetReserved(owner);
                        response.CodeStates().insert(info);
                        }
                    }
                break;
                }
            case CodeState::Used:
                status = RepositoryStatus::CodeUnavailable;
                if (wantInfos)
                    {
                    DgnCodeInfo info(code);
                    info.SetUsed(stmt.GetValueText(4));
                    response.CodeStates().insert(info);
                    }
                break;
            case CodeState::Discarded:
                // ###TODO: Check if briefcase has pulled the required revision...
                {
                DgnCodeInfo info(code);
                info.SetDiscarded(stmt.GetValueText(4));
                discarded.insert(info);
                }
                break;
            default:
                BeAssert(false);
                break;
            }
        }

    response.SetResult(status);
    if (RepositoryStatus::Success != status || queryOnly)
        return;

    auto bcId = static_cast<int>(db.GetBriefcaseId().GetValue());
    Statement insert;
    insert.Prepare(m_db, "INSERT OR REPLACE INTO " TABLE_Codes "(" CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_State "," CODE_Briefcase "," CODE_Revision ") VALUES (?,?,?,1,?,?)");
    for (auto const& code : req)
        {
        insert.BindId(1, code.GetCodeSpecId());
        insert.BindText(2, code.GetScopeString(), Statement::MakeCopy::No);
        insert.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        insert.BindInt(4, bcId);
        auto revIter = discarded.find(DgnCodeInfo(code));
        if (discarded.end() != revIter)
            insert.BindText(5, revIter->GetRevisionId(), Statement::MakeCopy::No);

        insert.Step();
        insert.Reset();
        }
    }

#define SELECT_ValidateRelease "SELECT " CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_Briefcase "," CODE_State "," CODE_Revision " FROM " TABLE_Codes

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::ValidateRelease(DgnCodeInfoSet& discarded, DgnCodeSet const& req, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE InVirtualSet(@vset, " CODE_CodeSpec "," CODE_Scope "," CODE_Value ")");
    VirtualCodeSet vset(req);
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_ReleaseCodes(DgnCodeSet const& req, DgnDbR db)
    {
    DgnCodeInfoSet discarded;
    RepositoryStatus status = ValidateRelease(discarded, req, db);
    if (RepositoryStatus::Success != status)
        return status;

    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " TABLE_Codes " WHERE " CODE_Briefcase "=? AND InVirtualSet(@vset, " CODE_CodeSpec "," CODE_Scope "," CODE_Value ")");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.BindVirtualSet(2, vset);
    stmt.Step();

    MarkDiscarded(discarded);

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::ValidateRelinquish(DgnCodeInfoSet& discarded, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE " CODE_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_RelinquishCodes(DgnDbR db)
    {
    DgnCodeInfoSet discarded;
    RepositoryStatus status = ValidateRelinquish(discarded, db);
    if (RepositoryStatus::Success != status)
        return status;

    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " TABLE_Codes " WHERE " CODE_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.Step();

    MarkDiscarded(discarded);

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_QueryCodeStates(DgnCodeInfoSet& infos, DgnCodeSet const& codes)
    {
    infos.clear();

    VirtualCodeSet vset(codes);
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_State "," CODE_Revision "," CODE_Briefcase
                    "   FROM " TABLE_Codes " WHERE InVirtualSet(@vset, " CODE_CodeSpec "," CODE_Scope "," CODE_Value ")");
    stmt.BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCodeInfo info(DgnCode::From(stmt.GetValueId<CodeSpecId>(0), stmt.GetValueText(1), stmt.GetValueText(2)));
        switch (static_cast<CodeState>(stmt.GetValueInt(3)))
            {
            case CodeState::Reserved:
                info.SetReserved(BeBriefcaseId(static_cast<uint32_t>(stmt.GetValueInt(5))));
                break;
            case CodeState::Used:
                info.SetUsed(stmt.GetValueText(4));
                break;
            case CodeState::Discarded:
                info.SetDiscarded(stmt.GetValueText(4));
                break;
            case CodeState::Available:
            default:
                BeAssert(false && "This value should never be in the server db!");
                return RepositoryStatus::SyncError;
            }

        infos.insert(info);
        }

    for (DgnCodeCR code : codes)
        {
        // Server doesn't keep track of "available" codes...
        DgnCodeInfo info(code);
        if (infos.end() == infos.find(info))
            infos.insert(info);
        }

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::_QueryCodes(DgnCodeSet& codes, DgnDbR db)
    {
    codes.clear();
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " CODE_CodeSpec "," CODE_Scope "," CODE_Value "   FROM " TABLE_Codes " WHERE " CODE_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    
    while (BE_SQLITE_ROW == stmt.Step())
        codes.insert(DgnCode::From(stmt.GetValueId<CodeSpecId>(0), stmt.GetValueText(1), stmt.GetValueText(2)));

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::MarkRevision(DgnCodeSet const& codes, bool discarded, Utf8StringCR revId)
    {
    if (codes.empty())
        return;

    Statement stmt;
    stmt.Prepare(m_db, "INSERT OR REPLACE INTO " TABLE_Codes "(" CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_State "," CODE_Revision ") VALUES (?,?,?,?,?)");
    for (DgnCodeCR code : codes)
        {
        stmt.BindId(1, code.GetCodeSpecId());
        stmt.BindText(2, code.GetScopeString(), Statement::MakeCopy::No);
        stmt.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        stmt.BindInt(4, static_cast<int>(discarded ? CodeState::Discarded : CodeState::Used));
        stmt.BindText(5, revId, Statement::MakeCopy::No);

        stmt.Step();
        stmt.Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepositoryManagerTest : public DgnDbTestFixture, DgnPlatformLib::Host::RepositoryAdmin
{
public:
    typedef IRepositoryManager::Request Request;
    typedef IRepositoryManager::Response Response;
    typedef IBriefcaseManager::ResponseOptions ResponseOptions;

    mutable RepositoryManager   m_server;

    RepositoryManagerTest()
        {
        RegisterServer();
        }

    ~RepositoryManagerTest()
        {
        UnregisterServer();
        }

    void RegisterServer() { m_host.SetRepositoryAdmin(this); }
    void UnregisterServer() { m_host.SetRepositoryAdmin(nullptr); }   // GetRepositoryManager() => nullptr; Attempts to contact server => RepositoryStatus::ServerUnavailable

    IRepositoryManagerP _GetRepositoryManager(DgnDbR) const override { return &m_server; }

    virtual void _OnSetupDb(DgnDbR db) { }

    DgnDbPtr SetupDb(WCharCP testFile, BeBriefcaseId bcId)
        {
        //** Force to copy the file in Sub-Directory of TestCase
        BeFileName testfixtureName(TEST_FIXTURE_NAME,BentleyCharEncoding::Utf8);
        BeFileName outPath;
        BeTest::GetHost().GetOutputRoot(outPath);
        outPath.AppendToPath(testfixtureName);

        WString fileName(TEST_NAME, BentleyCharEncoding::Utf8);
        fileName.append(L".bim");
        BeFileName sourceFilepath(outPath);
        sourceFilepath.AppendToPath(fileName.c_str());

        BeFileName outFileName(outPath);
        outFileName.AppendToPath(testFile);

        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(sourceFilepath, outFileName));
        //BeFileName::BeCopyFile(fullFileName, outFullFileName) != BeFileNameStatus::Success

        //EXPECT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFileName.c_str(), __FILE__));
        auto db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());
        if (!db.IsValid())
            return nullptr;

        if (bcId.GetValue() != db->GetBriefcaseId().GetValue())
            {
            BeFileName name(db->GetFileName());
            db->AssignBriefcaseId(bcId);
            db->SaveChanges();
            db->CloseDb();
            DbResult result = BE_SQLITE_OK;
            db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
            }

        _OnSetupDb(*db);

        return db;
        }

    void SetupMasterFile(bool andClearTxns=true)
        {
        SetupSeedProject();
        if (andClearTxns)
            ClearRevisions(*m_db);
        }

    void ClearRevisions(DgnDbR db)
        {
        // Ensure the seed file doesn't contain any changes pending for a revision
        if (!db.IsMasterCopy())
            {
            DgnRevisionPtr rev = db.Revisions().StartCreateRevision();
            if (rev.IsValid())
                {
                db.Revisions().FinishCreateRevision();
                db.SaveChanges();
                }
            }
        }
        
    DgnModelPtr CreateModel(Utf8CP name, DgnDbR db)
        {
        SubjectCPtr rootSubject = db.Elements().GetRootSubject();
        PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, name);
        EXPECT_TRUE(partition.IsValid());
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().AcquireForElementInsert(*partition));
        EXPECT_TRUE(db.Elements().Insert<PhysicalPartition>(*partition).IsValid());
        PhysicalModelPtr model = PhysicalModel::Create(*partition);
        EXPECT_TRUE(model.IsValid());
        IBriefcaseManager::Request req;
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForModelInsert(req, *model, IBriefcaseManager::PrepareAction::Acquire));
        auto status = model->Insert();
        EXPECT_EQ(DgnDbStatus::Success, status);
        return DgnDbStatus::Success == status ? model : nullptr;
        }
};

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   02/17
//=======================================================================================
struct LockableSchemas
{
private:
    DgnDbR m_dgndb;
public:
    LockableSchemas(DgnDbR dgndb) : m_dgndb(dgndb) {}
    DgnDbR GetDgnDb() const { return m_dgndb; }
};

typedef LockableSchemas const& LockableSchemasCR;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksManagerTest : RepositoryManagerTest
{
    static LockableId MakeLockableId(DgnElementCR el) { return LockableId(el.GetElementId()); }
    static LockableId MakeLockableId(DgnModelCR model) { return LockableId(model.GetModelId()); }
    static LockableId MakeLockableId(DgnDbCR db) { return LockableId(db); }
    static LockableId MakeLockableId(LockableSchemasCR lockableSchemas) { return LockableId(lockableSchemas.GetDgnDb().Schemas()); }

    static Utf8String MakeLockableName(LockableId lid)
        {
        Utf8CP typeName = nullptr;
        switch (lid.GetType())
            {
            case LockableType::Db: typeName = "DgnDb"; break;
            case LockableType::Model: typeName = "Model"; break;
            case LockableType::Schemas: typeName = "Schemas"; break;
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
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryLockLevel(actualLevel, id));
        EXPECT_EQ(expLevel, actualLevel) << MakeLockableName(id).c_str();
        }

    template<typename T> void ExpectLevel(T const& obj, LockLevel level)
        {
        ExpectLevel(MakeLockableId(obj), level, ExtractDgnDb(obj));
        }

    template<typename T> static DgnDbR ExtractDgnDb(T const& obj) { return obj.GetDgnDb(); }

    Response AcquireResponse(LockRequestR req, DgnDbR db, ResponseOptions opts=ResponseOptions::None)
        {
        auto response = db.BriefcaseManager().AcquireLocks(req, opts);
        return response;
        }

    template<typename T> Response AcquireResponse(T const& obj, LockLevel level)
        {
        LockRequest req;
        req.Insert(obj, level);
        return AcquireResponse(req, ExtractDgnDb(obj), ResponseOptions::LockState);
        }

    template<typename T> RepositoryStatus Acquire(T const& obj, LockLevel level)
        {
        return AcquireResponse(obj, level).Result();
        }

    template<typename T> void ExpectAcquire(T const& obj, LockLevel level)
        {
        EXPECT_EQ(RepositoryStatus::Success, Acquire(obj, level));
        }

    template<typename T> void ExpectDenied(T const& obj, LockLevel level)
        {
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, Acquire(obj, level));
        }

    RepositoryStatus QueryOwnership(DgnLockOwnershipR ownership, LockableId const& id)
        {
        DgnLockInfoSet states;
        LockableIdSet ids;
        ids.insert(id);
        auto stat = m_server.QueryLockStates(states, ids);
        if (RepositoryStatus::Success == stat)
            {
            EXPECT_EQ(1, states.size());
            ownership = states.begin()->GetOwnership();
            }

        return stat;
        }

    template<typename T> void ExpectInDeniedSet(T const& lockedObj, LockLevel level, Response const& response, DgnDbR requestor)
        {
        // Test that locked obj is in response
        auto lockableId = MakeLockableId(lockedObj);
        auto found = response.LockStates().find(DgnLockInfo(lockableId));
        EXPECT_FALSE(found == response.LockStates().end());
        if (response.LockStates().end() != found)
            EXPECT_EQ(found->GetOwnership().GetLockLevel(), level);

        // Test that response matches direct ownership query
        DgnLockOwnership ownership;
        EXPECT_EQ(RepositoryStatus::Success, QueryOwnership(ownership, lockableId));
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

    DgnElementCPtr CreateElement(DgnModelR model, bool acquireLocks=true)
        {
        auto elem = model.Is3d() ? Create3dElement(model) : Create2dElement(model);
        if (acquireLocks)
            {
            IBriefcaseManager::Request req;
            EXPECT_EQ(RepositoryStatus::Success, model.GetDgnDb().BriefcaseManager().PrepareForElementInsert(req, *elem, IBriefcaseManager::PrepareAction::Acquire));
            }

        auto persistentElem = elem->Insert();
        return persistentElem;
        }

    DgnElementPtr Create3dElement(DgnModelR model)
        {
        DgnDbR db = model.GetDgnDb();
        DgnCategoryId categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(db);
        return GenericPhysicalObject::Create(*model.ToPhysicalModelP(), categoryId);
        }

    DgnElementPtr Create2dElement(DgnModelR model)
        {
        DgnDbR db = model.GetDgnDb();
        DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Annotation2d::GetHandler());
        DgnCategoryId categoryId = DgnDbTestUtils::GetFirstDrawingCategoryId(db);
        return AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, model.GetModelId(), classId, categoryId));
        }

    size_t CountLocks(IOwnedLocksIteratorR iter)
        {
        size_t count = 0;
        for (/*iter*/; iter.IsValid(); ++iter)
            ++count;

        return count;
        }
};

/*---------------------------------------------------------------------------------**//**
* gcc errs if defined inside the class: explicit specialization in non-namespace scope 
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<> DgnDbR LocksManagerTest::ExtractDgnDb(DgnDb const& obj) { return const_cast<DgnDbR>(obj); }

template<> RepositoryManager::Response LocksManagerTest::AcquireResponse(LockableSchemasCR const& lockableSchemas, LockLevel level)
    {
    LockRequest req;
    req.InsertSchemasLock(lockableSchemas.GetDgnDb());
    return AcquireResponse(req, lockableSchemas.GetDgnDb(), ResponseOptions::LockState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SingleBriefcaseLocksTest : LocksManagerTest
{
    DEFINE_T_SUPER(LocksManagerTest);

    // Our DgnDb is masquerading as a briefcase...it has no actual master copy.
    BeBriefcaseId   m_bcId;
    DgnModelId      m_modelId;
    DgnElementId    m_elemId;

    void _OnSetupDb(DgnDbR db) override
        {
        m_db = &db;
        m_modelId = DgnModel::DictionaryId();
        m_elemId = DgnDbTestUtils::GetFirstSpatialCategoryId(db);
        }

    void SetUp()
        {
        SetupMasterFile();
        }

    SingleBriefcaseLocksTest() : m_bcId(2) { }
    ~SingleBriefcaseLocksTest() {if (m_db.IsValid()) m_db->CloseDb();}

    DgnModelPtr CreateModel(Utf8CP name) { return T_Super::CreateModel(name, *m_db); }
    void DoIterateOwnedLocks(bool bulkMode);
    void DoDisconnectedWorkflow(bool bulkMode);
};

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, AcquireLocks)
    {
    SetupDb(L"AcquireLocksTest.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr pModel = db.Models().GetModel(m_modelId);
    ASSERT_TRUE(pModel.IsValid());
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    ASSERT_TRUE(cpEl.IsValid());

    DgnModelCR model = *pModel;
    DgnElementCR el = *cpEl;
    LockableSchemas lockableSchemas(db);

    ExpectAcquire(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);

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

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    ExpectAcquire(el, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Exclusive);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    // An exclusive model lock results in exclusive locks on all of its elements
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);

    // An exclusive db lock results in exclusive locks on all models, elements and schemas
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);

    // If we obtain an exclusive lock on a model, exclusive locks on its elements should be retained after refresh
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention ... bulk mode version.
* @bsimethod                                                    Paul.Connelly   10/15
* @bsimethod                                                    Sam.Wilson      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, AcquireLocksBulkMode)
    {
    SetupDb(L"AcquireLocksTestBulkMode.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr pModel = db.Models().GetModel(m_modelId);
    ASSERT_TRUE(pModel.IsValid());
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);
    ASSERT_TRUE(cpEl.IsValid());

    DgnModelCR model = *pModel;
    DgnElementCR el = *cpEl;
    LockableSchemas lockableSchemas(db);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ASSERT_TRUE(db.BriefcaseManager().IsBulkOperation()) << "StartBulkOperation should start bulk ops mode";
    ExpectAcquire(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ASSERT_FALSE(db.BriefcaseManager().IsBulkOperation()) << "SaveChanges should end bulk ops mode";
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(el, LockLevel::Shared);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(lockableSchemas, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(el, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(lockableSchemas, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(lockableSchemas, LockLevel::None);

    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Shared);
    ExpectLevel(model, LockLevel::Exclusive);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    // An exclusive model lock results in exclusive locks on all of its elements
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);

    // An exclusive db lock results in exclusive locks on all models, elements and schemas
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(lockableSchemas, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    ExpectLevel(lockableSchemas, LockLevel::Exclusive);

    // If we obtain an exclusive lock on a model, exclusive locks on its elements should be retained after refresh
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::None);
    ExpectLevel(el, LockLevel::None);
    ExpectLevel(db, LockLevel::None);
    db.SaveChanges();                                       // <<<<<<< end bulk ops
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, RelinquishLocks)
    {
    SetupDb(L"RelinquishLocksTest.bim", m_bcId);

    // Create a new element - requires locking the dictionary model + the db
    DgnDbR db = *m_db;
    auto txnPos = db.Txns().GetCurrentTxnId();
    CategorySelector element(db.GetDictionaryModel(), TEST_NAME);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().AcquireForElementInsert(element));
    EXPECT_TRUE(element.Insert().IsValid());

    // Cannot relinquish locks with uncommitted changes
    EXPECT_EQ(RepositoryStatus::PendingTransactions, db.BriefcaseManager().RelinquishLocks());

    // Cannot relinquish locks if we have changed the locked object
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());
    EXPECT_TRUE(db.Txns().IsUndoPossible());
    EXPECT_EQ(RepositoryStatus::LockUsed, db.BriefcaseManager().RelinquishLocks());

    // Undo local changes
    EXPECT_EQ(DgnDbStatus::Success, db.Txns().ReverseTo(txnPos));
    EXPECT_TRUE(db.Txns().IsRedoPossible());
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());

    // Now we can relinquish locks because we have not changed the objects
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    // Cannot undo/redo after relinquishing locks
    EXPECT_FALSE(db.Txns().IsRedoPossible());
    EXPECT_FALSE(db.Txns().IsUndoPossible());
    }

/*---------------------------------------------------------------------------------**//**
* Newly-created models and elements are implicitly exclusively locked by that briefcase.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, LocallyCreatedObjects)
    {
    SetupDb(L"LocallyCreatedObjectsTest.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr model = db.Models().GetModel(m_modelId);
    DgnElementCPtr elem = db.Elements().GetElement(m_elemId);

    // Create a new model.
    DgnModelPtr newModel = CreateModel("NewModel");

    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    // Our exclusive locks are only known locally, because they refer to elements not known to the server
    // If we refresh our local locks from server, we will need to re-obtain them.
    // Server *will* know about shared locks on locally-created models
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectLevel(*newModel, LockLevel::None);
    ExpectLevel(db, LockLevel::Shared);

    // Create a new element in our new model
    DgnElementCPtr newElem = CreateElement(*newModel);

    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Shared);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
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

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RefreshFromRepository());
    ExpectAcquire(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* Newly-created models and elements are implicitly exclusively locked by that briefcase ... bulk mode version.
* @bsimethod                                                    Paul.Connelly   11/15
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, LocallyCreatedObjectsBulkMode)
    {
    SetupDb(L"LocallyCreatedObjectsTestBulkMode.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelPtr model = db.Models().GetModel(m_modelId);
    DgnElementCPtr elem = db.Elements().GetElement(m_elemId);

    // Create a new model.
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    DgnModelPtr newModel = CreateModel("NewModel");
    ExpectLevel(db, LockLevel::None);
    ExpectLevel(*newModel, LockLevel::None);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);
    db.BriefcaseManager().EndBulkOperation();               // <<<<<<<< end bulk ops
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);

    // Note: EndBulkOperation actually acquired the (exclusive) lock on the new model from the server.

    // Create a new element in our new model
    db.BriefcaseManager().StartBulkOperation();             // >>>>>>> start bulk ops
    DgnElementCPtr newElem = CreateElement(*newModel);
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::None);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);
    db.BriefcaseManager().EndBulkOperation();               // <<<<<<<< end bulk ops
    ExpectLevel(db, LockLevel::Shared);
    ExpectLevel(*newModel, LockLevel::Exclusive);
    ExpectLevel(*newElem, LockLevel::Exclusive);
    ExpectLevel(*model, LockLevel::None);
    ExpectLevel(*elem, LockLevel::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleBriefcaseLocksTest::DoIterateOwnedLocks(bool bulkMode)
    {
    WPrintfString bcname(L"IterateOwnedLocksTest%ls.bim", (bulkMode? L"BulkMode": L""));
    SetupDb(bcname.c_str(), m_bcId);
    DgnDbR db = *m_db;
    IBriefcaseManagerR bc = db.BriefcaseManager();

    // We hold no locks
    IOwnedLocksIteratorPtr pIter = bc.GetOwnedLocks();
    ASSERT_TRUE(pIter.IsValid());
    EXPECT_FALSE(pIter->IsValid());
    EXPECT_EQ(0, CountLocks(*pIter));

    // Create a new model
    if (bulkMode)
        db.BriefcaseManager().StartBulkOperation();

    DgnModelPtr model = CreateModel("NewModel");
    pIter = bc.GetOwnedLocks();

    if (bulkMode)
        {
        EXPECT_EQ(0, CountLocks(*pIter));
        db.BriefcaseManager().EndBulkOperation();
        pIter->Reset();
        }

    // Creating a model involves creating a partition, which involves:
    //  - Shared lock on db
    //  - Shared lock on model containing partition element
    //  - Exclusive lock on partition element
    //  - Exclusive lock on newly-created model
    EXPECT_EQ(4, CountLocks(*pIter));

    // Iteration consumes the iterator - must manually reset to restart iteration
    EXPECT_FALSE(pIter->IsValid());
    pIter->Reset();
    EXPECT_TRUE(pIter->IsValid());

    // Confirm the locks
    bool haveModel = false,
         haveDb = false,
         havePartitionModel = false,
         havePartitionElement = false;

    for (auto& iter = *pIter; iter.IsValid(); ++iter)
        {
        DgnLock lock = *iter;
        switch (iter->GetType())
            {
            case LockableType::Db:
                {
                EXPECT_FALSE(haveDb);
                EXPECT_FALSE(lock.IsExclusive());
                haveDb = true;
                break;
                }
            case LockableType::Model:
                {
                if (lock.GetId() == model->GetModelId())
                    {
                    EXPECT_FALSE(haveModel);
                    EXPECT_TRUE(lock.IsExclusive());
                    haveModel = true;
                    }
                else
                    {
                    // Partition element's model
                    EXPECT_FALSE(havePartitionModel);
                    EXPECT_FALSE(lock.IsExclusive());
                    havePartitionModel = true;
                    }
                break;
                }
            case LockableType::Element:
                {
                // Partition element
                EXPECT_FALSE(havePartitionElement);
                EXPECT_TRUE(lock.IsExclusive());
                havePartitionElement = true;
                break;
                }
            }
        }

    EXPECT_TRUE(haveDb);
    EXPECT_TRUE(haveModel);
    EXPECT_TRUE(havePartitionElement);
    EXPECT_TRUE(havePartitionModel);

    pIter = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, IterateOwnedLocks)
    {
    DoIterateOwnedLocks(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/17
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, IterateOwnedLocksBulkMode)
    {
    DoIterateOwnedLocks(true);
    }

/*---------------------------------------------------------------------------------**//**
* Test that locks + codes acquired while connected are retained when disconnected.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleBriefcaseLocksTest::DoDisconnectedWorkflow(bool bulkMode)
    {
    BeFileName filename;
    DgnModelId newModelId;
        {
        SetupDb(L"DisconnectedWorkflowTest.bim", m_bcId);

        DgnDbR db = *m_db;
        DgnModelPtr pModel = db.Models().GetModel(m_modelId);
        DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);

        if (bulkMode)
            db.BriefcaseManager().StartBulkOperation();

        // Acquire locks with server available
        EXPECT_TRUE(nullptr != T_HOST.GetRepositoryAdmin()._GetRepositoryManager(db));
        ExpectAcquire(*cpEl, LockLevel::Exclusive);
        if (bulkMode)
            {
            ExpectLevel(db, LockLevel::None);
            ExpectLevel(*pModel, LockLevel::None);
            ExpectLevel(*cpEl, LockLevel::None);
            }
        else
            {
            ExpectLevel(db, LockLevel::Shared);
            ExpectLevel(*pModel, LockLevel::Shared);
            ExpectLevel(*cpEl, LockLevel::Exclusive);
            }

        // Create a new model (implicitly exclusively locked)
        DgnModelPtr newModel = CreateModel("NewModel");
        newModelId = newModel->GetModelId();
        if (bulkMode)
            ExpectLevel(*newModel, LockLevel::None);
        else
            ExpectLevel(*newModel, LockLevel::Exclusive);

        // Close the db and unregister the server
        m_db->SaveChanges();

        if (bulkMode)
            {
            // in build operation mode, all locks are acquired by savechanges
            ExpectLevel(db, LockLevel::Shared);
            ExpectLevel(*pModel, LockLevel::Shared);
            ExpectLevel(*cpEl, LockLevel::Exclusive);
            ExpectLevel(*newModel, LockLevel::Exclusive);
            }

        filename = m_db->GetFileName();
        pModel = nullptr;
        newModel = nullptr;
        cpEl = nullptr;
        m_db->CloseDb();
        m_db = nullptr;
        UnregisterServer();
        }

    // Reopen the db and expect that the locks we acquired remain acquired
        {
        m_db = DgnDb::OpenDgnDb(nullptr, filename, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(m_db.IsValid());
        EXPECT_EQ(nullptr, T_HOST.GetRepositoryAdmin()._GetRepositoryManager(*m_db));

        DgnModelPtr pModel = m_db->Models().GetModel(m_modelId);
        DgnElementCPtr cpEl = m_db->Elements().GetElement(m_elemId);
        ASSERT_TRUE(pModel.IsValid());
        ASSERT_TRUE(cpEl.IsValid());

        ExpectLevel(*m_db, LockLevel::Shared);
        ExpectLevel(*pModel, LockLevel::Shared);
        ExpectLevel(*cpEl, LockLevel::Exclusive);

        // Expect we cannot acquire new locks due to unavailability of server
        EXPECT_EQ(RepositoryStatus::ServerUnavailable, Acquire(*pModel, LockLevel::Exclusive));

        // Expect that we implicitly acquire locks for locally-created objects despite unavailability of server
        DgnModelPtr pNewModel = m_db->Models().GetModel(newModelId);
        ASSERT_TRUE(pNewModel.IsValid());
        ExpectLevel(*pNewModel, LockLevel::Exclusive);
        DgnElementCPtr newElem = CreateElement(*pNewModel);
        ASSERT_TRUE(newElem.IsValid());
        ExpectLevel(*newElem, LockLevel::Exclusive);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test that locks + codes acquired while connected are retained when disconnected.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, DisconnectedWorkflow)
    {
    DoDisconnectedWorkflow(false);
    }

/*---------------------------------------------------------------------------------**//**
* Test that locks + codes acquired while connected are retained when disconnected.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, DisconnectedWorkflowBulkMode)
    {
    DoDisconnectedWorkflow(true);
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
        uint32_t nElemsFound = 0;
        for (auto const& elems : model.MakeIterator())
            {
            ids[nElemsFound++] = elems.GetElementId();
            if (2 == nElemsFound)
                return true;
            }

        return false;
        }

    void SetUp()
        {
        SetupMasterFile(false);
        auto defaultModel = GetDefaultPhysicalModel();
        CreateElement(*defaultModel, false);
        CreateElement(*defaultModel, false);
        DgnModelPtr modelA2d0 = CreateModel("Model2d", *m_db);
        CreateElement(*modelA2d0.get(), false);
        CreateElement(*modelA2d0.get(), false);
        _InitMasterFile();
        m_db->SaveChanges();
        ClearRevisions(*m_db);
        }

    virtual void _InitMasterFile() { };

    void SetupDbs(uint32_t baseBcId=2)
        {
        m_dbA = SetupDb(L"DbA.bim", BeBriefcaseId(baseBcId));
        m_dbB = SetupDb(L"DbB.bim", BeBriefcaseId(baseBcId+1));

        ASSERT_TRUE(m_dbA.IsValid());
        ASSERT_TRUE(m_dbB.IsValid());

        // Model + element IDs may vary each time we convert the v8 file...need to look them up.
        DgnModelId model2d, model3d;
        for (ModelIteratorEntryCR entry : m_dbA->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometricModel)))
            {
            auto model = m_dbA->Models().GetModel(entry.GetModelId());
            if (model.IsValid())
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
        EXPECT_EQ(RepositoryStatus::Success, m_dbA->BriefcaseManager().RelinquishLocks());
        EXPECT_EQ(RepositoryStatus::Success, m_dbB->BriefcaseManager().RelinquishLocks());
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
    EXPECT_EQ(RepositoryStatus::Success, dbA.BriefcaseManager().RelinquishLocks());
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
* Operations on elements and models USED TO automatically acquire locks.
* Now, they do NOT - they only verify that the requisite locks were acquired prior to the
* operation. Caller must explicitly acquire those locks.
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
    EXPECT_EQ(DgnDbStatus::LockNotHeld, modelA2d->Delete());
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, dbA.BriefcaseManager().PrepareForModelDelete(req, *modelA2d, IBriefcaseManager::PrepareAction::Acquire));
    ExpectLevel(*modelA2d, LockLevel::Exclusive);
    EXPECT_EQ(DgnDbStatus::Success, modelA2d->Delete());
    DgnModelPtr modelA3d = GetModel(dbA, false);
    ExpectLevel(*modelA3d, LockLevel::None);
    EXPECT_EQ(DgnDbStatus::LockNotHeld, modelA3d->Delete()); // because B has a shared lock on it
    ExpectLevel(*modelA3d, LockLevel::None);

    // Adding an element to a model requires shared ownership
    DgnModelPtr modelB2d = GetModel(dbB, true);
    ExpectLevel(*modelB2d, LockLevel::None);
    DgnElementCPtr newElemB2d = CreateElement(*modelB2d, false);
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
    EXPECT_EQ(DgnDbStatus::LockNotHeld, cpElemB3d->Delete());
    LockRequest lockReq;
    lockReq.Insert(*cpElemB3d, LockLevel::Exclusive);
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(lockReq).Result());
    ExpectLevel(*cpElemB3d, LockLevel::Exclusive);
    EXPECT_EQ(DgnDbStatus::Success, cpElemB3d->Delete());

    DgnElementPtr pElemB3d2 = GetElement(dbB, Elem3dId2())->CopyForEdit();
    ExpectLevel(*pElemB3d2, LockLevel::None);
    EXPECT_FALSE(pElemB3d2->Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::LockNotHeld, status);
    lockReq.Clear();
    lockReq.Insert(*pElemB3d2, LockLevel::Exclusive);
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(lockReq).Result());
    ExpectLevel(*pElemB3d2, LockLevel::Exclusive);
    EXPECT_TRUE(pElemB3d2->Update(&status).IsValid());
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

    LockableId dbLockId = MakeLockableId(dbB);

    // Model: Locked exclusively
        {
        ExpectAcquire(*modelA2d, LockLevel::Exclusive);
        auto response = AcquireResponse(*modelB2d, LockLevel::Shared);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
        EXPECT_EQ(1, response.LockStates().size());

        ExpectInDeniedSet(*modelA2d, LockLevel::Exclusive, response, dbB);
        // Reformulate request => remove request for model
        Request request;
        request.Locks().Insert(*modelB2d, LockLevel::Shared);
        dbB.BriefcaseManager().ReformulateRequest(request, response);
        EXPECT_EQ(1, request.Locks().Size());
        EXPECT_TRUE(request.Locks().Contains(dbLockId));
        }

    RelinquishAll();

    // Model: Locked shared by both
        {
        ExpectAcquire(*modelA2d, LockLevel::Shared);
        ExpectAcquire(*modelB2d, LockLevel::Shared);

        // Try to lock exclusively
        auto response = AcquireResponse(*modelB2d, LockLevel::Exclusive);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
        EXPECT_EQ(1, response.LockStates().size());
        ExpectInDeniedSet(*modelA2d, LockLevel::Shared, response, dbB);

        // Expect two shared owners
        DgnLockOwnership ownership;
        auto modelLockId = MakeLockableId(*modelA2d);
        EXPECT_EQ(RepositoryStatus::Success, QueryOwnership(ownership, modelLockId));
        EXPECT_EQ(LockLevel::Shared, ownership.GetLockLevel());
        auto const& sharedOwners = ownership.GetSharedOwners();
        EXPECT_EQ(2, sharedOwners.size());
        EXPECT_TRUE(sharedOwners.find(dbA.GetBriefcaseId()) != sharedOwners.end());
        EXPECT_TRUE(sharedOwners.find(dbB.GetBriefcaseId()) != sharedOwners.end());

        // Reformulate request => demote exclusive to shared
        Request request;
        request.Locks().Insert(*modelB2d, LockLevel::Exclusive);
        dbB.BriefcaseManager().ReformulateRequest(request, response);
        EXPECT_EQ(2, request.Locks().Size());
        EXPECT_EQ(LockLevel::Shared, request.Locks().GetLockLevel(modelLockId));
        EXPECT_TRUE(request.Locks().Contains(dbLockId));
        EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(request.Locks()).Result());
        }

    RelinquishAll();

    // One element locked, one not
        {
        ExpectAcquire(*elemA2d2, LockLevel::Exclusive);

        // Try to lock both elems
        DgnElementCPtr elemB2d1 = GetElement(dbB, Elem2dId1());
        Request req;
        req.Locks().Insert(*elemB2d2, LockLevel::Exclusive);
        req.Locks().Insert(*elemB2d1, LockLevel::Exclusive);
        auto response = AcquireResponse(req.Locks(), dbB, ResponseOptions::LockState);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());

        // Reformulate request => remove one element
        dbB.BriefcaseManager().ReformulateRequest(req, response);
        EXPECT_EQ(LockLevel::Exclusive, req.Locks().GetLockLevel(MakeLockableId(*elemB2d1)));
        EXPECT_EQ(LockLevel::None, req.Locks().GetLockLevel(MakeLockableId(*elemB2d2)));
        EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(req.Locks()).Result());
        }

    RelinquishAll();

    // One model exclusively locked, one not
        {
        DgnModelPtr modelA3d = GetModel(dbA, false);
        ExpectAcquire(*modelA2d, LockLevel::Exclusive);

        // Try to lock one elem from each model
        DgnElementCPtr elemB3d2 = GetElement(dbB, Elem3dId2());
        Request req;
        req.Locks().Insert(*elemB2d2, LockLevel::Exclusive);
        req.Locks().Insert(*elemB3d2, LockLevel::Exclusive);
        auto response = AcquireResponse(req.Locks(), dbB, ResponseOptions::LockState);
        EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());

        // Reformulate request => remove element in locked model
        dbB.BriefcaseManager().ReformulateRequest(req, response);
        EXPECT_EQ(LockLevel::Exclusive, req.Locks().GetLockLevel(MakeLockableId(*elemB3d2)));
        EXPECT_EQ(LockLevel::None, req.Locks().GetLockLevel(MakeLockableId(*elemB2d2)));
        EXPECT_EQ(LockLevel::Shared, req.Locks().GetLockLevel(MakeLockableId(*modelA3d)));
        EXPECT_EQ(LockLevel::None, req.Locks().GetLockLevel(MakeLockableId(*modelB2d)));
        EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().AcquireLocks(req.Locks()).Result());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void touchElement(DgnElementCR el)
    {
    BeThreadUtilities::BeSleep(1);
    auto mod = el.CopyForEdit();
    mod->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DoubleBriefcaseTest, DemoteLocks)
    {
    SetupDbs();
    DgnDbR db = *m_dbA;
    DgnModelPtr model2d = GetModel(db, true);
    DgnModelPtr model3d = GetModel(db, false);
    DgnElementCPtr elem2d2 = GetElement(db, Elem2dId2());
    DgnElementCPtr elem3d2 = GetElement(db, Elem3dId2());

    LockableId model2dLockId = MakeLockableId(*model2d);
    LockableId elem2d2LockId = MakeLockableId(*elem2d2);

    // Lock model + elements exclusively
    ExpectAcquire(*elem2d2, LockLevel::Exclusive);
    ExpectAcquire(*elem3d2, LockLevel::Exclusive);
    ExpectAcquire(*model2d, LockLevel::Exclusive);
    ExpectAcquire(*model3d, LockLevel::Exclusive);

    // We can demote model lock from exclusive to shared while retaining element lock
    DgnLockSet toRelease;
    toRelease.insert(DgnLock(model2dLockId, LockLevel::Shared));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*model2d, LockLevel::Shared);
    ExpectLevel(*elem2d2, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

    // Releasing shared lock on model also release lock on element within that model (but not the element in the other model)
    toRelease.clear();
    toRelease.insert(DgnLock(model2dLockId, LockLevel::None));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*model2d, LockLevel::None);
    ExpectLevel(*elem2d2, LockLevel::None);
    ExpectLevel(*elem3d2, LockLevel::Exclusive);
    ExpectLevel(*model3d, LockLevel::Exclusive);

    // Modify the 3d elem
    auto txnId = db.Txns().GetCurrentTxnId();
    touchElement(*elem3d2);
    db.SaveChanges();

    // Can demote model lock to shared while retaining lock on modified element within i
    LockableId model3dLockId = MakeLockableId(*model3d);
    toRelease.clear();
    toRelease.insert(DgnLock(model3dLockId, LockLevel::Shared));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*model3d, LockLevel::Shared);
    ExpectLevel(*elem3d2, LockLevel::Exclusive);

    // Cannot release model lock if elements within it have been modified
    toRelease.clear();
    toRelease.insert(DgnLock(model3dLockId, LockLevel::None));
    EXPECT_EQ(RepositoryStatus::LockUsed, db.BriefcaseManager().DemoteLocks(toRelease));

    // If we undo the element change, we can release the locks
    EXPECT_EQ(DgnDbStatus::Success, db.Txns().ReverseTo(txnId));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().DemoteLocks(toRelease));
    ExpectLevel(*elem3d2, LockLevel::None);
    ExpectLevel(*model3d, LockLevel::None);
    
    // After releasing lock, redo of change to previously-locked element is not possible
    EXPECT_FALSE(db.Txns().IsRedoPossible());
    }

/*---------------------------------------------------------------------------------**//**
* A 'standalone briefcase' is a transactable DgnDb which did not originate from a master
* copy - i.e., it is not associated with a repository on a server somewhere. Generally
* used for tests; for developer activities operating on local files; or for initializing
* a DgnDb which will later become promoted to be the master DgnDb for a new repository
* on a server.
* TxnManager is enabled; no requirement to acquire locks/codes.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleBriefcaseTest, StandaloneBriefcase)
    {
    SetupDbs(BeBriefcaseId::Master());

    DgnDbR master = *m_dbA; // master copy
    DgnDbR brief  = *m_dbB; // standalone briefcase

    EXPECT_TRUE(master.IsMasterCopy());
    EXPECT_FALSE(master.Txns().IsUndoPossible());

    // Locks unconditionally granted for both...
    ExpectLevel(master, LockLevel::Exclusive);
    ExpectLevel(*GetElement(master, Elem2dId2()), LockLevel::Exclusive);
    ExpectAcquire(*GetElement(master, Elem3dId2()), LockLevel::Exclusive);

    ExpectLevel(brief, LockLevel::Exclusive);
    ExpectLevel(*GetElement(brief, Elem2dId2()), LockLevel::Exclusive);
    ExpectAcquire(*GetElement(brief, Elem3dId2()), LockLevel::Exclusive);

    // So are codes...
    AnnotationTextStyle style(master);
    style.SetName("MyStyle");
    DgnCode code = style.GetCode(); // the only reason we created the style...
    EXPECT_EQ(RepositoryStatus::Success, master.BriefcaseManager().ReserveCode(code));
    EXPECT_EQ(RepositoryStatus::Success, brief.BriefcaseManager().ReserveCode(code));

    // TxnManager is ONLY enabled for briefcase...
    EXPECT_TRUE(style.Insert().IsValid());
    EXPECT_EQ(BE_SQLITE_OK, master.SaveChanges());
    EXPECT_FALSE(master.Txns().IsUndoPossible());

    AnnotationTextStyle briefStyle(brief);
    briefStyle.SetName("MyStyle");
    EXPECT_TRUE(briefStyle.Insert().IsValid());
    EXPECT_EQ(BE_SQLITE_OK, brief.SaveChanges());
    EXPECT_TRUE(brief.Txns().IsUndoPossible());
    }

/*---------------------------------------------------------------------------------**//**
* For queries which need to be fast and not necessarily 100% up to date with server,
* IBriefcaseManager supplies a FastQuery option which queries a local copy of locks+codes
* known to be unavailable to this briefcase, either because another briefcase owns them
* or because a required revision has not been pulled.
* The primary use case for this is tools which want to filter out elements for which the
* lock is not available.
* Test that:
*   - While the cache is in sync with server, fast and slow queries both return same results
*   - When cache becomes out of sync, fast queries continue to return previous results
*   - When cache is out of sync, attempts to actually acquire locks/codes are validated against the server, not the local cache
*   - After refreshing local cache, fast queries are again in sync with server state
* @bsistruct                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct FastQueryTest : DoubleBriefcaseTest
{
    DEFINE_T_SUPER(DoubleBriefcaseTest);

    typedef IBriefcaseManager::FastQuery FastQuery;
    typedef IBriefcaseManager::RequestPurpose Purpose;

    template<typename T> static Utf8String ToString(T const& t)
        {
        Json::Value v;
        t.ToJson(v);
        return Json::FastWriter::ToString(v);
        }

    void ExpectEqual(DgnLockInfo const& a, DgnLockInfo const& b)
        {
        EXPECT_EQ(a.IsTracked(), b.IsTracked());
        EXPECT_EQ(a.IsOwned(), b.IsOwned());
        EXPECT_EQ(a.GetOwnership().GetLockLevel(), b.GetOwnership().GetLockLevel());
        }

    void ExpectEqual(DgnCodeInfo const& a, DgnCodeInfo const& b)
        {
        EXPECT_EQ(a.IsAvailable(), b.IsAvailable());
        EXPECT_EQ(a.IsReserved(), b.IsReserved());
        EXPECT_EQ(a.IsUsed(), b.IsUsed());
        EXPECT_EQ(a.IsDiscarded(), b.IsDiscarded());
        }

    static bool AreSameEntity(DgnLockInfo const& a, DgnLockInfo const& b) { return a.GetLockableId() == b.GetLockableId(); }
    static bool AreSameEntity(DgnCodeInfo const& a, DgnCodeInfo const& b) { return a.GetCode() == b.GetCode(); }

    template<typename T> void ExpectEqual(T const& a, T const& b)
        {
        EXPECT_EQ(a.size(), b.size());

        for (auto const& aInfo : a)
            {
            auto pbInfo = std::find_if(b.begin(), b.end(), [&](decltype(aInfo) arg) { return AreSameEntity(aInfo, arg); });
            EXPECT_FALSE(b.end() == pbInfo) << "Present in a only: " << ToString(aInfo).c_str();
            if (b.end() != pbInfo)
                ExpectEqual(aInfo, *pbInfo);
            }

        for (auto const& bInfo : b)
            {
            auto paInfo = std::find_if(a.begin(), a.end(), [&](decltype(bInfo) arg) { return AreSameEntity(bInfo, arg); });
            EXPECT_FALSE(a.end() == paInfo);
            }
        }

    void ExpectResponsesEqual(Response const& a, Response const& b)
        {
        EXPECT_EQ(a.Result(), b.Result());
        ExpectEqual(a.LockStates(), b.LockStates());
        ExpectEqual(a.CodeStates(), b.CodeStates());
        }

    void ExpectResponsesEqual(Request& req, DgnDbR db)
        {
        Request fastReq = req; // function modifies input Request...
        Response response(Purpose::Query, req.Options()), fastResponse(Purpose::FastQuery, req.Options());
        EXPECT_EQ(db.BriefcaseManager().AreResourcesAvailable(req, &response, FastQuery::No), db.BriefcaseManager().AreResourcesAvailable(fastReq, &fastResponse, FastQuery::Yes));
        ExpectResponsesEqual(response, fastResponse);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (FastQueryTest, CacheLocks)
    {
    SetupDbs();

    // dbA will acquire locks. dbB will make fast queries.
    DgnDbR dbA = *m_dbA,
           dbB = *m_dbB;
    DgnModelPtr modelA1 = GetModel(dbA, true),
                modelA2 = GetModel(dbA, false),
                modelB1 = GetModel(dbB, true),
                modelB2 = GetModel(dbB, false);
    DgnElementCPtr elemA1 = GetElement(dbA, Elem2dId2()),
                   elemA2 = GetElement(dbA, Elem3dId2()),
                   elemB1 = GetElement(dbB, Elem2dId2()),
                   elemB2 = GetElement(dbB, Elem3dId2());

    ExpectAcquire(*modelA1, LockLevel::Exclusive);
    ExpectAcquire(*modelA2, LockLevel::Shared);
    ExpectAcquire(*elemA1, LockLevel::Exclusive);

    // Make sure local state is pulled for dbB
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().RefreshFromRepository());

    // Confirm fast queries return same results as queries against server
    Request req(ResponseOptions::LockState);
    ExpectResponsesEqual(req, dbB);

    req.Reset();
    req.SetOptions(ResponseOptions::LockState);
    req.Locks().Insert(*modelB1, LockLevel::Shared);    // unavailable
    req.Locks().Insert(*modelB2, LockLevel::Shared);    // available
    req.Locks().Insert(*elemB1, LockLevel::Exclusive);  // unavailable - but not in denied set, because in exclusively locked model
    req.Locks().Insert(*elemB2, LockLevel::Exclusive);  // available
    ExpectResponsesEqual(req, dbB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (FastQueryTest, CacheCodes)
    {
    SetupDbs();

    // dbA will acquire codes. dbB will make fast queries
    DgnDbR dbA = *m_dbA,
           dbB = *m_dbB;

    DefinitionModelR dictionaryA = dbA.GetDictionaryModel();

    DgnCode code1 = DgnMaterial::CreateCode(dictionaryA, "One"),
            code2 = DgnMaterial::CreateCode(dictionaryA, "Two");

    // reserve codes
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_EQ(RepositoryStatus::Success, dbA.BriefcaseManager().ReserveCodes(codes).Result());

    // Make sure local state is pulled for dbB
    EXPECT_EQ(RepositoryStatus::Success, dbB.BriefcaseManager().RefreshFromRepository());

    // Confirm fast queries return same results as queries against server
    Request req(ResponseOptions::CodeState);
    ExpectResponsesEqual(req, dbB);

    DgnCode code3 = DgnMaterial::CreateCode(dictionaryA, "Three");
    req.Reset();
    req.SetOptions(ResponseOptions::CodeState);
    req.Codes().insert(code1);  // unavailable
    req.Codes().insert(code3);  // available
    ExpectResponsesEqual(req, dbB);
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

        RevisionStatus revStat;
        DgnRevisionPtr rev = m_db->Revisions().StartCreateRevision(&revStat);
        if (rev.IsNull())
            {
            if (RevisionStatus::NoTransactions == revStat)
                {
                req.Clear();
                return DgnDbStatus::Success;
                }
            else
                {
                return DgnDbStatus::BadRequest;
                }
            }

        req.FromRevision(*rev, *m_db);
        m_db->Revisions().AbandonCreateRevision();
        return DgnDbStatus::Success;
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
    m_db = SetupDb(L"UsedLocksTests.bim", m_bcId);

    DgnDbR db = *m_db;
    DgnModelR model = *db.Models().GetModel(m_modelId);
    DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);

    LockRequest req;
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_TRUE(req.IsEmpty());

    // Modify an elem (it's a SpatialCategory...)
        {
        UndoScope V_V_V_Undo(db);
        auto pEl = cpEl->CopyForEdit();
        DgnCode newCode = SpatialCategory::CreateCode(db.GetDictionaryModel(), "RenamedCategory");
        EXPECT_EQ(DgnDbStatus::Success, pEl->SetCode(newCode));
        IBriefcaseManager::Request bcreq;
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForElementUpdate(bcreq, *pEl, IBriefcaseManager::PrepareAction::Acquire));
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
    EXPECT_EQ(5, req.Size());
    EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*newModel)));
    EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*newElem)));
    EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));
    // Should also have locks for the RepositoryModel + Subject for newModel

    // Delete the new element
    EXPECT_EQ(DgnDbStatus::Success, newElem->Delete());
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_EQ(4, req.Size());
    EXPECT_EQ(LockLevel::Exclusive, req.GetLockLevel(LockableId(*newModel)));
    EXPECT_EQ(LockLevel::Shared, req.GetLockLevel(LockableId(db)));

    // Delete the new model
    EXPECT_EQ(DgnDbStatus::Success, newModel->Delete());
    EXPECT_EQ(DgnDbStatus::Success, ExtractLocks(req));
    EXPECT_EQ(3, req.Size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesManagerTest : RepositoryManagerTest
{
    void SetUp()
        {
        SetupMasterFile();
        }

    static DgnCodeInfo MakeAvailable(DgnCodeCR code) { return DgnCodeInfo(code); }
    static DgnCodeInfo MakeUsed(DgnCodeCR code, Utf8StringCR revisionId) { return MakeUsedOrDiscarded(code, revisionId, false); }
    static DgnCodeInfo MakeDiscarded(DgnCodeCR code, Utf8StringCR revisionId) { return MakeUsedOrDiscarded(code, revisionId, true); }
    static DgnCodeInfo MakeReserved(DgnCodeCR code, DgnDbR db)
        {
        DgnCodeInfo info(code);
        info.SetReserved(db.GetBriefcaseId());
        return info;
        }

    static DgnCodeInfo MakeUsedOrDiscarded(DgnCodeCR code, Utf8StringCR rev, bool discarded)
        {
        DgnCodeInfo info(code);
        if (discarded)
            info.SetDiscarded(rev);
        else
            info.SetUsed(rev);
        return info;
        }

    void ExpectState(DgnCodeInfoCR expect, DgnDbR db)
        {
        DgnCodeInfoSet expectInfos;
        expectInfos.insert(expect);
        ExpectStates(expectInfos, db);
        }

    void ExpectStates(DgnCodeInfoSet const& expect, DgnDbR db)
        {
        DgnCodeInfoSet actual;
        DgnCodeSet codes;
        for (auto const& info : expect)
            codes.insert(info.GetCode());

        EXPECT_STATUS(Success, m_server.QueryCodeStates(actual, codes));
        ExpectEqual(expect, actual);
        }

    void ExpectEqual(DgnCodeInfoSet const& expected, DgnCodeInfoSet const& actual)
        {
        EXPECT_EQ(expected.size(), actual.size());
        for (auto expIter = expected.begin(), actIter = actual.begin(); expIter != expected.end() && actIter != actual.end(); ++expIter, ++actIter)
            ExpectEqual(*expIter, *actIter);
        }

    void ExpectEqual(DgnCodeInfoCR exp, DgnCodeInfoCR act)
        {
        EXPECT_EQ(exp.GetCode(), act.GetCode());
        ExpectEqual(static_cast<DgnCodeState>(exp), static_cast<DgnCodeState>(act));
        }

    void ExpectEqual(DgnCodeStateCR exp, DgnCodeStateCR act)
        {
        if (exp.IsAvailable())
            {
            EXPECT_TRUE(act.IsAvailable());
            }
        else if (exp.IsReserved())
            {
            EXPECT_TRUE(act.IsReserved());
            EXPECT_EQ(exp.GetReservedBy().GetValue(), act.GetReservedBy().GetValue());
            }
        else
            {
            EXPECT_EQ(exp.IsDiscarded(), act.IsDiscarded());
            EXPECT_EQ(exp.GetRevisionId(), act.GetRevisionId());
            EXPECT_FALSE(exp.GetRevisionId().empty());
            }
        }

    Utf8String CommitRevision(DgnDbR db);

    static DgnDbStatus InsertStyle(Utf8CP name, DgnDbR db, bool expectSuccess = true)
        {
        AnnotationTextStyle style(db);
        style.SetName(name);
        IBriefcaseManager::Request req;
        EXPECT_EQ(expectSuccess, RepositoryStatus::Success == db.BriefcaseManager().PrepareForElementInsert(req, style, IBriefcaseManager::PrepareAction::Acquire));
        DgnDbStatus status;
        style.DgnElement::Insert(&status);
        return status;
        }

    static PhysicalElementPtr InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, BeGuidCR federationGuid=BeGuid(), DgnCodeCR code=DgnCode())
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(model, categoryId);
        EXPECT_TRUE(element.IsValid());
        element->SetFederationGuid(federationGuid);
        element->SetCode(code);
        EXPECT_TRUE(element->Insert().IsValid());
        return element;
        }

    static DgnCode MakeStyleCode(Utf8CP name, DgnDbR db)
        {
        // Because AnnotationTextStyle::CreateCodeFromName() is private for some reason...
        AnnotationTextStyle style(db);
        style.SetName(name);
        return style.GetCode();
        }

    DgnCodeSet GetReservedCodes(DgnDbR db)
        {
        DgnCodeSet codes;
        EXPECT_STATUS(Success, m_server._QueryCodes(codes, db));
        return codes;
        }

    bool IsCodeReserved(IBriefcaseManager& briefcaseManager, DgnCodeCR code)
        {
        DgnCodeSet codes;
        codes.insert(code);
        return briefcaseManager.AreCodesReserved(codes);
        }

    void AcquireSharedLockOnModel(DgnDbR db, DgnModelId modelId)
        {
        DgnModelPtr model = db.Models().GetModel(modelId);
        EXPECT_TRUE(model.IsValid());
        IBriefcaseManager::Request request;
        request.Locks().Insert(*model, LockLevel::Shared);
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().Acquire(request).Result());
        }
};

/*---------------------------------------------------------------------------------**//**
* Simulate pushing a revision to the server.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CodesManagerTest::CommitRevision(DgnDbR db)
    {
    Utf8String revId;
    DgnRevisionPtr rev;
    if (BE_SQLITE_OK != db.SaveChanges() || (rev = db.Revisions().StartCreateRevision()).IsNull())
        return revId;

    if (RevisionStatus::Success == db.Revisions().FinishCreateRevision())
        {
        m_server.OnFinishRevision(*rev, db);
        revId = rev->GetId();
        }

    return revId;
    }

/*---------------------------------------------------------------------------------**//**
* Exercise the basic functions: reserve + release codes, query code state
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, ReserveQueryRelinquish)
    {
    DgnDbPtr pDb = SetupDb(L"ReserveQueryRelinquishTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;
    DefinitionModelR dictionary = db.GetDictionaryModel();
    IBriefcaseManagerR mgr = db.BriefcaseManager();

    // Empty request
    DgnCodeSet req;
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());

    // Reserve single code
    DgnCode code = DgnMaterial::CreateCode(dictionary, "Material");
    req.insert(code);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());
    ExpectState(MakeReserved(code, db), db);

    // Relinquish all
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeAvailable(code), db);

    // Reserve 2 codes
    DgnCode code2 = SpatialCategory::CreateCode(dictionary, "Category");
    req.insert(code2);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());
    ExpectState(MakeReserved(code, db), db);
    ExpectState(MakeReserved(code2, db), db);

    // Release one code
    DgnCodeSet codes;
    codes.insert(code);
    EXPECT_STATUS(Success, mgr.ReleaseCodes(codes));
    ExpectState(MakeAvailable(code), db);
    ExpectState(MakeReserved(code2, db), db);

    // Relinquish all
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeAvailable(code2), db);
    }

/*---------------------------------------------------------------------------------**//**
* Test that any INSERT or UPDATE will fail if the object's code has not been reserved.
* NOTE: Previously, insert/update would attempt to reserve the code automatically for us.
* That is no longer the case - if we don't explicitly reserve it prior to insert/update,
* the operation will fail with DgnDbStatus::CodeNotReserved.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, AutoReserveCodes)
    {
    DgnDbPtr pDb = SetupDb(L"AutoReserveCodesTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;

    // Simulate a pre-existing style having been committed in a previous revision
    static const Utf8String s_initialRevisionId("InitialRevision");
    DgnCode existingStyleCode = MakeStyleCode("Preexisting", db);
    m_server.MarkUsed(existingStyleCode, s_initialRevisionId);

    ExpectState(MakeUsed(existingStyleCode, s_initialRevisionId), db);
    EXPECT_EQ(0, GetReservedCodes(db).size());

    // When we insert an element without having explicitly reserved its code, an attempt to reserve it will automatically occur
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("MyStyle", db));
    EXPECT_EQ(1, GetReservedCodes(db).size());
    ExpectState(MakeReserved(MakeStyleCode("MyStyle", db), db), db);

    // An attempt to insert an element with the same code as an already-used code will fail
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, InsertStyle(existingStyleCode.GetValue().c_str(), db, false));

    // Updating an element and changing its code will NOT reserve the new code if we haven't done so already
    auto pStyle = AnnotationTextStyle::Get(db, "MyStyle")->CreateCopy();
    DgnDbStatus status;
    pStyle->SetName("MyRenamedStyle");
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);
    // Explicitly reserve the code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(pStyle->GetCode()));
    EXPECT_TRUE(pStyle->Update().IsValid());
    EXPECT_EQ(2, GetReservedCodes(db).size());
    ExpectState(MakeReserved(pStyle->GetCode(), db), db);
    pStyle = nullptr;

    // Attempting to change code to an already-used code will fail on update
    pStyle = AnnotationTextStyle::Get(db, "MyRenamedStyle")->CreateCopy();
    pStyle->SetName(existingStyleCode.GetValue().c_str());
    EXPECT_TRUE(pStyle->DgnElement::Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);

    pStyle = nullptr;

    db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* When we commit a revision, the changes made to the revision are processed such that:
*   - Any codes which were newly-assigned become marked as "used", and
*   - Any previously-used codes which became no-longer-used are marked as "discarded"
* In both cases the server records the revision ID.
* Codes which become "used" were necessarily previously "reserved" by the briefcase.
* After committing the revision, they are no longer "reserved".
* Codes which became "discarded" were necessarily previously "used".
* After committing the revision, they become available for reserving by any briefcase,
* provided the briefcase has pulled the revision in which they became discarded.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, CodesInRevisions)
    {
    DgnDbPtr pDb = SetupDb(L"CodesInRevisionsTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;
    IBriefcaseManagerR mgr = db.BriefcaseManager();

    // Reserve some codes
    DgnCode unusedCode = MakeStyleCode("Unused", db);
    DgnCode usedCode = MakeStyleCode("Used", db);
    DgnCodeSet req;
    req.insert(unusedCode);
    req.insert(usedCode);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());

    // Use one of the codes
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("Used", db));
    ExpectState(MakeReserved(unusedCode, db), db);
    ExpectState(MakeReserved(usedCode, db), db);

    // Commit the change as a revision
    Utf8String rev1 = CommitRevision(db);
    EXPECT_FALSE(rev1.empty());

    // The used code should not be marked as such
    ExpectState(MakeUsed(usedCode, rev1), db);
    ExpectState(MakeReserved(unusedCode, db), db);

    // We cannot reserve a used code
    EXPECT_STATUS(CodeUnavailable, mgr.ReserveCode(usedCode));

    // Swap the code so that "Used" becomes "Unused"
    auto pStyle = AnnotationTextStyle::Get(db, "Used")->CreateCopy();
    pStyle->SetName("Unused");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Commit the revision
    Utf8String rev2 = CommitRevision(db);
    EXPECT_FALSE(rev2.empty());

    // "Used" is now discarded; "Unused" is now used; both in the same revision
    ExpectState(MakeUsed(unusedCode, rev2), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);

    // Delete the style => its code becomes discarded
    // Ugh except you are not allowed to delete text styles...rename it again instead
    pStyle = AnnotationTextStyle::Get(db, "Unused")->CreateCopy();
    pStyle->SetName("Deleted");
    // Will fail because we haven't reserved code...
    DgnDbStatus status;
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(MakeStyleCode("Deleted", db)));
    EXPECT_TRUE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::Success, status);
    pStyle = nullptr;

    // Cannot release codes if transactions are pending
    EXPECT_EQ(1, GetReservedCodes(db).size());
    DgnCodeSet codes;
    codes.insert(MakeStyleCode("Deleted", db));
    EXPECT_STATUS(PendingTransactions, mgr.ReleaseCodes(codes));
    EXPECT_STATUS(PendingTransactions, mgr.RelinquishCodes());

    // Cannot release a code which is used locally
    db.SaveChanges();
    EXPECT_EQ(1, GetReservedCodes(db).size());
    EXPECT_STATUS(CodeUsed, mgr.ReleaseCodes(codes));
    EXPECT_STATUS(CodeUsed, mgr.RelinquishCodes());

    Utf8String rev3 = CommitRevision(db);
    EXPECT_FALSE(rev3.empty());
    ExpectState(MakeDiscarded(unusedCode, rev3), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);

    // We can reserve either code, since they are discarded and we have the latest revision
    EXPECT_STATUS(Success, mgr.ReserveCode(usedCode));
    EXPECT_STATUS(Success, mgr.ReserveCode(unusedCode));
    ExpectState(MakeReserved(usedCode, db), db);
    ExpectState(MakeReserved(unusedCode, db), db);

    // If we release these codes, they should return to "Discarded" and retain the most recent revision ID in which they were discarded.
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeDiscarded(unusedCode, rev3), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);
    }

/*---------------------------------------------------------------------------------**//**
* It is common in Plant to reserve codes before elements exist.
* @bsimethod                                                    Shaun.Sewall    05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, PlantScenario)
    {
    DgnDbPtr pDb = SetupDb(L"PlantScenarioTest.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;

    EXPECT_FALSE(db.BriefcaseManager().IsBulkOperation());
    db.BriefcaseManager().StartBulkOperation();
    EXPECT_TRUE(db.BriefcaseManager().IsBulkOperation());

    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(db, "TestPhysicalModel");
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "TestSpatialCategory");

    CodeSpecPtr unitCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Unit", CodeScopeSpec::CreateModelScope());
    EXPECT_TRUE(unitCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, unitCodeSpec->Insert());
    EXPECT_TRUE(unitCodeSpec->IsModelScope());
    EXPECT_FALSE(unitCodeSpec->GetScope().IsFederationGuidRequired());

    Utf8CP fakeRelationship = "Fake.EquipmentScopedByUnit"; // relationship name not validated by CodeScopeSpec yet
    CodeSpecPtr equipmentCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Equipment", CodeScopeSpec::CreateRelatedElementScope(fakeRelationship, DgnCode::ScopeRequirement::FederationGuid));
    EXPECT_TRUE(equipmentCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, equipmentCodeSpec->Insert());
    EXPECT_TRUE(equipmentCodeSpec->IsRelatedElementScope());
    EXPECT_STREQ(equipmentCodeSpec->GetScope().GetRelationship().c_str(), fakeRelationship);
    EXPECT_TRUE(equipmentCodeSpec->GetScope().IsFederationGuidRequired());

    CodeSpecPtr nozzleCodeSpec = CodeSpec::Create(db, "CodesManagerTest.Nozzle", CodeScopeSpec::CreateParentElementScope(DgnCode::ScopeRequirement::FederationGuid));
    EXPECT_TRUE(nozzleCodeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, nozzleCodeSpec->Insert());
    EXPECT_TRUE(nozzleCodeSpec->IsParentElementScope());
    EXPECT_TRUE(nozzleCodeSpec->GetScope().IsFederationGuidRequired());

    db.SaveChanges("1");

    BeGuid unitGuid(true);
    BeGuid equipment1Guid(true);
    BeGuid nozzle11Guid(true);
    BeGuid nozzle12Guid(true);
    BeGuid equipment2Guid(true);
    BeGuid nozzle21Guid(true);
    BeGuid nozzle22Guid(true);
    BeGuid nozzle23Guid(true);

    DgnCode unitCode = unitCodeSpec->CreateCode(*physicalModel, "U1");
    DgnCode equipment1Code = equipmentCodeSpec->CreateCode(unitGuid, "U1-E1");
    DgnCode nozzle11Code = nozzleCodeSpec->CreateCode(equipment1Guid, "N1");
    DgnCode nozzle12Code = nozzleCodeSpec->CreateCode(equipment1Guid, "N2");
    DgnCode equipment2Code = equipmentCodeSpec->CreateCode(unitGuid, "U1-E2");
    DgnCode nozzle21Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N1");
    DgnCode nozzle22Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N2");
    DgnCode nozzle23Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N3");
    DgnCode nozzle24Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N4");
    DgnCode nozzle25Code = nozzleCodeSpec->CreateCode(equipment2Guid, "N5");

    DgnCodeSet codesToReserve;
    codesToReserve.insert(unitCode);
    codesToReserve.insert(equipment1Code);
    codesToReserve.insert(nozzle11Code);
    codesToReserve.insert(nozzle12Code);
    codesToReserve.insert(equipment2Code);
    codesToReserve.insert(nozzle21Code);
    codesToReserve.insert(nozzle22Code);
    codesToReserve.insert(nozzle23Code);
    codesToReserve.insert(nozzle24Code);
    codesToReserve.insert(nozzle25Code);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCodes(codesToReserve).Result());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesToReserve));

    DgnCodeSet codesFromServer = GetReservedCodes(db);
    for (DgnCodeCR code : codesToReserve)
        {
        EXPECT_TRUE(codesFromServer.end() != codesFromServer.find(code));
        }

    DgnCodeInfoSet codeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(codeStates, codesToReserve));
    for (DgnCodeInfo const& codeState : codeStates)
        {
        EXPECT_TRUE(codeState.IsReserved());
        }

    IBriefcaseManager::Request request1, request2;
    request1.Codes() = codesToReserve;
    Json::Value requestJson(Json::objectValue);
    request1.ToJson(requestJson);
    EXPECT_TRUE(request2.FromJson(requestJson));
    EXPECT_EQ(request2.Codes().size(), codesToReserve.size());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(request2.Codes()));

    DgnCodeSet wrongCodes;
    wrongCodes.insert(unitCodeSpec->CreateCode(*physicalModel, "U2")); // wrong name
    wrongCodes.insert(unitCodeSpec->CreateCode(db.GetDictionaryModel(), "U1")); // wrong model
    wrongCodes.insert(DgnCode(unitCodeSpec->GetCodeSpecId(), DgnElementId(), "U1")); // invalid scope
    wrongCodes.insert(DgnCode(nozzleCodeSpec->GetCodeSpecId(), DgnCode::ScopeRequirement::Unknown, DgnElementId(), BeGuid(), "N1")); // invalid scope
    wrongCodes.insert(nozzleCodeSpec->CreateCode(equipment1Guid, "N3")); // wrong scope

    for (DgnCodeCR wrongCode : wrongCodes)
        {
        BeTest::SetFailOnAssert(false);
        bool isCodeReserved = IsCodeReserved(db.BriefcaseManager(), wrongCode);
        BeTest::SetFailOnAssert(true);

        EXPECT_FALSE(isCodeReserved) << "wrongCode should not be reserved";
        EXPECT_TRUE(codesFromServer.end() == codesFromServer.find(wrongCode)) << "Should not find wrongCode on server";
        }

    DgnCodeSet availableCodes;
    availableCodes.insert(unitCodeSpec->CreateCode(*physicalModel, "U2"));
    availableCodes.insert(unitCodeSpec->CreateCode(*physicalModel, "U3"));
    availableCodes.insert(nozzleCodeSpec->CreateCode(equipment1Guid, "N3"));
    availableCodes.insert(nozzleCodeSpec->CreateCode(equipment1Guid, "N4"));

    IBriefcaseManager::Request availableCodesRequest;
    availableCodesRequest.Codes() = availableCodes;
    EXPECT_TRUE(db.BriefcaseManager().AreResourcesAvailable(availableCodesRequest));
    EXPECT_FALSE(db.BriefcaseManager().AreCodesReserved(availableCodesRequest.Codes()));

    DgnCodeInfoSet availableCodeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(availableCodeStates, availableCodes));
    for (DgnCodeInfo const& codeState : availableCodeStates)
        {
        EXPECT_TRUE(codeState.IsAvailable());
        }

    DgnCodeSet codesEmpty;
    DgnCodeSet codesSkipped;
    codesSkipped.insert(DgnCode::CreateEmpty());
    codesSkipped.insert(DgnCode());
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesEmpty)) << "Should return true because there is nothing to reserve (empty list of codes)";
    EXPECT_TRUE(db.BriefcaseManager().AreCodesReserved(codesSkipped)) << "Should return true because there is nothing to reserve (effectively an empty list of codes)";

    DgnCodeSet codesToRelease;
    codesToRelease.insert(nozzle24Code);
    codesToRelease.insert(nozzle25Code);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReleaseCodes(codesToRelease));
    EXPECT_FALSE(db.BriefcaseManager().AreCodesReserved(codesToRelease));

    db.BriefcaseManager().StartBulkOperation();

    PhysicalElementPtr unitElement = InsertPhysicalElement(*physicalModel, categoryId, unitGuid, unitCode);
    PhysicalElementPtr equipment1Element = InsertPhysicalElement(*physicalModel, categoryId, equipment1Guid, equipment1Code);
    PhysicalElementPtr nozzle11Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle11Guid, nozzle11Code);
    PhysicalElementPtr nozzle12Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle12Guid, nozzle12Code);
    PhysicalElementPtr equipment2Element = InsertPhysicalElement(*physicalModel, categoryId, equipment2Guid, equipment2Code);
    PhysicalElementPtr nozzle21Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle21Guid, nozzle21Code);
    PhysicalElementPtr nozzle22Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle22Guid, nozzle22Code);
    PhysicalElementPtr nozzle23Element = InsertPhysicalElement(*physicalModel, categoryId, nozzle23Guid, nozzle23Code);

    EXPECT_EQ(unitCode, unitElement->GetCode());
    EXPECT_EQ(equipment1Code, equipment1Element->GetCode());
    EXPECT_EQ(nozzle11Code, nozzle11Element->GetCode());
    EXPECT_EQ(nozzle12Code, nozzle12Element->GetCode());
    EXPECT_EQ(equipment2Code, equipment2Element->GetCode());
    EXPECT_EQ(nozzle21Code, nozzle21Element->GetCode());
    EXPECT_EQ(nozzle22Code, nozzle22Element->GetCode());
    EXPECT_EQ(nozzle23Code, nozzle23Element->GetCode());

    DgnCodeSet reservedCodes;
    reservedCodes.insert(unitElement->GetCode());
    reservedCodes.insert(equipment1Element->GetCode());
    reservedCodes.insert(nozzle11Element->GetCode());
    reservedCodes.insert(nozzle12Element->GetCode());
    reservedCodes.insert(equipment2Element->GetCode());
    reservedCodes.insert(nozzle21Element->GetCode());
    reservedCodes.insert(nozzle22Element->GetCode());
    reservedCodes.insert(nozzle23Element->GetCode());

    DgnCodeInfoSet reservedCodeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(reservedCodeStates, reservedCodes));
    for (DgnCodeInfo const& codeState : reservedCodeStates)
        {
        EXPECT_TRUE(codeState.IsReserved());
        }

    db.SaveChanges("2");
    CommitRevision(db); // simulates pushing revision to server which should mark codes as used

    DgnCodeSet usedCodes;
    usedCodes.insert(unitCode);
    usedCodes.insert(equipment1Code);
    usedCodes.insert(nozzle11Code);
    usedCodes.insert(nozzle12Code);
    usedCodes.insert(equipment2Code);
    usedCodes.insert(nozzle21Code);
    usedCodes.insert(nozzle22Code);
    usedCodes.insert(nozzle23Code);

    DgnCodeInfoSet usedCodeStates;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().QueryCodeStates(usedCodeStates, usedCodes));
    for (DgnCodeInfo const& codeState : usedCodeStates)
        {
        EXPECT_TRUE(codeState.IsUsed());
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct IndirectLocksTest : DoubleBriefcaseTest
{
    DgnElementId    m_displayStyleId;

    void _InitMasterFile() override
        {
        DisplayStyle2d style(m_db->GetDictionaryModel(), "MyDisplayStyle");
        style.Insert();
        m_displayStyleId = style.GetElementId();
        ASSERT_TRUE(m_displayStyleId.IsValid());
        }

    void Acquire(DgnElementCR el, BeSQLite::DbOpcode op)
        {
        IBriefcaseManager::Request req;
        EXPECT_STATUS(Success, el.PopulateRequest(req, op));
        EXPECT_STATUS(Success, el.GetDgnDb().BriefcaseManager().Acquire(req).Result());
        }

    bool DeleteDisplayStyle(DgnDbR db);
    bool CreateView(DgnDbR db);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IndirectLocksTest::DeleteDisplayStyle(DgnDbR db)
    {
    DgnCode styleCode = DisplayStyle::CreateCode(db.GetDictionaryModel(), "MyDisplayStyle");
    auto style = db.Elements().Get<DisplayStyle>(db.Elements().QueryElementIdByCode(styleCode));
    EXPECT_TRUE(style.IsValid());
    if (!style.IsValid())
        return false;

    Acquire(*style, BeSQLite::DbOpcode::Delete);
    return DgnDbStatus::Success == style->Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IndirectLocksTest::CreateView(DgnDbR db)
    {
    CategorySelectorPtr categorySelector = new CategorySelector(db.GetDictionaryModel(), "MyCategorySelector");
    Acquire(*categorySelector, BeSQLite::DbOpcode::Insert);
    if (!categorySelector->Insert().IsValid())
        return false;

    auto displayStyleA = db.Elements().GetForEdit<DisplayStyle2d>(m_displayStyleId);
    EXPECT_TRUE(displayStyleA.IsValid());
    if (!displayStyleA.IsValid())
        return false;

    DrawingViewDefinition view(db.GetDictionaryModel(), "MyView", Model2dId(), *categorySelector, *displayStyleA);
    Acquire(view, BeSQLite::DbOpcode::Insert);
    return view.Insert().IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IndirectLocksTest, UseThenDelete)
    {
    SetupDbs();

    EXPECT_TRUE(CreateView(*m_dbA));
    EXPECT_FALSE(DeleteDisplayStyle(*m_dbB));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IndirectLocksTest, DeleteThenUse)
    {
    SetupDbs();

    EXPECT_TRUE(DeleteDisplayStyle(*m_dbB));
    EXPECT_FALSE(CreateView(*m_dbA));
    }

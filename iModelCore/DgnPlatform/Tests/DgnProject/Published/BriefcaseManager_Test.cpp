/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/BriefcaseManager_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnTrueColor.h>
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
private:
    Db  m_db;

    // impl
    virtual Response _ProcessRequest(Request const& req, DgnDbR db) override;
    virtual RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
    virtual RepositoryStatus _Relinquish(Resources which, DgnDbR db) override;
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db) override;
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override;

    DbResult CreateLocksTable();
    DbResult CreateCodesTable();

    // locks
    bool AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId requestor);
    void GetDeniedLocks(DgnLockInfoSet& locks, LockRequestCR reqs, BeBriefcaseId bcId);
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
    void _ReserveCodes(Response& response, DgnCodeSet const& req, DgnDbR db, Options opts);
    RepositoryStatus _ReleaseCodes(DgnCodeSet const&, DgnDbR);
    RepositoryStatus _RelinquishCodes(DgnDbR);
    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&);
public:
    RepositoryManager();

    // Simulates what the real server does with codes when a revision is pushed.
    void OnFinishRevision(DgnRevision const& rev)
        {
        MarkRevision(rev.GetAssignedCodes(), false, rev.GetId());
        MarkRevision(rev.GetDiscardedCodes(), true, rev.GetId());
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
    virtual bool _IsInSet(int nVals, DbValue const* vals) const override
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

    virtual bool _IsLockInSet(DgnLockCR lock) const override
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

    virtual bool _IsLockInSet(DgnLockCR lock) const override
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

    virtual bool _IsLockInSet(DgnLockCR lock) const override
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

    virtual bool _IsLockInSet(DgnLockCR lock) const override
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
#define CODE_Authority "Authority"
#define CODE_NameSpace "NameSpace"
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

    DbResult result = m_db.CreateNewDb(BEDB_MemoryDb);
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
            CODE_Authority " INTEGER,"
            CODE_NameSpace " TEXT,"
            CODE_Value " TEXT,"
            CODE_State " INTEGER,"
            CODE_Revision " TEXT,"
            CODE_Briefcase " INTEGER,"
            "PRIMARY KEY(" CODE_Authority "," CODE_NameSpace "," CODE_Value ")");
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
RepositoryManager::Response RepositoryManager::_ProcessRequest(Request const& req, DgnDbR db)
    {
    Response response;
    _AcquireLocks(response, req.Locks(), db, req.Options());
    if (RepositoryStatus::Success == response.Result())
        _ReserveCodes(response, req.Codes(), db, req.Options());

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
RepositoryStatus RepositoryManager::_QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnDbR db)
    {
    auto stat = _QueryLocks(locks, db);
    if (RepositoryStatus::Success == stat)
        stat = _QueryCodes(codes, db);

    return stat;
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
            DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
            DgnCodeInfo info(code);
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
    stmt.Prepare(m_db, "INSERT INTO " TABLE_Codes "(" CODE_Authority "," CODE_NameSpace "," CODE_Value "," CODE_State "," CODE_Revision
            ") VALUES (?,?,?,2,?)");

    for (auto const& info : discarded)
        {
        auto const& code = info.GetCode();
        stmt.BindId(1, code.GetAuthority());
        stmt.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
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

    virtual bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        return m_codes.end() != std::find_if(m_codes.begin(), m_codes.end(), [&](DgnCode const& arg)
            {
            return arg.GetAuthority().GetValueUnchecked() == vals[0].GetValueUInt64()
                && arg.GetNamespace().Equals(vals[1].GetValueText())
                && arg.GetValue().Equals(vals[2].GetValueText());
            });
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryManager::_ReserveCodes(Response& response, DgnCodeSet const& req, DgnDbR db, Options opts)
    {
    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " CODE_Authority "," CODE_NameSpace "," CODE_Value "," CODE_State "," CODE_Revision "," CODE_Briefcase
                    "   FROM " TABLE_Codes " WHERE InVirtualSet(@vset, " CODE_Authority "," CODE_NameSpace "," CODE_Value ")");

    DgnCodeInfoSet discarded;

    RepositoryStatus status = RepositoryStatus::Success;
    stmt.BindVirtualSet(1, vset);
    bool wantInfos = Options::CodeState == (opts & Options::CodeState);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
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
    if (RepositoryStatus::Success != status)
        return;

    auto bcId = static_cast<int>(db.GetBriefcaseId().GetValue());
    Statement insert;
    insert.Prepare(m_db, "INSERT OR REPLACE INTO " TABLE_Codes "(" CODE_Authority "," CODE_NameSpace "," CODE_Value "," CODE_State "," CODE_Briefcase "," CODE_Revision
                        ") VALUES (?,?,?,1,?,?)");
    for (auto const& code : req)
        {
        insert.BindId(1, code.GetAuthority());
        insert.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
        insert.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        insert.BindInt(4, bcId);
        auto revIter = discarded.find(DgnCodeInfo(code));
        if (discarded.end() != revIter)
            insert.BindText(5, revIter->GetRevisionId(), Statement::MakeCopy::No);

        insert.Step();
        insert.Reset();
        }
    }

#define SELECT_ValidateRelease "SELECT " CODE_Authority "," CODE_NameSpace "," CODE_Value "," CODE_Briefcase "," CODE_State "," CODE_Revision " FROM " TABLE_Codes

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus RepositoryManager::ValidateRelease(DgnCodeInfoSet& discarded, DgnCodeSet const& req, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE InVirtualSet(@vset, " CODE_Authority "," CODE_NameSpace "," CODE_Value ")");
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
    stmt.Prepare(m_db, "DELETE FROM " TABLE_Codes " WHERE " CODE_Briefcase "=? AND InVirtualSet(@vset, " CODE_Authority "," CODE_NameSpace "," CODE_Value ")");
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
    stmt.Prepare(m_db, "SELECT " CODE_Authority "," CODE_NameSpace "," CODE_Value "," CODE_State "," CODE_Revision "," CODE_Briefcase
                    "   FROM " TABLE_Codes " WHERE InVirtualSet(@vset, " CODE_Authority "," CODE_NameSpace "," CODE_Value ")");
    stmt.BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
        DgnCodeInfo info(code);
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

    for (auto const& code : codes)
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
    stmt.Prepare(m_db, "SELECT " CODE_Authority "," CODE_NameSpace "," CODE_Value "   FROM " TABLE_Codes " WHERE " CODE_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    
    while (BE_SQLITE_ROW == stmt.Step())
        codes.insert(DgnCode(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1)));

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
    stmt.Prepare(m_db, "INSERT OR REPLACE INTO " TABLE_Codes "(" CODE_Authority "," CODE_NameSpace "," CODE_Value "," CODE_State "," CODE_Revision
            ") VALUES (?,?,?,?,?)");
    for (auto const& code : codes)
        {
        stmt.BindId(1, code.GetAuthority());
        stmt.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
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
struct RepositoryManagerTest : public ::testing::Test, DgnPlatformLib::Host::RepositoryAdmin
{
    typedef IRepositoryManager::Request Request;
    typedef IRepositoryManager::Response Response;
    typedef IBriefcaseManager::ResponseOptions ResponseOptions;

    mutable RepositoryManager   m_server;
    ScopedDgnHost               m_host;

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

    virtual IRepositoryManagerP _GetRepositoryManager(DgnDbR) const override { return &m_server; }

    virtual void _OnSetupDb(DgnDbR db) { }

    DgnDbPtr SetupDb(WCharCP testFile, BeBriefcaseId bcId, WCharCP baseFile=L"3dMetricGeneral.ibim")
        {
        BeFileName outFileName;
        EXPECT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFile, __FILE__));
        auto db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());
        if (!db.IsValid())
            return nullptr;

        if (bcId.GetValue() != db->GetBriefcaseId().GetValue())
            {
            TestDataManager::MustBeBriefcase(db, Db::OpenMode::ReadWrite);
            db->ChangeBriefcaseId(bcId);
            }

        _OnSetupDb(*db);

        return db;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocksManagerTest : RepositoryManagerTest
{
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

    DgnModelPtr CreateModel(Utf8CP name, DgnDbR db)
        {
        DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
        SpatialModelPtr model = new SpatialModel(SpatialModel::CreateParams(db, classId, DgnModel::CreateModelCode(name)));
        IBriefcaseManager::Request req;
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForModelInsert(req, *model, IBriefcaseManager::PrepareAction::Acquire));
        auto status = model->Insert();
        EXPECT_EQ(DgnDbStatus::Success, status);
        return DgnDbStatus::Success == status ? model : nullptr;
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
        DgnCategoryId catId = DgnCategory::QueryHighestCategoryId(db);
        return GenericPhysicalObject::Create(*model.ToSpatialModelP(), catId);
        }

    DgnElementPtr Create2dElement(DgnModelR model)
        {
        DgnDbR db = model.GetDgnDb();
        DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Annotation2d::GetHandler());
        return AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, model.GetModelId(), classId, DgnCategory::QueryHighestCategoryId(db)));
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
    BeBriefcaseId   m_bcId;
    DgnDbPtr        m_db;
    DgnModelId      m_modelId;
    DgnElementId    m_elemId;

    virtual void _OnSetupDb(DgnDbR db) override
        {
        m_db = &db;
        m_modelId = DgnModel::DictionaryId();
        m_elemId = DgnCategory::QueryFirstCategoryId(db);
        }

    SingleBriefcaseLocksTest() : m_bcId(2) { }
    ~SingleBriefcaseLocksTest() {if (m_db.IsValid()) m_db->CloseDb();}

    DgnModelPtr CreateModel(Utf8CP name) { return T_Super::CreateModel(name, *m_db); }
};

/*---------------------------------------------------------------------------------**//**
* A single briefcase can always acquire locks with no contention
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, AcquireLocks)
    {
    SetupDb(L"AcquireLocks.bim", m_bcId);

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

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());

    ExpectAcquire(el, LockLevel::Shared);
    ExpectLevel(el, LockLevel::Exclusive);  // shared lock automatically upgraded to exclusive for elements, currently
    ExpectLevel(model, LockLevel::Shared);
    ExpectLevel(db, LockLevel::Shared);

    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishLocks());
    
    ExpectAcquire(model, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Shared);

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

    // An exclusive db lock results in exclusive locks on all models and elements
    ExpectAcquire(db, LockLevel::Exclusive);
    ExpectLevel(model, LockLevel::Exclusive);
    ExpectLevel(el, LockLevel::Exclusive);
    ExpectLevel(db, LockLevel::Exclusive);
    
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
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, RelinquishLocks)
    {
    SetupDb(L"RelinquishLocks.bim", m_bcId);

    // Create a new element, implicitly locking the dictionary model + the db
    DgnDbR db = *m_db;
    auto txnPos = db.Txns().GetCurrentTxnId();
    DgnTrueColor color(DgnTrueColor::CreateParams(db, ColorDef(1,2,3), "la", "lala"));
    EXPECT_TRUE(color.Insert().IsValid());

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
    SetupDb(L"LocallyCreatedObjects.bim", m_bcId);

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
* Test that locks + codes acquired while connected are retained when disconnected.
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SingleBriefcaseLocksTest, DisconnectedWorkflow)
    {
    BeFileName filename;
    DgnModelId newModelId;
        {
        SetupDb(L"DisconnectedWorkflow.bim", m_bcId);

        DgnDbR db = *m_db;
        DgnModelPtr pModel = db.Models().GetModel(m_modelId);
        DgnElementCPtr cpEl = db.Elements().GetElement(m_elemId);

        // Acquire locks with server available
        EXPECT_TRUE(nullptr != T_HOST.GetRepositoryAdmin()._GetRepositoryManager(db));
        ExpectAcquire(*cpEl, LockLevel::Exclusive);
        ExpectLevel(db, LockLevel::Shared);
        ExpectLevel(*pModel, LockLevel::Shared);
        ExpectLevel(*cpEl, LockLevel::Exclusive);

        // Create a new model (implicitly exclusively locked)
        DgnModelPtr newModel = CreateModel("NewModel");
        newModelId = newModel->GetModelId();
        ExpectLevel(*newModel, LockLevel::Exclusive);

        // Close the db and unregister the server
        filename = m_db->GetFileName();
        pModel = nullptr;
        newModel = nullptr;
        cpEl = nullptr;
        m_db->SaveChanges();
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

    static WCharCP SeedFileName() { return L"ElementsSymbologyByLeveldgn.i.ibim"; }

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

    void SetupDbs(uint32_t baseBcId=2)
        {
        m_dbA = SetupDb(L"DbA.bim", BeBriefcaseId(baseBcId), SeedFileName());
        m_dbB = SetupDb(L"DbB.bim", BeBriefcaseId(baseBcId+1), SeedFileName());

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
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractLocksTest : SingleBriefcaseLocksTest
{
    DgnDbStatus ExtractLocks(LockRequestR req)
        {
        if (BE_SQLITE_OK != m_db->SaveChanges())
            return DgnDbStatus::WriteError;

        RevisionStatus revStat;
        DgnRevisionPtr rev = m_db->Revisions().StartCreateRevision(&revStat, DgnRevision::Include::Locks);
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

        req.FromRevision(*rev);
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
    m_db = SetupDb(L"UsedLocks.bim", m_bcId);

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
        DgnCode newCode = DgnCategory::CreateCategoryCode("RenamedCategory");
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

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesManagerTest : RepositoryManagerTest
{
    static DgnCode MakeCode(Utf8StringCR name, Utf8CP nameSpace = nullptr)
        {
        return nullptr != nameSpace ? DgnMaterial::CreateMaterialCode(nameSpace, name) : DgnCategory::CreateCategoryCode(name);
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

    static DgnDbStatus InsertStyle(Utf8CP name, DgnDbR db)
        {
        AnnotationTextStyle style(db);
        style.SetName(name);
        IBriefcaseManager::Request req;
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForElementInsert(req, style, IBriefcaseManager::PrepareAction::Acquire));
        DgnDbStatus status;
        style.DgnElement::Insert(&status);
        return status;
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
};

/*---------------------------------------------------------------------------------**//**
* Simulate pushing a revision to the server.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CodesManagerTest::CommitRevision(DgnDbR db)
    {
    Utf8String revId;
    DgnRevisionPtr rev;
    if (BE_SQLITE_OK != db.SaveChanges() || (rev = db.Revisions().StartCreateRevision(nullptr, DgnRevision::Include::Codes)).IsNull())
        return revId;

    if (RevisionStatus::Success == db.Revisions().FinishCreateRevision())
        {
        m_server.OnFinishRevision(*rev);
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
    DgnDbPtr pDb = SetupDb(L"ReserveQueryRelinquish.bim", BeBriefcaseId(2));
    DgnDbR db = *pDb;
    IBriefcaseManagerR mgr = db.BriefcaseManager();

    // Empty request
    DgnCodeSet req;
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());

    // Reserve single code
    DgnCode code = MakeCode("Palette", "Material");
    req.insert(code);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).Result());
    ExpectState(MakeReserved(code, db), db);

    // Relinquish all
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeAvailable(code), db);

    // Reserve 2 codes
    DgnCode code2 = MakeCode("Category");
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
* (If we don't explicitly reserve it prior to insert/update, an attempt will be made to
* reserve it for us).
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, AutoReserveCodes)
    {
    DgnDbPtr pDb = SetupDb(L"AutoReserveCodes.bim", BeBriefcaseId(2));
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
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, InsertStyle(existingStyleCode.GetValue().c_str(), db));

    // Updating an element and changing its code will reserve the new code if we haven't done so already
    auto pStyle = AnnotationTextStyle::Get(db, "MyStyle")->CreateCopy();
    pStyle->SetName("MyRenamedStyle");
    EXPECT_TRUE(pStyle->Update().IsValid());
    EXPECT_EQ(2, GetReservedCodes(db).size());
    ExpectState(MakeReserved(pStyle->GetCode(), db), db);
    pStyle = nullptr;

    // Attempting to change code to an already-used code will fail on update
    pStyle = AnnotationTextStyle::Get(db, "MyRenamedStyle")->CreateCopy();
    pStyle->SetName(existingStyleCode.GetValue().c_str());
    DgnDbStatus status;
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
    DgnDbPtr pDb = SetupDb(L"CodesInRevisions.bim", BeBriefcaseId(2));
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



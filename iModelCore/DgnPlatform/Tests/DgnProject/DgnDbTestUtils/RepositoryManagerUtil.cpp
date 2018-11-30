/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/DgnDbTestUtils/RepositoryManagerUtil.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnPlatform/RepositoryManagerUtil.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGN

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TestRepositoryManager::TestRepositoryManager()
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
DbResult TestRepositoryManager::CreateLocksTable()
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
DbResult TestRepositoryManager::CreateCodesTable()
    {
    return m_db.CreateTable(TABLE_Codes,
            CODE_CodeSpec " INTEGER NOT NULL,"
            CODE_Scope " TEXT NOT NULL,"
            CODE_Value " TEXT NOT NULL COLLATE NOCASE,"
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
TestRepositoryManager::Response TestRepositoryManager::_ProcessRequest(Request const& req, DgnDbR db, bool queryOnly)
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
RepositoryStatus TestRepositoryManager::_Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
    {
    auto stat = _DemoteLocks(locks, db);
    if (RepositoryStatus::Success == stat)
        stat = _ReleaseCodes(codes, db);

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus TestRepositoryManager::_Relinquish(Resources which, DgnDbR db)
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
RepositoryStatus TestRepositoryManager::_QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
    {
    auto stat = _QueryLockStates(lockStates, locks);
    if (RepositoryStatus::Success == stat)
        stat = _QueryCodeStates(codeStates, codes);

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus TestRepositoryManager::_QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
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
void TestRepositoryManager::GetUnavailableLocks(DgnLockSet& locks, BeBriefcaseId bcId)
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
void TestRepositoryManager::GetUnavailableCodes(DgnCodeSet& codes, BeBriefcaseId bcId)
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
bool TestRepositoryManager::AreLocksAvailable(LockRequestCR reqs, BeBriefcaseId bcId)
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
void TestRepositoryManager::GetDeniedLocks(DgnLockInfoSet& states, LockRequestCR reqs, BeBriefcaseId bcId)
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
int32_t TestRepositoryManager::QueryLockCount(BeBriefcaseId bc)
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
void TestRepositoryManager::_AcquireLocks(Response& response, LockRequestCR inputReqs, DgnDbR db, Options opts)
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
RepositoryStatus TestRepositoryManager::_RelinquishLocks(DgnDbR db)
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
RepositoryStatus TestRepositoryManager::_DemoteLocks(DgnLockSet const& locks, DgnDbR db)
    {
    Relinquish(locks, db);
    Reduce(locks, db);

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRepositoryManager::Relinquish(DgnLockSet const& locks, DgnDbR db)
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
void TestRepositoryManager::Reduce(DgnLockSet const& locks, DgnDbR db)
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
RepositoryStatus TestRepositoryManager::_QueryLocks(DgnLockSet& locks, DgnDbR db)
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
RepositoryStatus TestRepositoryManager::_QueryLockState(DgnLockInfoR state, LockableId inputLockId)
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
RepositoryStatus TestRepositoryManager::_QueryLockStates(DgnLockInfoSet& states, LockableIdSet const& ids)
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
RepositoryStatus TestRepositoryManager::ValidateRelease(DgnCodeInfoSet& discarded, Statement& stmt, BeBriefcaseId bcId)
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
void TestRepositoryManager::MarkDiscarded(DgnCodeInfoSet const& discarded)
    {
    Statement stmt;
    stmt.Prepare(m_db, "INSERT INTO " TABLE_Codes "(" CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_State "," CODE_Revision ") VALUES (?,?,?,2,?)");

    for (DgnCodeInfo const& info : discarded)
        {
        DgnCodeCR code = info.GetCode();
        stmt.BindId(1, code.GetCodeSpecId());
        stmt.BindText(2, code.GetScopeString(), Statement::MakeCopy::No);
        stmt.BindText(3, code.GetValueUtf8(), Statement::MakeCopy::No);
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
void TestRepositoryManager::_ReserveCodes(Response& response, DgnCodeSet const& req, DgnDbR db, Options opts, bool queryOnly)
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
        insert.BindText(3, code.GetValueUtf8(), Statement::MakeCopy::No);
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
RepositoryStatus TestRepositoryManager::ValidateRelease(DgnCodeInfoSet& discarded, DgnCodeSet const& req, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE InVirtualSet(@vset, " CODE_CodeSpec "," CODE_Scope "," CODE_Value ")");
    VirtualCodeSet vset(req);
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus TestRepositoryManager::_ReleaseCodes(DgnCodeSet const& req, DgnDbR db)
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
RepositoryStatus TestRepositoryManager::ValidateRelinquish(DgnCodeInfoSet& discarded, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE " CODE_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus TestRepositoryManager::_RelinquishCodes(DgnDbR db)
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
RepositoryStatus TestRepositoryManager::_QueryCodeStates(DgnCodeInfoSet& infos, DgnCodeSet const& codes)
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
RepositoryStatus TestRepositoryManager::_QueryCodes(DgnCodeSet& codes, DgnDbR db)
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
void TestRepositoryManager::MarkRevision(DgnCodeSet const& codes, bool discarded, Utf8StringCR revId)
    {
    if (codes.empty())
        return;

    Statement stmt;
    stmt.Prepare(m_db, "INSERT OR REPLACE INTO " TABLE_Codes "(" CODE_CodeSpec "," CODE_Scope "," CODE_Value "," CODE_State "," CODE_Revision ") VALUES (?,?,?,?,?)");
    for (DgnCodeCR code : codes)
        {
        stmt.BindId(1, code.GetCodeSpecId());
        stmt.BindText(2, code.GetScopeString(), Statement::MakeCopy::No);
        stmt.BindText(3, code.GetValueUtf8(), Statement::MakeCopy::No);
        stmt.BindInt(4, static_cast<int>(discarded ? CodeState::Discarded : CodeState::Used));
        stmt.BindText(5, revId, Statement::MakeCopy::No);

        stmt.Step();
        stmt.Reset();
        }
    }


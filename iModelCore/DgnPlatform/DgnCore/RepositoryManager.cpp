/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RepositoryManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnChangeSummary.h>

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct MasterBriefcaseManager : IBriefcaseManager
{
private:
    MasterBriefcaseManager(DgnDbR db) : IBriefcaseManager(db) { }

    virtual Response _ProcessRequest(Request&) override { return Response(RepositoryStatus::Success); }
    virtual RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _Relinquish(Resources) override { return RepositoryStatus::Success; }
    virtual DgnDbStatus _LockElement(DgnElementCR, DgnCodeCR, DgnModelId) override { return RepositoryStatus::Success; }
    virtual DgnDbStatus _LockModel(DgnModelCR, LockLevel, DgnCodeCR) override { return RepositoryStatus::Success; }
    virtual DgnDbStatus _LockDb(LockLevel, DgnCodeCR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _ReserveCode(DgnCodeCR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryLockLevel(LockLevel& level, LockableId lockId) override { level = LockLevel::Exclusive; return RepositoryStatus::Success; }
    virtual RepositoryStatus _OnFinishRevision(DgnRevision const&) override { return RepositoryStatus::Success; }
    virtual void _OnElementInserted(DgnElementId) override { }
    virtual void _OnModelInserted(DgnModelId) override { }
    virtual RepositoryStatus _QueryLockLevels(DgnLockSet& levels, LockableIdSet& lockIds) override
        {
        for (auto const& id : lockId)
            levels.insert(DgnLock(id, LockLevel::Exclusive));

        return RepositoryStatus::Success;
        }
    virtual bool _AreResourcesHeld(DgnLockSet&, DgnCodeSet&, RepositoryStatus* status) override 
        {
        if (nullptr != status)
            *status = RepositoryStatus::Success;
        return true;
        }
    virtual RepositoryStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) override
        {
        auto bcId = GetDgnDb().GetBriefcaseId();
        for (auto const& code : codes)
            states.insert(DgnCodeInfo(code)).first->SetReserved(bcId);
        return RepositoryStatus::Success;
        }

public:
    static IBriefcaseManagerPtr Create(DgnDbR db) { return new MasterBriefcaseManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct BriefcaseManager : IBriefcaseManager
{
private:
    enum class DbState { New, Ready, Invalid };

    DbState m_dbState;

    virtual Response _ProcessRequest(Request&) override;
    virtual RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override;
    virtual RepositoryStatus _Relinquish(Resources which) override;
    virtual bool _AreResourcesHeld(DgnLockSet&, DgnCodeSet&, RepositoryStatus*) override;
    virtual RepositoryStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&) override;
    virtual RepositoryStatus _QueryLockLevel(LockLevel&, LockableId) override;
    virtual RepositoryStatus _QueryLockLevels(DgnLockSet&, LockableIdSet&) override;
    virtual RepositoryStatus _OnFinishRevision(DgnRevision const&) override;
    virtual void _OnElementInserted(DgnElementId) override;
    virtual void _OnModelInserted(DgnModelId) override;

    BriefcaseManager(DgnDbR db) : IBriefcaseManager(db), m_dbState(DbState::New) { }

    DbR GetLocalDb() { return GetDgnDb().GetLocalStateDb().GetDb(); }
    bool Validate(RepositoryStatus* status=nullptr);
    RepositoryStatus Refresh();
    RepositoryStatus Pull();
    DbResult Save() { return GetLocalDb().SaveChanges(); }
    void Cull(Request& req) { Cull(req.Codes()); Cull(req.Locks().GetLockSet()); }
    void Insert(Request const& req) { Insert(req.Codes()); Insert(req.Locks()); }

    // Codes...
    void Insert(DgnCodeSet const& codes);
    void Cull(DgnCodeSet& codes);
    RepositoryStatus Remove(DgnCodeSet const& codes);

    // Locks...
    void Insert(LockableId id, LockLevel level, bool overwrite=false);
    template<typename T> void Insert(T const& locks, bool checkExisting);
    void AddDependentElements(DgnLockSet& locks, bvector<DgnModelId> const& models);
    RepositoryStatus PromoteDependentElements(LockRequestCR usedLocks, bvector<DgnModelId> const& models);
    void Cull(DgnLockSet& locks);
    RepositoryStatus AcquireLocks(LockRequestR locks, bool cull);
public:
    static IBriefcaseManagerPtr Create(DgnDbR db) { return new BriefcaseManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_enableBriefcaseManagement = false;
void IBriefcaseManager::BackDoor_SetEnabled(bool enable) { s_enableBriefcaseManagement = enable; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManagerPtr DgnPlatformLib::Host::RepositoryAdmin::_CreateBriefcaseManager(DgnDbR db) const
    {
    return (db.IsMasterCopy() || !s_enableBriefcaseManagement) ? MasterBriefcaseManager::Create(db) : BriefcaseManager::Create(db);
    }

#define TABLE_Codes "Codes"
#define CODE_AuthorityId "AuthorityId"
#define CODE_NameSpace "NameSpace"
#define CODE_Value "CodeValue"
#define CODE_Columns CODE_AuthorityId "," CODE_NameSpace "," CODE_Value
#define CODE_Values "(" CODE_Columns ")"
#define STMT_InsertCode "INSERT INTO " TABLE_Codes " " CODE_Values " Values (?,?,?)"
#define STMT_SelectCodesInSet "SELECT " CODE_Columns " FROM " TABLE_Codes " WHERE InVirtualSet(@vset," CODE_Columns ")"
#define STMT_DeleteCodesInSet "DELETE FROM " TABLE_COdes " WHERE InVirtualSet(@vset," CODE_Columns ")"

enum CodeColumn { AuthorityId=0, NameSpace, Value };

#define TABLE_Locks "Locks"
#define LOCK_Type "Type"
#define LOCK_Id "Id"
#define LOCK_Level "Level"
#define LOCK_Columns LOCK_Type "," LOCK_Id "," LOCK_Level
#define LOCK_Values "(" LOCK_Columns ")"
#define STMT_SelectExistingLock "SELECT " LOCK_Level ",rowid FROM " TABLE_Locks " WHERE " LOCK_Type "=? AND " LOCK_Id "=?"
#define STMT_InsertNewLock "INSERT INTO " TABLE_Locks " " LOCK_Values " VALUES (?,?,?)"
#define STMT_UpdateLockLevel "UPDATE " TABLE_Locks " SET " LOCK_Level "=? WHERE rowid=?"
#define STMT_SelectLocksInSet "SELECT " LOCK_Type "," LOCK_Id " FROM " TABLE_Locks " WHERE InVirtualSet(@vset," LOCK_Columns ")"
#define STMT_SelectElementInModel " SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?"
#define STMT_InsertOrReplaceLock "INSERT OR REPLACE INTO " TABLE_Locks " " LOCK_Values " VALUES(?,?,?)"
#define STMT_SelectElemsInModels "SELECT " LOCK_Id " FROM " TABLE_Locks " WHERE " LOCK_Type "=2 AND InVirtualSet(@vset," LOCK_Id ")"
#define STMT_SelectLevelInSet "SELECT " LOCK_Columns " FROM " TABLE_Locks " WHERE InVirtualSet(@vset, " LOCK_Type "," LOCK_Id ")"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManager::Validate(RepositoryStatus* pStatus)
    {
    RepositoryStatus localStatus;
    RepositoryStatus& status = nullptr != pStatus ? *pStatus : localStatus;

    switch (m_dbState)
        {
        case DbState::Ready:
            status = RepositoryStatus::Success;
            return true;
        case DbState::Invalid:
            status = nullptr != GetRepositoryManager() ? RepositoryStatus::SyncError : RepositoryStatus::ServerUnavailable;
            return false;
        }

    m_dbState = DbState::Invalid;
    status = RepositoryStatus::SyncError;

    // Set up the local Db tables
    DgnDb::LocalStateDb& localState = GetDgnDb().GetLocalStateDb();
    if (!localState.IsValid())
        return false;

    DbResult result = localState.GetDb().CreateTable(TABLE_Codes,
                                                     CODE_AuthorityId " INTEGER,"
                                                     CODE_NameSpace " TEXT,"
                                                     CODE_Value " TEXT,"
                                                     "PRIMARY KEY" CODE_Values);
    if (BE_SQLITE_OK == result)
        {
        result = localState.GetDb().CreateTable(TABLE_Locks,
                                                LOCK_Type " INTEGER,"
                                                LOCK_Id " INTEGER,"
                                                LOCK_Level " INTEGER,"
                                                "PRIMARY KEY(" LOCK_Type "," LOCK_Id ")");
        }

    if (BE_SQLITE_OK != result)
        return false;

    status = Pull();
    return RepositoryStatus::Success == status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::Pull()
    {
    // Populate local Db with reserved codes and locks from the server
    DgnLockSet locks;
    DgnCodeSet codes;
    auto server = GetRepositoryManager();
    status = nullptr != server ? server->QueryHeldResources(locks, codes, GetDgnDb()) : RepositoryStatus::ServerUnavailable;
    if (RepositoryStatus::Success != status)
        return status;

    if (!locks.empty())
        Insert(locks, false);

    if (!codes.empty())
        Insert(codes);

    m_dbState = DbState::Ready;
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::Refresh()
    {
    if (DbState::Ready != m_dbState)
        {
        // Either we haven't yet initialized the localDB, or failed to do so previously. Retry that.
        RepositoryStatus stat;
        Validate(&stat);
        return stat;
        }

    // Empty out our local DB tables and re-populate from server
    m_dbState = DbState::Invalid; // assume something will go wrong...
    if (BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Locks) || BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Codes))
        return RepositoryStatus::SyncError;

    return Pull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::Insert(DgnCodeSet const& codes)
    {
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_InsertCode);
    for (auto const& code : codes)
        {
        if (code.IsEmpty() || !code.IsValid())
            {
            BeAssert(false);
            continue;
            }

        stmt->BindId(CodeColumn::AuthorityId+1, code.GetAuthority());
        stmt->BindText(CodeColumn::NameSpace+1, code.GetNamespace(), Statement::MakeCopy::No);
        stmt->BindText(CodeColumn::Value+1, code.GetValue(), Statement::MakeCopy::No);
        stmt->Step();
        }

    Save();
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
            return arg.GetAuthority().GetValueUnchecked() == vals[CodeColumn::AuthorityId].GetValueUInt64()
                && arg.GetNamespace().Equals(vals[CodeColumn::NameSpace].GetValueText())
                && arg.GetValue().Equals(vals[CodeColumn::Value].GetValueText());
            });
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::Cull(DgnCodeSet& codes)
    {
    // Don't bother asking server to reserve codes which we've already reserved...
    VirtualCodeSet vset(codes);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectCodesInSet);
    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        DgnCode code(stmt->GetValueId<DgnAuthorityId>(CodeColumn::AuthorityId), stmt->GetValueText(CodeColumn::NameSpace), stmt->GetValueText(CodeColumn::Value));
        codes.erase(code);
        }

    // Don't bother asking server to reserve empty codes...
    for (auto iter = codes.begin(); iter != codes.end(); /* */)
        {
        BeAssert(!iter->IsEmpty());
        BeAssert(iter->IsValid());
        if (iter->IsEmpty() || !iter->IsValid())
            iter = codes.erase(iter);
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::Remove(DgnCodeSet const& codes)
    {
    VirtualCodeSet vset(codes);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_DeleteCodesInSet);
    stmt->BindVirtualSet(1, vset);
    if (BE_SQLITE_OK != stmt->Step())
        return RepositoryStatus::SyncError;

    Save();
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void bindEnum(Statement& stmt, int32_t index, T val)
    {
    stmt.BindInt(index, static_cast<int32_t>(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::Insert(LockableId id, LockLevel level, bool overwrite)
    {
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(overwrite ? STMT_InsertOrReplaceLock : STMT_InsertNewLock);
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
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void BriefcaseManager::Insert(T const& locks, bool checkExisting)
    {
    CachedStatementPtr select = checkExisting ? GetLocalDb().GetCachedStatement(STMT_SelectExistingLock) : nullptr,
                       insert = GetLocalDb().GetCachedStatement(STMT_InsertNewLock),
                       update = checkExisting ? GetLocalDb().GetCachedStatement(STMT_UpdateLockLevel) : nullptr;

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
                if (static_cast<LockLevel>(select->GetValueInt(0)) < lock.GetLevel())
                    {
                    update->BindInt(1, static_cast<int32_t>(lock.GetLevel())); // new lock level
                    update->BindInt(2, select->GetValueInt(1)); // existing row id
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
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::AddDependentElements(DgnLockSet& locks, bvector<DgnModelId> const& models)
    {
    if (models.empty())
        return;

    struct VSet : VirtualSet
    {
        bvector<DgnModelId> const& m_models;
        DgnDbR m_db;

        VSet(bvector<DgnModelId> const& models, DgnDbR db) : m_models(models), m_db(db) { }

        virtual bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            BeAssert(1 == nVals);
            auto el = m_db.Elements().GetElement(DgnElementId(vals[0].GetValueUInt64()));
            return el.IsValid() && std::find(m_models.begin(), m_models.end(), el->GetModelId());
            }
    };

    VSet vset(models, GetDgnDb());
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectElemsInModels);
    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        locks.insert(DgnLock(LockableId(stmt->GetValueId<DgnElementId>(0)), LockLevel::None));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::PromoteDependentElements(LockRequestCR usedLocks, bvector<DgnModelId> const& models)
    {
    if (models.empty())
        return RepositoryStatus::Success;

    LockRequest elemRequest;
    for (auto const& usedLock : usedLocks)
        {
        if (LockableType::Element != usedLock.GetType() || LockLevel::Exclusive != usedLock.GetLevel())
            continue;

        DgnElementCPtr elem = GetDgnDb().Elements().GetElement(DgnElementId(usedLock.GetId().GetValue()));
        if (elem.IsValid() && models.end() != std::find(models.begin(), models.end(), elem->GetModelId()))
            elemRequest.Insert(*elem, LockLevel::Exclusive);
        }

    // NB: Do not cull - we still hold exclusive lock on model which implies exclusive lock on all elems within it. Want to explicitly acquire those locks.
    return AcquireLocks(elemRequest, false).GetStatus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::AcquireLocks(LockRequestR locks, bool cull)
    {
    if (!Validate())
        return RepositoryStatus::SyncError;

    if (cull)
        Cull(locks.GetLockSet());

    if (locks.IsEmpty())
        return RepositoryStatus::Success;

    auto server = GetRepositoryManager();
    if (nullptr == server)
        return RepositoryStatus::ServerUnavailable;

    std::swap(locks, req.Locks());
    auto result = server->ProcessRequest(req, GetDgnDb()).Result();
    std::swap(locks, req.Locks());

    if (RepositoryStatus::Success == result)
        {
        Insert(locks, true);
        Save();
        }

    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool lockSetContains(DgnLockSet const& locks, DgnLockCR lock, bool matchExactLevel=false)
    {
    auto iter = locks.find(DgnLock(lock.GetLockableId(), LockLevel::Exclusive));
    if (locks.end() == iter)
        return false;
    else if (matchExactLevel && iter->GetLevel() != lock.GetLevel())
        return false;
    else if (iter->GetLevel() > lock.GetLevel())
        return false;
    else
        return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::Cull(DgnLockSet& locks)
    {
    struct VSet : VirtualSet
    {
        DgnLockSet const& m_locks;
        VSet(DgnLockSet const& locks) : m_locks(locks) { }
        virtual bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            BeAssert(3 == nVals);
            LockableId id(static_cast<LockableType>(vals[0].GetValueInt()), BeInt64Id(vals[1].GetValueUInt64()));
            return lockSetContains(m_locks, DgnLock(id, static_cast<LockLevel>(vals[2].GetValueInt())));
            }
    };

    VSet vset(locks);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectLocksInSet);
    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        LockableId id(static_cast<LockableType>(stmt->GetValueInt(0)), BeInt64Id(stmt->GetValueUInt64(1)));
        locks.Remove(id);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response BriefcaseManager::_ProcessRequest(Request& req)
    {
    RepositoryStatus stat;
    if (!Validate(&stat))
        return Response(stat);

    Cull(req);
    if (req.IsEmpty())
        return Response(RepositoryStatus::Success);

    auto mgr = GetRepositoryManager();
    if (nullptr == mgr)
        return Response(RepositoryStatus::ServerUnavailable);

    auto response = mgr->ProcessRequest(req, *this);
    if (RepositoryStatus::Success == response.Result())
        {
        Insert(req);
        Save();
        }

    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReleaseContext
{
private:
    DgnDbR              m_db;
    RepositoryStatus    m_status;
    DgnCodeSet          m_usedCodes;
    LockRequest         m_usedLocks;
    TxnManager::TxnId   m_endTxnId;
public:
    explicit ReleaseContext(DgnDbR db, bool wantLocks, bool wantCodes);

    RepositoryStatus GetStatus() const { return m_status; }
    DgnCodeSet const& GetUsedCodes() const { return m_usedCodes; }
    LockRequestR GetUsedLocks() const { return m_usedLocks; }

    RepositoryStatus ClearTxns();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ReleaseContext::ReleaseContext(DgnDbR db, bool wantLocks, bool wantCodes) : m_db(db), m_status(RepositoryStatus::CannotCreateRevision)
    {
    TxnManager& txns = db.Txns();
    if (txns.HasChanges() || txns.IsInDynamics())
        {
        m_status = RepositoryStatus::PendingTransactions;
        return;
        }

    BeAssert((wantLocks || wantCodes) && "Waste of time attempting to relinquish/release nothing...");
    auto include = wantCodes ? DgnRevision::Include::Codes : DgnRevision::Include::None;
    if (wantLocks)
        include = include | DgnRevision::Include::Locks;

    RevisionStatus revStatus;
    DgnRevisionPtr rev = db.Revisions().StartCreateRevision(&revStatus, include);
    if (rev.IsValid())
        {
        m_usedLocks.FromRevision(*rev);
        rev->ExtractAssignedCodes(m_usedCodes);
        m_status = RepositoryStatus::Success;
        m_endTxnId = db.Revisions().GetCurrentRevisionEndTxnId();

        db.Revisions().AbandonCreateRevision();
        }
    else if (RevisionStatus::NoTransactions == revStatus)
        {
        // No changes => no locks or codes have been used
        m_status = RepositoryStatus::Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ReleaseContext::ClearTxns()
    {
    // NEEDSWORK: We need a way to persistently record that undo is not permitted beyond this point.
    // For now, disallow redo.
    if (RepositoryStatus::Success == m_status) // && m_endTxnId.IsValid())
        m_db.Txns().DeleteReversedTxns();

    return m_status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::_Relinquish(Resources which)
    {
    IRepositoryManagerP server;
    RepositoryStatus stat;
    if (!Validate(&stat))
        return stat;
    else if (nullptr == (server = GetRepositoryManager()))
        return RepositoryStatus::ServerUnavailable;

    bool wantLocks = Resources::Locks == (which & Resources::Locks),
         wantCodes = Resources::Codes == (which & Resources::Codes);

    ReleaseContext context(GetDgnDb(), wantLocks, wantCodes);
    if (RepositoryStatus::Success != context.GetStatus())
        return context.GetStatus();
    else if (!context.GetUsedLocks().IsEmpty())
        return RepositoryStatus::LockUsed;
    else if (!context.GetUsedCodes().IsEmpty())
        return RepositoryStatus::CodeUsed;

    DbResult result = wantLocks ? GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Locks) : BE_SQLITE_OK;
    if (BE_SQLITE_OK == result && wantCodes)
        result = GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Codes);

    if (BE_SQLITE_OK != result || BE_SQLITE_OK != Save())
        {
        BeAssert(false);
        m_dbState = DbState::Invalid;
        return RepositoryStatus::SyncError;
        }

    stat = server>Relinquish(which);
    if (RepositoryStatus::Success == stat)
        stat = context.ClearTxns();

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::_Demote(DgnLockSet& locks, DgnCodeSet const& codes)
    {
    // Releasing the db itself is equivalent to releasing all held locks...
    if (locks.end() != locks.find(DgnLock(LockableId(GetDgnDb()), LockLevel::None)))
        {
        auto stat = _Relinquish(Resources::Locks);
        if (RepositoryStatus::Success == stat && !codes.empty())
            {
            // NB: In this special, hypothetical case we're splitting into two operations...
            locks.clear();
            stat = _Demote(locks, codes);
            }

        return stat;
        }

    IRepositoryManagerP mgr;
    RepositoryStatus stat;
    if (!Validate(&stat))
        return stat;
    else if (nullptr == (mgr = GetRepositoryManager()))
        return RepositoryStatus::ServerUnavailable;

    // Cull any locks which are already at or below the desired level (this function cannot *increase* a lock's level...)
    for (auto iter = locks.begin(); iter != locks.end(); /**/)
        {
        DgnLock& lock = *iter;
        LockLevel curLevel;
        stat = QueryLockLevel(curLevel, lock.GetLockableId());
        if (RepositoryStatus::Success != stat)
            return stat;
        else if (curLevel <= lock.GetLevel())
            iter = locks.erase(iter);
        else
            ++iter
        }

    bool wantLocks = !locks.empty(),
         wantCodes = !codes.empty();

    if (!wantLocks && !wantCodes)
        return RepositoryStatus::Success;

    // Cannot release locks required for local changes, or codes assigned by local changes
    ReleaseContext context(GetDgnDb(), wantLocks, wantCodes);
    stat = context.GetStatus();
    if (RepositoryStatus::Success != stat)
        return stat;

    for (auto const& usedCode : context.GetUsedCodes())
        {
        auto iter = codes.find(usedCode);
        if (iter != codes.end())
            return RepositoryStatus::CodeUsed;
        }

    for (auto const& usedLock : context.GetUsedLocks())
        {
        BeAssert(usedLock.GetLevel() != LockLevel::None);
        auto iter = locks.find(usedLock);
        if (iter != locks.end())
            {
            BeAssert(iter->GetLevel() != LockLevel::Exclusive);
            if (usedLock.GetLevel() > iter->GetLevel())
                return RepositoryStatus::LockUsed;
            }
        }

    // Must release any dependent locks held as well
    bvector<DgnModelId> releasedModels; // any models we are relinquishing (LockLevel::None)
    bvector<DgnModelId> demotedModels; // any models we are demoting from Exclusive to Shared
    for (auto const& lock : locks)
        {
        if (LockableType::Model == lock.GetType())
            {
            bvector<DgnModelId>* modelList = nullptr;
            switch (lock.GetLevel())
                {
                case LockLevel::None:   modelList = &releasedModels; break;
                case LockLevel::Shared: modelList = &demotedModels; break;
                }

            if (nullptr != modelList)
                modelList->push_back(DgnModelId(lock.GetId().GetValue()));
            }
        }

    // If we're demoting exclusive locks on models, must acquire exclusive locks on any elements we've modified within them
    stat = PromoteDependentElements(context.GetUsedLocks(), demotedModels);
    if (RepositoryStatus::Success != stat)
        return stat;

    // If we're releasing model locks, also release locks on any elements within them
    AddDependentElements(locks, releasedModels);

    stat = mgr->Demote(locks, codes, GetDgnDb());
    if (RepositoryStatus::Success == stat)
        {
        stat = Refresh();
        context.ClearTxns();
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManager::_AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* pStatus)
    {
    if (!Validate(pStatus))
        return false;

    if (nullptr != pStatus)
        *pStatus = RepositoryStatus::Success;

    Cull(locks);
    Cull(codes);

    return locks.empty() && codes.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::_QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes)
    {
    auto server = GetRepositoryManager();
    return nullptr != server ? server->QueryCodeStates(states, codes) : RepositoryStatus::ServerUnavailable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::_QueryLockLevel(LockLevel& level, LockableId lockId)
    {
    DgnLockSet levels;
    LockableIdSet ids;
    ids.insert(lockId);
    auto stat = QueryLockLevels(levels, ids);
    BeAssert(RepositoryStatus::Success != stat || 1 == levels.size());
    level = RepositoryStatus::Success == stat && 1 == levels.size() ? levels.begin()->GetLevel() : LockLevel::None;
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::_QueryLockLevels(DgnLockSet& levels, LockableIdSet& lockIds)
    {
    // Our local DB was populated from the server...there's no need to contact server again
    RepositoryStatus stat;
    if (!Validate(&stat))
        return stat;

    struct VSet : VirtualSet
    {
        LockableIdSet const& m_ids;
        VSet(LockableIdSet const& ids) : m_ids(ids) { }
        virtual bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            LockableId id(static_cast<LockableType>(vals[0].GetValueInt()), BeInt64Id(vals[1].GetValueUInt64()));
            return m_ids.end() != m_ids.find(id);
            }
    };

    VSet vset(ids);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectLevelInSet);
    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        LockableId id(static_cast<LockableType>(stmt->GetValueInt(0)), BeInt64Id(stmt->GetValueUInt64(1)));
        ids.erase(ids.find(id));
        levels.insert(DgnLock(id, static_cast<LockLevel>(stmt->GetValueInt(2))));
        }

    // Any not found are not locked by us
    for (auto const& id : ids)
        levels.insert(DgnLock(id, LockLevel::None));

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::_OnFinishRevision(DgnRevision const& rev)
    {
    // Any codes which became Used as a result of these changes must necessarily have been Reserved by this briefcase,
    // and are now no longer Reserved by any briefcase
    // (Any codes which became Discarded were necessarily previously Used, therefore no local state needs to be updated for them).
    if (rev.GetAssignedCodes().empty())
        return RepositoryStatus::Success;
    else if (!Validate())
        return RepositoryStatus::SyncError;
    else
        return Remove(rev.GetAssignedCodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::_OnElementInserted(DgnElementId id)
    {
    if (Validate())
        {
        Insert(LockableId(id), LockLevel::Exclusive);
        Save();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::_OnModelInserted(DgnModelId id)
    {
    if (Validate())
        {
        Insert(LockableId(id), LockLevel::Exclusive);
        Save();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::OnElementInsert(DgnElementCR el) { return _LockModel(*el.GetModel(), LockLevel::Shared, el.GetCode()); }
DgnDbStatus IBriefcaseManager::OnElementUpdate(DgnElementCR el, DgnModelId originalModelId) { return LockElement(el, el.GetCode(), originalModelId); }
DgnDbStatus IBriefcaseManager::OnElementDelete(DgnElementCR el) { return LockElement(el, DgnCode()); }
DgnDbStatus IBriefcaseManager::OnModelInsert(DgnModelCR model) { return _LockDb(LockLevel::Shared, model.GetCode()); }
DgnDbStatus IBriefcaseManager::OnModelUpdate(DgnModelCR model) { return _LockModel(model, LockLevel::Exclusive, model.GetCode()); }
DgnDbStatus IBriefcaseManager::OnModelDelete(DgnModelCR model) { return _LockModel(model, LockLevel::Exclusive, DgnCode()); }
void IBriefcaseManager::OnElementInserted(DgnElementId id) { _OnElementInserted(id); }
void IBriefcaseManager::OnModelInserted(DgnModelId id) { _OnModelInserted(id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static DgnDbStatus processRequest(T const& obj, LockLevel level, DgnCodeCR code, IBriefcaseManagerR mgr, DgnModelId originalModelId=DgnModelId())
    {
    IBriefcaseManager::Request reql
    req.Locks().Insert(obj, level);
    bool wantCode = code.IsValid() && !code.IsEmpty();
    if (wantCode)
        {
        req.SetOptions(IBriefcaseManager::ResponseOptions::DeniedLocks);
        req.Codes().insert(code);
        }

    if (originalModelId.IsValid())
        {
        // An element update operation which moved the element from one model to another...
        DgnModelPtr originalModel = mgr.GetDgnDb().Models().GetModel(originalModelId);
        if (originalModel.IsValid())
            req.Locks().Insert(*originalModel, LockLevel::Shared);
        }

    auto response = mgr.ProcessRequest(req);
    if (RepositoryStatus::Success == response.Result())
        return DgnDbStatus::Success;
    else if (!wantCode)
        return DgnDbStatus::LockNotHeld;

    switch (response.Result())
        {
        case RepositoryStatus::LockAlreadyHeld:
        case RepositoryStatus::LockUsed:
            return DgnDbStatus::LockNotHeld;
        case RepositoryStatus::CodeUnavailable:
        case RepositoryStatus::CodeNotReserved:
        case RepositoryStatus::CodeUsed:
            return DgnDbStatus::CodeNotReserved;
        default:
            return response.DeniedLocks().empty() ? DgnDbStatus::CodeNotReserved : DgnDbStatus::LockNotHeld;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::_LockElement(DgnElementCR el, DgnCodeCR code, DgnModelId originalModelId)
    {
    if (originalModelId.IsValid() && originalModelId == el.GetModelId())
        originalModelId = DgnModelId();

    return processRequest(el, LockLevel::Exclusive, code, *this, originalModelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::_LockModel(DgnModelCR model, LockLevel level, DgnCodeCR code)
    {
    return processRequest(model, level, code, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::_LockDb(LockLevel level, DgnCodeCR code)
    {
    return processRequest(GetDgnDb(), level, code, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::_ReserveCode(DgnCodeCR code)
    {
    DgnCodeSet codes;
    codes.insert(code);
    return ReserveCodes(codes).GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP IBriefcaseManager::GetRepositoryManager() const
    {
    return T_HOST.GetRepositoryAdmin()._GetRepositoryManager(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response IBriefcaseManager::AcquireLocks(LockRequestR locks, ResponseOptions options)
    {
    Request req(options);
    std::swap(locks, req.Locks());
    return ProcessRequest(req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response IBriefcaseManager::ReserveCodes(DgnCodeSet& codes, ResponseOptions options)
    {
    Request req(options);
    std::swap(codes, req.Codes());
    return ProcessRequest(req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IRepositoryManager::QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes)
    {
    DgnOwnedLockSet unused;
    return QueryStates(unused, states, LockableIdSet(), codes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IRepositoryManager::QueryLockOwnerships(DgnOwnedLockSet& ownerships, LockableIdSet const& locks)
    {
    DgnCodeInfoSet unused;
    return QueryStates(ownerships, unused, locks, DgnCodeSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::ReformulateRequest(Request& req, Response const& response) const
    {
#ifdef TODO_REPOSITORY_MANAGER
    DgnLockSet& locks = req.GetLockSet();
    for (auto const& lock : denied)
        {
        auto found = locks.find(DgnLock(lock.GetLockableId(), LockLevel::Exclusive));
        if (locks.end() != found)
            {
            if (LockLevel::Exclusive == lock.GetLevel())
                {
                switch (lock.GetType())
                    {
                    case LockableType::Db:
                        // The entire Db is locked. We can do nothing.
                        req.Clear();
                        return;
                    case LockableType::Model:
                        locks.erase(found);
                        RemoveElements(req, DgnModelId(lock.GetId().GetValue()));
                        break;
                    default:
                        locks.erase(found);
                        break;
                    }
                }
            else if (LockLevel::Shared == lock.GetLevel())
                {
                // Shared lock should not have been denied if no one holds an exclusive lock on it...
                // Note that if we requested an exclusive lock, we will now downgrade it to a shared lock
                BeAssert(LockLevel::Exclusive == found->GetLevel());
                switch (lock.GetType())
                    {
                    case LockableType::Db:
                    case LockableType::Model:
                        found->SetLevel(LockLevel::Shared);
                        break;
                    default:
                        locks.erase(found);
                        break;
                    }
                }
            }
        }
#endif
    }


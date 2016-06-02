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
    virtual RepositoryStatus _ReserveCode(DgnCodeCR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryLockLevel(LockLevel& level, LockableId lockId) override { level = LockLevel::Exclusive; return RepositoryStatus::Success; }
    virtual RepositoryStatus _OnFinishRevision(DgnRevision const&) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _RefreshFromRepository() override { return RepositoryStatus::Success; }
    virtual void _OnElementInserted(DgnElementId) override { }
    virtual void _OnModelInserted(DgnModelId) override { }
    virtual RepositoryStatus _QueryLockLevels(DgnLockSet& levels, LockableIdSet& lockIds) override
        {
        for (auto const& id : lockIds)
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
    virtual RepositoryStatus _PrepareForElementOperation(Request&, DgnElementCR, BeSQLite::DbOpcode, DgnElementCP) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _PrepareForModelOperation(Request&, DgnModelCR, BeSQLite::DbOpcode) override { return RepositoryStatus::Success; }
public:
    static IBriefcaseManagerPtr Create(DgnDbR db) { return new MasterBriefcaseManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* Maintains a local state Db containing:
*   - Locks held by this briefcase
*   - Codes reserved by this briefcase
*   - Locks held by other briefcases - strictly to be used for queries which
*     need to be fast (avoid contacting server) and can tolerate potentially becoming
*     out of date with server state. e.g., for tools which want to reject elements which
*     are believed to be locked by another briefcase.
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct BriefcaseManager : IBriefcaseManager, TxnMonitor
{
private:
    enum class DbState { New, Ready, Invalid };

    Db      m_localDb;
    DbState m_localDbState;

    virtual Response _ProcessRequest(Request&) override;
    virtual RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override;
    virtual RepositoryStatus _Relinquish(Resources which) override;
    virtual bool _AreResourcesHeld(DgnLockSet&, DgnCodeSet&, RepositoryStatus*) override;
    virtual RepositoryStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&) override;
    virtual RepositoryStatus _QueryLockLevel(LockLevel&, LockableId) override;
    virtual RepositoryStatus _QueryLockLevels(DgnLockSet&, LockableIdSet&) override;
    virtual RepositoryStatus _OnFinishRevision(DgnRevision const&) override;
    virtual RepositoryStatus _RefreshFromRepository() override { return Refresh(); }
    virtual void _OnElementInserted(DgnElementId) override;
    virtual void _OnModelInserted(DgnModelId) override;
    virtual void _OnDgnDbDestroyed() override;

    void Save(TxnManager& mgr) { if (&mgr.GetDgnDb() == &GetDgnDb()) Save(); }
    virtual void _OnCommit(TxnManager& mgr) override { Save(mgr); }
    virtual void _OnReversedChanges(TxnManager& mgr) override { Save(mgr); }
    virtual void _OnUndoRedo(TxnManager& mgr, TxnAction) override { Save(mgr); }
    virtual RepositoryStatus _PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode op, DgnElementCP orig) override
        {
        return el.PopulateRequest(req, op, orig);
        }
    virtual RepositoryStatus _PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode op) override
        {
        return model.PopulateRequest(req, op);
        }

    BriefcaseManager(DgnDbR db) : IBriefcaseManager(db), m_localDbState(DbState::New)
        {
        T_HOST.GetTxnAdmin().AddTxnMonitor(*this);
        }
    ~BriefcaseManager()
        {
        Save();
        T_HOST.GetTxnAdmin().DropTxnMonitor(*this);
        }

    DbR GetLocalDb();
    bool Validate(RepositoryStatus* status=nullptr);
    RepositoryStatus Initialize();
    bool UseExistingLocalDb(BeFileNameCR filename);
    bool InitializeLocalDb();
    bool CreateLocksTable(Utf8CP tableName);
    RepositoryStatus Refresh();
    RepositoryStatus Pull();
    DbResult Save()
        {
        switch (m_localDbState)
            {
            case DbState::New:      return BE_SQLITE_OK;
            case DbState::Ready:    return GetLocalDb().SaveChanges();
            default:                return BE_SQLITE_ERROR;
            }
        }

    void Cull(Request& req) { Cull(req.Codes()); Cull(req.Locks().GetLockSet()); }

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

    BeFileName GetLocalDbFileName() const
        {
        BeFileName filename = GetDgnDb().GetFileName();
        filename.AppendExtension(L"local");
        return filename;
        }

    static PropSpec GetCreationDatePropSpec() { return PropSpec("BriefcaseLocalStateDbCreationDate"); }
    static PropSpec GetVersionPropSpec() { return PropSpec("BriefcaseLocalStateDbVersion"); }
    static Utf8CP GetCurrentVersion() { return "1"; }
public:
    static IBriefcaseManagerPtr Create(DgnDbR db) { return new BriefcaseManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbR BriefcaseManager::GetLocalDb()
    {
    Validate();
    return m_localDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::_OnDgnDbDestroyed()
    {
    if (DbState::New == m_localDbState)
        return;

    m_localDb.CloseDb();

    // NB: Keep a valid local db around for potential offline workflows...
    if (DbState::Invalid == m_localDbState)
        {
        BeFileName filename = GetLocalDbFileName();
        filename.BeDeleteFile();
        }

    m_localDbState = DbState::New;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManagerPtr DgnPlatformLib::Host::RepositoryAdmin::_CreateBriefcaseManager(DgnDbR db) const
    {
    return db.IsMasterCopy() || db.IsStandaloneBriefcase() ? MasterBriefcaseManager::Create(db) : BriefcaseManager::Create(db);
    }

#define TABLE_Codes "Codes"
#define CODE_AuthorityId "AuthorityId"
#define CODE_NameSpace "NameSpace"
#define CODE_Value "CodeValue"
#define CODE_Columns CODE_AuthorityId "," CODE_NameSpace "," CODE_Value
#define CODE_Values "(" CODE_Columns ")"
#define STMT_InsertCode "INSERT INTO " TABLE_Codes " " CODE_Values " Values (?,?,?)"
#define STMT_SelectCodesInSet "SELECT " CODE_Columns " FROM " TABLE_Codes " WHERE InVirtualSet(@vset," CODE_Columns ")"
#define STMT_DeleteCodesInSet "DELETE FROM " TABLE_Codes " WHERE InVirtualSet(@vset," CODE_Columns ")"

enum CodeColumn { AuthorityId=0, NameSpace, Value };

#define TABLE_Locks "Locks"
#define TABLE_UnavailableLocks "UnavailableLocks"
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
bool IBriefcaseManager::LocksRequired() const
    {
    // We don't acquire locks for indirect or dynamic changes.
    return (!GetDgnDb().Txns().InDynamicTxn() && TxnManager::Mode::Indirect != GetDgnDb().Txns().GetMode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManager::Validate(RepositoryStatus* pStatus)
    {
    switch (m_localDbState)
        {
        case DbState::Ready:
            if (nullptr != pStatus)
                *pStatus = RepositoryStatus::Success;
            return true;
        case DbState::Invalid:
            if (nullptr != pStatus)
                *pStatus = nullptr != GetRepositoryManager() ? RepositoryStatus::SyncError : RepositoryStatus::ServerUnavailable;
            return false;
        default:
            {
            auto initStatus = Initialize();
            if (nullptr != pStatus)
                *pStatus = initStatus;
            return RepositoryStatus::Success == initStatus;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManager::UseExistingLocalDb(BeFileNameCR filename)
    {
    // To support temporarily-offline workflows: if local state db already exists, AND server unavailable, AND pass verification checks, reuse existing.
    if (!filename.DoesPathExist() || nullptr != GetRepositoryManager() || BE_SQLITE_OK != m_localDb.OpenBeSQLiteDb(filename, Db::OpenParams(Db::OpenMode::ReadWrite)))
        return false;

    // Reject if local state db schema has changed
    Utf8String storedVersion;
    if (BE_SQLITE_ROW != m_localDb.QueryProperty(storedVersion, GetVersionPropSpec()) || !storedVersion.Equals(GetCurrentVersion()))
        return false;

    // Reject if the DgnDb has been replaced since local state db was created
    DateTime storedCreationDate, currentCreationDate;
    Utf8String creationDateString;
    if (BE_SQLITE_ROW != m_localDb.QueryProperty(creationDateString, GetCreationDatePropSpec()) || BSISUCCESS != DateTime::FromString(storedCreationDate, creationDateString.c_str())
        || BE_SQLITE_ROW != GetDgnDb().QueryCreationDate(currentCreationDate) || storedCreationDate != currentCreationDate)
        {
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManager::InitializeLocalDb()
    {
    // Save an identifier for the current schema version for later verification in offline mode...
    m_localDb.SavePropertyString(GetVersionPropSpec(), GetCurrentVersion());

    // Save the DgnDb creation date for later verification in offline mode...
    DateTime dgnDbCreationDate;
    if (BE_SQLITE_ROW == GetDgnDb().QueryCreationDate(dgnDbCreationDate))
        m_localDb.SavePropertyString(GetCreationDatePropSpec(), dgnDbCreationDate.ToUtf8String());

    // Set up the required tables
    auto result = m_localDb.CreateTable(TABLE_Codes,    CODE_AuthorityId " INTEGER,"
                                                        CODE_NameSpace " TEXT,"
                                                        CODE_Value " TEXT,"
                                                        "PRIMARY KEY" CODE_Values);

    return BE_SQLITE_OK == result && CreateLocksTable(TABLE_Locks) && CreateLocksTable(TABLE_UnavailableLocks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManager::CreateLocksTable(Utf8CP tableName)
    {
    return BE_SQLITE_OK == m_localDb.CreateTable(tableName,   LOCK_Type " INTEGER,"
                                                LOCK_Id " INTEGER,"
                                                LOCK_Level " INTEGER,"
                                                "PRIMARY KEY(" LOCK_Type "," LOCK_Id ")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::Initialize()
    {
    BeAssert(DbState::New == m_localDbState);

    m_localDbState = DbState::Invalid;

    // Local state DB stored alongside the DgnDb
    BeFileName filename = GetLocalDbFileName();

    // To support temporarily-offline workflows: if local state db already exists, AND server unavailable, AND pass verification checks, reuse existing.
    if (UseExistingLocalDb(filename))
        {
        m_localDbState = DbState::Ready;
        return RepositoryStatus::Success;
        }

    // Delete existing, if any, and create new
    filename.BeDeleteFile();
    if (BE_SQLITE_OK != m_localDb.CreateNewDb(filename))
        return RepositoryStatus::SyncError;

    // Save the DgnDb creation date for later verification in offline mode...
    DateTime dgnDbCreationDate;
    if (BE_SQLITE_ROW == GetDgnDb().QueryCreationDate(dgnDbCreationDate))
        m_localDb.SavePropertyString(GetCreationDatePropSpec(), dgnDbCreationDate.ToUtf8String());

    // Set up the required tables
    DbResult result = m_localDb.CreateTable(TABLE_Codes,
                                                     CODE_AuthorityId " INTEGER,"
                                                     CODE_NameSpace " TEXT,"
                                                     CODE_Value " TEXT,"
                                                     "PRIMARY KEY" CODE_Values);
    if (BE_SQLITE_OK == result)
        {
        result = m_localDb.CreateTable(TABLE_Locks,
                                                LOCK_Type " INTEGER,"
                                                LOCK_Id " INTEGER,"
                                                LOCK_Level " INTEGER,"
                                                "PRIMARY KEY(" LOCK_Type "," LOCK_Id ")");
        }

    return BE_SQLITE_OK == result ? Pull() : RepositoryStatus::SyncError;
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
    auto status = nullptr != server ? server->QueryHeldResources(locks, codes, GetDgnDb()) : RepositoryStatus::ServerUnavailable;
    if (RepositoryStatus::Success != status)
        return status;

    if (!locks.empty())
        Insert(locks, false);

    if (!codes.empty())
        Insert(codes);

    Save();

    m_localDbState = DbState::Ready;
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManager::Refresh()
    {
    if (DbState::Ready != m_localDbState)
        {
        // Either we haven't yet initialized the localDB, or failed to do so previously. Retry that.
        RepositoryStatus stat;
        Validate(&stat);
        return stat;
        }

    // Empty out our local DB tables and re-populate from server
    m_localDbState = DbState::Invalid; // assume something will go wrong...
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

    //Save();
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
        DgnCode code(stmt->GetValueId<DgnAuthorityId>(CodeColumn::AuthorityId), stmt->GetValueText(CodeColumn::Value), stmt->GetValueText(CodeColumn::NameSpace));
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

    //Save();
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
    return AcquireLocks(elemRequest, false);
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

    Request req;
    std::swap(locks, req.Locks());
    auto result = server->ProcessRequest(req, GetDgnDb()).Result();
    std::swap(locks, req.Locks());

    if (RepositoryStatus::Success == result)
        {
        Insert(locks, true);
        //Save();
        }

    return result;
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
        locks.erase(locks.find(DgnLock(id, LockLevel::Exclusive)));
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

    auto response = mgr->ProcessRequest(req, GetDgnDb());
    if (RepositoryStatus::Success == response.Result())
        {
        Insert(req.Codes());
        Insert(req.Locks(), true);
        //Save();
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
    LockRequestR GetUsedLocks() { return m_usedLocks; }

    RepositoryStatus ClearTxns();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ReleaseContext::ReleaseContext(DgnDbR db, bool wantLocks, bool wantCodes) : m_db(db), m_status(RepositoryStatus::CannotCreateRevision)
    {
    TxnManager& txns = db.Txns();
    if (txns.HasChanges() || txns.InDynamicTxn())
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
RepositoryStatus ReleaseContext::ClearTxns()
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
    if (nullptr == (server = GetRepositoryManager()))
        return RepositoryStatus::ServerUnavailable;

    bool wantLocks = Resources::Locks == (which & Resources::Locks),
         wantCodes = Resources::Codes == (which & Resources::Codes);

    ReleaseContext context(GetDgnDb(), wantLocks, wantCodes);
    if (RepositoryStatus::Success != context.GetStatus())
        return context.GetStatus();
    if (!context.GetUsedLocks().IsEmpty())
        return RepositoryStatus::LockUsed;
    if (!context.GetUsedCodes().empty())
        return RepositoryStatus::CodeUsed;

    DbResult result = wantLocks ? GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Locks) : BE_SQLITE_OK;
    if (BE_SQLITE_OK == result && wantCodes)
        result = GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Codes);

    if (BE_SQLITE_OK != result || BE_SQLITE_OK != Save())
        {
        BeAssert(false);
        m_localDbState = DbState::Invalid;
        return RepositoryStatus::SyncError;
        }

    stat = server->Relinquish(which, GetDgnDb());
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
        if (curLevel <= lock.GetLevel())
            iter = locks.erase(iter);
        else
            ++iter;
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

    if (!locks.empty())
        {
        if (nullptr != pStatus)
            *pStatus = RepositoryStatus::LockNotHeld;

        return false;
        }
    else if (!codes.empty())
        {
        if (nullptr != pStatus)
            *pStatus = RepositoryStatus::CodeNotReserved;

        return false;
        }

    return true;
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
RepositoryStatus BriefcaseManager::_QueryLockLevels(DgnLockSet& levels, LockableIdSet& ids)
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

    auto status = Remove(rev.GetAssignedCodes());
    if (RepositoryStatus::Success == status)
        Save();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::_OnElementInserted(DgnElementId id)
    {
    if (LocksRequired() && Validate())
        {
        Insert(LockableId(id), LockLevel::Exclusive);
        //Save();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManager::_OnModelInserted(DgnModelId id)
    {
    if (LocksRequired() && Validate())
        {
        Insert(LockableId(id), LockLevel::Exclusive);
        Save();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::ToDgnDbStatus(RepositoryStatus repoStatus, Request const& req)
    {
    switch (repoStatus)
        {
        case RepositoryStatus::Success:
            return DgnDbStatus::Success;
        case RepositoryStatus::InvalidRequest:
            return DgnDbStatus::BadRequest;
        case RepositoryStatus::RevisionRequired:
            return req.Codes().empty() ? DgnDbStatus::LockNotHeld : DgnDbStatus::CodeNotReserved;
        case RepositoryStatus::LockAlreadyHeld:
        case RepositoryStatus::LockNotHeld:
            return DgnDbStatus::LockNotHeld;
        case RepositoryStatus::CodeUnavailable:
        case RepositoryStatus::CodeNotReserved:
            return DgnDbStatus::CodeNotReserved;
        case RepositoryStatus::CodeUsed:
            return DgnDbStatus::DuplicateCode;
        case RepositoryStatus::ServerUnavailable:
        case RepositoryStatus::SyncError:
        case RepositoryStatus::InvalidResponse:
            return DgnDbStatus::RepositoryManagerError;
        case RepositoryStatus::PendingTransactions:
        case RepositoryStatus::LockUsed:
        case RepositoryStatus::CannotCreateRevision:
        default:
            // These are impossible return codes in the context in which this function is used
            return DgnDbStatus::RepositoryManagerError;
        }
    }

// ##TODO: temporary, until tool framework enhanced to handle explicitly acquiring requisite locks/codes
// Can be disabled for tests for now.
static bool s_acquireAutomatically = true;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::BackDoor_SetAutomaticAcquisition(bool acquireAutomatically)
    {
    s_acquireAutomatically = acquireAutomatically;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::OnElementOperation(DgnElementCR el, BeSQLite::DbOpcode opcode, DgnElementCP pre)
    {
    Request req;
    auto action = s_acquireAutomatically ? PrepareAction::Acquire : PrepareAction::Verify;
    return ToDgnDbStatus(PrepareForElementOperation(req, el, opcode, action, pre), req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::OnModelOperation(DgnModelCR model, BeSQLite::DbOpcode opcode)
    {
    Request req;
    auto action = s_acquireAutomatically ? PrepareAction::Acquire : PrepareAction::Verify;
    return ToDgnDbStatus(PrepareForModelOperation(req, model, opcode, action), req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::PerformAction(Request& req, PrepareAction action)
    {
    auto status = RepositoryStatus::Success;
    if (!req.IsEmpty())
        {
        switch (action)
            {
            case PrepareAction::Verify:
                AreResourcesHeld(req.Locks().GetLockSet(), req.Codes(), &status);
#ifdef DEBUG_LOCKS
                {
                Json::Value locksJson;
                req.Locks().ToJson(locksJson);
                printf("Locks not held: %s\n", Json::FastWriter::ToString(locksJson).c_str());
                }
#endif
                break;
            case PrepareAction::Acquire:
                {
                auto response = ProcessRequest(req);
#ifdef DEBUG_LOCKS
                Json::Value json;
                response.ToJson(json);
                printf("%s\n", Json::FastWriter::ToString(json).c_str());
#endif
                status = response.Result();
                break;
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode op, PrepareAction action, DgnElementCP orig)
    {
    if (!LocksRequired())
        return RepositoryStatus::Success;

    auto status = _PrepareForElementOperation(req, el, op, orig);
    if (RepositoryStatus::Success == status)
        status = PerformAction(req, action);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode op, PrepareAction action)
    {
    if (!LocksRequired())
        return RepositoryStatus::Success;

    auto status = _PrepareForModelOperation(req, model, op);
    if (RepositoryStatus::Success == status)
        status = PerformAction(req, action);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::PrepareForElementUpdate(Request& req, DgnElementCR el, PrepareAction action)
    {
    auto orig = GetDgnDb().Elements().GetElement(el.GetElementId());
    BeAssert(orig.IsValid());
    return PrepareForElementOperation(req, el, BeSQLite::DbOpcode::Update, action, orig.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::OnElementInsert(DgnElementCR el) { return OnElementOperation(el, BeSQLite::DbOpcode::Insert); }
DgnDbStatus IBriefcaseManager::OnElementUpdate(DgnElementCR el, DgnElementCR pre) { return OnElementOperation(el, BeSQLite::DbOpcode::Update, &pre); }
DgnDbStatus IBriefcaseManager::OnElementDelete(DgnElementCR el) { return OnElementOperation(el, BeSQLite::DbOpcode::Delete); }
DgnDbStatus IBriefcaseManager::OnModelInsert(DgnModelCR model) { return OnModelOperation(model, BeSQLite::DbOpcode::Insert); }
DgnDbStatus IBriefcaseManager::OnModelUpdate(DgnModelCR model) { return OnModelOperation(model, BeSQLite::DbOpcode::Update); }
DgnDbStatus IBriefcaseManager::OnModelDelete(DgnModelCR model) { return OnModelOperation(model, BeSQLite::DbOpcode::Delete); }
void IBriefcaseManager::OnElementInserted(DgnElementId id) { _OnElementInserted(id); }
void IBriefcaseManager::OnModelInserted(DgnModelId id) { _OnModelInserted(id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::_ReserveCode(DgnCodeCR code)
    {
    DgnCodeSet codes;
    codes.insert(code);
    return ReserveCodes(codes).Result();
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
    auto response = ProcessRequest(req);
    std::swap(locks, req.Locks());
    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response IBriefcaseManager::ReserveCodes(DgnCodeSet& codes, ResponseOptions options)
    {
    Request req(options);
    std::swap(codes, req.Codes());
    auto response = ProcessRequest(req);
    std::swap(codes, req.Codes());
    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IRepositoryManager::QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes)
    {
    DgnLockInfoSet unused;
    return QueryStates(unused, states, LockableIdSet(), codes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IRepositoryManager::QueryLockStates(DgnLockInfoSet& states, LockableIdSet const& locks)
    {
    DgnCodeInfoSet unused;
    return QueryStates(states, unused, locks, DgnCodeSet());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::RemoveElements(LockRequestR request, DgnModelId modelId) const
    {
    Statement stmt(m_db, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
    stmt.BindId(1, modelId);
    while (BE_SQLITE_ROW == stmt.Step())
        request.Remove(LockableId(stmt.GetValueId<DgnElementId>(0)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::ReformulateLockRequest(LockRequestR req, Response const& response) const
    {
    DgnLockSet& locks = req.GetLockSet();
    for (auto const& state : response.LockStates())
        {
        auto lockId = state.GetLockableId();
        auto found = locks.find(DgnLock(lockId, LockLevel::Exclusive));
        if (locks.end() == found)
            continue;

        auto level = state.GetOwnership().GetLockLevel();
        if (LockLevel::Shared == level)
            {
            // Shared lock should not have been denied if no one holds an exclusive lock on it...
            // Note that if we requested an exclusive lock, we will now downgrade it to a shared lock
            BeAssert(LockLevel::Exclusive == found->GetLevel());
            switch (lockId.GetType())
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
        else // Will be "None" if we need to pull a revision before acquiring this lock...treat same as if "Exclusive"
            {
            switch (lockId.GetType())
                {
                case LockableType::Db:
                    // The entire Db is locked. We can do nothing.
                    req.Clear();
                    return;
                case LockableType::Model:
                    locks.erase(found);
                    RemoveElements(req, DgnModelId(lockId.GetId().GetValue()));
                    break;
                default:
                    locks.erase(found);
                    break;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::ReformulateCodeRequest(DgnCodeSet& req, Response const& response) const
    {
    for (auto const& codeState : response.CodeStates())
        if (!codeState.IsAvailable())
            req.erase(req.find(codeState.GetCode()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::ReformulateRequest(Request& req, Response const& response) const
    {
    ReformulateLockRequest(req.Locks(), response);
    ReformulateCodeRequest(req.Codes(), response);
    }

#define JSON_Status "Status"            // RepositoryStatus
#define JSON_Locks "Locks"              // list of DgnLock.
#define JSON_LockableId "LockableId"    // LockableId
#define JSON_Id "Id"                    // BeInt64Id
#define JSON_LockType "Type"            // LockType
#define JSON_LockLevel "Level"          // LockLevel
#define JSON_Owner "Owner"              // BeBriefcaseId
#define JSON_LockStates "LockStates"    // list of DgnLockInfo
#define JSON_ExclusiveOwner "Exclusive" // BeBriefcaseId
#define JSON_SharedOwners "Shared"      // list of BeBriefcaseId
#define JSON_Ownership "Ownership"      // DgnLockOwnership
#define JSON_RevisionId "Revision"      // string
#define JSON_Tracked "Tracked"          // boolean
#define JSON_AuthorityId "Authority"    // BeInt64Id
#define JSON_Namespace "Namespace"      // string
#define JSON_Name "Name"                // string
#define JSON_Code "Code"                // DgnCode
#define JSON_CodeStateType "Type"       // DgnCodeState::Type
#define JSON_Codes "Codes"              // DgnCodeSet
#define JSON_Options "Options"          // ResponseOptions
#define JSON_CodeStates "CodeStates"    // list of DgnCodeInfo

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::BeInt64IdFromJson(BeInt64Id& id, JsonValueCR value)
    {
    if (value.isNull())
        return false;

    id = BeInt64Id(value.asInt64());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::BeInt64IdToJson(JsonValueR value, BeInt64Id id)
    {
    value = id.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::BriefcaseIdFromJson(BeBriefcaseId& bcId, JsonValueCR value)
    {
    if (!value.isConvertibleTo(Json::uintValue))
        return false;

    bcId = BeBriefcaseId(value.asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::BriefcaseIdToJson(JsonValueR value, BeBriefcaseId id)
    {
    value = id.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::LockLevelFromJson(LockLevel& level, JsonValueCR value)
    {
    if (value.isConvertibleTo(Json::uintValue))
        {
        level = static_cast<LockLevel>(value.asUInt());
        switch (level)
            {
            case LockLevel::None:
            case LockLevel::Shared:
            case LockLevel::Exclusive:
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::LockLevelToJson(JsonValueR value, LockLevel level)
    {
    value = static_cast<uint32_t>(level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::LockableTypeFromJson(LockableType& type, JsonValueCR value)
    {
    if (value.isConvertibleTo(Json::uintValue))
        {
        type = static_cast<LockableType>(value.asUInt());
        switch (type)
            {
            case LockableType::Db:
            case LockableType::Model:
            case LockableType::Element:
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::LockableTypeToJson(JsonValueR value, LockableType type)
    {
    value = static_cast<uint32_t>(type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::RepositoryStatusFromJson(RepositoryStatus& status, JsonValueCR value)
    {
    if (!value.isConvertibleTo(Json::uintValue))
        return false;

    status = static_cast<RepositoryStatus>(value.asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::RepositoryStatusToJson(JsonValueR value, RepositoryStatus status)
    {
    value = static_cast<uint32_t>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockableId::ToJson(JsonValueR value) const
    {
    RepositoryJson::BeInt64IdToJson(value[JSON_Id], m_id);
    RepositoryJson::LockableTypeToJson(value[JSON_LockType], m_type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockableId::FromJson(JsonValueCR value)
    {
    if (!RepositoryJson::BeInt64IdFromJson(m_id, value[JSON_Id]) || !RepositoryJson::LockableTypeFromJson(m_type, value[JSON_LockType]))
        {
        Invalidate();
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLock::ToJson(JsonValueR value) const
    {
    m_id.ToJson(value[JSON_LockableId]);
    RepositoryJson::LockLevelToJson(value[JSON_LockLevel], m_level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLock::FromJson(JsonValueCR value)
    {
    if (!m_id.FromJson(value[JSON_LockableId]) || !RepositoryJson::LockLevelFromJson(m_level, value[JSON_LockLevel]))
        {
        Invalidate();
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void toJson(JsonValueR value, T const& obj) { obj.ToJson(value); }
template<> void toJson(JsonValueR value, BeBriefcaseId const& id) { RepositoryJson::BriefcaseIdToJson(value, id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void collectionToJson(JsonValueR value, T const& objs)
    {
    uint32_t nObjs = static_cast<uint32_t>(objs.size());
    Json::Value array(Json::arrayValue);
    array.resize(nObjs);

    uint32_t i = 0;
    for (auto const& obj : objs)
        toJson(array[i++], obj);

    value = array;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static bool setFromJson(bset<T>& objs, JsonValueCR value)
    {
    if (!value.isArray())
        return false;

    T obj;
    uint32_t nObjs = value.size();
    for (uint32_t i = 0; i < nObjs; i++)
        {
        if (!obj.FromJson(value[i]))
            return false;

        objs.insert(obj);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLockOwnership::ToJson(JsonValueR value) const
    {
    auto level = GetLockLevel();
    RepositoryJson::LockLevelToJson(value[JSON_LockLevel], level);
    switch (level)
        {
        case LockLevel::Exclusive:
            value[JSON_ExclusiveOwner] = GetExclusiveOwner().GetValue();
            break;
        case LockLevel::Shared:
            collectionToJson(value[JSON_SharedOwners], m_sharedOwners);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLockOwnership::FromJson(JsonValueCR value)
    {
    Reset();
    LockLevel level;
    if (!RepositoryJson::LockLevelFromJson(level, value[JSON_LockLevel]))
        return false;

    switch (level)
        {
        case LockLevel::None:
            return true;
        case LockLevel::Exclusive:
            return RepositoryJson::BriefcaseIdFromJson(m_exclusiveOwner, value[JSON_ExclusiveOwner]);
        case LockLevel::Shared:
            {
            JsonValueCR owners = value[JSON_SharedOwners];
            if (!owners.isArray())
                return false;

            BeBriefcaseId owner;
            uint32_t nOwners = owners.size();
            for (uint32_t i = 0; i < nOwners; i++)
                {
                if (!RepositoryJson::BriefcaseIdFromJson(owner, value[i]))
                    {
                    Reset();
                    return false;
                    }

                AddSharedOwner(owner);
                }

            return true;
            }
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::ToJson(JsonValueR value) const
    {
    collectionToJson(value[JSON_Locks], m_locks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockRequest::FromJson(JsonValueCR value)
    {
    Clear();
    JsonValueCR locks = value[JSON_Locks];
    if (!locks.isArray())
        return false;

    DgnLock lock;
    uint32_t nLocks = locks.size();
    for (uint32_t i = 0; i < nLocks; i++)
        {
        if (!lock.FromJson(locks[i]))
            {
            Clear();
            return false;
            }

        m_locks.insert(lock);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLockState::ToJson(JsonValueR value) const
    {
    m_ownership.ToJson(value[JSON_Ownership]);
    value[JSON_RevisionId] = m_revisionId;
    value[JSON_Tracked] = m_tracked;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLockState::FromJson(JsonValueCR value)
    {
    Reset();
    if (!m_ownership.FromJson(value[JSON_Ownership]))
        return false;

    m_revisionId = value[JSON_RevisionId].asString();
    m_tracked = value[JSON_Tracked].asBool();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLockInfo::ToJson(JsonValueR value) const
    {
    DgnLockState::ToJson(value);
    m_id.ToJson(value[JSON_LockableId]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLockInfo::FromJson(JsonValueCR value)
    {
    return m_id.FromJson(value[JSON_LockableId]) && DgnLockState::FromJson(value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCode::ToJson(JsonValueR value) const
    {
    RepositoryJson::BeInt64IdToJson(value[JSON_Id], m_authority);
    value[JSON_Namespace] = m_nameSpace;
    value[JSON_Name] = m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCode::FromJson(JsonValueCR value)
    {
    if (!RepositoryJson::BeInt64IdFromJson(m_authority, value[JSON_Id]))
        {
        *this = DgnCode();
        return false;
        }

    m_nameSpace = value[JSON_Namespace].asString();
    m_value = value[JSON_Name].asString();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCodeState::ToJson(JsonValueR value) const
    {
    value[JSON_CodeStateType] = static_cast<uint32_t>(m_type);
    RepositoryJson::BriefcaseIdToJson(value[JSON_Owner], m_reservedBy);
    value[JSON_RevisionId] = m_revisionId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCodeState::FromJson(JsonValueCR value)
    {
    if (!RepositoryJson::BriefcaseIdFromJson(m_reservedBy, value[JSON_Owner]))
        {
        *this = DgnCodeState();
        return false;
        }

    m_revisionId = value[JSON_RevisionId].asString();
    m_type = static_cast<Type>(value[JSON_CodeStateType].asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCodeInfo::ToJson(JsonValueR value) const
    {
    DgnCodeState::ToJson(value);
    m_code.ToJson(value[JSON_Code]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCodeInfo::FromJson(JsonValueCR value)
    {
    return m_code.FromJson(value[JSON_Code]) && DgnCodeState::FromJson(value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::ResponseOptionsToJson(JsonValueR value, IBriefcaseManager::ResponseOptions options)
    {
    value = static_cast<uint32_t>(options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::ResponseOptionsFromJson(IBriefcaseManager::ResponseOptions& options, JsonValueCR value)
    {
    options = static_cast<IBriefcaseManager::ResponseOptions>(value.asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::Request::ToJson(JsonValueR value) const
    {
    m_locks.ToJson(value);
    collectionToJson(value[JSON_Codes], m_codes);
    RepositoryJson::ResponseOptionsToJson(value[JSON_Options], m_options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IBriefcaseManager::Request::FromJson(JsonValueCR value)
    {
    Reset();
    if (RepositoryJson::ResponseOptionsFromJson(m_options, value[JSON_Options]) && m_locks.FromJson(value) && setFromJson(m_codes, value[JSON_Codes]))
        return true;

    Reset();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::Response::ToJson(JsonValueR value) const
    {
    RepositoryJson::RepositoryStatusToJson(value[JSON_Status], m_status);
    collectionToJson(value[JSON_LockStates], m_lockStates);
    collectionToJson(value[JSON_CodeStates], m_codeStates);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IBriefcaseManager::Response::FromJson(JsonValueCR value)
    {
    Invalidate();
    if (RepositoryJson::RepositoryStatusFromJson(m_status, value[JSON_Status]) && setFromJson(m_lockStates, value[JSON_LockStates]) && setFromJson(m_codeStates, value[JSON_CodeStates]))
        return true;

    Invalidate();
    return false;
    }


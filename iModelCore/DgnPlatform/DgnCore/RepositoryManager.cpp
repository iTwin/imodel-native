/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnChangeSummary.h>

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct MasterBriefcaseManager : IBriefcaseManager
{
private:
    MasterBriefcaseManager(DgnDbR db) : IBriefcaseManager(db) { }

    Response _ProcessRequest(Request& req, RequestPurpose purpose) override { return Response(purpose, req.Options(), RepositoryStatus::Success); }
    RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override { return RepositoryStatus::Success; }
    RepositoryStatus _Relinquish(Resources) override { return RepositoryStatus::Success; }
    RepositoryStatus _ReserveCode(DgnCodeCR) override { return RepositoryStatus::Success; }
    RepositoryStatus _QueryLockLevel(LockLevel& level, LockableId lockId) override { level = LockLevel::Exclusive; return RepositoryStatus::Success; }
    IOwnedLocksIteratorPtr _GetOwnedLocks(FastQuery fast) override { return nullptr; }
    RepositoryStatus _OnFinishRevision(DgnRevision const&) override { return RepositoryStatus::Success; }
    RepositoryStatus _RefreshFromRepository() override { return RepositoryStatus::Success; }
    RepositoryStatus _ClearUserHeldCodesLocks() override { return RepositoryStatus::Success; }
    void _OnElementInserted(DgnElementId) override { }
    void _OnModelInserted(DgnModelId) override { }
    void _StartBulkOperation() override {}
    bool _IsBulkOperation() const override {return false;}
    Response _EndBulkOperation() override {return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::Success);}

    RepositoryStatus _QueryLockLevels(DgnLockSet& levels, LockableIdSet& lockIds) override
        {
        for (auto const& id : lockIds)
            levels.insert(DgnLock(id, LockLevel::Exclusive));

        return RepositoryStatus::Success;
        }
    bool _AreResourcesHeld(DgnLockSet&, DgnCodeSet&, RepositoryStatus* status) override
        {
        if (nullptr != status)
            *status = RepositoryStatus::Success;
        return true;
        }
    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) override
        {
        auto bcId = GetDgnDb().GetBriefcaseId();
        for (auto const& code : codes)
            states.insert(DgnCodeInfo(code)).first->SetReserved(bcId);
        return RepositoryStatus::Success;
        }
    RepositoryStatus _PrepareForElementOperation(Request&, DgnElementCR, BeSQLite::DbOpcode) override { return RepositoryStatus::Success; }
    RepositoryStatus _PrepareForModelOperation(Request&, DgnModelCR, BeSQLite::DbOpcode) override { return RepositoryStatus::Success; }
public:
    static IBriefcaseManagerPtr Create(DgnDbR db) { return new MasterBriefcaseManager(db); }
};

#define STMT_ModelIdFromElement "SELECT ModelId FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?"

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
struct BriefcaseManagerBase : IBriefcaseManager, TxnMonitor
{
protected:
    enum class DbState { New, Ready, Invalid };
    enum class TableType { Owned, Unavailable };

    Db      m_localDb;
    DbState m_localDbState;

    bset<BeInt64Id>    m_exclusivelyLockedModels;

    Response _ProcessRequest(Request&, RequestPurpose purpose) override;
    RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override;
    RepositoryStatus _Relinquish(Resources which) override;
    bool _AreResourcesHeld(DgnLockSet&, DgnCodeSet&, RepositoryStatus*) override;
    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&) override;
    RepositoryStatus _QueryLockLevel(LockLevel&, LockableId) override;
    RepositoryStatus _QueryLockLevels(DgnLockSet&, LockableIdSet&) override;
    IOwnedLocksIteratorPtr _GetOwnedLocks(FastQuery fast) override;
    RepositoryStatus _OnFinishRevision(DgnRevision const&) override;
    RepositoryStatus _RefreshFromRepository() override { return Refresh(); }
    RepositoryStatus _ClearUserHeldCodesLocks() override { return ClearUserHeldCodesLocks(); }
    void _OnElementInserted(DgnElementId) override;
    void _OnModelInserted(DgnModelId) override;
    void _OnDgnDbDestroyed() override;

    void Save(TxnManager& mgr) { if (&mgr.GetDgnDb() == &GetDgnDb()) Save(); }
    void _OnCommit(TxnManager& mgr) override { Save(mgr); }
    void _OnAppliedChanges(TxnManager& mgr) override { Save(mgr); }
    void _OnUndoRedo(TxnManager& mgr, TxnAction) override { Save(mgr); }
    RepositoryStatus _PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode op) override
        {
        RepositoryStatus status = el.PopulateRequest(req, op);

        if (!LockRequired(el))
            req.Locks().Remove(LockableId(el));

        return status;
        }
    RepositoryStatus _PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode op) override
        {
        return model.PopulateRequest(req, op);
        }

    BriefcaseManagerBase(DgnDbR db) : IBriefcaseManager(db), m_localDbState(DbState::New)
        {
        m_exclusivelyLockedModels = bset<BeInt64Id>();
        T_HOST.GetTxnAdmin().AddTxnMonitor(*this);
        }
    ~BriefcaseManagerBase()
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
    bool CreateCodesTable(Utf8CP tableName);
    RepositoryStatus Refresh();
    RepositoryStatus ClearUserHeldCodesLocks();
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
    void InsertCodes(DgnCodeSet const& codes, TableType tableType);
    void Cull(DgnCodeSet& codes);
    RepositoryStatus Remove(DgnCodeSet const& codes);

    // Locks...
    void InsertLock(LockableId id, LockLevel level, TableType tableType, bool overwrite=false);
    template<typename T> void InsertLocks(T const& locks, TableType tableType, bool checkExisting);
    void AddDependentElements(DgnLockSet& locks, bvector<DgnModelId> const& models);
    RepositoryStatus PromoteDependentElements(LockRequestCR usedLocks, bvector<DgnModelId> const& models);
    void Cull(DgnLockSet& locks);
    void CullElementsInExclusiveModels(DgnLockSet& locks);
    RepositoryStatus AcquireLocks(LockRequestR locks, bool cull);
    Response DoFastQuery(Request const&);
    RepositoryStatus FastQueryLocks(Response& response, LockRequest const& locks, ResponseOptions options);
    RepositoryStatus FastQueryCodes(Response& response, DgnCodeSet const& codes, ResponseOptions options);

    /*---------------------------------------------------------------------------------**//**
     * Check if an element's model id is exclusively locked, if it is do not require a lock
     * @bsimethod                                                    Diego.Pinate    03/18
     +---------------+---------------+---------------+---------------+---------------+------*/
    bool LockRequired(DgnModelId elementsModelId)
        {
        if (LocksRequired())
            return (m_exclusivelyLockedModels.find(elementsModelId) == m_exclusivelyLockedModels.end());

        return false;
        }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod                                                    Diego.Pinate    03/18
     +---------------+---------------+---------------+---------------+---------------+------*/
    bool LockRequired(DgnElementId elementId)
        {
        CachedStatementPtr stmt = GetDgnDb().GetCachedStatement(STMT_ModelIdFromElement);
        stmt->BindId(1, elementId);
        stmt->Step();
        DgnModelId modelId = stmt->GetValueId<DgnModelId>(0);
        stmt->Reset();
        return modelId.IsValid() ? LockRequired(modelId) : LocksRequired();
        }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod                                                    Diego.Pinate    03/18
     +---------------+---------------+---------------+---------------+---------------+------*/
    bool LockRequired(DgnElementCR element)
        {
        return LockRequired(element.GetModelId());
        }


    BeFileName GetLocalDbFileName() const
        {
        BeFileName filename = GetDgnDb().GetFileName();
        filename.AppendExtension(L"local");
        return filename;
        }

    static PropSpec GetCreationDatePropSpec() { return PropSpec("BriefcaseLocalStateDbCreationDate"); }
    static PropSpec GetVersionPropSpec() { return PropSpec("BriefcaseLocalStateDbVersion"); }
    static Utf8CP GetCurrentVersion() { return "1"; }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct BulkUpdateBriefcaseManager : BriefcaseManagerBase
{
    DEFINE_T_SUPER(BriefcaseManagerBase)
protected:
    Request m_req;      // locks and codes that we must acquire before we can say that update has succeeded
    int m_inBulkUpdate = 0;
    bset<CodeSpecId>    m_filterCodeSpecs;  // code specs to filter from code requests
#ifndef NDEBUG
    intptr_t m_threadId = 0;
#endif

    BulkUpdateBriefcaseManager(DgnDbR db) : T_Super(db) {;}

    Response _ProcessRequest(Request& req, RequestPurpose purpose) override;
    RepositoryStatus _Relinquish(Resources) override;
    RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override;
    RepositoryStatus _PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode op) override;
    RepositoryStatus _PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode op) override;
    void _OnElementInserted(DgnElementId) override;
    void _OnModelInserted(DgnModelId) override;
    RepositoryStatus _ReserveCode(DgnCodeCR) override;
    void _OnDgnDbDestroyed() override {m_req.Reset(); m_inBulkUpdate=false; T_Super::_OnDgnDbDestroyed();}
    RepositoryStatus _OnFinishRevision(DgnRevision const& rev) override;
    void _OnCommit(TxnManager& mgr) override;
    void _OnAppliedChanges(TxnManager& mgr) override;
    void _OnUndoRedo(TxnManager& mgr, TxnAction) override;
    bool _AreResourcesHeld(DgnLockSet& l, DgnCodeSet& c, RepositoryStatus* status) override
        {
        auto ret = T_Super::_AreResourcesHeld(l, c, status);
        if (nullptr != status && _IsBulkOperation() && !m_req.IsEmpty())
            *status = RepositoryStatus::Success; // Don't report missing locks and codes if we are in the middle of a bulk op and haven't made our request yet.
        return ret;
        }

    /*---------------------------------------------------------------------------------**//**
     * Create the set to filter codes from request
     * @bsimethod                                                    Diego.Pinate    03/18
     +---------------+---------------+---------------+---------------+---------------+------*/
    void _PopulateFilterCodeSpecs()
        {
        m_filterCodeSpecs = bset<CodeSpecId>();
        // Geometry Parts
        auto geomPartCodeSpec = GetDgnDb().CodeSpecs().GetCodeSpec(BIS_CODESPEC_GeometryPart);
        m_filterCodeSpecs.insert(geomPartCodeSpec->GetCodeSpecId());
        }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod                                                    Diego.Pinate    03/18
     +---------------+---------------+---------------+---------------+---------------+------*/
    bset<CodeSpecId> _GetFilteredCodeSpecIds() override
        {
        // Cache filter code spec set
        if (m_filterCodeSpecs.empty())
            _PopulateFilterCodeSpecs();

        return m_filterCodeSpecs;
        }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod                                                    Diego.Pinate    03/18
     +---------------+---------------+---------------+---------------+---------------+------*/
    void FilterCodes(DgnCodeSet& outputCodes, DgnCodeSet const& inputCodes)
        {
        if (m_filterCodeSpecs.empty())
            _PopulateFilterCodeSpecs();

        // Only add output codes that are not in our filter set
        for (auto code : inputCodes)
            if (m_filterCodeSpecs.find(code.GetCodeSpecId()) == m_filterCodeSpecs.end())
                outputCodes.insert(code);
        }

    // Note: functions like _QueryCodeStates and _QueryLockLevel do NOT look in m_req. They check what we actually have obtained from the server.

    void _StartBulkOperation() override;
    bool _IsBulkOperation() const override {return 0 != m_inBulkUpdate;}
    Response _EndBulkOperation() override;

    void _ExtractRequestFromBulkOperation(Request& reqOut, bool locks, bool codes) override
        {
        auto control = GetDgnDb().GetConcurrencyControl();
        if (nullptr != control)
            control->_OnExtractRequest(m_req, *this);

        if (locks)
            {
            reqOut.Codes().insert(m_req.Codes().begin(), m_req.Codes().end());
            m_req.Codes().clear();
            }
        if (codes)
            {
            reqOut.Locks().GetLockSet().insert(m_req.Locks().GetLockSet().begin(), m_req.Locks().GetLockSet().end());
            m_req.Locks().Clear();
            }
        // TODO: merge options
        // reqOut.SetOptions(m_req.Options());

        if (nullptr != control)
            control->_OnExtractedRequest(m_req, *this);
        }

    void AccumulateRequests(Request const&);
public:
    static RefCountedPtr<BulkUpdateBriefcaseManager> Create(DgnDbR db) {return new BulkUpdateBriefcaseManager(db);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbR BriefcaseManagerBase::GetLocalDb()
    {
    Validate();
    return m_localDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::_OnDgnDbDestroyed()
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
    IBriefcaseManagerPtr bc;
    if (db.IsLegacyMaster() || db.IsLegacyStandalone() || db.IsSnapshot() || db.IsFutureStandalone())
        bc = MasterBriefcaseManager::Create(db);
    else
        bc = BulkUpdateBriefcaseManager::Create(db);
    if (bc.IsValid() && (nullptr != db.GetConcurrencyControl()))
        db.GetConcurrencyControl()->_ConfigureBriefcaseManager(*bc);
    return bc.get();
    }

#define TABLE_Codes "Codes"
#define TABLE_UnavailableCodes "UnavailableCodes"
#define CODE_CodeSpecId "CodeSpecId"
#define CODE_Scope "Scope"
#define CODE_Value "CodeValue"
#define CODE_Columns CODE_CodeSpecId "," CODE_Scope "," CODE_Value
#define CODE_Values "(" CODE_Columns ")"
#define STMT_InsertCode "INSERT INTO " TABLE_Codes " " CODE_Values " Values (?,?,?)"
#define STMT_InsertUnavailableCode "INSERT INTO " TABLE_UnavailableCodes " " CODE_Values " Values (?,?,?)"
#define STMT_SelectUnavailableCodesInSet "SELECT " CODE_Columns " FROM " TABLE_UnavailableCodes " WHERE InVirtualSet(@vset," CODE_Columns ")"
#define STMT_DeleteCodesInSet "DELETE FROM " TABLE_Codes " WHERE InVirtualSet(@vset," CODE_Columns ")"
#define STMT_SelectCode "SELECT * FROM " TABLE_Codes " WHERE " CODE_CodeSpecId "=? AND " CODE_Scope "=? AND " CODE_Value "=?"

enum CodeColumn { CodeSpec=0, Scope, Value };

#define TABLE_Locks "Locks"
#define TABLE_UnavailableLocks "UnavailableLocks"
#define LOCK_Type "Type"
#define LOCK_Id "Id"
#define LOCK_Level "Level"
#define LOCK_Columns LOCK_Type "," LOCK_Id "," LOCK_Level
#define LOCK_Values "(" LOCK_Columns ")"
#define STMT_SelectExistingLock "SELECT " LOCK_Level ",rowid FROM " TABLE_Locks " WHERE " LOCK_Type "=? AND " LOCK_Id "=?"
#define STMT_SelectExistingUnavailableLock "SELECT " LOCK_Level ",rowid FROM " TABLE_UnavailableLocks " WHERE " LOCK_Type "=? AND " LOCK_Id "=?"
#define STMT_InsertNewLock "INSERT INTO " TABLE_Locks " " LOCK_Values " VALUES (?,?,?)"
#define STMT_InsertUnavailableLock "INSERT INTO " TABLE_UnavailableLocks " " LOCK_Values " VALUES (?,?,?)"
#define STMT_UpdateLockLevel "UPDATE " TABLE_Locks " SET " LOCK_Level "=? WHERE rowid=?"
#define STMT_UpdateUnavailableLockLevel "UPDATE " TABLE_UnavailableLocks " SET " LOCK_Level "=? WHERE rowid=?"
#define STMT_SelectLocksInSet "SELECT " LOCK_Type "," LOCK_Id " FROM " TABLE_Locks " WHERE InVirtualSet(@vset," LOCK_Columns ")"
#define STMT_SelectUnavailableLocksInSet "SELECT " LOCK_Type "," LOCK_Id "," LOCK_Level " FROM " TABLE_UnavailableLocks " WHERE InVirtualSet(@vset," LOCK_Columns ")"
#define STMT_SelectElementInModel " SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ModelId=?"
#define STMT_InsertOrReplaceLock "INSERT OR REPLACE INTO " TABLE_Locks " " LOCK_Values " VALUES(?,?,?)"
#define STMT_InsertOrReplaceUnavailableLock "INSERT OR REPLACE INTO " TABLE_UnavailableLocks " " LOCK_Values " VALUES(?,?,?)"
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
bool BriefcaseManagerBase::Validate(RepositoryStatus* pStatus)
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
bool BriefcaseManagerBase::UseExistingLocalDb(BeFileNameCR filename)
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
bool BriefcaseManagerBase::InitializeLocalDb()
    {
    // Save an identifier for the current schema version for later verification in offline mode...
    m_localDb.SavePropertyString(GetVersionPropSpec(), GetCurrentVersion());

    // Save the DgnDb creation date for later verification in offline mode...
    DateTime dgnDbCreationDate;
    if (BE_SQLITE_ROW == GetDgnDb().QueryCreationDate(dgnDbCreationDate))
        m_localDb.SavePropertyString(GetCreationDatePropSpec(), dgnDbCreationDate.ToString());

    // Set up the required tables
    return CreateCodesTable(TABLE_Codes) && CreateLocksTable(TABLE_Locks)
        && CreateCodesTable(TABLE_UnavailableCodes) && CreateLocksTable(TABLE_UnavailableLocks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManagerBase::CreateCodesTable(Utf8CP tableName)
    {
    return BE_SQLITE_OK == m_localDb.CreateTable(tableName, CODE_CodeSpecId " INTEGER,"
                                                            CODE_Scope " TEXT,"
                                                            CODE_Value " TEXT COLLATE NOCASE,"
                                                            "PRIMARY KEY" CODE_Values);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseManagerBase::CreateLocksTable(Utf8CP tableName)
    {
    return BE_SQLITE_OK == m_localDb.CreateTable(tableName,   LOCK_Type " INTEGER,"
                                                LOCK_Id " INTEGER,"
                                                LOCK_Level " INTEGER,"
                                                "PRIMARY KEY(" LOCK_Type "," LOCK_Id ")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::Initialize()
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

    // Set up the required tables and properties, then populate from server
    return InitializeLocalDb() ? Pull() : RepositoryStatus::SyncError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::Pull()
    {
    // Populate local Db with reserved codes and locks from the server
    DgnLockSet locks, unavailableLocks;
    DgnCodeSet codes, unavailableCodes;
    auto server = GetRepositoryManager();
    auto status = nullptr != server ? server->QueryHeldResources(locks, codes, unavailableLocks, unavailableCodes, GetDgnDb()) : RepositoryStatus::ServerUnavailable;
    if (RepositoryStatus::Success != status)
        return status;

    if (!locks.empty())
        InsertLocks(locks, TableType::Owned, false);

    if (!codes.empty())
        InsertCodes(codes, TableType::Owned);

    if (!unavailableLocks.empty())
        InsertLocks(unavailableLocks, TableType::Unavailable, false);

    if (!unavailableCodes.empty())
        InsertCodes(unavailableCodes, TableType::Unavailable);

    Save();

    m_localDbState = DbState::Ready;
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::Refresh()
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
    if (BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Locks) || BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_UnavailableLocks)
        || BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Codes) || BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_UnavailableCodes))
        {
        return RepositoryStatus::SyncError;
        }

    m_exclusivelyLockedModels.clear();

    return Pull();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     01/18
//-------------------------------------------------------------------------------------------
RepositoryStatus BriefcaseManagerBase::ClearUserHeldCodesLocks()
    {
    if (DbState::Ready != m_localDbState)
        {
        // Either we haven't yet initialized the localDB, or failed to do so previously. Retry that.
        RepositoryStatus stat;
        Validate(&stat);
        return stat;
        }

    // Clear user held locks and codes
    m_localDbState = DbState::Invalid; // assume something will go wrong...
    if (BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Locks)
        || BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " TABLE_Codes))
        {
        return RepositoryStatus::SyncError;
        }

    m_exclusivelyLockedModels.clear();

    Save();

    m_localDbState = DbState::Ready;
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::InsertCodes(DgnCodeSet const& codes, TableType tableType)
    {
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(TableType::Owned == tableType ? STMT_InsertCode : STMT_InsertUnavailableCode);
    for (auto const& code : codes)
        {
        if (code.IsEmpty() || !code.IsValid())
            {
            BeAssert(false);
            continue;
            }

        stmt->BindId(CodeColumn::CodeSpec+1, code.GetCodeSpecId());
        stmt->BindText(CodeColumn::Scope+1, code.GetScopeString(), Statement::MakeCopy::No);
        stmt->BindText(CodeColumn::Value+1, code.GetValue().GetUtf8(), Statement::MakeCopy::No);
        stmt->Step();
        stmt->Reset();
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
            return arg.GetCodeSpecId().GetValueUnchecked() == vals[CodeColumn::CodeSpec].GetValueUInt64()
                && arg.GetScopeString().Equals(vals[CodeColumn::Scope].GetValueText())
                && arg.GetValue().Equals(vals[CodeColumn::Value].GetValueText());
            });
        }
};

/*---------------------------------------------------------------------------------**//**
* Don't bother asking server to reserve codes which the briefcase knows are already reserved...
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::Cull(DgnCodeSet& codes)
    {
    // Don't bother asking server to reserve codes which we've already reserved...
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectCode);
    auto iter = codes.begin();
    while (iter != codes.end())
        {
        DgnCodeCR code = *iter;

        // Don't bother asking server to reserve empty or invalid codes...
        if (code.IsEmpty() || !code.IsValid())
            {
            iter = codes.erase(iter);
            continue;
            }

        // Don't bother asking server to reserve codes not managed by iModelHub
        CodeSpecCPtr codeSpec = GetDgnDb().CodeSpecs().GetCodeSpec(code.GetCodeSpecId());
        if (!codeSpec.IsValid() || !codeSpec->IsManagedWithDgnDb())
            {
            iter = codes.erase(iter);
            continue;
            }

        stmt->BindId(CodeColumn::CodeSpec+1, code.GetCodeSpecId());
        stmt->BindText(CodeColumn::Scope+1, code.GetScopeString(), Statement::MakeCopy::No);
        stmt->BindText(CodeColumn::Value+1, code.GetValueUtf8(), Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == stmt->Step())
            iter = codes.erase(iter);
        else
            ++iter;

        stmt->Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::Remove(DgnCodeSet const& codes)
    {
    VirtualCodeSet vset(codes);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_DeleteCodesInSet);
    stmt->BindVirtualSet(1, vset);
    if (BE_SQLITE_OK != stmt->Step())
        return RepositoryStatus::SyncError;

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
void BriefcaseManagerBase::InsertLock(LockableId id, LockLevel level, TableType tableType, bool overwrite)
    {
    auto sql = TableType::Owned == tableType
             ? (overwrite ? STMT_InsertOrReplaceLock : STMT_InsertNewLock)
             : (overwrite ? STMT_InsertOrReplaceUnavailableLock : STMT_InsertUnavailableLock);

    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(sql);
    bindEnum(*stmt, 1, id.GetType());
    stmt->BindId(2, id.GetId());
    bindEnum(*stmt, 3, level);

    stmt->Step();

    // Maintain set of exclusively locked models
    if (id.GetType() == LockableType::Model)
        {
        // Not exclusively locked anymore
        if (m_exclusivelyLockedModels.find(id.GetId()) != m_exclusivelyLockedModels.end() && level != LockLevel::Exclusive)
            m_exclusivelyLockedModels.erase(id.GetId());
        // Exclusively locked, add to set
        if (level == LockLevel::Exclusive)
            m_exclusivelyLockedModels.insert(id.GetId());
        }
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
template<typename T> void BriefcaseManagerBase::InsertLocks(T const& locks, TableType tableType, bool checkExisting)
    {
    CachedStatementPtr select, insert, update;
    bool insertingOwnedLocks = (TableType::Owned == tableType);
    if (insertingOwnedLocks)
        {
        insert = GetLocalDb().GetCachedStatement(STMT_InsertNewLock);
        if (checkExisting)
            {
            select = GetLocalDb().GetCachedStatement(STMT_SelectExistingLock);
            update = GetLocalDb().GetCachedStatement(STMT_UpdateLockLevel);
            }
        }
    else
        {
        insert = GetLocalDb().GetCachedStatement(STMT_InsertUnavailableLock);
        if (checkExisting)
            {
            select = GetLocalDb().GetCachedStatement(STMT_SelectExistingUnavailableLock);
            update = GetLocalDb().GetCachedStatement(STMT_UpdateUnavailableLockLevel);
            }
        }

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

        if (insertingOwnedLocks && lock.IsExclusive())
            {
            switch (lock.GetType())
                {
                case LockableType::Db:
                    dbExclusivelyLocked = true;
                    exclusivelyLockedModels.clear();
                    for (ModelIteratorEntryCR model : GetDgnDb().Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)))
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

    // The 'unavailable locks' table is mimicking the server, which doesn't track these implicitly owned locks...
    if (!insertingOwnedLocks)
        return;

    if (dbExclusivelyLocked)
        {
        for (auto const& model : exclusivelyLockedModels)
            InsertLock(LockableId(LockableType::Model, model), LockLevel::Exclusive, tableType, true);
        InsertLock(LockableId(GetDgnDb().Schemas()), LockLevel::Exclusive, tableType, true);
        }

    if (!exclusivelyLockedModels.empty())
        {
        for (auto const& model : exclusivelyLockedModels)
            {
            m_exclusivelyLockedModels.insert(model);
            ModelElementLocks elemLocks(model, GetDgnDb());
            InsertLocks(elemLocks, tableType, true);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::AddDependentElements(DgnLockSet& locks, bvector<DgnModelId> const& models)
    {
    if (models.empty())
        return;

    struct VSet : VirtualSet
    {
        bvector<DgnModelId> const& m_models;
        DgnDbR m_db;

        VSet(bvector<DgnModelId> const& models, DgnDbR db) : m_models(models), m_db(db) { }

        bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            BeAssert(1 == nVals);
            auto el = m_db.Elements().GetElement(DgnElementId(vals[0].GetValueUInt64()));
            return el.IsValid() && (m_models.end() != std::find(m_models.begin(), m_models.end(), el->GetModelId()));
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
RepositoryStatus BriefcaseManagerBase::PromoteDependentElements(LockRequestCR usedLocks, bvector<DgnModelId> const& models)
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
RepositoryStatus BriefcaseManagerBase::AcquireLocks(LockRequestR locks, bool cull)
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
    auto result = server->Acquire(req, GetDgnDb()).Result();
    std::swap(locks, req.Locks());

    if (RepositoryStatus::Success == result)
        InsertLocks(locks, TableType::Owned, true);

    return result;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   06/16
//=======================================================================================
struct VirtualLockSet : VirtualSet
{
    DgnLockSet const& m_locks;
    VirtualLockSet(DgnLockSet const& locks) : m_locks(locks) { }
    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        BeAssert(3 == nVals);
        LockableId id(static_cast<LockableType>(vals[0].GetValueInt()), BeInt64Id(vals[1].GetValueUInt64()));
        return _IsLockInSet(DgnLock(id, static_cast<LockLevel>(vals[2].GetValueInt())));
        }
    virtual bool _IsLockInSet(DgnLockCR lock) const = 0;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::Cull(DgnLockSet& locks)
    {
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectExistingLock);
    auto iter = locks.begin();
    while (iter != locks.end())
        {
        auto const& lock = *iter;
        bindEnum(*stmt, 1, lock.GetType());
        stmt->BindId(2, lock.GetId());
        if (BE_SQLITE_ROW != stmt->Step() || lock.GetLevel() > static_cast<LockLevel>(stmt->GetValueInt(0)))
            ++iter;
        else
            iter = locks.erase(iter);

        stmt->Reset();
        }

    CullElementsInExclusiveModels(locks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    06/18
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::CullElementsInExclusiveModels(DgnLockSet& locks)
    {
    DgnLockSet toDelete;
    for (DgnLock lock : locks)
        {
        if (lock.GetType() != LockableType::Element)
            continue;
        DgnElementId elementId(lock.GetId().GetValue());
        if (!LockRequired(elementId))
            toDelete.insert(lock);
        }

    for (DgnLock lock : toDelete)
        locks.erase(lock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response BriefcaseManagerBase::_ProcessRequest(Request& req, RequestPurpose purpose)
    {
    if (req.IsEmpty())
        return Response(purpose, req.Options(), RepositoryStatus::Success);

    RepositoryStatus stat;
    if (!Validate(&stat))
        return Response(purpose, req.Options(), stat);

    Cull(req);
    if (req.IsEmpty())
        return Response(purpose, req.Options(), RepositoryStatus::Success);

    if (RequestPurpose::FastQuery == purpose)
        return DoFastQuery(req);

    auto mgr = GetRepositoryManager();
    if (nullptr == mgr)
        return Response(purpose, req.Options(), RepositoryStatus::ServerUnavailable);

    auto control = GetDgnDb().GetConcurrencyControl();
    if (nullptr != control)
        control->_OnProcessRequest(req, *this, purpose);

    auto response = RequestPurpose::Acquire == purpose ? mgr->Acquire(req, GetDgnDb()) : mgr->QueryAvailability(req, GetDgnDb());
    if (RequestPurpose::Acquire == purpose && RepositoryStatus::Success == response.Result())
        {
        InsertCodes(req.Codes(), TableType::Owned);
        InsertLocks(req.Locks(), TableType::Owned, true);
        }

    if (nullptr != control)
        control->_OnProcessedRequest(req, *this, purpose, response);

    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response BriefcaseManagerBase::DoFastQuery(Request const& req)
    {
    Response response(RequestPurpose::FastQuery, req.Options());
    response.SetResult(FastQueryLocks(response, req.Locks(), req.Options()));
    auto codeResult = FastQueryCodes(response, req.Codes(), req.Options());
    if (RepositoryStatus::Success == response.Result())
        response.SetResult(codeResult);

    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::FastQueryLocks(Response& response, LockRequest const& locks, ResponseOptions options)
    {
    struct VSet : VirtualLockSet
    {
        VSet(DgnLockSet const& locks) : VirtualLockSet(locks) { }
        bool _IsLockInSet(DgnLockCR lock) const override
            {
            BeAssert(LockLevel::None != lock.GetLevel());

            auto iter = m_locks.find(DgnLock(lock.GetLockableId(), LockLevel::Exclusive));
            if (m_locks.end() == iter)
                return false;

            switch (iter->GetLevel())
                {
                case LockLevel::Exclusive:  return true;
                case LockLevel::Shared:     return LockLevel::Exclusive == lock.GetLevel();
                }

            BeAssert(false && "LockRequest requests LockLevel::None");
            return false;
            }
    };

    VSet vset(locks.GetLockSet());
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectUnavailableLocksInSet);
    stmt->BindVirtualSet(1, vset);

    RepositoryStatus status = RepositoryStatus::Success;
    bool wantDetails = ResponseOptions::None != (ResponseOptions::LockState & options);
    BeBriefcaseId bcId(BeBriefcaseId::LegacyStandalone()); // a lie...
    while (BE_SQLITE_ROW == stmt->Step())
        {
        status = RepositoryStatus::LockAlreadyHeld;
        if (!wantDetails)
            break;

        // NB: FastQuery cannot supply all ownership/revision details...callers who care can check the RequestPurpose of the Response and query server for details.
        LockableId id(static_cast<LockableType>(stmt->GetValueInt(0)), BeInt64Id(stmt->GetValueUInt64(1)));
        auto level = static_cast<LockLevel>(stmt->GetValueInt(2));

        DgnLockInfo details(id, true);
        if (LockLevel::Exclusive == level)
            details.GetOwnership().SetExclusiveOwner(bcId);
        else
            details.GetOwnership().AddSharedOwner(bcId);

        response.LockStates().insert(details);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::FastQueryCodes(Response& response, DgnCodeSet const& codes, ResponseOptions options)
    {
    VirtualCodeSet vset(codes);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectUnavailableCodesInSet);
    stmt->BindVirtualSet(1, vset);

    RepositoryStatus status = RepositoryStatus::Success;
    bool wantDetails = ResponseOptions::None != (ResponseOptions::CodeState & options);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        status = RepositoryStatus::CodeUnavailable;
        if (!wantDetails)
            break;

        // NB: FastQuery cannot supply all ownership/revision details...callers who care can check the RequestPurpose of the Response and query server for details.
        DgnCode code = DgnCode::From(stmt->GetValueId<CodeSpecId>(CodeColumn::CodeSpec), stmt->GetValueText(CodeColumn::Scope), stmt->GetValueText(CodeColumn::Value));
        if (!code.IsValid())
            continue;

        DgnCodeInfo details(code);
        details.SetReserved(BeSQLite::BeBriefcaseId());
        response.CodeStates().insert(details);
        }

    return status;
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

    RevisionStatus revStatus;
    DgnRevisionPtr rev = db.Revisions().StartCreateRevision(&revStatus);
    if (rev.IsValid())
        {
        if (wantLocks)
            m_usedLocks.FromRevision(*rev, db, true, false);

        if (wantCodes)
            {
            DgnCodeSet discardedCodes;
            rev->ExtractCodes(m_usedCodes, discardedCodes, db);
            }

        m_status = RepositoryStatus::Success;
        m_endTxnId = db.Revisions().QueryCurrentRevisionEndTxnId();

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
RepositoryStatus BriefcaseManagerBase::_Relinquish(Resources which)
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

    stat = server->Relinquish(which, GetDgnDb());
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
RepositoryStatus BriefcaseManagerBase::_Demote(DgnLockSet& locks, DgnCodeSet const& codes)
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
bool BriefcaseManagerBase::_AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* pStatus)
    {
    if (!Validate(pStatus))
        return false;

    if (nullptr != pStatus)
        *pStatus = RepositoryStatus::Success;

    Cull(locks);
    Cull(codes);

    auto control = GetDgnDb().GetConcurrencyControl();
    if (nullptr != control)
        control->_OnQueryHeld(locks, codes, *this);

    bool allHeld = true;

    if (!locks.empty())
        {
        if (nullptr != pStatus)
            *pStatus = RepositoryStatus::LockNotHeld;
        allHeld = false;
        }
    else if (!codes.empty())
        {
        if (nullptr != pStatus)
            *pStatus = RepositoryStatus::CodeNotReserved;

        allHeld = false;
        }

    if (nullptr != control)
        control->_OnQueriedHeld(locks, codes, *this);

    return allHeld;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::_QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes)
    {
    auto server = GetRepositoryManager();
    return nullptr != server ? server->QueryCodeStates(states, codes) : RepositoryStatus::ServerUnavailable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::_QueryLockLevel(LockLevel& level, LockableId lockId)
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
RepositoryStatus BriefcaseManagerBase::_QueryLockLevels(DgnLockSet& levels, LockableIdSet& ids)
    {
    // Our local DB was populated from the server...there's no need to contact server again
    RepositoryStatus stat;
    if (!Validate(&stat))
        return stat;

    struct VSet : VirtualSet
    {
        LockableIdSet const& m_ids;
        VSet(LockableIdSet const& ids) : m_ids(ids) { }
        bool _IsInSet(int nVals, DbValue const* vals) const override
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

    // We are avoiding obtaining locks for elements inside a exclusively locked model, so account for those here
    if (!ids.empty())
        {
        stmt = GetDgnDb().GetCachedStatement(STMT_ModelIdFromElement);
        for (auto iter = ids.begin(); iter != ids.end(); )
            {
            LockableId id = *iter;
            if (id.GetType() == LockableType::Element)
                {
                stmt->BindId(1, id.GetId());
                stmt->Step();
                DgnModelId modelId = stmt->GetValueId<DgnModelId>(0);
                stmt->Reset();
                LockLevel modelLockLevel;
                _QueryLockLevel(modelLockLevel, LockableId(modelId));
                if (modelLockLevel == LockLevel::Exclusive)
                    {
                    levels.insert(DgnLock(id, LockLevel::Exclusive));
                    iter = ids.erase(iter);
                    }
                else
                    ++iter;
                }
            else
                ++iter;
            }
        }

    // Any not found are not locked by us
    for (auto const& id : ids)
        levels.insert(DgnLock(id, LockLevel::None));

    return RepositoryStatus::Success;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct OwnedLocksIterator : IOwnedLocksIterator
{
private:
    static constexpr Utf8CP GetSql() { return "SELECT " LOCK_Id "," LOCK_Type "," LOCK_Level " FROM " TABLE_Locks; }

    struct Iter : DbTableIterator::Entry
    {
        Iter() : DbTableIterator::Entry(nullptr, false) { }
        Iter(BeSQLite::Statement& stmt) : DbTableIterator::Entry(&stmt, BE_SQLITE_ROW == stmt.Step()) { }

        DgnLock GetLock() const
            {
            auto id = m_sql->GetValueId<BeInt64Id>(0);
            auto type = static_cast<LockableType>(m_sql->GetValueInt(1));
            auto level = static_cast<LockLevel>(m_sql->GetValueInt(2));
            return DgnLock(LockableId(type, id), level);
            }
    };

    BeSQLite::CachedStatementPtr    m_stmt;
    Iter                            m_cur;
    Iter                            m_end;
    DgnLock                         m_lock;

    OwnedLocksIterator(BeSQLite::Db& db) : m_stmt(db.GetCachedStatement(GetSql())), m_cur(*m_stmt)
        {
        InitLock();
        }

    void InitLock()
        {
        if (m_cur != m_end)
            m_lock = m_cur.GetLock();
        }

    DgnLockCR _GetCurrent() const override { return m_lock; }
    bool _IsAtEnd() const override { return m_cur == m_end; }
    void _Reset() override { m_stmt->Reset(); m_cur = Iter(*m_stmt); InitLock(); }
    void _MoveNext() override
        {
        if (m_cur != m_end)
            {
            ++m_cur;
            InitLock();
            }
        }
public:
    static IOwnedLocksIteratorPtr Create(BeSQLite::Db& db) { return new OwnedLocksIterator(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
IOwnedLocksIteratorPtr BriefcaseManagerBase::_GetOwnedLocks(FastQuery fast)
    {
    bool valid = FastQuery::Yes == fast ? Validate() : RepositoryStatus::Success == RefreshFromRepository();
    return valid ? OwnedLocksIterator::Create(GetLocalDb()).get() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BriefcaseManagerBase::_OnFinishRevision(DgnRevision const& rev)
    {
    // Any codes which became Used as a result of these changes must necessarily have been Reserved by this briefcase,
    // and are now no longer Reserved by any briefcase
    // (Any codes which became Discarded were necessarily previously Used, therefore no local state needs to be updated for them).
    DgnCodeSet assignedCodes, discardedCodes;
    rev.ExtractCodes(assignedCodes, discardedCodes, GetDgnDb());

    if (assignedCodes.empty())
        return RepositoryStatus::Success;
    else if (!Validate())
        return RepositoryStatus::SyncError;

    auto status = Remove(assignedCodes);
    if (RepositoryStatus::Success == status)
        Save();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::_OnElementInserted(DgnElementId id)
    {
    if (LockRequired(id) && Validate())
        InsertLock(LockableId(id), LockLevel::Exclusive, TableType::Owned);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseManagerBase::_OnModelInserted(DgnModelId id)
    {
    if (LocksRequired() && Validate())
        {
        InsertLock(LockableId(id), LockLevel::Exclusive, TableType::Owned);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::OnElementOperation(DgnElementCR el, BeSQLite::DbOpcode opcode)
    {
    Request req;
    return ToDgnDbStatus(PrepareForElementOperation(req, el, opcode, PrepareAction::Verify), req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::OnModelOperation(DgnModelCR model, BeSQLite::DbOpcode opcode)
    {
    Request req;
    return ToDgnDbStatus(PrepareForModelOperation(req, model, opcode, PrepareAction::Verify), req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::_PerformPrepareAction(Request& req, PrepareAction action)
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
                auto response = Acquire(req);
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

RepositoryStatus IBriefcaseManager::PerformPrepareAction(Request& req, PrepareAction action) {return _PerformPrepareAction(req, action);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode op, PrepareAction action)
    {
    if (!LocksRequired())
        return RepositoryStatus::Success;

    auto status = _PrepareForElementOperation(req, el, op);
    if (RepositoryStatus::Success == status)
        status = PerformPrepareAction(req, action);

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
        status = PerformPrepareAction(req, action);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IBriefcaseManager::PrepareForElementUpdate(Request& req, DgnElementCR el, PrepareAction action)
    {
    auto orig = GetDgnDb().Elements().GetElement(el.GetElementId());
    BeAssert(orig.IsValid());
    return PrepareForElementOperation(req, el, BeSQLite::DbOpcode::Update, action);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IBriefcaseManager::OnElementInsert(DgnElementCR el) { return OnElementOperation(el, BeSQLite::DbOpcode::Insert); }
DgnDbStatus IBriefcaseManager::OnElementUpdate(DgnElementCR el) { return OnElementOperation(el, BeSQLite::DbOpcode::Update); }
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
    auto response = Acquire(req);
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
    auto response = Acquire(req);
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
    Statement stmt(m_db, "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ModelId=?");
    stmt.BindId(1, modelId);
    while (BE_SQLITE_ROW == stmt.Step())
        request.Remove(LockableId(stmt.GetValueId<DgnElementId>(0)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IBriefcaseManager::ReformulateLockRequest(LockRequestR req, Response const& response) const
    {
    BeAssert(!_IsBulkOperation() && "this function does not make sense in bulk operation mode");

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IBriefcaseManager::AreResourcesAvailable(Request& req, Response* pResponse, FastQuery fast)
    {
    auto purpose = FastQuery::Yes == fast ? RequestPurpose::FastQuery : RequestPurpose::Query;
    Response localResponse(purpose, req.Options());
    auto& response = nullptr != pResponse ? *pResponse : localResponse;
    response = _ProcessRequest(req, purpose);
    return RepositoryStatus::Success == response.Result();
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
#define JSON_CodeSpecId "CodeSpec"      // BeInt64Id
#define JSON_Scope "Scope"              // string
#define JSON_Name "Name"                // string
#define JSON_Code "Code"                // DgnCode
#define JSON_CodeStateType "Type"       // DgnCodeState::Type
#define JSON_Codes "Codes"              // DgnCodeSet
#define JSON_Options "Options"          // ResponseOptions
#define JSON_Purpose "Purpose"          // RequestPurpose
#define JSON_CodeStates "CodeStates"    // list of DgnCodeInfo

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::BeInt64IdFromJson(BeInt64Id& id, JsonValueCR value)
    {
    if (value.isNull())
        return false;

    if (value.isString()) // string is expected
        {
        BentleyStatus status;
        id = BeInt64Id::FromString(value.asCString(), &status);
        return BentleyStatus::SUCCESS == status;
        }

    if (value.isIntegral()) // integer is not expected, but might as well support it as this was the previous behavior
        {
        id = BeInt64Id(value.asInt64());
        return true;
        }

    BeAssert(false); // should never get here
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::BeInt64IdToJson(JsonValueR value, BeInt64Id id)
    {
    value = id.ToHexStr();
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
    return value.isConvertibleTo(Json::uintValue) && LockLevelFromUInt(level, value.asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::LockLevelFromUInt(LockLevel& level, unsigned int value)
    {
    switch (static_cast<LockLevel>(value))
        {
        case LockLevel::None:
        case LockLevel::Shared:
        case LockLevel::Exclusive:
            level = static_cast<LockLevel>(value);
            return true;
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
    return value.isConvertibleTo(Json::uintValue) && LockableTypeFromUInt(type, value.asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::LockableTypeFromUInt(LockableType& type, unsigned int value)
    {
    switch (static_cast<LockableType>(value))
        {
        case LockableType::Db:
        case LockableType::Model:
        case LockableType::Element:
        case LockableType::Schemas:
        case LockableType::CodeSpecs:
            type = static_cast<LockableType>(value);
            return true;
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
    return value.isConvertibleTo(Json::uintValue) && RepositoryStatusFromUInt(status, value.asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::RepositoryStatusFromUInt(RepositoryStatus& status, unsigned int value)
    {
    status = static_cast<RepositoryStatus>(value);
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
    RepositoryJson::BeInt64IdToJson(value[JSON_Id], m_specId);
    value[JSON_Scope] = m_scope;
    value[JSON_Name] = m_value.GetUtf8();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCode::FromJson(JsonValueCR value)
    {
    if (!RepositoryJson::BeInt64IdFromJson(m_specId, value[JSON_Id]))
        {
        *this = DgnCode();
        return false;
        }

    m_scope = value[JSON_Scope].asString();
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
    return value.isConvertibleTo(Json::uintValue) && ResponseOptionsFromUInt(options, value.asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::ResponseOptionsFromUInt(IBriefcaseManager::ResponseOptions& options, unsigned int value)
    {
    options = static_cast<IBriefcaseManager::ResponseOptions>(value);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::RequestPurposeFromJson(IBriefcaseManager::RequestPurpose& purpose, JsonValueCR value)
    {
    return value.isConvertibleTo(Json::uintValue) && RequestPurposeFromUInt(purpose, value.asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryJson::RequestPurposeFromUInt(IBriefcaseManager::RequestPurpose& purpose, unsigned int value)
    {
    switch (static_cast<IBriefcaseManager::RequestPurpose>(value))
        {
        case IBriefcaseManager::RequestPurpose::Acquire:
        case IBriefcaseManager::RequestPurpose::Query:
        case IBriefcaseManager::RequestPurpose::FastQuery:
            purpose = static_cast<IBriefcaseManager::RequestPurpose>(value);
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryJson::RequestPurposeToJson(JsonValueR value, IBriefcaseManager::RequestPurpose purpose)
    {
    value = static_cast<uint32_t>(purpose);
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
    RepositoryJson::RequestPurposeToJson(value[JSON_Purpose], m_purpose);
    RepositoryJson::ResponseOptionsToJson(value[JSON_Options], m_options);
    collectionToJson(value[JSON_LockStates], m_lockStates);
    collectionToJson(value[JSON_CodeStates], m_codeStates);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool IBriefcaseManager::Response::FromJson(JsonValueCR value)
    {
    Invalidate();
    if (RepositoryJson::RepositoryStatusFromJson(m_status, value[JSON_Status])
        && RepositoryJson::RequestPurposeFromJson(m_purpose, value[JSON_Purpose])
        && RepositoryJson::ResponseOptionsFromJson(m_options, value[JSON_Options])
        && setFromJson(m_lockStates, value[JSON_LockStates])
        && setFromJson(m_codeStates, value[JSON_CodeStates]))
        return true;

    Invalidate();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkUpdateBriefcaseManager::AccumulateRequests(Request const& req)
    {
    DgnCodeSet filteredCodes;
    FilterCodes(filteredCodes, req.Codes());

    BeAssert(m_inBulkUpdate);
    m_req.Codes().insert(filteredCodes.begin(), filteredCodes.end());
    m_req.Locks().GetLockSet().insert(req.Locks().GetLockSet().begin(), req.Locks().GetLockSet().end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response BulkUpdateBriefcaseManager::_ProcessRequest(IBriefcaseManager::Request& req, IBriefcaseManager::RequestPurpose purpose)
    {
    if (!m_inBulkUpdate || (IBriefcaseManager::RequestPurpose::Acquire != purpose))
        return T_Super::_ProcessRequest(req, purpose);

#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif

    AccumulateRequests(req);
    return IBriefcaseManager::Response(purpose, req.Options(), RepositoryStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BulkUpdateBriefcaseManager::_Relinquish(Resources res)
    {
    if (m_inBulkUpdate)
        {
        BeAssert(false && "makes no sense in bulk operation mode");
        return  RepositoryStatus::InvalidRequest;
        }

    return T_Super::_Relinquish(res);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BulkUpdateBriefcaseManager::_ReserveCode(DgnCodeCR code)
    {
    if (!m_inBulkUpdate)
        return T_Super::_ReserveCode(code);

#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif

    m_req.Codes().insert(code);
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BulkUpdateBriefcaseManager::_Demote(DgnLockSet& locks, DgnCodeSet const& codes)
    {
    if (!m_inBulkUpdate)
        return T_Super::_Demote(locks, codes);

#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif

    for (auto& code : codes)
        {
        auto i = m_req.Codes().find(code);
        if (i != m_req.Codes().end())
            *i = code;
        }
    for (auto& lock : locks)
        {
        auto& reqlocks = m_req.Locks();
        reqlocks.Remove(lock.GetLockableId());
        reqlocks.InsertLock(lock.GetLockableId(), lock.GetLevel());
        }
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkUpdateBriefcaseManager::_StartBulkOperation()
    {
    ++m_inBulkUpdate;

#ifndef NDEBUG
    if (0 == m_threadId)
        m_threadId = BeThreadUtilities::GetCurrentThreadId();
    else
        {
        BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response BulkUpdateBriefcaseManager::_EndBulkOperation()
    {
#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif

    if (m_inBulkUpdate <= 0)
        {
        BeAssert(false);
        return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::InvalidRequest);
        }

    if (0 != --m_inBulkUpdate)
        return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::Success);

    m_req.SetOptions(ResponseOptions::All); // Get details when requests such as code reservations fail.
    auto resp = T_Super::_ProcessRequest(m_req, RequestPurpose::Acquire);

    if (RepositoryStatus::Success != resp.Result())
        return resp;

    m_req.Reset();

#ifndef NDEBUG
    m_threadId = 0;
#endif
    return resp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BulkUpdateBriefcaseManager::_PrepareForElementOperation(Request& reqOut, DgnElementCR el, BeSQLite::DbOpcode op)
    {
    if (!m_inBulkUpdate)
        return T_Super::_PrepareForElementOperation(reqOut, el, op);

#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif

    auto rstat = el.PopulateRequest(reqOut, op);
    if (RepositoryStatus::Success != rstat)
        return rstat;

    if (!LockRequired(el))
        reqOut.Locks().Remove(LockableId(el));

    AccumulateRequests(reqOut);
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BulkUpdateBriefcaseManager::_PrepareForModelOperation(Request& reqOut, DgnModelCR model, BeSQLite::DbOpcode op)
    {
    if (!m_inBulkUpdate)
        return T_Super::_PrepareForModelOperation(reqOut, model, op);

#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif

     auto rstat = model.PopulateRequest(reqOut, op);

    if (RepositoryStatus::Success != rstat)
        return rstat;

    AccumulateRequests(reqOut);
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkUpdateBriefcaseManager::_OnElementInserted(DgnElementId id)
    {
    if (!m_inBulkUpdate)
        {
        T_Super::_OnElementInserted(id);
        return;
        }

#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif
    if (LockRequired(id)) // don't Validate
        m_req.Locks().GetLockSet().insert(DgnLock(LockableId(id), LockLevel::Exclusive));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkUpdateBriefcaseManager::_OnModelInserted(DgnModelId id)
    {
    if (!m_inBulkUpdate)
        {
        T_Super::_OnModelInserted(id);
        return;
        }

#ifndef NDEBUG
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif
    if (LocksRequired()) // don't Validate
        {
        m_req.Locks().GetLockSet().insert(DgnLock(LockableId(id), LockLevel::Exclusive));
        m_exclusivelyLockedModels.insert(id);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus BulkUpdateBriefcaseManager::_OnFinishRevision(DgnRevision const& rev)
    {
    if (!m_inBulkUpdate)
        return T_Super::_OnFinishRevision(rev);

    BeAssert(false);
    return RepositoryStatus::PendingTransactions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkUpdateBriefcaseManager::_OnCommit(TxnManager& mgr)
    {
    BeAssert (!m_inBulkUpdate && "somebody called SaveChanges while in a NESTED bulk op");
    T_Super::_OnCommit(mgr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkUpdateBriefcaseManager::_OnAppliedChanges(TxnManager& mgr)
    {
    // This is called by CancelChanges, undo/redo and by revision manager
    if (m_inBulkUpdate)
        {
#ifndef NDEBUG
        BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif
        m_inBulkUpdate = false;
        m_req.Reset();
        }
    T_Super::_OnAppliedChanges(mgr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkUpdateBriefcaseManager::_OnUndoRedo(TxnManager& mgr, TxnAction action)
    {
    if (m_inBulkUpdate)
        {
#ifndef NDEBUG
        BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_threadId);
#endif
        m_inBulkUpdate = false;
        m_req.Reset();
        }
    T_Super::_OnUndoRedo(mgr, action);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/19
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus IRepositoryManager::_QueryHeldLocks(DgnLockSet& locks, DgnDbR db)
    {
    DgnLockSet unavailableLocks;
    DgnCodeSet codes, unavailableCodes;
    return QueryHeldResources(locks, codes, unavailableLocks, unavailableCodes, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId IBriefcaseManager::GetNormalChannelParentOf(DgnElementCR el)
    {
    return GetNormalChannelParentOf(el.GetModelId(), &el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManager::Response IBriefcaseManager::_LockChannelParent()
    {
    auto channelParentInfo = GetChannelProps();
    auto channelParent = GetDgnDb().Elements().GetElement(channelParentInfo.channelParentId);
    if (!channelParent.IsValid())
        {
        BeAssert(false);
        return Response(RequestPurpose::Acquire, ResponseOptions::All, RepositoryStatus::InvalidRequest);
        }

    Request req;
    // _PrepareForElementOperation(req, *channelParent, BeSQLite::DbOpcode::Update); No. This will trigger a channel lock error
    req.Locks().InsertLock(LockableId(channelParentInfo.channelParentId), LockLevel::Exclusive);
    req.Locks().InsertLock(LockableId(channelParent->GetModelId()), LockLevel::Shared);
    req.Locks().InsertLock(LockableId(channelParent->GetDgnDb()), LockLevel::Shared);
 
    req.SetOptions(ResponseOptions::All);

    return Acquire(req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IRepositoryManager::RepositoryStatusToString(RepositoryStatus s)
    {
    switch(s)
        {
        case RepositoryStatus::Success: return "";
        case RepositoryStatus::ServerUnavailable:       return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_ServerUnavailable());
        case RepositoryStatus::LockAlreadyHeld:         return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_LockAlreadyHeld());
        case RepositoryStatus::SyncError:               return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_SyncError());
        case RepositoryStatus::InvalidResponse:         return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_InvalidResponse());
        case RepositoryStatus::PendingTransactions:     return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_PendingTransactions());
        case RepositoryStatus::LockUsed:                return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_LockUsed());
        case RepositoryStatus::CannotCreateRevision:    return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_CannotCreateRevision());
        case RepositoryStatus::InvalidRequest:          return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_InvalidRequest());
        case RepositoryStatus::RevisionRequired:        return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_RevisionRequired());
        case RepositoryStatus::CodeUnavailable:         return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_CodeUnavailable());
        case RepositoryStatus::CodeNotReserved:         return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_CodeNotReserved());
        case RepositoryStatus::CodeUsed:                return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_CodeUsed());
        case RepositoryStatus::LockNotHeld:             return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_LockNotHeld());
        case RepositoryStatus::RepositoryIsLocked:      return DgnCoreL10N::GetString(DgnCoreL10N::RepositoryStatus_RepositoryIsLocked());
        }
    return "?";
    }
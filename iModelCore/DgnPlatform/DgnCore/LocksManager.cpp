/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/LocksManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnChangeSummary.h>

static const BeInt64Id s_dbId((uint64_t)1);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnDbCR db)
    : m_id(s_dbId), m_type(LockableType::Db)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnModelCR model) : m_id(model.GetModelId()), m_type(LockableType::Model)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnElementCR el) : m_id(el.GetElementId()), m_type(LockableType::Element)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLockCP LockRequest::Find(DgnLockCR lock, bool matchExactLevel) const
    {
    auto iter = m_locks.find(DgnLock(lock.GetLockableId(), LockLevel::Exclusive));
    if (m_locks.end() == iter)
        return nullptr;
    else if (matchExactLevel && iter->GetLevel() != lock.GetLevel())
        return nullptr;
    else if (iter->GetLevel() > lock.GetLevel())
        return nullptr;

    return &(*iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnElementCR el, LockLevel level)
    {
    // Currently no reason to obtain shared lock on an element...may have a use case for parent elements at some point.
    if (LockLevel::Shared == level)
        level = LockLevel::Exclusive;

    InsertLock(LockableId(el.GetElementId()), level);
    if (LockLevel::Exclusive == level)
        {
        DgnModelPtr model = el.GetModel();
        BeAssert(model.IsValid());
        if (model.IsValid())
            Insert(*model, LockLevel::Shared);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnModelCR model, LockLevel level)
    {
    InsertLock(LockableId(model.GetModelId()), level);
    if (LockLevel::None != level)
        Insert(model.GetDgnDb(), LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnDbCR db, LockLevel level)
    {
    InsertLock(LockableId(db), level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::InsertLock(LockableId id, LockLevel level)
    {
    if (LockLevel::None == level || !id.IsValid())
        return;

    auto pair = m_locks.insert(DgnLock(id, level));
    pair.first->EnsureLevel(level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Remove(LockableId id)
    {
    auto iter = m_locks.find(DgnLock(id, LockLevel::Exclusive));
    if (m_locks.end() != iter)
        m_locks.erase(iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t LockRequest::Subtract(DgnLockSet const& rhs)
    {
    size_t nRemoved = 0;
    for (auto const& lock : rhs)
        {
        auto found = m_locks.find(DgnLock(lock.GetLockableId(), LockLevel::Exclusive));
        if (m_locks.end() != found)
            {
            m_locks.erase(found);
            ++nRemoved;
            }
        }

    return nRemoved;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LockRequest::FromChangeSet(IChangeSetR changes, DgnDbR db)
    {
    Clear();

    DgnChangeSummary summary(db);
    if (SUCCESS != summary.FromChangeSet(changes))
        return DgnDbStatus::BadArg;

    for (auto const& entry : summary.MakeElementIterator())
        {
        if (entry.IsIndirectChange())
            continue;   // ###TODO: Allow iterator options to specify exclusion of indirect changes

        DgnModelId modelId;
        switch (entry.GetDbOpcode())
            {
            case DbOpcode::Insert:  modelId = entry.GetCurrentModelId(); break;
            case DbOpcode::Delete:  modelId = entry.GetOriginalModelId(); break;
            case DbOpcode::Update:
                {
                modelId = entry.GetCurrentModelId();
                auto oldModelId = entry.GetOriginalModelId();
                if (oldModelId != modelId)
                    InsertLock(LockableId(oldModelId), LockLevel::Shared);

                break;
                }
            }

        BeAssert(modelId.IsValid());
        InsertLock(LockableId(modelId), LockLevel::Shared);
        InsertLock(LockableId(entry.GetElementId()), LockLevel::Exclusive);
        }

    // Any models directly changed?
    ECClassId classId = db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Model);
    DgnChangeSummary::InstanceIterator::Options options(classId);
    for (auto const& entry : summary.MakeInstanceIterator(options))
        {
        if (!entry.GetIndirect())
            InsertLock(LockableId(LockableType::Model, entry.GetInstanceId()), LockLevel::Exclusive);
        }

    // Anything changed at all?
    if (!IsEmpty())
        Insert(db, LockLevel::Shared);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Defined here because we don't want them called externally...
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ILocksManager::OnElementInserted(DgnElementId id) { _OnElementInserted(id); }
void ILocksManager::OnModelInserted(DgnModelId id) { _OnModelInserted(id); }
LockStatus ILocksManager::LockElement(DgnElementCR el, LockLevel lvl, DgnModelId originalModelId) { return _LockElement(el, lvl, originalModelId); }
LockStatus ILocksManager::LockModel(DgnModelCR model, LockLevel lvl) { return _LockModel(model, lvl); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ILocksManager::GetLockTableFileName() const
    {
    // Assumption is that if the dgndb is writable, its directory is too, given that sqlite also needs to create files in that directory for journaling.
    BeFileName filename = GetDgnDb().GetFileName();
    filename.AppendExtension(L"locks");
    return filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus ILocksManager::_LockElement(DgnElementCR el, LockLevel level, DgnModelId originalModelId)
    {
    // We don't acquire locks for indirect or dynamic changes.
    if (GetDgnDb().Txns().IsInDynamics() || TxnManager::Mode::Indirect == GetDgnDb().Txns().GetMode())
        return LockStatus::Success;

    LockRequest request;
    request.Insert(el, level);
    if (LockLevel::None != level && originalModelId.IsValid() && originalModelId != el.GetModelId())
        {
        // An update operation which moved an element from one model to another...
        DgnModelPtr originalModel = GetDgnDb().Models().GetModel(originalModelId);
        if (originalModel.IsValid())
            request.Insert(*originalModel, LockLevel::Shared);
        }

    return AcquireLocks(request).GetStatus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus ILocksManager::_LockModel(DgnModelCR model, LockLevel level)
    {
    // We don't acquire locks for indirect or dynamic changes.
    if (GetDgnDb().Txns().IsInDynamics() || TxnManager::Mode::Indirect == GetDgnDb().Txns().GetMode())
        return LockStatus::Success;

    LockRequest request;
    request.Insert(model, level);
    return AcquireLocks(request).GetStatus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus ILocksManager::LockDb(LockLevel level)
    {
    LockRequest request;
    request.Insert(GetDgnDb(), level);
    return AcquireLocks(request).GetStatus();
    }

/*---------------------------------------------------------------------------------**//**
* I'm creating a new master DgnDb. I haven't handed out any briefcases, or hosted it
* on a server. Therefore, I don't care about locking.
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnrestrictedLocksManager : ILocksManager
{
private:
    UnrestrictedLocksManager(DgnDbR db) : ILocksManager(db) { }

    virtual bool _QueryLocksHeld(LockRequestR, bool) override { return true; }
    virtual LockRequest::Response _AcquireLocks(LockRequestR, AcquireAll) override { return LockRequest::Response(LockStatus::Success); }
    virtual LockStatus _RelinquishLocks() override { return LockStatus::Success; }
    virtual void _OnElementInserted(DgnElementId) override { }
    virtual void _OnModelInserted(DgnModelId) override { }
    virtual LockStatus _LockElement(DgnElementCR, LockLevel, DgnModelId) override { return LockStatus::Success; }
    virtual LockStatus _LockModel(DgnModelCR, LockLevel) override { return LockStatus::Success; }
    virtual LockStatus _QueryLockLevel(LockLevel& level, LockableId, bool) override { level = LockLevel::Exclusive; return LockStatus::Success; }
    virtual LockStatus _RefreshLocks() override { return LockStatus::Success; }
public:
    static ILocksManagerPtr Create(DgnDbR db) { return new UnrestrictedLocksManager(db); }
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
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalLocksManager : ILocksManager
{
private:
    enum class DbState { New, Ready, Invalid };

    Db m_db;
    DbState m_dbState;

    LocalLocksManager(DgnDbR db) : ILocksManager(db), m_dbState(DbState::New) { }

    BeBriefcaseId GetId() const { return GetDgnDb().GetBriefcaseId(); }

    virtual bool _QueryLocksHeld(LockRequestR locks, bool localOnly) override;
    virtual LockRequest::Response _AcquireLocks(LockRequestR locks, AcquireAll acquireAll) override;
    virtual LockStatus _RelinquishLocks() override;
    virtual LockStatus _QueryLockLevel(LockLevel& level, LockableId lockId, bool localOnly) override;
    virtual LockStatus _RefreshLocks() override;

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

    bool Validate(LockStatus* status = nullptr);
    void Cull(LockRequestR locks);
    DbResult Save() { return m_db.SaveChanges(); }
public:
    static ILocksManagerPtr Create(DgnDbR db) { return new LocalLocksManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalLocksManager::Validate(LockStatus* pStatus)
    {
    LockStatus localStatus = LockStatus::Success;
    LockStatus& status = nullptr != pStatus ? *pStatus : localStatus;

    switch (m_dbState)
        {
        case DbState::Ready:    return true;
        case DbState::Invalid: return false;
        }

    // Assume something will go wrong.
    m_dbState = DbState::Invalid;

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

    if (BE_SQLITE_OK != result)
        {
        status = LockStatus::SyncError;
        return false;
        }

    // Request a list of current locks from server
    DgnLockSet locks;
    auto server = GetLocksServer();
    status = nullptr != server ? server->QueryLocks(locks, GetDgnDb()) : LockStatus::ServerUnavailable;
    if (LockStatus::Success != status)
        return false;

    if (!locks.empty())
        Insert(locks, false);

    m_dbState = DbState::Ready;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus LocalLocksManager::_RefreshLocks()
    {
    if (DbState::Ready != m_dbState)
        {
        // Either we haven't yet initialized the local DB, or failed to do so previously. Retry that.
        LockStatus status;
        Validate(&status);
        return status;
        }

    // Drop our local DB and re-populate from server
    auto server = GetLocksServer();
    if (nullptr == server)
        return LockStatus::ServerUnavailable;

    DgnLockSet locks;
    auto status = server->QueryLocks(locks, GetDgnDb());
    if (LockStatus::Success != status)
        return status;

    if (BE_SQLITE_OK != m_db.ExecuteSql("DELETE FROM " LOCAL_Table))
        return LockStatus::SyncError;

    if (!locks.empty())
        Insert(locks, false);

    Save();
    return LockStatus::Success;
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
LockStatus LocalLocksManager::_QueryLockLevel(LockLevel& level, LockableId id, bool localOnly)
    {
    level = LockLevel::None;
    CachedStatementPtr select = m_db.GetCachedStatement(STMT_SelectExisting);
    bindEnum(*select, 1, id.GetType());
    select->BindId(2, id.GetId());
    if (BE_SQLITE_ROW == select->Step())
        {
        level = static_cast<LockLevel>(select->GetValueInt(0));
        return LockStatus::Success;
        }
    else if (localOnly)
        {
        return LockStatus::Success;
        }

    auto server = GetLocksServer();
    return nullptr != server ? server->QueryLockLevel(level, id, GetDgnDb()) : LockStatus::ServerUnavailable;
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
    ILocksServerP server;
    if (!Validate() || nullptr == (server = GetLocksServer()))
        return LockStatus::SyncError;
    else if (BE_SQLITE_OK == m_db.ExecuteSql("DELETE FROM " LOCAL_Table) && BE_SQLITE_OK == Save())
        return server->RelinquishLocks(GetDgnDb());
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
    if (localOnly || locks.IsEmpty())
        return locks.IsEmpty();

    auto server = GetLocksServer();
    return nullptr != server && server->QueryLocksHeld(locks, GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockRequest::Response LocalLocksManager::_AcquireLocks(LockRequestR locks, AcquireAll acquireAll)
    {
    if (!Validate())
        return LockRequest::Response(LockStatus::SyncError);

    Cull(locks);
    if (locks.IsEmpty())
        return LockRequest::Response(LockStatus::Success);

    auto server = GetLocksServer();
    if (nullptr == server)
        return LockRequest::Response(LockStatus::ServerUnavailable);

    auto responseOptions = (AcquireAll::Yes == acquireAll) ? ILocksServer::ResponseOptions::DeniedLocks : ILocksServer::ResponseOptions::None;
    auto response = server->AcquireLocks(locks, GetDgnDb(), responseOptions);
    if (AcquireAll::No == acquireAll && LockStatus::AlreadyHeld == response.GetStatus())
        {
        // ###TODO: Remove denied locks from request and retry.
        return response;
        }

    if (LockStatus::Success != response.GetStatus())
        return response;

    Insert(locks, true);
    Save();
    return response;
    }

/*---------------------------------------------------------------------------------**//**
* We still don't have a server. Some apps apparently use briefcase IDs other than zero
* (ConceptStation). Therefore, always use the unrestricted locks manager until we have
* an actual server implementation; or while explicitly enabled for tests.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_enableLocking = false;
void ILocksManager::BackDoor_SetLockingEnabled(bool enable) { s_enableLocking = enable; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
ILocksManagerPtr DgnPlatformLib::Host::LocksAdmin::_CreateLocksManager(DgnDbR db) const
    {
    // NEEDSWORK: Bogus. Currently we have no way of determining if locking is required for a given DgnDb...and we have no actual server
    return (db.IsMasterCopy() || !s_enableLocking) ? UnrestrictedLocksManager::Create(db) : LocalLocksManager::Create(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ILocksServerP ILocksManager::GetLocksServer() const
    {
    return T_HOST.GetLocksAdmin()._GetLocksServer(GetDgnDb());
    }

#define JSON_Status "Status"            // LockStatus
#define JSON_AllAcquired "AllAcquired"  // boolean
#define JSON_Locks "Locks"              // list of DgnLock.
#define JSON_LockableId "LockableId"    // LockableId
#define JSON_Id "Id"                    // BeInt64Id
#define JSON_LockType "Type"            // LockType
#define JSON_LockLevel "Level"          // LockLevel
#define JSON_Owner "Owner"              // BeBriefcaseId
#define JSON_DeniedLocks "DeniedLocks"  // list of DgnHeldLock. Only supplied if AllAcquired=false

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool idFromJson(BeInt64Id& id, JsonValueCR value)
    {
    if (!value.isConvertibleTo(Json::intValue))
        return false;

    id = BeInt64Id(value.asInt64());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool lockLevelFromJson(LockLevel& level, JsonValueCR value)
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
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool lockTypeFromJson(LockableType& type, JsonValueCR value)
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
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool lockStatusFromJson(LockStatus& status, JsonValueCR value)
    {
    if (!value.isConvertibleTo(Json::uintValue))
        return false;

    status = static_cast<LockStatus>(value.asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockableId::ToJson(JsonValueR value) const
    {
    value[JSON_Id] = m_id.GetValue();
    value[JSON_LockType] = static_cast<uint32_t>(m_type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockableId::FromJson(JsonValueCR value)
    {
    if (!idFromJson(m_id, value[JSON_Id]) || !lockTypeFromJson(m_type, value[JSON_LockType]))
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
    value[JSON_LockLevel] = static_cast<uint32_t>(m_level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLock::FromJson(JsonValueCR value)
    {
    if (!m_id.FromJson(value[JSON_LockableId]) || !lockLevelFromJson(m_level, value[JSON_LockLevel]))
        {
        Invalidate();
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnHeldLock::ToJson(JsonValueR value) const
    {
    DgnLock::ToJson(value);
    value[JSON_Owner] = m_owner.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnHeldLock::FromJson(JsonValueCR value)
    {
    auto const& ownerValue = value[JSON_Owner];
    if (!ownerValue.isConvertibleTo(Json::uintValue) || !DgnLock::FromJson(value))
        {
        Invalidate();
        return false;
        }

    m_owner = BeBriefcaseId(ownerValue.asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::ToJson(JsonValueR value) const
    {
    uint32_t nLocks = static_cast<uint32_t>(m_locks.size());
    Json::Value locks(Json::arrayValue);
    locks.resize(nLocks);

    uint32_t i = 0;
    for (auto const& lock : m_locks)
        lock.ToJson(locks[i++]);

    value[JSON_Locks] = locks;
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
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Response::ToJson(JsonValueR value) const
    {
    uint32_t nLocks = static_cast<uint32_t>(m_denied.size());
    Json::Value locks(Json::arrayValue);
    locks.resize(nLocks);

    uint32_t i = 0;
    for (auto const& lock : m_denied)
        lock.ToJson(locks[i++]);

    value[JSON_DeniedLocks] = locks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockRequest::Response::FromJson(JsonValueCR value)
    {
    m_denied.clear();
    JsonValueCR locks = value[JSON_DeniedLocks];
    if (!locks.isArray() || !lockStatusFromJson(m_status, value[JSON_Status]))
        {
        Invalidate();
        return false;
        }

    DgnHeldLock lock;
    uint32_t nLocks = locks.size();
    for (uint32_t i = 0; i < nLocks; i++)
        {
        if (!lock.FromJson(locks[i++]))
            {
            Invalidate();
            return false;
            }

        m_denied.insert(lock);
        }

    return true;
    }


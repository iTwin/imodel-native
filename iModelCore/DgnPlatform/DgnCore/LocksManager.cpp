/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/LocksManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

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
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnElementCR el, LockLevel level)
    {
    Insert(LockableId(el.GetElementId()), level);
    if (LockLevel::Exclusive == level)
        {
        Insert(el.GetDgnDb(), LockLevel::Shared);

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
    Insert(LockableId(model.GetModelId()), level);
    if (LockLevel::Exclusive == level)
        Insert(model.GetDgnDb(), LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnDbCR db, LockLevel level)
    {
    Insert(LockableId(db), level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(LockableId id, LockLevel level)
    {
    if (LockLevel::None == level || !id.IsValid())
        return;

    auto pair = m_locks.insert(DgnLock(id, level));
    pair.first->EnsureLevel(level);
    }

/*---------------------------------------------------------------------------------**//**
* Defined here because we don't want them called externally...
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ILocksManager::OnElementInserted(DgnElementId id) { _OnElementInserted(id); }
void ILocksManager::OnModelInserted(DgnModelId id) { _OnModelInserted(id); }
LockStatus ILocksManager::LockElement(DgnElementCR el, LockLevel lvl) { return _LockElement(el, lvl); }
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
LockStatus ILocksManager::_LockElement(DgnElementCR el, LockLevel level)
    {
    // We don't acquire locks for indirect changes.
    if (TxnManager::Mode::Indirect == GetDgnDb().Txns().GetMode())
        return LockStatus::Success;

    LockRequest request;
    request.Insert(el, level);
    return AcquireLocks(request);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus ILocksManager::_LockModel(DgnModelCR model, LockLevel level)
    {
    // We don't acquire locks for indirect changes.
    if (TxnManager::Mode::Indirect == GetDgnDb().Txns().GetMode())
        return LockStatus::Success;

    LockRequest request;
    request.Insert(model, level);
    return AcquireLocks(request);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockStatus ILocksManager::LockDb(LockLevel level)
    {
    LockRequest request;
    request.Insert(GetDgnDb(), level);
    return AcquireLocks(request);
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

    virtual bool _QueryLocksHeld(LockRequestCR, bool) override { return true; }
    virtual LockStatus _AcquireLocks(LockRequestCR) override { return LockStatus::Success; }
    virtual LockStatus _RelinquishLocks() override { return LockStatus::Success; }
    virtual void _OnElementInserted(DgnElementId) override { }
    virtual void _OnModelInserted(DgnModelId) override { }
    virtual LockStatus _LockElement(DgnElementCR, LockLevel) override { return LockStatus::Success; }
    virtual LockStatus _LockModel(DgnModelCR, LockLevel) override { return LockStatus::Success; }
public:
    static ILocksManagerPtr Create(DgnDbR db) { return new UnrestrictedLocksManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
ILocksManagerPtr DgnPlatformLib::Host::LocksAdmin::_CreateLocksManager(DgnDbR db) const
    {
    // NEEDSWORK: Currently we have no way of determining if locking is required for a given DgnDb...
    return UnrestrictedLocksManager::Create(db);
    }


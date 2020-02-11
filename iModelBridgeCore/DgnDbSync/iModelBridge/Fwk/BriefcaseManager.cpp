/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include <DgnPlatform/RepositoryManager.h>
#include <WebServices/iModelHub/Client/iModelManager.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_LOGGING

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridgeFwk"))

#define MUST_INITIALIZE_LOCK_CACHE                          \
    if (RepositoryStatus::Success != InitializeLockCache()) \
        return m_lockCacheInitializeStatus;

#define MUST_INITIALIZE_LOCK_CACHE_RETURN(__ERR_RET__)      \
    if (RepositoryStatus::Success != InitializeLockCache()) \
        return __ERR_RET__;

namespace
{

//=======================================================================================
// @bsistruct                                                    Sam.Wilson      7/2019
//=======================================================================================
struct OwnedLocksIterator : IOwnedLocksIterator
{
    DgnLockSet m_locks;
    DgnLockSet::const_iterator m_i;

    OwnedLocksIterator(DgnLockSet const &l) : m_locks(l), m_i(m_locks.begin()) {}

    DgnLockCR _GetCurrent() const override { return *m_i; }
    bool _IsAtEnd() const override { return m_i == m_locks.end(); }
    void _Reset() override { m_i = m_locks.begin(); }
    void _MoveNext() override { ++m_i; }
};

//=======================================================================================
// @bsistruct                                                    Sam.Wilson      7/2019
//=======================================================================================
struct BriefcaseManager : IBriefcaseManager, TxnMonitor
{
    bmap<DgnModelId, DgnElementId> m_normalChannelParents;
    DgnLockSet m_heldLocks;
    Request m_req; // locks and codes that we must acquire before we can say that update has succeeded
    int m_inBulkUpdate = 0;
    bool m_lockCacheInitialized = false;
    RepositoryStatus m_lockCacheInitializeStatus;

    BriefcaseManager(DgnDbR db) : IBriefcaseManager(db)
    {
        // Set default policies for bridges
        m_channelProps.reportCodesInLockedModels = false;   // I take care of obtaining locks when and as necessary before calling push
        m_channelProps.includeUsedLocksInChangeSet = false; // by default, don't clutter up a push with reporting codes that are private to the bridge. A bridge can override this.
        m_channelProps.stayInChannel = true;                // bridge code must be restricted to write to shared definition models *only* in the Shared channel and to bridge models *only* in the Normal channel
        m_channelProps.oneBriefcaseOwnsChannel = false;     // A given briefcase does *not* hold onto channel locks persistently
        T_HOST.GetTxnAdmin().AddTxnMonitor(*this);
    }

    ~BriefcaseManager()
    {
        T_HOST.GetTxnAdmin().DropTxnMonitor(*this);
    }

    Utf8String FmtCode(DgnCodeCR code)
    {
        if (!code.IsValid())
            return "";
        return Utf8PrintfString("Spec: %llx, Scope: %s, Value: %s", code.GetCodeSpecId().GetValue(), code.GetScopeString().c_str(), code.GetValueUtf8CP());
    }

    Utf8String FmtModel(DgnModelCR m)
    {
        return Utf8PrintfString("%s (0x%llx)", m.GetName().c_str(), m.GetModelId().GetValueUnchecked());
    }

    Utf8String FmtElement(DgnElementCR el)
    {
        auto ecclass = el.GetElementClass();
        Utf8String ecclassName(ecclass ? ecclass->GetName() : "?");
        Utf8PrintfString elMsg("%s ID=0x%llx Code=%s", ecclassName.c_str(), el.GetElementId().GetValueUnchecked(), FmtCode(el.GetCode()).c_str());

        if (el.GetModel()->GetModeledElementId() == el.GetElementId())
            return elMsg;
        return elMsg.append(", Model=").append(FmtModel(*el.GetModel()));
    }

    RepositoryStatus ReportChannelError(DgnElementCR el, Utf8StringCR reason)
    {
        BeAssert(false);
        LOG.fatalv("Channel error - %s - %s", FmtElement(el).c_str(), reason.c_str());
        return RepositoryStatus::InvalidRequest;
    }

    RepositoryStatus ReportChannelError(Utf8StringCR reason)
    {
        BeAssert(false);
        LOG.fatalv("Channel error - %s", reason.c_str());
        return RepositoryStatus::InvalidRequest;
    }

    RepositoryStatus InitializeLockCache()
    {
        if (m_lockCacheInitialized)
            return m_lockCacheInitializeStatus;

        auto mgr = GetRepositoryManager();
        if (nullptr == mgr)
            return RepositoryStatus::ServerUnavailable;

        m_lockCacheInitialized = true;
        m_lockCacheInitializeStatus = mgr->QueryHeldLocks(m_heldLocks, GetDgnDb());

        if (RepositoryStatus::Success != m_lockCacheInitializeStatus)
            return m_lockCacheInitializeStatus;

        return m_lockCacheInitializeStatus;
    }

    void ClearLockCache()
    {
        m_lockCacheInitialized = false;
        m_heldLocks.clear();
    }

    DgnElementId ComputeNormalChannelParentOfElement(DgnElementCR el)
    {
        auto i = m_normalChannelParents.find(el.GetModelId());
        if (i != m_normalChannelParents.end())
            return i->second;

        auto& channelParentId = m_normalChannelParents[el.GetModelId()];

        iModelBridge::JobMemberInfo jobMemberInfo = iModelBridge::ComputeJobMemberInfo(el);
        if (jobMemberInfo.m_jobSubject.IsValid())
            channelParentId = jobMemberInfo.m_jobSubject->GetElementId();

        return channelParentId;
    }

    DgnElementId _GetNormalChannelParentOf(DgnModelId mid, DgnElementCP el) override
    {
        auto i = m_normalChannelParents.find(mid);
        if (i != m_normalChannelParents.end())
            return i->second;

        if (el != nullptr)
        {
            return ComputeNormalChannelParentOfElement(*el);
        }

        auto model = GetDgnDb().Models().GetModel(mid);
        if (!model.IsValid())
        {
            BeAssert(false);
            return DgnElementId();
        }
        auto modeledElement = model->GetModeledElement();
        if (!modeledElement.IsValid())
        {
            BeAssert(false);
            return DgnElementId();
        }
        return ComputeNormalChannelParentOfElement(*modeledElement);
    }

    bool IsLockRequiredById(DgnElementId elementId)
    {
        auto el = GetDgnDb().Elements().GetElement(elementId);
        return el.IsValid() ? IsLockRequired(*el) : false;
    }

    bool _IsLockRequired(DgnElementCR element) override
    {
        return !GetNormalChannelParentOf(element).IsValid(); // only need locks on elements outside of bridge job's channel
    }

    RepositoryStatus AcquireLocks(LockRequest &req)
    {
        MUST_INITIALIZE_LOCK_CACHE

        RemoveHeldLocks(req.GetLockSet());

        DgnLockSet locksAcquired;
        for (auto const &lock : req)
            locksAcquired.insert(lock);

        auto stat = IBriefcaseManager::AcquireLocks(req, ResponseOptions::All).Result();
        if (RepositoryStatus::Success != stat)
            return stat;

        m_heldLocks.insert(locksAcquired.begin(), locksAcquired.end());

        return RepositoryStatus::Success;
    }

    RepositoryStatus AcquireLock(DgnLockCR lock)
    {
        LockRequest req;
        req.GetLockSet().insert(lock);
        return AcquireLocks(req);
    }

    // RepositoryStatus AcquireLock(DgnElementCR el, LockLevel level)
    // {
    //     LockRequest req;
    //     req.Insert(el, level);
    //     return AcquireLocks(req);
    // }

    // RepositoryStatus AcquireLock(DgnModelCR model, LockLevel level)
    // {
    //     LockRequest req;
    //     req.Insert(model, level);
    //     return AcquireLocks(req);
    // }

    bool NeedsUpgrade(DgnLock const &requestedLock, LockLevel alreadyHeldLevel)
    {
        if (alreadyHeldLevel >= requestedLock.GetLevel())
            return false;

        if (requestedLock.GetType() != LockableType::Element)
            return true;

        return IsLockRequiredById(DgnElementId(requestedLock.GetId().GetValue()));
    }

    // Remove each lock from `locks` if it matches an item in `alreadyHeld` with the same or higher level
    void RemoveLocksNeedingNoUpgrade(DgnLockSet &locks, DgnLockSet const &alreadyHeld)
    {
        for (auto iLock = locks.begin(); iLock != locks.end();)
        {
            auto iHeld = alreadyHeld.find(*iLock);
            if ((iHeld != alreadyHeld.end()) && !NeedsUpgrade(*iLock, iHeld->GetLevel()))
                iLock = locks.erase(iLock);
            else
                ++iLock;
        }
    }

    void RemoveHeldLocks(DgnLockSet &locks)
    {
        BeAssert(m_lockCacheInitialized);
        RemoveLocksNeedingNoUpgrade(locks, m_heldLocks);
    }

    void RemoveScheduledLocks(DgnLockSet &locks)
    {
        if (!m_req.Locks().GetLockSet().empty())
            RemoveLocksNeedingNoUpgrade(locks, m_req.Locks().GetLockSet());
    }

    RepositoryStatus CheckLockRequest(DgnLockSet const &locks)
    {
        if (IsNoChannel())
            return RepositoryStatus::Success; // It's probably too early in the DgnDb opening process to know the channel

        for (auto const &lock : locks)
        {
            if (IsSharedChannel())
            {
                // Nothing to check here.
                //  The rule is that the bridge must not write to its Job-specific models during the definitions phase.
                //  The _PrepareForElement/ModelOperation callbacks check for this, and I am pretty sure that those
                //  functions cover all the cases that are likely to come up.
            }
            else
            {
                if (lock.GetType() == LockableType::Schemas)
                    return ReportChannelError("Bridges must not attempt to import schemas during the data phase.");

                if (lock.GetType() == LockableType::CodeSpecs)
                {
                    LOG.warning("The bridge is inserting CodeSpecs during the data phase. All CodeSpecs should be created during the definitions phase.");
                    // return RepositoryStatus::InvalidRequest;     *** NEEDS WORK: must allow this for now, as existing bridges do this.
                }
            }
        }
        return RepositoryStatus::Success;
    }

    Response _ProcessRequest(Request &req, RequestPurpose purpose) override
    {
        MUST_INITIALIZE_LOCK_CACHE_RETURN(Response(purpose, req.Options(), RepositoryStatus::ServerUnavailable))

        RemoveHeldLocks(req.Locks().GetLockSet());

        IBriefcaseManager::Response resp(purpose, req.Options(), RepositoryStatus::Success);

        if (req.IsEmpty())
            return resp;

        auto cstatus = CheckLockRequest(req.Locks().GetLockSet());
        if (RepositoryStatus::Success != cstatus)
            return Response(purpose, req.Options(), cstatus);

        if (RequestPurpose::Acquire == purpose && m_inBulkUpdate)
        {
            AccumulateRequests(req);
            return resp;
        }

        auto mgr = GetRepositoryManager();
        if (nullptr == mgr)
            return Response(purpose, req.Options(), RepositoryStatus::ServerUnavailable);

        resp = (RequestPurpose::Acquire == purpose)
                   ? mgr->Acquire(req, GetDgnDb())
                   : mgr->QueryAvailability(req, GetDgnDb());

        if (RepositoryStatus::LockAlreadyHeld == resp.Result())
            return resp;

        if (RepositoryStatus::Success == resp.Result())
            ClearLockCache();

        return resp;
    }

    RepositoryStatus _Relinquish(Resources which) override
    {
        if (m_inBulkUpdate)
            return ReportChannelError("A bridge must not attempt to relinquish codes while in bulk update mode.");

        IRepositoryManagerP server;
        RepositoryStatus stat;

        if (nullptr == (server = GetRepositoryManager()))
            return RepositoryStatus::ServerUnavailable;

        stat = server->Relinquish(which, GetDgnDb());

        ClearLockCache();

        return stat;
    }

    LockLevel GetHeldLockLevel(DgnLock const &lock)
    {
        BeAssert(m_lockCacheInitialized);
        auto i = m_heldLocks.find(lock);
        return (i == m_heldLocks.end()) ? LockLevel::None : i->GetLevel();
    }

    void AddElementsInModel(DgnLockSet &locks, DgnModelId mid)
    {
        for (auto const &lock : m_heldLocks)
        {
            if (lock.GetType() != LockableType::Element)
                continue;
            auto el = GetDgnDb().Elements().GetElement(DgnElementId(lock.GetId().GetValue()));
            if (el.IsValid() && (el->GetModelId() == mid))
            {
                locks.insert(DgnLock(el->GetElementId(), LockLevel::None));
            }
        }
    }

    DgnElementCPtr GetLockedElement(DgnLockCR lock)
    {
        return GetDgnDb().Elements().GetElement(DgnElementId(lock.GetLockableId().GetId().GetValue()));
    }

    RepositoryStatus _Demote(DgnLockSet &locks, DgnCodeSet const &codes) override
    {
        if (codes.size() != 0)
            return ReportChannelError("Bridges must not release reserved Codes directly. Unused Codes are released automatically.");

        MUST_INITIALIZE_LOCK_CACHE

#ifndef NDEBUG
        for (auto const &lock : locks)
        {
            BeAssert(m_heldLocks.find(lock) != m_heldLocks.end());
        }
#endif

        IRepositoryManagerP mgr;
        if (nullptr == (mgr = GetRepositoryManager()))
            return RepositoryStatus::ServerUnavailable;

        // Cull any locks which we known are already at OR BELOW the desired level (this function cannot *increase* a lock's level...)
        for (auto iter = locks.begin(); iter != locks.end(); /**/)
        {
            DgnLock &lock = *iter;
            LockLevel curLevel;
            if (GetHeldLockLevel(lock) <= lock.GetLevel())
                iter = locks.erase(iter);
            else
                ++iter;
        }

        if (locks.empty())
            return RepositoryStatus::Success;

        // Enforce bridge locking rules
        if (locks.end() != locks.find(DgnLock(LockableId(GetDgnDb()), LockLevel::None)))
            return ReportChannelError("A bridge must never release its shared lock on the Db");

        DgnLockSet additionalLocks;
        for (auto const &lock : locks)
        {
            if (GetHeldLockLevel(lock) == LockLevel::Exclusive)
            {
                if (LockableType::Model == lock.GetType())
                {
                    return ReportChannelError("A bridge must never release or demote its exclusive model locks.");
                }
                else
                {
                    if (LockableType::Element == lock.GetType())
                    {
                        auto lockedElement = GetLockedElement(lock);
                        if (lockedElement.IsValid())
                        {
                            auto jobMemberInfo = iModelBridge::ComputeJobMemberInfo(*lockedElement);
                            if (jobMemberInfo.IsJobOrChild())
                            {
                                return ReportChannelError("A bridge must never release or demote its exclusive element or model locks.");
                            }
                        }
                    }
                }
            }
            else // Release Shared lock
            {
                BeAssert(GetHeldLockLevel(lock) == LockLevel::Shared);

                if (LockableType::Model == lock.GetType()) // If we're releasing a shared model lock, we must also release locks on any elements within the model
                    AddElementsInModel(additionalLocks, DgnModelId(lock.GetId().GetValue()));
            }
        }

        locks.insert(additionalLocks.begin(), additionalLocks.end());

        RepositoryStatus stat = mgr->Demote(locks, codes, GetDgnDb());
        if (RepositoryStatus::Success != stat)
            return stat;

        ClearLockCache();
        return RepositoryStatus::Success;
    }

    RepositoryStatus CheckWriteToModelInSharedChannel(DgnElementCR el, bool exclusiveModel, iModelBridge::JobMemberInfo const &jobMemberInfo)
    {
        BeAssert(IsSharedChannel());

        if (GetChannelProps().isInitializingChannel) // (To avoid breaking existing code, allow the bridge to create its breakdown structure at job *initialization* time.)
            return RepositoryStatus::Success;

        if (exclusiveModel)
            return ReportChannelError(el, "During the definitions phase, a bridge must not write to its Job-specific models.");

        // Bridge is writing to a shared model.

        if (jobMemberInfo.IsChildOfJob())
            return ReportChannelError(el, "During the definitions phase, a bridge must not modify its Job-specific elements. It may modify its Job Subject element.");

        return RepositoryStatus::Success;
    }

    bool IsAssignedToChannel(DgnElementCR el)
    {
        //  A RepositoryLink element is formally in the shared channel and is not owned by any one bridge.
        //  However, there is one bridge that owns the content of the file identified by a given RepositoryLink element, and that bridge
        //  needs to be able to update the properties of the RepositoryLink element when it finishes an update.
        return (nullptr != dynamic_cast<RepositoryLink const *>(&el));
    }

    RepositoryStatus CheckWriteToModelInNormalChannel(DgnElementCR el, bool exclusiveModel, iModelBridge::JobMemberInfo const &jobMemberInfo)
    {
        BeAssert(IsNormalChannel());

        if (!exclusiveModel) // If bridge is *not* writing to one of its own models ...
        {
            if (!jobMemberInfo.IsChildOfJob()) // If bridge is writing to an element that is *not* a child of the bridge's Job Subject ...
            {
                if (!IsAssignedToChannel(el)) // and if that element is not "assigned" to the bridge's channel ...
                    return ReportChannelError(el, "During the data phases, a bridge may only write to its Job-specific elements (not including the Job Subject element itself).");
            }

            BeAssert(el.GetElementId() != DgnModel::RepositoryModelId());
        }

        return RepositoryStatus::Success;
    }

    RepositoryStatus CheckWriteToModel(DgnElementCR el, iModelBridge::JobMemberInfo const &jobMemberInfo)
    {
        BeAssert(!IsNoChannel());

        bool exclusiveModel = GetNormalChannelParentOf(el).IsValid();

        if (IsSharedChannel())
            return CheckWriteToModelInSharedChannel(el, exclusiveModel, jobMemberInfo);

        return CheckWriteToModelInNormalChannel(el, exclusiveModel, jobMemberInfo);
    }

    RepositoryStatus CheckModelInsertInSharedChannel(DgnElementCR modeledElement, iModelBridge::JobMemberInfo const &jobMemberInfo)
    {
        BeAssert(IsSharedChannel());

        // In the shared channel, the channel "parent" is element #1

        if (jobMemberInfo.IsChildOfJob()) // Bridge is inserting a model in its Job-specific hierarchy
        {
            if (!GetChannelProps().isInitializingChannel) // To avoid breaking existing code, allow the bridge to create its breakdown structure at job *initialization* time.
                return ReportChannelError(modeledElement, "During the definitions phase, a bridge may not insert Job-specific models.");
        }
        else // A Bridge is inserting a model outside of its private hierarchy
        {
            if (modeledElement.GetModelId() != modeledElement.GetDgnDb().GetDictionaryModel().GetModelId())
                {
                // Make a special case for PresentationRules.
                if (modeledElement.GetCode().GetValue().Equals("PresentationRules"))
                    {}
                else
                    return ReportChannelError(modeledElement, "During the definitions phase, a bridge may not insert top-level shared models. A bridge may add a sub-model of an element in the DictionaryModel only.");
                }
        }
        return RepositoryStatus::Success;
    }

    RepositoryStatus CheckModelInsertInNormalChannel(DgnElementCR modeledElement, iModelBridge::JobMemberInfo const &jobMemberInfo)
    {
        BeAssert(IsNormalChannel());

        if (!jobMemberInfo.IsChildOfJob() && !IsAssignedToChannel(modeledElement)) // (the submodel of any RepositoryLink element is open for writes by any bridge any time)
            return ReportChannelError(modeledElement, "During the data phases, a bridge may insert Job-specific models only. Top-level models and sub-models of dictionary elements are off-limits.");

        return RepositoryStatus::Success;
    }

    RepositoryStatus CheckModelInsert(DgnElementCR modeledElement, iModelBridge::JobMemberInfo const &jobMemberInfo)
    {
        if (IsSharedChannel())
            return CheckModelInsertInSharedChannel(modeledElement, jobMemberInfo);

        return CheckModelInsertInNormalChannel(modeledElement, jobMemberInfo);
    }

    RepositoryStatus CheckCodeScope(DgnCodeCR code, DgnElementCR el, iModelBridge::JobMemberInfo const &elementJobMemberInfo)
    {
        if (!code.IsValid())
            return RepositoryStatus::Success;

        // These checks apply to both Shared and Normal channels

        auto scopeElement = GetDgnDb().Elements().GetElement(code.GetScopeElementId(GetDgnDb()));
        if (!scopeElement.IsValid())
        {
            if (elementJobMemberInfo.IsChildOfJob())
                return ReportChannelError(el, Utf8PrintfString("Incorrectly scoped code. Code=%s. The Code of an element in the bridge's Job hierarchy must be scoped to an element that is inside that hierarchy.", FmtCode(code).c_str()));
            return RepositoryStatus::Success; // there is no restriction on how the Codes of elements outside the bridge's hierarhcy most be scoped
        }

        // The Code is scoped to an element

        auto scopeJobMemberInfo = iModelBridge::ComputeJobMemberInfo(*scopeElement);

        if (!elementJobMemberInfo.IsChildOfJob()) // This is a Code for an element outside of the bridge's private hierarchy.
        {
            if (scopeJobMemberInfo.IsJobOrChild())
                return ReportChannelError(el, Utf8PrintfString("Element %s is an invalid scope. The Code of an element that is outside of the bridge's Job hierarchy must not be scoped to an element that is inside that hierarchy.",
                                                               FmtElement(*scopeElement).c_str()));
        }
        else // This is a Code for an element inside the bridge's private hierarchy.
        {
            if (!scopeJobMemberInfo.IsJobOrChild())
                return ReportChannelError(el, Utf8PrintfString("Element %s is an invalid scope. The Code of an element that is inside the bridge's Job hierarchy must be scoped to an element that is also inside that hierarchy.",
                                                               FmtElement(*scopeElement).c_str()));
        }
        return RepositoryStatus::Success;
    }

    RepositoryStatus _PrepareForElementOperation(Request &req, DgnElementCR el, BeSQLite::DbOpcode op) override
    {
        MUST_INITIALIZE_LOCK_CACHE

        auto rstat = el.PopulateRequest(req, op);
        if (RepositoryStatus::Success != rstat)
            return rstat;

        iModelBridge::JobMemberInfo jobMemberInfo = iModelBridge::ComputeJobMemberInfo(el);

        if (RepositoryStatus::Success != (rstat = CheckWriteToModel(el, jobMemberInfo)))
            return rstat;

        for (auto const &code : req.Codes())
        {
            if (RepositoryStatus::Success != (rstat = CheckCodeScope(code, el, jobMemberInfo)))
                return rstat;
        }

        // Filter out resources that are not required
        if (!IsLockRequired(el))
            req.Locks().Remove(LockableId(el)); // We don't require locks on elements in Job-specific models.

        if (jobMemberInfo.IsChildOfJob())
            req.Codes().clear(); // We don't reserve or report the Codes of elements in Job-specific models.

        return RepositoryStatus::Success;
    }

    RepositoryStatus _PrepareForModelOperation(Request &req, DgnModelCR model, BeSQLite::DbOpcode op) override
    {
        MUST_INITIALIZE_LOCK_CACHE

        auto rstat = model.PopulateRequest(req, op);
        if (RepositoryStatus::Success != rstat)
            return rstat;

        // Check that this operation is permitted
        auto modeledElement = model.GetModeledElement();
        if (!modeledElement.IsValid())
        {
            BeAssert(false);
            return RepositoryStatus::InvalidRequest;
        }

        auto jobMemberInfo = iModelBridge::ComputeJobMemberInfo(*modeledElement);

        rstat = (BeSQLite::DbOpcode::Insert == op) ? CheckModelInsert(*modeledElement, jobMemberInfo) : CheckWriteToModel(*modeledElement, jobMemberInfo);

        if (RepositoryStatus::Success != rstat)
            return rstat;

        return RepositoryStatus::Success;
    }

    RepositoryStatus _PerformPrepareAction(Request &req, PrepareAction action) override
    {
        if (!m_inBulkUpdate || (PrepareAction::Acquire == action))
            return IBriefcaseManager::_PerformPrepareAction(req, action);

        if (m_inBulkUpdate && (PrepareAction::Verify == action))
        {
            // The app is signalling that it intends to carry out this operation. The request is to verify that it can be done legally.
            // But the app is running in bulk update mode. It did not request these resources. Instead, it
            // expects all required resources to be accumlated and then acquired in one shot when the transaction is closed.
            // Therefore "verify" = accumlate.
            AccumulateRequests(req);
        }

        return RepositoryStatus::Success;
    }

    void _OnElementInserted(DgnElementId id) override
    {
        MUST_INITIALIZE_LOCK_CACHE_RETURN(;)

        if (!IsLockRequiredById(id))
            return;

        DgnLock lock(LockableId(id), LockLevel::Exclusive);
        if (m_inBulkUpdate)
            m_req.Locks().GetLockSet().insert(lock);
        else
            AcquireLock(lock);
    }

    void _OnModelInserted(DgnModelId id) override
    {
        MUST_INITIALIZE_LOCK_CACHE_RETURN(;)

        DgnLock lock(LockableId(id), LockLevel::Exclusive);

        if (m_inBulkUpdate)
            m_req.Locks().GetLockSet().insert(lock);
        else
            AcquireLock(lock);
    }

    RepositoryStatus _ReserveCode(DgnCodeCR code) override
    {
        // While I cannot verify if this code is legal here, I will get another chance
        //  when _PrepareForElementOperation on the element that attempts to use it.

        if (m_inBulkUpdate)
        {
            m_req.Codes().insert(code);
            return RepositoryStatus::Success;
        }

        DgnCodeSet codes;
        codes.insert(code);
        return ReserveCodes(codes).Result();
    }

    void _OnDgnDbDestroyed() override
    {
        m_req.Reset();
        m_inBulkUpdate = false;
        ClearLockCache();
        IBriefcaseManager::_OnDgnDbDestroyed();
    }

    RepositoryStatus _ClearUserHeldCodesLocks() override
    {
        ClearLockCache();
        return RepositoryStatus::Success;
    }

    RepositoryStatus _OnFinishRevision(DgnRevision const &rev) override
    {
        if (m_inBulkUpdate)
        {
            BeAssert(false);
            return RepositoryStatus::PendingTransactions;
        }
        return RepositoryStatus::Success;
    }

    void _OnCommit(TxnManager &mgr) override
    {
        BeAssert(!m_inBulkUpdate && "somebody called SaveChanges while in a NESTED bulk op");
    }

    void _OnAppliedChanges(TxnManager &mgr) override
    {
        // This event is invoked whenver any DgnDb is updated, not just mine.
        if (&GetDgnDb() != &mgr.GetDgnDb())
            return;
        m_inBulkUpdate = 0;
        m_req.Reset();
        ClearLockCache();
    }

    void _OnUndoRedo(TxnManager &mgr, TxnAction) override
    {
        // This event is invoked whenver any DgnDb is updated, not just mine.
        if (&GetDgnDb() != &mgr.GetDgnDb())
            return;
        m_inBulkUpdate = 0;
        m_req.Reset();
        ClearLockCache();
    }

    bool _AreResourcesHeld(DgnLockSet &locks, DgnCodeSet &codes, RepositoryStatus *statusOut) override
    {
        RepositoryStatus status_;
        RepositoryStatus *status = statusOut ? statusOut : &status_;

        *status = RepositoryStatus::ServerUnavailable;
        MUST_INITIALIZE_LOCK_CACHE_RETURN(false)

        RemoveHeldLocks(locks);
        RemoveScheduledLocks(locks);
        if (!locks.empty())
        {
            *status = RepositoryStatus::LockNotHeld;
            return false;
        }

        // The following code is commented out because there is almost no need to check if codes are reserved,
        //      and the cost of supporting such verification is not worth it.
        //      Skipping the verification does not endanger the iModel. iModelHub/Bank will tell us soon enough if a bridge
        //      has tried to use a Code that is already reserved or marked as in use. If that is the case,
        //      iModelHub/Bank will abort the push. If we did the check here, then we would abort the call to SaveChanges.
        //      Note that Code collisions can only happen on elements outside the bridge's channel.
        //      Note that, if a bridge wants to know if a Code is used, it can do a pretty good check just by looking it up
        //      in the briefcase.
        // if (!codes.empty()) // TODO - cache Codes reserved by this briefcase and answer this query from the cache. Build that cache only on demand. Accumulate reservations as they are made.
        // {
        //     DgnCodeInfoSet codeStates;
        //     if (RepositoryStatus::Success != (*status = QueryCodeStates(codeStates, codes)))
        //         return false;

        //     for (auto const &codeState : codeStates)
        //     {
        //         if (codeState.GetReservedBy() != GetDgnDb().GetBriefcaseId())
        //         {
        //             *status = RepositoryStatus::CodeNotReserved;
        //             return false;
        //         }
        //     }
        // }

        *status = RepositoryStatus::Success;
        return true;
    }

    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet &states, DgnCodeSet const &codes) override
    {
        auto server = GetRepositoryManager();
        return nullptr != server ? server->QueryCodeStates(states, codes) : RepositoryStatus::ServerUnavailable;
    }

    // Query the levels of all locks held by this briefcase.
    RepositoryStatus _QueryLockLevels(DgnLockSet &levels, LockableIdSet &lockIds) override
    {
        MUST_INITIALIZE_LOCK_CACHE

        for (auto const &lockId : lockIds)
        {
            DgnLock lock(lockId, LockLevel::None);
            auto i = m_heldLocks.find(lock);
            if (i != m_heldLocks.end())
                lock = *i;
            levels.insert(lock);
        }
        return RepositoryStatus::Success;
    }

    RepositoryStatus _QueryLockLevel(LockLevel &level, LockableId lockId) override
    {
        DgnLockSet levels;
        LockableIdSet ids;
        ids.insert(lockId);
        auto stat = QueryLockLevels(levels, ids);
        BeAssert(RepositoryStatus::Success != stat || 1 == levels.size());
        level = (RepositoryStatus::Success == stat && 1 == levels.size()) ? levels.begin()->GetLevel() : LockLevel::None;
        return stat;
    }

    IOwnedLocksIteratorPtr _GetOwnedLocks(FastQuery fast) override
    {
        MUST_INITIALIZE_LOCK_CACHE_RETURN(nullptr)
        return new OwnedLocksIterator(m_heldLocks);
    }

    RepositoryStatus _RefreshFromRepository() override
    {
        ClearLockCache();
        MUST_INITIALIZE_LOCK_CACHE
        return RepositoryStatus::Success;
    }

    void _StartBulkOperation() override { ++m_inBulkUpdate; }

    bool _IsBulkOperation() const override { return 0 != m_inBulkUpdate; }

    Response _EndBulkOperation() override
    {
        if (m_inBulkUpdate <= 0)
        {
            BeAssert(false);
            return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::InvalidRequest);
        }

        --m_inBulkUpdate;

        if (0 != m_inBulkUpdate)
            return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::Success);

        m_req.SetOptions(ResponseOptions::All); // Get details when requests such as code reservations fail.
        auto resp = _ProcessRequest(m_req, RequestPurpose::Acquire);

        if (RepositoryStatus::Success != resp.Result())
            return resp;

        m_req.Reset();

        return resp;
    }

    void _ExtractRequestFromBulkOperation(Request &reqOut, bool locks, bool codes) override
    {
        if (codes)
        {
            reqOut.Codes().insert(m_req.Codes().begin(), m_req.Codes().end());
            m_req.Codes().clear();
        }

        if (locks)
        {
            reqOut.Locks().GetLockSet().insert(m_req.Locks().GetLockSet().begin(), m_req.Locks().GetLockSet().end());
            m_req.Locks().Clear();
        }

        // TODO: merge options
        // reqOut.SetOptions(m_req.Options());
    }

    void AccumulateRequests(Request const &req)
    {
        BeAssert(m_inBulkUpdate);
        m_req.Codes().insert(req.Codes().begin(), req.Codes().end());
        m_req.Locks().GetLockSet().insert(req.Locks().GetLockSet().begin(), req.Locks().GetLockSet().end());
    }
};

/*---------------------------------------------------------------------------------**/ /**
* @bsistruct                                                    Sam.Wilson      7/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct MasterBriefcaseManager : IBriefcaseManager
{
private:
    MasterBriefcaseManager(DgnDbR db) : IBriefcaseManager(db) {}

    Response _ProcessRequest(Request &req, RequestPurpose purpose) override { return Response(purpose, req.Options(), RepositoryStatus::Success); }
    RepositoryStatus _Demote(DgnLockSet &, DgnCodeSet const &) override { return RepositoryStatus::Success; }
    RepositoryStatus _Relinquish(Resources) override { return RepositoryStatus::Success; }
    RepositoryStatus _ReserveCode(DgnCodeCR) override { return RepositoryStatus::Success; }
    RepositoryStatus _QueryLockLevel(LockLevel &level, LockableId lockId) override
    {
        level = LockLevel::Exclusive;
        return RepositoryStatus::Success;
    }
    IOwnedLocksIteratorPtr _GetOwnedLocks(FastQuery fast) override { return nullptr; }
    RepositoryStatus _OnFinishRevision(DgnRevision const &) override { return RepositoryStatus::Success; }
    RepositoryStatus _RefreshFromRepository() override { return RepositoryStatus::Success; }
    RepositoryStatus _ClearUserHeldCodesLocks() override { return RepositoryStatus::Success; }
    void _OnElementInserted(DgnElementId) override {}
    void _OnModelInserted(DgnModelId) override {}
    void _StartBulkOperation() override {}
    bool _IsBulkOperation() const override { return false; }
    Response _EndBulkOperation() override { return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::Success); }

    RepositoryStatus _QueryLockLevels(DgnLockSet &levels, LockableIdSet &lockIds) override
    {
        for (auto const &id : lockIds)
            levels.insert(DgnLock(id, LockLevel::Exclusive));

        return RepositoryStatus::Success;
    }
    bool _AreResourcesHeld(DgnLockSet &, DgnCodeSet &, RepositoryStatus *status) override
    {
        if (nullptr != status)
            *status = RepositoryStatus::Success;
        return true;
    }
    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet &states, DgnCodeSet const &codes) override
    {
        auto bcId = GetDgnDb().GetBriefcaseId();
        for (auto const &code : codes)
            states.insert(DgnCodeInfo(code)).first->SetReserved(bcId);
        return RepositoryStatus::Success;
    }
    RepositoryStatus _PrepareForElementOperation(Request &, DgnElementCR, BeSQLite::DbOpcode) override { return RepositoryStatus::Success; }
    RepositoryStatus _PrepareForModelOperation(Request &, DgnModelCR, BeSQLite::DbOpcode) override { return RepositoryStatus::Success; }

public:
    static IBriefcaseManagerPtr Create(DgnDbR db) { return new MasterBriefcaseManager(db); }
};

} // anonymous namespace

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                                    Sam.Wilson      7/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IBriefcaseManagerPtr
iModelBridgeFwk::FwkRepoAdmin::_CreateBriefcaseManager(DgnDbR db) const
{
    static bool s_useNewBcMgr;
    static bool s_checkedFF;

    if (!s_checkedFF)
    {
        s_checkedFF = true;
        m_fwk.TestFeatureFlag("imodel-bridge-fwk-briefcase-manager", s_useNewBcMgr);

        if (getenv("imodel-bridge-fwk-briefcase-manager") != nullptr) // allow envvar to override
            s_useNewBcMgr = true;

        if (s_useNewBcMgr)
        {
            LOG.info("Using new iModelBridgeFwk BriefcaseManager");
            printf("Using new iModelBridgeFwk BriefcaseManager\n");
        }
    }

    if (!s_useNewBcMgr)
    {
        //  -- old briefcase manager --
        return DgnPlatformLib::Host::RepositoryAdmin::_CreateBriefcaseManager(db);
    }

    //  -- new briefcase manager --
    if (db.IsMasterCopy() || db.IsStandaloneBriefcase())
        return MasterBriefcaseManager::Create(db);

    return new BriefcaseManager(db);
}

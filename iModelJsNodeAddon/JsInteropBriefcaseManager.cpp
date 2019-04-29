/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"

namespace IModelJsNative {

static bool s_okEndBulkMode;

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
    bool _IsCodeInLockedScope(DgnCodeCR) const override {return true;}

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

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct BriefcaseManager : IBriefcaseManager, TxnMonitor
    {
    Request m_req;      // locks and codes that we must acquire before we can say that update has succeeded
    int m_inBulkUpdate = 0;

    BriefcaseManager(DgnDbR db) : IBriefcaseManager(db)
        {
        T_HOST.GetTxnAdmin().AddTxnMonitor(*this);
        }

    ~BriefcaseManager()
        {
        T_HOST.GetTxnAdmin().DropTxnMonitor(*this);
        }

    Response _ProcessRequest(Request& req, RequestPurpose purpose) override
        {
        if (req.IsEmpty())
            return Response(purpose, req.Options(), RepositoryStatus::Success);

        auto control = GetDgnDb().GetConcurrencyControl();
        if (nullptr != control)
            control->_OnProcessRequest(req, *this, purpose);

        IBriefcaseManager::Response resp(purpose, req.Options(), RepositoryStatus::Success);

        if (RequestPurpose::Acquire == purpose)
            {
            if (m_inBulkUpdate)
                AccumulateRequests(req);
            else
                {
                JsInterop::ThrowJsException("Cannot acquire resources in native code. See startBulkOperation.");
                BeAssert(req.IsEmpty() && "Cannot acquire resources in native code. See startBulkOperation.");
                }
            }
        else
            {
            // TODO: Forward this query to TypeScript?
            }

        if (nullptr != control)
            control->_OnProcessedRequest(req, *this, purpose, resp);

        return resp;
        }

    RepositoryStatus _Relinquish(Resources) override
        {
        BeAssert(false && "Cannot process requests from native code. See startBulkOperation.");
        return RepositoryStatus::ServerUnavailable;
        }

    RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override
        {
        BeAssert(false && "Cannot process requests from native code. See startBulkOperation.");
        return RepositoryStatus::ServerUnavailable;
        }

    RepositoryStatus _PrepareForElementOperation(Request& req, DgnElementCR el, BeSQLite::DbOpcode op) override
        {
        auto rstat = el.PopulateRequest(req, op);
        if (RepositoryStatus::Success != rstat)
            return rstat;

        if (m_inBulkUpdate)
            AccumulateRequests(req);

        return RepositoryStatus::Success;
        }

    RepositoryStatus _PrepareForModelOperation(Request& req, DgnModelCR model, BeSQLite::DbOpcode op) override
        {
        auto rstat = model.PopulateRequest(req, op);

        if (RepositoryStatus::Success != rstat)
            return rstat;

        if (m_inBulkUpdate)
            AccumulateRequests(req);

        return RepositoryStatus::Success;
        }

    void _OnElementInserted(DgnElementId id) override
        {
        if (!m_inBulkUpdate)
            return;

        if (LocksRequired())
            m_req.Locks().GetLockSet().insert(DgnLock(LockableId(id), LockLevel::Exclusive));
        }

    void _OnModelInserted(DgnModelId id) override
        {
        if (!m_inBulkUpdate)
            return;

        if (LocksRequired())
            m_req.Locks().GetLockSet().insert(DgnLock(LockableId(id), LockLevel::Exclusive));
        }

    RepositoryStatus _ReserveCode(DgnCodeCR code) override
        {
        if (m_inBulkUpdate)
            {
            m_req.Codes().insert(code);
            return RepositoryStatus::Success;
            }

        BeAssert(false && "Cannot process requests from native code. See startBulkOperation.");
        return RepositoryStatus::ServerUnavailable;
        }

    void _OnDgnDbDestroyed() override { m_req.Reset(); m_inBulkUpdate = false; IBriefcaseManager::_OnDgnDbDestroyed(); }

    RepositoryStatus _ClearUserHeldCodesLocks() override
        {
        BeAssert(false && "native code should not be trying to manage locks and codes");
        // TODO: forward to TypeScript?
        return RepositoryStatus::Success; 
        }

    RepositoryStatus _OnFinishRevision(DgnRevision const& rev) override
        {
        // The schema upgrade logic calls Merge. Nothing we do about that.
#ifdef COMMENT_OUT
        if (!m_inBulkUpdate)
            {
            // TODO: Notify TypeScript about this!
            // Any codes which became Used as a result of these changes must necessarily have been Reserved by this briefcase,
            // and are now no longer Reserved by any briefcase
            // (Any codes which became Discarded were necessarily previously Used, therefore no local state needs to be updated for them).
            return RepositoryStatus::Success;
            }

        BeAssert(false && "don't merge changes while in a bulk op");
        return RepositoryStatus::PendingTransactions;
#endif
        return RepositoryStatus::Success;
        }

    void _OnCommit(TxnManager& mgr) override
        {
        // The schema upgrade logic calls SaveChanges. Nothing we do about that.
#ifdef COMMENT_OUT
        // This event is invoked whenver any DgnDb is committed, not just mine.
        if (&GetDgnDb() != &mgr.GetDgnDb())
            return;
        BeAssert(!m_inBulkUpdate && "don't call SaveChanges while in a bulk op");
#endif
        }

    void _OnAppliedChanges(TxnManager& mgr) override
        {
        // This event is invoked whenver any DgnDb is updated, not just mine.
        if (&GetDgnDb() != &mgr.GetDgnDb())
            return;
        m_inBulkUpdate = 0;
        m_req.Reset();
        }

    void _OnUndoRedo(TxnManager& mgr, TxnAction) override
        {
        // This event is invoked whenver any DgnDb is updated, not just mine.
        if (&GetDgnDb() != &mgr.GetDgnDb())
            return;
        m_inBulkUpdate = 0;
        m_req.Reset();
        }

    bool _AreResourcesHeld(DgnLockSet& locks, DgnCodeSet& codes, RepositoryStatus* status) override
        {
        auto control = GetDgnDb().GetConcurrencyControl();
        if (nullptr != control)
            control->_OnQueryHeld(locks, codes, *this);

        // TODO: this query would have to be forwarded to TypeScript
        if (status)
            *status = RepositoryStatus::Success;

        if (nullptr != control)
            control->_OnQueriedHeld(locks, codes, *this);

        return true;
        }

    RepositoryStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) override
        {
        // TODO: this query would have to be forwarded to TypeScript

        auto bcId = GetDgnDb().GetBriefcaseId();

        for (auto const& code : codes)
            states.insert(DgnCodeInfo(code)).first->SetReserved(bcId);
        return RepositoryStatus::Success;
        }

    RepositoryStatus _QueryLockLevels(DgnLockSet& levels, LockableIdSet& lockIds) override
        {
        // TODO: this query would have to be forwarded to TypeScript
        for (auto const& id : lockIds)
            levels.insert(DgnLock(id, LockLevel::Exclusive));

        return RepositoryStatus::Success;
        }

    RepositoryStatus _QueryLockLevel(LockLevel& level, LockableId lockId) override
        {
        // TODO: this query would have to be forwarded to TypeScript
        level = LockLevel::Exclusive; return RepositoryStatus::Success;
        }

    IOwnedLocksIteratorPtr _GetOwnedLocks(FastQuery fast) override
        {
        // TODO: this query would have to be forwarded to TypeScript
        return nullptr;
        }

    RepositoryStatus _RefreshFromRepository() override
        {
        // TODO: Forward to TypeScript?
        return RepositoryStatus::Success;
        }

    void _StartBulkOperation() override { ++m_inBulkUpdate; }
    
    bool _IsBulkOperation() const override { return 0 != m_inBulkUpdate; }

    Response _EndBulkOperation() override
        {
        if (!s_okEndBulkMode)   // We ignore requests to end bulk mode unless they come from imodeljsNative.
            return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::Success);

        if (m_inBulkUpdate <= 0)
            {
            BeAssert(false);
            return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::InvalidRequest);
            }

        --m_inBulkUpdate;

        if (0 != m_inBulkUpdate)
            return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::Success);

        BeAssert(m_req.IsEmpty() && "Cannot acquire resources in native code. You must capture the bulk op request and process it in TypeScript before calling endBulkOperation.");
        return Response(RequestPurpose::Acquire, ResponseOptions::None, RepositoryStatus::Success);
        }

    void _ExtractRequestFromBulkOperation(Request& reqOut, bool locks, bool codes) override
        {
        auto control = GetDgnDb().GetConcurrencyControl();
        if (nullptr != control)
            control->_OnExtractRequest(m_req, *this);

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

        if (nullptr != control)
            control->_OnExtractedRequest(m_req, *this);
        }

    void AccumulateRequests(Request const& req)
        {
        BeAssert(m_inBulkUpdate);
        m_req.Codes().insert(req.Codes().begin(), req.Codes().end());
        m_req.Locks().GetLockSet().insert(req.Locks().GetLockSet().begin(), req.Locks().GetLockSet().end());
        }
    
    bool _IsCodeInLockedScope(DgnCodeCR) const override {return true;}
    };

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  01/18
//=======================================================================================
struct RepositoryManagerWatchDog : IRepositoryManager
    {
    using ResponseOptions = IBriefcaseManager::ResponseOptions;
    using RequestPurpose = IBriefcaseManager::RequestPurpose;
    using Response = IBriefcaseManager::Response;

    Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override
        {
        if (queryOnly)
            return Response(RequestPurpose::Query, req.Options(), RepositoryStatus::Success);

        // This is just a watchdog to ensure that there are *NO LOCKS OR CODES REQUIRED* at the time native code needs to acquire or assert them.
        // The TS app must get all locks and codes before attempting to save changes.
        if (req.Locks().IsEmpty() && req.Codes().empty())
            return Response(RequestPurpose::Acquire, req.Options(), RepositoryStatus::Success);

        JsInterop::ThrowJsException("acquire all locks and codes before calling endBulkOperation or saveChanges");
        return Response(RequestPurpose::Acquire, req.Options(), RepositoryStatus::InvalidRequest);
        }

    RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override { BeAssert(false && "should not be used"); return RepositoryStatus::InvalidRequest; }
    RepositoryStatus _Relinquish(Resources which, DgnDbR db) override { BeAssert(false && "should not be used"); return RepositoryStatus::InvalidRequest; }

    RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override
        {
        return RepositoryStatus::Success;
        }

    RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override
        {
        BeAssert(false && "should not be used");
        return RepositoryStatus::InvalidRequest;
        }
    };

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
struct NativeRepositoryAdmin : DgnPlatformLib::Host::RepositoryAdmin
    {
    DEFINE_T_SUPER(RepositoryAdmin);

    NativeRepositoryAdmin() {}

    IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override
        {
        return new RepositoryManagerWatchDog();
        }

    IBriefcaseManagerPtr _CreateBriefcaseManager(DgnDbR db) const override
        {
        IBriefcaseManagerPtr bc;
        if (db.IsMasterCopy() || db.IsStandaloneBriefcase())
            bc = MasterBriefcaseManager::Create(db);
        else
            {
            bc = new BriefcaseManager(db);
            if (nullptr != db.GetConcurrencyControl())
                db.GetConcurrencyControl()->_ConfigureBriefcaseManager(*bc);
            }
        return bc;
        }
    };

} // IModelJsNative

using namespace IModelJsNative;

bool JsInterop::SetOkEndBulkMode(bool b)
    {
    bool was = s_okEndBulkMode;
    s_okEndBulkMode = b;
    return was;
    }

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
DgnPlatformLib::Host::RepositoryAdmin& JsInterop::GetRepositoryAdmin()
    {
    static NativeRepositoryAdmin* s_admin;
    if (nullptr == s_admin)
        s_admin = new NativeRepositoryAdmin();
    return *s_admin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BuildBriefcaseManagerResourcesRequestToInsertElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemProps)
    {
    // *** NEEDS WORK: We don't want to go to the expense of creating a DgnElement just so that we can invoke its _PopulateRequest virtual method.
    // ***              We replicate here what the base DgnElement::_PopulateRequest method does.
    //

    if (!elemProps.isMember("modelid") || !elemProps.isMember("code"))
        {
        BeAssert(false);
        return RepositoryStatus::InvalidRequest;
        }

    DgnModelId mid;
    mid.FromJson(elemProps["modelid"]);
    auto rc = BuildBriefcaseManagerResourcesRequestToLockModel(req, dgndb, mid, LockLevel::Shared);
    if (RepositoryStatus::Success != rc)
        return rc;

    DgnCode code = DgnCode::FromJson2(elemProps["code"]);
    if (code.IsValid() && !code.IsEmpty())
        {
        // Avoid asking repository manager to reserve code if we know it's already in use...
        if (dgndb.Elements().QueryElementIdByCode(code).IsValid())
            return RepositoryStatus::CodeUsed;

        req.Codes().insert(code);
        }

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BuildBriefcaseManagerResourcesRequestForElementById(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemIdJson, BeSQLite::DbOpcode opcode)
    {
    DgnElementId eid;
    eid.FromJson(elemIdJson);
    DgnElementCPtr elem = dgndb.Elements().GetElement(eid);
    if (!elem.IsValid())
        {
        BeAssert(false);
        return RepositoryStatus::InvalidRequest;
        }
    return elem->PopulateRequest(req, opcode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BuildBriefcaseManagerResourcesRequestForElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode)
    {
    if (elemPropsJson.isNull())
        return RepositoryStatus::Success;

    RepositoryStatus rc;

    if (BeSQLite::DbOpcode::Insert == opcode)
        rc = BuildBriefcaseManagerResourcesRequestToInsertElement(req, dgndb, elemPropsJson);
    else
        rc = BuildBriefcaseManagerResourcesRequestForElementById(req, dgndb, elemPropsJson, opcode);

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BuildBriefcaseManagerResourcesRequestToLockModel(IBriefcaseManager::Request& req, DgnDbR dgndb, DgnModelId mid, LockLevel level)
    {
    auto model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        return RepositoryStatus::InvalidRequest;
    req.Locks().Insert(*model, level);
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BuildBriefcaseManagerResourcesRequestForModel(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op)
    {
    DgnModelId mid;
    mid.FromJson(modelPropsJson);
    if (BeSQLite::DbOpcode::Insert == op)
        {
        // *** NEEDS WORK: We don't want to go to the expense of creating a DgnElement just so that we can invoke its _PopulateRequest virtual method.
        // ***              We replicate here what the base DgnModel::_PopulateRequest method does.
        //
        req.Locks().Insert(dgndb, LockLevel::Shared);
        return RepositoryStatus::Success;
        }

    auto model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        return RepositoryStatus::InvalidRequest;

    return model->PopulateRequest(req, op);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op)
    {
    // *** TODO: What should we lock?
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BuildBriefcaseManagerResourcesRequestForCodeSpec(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op)
    {
    // *** TODO: What should we lock?
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BriefcaseManagerStartBulkOperation(DgnDbR dgndb)
    {
    dgndb.BriefcaseManager().StartBulkOperation();
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsInterop::BriefcaseManagerEndBulkOperation(DgnDbR dgndb)
    {
    auto& bcm = dgndb.BriefcaseManager();
    auto was = SetOkEndBulkMode(true);
    auto res = bcm.EndBulkOperation().Result();
    SetOkEndBulkMode(was);
    return res;
    }

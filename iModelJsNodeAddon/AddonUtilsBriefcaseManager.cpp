/*--------------------------------------------------------------------------------------+
|
|     $Source: AddonUtilsBriefcaseManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AddonUtils.h"

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  12/17
//=======================================================================================
struct OptimisticBriefcaseManager : IBriefcaseManager
{
    AddonUtils::BriefcaseManagerConflictResolution uu, ud, du;

    OptimisticBriefcaseManager(DgnDbR db) : IBriefcaseManager(db) { }

    Response _ProcessRequest(Request& req, RequestPurpose purpose) override { return Response(purpose, req.Options(), RepositoryStatus::Success); }
    RepositoryStatus _Demote(DgnLockSet&, DgnCodeSet const&) override { return RepositoryStatus::Success; }
    RepositoryStatus _Relinquish(Resources) override { return RepositoryStatus::Success; }
    RepositoryStatus _ReserveCode(DgnCodeCR) override { return RepositoryStatus::Success; }
    RepositoryStatus _QueryLockLevel(LockLevel& level, LockableId lockId) override { level = LockLevel::Exclusive; return RepositoryStatus::Success; }
    IOwnedLocksIteratorPtr _GetOwnedLocks(FastQuery fast) override { return nullptr; }
    RepositoryStatus _OnFinishRevision(DgnRevision const&) override { return RepositoryStatus::Success; }
    RepositoryStatus _RefreshFromRepository() override { return RepositoryStatus::Success; }
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
};

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
struct AddonRepositoryAdmin : DgnPlatformLib::Host::RepositoryAdmin
{
    DEFINE_T_SUPER(RepositoryAdmin);

    bset<DgnDbP> m_optimistic;

    AddonRepositoryAdmin() {}
    
    IBriefcaseManagerPtr _CreateBriefcaseManager(DgnDbR db) const override
        {
        if (m_optimistic.find(&db) != m_optimistic.end())
            return new OptimisticBriefcaseManager(db);
        return T_Super::_CreateBriefcaseManager(db);
        }
};

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
DgnPlatformLib::Host::RepositoryAdmin& BentleyApi::Dgn::AddonUtils::GetRepositoryAdmin()
    {
    static AddonRepositoryAdmin* s_admin;
    if (nullptr == s_admin)
        s_admin = new AddonRepositoryAdmin();
    return *s_admin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus AddonUtils::BuildBriefcaseManagerResourcesRequestToInsertElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemProps)
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

    DgnCode code;
    code.FromJson(elemProps["code"]);

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
RepositoryStatus AddonUtils::BuildBriefcaseManagerResourcesRequestForElementById(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemIdJson, BeSQLite::DbOpcode opcode)
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
RepositoryStatus AddonUtils::BuildBriefcaseManagerResourcesRequestForElement(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR elemPropsJson, BeSQLite::DbOpcode opcode)
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
RepositoryStatus AddonUtils::BuildBriefcaseManagerResourcesRequestToLockModel(IBriefcaseManager::Request& req, DgnDbR dgndb, DgnModelId mid, LockLevel level)
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
RepositoryStatus AddonUtils::BuildBriefcaseManagerResourcesRequestForModel(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op)
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
RepositoryStatus AddonUtils::SetBriefcaseManagerOptimisticConcurrencyControlPolicy(DgnDbR dgndb, BriefcaseManagerConflictResolution uu, 
                                                                                BriefcaseManagerConflictResolution ud, BriefcaseManagerConflictResolution du)
    {
    BeSqliteDbMutexHolder serializeAccess(dgndb);
    // TBD: assert main thread

    if (true)
        {
        auto& existingBcm = dgndb.BriefcaseManager();
        if (nullptr != dynamic_cast<OptimisticBriefcaseManager*>(&existingBcm))
            {
            // Already in optimistic mode
            return RepositoryStatus::Success;
            }

        // Must switch from pessimistic to optimistic
        if (dgndb.Txns().HasChanges())
            {
            return RepositoryStatus::InvalidRequest;
            }
        }

    dgndb.DestroyBriefcaseManager();
    static_cast<AddonRepositoryAdmin&>(T_HOST.GetRepositoryAdmin()).m_optimistic.insert(&dgndb);
    auto& bcm = dgndb.BriefcaseManager();
    static_cast<AddonRepositoryAdmin&>(T_HOST.GetRepositoryAdmin()).m_optimistic.erase(&dgndb);

    OptimisticBriefcaseManager* obm = dynamic_cast<OptimisticBriefcaseManager*>(&bcm);
    if (nullptr == obm)
        {
        BeAssert(false);
        return RepositoryStatus::InvalidRequest;
        }
    obm->uu = uu;
    obm->ud = ud;
    obm->du = du;
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus AddonUtils::SetBriefcaseManagerPessimisticConcurrencyControlPolicy(DgnDbR dgndb)
    {
    BeSqliteDbMutexHolder serializeAccess(dgndb);
    // TBD: assert main thread

    if (true)
        {
        auto& existingBcm = dgndb.BriefcaseManager();
        if (nullptr == dynamic_cast<OptimisticBriefcaseManager*>(&existingBcm))
            {
            // Already in Pessimistic mode
            return RepositoryStatus::Success;
            }

        // Must switch from optimistic to pessimistic
        if (dgndb.Txns().HasChanges())
            {
            return RepositoryStatus::InvalidRequest;
            }
        }

    dgndb.DestroyBriefcaseManager();
    return RepositoryStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus AddonUtils::BriefcaseManagerStartBulkOperation(DgnDbR dgndb)
    {
    // TBD: assert main thread
    auto& bcm = dgndb.BriefcaseManager();
    if (nullptr != dynamic_cast<OptimisticBriefcaseManager*>(&bcm))
        {
        GetLogger().error("Not in pessimistic concurrency mode");
        return RepositoryStatus::InvalidRequest;
        }

    bcm.StartBulkOperation();
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus AddonUtils::BriefcaseManagerEndBulkOperation(DgnDbR dgndb)
    {
    // TBD: assert main thread
    auto& bcm = dgndb.BriefcaseManager();
    if (nullptr != dynamic_cast<OptimisticBriefcaseManager*>(&bcm))
        {
        GetLogger().error("Not in pessimistic concurrency mode");
        return RepositoryStatus::InvalidRequest;
        }

    return bcm.EndBulkOperation().Result();
    }

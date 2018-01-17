/*--------------------------------------------------------------------------------------+
|
|     $Source: AddonUtilsBriefcaseManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AddonUtils.h"

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

        AddonUtils::ThrowJsException("acquire all locks and codes before calling endBulkOperation or saveChanges");
        return Response(RequestPurpose::Acquire, req.Options(), RepositoryStatus::InvalidRequest);
        }

    RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override {BeAssert(false && "should not be used"); return RepositoryStatus::InvalidRequest;}
    RepositoryStatus _Relinquish(Resources which, DgnDbR db) override {BeAssert(false && "should not be used"); return RepositoryStatus::InvalidRequest;}

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
struct AddonRepositoryAdmin : DgnPlatformLib::Host::RepositoryAdmin
{
    DEFINE_T_SUPER(RepositoryAdmin);

    AddonRepositoryAdmin() {}
    
    IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override 
        {
        return new RepositoryManagerWatchDog();
        }

    IBriefcaseManagerPtr _CreateBriefcaseManager(DgnDbR db) const override
        {
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
RepositoryStatus AddonUtils::BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op)
    {
    // *** TODO: What should we lock?
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus AddonUtils::BuildBriefcaseManagerResourcesRequestForCodeSpec(IBriefcaseManager::Request& req, DgnDbR dgndb, JsonValueCR modelPropsJson, BeSQLite::DbOpcode op)
    {
    // *** TODO: What should we lock?
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus AddonUtils::BriefcaseManagerStartBulkOperation(DgnDbR dgndb)
    {
    dgndb.BriefcaseManager().StartBulkOperation();
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus AddonUtils::BriefcaseManagerEndBulkOperation(DgnDbR dgndb)
    {
    auto& bcm = dgndb.BriefcaseManager();
    return bcm.EndBulkOperation().Result();
    }

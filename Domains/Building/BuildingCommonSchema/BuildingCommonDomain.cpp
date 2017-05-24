/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingCommonSchema/BuildingCommonDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuildingCommonSchemaInternal.h"


BEGIN_BENTLEY_BUILDING_COMMON_NAMESPACE

DOMAIN_DEFINE_MEMBERS(BuildingCommonDomain)

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Marc.Bedard                     10/2016
 +---------------+---------------+---------------+---------------+---------------+------*/
BuildingCommonDomain::BuildingCommonDomain() : DgnDomain(BENTLEY_BUILDING_COMMON_SCHEMA_NAME, "Bentley Building Common Domain", 1)
    {
    /*RegisterHandler(RadialDistortionHandler::GetHandler());
    RegisterHandler(TangentialDistortionHandler::GetHandler());
    RegisterHandler(CameraDeviceHandler::GetHandler());
    RegisterHandler(CameraDeviceModelHandler::GetHandler());
    RegisterHandler(ShotHandler::GetHandler());
    RegisterHandler(PoseHandler::GetHandler()); */
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Marc.Bedard                     10/2016
 +---------------+---------------+---------------+---------------+---------------+------*/
void BuildingCommonDomain::_OnSchemaImported(DgnDbR dgndb) const
    {

    DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);
    /*
    DgnCategory cameraDeviceCategory(DgnCategory::CreateParams(dgndb, BDCP_CATEGORY_CameraDevice, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    cameraDeviceCategory.Insert(defaultApperance);
    BeAssert(cameraDeviceCategory.GetCategoryId().IsValid());
    DgnCategory shotCategory(DgnCategory::CreateParams(dgndb, BDCP_CATEGORY_Shot, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    shotCategory.Insert(defaultApperance);
    BeAssert(shotCategory.GetCategoryId().IsValid());
    DgnCategory PoseCategory(DgnCategory::CreateParams(dgndb, BDCP_CATEGORY_Pose, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    PoseCategory.Insert(defaultApperance);
    BeAssert(PoseCategory.GetCategoryId().IsValid());


    auto authority = NamespaceAuthority::CreateNamespaceAuthority(BDCP_AUTHORITY_DataCapture, dgndb);
    BeAssert(authority.IsValid());
    if (authority.IsValid())
    {
    authority->Insert();
    BeAssert(authority->GetAuthorityId().IsValid());
    } */
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Marc.Bedard                     12/2016
 +---------------+---------------+---------------+---------------+---------------+------*/
Dgn::CodeSpecId  BuildingCommonDomain::QueryBuildingCommonCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_BUILDING_COMMON_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Marc.Bedard                     12/2016
 +---------------+---------------+---------------+---------------+---------------+------*/
DgnCode BuildingCommonDomain::CreateCode(DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BENTLEY_BUILDING_COMMON_AUTHORITY, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/16
//---------------------------------------------------------------------------------------
void BuildingCommonDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
ECN::IECInstancePtr BuildingCommonDomain::AddAspect(Dgn::PhysicalModelR model, Dgn::PhysicalElementPtr element, Utf8StringCR className)
    {

    // Find the class

    ECN::ECClassCP aspectClassP = model.GetDgnDb().GetClassLocater().LocateClass(BENTLEY_BUILDING_COMMON_SCHEMA_NAME, className.c_str());

    if (nullptr == aspectClassP)
        return nullptr;

    // If the element is already persisted and has the Aspect class, you can't add another

    if (element->GetElementId().IsValid())
        {
        ECN::IECInstanceCP instance = DgnElement::GenericUniqueAspect::GetAspect(*element, *aspectClassP);

        if (nullptr != instance)
            return nullptr;
        }

    ECN::StandaloneECEnablerPtr enabler = aspectClassP->GetDefaultStandaloneEnabler();

    if (!enabler.IsValid())
        return nullptr;

    ECN::IECInstancePtr instance = enabler->CreateInstance().get();
    if (!instance.IsValid())
        return nullptr;

    Dgn::DgnDbStatus status = DgnElement::GenericUniqueAspect::SetAspect(*element, *instance);

    if (Dgn::DgnDbStatus::Success != status)
        return nullptr;

    return instance;
    }


END_BENTLEY_BUILDING_COMMON_NAMESPACE

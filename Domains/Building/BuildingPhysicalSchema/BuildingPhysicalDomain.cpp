/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingPhysicalSchema/BuildingPhysicalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuildingPhysicalSchemaInternal.h"


BEGIN_BENTLEY_BUILDING_PHYSICAL_NAMESPACE

DOMAIN_DEFINE_MEMBERS(BuildingPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Marc.Bedard                     10/2016
 +---------------+---------------+---------------+---------------+---------------+------*/
BuildingPhysicalDomain::BuildingPhysicalDomain() : DgnDomain(BENTLEY_BUILDING_PHYSICAL_SCHEMA_NAME, "Bentley Building Physical Domain", 1)
    {
    RegisterHandler(BuildingPhysicalModelHandler::GetHandler());
    RegisterHandler(BuildingTypeDefinitionModelHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Marc.Bedard                     10/2016
 +---------------+---------------+---------------+---------------+---------------+------*/
void BuildingPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
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
Dgn::CodeSpecId  BuildingPhysicalDomain::QueryBuildingPhysicalCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_BUILDING_PHYSICAL_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Marc.Bedard                     12/2016
 +---------------+---------------+---------------+---------------+---------------+------*/
DgnCode BuildingPhysicalDomain::CreateCode(DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BENTLEY_BUILDING_PHYSICAL_AUTHORITY, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/16
//---------------------------------------------------------------------------------------
void BuildingPhysicalDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    }

END_BENTLEY_BUILDING_PHYSICAL_NAMESPACE

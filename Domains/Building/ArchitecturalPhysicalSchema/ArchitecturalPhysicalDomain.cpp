/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/ArchitecturalPhysicalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalSchemaInternal.h"

DOMAIN_DEFINE_MEMBERS(ArchitecturalPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ArchitecturalPhysicalDomain::ArchitecturalPhysicalDomain() : DgnDomain(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, "Bentley Architectural Physical Domain", 1)
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
void ArchitecturalPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
  /*  DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);

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
Dgn::CodeSpecId  ArchitecturalPhysicalDomain::QueryArchitecturalPhysicalCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ArchitecturalPhysicalDomain::CreateCode(DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      07/16
//---------------------------------------------------------------------------------------
void ArchitecturalPhysicalDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    }


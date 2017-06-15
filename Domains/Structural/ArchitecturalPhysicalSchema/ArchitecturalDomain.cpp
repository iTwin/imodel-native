/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/ArchitecturalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DataCaptureSchemaInternal.h>

#ifdef NOTHING
DOMAIN_DEFINE_MEMBERS(DataCaptureDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataCaptureDomain::DataCaptureDomain() : DgnDomain(BDCP_SCHEMA_NAME, "Bentley DataCapture Domain", 1)
    {
    RegisterHandler(RadialDistortionHandler::GetHandler());
    RegisterHandler(TangentialDistortionHandler::GetHandler());
    RegisterHandler(CameraDeviceHandler::GetHandler());    
    RegisterHandler(CameraDeviceModelHandler::GetHandler());
    RegisterHandler(ShotHandler::GetHandler());
    RegisterHandler(PoseHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnSubCategory::Appearance defaultApperance;
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
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DataCaptureDomain::QueryDataCaptureAuthorityId(DgnDbCR dgndb)
    {
    DgnAuthorityId authorityId = dgndb.Authorities().QueryAuthorityId(BDCP_AUTHORITY_DataCapture);
    BeAssert(authorityId.IsValid());
    return authorityId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DataCaptureDomain::CreateCode(DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    BeAssert((nameSpace == BDCP_CLASS_CameraDeviceModel) ||
             (nameSpace == BDCP_CLASS_CameraDevice) ||
             (nameSpace == BDCP_CLASS_Pose)         ||
             (nameSpace == BDCP_CLASS_Shot));

    return NamespaceAuthority::CreateCode(BDCP_AUTHORITY_DataCapture, value, dgndb, nameSpace);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2015
//---------------------------------------------------------------------------------------
Utf8String DataCaptureDomain::FormatId(BeBriefcaseBasedId id)
    {
    Utf8PrintfString formattedId("%" PRIu32 "-%" PRIu64, id.GetBriefcaseId().GetValue(), (uint64_t) (0xffffffffffLL & id.GetValue()));
    return formattedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2015
//---------------------------------------------------------------------------------------
Utf8String DataCaptureDomain::BuildDefaultName(Utf8CP prefix, BeBriefcaseBasedId id)
    {
    Utf8String defaultName(prefix);
    defaultName += "-";
    defaultName += FormatId(id);
    return defaultName;
    }
    
#endif



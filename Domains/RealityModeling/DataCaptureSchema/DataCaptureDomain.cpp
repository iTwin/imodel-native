/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/DataCaptureDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DataCaptureSchemaInternal.h>

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
    }


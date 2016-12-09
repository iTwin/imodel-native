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
    RegisterHandler(CameraHandler::GetHandler());    
    RegisterHandler(CameraTypeHandler::GetHandler());
    RegisterHandler(PhotoHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);

    DgnCategory cameraCategory(DgnCategory::CreateParams(dgndb, BDCP_CATEGORY_Camera, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    cameraCategory.Insert(defaultApperance);
    BeAssert(cameraCategory.GetCategoryId().IsValid());
    DgnCategory photoCategory(DgnCategory::CreateParams(dgndb, BDCP_CATEGORY_Photo, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    photoCategory.Insert(defaultApperance);
    BeAssert(photoCategory.GetCategoryId().IsValid());
    }


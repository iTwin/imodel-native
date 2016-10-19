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
//     RegisterHandler(SegmentRangeElementHandler::GetHandler());    
//     RegisterHandler(RailRangeHandler::GetHandler());
//     RegisterHandler(RoadRangeHandler::GetHandler());
//     RegisterHandler(SegmentElementHandler::GetHandler());
//     RegisterHandler(RoadSegmentElementHandler::GetHandler());
//     RegisterHandler(RoadSegmentHandler::GetHandler());
//     RegisterHandler(RoadSegmentOnBridgeHandler::GetHandler());
//     RegisterHandler(TransitionSegmentHandler::GetHandler());

//     RegisterHandler(StatusAspectHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnCategory cameraCategory(DgnCategory::CreateParams(dgndb, BDCP_CATEGORY_Camera, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    cameraCategory.Insert(DgnSubCategory::Appearance());
    BeAssert(cameraCategory.GetCategoryId().IsValid());
    }


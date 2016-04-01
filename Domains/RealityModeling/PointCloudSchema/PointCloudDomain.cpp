/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA

DOMAIN_DEFINE_MEMBERS(PointCloudDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
PointCloudDomain::PointCloudDomain() : DgnDomain(POINTCLOUD_SCHEMA_NAME, "Point Cloud Domain", 1) 
    {
    RegisterHandler(PointCloudModelHandler::GetHandler());

    // Initialize Pointools API, among others.
    InitializeApi();
    }
 
//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
void PointCloudDomain::_OnSchemaImported(DgnDbR db) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
void PointCloudDomain::InitializeApi()
    {
    static bool s_initialized = false;
    if(!s_initialized)
        {
        BePointCloud::BePointCloudApi::Initialize();
        s_initialized = true;
        }
    }
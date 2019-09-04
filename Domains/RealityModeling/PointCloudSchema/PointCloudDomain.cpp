/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUD

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

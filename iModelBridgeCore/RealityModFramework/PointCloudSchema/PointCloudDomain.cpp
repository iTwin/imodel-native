/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA

DOMAIN_DEFINE_MEMBERS(PointCloudDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
PointCloudDomain::PointCloudDomain() : DgnDomain(BENTLEY_POINTCLOUD_SCHEMA_NAME, "Bentley Point Cloud Domain", 1) 
    {
    RegisterHandler(PointCloudModelHandler::GetHandler());
    }
 
//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
void PointCloudDomain::_OnSchemaImported(DgnDbR db) const
    {
    }


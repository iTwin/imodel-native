/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudSchemaApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>

#include <PointCloudSchema/ExportMacros.h>
#include <PointCloudSchema/PointCloudSchemaCommon.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_POINTCLOUD_SCHEMA_NAME                      "BentleyPointCloud"
#define BENTLEY_POINTCLOUD_SCHEMA_PATH                      L"ECSchemas/Domain/BentleyPointCloud.01.00.ecschema.xml"
#define POINTCLOUD_SCHEMA(className)                        BENTLEY_POINTCLOUD_SCHEMA_NAME "." className

//-----------------------------------------------------------------------------------------
// Include from PublicApi
//-----------------------------------------------------------------------------------------
#include "PointCloudSchemaCommon.h"
#include "ExportMacros.h"
#include "PointCloudDomain.h"

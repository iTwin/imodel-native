/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudSchemaApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#include <PointCloudSchema/PointCloudSchemaTypes.h>
#include <PointCloudSchema/PointCloudSchemaCommon.h>
#include <PointCloudSchema/PointCloudDomain.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define POINTCLOUD_SCHEMA_NAME                              "PointCloud"
#define POINTCLOUD_SCHEMA_PATH                              L"ECSchemas/Domain/PointCloud.01.00.ecschema.xml"
#define POINTCLOUD_SCHEMA(className)                        POINTCLOUD_SCHEMA_NAME "." className

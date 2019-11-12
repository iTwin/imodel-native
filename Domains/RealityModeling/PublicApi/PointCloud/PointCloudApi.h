/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>

#define POINTCLOUD_SCHEMA_NAME "PointCloud"
#define POINTCLOUD_SCHEMA_FILE L"PointCloud.ecschema.xml"
#define POINTCLOUD_SCHEMA(className) POINTCLOUD_SCHEMA_NAME "." className

#include <PointCloud/ExportMacros.h>
#include <PointCloud/PointCloudTypes.h>
#include <PointCloud/PointCloudCommon.h>
#include <PointCloud/PointCloudDomain.h>


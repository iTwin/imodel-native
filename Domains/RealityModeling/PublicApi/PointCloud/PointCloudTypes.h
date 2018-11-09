/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloud/PointCloudTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <DgnPlatform/DgnPlatform.h>

#include <PointCloud/PointCloudCommon.h>
#include <PointCloud/ExportMacros.h>

BEGIN_BENTLEY_POINTCLOUD_NAMESPACE

struct  IPointCloudViewSettings;
struct  IPointCloudClassificationViewSettings;
struct  PointCloudClassificationViewSettings;
struct  PointCloudModel;

typedef RefCountedPtr<IPointCloudViewSettings>                  IPointCloudViewSettingsPtr;
typedef RefCountedPtr<IPointCloudClassificationViewSettings>    IPointCloudClassificationViewSettingsPtr;
typedef RefCountedPtr<PointCloudClassificationViewSettings>     PointCloudClassificationViewSettingsPtr;
typedef RefCountedPtr<PointCloudModel>                          PointCloudModelPtr;

END_BENTLEY_POINTCLOUD_NAMESPACE


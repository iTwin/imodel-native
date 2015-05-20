/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudSchemaTypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <DgnPlatform/DgnPlatform.h>

#include <PointCloudSchema/PointCloudSchemaCommon.h>
#include <PointCloudSchema/ExportMacros.h>

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

struct  IPointCloudViewSettings;
struct  IPointCloudClassificationViewSettings;
struct  PointCloudClassificationViewSettings;
struct  PointCloudModel;

typedef RefCountedPtr<IPointCloudViewSettings>                  IPointCloudViewSettingsPtr;
typedef RefCountedPtr<IPointCloudClassificationViewSettings>    IPointCloudClassificationViewSettingsPtr;
typedef RefCountedPtr<PointCloudClassificationViewSettings>     PointCloudClassificationViewSettingsPtr;
typedef RefCountedPtr<PointCloudModel>                          PointCloudModelPtr;

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE


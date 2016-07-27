/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudSchemaInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __POINTCLOUDSCHEMAINTERNAL_H__
#define __POINTCLOUDSCHEMAINTERNAL_H__

#include <PointCloudSchema/PointCloudSchemaApi.h>

#include <Bentley/Bentley.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/JsonUtils.h>
#include <BePointCloud/BePointCloudApi.h>
#include <BePointCloud/PointCloudTypes.h>
#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/PointCloudColorDef.h>
#include <BePointCloud/PointCloudHandle.h>
#include <BePointCloud/PointCloudScene.h>
#include <BePointCloud/PointCloudChannelHandler.h>
#include <BePointCloud/PointCloudDataQuery.h>
#include <BePointCloud/PointCloudQueryBuffer.h>
#include <BePointCloud/PointCloudVortex.h>

#include <PointCloudSchema/PointCloudSchemaTypes.h>
#include <PointCloudSchema/PointCloudSchemaCommon.h>
#include <PointCloudSchema/ExportMacros.h>
#include <PointCloudSchema/PointCloudDrawBuffer.h>
#include <PointCloudSchema/PointCloudHandler.h>
#include <PointCloudSchema/PointCloudDomain.h>
#include <PointCloudSchema/PointCloudRamps.h>
#include <PointCloudSchema/PointCloudSettings.h>

#include "VisualizationManager.h"
#include "PointCloudGcsFacility.h"
#include "PointCloudViewport.h"
#include "PointCloudRenderer.h"
#include "PointCloudProgressiveDisplay.h"

#include <list>

#include <Logging/bentleylogging.h>

#ifndef NDEBUG
#define POINTCLOUD_TRACE 1
#endif

#if defined (POINTCLOUD_TRACE)
#   define PCLOG (*NativeLogging::LoggingManager::GetLogger ("PointCloud"))
#   define DEBUG_PRINTF PCLOG.debugv
#   define INFO_PRINTF  PCLOG.infov
#   define WARN_PRINTF  PCLOG.warningv
#   define ERROR_PRINTF PCLOG.errorv
#else
#   define DEBUG_PRINTF(...)
#   define INFO_PRINTF(...)
#   define WARN_PRINTF(...)
#   define ERROR_PRINTF(...)
#endif

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

#endif // __POINTCLOUDSCHEMAINTERNAL_H__

/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef __POINTCLOUDINTERNAL_H__
#define __POINTCLOUDINTERNAL_H__

#include <PointCloud/PointCloudApi.h>

#include <Bentley/Bentley.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeSystemInfo.h>
#include <folly/BeFolly.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DgnDomain.h>
#include <GeomJsonWireFormat/JsonUtils.h>
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

#include <PointCloud/PointCloudTypes.h>
#include <PointCloud/PointCloudCommon.h>
#include <PointCloud/ExportMacros.h>
#include <PointCloud/PointCloudDrawBuffer.h>
#include <PointCloud/PointCloudHandler.h>
#include <PointCloud/PointCloudDomain.h>
#include <PointCloud/PointCloudRamps.h>
#include <PointCloud/PointCloudSettings.h>

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

#endif // __POINTCLOUDINTERNAL_H__

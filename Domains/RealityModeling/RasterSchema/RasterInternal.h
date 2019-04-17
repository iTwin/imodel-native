/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#ifndef __RASTERINTERNAL_H__
#define __RASTERINTERNAL_H__

#include <Raster/RasterApi.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <GeomSerialization/GeomLibsFlatBufferApi.h>

#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/image.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <DgnPlatform/CesiumTileTree.h>
#include <folly/BeFolly.h>


#include <ImagePP/h/ImageppAPI.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/all/h/HRFSLOStripAdapter.h>
#include <ImagePP/all/h/HRABitmapBase.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HRFRasterFileBlockAdapter.h>
#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/HGF2DWorldCluster.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HRPPixelTypeV8Gray8.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HGF2DAffine.h>
#include <ImagePP/all/h/HRFPageFile.h>
#include <ImagePP/all/h/HRFPageFileFactory.h>
#include <ImagePP/all/h/HRFRasterFilePageDecorator.h>
#include <ImagePP/all/h/HRFiTiffCacheFileCreator.h>
#include <ImagePP/all/h/HRFRasterFileResBooster.h>
#include <ImagePP/all/h/HRFRasterFileCache.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HRPDEMFilter.h>
#include <ImagePP/all/h/HPMAttribute.h>
#include <ImagePP/all/h/HRADEMRaster.h>
#include <ImagePP/all/h/HGF2DPolygonOfSegments.h>
#include <ImagePP/all/h/HGF2DDCTransfoModel.h>


#include <Raster/RasterTypes.h>
#include <Raster/RasterCommon.h>
#include <Raster/ExportMacros.h>
#include <Raster/RasterHandler.h>
#include <Raster/RasterFileHandler.h>
#include <Raster/RasterDomain.h>


//
// Internal header files
//
#include "RasterTileTree.h"
#include "ImagePPAdmin.h"
#include "RasterFile.h"

#ifndef NDEBUG
#define RASTER_TRACE 1
#endif

#if defined (RASTER_TRACE)
#   define PCLOG (*NativeLogging::LoggingManager::GetLogger ("Raster"))
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

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_IMAGEPP
USING_NAMESPACE_BENTLEY_RASTER


#endif // __RASTERINTERNAL_H__

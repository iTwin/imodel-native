/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterSchemaInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __RASTERSCHEMAINTERNAL_H__
#define __RASTERSCHEMAINTERNAL_H__

#include <RasterSchema/RasterSchemaApi.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <GeomSerialization/GeomLibsFlatBufferApi.h>

#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/image.h>
#include <DgnPlatform/JsonUtils.h>
#include <DgnPlatform/TileTree.h>
#include <folly/BeFolly.h>


#include <Imagepp/h/ImageppAPI.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRABitmapBase.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HRFPageFile.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFileResBooster.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRPDEMFilter.h>
#include <Imagepp/all/h/HPMAttribute.h>
#include <Imagepp/all/h/HRADEMRaster.h>
#include <ImagePP/all/h/HGF2DPolygonOfSegments.h>
#include <ImagePP/all/h/HGF2DDCTransfoModel.h>


#include <RasterSchema/RasterSchemaTypes.h>
#include <RasterSchema/RasterSchemaCommon.h>
#include <RasterSchema/ExportMacros.h>
#include <RasterSchema/RasterHandler.h>
#include <RasterSchema/RasterFileHandler.h>
#include <RasterSchema/RasterDomain.h>

RASTERSCHEMA_TYPEDEFS(RasterSource)
RASTERSCHEMA_REF_COUNTED_PTR(RasterSource)

//
// Internal header files
//
#include "RasterSource.h"
#include "RasterTileTree.h"
#include "ImagePPAdmin.h"
#include "RasterFile.h"

#ifndef NDEBUG
//#define RASTER_TRACE 1
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
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_IMAGEPP
USING_NAMESPACE_BENTLEY_RASTERSCHEMA


#endif // __RASTERSCHEMAINTERNAL_H__

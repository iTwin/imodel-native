/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterSchemaInternal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __RASTERSCHEMAINTERNAL_H__
#define __RASTERSCHEMAINTERNAL_H__

#include <RasterSchema/RasterSchemaApi.h>
#include <unordered_map>

#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnCore/DgnDomain.h>
#include <DgnPlatform/DgnCore/RasterBaseModel.h>
#include <DgnPlatform/DgnCore/ImageUtilities.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DgnHandlers/image.h>
#include <windows.h>
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

#include <RasterSchema/RasterSchemaTypes.h>
#include <RasterSchema/RasterSchemaCommon.h>
#include <RasterSchema/ExportMacros.h>
#include <RasterSchema/RasterHandler.h>
#include <RasterSchema/RasterFileHandler.h>
#include <RasterSchema/RasterDomain.h>

RASTERSCHEMA_TYPEDEFS(RasterTile)
RASTERSCHEMA_REF_COUNTED_PTR(RasterTile)

RASTERSCHEMA_TYPEDEFS(RasterSource)
RASTERSCHEMA_REF_COUNTED_PTR(RasterSource)

RASTERSCHEMA_TYPEDEFS(DisplayTile)
RASTERSCHEMA_REF_COUNTED_PTR(DisplayTile)

RASTERSCHEMA_TYPEDEFS(Bitmap)
RASTERSCHEMA_REF_COUNTED_PTR(Bitmap)

//
// Internal header files
//
#include "RasterSource.h"
#include "RasterQuadTree.h"
#include "ImagePPAdmin.h"
#include "RasterFile.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_IMAGEPP
USING_NAMESPACE_BENTLEY_RASTERSCHEMA


#endif // __RASTERSCHEMAINTERNAL_H__

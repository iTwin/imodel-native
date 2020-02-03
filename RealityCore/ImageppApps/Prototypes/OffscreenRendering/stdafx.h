/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/stdafx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


//#include <windows.h>

#include <WINSOCK2.H>

//#include <wingdi.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <D3DCompiler.h>
#include <D3DX10math.h>

#include <assert.h>
#include <memory>

#include <string>

#include <Imagepp/h/ImageppAPI.h>
#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <ImagePP\all\h\HGF2DCoordSys.h>
#include <ImagePP\all\h\HGF2DWorldCluster.h>
#include <ImagePP\all\h\HGFHMRStdWorldCluster.h>
#include <ImagePP\all\h\HGF2DIdentity.h>
#include <ImagePP\all\h\HGF2DStretch.h>
#include <ImagePP\all\h\HGF2DAffine.h>

#include <ImagePP\all\h\HRPPixelTypeFactory.h>
#include <ImagePP\all\h\HRPPixelTypeV1Gray1.h>
#include <ImagePP\all\h\HRPPixelTypeV1GrayWhite1.h>
#include <ImagePP\all\h\HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP\all\h\HRPPixelTypeI1R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeV24R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP\all\h\HRPPixelTypeV8Gray8.h>
#include <ImagePP\all\h\HRPPixelTypeV8GrayWhite8.h>
#include <ImagePP\all\h\HRPPixelTypeI8R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeI8R8G8B8A8.h>
#include <ImagePP\all\h\HRPPixelTypeI4R8G8B8.h>
#include <ImagePP\all\h\HRPPixelPalette.h>
#include <ImagePP\all\h\HRPChannelOrgRGB.h>
#include <ImagePP\all\h\HPMPool.h>

#include <ImagePP\all\h\HRFCapability.h>
#include <ImagePP\all\h\HRFRasterFile.h>
#include <ImagePP\all\h\HRFResolutionEditor.h>
#include <ImagePP\all\h\HRFRasterFileFactory.h>
#include <ImagePP\all\h\HRFRasterFileCapabilities.h>
#include <ImagePP\all\h\HRFRasterFileBlockAdapter.h>
#include <ImagePP\all\h\HRFSLOStripAdapter.h>
#include <ImagePP\all\h\HRFIntergraphRGBFile.h>
#include <ImagePP\all\h\HRFIntergraphRGBCompressFile.h>
#include <ImagePP\all\h\HRFPWRasterFile.h>
#include <ImagePP\all\h\HRFCalsFile.h>

#include <ImagePP\all\h\HRAStoredRaster.h>
#include <ImagePP\all\h\HRABitmap.h>
#include <ImagePP\all\h\HRABitmapRLE.h>
#include <ImagePP\all\h\HRSObjectStore.h>
#include <ImagePP\all\h\HRAPixelTypeReplacer.h>
#include <ImagePP\all\h\HRACopyFromOptions.h>

#include <ImagePP\all\h\HRPPixelTypeI8R8G8B8Mask.h>
#include <ImagePP\all\h\HRPChannelOrgRGB.h>
#include <ImagePP\all\h\HRPChannelOrgGray.h>
#include <ImagePP\all\h\HRPPixelPalette.h>
#include <ImagePP\all\h\HUTImportFromRasterExportToFile.h>
#include <ImagePP\all\h\HRPPixelTypeI1R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeI1R8G8B8A8.h>
#include <ImagePP\all\h\HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP\all\h\HRPPixelTypeI1R8G8B8A8RLE.h>
#include <ImagePP\all\h\HRPPixelTypeI2R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeI4R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeI4R8G8B8A8.h>
#include <ImagePP\all\h\HRPPixelTypeI8R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeI8R8G8B8A8.h>
#include <ImagePP\all\h\HRPPixelTypeI8VA8R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeV1Gray1.h>
#include <ImagePP\all\h\HRPPixelTypeV1GrayWhite1.h>
#include <ImagePP\all\h\HRPPixelTypeV8Gray8.h>
#include <ImagePP\all\h\HRPPixelTypeV8GrayWhite8.h>
#include <ImagePP\all\h\HRPPixelTypeV16B5G5R5.h>
#include <ImagePP\all\h\HRPPixelTypeV16PRGray8A8.h>
#include <ImagePP\all\h\HRPPixelTypeV16R5G6B5.h>
#include <ImagePP\all\h\HRPPixelTypeV24B8G8R8.h>
#include <ImagePP\all\h\HRPPixelTypeV24PhotoYCC.h>
#include <ImagePP\all\h\HRPPixelTypeV24R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeV32A8R8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeV32PRPhotoYCCA8.h>
#include <ImagePP\all\h\HRPPixelTypeV32PR8PG8PB8A8.h>
#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP\all\h\HRPPixelTypeV32B8G8R8X8.h>
#include <ImagePP\all\h\HRPPixelTypeV32R8G8B8X8.h>
#include <ImagePP\all\h\HRPPixelTypeV48R16G16B16.h>
#include <ImagePP\all\h\HRPPixelTypeV96R32G32B32.h>
#include <ImagePP\all\h\HRPPixelTypeV32CMYK.h>
#include <ImagePP\all\h\HRPPixelTypeV16Gray16.h>
#include <ImagePP\all\h\HRPPixelTypeI8Gray8.h>
#include <ImagePP\all\h\HRPPixelTypeV16Int16.h>
#include <ImagePP\all\h\HRPPixelTypeV32Float32.h>
#include <ImagePP\all\h\HRPPixelTypeV64R16G16B16A16.h>
#include <ImagePP\all\h\HRPPixelTypeV64R16G16B16X16.h>
#include <ImagePP\all\h\HRPPixelTypeRGB.h>

#include <ImagePP\all\h\HRAClearOptions.h>
#include <ImagePP\all\h\HCDCodecIdentity.h>
#include <ImagePP\all\h\HGFLuvColorSpace.h>

using namespace std;


// typedef unsigned char   Byte;
// typedef unsigned short  UShort;
// typedef ULONG           UInt32;
// typedef LONG            Int32;
// typedef ULONG           uint32_t;
// 
// 
// 
// #include "HFCMath.h"

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }

#define TILE_SIZE 256

#define TILE_SIZE_BYTES TILE_SIZE*TILE_SIZE*4   // RGBA

#if !defined(IN_RANGE)
#define IN_RANGE(x,min,max)         (((x) >= (min)) && ((x) <= (max)))
#endif

#define UPPER_LEFT_COORDSYS gWorldClusterP->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)     // Origin upper-Left 
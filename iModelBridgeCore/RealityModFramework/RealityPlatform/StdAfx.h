/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/StdAfx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>

#include <Geom/GeomApi.h>
#include <Imagepp/h/ImageppAPI.h>
#include <ImagePP/all/h/HFCMacros.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HCDCodec.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/basegeocoordapi.h>

#ifdef _MANAGED
#error File type handlers cannot be built as managed assemblies.  Set the Common Language Runtime options to no CLR support in project properties.
#endif

// #ifndef _UNICODE
// #error File type handlers must be built Unicode.  Set the Character Set option to Unicode in project properties.
// #endif

#include <RealityPlatform/RealityPlatformApi.h>



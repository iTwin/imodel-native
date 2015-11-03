//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PrivateApi/ImagePPInternal/hstdcpp.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//*****************************************************************************
// hstdcpp.h
//
//      Header for HMR C++ standards
//
//*****************************************************************************
#pragma once

#include <ImagePP/h/ImageppAPI.h>

#include <Bentley/BeFileListIterator.h>

// General compiler Include files
#if defined (ANDROID) || defined (__APPLE__)


#elif defined(_WIN32) || defined(WIN32)

#   pragma inline_depth(64)

#   include <Winsock2.h>
#   include <Winerror.h>
#   include <wininet.h>
#   include <conio.h>
#   include <direct.h>
#   include <io.h>
#   include <wtypes.h>
#   include <urlmon.h>
#   include <initguid.h>
#   include <wtypes.h>     // Windows timer system
#   include <mmsystem.h>    // Windows timer system

#  if !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN
#    include "windows.h"
#    undef WIN32_LEAN_AND_MEAN
#  else
#    include "windows.h"
#  endif

#   include <concrt.h>
#   include <ppl.h>

#else
#   error Unknown compiler - No STL inclusion Standard defined
#endif


#include <Geom/GeomApi.h>
#include <BeXml/BeXml.h>
#include <GeoCoord/BaseGeoCoord.h>

#include <ImagePP/all/h/HFCMemoryLineStream.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDCodecImage.h>

#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPConvFilter.h>
#include <ImagePP/all/h/HRPFunctionFilters.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8.h>

#include <ImagePP/all/h/HGF2DPolygonOfSegments.h>
#include <ImagePP/all/h/HVEShape.h>
#include <ImagePP/all/h/HVE2DRectangle.h>
#include <ImagePP/all/h/HVE2DUniverse.h>

#include <ImagePP/all/h/HMDLayers.h>
#include <ImagePP/all/h/HPMAttribute.h>
#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HRFcTiffFile.h>
#include <ImagePP/all/h/HRFWMSFile.h>

#include <ImagePP/all/h/HRAStoredRaster.h>
#include <ImagePP/all/h/HRABitmap.h>
#include <ImagePP/all/h/HRAPyramidRaster.h>
#include <ImagePP/all/h/HPADynamicParser.h>
#include <ImagePP/all/h/HPSParser.h>

#include <ImagePP/all/h/HIMMosaic.h>
#include <ImagePP/all/h/HIMOnDemandMosaic.h>

#include <ImagePP/all/h/HGSMemorySurfaceDescriptor.h>

#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePPInternal/gra/HRAImageSampler.h>
#include <ImagePPInternal/gra/DownSampling.h>
#include <ImagePPInternal/gra/HRAImageNode.h>

USING_NAMESPACE_IMAGEPP














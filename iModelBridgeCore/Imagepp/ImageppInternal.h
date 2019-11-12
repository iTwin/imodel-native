//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------


// pragma once is giving 'error: #pragma once in main file' with GCC. Looks like a GCC bug :
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47857
//#pragma once

#ifndef __IMAGEPPINTERNAL_H__
#define __IMAGEPPINTERNAL_H__

#include <ImagePP/h/ImageppAPI.h>

#include <Bentley/BeFileListIterator.h>

// General compiler Include files
#if defined (BENTLEY_WIN32) ||defined(BENTLEY_WINRT)
    #pragma inline_depth(64)

    #include <concrt.h>
    #include <ppl.h>
    #define HAVE_CONCURRENCY_RUNTIME 

    #if defined(_M_IX86) || defined(_M_X64)
        // We include this header to support SSE2 intrinsics. 
        //Important note : There is only SSE2 intrinsics in emmintrin.h ans all Windows x64 CPU support SSE2.
        #define HAVE_SIMD_INTRINSICS
        #include <emmintrin.h>
    #endif

#elif defined(__unix__)

    #if defined (__APPLE__)
    // apple specific here
    #endif
   
    #if defined (ANDROID)
    // android specific here
    #endif

#else
    #error unknown compiler
#endif

// C++ 11 std headers
#include <thread>
#include <mutex>
#include <condition_variable>

#include <Geom/GeomApi.h>
#include <BeXml/BeXml.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <Bentley/BeThread.h>

#include <ImagePP/all/h/HFCMemoryLineStream.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDCodecImage.h>

#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPConvFilter.h>
#include <ImagePP/all/h/HRPFunctionFilters.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <ImagePP/all/h/HGF2DPolygonOfSegments.h>
#include <ImagePP/all/h/HGF2DComplexShape.h>
#include <ImagePP/all/h/HVEShape.h>
#include <ImagePP/all/h/HVE2DRectangle.h>
#include <ImagePP/all/h/HVE2DUniverse.h>

#include <ImagePP/all/h/HMDLayers.h>
#include <ImagePP/all/h/HPMAttribute.h>
#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HRFcTiffFile.h>

#include <ImagePP/all/h/HRAStoredRaster.h>
#include <ImagePP/all/h/HRABitmap.h>
#include <ImagePP/all/h/HRAPyramidRaster.h>
#include <ImagePP/all/h/HPADynamicParser.h>
#include <ImagePP/all/h/HPSParser.h>

#include <ImagePP/all/h/HIMMosaic.h>
#include <ImagePP/all/h/HIMOnDemandMosaic.h>

#include <ImagePP/all/h/HGSMemorySurfaceDescriptor.h>

#include <ImagePP/all/h/HFCLocalBinStream.h>

#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePPInternal/gra/HRAImageSampler.h>
#include <ImagePPInternal/gra/DownSampling.h>
#include <ImagePPInternal/gra/HRAImageNode.h>

USING_NAMESPACE_IMAGEPP


#endif //__IMAGEPPINTERNAL_H__

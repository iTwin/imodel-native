/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/imagepptestpch.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifdef _WIN32
// Normally I would have add this lib to BEGTEST_LIBRARIES but it requires a full path and I can't find a way to resolve it using $(LIB) delimited list.
//So I ended up adding this pragma here. see imagepp\test\dependencies.mki   
#pragma comment(lib, "Ws2_32.lib")

#include "windows.h"
#include <Winsock2.h>
#include <Winerror.h>
#include <wininet.h>
#include <conio.h>
#include <direct.h>
#include <io.h>
#include <wtypes.h>
#include <urlmon.h>
#include <initguid.h>
#include <wtypes.h>     // Windows timer system
#include <mmsystem.h>    // Windows timer system
#endif



#define IPP_USING_STATIC_LIBRARIES

/*----------------------------------------------------------------------------------------------------------------------------------------+
| Google Test
+----------------------------------------------------------------------------------------------------------------------------------------*/
//#include <gtest\gtest.h>
#include <Bentley/BeTest.h>

/*----------------------------------------------------------------------------------------------------------------------------------------+
| Host includes
+----------------------------------------------------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>

// I++
#include <ImagePP\h\ImageppAPI.h>
#include <ImagePP\all\h\HFCPtr.h>
#include <ImagePP\h\HAutoPtr.h>
#include <ImagePP\all\h\HFCMacros.h>
#include <ImagePP\all\h\HFCException.h>
#include <ImagePP\all\h\HFCMatrix.h>
#include <ImagePP\all\h\HFCURLFile.h>
#include <ImagePP\all\h\HFCStat.h>
#include <ImagePP\all\h\HFCBinStream.h>
#include <ImagePP\all\h\HFCURL.h>
#include <ImagePP\all\h\HGF2DWorld.h>
#include <ImagePP\all\h\HGFHMRStdWorldCluster.h>
#include <ImagePP\all\h\HRABitmapRLE.h>

//HPM
#include <ImagePP\all\h\HPMPool.h>

// HVE
#include <ImagePP\all\h\HVEShape.h>       
#include <ImagePP\all\h\HVE2DVoidShape.h>
#include <Imagepp\all\h\HVE2DPolygon.h>
#include <ImagePP\all\h\HVE2DPolySegment.h>
#include <ImagePP\all\h\HVE2DPolygonOfSegments.h>
#include <ImagePP\all\h\HVE2DRectangle.h>
#include <ImagePP\all\h\HVE2DShape.h>
#include <ImagePP\all\h\HVE2DSegment.h>
#include <ImagePP\all\h\HVE2DUniverse.h>
#include <ImagePP\all\h\HVE2DHoledShape.h>
#include <ImagePP\all\h\HVE2DComplexShape.h>

// HGF
#include <ImagePP\all\h\HGF2DWorld.h>
#include <ImagePP\all\h\HGF2DWorldCluster.h>
#include <ImagePP\all\h\HGF2DCoord.h>
#include <ImagePP\all\h\HGF2DPosition.h>
#include <ImagePP\all\h\HGF2DTransfoModel.h>
#include <ImagePP\all\h\HGF2DDisplacement.h>
#include <ImagePP\all\h\HGF2DIdentity.h>
#include <ImagePP\all\h\HGF2DStretch.h>
#include <ImagePP\all\h\HGF2DTranslation.h>
#include <ImagePP\all\h\HGF2DAffine.h>
#include <ImagePP\all\h\HGF2DSimilitude.h>
#include <ImagePP\all\h\HGF2DLiteExtent.h>
#include <ImagePP\all\h\HGF2DProjective.h>
#include <ImagePP\all\h\HGFHMRStdWorldCluster.h>
#include <ImagePP\all\h\HGF2DCoordSys.h>
#include <ImagePP\all\h\HGF2DLocation.h>
#include <ImagePP\all\h\HGF2DGridModel.h>
#include <ImagePP\all\h\HGF2DTransfoModelAdapter.h>
#include <ImagePP\all\h\HGFException.h>
#include <ImagePP\all\h\HGF2DHelmert.h>
#include <ImagePP\all\h\HGF2DLinearModelAdapter.h>
#include <ImagePP\all\h\HGF2DLocalProjectiveGrid.h>
#include <ImagePP\all\h\HGFAngle.h>
#include <ImagePP\all\h\HGF2DExtent.h>
#include <ImagePP\all\h\HGF2DComplexTransfoModel.h>
#include <ImagePP\all\h\HGFTolerance.h>
#include <ImagePP\all\h\HGFScanLines.h>
#include <imagePP\all\h\HGFResolutionDescriptor.h>
#include <ImagePP\all\h\HGF2DProjectiveGrid.h>
#include <ImagePP\all\h\HGF2DLiteLine.h>
#include <ImagePP\all\h\HGF2DComplexShape.h>
#include <ImagePP\all\h\HGF2DHoledShape.h>

// HGS
#include <ImagePP\all\h\HGSTypes.h>

// HRF
#include <ImagePP\all\h\HRFRasterFileFactory.h>
#include <ImagePP\all\h\HRFRasterFile.h>
#include <ImagePP\all\h\HRFcTiffFile.h>
// #include <ImagePP\all\h\HRFTypes.h>
// #include <ImagePP\all\h\HRFImportExport.h>
// #include <ImagePP\all\h\HRFTiffFile.h>
// #include <ImagePP\all\h\HRFGeoTiffFile.h>
// #include <ImagePP\all\h\HRFPageFileFactory.h>
// #include <ImagePP\all\h\HRFIntergraphCitFile.h>
// #include <ImagePP\all\h\HRFIntergraphRLEFile.h>
// #include <ImagePP\all\h\HRFHMRFile.h>
// #include <ImagePP\all\h\HRFIntergraphTG4File.>h700
// #include <ImagePP\all\h\HRFRasterFileExtender.h>
// #include <ImagePP\all\h\HRFPDFFile.h>
// #include <ImagePP\all\h\HRFUtility.h>
// #include <ImagePP\all\h\HRFAnnotationInfoPDF.h>
// #include <ImagePP\all\h\HRFCacheFileCreator.h>
// #include <ImagePP\all\h\HRFiTiffCacheFileCreator.h>
// #include <ImagePP\all\h\HRFTWFPageFile.h>
// #include <ImagePP\all\h\HRFHGRPageFile.h>
// #include <ImagePP\all\h\HRFIntergraphFile.h>
// #include <ImagePP\all\h\HRFTiffIntgrFile.h>
// #include <ImagePP\all\h\interface/IHRFPWFileHandler.h>
// #include <ImagePP\all\h\HRFRasterFileCache.h>
// #include <ImagePP\all\h\HRFGeoRasterFile.h>
// #include <ImagePP\all\h\HRFPngFile.h>
// #include <ImagePP\all\h\HRFJpegFile.h>
// #include <ImagePP\all\h\HRFRasterFileResBooster.h>
// #include <ImagePP\all\h\HRFGdalSupportedFile.h>
// #include <ImagePP\all\h\HRFGeoTiffCoordSysTable.h>
// #include <ImagePP\all\h\HRFGeoTiffProjectionTable.h>
// #include <ImagePP\all\h\HRFRasterFileBlockAdapter.h>
// #include <ImagePP\all\h\HRFSLOStripAdapter.h>
// #include <ImagePP\all\h\HRFRasterFilePageDecorator.h>
// #include <ImagePP\all\h\HRFPageDescriptor.h>
// #include <ImagePP\all\h\HRFException.h>
// #include <ImagePP\all\h\HRFCalsFile.h>
// #include <ImagePP\all\h\HRFLRDFile.h>
// #include <ImagePP\all\h\HRFErdasImgFile.h>
// #include <ImagePP\all\h\HRFPWRasterFile.h>
// #include <ImagePP\all\h\HRFWMSFile.h>
// #include <ImagePP\all\h\HRFERSPageFile.h>
// #include <ImagePP\all\h\HRFMrSIDFile.h>
// #include <ImagePP\all\h\HRFErMapperSupportedFile.h>
// #include <ImagePP\all\h\HRFErdasImgFile.h>
// #include <ImagePP\all\h\HRFVirtualEarthFile.h>
// #include <ImagePP\all\h\HRFIntergraphRGBFile.h>
// #include <ImagePP\all\h\HRFResolutionEditor.h>
// #include <ImagePP\all\h\HRFRasterFileCapabilities.h>
// #include <ImagePP\all\h\HRFPDFException.h>
// #include <ImagePP\all\h\HRFOracleException.h>


// HRP
#include <ImagePP\all\h\HRPFunctionFilter.h>
#include <ImagePP\all\h\HRPPixelTypeFactory.h>
#include <ImagePP\all\h\HRPPixelConverter.h>
#include <ImagePP\all\h\HRPComplexFilter.h>
#include <ImagePP\all\h\HRPPixelType.h>
#include <ImagePP\all\h\HRPMapFilters8.h>
#include <ImagePP\all\h\HRPDensitySlicingFilter8.h>
#include <ImagePP\all\h\HRPPaletteOctreeR8G8B8.h>
#include <ImagePP\all\h\HRPPixelTypeI8R8G8B8Mask.h>
#include <ImagePP\all\h\HRPChannelOrgRGB.h>
#include <ImagePP\all\h\HRPChannelOrgGray.h>
#include <ImagePP\all\h\HRPPixelPalette.h>
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
#include <ImagePP\all\h\HRPFunctionFilters.h>
#include <ImagePP\all\h\HRPContrastStretchFilter8.h>
#include <ImagePP\all\h\HRPLigthnessContrastStretch8.h>
#include <ImagePP\all\h\HRPDEMFilter.h>
#include <ImagePP\all\h\HRPComplexConverter.h>
#include <ImagePP\all\h\HRPCustomConvFilter.h>

//HRA
#include <ImagePP\all\h\HRABitmap.h>
#include <ImagePP\all\h\HRAReferenceToRaster.h>
#include <ImagePP\all\h\HRARaster.h>
#include <ImagePP\all\h\HRAStoredRaster.h>
#include <ImagePP\all\h\HRAImageView.h>
#include <ImagePP\all\h\HRATransactionRecorder.h>
#include <ImagePP\all\h\HRABitmapBase.h>
#include <ImagePP\all\h\HRACopyFromOptions.h>
#include <ImagePP\all\h\HRAPyramidRaster.h>
#include <ImagePP\all\h\HRAPixelTypeReplacer.h>
#include <ImagePP\all\h\HRADEMRaster.h>
#include <ImagePP\all\h\HRAClearOptions.h>
#include <ImagePP\all\h\HRAImageOpFunctionFilters.h>
#include <ImagePP\all\h\HRAImageOpContrastStretchFilter.h>
#include <ImagePP\all\h\HRAImageOpDensitySlicingFilter.h>
#include <ImagePP\all\h\HRAImageOpMapFilters.h>
#include <ImagePP\all\h\HRAImageOpConvFilter.h>

//HCD
#include <ImagePP\all\h\HCDCodecIdentity.h>
// #include <ImagePP\all\h\HCDCodecZlib.h>
// #include <ImagePP\all\h\HCDCodecLZW.h>
// #include <ImagePP\all\h\HCDPacket.h>
// #include <ImagePP\all\h\HCDCodecHMRRLE1.h>
// #include <ImagePP\all\h\HCDCodec.h>
// #include <ImagePP\all\h\HCDCodecDeflate.h>
// #include <ImagePP\all\h\HCDCodecIJG.h>
// #include <ImagePP\all\h\HCDCodecCCITT.h>
// #include <ImagePP\all\h\HCDException.h>

//HUT
#include <ImagePP\all\h\HUTImportFromRasterExportToFile.h>
#include <ImagePP\all\h\HUTImportFromFileExportToFile.h>
#include <ImagePP\all\h\HUTExportProgressIndicator.h>

//HIM
#include <ImagePP\all\h\HIMFilteredImage.h>
#include <ImagePP\all\h\HIMStripAdapter.h>
#include <ImagePP\all\h\HIMMosaic.h>
#include <ImagePP\all\h\HIMOnDemandMosaic.h>

//HRS
#include <ImagePP\all\h\HRSObjectStore.h>


// Extra STL no included by hstdcpp.h
// #include <stack>
// #include <functional>
// #include <iomanip>
#include <concrt.h>
#include <ppl.h>

// Private APIs
#include "../PrivateApi/ImagePPInternal/gra/HRAImageNearestSamplerN8.h"
#include "../PrivateApi/ImagePPInternal/gra/HRAImageBilinearSamplerN8.h"
#include "../PrivateApi/ImagePPInternal/gra/HRAImageBicubicSamplerN8.h"
#include "../PrivateApi/ImagePPInternal/gra/HRAImageSampler.h"
#include "../PrivateApi/ImagePPInternal/gra/ImageCommon.h"
#include "../PrivateApi/ImagePPInternal/gra/HRAImageEditor.h"
#include "../PrivateApi/ImagePPInternal/gra/ImageAllocator.h"

/*---------------------------------------------------------------------------------**//**
* Generic host for all our tests.
* @bsiclass                                              
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestImageppLibHost : ImagePP::ImageppLib::Host
    {
    TestImageppLibHost(){};
    virtual ~TestImageppLibHost(){};
    virtual void _RegisterFileFormat() override {}
    };

USING_NAMESPACE_IMAGEPP

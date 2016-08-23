//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFFileFormats.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This header contains all the file formats headers
//-----------------------------------------------------------------------------
//#pragma once

// To include the creators in the factory
#include "HRFMacros.h"
#include "HRFRasterFileFactory.h"

#include "HRFArcInfoGridFile.h"
#include "HRFArcInfoAsciiGridFile.h"
#include "HRFBmpFile.h"
#include "HRFBsbFile.h"
#include "HRFPictFile.h"
#include "HRFWbmpFile.h"
#include "HRFCalsFile.h"
#include "HRFGeoTiffFile.h"
#include "HRFHMRFile.h"
#include "HRFImgMappedFile.h"
#include "HRFImgRGBFile.h"
#include "HRFIntergraphCITFile.h"
#include "HRFIntergraphCOT29File.h"
#include "HRFIntergraphC30File.h"
#include "HRFIntergraphC31File.h"
#include "HRFIntergraphCotFile.h"
#include "HRFIntergraphRGBFile.h"
#include "HRFIntergraphRLEFile.h"
#include "HRFIntergraphTG4File.h"
#include "HRFIntergraphCRLFile.h"
#include "HRFIntergraphMPFFile.h"
#include "HRFIrasbRSTFile.h"
#include "HRFiTiffFile.h"
#include "HRFJpegFile.h"
#include "HRFUSgsDEMFile.h"
#include "HRFUSgsFastL7AFile.h"
#include "HRFUSgsNDFFile.h"
#include "HRFUSgsSDTSDEMFile.h"

#include "HRFPngFile.h"
#include "HRFTiffFile.h"
#include "HRFGifFile.h"
#include <ImagePP\IppImaging\HRFInternetImagingFile.h>
#include "HRFcTiffFile.h"
#include "HRFTgaFile.h"
#include "HRFRawFile.h"
#include "HRFPcxFile.h"
#include "HRFSunRasterFile.h"
#include "HRFRLCFile.h"
#include "HRFEpsFile.h"
#include "HRFTiffIntgrFile.h"
#include "HRFxChFile.h"
#include "HRFBilFile.h"
#include "HRFLRDFile.h"
#include "HRFFliFile.h"
#include "HRFDoqFile.h"
#include "HRFSpotCAPFile.h"
#include "HRFSpotDimapFile.h"
#include "HRFPWRasterFile.h"
#include "HRFErdasImgFile.h"
#include "HRFWMSFile.h"
#include "HRFNitfFile.h"
#include "HRFDtedFile.h"
#include "HRFVirtualEarthFile.h"
#include "HRFHgtFile.h"

#include "HRFErMapperSupportedFile.h"
#include "HRFMrSIDFile.h"
#include "HRFGeoRasterFile.h"
#include "HRFPDFFile.h"
#include "HRFMapboxFile.h"

#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFTWFPageFile.h>
#include <Imagepp/all/h/HRFHGRPageFile.h>
#include <Imagepp/all/h/HRFERSPageFile.h>
#include <Imagepp/all/h/HRFPRJPageFile.h>


//***********************************************
// REGISTER_FILEFORMAT_AT_HOST_INITIALIZATION
//***********************************************
#if !defined(BENTLEYPUBLISHER_FILE_FORMATS) && !defined(RASTERLIB_FILE_FORMATS)
#define HOST_REGISTER_cTiff_FILEFORMAT       HOST_REGISTER_FILEFORMAT(HRFcTiffCreator)
#else
#define HOST_REGISTER_cTiff_FILEFORMAT      
#endif

#if defined(IPP_HAVE_ERMAPPER_SUPPORT)
#define HOST_REGISTER_ERMAPPER_FILEFORMAT       \
    HOST_REGISTER_FILEFORMAT(HRFEcwCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFJpeg2000Creator)
#else
#define HOST_REGISTER_ERMAPPER_FILEFORMAT 
#endif      

#if defined(IPP_HAVE_MRSID_SUPPORT)
#define HOST_REGISTER_MRSID_FILEFORMAT   HOST_REGISTER_FILEFORMAT(HRFMrSIDCreator) 
#else
#define HOST_REGISTER_MRSID_FILEFORMAT       
#endif                                       


#if !defined(BENTLEYACTIVEXPRO_FILE_FORMATS)     // Technical problem with the ActiveX and Gdal library

#if !defined(PREVIEWHANDLER_FILE_FORMATS)
#define HOST_REGISTER_GDAL_FILEFORMAT               \
    HOST_REGISTER_FILEFORMAT(HRFBsbCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFUSgsNDFCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFUSgsFastL7ACreator) \
    HOST_REGISTER_FILEFORMAT(HRFNitfCreator)        \
    HOST_REGISTER_FILEFORMAT(HRFCalsCreator)        \
    HOST_REGISTER_FILEFORMAT(HRFLRDCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFInternetImagingFileCreator) \
    HOST_REGISTER_FILEFORMAT(HRFWMSCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFxChCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFBilCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFFliCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFDoqCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFSpotCAPCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFSpotDimapCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFErdasImgCreator)    \
    HOST_REGISTER_FILEFORMAT(HRFDtedCreator)        \
    HOST_REGISTER_FILEFORMAT(HRFUSgsSDTSDEMCreator) \
    HOST_REGISTER_FILEFORMAT(HRFUSgsDEMCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFArcInfoGridCreator) \
    HOST_REGISTER_FILEFORMAT(HRFArcInfoAsciiGridCreator)
#else
#define HOST_REGISTER_GDAL_FILEFORMAT               \
    HOST_REGISTER_FILEFORMAT(HRFBsbCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFUSgsNDFCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFUSgsFastL7ACreator) \
    HOST_REGISTER_FILEFORMAT(HRFNitfCreator)        \
    HOST_REGISTER_FILEFORMAT(HRFCalsCreator)        \
    HOST_REGISTER_FILEFORMAT(HRFLRDCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFxChCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFBilCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFFliCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFDoqCreator)         \
    HOST_REGISTER_FILEFORMAT(HRFSpotCAPCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFSpotDimapCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFErdasImgCreator)    \
    HOST_REGISTER_FILEFORMAT(HRFDtedCreator)        \
    HOST_REGISTER_FILEFORMAT(HRFUSgsSDTSDEMCreator) \
    HOST_REGISTER_FILEFORMAT(HRFUSgsDEMCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFArcInfoGridCreator) \
    HOST_REGISTER_FILEFORMAT(HRFArcInfoAsciiGridCreator)
#endif

#else
#define HOST_REGISTER_GDAL_FILEFORMAT            
#endif

// RasterLib exclusion
#if !defined(RASTERLIB_FILE_FORMATS) 
#if !defined(PROJECTWISE_FILE_FORMATS)
#define HOST_REGISTER_Irasb_FILEFORMAT  HOST_REGISTER_FILEFORMAT(HRFIrasbRSTCreator)     
#else
#define HOST_REGISTER_Irasb_FILEFORMAT   
#endif

#define HOST_REGISTER_Raw_FILEFORMAT    HOST_REGISTER_FILEFORMAT(HRFRawCreator)       
#define HOST_REGISTER_Eps_FILEFORMAT    HOST_REGISTER_FILEFORMAT(HRFEpsCreator)     
#else
#define HOST_REGISTER_Irasb_FILEFORMAT
#define HOST_REGISTER_Raw_FILEFORMAT              
#define HOST_REGISTER_Eps_FILEFORMAT              
#endif

#if !defined(PROJECTWISE_FILE_FORMATS) && !defined(IPPSTATICLIB_FILE_FORMATS) && !defined(PREVIEWHANDLER_FILE_FORMATS) && !defined(_DEBUG) && defined(__IPP_EXTERNAL_THIRD_PARTY_SUPPORTED)
#define HOST_REGISTER_PDF_FILEFORMAT    HOST_REGISTER_FILEFORMAT(HRFPDFCreator)       
#else
#define HOST_REGISTER_PDF_FILEFORMAT   
#endif

#if !defined(PROJECTWISE_FILE_FORMATS) && !defined(IPPSTATICLIB_FILE_FORMATS) && !defined(PREVIEWHANDLER_FILE_FORMATS) && !defined(_DEBUG) && defined(__IPP_EXTERNAL_THIRD_PARTY_SUPPORTED)
#define HOST_REGISTER_GeoRaster_FILEFORMAT  HOST_REGISTER_FILEFORMAT(HRFGeoRasterCreator)
#else
#define HOST_REGISTER_GeoRaster_FILEFORMAT  
#endif

#if !defined(PROJECTWISE_FILE_FORMATS) && !defined(BENTLEYPUBLISHER_FILE_FORMATS) && !defined(IPPSTATICLIB_FILE_FORMATS) && !defined(PREVIEWHANDLER_FILE_FORMATS)
#define HOST_REGISTER_VirtualEarth_FILEFORMAT   HOST_REGISTER_FILEFORMAT(HRFVirtualEarthCreator)
#else
#define HOST_REGISTER_VirtualEarth_FILEFORMAT   
#endif

//PROTOTYPE - NOT YET LEGAL TO USE IN COMMERCIAL APPLICATION
//#define HOST_REGISTER_MapBox_FILEFORMAT   HOST_REGISTER_FILEFORMAT(HRFMapBoxCreator)
#define HOST_REGISTER_MapBox_FILEFORMAT   

#if !defined(PREVIEWHANDLER_FILE_FORMATS)
#define HOST_REGISTER_ProjectWise_FILEFORMAT  HOST_REGISTER_FILEFORMAT(HRFPWCreator)              
#else
#define HOST_REGISTER_ProjectWise_FILEFORMAT                

#endif


#ifndef EXCLUDE_WINDOWS_FILE_FORMATS
#   if !defined(BENTLEYPUBLISHER_FILE_FORMATS) && !defined(PROJECTWISE_FILE_FORMATS) && !defined(IPPSTATICLIB_FILE_FORMATS) && !defined(PREVIEWHANDLER_FILE_FORMATS)
#       include <ImagePP/all/h/HRFWebFile.h>
#define HOST_REGISTER_WebFile_FILEFORMAT    HOST_REGISTER_FILEFORMAT(HRFWebFileCreator) 
#   else
#define HOST_REGISTER_WebFile_FILEFORMAT   
#   endif
#else
#define HOST_REGISTER_WebFile_FILEFORMAT   
#endif

#ifdef EXCLUDE_PAGE_FILES
    #define HOST_REGISTER_PAGEFILES
#else
    #if defined(IPP_HAVE_ERMAPPER_SUPPORT) 
        #define HOST_REGISTER_PAGEFILES \
            HRFPageFileFactory::GetInstance()->Register(HRFPRJPageFileCreator::GetInstance()); \
            HRFPageFileFactory::GetInstance()->Register(HRFERSPageFileCreator::GetInstance()); \
            HRFPageFileFactory::GetInstance()->Register(HRFTWFPageFileCreator::GetInstance()); \
            HRFPageFileFactory::GetInstance()->Register(HRFHGRPageFileCreator::GetInstance());
    #else
        #define HOST_REGISTER_PAGEFILES \
            HRFPageFileFactory::GetInstance()->Register(HRFPRJPageFileCreator::GetInstance()); \
            HRFPageFileFactory::GetInstance()->Register(HRFTWFPageFileCreator::GetInstance()); \
            HRFPageFileFactory::GetInstance()->Register(HRFHGRPageFileCreator::GetInstance());
    #endif        
#endif

#define REGISTER_SUPPORTED_FILEFORMAT            \
    HOST_REGISTER_cTiff_FILEFORMAT               \
    HOST_REGISTER_PAGEFILES                      \
    HOST_REGISTER_FILEFORMAT(HRFiTiffCreator)    \
    HOST_REGISTER_FILEFORMAT(HRFiTiff64Creator)  \
    HOST_REGISTER_FILEFORMAT(HRFHMRCreator)      \
    HOST_REGISTER_FILEFORMAT(HRFTiffCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFGeoTiffCreator)  \
    HOST_REGISTER_FILEFORMAT(HRFJpegCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFPictCreator)     \
    HOST_REGISTER_FILEFORMAT(HRFGifCreator)      \
    HOST_REGISTER_FILEFORMAT(HRFTgaCreator)      \
    HOST_REGISTER_FILEFORMAT(HRFSunRasterCreator)\
    HOST_REGISTER_FILEFORMAT(HRFImgMappedCreator)\
    HOST_REGISTER_FILEFORMAT(HRFImgRGBCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFRLCCreator)      \
    HOST_REGISTER_FILEFORMAT(HRFBmpCreator)      \
    HOST_REGISTER_FILEFORMAT(HRFPngCreator)      \
    HOST_REGISTER_FILEFORMAT(HRFPcxCreator)      \
    HOST_REGISTER_ERMAPPER_FILEFORMAT            \
    HOST_REGISTER_MRSID_FILEFORMAT               \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphMPFCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphCitCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphCot29Creator) \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphC30Creator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphC31Creator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphCotCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphRGBCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphRleCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphTG4Creator)   \
    HOST_REGISTER_FILEFORMAT(HRFIntergraphCRLCreator)   \
    HOST_REGISTER_FILEFORMAT(HRFTiffIntgrCreator)       \
    HOST_REGISTER_FILEFORMAT(HRFWbmpCreator)            \
    HOST_REGISTER_FILEFORMAT(HRFHgtCreator)             \
    HOST_REGISTER_GDAL_FILEFORMAT                       \
    HOST_REGISTER_Irasb_FILEFORMAT                      \
    HOST_REGISTER_Raw_FILEFORMAT                        \
    HOST_REGISTER_Eps_FILEFORMAT                        \
    HOST_REGISTER_PDF_FILEFORMAT                        \
    HOST_REGISTER_GeoRaster_FILEFORMAT                  \
    HOST_REGISTER_VirtualEarth_FILEFORMAT               \
    HOST_REGISTER_ProjectWise_FILEFORMAT                \
    HOST_REGISTER_WebFile_FILEFORMAT                    \
    HOST_REGISTER_MapBox_FILEFORMAT




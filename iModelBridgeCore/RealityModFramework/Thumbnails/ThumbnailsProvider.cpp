/*--------------------------------------------------------------------------------------+
|
|     $Source: Thumbnails/ThumbnailsProvider.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRFThumbnail.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRFBmpFile.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFGeoTiffFile.h>
#include <Imagepp/all/h/HRFHMRFile.h>
#include <Imagepp/all/h/HRFImgRGBFile.h>
#include <Imagepp/all/h/HRFIntergraphCITFile.h>
#include <Imagepp/all/h/HRFIntergraphCOT29File.h>
#include <Imagepp/all/h/HRFIntergraphCotFile.h>
#include <Imagepp/all/h/HRFIntergraphRGBFile.h>
#include <Imagepp/all/h/HRFIntergraphRLEFile.h>
#include <Imagepp/all/h/HRFIntergraphTG4File.h>
#include <Imagepp/all/h/HRFIntergraphC30File.h>
#include <Imagepp/all/h/HRFIntergraphC31File.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFPngFile.h>
#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HRFGifFile.h>
#include <Imagepp/all/h/HRFRLCFile.h>
#include <Imagepp/all/h/HRFImgMappedFile.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFTgaFile.h>
#include <Imagepp/all/h/HRFPcxFile.h>
#include <Imagepp/all/h/HRFSunRasterFile.h>
#include <Imagepp/all/h/HRFTiffIntgrFile.h>
#include <Imagepp/all/h/HRFBilFile.h>

#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HFCStat.h>

/*----------------------------------------------------------------------------+
* @bsiclass
+----------------------------------------------------------------------------*/
struct FactoryScanOnOpenGuard
    {
    private:
        bool              m_oldValue;

        // Disabled
        FactoryScanOnOpenGuard(FactoryScanOnOpenGuard const & object);
        FactoryScanOnOpenGuard& operator=(FactoryScanOnOpenGuard const & object);

    public:
        FactoryScanOnOpenGuard(bool newValue)
            {
            m_oldValue = HRFRasterFileFactory::GetInstance()->GetFactoryScanOnOpen();
            HRFRasterFileFactory::GetInstance()->SetFactoryScanOnOpen(newValue);
            }
        ~FactoryScanOnOpenGuard()
            {
            HRFRasterFileFactory::GetInstance()->SetFactoryScanOnOpen(m_oldValue);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThumbnailsProvider::GetRasterThumbnail(HBITMAP *pThumbnailBmp, WCharCP filename, UInt32 width, UInt32 height)
    {
    StatusInt hr = ExtractRasterThumbnail(pThumbnailBmp, filename, width, height);

    if (*pThumbnailBmp == NULL)
        return ERROR;

    return hr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static HFCPtr<HRFRasterFile> GetRasterFile(WCharCP InputFilename)
    {
    HFCPtr<HRFRasterFile> RasterFile;

    try
        {
        WString filename(InputFilename);

        if (filename.empty())
            return NULL;

        // Create URL
        HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(filename));
        if (SrcFileName == 0)
            {
            // Open the raster file as a file
            SrcFileName = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + filename);
            }

        // Open Raster file
        {
//        HFCMonitor __keyMonitor(m_KeyByMethod);
        FactoryScanOnOpenGuard __wantScan(false);

        // Create URL
        HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(filename));
        if (SrcFileName == 0)
            {
            // Open the raster file as a file
            SrcFileName = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + filename);
            }

        // Open Raster file without checking "isKindOfFile"
        RasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, true);
        }

        if (RasterFile == 0)
            return RasterFile;

        // Check if we have an internet imaging file
        // DISABLED: We do not support HRFInternetImagingFile
        //         if (RasterFile->IsCompatibleWith(HRFInternetImagingFile::CLASS_ID))
        //             ((HFCPtr<HRFInternetImagingFile>&)RasterFile)->DownloadAttributes();

        // Adapt Scan Line Orientation (1 bit images)
        bool CreateSLOAdapter = false;

        if ((RasterFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID)) ||
            (RasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID)))
            {
            if (HRFSLOStripAdapter::NeedSLOAdapterFor(RasterFile))
                {
                // Adapt only when the raster file has not a standard scan line orientation
                // i.e. with an upper left origin, horizontal scan line.
                //pi_rpRasterFile = HRFSLOStripAdapter::CreateBestAdapterFor(pi_rpRasterFile);
                CreateSLOAdapter = true;
                }
            }
        }
    catch (HFCException&)
        {
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"Exception!-%ls", e.GetExceptionMessage().c_str());
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"LoadRasterFile %ls FAILED!", m_fileName.c_str());

        return NULL;
        }

    catch (exception &e)
        {
        //C++ exception
        ostringstream errorStr;

        errorStr << "Caught " << e.what() << endl;
        errorStr << "Type " << typeid(e).name() << endl;
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"Exception!-%s", errorStr.str().c_str());
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"LoadRasterFile %ls FAILED!", m_fileName.c_str());

        return NULL;
        }

    catch (...)
        {
//         RasterFileHandlerLogger::GetLogger()->message(LOG_FATAL, L"Unknown Exception!");
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"LoadRasterFile %ls FAILED!", m_fileName.c_str());

        return NULL;
        }

    return RasterFile;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThumbnailsProvider::ExtractRasterThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height)
    {
    try
        {
        // Get the rasterFile 
        HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(InputFilename);

        if (rasterFile == NULL)
            return ERROR;

        // Generate the thumbnail
        HFCPtr<HRFThumbnail>  pThumbnail = HRFThumbnailMaker(rasterFile, 0, &width, &height, false);

        bool isAborted((pThumbnail == NULL));

//         if (pThumbnail == NULL)
//             RasterFileHandlerLogger::GetLogger()->messagev(LOG_ERROR, L"End extract thumbnail:%ls-%ls", m_fileName.c_str(), L" thumbnail is null.");
//         else if (m_wasAborted)
//             RasterFileHandlerLogger::GetLogger()->messagev(LOG_ERROR, L"End extract thumbnail:%ls-%ls", m_fileName.c_str(), L" TIMEOUT; abort operation!");


        if (isAborted)
            return ERROR;

//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_TRACE, L"End extract thumbnail:%ls-SUCCESS!", m_fileName.c_str());

        HFCPtr<HRPPixelType> pPixelType = rasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
        RasterFacility::CreateHBitmapFromHRFThumbnail(pThumbnailBmp, pThumbnail, pPixelType);

//         RasterFileHandlerLogger::GetLogger()->message(LOG_TRACE, L"SUCCESS!");

        return SUCCESS;
        }
    catch (HFCException& )
        {
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"Exception!-%ls", e.GetExceptionMessage().c_str());
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"%ls-FAILED!", m_fileName.c_str());

        return ERROR;
        }

    catch (exception &e)
        {
        //C++ exception
        ostringstream errorStr;

        errorStr << "Caught " << e.what() << endl;
        errorStr << "Type " << typeid(e).name() << endl;
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"Exception!-%s", errorStr.str().c_str());
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"%ls-FAILED!", m_fileName.c_str());

        return ERROR;
        }

    catch (...)
        {
//         RasterFileHandlerLogger::GetLogger()->message(LOG_FATAL, L"Unknown Exception!");
//         RasterFileHandlerLogger::GetLogger()->messagev(LOG_FATAL, L"%ls-FAILED!", m_fileName.c_str());

        return ERROR;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int ThumbnailsProvider::GetPointCloudThumbnail(HBITMAP *pThumbnailBmp, WCharCP filename, UInt32 width, UInt32 height, PointCloudView pointCloudView)
    {
    StatusInt hr = ExtractPointCloudThumbnail(pThumbnailBmp, filename, width, height, pointCloudView);

    if (*pThumbnailBmp == NULL)
        return ERROR;

    return hr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThumbnailsProvider::ExtractPointCloudThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height, PointCloudView pointCloudView)
{
    if (InputFilename == NULL)
        return ERROR;

    PtHandle cloudFileHandle;
    PtHandle cloudHandle;
    if (PointCloudVortex::OpenPOD(cloudFileHandle, cloudHandle, InputFilename) != S_OK)
        {
        return ERROR;
        }

    // Set density (number of points retrieved) according to number of pixels in bitmap. Using the total number of pixels in the bitmap * 4
    // seems to produce generally good results.
    float densityValue = (float) (width * height * 4);

    // Get transfo matrix to fit the point cloud in the thumbnail
    Transform transform;
    PointCloudVortex::GetTransformForThumbnail(transform, cloudHandle, width, height, pointCloudView);

    bool needsWhiteBackground = PointCloudVortex::PointCloudNeedsWhiteBackground(cloudHandle);

	HRESULT hr = PointCloudVortex::ExtractPointCloud(pThumbnailBmp, width, height, cloudHandle, densityValue, transform, needsWhiteBackground);
    if (*pThumbnailBmp == NULL)
        return ERROR;

    // Close point cloud file
    PointCloudVortex::ClosePOD(cloudFileHandle);

    if (hr != NOERROR)
        return ERROR;
    return SUCCESS;
    }


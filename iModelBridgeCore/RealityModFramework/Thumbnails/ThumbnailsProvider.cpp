/*--------------------------------------------------------------------------------------+
|
|     $Source: Thumbnails/ThumbnailsProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HVE2DRectangle.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/basegeocoordapi.h>

USING_NAMESPACE_IMAGEPP

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
StatusInt ThumbnailsProvider::GetRasterThumbnail(HBITMAP *pThumbnailBmp, WCharCP filename, uint32_t width, uint32_t height)
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
StatusInt ThumbnailsProvider::ExtractRasterThumbnail(HBITMAP* pThumbnailBmp, WCharCP inputFilename, uint32_t width, uint32_t height)
    {
    try
        {
        // Get the rasterFile 
        HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(inputFilename);
        
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
int ThumbnailsProvider::GetPointCloudThumbnail(HBITMAP *pThumbnailBmp, WCharCP filename, uint32_t width, uint32_t height, PointCloudView pointCloudView)
    {
    StatusInt hr = ExtractPointCloudThumbnail(pThumbnailBmp, filename, width, height, pointCloudView);

    if (*pThumbnailBmp == NULL)
        return ERROR;

    return hr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThumbnailsProvider::ExtractPointCloudThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView)
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


//-----------------------------------------------------------------------------
// public
// GetTransfoModelToMeters
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> GetTransfoModelToMeters(IRasterBaseGcsCR projection)
{
    double toMeters = 1.0 / projection.GetUnitsFromMeters();

    HFCPtr<HGF2DTransfoModel> pUnitConvertion(new HGF2DStretch(HGF2DDisplacement(0.0, 0.0), toMeters, toMeters));

    return pUnitConvertion;
}



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* GetBoundingBox(HFCPtr<HRFRasterFile>& pRasterFile, uint32_t& nbPts)
{
    if (pRasterFile == 0 || pRasterFile->CountPages() <= 0)
        return 0;

    try
    {
        HGFHMRStdWorldCluster worldCluster;

        HFCPtr<HRFPageDescriptor> pPageDescriptor = pRasterFile->GetPageDescriptor(0);
        IRasterBaseGcsCP pSrcFileGeocoding = pPageDescriptor->GetGeocodingCP();

        
        IRasterGeoCoordinateServices* geoCoordServices = ImageppLib::GetDefaultIRasterGeoCoordinateServicesImpl();
        
        GeoCoordinates::BaseGCSPtr initialGCS = GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
        if (!initialGCS->IsValid())
            return 0;

        IRasterBaseGcsPtr pDestGeoCoding = geoCoordServices->_CreateRasterBaseGcsFromBaseGcs(initialGCS.get());
        
        // Georeference
        if ((pSrcFileGeocoding != 0) &&
            (pSrcFileGeocoding->IsValid()) &&
            (pDestGeoCoding->IsValid()))
        {
            IRasterBaseGcsPtr     pDestCoordSys(pDestGeoCoding->Clone());

            // Compute the image extent expressed in the src projection units
            HFCPtr<HGF2DCoordSys> pSrcWorldMeters(worldCluster.GetCoordSysReference(HGF2DWorld_HMRWORLD));

            HFCPtr<HGF2DCoordSys> pSrcUnitCoordSys = new HGF2DCoordSys(*GetTransfoModelToMeters(*(pSrcFileGeocoding->Clone())), pSrcWorldMeters);
            HFCPtr<HGF2DCoordSys> pPhysicalCoordSys;

            HFCPtr<HRFResolutionDescriptor> pRes(pPageDescriptor->GetResolutionDescriptor(0));

            if (pPageDescriptor->HasTransfoModel())
            {
                pPhysicalCoordSys = new HGF2DCoordSys(*pPageDescriptor->GetTransfoModel(),
                                                      worldCluster.GetCoordSysReference(pRasterFile->GetWorldIdentificator()));
            }
            else
            {
                pPhysicalCoordSys = new HGF2DCoordSys(HGF2DIdentity(),
                                                      worldCluster.GetCoordSysReference(pRasterFile->GetWorldIdentificator()));
            }

            // Create the reprojection transfo model (non adapted)
            HCPGCoordModel bob1(*pDestCoordSys, *(pSrcFileGeocoding->Clone()));
            HFCPtr<HGF2DTransfoModel> pDstToSrcTransfoModel(new HCPGCoordModel(*pDestCoordSys, *(pSrcFileGeocoding->Clone())));

            // Create the reprojected coordSys
            HGF2DCoordSys bob2(*pDstToSrcTransfoModel, pSrcUnitCoordSys);
            HFCPtr<HGF2DCoordSys> pDstCoordSys(new HGF2DCoordSys(*pDstToSrcTransfoModel, pSrcUnitCoordSys));

            CHECK_HUINT64_TO_HDOUBLE_CONV(pRes->GetWidth())
                CHECK_HUINT64_TO_HDOUBLE_CONV(pRes->GetHeight())

                double ImageWidth = (double)pRes->GetWidth();
            double ImageHeight = (double)pRes->GetHeight();

            HFCPtr<HVEShape> pImageRectangle(new HVEShape(HVE2DRectangle(0.0, 0.0, ImageWidth, ImageHeight, pPhysicalCoordSys)));
            pImageRectangle->ChangeCoordSys(pSrcUnitCoordSys);
            HGF2DExtent ImageExtent(pImageRectangle->GetExtent());
            HGF2DLiteExtent ImageLiteExtent(ImageExtent.GetXMin(), ImageExtent.GetYMin(), ImageExtent.GetXMax(), ImageExtent.GetYMax());

            // Compute the expected mean error
            double ImageCenter_x(ImageWidth / 2.0);
            double ImageCenter_y(ImageHeight / 2.0);

            HFCPtr<HVEShape> pPixelShape(new HVEShape(HVE2DRectangle(ImageCenter_x - 0.5*ImageWidth, ImageCenter_y - 0.5*ImageHeight, ImageCenter_x + 0.5*ImageWidth, ImageCenter_y + 0.5*ImageHeight, pPhysicalCoordSys)));
            pPixelShape->ChangeCoordSys(pDstCoordSys);

            // Get shape and list of points.
            HGF2DLocationCollection pointCollection;
            pPixelShape->GetShapePtr()->Drop(&pointCollection, 0);
            
            assert(pointCollection.size() != 0);
            nbPts = (uint32_t)pointCollection.size();
           
            double* pPts = new double[nbPts*2];

            size_t j = -1;
            for (size_t i = 0; i < nbPts; ++i)
            {
                pPts[++j] = pointCollection[i].GetX();
                pPts[++j] = pointCollection[i].GetY();
            }
            
            return pPts;
        }
        return 0;
    }
    catch (...)
    {
        return 0;
    }


        /*
        HFCPtr<HRFPageDescriptor> pPageDescriptor = pRasterFile->GetPageDescriptor(0);

        // Create logical and physical CoordSys
        HGFHMRStdWorldCluster MyWorldCluster;
        HFCPtr<HGF2DCoordSys> pLogicalCoordSys(MyWorldCluster.GetCoordSysReference(pRasterFile->GetWorldIdentificator()));
        HFCPtr<HGF2DCoordSys> pPhysicalCoordSys;

        if ((pRasterFile->GetCapabilities()->GetCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_READ_ONLY) != 0) &&
            (pPageDescriptor->HasTransfoModel()))
        {
            pPhysicalCoordSys = new HGF2DCoordSys(*pPageDescriptor->GetTransfoModel(), pLogicalCoordSys);
        }
        else
        {
            pPhysicalCoordSys = new HGF2DCoordSys(HGF2DIdentity(), pLogicalCoordSys);
        }

        // Get the shape in logical CS
        HFCPtr<HVEShape> pShape;
        if (pPageDescriptor->HasClipShape())
        {
            pShape = new HVEShape(*pPageDescriptor->GetClipShape());

            if (pPageDescriptor->GetClipShape()->GetCoordinateType() == HRFCoordinateType::PHYSICAL)
                pShape->SetCoordSys(pPhysicalCoordSys);
            else
                pShape->SetCoordSys(pLogicalCoordSys);

            pShape->ChangeCoordSys(pLogicalCoordSys);
        }
        else
        {
            if (pPageDescriptor->CountResolutions() <= 0)
                return ERROR;

            HFCPtr<HRFResolutionDescriptor> pResDescriptor(pPageDescriptor->GetResolutionDescriptor(0));

            pShape = new HVEShape(0.0, 0.0,
                                  (double)pResDescriptor->GetWidth(), (double)pResDescriptor->GetHeight(),
                                  pPhysicalCoordSys);
            pShape->ChangeCoordSys(pLogicalCoordSys);
        }

        if (pShape == 0)
        {
            status = ERROR;
        }
        else
        {
            // Make sure the shape is always in "HMR" world, with Y axis pointing upwards.
            HFCPtr<HGF2DCoordSys> coordSys = MyWorldCluster.GetWorldReference(HGF2DWorld_HMRWORLD);
            pShape->ChangeCoordSys(coordSys);

            // Extract the extent
            HGF2DExtent ShapeExtent(pShape->GetExtent());

            *pXMin = ShapeExtent.GetOrigin().GetX();
            *pYMin = ShapeExtent.GetOrigin().GetY();
            *pXMax = ShapeExtent.GetCorner().GetX();
            *pYMax = ShapeExtent.GetCorner().GetY();
        }
    }
    catch (...)
    {
        status = ERROR;
    }

    return status;
    */
}

enum WktFlavor
{
    WktFlavor_Oracle9 = 1,
    WktFlavor_Autodesk,
    WktFlavor_OGC,
    WktFlavor_End,
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool MapWktFlavorEnum(GeoCoordinates::BaseGCS::WktFlavor& baseGcsWktFlavor, WktFlavor wktFlavor)
{
    //Temporary use numeric value until the basegcs enum match the csmap's one.
    switch (wktFlavor)
    {
    case WktFlavor::WktFlavor_Oracle9:
        baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorOracle9;
        break;

    case WktFlavor::WktFlavor_Autodesk:
        baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorAutodesk;
        break;

    case WktFlavor::WktFlavor_OGC:
        baseGcsWktFlavor = GeoCoordinates::BaseGCS::wktFlavorOGC;
        break;

    default:
        return false;
    }

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCS::WktFlavor GetWKTFlavor(WString* wktStrWithoutFlavor, const WString& wktStr)
{
    WktFlavor wktFlavor = WktFlavor::WktFlavor_Oracle9;

    size_t charInd = wktStr.size() - 1;

    for (charInd = wktStr.size() - 1; charInd >= 0; charInd--)
    {
        if (wktStr[charInd] == L']')
        {
            break;
        }
        else
            if (((short)wktStr[charInd] >= 1) || ((short)wktStr[charInd] < WktFlavor::WktFlavor_End))
            {
                wktFlavor = (WktFlavor)wktStr[charInd];
            }
    }

    if (wktStrWithoutFlavor != 0)
    {
        *wktStrWithoutFlavor = wktStr.substr(0, charInd + 1);
    }

    GeoCoordinates::BaseGCS::WktFlavor baseGcsWktFlavor;

    bool result = MapWktFlavorEnum(baseGcsWktFlavor, wktFlavor);

    assert(result == true);

    return baseGcsWktFlavor;
}



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* GetBoundingBox(uint32_t& nbPts, PtHandle cloudHandle, PointCloudView  pointCloudView)
{
    // Get bounds for first point cloud in the scene
    double   lower[3], upper[3];
    PointCloudVortex::GetPointCloudBounds(cloudHandle, lower, upper);

    PtHandle metadataHandle = PointCloudVortex::GetMetaDataHandle(cloudHandle);
    if (0 == metadataHandle)
        return 0;

    wchar_t buffer[1024] = L"";

    if (!PointCloudVortex::GetMetaTag(metadataHandle, L"Survey.GeoReference", buffer))
        return 0;

    WString podWkt(buffer);
    if (podWkt.empty())
        return 0;

    // SrcGCS
    StatusInt warning;
    WString warningErrorMsg;

    WString wktWithoutFlavor;
    GeoCoordinates::BaseGCS::WktFlavor wktFlavor = GetWKTFlavor(&wktWithoutFlavor, podWkt);

    GeoCoordinates::BaseGCSPtr pSrcGcs = GeoCoordinates::BaseGCS::CreateGCS();
    if (pSrcGcs.IsValid())
    {
        pSrcGcs->InitFromWellKnownText(&warning, &warningErrorMsg, wktFlavor, wktWithoutFlavor.GetWCharCP());
    }

    // DestGCS
    GeoCoordinates::BaseGCSPtr pDestGcs = GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
    if (!pDestGcs->IsValid())
        return 0;


    double lowerX = 0.;
    double lowerY = 0.;
    double upperX = 0.;
    double upperY = 0.;

    baseGeoCoord_reproject(&lowerX, &lowerY, lower[0], lower[1], &*pSrcGcs, &*pDestGcs);
    baseGeoCoord_reproject(&upperX, &upperY, upper[0], upper[1], &*pSrcGcs, &*pDestGcs);

    // Create the bounding rectangle.
    nbPts = 5;
    DPoint2d rectPts[5];
    rectPts[0].x = lowerX;
    rectPts[0].y = lowerY;

    rectPts[1].x = lowerX;
    rectPts[1].y = upperY;

    rectPts[2].x = upperX;
    rectPts[2].y = upperY;

    rectPts[3].x = upperX;
    rectPts[3].y = lowerY;

    rectPts[4].x = lowerX;
    rectPts[4].y = lowerY;

    double* pPts = new double[nbPts * 2];

    size_t j = -1;
    for (size_t i = 0; i < nbPts; ++i)
    {
        pPts[++j] = rectPts[i].x;
        pPts[++j] = rectPts[i].y;
    }

    return pPts;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* ThumbnailsProvider::GetRasterFootprint(uint32_t& nbPts, WCharCP inputFilename)
{  
    return ExtractRasterFootprint(nbPts, inputFilename);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* ThumbnailsProvider::GetPointCloudFootprint(uint32_t& nbPts, WCharCP inputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView)
{
    return ExtractPointCloudFootprint(nbPts, inputFilename, width, height, pointCloudView);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* ThumbnailsProvider::ExtractRasterFootprint(uint32_t& nbPts, WCharCP inputFilename)
{
    try
    {
        // Get raster.
        HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(inputFilename);

        if (rasterFile == NULL)
            return 0;
        
        // Get bounding box.
        return GetBoundingBox(rasterFile, nbPts);      
    }
    catch (...)
    {
        return 0;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* ThumbnailsProvider::ExtractPointCloudFootprint(uint32_t& nbPts, WCharCP inputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView)
{
    if (inputFilename == NULL)
        return 0;

    PtHandle cloudFileHandle;
    PtHandle cloudHandle;
    if (PointCloudVortex::OpenPOD(cloudFileHandle, cloudHandle, inputFilename) != S_OK)
    {
        return 0;
    }

    return GetBoundingBox(nbPts, cloudHandle, pointCloudView);
}


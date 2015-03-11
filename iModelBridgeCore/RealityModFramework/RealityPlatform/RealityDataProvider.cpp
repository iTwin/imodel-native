/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include <RealityPlatform/RealityDataProvider.h>
#include "PointCloudVortex.h"

// For now we are only dealing with bounding boxes (5 points, first = last) and points are stored in an array of doubles (coordX, coordY).
#define FOOTPRINT_SIZE      10
#define FOOTPRINT_PTS_NBR   5

#define THUMBNAIL_WIDTH     256
#define THUMBNAIL_HEIGHT    256

USING_NAMESPACE_IMAGEPP

enum WktFlavor
    {
    WktFlavor_Oracle9 = 1,
    WktFlavor_Autodesk,
    WktFlavor_OGC,
    WktFlavor_End,
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
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
            //&&MM remove this patch from the the factory. It could be a method param but not a setting guard thing.
            HRFRasterFileFactory::GetInstance()->SetFactoryScanOnOpen(m_oldValue);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGF2DTransfoModel> GetTransfoModelToMeters(IRasterBaseGcsCR projection)
    {
    double toMeters = 1.0 / projection.GetUnitsFromMeters();

    HFCPtr<HGF2DTransfoModel> pUnitConvertion(new HGF2DStretch(HGF2DDisplacement(0.0, 0.0), toMeters, toMeters));

    return pUnitConvertion;
    }

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
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static HFCPtr<HRFRasterFile> GetRasterFile(WCharCP inFilename)
    {
    HFCPtr<HRFRasterFile> rasterFile;

    try
        {
        WString filename(inFilename);

        if (filename.empty())
            return NULL;

        // Create URL
        HFCPtr<HFCURL>  srcFilename(HFCURL::Instanciate(filename));
        if (srcFilename == 0)
            {
            // Open the raster file as a file
            srcFilename = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + filename);
            }

        // Open Raster file
            {
            // HFCMonitor __keyMonitor(m_KeyByMethod);
            FactoryScanOnOpenGuard __wantScan(false);

            // Create URL
            HFCPtr<HFCURL>  srcFilename(HFCURL::Instanciate(filename));
            if (srcFilename == 0)
                {
                // Open the raster file as a file
                srcFilename = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + filename);
                }

            // Open Raster file without checking "isKindOfFile"
            rasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)srcFilename, true);
                }

        if (rasterFile == 0)
            return rasterFile;

        // Check if we have an internet imaging file
        // DISABLED: We do not support HRFInternetImagingFile
        //         if (RasterFile->IsCompatibleWith(HRFInternetImagingFile::CLASS_ID))
        //             ((HFCPtr<HRFInternetImagingFile>&)RasterFile)->DownloadAttributes();

        // Adapt Scan Line Orientation (1 bit images)
        bool CreateSLOAdapter = false;

        if ((rasterFile->IsCompatibleWith(HRFFileId_Intergraph)) ||
            (rasterFile->IsCompatibleWith(HRFFileId_Cals)))
            {
            if (HRFSLOStripAdapter::NeedSLOAdapterFor(rasterFile))
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
        return NULL;
        }
    catch (exception &e)
        {
        //C++ exception
        ostringstream errorStr;

        errorStr << "Caught " << e.what() << endl;
        errorStr << "Type " << typeid(e).name() << endl;

        return NULL;
        }
    catch (...)
        {
        return NULL;
        }

    return rasterFile;
    }

#if 0 //&&JFC TODO
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* RasterProperties::GetFootprint()
    {
    cout << "RasterProperties - Footprint" << endl;

    return ExtractFootprint();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* RasterProperties::ExtractFootprint()
    {
    // Get the rasterFile 
    HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(mFilename);

    if (rasterFile == 0 || rasterFile->CountPages() <= 0)
        return 0;

    try
        {
        HGFHMRStdWorldCluster worldCluster;

        HFCPtr<HRFPageDescriptor> pPageDescriptor = rasterFile->GetPageDescriptor(0);
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
                    worldCluster.GetCoordSysReference(rasterFile->GetWorldIdentificator()));
                }
            else
                {
                pPhysicalCoordSys = new HGF2DCoordSys(HGF2DIdentity(),
                    worldCluster.GetCoordSysReference(rasterFile->GetWorldIdentificator()));
                }

            // Create the reprojection transfo model (non adapted)
            HFCPtr<HGF2DTransfoModel> pDstToSrcTransfoModel(new HCPGCoordModel(*pDestCoordSys, *(pSrcFileGeocoding->Clone())));

            // Create the reprojected coordSys
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

            // *2 for storing coordX and coordY independently.
            double* pPts = new double[pointCollection.size() * 2]; //&&JFC cannot returned memory that you have newed. use DRange3d?

            size_t j = -1;
            for (size_t i = 0; i < pointCollection.size(); ++i)
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterProperties::GetThumbnail(HBITMAP *pThumbnailBmp)
    {
    cout << "RasterProperties - Thumbnail" << endl;

    return ExtractThumbnail(pThumbnailBmp, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterProperties::ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height)
    {
    try
        {
        // Get the rasterFile 
        HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(m_Filename);
        if (NULL == rasterFile)
            return ERROR;

        // Generate the thumbnail
        HFCPtr<HRFThumbnail>  pThumbnail = HRFThumbnailMaker(rasterFile, 0, &width, &height, false);
        if (NULL == pThumbnail)
            return ERROR;

        HFCPtr<HRPPixelType> pPixelType = rasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
        RasterFacility::CreateHBitmapFromHRFThumbnail(pThumbnailBmp, pThumbnail, pPixelType);

        return SUCCESS;
        }
    catch (HFCException&)
        {
        return ERROR;
        }
    catch (exception &e)
        {
        //C++ exception
        ostringstream errorStr;

        errorStr << "Caught " << e.what() << endl;
        errorStr << "Type " << typeid(e).name() << endl;

        return ERROR;
        }
    catch (...)
        {
        return ERROR;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudProperties::GetFile(WCharCP inFilename)
    {
    PointCloudVortex::OpenPOD(m_cloudFileHandle, m_cloudHandle, inFilename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudProperties::CloseFile()
    {
    PointCloudVortex::ClosePOD(m_cloudFileHandle);
    }

#if 0 //&&JFC todo maybe we should split pointcloud and raster provider in 2 files.  They will both involve
      // into a more complex class so we might split them right away.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* PointCloudProperties::GetFootprint()
    {
    cout << "PointCloudProperties - Footprint" << endl;

    return ExtractFootprint();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double* PointCloudProperties::ExtractFootprint()
    {
    if (NULL == mCloudFileHandle || NULL == mCloudHandle)
        return 0;

    // Get bounds for first point cloud in the scene
    double   lower[3], upper[3];
    PointCloudVortex::GetPointCloudBounds(mCloudHandle, lower, upper);

    PtHandle metadataHandle = PointCloudVortex::GetMetaDataHandle(mCloudHandle);
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
    DPoint2d rectPts[FOOTPRINT_PTS_NBR];
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

    double* pPts = new double[FOOTPRINT_SIZE];

    size_t j = -1;
    for (size_t i = 0; i < FOOTPRINT_PTS_NBR; ++i)
        {
        pPts[++j] = rectPts[i].x;
        pPts[++j] = rectPts[i].y;
        }

    return pPts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudProperties::GetThumbnail(HBITMAP *pThumbnailBmp)
    {
    cout << "PointCloudProperties - Thumbnail" << endl; //&&JFC we should not use cout. look for more...

    return ExtractThumbnail(pThumbnailBmp, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudProperties::ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height)
    {
    if (NULL == mCloudFileHandle || NULL == mCloudHandle)
        return ERROR;

    // Set density (number of points retrieved) according to number of pixels in bitmap. Using the total number of pixels in the bitmap * 4
    // seems to produce generally good results.
    float densityValue = (float)(width * height * 4);

    // Get transfo matrix to fit the point cloud in the thumbnail
    Transform transform;
    PointCloudVortex::GetTransformForThumbnail(transform, mCloudHandle, width, height, mView);

    bool needsWhiteBackground = PointCloudVortex::PointCloudNeedsWhiteBackground(mCloudHandle);

    HRESULT hr = PointCloudVortex::ExtractPointCloud(pThumbnailBmp, width, height, mCloudHandle, densityValue, transform, needsWhiteBackground);
    if (*pThumbnailBmp == NULL)
        return ERROR;

    if (hr != NOERROR)
        return ERROR;

    return SUCCESS;
    }

#endif
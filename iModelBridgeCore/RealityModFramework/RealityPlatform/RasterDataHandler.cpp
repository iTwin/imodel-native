/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RasterDataHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "RealityPlatformUtil.h"
#include <RealityPlatform/RealityDataHandler.h>

#define THUMBNAIL_WIDTH     256
#define THUMBNAIL_HEIGHT    256

USING_NAMESPACE_IMAGEPP
USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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
RasterDataHandler::RasterDataHandler(WCharCP inFilename)
    : m_filename(inFilename)
    {
    //&&JFC TODO: Need to do this only once per session and out of the properties object.
    Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RasterDataHandler::~RasterDataHandler()
    {
    Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDataHandler::Initialize()
    {
    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    if (!SessionManager::InitBaseGCS())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterDataHandler::Terminate()
    {
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<RealityDataHandler> RasterDataHandler::Create(WCharCP inFilename)
    {
    return new RasterDataHandler(inFilename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterDataHandler::_GetFootprint(DRange2dP pFootprint) const
    {
    return ExtractFootprint(pFootprint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterDataHandler::ExtractFootprint(DRange2dP pFootprint) const
    {
    // Get the rasterFile 
    HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(m_filename);

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

                double ImageWidth = (double) pRes->GetWidth();
            double ImageHeight = (double) pRes->GetHeight();

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

            HGF2DExtent extent = pPixelShape->GetExtent();
            pFootprint->InitFrom(extent.GetXMin(), extent.GetYMin(), extent.GetXMax(), extent.GetYMax());

            return SUCCESS;
            }
        return 0;
        }
    catch (...)
        {
        return 0;
        }
    }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterDataHandler::_GetThumbnail(HBITMAP *pThumbnailBmp) const
    {
    return ExtractThumbnail(pThumbnailBmp, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chantal.Poulin                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterDataHandler::ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height) const
    {
    try
        {
        // Get the rasterFile 
        HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(m_filename);
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
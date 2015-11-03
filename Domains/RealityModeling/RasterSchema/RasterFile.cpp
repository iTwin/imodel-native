/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>

USING_NAMESPACE_BENTLEY_RASTERSCHEMA

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFile::RasterFile(Utf8StringCR resolvedName)
    {
    m_rasterPtr = nullptr;
    m_pageFilePtr = nullptr;

    m_HRFRasterFilePtr = OpenRasterFile(resolvedName);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFilePtr RasterFile::Create(Utf8StringCR resolvedName)
    {
    RasterFilePtr rasterFilePtr = new RasterFile(resolvedName);
    if (rasterFilePtr->GetHRFRasterFileP() == nullptr)
        {
        // Could not open raster file
        return nullptr;
        }

    return rasterFilePtr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HRFRasterFile*   RasterFile::GetHRFRasterFileP() const
    {
    return m_HRFRasterFilePtr.GetPtr();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HPMPool*  GetMemoryPool()
    {
#if defined(_WIN32) || defined(WIN32)
    static HPMPool  s_pool(256*1024); // 256 Mb
#else
    #error On other systems (Android,...), we need to define available RAM to define the pool size
    static HPMPool  s_pool(32*1024); // 32 Mb
#endif
    return &s_pool;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HRARaster* RasterFile::GetRasterP()
    {
    // Load on first request.
    if(NULL != m_rasterPtr)
        return m_rasterPtr.GetPtr();

    HFCPtr<HGF2DCoordSys> pLogical = GetWorldCluster().GetCoordSysReference(m_HRFRasterFilePtr->GetWorldIdentificator());
    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore (GetMemoryPool(), m_HRFRasterFilePtr, 0/*page*/, pLogical);

    // Specify we do not want to use the file's clip shapes if any. Maybe we'll need to support the native clips some day...
    pStore->SetUseClipShape(false);

    HFCPtr<HRAStoredRaster> pStoredRaster = pStore->LoadRaster();
    m_pPhysicalCoordSys = pStoredRaster->GetPhysicalCoordSys();

    m_rasterPtr = DecorateRaster(pStoredRaster);

    return m_rasterPtr.GetPtr();
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
HFCPtr<HRARaster> RasterFile::DecorateRaster(HFCPtr<HRAStoredRaster>& pStoredRaster)
    {
    double minValue = 0, maxValue = 0;
    // Apply contrast stretch on single channel only 
    if(pStoredRaster->GetPixelType()->GetChannelOrg().CountChannels() == 1 && 
       pStoredRaster->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetSize() >= 16 &&
       SUCCESS == GetSampleStatistics(minValue, maxValue))
        {
        HRPDEMFilter::HillShadingSettings hillShading;
        hillShading.SetHillShadingState(false);
        HFCPtr<HRPDEMFilter> pDEMFilter = new HRPDEMFilter(hillShading, HRPDEMFilter::Style_Elevation);
        pDEMFilter->SetClipToEndValues(true);

        // Generate a greyscale range values.
        double step = (maxValue-minValue) / 255;    // First step is minimum.
        HRPDEMFilter::UpperRangeValues upperRangeValues;
        for(uint32_t i=0; i < 256; ++i)
            upperRangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(minValue+(i*step), HRPDEMFilter::RangeInfo((Byte)i,(Byte)i,(Byte)i,true)));
        pDEMFilter->SetUpperRangeValues(upperRangeValues);

        return new HRADEMRaster(pStoredRaster, 1.0, 1.0, new HGF2DIdentity(), pDEMFilter);
        }

    return pStoredRaster.GetPtr();    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  RasterFile::GetSampleStatistics(double& minValue, double& maxValue)
    {
    HRFAttributeMinSampleValue const* pMinValueTag = GetPageDescriptor()->FindTagCP<HRFAttributeMinSampleValue>();
    HRFAttributeMaxSampleValue const* pMaxValueTag = GetPageDescriptor()->FindTagCP<HRFAttributeMaxSampleValue>();
    if (pMinValueTag == NULL || pMaxValueTag == NULL)
        return ERROR;

    minValue = pMinValueTag->GetData()[0/*First channel*/];
    maxValue = pMaxValueTag->GetData()[0/*First channel*/];

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFile::GetSize(Point2d* sizeP) const
    {
    sizeP->x = GetWidth();
    sizeP->y = GetHeight();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
uint32_t    RasterFile::GetWidth() const
    {
    return (uint32_t)GetPageDescriptor()->GetResolutionDescriptor(0)->GetWidth();
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
uint32_t    RasterFile::GetHeight() const
    {
    return (uint32_t)GetPageDescriptor()->GetResolutionDescriptor(0)->GetHeight();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
GeoCoordinates::BaseGCSPtr RasterFile::GetBaseGcs() const
    {
    GeoCoordinates::BaseGCSCP baseGcsP = GetPageDescriptor()->GetGeocodingCP();
    if(baseGcsP != nullptr && baseGcsP->IsValid())
        return GeoCoordinates::BaseGCS::CreateGCS(*baseGcsP);

    return nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2015
//----------------------------------------------------------------------------------------
DMatrix4d RasterFile::GetPhysicalToLowerLeft() const
    {
    DMatrix4d physicalToLowerLeft;
    physicalToLowerLeft.InitIdentity();

    uint32_t width = (uint32_t)GetPageDescriptor()->GetResolutionDescriptor(0)->GetWidth();
    uint32_t height = (uint32_t)GetPageDescriptor()->GetResolutionDescriptor(0)->GetHeight();

    if (GetPageDescriptor()->GetResolutionDescriptor(0)->GetScanlineOrientation().IsScanlineVertical())
         std::swap(width, height);
   
    switch(GetPageDescriptor()->GetResolutionDescriptor(0)->GetScanlineOrientation().m_ScanlineOrientation)
        {
        // SLO 0
        case HRFScanlineOrientation::UPPER_LEFT_VERTICAL:
            {
            physicalToLowerLeft.InitFromRowValues(0.0, 1.0, 0.0, 0.0,
                                                  -1.0, 0.0, 0.0, height,
                                                  0.0, 0.0, 1.0, 0.0,
                                                  0.0, 0.0, 0.0, 1.0);
            }
            break;

        // SLO 1
        case HRFScanlineOrientation::UPPER_RIGHT_VERTICAL:
            {
            physicalToLowerLeft.InitFromRowValues(0.0, -1.0, 0.0, width,
                                                  -1.0, 0.0, 0.0, height,
                                                  0.0, 0.0, 1.0, 0.0,
                                                  0.0, 0.0, 0.0, 1.0);
            }
        break;

        // SLO 2
        case HRFScanlineOrientation::LOWER_LEFT_VERTICAL:
            {
            physicalToLowerLeft.InitFromRowValues(0.0, 1.0, 0.0, 0.0,
                                                  1.0, 0.0, 0.0, 0.0,
                                                  0.0, 0.0, 1.0, 0.0,
                                                  0.0, 0.0, 0.0, 1.0);
            }
        break;

        //SLO 3
        case HRFScanlineOrientation::LOWER_RIGHT_VERTICAL:
            {
            physicalToLowerLeft.InitFromRowValues(0.0, -1.0, 0.0, width,
                                                  1.0, 0.0, 0.0, 0.0,
                                                  0.0, 0.0, 1.0, 0.0,
                                                  0.0, 0.0, 0.0, 1.0);
            }
        break;
            
        // SLO 4
        case HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL:
            {
            physicalToLowerLeft.InitFromRowValues(1.0, 0.0, 0.0, 0.0,
                                                  0.0, -1.0, 0.0, height,
                                                  0.0, 0.0, 1.0, 0.0,
                                                  0.0, 0.0, 0.0, 1.0);
            }
        break;
            
        // SLO 5
        case HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL:
            {
            physicalToLowerLeft.InitFromRowValues(-1.0, 0.0, 0.0, width,
                                                  0.0, -1.0, 0.0, height,
                                                  0.0, 0.0, 1.0, 0.0,
                                                  0.0, 0.0, 0.0, 1.0);
            }
        break;
            
        // SLO 6
        case HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL:
            physicalToLowerLeft.InitIdentity();
        break;
            
        // SLO 7
        case HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL:
            {
            physicalToLowerLeft.InitFromRowValues(-1.0, 0.0, 0.0, width,
                                                  0.0, 1.0, 0.0, 0.0,
                                                  0.0, 0.0, 1.0, 0.0,
                                                  0.0, 0.0, 0.0, 1.0);
            }
        break;
            
        default:
            BeAssert(!"GetPhysicalToLowerLeft() Unknown SLO");
            physicalToLowerLeft.InitIdentity();
        break;
        }

    return physicalToLowerLeft;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
DMatrix4d RasterFile::GetGeoTransform() const
    {
    // Retrieve the logical CS associated to the world of the raster
    HFCPtr<HGF2DCoordSys> pLogical = GetWorldCluster().GetCoordSysReference(m_HRFRasterFilePtr->GetWorldIdentificator());

    if (GetPageDescriptor()->HasTransfoModel())
        {
        // Create CS from pTransfoModel (transformation from pixels to the world of the raster) to logical
        HFCPtr<HGF2DTransfoModel> pTransfoModel(GetPageDescriptor()->GetTransfoModel());
        HFCPtr<HGF2DCoordSys> pLogicalToPhysRasterWorld(new HGF2DCoordSys(*pTransfoModel, pLogical));

        // Normalize to HGF2DWorld_HMRWORLD(lower left origin), which is the expected CS for QuadTree
        HFCPtr<HGF2DCoordSys> pHmrWorld = GetWorldCluster().GetCoordSysReference(HGF2DWorld_HMRWORLD);
        HFCPtr<HGF2DTransfoModel> pLogicalToHmrWorldTransfo(pLogical->GetTransfoModelTo(pHmrWorld));
        HFCPtr<HGF2DCoordSys> pLogicalToHmrWorld(new HGF2DCoordSys(*pLogicalToHmrWorldTransfo, pLogical));
        HFCPtr<HGF2DTransfoModel> pLogicalToCartesian(pLogicalToPhysRasterWorld->GetTransfoModelTo(pLogicalToHmrWorld));

        // Initialize geoTransform with the significant rows/columns of matrix.         
        HFCMatrix<3, 3> matrix = pLogicalToCartesian->GetMatrix();
        DMatrix4d geoTransform;
        geoTransform.InitFromRowValues( matrix[0][0], matrix[0][1], 0.0, matrix[0][2],
                                        matrix[1][0], matrix[1][1], 0.0, matrix[1][2],
                                        0.0,          0.0,          1.0, 0.0,
                                        matrix[2][0], matrix[2][1], 0.0, matrix[2][2]);
        return geoTransform;
        }
    else
        {
        // Raster has no transfo model. Simply use the transformation to lower left corner.
        DMatrix4d physicalToLowerLeft = GetPhysicalToLowerLeft();
        return physicalToLowerLeft;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> RasterFile::GetPageDescriptor() const
    {
    // Always 0 for now
    return m_HRFRasterFilePtr->GetPageDescriptor(0);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HFCPtr<HGF2DCoordSys> RasterFile::GetPhysicalCoordSys() 
    {
    GetRasterP();   // trigger load if not already loaded.

    BeAssert(m_pPhysicalCoordSys != nullptr);

    return m_pPhysicalCoordSys;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HFCPtr<HRFRasterFile> RasterFile::OpenRasterFile(Utf8StringCR resolvedName)
    {
    HFCPtr<HRFRasterFile> rasterFile;

    try
        {
        BeFileName filename(resolvedName);

        if (filename.empty())
            return NULL;

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

        rasterFile = GenericImprove(rasterFile, HRFiTiffCacheFileCreator::GetInstance());
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HGF2DWorldCluster& RasterFile::GetWorldCluster()
    {
    static HFCPtr<HGF2DWorldCluster> worldClusterPtr = nullptr;

    if (worldClusterPtr == nullptr)
        worldClusterPtr = new HGFHMRStdWorldCluster();
        
    return *worldClusterPtr;
    }

//----------------------------------------------------------------------------------------
// This method returns the raster corners "in its own world"; that is, corners are transformed
// into the CS of the raster.
//
// @bsimethod                                                       Eric.Paquet     7/2015
//----------------------------------------------------------------------------------------
void RasterFile::GetCorners (DPoint3dP corners) const
    {
    DPoint3d srcCorners[4];

    uint32_t width = GetWidth();
    uint32_t height = GetHeight();

    // Start with corners of the physical raster
    // lower left
    srcCorners[0].x = 0;
    srcCorners[0].y = 0;
    srcCorners[0].z = 0;
    // upper left
    srcCorners[1].x = 0;
    srcCorners[1].y = height;
    srcCorners[1].z = 0;
    // upper right
    srcCorners[2].x = width;
    srcCorners[2].y = height;
    srcCorners[2].z = 0;
    // lower right
    srcCorners[3].x = width;
    srcCorners[3].y = 0;
    srcCorners[3].z = 0;

    // Transform corners to the raster world
    DMatrix4d geoTransform = GetGeoTransform();
    geoTransform.MultiplyAndRenormalize(corners, srcCorners, 4);
    }

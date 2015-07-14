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
    m_storedRasterPtr = nullptr;
    m_worldClusterPtr = nullptr;
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
void RasterFile::GetBitmap(HFCPtr<HRABitmapBase> pBitmap)
    {
    HRACopyFromOptions opts;
    opts.SetResamplingMode(HGSResampling::AVERAGE);

    // If destination doesn't support alpha we blend so transparent pixels are replaced with the default color. 
    if(pBitmap->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE)
        {
        pBitmap->Clear();
        opts.SetAlphaBlend(true);       
        }
        
    pBitmap->CopyFrom(*GetStoredRasterP(), opts);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HPMPool*  GetMemoryPool()
    {
    static HPMPool  s_pool(0/*illimited*/); // Memory pool shared by all rasters. (in KB)
    return &s_pool;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HRAStoredRaster* RasterFile::GetStoredRasterP()
    {
    // Load on first request.
    if(NULL != m_storedRasterPtr)
        return m_storedRasterPtr.GetPtr();

    // load raster file in memory in a single chunk
    if (!HRFRasterFileBlockAdapter::CanAdapt(m_HRFRasterFilePtr, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT) &&
        GetPageDescriptor()->GetResolutionDescriptor(0)->GetBlockType() != HRFBlockType::IMAGE) //Make sure we don't try to adapt something we don't have to adapt
        return nullptr;

    HFCPtr<HRFRasterFile> pAdaptedRasterFile(new HRFRasterFileBlockAdapter(m_HRFRasterFilePtr, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT));
    HFCPtr<HGF2DCoordSys> pLogical = GetWorldClusterP()->GetCoordSysReference(pAdaptedRasterFile->GetWorldIdentificator());
    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore (GetMemoryPool(), pAdaptedRasterFile, 0/*page*/, pLogical);

    // Specify we do not want to use the file's clip shapes if any. Maybe we'll need to support the native clips some day...
    pStore->SetUseClipShape(false);

    m_storedRasterPtr = pStore->LoadRaster();

    return m_storedRasterPtr.GetPtr();
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
GeoCoordinates::BaseGCSPtr RasterFile::GetBaseGcs() 
    {
    IRasterBaseGcsCP pSrcFileGeocoding = GetPageDescriptor()->GetGeocodingCP();

    if (pSrcFileGeocoding == nullptr)
        return nullptr;

    GeoCoordinates::BaseGCS* baseGcsP = pSrcFileGeocoding->GetBaseGCS();
    if (baseGcsP == nullptr)
        return nullptr;

    GeoCoordinates::BaseGCSPtr pGcs = GeoCoordinates::BaseGCS::CreateGCS(*baseGcsP);
    return pGcs;
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
DMatrix4d RasterFile::GetGeoTransform()
    {
    // Retrieve the logical CS associated to the world of the raster
    HFCPtr<HRFRasterFile> pAdaptedRasterFile(new HRFRasterFileBlockAdapter(m_HRFRasterFilePtr, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT));
    HFCPtr<HGF2DCoordSys> pLogical = GetWorldClusterP()->GetCoordSysReference(pAdaptedRasterFile->GetWorldIdentificator());

    if (GetPageDescriptor()->HasTransfoModel())
        {
        // Create CS from pTransfoModel (transformation from pixels to the world of the raster) to logical
    HFCPtr<HGF2DTransfoModel> pSimplifiedModel (pModel->CreateSimplifiedModel());
    if (pSimplifiedModel)

        // Normalize to HGF2DWorld_HMRWORLD, which is the expected CS for QuadTree
        HFCPtr<HGF2DCoordSys> pHmrWorld = GetWorldClusterP()->GetCoordSysReference(HGF2DWorld_HMRWORLD);
        HFCPtr<HGF2DTransfoModel> pLogicalToHmrWorldTransfo(pLogical->GetTransfoModelTo(pHmrWorld));
        HFCPtr<HGF2DCoordSys> pLogicalToHmrWorld(new HGF2DCoordSys(*pLogicalToHmrWorldTransfo, pLogical));
        HFCPtr<HGF2DTransfoModel> pLogicalToCartesian(pLogicalToPhysRasterWorld->GetTransfoModelTo(pLogicalToHmrWorld));

        // Initialize geoTransform with the significant rows/columns of matrix.         
        HFCMatrix<3, 3> matrix = pLogicalToCartesian->GetMatrix();
        return pSimplifiedModel;
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
    return GetStoredRasterP()->GetPhysicalCoordSys();
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

        // Take care of sister file. Until requirements prove that wrong, always use sister file for georeference.
        bool useSisterFileOfGeoreferencedFile = true;
        const HRFPageFileCreator (* pPageFileCreator)(HRFPageFileFactory::GetInstance()->FindCreatorFor(rasterFile, useSisterFileOfGeoreferencedFile));
        if(pPageFileCreator != NULL)
            {
            m_pageFilePtr = pPageFileCreator->CreateFor(rasterFile);
            m_pageFilePtr->SetDefaultRatioToMeter(1000);
            rasterFile = new HRFRasterFilePageDecorator(rasterFile, m_pageFilePtr);
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HGF2DWorldCluster*   RasterFile::GetWorldClusterP()
    {
    if (m_worldClusterPtr == nullptr)
        {
        m_worldClusterPtr = new HGFHMRStdWorldCluster();
        }
    return m_worldClusterPtr.GetPtr();
    }

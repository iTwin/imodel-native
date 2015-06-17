/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>

//&&ep - does this go here ?
HFCPtr<HGF2DWorldCluster> gWorldClusterP(new HGFHMRStdWorldCluster());  //&&ep include it in method get; not here
static HPMPool  s_pool(0/*illimited*/); // Memory pool shared by all rasters. (in KB) //&&ep include it in method get; not here

USING_NAMESPACE_BENTLEY_RASTERSCHEMA

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFile::RasterFile(WCharCP inFilename)
    {
    m_storedRasterPtr = nullptr;
    m_HRFRasterFilePtr = OpenRasterFile(inFilename);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFilePtr RasterFile::Create(WCharCP inFilename)
    {
    RasterFilePtr rasterFilePtr = new RasterFile(inFilename);
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
        
    pBitmap->CopyFrom(*GetStoredRaster(), opts);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HFCPtr<HRAStoredRaster> RasterFile::GetStoredRaster() //&&ep return R ou P
    {

    // Load on first request.
    if(NULL != m_storedRasterPtr)
        return m_storedRasterPtr;

    // load raster file in memory in a single chunk
    if (!HRFRasterFileBlockAdapter::CanAdapt(m_HRFRasterFilePtr, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT) &&
        m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType() != HRFBlockType::IMAGE) //Make sure we don't try to adapt something we don't have to adapt
        //&&ep handle error correctly
        return nullptr;

    HFCPtr<HRFRasterFile> pAdaptedRasterFile(new HRFRasterFileBlockAdapter(m_HRFRasterFilePtr, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT));

    HFCPtr<HGF2DCoordSys> pLogical = gWorldClusterP->GetCoordSysReference(pAdaptedRasterFile->GetWorldIdentificator());
    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore (&s_pool, pAdaptedRasterFile, 0/*page*/, pLogical);

    // Specify we do not want to use the file's clip shapes if any. Maybe we'll need to support the native clips some day...
    pStore->SetUseClipShape(false);

    m_storedRasterPtr = pStore->LoadRaster();

    return m_storedRasterPtr;
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
    if (m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetScanlineOrientation().IsScanlineVertical())
        return (uint32_t)m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight();

    return (uint32_t)m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth();
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
uint32_t    RasterFile::GetHeight() const
    {
    if (m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetScanlineOrientation().IsScanlineVertical())
        return (uint32_t)m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth();

    return (uint32_t)m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> RasterFile::GetSLOTransfoModel() const
    {
    uint32_t width = GetWidth();
    uint32_t height = GetHeight();

    HFCPtr<HGF2DAffine> pModel(new HGF2DAffine());

    switch(m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetScanlineOrientation().m_ScanlineOrientation)
        {
        // SLO 0
        case HRFScanlineOrientation::UPPER_LEFT_VERTICAL:
            {
            pModel->SetByMatrixParameters(0.0, 0.0, 1.0, 0.0, 1.0, 0.0);
            }
        break;

        // SLO 1
        case HRFScanlineOrientation::UPPER_RIGHT_VERTICAL:
            {
            pModel->SetByMatrixParameters(width, 0.0, -1.0, 0.0, 1.0, 0.0);        
            }
        break;

        // SLO 2
        case HRFScanlineOrientation::LOWER_LEFT_VERTICAL:
            {
            pModel->SetByMatrixParameters(0.0, 0.0, 1.0, height, -1.0, 0.0);
            }
        break;

        // SLO 3
        case HRFScanlineOrientation::LOWER_RIGHT_VERTICAL:
            {
            pModel->SetByMatrixParameters(width, 0.0, -1.0, height, -1.0, 0.0);
            }
        break;

        // SLO 4
        case HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL:
            {
            pModel->SetByMatrixParameters(0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
            }
        break;

        // SLO 5
        case HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL:
            {
            pModel->SetByMatrixParameters(width, -1.0, 0.0, 0.0, 0.0, 1.0);
            }
        break;

        // SLO 6
        case HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL:
            {
            pModel->SetByMatrixParameters(0.0, 1.0, 0.0, height, 0.0, -1.0);
            }
        break;

        // SLO 7
        case HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL:
            {
            pModel->SetByMatrixParameters(width, -1.0, 0.0, height, 0.0, -1.0);
            }
        break;

        default:
            BeAssert(!"HIEUSSLO::Get2DTransfoModel(...) Unknown SLO");
        break;
        }

    HFCPtr<HGF2DTransfoModel> pSimplifiedModel (pModel->CreateSimplifiedModel());
    if (pSimplifiedModel)
        return pSimplifiedModel;

    return pModel.GetPtr();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
HFCPtr<HGF2DCoordSys> RasterFile::GetPhysicalCoordSys() 
    {
/* &&ep - needswork (for bmp, monochrome)
    // Create a transformation model from the raster file SLOx to SLO4
    HFCPtr<HGF2DTransfoModel> pSloTransfo = GetSLOTransfoModel();

    // Reverse the model (SLO4->SLOx) 
    pSloTransfo->Reverse();

    return new HGF2DCoordSys(*pSloTransfo, GetStoredRaster()->GetPhysicalCoordSys());
*/
    return GetStoredRaster()->GetPhysicalCoordSys();

    }


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
HFCPtr<HRFRasterFile> RasterFile::OpenRasterFile(WCharCP inFilename)
    {
    HFCPtr<HRFRasterFile> rasterFile;

    try
        {
        WString filename(inFilename);

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

        // Check if we have an internet imaging file
        // DISABLED: We do not support HRFInternetImagingFile
        //         if (RasterFile->IsCompatibleWith(HRFInternetImagingFile::CLASS_ID))
        //             ((HFCPtr<HRFInternetImagingFile>&)RasterFile)->DownloadAttributes();

        // Adapt Scan Line Orientation (1 bit images) //&&ep - investigate what to do with SLO. SLO is for binaries
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


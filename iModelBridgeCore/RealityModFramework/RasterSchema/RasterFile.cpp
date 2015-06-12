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
RasterFile::RasterFile(WCharCP inFilename) //&&ep - remove inFilename param
    {
    m_storedRasterPtr = nullptr;
    m_HRFRasterFilePtr = OpenRasterFile(inFilename);

//&&ep    return exception if m_HRFRasterFilePtr is null ? work in concordance with RasterFileHandler
// A. No. see below (OpenRasterFile)
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFilePtr RasterFile::Create(WCharCP inFilename) //&&ep - should be url ?
    {
//&&ep o    return new RasterFile(inFilename);
    RasterFilePtr rasterFilePtr = new RasterFile(inFilename);
/* &&ep d
    if (rasterFilePtr->OpenRasterFile(inFilename) != SUCCESS)
        {
        return nullptr;
        }
*/
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
/*&&ep d
HFCPtr<HRFRasterFile>   RasterFile::GetHRFRasterFilePtr() const
    {
    return m_HRFRasterFilePtr;  //&&ep return a P or R
    }
*/

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
BentleyStatus RasterFile::ReadFileToBitmap(Byte** imageBufferPP, Point2d *imageSizeP)
    {
    BentleyStatus status = SUCCESS;


//&&ep - not now
#if defined (notNow)

    Point2d dummy_imageSize;
    if(NULL == imageSizeP)
        imageSizeP = &dummy_imageSize;

    *imageBufferPP = NULL;
            
/* &&ep o

    if (requestedSizeP)
        *imageSizeP = *requestedSizeP;
    else
        pRaster->GetSize(imageSizeP);
*/

    GetSize(imageSizeP);


/* &&ep o (uses HGFDistance); and Stretch
    Point2d bitmapSize = *imageSizeP;  
    HGF2DStretch Stretch(HGF2DDisplacement(HGFDistance(0, HGFDistanceUnit(1)), HGFDistance(0, HGFDistanceUnit(1))), 
                         GetWidth() / (double)bitmapSize.x, 
                         GetHeight() / (double)bitmapSize.y);

    HFCPtr<HRABitmap> pBitmap = new HRABitmap(bitmapSize.x, bitmapSize.y, &Stretch, pRaster->GetPhysicalCoordSys(), new HRPPixelTypeV24R8G8B8());
*/
    Point2d bitmapSize = *imageSizeP;  
    HFCPtr<HRABitmap> pBitmap = HRABitmap::Create (bitmapSize.x, bitmapSize.y, GetStoredRaster()->GetTransfoModel(), GetPhysicalCoordSys(), new HRPPixelTypeV24R8G8B8());


    // Need intermediate buffer (non separated)

//&&ep - devrait pas avoir besoin; mon buffer size devrait etre 256*256
    size_t bufferSize;
    if (SUCCESS != ComputeBufferSize(bufferSize, bitmapSize, IMAGEFORMAT_RGB))
//&&ep o        return IMGALIB_STATUS_InsufficientMemory;
        return ERROR;

//&&ep creer le buffer 1 seule fois; chaque source a son buffer; tile size: 256
    HFCPtr<HCDPacket> pPacket = new HCDPacket(new HCDCodecIdentity(), new Byte[bufferSize], bufferSize, bufferSize);
    pPacket->SetBufferOwnership(true);

    if(pPacket->GetBufferAddress() == NULL)
//&&ep o        return IMGALIB_STATUS_InsufficientMemory;
        return ERROR;

    pBitmap->SetPacket(pPacket);   

    // Alloc buffer to receive the data
//&&ep o    if (NULL == (*imageBufferPP = (Byte*)IMAGELIB_MALLOC (bufferSize)))   
    if (NULL == (*imageBufferPP = (Byte*)ImagelibMemoryMngr::Malloc(bufferSize)))   

//&&ep o        return IMGALIB_STATUS_InsufficientMemory;
        return ERROR;

    // Query data.
//&&ep o    pRaster->GetBitmap(pBitmap.GetPtr());
    GetBitmap(pBitmap.GetPtr());


#endif



    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
/* &&ep - maybe not needed

int RasterFile::ComputeBufferSize 
(
    size_t&         bufferSize,     // <= size of the buffer that can hold an image of the specified size and format
    const Point2d&  imageSize,      // => size of the image
    int             imageFormat     // => image format
) const
    {
    uint64_t& bufferSizeRef = bufferSize;

    switch (imageFormat)
        {
        case IMAGEFORMAT_RGB:
        case IMAGEFORMAT_BGRSeparate:
            bufferSizeRef = (uint64_t)imageSize.x*(uint64_t)imageSize.y*3; 
            break;
        case IMAGEFORMAT_RGBA:
        case IMAGEFORMAT_RGBASeparate:
        case IMAGEFORMAT_BGRA:
            bufferSizeRef = (uint64_t)imageSize.x*(uint64_t)imageSize.y*4; 
            break;
        case IMAGEFORMAT_BitMap:
            bufferSizeRef = BITMAP_ROWBYTES((uint64_t)imageSize.x) * (uint64_t)imageSize.y;
            break;
        case IMAGEFORMAT_ByteMap:
        case IMAGEFORMAT_GreyScale:
            bufferSizeRef = (uint64_t)imageSize.x*(uint64_t)imageSize.y;
            break;
        default:
            BeAssert(!"Not implemented yet");
            return IMGALIB_STATUS_BadArg;
        }

    // NOTE: The system could not allocate buffer of greater size
    // then INT_MAX because of the signature of the allocation
    // functions. e.g.: malloc.
    // TDORAY: Remove if malloc and other are adapted for size_t in Beijing.
    if (bufferSize > INT_MAX) 
        return IMGALIB_STATUS_InsufficientMemory;

    return SUCCESS;
    }
*/

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
        
//&&ep o    pBitmap->CopyFrom(GetRaster().GetPtr(), opts);
//    HFCPtr<HRAStoredRaster> srPtr;
//    GetStoredRaster();
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
/* &&ep o
    if (!HRFRasterFileBlockAdapter::CanAdapt(m_HRFRasterFilePtr, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT) &&
        m_HRFRasterFilePtr->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType() != HRFBlockType::IMAGE) //Make sure we don't try to adapt something we don't have to adapt
        throw HRFException(HFC_CANNOT_OPEN_FILE_EXCEPTION, m_HRFRasterFilePtr->GetURL()->GetURL());
*/                
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
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------

/* &&ep - later
HFCPtr<HGF2DCoordSys> RasterFile::GetPhysicalCoordSys() 
    {
    // Create a transformation model from the raster file SLOx to SLO4
    HFCPtr<HGF2DTransfoModel> pSloTransfo = GetSLOTransfoModel();

    // Reverse the model (SLO4->SLOx) 
    pSloTransfo->Reverse();

    return new HGF2DCoordSys(*pSloTransfo, GetRaster()->GetPhysicalCoordSys());
    }
*/

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
//&&ep o static HFCPtr<HRFRasterFile> GetRasterFile(WCharCP inFilename)
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


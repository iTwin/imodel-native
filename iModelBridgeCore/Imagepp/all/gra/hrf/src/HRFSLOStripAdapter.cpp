//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSLOStripAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSLOStripAdapter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRFSLOStripEditor.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HCDCodecFactory.h>

#define HRF_SLO_ADAPTER_STRIP_HEIGHT 512

//-----------------------------------------------------------------------------
// HRFSLOStripAdapterBlockCapabilities
//-----------------------------------------------------------------------------
class HRFSLOStripAdapterBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSLOStripAdapterBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE_CREATE, // AccessMode
                                   LONG_MAX,              // MaxSizeInBytes
                                   32,                    // MinHeight
                                   LONG_MAX,              // MaxHeight
                                   1));                   // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFRasterFileBlockAdapterCapabilities
//-----------------------------------------------------------------------------
HRFRasterFileSLOAdapterCapabilities::HRFRasterFileSLOAdapterCapabilities(const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilities)
    : HRFRasterFileCapabilities()
    {
    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL));

    // Obtain the list of original pixel capability
    HFCPtr<HRFRasterFileCapabilities> ListOfPixelType = pi_rpCapabilities->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID,
                                                        HFC_READ_WRITE_CREATE);
    HASSERT(ListOfPixelType != 0);

    // Parse the list of pixel capability
    for (uint32_t Index=0; Index < ListOfPixelType->CountCapabilities(); Index++)
        {
        // Get Original Pixel Capability
        HFCPtr<HRFPixelTypeCapability> pPixelTypeCap;
        pPixelTypeCap = ((HFCPtr<HRFPixelTypeCapability>&)ListOfPixelType->GetCapability(Index));

        // Create the new pixel type to obtain the number of bits
        HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeCap->GetPixelTypeClassID());

        // Create the codec list to be attach to the PixelType Capability.
        HFCPtr<HRFRasterFileCapabilities> pCodecCapabilities = new HRFRasterFileCapabilities();

        // Add the default codec used by the resolution editor
        // Add the codec RLE1 when 1 bit otherwise add the zlib codec
        if (pPixelType->CountPixelRawDataBits() == 1)
            pCodecCapabilities->Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                                           HCDCodecHMRRLE1::CLASS_ID,
                                                           new HRFSLOStripAdapterBlockCapabilities()));

        // Add original codecs to the new pixel capabilities
        for (uint32_t CodecIndex=0; CodecIndex < pPixelTypeCap->CountCodecs(); CodecIndex++)
            {
            if(pPixelTypeCap->GetCodecCapabilityByIndex(CodecIndex)->GetCodecClassID() != HCDCodecHMRRLE1::CLASS_ID)
                pCodecCapabilities->Add(new HRFCodecCapability(pPixelTypeCap->GetCodecCapabilityByIndex(CodecIndex)->GetAccessMode(),
                                                               pPixelTypeCap->GetCodecCapabilityByIndex(CodecIndex)->GetCodecClassID(),
                                                               new HRFSLOStripAdapterBlockCapabilities()));
            }
        HASSERT(pPixelTypeCap->CountCodecs() > 0);

        // Create the pixel capability
        HFCPtr<HRFPixelTypeCapability> pPixelTypeNewCap;
        pPixelTypeNewCap = new HRFPixelTypeCapability(pPixelTypeCap->GetAccessMode(),
                                                      pPixelTypeCap->GetPixelTypeClassID(),
                                                      pCodecCapabilities);

        Add((HFCPtr<HRFCapability>)pPixelTypeNewCap);
        }
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFSLOStripAdapter::HRFSLOStripAdapter(HFCPtr<HRFRasterFile>&  pi_rpAdaptedFile)
    : HRFRasterFileExtender(pi_rpAdaptedFile)
    {
    HPRECONDITION(pi_rpAdaptedFile->CountPages() > 0);
    HPRECONDITION(pi_rpAdaptedFile->GetPageDescriptor(0)->CountResolutions() > 0);

    // The ancestor store the access mode
    m_IsOpen          = false;

    // If we dont already have a Line acess type, create an adaptor to simulate it.
    bool NeedLineAdapter = false;
    HRFRasterFileBlockAdapter::BlockDescriptorMap BlockDescMap;
    HRFRasterFileBlockAdapter::BlockDescriptor    BlockDesc;

    for (uint32_t Page = 0; (Page < m_pOriginalFile->CountPages()) && (NeedLineAdapter == false); Page++)
        {
        NeedLineAdapter = false;
        HFCPtr<HRFPageDescriptor> pPageDescriptor = m_pOriginalFile->GetPageDescriptor(Page);

        for (unsigned short Resolution = 0; (Resolution < pPageDescriptor->CountResolutions()) && (NeedLineAdapter == false); Resolution++)
            {
            if (pPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockType() != HRFBlockType::LINE)
                NeedLineAdapter = true;
            }

        if (NeedLineAdapter)
            {
            BlockDesc.m_BlockType   = HRFBlockType::LINE;

            HASSERT(m_pOriginalFile->GetPageDescriptor(Page)->GetResolutionDescriptor(0)->GetWidth() <= ULONG_MAX);

            BlockDesc.m_BlockWidth  = (uint32_t)m_pOriginalFile->GetPageDescriptor(Page)->GetResolutionDescriptor(0)->GetWidth();
            BlockDesc.m_BlockHeight = 1;
            BlockDescMap.insert(HRFRasterFileBlockAdapter::BlockDescriptorMap::value_type(Page, BlockDesc));
            }
        }

    if (BlockDescMap.size() > 0)
        {
        // Change block acess mode
        HPRECONDITION(HRFRasterFileBlockAdapter::CanAdapt(m_pOriginalFile, BlockDescMap));
        m_pOriginalFile = new HRFRasterFileBlockAdapter(m_pOriginalFile, BlockDescMap);
        }

    // Create the capabilities
    m_pRasterFileCapabilities = new HRFCombinedRasterFileCapabilities(new HRFRasterFileSLOAdapterCapabilities(m_pOriginalFile->GetCapabilities()),
                                                                      m_pOriginalFile->GetCapabilities());

    if (Open() && !GetAccessMode().m_HasCreateAccess)     // if Open success and it is not a new file
        CreateDescriptors();                              // Create Page and Res Descriptors.
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFSLOStripAdapter::~HRFSLOStripAdapter()
    {
    Close();
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFSLOStripAdapter::CreateResolutionEditor( uint32_t pi_Page,
                                                                 unsigned short pi_Resolution,
                                                                 HFCAccessMode pi_AccessMode)
    {
    HRFResolutionEditor* pAdapterResolutionEditor;
    // We create an editor on the file
    HRFResolutionEditor* pSrcResolutionEditor = m_pOriginalFile->CreateResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);
    pAdapterResolutionEditor = new HRFSLOStripEditor(this,
                                                     pi_Page,
                                                     pi_Resolution,
                                                     pi_AccessMode,
                                                     pSrcResolutionEditor);

    return pAdapterResolutionEditor;
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFSLOStripAdapter::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    // ??????????????????????????????????????? NOT SUPPORTED NOW
    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFSLOStripAdapter::GetCapabilities () const
    {
    return m_pRasterFileCapabilities;
    }

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFSLOStripAdapter::Open()
    {
    if (!m_IsOpen && GetAccessMode().m_HasCreateAccess)
        {
        // Create the new decoration
        m_IsOpen = Create();
        CreateDescriptors();
        }
    else if (!m_IsOpen) // Open the file
        {
        m_IsOpen = true;
        }

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFSLOStripAdapter::CreateDescriptors ()
    {
    HPRECONDITION (m_IsOpen);

    uint32_t RasterWidth;
    uint32_t RasterHeight;

    uint32_t BlockHeight;
    uint32_t BlockWidth;
    HRFBlockType BlockType;

    // adapt all pages and all resolutions to the new storage type
    for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
        {
        HRFPageDescriptor::ListOfResolutionDescriptor ListOfResolutionDescriptor;
        HFCPtr<HRFPageDescriptor> pAdaptedPageDescriptor = m_pOriginalFile->GetPageDescriptor(Page);

        // Adapt each resolutions from adapted file when their access is different of Strip
        // for (UShort Resolution=0; Resolution < pAdaptedPageDescriptor->CountResolutions(); Resolution++)
        for (unsigned short Resolution=0; Resolution < 1; Resolution++)  // HChkSebG Purpose: Disable unsupported sub-res.
            {
            HFCPtr<HRFResolutionDescriptor> pAdaptedResDescriptor = pAdaptedPageDescriptor->GetResolutionDescriptor(Resolution);

            HASSERT(pAdaptedResDescriptor->GetHeight() <= ULONG_MAX);
            HASSERT(pAdaptedResDescriptor->GetWidth() <= ULONG_MAX);

            if (pAdaptedResDescriptor->GetScanlineOrientation().IsScanlineVertical())
                {
                RasterWidth  = (uint32_t)pAdaptedResDescriptor->GetHeight();
                RasterHeight = (uint32_t)pAdaptedResDescriptor->GetWidth();
                BlockWidth   = RasterWidth;
                BlockHeight  = HRF_SLO_ADAPTER_STRIP_HEIGHT;
                BlockType    = HRFBlockType::STRIP;
                }
            else
                {
                RasterWidth  = (uint32_t)pAdaptedResDescriptor->GetWidth();
                RasterHeight = (uint32_t)pAdaptedResDescriptor->GetHeight();
                BlockWidth   = (uint32_t)pAdaptedResDescriptor->GetWidth();
                BlockHeight  = HRF_SLO_ADAPTER_STRIP_HEIGHT;
                BlockType    = HRFBlockType::STRIP;
                }

            // Get the default codec for the PixelType
            HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability =
                new HRFPixelTypeCapability(pAdaptedResDescriptor->GetAccessMode(),
                                           pAdaptedResDescriptor->GetPixelType()->GetClassID(),
                                           new HRFRasterFileCapabilities());

            pPixelTypeCapability = static_cast<HRFPixelTypeCapability*>(GetCapabilities()->GetCapabilityOfType(((HFCPtr<HRFCapability>)pPixelTypeCapability)).GetPtr());

            HASSERT(pPixelTypeCapability != 0);
            HASSERT(pPixelTypeCapability->CountCodecs() > 0);
            
            HFCPtr<HCDCodec> pCodec = HCDCodecFactory::GetInstance().Create(pPixelTypeCapability->GetCodecCapabilityByIndex(0)->GetCodecClassID());

            HFCPtr<HRFResolutionDescriptor> AdapterResolutionDescriptor = new HRFResolutionDescriptor(
                pAdaptedResDescriptor->GetAccessMode(),        // AccessMode
                GetCapabilities(),                             // Capabilities,
                pAdaptedResDescriptor->GetResolutionXRatio(),  // ResolutionRatio,
                pAdaptedResDescriptor->GetResolutionYRatio(),  // ResolutionRatio,
                pAdaptedResDescriptor->GetPixelType(),         // PixelType,
                pCodec,                                        // Codec,
                HRFBlockAccess::SEQUENTIAL,                    // pAdaptedResDescriptor->GetReaderBlockAccess(), // StorageAccess,
                HRFBlockAccess::SEQUENTIAL,                    // pAdaptedResDescriptor->GetWriterBlockAccess(), // StorageAccess,
                HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL, // ScanLineOrientation,
                pAdaptedResDescriptor->GetInterleaveType(),    // InterleaveType
                pAdaptedResDescriptor->IsInterlace(),          // IsInterlace,
                RasterWidth,                                   // Width,
                RasterHeight,                                  // Height,
                BlockWidth,                                    // BlockWidth,
                BlockHeight,                                   // BlockHeight,
                0,                                             // pAdaptedResDescriptor->GetBlocksDataFlag(),    // BlockDataFlag
                BlockType,                                     // BlocksDataFlag
                pAdaptedResDescriptor->GetNumberOfPass(),      // NumberOfPass
                pAdaptedResDescriptor->GetPaddingBits(),       // PaddingBits
                pAdaptedResDescriptor->GetDownSamplingMethod()); // DownSamplingMethod

            // We set all flags to LOADED
            if (GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, HFC_WRITE_ONLY) != 0)
                {
                HASSERT(AdapterResolutionDescriptor->CountBlocks() <= SIZE_MAX);
                HASSERT(AdapterResolutionDescriptor->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);

                // UInt64 CountBlock = AdapterResolutionDescriptor->CountBlocks();
                HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
                pBlocksDataFlag = new HRFDataFlag[(size_t)AdapterResolutionDescriptor->CountBlocks()];
                memset(pBlocksDataFlag, HRFDATAFLAG_LOADED, (size_t)(AdapterResolutionDescriptor->CountBlocks() * sizeof(HRFDataFlag)));
                AdapterResolutionDescriptor->SetBlocksDataFlag(pBlocksDataFlag);
                }
            ListOfResolutionDescriptor.push_back(AdapterResolutionDescriptor);
            }

        // Best match pages and resolutions
        HRPPixelPalette*   pRepresentativePalette = 0;
        HRPHistogram*      pHistogram             = 0;
        HRFThumbnail*      pThumbnail             = 0;
        HRFClipShape*      pClipShape             = 0;
        HFCPtr<HGF2DTransfoModel> pTransfoModel;
        HRPFilter*         pFilters               = 0;
        HFCPtr<HRFCapability> pTransfoCapability;

        // Best match the TransfoModel
        if (pAdaptedPageDescriptor->HasTransfoModel())
            pTransfoModel = pAdaptedPageDescriptor->GetTransfoModel();

        // Change the raster file origin according it's slo.
        HFCPtr<HRFResolutionDescriptor> pAdaptedResDescriptor = pAdaptedPageDescriptor->GetResolutionDescriptor(0);

        HASSERT(ListOfResolutionDescriptor[0]->GetWidth() <= ULONG_MAX);
        HASSERT(ListOfResolutionDescriptor[0]->GetHeight() <= ULONG_MAX);

        RasterWidth  = (uint32_t)ListOfResolutionDescriptor[0]->GetWidth();
        RasterHeight = (uint32_t)ListOfResolutionDescriptor[0]->GetHeight();

        // Validation with the capabilities if it's possible to Set a Transfo Model
        if (m_pOriginalFile->GetCapabilities()->GetCapabilityOfType(HRFTransfoModelCapability::CLASS_ID) != 0)
            {
            if (pTransfoModel)
                {
                HFCPtr<HGF2DTransfoModel> pSLOTransfoModel = CreateSLOTransfoModel(pAdaptedResDescriptor->GetScanlineOrientation(), RasterWidth, RasterHeight);
                //pTranslationModel->Reverse();
                pTransfoModel = pSLOTransfoModel->ComposeInverseWithDirectOf(*pTransfoModel);
//                pTransfoModel = pTransfoModel->ComposeInverseWithDirectOf(*pSLOTransfoModel);
                }
            else
                pTransfoModel = CreateSLOTransfoModel(pAdaptedResDescriptor->GetScanlineOrientation(), RasterWidth,RasterHeight);
            }

        // Best match  the Filter
        if (pAdaptedPageDescriptor->HasFilter())
            pFilters = (HRPFilter*)&pAdaptedPageDescriptor->GetFilter();

        // Best match  the ClipShape
        if (pAdaptedPageDescriptor->HasClipShape())
            pClipShape = pAdaptedPageDescriptor->GetClipShape();

        // Best match  the Histogram
        if (pAdaptedPageDescriptor->HasHistogram())
            pHistogram = pAdaptedPageDescriptor->GetHistogram();

        // Best match  the Thumbnail
        if (pAdaptedPageDescriptor->HasThumbnail())
            pThumbnail = pAdaptedPageDescriptor->GetThumbnail();

        // Best match  the RepresentativePalette
        if (pAdaptedPageDescriptor->HasRepresentativePalette())
            pRepresentativePalette = (HRPPixelPalette*)&pAdaptedPageDescriptor->GetRepresentativePalette();

        HFCPtr<HRFPageDescriptor> pAdapterPageDescriptor;
        pAdapterPageDescriptor = new HRFPageDescriptor(pAdaptedResDescriptor->GetAccessMode(),
                                                       GetCapabilities(),
                                                       ListOfResolutionDescriptor,
                                                       pRepresentativePalette,    // RepresentativePalette
                                                       pHistogram,                // Histogram
                                                       pThumbnail,                // Thumbnail
                                                       pClipShape,                // ClipShape
                                                       pTransfoModel,             // TransfoModel
                                                       pFilters,                  // Filters
                                                       (HPMAttributeSet*)&pAdaptedPageDescriptor->GetTags());

        // MetaData                                                            
        HFCPtr<HMDMetaDataContainerList> pMDContainers(pAdaptedPageDescriptor->GetListOfMetaDataContainer());
        if (pMDContainers != 0)
            {
            pMDContainers = new HMDMetaDataContainerList(*pMDContainers);
            pAdapterPageDescriptor->SetListOfMetaDataContainer(pMDContainers);
            }

        // Geocoding
        IRasterBaseGcsCP baseGCS = pAdaptedPageDescriptor->GetGeocodingCP();
        pAdapterPageDescriptor->SetGeocoding(const_cast<IRasterBaseGcsP>(baseGCS));

        // Add the page descriptor to the list
        m_ListOfPageDescriptor.push_back(pAdapterPageDescriptor);
        }
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Save the Adaptor and the adaptee.
//-----------------------------------------------------------------------------
void HRFSLOStripAdapter::Save()
    {
    SynchronizeFiles();
    HRFRasterFileExtender::Save();
    }

//-----------------------------------------------------------------------------
// Private
// This method close the file.
//-----------------------------------------------------------------------------
void HRFSLOStripAdapter::Close()
    {
    SynchronizeFiles();

    // close the decorator file
    if (m_IsOpen)
        m_IsOpen = false;

    }

//-----------------------------------------------------------------------------
// Private
// SynchronizeFiles
// Synchronize the cached file and the cache file.
//-----------------------------------------------------------------------------
void HRFSLOStripAdapter::SynchronizeFiles()
    {
    // Get the original file page descriptor.
    HFCPtr<HRFPageDescriptor> pOriginalPageDescriptor = m_pOriginalFile->GetPageDescriptor(0);

    // If original file has transfo model and the adapter model has changed.
    if (pOriginalPageDescriptor->HasTransfoModel() && (GetPageDescriptor(0)->TransfoModelHasChanged()))
        {
        HASSERT(GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth() <= ULONG_MAX);
        HASSERT(GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight() <= ULONG_MAX);

        // Remove adaptation from model.
        HFCPtr<HGF2DTransfoModel> pSLOTransfoModel = CreateSLOTransfoModel(pOriginalPageDescriptor->GetResolutionDescriptor(0)->GetScanlineOrientation(),
                                                                           (uint32_t)GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth(),
                                                                           (uint32_t)GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight());
        pSLOTransfoModel->Reverse();

        HFCPtr<HGF2DTransfoModel> pNewTransfoModel;
        pNewTransfoModel = pSLOTransfoModel->ComposeInverseWithDirectOf(*GetPageDescriptor(0)->GetTransfoModel());

        // Simplifie the model.
        HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pNewTransfoModel->CreateSimplifiedModel();

        if (pTempTransfoModel != 0)
            pNewTransfoModel = pTempTransfoModel;

        // Validation with the capabilities if it's possible to set a light Transfo Model
        HFCPtr< HRFCapability> pTransfoCapability = new HRFTransfoModelCapability(HFC_WRITE_ONLY, pNewTransfoModel->GetClassID());
        if (m_pOriginalFile->GetCapabilities()->Supports(pTransfoCapability))
            m_pOriginalFile->GetPageDescriptor(0)->SetTransfoModel(*pNewTransfoModel);
        }
    }

//-----------------------------------------------------------------------------
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFSLOStripAdapter::Create()
    {
    return true;
    }

//-----------------------------------------------------------------------------
// This methods allow to know if the specified raster file need a cache.
//-----------------------------------------------------------------------------
bool HRFSLOStripAdapter::NeedSLOAdapterFor(HFCPtr<HRFRasterFile> const& pi_rpForRasterFile)
    {
    bool NeedAdapter= false;

    for (uint32_t Page = 0; (Page < pi_rpForRasterFile->CountPages()) && (NeedAdapter == false); Page++)
        {
        HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpForRasterFile->GetPageDescriptor(Page);

        for (unsigned short Resolution = 0; (Resolution < pPageDescriptor->CountResolutions()) && (NeedAdapter == false); Resolution++)
            {
            // Having SLO different than UPPER_LEFT_HORIZONTAL and 1 bit only
            if (pPageDescriptor->GetResolutionDescriptor(Resolution)->GetScanlineOrientation() != HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL &&
                pPageDescriptor->GetResolutionDescriptor(Resolution)->GetPixelType()->CountPixelRawDataBits() == 1)
                NeedAdapter = true;
            }
        }
    return NeedAdapter;
    }

//-----------------------------------------------------------------------------
// Public
// This Static method create the file.
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFSLOStripAdapter::CreateBestAdapterFor(HFCPtr<HRFRasterFile> pi_rpForRasterFile)
    {
    HFCPtr<HRFRasterFile> pRasterFile = pi_rpForRasterFile;

    if (CanAdapt(pRasterFile))
        pRasterFile = new HRFSLOStripAdapter(pRasterFile);

    return pRasterFile;
    }

//-----------------------------------------------------------------------------
// Public
// This Static methods allow to if it is possible to adapt this raster file.
//-----------------------------------------------------------------------------

bool HRFSLOStripAdapter::CanAdapt(HFCPtr<HRFRasterFile> pi_rpFromRasterFile)
    {
    bool retour = false;

    HFCPtr<HRFPageDescriptor>       pPageDescriptor = pi_rpFromRasterFile->GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResDescriptor  = pPageDescriptor->GetResolutionDescriptor(0);

    // Can adapt file having SLO different from UPPER_LEFT_HORIZONTAL
    if (pResDescriptor->GetScanlineOrientation() != HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL )
        {
        // Can adapt 1 bit files only
        if (pResDescriptor->GetPixelType()->CountPixelRawDataBits() == 1)
            {
            retour = true;
            }
        }

    return retour;
    }

//-----------------------------------------------------------------------------
// Private
// CreateSLOTransfoModel
// This Static methods creates a model from any SLO to SLO 4
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFSLOStripAdapter::CreateSLOTransfoModel(HRFScanlineOrientation  pi_ScanlineOrientation,
                                                                    uint32_t pi_RasterOrientedPhysicalWidth,
                                                                    uint32_t pi_RasterOrientedPhysicalHeight)
    {
#if (0)
    int32_t XTranslation = 0;
    int32_t YTranslation = 0;
#endif

    HFCPtr<HGF2DAffine> pSLOAffine = new HGF2DAffine();

    // SLO 0
    if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 0.0, 1.0, 0.0, 1.0, 0.0);
        }
    // SLO 1
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(pi_RasterOrientedPhysicalWidth, 0.0, -1.0, 0.0, 1.0, 0.0);
//        YTranslation += pi_RasterPhysicalHeight;
        }
    // SLO 2
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 0.0, 1.0, pi_RasterOrientedPhysicalHeight, -1.0, 0.0);
//        XTranslation -= pi_RasterPhysicalWidth;
        }
    // SLO 3
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(pi_RasterOrientedPhysicalWidth, 0.0, -1.0, pi_RasterOrientedPhysicalHeight, -1.0, 0.0);
//        YTranslation += pi_RasterPhysicalHeight;
//        XTranslation -= pi_RasterPhysicalWidth;
        }
    // SLO 4
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
//        YTranslation += pi_RasterPhysicalHeight;
//        XTranslation -= pi_RasterPhysicalWidth;
        }
    // SLO 5
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(pi_RasterOrientedPhysicalWidth, -1.0, 0.0, 0.0, 0.0, 1.0);
//        XTranslation -= pi_RasterPhysicalWidth;
        }
    // SLO 6
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 1.0, 0.0, pi_RasterOrientedPhysicalHeight, 0.0, -1.0);
//        YTranslation += pi_RasterPhysicalHeight;
        }
    // SLO 7
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(pi_RasterOrientedPhysicalWidth, -1.0, 0.0, pi_RasterOrientedPhysicalHeight, 0.0, -1.0);
//        YTranslation += pi_RasterPhysicalHeight;
//        XTranslation -= pi_RasterPhysicalWidth;
        }

    pSLOAffine->Reverse();
    return((HFCPtr<HGF2DTransfoModel>&)pSLOAffine);

    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFSLOStripAdapter::GetWorldIdentificator () const
    {
    return m_pOriginalFile->GetWorldIdentificator();
    }
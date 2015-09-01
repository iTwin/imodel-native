//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFileBlockAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFRasterFileBlockAdapter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>
#include <Imagepp/all/h/HRFBlockAdapterFactory.h>
#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HRFCacheFileCreator.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

//-----------------------------------------------------------------------------
// HRFRasterFileBlockAdapterBlockCapabilities
//-----------------------------------------------------------------------------
HRFRasterFileBlockAdapterBlockCapabilities::HRFRasterFileBlockAdapterBlockCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Tile Capability
    Add(new HRFTileCapability  (HFC_READ_WRITE_CREATE, // AccessMode
                                LONG_MAX,              // MaxSizeInBytes
                                2,                     // MinWidth
                                4096,                  // MaxWidth
                                1,                     // WidthIncrement
                                2,                     // MinHeight
                                4096,                  // MaxHeight
                                1));                   // HeightIncrement

    // Strip Capability
    Add(new HRFStripCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                               LONG_MAX,               // MaxSizeInBytes
                               1,                      // MinHeight
                               LONG_MAX,               // MaxHeight
                               1));                    // HeightIncrement
    // Line Capability
    Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,   // AccessMode
                              LONG_MAX));              // MaxWidth

    // Image Capability
    Add(new HRFImageCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                               LONG_MAX,               // MaxSizeInBytes
                               1,                      // MinWidth
                               LONG_MAX,               // MaxWidth
                               1,                      // MinHeight
                               LONG_MAX));             // MaxHeight
    }


//-----------------------------------------------------------------------------
// HRFRasterFileBlockAdapterCapabilities
//-----------------------------------------------------------------------------
HRFRasterFileBlockAdapterCapabilities::HRFRasterFileBlockAdapterCapabilities(const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilities)
    : HRFRasterFileCapabilities()
    {
    // Obtain the list of original pixel capability
    HFCPtr<HRFRasterFileCapabilities> ListOfPixelType = pi_rpCapabilities->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID);

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

        bool HasCodecIdentity = false;
        // Add original codecs to the new pixel capabilities
        for (uint32_t CodecIndex=0; CodecIndex < pPixelTypeCap->CountCodecs(); CodecIndex++)
            {
            // Find if codec identity is already in the list
            if(pPixelTypeCap->GetCodecCapabilityByIndex(CodecIndex)->GetCodecClassID() == HCDCodecIdentity::CLASS_ID)
                HasCodecIdentity = true;

            pCodecCapabilities->Add(new HRFCodecCapability(pPixelTypeCap->GetCodecCapabilityByIndex(CodecIndex)->GetAccessMode(),
                                                           pPixelTypeCap->GetCodecCapabilityByIndex(CodecIndex)->GetCodecClassID(),
                                                           new HRFRasterFileBlockAdapterBlockCapabilities()));
            }

        // If not already presant, add the codec Identity. This is need besause some file format do not already
        // have the codec Identity (GIF,PNG, JPEG ...)
        if(!HasCodecIdentity)
            pCodecCapabilities->Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                                           HCDCodecIdentity::CLASS_ID,
                                                           new HRFRasterFileBlockAdapterBlockCapabilities()));
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
// This methods allow to Create the best adapter for the specified raster file.
// If it is not possible to adapt the raster file we return the original raster file.
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileBlockAdapter::CreateBestAdapterFor(HFCPtr<HRFRasterFile> pi_rpForRasterFile)
    {
    HPRECONDITION(pi_rpForRasterFile != 0);

    HFCPtr<HRFRasterFile> pRasterFile = pi_rpForRasterFile;
    BlockDescriptorMap  BlockDescMap;

    if (FindBestAdapterTypeFor(pRasterFile, &BlockDescMap) && CanAdapt(pRasterFile, BlockDescMap))
        pRasterFile = new HRFRasterFileBlockAdapter(pi_rpForRasterFile, BlockDescMap);

    return pRasterFile;
    }

//-----------------------------------------------------------------------------
// This method allows to Create the adapter for the specified raster file and
// its associate cache file.
// If it is not possible to adapt the raster file we return the original raster file.
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileBlockAdapter::CreateBestAdapterBasedOnCacheFor(HFCPtr<HRFRasterFile> pi_rpForRasterFile,
        const HRFCacheFileCreator* pi_pCreator)
    {
    HPRECONDITION(pi_rpForRasterFile != 0);

    // Check if we already have a cache for the source file, if so we open it!
    if ((pi_pCreator != 0) && pi_pCreator->HasCacheFor(pi_rpForRasterFile))
        {
        HFCPtr<HRFRasterFile> pRasterFile = pi_rpForRasterFile;
        BlockDescriptorMap  BlockDescMap;
        HFCPtr<HRFRasterFile> pCacheFile;

        // Get the cache and check if it is a resolution booster (when the cache don't have
        // the same number of resolutions than the source file).
        pCacheFile   = pi_pCreator->GetCacheFileFor(pi_rpForRasterFile);
        HASSERT(pCacheFile != 0);

        // Is the cache valid?
        if (pCacheFile->CountPages() > 0)
            {
            // Try to adapt based on the cache setting. [Page=0 and resolution = 0]
            BlockDescriptor BlockDesc;
            BlockDesc.m_BlockType = pCacheFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType();
            BlockDesc.m_BlockWidth = pCacheFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth();
            BlockDesc.m_BlockHeight = pCacheFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight();
            for (uint32_t Page = 0; Page < pCacheFile->CountPages(); Page++)
                BlockDescMap.insert(BlockDescriptorMap::value_type(Page, BlockDesc));

            pCacheFile = 0;

            if (CanAdapt(pRasterFile, BlockDescMap))
                pRasterFile = new HRFRasterFileBlockAdapter(pi_rpForRasterFile, BlockDescMap);

            return pRasterFile;
            }
        else
            {
            HFCPtr<HFCURL> fileName(pCacheFile->GetURL());
            pCacheFile = 0;

            HASSERT(fileName->IsCompatibleWith(HFCURLFile::CLASS_ID) == true);
            BeFileName::BeDeleteFile((static_cast<HFCURLFile*>(fileName.GetPtr()))->GetAbsoluteFileName().c_str());

            return CreateBestAdapterFor(pi_rpForRasterFile);    // Invalid cache, create a new one
            }
        }
    else
        return CreateBestAdapterFor(pi_rpForRasterFile);
    }

//-----------------------------------------------------------------------------
// This methods allow to find the best adapter type for the specified raster file.
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::FindBestAdapterTypeFor(HFCPtr<HRFRasterFile>  pi_rpForRasterFile,
                                                        HRFBlockType*          po_ToBlockType,
                                                        uint32_t*                po_ToBlockWidth,
                                                        uint32_t*                po_ToBlockHeight)
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    HPRECONDITION(pi_rpForRasterFile->CountPages() == 1);

    BlockDescriptorMap BlockDescMap;

    bool RetValue = FindBestAdapterTypeFor(pi_rpForRasterFile, &BlockDescMap);
    if (RetValue)
        {
        HPRECONDITION(BlockDescMap.size() == 1);
        BlockDescriptorMap::const_iterator Itr(BlockDescMap.begin());
        *po_ToBlockType = Itr->second.m_BlockType;
        *po_ToBlockWidth = Itr->second.m_BlockWidth;
        *po_ToBlockHeight = Itr->second.m_BlockHeight;
        }
    return RetValue;
    }

//-----------------------------------------------------------------------------
// This methods allow to find the best adapter type for the specified raster file.
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::FindBestAdapterTypeFor(const HFCPtr<HRFRasterFile>&    pi_rpForRasterFile,
                                                        BlockDescriptorMap*             po_pBlockDescMap)
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    HPRECONDITION(po_pBlockDescMap != 0);
    HPRECONDITION(po_pBlockDescMap->size() == 0);

    BlockDescriptor     BlockDesc;

    // scan all pages
    for (uint32_t Page = 0; Page < pi_rpForRasterFile->CountPages(); Page++)
        {

        HASSERT(pi_rpForRasterFile->GetPageDescriptor(Page)->CountResolutions() > 0);
        if (HRFBlockAdapterFactory::GetInstance()->FindBestAdapterTypeFor(pi_rpForRasterFile,
                                                                          Page,      // page
                                                                          0,         // resolution
                                                                          &BlockDesc.m_BlockType,
                                                                          &BlockDesc.m_BlockWidth,
                                                                          &BlockDesc.m_BlockHeight))
            {
            po_pBlockDescMap->insert(BlockDescriptorMap::value_type(Page, BlockDesc));
            }
        }

    return (po_pBlockDescMap->size() > 0);
    }

//-----------------------------------------------------------------------------
// This methods allow to if it is possible to adapt this raster file.
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::CanAdapt(HFCPtr<HRFRasterFile>  pi_rpFromRasterFile,
                                          HRFBlockType           pi_ToBlockType,
                                          uint32_t               pi_ToWidth,
                                          uint32_t               pi_ToHeight)
    {
    HPRECONDITION(pi_rpFromRasterFile != 0);

    BlockDescriptor BlockDesc;
    BlockDesc.m_BlockType = pi_ToBlockType;
    BlockDesc.m_BlockWidth = pi_ToWidth;
    BlockDesc.m_BlockHeight = pi_ToHeight;
    BlockDescriptorMap BlockDescMap;

    for(uint32_t Page(0); Page < pi_rpFromRasterFile->CountPages(); ++Page)
        {
        if (!pi_rpFromRasterFile->GetPageDescriptor(Page)->IsUnlimitedResolution())
            BlockDescMap.insert(BlockDescriptorMap::value_type(Page, BlockDesc));
        }

    if (!BlockDescMap.empty())
        return CanAdapt(pi_rpFromRasterFile, BlockDescMap);
    else
        return false;
    }

//-----------------------------------------------------------------------------
// This methods allow to if it is possible to adapt this raster file.
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::CanAdapt(const HFCPtr<HRFRasterFile>&  pi_rpFromRasterFile,
                                          const BlockDescriptorMap&     pi_rBlockDescMap)
    {
    HPRECONDITION(pi_rpFromRasterFile != 0);
    HPRECONDITION(pi_rBlockDescMap.size() <= pi_rpFromRasterFile->CountPages());

    if (pi_rBlockDescMap.size() != 0)
        {
        bool CanAdapt = true;

        BlockDescriptorMap::const_iterator Itr(pi_rBlockDescMap.begin());
        while (Itr != pi_rBlockDescMap.end() && CanAdapt)
            {
            HPRECONDITION(Itr->first < pi_rpFromRasterFile->CountPages());
            HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpFromRasterFile->GetPageDescriptor(Itr->first);

            for (unsigned short Resolution = 0; Resolution < pPageDescriptor->CountResolutions() && CanAdapt; Resolution++)
                {
                if (!HRFBlockAdapterFactory::GetInstance()->CanAdapt(pi_rpFromRasterFile,
                                                                     Itr->first,        // page
                                                                     Resolution,        // resoution
                                                                     Itr->second.m_BlockType,
                                                                     Itr->second.m_BlockWidth,
                                                                     Itr->second.m_BlockHeight))
                    {
                    CanAdapt = false;
                    }
                }
            Itr++;
            }
        return CanAdapt;
        }
    else
        return false;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRasterFileBlockAdapter::HRFRasterFileBlockAdapter(HFCPtr<HRFRasterFile>&  pi_rpAdaptedFile,
                                                     HRFBlockType            pi_AdaptToBlockType,
                                                     uint32_t                pi_AdaptToBlockWidth,
                                                     uint32_t                pi_AdaptToBlockHeight)
    : HRFRasterFileExtender(pi_rpAdaptedFile)
    {
    HPRECONDITION(pi_rpAdaptedFile != 0);

    // if the raster file is an unlimited resolution raster,
    // all page will be adapted with the same parameters
    BlockDescriptorMap BlockDescMap;
    BlockDescriptor BlockDesc;
    BlockDesc.m_BlockType = pi_AdaptToBlockType;
    BlockDesc.m_BlockWidth = pi_AdaptToBlockWidth;
    BlockDesc.m_BlockHeight = pi_AdaptToBlockHeight;
    for (uint32_t Page = 0; Page < pi_rpAdaptedFile->CountPages(); Page++)\
        BlockDescMap.insert(BlockDescriptorMap::value_type(Page, BlockDesc));

    m_IsOpen = false;
    // Create the capabilities
    m_pRasterFileCapabilities =
        new HRFCombinedRasterFileCapabilities(new HRFRasterFileBlockAdapterCapabilities(m_pOriginalFile->GetCapabilities()),
                                              m_pOriginalFile->GetCapabilities());

    if (GetAccessMode().m_HasCreateAccess)
        Create(BlockDescMap);
    else
        Open(BlockDescMap);

    HASSERT(m_IsOpen);
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRasterFileBlockAdapter::HRFRasterFileBlockAdapter(HFCPtr<HRFRasterFile>&     pi_rpAdaptedFile,
                                                     const BlockDescriptorMap&  pi_rBlockDescMap)
    : HRFRasterFileExtender(pi_rpAdaptedFile)
    {
    HPRECONDITION(pi_rpAdaptedFile != 0);
    HPRECONDITION(CanAdapt(pi_rpAdaptedFile,
                           pi_rBlockDescMap));

    m_IsOpen = false;
    // Create the capabilities
    m_pRasterFileCapabilities =
        new HRFCombinedRasterFileCapabilities(new HRFRasterFileBlockAdapterCapabilities(m_pOriginalFile->GetCapabilities()),
                                              m_pOriginalFile->GetCapabilities());

    if (GetAccessMode().m_HasCreateAccess)
        Create(pi_rBlockDescMap);
    else
        Open(pi_rBlockDescMap);

    HASSERT(m_IsOpen);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFRasterFileBlockAdapter::~HRFRasterFileBlockAdapter()
    {
    Close();
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFRasterFileBlockAdapter::CreateResolutionEditor(uint32_t        pi_Page,
                                                                       unsigned short pi_Resolution,
                                                                       HFCAccessMode   pi_AccessMode)
    {
    // Create the editor for the specified resolution
    HRFResolutionEditor* pAdapterResolutionEditor = 0;

    if ((pi_Page < m_pOriginalFile->CountPages()) && (pi_Resolution < m_pOriginalFile->GetPageDescriptor(pi_Page)->CountResolutions()) &&

        (m_pOriginalFile->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType()
         == GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType()) &&

        (m_pOriginalFile->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockWidth()
         == GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockWidth()) &&

        (m_pOriginalFile->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockHeight()
         == GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockHeight()))
        {
        pAdapterResolutionEditor = m_pOriginalFile->CreateResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);
        }
    else
        pAdapterResolutionEditor = HRFBlockAdapterFactory::GetInstance()->New(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pAdapterResolutionEditor;
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
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
const HFCPtr<HRFRasterFileCapabilities>& HRFRasterFileBlockAdapter::GetCapabilities () const
    {
    return m_pRasterFileCapabilities;
    }

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::Open(const BlockDescriptorMap& pi_rBlockDescMap)
    {
    if (!m_IsOpen)
        {
        m_IsOpen = true;
        CreateDescriptors(pi_rBlockDescMap);
        }

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFRasterFileBlockAdapter::CreateDescriptors(const BlockDescriptorMap& pi_rBlockDescMap)
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(pi_rBlockDescMap.size() <= m_pOriginalFile->CountPages());

    HFCPtr<HRFPageDescriptor> pAdapterPageDescriptor;
    BlockDescriptorMap::const_iterator Itr;
    BlockDescriptor BlockDesc;

    // adapt all pages and all resolutions to the new storage type
    for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
        {
        if ((Itr = pi_rBlockDescMap.find(Page)) != pi_rBlockDescMap.end())
            {
            BlockDesc = Itr->second;
            HRFPageDescriptor::ListOfResolutionDescriptor ListOfResolutionDescriptor;
            HFCPtr<HRFPageDescriptor>  pAdaptedPageDescriptor  = m_pOriginalFile->GetPageDescriptor(Page);

            // Adapt each resolutions from adapted file when their access is different of tile
            for (unsigned short Resolution=0; Resolution < pAdaptedPageDescriptor->CountResolutions(); Resolution++)
                {
                HFCPtr<HRFResolutionDescriptor> pAdaptedResDescriptor = pAdaptedPageDescriptor->GetResolutionDescriptor(Resolution);

                // BestMatch a resolution descriptor and insert it to the list
                uint32_t            BlockWidth=0;
                uint32_t            BlockHeight=0;
                HRFBlockAccess      BlockAccess = HRFBlockAccess::RANDOM;


                HFCPtr<HRFRasterFileCapabilities> pPixelTypeCapabilities;
                pPixelTypeCapabilities =
                    GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID,
                                                             m_pOriginalFile->GetAccessMode());

                HASSERT(pPixelTypeCapabilities != 0 && pPixelTypeCapabilities->CountCapabilities() > 0);

                HCLASS_ID CodecID = pAdaptedResDescriptor->GetCodec()->GetClassID();

                HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
                HFCPtr<HRFCodecCapability> pCodecCapability;
                for (uint32_t i = 0; i < pPixelTypeCapabilities->CountCapabilities() && pPixelTypeCapability == 0; i++)
                    {
                    pPixelTypeCapability = (const HFCPtr<HRFPixelTypeCapability>&)pPixelTypeCapabilities->GetCapability(i);
                    if (!(pPixelTypeCapability->GetPixelTypeClassID() == pAdaptedResDescriptor->GetPixelType()->GetClassID() && pPixelTypeCapability->SupportsCodec(CodecID)))
                        pPixelTypeCapability = 0;
                    }

                HASSERT(pPixelTypeCapability != 0);
                HASSERT(pPixelTypeCapability->CountCodecs() > 0);

                // Get the block type attribut.
                if (BlockDesc.m_BlockType == HRFBlockType::TILE)
                    {
                    if (pAdaptedResDescriptor->GetBlockType() == HRFBlockType::TILE)
                        BlockWidth = BlockDesc.m_BlockWidth / pAdaptedResDescriptor->GetBlockWidth() * pAdaptedResDescriptor->GetBlockWidth();
                    else
                        BlockWidth = BlockDesc.m_BlockWidth;

                    BlockHeight = BlockDesc.m_BlockHeight / pAdaptedResDescriptor->GetBlockHeight() * pAdaptedResDescriptor->GetBlockHeight();

                    // Validate the new computed data with the capability.

                    // Find the codec capabiliy associate to the source pixel type and codec
                    HFCPtr<HRFCodecCapability> pCodecCapability;
                    pCodecCapability =
                        pPixelTypeCapability->GetCodecCapability(pAdaptedResDescriptor->GetCodec()->GetClassID());

                    // Get all the valid block type for the source pixel type and codec.
                    HFCPtr<HRFRasterFileCapabilities> pBlockTypeCapabilities;
                    pBlockTypeCapabilities = pCodecCapability->GetBlockTypeCapabilities();

                    HFCPtr<HRFTileCapability> pTileCapability;
                    pTileCapability = static_cast<HRFTileCapability*>(pBlockTypeCapabilities->GetCapabilityOfType(HRFTileCapability::CLASS_ID).GetPtr());
                    HASSERT(pTileCapability != 0);

                    BlockWidth  = pTileCapability->ValidateWidth(BlockWidth);
                    BlockHeight = pTileCapability->ValidateHeight(BlockHeight);
                    }
                else if (BlockDesc.m_BlockType == HRFBlockType::IMAGE)
                    {
                    HASSERT(pAdaptedResDescriptor->GetWidth() <= ULONG_MAX);
                    HASSERT(pAdaptedResDescriptor->GetHeight() <= ULONG_MAX);

                    BlockWidth  = (uint32_t)pAdaptedResDescriptor->GetWidth();
                    BlockHeight = (uint32_t)pAdaptedResDescriptor->GetHeight();
                    }
                else if (BlockDesc.m_BlockType == HRFBlockType::STRIP)
                    {
                    HASSERT(pAdaptedResDescriptor->GetWidth() <= ULONG_MAX);

                    BlockWidth  = (uint32_t)pAdaptedResDescriptor->GetWidth();

                    // Adjust to a multiple of adapted blocks
                    if (BlockDesc.m_BlockHeight > pAdaptedResDescriptor->GetHeight())
                        {
                        HASSERT(pAdaptedResDescriptor->GetBlocksPerHeight() <= ULONG_MAX);

                        BlockHeight = (uint32_t)pAdaptedResDescriptor->GetBlocksPerHeight() * pAdaptedResDescriptor->GetBlockHeight();
                        }
                    else
                        BlockHeight = BlockDesc.m_BlockHeight / pAdaptedResDescriptor->GetBlockHeight() * pAdaptedResDescriptor->GetBlockHeight();

                    // Take src block height
                    if (BlockHeight == 0)
                        BlockHeight = pAdaptedResDescriptor->GetBlockHeight();

                    // Validate the new computed data with the capability.

                    // Find the codec capability associate to the source pixel type and codec
                    HFCPtr<HRFCodecCapability> pCodecCapability;
                    pCodecCapability =
                        pPixelTypeCapability->GetCodecCapability(pAdaptedResDescriptor->GetCodec()->GetClassID());

                    // Get all the valid block type for the source pixel type and codec.
                    HFCPtr<HRFRasterFileCapabilities> pBlockTypeCapabilities;
                    pBlockTypeCapabilities = pCodecCapability->GetBlockTypeCapabilities();

                    HFCPtr<HRFStripCapability> pStripCapability;
                    pStripCapability = static_cast<HRFStripCapability*>(pBlockTypeCapabilities->GetCapabilityOfType(HRFStripCapability::CLASS_ID).GetPtr());

                    HASSERT(pStripCapability !=0);

                    BlockHeight = pStripCapability->ValidateHeight(BlockHeight);
                    }
                else if (BlockDesc.m_BlockType == HRFBlockType::LINE)
                    {
                    HASSERT(pAdaptedResDescriptor->GetWidth() <= ULONG_MAX);

                    BlockWidth  = (uint32_t)pAdaptedResDescriptor->GetWidth();
                    BlockHeight = 1;
                    }

                HFCPtr<HRFResolutionDescriptor> pAdapterResolutionDescriptor =
                    new HRFResolutionDescriptor(pAdaptedResDescriptor->GetAccessMode(),          // AccessMode
                                                GetCapabilities(),                               // Capabilities,
                                                pAdaptedResDescriptor->GetResolutionXRatio(),    // ResolutionRatio,
                                                pAdaptedResDescriptor->GetResolutionYRatio(),    // ResolutionRatio,
                                                pAdaptedResDescriptor->GetPixelType(),           // PixelType,
                                                new HCDCodecIdentity,                            // Codec,
                                                pAdaptedResDescriptor->GetReaderBlockAccess(),   // BlockAccess,
                                                pAdaptedResDescriptor->GetWriterBlockAccess(),   // BlockAccess,
                                                pAdaptedResDescriptor->GetScanlineOrientation(), // ScanLineOrientation,
                                                pAdaptedResDescriptor->GetInterleaveType(),      // InterleaveType
                                                pAdaptedResDescriptor->IsInterlace(),            // IsInterlace,
                                                pAdaptedResDescriptor->GetWidth(),               // Width,
                                                pAdaptedResDescriptor->GetHeight(),              // Height,
                                                BlockWidth,                                      // BlockWidth,
                                                BlockHeight,                                     // BlockHeight,
                                                0,                                               // BlocksDataFlag
                                                BlockDesc.m_BlockType,
                                                pAdaptedResDescriptor->GetNumberOfPass(),        // NumberOfPass
                                                pAdaptedResDescriptor->GetPaddingBits(),         // PaddingBits
                                                pAdaptedResDescriptor->GetDownSamplingMethod()); // DownSamplingMethod

                // We set all flags to LOADED
                if (GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, HFC_WRITE_ONLY) != 0)
                    {
                    HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
                    HPRECONDITION(pAdapterResolutionDescriptor->CountBlocks() <= SIZE_MAX);
                    HPRECONDITION(pAdapterResolutionDescriptor->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);
                    pBlocksDataFlag = new HRFDataFlag[(size_t)pAdapterResolutionDescriptor->CountBlocks()];
                    memset(pBlocksDataFlag,
                           HRFDATAFLAG_LOADED,
                           (size_t)(pAdapterResolutionDescriptor->CountBlocks() * sizeof(HRFDataFlag)));
                    pAdapterResolutionDescriptor->SetBlocksDataFlag(pBlocksDataFlag);
                    }
                ListOfResolutionDescriptor.push_back(pAdapterResolutionDescriptor);
                }

            // Best match pages and resolutions
            HRPPixelPalette*                  pRepresentativePalette    = 0;
            HRPHistogram*                     pHistogram                = 0;
            HRFThumbnail*                     pThumbnail                = 0;
            HRFClipShape*                     pClipShape                = 0;
            HGF2DTransfoModel*                pTransfoModel             = 0;
            HRPFilter*                        pFilters                  = 0;

            // Best match the TransfoModel
            if (pAdaptedPageDescriptor->HasTransfoModel())
                pTransfoModel = pAdaptedPageDescriptor->GetTransfoModel();

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

            pAdapterPageDescriptor = new HRFPageDescriptor(pAdaptedPageDescriptor->GetAccessMode(),
                                                           GetCapabilities(),
                                                           ListOfResolutionDescriptor,
                                                           pRepresentativePalette,
                                                           pHistogram,
                                                           pThumbnail,
                                                           pClipShape,
                                                           pTransfoModel,
                                                           pFilters,
                                                           (HPMAttributeSet*)&pAdaptedPageDescriptor->GetTags(),
                                                           pAdaptedPageDescriptor->GetDuration(),
                                                           pAdaptedPageDescriptor->IsResizable());

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
            }
        else
            pAdapterPageDescriptor = m_pOriginalFile->GetPageDescriptor(Page);

        // Add the page descriptor to the list
        m_ListOfPageDescriptor.push_back(pAdapterPageDescriptor);
        }
    }


//-----------------------------------------------------------------------------
// Public
// ResizePage
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::ResizePage(uint32_t pi_Page,
                                            uint64_t pi_NewWidth,
                                            uint64_t pi_NewHeight)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page)->CountResolutions() == 1);

    bool Result = T_Super::ResizePage(pi_Page, pi_NewWidth, pi_NewHeight);

    return (Result ? m_pOriginalFile->ResizePage(pi_Page, pi_NewWidth, pi_NewHeight) : Result);
    }


//-----------------------------------------------------------------------------
// Public
// Save
// Save the decorator and the extended.
//-----------------------------------------------------------------------------
void HRFRasterFileBlockAdapter::Save()
    {
    SynchronizeFiles();
    HRFRasterFileExtender::Save();
    }

//-----------------------------------------------------------------------------
// Private
// This method close the file.
//-----------------------------------------------------------------------------
void HRFRasterFileBlockAdapter::Close()
    {
    SynchronizeFiles();
    m_IsOpen = false;
    }

//-----------------------------------------------------------------------------
// Private
// SynchronizeFiles
// Synchronize the cached file and the cache file.
//-----------------------------------------------------------------------------
void HRFRasterFileBlockAdapter::SynchronizeFiles()
    {
    // close the decorator file
    if (m_IsOpen)
        {
        // Update Descriptors and raster data
        for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
            {
            // Obtain the Page descriptor
            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

            // Update the page Descriptors

            // Update the page size
            if (pPageDescriptor->PageSizeHasChanged())
                {
//                m_pOriginalFile->GetPageDescriptor(Page)->Resize(pPageDescriptor->GetResolutionDescriptor(0)->GetWidth(),
//                                                                 pPageDescriptor->GetResolutionDescriptor(0)->GetHeight());
                }

            // Update the TransfoModel
            if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                {
                // Update to original file
                m_pOriginalFile->GetPageDescriptor(Page)->SetTransfoModel(*pPageDescriptor->GetTransfoModel());
                }
            // Update the Filter
            if ((pPageDescriptor->HasFilter()) && (pPageDescriptor->FiltersHasChanged()))
                {
                // Update to original file
                m_pOriginalFile->GetPageDescriptor(Page)->SetFilter(pPageDescriptor->GetFilter());
                }

            // Update the ClipShape
            if ((pPageDescriptor->HasClipShape()) && (pPageDescriptor->ClipShapeHasChanged()))
                {
                // Update to original file
                m_pOriginalFile->GetPageDescriptor(Page)->SetClipShape(*pPageDescriptor->GetClipShape());
                }

            // Update the Histogram
            if ((pPageDescriptor->HasHistogram()) && (pPageDescriptor->HistogramHasChanged()))
                {
                // Update to original file
                m_pOriginalFile->GetPageDescriptor(Page)->SetHistogram(*pPageDescriptor->GetHistogram());
                }

            // Update the Thumbnail
            if ((pPageDescriptor->HasThumbnail()) && (pPageDescriptor->ThumbnailHasChanged()))
                {
                // Update to original file
                m_pOriginalFile->GetPageDescriptor(Page)->SetThumbnail(*pPageDescriptor->GetThumbnail());
                }

            // Update the RepresentativePalette
            if ((pPageDescriptor->HasRepresentativePalette()) && (pPageDescriptor->RepresentativePaletteHasChanged()))
                {
                // Update to original file
                m_pOriginalFile->GetPageDescriptor(Page)->SetRepresentativePalette(pPageDescriptor->GetRepresentativePalette());
                }

            // Update each tag to the original file and cache file.
            HPMAttributeSet::HPMASiterator TagIterator;

            for (TagIterator  = pPageDescriptor->GetTags().begin();
                 TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)
                {

                if (pPageDescriptor->TagHasChanged(**TagIterator) || GetAccessMode().m_HasCreateAccess)
                    {
                    HFCPtr<HRFCapability> pWriteCapability( new HRFTagCapability(HFC_WRITE_ONLY, *TagIterator) );

                    // Update to Source file
                    if (m_pOriginalFile->GetCapabilities()->Supports(pWriteCapability))
                        m_pOriginalFile->GetPageDescriptor(Page)->SetTag((*TagIterator)->Clone());
                    }
                }

            pPageDescriptor->Saved();
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::Create(const BlockDescriptorMap& pi_rBlockDescMap)
    {
    m_IsOpen = true;
    CreateDescriptors(pi_rBlockDescMap);

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFRasterFileBlockAdapter::GetWorldIdentificator () const
    {
    return m_pOriginalFile->GetWorldIdentificator();
    }

//-----------------------------------------------------------------------------
// Public
// IsOriginalRasterDataStorage
//-----------------------------------------------------------------------------
bool HRFRasterFileBlockAdapter::IsOriginalRasterDataStorage() const
    {
    // false, this class change the block type
    return false;
    }
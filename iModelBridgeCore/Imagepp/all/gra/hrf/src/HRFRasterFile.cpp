//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRasterFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFResolutionEditor.h>
#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HVETileIDIterator.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HVE2DRectangle.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HFCMemoryBinStream.h>
#include <ImagePP/all/h/HFCStat.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>

//-----------------------------------------------------------------------------
// Public
// GetDefaultExtension
// Get the Default Extension
//-----------------------------------------------------------------------------
HRFRasterFileCreator::HRFRasterFileCreator(HCLASS_ID pi_ClassID)
    {
    m_ClassID    = pi_ClassID;
    m_AccessMode = HFC_NO_ACCESS;
    }

//-----------------------------------------------------------------------------
// Public
// Destruc
//-----------------------------------------------------------------------------
HRFRasterFileCreator::~HRFRasterFileCreator()
    {
    }

//-----------------------------------------------------------------------------
// Public
// CanRegister
// This method is use by the factory. Return true if the raster file can be
// registered, false otherwise
//-----------------------------------------------------------------------------
bool HRFRasterFileCreator::CanRegister() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// Public
// Returns the class ID of the raster file associated with the create
//-----------------------------------------------------------------------------
HCLASS_ID HRFRasterFileCreator::GetRasterFileClassID() const
    {
    return m_ClassID;
    }

//-----------------------------------------------------------------------------
// Public
// GetDefaultExtension
// Get the Default Extension
//-----------------------------------------------------------------------------
WString HRFRasterFileCreator::GetDefaultExtension() const
    {
    // Find the first file extension
    WString::size_type SeparatorPos = GetExtensions().find(L';');
    WString Extension;

    if (SeparatorPos != WString::npos)
        Extension = GetExtensions() .substr(0, SeparatorPos);
    else
        Extension = GetExtensions();

    return Extension;
    }

//-----------------------------------------------------------------------------
// Public
// SupportsURL
// This Generic function can be overwrite
//-----------------------------------------------------------------------------
bool HRFRasterFileCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    bool Support = false;

    if (SupportsScheme(pi_rpURL->GetSchemeType()))
        {
        // Generic Test for Local file
        if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
            {
            // Extract the extension
            Support = SupportsExtension(((HFCPtr<HFCURLFile>&)pi_rpURL)->GetExtension());
            }
        else if (pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID))
            {
            // Extract the extension
            Support = SupportsExtension(((HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetExtension());
            }
        }

    return Support;
    }

//-----------------------------------------------------------------------------
// Public
// SupportsScheme
//-----------------------------------------------------------------------------
bool HRFRasterFileCreator::SupportsScheme(const WString& pi_rScheme) const
    {
    // Find the specified Scheme
    WString Schemes(GetSchemes());
    CaseInsensitiveStringTools().ToLower(Schemes);

    WString SchemeToFind(pi_rScheme);
    CaseInsensitiveStringTools().ToLower(SchemeToFind);

    WString::size_type Pos = Schemes.find(SchemeToFind);
    bool  Support = false;

    if (Pos != WString::npos)
        {
        // We found the Scheme only when the Scheme is the last or
        // the next char is a separator
        if ((Pos + pi_rScheme.length() < Schemes.length()) &&
            (Schemes.substr(Pos + pi_rScheme.length(), 1) != L";"))
            Support = false;
        else
            Support = true;
        }

    return Support;
    }

//-----------------------------------------------------------------------------
// Public
// SupportsExtension
//-----------------------------------------------------------------------------
bool HRFRasterFileCreator::SupportsExtension(const WString& pi_rExtension) const
    {
    // Find the specified Extension
    WString Extensions(GetExtensions());
    CaseInsensitiveStringTools().ToLower(Extensions);

    WString ExtensionToFind(pi_rExtension);
    CaseInsensitiveStringTools().ToLower(ExtensionToFind);

    WString::size_type Pos = Extensions.find(ExtensionToFind);
    bool  Support = false;

    if (Pos != WString::npos)
        {
        // We found the extension only when the extension is the last or
        // the next char is a separator
        if (((Pos + pi_rExtension.length() < Extensions.length()) &&
             (Extensions.substr(Pos + pi_rExtension.length(), 1) != L";")) ||
            (Extensions.substr(Pos-1, 1) != L"."))
            Support = false;
        else
            Support = true;
        }

    return Support;
    }

//-----------------------------------------------------------------------------
// Capability of access mode
//-----------------------------------------------------------------------------
HFCAccessMode HRFRasterFileCreator::GetSupportedAccessMode() const
    {
    HRFRasterFileCreator* This = ((HRFRasterFileCreator*)this);

    if ((m_AccessMode == HFC_NO_ACCESS) && (This->GetCapabilities() != 0))
        {
        // Compute a generic access mode

        // READ access
        if (This->GetCapabilities()->GetCapabilityOfType(HRFPixelTypeCapability::CLASS_ID, HFC_READ_ONLY).GetPtr() != 0)
            This->m_AccessMode.m_HasReadAccess = true;

        // WRITE access
        if (This->GetCapabilities()->GetCapabilityOfType(HRFPixelTypeCapability::CLASS_ID, HFC_WRITE_ONLY).GetPtr() != 0)
            This->m_AccessMode.m_HasWriteAccess = true;

        // CREATE access
        if (This->GetCapabilities()->GetCapabilityOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr() != 0)
            This->m_AccessMode.m_HasCreateAccess = true;
        }
    return m_AccessMode;
    }


//-----------------------------------------------------------------------------
// Public
// This must be overwritten by classes that need a path for dll.
//-----------------------------------------------------------------------------
bool HRFRasterFileCreator::NeedRasterDllDirectory() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// Public
// Set a path for dll.
//-----------------------------------------------------------------------------
void HRFRasterFileCreator::SetRasterDllDirectory(const WString& pi_rDllDirectory)
    {
    HPRECONDITION(NeedRasterDllDirectory());

    m_DllDirectory = pi_rDllDirectory;
    }

//-----------------------------------------------------------------------------
// Public
// Return the dll path setted by GetRasterDllDirectory.
//-----------------------------------------------------------------------------
const WString& HRFRasterFileCreator::GetRasterDllDirectory() const
    {
    return m_DllDirectory;
    }


//-----------------------------------------------------------------------------
// HMR Message Map
//-----------------------------------------------------------------------------
HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRFRasterFile, HMGMessageDuplex, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRFRasterFile, HRFBlockNotificationMsg, NotifyBlockReady)
HMG_END_MESSAGE_MAP()



//-----------------------------------------------------------------------------
// Static member initialization
//-----------------------------------------------------------------------------

// Tile Descriptor used only to build IDs from an index and resolution
const HGFTileIDDescriptor HRFRasterFile::s_TileDescriptor;

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRasterFile::HRFRasterFile(const HFCPtr<HFCURL>& pi_rpURL,
                             HFCAccessMode         pi_PhysicalAccessMode,
                             uint64_t             pi_Offset)
    {
    HPRECONDITION(pi_rpURL != 0);
    m_pURL                   = pi_rpURL;
    m_Offset                 = pi_Offset;
    m_PhysicalAccessMode     = pi_PhysicalAccessMode;
    m_LogicalAccessMode      = pi_PhysicalAccessMode;
    m_IsCreateCancel         = false;

    //By default false as it was in HIERasterFile::IsGeotiffUnitFoundInFile
    //before creating a generic interface.
    SetUnitFoundInFile(false);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFRasterFile::~HRFRasterFile()
    {
    // All editors should be destroying before destroying this rasterfile
    HASSERT(m_ResolutionEditorRegistry.size() == 0);

    HFCMonitor Monitor(m_ConsumersKey);
    PageConsumerMap::iterator PageConsumers(m_PageConsumers.begin());
    while (PageConsumers != m_PageConsumers.end())
        {
        ConsumerMap::iterator Consumer(PageConsumers->second.begin());
        while (Consumer != PageConsumers->second.end())
            {
            delete Consumer->second;
            Consumer++;
            }
        PageConsumers++;
        }
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFRasterFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation with the capabilities if it's possible to add a page
    m_ListOfPageDescriptor.push_back(pi_pPage);

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// Resize
// Change the page size
//-----------------------------------------------------------------------------
bool HRFRasterFile::ResizePage(uint32_t pi_Page,
                                uint64_t pi_Width,
                                uint64_t pi_Height)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page)->IsResizable());
    HPRECONDITION(GetPageDescriptor(pi_Page)->CountResolutions() == 1);

    bool Result = true;
    HFCPtr<HRFPageDescriptor> pPage(GetPageDescriptor(pi_Page));
    HFCPtr<HRFResolutionDescriptor> pRes(pPage->GetResolutionDescriptor(0));

    // first, change the resolution descriptor

    uint32_t BlockWidth=0;
    uint32_t BlockHeight=0;

    if (pRes->GetBlockType() == HRFBlockType::LINE)
        {
        HPRECONDITION(pi_Width <= ULONG_MAX);
        BlockWidth = (uint32_t)pi_Width;
        BlockHeight = pRes->GetBlockHeight();
        }
    else if (pRes->GetBlockType() == HRFBlockType::TILE)
        {
        BlockWidth = pRes->GetBlockWidth();
        BlockHeight = pRes->GetBlockHeight();
        }
    else if (pRes->GetBlockType() == HRFBlockType::STRIP)
        {
        HPRECONDITION(pi_Width <= ULONG_MAX);
        BlockWidth = (uint32_t)pi_Width;
        BlockHeight = pRes->GetBlockHeight();
        }
    else if (pRes->GetBlockType() == HRFBlockType::IMAGE)
        {
        HPRECONDITION(pi_Width <= ULONG_MAX && pi_Height <= ULONG_MAX);

        BlockWidth = (uint32_t)pi_Width;
        BlockHeight = (uint32_t)pi_Height;
        }
    else
        {
        HASSERT(0);
        Result = false;
        }

    HFCPtr<HRFResolutionDescriptor> pNewRes(new HRFResolutionDescriptor(GetAccessMode(),
                                                                        GetCapabilities(),
                                                                        pRes->GetResolutionXRatio(),
                                                                        pRes->GetResolutionYRatio(),
                                                                        pRes->GetPixelType(),
                                                                        pRes->GetCodec(),
                                                                        pRes->GetReaderBlockAccess(),
                                                                        pRes->GetWriterBlockAccess(),
                                                                        pRes->GetScanlineOrientation(),
                                                                        pRes->GetInterleaveType(),
                                                                        pRes->IsInterlace(),
                                                                        pi_Width,
                                                                        pi_Height,
                                                                        BlockWidth,
                                                                        BlockHeight));

    pPage->m_ListOfResolutionDescriptor[0] = pNewRes;
    pPage->m_PageSizeHasChanged = true;

    // notify all registered resolution editor that the page size has changed
    ResolutionEditorRegistry::iterator Itr(m_ResolutionEditorRegistry.begin());
    while (Itr != m_ResolutionEditorRegistry.end())
        {
        if ((*Itr)->GetPage() == pi_Page)          // supported only for single resolution, no need to check the editor resolution
            (*Itr)->ResolutionSizeHasChanged();
        Itr++;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Private
// RegisterResolutionEditor
// Add a resolution descriptor to the list
//-----------------------------------------------------------------------------
void HRFRasterFile::RegisterResolutionEditor(const HRFResolutionEditor* pi_pResolutionEditor)
    {
    m_ResolutionEditorRegistry.push_back(pi_pResolutionEditor);
    }

//-----------------------------------------------------------------------------
// Private
// UnregisterResolutionEditor
// Add a resolution descriptor to the list
//-----------------------------------------------------------------------------
void HRFRasterFile::UnregisterResolutionEditor(const HRFResolutionEditor* pi_pResolutionEditor)
    {
    ResolutionEditorRegistry::iterator Itr(find(m_ResolutionEditorRegistry.begin(),
                                                m_ResolutionEditorRegistry.end(),
                                                pi_pResolutionEditor));

    if (Itr != m_ResolutionEditorRegistry.end())
        {
        m_ResolutionEditorRegistry.erase(Itr);
        }
    }

//-----------------------------------------------------------------------------
// Public
// Return the current size of the file
//-----------------------------------------------------------------------------
uint64_t HRFRasterFile::GetFileCurrentSize() const
    {
    //HASSERT(0); //Must be defined by the derived classes.

    HFCStat fileStat(GetURL());
    return fileStat.GetSize();
    }

//-----------------------------------------------------------------------------
// Public
// Return the current size of the file, which might not be accurate in the
// case of a file access by a third party library due to internal cache in
// such a library.
//-----------------------------------------------------------------------------
uint64_t HRFRasterFile::GetFileCurrentSize(HFCBinStream* pi_pBinStream) const
    {
    HPRECONDITION(dynamic_cast<HFCLocalBinStream*>(pi_pBinStream) != 0);

    HFCLocalBinStream* pLocalBinStream = dynamic_cast<HFCLocalBinStream*>(pi_pBinStream);

    HASSERT(pLocalBinStream != 0);

    return pLocalBinStream->GetCurrentFileSize();
    }

//-----------------------------------------------------------------------------
// Public
// Set the default ratio to meter in case the file has no units information.
// The purpose of the last three parameters is to have an interface working
// with GeoTIFF.
//-----------------------------------------------------------------------------
void HRFRasterFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                           uint32_t pi_Page,
                                           bool   pi_CheckSpecificUnitSpec,
                                           bool   pi_InterpretUnitINTGR)
    {
    HPRECONDITION(pi_Page < CountPages());

    InitializeDefaultRatioToMeter();

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(pi_Page);
    if ((pPageDescriptor != 0) &&
        (pPageDescriptor->HasTransfoModel() == true) &&
        (pi_RatioToMeter != m_DefaultRatioToMeter[pi_Page]))
        {
        double RatioToMeter = pi_RatioToMeter / m_DefaultRatioToMeter[pi_Page];
        bool   DefaultUnitWasFound = false;

        m_DefaultRatioToMeter[pi_Page] = pi_RatioToMeter;

        GeoCoordinates::BaseGCSCP pBaseGCS = pPageDescriptor->GetGeocodingCP();

        if (pBaseGCS != NULL && pBaseGCS->IsValid())    // &&AR GCS assumed that pPageDescriptor->GetRasterFileGeocoding() is using pBaseGCS.
            {

            HFCPtr<HGF2DTransfoModel> pTransfoModel = pPageDescriptor->GetRasterFileGeocoding().TranslateToMeter(pPageDescriptor->GetTransfoModel(),
                                                                                     RatioToMeter,
                                                                                     pi_CheckSpecificUnitSpec,
                                                                                     &DefaultUnitWasFound);

            if (DefaultUnitWasFound == false)
                {
                pPageDescriptor->SetTransfoModel(*pTransfoModel, true /*ignore capabilities*/);
                pPageDescriptor->SetTransfoModelUnchanged();
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// Public
// Get the default ratio to meter
//-----------------------------------------------------------------------------
double HRFRasterFile::GetDefaultRatioToMeter(uint32_t pi_Page) const
    {
    double RatioToMeter = 1.0;

    //Since m_DefaultRatioToMeter isn't updated when a page is added,
    //it is possible that there is no default value for the requested page.
    if (pi_Page < m_DefaultRatioToMeter.size())
        {
        RatioToMeter = m_DefaultRatioToMeter[pi_Page];
        }

    return RatioToMeter;
    }

//-----------------------------------------------------------------------------
// LookAhead section begin
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Public
// Prefetch data
//-----------------------------------------------------------------------------
void HRFRasterFile::SetLookAhead(uint32_t              pi_Page,
                                 const HGFTileIDList&   pi_rBlocks,
                                 uint32_t              pi_ConsumerID,
                                 bool                   pi_Async)
    {
    HPRECONDITION(HasLookAhead(pi_Page));

    // verify if the look ahead can be performed right now
    if (!pi_rBlocks.empty() && CanPerformLookAhead(pi_Page))
        {
        HGFTileIDList Blocks(pi_rBlocks);
        HFCMonitor    Monitor(m_ConsumersKey);

        // Cancellation
        if (CanCancelLookAhead(pi_Page, Blocks, pi_ConsumerID) && !Blocks.empty())
            {
            // Add the new blocks to the current consumer
            AddBlocksToConsumer(pi_Page, pi_ConsumerID, pi_rBlocks, true);

            // Cancel the current LookAhead
            CancelLookAhead(pi_Page);

            HDEBUGTEXT(L"Resetting after cancel\r\n");

            // Reset the look Ahead for everybody
            Monitor.ReleaseKey();
            ResetLookAhead(pi_Page, pi_Async);
            }

        // no cancellation
        else if (!Blocks.empty())
            {
            // Add the new blocks to the current consumer
            AddBlocksToConsumer(pi_Page, pi_ConsumerID, Blocks, false);

            // Request the needed blocks
            Monitor.ReleaseKey();
            RequestLookAhead(pi_Page, Blocks, pi_Async);
            }
        }
    else if (pi_rBlocks.empty() && CanCancelLookAhead(pi_Page, (HGFTileIDList&)pi_rBlocks, pi_ConsumerID))
        CancelLookAhead(pi_Page);
    }


//-----------------------------------------------------------------------------
// Public
// Prefetch data in a given region
//-----------------------------------------------------------------------------
void HRFRasterFile::SetLookAhead(uint32_t          pi_Page,
                                 unsigned short     pi_Resolution,
                                 const HVEShape&    pi_rShape,
                                 uint32_t          pi_ConsumerID,
                                 bool               pi_Async)
    {
    HPRECONDITION(HasLookAhead(pi_Page));

    // verify if the look ahead can be performed right now
    if (!pi_rShape.IsEmpty() && CanPerformLookAhead(pi_Page))
        {
        HGFTileIDList Blocks;
        HFCMonitor    Monitor(m_ConsumersKey);

        HFCPtr<HRFResolutionDescriptor> pResDesc;

        if (GetPageDescriptor(pi_Page)->IsUnlimitedResolution())
            {
            // an unlimited resolution raster has only the main resolution.
            // A resolution index was associated to an editor...
            // find the resolution editor into the ResolutionEditorRegistry
            ResolutionEditorRegistry::const_iterator ResEditorItr(m_ResolutionEditorRegistry.begin());
            HRFResolutionEditor* pResEditor = 0;
            while (pResEditor == 0 && ResEditorItr != m_ResolutionEditorRegistry.end())
                {
                if ((*ResEditorItr)->GetPage() == pi_Page && (*ResEditorItr)->GetResolutionIndex() == pi_Resolution)
                    pResEditor = (HRFResolutionEditor*)*ResEditorItr;
                else
                    ResEditorItr++;
                }
            HASSERT(pResEditor != 0);

            pResDesc = pResEditor->GetResolutionDescriptor();
            }
        else
            {
            pResDesc = GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution);
            }

        // Extract the needed blocks from the region
        SnapRegionToGrid(pi_Page, pi_Resolution, pi_rShape, &Blocks);

        SetLookAhead(pi_Page, Blocks, pi_ConsumerID, pi_Async);
        }
    }


//-----------------------------------------------------------------------------
// Public
// Prefetch data in a given region
//-----------------------------------------------------------------------------
void HRFRasterFile::StopLookAhead(uint32_t pi_Page, uint32_t pi_ConsumerID)
    {
    HPRECONDITION(HasLookAhead(pi_Page));

    // Set LookAhead with an empty region so that cancellation may occur
    SetLookAhead(pi_Page, 0, HGF2DExtent(), pi_ConsumerID, true);

    // remove the consumer from the map
    HFCMonitor Monitor(m_ConsumersKey);
    PageConsumerMap::iterator PageConsumers(m_PageConsumers.find(pi_Page));
    HPOSTCONDITION(PageConsumers != m_PageConsumers.end());
    ConsumerMap::iterator Consumer(PageConsumers->second.find(pi_ConsumerID));
    if (Consumer != PageConsumers->second.end())
        {
        delete Consumer->second;
        PageConsumers->second.erase(Consumer);
        }
    }


//-----------------------------------------------------------------------------
// Public
// This method is used in SetLookAhead to verify if the derived class is
// ready to receive LookAhead request.  It returns true by default.
//-----------------------------------------------------------------------------
bool HRFRasterFile::CanPerformLookAhead (uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return (true);
    }

//-----------------------------------------------------------------------------
// public
// Local version - called by the descendant when a block is ready.  It will notify
// any other HRF object linked unto us.
//-----------------------------------------------------------------------------
void HRFRasterFile::NotifyBlockReady (uint32_t  pi_Page,
                                      uint64_t   pi_BlockID)
    {
    // remove the block from all the local consumers
    RemoveBlockFromConsumers(pi_Page, pi_BlockID);

    // Notify any other HRF linked unto us
    Propagate(HRFBlockNotificationMsg(pi_Page, pi_BlockID));
    }


//-----------------------------------------------------------------------------
// public
// Received when a class
//-----------------------------------------------------------------------------
bool HRFRasterFile::NotifyBlockReady (const HMGMessage& pi_rMessage)
    {
    HPRECONDITION(pi_rMessage.IsCompatibleWith(HRFBlockNotificationMsg::CLASS_ID));

    // remove the block from all the local consumers
    RemoveBlockFromConsumers(((HRFBlockNotificationMsg&)pi_rMessage).GetPage(),
                             ((HRFBlockNotificationMsg&)pi_rMessage).GetBlockID());

    // Notify any other HRF linked unto us
    return (true);
    }

//-----------------------------------------------------------------------------
// LookAhead section end
//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// LookAhead section begin
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// protected
// This method is called by a derived-class when the link with the
// source of data blocks has been re-established
//-----------------------------------------------------------------------------
void HRFRasterFile::ResetLookAhead (uint32_t pi_Page,
                                    bool   pi_Async)
    {
    // verify if the look ahead can be performed right now
    if (CanPerformLookAhead(pi_Page))
        {
        HFCMonitor                  Monitor(m_ConsumersKey);
        PageConsumerMap::const_iterator
        PageConsumer(m_PageConsumers.find(pi_Page));
        HPOSTCONDITION(PageConsumer != m_PageConsumers.end());
        ConsumerMap::const_iterator Consumer;

        // Request all the tiles from all the consumers
        for (Consumer = PageConsumer->second.begin(); Consumer != PageConsumer->second.end(); Consumer++)
            RequestLookAhead(pi_Page, *(Consumer->second), pi_Async);
        }
    }


//-----------------------------------------------------------------------------
// Protected
// This method is used in SetLookAhead to give the list of needed tiles
// to a derived class, since it knows how to obtain the tiles.
//-----------------------------------------------------------------------------
void HRFRasterFile::RequestLookAhead(uint32_t               pi_Page,
                                     const HGFTileIDList&   pi_rBlocks,
                                     bool                  pi_Async)
    {
    HPRECONDITION(pi_Page < CountPages());

    HASSERT(false);
    }


//-----------------------------------------------------------------------------
// Protected
// This method is used in SetLookAhead to indicate to a derived class that
// the current LookAhead has been cancelled.
//-----------------------------------------------------------------------------
void HRFRasterFile::CancelLookAhead(uint32_t pi_Page)
    {
    HPRECONDITION(pi_Page < CountPages());
    }


//-----------------------------------------------------------------------------
// LookAhead section end
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// protected
// this methods converts a region into a list of tiles
//-----------------------------------------------------------------------------
void HRFRasterFile::SnapRegionToGrid(uint32_t                   pi_Page,
                                     unsigned short            pi_Resolution,
                                     const HVEShape&            pi_rShape,
                                     HGFTileIDList*             po_pBlocks) const
    {
    HPRECONDITION(po_pBlocks != 0);
    HPRECONDITION(po_pBlocks->size() == 0);

    HFCPtr<HRFResolutionDescriptor> pResDesc;

    if (GetPageDescriptor(pi_Page)->IsUnlimitedResolution())
        {
        // an unlimited resolution raster has only the main resolution.
        // A resolution index was associated to an editor...
        // find the resolution editor into the ResolutionEditorRegistry
        ResolutionEditorRegistry::const_iterator ResEditorItr(m_ResolutionEditorRegistry.begin());
        HRFResolutionEditor* pResEditor = 0;
        while (pResEditor == 0 && ResEditorItr != m_ResolutionEditorRegistry.end())
            {
            if ((*ResEditorItr)->GetPage() == pi_Page && (*ResEditorItr)->GetResolutionIndex() == pi_Resolution)
                pResEditor = (HRFResolutionEditor*)*ResEditorItr;
            else
                ResEditorItr++;
            }
        HASSERT(pResEditor != 0);

        pResDesc = pResEditor->GetResolutionDescriptor();
        }
    else
        {

        pResDesc = GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution);
        }

    HGFTileIDDescriptor TileDesc(pResDesc->GetWidth(),
                                 pResDesc->GetHeight(),
                                 pResDesc->GetBlockWidth(),
                                 pResDesc->GetBlockHeight());

    // Make sure the input shape will not be destroyed
    HVEShape* pShape = const_cast<HVEShape*>(&pi_rShape);
    pShape->IncrementRef();
        {
        HVETileIDIterator TileIterator(&TileDesc, HFCPtr<HVEShape>(pShape));

        uint64_t BlockIndex = TileIterator.GetFirstTileIndex();
        while (BlockIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
            {
            po_pBlocks->push_back(s_TileDescriptor.ComputeIDFromIndex(BlockIndex, pi_Resolution));

            BlockIndex = TileIterator.GetNextTileIndex();
            }
        }
    // Decrement after the block because the HVETileIDIterator keeps an
    // HFCPtr to the input shape, so we want the Iterator to be destroyed
    // before we decrement.
    pShape->DecrementRef();
    }


//-----------------------------------------------------------------------------
// Protected
// GetResolutionEditor
// Get the resolution editor.
//-----------------------------------------------------------------------------
const HRFResolutionEditor* HRFRasterFile::GetResolutionEditor(unsigned short pi_Resolution)
    {
    ResolutionEditorRegistry::const_iterator Itr(m_ResolutionEditorRegistry.begin());
    const HRFResolutionEditor* pResEditor = 0;
    while (pResEditor == 0 && Itr != m_ResolutionEditorRegistry.end())
        {
        if ((*Itr)->GetResolutionIndex() == pi_Resolution)
            pResEditor = *Itr;
        else
            Itr++;
        }

    return pResEditor;
    }


//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LookAhead section begin
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Private
// Verify if a cancellation is required.  It also removes the blocks that were
// previously requested from the given list
//-----------------------------------------------------------------------------
bool HRFRasterFile::CanCancelLookAhead(uint32_t        pi_Page,
                                        HGFTileIDList&  pi_rRequest,
                                        uint32_t        pi_ConsumerID) const
    {
    HPRECONDITION(pi_Page < CountPages());

    HGFTileIDList::iterator         ListItr;
    const HGFTileIDList*            pBlocks;
    size_t                          PoolSize;
    uint32_t                        BlocksInPool;
    bool                           Result = false;

    if (HasLookAheadByBlock(pi_Page))
        {
        // get the list of blocks for this consumer
        HFCMonitor Monitor(m_ConsumersKey);
        PageConsumerMap::const_iterator PageConsumer(m_PageConsumers.find(pi_Page));
        if (PageConsumer != m_PageConsumers.end())
            {
            ConsumerMap::const_iterator Consumer(PageConsumer->second.find(pi_ConsumerID));
            if (Consumer != PageConsumer->second.end())
                {
                pBlocks = Consumer->second;
                HASSERT(pBlocks != 0);

                // Count the number of blocks in the current consumer pool
                PoolSize = pBlocks->size();

                // If there is less blocks in the pool that it takes to cancel,
                // it is not useful to check all the tiles
                if (PoolSize > ImageppLib::GetHost().GetImageppLibAdmin()._GetLookAheadCancelThreshold())
                    {
                    BlocksInPool = 0;

                    // Parse each block in the given list and check if there in
                    // the pool.  In
                    ListItr = pi_rRequest.begin();
                    while (ListItr != pi_rRequest.end())
                        {
                        // verify if the block is in the consumer list
                        if (find(pBlocks->begin(), pBlocks->end(), *ListItr) != pBlocks->end())
                            {
                            BlocksInPool++;
                            ListItr = pi_rRequest.erase(ListItr);
                            }
                        else
                            ListItr++;
                        }

                    // If the number of blocks in the pool that are not needed exceeds the threshold,
                    // cancel is needed
                    Result = ((PoolSize - BlocksInPool) > ImageppLib::GetHost().GetImageppLibAdmin()._GetLookAheadCancelThreshold());
                    }
                }
            }
        }
    else
        {
        // the LookAhead is supported only by extent, always cancel the current one
        Result = true;
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Private
// Adds new tiles to a consumer
//-----------------------------------------------------------------------------
void HRFRasterFile::AddBlocksToConsumer(uint32_t                pi_Page,
                                        uint32_t                pi_ConsumerID,
                                        const HGFTileIDList&    pi_rBlocks,
                                        bool                   pi_Cancelled)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(pi_rBlocks.size() > 0);

    HFCMonitor Monitor(m_ConsumersKey);
    PageConsumerMap::iterator PageConsumers(m_PageConsumers.find(pi_Page));

    if (PageConsumers != m_PageConsumers.end())
        {
        // find the entry for this consumer
        ConsumerMap::iterator Consumer(PageConsumers->second.find(pi_ConsumerID));

        // The consumer already exists
        if (Consumer != PageConsumers->second.end())
            {
            // if there was a cancellation, we simply replace the tiles
            if (pi_Cancelled)
                {
                // destroy the previous list
                delete Consumer->second;

                // replace it with this new list
                Consumer->second = new HGFTileIDList(pi_rBlocks);
                }

            // Otherwise, we have to merge the two block lists
            else
                {
                // HChk ST   One word can describe this: beurk!
                Consumer->second->insert(Consumer->second->end(), pi_rBlocks.begin(), pi_rBlocks.end());
                Consumer->second->sort();
                Consumer->second->unique();
                }
            }

        // the consumer does not exist
        else
            {
            PageConsumers->second.insert(ConsumerMap::value_type(pi_ConsumerID, new HGFTileIDList(pi_rBlocks)));
            }
        }
    else
        {
        // first consumer for this page
        pair<PageConsumerMap::iterator, bool> InsertPair(m_PageConsumers.insert(PageConsumerMap::value_type(pi_Page, ConsumerMap())));
        HPOSTCONDITION(InsertPair.second);
        InsertPair.first->second.insert(ConsumerMap::value_type(pi_ConsumerID, new HGFTileIDList(pi_rBlocks)));
        }
    }


//-----------------------------------------------------------------------------
// Private
// Removes a block from all the consumers that have it
//-----------------------------------------------------------------------------
void HRFRasterFile::RemoveBlockFromConsumers(uint32_t pi_Page,
                                             uint64_t pi_BlockID)
    {
    HFCMonitor Monitor(m_ConsumersKey);
    PageConsumerMap::iterator PageConsumers(m_PageConsumers.find(pi_Page));

    if (PageConsumers != m_PageConsumers.end())
        {
        // Remove the block from all the consumers
        ConsumerMap::iterator   Consumer(PageConsumers->second.begin());
        HGFTileIDList::iterator ListItr;
        HGFTileIDList*          pBlocks;

        while (Consumer != PageConsumers->second.end())
            {
            // find the block in this list
            pBlocks = Consumer->second;
            if ((ListItr = find(pBlocks->begin(),
                                pBlocks->end(),
                                pi_BlockID)) != pBlocks->end())
                {
                pBlocks->erase(ListItr);
                }

            if (Consumer->second->empty())
                {
                delete Consumer->second;
                Consumer = PageConsumers->second.erase(Consumer);
                }
            else
                Consumer++;
            }

        if (PageConsumers->second.empty())
            m_PageConsumers.erase(PageConsumers);
        }
    }

//-----------------------------------------------------------------------------
// LookAhead section end
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Private
// Removes a block from all the consumers that have it
//-----------------------------------------------------------------------------
uint64_t HRFRasterFile::GetMaxFileSizeInBytes() const
    {
    static const uint64_t sDefaultMaxFileSize = (uint64_t)pow(2.0,32) - 1;

    const HFCPtr<HRFCapability> pMaxFileSizeCapability = GetCapabilities()->GetCapabilityOfType(new HRFMaxFileSizeCapability(GetAccessMode(), 1));

    if (pMaxFileSizeCapability == 0)
        {
        return sDefaultMaxFileSize;
        }
    else
        {
        return ((HRFMaxFileSizeCapability*)pMaxFileSizeCapability.GetPtr())->GetMaxFileSize();
        }
    }

//-----------------------------------------------------------------------------
// Public
// Set the context
//-----------------------------------------------------------------------------
void HRFRasterFile::SetContext(uint32_t                 pi_PageIndex,
                               const HFCPtr<HMDContext>& pi_rpContext)
    {
    pair<ContextMap::iterator, bool> RetVal;
    RetVal = m_Contexts.insert(ContextMap::value_type(pi_PageIndex, pi_rpContext));

    //If the map already contains an element with the given key value,
    //just set the new context on this element
    if (RetVal.second == false)
        {
        RetVal.first->second = pi_rpContext;
        }
    }

//-----------------------------------------------------------------------------
// Public
// Get the context
//-----------------------------------------------------------------------------
HFCPtr<HMDContext> HRFRasterFile::GetContext(uint32_t pi_PageIndex) const
    {
    HFCPtr<HMDContext> pContextFound;
    ContextMap::const_iterator ContextIter = m_Contexts.find(pi_PageIndex);

    if (ContextIter != m_Contexts.end())
        {
        pContextFound = ContextIter->second;
        }

    return pContextFound;
    }

//-----------------------------------------------------------------------------
// Private
// Initialize the vector of default ratio to meter.
//-----------------------------------------------------------------------------
void HRFRasterFile::InitializeDefaultRatioToMeter()
    {
    if (m_DefaultRatioToMeter.size() < CountPages())
        {
        for (unsigned short PageInd = 0; PageInd < CountPages(); PageInd++)
            {
            m_DefaultRatioToMeter.push_back(1.0);
            }
        }
    }

//-----------------------------------------------------------------------------
// Protected
// SetUnitFoundInFile
// Sets if a unit was found in the file or not.
//-----------------------------------------------------------------------------
void HRFRasterFile::SetUnitFoundInFile(bool  pi_UnitFound,
                                       uint32_t pi_Page)
    {
    //This variable should be set initially in increasing page number
    HPRECONDITION((pi_Page < m_DefaultUnitWasFound.size()) ||
                  (pi_Page == m_DefaultUnitWasFound.size()));

    if (pi_Page < m_DefaultUnitWasFound.size())
        {
        m_DefaultUnitWasFound[pi_Page] = pi_UnitFound;
        }
    else
        {
        m_DefaultUnitWasFound.push_back(pi_UnitFound);
        }
    }

//-----------------------------------------------------------------------------
// Public
// IsUnitFoundInFile
// Return true if the units could be found with the geocoding information,
// false otherwise. Note that false should be return for file formats whose
// units is intrinsically defined by the format specification
// (i.e. meter for iTiff, UOR for Intergraph)
//-----------------------------------------------------------------------------
bool HRFRasterFile::IsUnitFoundInFile(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < m_DefaultUnitWasFound.size());

    return m_DefaultUnitWasFound[pi_Page];
    }

//-----------------------------------------------------------------------------
// Public
// Return the pointer of the memory file.
//
// This mehtod must be define by child class
//-----------------------------------------------------------------------------
const HFCMemoryBinStream* HRFRasterFile::GetMemoryFilePtr() const
    {
    HASSERT(0);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRFRasterFile::GetFileStatistics(time_t* pCreated, time_t* pModified, time_t* pAccessed) const
    {
    HFCStat FileStat(GetURL());

    if(pCreated != NULL)
        *pCreated = FileStat.GetCreationTime();

    if(pModified != NULL)
        *pModified = FileStat.GetModificationTime();

    if(pAccessed != NULL)
        *pAccessed = FileStat.GetLastAccessTime();
    }


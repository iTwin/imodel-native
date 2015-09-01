//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFileResBooster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFRasterFileResBooster
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HRFResBoosterEditor.h>
#include <Imagepp/all/h/HRFCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFileResBooster.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>
#include <Imagepp/all/h/HRFLocalCacheFileCreator.h>
#include <Imagepp/all/h/HRFCacheRandomBlockEditor.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFCacheSequentialBlockEditor.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFBilFile.h>

#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecFactory.h>



#define SMALL_IMAGE_SIZE    262144
#define MEDIUM_IMAGE_SIZE   1024000

//-----------------------------------------------------------------------------
// This methods allow to know if the specified raster file need a res booster.
//-----------------------------------------------------------------------------
bool HRFRasterFileResBooster::NeedResBoosterFor(HFCPtr<HRFRasterFile>& pi_rpForRasterFile)
    {
    bool NeedResBooster = false;

    for (uint32_t Page = 0; (Page < pi_rpForRasterFile->CountPages()) && (NeedResBooster == false); Page++)
        {
        HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpForRasterFile->GetPageDescriptor(Page);
        //bool  IsSingleRes    = (pPageDescriptor->CountResolutions() == 1);

        if ((!pPageDescriptor->IsUnlimitedResolution()) &&
            (pPageDescriptor->CountResolutions() == 1) &&
            (pPageDescriptor->GetResolutionDescriptor(0)->GetSizeInBytes() > MEDIUM_IMAGE_SIZE) &&
            (pPageDescriptor->GetResolutionDescriptor(0)->GetBitsPerPixel() != 1) &&
            (pPageDescriptor->GetResolutionDescriptor(0)->GetBlockType() != HRFBlockType::IMAGE))
            {
            NeedResBooster = true;
            }
        }

    return NeedResBooster;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRasterFileResBooster::HRFRasterFileResBooster(HFCPtr<HRFRasterFile>&         pi_rpOriginalFile,
                                                 const HRFCacheFileCreator*     pi_pCreator,
                                                 const WantedResolutionsMap*    pi_pWantedResolutionsMap,
                                                 bool                          pi_AutoErase)

    : HRFRasterFileExtender(pi_rpOriginalFile)
    {
    // The ancestor store the access mode
    m_IsOpen          = false;
    m_pCreator        = pi_pCreator;
    m_AutoErase       = pi_AutoErase;

    // Keep the raster file and Booster
    HPRECONDITION(pi_pCreator != 0);
    m_pBoosterFile = m_pCreator->GetCacheFileFor(m_pOriginalFile);
    HPRECONDITION(m_pBoosterFile != 0);

    HPRECONDITION(m_pBoosterFile->GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, HFC_READ_WRITE) != 0);

    m_pRasterFileCapabilities = new HRFCombinedRasterFileCapabilities(m_pBoosterFile->GetCapabilities(),
                                                                      m_pOriginalFile->GetCapabilities());

    // Empty booster
    if (m_pBoosterFile->CountPages() == 0)
        {
        // First step: we create the cache file for all applications

        // Create the new booster
        Create(pi_pWantedResolutionsMap);

        // set the software cache tags
        HRFLocalCacheFileCreator::SetCacheTags(m_pBoosterFile);

        HFCPtr<HFCURL> pBoosterFileURL(m_pBoosterFile->GetURL());

        // for now, we have only a cTiff
        HPRECONDITION(m_pBoosterFile->IsCompatibleWith(HRFcTiffFile::CLASS_ID));

        // Close the file
        m_pBoosterFile = 0;

        // Next step: we ReOpen the cache for this application
        m_pBoosterFile = HRFcTiffCreator::GetInstance()->Create(pBoosterFileURL,
                                                                HFC_READ_WRITE  | HFC_SHARE_READ_ONLY | HFC_SHARE_WRITE_ONLY);

        ((HFCPtr<HRFcTiffFile>&)m_pBoosterFile)->SetOriginalFileAccessMode(pi_rpOriginalFile->GetAccessMode());

        }

    HASSERT(m_pBoosterFile->CountPages() > 0);
    HASSERT(m_pBoosterFile->CountPages() == m_pOriginalFile->CountPages());

    HRFRasterFileBlockAdapter::BlockDescriptorMap BlockDescMap;
    HRFRasterFileBlockAdapter::BlockDescriptor    BlockDesc;
    for (uint32_t Page = 0; Page < m_pBoosterFile->CountPages(); Page++)
        {
        // if different we adapt original block type to the cache block type
        if (!m_pBoosterFile->GetPageDescriptor(Page)->IsEmpty())
            {
            HFCPtr<HRFResolutionDescriptor> pCacheResolutionDescriptor_1_1  = m_pBoosterFile->GetPageDescriptor(Page)->GetResolutionDescriptor(0);
            HFCPtr<HRFResolutionDescriptor> pSourceResolutionDescriptor_1_1 = m_pOriginalFile->GetPageDescriptor(Page)->GetResolutionDescriptor(0);

            // allow to adapt only when the block size are different
            // ex: if source is IMAGE and the cache is STRIP and we have only one strip
            //     it is not necessary to adapt the source on the cache block
            if ((pSourceResolutionDescriptor_1_1->GetBlockWidth()  != pCacheResolutionDescriptor_1_1->GetBlockWidth()) ||
                (pSourceResolutionDescriptor_1_1->GetBlockHeight() != pCacheResolutionDescriptor_1_1->GetBlockHeight()))
                {
                BlockDesc.m_BlockType   = pCacheResolutionDescriptor_1_1->GetBlockType();
                BlockDesc.m_BlockWidth  = pCacheResolutionDescriptor_1_1->GetBlockWidth();
                BlockDesc.m_BlockHeight = pCacheResolutionDescriptor_1_1->GetBlockHeight();
                BlockDescMap.insert(HRFRasterFileBlockAdapter::BlockDescriptorMap::value_type(Page, BlockDesc));
                }
            }
        }

    if (BlockDescMap.size() > 0 && HRFRasterFileBlockAdapter::CanAdapt(m_pOriginalFile, BlockDescMap))
        m_pOriginalFile = new HRFRasterFileBlockAdapter(m_pOriginalFile, BlockDescMap);

    // Create Page and Res Descriptors.
    m_IsOpen = true;
    CreateDescriptors();
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFRasterFileResBooster::~HRFRasterFileResBooster()
    {
    WString FileName;

    if (m_AutoErase && (m_pBoosterFile != 0))
        {
        if (m_pBoosterFile->GetURL()->GetSchemeType() == HFCURLFile::s_SchemeName())
            {
#ifdef _WIN32
            FileName = (((HFCPtr<HFCURLFile>&)m_pBoosterFile->GetURL())->GetHost());
            FileName += L"\\";
            FileName += ((HFCPtr<HFCURLFile>&)m_pBoosterFile->GetURL())->GetPath();
#endif
            }
        }

    Close();

    // Erase the cache file
    if (m_AutoErase && !FileName.empty())
        {
#ifdef _WIN32
        m_pBoosterFile = 0;
        _wunlink(FileName.c_str());
#endif
        }
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFRasterFileResBooster::CreateResolutionEditor(uint32_t        pi_Page,
                                                                     unsigned short pi_Resolution,
                                                                     HFCAccessMode   pi_AccessMode)
    {
    // Create the original editor and the booster editor
    HRFResolutionEditor* pResolutionEditor         = 0;
    HRFResolutionEditor* pOriginalResolutionEditor = 0;
    HRFResolutionEditor* pBoosterResolutionEditor  = 0;

    if (m_pPageFromOriginalPage[pi_Page])
        pResolutionEditor = m_pOriginalFile->CreateResolutionEditor(pi_Page,
                                                                    pi_Resolution,
                                                                    pi_AccessMode);
    else
        {
        // The true Orignal file
        HFCPtr<HRFRasterFile> pTheTrueOriginalFile = m_pOriginalFile;

        if (m_pOriginalFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
            pTheTrueOriginalFile = ((HFCPtr<HRFRasterFileExtender>&)m_pOriginalFile)->GetOriginalFile();

        // Create an editor on the original file
        if (pi_Resolution < m_pOriginalFile->GetPageDescriptor(pi_Page)->CountResolutions())
            {
            pOriginalResolutionEditor = m_pOriginalFile->CreateResolutionEditor(pi_Page, pi_Resolution, m_pOriginalFile->GetAccessMode());

//HCHK DM  Don't cache the 1:1 if random
#if 1
            // Don't cache the 1:1 if the original file is random and tile
            if ((pi_Resolution == 0) && (pOriginalResolutionEditor->GetResolutionDescriptor()->GetReaderBlockAccess() == HRFBlockAccess::RANDOM) &&
                (pOriginalResolutionEditor->GetResolutionDescriptor()->GetWriterBlockAccess() == HRFBlockAccess::RANDOM) &&
                (pTheTrueOriginalFile->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType() == HRFBlockType::TILE))
                {
                pResolutionEditor = pOriginalResolutionEditor;

                // Set all tiles in the 1:1 to LOADED, because we will read it directly from the original file instead of from the cache file.
                //   Without these lines, you will have an assertion in HRFiTiffTileEditor::WriteBlock, try to rewrite a block already written, when
                //   you try to reopen the cache file.
                //
                HFCPtr<HRFResolutionDescriptor> pResDescriptor = m_pBoosterFile->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution);
                HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
                pBlocksDataFlag = new HRFDataFlag[(size_t)pResDescriptor->CountBlocks()];
                memset(pBlocksDataFlag, HRFDATAFLAG_LOADED, (size_t)(pResDescriptor->CountBlocks() * sizeof(HRFDataFlag)));
                pResDescriptor->SetBlocksDataFlag(pBlocksDataFlag);
                }
            else
#endif
                {
                // Create an editor on the booster file
                pBoosterResolutionEditor = m_pBoosterFile->CreateResolutionEditor(pi_Page, pi_Resolution, m_pBoosterFile->GetAccessMode());

                // We cache with RandomCacheEditor only when the source is tile.
                // Because a large strip is too long to reload.
                if ((pOriginalResolutionEditor->GetResolutionDescriptor()->GetReaderBlockAccess() == HRFBlockAccess::RANDOM) &&
                    (pTheTrueOriginalFile->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType() == HRFBlockType::TILE))
                    {
                    // Create the editor for the specified resolution
                    pResolutionEditor = new HRFCacheRandomBlockEditor(this,
                                                                      pi_Page,
                                                                      pi_Resolution,
                                                                      pi_AccessMode,
                                                                      pOriginalResolutionEditor,
                                                                      pBoosterResolutionEditor);
                    }
                else
                    {
                    // Create the editor for the specified resolution
                    pResolutionEditor = new HRFCacheSequentialBlockEditor(this,
                                                                          pi_Page,
                                                                          pi_Resolution,
                                                                          pi_AccessMode,
                                                                          pOriginalResolutionEditor,
                                                                          pBoosterResolutionEditor);
                    }
                }
            }
        else
            {
            // Create an editor on the booster file
            pBoosterResolutionEditor = m_pBoosterFile->CreateResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);

            // Create the editor for the specified resolution
            pResolutionEditor = new HRFResBoosterEditor(this,
                                                        pi_Page,
                                                        pi_Resolution,
                                                        pi_AccessMode,
                                                        pBoosterResolutionEditor);
            }
        }
    return pResolutionEditor;
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
void HRFRasterFileResBooster::SetAutoErase(bool pi_autoErase)
    {
    m_AutoErase = pi_autoErase;
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
bool HRFRasterFileResBooster::GetAutoErase()const
    {
    return m_AutoErase;
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFRasterFileResBooster::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
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
const HFCPtr<HRFRasterFileCapabilities>& HRFRasterFileResBooster::GetCapabilities () const
    {
    return m_pRasterFileCapabilities;
    }

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFRasterFileResBooster::Open()
    {
    m_IsOpen = true;

    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFRasterFileResBooster::CreateDescriptors ()
    {
    HPRECONDITION (m_IsOpen);

    m_pPageFromOriginalPage = new bool[m_pOriginalFile->CountPages()];

    // Unify descriptors from original file and Booster file
    for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
        {
        HFCPtr<HRFPageDescriptor> pBoosterPageDescriptor  = m_pBoosterFile->GetPageDescriptor(Page);
        HFCPtr<HRFPageDescriptor> pOriginalPageDescriptor = m_pOriginalFile->GetPageDescriptor(Page);

        HRFScanlineOrientation ScanlineOrientation(pOriginalPageDescriptor->GetResolutionDescriptor(0)->GetScanlineOrientation());

        // Add all resolutions descriptor from Booster file to the ResBooster
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (unsigned short Resolution=0; Resolution < pBoosterPageDescriptor->CountResolutions(); Resolution++)
            {
            // DEBUG MODE ONLY
            // JPEG ISO Compression
            // Select the page
#if 0
            HDEBUGCODE(
                HFCPtr<HRFResolutionDescriptor> pResDescriptor = pBoosterPageDescriptor->GetResolutionDescriptor(Resolution);
                if ((pResDescriptor->GetCodec() != 0) &&
                    ((pResDescriptor->GetCodec()->GetClassID() == HCDCodecIJG::CLASS_ID) ||
                     (pResDescriptor->GetCodec()->GetClassID() == HCDCodecFlashpix::CLASS_ID)))
            {
            HASSERT(pResDescriptor->GetBlockWidth() <= 65535);
                HASSERT(pResDescriptor->GetBlockHeight() <= 65535);
                if (pResDescriptor->GetBlockType() == HRFBlockType::TILE)
                    {
                    HASSERT(pResDescriptor->GetBlockWidth() % 16 == 0);
                    HASSERT(pResDescriptor->GetBlockHeight() % 16 == 0);
                    }
                else if (pResDescriptor->CountBlocks() > 1)
                    HASSERT(pResDescriptor->GetBlockHeight() % 16 == 0);
                }
            )
#endif
            HASSERT(pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->HasBlocksDataFlag());
            // Create a copy of this resolution descriptor for the ResBooster
            HFCPtr<HRFResolutionDescriptor> pResolution = new HRFResolutionDescriptor(
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetAccessMode(),       // AccessMode
                GetCapabilities(),                                                                  // Capabilities,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetResolutionXRatio(), // XResolutionRatio,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetResolutionYRatio(), // YResolutionRatio,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetPixelType(),        // PixelType,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetCodec(),            // CodecsList,
                HRFBlockAccess::RANDOM,                                                             // RStorageAccess,
                HRFBlockAccess::RANDOM,                                                             // WStorageAccess,
                ScanlineOrientation,                                                                // ScanLineOrientation,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetInterleaveType(),   // InterleaveType
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->IsInterlace(),         // IsInterlace,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetWidth(),            // Width,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetHeight(),           // Height,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockWidth(),       // BlockWidth,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockHeight(),      // BlockHeight,
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlocksDataFlag(),   // BlocksDataFlag
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockType(),        // Storage Type
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetNumberOfPass(),     // NumberOfPass
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetPaddingBits(),      // PaddingBits
                pBoosterPageDescriptor->GetResolutionDescriptor(Resolution)->GetDownSamplingMethod());  // DownSamplingMethod


            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // combine of Page descriptors information from original file and booster file
        // We prefered the original information
        HFCPtr<HRFPageDescriptor> pPage;
        if (pBoosterPageDescriptor->IsEmpty())
            {
            pPage = new HRFPageDescriptor(*pOriginalPageDescriptor);
            m_pPageFromOriginalPage[Page] = true;
            }
        else
            {
            pPage = new HRFPageDescriptor(pOriginalPageDescriptor->GetAccessMode(),
                                          GetCapabilities(),
                                          pOriginalPageDescriptor,
                                          pBoosterPageDescriptor,
                                          ListOfResolutionDescriptor);
            m_pPageFromOriginalPage[Page] = false;
            }
        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Protected
// This method close the file.
//-----------------------------------------------------------------------------
void HRFRasterFileResBooster::Close()
    {
    SynchronizeFiles();

    // Set the TimeStamp to the new booster file
    // Keep a copy of URL
    HFCPtr<HFCURL>  pOriginalURL = HFCURL::Instanciate(m_pOriginalFile->GetURL()->GetURL());
    HFCPtr<HFCURL>  pBoosterURL  = HFCURL::Instanciate(m_pBoosterFile->GetURL()->GetURL());
    HFCStat BoosterFileStat (m_pBoosterFile->GetURL());
    HFCStat OriginalFileStat(m_pOriginalFile->GetURL());

    m_IsOpen = false;

    // Force to close these raster file
    m_pOriginalFile = 0;
    m_pBoosterFile  = 0;

    // Update the booster modification time with the original file
    BoosterFileStat.SetModificationTime(OriginalFileStat.GetModificationTime());
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Save the decorator and the extended.
//-----------------------------------------------------------------------------
void HRFRasterFileResBooster::Save()
    {
    SynchronizeFiles();
    m_pBoosterFile->Save();
    HRFRasterFileExtender::Save();
    }

//-----------------------------------------------------------------------------
// Private
// SynchronizeFiles
// Synchronize the cached file and the cache file.
//-----------------------------------------------------------------------------
void HRFRasterFileResBooster::SynchronizeFiles()
    {
    // close the ResBooster file
    if (m_IsOpen)
        {
        // Update Descriptors and raster data
        for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
            {
            // Obtain the Page descriptor
            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

            // Update the page Descriptors

            // Update the TransfoModel
            if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                {
                HFCPtr<HRFCapability> pTransfoCapability = new HRFTransfoModelCapability(HFC_WRITE_ONLY, pPageDescriptor->GetTransfoModel()->GetClassID());

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pTransfoCapability))
                    m_pOriginalFile->GetPageDescriptor(Page)->SetTransfoModel(*pPageDescriptor->GetTransfoModel());

                // Update to Cache file
                else if (m_pBoosterFile->GetCapabilities()->Supports(pTransfoCapability))
                    m_pBoosterFile->GetPageDescriptor(Page)->SetTransfoModel(*pPageDescriptor->GetTransfoModel());

                }
            // Update the Filter
            if ((pPageDescriptor->HasFilter()) && (pPageDescriptor->FiltersHasChanged()))
                {
                HFCPtr<HRFCapability> pFilterCapability = new HRFFilterCapability(HFC_WRITE_ONLY, pPageDescriptor->GetFilter().GetClassID());
                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pFilterCapability))
                    m_pOriginalFile->GetPageDescriptor(Page)->SetFilter(pPageDescriptor->GetFilter());

                // Update to Cache file
                else if (m_pBoosterFile->GetCapabilities()->Supports(pFilterCapability))
                    m_pBoosterFile->GetPageDescriptor(Page)->SetFilter(pPageDescriptor->GetFilter());

                }

            // Update the ClipShape
            if ((pPageDescriptor->HasClipShape()) && (pPageDescriptor->ClipShapeHasChanged()))
                {
                HFCPtr<HRFCapability> pShapeCapability = new HRFClipShapeCapability(HFC_WRITE_ONLY, pPageDescriptor->GetClipShape()->GetCoordinateType());

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pShapeCapability))
                    m_pOriginalFile->GetPageDescriptor(Page)->SetClipShape(*pPageDescriptor->GetClipShape());
                // Update to Cache file
                else if (m_pBoosterFile->GetCapabilities()->Supports(pShapeCapability))
                    m_pBoosterFile->GetPageDescriptor(Page)->SetClipShape(*pPageDescriptor->GetClipShape());

                }

            // Update the Histogram
            if ((pPageDescriptor->HasHistogram()) && (pPageDescriptor->HistogramHasChanged()))
                {
                HFCPtr<HRFCapability> pHistogramCapability = new HRFHistogramCapability(HFC_WRITE_ONLY);

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pHistogramCapability))
                    m_pOriginalFile->GetPageDescriptor(Page)->SetHistogram(*pPageDescriptor->GetHistogram());

                // Update to Cache file
                else if (m_pBoosterFile->GetCapabilities()->Supports(pHistogramCapability))
                    m_pBoosterFile->GetPageDescriptor(Page)->SetHistogram(*pPageDescriptor->GetHistogram());
                }

            // Update the Thumbnail
            if ((pPageDescriptor->HasThumbnail()) && (pPageDescriptor->ThumbnailHasChanged()))
                {
                HFCPtr<HRFCapability> pThumbnailCapability = new HRFThumbnailCapability(HFC_WRITE_ONLY);

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pThumbnailCapability))
                    m_pOriginalFile->GetPageDescriptor(Page)->SetThumbnail(*pPageDescriptor->GetThumbnail());

                // Update to Cache file
                else if (m_pBoosterFile->GetCapabilities()->Supports(pThumbnailCapability))
                    m_pBoosterFile->GetPageDescriptor(Page)->SetThumbnail(*pPageDescriptor->GetThumbnail());
                }

            // Update the RepresentativePalette
            if ((pPageDescriptor->HasRepresentativePalette()) && (pPageDescriptor->RepresentativePaletteHasChanged()))
                {
                HFCPtr<HRFCapability> pRepresentativePaletteCapability = new HRFRepresentativePaletteCapability(HFC_WRITE_ONLY);

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pRepresentativePaletteCapability))
                    m_pOriginalFile->GetPageDescriptor(Page)->SetRepresentativePalette(pPageDescriptor->GetRepresentativePalette());
                else if (m_pBoosterFile->GetCapabilities()->Supports(pRepresentativePaletteCapability))
                    m_pBoosterFile->GetPageDescriptor(Page)->SetRepresentativePalette(pPageDescriptor->GetRepresentativePalette());
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
                        m_pOriginalFile->GetPageDescriptor(Page)->SetTag((*TagIterator)->Clone() );

                    // Update to Cache file
                    else if (m_pBoosterFile->GetCapabilities()->Supports(pWriteCapability))
                        m_pBoosterFile->GetPageDescriptor(Page)->SetTag((*TagIterator)->Clone() );
                    }
                }

            pPageDescriptor->Saved();

            // TR #259243
            // on multi page, some pages are not cached (empty page). In this case, the page descriptor
            // is the original page and the data was edited into the original file (check CreateResolutionEditor())
            if (!m_pPageFromOriginalPage[Page])
                {
                // Obtain the Original Page descriptor
                HFCPtr<HRFPageDescriptor> pOriginalPageDescriptor = m_pOriginalFile->GetPageDescriptor(Page);

                // Update original resolution only
                for (unsigned short Resolution=0; Resolution < pOriginalPageDescriptor->CountResolutions(); Resolution++)
                    {
                    // Obtain the resolution descriptor
                    HFCPtr<HRFResolutionDescriptor> pResolution = pPageDescriptor->GetResolutionDescriptor(Resolution);

                    // Flag to know if this resolution must be updated to the Original file
                    bool ToUpdate = false;

                    // if the file is in creation we must update this resolution
                    if (m_pOriginalFile->GetAccessMode().m_HasCreateAccess)
                        {
                        ToUpdate = true;
                        }
                    else
                        {
                        // if the file has write access we check if we want to update this resolution
                        if (m_pOriginalFile->GetAccessMode().m_HasWriteAccess)
                            {
                            // Look if we have an overwritten block
                            // if we have an overwritten block this resolution must updated to the source file
                            uint64_t FlagCount = pResolution->CountBlocks();
                            for (uint64_t FlagIndex=0; FlagIndex < FlagCount; FlagIndex++)
                                {
                                if (pResolution->GetBlockDataFlag(FlagIndex) & HRFDATAFLAG_OVERWRITTEN)
                                    {
                                    ToUpdate        = true;
                                    break;
                                    }
                                }
                            }
                        }

                    // update the data only when overwritten
                    if (ToUpdate)
                        {
                        // Create a buffer to copy the data from the cache to the original resolution
                        HArrayAutoPtr<Byte> pBlockBuffer;

                        pBlockBuffer = new Byte[pResolution->GetBlockSizeInBytes()];

                        // Define an editor on the Source and cache file
                        HAutoPtr<HRFResolutionEditor>   pOriginalResolutionEditor ;
                        HAutoPtr<HRFResolutionEditor>   pBoosterResolutionEditor;

                        // if the original file has sequential writer then complete the cache with the original data
                        if ((m_pOriginalFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWriterBlockAccess() == HRFBlockAccess::SEQUENTIAL) &&
                            (!m_pOriginalFile->GetAccessMode().m_HasCreateAccess))
                            {
                            // We create an editor on the Cache and original file
                            pOriginalResolutionEditor = m_pOriginalFile->CreateResolutionEditor(Page, Resolution, HFC_READ_ONLY);
                            pBoosterResolutionEditor = m_pBoosterFile->CreateResolutionEditor(Page, Resolution, HFC_WRITE_ONLY);

                            // Cache the image Sequentially block by block.
                            for (uint32_t PosY=0; PosY<pResolution->GetHeight(); PosY += pResolution->GetBlockHeight())
                                {
                                for (uint32_t PosX=0; PosX<pResolution->GetWidth(); PosX += pResolution->GetBlockWidth())
                                    {
                                    if ((pResolution->GetBlockDataFlag(pResolution->ComputeBlockIndex(PosX, PosY)) & HRFDATAFLAG_EMPTY))
                                        {
                                        if (pOriginalResolutionEditor->ReadBlock(PosX, PosY, pBlockBuffer) == H_SUCCESS)
                                            pBoosterResolutionEditor->WriteBlock(PosX, PosY, pBlockBuffer);
                                        }
                                    }
                                }
                            }

                        // We create an editor on the Source and cache file
                        pOriginalResolutionEditor = m_pOriginalFile->CreateResolutionEditor(Page, Resolution, HFC_WRITE_ONLY);
                        pBoosterResolutionEditor  = m_pBoosterFile->CreateResolutionEditor(Page, Resolution, HFC_READ_ONLY);


                        // Copy the image Sequentially block by block.
                        for (uint32_t PosY=0; PosY<pResolution->GetHeight(); PosY += pResolution->GetBlockHeight())
                            {
                            for (uint32_t PosX=0; PosX<pResolution->GetWidth(); PosX += pResolution->GetBlockWidth())
                                {
                                if (pBoosterResolutionEditor->ReadBlock(PosX, PosY, pBlockBuffer) == H_SUCCESS)
                                    pOriginalResolutionEditor->WriteBlock(PosX, PosY, pBlockBuffer);
                                }
                            }
                        }
                    }

                // Update Flags to the Booster file
                // Obtain the Booster Page descriptor
                HFCPtr<HRFPageDescriptor> pBoosterPageDescriptor = m_pBoosterFile->GetPageDescriptor(Page);

                // Update original resolution only
                for (unsigned short BoosterResolution=0; BoosterResolution < pBoosterPageDescriptor->CountResolutions(); BoosterResolution++)
                    {
                    // Obtain the resolution descriptor
                    HFCPtr<HRFResolutionDescriptor> pResolution(pPageDescriptor->GetResolutionDescriptor(BoosterResolution));

                    // We change the cache blocks status overwritten to load
                    if (pResolution->BlockDataFlagHasChanged())
                        {
                        uint64_t FlagCount = pResolution->CountBlocks();
                        const HRFDataFlag* pFlags = pResolution->GetBlocksDataFlag();
                        for (uint64_t FlagIndex=0; FlagIndex < FlagCount; FlagIndex++)
                            {
                            if (pResolution->GetBlockDataFlag(FlagIndex) & HRFDATAFLAG_OVERWRITTEN)
                                pResolution->SetBlockDataFlag(FlagIndex,
                                                              (pFlags[FlagIndex] & (0xff-HRFDATAFLAG_OVERWRITTEN)) | HRFDATAFLAG_LOADED);
                            }

                        // Synchronize the flags in to the booster
                        m_pBoosterFile->GetPageDescriptor(Page)->GetResolutionDescriptor(BoosterResolution)->SetBlocksDataFlag(pResolution->GetBlocksDataFlag());
                        // data flag synchronization is done...
                        pResolution->BlockDataFlagSaved();
                        }
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFRasterFileResBooster::Create(const WantedResolutionsMap* pi_pWantedResolutionsMap)
    {
    m_IsOpen = true;

    // Get the first supported scanline oriention from the cache file.
    HFCPtr<HRFScanlineOrientationCapability> pScalineOrientationCapability;
    pScalineOrientationCapability = static_cast<HRFScanlineOrientationCapability*>(m_pBoosterFile->GetCapabilities()->GetCapabilityOfType(HRFScanlineOrientationCapability::CLASS_ID, HFC_READ_ONLY).GetPtr());
    HASSERT(pScalineOrientationCapability != 0);

    const HGFResolutionDescriptor* pWantedResolutions;
    HFCPtr<HRFPageDescriptor> pBoosterPageDescriptor;
    WantedResolutionsMap::const_iterator Itr;

    // Trough each pages and resolutions boost with multi-resolution
    for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
        {
        pWantedResolutions = 0;
        if (pi_pWantedResolutionsMap != 0)
            {
            Itr = pi_pWantedResolutionsMap->find(Page);
            if (Itr != pi_pWantedResolutionsMap->end())
                pWantedResolutions = &Itr->second;
            }

        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        HFCPtr<HRFPageDescriptor> pOriginalPageDescriptor  = m_pOriginalFile->GetPageDescriptor(Page);

        // cache the page, same condition than NeedResBooster()
        if ((!pOriginalPageDescriptor->IsUnlimitedResolution()) &&
            (pOriginalPageDescriptor->CountResolutions() == 1) &&
            (pOriginalPageDescriptor->GetResolutionDescriptor(0)->GetSizeInBytes() > MEDIUM_IMAGE_SIZE) &&
            (pOriginalPageDescriptor->GetResolutionDescriptor(0)->GetBitsPerPixel() != 1) &&
            (pOriginalPageDescriptor->GetResolutionDescriptor(0)->GetBlockType() != HRFBlockType::IMAGE))
            {
            // Here we cache each resolution to the Booster file
            for (unsigned short Resolution=0; Resolution < pOriginalPageDescriptor->CountResolutions(); Resolution++)
                {
                HFCPtr<HRFResolutionDescriptor> pOriginalResDescriptor = pOriginalPageDescriptor->GetResolutionDescriptor(Resolution);

                // Best Match a resolution descriptor for the Booster file
                HRFBlockType BoosterBlockType = pOriginalResDescriptor->GetBlockType();

                // When the source storage type is LINE we force the cache to STRIP of 1 line
                if (BoosterBlockType == HRFBlockType::LINE)
                    BoosterBlockType = HRFBlockType::STRIP;

                // When the source storage type is IMAGE we force the cache to STRIP of image height
                if (BoosterBlockType == HRFBlockType::IMAGE)
                    BoosterBlockType = HRFBlockType::STRIP;

                // Get the default codec for the PixelType
                HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
                pPixelTypeCapability = new HRFPixelTypeCapability(pOriginalResDescriptor->GetAccessMode(),
                                                                  pOriginalResDescriptor->GetPixelType()->GetClassID(),
                                                                  new HRFRasterFileCapabilities());

                pPixelTypeCapability =
                    static_cast<HRFPixelTypeCapability*>(m_pBoosterFile->GetCapabilities()->
                                            GetCapabilityOfType(((HFCPtr<HRFCapability>&)pPixelTypeCapability)).GetPtr());

                HASSERT(pPixelTypeCapability != 0);
                HASSERT(pPixelTypeCapability->CountCodecs() > 0);
               
                // Otherwise we add the default codec found in the raster file creator.
                HFCPtr<HCDCodec> pCodec =
                    HCDCodecFactory::GetInstance().Create(m_pCreator->GetSelectedCodecFor(pPixelTypeCapability->GetPixelTypeClassID()));

                // Set the quality if need.
                if (m_pCreator->CountCompressionStepFor(pPixelTypeCapability->GetPixelTypeClassID()) > 0)
                    {
                    uint32_t SelCompQuality =
                        m_pCreator->GetSelectedCompressionQualityFor(pPixelTypeCapability->GetPixelTypeClassID());

                    if (pCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
                        ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality((Byte)SelCompQuality);
                    else if (pCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
                        ((HFCPtr<HCDCodecFlashpix>&)pCodec)->SetQuality((Byte)SelCompQuality);
                    }

                // DEBUG MODE ONLY
                // JPEG ISO Compression
                // Select the page
#if 0
                HDEBUGCODE(
                    HFCPtr<HRFResolutionDescriptor> pResDescriptor = pOriginalResDescriptor;
                    if ((pCodec != 0) &&
                        ((pCodec->GetClassID() == HCDCodecIJG::CLASS_ID) ||
                         (pCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)))
                {
                HASSERT(pResDescriptor->GetBlockWidth() <= 65535);
                    HASSERT(pResDescriptor->GetBlockHeight() <= 65535);
                    if (pResDescriptor->GetBlockType() == HRFBlockType::TILE)
                        {
                        HASSERT(pResDescriptor->GetBlockWidth() % 16 == 0);
                        HASSERT(pResDescriptor->GetBlockHeight() % 16 == 0);
                        }
                    else if (pResDescriptor->CountBlocks() > 1)
                        HASSERT(pResDescriptor->GetBlockHeight() % 16 == 0);
                    }
                )
#endif

                HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor =
                    new HRFResolutionDescriptor(pOriginalResDescriptor->GetAccessMode(),                    // AccessMode,
                                                m_pBoosterFile->GetCapabilities(),                          // Capabilities,
                                                pOriginalResDescriptor->GetResolutionXRatio(),              // XResolutionRatio,
                                                pOriginalResDescriptor->GetResolutionYRatio(),              // YResolutionRatio,
                                                pOriginalResDescriptor->GetPixelType(),                     // PixelType,
                                                pCodec,                                                     // Codecs,
                                                HRFBlockAccess::RANDOM,                                     // RStorageAccess,
                                                HRFBlockAccess::RANDOM,                                     // WStorageAccess,
                                                pScalineOrientationCapability->GetScanlineOrientation(),    // ScanLineOrientation,
                                                pOriginalResDescriptor->GetInterleaveType(),                // InterleaveType
                                                pOriginalResDescriptor->IsInterlace(),                      // IsInterlace,
                                                pOriginalResDescriptor->GetWidth(),                         // Width,
                                                pOriginalResDescriptor->GetHeight(),                        // Height,
                                                pOriginalResDescriptor->GetBlockWidth(),                    // BlockWidth,
                                                pOriginalResDescriptor->GetBlockHeight(),                   // BlockHeight,
                                                0,                                                          // BlocksDataFlag
                                                BoosterBlockType,                                           // Storage Type
                                                pOriginalResDescriptor->GetNumberOfPass(),                  // NumberOfPass
                                                pOriginalResDescriptor->GetPaddingBits(),                   // PaddingBits
                                                pOriginalResDescriptor->GetDownSamplingMethod());           // DownSamplingMethod

                // We set in creation all flags to empty
                HASSERT(pResolutionDescriptor->CountBlocks() <= SIZE_MAX);
                HASSERT(pResolutionDescriptor->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);

                HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
                pBlocksDataFlag = new HRFDataFlag[(size_t)pResolutionDescriptor->CountBlocks()];
                memset(pBlocksDataFlag,
                       HRFDATAFLAG_EMPTY | HRFDATAFLAG_DIRTYFORSUBRES,
                       (size_t)(pResolutionDescriptor->CountBlocks() * sizeof(HRFDataFlag)));
                pResolutionDescriptor->SetBlocksDataFlag(pBlocksDataFlag);

                // insert the resolution descriptor to the list
                ListOfResolutionDescriptor.push_back(pResolutionDescriptor);
                }

            // Here we add boosted resolutions "not defined in the original file" the Booster file
            HFCPtr<HRFResolutionDescriptor> OriginalResolutionDescriptor1_1 = pOriginalPageDescriptor->GetResolutionDescriptor(0);

            // Check number of bit
            bool Is1Bit = (OriginalResolutionDescriptor1_1->GetPixelType()->CountPixelRawDataBits() == 1);

            // When the source storage type is LINE we force the Booster to STRIP of 1 line
            HRFBlockType BoosterBlockType= OriginalResolutionDescriptor1_1->GetBlockType();

            // When the source storage type is LINE we force the cache to STRIP of 1 line
            if (BoosterBlockType == HRFBlockType::LINE)
                BoosterBlockType = HRFBlockType::STRIP;

            // When the source storage type is IMAGE we force the cache to STRIP of image height
            if (BoosterBlockType == HRFBlockType::IMAGE)
                BoosterBlockType = HRFBlockType::STRIP;

            // Create a resolution descriptor if we didn't receive one...
            HAutoPtr<HGFResolutionDescriptor> pLocalPyramidDescriptor;
            const HGFResolutionDescriptor* pPyramidDescriptor;
            if (pWantedResolutions == 0)
                {
                HASSERT(OriginalResolutionDescriptor1_1->GetWidth() <= ULONG_MAX);
                HASSERT(OriginalResolutionDescriptor1_1->GetHeight() <= ULONG_MAX);

                pLocalPyramidDescriptor = new HGFResolutionDescriptor((uint32_t)OriginalResolutionDescriptor1_1->GetWidth(),
                                                                      (uint32_t)OriginalResolutionDescriptor1_1->GetHeight(),
                                                                      256,
                                                                      256,
                                                                      (BoosterBlockType == HRFBlockType::TILE));
                pPyramidDescriptor = pLocalPyramidDescriptor;
                }
            else
                {
                pPyramidDescriptor = pWantedResolutions;

                // Just make sure that what we received has dimensions that
                // represent the original file...
                HASSERT(pPyramidDescriptor->GetWidth(0) == OriginalResolutionDescriptor1_1->GetWidth() &&
                        pPyramidDescriptor->GetHeight(0) == OriginalResolutionDescriptor1_1->GetHeight());
                }

            // Calc the block size
            uint32_t BlockWidth  = OriginalResolutionDescriptor1_1->GetBlockWidth();
            uint32_t BlockHeight = OriginalResolutionDescriptor1_1->GetBlockHeight();

            // Get the default codec for the PixelType
            HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
            pPixelTypeCapability = new HRFPixelTypeCapability(OriginalResolutionDescriptor1_1->GetAccessMode(),
                                                              OriginalResolutionDescriptor1_1->GetPixelType()->GetClassID(),
                                                              new HRFRasterFileCapabilities());

            pPixelTypeCapability =
                static_cast<HRFPixelTypeCapability*>(m_pBoosterFile->GetCapabilities()->GetCapabilityOfType(((HFCPtr<HRFCapability>&)pPixelTypeCapability)).GetPtr());

            HASSERT(pPixelTypeCapability != 0);
            HASSERT(pPixelTypeCapability->CountCodecs() > 0);


            HFCPtr<HCDCodec> pCodec = HCDCodecFactory::GetInstance().Create(m_pCreator->GetSelectedCodecFor(pPixelTypeCapability->GetPixelTypeClassID()));
            // Set the quality if need.
            if (m_pCreator->CountCompressionStepFor(pPixelTypeCapability->GetPixelTypeClassID()) > 0)
                {
                uint32_t SelCompQuality =
                    m_pCreator->GetSelectedCompressionQualityFor(pPixelTypeCapability->GetPixelTypeClassID());

                if (pCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
                    ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality((Byte)SelCompQuality);
                else if (pCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
                    ((HFCPtr<HCDCodecFlashpix>&)pCodec)->SetQuality((Byte)SelCompQuality);
                }

            if (pOriginalPageDescriptor->CountResolutions() == 1)
                {
                // Special case for 1 bit images, by default
                if (Is1Bit && pWantedResolutions == 0)
                    {
                    // just keep sub res x/4 until a size of 1024x1024
                    for (unsigned short PyramidResolution = 1; PyramidResolution < pPyramidDescriptor->CountResolutions(); PyramidResolution++)
                        {
                        // Obtain the resolution size near of 256x256
                        uint32_t Width  = pPyramidDescriptor->GetWidth(PyramidResolution);
                        uint32_t Height = pPyramidDescriptor->GetHeight(PyramidResolution);

                        // the current resolution is a sub-res by 4 and bigger that 1024x1024
                        if ((PyramidResolution%2 == 0)  &&
                            (Width  > 1024)             &&
                            (Height > 1024)             )
                            {
                            // When the storage type is strip we resize the BlockWidth to the resolution width
                            if (BoosterBlockType == HRFBlockType::STRIP)
                                {
                                BlockWidth = Width;
                                if (BlockHeight > Height)
                                    BlockHeight = Height;
                                }


                            HFCPtr<HRFResolutionDescriptor> pResolution =
                                new HRFResolutionDescriptor(OriginalResolutionDescriptor1_1->GetAccessMode(),           // Access mode,
                                                            m_pBoosterFile->GetCapabilities(),                          // Capabilities,
                                                            pPyramidDescriptor->GetResolution(PyramidResolution),       // XResolutionRatio,
                                                            pPyramidDescriptor->GetResolution(PyramidResolution),       // YResolutionRatio,
                                                            OriginalResolutionDescriptor1_1->GetPixelType(),            // PixelType,
                                                            pCodec,                                                     // Codec
                                                            OriginalResolutionDescriptor1_1->GetReaderBlockAccess(),    // RStorageAccess,
                                                            OriginalResolutionDescriptor1_1->GetWriterBlockAccess(),    // WStorageAccess,
                                                            pScalineOrientationCapability->GetScanlineOrientation(),    // ScanLineOrientation,
                                                            OriginalResolutionDescriptor1_1->GetInterleaveType(),       // InterleaveType
                                                            OriginalResolutionDescriptor1_1->IsInterlace(),             // IsInterlace,
                                                            pPyramidDescriptor->GetWidth(PyramidResolution),            // Width,
                                                            pPyramidDescriptor->GetHeight(PyramidResolution),           // Height,
                                                            BlockWidth,                                                 // BlockWidth,
                                                            BlockHeight,                                                // BlockHeight,
                                                            0,                                                          // BlocksDataFlag
                                                            BoosterBlockType,                                           // BlockType
                                                            OriginalResolutionDescriptor1_1->GetNumberOfPass(),         // NumberOfPass
                                                            OriginalResolutionDescriptor1_1->GetPaddingBits(),          // PaddingBits
                                                            HRFDownSamplingMethod::ORING4);                             // DownSamplingMethod

                            // We set in creation all flags to empty
                            HASSERT(pResolution->CountBlocks() <= SIZE_MAX);
                            HASSERT(pResolution->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);

                            HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
                            pBlocksDataFlag = new HRFDataFlag[(size_t)pResolution->CountBlocks()];
                            memset(pBlocksDataFlag,
                                   HRFDATAFLAG_EMPTY | HRFDATAFLAG_DIRTYFORSUBRES,
                                   (size_t)(pResolution->CountBlocks() * sizeof(HRFDataFlag)));
                            pResolution->SetBlocksDataFlag(pBlocksDataFlag);
                            ListOfResolutionDescriptor.push_back(pResolution);
                            }
                        }
                    }
                else
                    {
                    // Set the default down sampling methode depending of the Pixel Type.
                    HRFDownSamplingMethod DownSamplingMethod;
                    if (OriginalResolutionDescriptor1_1->GetPixelType()->CountIndexBits() == 0)
                        {
                        if (OriginalResolutionDescriptor1_1->GetPixelType()->CountPixelRawDataBits() == 1)
                            DownSamplingMethod = m_pCreator->GetDownSamplingMethodFor1BitPixelType();
                        else
                            DownSamplingMethod = m_pCreator->GetDownSamplingMethodForValuesPixelType();
                        }
                    else
                        DownSamplingMethod = m_pCreator->GetDownSamplingMethodForIndexedPixelType();

                    // Generate all sub-resolutons for N8 bit
                    for (unsigned short PyramidResolution=1; PyramidResolution < pPyramidDescriptor->CountResolutions(); PyramidResolution++)
                        {
                        // When the storage type is strip we resize the BlockWidth to the resolution width
                        if (BoosterBlockType == HRFBlockType::STRIP)
                            {
                            BlockWidth = pPyramidDescriptor->GetWidth(PyramidResolution);

                            if (BlockHeight > pPyramidDescriptor->GetHeight(PyramidResolution))
                                BlockHeight = pPyramidDescriptor->GetHeight(PyramidResolution);
                            }

                        HFCPtr<HRFResolutionDescriptor> pResolution =
                            new HRFResolutionDescriptor(OriginalResolutionDescriptor1_1->GetAccessMode(),           // AccessMode
                                                        m_pBoosterFile->GetCapabilities(),                          // Capabilities,
                                                        pPyramidDescriptor->GetResolution(PyramidResolution),       // ResolutionRatio,
                                                        pPyramidDescriptor->GetResolution(PyramidResolution),       // ResolutionRatio,
                                                        OriginalResolutionDescriptor1_1->GetPixelType(),            // PixelType,
                                                        pCodec,                                                     // Codec
                                                        OriginalResolutionDescriptor1_1->GetReaderBlockAccess(),    // StorageAccess,
                                                        OriginalResolutionDescriptor1_1->GetWriterBlockAccess(),    // StorageAccess,
                                                        pScalineOrientationCapability->GetScanlineOrientation(),    // ScanLineOrientation,
                                                        OriginalResolutionDescriptor1_1->GetInterleaveType(),       // InterleaveType
                                                        OriginalResolutionDescriptor1_1->IsInterlace(),             // IsInterlace,
                                                        pPyramidDescriptor->GetWidth(PyramidResolution),            // Width,
                                                        pPyramidDescriptor->GetHeight(PyramidResolution),           // Height,
                                                        BlockWidth,                                                 // BlockWidth,
                                                        BlockHeight,                                                // BlockHeight,
                                                        0,                                                          // BlocksDataFlag
                                                        BoosterBlockType,                                           // BlockType
                                                        OriginalResolutionDescriptor1_1->GetNumberOfPass(),         // NumberOfPass
                                                        OriginalResolutionDescriptor1_1->GetPaddingBits(),          // PaddingBits
                                                        DownSamplingMethod);                                        // DownSamplingMethod

                        // We set in creation all flags to empty
                        HASSERT(pResolution->CountBlocks() <= SIZE_MAX);
                        HASSERT(pResolution->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);

                        HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
                        pBlocksDataFlag = new HRFDataFlag[(size_t)pResolution->CountBlocks()];
                        memset(pBlocksDataFlag,
                               HRFDATAFLAG_EMPTY | HRFDATAFLAG_DIRTYFORSUBRES,
                               (size_t)(pResolution->CountBlocks() * sizeof(HRFDataFlag)));
                        pResolution->SetBlocksDataFlag(pBlocksDataFlag);
                        ListOfResolutionDescriptor.push_back(pResolution);
                        }
                    }

                // check if we added resolution
                if (ListOfResolutionDescriptor.size() > 1)
                    {
                    // we are in the case that we added at least one resolution
                    // we must set the down sampling method for the first resolution
                    HFCPtr<HRFResolutionDescriptor> pResolution(ListOfResolutionDescriptor[0]);

                    // create a new resolution descriptor with a right down sampling method
                    pResolution =
                        new HRFResolutionDescriptor(pResolution->GetAccessMode(),
                                                    m_pBoosterFile->GetCapabilities(),
                                                    pResolution->GetResolutionXRatio(),
                                                    pResolution->GetResolutionYRatio(),
                                                    pResolution->GetPixelType(),
                                                    pResolution->GetCodec(),
                                                    pResolution->GetReaderBlockAccess(),
                                                    pResolution->GetWriterBlockAccess(),
                                                    pResolution->GetScanlineOrientation(),
                                                    pResolution->GetInterleaveType(),
                                                    pResolution->IsInterlace(),
                                                    pResolution->GetWidth(),
                                                    pResolution->GetHeight(),
                                                    pResolution->GetBlockWidth(),
                                                    pResolution->GetBlockHeight(),
                                                    pResolution->GetBlocksDataFlag(),
                                                    pResolution->GetBlockType(),
                                                    pResolution->GetNumberOfPass(),
                                                    pResolution->GetPaddingBits(),
                                                    ListOfResolutionDescriptor[1]->GetDownSamplingMethod());

                    // replace the first resolution descriptor
                    ListOfResolutionDescriptor[0] = pResolution;
                    }
                }

            // Best match pages and resolutions
            HRPPixelPalette*                  pRepresentativePalette    = 0;
            HRPHistogram*                     pHistogram                = 0;
            HRFThumbnail*                     pThumbnail                = 0;
            HRFClipShape*                     pClipShape                = 0;
            HGF2DTransfoModel*                pTransfoModel             = 0;
            HRPFilter*                        pFilters                  = 0;

            // Best match the TransfoModel
            if (pOriginalPageDescriptor->HasTransfoModel())
                pTransfoModel = pOriginalPageDescriptor->GetTransfoModel();

            // Best match  the Filter
            if (pOriginalPageDescriptor->HasFilter())
                pFilters = (HRPFilter*)&pOriginalPageDescriptor->GetFilter();

            // Best match  the ClipShape
            if (pOriginalPageDescriptor->HasClipShape())
                pClipShape = pOriginalPageDescriptor->GetClipShape();

            // Best match  the Histogram
            if (pOriginalPageDescriptor->HasHistogram())
                pHistogram = pOriginalPageDescriptor->GetHistogram();

            // Best match  the Thumbnail
            if (pOriginalPageDescriptor->HasThumbnail())
                pThumbnail = pOriginalPageDescriptor->GetThumbnail();

            // Best match  the RepresentativePalette
            if (pOriginalPageDescriptor->HasRepresentativePalette())
                pRepresentativePalette = (HRPPixelPalette*)&pOriginalPageDescriptor->GetRepresentativePalette();

            // Cache only supported tags
            HAutoPtr<HPMAttributeSet> pTagList;

            //HCHK yn Atention do not enable it for now
            //        Presently we take in the constructor of HRFPageDescriptor a copy from the source tags
            //        If we enable it we will be have two copies of tags
#if 0
            // Best match each tag.
            HPMAttributeSet::HPMASiterator TagIterator;

            for (TagIterator  = pOriginalPageDescriptor->GetTags().begin();
                 TagIterator != pOriginalPageDescriptor->GetTags().end(); TagIterator++)
                {
                HFCPtr<HPMGenericAttribute> pTag = (*TagIterator)->Clone();
                HFCPtr<HRFCapability>       pTagCapability = new HRFTagCapability(HFC_READ_WRITE, pTag);

                if (pTagList == 0)
                    pTagList = new HPMAttributeSet();
                pTagList->Set(pTag);
                }
#endif

            pBoosterPageDescriptor = new HRFPageDescriptor(m_pBoosterFile->GetAccessMode(),
                                                           m_pBoosterFile->GetCapabilities(),
                                                           ListOfResolutionDescriptor,
                                                           pRepresentativePalette,
                                                           pHistogram,
                                                           pThumbnail,
                                                           pClipShape,
                                                           pTransfoModel,
                                                           pFilters,
                                                           pTagList);
            m_pBoosterFile->AddPage(pBoosterPageDescriptor);
            }
        else
            m_pBoosterFile->AddPage(new HRFPageDescriptor(true)); // add an empty page
        }
    return true;
    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFRasterFileResBooster::GetWorldIdentificator () const
    {
    return m_pOriginalFile->GetWorldIdentificator();
    }

//-----------------------------------------------------------------------------
// public
// GetBoosterFile
// Allow to obtain the booster raster file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileResBooster::GetBoosterFile()
    {
    return (m_pBoosterFile);
    }


//-----------------------------------------------------------------------------
// public
// IsCacheExtender
//
// return true
//-----------------------------------------------------------------------------
bool HRFRasterFileResBooster::IsCacheExtender() const
    {
    return true;
    }


//-----------------------------------------------------------------------------
// public
// IsOriginalRasterDataStorage
//
// return false
//-----------------------------------------------------------------------------
bool HRFRasterFileResBooster::IsOriginalRasterDataStorage() const
    {
    return false;
    }
//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFileCache.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Class : HRFRasterFileCache
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFCacheRandomBlockEditor.h>
#include <Imagepp/all/h/HRFCacheSequentialBlockEditor.h>
#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HCDCodecFactory.h>
#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HRFLocalCacheFileCreator.h>

#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFcTiffFile.h>


//-----------------------------------------------------------------------------
// HMR Message Map
//-----------------------------------------------------------------------------
HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRFRasterFileCache, HRFRasterFile, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRFRasterFileCache, HRFProgressImageChangedMsg, NotifyProgressImageChanged)
HMG_END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// This methods allow to know if the specified raster file need a cache.
//-----------------------------------------------------------------------------
bool HRFRasterFileCache::NeedCacheFor(HFCPtr<HRFRasterFile>& pi_rpForRasterFile)
    {
    bool NeedCache = false;

    if (pi_rpForRasterFile->CountPages() > 0 && !pi_rpForRasterFile->GetPageDescriptor(0)->IsUnlimitedResolution())
        {
        for (uint32_t Page = 0; (Page < pi_rpForRasterFile->CountPages()) && (NeedCache == false); Page++)
            {
            HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpForRasterFile->GetPageDescriptor(Page);

            HASSERT(!pPageDescriptor->IsUnlimitedResolution());
            uint32_t ResCount = pPageDescriptor->CountResolutions();
            for (unsigned short Resolution = 0; (Resolution < ResCount) && (NeedCache == false); Resolution++)
                {
                if ((pPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockType() != HRFBlockType::IMAGE) &&
                    ((pPageDescriptor->GetResolutionDescriptor(Resolution)->GetReaderBlockAccess() == HRFBlockAccess::SEQUENTIAL) ||
                     (pPageDescriptor->GetResolutionDescriptor(Resolution)->GetWriterBlockAccess() == HRFBlockAccess::SEQUENTIAL)))
                    NeedCache = true;
                }
            }
        }

    return NeedCache;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFRasterFileCache::HRFRasterFileCache(const HFCPtr<HRFRasterFile>&     pi_rpSource,
                                       const HRFCacheFileCreator*       pi_pCreator,
                                       bool                             pi_AutoErase,
                                       uint32_t                        pi_Page)
    : HRFRasterFileExtender(pi_rpSource),
      m_pCreator(pi_pCreator),
      m_OriginalPageID(pi_Page),
      m_CacheInvalidate(false)
    {
    HPRECONDITION(pi_rpSource != 0);
    HPRECONDITION(pi_pCreator != 0);
    HPRECONDITION(pi_Page == -1 || (pi_Page >= 0 && pi_Page < pi_rpSource->CountPages()));


    m_pCache = m_pCreator->GetCacheFileFor(m_pOriginalFile, pi_Page);
    HPRECONDITION(m_pCache != 0);

    HPRECONDITION(m_pCache->GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, m_pCache->GetAccessMode()) != 0);
    HFCMonitor Monitor(GetKey());

    m_AutoErase = pi_AutoErase;

    // Link on the source
    LinkTo(m_pOriginalFile);

    // Cache is empty
    if (m_pCache->CountPages() == 0)
        {
        // First step: we create the cache file for all applications

        // Build the unified capabilities
        BuildCapabilities();

        // Build the cache descriptors
        if (m_OriginalPageID != -1)
            {
            // cache the specific page
            BuildDescriptors(m_OriginalPageID);
            }
        else
            {
            // cache all pages
            for (uint32_t Page = 0; Page < m_pOriginalFile->CountPages(); Page++)
                BuildDescriptors(Page);
            }

        // set the software cache tags
        HRFLocalCacheFileCreator::SetCacheTags(m_pCache);

        HFCPtr<HFCURL> pCacheURL(m_pCache->GetURL());

        // for now, we have only a cTiff
        HPRECONDITION(m_pCache->IsCompatibleWith(HRFcTiffFile::CLASS_ID));

        // Close the file
        m_pCache = 0;

        // Next step: we ReOpen the cache for this application
        m_pCache = HRFcTiffCreator::GetInstance()->Create(pCacheURL,
                                                          HFC_READ_WRITE  | HFC_SHARE_READ_ONLY | HFC_SHARE_WRITE_ONLY);

        ((HFCPtr<HRFcTiffFile>&)m_pCache)->SetOriginalFileAccessMode(m_pOriginalFile->GetAccessMode());
        }

    HASSERT(m_pCache->CountPages() > 0);
    // we cache all pages or only a specific page
    HASSERT((pi_Page == -1 && m_pCache->CountPages() == m_pOriginalFile->CountPages()) || m_pCache->CountPages() == 1);

    HRFRasterFileBlockAdapter::BlockDescriptor BlockDesc;
    HRFRasterFileBlockAdapter::BlockDescriptorMap BlockDescMap;

    if (m_OriginalPageID != -1)
        {
        // cache a specific page
        if (AdaptBlockForPage(m_pOriginalFile->GetPageDescriptor(m_OriginalPageID), m_pCache->GetPageDescriptor(0),&BlockDesc))
            BlockDescMap.insert(HRFRasterFileBlockAdapter::BlockDescriptorMap::value_type(m_OriginalPageID, BlockDesc));
        }
    else
        {
        // cache all page
        for (uint32_t Page = 0; Page < m_pOriginalFile->CountPages(); Page++)
            {
            if (AdaptBlockForPage(m_pOriginalFile->GetPageDescriptor(Page),m_pCache->GetPageDescriptor(Page), &BlockDesc))
                BlockDescMap.insert(HRFRasterFileBlockAdapter::BlockDescriptorMap::value_type(Page, BlockDesc));
            }
        }

    if (BlockDescMap.size() > 0 && HRFRasterFileBlockAdapter::CanAdapt(m_pOriginalFile, BlockDescMap))
        {
        m_pOriginalFile = new HRFRasterFileBlockAdapter(m_pOriginalFile, BlockDescMap);
        }

    // Build the unified capabilities
    BuildCapabilities();

    if (m_OriginalPageID != -1)
        {
        m_ListOfPageDescriptor.push_back(BuildPageDescriptor(m_pOriginalFile->GetPageDescriptor(m_OriginalPageID),
                                                             m_pCache->GetPageDescriptor(0)));
        }
    else
        {
        // cache all pages
        for (uint32_t Page = 0; Page < m_pOriginalFile->CountPages(); Page++)
            m_ListOfPageDescriptor.push_back(BuildPageDescriptor(m_pOriginalFile->GetPageDescriptor(Page),
                                                                 m_pCache->GetPageDescriptor(Page)));
        }

    // Specify the FileCache is Open...
    m_IsOpen = true;
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFRasterFileCache::~HRFRasterFileCache()
    {
    HFCMonitor Monitor(GetKey());

    // Unlink from the source
    UnlinkFrom(m_pOriginalFile);

    // The raster file close will kill it's internal key, so release it now.
    Monitor.ReleaseKey();

    WString FileName;
    // Erase the cache file
    if (m_AutoErase)
        {
        HASSERT(m_pCache != 0);

        if (m_pCache->GetURL()->GetSchemeType() == HFCURLFile::s_SchemeName())
            {
#ifdef _WIN32
            FileName = ((HFCPtr<HFCURLFile>&)m_pCache->GetURL())->GetHost();
            FileName += L"\\";
            FileName += ((HFCPtr<HFCURLFile>&)m_pCache->GetURL())->GetPath();
#endif
            }
        }

    try
        {
        Close();
        }
    catch (...)
        {
        }

    // Erase the cache file
    if (m_AutoErase && !FileName.empty())
        {
#ifdef _WIN32
        m_pCache = 0;
        _wunlink(FileName.c_str());
#endif
        }
    }


//-----------------------------------------------------------------------------
// Protected
// This method close the file.
//-----------------------------------------------------------------------------
void HRFRasterFileCache::Close()
    {
    SynchronizeFiles();

    // Keep a copy of URL
    HFCPtr<HFCURL>  pSrcURL   = HFCURL::Instanciate(m_pOriginalFile->GetURL()->GetURL());
    HFCPtr<HFCURL>  pCacheURL = HFCURL::Instanciate(m_pCache->GetURL()->GetURL());
    HFCStat CacheFileStat   (pCacheURL);
    HFCStat OriginalFileStat(pSrcURL);

    m_IsOpen = false;

    // Force to close these raster file
    m_pOriginalFile  = 0;
    m_pCache         = 0;

    // Update the booster modification time with the original file
    CacheFileStat.SetModificationTime(OriginalFileStat.GetModificationTime());
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Save the decorator and the extended.
//-----------------------------------------------------------------------------
void HRFRasterFileCache::Save()
    {
    SynchronizeFiles();
    m_pCache->Save();
    HRFRasterFileExtender::Save();
    }

//-----------------------------------------------------------------------------
// Private
// SynchronizeFiles
// Synchronize the cached file and the cache file.
//-----------------------------------------------------------------------------
void HRFRasterFileCache::SynchronizeFiles()
    {
    // close the cache file
    if (m_IsOpen)
        {
        HFCPtr<HRFPageDescriptor> pOriginalPage;
        uint32_t                  OriginalPageID;
        HFCPtr<HRFPageDescriptor> pCachedPage;
        // Update Descriptors and raster data
        for (uint32_t Page = 0; Page < CountPages(); Page++)
            {
            pCachedPage = m_pCache->GetPageDescriptor(Page);

            // if all pages are cached, OrignalPage == Page
            if (m_OriginalPageID == -1)
                {
                pOriginalPage = m_pOriginalFile->GetPageDescriptor(Page);
                OriginalPageID = Page;
                }
            else
                {
                // we have cached a specific page
                HASSERT(CountPages() == 1);
                pOriginalPage = m_pOriginalFile->GetPageDescriptor(m_OriginalPageID);
                OriginalPageID = m_OriginalPageID;
                }

            // Obtain the Page descriptor
            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);
            bool PageCached = !pCachedPage->IsEmpty();

            // Update the page Descriptors

            // Update the TransfoModel
            if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                {
                HFCPtr<HRFCapability> pTransfoCapability = new HRFTransfoModelCapability(HFC_WRITE_ONLY, pPageDescriptor->GetTransfoModel()->GetClassID());

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pTransfoCapability))
                    pOriginalPage->SetTransfoModel(*pPageDescriptor->GetTransfoModel());
                else if (PageCached && m_pCache->GetCapabilities()->Supports(pTransfoCapability))  // Update to Cache file
                    pCachedPage->SetTransfoModel(*pPageDescriptor->GetTransfoModel());

                }
            // Update the Filter
            if ((pPageDescriptor->HasFilter()) && (pPageDescriptor->FiltersHasChanged()))
                {
                HFCPtr<HRFCapability> pFilterCapability = new HRFFilterCapability(HFC_WRITE_ONLY, pPageDescriptor->GetFilter().GetClassID());
                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pFilterCapability))
                    pOriginalPage->SetFilter(pPageDescriptor->GetFilter());
                else if (PageCached && m_pCache->GetCapabilities()->Supports(pFilterCapability))  // Update to Cache file
                    pCachedPage->SetFilter(pPageDescriptor->GetFilter());
                }

            // Update the ClipShape
            if ((pPageDescriptor->HasClipShape()) && (pPageDescriptor->ClipShapeHasChanged()))
                {
                HFCPtr<HRFCapability> pShapeCapability = new HRFClipShapeCapability(HFC_WRITE_ONLY, pPageDescriptor->GetClipShape()->GetCoordinateType());

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pShapeCapability))
                    pOriginalPage->SetClipShape(*pPageDescriptor->GetClipShape());
                else if (PageCached && m_pCache->GetCapabilities()->Supports(pShapeCapability))  // Update to Cache file
                    pCachedPage->SetClipShape(*pPageDescriptor->GetClipShape());
                }

            // Update the Histogram
            if ((pPageDescriptor->HasHistogram()) && (pPageDescriptor->HistogramHasChanged()))
                {
                HFCPtr<HRFCapability> pHistogramCapability = new HRFHistogramCapability(HFC_WRITE_ONLY);

                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pHistogramCapability))
                    pOriginalPage->SetHistogram(*pPageDescriptor->GetHistogram());
                else if (PageCached && m_pCache->GetCapabilities()->Supports(pHistogramCapability))   // Update to Cache file
                    pCachedPage->SetHistogram(*pPageDescriptor->GetHistogram());
                }

            // Update the Thumbnail
            if ((pPageDescriptor->HasThumbnail()) && (pPageDescriptor->ThumbnailHasChanged()))
                {
                HFCPtr<HRFCapability> pThumbnailCapability = new HRFThumbnailCapability(HFC_WRITE_ONLY);
                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pThumbnailCapability))
                    pOriginalPage->SetThumbnail(*pPageDescriptor->GetThumbnail());
                else if (PageCached && m_pCache->GetCapabilities()->Supports(pThumbnailCapability))  // Update to Cache file
                    pCachedPage->SetThumbnail(*pPageDescriptor->GetThumbnail());
                }

            // Update the RepresentativePalette
            if ((pPageDescriptor->HasRepresentativePalette()) && (pPageDescriptor->RepresentativePaletteHasChanged()))
                {
                HFCPtr<HRFCapability> pRepresentativePaletteCapability = new HRFRepresentativePaletteCapability(HFC_WRITE_ONLY);
                // Update to Source file
                if (m_pOriginalFile->GetCapabilities()->Supports(pRepresentativePaletteCapability))
                    pOriginalPage->SetRepresentativePalette(pPageDescriptor->GetRepresentativePalette());
                else if (PageCached && m_pCache->GetCapabilities()->Supports(pRepresentativePaletteCapability))  // Update to Cache file
                    pCachedPage->SetRepresentativePalette(pPageDescriptor->GetRepresentativePalette());
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
                        pOriginalPage->SetTag((*TagIterator)->Clone() );
                    else if (PageCached && m_pCache->GetCapabilities()->Supports(pWriteCapability))  // Update to Cache file
                        pCachedPage->SetTag((*TagIterator)->Clone() );
                    }
                }

            if (PageCached)
                {
                //TR 109180 (we need to copy all the source data to the cache, before to begin to rewrite in the source
                HArrayAutoPtr<bool> aToUpdate(new bool[pPageDescriptor->CountResolutions()]);
                for (unsigned short Resolution=0; Resolution < pPageDescriptor->CountResolutions(); Resolution++)
                    {
                    // Obtain the resolution descriptor
                    HFCPtr<HRFResolutionDescriptor> pResolution = pPageDescriptor->GetResolutionDescriptor(Resolution);

                    // Flag to know if this resolution must be updated to the source file
                    aToUpdate[Resolution] = false;

                    // if the file is in creation the we must update this resolution
                    if (m_pOriginalFile->GetAccessMode().m_HasCreateAccess)
                        aToUpdate[Resolution] = true;
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
                                    aToUpdate[Resolution] = true;
                                    break;
                                    }
                                }
                            }
                        }

                    // update the data only when overwritten
                    if (aToUpdate[Resolution])
                        {
                        // Create a buffer to copy the data from the cache to the source
                        HArrayAutoPtr<Byte> pBlockBuffer;
                        pBlockBuffer = new Byte[pResolution->GetBlockSizeInBytes()];

                        // Define an editor on the Source and cache file
                        HAutoPtr<HRFResolutionEditor>   pSourceResolutionEditor ;
                        HAutoPtr<HRFResolutionEditor>   pCacheResolutionEditor;

                        // if the original file has sequential writer then complete the cache with the original data
                        if ((pOriginalPage->GetResolutionDescriptor(0)->GetWriterBlockAccess() == HRFBlockAccess::SEQUENTIAL) &&
                            (!m_pOriginalFile->GetAccessMode().m_HasCreateAccess))
                            {
                            // We create an editor on the Cache and original file
                            pSourceResolutionEditor = m_pOriginalFile->CreateResolutionEditor(OriginalPageID, Resolution, HFC_READ_ONLY);
                            pCacheResolutionEditor = m_pCache->CreateResolutionEditor(Page, Resolution, HFC_WRITE_ONLY);

                            // Cache the image Sequentially block by block.
                            for (uint32_t PosY=0; PosY<pResolution->GetHeight(); PosY += pResolution->GetBlockHeight())
                                {
                                for (uint32_t PosX=0; PosX<pResolution->GetWidth(); PosX += pResolution->GetBlockWidth())
                                    {
                                    if ((pResolution->GetBlockDataFlag(pResolution->ComputeBlockIndex(PosX, PosY)) & HRFDATAFLAG_EMPTY))
                                        {
                                        if (pSourceResolutionEditor->ReadBlock(PosX, PosY, pBlockBuffer) == H_SUCCESS)
                                            pCacheResolutionEditor->WriteBlock(PosX, PosY, pBlockBuffer);
                                        }
                                    }
                                }
                            }
                        }
                    }

                if (m_CacheInvalidate)
                    {
                    for (unsigned short Resolution=0; Resolution < pCachedPage->CountResolutions(); Resolution++)
                        {
                        // Obtain the resolution descriptor
                        HFCPtr<HRFResolutionDescriptor> pCacheResolutionDescriptor = pCachedPage->GetResolutionDescriptor(Resolution);

                        uint64_t FlagCount = pCacheResolutionDescriptor->CountBlocks();

                        for (uint64_t FlagIndex=0; FlagIndex < FlagCount; FlagIndex++)
                            {
                            pCacheResolutionDescriptor->SetBlockDataFlag(FlagIndex, HRFDATAFLAG_EMPTY);
                            }
                        }
                    }
                else
                    {
                    for (unsigned short Resolution=0; Resolution < pPageDescriptor->CountResolutions() && aToUpdate[Resolution]; Resolution++)
                        {
                        // Obtain the resolution descriptor
                        HFCPtr<HRFResolutionDescriptor> pResolution = pPageDescriptor->GetResolutionDescriptor(Resolution);

                        // Create a buffer to copy the data from the cache to the source
                        HArrayAutoPtr<Byte> pBlockBuffer;
                        pBlockBuffer = new Byte[pResolution->GetBlockSizeInBytes()];

                        // Define an editor on the Source and cache file
                        HAutoPtr<HRFResolutionEditor>   pSourceResolutionEditor;
                        HAutoPtr<HRFResolutionEditor>   pCacheResolutionEditor;

                        // We create an editor on the Source and cache file
                        pSourceResolutionEditor = m_pOriginalFile->CreateResolutionEditor(OriginalPageID, Resolution, HFC_WRITE_ONLY);
                        pCacheResolutionEditor  = m_pCache->CreateResolutionEditor(Page, Resolution, HFC_READ_ONLY);

                        // Copy the image Sequentially block by block.
                        for (uint32_t PosY=0; PosY<pResolution->GetHeight(); PosY += pResolution->GetBlockHeight())
                            {
                            for (uint32_t PosX=0; PosX<pResolution->GetWidth(); PosX += pResolution->GetBlockWidth())
                                {
                                if (pCacheResolutionEditor->ReadBlock(PosX, PosY, pBlockBuffer) == H_SUCCESS)
                                    pSourceResolutionEditor->WriteBlock(PosX, PosY, pBlockBuffer);
                                }
                            }


                        // We Update the flag to the cache
                        HFCPtr<HRFResolutionDescriptor> pCacheResolutionDescriptor = pCachedPage->GetResolutionDescriptor(Resolution);
                        pCacheResolutionDescriptor->SetBlocksDataFlag(pResolution->GetBlocksDataFlag());

                        // We change the cache blocks status overwritten to load
                        uint64_t FlagCount = pCacheResolutionDescriptor->CountBlocks();
                        const HRFDataFlag* pFlags = pCacheResolutionDescriptor->GetBlocksDataFlag();

                        for (uint64_t FlagIndex=0; FlagIndex < FlagCount; FlagIndex++)
                            {
                            if (pCacheResolutionDescriptor->GetBlockDataFlag(FlagIndex) & HRFDATAFLAG_OVERWRITTEN)
                                pCacheResolutionDescriptor->SetBlockDataFlag(FlagIndex,
                                                                             (pFlags[FlagIndex] & (0xff-HRFDATAFLAG_OVERWRITTEN)) | HRFDATAFLAG_LOADED);
                            }
                        }
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// Public
// Obtain capabilities of the raster file
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFRasterFileCache::GetCapabilities() const
    {
    HFCMonitor Monitor(GetKey());

    return (m_CombinedRasterFileCapabilities);
    }

//-----------------------------------------------------------------------------
// Public
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFRasterFileCache::GetWorldIdentificator() const
    {
    HFCMonitor Monitor(GetKey());

    return (m_pOriginalFile->GetWorldIdentificator());
    }

//-----------------------------------------------------------------------------
// Public
// File manipulation
//-----------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> HRFRasterFileCache::GetPageDescriptor(uint32_t pi_Page) const
    {
    if (m_OriginalPageID != -1)
        {
        HPRECONDITION(CountPages() == 1 && (pi_Page == m_OriginalPageID || pi_Page == 0));
        return HRFRasterFile::GetPageDescriptor(0);
        }
    else
        return HRFRasterFile::GetPageDescriptor(pi_Page);
    }


//-----------------------------------------------------------------------------
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFRasterFileCache::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HFCMonitor Monitor(GetKey());

    return (false);
    }

//-----------------------------------------------------------------------------
// Public
// Editor creation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFRasterFileCache::CreateResolutionEditor(uint32_t      pi_Page,
                                                                unsigned short pi_Resolution,
                                                                HFCAccessMode pi_AccessMode)
    {
    HPRECONDITION((m_OriginalPageID == -1 &&  pi_Page < CountPages()) || pi_Page == m_OriginalPageID);

    HFCMonitor Monitor(GetKey());

    // If the source file is not in creation mode
    // We map this resolution with the decorator editor
    // The decorator editor cache the raster data
    // Otherwise we map the editor to the decoration editor

    // Create the Source editor and the Cache editor
    HRFResolutionEditor* pResolutionEditor       = 0;

    // The true Orignal file
    HFCPtr<HRFRasterFile> pOriginalFile = m_pOriginalFile;

    if (m_pOriginalFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        pOriginalFile = ((HFCPtr<HRFRasterFileExtender>&)m_pOriginalFile)->GetOriginalFile();

    // if the resolution of the source file is random, return the source editor

    uint32_t CachePage;
    uint32_t OriginalPage;
    if (m_OriginalPageID != -1)
        {
        // we have cached a specific page
        OriginalPage = m_OriginalPageID;
        CachePage = 0;
        }
    else
        {
        OriginalPage = pi_Page;
        CachePage = pi_Page;
        }

    if (m_pCache->GetPageDescriptor(CachePage)->IsEmpty())
        {
        // this page is not cached, create an editor on the source
        pResolutionEditor = m_pOriginalFile->CreateResolutionEditor(OriginalPage, pi_Resolution, m_pOriginalFile->GetAccessMode());
        }
    else
        {
        HRFResolutionEditor* pSourceResolutionEditor = 0;
        HRFResolutionEditor* pCacheResolutionEditor  = 0;

        // Create an editor on the Source file
        pSourceResolutionEditor = m_pOriginalFile->CreateResolutionEditor(OriginalPage, pi_Resolution, m_pOriginalFile->GetAccessMode());
        // Create an editor on the Cache file, the page was cached in first page
        pCacheResolutionEditor = m_pCache->CreateResolutionEditor(CachePage, pi_Resolution, m_pCache->GetAccessMode());

        // We cache with RandomCacheEditor only when the source is tile.
        // Because a large strip is too long to reload.
        if ((pSourceResolutionEditor->GetResolutionDescriptor()->GetReaderBlockAccess() == HRFBlockAccess::RANDOM) &&
            (pOriginalFile->GetPageDescriptor(OriginalPage)->GetResolutionDescriptor(pi_Resolution)->GetBlockType() == HRFBlockType::TILE))
            {
            // Create the editor for the specified resolution
            pResolutionEditor = new HRFCacheRandomBlockEditor(this,
                                                              CachePage,
                                                              pi_Resolution,
                                                              pi_AccessMode,
                                                              pSourceResolutionEditor,
                                                              pCacheResolutionEditor);
            }
        else
            {
            // Create the editor for the specified resolution
            pResolutionEditor = new HRFCacheSequentialBlockEditor(this,
                                                                  CachePage,
                                                                  pi_Resolution,
                                                                  pi_AccessMode,
                                                                  pSourceResolutionEditor,
                                                                  pCacheResolutionEditor);
            }
        }

    return pResolutionEditor;
    }


//------------------------------------------------------------------------------
// Public
// Indicates if the file supports LookAhead optimization
//------------------------------------------------------------------------------
bool HRFRasterFileCache::HasLookAheadByBlock(uint32_t pi_Page) const
    {
    return m_pOriginalFile->HasLookAheadByBlock(pi_Page);
    }

//------------------------------------------------------------------------------
// Public
// Indicates if the file supports LookAhead optimization
//------------------------------------------------------------------------------
bool HRFRasterFileCache::HasLookAheadByExtent(uint32_t pi_Page) const
    {
    return m_pOriginalFile->HasLookAheadByExtent(pi_Page);
    }

//------------------------------------------------------------------------------
// Public
// This method indicates if the source is ready to receive LookAhead notifications
//------------------------------------------------------------------------------
bool HRFRasterFileCache::CanPerformLookAhead (uint32_t pi_Page) const
    {
    return m_pOriginalFile->CanPerformLookAhead(pi_Page);
    }


//------------------------------------------------------------------------------
// Public
// Sets the LookAhead for a whole extent
//------------------------------------------------------------------------------
void HRFRasterFileCache::SetLookAhead(uint32_t               pi_Page,
                                      const HGFTileIDList&   pi_rBlocks,
                                      uint32_t               pi_ConsumerID,
                                      bool                  pi_Async)

    {
    HPRECONDITION(HasLookAhead(pi_Page));
    HPRECONDITION(CanPerformLookAhead(pi_Page));

    HFCMonitor                      Monitor(GetKey());
    HGFTileIDList                   MissingBlockList;
    HGFTileIDList                   AvailableBlockList;
    HGFTileIDList::const_iterator   Itr;
    uint32_t                        TileResolution;
    uint64_t                       TileIndex;
    HFCPtr<HRFResolutionDescriptor> pRes;

    // parse each block in the given list and create a list that filtered
    // out the blocks already available from the cache
    for (Itr = pi_rBlocks.begin(); Itr != pi_rBlocks.end(); Itr++)
        {
        // Extract the resolution and the index from the ID
        TileResolution = s_TileDescriptor.GetLevel((*Itr));
        TileIndex      = s_TileDescriptor.GetIndex((*Itr));

        // Get the res descriptor for that res
        HASSERT(pi_Page < CountPages());
        HASSERT(TileResolution < GetPageDescriptor(pi_Page)->CountResolutions());
        pRes = GetPageDescriptor(pi_Page)->GetResolutionDescriptor((unsigned short)TileResolution);

        // Verify the data flag of the current tile.  If it is empty, add it
        // to the block list to request from the source
        if (pRes->GetBlockDataFlag(TileIndex) & HRFDATAFLAG_EMPTY)
            MissingBlockList.push_back(*Itr);
        else
            AvailableBlockList.push_back(*Itr);
        }

    // send the request to the source
    Monitor.ReleaseKey();
    if (MissingBlockList.size() > 0)
        m_pOriginalFile->SetLookAhead(pi_Page, MissingBlockList, pi_ConsumerID, pi_Async);

    // send notifications for the available blocks
    for (Itr = AvailableBlockList.begin(); Itr != AvailableBlockList.end(); Itr++)
        NotifyBlockReady(pi_Page, *Itr);
    }

//------------------------------------------------------------------------------
// Public
// Sets the LookAhead for a whole extent
//------------------------------------------------------------------------------
void HRFRasterFileCache::SetLookAhead(uint32_t           pi_Page,
                                      unsigned short    pi_Resolution,
                                      const HVEShape&    pi_rShape,
                                      uint32_t           pi_ConsumerID,
                                      bool              pi_Async)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());

    HFCPtr<HRFResolutionDescriptor> pRes(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution));
    HGFTileIDList AvailableBlockList;
    HGFTileIDList MissingBlockList;

    // Extract the needed blocks from the region
    SnapRegionToGrid(pi_Page, pi_Resolution, pi_rShape, &AvailableBlockList);

    HGFTileIDList::iterator Itr(AvailableBlockList.begin());
    while (Itr != AvailableBlockList.end())
        {
        if (pRes->GetBlockDataFlag(s_TileDescriptor.GetIndex(*Itr)) & HRFDATAFLAG_EMPTY)
            {
            // the block is not in the cache, add it in the MissingBlockList and remove it from AvailableBlockList
            MissingBlockList.push_back(*Itr);
            Itr = AvailableBlockList.erase(Itr);
            }
        else
            Itr++;
        }

    // call look a head for missing tiles
    if (!MissingBlockList.empty())
        m_pOriginalFile->SetLookAhead(pi_Page, MissingBlockList, pi_ConsumerID, pi_Async);

    // notify tiles already available
    Itr = AvailableBlockList.begin();
    while (Itr != AvailableBlockList.end())
        {
        NotifyBlockReady(pi_Page, *Itr);
        Itr++;
        }
    }

//-----------------------------------------------------------------------------
// Public
// When the source notifies us that a tile has arrived
//-----------------------------------------------------------------------------
bool HRFRasterFileCache::NotifyProgressImageChanged (const HMGMessage& pi_rMessage)
    {
    HPRECONDITION(pi_rMessage.GetClassID() == HRFProgressImageChangedMsg::CLASS_ID);
    HFCMonitor Monitor(GetKey());
    HRFProgressImageChangedMsg& rMessage = (HRFProgressImageChangedMsg&)pi_rMessage;

    // Convert the message
    uint32_t Page       = rMessage.GetPage();
    unsigned short Resolution = rMessage.GetSubResolution();
    uint64_t PosX       = rMessage.GetPosX();
    uint64_t PosY       = rMessage.GetPosY();

    // Get the resolution descriptor for the given resolution
    HASSERT(Page < CountPages());
    HASSERT(Resolution < GetPageDescriptor(Page)->CountResolutions());
    HFCPtr<HRFResolutionDescriptor> pResolution(GetPageDescriptor(Page)->GetResolutionDescriptor(Resolution));
    HASSERT(pResolution != 0);

    // Compute the tile index from the position
    uint64_t Index = pResolution->ComputeBlockIndex(PosX, PosY);

    // Mark the tile as empty
    pResolution->SetBlockDataFlag(Index, HRFDATAFLAG_EMPTY);

    // Propagate the message upward, so that the tile be re-read.
    return (true);
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
void HRFRasterFileCache::SetAutoErase(bool pi_autoErase)
    {
    m_AutoErase = pi_autoErase;
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
bool HRFRasterFileCache::GetAutoErase()const
    {
    return m_AutoErase;
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
void HRFRasterFileCache::SetCacheInvalidate(bool pi_cacheInvalidate)
    {
    m_CacheInvalidate = pi_cacheInvalidate;
    }

//-----------------------------------------------------------------------------
// Private
// Create Descriptor for the cache
//-----------------------------------------------------------------------------
void HRFRasterFileCache::BuildDescriptors(uint32_t pi_Page)
    {
    HPRECONDITION(m_pCache != 0);
    HPRECONDITION(m_pOriginalFile != 0);
    HPRECONDITION(pi_Page < m_pOriginalFile->CountPages());

    HFCMonitor Monitor(GetKey());

    HFCPtr<HRFPageDescriptor> pOriginalPageDesc(m_pOriginalFile->GetPageDescriptor(pi_Page));

    // we always cache the page if the raster file contains only 1 page or we cache a specific page
    bool CachePage = (m_pOriginalFile->CountPages() == 1 || m_OriginalPageID != -1 ? true : false);

    // Adapt each resolutions from adapted file when their access is different of tile
    HRFPageDescriptor::ListOfResolutionDescriptor ListOfResolutionDescriptor;

    // !!!! SebG : Forbid 1 bit multi-resolution cache file when keepping the cache
    //             to ensure backward compatibility with older Imagepp software.
    // We allow the 1bit Multires cache, if m_sEnable1BitMultiresCacheSupport is at true or we create
    // a temporary cache file.
    // see SetEnable1BitMultiresCacheSupport()
    //
    unsigned short OriginalResolutionCount = pOriginalPageDesc->CountResolutions();
    if ((pOriginalPageDesc->GetResolutionDescriptor(0)->GetPixelType()->CountPixelRawDataBits() == 1) &&
        !(m_AutoErase || ImageppLib::GetHost().GetImageppLibAdmin()._Is1BitMultiresCacheSupportEnable()))
        {
        // if we have an internet file, cache exactly the file
        HFCPtr<HRFRasterFile> pOriginalFile(m_pOriginalFile);
        while (pOriginalFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
            pOriginalFile = ((HFCPtr<HRFRasterFileExtender>&)pOriginalFile)->GetOriginalFile();

        if (!pOriginalFile->IsCompatibleWith(HRFFileId_InternetImaging/*HRFInternetImagingFile::CLASS_ID*/))
            OriginalResolutionCount = 1;
        }

    for (unsigned short Resolution = 0; Resolution < OriginalResolutionCount; Resolution++)
        {
        if ((pOriginalPageDesc->GetResolutionDescriptor(Resolution)->GetBlockType() != HRFBlockType::IMAGE) &&
            ((pOriginalPageDesc->GetResolutionDescriptor(Resolution)->GetReaderBlockAccess() == HRFBlockAccess::SEQUENTIAL) ||
             (pOriginalPageDesc->GetResolutionDescriptor(Resolution)->GetWriterBlockAccess() == HRFBlockAccess::SEQUENTIAL)))
            CachePage = true;

        // Get the current res descriptor
        HFCPtr<HRFResolutionDescriptor> pSourceRes = pOriginalPageDesc->GetResolutionDescriptor(Resolution);
        HASSERT(pSourceRes != 0);
        HRFBlockType CacheBlockType= pSourceRes->GetBlockType();

        // When the source storage type is LINE we force the cache to STRIP of 1 line
        if (CacheBlockType == HRFBlockType::LINE)
            {
            CacheBlockType = HRFBlockType::STRIP;
            CachePage = true;
            }

        // When the source storage type is IMAGE we force the cache to STRIP of image height
        if (CacheBlockType == HRFBlockType::IMAGE)
            {
            CacheBlockType = HRFBlockType::STRIP;
            CachePage = true;
            }

        // Get the default codec for the PixelType
        HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
        pPixelTypeCapability = new HRFPixelTypeCapability(pSourceRes->GetAccessMode(),
                                                          pSourceRes->GetPixelType()->GetClassID(),
                                                          new HRFRasterFileCapabilities());

        pPixelTypeCapability = static_cast<HRFPixelTypeCapability*>(m_pCache->GetCapabilities()->
                               GetCapabilityOfType(((HFCPtr<HRFCapability>&)pPixelTypeCapability)).GetPtr());

        HASSERT(pPixelTypeCapability != 0);
        HASSERT(pPixelTypeCapability->CountCodecs() > 0);

        HFCPtr<HCDCodec> pCodec = HCDCodecFactory::GetInstance().Create(m_pCreator->
            GetSelectedCodecFor(pPixelTypeCapability->
            GetPixelTypeClassID()));

        if (m_pCreator->CountCompressionStepFor(pPixelTypeCapability->GetPixelTypeClassID()) > 0)
            {
            uint32_t SelCompQuality = m_pCreator->GetSelectedCompressionQualityFor(pPixelTypeCapability->
                GetPixelTypeClassID());
            if (pCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
                ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality((Byte)SelCompQuality);
            else if (pCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
                ((HFCPtr<HCDCodecFlashpix>&)pCodec)->SetQuality((Byte)SelCompQuality);
            }

        // DEBUG MODE ONLY
        // JPEG ISO Compression
        // Select the page
#if 0
        HDEBUGCODE
        (
            HFCPtr<HRFResolutionDescriptor> pResDescriptor = pSourceRes;
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
        HFCPtr<HRFResolutionDescriptor> pRes(new HRFResolutionDescriptor(
                                                 pSourceRes->GetAccessMode(),            // AccessMode,
                                                 GetCapabilities(),                      // Capabilities,
                                                 pSourceRes->GetResolutionXRatio(),      // XResolutionRatio,
                                                 pSourceRes->GetResolutionYRatio(),      // YResolutionRatio,
                                                 pSourceRes->GetPixelType(),             // PixelType,
                                                 pCodec,                                 // Codec,
                                                 HRFBlockAccess::RANDOM,                 // RStorageAccess,
                                                 HRFBlockAccess::RANDOM,                 // WStorageAccess,
                                                 pSourceRes->GetScanlineOrientation(),   // ScanLineOrientation,
                                                 pSourceRes->GetInterleaveType(),        // InterleaveType
                                                 pSourceRes->IsInterlace(),              // IsInterlace,
                                                 pSourceRes->GetWidth(),                 // Width,
                                                 pSourceRes->GetHeight(),                // Height,
                                                 pSourceRes->GetBlockWidth(),            // BlockWidth,
                                                 pSourceRes->GetBlockHeight(),           // BlockHeight,
                                                 0,                                      // BlocksDataFlag
                                                 CacheBlockType,
                                                 pSourceRes->GetNumberOfPass(),          // NumberOfPass
                                                 pSourceRes->GetPaddingBits(),           // PaddingBits
                                                 pSourceRes->GetDownSamplingMethod()));  // DownSamplingMethod

        // We set in creation all flags to empty
        HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
        HPRECONDITION(pRes->CountBlocks() <= SIZE_MAX);
        HPRECONDITION(pRes->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);
        pBlocksDataFlag = new HRFDataFlag[(size_t)pRes->CountBlocks()];
        memset(pBlocksDataFlag, HRFDATAFLAG_EMPTY, (size_t)(pRes->CountBlocks() * sizeof(HRFDataFlag)));
        pRes->SetBlocksDataFlag(pBlocksDataFlag);

        ListOfResolutionDescriptor.push_back(pRes);
        }

    // Best match pages and resolutions
    HRPPixelPalette*                  pRepresentativePalette    = 0;
    HRPHistogram*                     pHistogram                = 0;
    HRFThumbnail*                     pThumbnail                = 0;
    HRFClipShape*                     pClipShape                = 0;
    HGF2DTransfoModel*                pTransfoModel             = 0;
    HRPFilter*                        pFilters                  = 0;

    // Best match the TransfoModel
    if (pOriginalPageDesc->HasTransfoModel())
        pTransfoModel = pOriginalPageDesc->GetTransfoModel();

    // Best match  the Filter
    if (pOriginalPageDesc->HasFilter())
        pFilters = (HRPFilter*)&pOriginalPageDesc->GetFilter();

    // Best match  the ClipShape
    if (pOriginalPageDesc->HasClipShape())
        pClipShape = pOriginalPageDesc->GetClipShape();

    // Best match  the Histogram
    if (pOriginalPageDesc->HasHistogram())
        pHistogram = pOriginalPageDesc->GetHistogram();

    // Best match  the Thumbnail
    if (pOriginalPageDesc->HasThumbnail())
        pThumbnail = pOriginalPageDesc->GetThumbnail();

    // Best match  the RepresentativePalette
    if (pOriginalPageDesc->HasRepresentativePalette())
        pRepresentativePalette = (HRPPixelPalette*)&pOriginalPageDesc->GetRepresentativePalette();

    // Cache only supported tags
    HAutoPtr<HPMAttributeSet> pTagList;

    if (CachePage)
        {
        m_pCache->AddPage(new HRFPageDescriptor(m_pCache->GetAccessMode(),
                                                m_pCache->GetCapabilities(),
                                                ListOfResolutionDescriptor,
                                                pRepresentativePalette,
                                                pHistogram,
                                                pThumbnail,
                                                pClipShape,
                                                pTransfoModel,
                                                pFilters,
                                                pTagList));
        }
    else
        {
        m_pCache->AddPage(new HRFPageDescriptor(true)); // empty page
        }
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFRasterFileCache::BuildCapabilities()
    {
    HPRECONDITION(m_pOriginalFile != 0);
    HPRECONDITION(m_pCache  != 0);
    HFCMonitor Monitor(GetKey());

    // Create the capabilities
    m_CombinedRasterFileCapabilities = new HRFCombinedRasterFileCapabilities(m_pCache->GetCapabilities(),
                                                                             m_pOriginalFile->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFRasterFileCache::AdaptBlockForPage(const HFCPtr<HRFPageDescriptor>&             pi_rFromPage,
                                            const HFCPtr<HRFPageDescriptor>&             pi_rToPage,
                                            HRFRasterFileBlockAdapter::BlockDescriptor*  po_pBlockDesc) const
    {
    HPRECONDITION(pi_rFromPage != 0);
    HPRECONDITION(pi_rToPage != 0);

    bool Result = false;

    if (!pi_rFromPage->IsEmpty() && !pi_rToPage->IsEmpty())
        {
        // if different we adapt original block type to the cache block type
        HFCPtr<HRFResolutionDescriptor> pToResolutionDescriptor_1_1  = pi_rToPage->GetResolutionDescriptor(0);
        HFCPtr<HRFResolutionDescriptor> pFromResolutionDescriptor_1_1 = pi_rFromPage->GetResolutionDescriptor(0);


        // allow to adapt only when the block size are different
        // ex: if source is IMAGE and the cache is STRIP and we have only one strip
        //     it is not necessary to adapt the source on the cache block
        if ((pFromResolutionDescriptor_1_1->GetBlockWidth()  != pToResolutionDescriptor_1_1->GetBlockWidth()) ||
            (pFromResolutionDescriptor_1_1->GetBlockHeight() != pToResolutionDescriptor_1_1->GetBlockHeight()))
            {
            po_pBlockDesc->m_BlockType = pToResolutionDescriptor_1_1->GetBlockType();
            po_pBlockDesc->m_BlockWidth = pToResolutionDescriptor_1_1->GetBlockWidth();
            po_pBlockDesc->m_BlockHeight = pToResolutionDescriptor_1_1->GetBlockHeight();
            Result = true;
            }
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> HRFRasterFileCache::BuildPageDescriptor(const HFCPtr<HRFPageDescriptor>& pi_rpOriginalPage,
                                                                  const HFCPtr<HRFPageDescriptor>& pi_rpCachePage) const
    {
    HPRECONDITION(pi_rpOriginalPage != 0);
    HPRECONDITION(pi_rpCachePage != 0);


    // !!!! HChkSebG !!!!
    // Do not allow accessing a ressolution who do not exist into the pOriginalPageDescriptor.
    HASSERT(pi_rpCachePage->CountResolutions() <= pi_rpOriginalPage->CountResolutions());

    HFCPtr<HRFPageDescriptor> pPage;
    if (pi_rpCachePage->IsEmpty())
        pPage = pi_rpOriginalPage;
    else
        {
        // Add all resolutions descriptor from Cache file
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;

        for (unsigned short Resolution=0; Resolution < pi_rpCachePage->CountResolutions(); Resolution++)
            {
            HASSERT(pi_rpCachePage->GetResolutionDescriptor(Resolution)->HasBlocksDataFlag());

            // DEBUG MODE ONLY
            // JPEG ISO Compression
            // Select the page
#if 0
            HDEBUGCODE
            (
                HFCPtr<HRFResolutionDescriptor> pResDescriptor = pi_rpCachePage->GetResolutionDescriptor(Resolution);
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

            // We don't want to lose the default raw data when we create a cache file.
            HASSERT(pi_rpOriginalPage->CountResolutions() > 0);
            HFCPtr<HRPPixelType> pOriginalPagePixelType(pi_rpOriginalPage->GetResolutionDescriptor(0)->GetPixelType());
            HFCPtr<HRPPixelType> pCachePagePixelType(pi_rpCachePage->GetResolutionDescriptor(Resolution)->GetPixelType());

            HASSERT(pOriginalPagePixelType->GetClassID() == pCachePagePixelType->GetClassID());
            if (pOriginalPagePixelType->GetClassID() == pCachePagePixelType->GetClassID())
                {
                pCachePagePixelType->SetDefaultRawData(pOriginalPagePixelType->GetDefaultRawData());
                }

            // Create a copy of this resolution descriptor for the ResBooster
            HFCPtr<HRFResolutionDescriptor> pOriginalResDesc(pi_rpOriginalPage->GetResolutionDescriptor(Resolution));
            HFCPtr<HRFResolutionDescriptor> pCacheResDesc(pi_rpCachePage->GetResolutionDescriptor(Resolution));

            HFCPtr<HRFResolutionDescriptor> pResolution =
                new HRFResolutionDescriptor(pOriginalResDesc->GetAccessMode(),          // AccessMode
                                            GetCapabilities(),                          // Capabilities,
                                            pCacheResDesc->GetResolutionXRatio(),       // XResolutionXRatio,
                                            pCacheResDesc->GetResolutionYRatio(),       // YResolutionYRatio,
                                            pCachePagePixelType,                        // PixelType,
                                            pCacheResDesc->GetCodec(),                  // CodecsList,
                                            HRFBlockAccess::RANDOM,                     // RStorageAccess,
                                            HRFBlockAccess::RANDOM,                     // WStorageAccess,
                                            pOriginalResDesc->GetScanlineOrientation(), // ScanLineOrientation,
                                            pCacheResDesc->GetInterleaveType(),         // InterleaveType
                                            pCacheResDesc->IsInterlace(),               // IsInterlace,
                                            pCacheResDesc->GetWidth(),                  // Width,
                                            pCacheResDesc->GetHeight(),                 // Height,
                                            pCacheResDesc->GetBlockWidth(),             // BlockWidth,
                                            pCacheResDesc->GetBlockHeight(),            // BlockHeight,
                                            pCacheResDesc->GetBlocksDataFlag(),         // BlocksDataFlag
                                            pCacheResDesc->GetBlockType(),              // Storage Type
                                            pCacheResDesc->GetNumberOfPass(),           // NumberOfPass
                                            pCacheResDesc->GetPaddingBits(),            // PaddingBits
                                            pCacheResDesc->GetDownSamplingMethod());    // DownSamplingMethod

            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // The original information have priority for the page descriptor
        // combine the Page descriptors information from original file and Cache file
        pPage = new HRFPageDescriptor(pi_rpOriginalPage->GetAccessMode(),
                                      GetCapabilities(),
                                      pi_rpOriginalPage,
                                      pi_rpCachePage,
                                      ListOfResolutionDescriptor);
        }
    return pPage;
    }
//-----------------------------------------------------------------------------
// Public
// Obtain a smart pointer on the cache
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile>& HRFRasterFileCache::GetCacheFile() const
    {
    HFCMonitor Monitor(GetKey());

    return ((HFCPtr<HRFRasterFile>&)m_pCache);
    }


//-----------------------------------------------------------------------------
// Public
// Return true.
//
// overwrite method from HRFRasterFileExtender
//-----------------------------------------------------------------------------
bool HRFRasterFileCache::IsCacheExtender() const
    {
    return true;
    }
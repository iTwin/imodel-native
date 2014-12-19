//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetImagingRasterFileCache.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Class : HRFInternetImagingRasterFileCache
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetImagingRasterFileCache.h>
#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HRFLocalCacheFileCreator.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFCombinedRasterFileCapabilities.h>

#include <Imagepp/all/h/HFCURLFile.h>


//-----------------------------------------------------------------------------
// HMR Message Map
//-----------------------------------------------------------------------------
HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRFInternetImagingRasterFileCache, HRFRasterFile, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRFInternetImagingRasterFileCache, HRFProgressImageChangedMsg, NotifyProgressImageChanged)
HMG_END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// This methods allow to know if the specified raster file need a cache.
//-----------------------------------------------------------------------------
bool HRFInternetImagingRasterFileCache::NeedCacheFor(HFCPtr<HRFRasterFile>& pi_rpForRasterFile)
    {
    bool NeedCache = false;

    for (uint32_t Page = 0; (Page < pi_rpForRasterFile->CountPages()) && (NeedCache == false); Page++)
        {
        HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpForRasterFile->GetPageDescriptor(Page);

        uint32_t ResCount = pPageDescriptor->IsUnlimitedResolution() ? 1 : pPageDescriptor->CountResolutions();
        for (unsigned short Resolution = 0; (Resolution < ResCount) && (NeedCache == false); Resolution++)
            {
            if ((pPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockType() != HRFBlockType::IMAGE) &&
                ((pPageDescriptor->GetResolutionDescriptor(Resolution)->GetReaderBlockAccess() == HRFBlockAccess::SEQUENTIAL) ||
                 (pPageDescriptor->GetResolutionDescriptor(Resolution)->GetWriterBlockAccess() == HRFBlockAccess::SEQUENTIAL)))
                NeedCache = true;
            }
        }

    return NeedCache;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingRasterFileCache::HRFInternetImagingRasterFileCache(HFCPtr<HRFRasterFile>&     pi_rpSource,
                                                                     const HRFCacheFileCreator* pi_pCreator,
                                                                     bool                      pi_AutoErase)
    : HRFRasterFileExtender(pi_rpSource),
      m_pCreator(pi_pCreator),
      m_AutoErase(pi_AutoErase)
    {
    HPRECONDITION(pi_rpSource != 0);
    HPRECONDITION(pi_pCreator != 0);

    HFCMonitor Monitor(GetKey());

    // always cache the first page
    HFCPtr<HRFRasterFileCache> pCache(new HRFRasterFileCache(pi_rpSource, pi_pCreator, pi_AutoErase, 0));
    HPRECONDITION(pCache != 0);
    HPRECONDITION(pCache->GetCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, pCache->GetAccessMode()) != 0);

    m_PageCache.insert(PageCacheMap::value_type(0, pCache));

    // Link on the source
    LinkTo(m_pOriginalFile);

    // Specify the FileCache is Open...
    m_IsOpen = true;
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetImagingRasterFileCache::~HRFInternetImagingRasterFileCache()
    {
    HFCMonitor Monitor(GetKey());

    // Unlink from the source
    UnlinkFrom(m_pOriginalFile);

    m_PageCache.clear();

    // The raster file close will kill it's internal key, so release it now.
    Monitor.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Save the cached file and the cached.
//-----------------------------------------------------------------------------
void HRFInternetImagingRasterFileCache::Save()
    {
    if (m_IsOpen == true)
        {
        for (uint32_t PageInd = 0; PageInd < CountPages(); PageInd++)
            {
            GetPageFile(PageInd)->Save();
            }

        //Don't need to call the save to the original file since it should be called
        //in the Save above for the page 0.
        //HRFRasterFileExtender::Save()
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFInternetImagingRasterFileCache::GetCapabilities() const
    {
    HFCMonitor Monitor(GetKey());

    // return the capabilities of the first page
    return (const_cast<HRFInternetImagingRasterFileCache*>(this))->GetCache(0)->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Public
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFInternetImagingRasterFileCache::GetWorldIdentificator() const
    {
    HFCMonitor Monitor(GetKey());

    return (m_pOriginalFile->GetWorldIdentificator());
    }

//-----------------------------------------------------------------------------
// Public
// File manipulation
//-----------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> HRFInternetImagingRasterFileCache::GetPage(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return (const_cast<HRFInternetImagingRasterFileCache*>(this))->GetCache(pi_Page)->GetPageDescriptor(0);
    }

//-----------------------------------------------------------------------------
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFInternetImagingRasterFileCache::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HFCMonitor Monitor(GetKey());

    return (false);
    }

//-----------------------------------------------------------------------------
// Public
// CountPages
//
// return the original file count page
//-----------------------------------------------------------------------------
uint32_t HRFInternetImagingRasterFileCache::CountPages() const
    {
    return m_pOriginalFile->CountPages();
    }

//-----------------------------------------------------------------------------
// Public
// GetPageDescriptor
//-----------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> HRFInternetImagingRasterFileCache::GetPageDescriptor(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());
    HFCMonitor Monitor(GetKey());

    return (const_cast<HRFInternetImagingRasterFileCache*>(this))->GetCache(pi_Page)->GetPageDescriptor(0);
    }

//-----------------------------------------------------------------------------
// Public
// Editor creation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFInternetImagingRasterFileCache::CreateResolutionEditor(uint32_t      pi_Page,
                                                                               unsigned short pi_Resolution,
                                                                               HFCAccessMode pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());

    HFCMonitor Monitor(GetKey());

    return GetCache(pi_Page)->CreateResolutionEditor(0, pi_Resolution, pi_AccessMode);
    }


//------------------------------------------------------------------------------
// Public
// Indicates if the file supports LookAhead optimization
//------------------------------------------------------------------------------
bool HRFInternetImagingRasterFileCache::HasLookAhead() const
    {
    HASSERT(0); // must be execute on a specific page
    return false;
    }


//------------------------------------------------------------------------------
// Public
// This method indicates if the source is ready to receive LookAhead notifications
//------------------------------------------------------------------------------
bool HRFInternetImagingRasterFileCache::CanPerformLookAhead (uint32_t pi_Page) const
    {
    HASSERT(0); // must be execute on a specific page
    return false;
    }


//------------------------------------------------------------------------------
// Public
// Sets the LookAhead for a whole extent
//------------------------------------------------------------------------------
void HRFInternetImagingRasterFileCache::RequestLookAhead(uint32_t               pi_Page,
                                                         const HGFTileIDList&   pi_rBlocks,
                                                         bool                  pi_Async)
    {
    HASSERT(0); // must be execute on a specific page
    }


//------------------------------------------------------------------------------
// Public
// This method is used in SetLookAhead to indicate to a derived class that
// the current LookAhead has been cancelled.
//------------------------------------------------------------------------------
void HRFInternetImagingRasterFileCache::CancelLookAhead(uint32_t pi_Page)
    {
    HASSERT(0); // must be execute on a specific page
    }


//-----------------------------------------------------------------------------
// Public
// When the source notifies us that a tile has arrived
//-----------------------------------------------------------------------------
bool HRFInternetImagingRasterFileCache::NotifyProgressImageChanged (const HMGMessage& pi_rMessage)
    {
    // Propagate the message upward, so that the tile be re-read.
    return (true);
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
void HRFInternetImagingRasterFileCache::SetAutoErase(bool pi_autoErase)
    {
    HFCMonitor Monitor(GetKey());

    m_AutoErase = pi_autoErase;

    PageCacheMap::iterator Itr(m_PageCache.begin());
    while (Itr != m_PageCache.end())
        {
        Itr->second->SetAutoErase(m_AutoErase);
        Itr++;
        }
    }

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
bool HRFInternetImagingRasterFileCache::GetAutoErase()const
    {
    HFCMonitor Monitor(GetKey());

    return m_AutoErase;
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFileCache>& HRFInternetImagingRasterFileCache::GetCache(uint32_t pi_Page)
    {
    HPRECONDITION(pi_Page < CountPages());

    // check if the page is already cached
    PageCacheMap::iterator Itr(m_PageCache.find(pi_Page));
    if (Itr == m_PageCache.end())
        {
        // cache the page
        HFCPtr<HRFRasterFileCache> pPage(new HRFRasterFileCache(m_pOriginalFile->GetPageFile(pi_Page),
                                                                m_pCreator,
                                                                m_AutoErase,
                                                                pi_Page));
        m_PageCache.insert(PageCacheMap::value_type(pi_Page, pPage));
        Itr = m_PageCache.find(pi_Page);
        }

    HPOSTCONDITION(Itr != m_PageCache.end() && Itr->second->CountPages() == 1);

    return Itr->second;
    }

//-----------------------------------------------------------------------------
// Public
// Return true.
//
// overwrite method from HRFRasterFileExtender
//-----------------------------------------------------------------------------
bool HRFInternetImagingRasterFileCache::IsCacheExtender() const
    {
    return true;
    }


bool HRFInternetImagingRasterFileCache::PagesAreRasterFile() const
    {
    return true;
    }

HFCPtr<HRFRasterFile> HRFInternetImagingRasterFileCache::GetPageFile(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return (HFCPtr<HRFRasterFile>&)(const_cast<HRFInternetImagingRasterFileCache*>(this))->GetCache(pi_Page);
    }
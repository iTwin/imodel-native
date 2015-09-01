//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileCache.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFileCache
//-----------------------------------------------------------------------------
#pragma once

#include "HRFRasterFileExtender.h"
#include "HRFCacheFileCreator.h"
#include "HRFRasterFileBlockAdapter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFileCache : public HRFRasterFileExtender
    {
    //--------------------------------------
    // Macros
    //--------------------------------------

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFRasterFileId_Cache, HRFRasterFileExtender)


public:
    // This methods allow to know if the specified raster file need a cache.
    IMAGEPP_EXPORT static bool    NeedCacheFor(HFCPtr<HRFRasterFile>& pi_rpForRasterFile);


    //--------------------------------------
    // Constructor/Destructor
    //--------------------------------------

    IMAGEPP_EXPORT                         HRFRasterFileCache(const HFCPtr<HRFRasterFile>&  pi_rpSource,
                                                      const HRFCacheFileCreator*    pi_pCreator,
                                                      bool                          pi_AutoErase = false,
                                                      uint32_t                     pi_Page = -1);

    IMAGEPP_EXPORT virtual                 ~HRFRasterFileCache();


    //--------------------------------------
    // Overriden Methods
    //--------------------------------------

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities      () const;

    // File information
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator() const;

    // File manipulation
    virtual HFCPtr<HRFPageDescriptor>       GetPageDescriptor     (uint32_t pi_Page) const;
    virtual bool                           AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    // Obtain a smart pointer on the source and its cache
    HFCPtr<HRFRasterFile>&                  GetCacheFile() const;
    bool                                   IsCacheExtender() const;

    // Editor creation
    virtual HRFResolutionEditor*            CreateResolutionEditor(uint32_t      pi_Page,
                                                                   unsigned short pi_Resolution,
                                                                   HFCAccessMode pi_AccessMode);

    virtual void                            Save();
    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------

    // Indicates if the file supports LookAhead optimization
    virtual bool                           HasLookAheadByBlock (uint32_t pi_Page) const;
    virtual bool                           HasLookAheadByExtent(uint32_t pi_Page) const;

    // This method is used in SetLookAhead to verify if the derived class is
    // ready to receive LookAhead request.  It returns true by default.
    virtual bool                           CanPerformLookAhead (uint32_t pi_Page) const;

    // This method is used in SetLookAhead to give the list of needed tiles
    // to a derived class, since it knows how to obtain the tiles.
    virtual void                            SetLookAhead        (uint32_t               pi_Page,
                                                                 const HGFTileIDList&   pi_rBlocks,
                                                                 uint32_t               pi_ConsumerID,
                                                                 bool                  pi_Async);

    virtual void                            SetLookAhead        (uint32_t               pi_Page,
                                                                 unsigned short        pi_Resolution,
                                                                 const HVEShape&        pi_rShape,
                                                                 uint32_t               pi_ConsumerID,
                                                                 bool                  pi_Async);



    //--------------------------------------
    // Message Handlers
    //--------------------------------------

    bool                                    NotifyProgressImageChanged (const HMGMessage& pi_rMessage);

    //--------------------------------------
    // Methods
    //--------------------------------------

    IMAGEPP_EXPORT void                             SetAutoErase(bool pi_autoErase);
    IMAGEPP_EXPORT bool                             GetAutoErase()const;

    // for internal use (for IRASB)
    // The impact will be only on the destructor
    IMAGEPP_EXPORT void                             SetCacheInvalidate(bool pi_cacheInvalidate);

protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Source and cached file
    HFCPtr<HRFRasterFile>           m_pCache;
    const HRFCacheFileCreator*      m_pCreator;
    bool                           m_AutoErase;
    uint32_t                        m_OriginalPageID;
    bool                           m_CacheInvalidate;


    // unified capabilities
    HFCPtr<HRFRasterFileCapabilities>
    m_CombinedRasterFileCapabilities;


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Build the unified capabilities & resulting descriptors
    void                    BuildCapabilities();
    void                    BuildDescriptors(uint32_t pi_Page);
    void                    Close();

private:

    bool                       AdaptBlockForPage   (const HFCPtr<HRFPageDescriptor>&               pi_rFromPage,
                                                     const HFCPtr<HRFPageDescriptor>&               pi_rToPage,
                                                     HRFRasterFileBlockAdapter::BlockDescriptor*    po_pBlockDesc) const;

    HFCPtr<HRFPageDescriptor>   BuildPageDescriptor (const HFCPtr<HRFPageDescriptor>&               pi_rpOriginalPage,
                                                     const HFCPtr<HRFPageDescriptor>&               pi_rpCachePage) const;

    void                        SynchronizeFiles();

    // Methods Disabled
    HRFRasterFileCache(const HRFRasterFileCache& pi_rObj);
    HRFRasterFileCache&  operator=(const HRFRasterFileCache& pi_rObj);

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT);
    };
END_IMAGEPP_NAMESPACE


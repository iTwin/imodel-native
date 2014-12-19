//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImagingRasterFileCache.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetImagingRasterFileCache
//-----------------------------------------------------------------------------
#pragma once

#include "HRFRasterFileExtender.h"
#include "HRFCacheFileCreator.h"
#include "HRFRasterFileCache.h"

class HRFInternetImagingRasterFileCache : public HRFRasterFileExtender
    {
    //--------------------------------------
    // Macros
    //--------------------------------------

    // Class ID for this class.
    HDECLARE_CLASS_ID(1554, HRFRasterFileExtender)


public:
    // This methods allow to know if the specified raster file need a cache.
    _HDLLg static bool    NeedCacheFor(HFCPtr<HRFRasterFile>& pi_rpForRasterFile);


    //--------------------------------------
    // Constructor/Destructor
    //--------------------------------------

    _HDLLg                         HRFInternetImagingRasterFileCache(HFCPtr<HRFRasterFile>&     pi_rpSource,
                                                                     const HRFCacheFileCreator* pi_pCreator,
                                                                     bool                      pi_AutoErase = false);

    _HDLLg virtual                 ~HRFInternetImagingRasterFileCache();


    //--------------------------------------
    // Overriden Methods
    //--------------------------------------

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities      () const;

    // File information
    virtual const HGF2DWorldIdentificator
    GetWorldIdentificator() const;

    // File manipulation
    virtual bool                       PagesAreRasterFile() const;
    virtual HFCPtr<HRFRasterFile>       GetPageFile(uint32_t pi_Page) const;
    virtual uint32_t                    CountPages() const;
    HFCPtr<HRFPageDescriptor>           GetPageDescriptor(uint32_t pi_Page) const;

    virtual HFCPtr<HRFPageDescriptor>   GetPage(uint32_t pi_Page) const;
    virtual bool                       AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    // Obtain a smart pointer on the source and its cache
    HFCPtr<HRFRasterFile>&              GetCacheFile(uint32_t pi_Page) const;
    bool                               IsCacheExtender() const;

    // Editor creation
    virtual HRFResolutionEditor*        CreateResolutionEditor(uint32_t      pi_Page,
                                                               unsigned short pi_Resolution,
                                                               HFCAccessMode pi_AccessMode);


    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------

    // Indicates if the file supports LookAhead optimization
    virtual bool   HasLookAhead() const;

    // This method is used in SetLookAhead to verify if the derived class is
    // ready to receive LookAhead request.  It returns true by default.
    virtual bool   CanPerformLookAhead (uint32_t pi_Page) const;

    // This method is used in SetLookAhead to give the list of needed tiles
    // to a derived class, since it knows how to obtain the tiles.
    virtual void    RequestLookAhead    (uint32_t               pi_Page,
                                         const HGFTileIDList&   pi_rBlocks,
                                         bool                  pi_Async = false);

    // This method is used in SetLookAhead to indicate to a derived class that
    // the current LookAhead has been cancelled.
    virtual void    CancelLookAhead     (uint32_t pi_Page);

    // Save the cache file and the cached file
    virtual void    Save                ();

    //--------------------------------------
    // Message Handlers
    //--------------------------------------

    bool           NotifyProgressImageChanged (const HMGMessage& pi_rMessage);

    //--------------------------------------
    // Methods
    //--------------------------------------

    _HDLLg void            SetAutoErase(bool pi_autoErase);
    _HDLLg bool           GetAutoErase()const;

protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Source and cached file
    typedef map<uint32_t, HFCPtr<HRFRasterFileCache> > PageCacheMap;
    PageCacheMap                m_PageCache;
    const HRFCacheFileCreator*  m_pCreator;
    bool                       m_AutoErase;

    //--------------------------------------
    // Methods
    //--------------------------------------

    // Build the unified capabilities & resulting descriptors
    void                    BuildDescriptors();

private:


    HFCPtr<HRFRasterFileCache>&     GetCache(uint32_t pi_Page);

    // Methods Disabled
    HRFInternetImagingRasterFileCache(const HRFInternetImagingRasterFileCache& pi_rObj);
    HRFInternetImagingRasterFileCache&  operator=(const HRFInternetImagingRasterFileCache& pi_rObj);

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLg);
    };

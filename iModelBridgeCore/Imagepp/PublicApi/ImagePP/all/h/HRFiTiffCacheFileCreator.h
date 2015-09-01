//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFiTiffCacheFileCreator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFiTiffCacheFileCreator
//-----------------------------------------------------------------------------
// This class describes the stretcher implementation
//-----------------------------------------------------------------------------
#pragma once

#include "HRFLocalCacheFileCreator.h"
#include "HFCMacros.h"

//-----------------------------------------------------------------------------
// This is a helper class to instantiate a cache file object
// without knowing the different cache file format.
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFiTiffCacheFileCreator :  public HRFLocalCacheFileCreator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFiTiffCacheFileCreator)

public:
    // Destructor
    virtual ~HRFiTiffCacheFileCreator();

    // This methods allow to know if we have a cache file for the specified raster file.
    virtual bool                 HasCacheFor   (const HFCPtr<HRFRasterFile>&       pi_rpForRasterFile,
                                                 uint32_t                           pi_Page = -1) const;

    virtual bool                 HasCacheFor   (const HFCPtr<HFCURL>&              pi_rForRasterFileURL,
                                                 uint32_t                           pi_Page = -1) const;

    virtual HFCPtr<HFCURL>        GetCacheURLFor(const HFCPtr<HRFRasterFile>&       pi_rpForRasterFile,
                                                 uint32_t                           pi_Page = -1) const;

    virtual HFCPtr<HFCURL>        GetCacheURLFor(const HFCPtr<HFCURL>&              pi_rForRasterFileURL,
                                                 uint32_t                           pi_Page = -1,
                                                 uint64_t                          pi_Offset = 0) const;

    // This factory methods allow to instantiate the cache file for the specified raster file.
    virtual HFCPtr<HRFRasterFile> GetCacheFileFor(HFCPtr<HRFRasterFile>&            pi_rpForRasterFile,
                                                  uint32_t                          pi_Page = -1) const;

    IMAGEPP_EXPORT HFCPtr<HFCURL>         ComposeURLFor(const HFCPtr<HFCURL>& pi_rpURLFileName,
                                                const WString&        pi_Extension,
                                                uint64_t             pi_Offset = 0,
                                                uint32_t              pi_Page = -1) const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const;

    IMAGEPP_EXPORT bool IsModificationTimeValid(time_t pi_FileModificationTime,
                                        time_t pi_CacheFileModificationTime) const;

protected:

    bool HasCacheFor(const HFCPtr<HFCURL>& pi_rForRasterFileURL,
                      time_t               pi_FileModificationTime,
                      uint32_t            pi_Page,
                      uint64_t             pi_Offset) const;

private:
    
    // Disabled methods
    HRFiTiffCacheFileCreator(const HRFiTiffCacheFileCreator&);
    HRFiTiffCacheFileCreator& operator=(const HRFiTiffCacheFileCreator&);
    HRFiTiffCacheFileCreator();
    };
END_IMAGEPP_NAMESPACE


//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileResBooster.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFileResBooster
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCURL.h"
//#include "HRFMacros.h"
#include "HRFRasterFileExtender.h"
//#include "HRFCacheFileCreator.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRFCacheFileCreator;
class HGFResolutionDescriptor;


class HRFRasterFileResBooster : public HRFRasterFileExtender
    {
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFRasterFileId_ResBooster, HRFRasterFileExtender)

public:
    // This methods allow to know if the specified raster file need a resolution booster.
    IMAGEPP_EXPORT static bool NeedResBoosterFor(HFCPtr<HRFRasterFile>& pi_rpForRasterFile);

    // map that contain the pages to boost
    typedef map<uint32_t, HGFResolutionDescriptor> WantedResolutionsMap;

    // allow to Open an image file
    IMAGEPP_EXPORT                                       HRFRasterFileResBooster(HFCPtr<HRFRasterFile>&        pi_rpOriginalFile,
                                                                         const HRFCacheFileCreator*    pi_pCreator,
                                                                         const WantedResolutionsMap*   pi_pWantedResolutionsMap = 0,
                                                                         bool                         pi_AutoErase = false);

    IMAGEPP_EXPORT virtual                               ~HRFRasterFileResBooster();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const override;

    // File information
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;

    // File manipulation
    bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode) override;
    // Get the raster file
    virtual  HFCPtr<HRFRasterFile>        GetBoosterFile ();
    bool                                 IsCacheExtender() const override;

    bool                                 IsOriginalRasterDataStorage() const override;


    // Auto Erase File
    IMAGEPP_EXPORT void                                   SetAutoErase(bool pi_autoErase);
    IMAGEPP_EXPORT bool                                  GetAutoErase()const;

    void                                  Save() override;

protected:
    // Raster File
    HFCPtr<HRFRasterFile>                 m_pBoosterFile;
    const HRFCacheFileCreator*            m_pCreator;
    bool                                 m_AutoErase;

    // ResBosster
    HFCPtr<HRFRasterFileCapabilities>     m_pRasterFileCapabilities;

    // Methods
    virtual bool                         Open                ();
    virtual void                          CreateDescriptors   ();


private:
    HArrayAutoPtr<bool>                  m_pPageFromOriginalPage;
    // Create the file
    void                                  Close               ();

    void                                  SynchronizeFiles();

    bool                                 Create              (const WantedResolutionsMap* pi_pWantedResolutions);

    // Methods Disabled
    HRFRasterFileResBooster(const HRFRasterFileResBooster& pi_rObj);
    HRFRasterFileResBooster&  operator=(const HRFRasterFileResBooster& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


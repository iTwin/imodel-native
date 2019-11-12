//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFilePageDecorator
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HRFPageFile.h"
#include "HRFRasterFileExtender.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DTransfoModel;

class HRFRasterFilePageDecorator : public HRFRasterFileExtender
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFRasterFileId_PageDecorator, HRFRasterFileExtender)

    // allow to Open an image file
    IMAGEPP_EXPORT                                       HRFRasterFilePageDecorator(HFCPtr<HRFRasterFile>&   pi_rpOriginalFile,
                                                                            const HRFPageFileCreator* pi_pCreator);
    // allow to Open an image file
    IMAGEPP_EXPORT                                       HRFRasterFilePageDecorator(HFCPtr<HRFRasterFile>&   pi_rpOriginalFile,
                                                                            HFCPtr<HRFPageFile>&      pi_rpPageFile);

    IMAGEPP_EXPORT virtual                               ~HRFRasterFilePageDecorator();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const override;

    // File information
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;
    virtual const HGF2DWorldIdentificator GetPageWorldIdentificator (uint32_t pi_Page = 0) const override;

    // File manipulation
    bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode) override;

    HRFResolutionEditor*          CreateUnlimitedResolutionEditor(uint32_t                  pi_Page,
                                                                          double                   pi_Resolution,
                                                                          HFCAccessMode             pi_AccessMode) override;
    void Save() override;

protected:
    // Raster File
    HFCPtr<HRFPageFile>                   m_pPageFile;

    // ResBosster
    HFCPtr<HRFRasterFileCapabilities>     m_CombinedRasterFileCapabilities;

    // Methods
    virtual bool                         Open                ();
    virtual void                          CreateDescriptors   ();


    void                                  Save(bool pi_IsToBeClosed);

    // Get the raster file
    HFCPtr<HRFPageFile>                     GetPageFile();
    HFCPtr<HRFRasterFile>           GetPageFile               (uint32_t pi_Page) const override{
        return T_Super::GetPageFile(pi_Page);
        }

private:
    // Members
    HFCPtr<HGF2DTransfoModel>             m_pSLOConverterModel;
    // Create the file
    void                                  Close               ();

    // Methods Disabled
    HRFRasterFilePageDecorator(const HRFRasterFilePageDecorator& pi_rObj);
    HRFRasterFilePageDecorator&  operator=(const HRFRasterFilePageDecorator& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


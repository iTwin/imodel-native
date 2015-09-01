//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFilePageDecorator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;
    virtual const HGF2DWorldIdentificator GetPageWorldIdentificator (uint32_t pi_Page = 0) const override;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual HRFResolutionEditor*          CreateUnlimitedResolutionEditor(uint32_t                  pi_Page,
                                                                          double                   pi_Resolution,
                                                                          HFCAccessMode             pi_AccessMode);
    virtual void Save();

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
    virtual HFCPtr<HRFRasterFile>           GetPageFile               (uint32_t pi_Page) const  {
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


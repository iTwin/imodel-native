//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGSGAsciiGridFile.h $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGSGAsciiGridFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#ifdef IPP_HAVE_GDAL_SUPPORT
#include "HFCMacros.h"
#include "HRFMacros.h"

#include "HRFGdalSupportedFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
//--------------------------------------------------
// class HRFGSGAsciiGridCapabilities
//--------------------------------------------------
class HRFGSGAsciiGridCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFGSGAsciiGridCapabilities();
    };


struct HRFGSGAsciiGridCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                       IsKindOfFile       (const HFCPtr<HFCURL>&    pi_rpURL,
                                                           uint64_t                 pi_Offset = 0) const override;
    // Identification information
    virtual Utf8String                 GetLabel           () const override;
    virtual Utf8String                 GetSchemes         () const override;
    virtual Utf8String                 GetExtensions      () const override;

    virtual Utf8String GetShortName() const override { return "GSG"; }

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities    () override;

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>       Create             (const HFCPtr<HFCURL>& pi_rpURL,
                                                            HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                            uint64_t              pi_Offset = 0) const override;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFGSGAsciiGridCreator)

    // Disabled methods
    HRFGSGAsciiGridCreator                                  ();
    };


class HRFGSGAsciiGridFile : public HRFGdalSupportedFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_GSGAsciiGrid, HRFGdalSupportedFile)

    // Allow to Open an image file
    HRFGSGAsciiGridFile     (const HFCPtr<HFCURL>&           pi_rpURL,
                             HFCAccessMode                   pi_AccessMode = HFC_READ_ONLY,
                             uint64_t                        pi_Offset = 0);

    virtual ~HRFGSGAsciiGridFile ();


    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File manipulation
    virtual bool    AddPage (HFCPtr<HRFPageDescriptor>       pi_pPage);

    virtual void    Save ();

    // NOTE: Uses default HRFGdalSupportedFile CreateResolutionEditor

protected:

    // Protected Methods
    virtual void    CreateDescriptors ();
    virtual void    HandleNoDisplayBands ();
    virtual void    DetectPixelType ();

    virtual HRFScanlineOrientation GetScanLineOrientation () const;

private:

    // Methods Disabled
    HRFGSGAsciiGridFile         (const HRFGSGAsciiGridFile&       pi_rObj);
    HRFGSGAsciiGridFile&        operator= (const HRFGSGAsciiGridFile&       pi_rObj);
    };

END_IMAGEPP_NAMESPACE

#endif  // IPP_HAVE_GDAL_SUPPORT

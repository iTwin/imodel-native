//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFArcInfoGridFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFArcInfoGridFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#include "HRFGdalSupportedFile.h"
#include "HRFRasterFileCapabilities.h"

#include "HFCMacros.h"
#include "HRFMacros.h"

//--------------------------------------------------
// class HRFArcInfoGridCapabilities
//--------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFArcInfoGridCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFArcInfoGridCapabilities();
    };


struct HRFArcInfoGridCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                       IsKindOfFile       (const HFCPtr<HFCURL>&    pi_rpURL,
                                                            uint64_t                pi_Offset = 0) const;
    // Identification information
    virtual WString                     GetLabel           () const;
    virtual WString                     GetSchemes         () const;
    virtual WString                     GetExtensions      () const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities    ();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>       Create             (const HFCPtr<HFCURL>& pi_rpURL,
                                                            HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                            uint64_t             pi_Offset = 0) const;

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFArcInfoGridCreator)

private:

    // Disabled methods
    HRFArcInfoGridCreator                                  ();

    };


class HRFArcInfoGridFile : public HRFGdalSupportedFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_ArcInfoGrid, HRFGdalSupportedFile)

    // Allow to Open an image file
    HRFArcInfoGridFile         (const HFCPtr<HFCURL>&           pi_rpURL,
                                HFCAccessMode                   pi_AccessMode = HFC_READ_ONLY,
                                uint64_t                       pi_Offset = 0);

    virtual                                 ~HRFArcInfoGridFile        ();


    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities            () const;

    // File manipulation
    virtual bool                           AddPage                    (HFCPtr<HRFPageDescriptor>       pi_pPage);

    virtual void                            Save                       ();

    // NOTE: Uses default HRFGdalSupportedFile CreateResolutionEditor

protected:

    // Protected Methods
    virtual void                            CreateDescriptors ()  override;

    virtual void                            HandleNoDisplayBands () override;

    virtual HRFScanlineOrientation          GetScanLineOrientation () const  override;

private:

    // Methods Disabled
    HRFArcInfoGridFile         (const HRFArcInfoGridFile&       pi_rObj);
    HRFArcInfoGridFile&                     operator=                  (const HRFArcInfoGridFile&       pi_rObj);
    };
END_IMAGEPP_NAMESPACE

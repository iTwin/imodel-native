//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFNitfFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFNitfFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#include "HRFGdalSupportedFile.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HFCBinStream.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFNitfEditor.h"
#include "HRFMacros.h"
#include "HTIFFTag.h"

//--------------------------------------------------
// class HRFDoqCapabilities
//--------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFNitfCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFNitfCapabilities();
    };


struct HRFNitfCreator : public HRFRasterFileCreator
    {

    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;


    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFNitfCreator)
private:



    // Disabled methods
    HRFNitfCreator();
    };



class HRFNitfFile : public HRFGdalSupportedFile
    {
public:
    friend HRFNitfCreator;
    friend HRFNitfEditor;


    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Nitf, HRFGdalSupportedFile)

    // allow to Open an image file
    HRFNitfFile          (const HFCPtr<HFCURL>&          pi_rpURL,
                          HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                          uint64_t                      pi_Offset = 0);

    virtual                               ~HRFNitfFile           ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();



protected:

    // Protected Methods
    virtual bool            Open() override;
    virtual void            CreateDescriptors() override;

    virtual void            HandleNoDisplayBands () override;

private:

    // Methods Disabled
    HRFNitfFile(const HRFNitfFile& pi_rObj);
    HRFNitfFile& operator=(const HRFNitfFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

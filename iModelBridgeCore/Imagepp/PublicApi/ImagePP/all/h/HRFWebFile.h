//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFWebFile
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HRFRasterFileCapabilities.h>
#include <ImagePP/all/h/HFCMacros.h>

BEGIN_IMAGEPP_NAMESPACE
class HFCExclusiveKey;

//------------------------------------------------------------------------------
// WebFile Capabilities
//------------------------------------------------------------------------------
class HRFWebFileCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFWebFileCapabilities();

    };

//------------------------------------------------------------------------------
// Class HRFWebFile
//------------------------------------------------------------------------------
class HRFWebFile : public HRFRasterFile
    {
    //--------------------------------------
    // Macros & Friends
    //--------------------------------------
    HDECLARE_CLASS_ID(HRFFileId_Web, HRFRasterFile)

public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFWebFile(const HFCPtr<HFCURL> &  pi_rpURL,
               HFCAccessMode           pi_AccessMode = HFC_READ_ONLY,
               uint64_t               pi_Offset = 0
              );

    virtual         ~HRFWebFile();

    // Returns the file exlusive key
    HFCExclusiveKey&              GetKey() const override;

    //--------------------------------------
    // Methods
    //--------------------------------------

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const override;

    // World attching the imaging
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;

    // File manipulation
    bool   AddPage(HFCPtr<HRFPageDescriptor> pi_pPage) override;

    // Editing
    HRFResolutionEditor* CreateResolutionEditor(uint32_t pi_Page, uint16_t pi_Resolution, HFCAccessMode pi_AccessMode) override;

    // Save
    void    Save() override;

    //--------------------------------------
    // Utility methods
    //--------------------------------------

    IMAGEPP_EXPORT const HFCPtr<HRFRasterFile> & GetLocalRasterFile() const;
    const HFCPtr<HFCURL>&         GetLocalURL () const;

private:
    //--------------------------------------
    // Methods
    //--------------------------------------

    //--------------------------------------
    // Attributes
    //--------------------------------------
    HFCPtr<HFCURL>                  m_pLocalURL;
    HFCPtr<HRFRasterFile>           m_pLocalRasterFile;

    //--------------------------------------
    // Others
    //--------------------------------------

    // Methods Disabled
    HRFWebFile(const HRFRasterFile& pi_rObj);
    HRFWebFile& operator=(const HRFRasterFile& pi_rObj);
    };


//------------------------------------------------------------------------------
// Main Creator
//------------------------------------------------------------------------------
struct HRFWebFileCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool       IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                     uint64_t             pi_Offset = 0) const override;

    bool   SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const override;

    // Identification information
    Utf8String      GetExtensions() const override;
    Utf8String      GetLabel() const override;
    Utf8String      GetSchemes() const override;

    virtual Utf8String GetShortName() const override { return "WEB"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities> & GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const override;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFWebFileCreator)

    // Disabled methodes
    HRFWebFileCreator();

    // Label & schemes
    Utf8String         m_Label;
    Utf8String         m_Schemes;
    Utf8String         m_Extensions;
    };
END_IMAGEPP_NAMESPACE

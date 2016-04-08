//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFWebFile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    virtual HFCExclusiveKey&              GetKey() const;

    //--------------------------------------
    // Methods
    //--------------------------------------

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // World attching the imaging
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool   AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    // Editing
    virtual HRFResolutionEditor* CreateResolutionEditor(uint32_t pi_Page, uint16_t pi_Resolution, HFCAccessMode pi_AccessMode);

    // Save
    virtual void    Save();

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
    virtual bool       IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                     uint64_t             pi_Offset = 0) const;

    virtual bool   SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;

    // Identification information
    virtual Utf8String      GetExtensions() const;
    virtual Utf8String      GetLabel() const;
    virtual Utf8String      GetSchemes() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities> & GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const;

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

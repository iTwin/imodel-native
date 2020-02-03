//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HFCBinStream.h"
#include "HRFMacros.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFEpsLineEditor;


class HRFEpsCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFEpsCapabilities();

    };


/** -----------------------------------------------------------------------------
    Encapsulated Postscript (EPS) file format. Can only be created, no reading
    or updating. Supported pixel types are 1BitGray, 8BitGray and 24BitsRGB.
    -----------------------------------------------------------------------------
*/
class HRFEpsFile : public HRFRasterFile
    {
    friend class HRFEpsLineEditor;

public:

    HDECLARE_CLASS_ID(HRFFileId_Eps, HRFRasterFile)

    // allow to Open or to create an empty file

    HRFEpsFile(const HFCPtr<HFCURL>& pi_rpURL,
               HFCAccessMode         pi_AccessMode = HFC_CREATE_ONLY,
               uint64_t             pi_Offset = 0);

    virtual         ~HRFEpsFile();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() const override;

    // File information
    const HGF2DWorldIdentificator
    GetWorldIdentificator() const override;

    // File manipulation
    bool   AddPage(HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*
    CreateResolutionEditor(uint32_t      pi_Page,
                           uint16_t pi_Resolution,
                           HFCAccessMode pi_AccessMode) override;

    void    Save() override;

    uint64_t  GetFileCurrentSize() const override;


protected:


private:

    ////////////////
    // Methods
    ////////////////

    //:> Disabled
    HRFEpsFile(const HRFEpsFile& pi_rObj);
    HRFEpsFile& operator=(const HRFEpsFile& pi_rObj);

    // Create a new file
    bool           Create();

    void            SaveEpsFile(bool pi_CloseFile);

    void            WriteHeader(HFCPtr<HRFResolutionDescriptor>& pi_rpResDescriptor);
    void            WriteFooter();


    ////////////////
    // Attributes
    ////////////////

    // The Eps file on disk.
    HAutoPtr<HFCBinStream>
    m_pFile;
    };


// Creator.
struct HRFEpsCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool   IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                 uint64_t             pi_Offset = 0) const override;

    // Identification information
    Utf8String GetLabel() const override;
    Utf8String GetSchemes() const override;
    Utf8String GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "EPS"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>
    Create(const HFCPtr<HFCURL>& pi_rpURL,
           HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
           uint64_t             pi_Offset = 0) const override;

private:

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFEpsCreator)

    // Disabled methods
    HRFEpsCreator();
    };
END_IMAGEPP_NAMESPACE


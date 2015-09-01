//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFEPSFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() const;

    // File information
    virtual const HGF2DWorldIdentificator
    GetWorldIdentificator() const;

    // File manipulation
    virtual bool   AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*
    CreateResolutionEditor(uint32_t      pi_Page,
                           unsigned short pi_Resolution,
                           HFCAccessMode pi_AccessMode);

    virtual void    Save();

    virtual uint64_t  GetFileCurrentSize() const;


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
    virtual bool   IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                 uint64_t             pi_Offset = 0) const;

    // Identification information
    virtual WString GetLabel() const;
    virtual WString GetSchemes() const;
    virtual WString GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>
    Create(const HFCPtr<HFCURL>& pi_rpURL,
           HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
           uint64_t             pi_Offset = 0) const;

private:

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFEpsCreator)

    // Disabled methods
    HRFEpsCreator();
    };
END_IMAGEPP_NAMESPACE


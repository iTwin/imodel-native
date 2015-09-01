//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphTG4File.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HRFIntergraphMonochromeFile.h"

#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFTG4Capabilities : public HRFRasterFileCapabilities
    {
public:
    HRFTG4Capabilities();

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFTG4Capabilities)
    };


class HRFIntergraphTG4File : public HRFIntergraphMonochromeFile
    {
public:
    friend class HRFIntergraphResolutionEditor;
    friend struct HRFIntergraphTG4Creator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IntergraphTG4, HRFIntergraphMonochromeFile)

    // allow to Open an image file
    HRFIntergraphTG4File (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode   pi_AccessMode = HFC_READ_ONLY,
                          uint64_t       pi_Offset = 0);

    // File capabilities
    virtual const   HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const;

    virtual            uint64_t                             GetFileCurrentSize() const;

protected:

    // Constructor use only to create a child
    HRFIntergraphTG4File (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode          pi_AccessMode,
                          uint64_t              pi_Offset,
                          bool                  pi_DontOpenFile);

    virtual void    CreateDescriptors();

private:
    // Methods Disabled
    HRFIntergraphTG4File(const HRFIntergraphTG4File& pi_rObj);
    HRFIntergraphTG4File& operator=(const HRFIntergraphTG4File& pi_rObj);
    };

//********************************************************************************//
//  Intergraph Creator.
//********************************************************************************//

struct HRFIntergraphTG4Creator : public HRFIntergraphFile::Creator
    {

    // Opens the file and verifies if it is the right type
    virtual bool       IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                     uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString     GetLabel() const;
    virtual WString     GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>
    Create(const HFCPtr<HFCURL>& pi_rpURL,
           HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
           uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIntergraphTG4Creator)

    // Disabled methodes
    HRFIntergraphTG4Creator();
    };
END_IMAGEPP_NAMESPACE


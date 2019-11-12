//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HRFIntergraphMonochromeFile.h"

#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HMGMessage;

class HRFCitCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFCitCapabilities();

    };


class HRFIntergraphCitFile : public HRFIntergraphMonochromeFile
    {
public:
    friend class HRFIntergraphResolutionEditor;
    friend struct HRFIntergraphCitCreator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IntergraphCit, HRFIntergraphMonochromeFile)

    // allow to Open an image file
    HRFIntergraphCitFile (const HFCPtr<HFCURL>& pi_rpURL,
                          HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                          uint64_t             pi_Offset = 0);

    // File capabilities
    const   HFCPtr<HRFRasterFileCapabilities>&  GetCapabilities() const override;

    uint64_t                           GetFileCurrentSize() const override;


protected:

    // Constructor use only to create a child
    HRFIntergraphCitFile (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode          pi_AccessMode,
                          uint64_t              pi_Offset,
                          bool                  pi_DontOpenFile);

    virtual void    CreateDescriptors();


private:
    // Methods Disabled
    HRFIntergraphCitFile(const HRFIntergraphCitFile& pi_rObj);
    HRFIntergraphCitFile& operator=(const HRFIntergraphCitFile& pi_rObj);

    };

//********************************************************************************//
//  Intergraph Creator.
//********************************************************************************//

struct HRFIntergraphCitCreator : public HRFIntergraphFile::Creator
    {

    // Opens the file and verifies if it is the right type
    virtual bool       IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                     uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual Utf8String     GetLabel() const;
    virtual Utf8String     GetExtensions() const;

    virtual Utf8String GetShortName() const { return "CIT"; }

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>
    Create(const HFCPtr<HFCURL>& pi_rpURL,
           HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
           uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIntergraphCitCreator)

    // Disabled methodes
    HRFIntergraphCitCreator();
    };
END_IMAGEPP_NAMESPACE


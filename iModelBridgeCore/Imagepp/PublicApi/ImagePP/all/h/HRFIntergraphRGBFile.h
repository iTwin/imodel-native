//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphRGBFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HRFIntergraphColorFile.h"

#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRGBCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFRGBCapabilities();

    };

class HRFIntergraphRGBFile : public HRFIntergraphColorFile
    {
public:
    friend class HRFIntergraphResolutionEditor;
    friend struct HRFIntergraphRGBCreator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IntergraphRGB, HRFIntergraphFile)

    // allow to Open an image file
    HRFIntergraphRGBFile (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode   pi_AccessMode = HFC_READ_ONLY,
                          uint64_t       pi_Offset = 0);

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const;

    virtual          uint64_t
    GetFileCurrentSize() const;

    virtual bool AddPage (HFCPtr<HRFPageDescriptor> pi_pPage) override;

protected:

    virtual void    CreateDescriptors ();   

private:
    // Methods Disabled
    HRFIntergraphRGBFile(const HRFIntergraphRGBFile& pi_rObj);
    HRFIntergraphRGBFile& operator=(const HRFIntergraphRGBFile& pi_rObj);
    };

//********************************************************************************//
//  Intergraph Creator.
//********************************************************************************//

struct HRFIntergraphRGBCreator : public HRFIntergraphFile::Creator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIntergraphRGBCreator)

    // Disabled methodes
    HRFIntergraphRGBCreator();
    };
END_IMAGEPP_NAMESPACE


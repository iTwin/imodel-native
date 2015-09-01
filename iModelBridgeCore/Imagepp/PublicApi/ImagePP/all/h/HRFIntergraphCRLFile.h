//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphCRLFile.h $
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
class HRFCRLCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFCRLCapabilities();

    };


class HRFIntergraphCRLFile : public HRFIntergraphColorFile
    {
public:
    friend class HRFIntergraphResolutionEditor;
    friend struct HRFIntergraphCRLCreator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IntergraphCRL, HRFIntergraphFile)

    // allow to Open an image file
    HRFIntergraphCRLFile (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode   pi_AccessMode = HFC_READ_ONLY,
                          uint64_t       pi_Offset = 0);

    // File capabilities
    virtual const   HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const;

    virtual            uint64_t                             GetFileCurrentSize() const;

protected:

    // Constructor use only to create a child
    HRFIntergraphCRLFile (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode          pi_AccessMode,
                          uint64_t              pi_Offset,
                          bool                  pi_DontOpenFile);

    virtual void    CreateDescriptors();

private:
    // Methods Disabled
    HRFIntergraphCRLFile(const HRFIntergraphCRLFile& pi_rObj);
    HRFIntergraphCRLFile& operator=(const HRFIntergraphCRLFile& pi_rObj);
    };

//********************************************************************************//
//  Intergraph Creator.
//********************************************************************************//

struct HRFIntergraphCRLCreator : public HRFIntergraphFile::Creator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIntergraphCRLCreator)

    // Disabled methodes
    HRFIntergraphCRLCreator();
    };
END_IMAGEPP_NAMESPACE


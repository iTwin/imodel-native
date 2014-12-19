//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphRGBCompressFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HRFIntergraphColorFile.h"
#include "HRFRasterFileCapabilities.h"

class HRFRGBCompressCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFRGBCompressCapabilities();

    };

class HRFIntergraphRGBCompressFile : public HRFIntergraphColorFile
    {
public:
    friend class  HRFIntergraphResolutionEditor;
    friend struct HRFIntergraphRGBCreator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(1488, HRFIntergraphFile)

    // allow to Open an image file
    HRFIntergraphRGBCompressFile (const HFCPtr<HFCURL>& pi_rpURL,
                                  HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                  uint64_t             pi_Offset = 0);

    virtual            uint64_t GetFileCurrentSize() const;

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const;

protected:

    // Constructor use only to create a child
    HRFIntergraphRGBCompressFile (const HFCPtr<HFCURL>&  pi_rpURL,
                                  HFCAccessMode          pi_AccessMode,
                                  uint64_t              pi_Offset,
                                  bool                  pi_DontOpenFile);

    virtual void    CreateDescriptors ();

private:
    // Methods Disabled
    HRFIntergraphRGBCompressFile(const HRFIntergraphRGBCompressFile& pi_rObj);
    HRFIntergraphRGBCompressFile& operator=(const HRFIntergraphRGBCompressFile& pi_rObj);
    };

//-----------------------------------------------------------------------------
//  Intergraph RGB Compress Creator.
//-----------------------------------------------------------------------------

struct HRFIntergraphRGBCompressCreator : public HRFIntergraphFile::Creator
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
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFIntergraphRGBCompressCreator)

    // Disabled methodes
    HRFIntergraphRGBCompressCreator();
    };


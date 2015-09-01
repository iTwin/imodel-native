//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphMPFFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HRFIntergraphMonochromeFile.h"
#include "HRFRasterFileCapabilities.h"

//-----------------------------------------------------------------------------
//  Intergraph MPF Capabilities
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRFMPFCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFMPFCapabilities();

    };

//-----------------------------------------------------------------------------
//  Intergraph MPF file
//-----------------------------------------------------------------------------


class HRFIntergraphMPFFile : public HRFIntergraphMonochromeFile
    {
public:
    friend class HRFIntergraphResolutionEditor;
    friend struct HRFIntergraphMPFCreator;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IntergraphMPF, HRFIntergraphMonochromeFile)

    virtual   ~HRFIntergraphMPFFile();

    // allow to Open an image file
    HRFIntergraphMPFFile (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode          pi_AccessMode = HFC_READ_ONLY,
                          uint64_t              pi_Offset = 0);

    // File capabilities
    virtual const   HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const;

    virtual void                                       Save();

    virtual            uint64_t                             GetFileCurrentSize() const;

protected:

    // Constructor use only to create a child
    HRFIntergraphMPFFile (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode          pi_AccessMode,
                          uint64_t              pi_Offset,
                          bool                  pi_DontOpenFile);

    virtual void    CreateDescriptors();
    virtual bool   Open();

    virtual HRFResolutionEditor*
    CreateResolutionEditor (uint32_t      pi_Page,
                            unsigned short pi_Resolution,
                            HFCAccessMode pi_AccessMode);

private:
    // Methods Disabled
    HRFIntergraphMPFFile(const HRFIntergraphMPFFile& pi_rObj);
    HRFIntergraphMPFFile& operator=(const HRFIntergraphMPFFile& pi_rObj);

    typedef vector< HFCPtr<HRFRasterFile> > IntergraphRasterFileVector;

    IntergraphRasterFileVector m_IntergraphRasterFiles;
    };

//-----------------------------------------------------------------------------
//  Intergraph MPF Creator.
//-----------------------------------------------------------------------------

struct HRFIntergraphMPFCreator : public HRFIntergraphFile::Creator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIntergraphMPFCreator)

    // Disabled methodes
    HRFIntergraphMPFCreator();
    };
END_IMAGEPP_NAMESPACE


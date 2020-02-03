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

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCBinStream.h"
#include "HFCAccessMode.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

// Forward declare png structs because we do not want to publish png.h
typedef struct png_struct_def * png_structp;
typedef struct png_info_def * png_infop;

BEGIN_IMAGEPP_NAMESPACE
class HRFPngCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFPngCapabilities();

    };

class HRFPngFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Png, HRFRasterFile)

    friend class HRFPngLineEditor;
    friend class HRFPngImageEditor;

    // allow to Open an image file
    HRFPngFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                uint64_t        pi_Offset = 0);

    virtual     ~HRFPngFile();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const override;

    // File information
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;

    // File manipulation
    bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode) override;

    void                          Save() override;

    const HFCMemoryBinStream*     GetMemoryFilePtr() const override;

    //! Read png data and extract image to a byte stream. Thread-safe and without HRF overhead.
    static BentleyStatus ReadToBuffer(bvector<Byte>& outPixels, uint32_t& width, uint32_t& height, bool& isRGBA, Byte const* pPngData, size_t pngDataSize);

protected:

    // Members.

    // Methods
    // Constructor use only to create a child
    //
    HRFPngFile          (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);

    virtual bool               Open();
    virtual void                CreateDescriptors();
    HFCBinStream*       GetFilePtr();

private:

    // Members
    bool                       m_InterlaceFileReaded;
    HAutoPtr<HFCBinStream>      m_pPngFile;
    png_structp                 m_pPngFileStruct;
    png_infop                   m_pPngInfo;
    png_infop                   m_pPngEndInfo;

    // Histogramm
    HAutoPtr<HRPHistogram>      m_pHistogram;

    // Create the file
    void                    SavePngFile(bool pi_CloseFile);
    bool                   Create();

    bool                   AssignStructTo(HFCPtr<HRFPageDescriptor> pi_pPage);
    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile () const;
    void                    GetTransfoModel         ();
    void                    SetTransfoModel         (HFCPtr<HRFPageDescriptor> pi_pResolutionDescriptor);
    void                    SetPixelTypeToPage      (HFCPtr<HRFResolutionDescriptor> pi_pResolutionDescriptor);

    // Methods Disabled
    HRFPngFile(const HRFPngFile& pi_rObj);
    HRFPngFile&             operator= (const HRFPngFile& pi_rObj);
    };


// Png Creator.
struct HRFPngCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "PNG"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPngCreator)

    // Disabled methodes
    HRFPngCreator();
    };
END_IMAGEPP_NAMESPACE


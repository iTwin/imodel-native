//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPngFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
struct png_struct_def;
typedef png_struct_def* png_structp;
struct png_info_struct;
typedef png_info_struct* png_infop;

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
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    virtual const HFCMemoryBinStream*     GetMemoryFilePtr() const;

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
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPngCreator)

    // Disabled methodes
    HRFPngCreator();
    };
END_IMAGEPP_NAMESPACE


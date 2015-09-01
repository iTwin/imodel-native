//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRawFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HRFMacros.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFRasterFile.h"
#include "HRFRawLineEditor.h"
#include "HFCBinStream.h"


BEGIN_IMAGEPP_NAMESPACE
//--------------------------------------------------
// class HRFRawCapabilities
//--------------------------------------------------
class HRFRawCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFRawCapabilities();

    };

//--------------------------------------------------
// struct HRFRawCreator
//--------------------------------------------------
struct HRFRawCreator : public HRFRasterFileCreator
    {
public :

    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                   uint64_t             pi_Offset) const;

    // Identification information
    virtual WString                   GetLabel      ()  const;
    virtual WString                   GetSchemes    ()  const;
    virtual WString                   GetExtensions ()  const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;

    IMAGEPP_EXPORT void                              SetImageData          (uint32_t              pi_Width,
                                                                    uint32_t              pi_Height,
                                                                    uint64_t             pi_Offset);
    IMAGEPP_EXPORT void                              SetImagePixelType     (HFCPtr<HRPPixelType>  pi_pPixelType);
    IMAGEPP_EXPORT HFCPtr<HRPPixelType>              GetImagePixelType     () const;
    IMAGEPP_EXPORT uint64_t                         GetOffset             () const;
    IMAGEPP_EXPORT uint64_t                         GetFileSize           () const;
    IMAGEPP_EXPORT uint32_t                          GetBestFitWidth       (uint32_t              pi_Height,
                                                                    uint64_t             pi_Offset,
                                                                    size_t                pi_Footer) const;
    IMAGEPP_EXPORT uint32_t                          GetBestFitHeight      (uint32_t              pi_Width,
                                                                    uint64_t             pi_Offset,
                                                                    size_t                pi_Footer) const;

    IMAGEPP_EXPORT HFCPtr<HRPPixelType>              AutoDetectPixelType   (const HFCPtr<HFCURL>& pi_rpURL,
                                                                    uint64_t             pi_Offset,
                                                                    uint32_t              pi_Footer,
                                                                    uint32_t              pi_Width,
                                                                    uint32_t              pi_Height);
    IMAGEPP_EXPORT void                              AutoDetectFileSize    (const HFCPtr<HFCURL>& pi_rpURL,
                                                                    uint64_t             pi_Offset,
                                                                    uint32_t              pi_Foter,
                                                                    uint32_t              pi_BitsPerPixel,
                                                                    uint32_t&               po_Width,
                                                                    uint32_t&               po_Height);

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFRawCreator)

private:


    // Members
    uint32_t                            m_Width;
    uint32_t                            m_Height;
    uint64_t                           m_Offset;
    uint64_t                           m_FileSize;
    HFCPtr<HRPPixelType>                m_pPixelType;

    // Disabled methodes
    HRFRawCreator();
    };

//--------------------------------------------------
// class HRFRawFile
//--------------------------------------------------
class HRFRawFile : public HRFRasterFile
    {
    friend HRFRawCreator;
    friend HRFRawLineEditor;

public:
    HDECLARE_CLASS_ID(HRFFileId_Raw, HRFRasterFile)

    // allow to Open or to create an empty file
    HRFRawFile       (const HFCPtr<HFCURL>& pi_rpURL,
                      uint32_t              pi_Width,
                      uint32_t              pi_Height,
                      HFCPtr<HRPPixelType>  pi_pPixelType,
                      HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                      uint64_t             pi_Offset = 0);

    virtual                               ~HRFRawFile();

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

protected:
    // Methods
    // Constructor use only to create a child
    HRFRawFile          (const HFCPtr<HFCURL>&       pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);

    virtual bool                       Open                    ();
    virtual void                        CreateDescriptors       ();
    HFCBinStream*               GetFilePtr              () const;


private:

    // Attributs
    uint32_t                            m_ImageWidth;
    uint32_t                            m_ImageHeight;

    HAutoPtr<HFCBinStream>              m_pRawFile;
    HFCPtr<HRPPixelType>                m_pPixelType;

    // Methods Disabled
    HRFRawFile  (const HRFRawFile& pi_rObj);
    HRFRawFile& operator=(const HRFRawFile& pi_rObj);

    // Methods
    bool                               Create                  ();
    };
END_IMAGEPP_NAMESPACE



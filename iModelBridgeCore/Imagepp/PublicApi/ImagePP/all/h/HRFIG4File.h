//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIG4File.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFIG4File
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecCCITTFax4.h"

#include "HFCURL.h"
#include "HFCMacros.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRPPixelType;
class  HFCBinStream;



#define HEADER_SIZE_AT_FILE_END_IN_BYTES 344

struct HeaderAtEndOfFile
    {
    Byte   m_UnknownData[8];
    Byte   m_MagicBytes[2];
    Byte   m_MagicAsciiString[8];
    Byte   m_UnknownData2[18];
    uint32_t m_ImageInfoOffset;
    Byte   m_UnknownData3[8];
    uint32_t m_ImageInfoOffset2;
    Byte   m_UnknownData4[12];
    uint32_t m_ImageStripInfoOffset;
    };

struct ImageStripInfo
    {
    Byte                  m_NbStrips;
    unsigned short         m_StripHeight;
    HArrayAutoPtr<uint32_t>  m_pStripOffsets;
    };

class HRFIG4Capabilities : public HRFRasterFileCapabilities
    {
public:
    HRFIG4Capabilities();

    };

class HRFIG4File : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IG4, HRFRasterFile)

    friend class HRFIG4StripEditor;

    // allow to Open an image file
    HRFIG4File (const HFCPtr<HFCURL>& pi_rpURL,
                HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                uint64_t             pi_Offset = 0);

    virtual        ~HRFIG4File();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;
    const   HRFScanlineOrientation
    GetScanlineOrientation () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    void                                  GetStripInfo(uint32_t pi_PosY,
                                                       uint32_t& po_rOffset,
                                                       uint32_t& po_rSize);

    uint32_t                             GetStripHeight();

protected:
    // Methods
    // Constructor use only to create a child
    //
    HRFIG4File (const HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode          pi_AccessMode,
                uint64_t              pi_Offset,
                bool                  pi_DontOpenFile);
    virtual bool   Open ();
    virtual void    CreateDescriptors ();

    virtual const HFCBinStream* HRFIG4File::GetIG4FilePtr() const;

    const  HFCPtr<HCDCodecCCITT>& GetIG4CodecPtr () const;

private:

    uint32_t                    m_Width;
    uint32_t                    m_Height;

    HAutoPtr<HFCBinStream>      m_pIG4File;

    HFCPtr<HRPPixelType>        m_pPixelType;

    HFCPtr<HCDCodecCCITT>       m_pCodec;

    HeaderAtEndOfFile           m_HeaderAtEOF;
    ImageStripInfo              m_StripInfo;

    // Method
    void            Close();

    // Methods Disabled
    HRFIG4File (const HRFIG4File& pi_rObj);
    HRFIG4File& operator=(const HRFIG4File& pi_rObj);
    };

// IG4 Creator.
struct HRFIG4Creator : public HRFRasterFileCreator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIG4Creator)

    // Disabled methodes
    HRFIG4Creator();
    };
END_IMAGEPP_NAMESPACE



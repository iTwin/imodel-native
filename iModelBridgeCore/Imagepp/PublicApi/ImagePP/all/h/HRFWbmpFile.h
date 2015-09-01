//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFWbmpFile.h $
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

#include "HRFMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFWBMPCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFWBMPCapabilities();
    };


typedef struct WbmpFileHeader
    {
    uint32_t  m_TypeField;
    Byte     m_FixHeaderField;
    Byte*    m_pExtFields;
    uint32_t  m_Width;
    uint32_t  m_Height;
    } WbmpFileHeader;


class HRFWbmpFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Wbmp, HRFRasterFile)

    friend class HRFWbmpLineEditor;

    // allow to Open an image file
    HRFWbmpFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                 HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                 uint64_t        pi_Offset = 0);

    virtual     ~HRFWbmpFile();

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

    static size_t                         ReadNextMultiByteInteger(HFCBinStream& pi_rFile, uint32_t& po_rValRead);
    static size_t                         WriteNextMultiByteInteger(HFCBinStream& pi_rFile, uint32_t pi_valToWrite);

    //Static method
    static size_t                         GetFileHeaderFromFile(HFCBinStream& pi_rFile,
                                                                WbmpFileHeader&         po_rHeader);

protected:

    // Methods
    // Constructor use only to create a child
    //
    HRFWbmpFile          (const HFCPtr<HFCURL>&  pi_rpURL,
                          HFCAccessMode          pi_AccessMode,
                          uint64_t              pi_Offset,
                          bool                  pi_DontOpenFile);
    virtual bool                       Open                ();
    virtual void                        CreateDescriptors   ();
    HFCBinStream*                        GetFilePtr          ();

private:

    // Members
    HAutoPtr<HFCBinStream>  m_pWbmpFile;

    WbmpFileHeader          m_WbmpFileHeader;
    size_t                  m_OffsetToFirstRowInByte;


    // Number of bits to use for line padding per row
    unsigned short         m_PaddingBitsPerRow;

    // Create the file
    void                    SaveWbmpFile(bool pi_CloseFile);
    bool                   Create();

    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile() const;
    void                    SetFileHeaderToFile();

    // Methods Disabled
    HRFWbmpFile(const HRFWbmpFile& pi_rObj);
    HRFWbmpFile&             operator= (const HRFWbmpFile& pi_rObj);
    };


// Wbmp Creator.
struct HRFWbmpCreator : public HRFRasterFileCreator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFWbmpCreator)


    // Disabled methodes
    HRFWbmpCreator();
    };
END_IMAGEPP_NAMESPACE


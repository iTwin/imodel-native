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

#include "HRFMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFBMPCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFBMPCapabilities();
    };


typedef struct BmpFileHeader
    {
    uint16_t m_Type;
    uint32_t m_FileSize;
    uint16_t m_Reserved1;
    uint16_t m_Reserved2;
    uint32_t m_OffBitsToData;
    } BmpFileHeader;

typedef struct BmpInfoHeader
    {
    uint32_t m_StructSize;
    uint32_t m_Width;
    uint32_t m_Height;
    uint16_t m_Planes;             // Must be set to 1
    uint16_t m_BitCount;           // Number of bits-per-pixels
    uint32_t m_Compression;        // Type of compression (BI_RGB=0, BI_RLE8=1, BI_RLE4=2)
    uint32_t m_SizeImage;          // Size in bytes of the image
    uint32_t m_XPelsPerMeter;
    uint32_t m_YPelsPerMeter;
    uint32_t m_ClrUsed;
    uint32_t m_ClrImportant;
    } BmpInfoHeader;

typedef struct BitMapCoreHeader
    {
    uint32_t m_HeaderSize;
    uint16_t m_Width;
    uint16_t m_Height;
    uint16_t m_ColorPlanes;
    uint16_t m_BitsPerPixel;
    } BitMapCoreHeader;


typedef struct BitMapArrayHeader
    {
    uint16_t m_Type;
    uint32_t m_HeaderSize;
    uint32_t m_NextOffset;
    uint16_t m_XDisplay;
    uint16_t m_YDisplay;
    } BmpArrayHeader;

typedef struct RGBColor {
    Byte    m_rgbBlue;
    Byte    m_rgbGreen;
    Byte    m_rgbRed;
    Byte    m_rgbReserved;
    } RGBColor;

typedef struct BmpInfo
    {
    BmpInfoHeader      m_BmpInfoHeader;
    HAutoPtr<RGBColor> m_RgbColors;
    } BmpInfo;




/*
    This class is the representation of an optional BMP header containing the
    red, green and blue filters used to get the values of each color channel.
    This header is necessary when the m_Compression field of BmpInfoHeader is
    set to 3.
*/
class BmpBitMasksHeader
    {

public :

    /** Value of the red mask */
    uint32_t m_RedMask;
    /** Value of the green mask */
    uint32_t m_GreenMask;
    /** Value of the blue mask */
    uint32_t m_BlueMask;

    /** Constructor */
    BmpBitMasksHeader(uint32_t pi_RedMask = 0,
                      uint32_t pi_GreenMask = 0,
                      uint32_t pi_BlueMask = 0 );

    /** Is Equal operator */
    bool operator==(BmpBitMasksHeader pi_Mask) const;

    /** Not equal operator */
    bool operator!=(BmpBitMasksHeader pi_Mask) const;

    /** Assignation operator */
    BmpBitMasksHeader& operator=(BmpBitMasksHeader pi_Mask);

    /** Checks wether filters have been set */
    bool IsInitialized() const;

    }; // Class BmpBitMasksHeader




class HRFBmpFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Bmp, HRFRasterFile)

    friend class    HRFBmpLineEditor;
    friend class    HRFBmpCompressLineEditor;
    friend class    HRFBmpCompressImageEditor;
    friend struct   HRFBmpCreator;

    // allow to Open an image file
    HRFBmpFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                uint64_t        pi_Offset = 0);

    virtual     ~HRFBmpFile();

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

    uint64_t                        GetFileCurrentSize() const override;

protected:

    // Methods
    // Constructor use only to create a child
    //
    HRFBmpFile          (const HFCPtr<HFCURL>&      pi_rpURL,
                         HFCAccessMode              pi_AccessMode,
                         uint64_t                  pi_Offset,
                         bool                      pi_DontOpenFile);
    virtual bool                       Open                ();
    virtual void                        CreateDescriptors   ();
    HFCBinStream*               GetFilePtr          ();

private:

    static bool            ReadBestResolutionArrayHeader   (HFCBinStream*              pio_pFile,
                                                             BitMapArrayHeader*         po_pBmpArrayHeader);

    // Members
    HAutoPtr<HFCBinStream>  m_pBmpFile;

    BitMapArrayHeader       m_BmpArrayHeader;
    uint64_t               m_BmpFileHeaderOffset;
    BmpFileHeader           m_BmpFileHeader;
    BmpInfo                 m_BmpInfo;
    BmpBitMasksHeader       m_BmpBitMasksHeader;
    bool                   m_IsBitmapArrayFile;
    bool                   m_IsRGBQuad;

    // Array of lines offset used by the compress line editor..
    uint32_t*     m_pLinesOffsetBuffer;

    // Number of bits to use for line padding per row
    uint16_t         m_PaddingBitsPerRow;


    // Create the file
    void                    SaveBmpFile(bool pi_CloseFile);
    bool                   Create();

    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile() const;
    void                    SetPixelTypeToPage(HFCPtr<HRFResolutionDescriptor> pi_pResolutionDescriptor);
    bool                   GetFileHeaderFromFile();
    void                    SetFileHeaderToFile();
    bool                   GetBmpInfoHeaderFromFile();
    void                    SetBmpInfoHeaderToFile();
    void                    GetPaletteFromFile();
    void                    SetPaletteToFile();

    // Methods Disabled
    HRFBmpFile(const HRFBmpFile& pi_rObj);
    HRFBmpFile&             operator= (const HRFBmpFile& pi_rObj);
    };


// Bmp Creator.
struct HRFBmpCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "BMP"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;

#ifdef __HMR_DEBUG

    IMAGEPP_EXPORT static void                CreateBmpFileFromImageData(HFCPtr<HFCURL>&       pi_rpFileName, 
                                                                 uint32_t              pi_ImageDataWidth,
                                                                 uint32_t              pi_ImageDataHeight,
                                                                 HFCPtr<HRPPixelType>& pi_rpImageDataPixelType, 
                                                                 Byte*               pi_ImageData);
#endif

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFBmpCreator)


    // Disabled methodes
    HRFBmpCreator();
    };
END_IMAGEPP_NAMESPACE


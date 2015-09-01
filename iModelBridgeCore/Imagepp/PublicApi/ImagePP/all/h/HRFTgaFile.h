//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTgaFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HRFMacros.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFRasterFile.h"
#include "HRFTgaLineEditor.h"
#include "HRFTgaCompressLineEditor.h"
#include "HRFTgaCompressImageEditor.h"
#include "HFCBinStream.h"

//:Ignore
// Disable the automatic word alingnement for TGA structure.
#pragma pack( push, TgaIdent,  1)


#define TGA_SIGNATURE "TRUEVISION-XFILE."
//:End Ignore

/** ---------------------------------------------------------------------------
    This is a classe to declare the generic capabilities of the TGA file format.

    @see HRFRasterFileCapabilities
---------------------------------------------------------------------------  */
BEGIN_IMAGEPP_NAMESPACE
class HRFTgaCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFTgaCapabilities();

    };

/** ---------------------------------------------------------------------------
    This is a kind of object required to create new files and reopen raster files
    that are specific to TGA raster file. It may be required by applications that
    deal with raster files, especially for creating and opening raster files. Raster
    file creators may be obtained from the factory. The instance is accessible from
    the static member GetInstance().

    @see HRFRasterFileCreator
    @see HRFRasterFileFactory
---------------------------------------------------------------------------  */
struct HRFTgaCreator : public HRFRasterFileCreator
    {
public :

    //:> Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                   uint64_t             pi_Offset) const;

    //:> Identification information
    virtual WString                   GetLabel      ()  const;
    virtual WString                   GetSchemes    ()  const;
    virtual WString                   GetExtensions ()  const;

    //:> capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities();

    //:> allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFTgaCreator)

    //:> Disabled methodes
    HRFTgaCreator();
    };

/** ---------------------------------------------------------------------------
    This is the class that is used to create, open, and close a TGA file in
    the HRF architecture. It will ensure that every informations in the file
    are valid.

    @see HRFRasterFile
    @see HRFTgaLineEditor
    @see HRFTgaCompressLineEditor
---------------------------------------------------------------------------  */
class HRFTgaFile : public HRFRasterFile
    {
    friend HRFTgaCreator;
    friend HRFTgaLineEditor;
    friend HRFTgaCompressLineEditor;
    friend HRFTgaCompressImageEditor;

public:
    HDECLARE_CLASS_ID(HRFFileId_Tga, HRFRasterFile)

    //:> allow to Open or to create an empty file
    HRFTgaFile       (const HFCPtr<HFCURL>& pi_rpURL,
                      HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                      uint64_t             pi_Offset = 0);

    virtual                               ~HRFTgaFile();

    //:> File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    //:> File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    //:> File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    virtual uint64_t                        GetFileCurrentSize() const;


protected:
    // Methods
    // Constructor use only to create a child
    HRFTgaFile          (const HFCPtr<HFCURL>&       pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);

    virtual bool                       Open                    ();
    void                        SaveTgaFile             (bool pi_CloseFile);
    virtual void                        CreateDescriptors       ();
    HFCBinStream*               GetFilePtr              () const;
    uint32_t                    GetRasterDataOffset     () const;
    uint32_t                    GetRasterDataEndOffset  () const;
    unsigned short             GetBitsPerPixel         () const;
    bool                       HasPalette              () const;
    bool                       IsCompressed            () const;


private:

    //:> Type definition
    struct TgaFileHeader
        {
        Byte  m_IdLength;                     //:> Number of bytes in Image ID field
        Byte  m_ColorMapType;                 //:> 0 -> no color-map & 1 -> color-map present
        Byte  m_ImageType;                    //:> 0, 1, 2, 3, 9, 10, 11... Supported image type

        //:> Color Map Specification (5 bytes)
        unsigned short m_ColorMapFirstEntryIndex;      //:> if not 0
        unsigned short m_ColorMapLength;               //:> number of color in color-map
        Byte  m_ColorMapEntrySize;            //:> number of bits per entry in color-map

        //:> Image Specification (10 bytes)
        unsigned short m_XOrigin;                      //:> Lower Left corner
        unsigned short m_YOrigin;                      //:> Lower Left corner
        unsigned short m_ImageWidth;
        unsigned short m_ImageHeight;
        Byte  m_PixelDepth;                   //:> Number of bits per pixels (8,16,24,32)
        Byte  m_ImageDescriptor;              //:> Bit flag...
        };

    //:>--------------------------------------------------

    struct TgaImageData
        {
        HArrayAutoPtr<char>   m_pImageId;
        HArrayAutoPtr<Byte>  m_pColorMap;
        //:>HArrayAutoPtr<Byte>  m_pImageData;
        };

    //:>--------------------------------------------------
    //:> A struct can be added for developper area
    //:>--------------------------------------------------

    struct TgaExtensionArea
        {
        unsigned short m_ExtensionSize;
        unsigned char  m_AuthorName[41];               //:> [41] must be 0x00
        unsigned char  m_AuthorComments[324];          //:> 4 line of 81 char where [81] must be 0x00

        //:> Date/Time Stamp (12 bytes)
        unsigned short m_Month;
        unsigned short m_Day;
        unsigned short m_Year;
        unsigned short m_Hour;
        unsigned short m_Minute;
        unsigned short m_Second;

        unsigned char  m_JobNameId[41];                //:> [41] must be 0x00

        //:> Job Time
        unsigned short m_JobHours;
        unsigned short m_JobMinutes;
        unsigned short m_JobSeconds;

        unsigned char  m_SoftwareId[41];               //:> [41] must be 0x00
        unsigned short m_SoftwareVersion;              //:> Version number * 100 - e.g. v4.27b is 427
        unsigned char  m_SoftwareVersionLetter;        //:> e.g. v4.27b -> this field is "b"
        uint32_t m_BackgroundColor;              //:> format is A:R:G:B where A is the alpha channel color.

        //:> Pixel Aspect Ration
        unsigned short m_PixelRatioNum;               //:> Numerator
        unsigned short m_PixelRatioDen;               //:> Denominator

        //:> Gamma Value - Should be between 0.0 and 10.0 with only one decimal
        unsigned short m_GammaNum;                     //:> Numerator
        unsigned short m_GammaDen;                     //:> Denominator

        uint32_t m_ColorCorrectionOffset;        //:> Offset for the color correction table in the file
        uint32_t m_PostageStampOffset;           //:> Offset for the postage stamp image in the file
        uint32_t m_ScanLineOffset;               //:> Offset for the scan line table in the file
        Byte  m_AttributesType;               //:> Type of Alpha channel data contained in the file.
        };

    //:>--------------------------------------------------

    struct TgaExtTableArea
        {
        HArrayAutoPtr<uint32_t>   m_pScanLineTable;           //:> Offset for the first pixel of each line in the data
        Byte                  m_StampWidth;
        Byte                  m_StampHeight;
        HArrayAutoPtr<Byte>   m_pStampData;               //:> Same data type than image data but without compression
        HArrayAutoPtr<unsigned short>  m_pColorCorrectionTable;    //:> If it exist, it is 256*4 short long.
        };

    //:>--------------------------------------------------

    struct TgaFileFooter
        {
        uint32_t m_ExtensionAreaOffset;          //:> Offset for the extention area section
        uint32_t m_DeveloperDirectoryOffset;     //:> Offset for the developer directory section
        char   m_Signature[18];                //:> Must be "TRUEVISION-XFILE." with 0x00 at the end
        };


    //:> Attributes
    HAutoPtr<TgaFileHeader>             m_pTgaFileHeader;
    HAutoPtr<TgaImageData>              m_pTgaImageData;
    HAutoPtr<TgaExtensionArea>          m_pTgaExtentionArea;
    HAutoPtr<TgaExtTableArea>           m_pTgaExtTableArea;
    HAutoPtr<TgaFileFooter>             m_pTgaFileFooter;

    HAutoPtr<HFCBinStream>              m_pTgaFile;

    uint32_t                            m_RasterDataOffset;
    uint32_t                            m_RasterDataEndOffset;

    //:> Methods Disabled
    HRFTgaFile  (const HRFTgaFile& pi_rObj);
    HRFTgaFile& operator=(const HRFTgaFile& pi_rObj);

    //:> Methods
    HFCPtr<HRPPixelType>                CreatePixelTypeFromFile         () const;
    HFCPtr<HCDCodec>                    CreateCodecFromFile             () const;

    bool                               Create                          ();

    void                                GetFileHeaderFromFile           ();
    void                                SetFileHeaderToFile             ();
    void                                ResetHeader();

    void                                GetFileFooterFromFile           ();
    void                                InitializeFileFooter            ();
    void                                SetFileFooterToFile             (bool pi_HasExt);

    void                                GetMapInfoFromFile              ();
    void                                GetExtensionAreaFromFile        ();

    void                                SetPaletteToFile                ();
    bool                               SetThumbnailToFile              ();

    bool                               RunLengthsSpanScanlines         ();
    };
END_IMAGEPP_NAMESPACE

//:Ignore
// Re-enable the automatic word alignement by removing the previous pragma instruction
// at the begining of this class
#pragma pack( pop, TgaIdent)

//:End Ignore


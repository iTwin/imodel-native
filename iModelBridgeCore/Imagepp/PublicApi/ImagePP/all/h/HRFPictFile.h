//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPictFile.h $
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
class HRFPictCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFPictCapabilities();
    };

typedef struct PictRect
    {
    unsigned short m_Top;
    unsigned short m_Left;
    unsigned short m_Bottom;
    unsigned short m_Right;
    } PictRect;
#define PictRect_Size 8

typedef struct OpHeaderV2
    {
    unsigned short m_Version;      /* Always -1 */
    unsigned short m_Reserved1;    /* Always -1 */
    uint32_t   m_HRes;         /* Ignored */
    uint32_t   m_VRes;         /* Ignored */
    PictRect    m_SrcRect;      /* Ignored */
    uint32_t   m_Reserved2;    /* Ignored */
    } OpHeaderV2;
#define OpHeaderV2_Size 24

typedef struct PictFileHeader
    {
    unsigned short m_FileSize;     /* Always 0x0000 */
    PictRect    m_Rect;
    unsigned short m_VersionOpcode;/* Always 0x0011 */
    unsigned short m_Version;      /* Always 0x02ff, PICT 2 */
    unsigned short m_HeaderOpcode; /* Always 0x0C00 */
    OpHeaderV2  m_OpHeader;
    unsigned short m_DefHilite;    /* Always 0x001E */
    unsigned short m_ClipOpcode;   /* Always 0x0001 */
    unsigned short m_ClipSize;     /* Always 0x000A */
    PictRect    m_ClipRect;     /* Same as rect above */
    unsigned short m_OpCode;       /* Always 0x0098 (PictPackBitsHeader) Or  0x009A (PictPixelMapHeader) */
    } PictFileHeader;
#define PictFileHeader_Size 56

typedef struct PictPackBitsHeader
    {
    unsigned short m_RowBytes;     /* Row bytes (unpacked). Its value is given by (width*4)|0x8000 and the 0x8000 part must be removed */
    PictRect    m_Bounds;       /* Same as rect below */
    unsigned short m_Version;      /* Always 0 */
    unsigned short m_PackType;     /* Always 4 (PackBits) */
    uint32_t   m_PackSize;     /* Always 0 */
    uint32_t   m_HRes;         /* Always 0x00480000 */
    uint32_t   m_VRes;         /* Always 0x00480000 */
    unsigned short m_PixelType;    /* 16(24 bit), 0 otherwise */
    unsigned short m_PixelSize;    /* 32(24-bit), 8 (256Col+GrayScale) 0(16Col) */
    unsigned short m_CmpCount;     /* 3 (24-bit), 1 (256Col+GrayScale) 0(16Col) */
    unsigned short m_CmpSize;      /* 8 (24-bit), 8 (256Col+GrayScale) 0(16Col) */
    uint32_t   m_PlaneBytes;   /* Always 0 */
    uint32_t   m_PmTable;      /* Always 0 */
    uint32_t   m_PmReserved;   /* Always 0 */

    } PictPackBitsHeader;
#define PictPackBitsHeader_Size 46

typedef struct PictPixelMapHeader
    {
    uint32_t           m_BaseAddr;     /* Always 0x000000ff */
    PictPackBitsHeader  m_PackBitHeader;
    PictRect            m_SrcRect;      /* Same as bounds above */
    PictRect            m_DstRect;      /* Same as bounds above */
    unsigned short      m_CopyMode;     /* Always 0 */
    } PictPixelMapHeader;
#define PictPixelMapHeader_Size 68

typedef struct PictColorTable
    {
    uint32_t           m_CtSeed;       /* Always 0xffff0000 */
    unsigned short      m_CtFlags;      /* Always 0x8000 */
    unsigned short      m_CtSize;
    } PictColorTable;
#define PictColorTable_Size 8

typedef struct PictRGBColor
    {
    unsigned short m_rgbReserved;
    unsigned short m_rgbRed;
    unsigned short m_rgbGreen;
    unsigned short m_rgbBlue;
    } PictRGBColor;
#define PictRGBColor_Size 8

struct HRFPictCreator;

class HRFPictFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Pict, HRFRasterFile)

    friend class    HRFPictLineEditor;
    friend struct   HRFPictCreator;

    // allow to Open an image file
    HRFPictFile        (const  HFCPtr<HFCURL>&  pi_rpURL,
                        HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                        uint64_t        pi_Offset = 0);

    virtual     ~HRFPictFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&    GetCapabilities        () const;

    // File information
    virtual const HGF2DWorldIdentificator               GetWorldIdentificator  () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    virtual uint64_t                     GetFileCurrentSize() const;

protected:

    // Methods
    // Constructor use only to create a child
    //
    HRFPictFile        (const HFCPtr<HFCURL>&  pi_rpURL,
                        HFCAccessMode          pi_AccessMode,
                        uint64_t              pi_Offset,
                        bool                  pi_DontOpenFile);
    virtual bool                       Open               ();
    virtual void                        CreateDescriptors  ();
    HFCBinStream*               GetFilePtr         ();

private:

    typedef enum
        {
        HEADER_UNSUPPORTED,
        HEADER_PACKBITS,
        HEADER_PIXELMAP
        } DataHeaderType;


    // Members
    HAutoPtr<HFCBinStream>          m_pPictFile;

    PictFileHeader                  m_FileHeader;
    PictPixelMapHeader              m_DataHeader;
    PictColorTable                  m_ColorTable;
    HArrayAutoPtr<PictRGBColor>     m_pColorTableItemsList;

    DataHeaderType                  m_DataHeaderType;

    static void             SwabPictureFrame           (PictRect*                       pio_pFrame);
    static void             SwabOpCodeHeader           (OpHeaderV2*                     pio_pOpHeader);
    static void             SwabPackBitsHeader         (PictPackBitsHeader*             pio_pPackBitsHeader);
    static void             SwabPixelMapHeader         (PictPixelMapHeader*             pio_pPixelMapHeader);
    static void             SwabPictFileHeader         (PictFileHeader*                 pio_pFileHeader);



    static bool            ReadPictFileHeader         (PictFileHeader*                 po_pFileHeader,
                                                        DataHeaderType*                 po_pDataHeaderType,
                                                        HFCBinStream*                   pio_pFile);
    static bool            ReadPixelMapHeader         (PictPixelMapHeader*             po_pPixelMapHeader,
                                                        HFCBinStream*                   pio_pFile);
    static bool            ReadPackBitsHeader         (PictPackBitsHeader*             po_pPackBitsHeader,
                                                        HFCBinStream*                   pio_pFile);
    static bool            ReadColorTable             (PictColorTable*                 po_pColorTable,
                                                        HArrayAutoPtr<PictRGBColor>&    po_rColorTableItems,
                                                        HFCBinStream*                   pio_pFile);


    static bool            WritePictFileHeader        (const PictFileHeader&           pi_rFileHeader,
                                                        const DataHeaderType&           pi_rDataHeaderType,
                                                        HFCBinStream*                   pio_pFile);
    static bool            WritePixelMapHeader        (const PictPixelMapHeader&       pi_rPixelMapHeader,
                                                        HFCBinStream*                   pio_pFile);
    static bool            WritePackBitsHeader        (const PictPackBitsHeader&       pi_rPackBitsHeader,
                                                        HFCBinStream*                   pio_pFile);
    static bool            WriteColorTable            (const PictColorTable&           pi_rColorTable,
                                                        const PictRGBColor*             pi_pColorTableItems,
                                                        HFCBinStream*                   pio_pFile);


    bool                   ReadHeaders                ();
    bool                   WriteHeaders               ();


    uint64_t                GetOffBytesToData          () const;
    uint64_t                GetOffBytesToColorTable    () const;


    // Create the file
    void                    SavePictFile               (bool                           pi_CloseFile);
    bool                   Create                     ();

    void                    SetPixelTypeToPage         (const HFCPtr<HRPPixelType>      pi_pPixelType);
    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile    () const;

    // Methods Disabled
    HRFPictFile                (const HRFPictFile&              pi_rObj);
    HRFPictFile&            operator=                  (const HRFPictFile&              pi_rObj);
    };


// Bmp Creator.
struct HRFPictCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile         (const HFCPtr<HFCURL>&       pi_rpURL,
                                                            uint64_t                   pi_Offset       = 0) const;

    // Identification information
    virtual WString                   GetLabel             () const;
    virtual WString                   GetSchemes           () const;
    virtual WString                   GetExtensions        () const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities      ();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create               (const HFCPtr<HFCURL>&       pi_rpURL,
                                                            HFCAccessMode               pi_AccessMode   = HFC_READ_ONLY,
                                                            uint64_t                   pi_Offset       = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPictCreator)


    // Disabled methodes
    HRFPictCreator                                          ();
    };
END_IMAGEPP_NAMESPACE

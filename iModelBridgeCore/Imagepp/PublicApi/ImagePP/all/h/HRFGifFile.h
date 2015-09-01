//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGifFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HFCBinStream.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFGifCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFGifCapabilities();
    };

//-----------------------------------------------------------------------------
// Struct - The GIF Global Color Table.
// The size of a color table may be any power of two from 2 to 256.
// We specify the maximum size here for clarity.
//-----------------------------------------------------------------------------

typedef struct _GifColorTable
    {
    Byte    Red;                   // Red Global Color Table Data.
    Byte    Green;                 // Green Global Color Table Data.
    Byte    Blue;                  // Blue Global Color Table Data.
    } GifColorTable[256];

//-----------------------------------------------------------------------------
//  The GIF header format.
//
//  This structure actually contains the header, logical screen
//  descriptor, and the global color table for the GIF image.
//-----------------------------------------------------------------------------

typedef struct GifHeader                // Offset   Description.
    {
    Byte              Signature[3];   //  00h     ID Signature.
    Byte              Version[3];     //  03h     Version Number.
    unsigned short     ScreenWidth;    //  06h     Logical Screen Width.
    unsigned short     ScreenHeight;   //  08h     Logical Screen Height.
    Byte              PackedField;    //  0Ah     Color Information.
    Byte              ColorIndex;     //  0Bh     Background Color Index.
    Byte              AspectRatio;    //  0Ch     Pixel Aspect Ratio.
    GifColorTable       GlobalCT;       //  0Dh     Global Color Table.
    } GifHeader;


//-----------------------------------------------------------------------------
//  The GIF Image Descriptor.
//-----------------------------------------------------------------------------
typedef struct GifImageDescriptor
    {
    Byte              ImageSeparator; // Image Descriptor identifier.
    unsigned short     ImageLeft;      // X position of image on the display.
    unsigned short     ImageTop;       // Y position of image on the display.
    unsigned short     ImageWidth;     // Width of the image in pixels.
    unsigned short     ImageHeight;    // Height of the image in pixels.
    Byte              PackedField;    // Image and Color Table Data Information.
    GifColorTable       LocalCT;        // Local Color Table.
    uint32_t            LineIndex;      // Internal information about the current line index.
    } GifImageDescriptor;


//-----------------------------------------------------------------------------
//  GIF 89a Graphic Control Extension Block
//-----------------------------------------------------------------------------
typedef struct GifGraphicControl
    {
    Byte              Introducer;         // Extension Introducer (always 21h).
    Byte              Label;              // Extension Label (always F9h).
    Byte              BlockSize;          // Size of Extension Block (always 04h).
    Byte              PackedField;        // Graphic Data Information.
    unsigned short     DelayTime;          // Delay time in hundredths of a second.
    Byte              ColorIndex;         // Transparent Color Index.
    Byte              Terminator;         // Block Terminator (always 0).
    } GifGraphicControl;

//-----------------------------------------------------------------------------
//  GIF 89a Graphic Block
//-----------------------------------------------------------------------------
typedef struct GifGraphicBlock
    {
    GifImageDescriptor  ImageDescriptor;
    GifGraphicControl   GraphicControl;
    } GifGraphicBlock;


//-----------------------------------------------------------------------------
//  GIF 89a Plain Text Extension Block
//-----------------------------------------------------------------------------
typedef struct GifPlainText
    {
    Byte              Introducer;         /* Extension Introducer (always 21h)    */
    Byte              Label;              /* Extension Label (always 01h)         */
    Byte              BlockSize;          /* Size of Extension Block (always 0Ch) */
    unsigned short     TextGridLeft;       /* X position of text grid in pixels    */
    unsigned short     TextGridTop;        /* Y position of text grid in pixels    */
    unsigned short     TextGridWidth;      /* Width of the text grid in pixels     */
    unsigned short     TextGridHeight;     /* Height of the text grid in pixels    */
    Byte              CellWidth;          /* Width of a grid cell in pixels       */
    Byte              CellHeight;         /* Height of a grid cell in pixels      */
    Byte              TextFgColorIndex;   /* Text foreground color index value    */
    Byte              TextBgColorIndex;   /* Text background color index value    */
    Byte*              PlainTextData;     /* Plain Text data sub-blocks           */
    Byte              Terminator;         /* Block Terminator (always 0)          */
    } GifPlainText;


//-----------------------------------------------------------------------------
//  GIF 89a Application Extension Block
//-----------------------------------------------------------------------------
typedef struct GifApplication
    {
    Byte              Introducer;         /* Extension Introducer (always 21h)    */
    Byte              Label;              /* Extension Label (always FFh)         */
    Byte              BlockSize;          /* Size of Extension Block (always 0Bh) */
    Byte              Identifier[8];      /* Application Identifier               */
    Byte              AuthentCode[3];     /* Application Authentication Code      */
    Byte*              ApplicationData;   /* Application data sub-blocks          */
    Byte              Terminator;         /* Block Terminator (always 0)          */
    } GifApplication;


//-----------------------------------------------------------------------------
//  GIF 89a Comment Extension Block
//-----------------------------------------------------------------------------
typedef struct GifComment
    {
    Byte              Introducer;         /* Extension Introducer (always 21h)    */
    Byte              Label;              /* Comment Label (always FEh)           */
    Byte*              CommentData;       /* Comment data sub-blocks              */
    Byte              Terminator;         /* Block Terminator (always 0)          */
    } GifComment;


class HRFGifFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Gif, HRFRasterFile)

    // allow to Open an image file
    HRFGifFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                uint64_t        pi_Offset = 0);

    virtual     ~HRFGifFile();

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

    virtual uint64_t                        GetFileCurrentSize() const;

protected:

    // Methods
    // Constructor use only to create a child
    //
    HRFGifFile          (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);
    virtual bool                       Open                ();
    virtual void                        CreateDescriptors   ();
    HFCBinStream*               GetFilePtr          ();

private:
    friend struct HRFGifCreator;
    friend class  HRFGifLineEditor;
    friend class  HRFGifImageEditor;

    // Members.
    HAutoPtr<HFCBinStream>  m_pGifFile;
    // Count of the number of images in the file.
    unsigned short         m_ImageCount;
    short m_AlphaColorIndex;

    bool                   m_HasComment;
    bool                   m_HasApplication;
    bool                   m_HasPlainText;

    HRPPixelPalette*        m_pGlobalPalette;

    // m_GifGraphicControl containt valid data to be used in next GifGraphicBlock.
    bool                   m_ValidGraphicControl;
    // Header information.
    GifHeader               m_GifHeader;
    // Graphic Control used in GifGraphicBlock struct.
    GifGraphicControl       m_GifGraphicControl;

    // List of offset from the begining of the file to the actual image data.
    vector<int32_t>           m_ListPageDataOffset;

    // List of min code size use for decompression.
    vector<short>          m_ListPageDecompressMinCodeSize;

    // List usedfull information to create the image descriptor.
    vector<GifGraphicBlock> m_ListGifGraphicBlock;

    // List of all genaral tag to be apply to each images in the file.
    HPMAttributeSet         m_GeneralTagList;

    // Histogram uses to indentify the background if possible between many
    // transparent color...in Creation mode only.
    // 256 entries
    HAutoPtr<uint32_t>        m_pHistoCreationMode;

    //Flag to know if the file has been modified
    bool                   m_RasterIsDirty;

    // Create the file
    void                    Close();
    bool                   Create();

    static Byte*          ReadDataSubBlocks    (HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster);
    static bool            ReadGifHeader        (GifHeader*          pio_pGifHeader,         HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster);
    static bool            ReadGifImageDesc     (GifImageDescriptor* pio_pGifImageDesc,      HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster);
    static bool            ReadGifComment       (GifComment*         pio_pGifComment,        HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster);
    static bool            ReadGifApplication   (GifApplication*     pio_pGifApplication,    HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster);
    static bool            ReadGifPlainText     (GifPlainText*       pio_pGifPlainText,      HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster);
    static bool            ReadGifGraphicControl(GifGraphicControl*  pio_pGifGraphicControl, HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster);

    bool                   LookUpBlocks         ();
    bool                   LookUpExtensionBlocks();
    bool                   AssignStructTo(HFCPtr<HRFPageDescriptor> pi_pPage);

    bool                   WriteGifHeader        (GifHeader*          pio_pGifHeader,         HFCBinStream* pi_pGifFile);
    bool                   WriteGifImageDesc     (GifImageDescriptor* pio_pGifImageDesc,      HFCBinStream* pi_pGifFile);
    bool                   WriteGifGraphicControl(GifGraphicControl*  pio_pGifGraphicControl, HFCBinStream* pi_pGifFile);

    bool                   WriteGifPlainText     (GifPlainText*       pi_pGifPlainText,
                                                   uint32_t             pi_SizeOfData,
                                                   HFCBinStream*       pio_pGifFile);

    bool                   WriteGifApplication   (GifApplication*     pi_pGifApplication,
                                                   uint32_t             pi_SizeOfData,
                                                   HFCBinStream*       pio_pGifFile);

    bool                   WriteGifComment       (GifComment*         pi_pGifComment,
                                                   uint32_t             pi_SizeOfData,
                                                   HFCBinStream*       pio_pGifFile);

    bool                   WriteDataSubBlocks    (uint32_t             pi_BufferSize,
                                                   Byte*             pi_pBuffer,
                                                   HFCBinStream*       pio_pGifFile);

    bool                   WritePalette          (GifColorTable       pi_pColorTable,
                                                   uint32_t             pi_TableSize,
                                                   HFCBinStream*       pio_pGifFile);

    void  SetHeader(unsigned short   pi_Width,
                    unsigned short   pi_Height,
                    unsigned short   pi_BitsColorResolution,
                    unsigned short   pi_BackgroundColor,
                    unsigned short   pi_BitsByPixel,
                    Byte            pi_AspectRatio,                     // TO DO
                    HRPPixelPalette*  pi_pPalette);

    void  SetGraphicControl(unsigned short    pi_DelayTime,              // TO DO
                            Byte             pi_TransparentColorIndex,
                            Byte             pi_DisposalMethode,        // TO DO
                            Byte             pi_UserInput,              // TO DO
                            GifGraphicControl* po_pGraphicControl);

    void  SetImageDesc(unsigned short     pi_LeftEdge,
                       unsigned short     pi_TopEdge,
                       unsigned short     pi_Width,
                       unsigned short     pi_Height,
                       unsigned short     pi_BitsColorResolution,
                       bool               pi_Interlaced,
                       unsigned short     pi_BitsByPixel,
                       HRPPixelPalette*    pi_pPalette,
                       GifImageDescriptor* po_pImageDescriptor);

    void  SetPalette (unsigned short   pi_BitsColorResolution,
                      unsigned short   pi_BitsByPixel,
                      HRPPixelPalette*  pi_pPalette,
                      GifColorTable     po_ColorTable);

    void SaveGifFile(bool pi_CloseFile);
    void SetDirty(bool pi_Dirty);


    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile(uint32_t pi_PageIndex) const;

    // Methods Disabled
    HRFGifFile(const HRFGifFile& pi_rObj);
    HRFGifFile&             operator= (const HRFGifFile& pi_rObj);
    };


// Gif Creator.
struct HRFGifCreator : public HRFRasterFileCreator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFGifCreator)

    // Disabled methodes
    HRFGifCreator();
    };
END_IMAGEPP_NAMESPACE



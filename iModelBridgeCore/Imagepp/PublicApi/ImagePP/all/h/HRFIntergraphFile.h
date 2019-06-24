//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/h/HArrayAutoPtr.h>
#include "HFCMacros.h"
#include "HRFMacros.h"

#include "HRFRasterFileCapabilities.h"
#include "HGFTileIDDescriptor.h"
#include "HCDCodecImage.h"
#include "HFCBinStream.h"
#include  <ImagePP/h/HAutoPtr.h>
#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelPalette;
class HGF2DTransfoModel;
class HGF2DIdentity;

#define HRF_INTERGRAPH_TILE_CODE                       65
#define HRF_INTERGRAGH_HEADER_BLOCK_LENGTH            512
#define HRF_INTERGRAPH_STANDARD_HEADER_BLOCK_QTY        2

#define HRF_INTERGRAPH_TYPICAL_READ_BUFFER_SIZE      1024
#define HRF_INTERGRAPH_CACHED_READ_STROKE             512

// Disable the automatic word alingnement for intergraph structure.
#pragma pack( push, IntergraphIdent,  1)

class HRFIntergraphFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Intergraph, HRFRasterFile)

    struct Creator : public HRFRasterFileCreator
        {
        Creator(HCLASS_ID pi_ClassID);
        virtual Utf8String GetSchemes () const;

        bool IsMultiPage(HFCBinStream& pi_rSrcFile, uint32_t pi_HeaderBlockcount) const;
        };

    HRFIntergraphFile (const HFCPtr<HFCURL>&  pi_rpURL,
                       HFCAccessMode   pi_AccessMode = HFC_READ_ONLY,
                       uint64_t       pi_Offset = 0);

    virtual        ~HRFIntergraphFile ();

    // World identification.
    const   HGF2DWorldIdentificator     GetWorldIdentificator () const override;

    // File manipulation
    bool                           AddPage (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    bool                           ResizePage(uint32_t pi_Page,
                                                           uint64_t pi_NewWidth,
                                                           uint64_t pi_NewHeight) override;

    HRFResolutionEditor*            CreateResolutionEditor (uint32_t      pi_Page,
                                                                        uint16_t pi_Resolution,
                                                                        HFCAccessMode pi_AccessMode) override;

    void                                Save() override;

    uint64_t                           GetFileCurrentSize() const override;

    void                                SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                       uint32_t pi_Page = 0,
                                                                       bool   pi_CheckSpecificUnitSpec = false,
                                                                       bool   pi_InterpretUnitINTGR = false) override;


    IMAGEPP_EXPORT bool                                HasLUTColorCorrection() const;
    IMAGEPP_EXPORT bool                                ResetLUT();


    static  bool                               GetIntergraphLUTApplyReset();           
    static  void                               SetIntergraphLUTApplyReset(bool value); 

    /*---------------------------------------------------------------------------------**//**
    * @bsiclass
    +---------------+---------------+---------------+---------------+---------------+------*/
    class LUTOverrideAccessLimitationGuard
        {
        public:
            IMAGEPP_EXPORT LUTOverrideAccessLimitationGuard(bool pi_OverrideAccessLimitation);
            IMAGEPP_EXPORT ~LUTOverrideAccessLimitationGuard();
        private:
            bool m_intergraphLUT_ApplyReset;
        };

protected:
    friend class HRFIntergraphLineEditor;
    friend class HRFIntergraphTileEditor;
    friend struct HRFIntergraphCreator;

    // For the following struct, we will not completely follow the HMR naming standard.
    // We will use the Intergraph standard for an easier link with their documentation.

    enum IntergraphColorTable                          // Palette type
        {
        NONE      = 0,                                 // No palette, grayscale
        IGDS      = 1,                                 // Palette 256 colors, ordered
        ENVIRON_V = 2                                  // Palette x colors, not ordered
        };

    enum IntergraphDataTypeCode                        // Sub type of intergraph format.
        {
        COT            = 2,                            // 8 Bit Palette Not compress
        RLE            = 9,                            // 1 Bit Run length encoded compression
        CRL            = 10,                           // 8 Bit Palette Run length encoded (not the same as COMPRESS_8BITS)
        CCITT          = 24,                           // 1 Bit CCITT group 4 compression
        RBG_COMPRESS   = 27,                           // 24 Bits compress RLE8
        RBG_UNCOMPRESS = 28,                           // 24 Bits Not compress
        COMPRESS_8BITS = 29,                           // 8 Bit Palette Run length encoded compression
        JPEG_GRAYSCALE = 30,                           // 8  Bits gray scale compress JPeg
        JPEG_RBG       = 31                            // 24 Bits compress JPeg
        };

    // Intergraph Palette Environ-V structure...
    struct EnvironV                                    // Palette x colors, not ordered
        {
        uint16_t Slot;                           // Palette index
        uint16_t Red;                            // Red   component value
        uint16_t Green;                          // Green component value
        uint16_t Blue;                           // Blue  component value
        };

    // Intergraph Header Block 1 File structure...
    struct IntergraphHeaderBlock1
        {
        uint16_t htc;                                   // Header type code
        uint16_t wtf;                                   // Word to follow
        uint16_t dtc;                                   // Data type code
        uint16_t utc;                                   // Application type
        double xori;                                  // X view origin          xor generate a conflict in Android.
        double yor;                                   // Y view origin
        double zor;                                   // Z view origin
        double xdl;                                   // X view extent
        double ydl;                                   // Y view extent
        double zdl;                                   // Z view extent
        double trn[16];                               // Transformation matrix
        uint32_t ppl;                                   // Pixel per line
        uint32_t nol;                                   // Number of line
        int16_t drs;                                   // Device resolution
        Byte  slo;                                   // Scan line orietation
        Byte  scn;                                   // Scannable flag
        double rot;                                   // Rotation angle
        double skw;                                   // Skew angle (anorthogonality)
        uint16_t dtm;                                   // Data type modifier
        Byte   dgn[66];                               // Design file name
        Byte   dbs[66];                               // Database file name
        Byte   prn[66];                               // Parent grid file name
        Byte   des[80];                               // File description
        double min;                                   // Minimum value
        double max;                                   // Maximum value
        Byte   rv1[3];                                // Reserved
        Byte  ver;                                   // Grid file version
        };

    // Intergraph Header Block 2 File structure...
    struct IntergraphHeaderBlock2
        {   Byte  gan;                                   // Gain
        Byte  oft;                                   // Offset/threshold
        Byte  vf1;                                   // View # - Screen #1
        Byte  vf2;                                   // View # - Screen #2
        Byte  vno;                                   // View Number
        Byte  rv2;                                   // Reserved
        uint16_t rv3;                                   // Reserved
        double asr;                                   // Aspect ratio
        uint32_t cfp;                                   // Concatenate file pointer
        uint16_t ctv;                                   // Color table type
        uint16_t rv8;                                   // Reserved
        uint32_t cte;                                   // Number of Color table type entires
        uint32_t app;                                   // Application packet pointer
        uint32_t apl;                                   // Application packet length
        uint16_t rv9[110];                              // Reserved
        uint16_t use[128];                              // Application data
        };

    struct IntergraphHeaderBlocks
        {
        IntergraphHeaderBlock1 IBlock1;
        IntergraphHeaderBlock2 IBlock2;
        };

    // Intergraph Tile Entry structure...
    struct TileEntry                                   // Tile entries sructure
        {
        uint32_t S;                                      // Starting offset
        uint32_t A;                                      // Allocated length
        uint32_t U;                                      // Used Length
        };

    // Intergraph Tile Directory structure...
    struct TileDirectoryInfo
        {
        uint16_t ApplicationType;
        uint16_t SubTypeCode;
        uint32_t WordsToFollow;
        uint16_t PacketVersion;
        uint16_t Identifier;
        uint16_t Reserved1[2];
        uint16_t Properties;
        uint16_t DataTypeCode;
        Byte Reserved2[100];
        uint32_t TileSize;
        uint32_t Reserved3;
        };

    // Intergraph application packet structure...
    struct ApplicationPacket
        {
        uint16_t ApplicationType;
        uint16_t SubTypeCode;
        uint32_t WordsToFollow;
        uint16_t PacketVersion;
        uint16_t Identifier;
        uint32_t Reserved;
        uint32_t NumberOverview;
        };

    struct GenericApplicationPacket
        {
        uint16_t ApplicationType;
        uint16_t SubTypeCode;
        uint32_t WordsToFollow;
        };

    // Intergraph JPEG packet structure...
    struct JpegPacketPacket
        {
        uint16_t ApplicationType;
        uint16_t SubTypeCode;
        uint32_t WordsToFollow;
        uint16_t Mode;
        uint16_t QualityFactor;
        uint16_t Precision;
        uint16_t NumTables;
        Byte   Reserved[56];
        Byte   QTables[1];
        };

    // Intergraph overview packet structure...
    struct Overview
        {
        uint32_t NumberLines;
        uint32_t NumberPixels;
        uint16_t SamplingMethod;
        uint16_t Flag;
        uint32_t Reserved;
        };

    struct IntergraphResolutionDescriptor
        {
        Overview*             pOverview;
        TileEntry*            pOverviewEntry;
        TileDirectoryInfo     TileDirectory;
        TileEntry*            pTileDirectoryEntry;
        HFCPtr<HCDCodecImage> pCodec;
        };

    struct StreamFreeBlock
        {
        uint32_t Offset;
        uint32_t Size;


        StreamFreeBlock(uint32_t pi_Offset, uint32_t pi_Size)
            {
            Offset = pi_Offset;
            Size = pi_Size;
            };

        StreamFreeBlock()
            {
            Offset = 0;
            Size = 0;
            };

        bool operator==(const StreamFreeBlock& pi_rObj) const
            {
            return (Offset == pi_rObj.Offset);
            };

        bool operator!=(const StreamFreeBlock& pi_rObj) const
            {
            return (Offset != pi_rObj.Offset);
            };

        bool operator<(const StreamFreeBlock& pi_rObj) const
            {
            return (Offset < pi_rObj.Offset);
            };

        bool operator>(const StreamFreeBlock& pi_rObj) const
            {
            return (Offset > pi_rObj.Offset);
            };
        };

    typedef list<StreamFreeBlock, allocator<StreamFreeBlock> > ListOfFreeBlock;

    typedef vector<IntergraphResolutionDescriptor*, allocator<IntergraphResolutionDescriptor* > >
    IntergraphResolutionDescriptors;

    typedef vector<IntergraphHeaderBlocks*, allocator<IntergraphHeaderBlocks*> > IntergraphHeaderArray;

    typedef Byte  IntergraphHeaderBlockN[HRF_INTERGRAGH_HEADER_BLOCK_LENGTH];
    typedef Byte* IntergraphHeaderBlockP;

    typedef vector<IntergraphHeaderBlockP , allocator<IntergraphHeaderBlockP > >
    IntergraphGenericHeaderBlock;

    // Constructor use only to create a child
    HRFIntergraphFile (const HFCPtr<HFCURL>& pi_rpURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset,
                       bool                 pi_DontOpenFile);

    virtual void    GetTransfoModel      () = 0;
    virtual bool   SetGlobalTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) = 0;

    virtual void    Close ();
    void            IntergraphTagUpdate (HFCPtr<HRFPageDescriptor> pi_pPageDescriptor);

    virtual bool   Open ();
    virtual bool   Create ();
    bool           InitOpenedFile      (HFCPtr<HRFPageDescriptor> pi_pPage);
    bool           HasTileAccess       (uint16_t pi_SubImage) const;
    bool           HasImageAccess      (uint16_t pi_SubImage) const;
    bool           HasLineAccess       (uint16_t pi_SubImage) const;

    bool           GetLUTColorCorrection(uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader);

    const Byte*   GetRedLUTColorTablePtr  () const;
    const Byte*   GetGreenLUTColorTablePtr() const;
    const Byte*   GetBlueLUTColorTablePtr () const;

    uint32_t        CalcNumberOfPage    () const;
    uint32_t        GetWidth            (uint16_t pi_SubImage) const;
    uint32_t        GetHeight           (uint16_t pi_SubImage) const;
    uint32_t        GetTileWidth        (uint16_t pi_SubImage) const;
    uint32_t        GetTileHeight       (uint16_t pi_SubImage) const;
    uint32_t        GetBlockNumInHeader () const;

    uint16_t CountSubResolution  () const;
    uint16_t GetBitPerPixel      () const;
    const uint16_t GetDatatypeCode     () const;

    double         GetResolution       (uint16_t pi_SubImage);

    void            SetBitPerPixel      (uint16_t pi_BitPerPixel);
    void            SetDatatypeCode     (uint16_t pi_DataTypeCode);
    void            GetTileDirectory    (uint16_t     pi_Resolution,
                                         TileDirectoryInfo  pi_TileDirectory,
                                         TileEntry*         pi_pTileDirectoryEntry);

    uint32_t         GetFullResolutionSize       ();
    void            UpdateOffsetNextResolutions (uint16_t pi_CurrentSubImage, uint32_t pi_NbByteToAdd);

    HFCPtr<HRPPixelType>
    GetPixelType        () const;

    const   HFCBinStream*
    GetIntergraphFilePtr() const;

    const   HRFScanlineOrientation
    GetScanlineOrientation() const;

    uint32_t        GetpageOffset       (uint32_t pi_PageIndex);

    bool           IsHeaderFilled        () const;
    bool           FillFileHeader        ();
    bool           Set8BitsPalette       (uint32_t          pi_PaletteType,
                                           HRPPixelPalette* po_pPalette);

    bool           CreateFileHeader      (HFCPtr<HRFPageDescriptor> pi_pPage);
    bool           CreateTileDirectory   (uint16_t pi_SubImage,
                                           HRFResolutionDescriptor* pi_pResolutionDescriptor);
    bool           WriteTileDirectory    (uint16_t pi_SubImage);
    bool           CreatePacketOverview  (HFCPtr<HRFPageDescriptor> pi_pPage);
    bool           UpdatePacketOverview  (uint32_t pi_FileCursorPosition, uint32_t pi_Resolution);
    bool           HasCompression ();
    bool           WriteTransfoModel     (const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);
    bool           WriteFileHeader       (HFCPtr<HRFPageDescriptor> pi_pPage);

    IntergraphColorTable
    AnalysePalette        (const HRPPixelPalette& po_pPalette);

    void            CheckNotNullTransfoModel ();
    void            ReadTileDirectory     (uint16_t pi_SubImage);
    void            GenerateHeader8BitsPalette
    (IntergraphColorTable   ColorTableValue,
     const HRPPixelPalette& po_pPalette);
    void            CreateHeaderBlock1    (HRFResolutionDescriptor*  pi_pResolutionDescriptor);
    void            CreateHeaderBlock2    (IntergraphHeaderBlocks& po_rIntergraphHeaderBlocks) const;
    void            InitScanlineOrientation();
    void            SetPalette            (const HRPPixelPalette& pi_rPalette);

    bool           IsSingleColor();
    bool           IsIdentityLUT() const;

    bool           ReadAllApplicationPacket();
    bool           GetLUTColorCorection(uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader);
    bool           GetJpegAppPacket    (uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader);
    bool           GetPacketOverview   (uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader);
    bool           GetHuffmanPacket    (uint32_t pi_FileOffset, const GenericApplicationPacket& pi_PacketHeader);

    // Static Member
    static bool                    m_sIntergraphLUT_ApplyReset;

    IntergraphHeaderBlocks          m_IntergraphHeader;
    IntergraphGenericHeaderBlock    m_pIntergraphBlockSupp;
    IntergraphResolutionDescriptors m_IntergraphResDescriptors;

    HFCPtr<HGF2DTransfoModel>       m_pTransfoModel;
    HRFScanlineOrientation          m_ScanlineOrientation;

    ApplicationPacket*              m_pApplicationPacket;
    JpegPacketPacket*               m_pJpegPacketPacket;

    ListOfFreeBlock                 m_ListOfFreeBlock;

    HAutoPtr<HFCBinStream>          m_pIntergraphFile;
    HFCPtr<HRPPixelType>            m_pPixelType;
    HFCPtr <HGF2DTransfoModel>      m_SLOModel;

    uint16_t                 m_BitPerPixel;
    uint32_t                         m_CurentPageIndex;

    bool                           m_HasLineAcess;
    bool                           m_HasTileAccess;
    bool                           m_HasTransfoModel;
    bool                           m_HasHeaderFilled;
    bool                           m_ZeroMatrixFound;

    uint32_t                         m_SubResolution;
    int32_t                         m_CurrentSubImage;
    int32_t                         m_BlockNumInHeader;
    int32_t                         m_BlockHeaderAddition;

    uint16_t                 m_DataTypeCode;

    bool                           m_OverviewCountChanged;
    bool                           m_LUTColorCorrected;
    uint32_t                        m_LUTColorPacketOffset;

    Byte*                         m_pRedLUTColorTable;
    Byte*                         m_pGreenLUTColorTable;
    Byte*                         m_pBlueLUTColorTable;

private:

    // Disabled Methods
    HRFIntergraphFile (const HRFIntergraphFile& pi_rObj);
    HRFIntergraphFile& operator= (const HRFIntergraphFile& pi_rObj);
    };



// Re-enable the automatic word alingnement by removing the previous pragma instruction
// at the begining of this class
#pragma pack( pop, IntergraphIdent)

END_IMAGEPP_NAMESPACE

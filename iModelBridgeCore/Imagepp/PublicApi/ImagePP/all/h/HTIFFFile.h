//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFFile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTIFFFile
//-----------------------------------------------------------------------------

#pragma once

#include "HTagFile.h"
#include "HFCAccessMode.h"

BEGIN_IMAGEPP_NAMESPACE
// Special value for the channel index. It indicates that all channels are concerned
static const uint32_t  CHANNEL_INDEX_ALL_CHANNELS         = UINT32_MAX;

class HTIFFDirectory;
class HPMAttributeSet;
class HFCURL;
class HCDPacket;
class HCDCodec;
class HTIFFError;
class HTIFFGeoKey;

// Data Type
//
typedef struct Tag_RATIONAL
    {
    double Value;
    } RATIONAL;

// Macro
#define FILL_VECTOR_WITH_PTR_VALS(pi_Vec, pi_Count, pi_ValPointer) \
    {\
        pi_Vec.assign(pi_ValPointer, pi_ValPointer + pi_Count);\
    }\

class HTIFFFile : public HTagFile
    {
public:
    // Primary methods

    IMAGEPP_EXPORT HTIFFFile   (const WString&         pi_pFilename,
                        HFCAccessMode          pi_Mode,
                        uint64_t              pi_OriginOffset = 0,
                        bool                  pi_CreateBigTifFormat = false,
                        bool                  pi_ValidateDir = true,
                        bool*                 pi_NewFile = 0);

    HTIFFFile   (const HFCPtr<HFCURL>&  pi_rpURL,
                 uint64_t               pi_offset,
                 HFCAccessMode          pi_AccessMode = HFC_READ_ONLY,
                 bool                  pi_CreateBigTifFormat = false,
                 bool*                 pi_NewFile = 0);

    virtual             ~HTIFFFile  ();

    uint32_t            NumberOfPages       () const;
    bool               SetPage             (uint32_t pi_Page);


    uint32_t            ScanlineSize        () const;
    IMAGEPP_EXPORT uint32_t     StripSize           () const;
    size_t              StripBlockSize      (uint32_t pi_Strip) const;

    IMAGEPP_EXPORT uint32_t     NumberOfStrips      () const;
    uint32_t            NumberOfTiles       () const;
    IMAGEPP_EXPORT uint32_t     TileSize            () const;
    size_t              TileBlockSize       (uint32_t pi_PosX, uint32_t pi_PosY) const;
    IMAGEPP_EXPORT bool         IsTiled             () const;

    IMAGEPP_EXPORT HSTATUS      TileRead            (Byte* po_pData, uint32_t pi_PosX, uint32_t pi_PosY);
    HSTATUS             TileWrite           (const Byte* pi_pData, uint32_t pi_PosX, uint32_t pi_PosY);
    IMAGEPP_EXPORT HSTATUS      StripRead           (Byte* po_pData, uint32_t pi_Strip);
    HSTATUS             StripWrite          (const Byte* pi_pData, uint32_t pi_Strip);

    // Compressed Data access
    //
    HSTATUS             TileWriteCompress   (const Byte* pi_pData, uint32_t pi_DataLen, uint32_t pi_PosX, uint32_t pi_PosY);
    HSTATUS             StripWriteCompress  (const Byte* pi_pData, uint32_t pi_DataLen, uint32_t pi_Strip);

    // Data access with Packets
    HSTATUS             TileRead            (HFCPtr<HCDPacket>& po_prPacket, uint32_t pi_PosX, uint32_t pi_PosY);
    HSTATUS             StripRead           (HFCPtr<HCDPacket>& po_pPacket, uint32_t pi_Strip);

    // GeoTIFF, Get GeoKey Object, Interpretation of GeoKey Tags
    HTIFFGeoKey&        GetGeoKeyInterpretation();

    bool               GetConvertedField (HTagID pi_Tag, vector<double>& po_rValues) const;

    // Make hidden HTagFile overloads visible
    using HTagFile::GetField;
    using HTagFile::SetField;

    IMAGEPP_EXPORT bool        GetField (HTagID pi_Tag, unsigned short* po_pVal) const;
    IMAGEPP_EXPORT bool        GetField (HTagID pi_Tag, uint32_t* po_pVal) const;
    bool               GetField (HTagID pi_Tag, uint32_t* po_pCount, unsigned short** po_ppVal) const;

    bool               GetField (HTagID pi_Tag, unsigned short** po_ppVal1, unsigned short** po_ppVal2, unsigned short** po_ppVal3) const;
    bool               GetField (HTagID pi_Tag, RATIONAL* po_pVal) const;
    bool               GetField (HTagID pi_Tag, uint32_t* po_pCount, RATIONAL* po_pVal) const;


    bool               SetField (HTagID pi_Tag, unsigned short pi_Val);
    bool               SetField (HTagID pi_Tag, uint32_t pi_Val);
    bool               SetField (HTagID pi_Tag, uint32_t pi_Count, const unsigned short* pi_pVal);
    bool               SetField (HTagID pi_Tag, const unsigned short* pi_pVal1, const unsigned short* pi_pVal2, const unsigned short* pi_pVal3);
    bool               SetField (HTagID pi_Tag, RATIONAL pi_Val);

    void                SetInterpretMaxSampleValue(bool    pi_Interpret);

    // Needed to support old HMR library
    void                FillAllEmptyDataBlock();


    // Special for AltaPhoto project
#if 0
    bool               ReadAltaPhotoBlob   (Byte* po_pData, uint32_t* po_pSize) const;
    bool               WriteAltaPhotoBlob  (const Byte* pi_pData, uint32_t pi_pSize);
#endif
    bool               ReadProjectWiseBlob(uint32_t pi_Page, Byte* po_pData, uint32_t* po_pSize) const;
    bool               WriteProjectWiseBlob(uint32_t pi_Page, const Byte* pi_pData, uint32_t pi_pSize);

    // Method use to implement the concurrence.
    // Return an offset of UInt32(4 bytes) in the file.
// Disable        UInt32              GetSynchroOffsetInFile  () const;


    // Special for Itiff compression Wavelet Only.
    HTIFFStream*        GetInfoFileData(uint64_t* po_Offset);

    // Special file types, simulated sequential access
    // With this file type: ReadOnly + Sequential access + single resolution.
    bool               IsSimulateLine_OneStripPackBitRGBAFile() const;

    //EXIF Tags Access Functions
    bool               GetEXIFDefinedGPSTags(uint32_t         pi_PageDirInd,
                                              HPMAttributeSet& po_rExifGpsTags);

    bool               GetEXIFTags(uint32_t         pi_PageDirInd,
                                    HPMAttributeSet& po_rExifGpsTags);

    IMAGEPP_EXPORT void         PrintEXIFDefinedGPSTags(uint32_t pi_PageDirInd,
                                                FILE*  po_pOutput);

    IMAGEPP_EXPORT void         PrintEXIFTags(uint32_t pi_PageDirInd,
                                      FILE*  po_pOutput);

    //Compact the iTIFF file, removing freeblocks
    bool                CompactITIFF();

    // Utility function
    //
    static HTIFFFile* UndoRedoFile(const WString& pi_rFilename, HFCAccessMode pi_Mode);

protected:

    //Check if the info for accessing currently the striped or tiled image
    //are correct. Simplify and correct the information when there are too much
    //available.
    bool                ValidateAndCorrectBlocInfo();

private:
    unsigned short     m_FillOrder;    // System Bit ordering.
    unsigned short*            m_pBitsBySample;
    uint32_t            m_NbSampleFromFile; // Sample store in file... can be
    // diff from m_SamplesByPixel
    uint32_t            m_SamplesByPixel;
    uint32_t            m_RowsByStrip;
    uint32_t            m_ImageWidth;       // Image width update when the resoltion change.
    uint32_t            m_ImageLength;      // Image length update when the resoltion change.
    uint32_t            m_CompressionQuality;
    unsigned short     m_BitsByPixel;      // Number of Bits by pixel.
    unsigned short     m_PlanarConfig;
    unsigned short     m_Photometric;

    bool               m_IsCompress;       // Data is compressed.
    uint32_t            m_StripTileSize;    // Size in Bytes




    //------------------------------------------------------------------------------------------------

    bool               m_HasBlockFlags;
    bool               m_HasHMRFormat;
    bool               m_HasiTiffFormat;
    bool               m_StripAreSimulated;    // true if the strips are simulated

    bool               m_InterpretMaxSampleValue;
    unsigned short     m_NbBitUsed;            // Only use with 16 bits per channel

    HFCPtr<HCDPacket>   m_pPacket;              // a packet for the compression

    bool               m_UndoRedoFileMode;

    // Offset in the file, LONG(4bytes), used as a Counter by the concurrence
    // system.
// Disable        UInt32              m_SynchroOffset;

    HSTATUS             (HTIFFFile::*m_pCompressFunc)(const Byte* pi_pSrcBuffer,
                                                      uint32_t      pi_SrcLen,
                                                      const HFCPtr<HCDPacket>& pio_rpPacket);
    HSTATUS             (HTIFFFile::*m_pUncompressFunc)(const HFCPtr<HCDPacket>& pi_rpPacket,
                                                        Byte*       po_pDstBuffer,
                                                        uint32_t*       po_pDstLen);
    HSTATUS             (HTIFFFile::*m_pSetHeightFunc)(uint32_t pi_Height);



    // Support special case
    //    see method IsSimulateLine_OneStripPackBitRGBAFile for comment
    bool                       m_SimulateLine_OneStripPackBitRGBA;
    uint32_t                    m_CurrentLineSimulateLine;
    HFCPtr <HCDPacket>          m_pCompressedPacketSimulateLine;

    // Metbods

    // Not implemented
    HTIFFFile   (const HTIFFFile& pi_rObj);
    HTIFFFile&     operator=(const HTIFFFile& pi_rObj);

    void            Initialize          ();

    virtual MagicNumber
    GetLittleEndianMagicNumber     () const override;
    virtual MagicNumber
    GetBigEndianMagicNumber        () const override;

    virtual HTagID  GetPacketOffsetsTagID      () const override;
    virtual HTagID  GetPacketByteCountsTagID   () const override;

    virtual uint32_t _NumberOfDirectory  (HTagFile::DirectoryType pi_DirType = STANDARD) const override;


    virtual bool   IsValidTopDirectory (HTIFFError**            po_ppError) override;

    virtual void    OnTopDirectoryFirstInitialized () override;

    virtual bool   DirectoryIsValid    (HTIFFDirectory*        pi_Dir,
                                         HTIFFDirectory*        pi_pCurPageDir) override;

    virtual bool   IsValidReducedImage (HTIFFDirectory*        pi_ReducedImageDir,
                                         HTIFFDirectory*        pi_pCurPageDir) override;

    virtual void    _PrintCurrentDirectory
    (FILE* po_pOutput, uint32_t pi_Flag) override;

    IMAGEPP_EXPORT bool    IsTiled             (HTIFFDirectory*        pi_Dir) const;

    IMAGEPP_EXPORT uint32_t TileSizePrivate     () const;
    IMAGEPP_EXPORT uint32_t StripSizePrivate    () const;

    uint32_t        ComputeStrip        (uint32_t pi_Line) const;
    uint32_t        ComputeTile         (uint32_t pi_PosX, uint32_t pi_PosY) const;
    void            InitStripList       ();
    void            SimulateStripList   (uint32_t pi_CompressMode);
    HSTATUS         ReadData            (Byte* po_pData, uint32_t pi_StripTile);
    HSTATUS         ReadDataWithPacket  (HFCPtr<HCDPacket>& po_pPacket, uint32_t pi_StripTile);
    HSTATUS         WriteData           (const Byte* pi_pData, uint32_t pi_StripTile, uint32_t pi_pCompressSize=0);

    // Separate planes methods
    HSTATUS         ReadSeparateOrContigData (HFCPtr<HCDPacket>& po_pPacket, uint32_t pi_StripTile);
    HSTATUS         ReadSeparateData    (HFCPtr<HCDPacket>& po_rpPacket, uint32_t pi_StripTile);

    HSTATUS         WriteSeparateOrContigData(const Byte* pi_pData, uint32_t pi_StripTile, uint32_t pi_pCompressSize=0);
    HSTATUS         WriteSeparateData   (const Byte* pi_pData, uint32_t pi_StripTile, uint32_t pi_pCompressSize);

    virtual void    _PostReallocDirectories
    (uint32_t pi_NewDirCountCapacity) override;

    virtual void    _PreWriteCurrentDir(HTagFile::DirectoryID pi_DirID) override;

    virtual bool   PreCurrentDirectoryChanged
    (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;
    virtual bool   OnCurrentDirectoryChanged
    (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;
    virtual bool   PostCurrentDirectorySet
    (HTagFile::DirectoryID pi_DirID, bool pi_ReloadCodec) override;

    // HTIFFFileCoDec.cpp
    void            SetNoneAlgo         ();
    void            SetDeflateAlgo      (uint32_t pi_BitsPerPixel=8, uint16_t pi_Predictor=1/*none*/, uint32_t pi_SamplesPerPixel=1);
    void            SetRLE1Algo         ();
    void            SetPackBitsAlgo     (uint32_t pi_BitsPerPixel);
    void            SetLuraWavePaddedAlgo    (uint32_t pi_BitsPerPixel);
    void            SetLuraWaveNonPaddedAlgo (uint32_t pi_BitsPerPixel);
    void            SetJPEGAlgo         (uint32_t pi_BitsPerPixel);
    void            SetCCITTAlgo        (uint32_t pi_CompressMode, bool pi_BitRev);
    void            SetFlashpixAlgo     (uint32_t pi_BitsPerPixel);
    void            SetQualityToCodec   ();
    void            SetLZWAlgo(uint32_t pi_BitsPerPixel, unsigned short pi_Predictor, uint32_t pi_SamplesPerPixel);
    void            SetJPEGKodakAlgo    ();

    
    HSTATUS         CompressBlock       (const Byte* pi_pSrcBuffer,
                                         uint32_t      pi_SrcLen,
                                         const HFCPtr<HCDPacket>& pio_rpPacket);
    HSTATUS         UncompressBlock     (const HFCPtr<HCDPacket>& pi_rpPacket,
                                         Byte*       po_pDstBuffer,
                                         uint32_t*       po_pDstLen
                                        );
    HSTATUS         CompressBlockRLE1   (const Byte* pi_pSrcBuffer,
                                         uint32_t      pi_SrcLen,
                                         const HFCPtr<HCDPacket>& pio_rpPacket);
    HSTATUS         UncompressBlockRLE1 (const HFCPtr<HCDPacket>& pi_rpPacket,
                                         Byte*       po_pDstBuffer,
                                         uint32_t*       po_pDstLen
                                        );
    HSTATUS         SetHeight           (uint32_t pi_Height);
    size_t          GetSubsetMaxCompressedSize
    (size_t pi_CurrentSize);


    void            ExtractWidthHeight  (uint32_t* po_pWidth, uint32_t* po_pHeight);


    void            PrepareForJPEG      (Byte* pio_pData,
                                         uint32_t pi_Width,
                                         uint32_t pi_Height,
                                         uint32_t pi_RelevantWidth,
                                         uint32_t pi_RelevantHeight,
                                         uint32_t pi_BitsPerPixel);

    bool           IsAllSamplesWithSameBitsCount() const;

    void            Treat16bitPerChannelForRead(unsigned short* pio_pData, size_t pi_DataCoutn) const;
    void            Treat16bitPerChannelForWrite(unsigned short* pio_pData, size_t pi_DataCoutn) const;
    void            Treat32bitPerChannelForRead(uint32_t* pio_pData, size_t pi_DataCount) const;

    void            ComputeNbBitUsed();

    HFCPtr<HCDCodec>    GetCurrentCodec     () const;       // Do not make public because codec is specially setup for separate planar. i.e. we use bits per sample instead of bits per pixel.



    void                SetTileData                     ();


    void                ReadWriteNewPosition            (uint64_t&  p_CountFreeBlockTotal,
                                                         uint32_t  p_PositionSortedData,
                                                         uint32_t  p_StartPositionSortedData,
                                                         uint64_t p_CountFreeBlock);

    void                SetDirectoryTouched            ();


    // Disable        bool           SetSynchroField();
    };


END_IMAGEPP_NAMESPACE


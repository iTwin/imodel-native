//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSLOStripEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFSLOStripEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HCDPacket.h"
#include "HRFTypes.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFSLOStripEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFSLOStripAdapter;

    virtual         ~HRFSLOStripEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              Byte*  po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>& po_rpPacket,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const Byte* pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const HFCPtr<HCDPacket>&  pi_rpPacket,
                               HFCLockMonitor const*     pi_pSisterFileLock = 0) override;

protected:
    HAutoPtr<HRFResolutionEditor> m_pSrcResolutionEditor;

    HRFSLOStripEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t pi_Page,
                      unsigned short pi_Resolution,
                      HFCAccessMode pi_AccessMode,
                      HRFResolutionEditor* pi_pResolutionEditor);


private:

    // Predefine bit mask table, see initialisation in the HRFSLOStripEditor.cpp
    static Byte   m_BitMask[8];

    bool           m_AllImageRead;             // Flag to indicate if the image has been stored in memory
    Byte          m_BitsPerPixel;             // How many pixels per byte for the original image
    uint32_t        m_AdaptorWidth;             // Width of the image after transformation
    uint32_t        m_AdaptorHeight;            // Height of the image after transformation
    uint32_t        m_AdaptorBlockHeight;       // Block Height of the image after transformation
    uint32_t        m_AdaptorBlockWidth;        // Block Width of the image after transformation
    uint32_t        m_AdaptorBlockPerWidth;     // Number of blocs per width of the transformed image
    uint32_t        m_AdaptorBlockPerHeight;    // Number of blocs per height of the transformed image

    uint32_t        m_SrcImageHeight;           // Height of the original image
    uint32_t        m_SrcImageWidth;            // Width of the original image
    uint32_t        m_SrcImageBlockHeight;      // Block Height of the original image
    uint32_t        m_SrcImageStripHeight;      // Height of a source strip
    uint32_t        m_SrcImageBytesPerWidth;    // Number of bytes per lines of the Original image

    uint32_t        m_SrcStripsHeight;
    uint32_t        m_SrcStripsWidth;
    uint32_t        m_SrcStripsPerHeight;
    uint32_t        m_SrcStripsPerWidth;
    uint32_t        m_SrcStripsBytesPerWidth;
    uint32_t        m_SrcStripsPadding;


    typedef HArrayAutoPtr<HArrayAutoPtr<HFCPtr<HCDPacket> > >    BlockTable;

    BlockTable      m_ppBlocks;
    BlockTable      m_ppTempBlocks;

    HRFScanlineOrientation  m_ScanLineOrientation;

    HSTATUS         BuildRLE1Image                      (BlockTable& po_ppBlocks);

    HSTATUS         BuildRLE1HorizontalStrippedImage    (BlockTable& pio_ppBlock);

    HSTATUS         BuildRLE1VerticalStrippedImage      (BlockTable& pio_ppPackets);

    HSTATUS         BuildHorizontalStrips       (BlockTable& po_rpppBlocks,
                                                 BlockTable& pi_rpppBlocks,
                                                 uint32_t    pi_height,
                                                 uint32_t    pi_width,
                                                 uint32_t    pi_StripHeight);

    HSTATUS         Read1BitBlock               (uint32_t   x,
                                                 uint32_t   y,
                                                 Byte*    pio_pData);

    void            Compress1BitData            (const Byte*      pi_pData,
                                                 HFCPtr<HCDPacket>& pio_pPacket);

    void            TransposeRLE1Packet         (HFCPtr<HCDPacket>& pio_pPacket,
                                                 bool              pi_Flip,
                                                 bool              pi_Swap);

    void            TransposeRLE1Line           (Byte*   po_pTransposedData,
                                                 unsigned short*  pi_pSourceRLE1Line,
                                                 uint32_t  pi_LineNumber,
                                                 uint32_t  pi_NumberOfPixels,
                                                 uint32_t  pi_TransposedDataWidthInBytes);

    void            TransposeAndFlipRLE1Line    (Byte*   po_pTransposedData,
                                                 unsigned short*  pi_pSourceRLE1Line,
                                                 uint32_t  pi_LineNumber,
                                                 uint32_t  pi_NumberOfPixels,
                                                 uint32_t  pi_TransposedDataWidthInBytes);

    void            Transpose1BitBuffer         (Byte*   pi_pSource,
                                                 Byte*   po_ppDest,
                                                 uint32_t  pi_dimX,
                                                 uint32_t  pi_dimY);

    void            TransposeAndFlip1BitBuffer  (Byte*   pi_pSource,
                                                 Byte*   po_ppDest,
                                                 uint32_t  pi_dimX,
                                                 uint32_t  pi_dimY);

    void            HorizontalSwapRLE1Packet    (HFCPtr<HCDPacket>& pio_pPacket);

    void            TransformRLE1PacketToSlo4   (HFCPtr<HCDPacket>& pio_pPacket);

    void            Transform1BitBuffer         (Byte* po_pTransformedData,
                                                 Byte* pi_pSourceData,
                                                 uint32_t pi_dimX,
                                                 uint32_t pi_dimY);

    void            GetAdaptorBlockIndex        (uint32_t   pi_PosBlockX,
                                                 uint32_t   pi_PosBlockY,
                                                 uint32_t*    po_pIndexX,
                                                 uint32_t*    po_pIndexY);

    uint32_t        GetSrcStripHeight           (uint32_t x,
                                                 uint32_t y );

    uint32_t        CalcPadding                 (uint32_t pi_TotalWidth,
                                                 uint32_t pi_Width );

    void            AllocBlockTable             (BlockTable& pio_ppBlock,
                                                 uint32_t    pi_Line,
                                                 uint32_t    pi_Column);

    void            DeleteBlockTable            (BlockTable& pio_ppBlock);

    void            AddBlock                    (BlockTable&        pio_ppBlock,
                                                 HFCPtr<HCDPacket>& pi_pPaacket,
                                                 uint32_t           pi_pAdaptorXIndex,
                                                 uint32_t           pi_pAdaptorYIndex);

    void            TransformRLE1PacketFromSLO4 (HFCPtr<HCDPacket>& pio_pPacket);

    HSTATUS         SaveBlock                   (uint32_t           pi_PosBlockX,
                                                 uint32_t           pi_PosBlockY,
                                                 HFCPtr<HCDPacket>& pi_rpTransformedPacket);

    HSTATUS         sWriteBlock                 (uint32_t           pi_PosBlockX,
                                                 uint32_t           pi_PosBlockY,
                                                 HFCPtr<HCDPacket>& rpPacket);

    // Inline methods
    void            SetBitOn                    (Byte* pByte,
                                                 int32_t bit);

    void            SetBitOff                   (Byte* pByte,
                                                 int32_t bit);

    Byte          GetBit                      (Byte* pByte,
                                                 int32_t bit);


    // Methods Disabled
    HRFSLOStripEditor                           (const HRFSLOStripEditor& pi_rObj);
    HRFSLOStripEditor&                 operator=(const HRFSLOStripEditor& pi_rObj);

    };
END_IMAGEPP_NAMESPACE

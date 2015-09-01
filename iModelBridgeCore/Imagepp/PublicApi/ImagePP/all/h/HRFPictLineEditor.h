//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPictLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------
#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
static const Byte s_FilePadding[4] = {0x00, 0x00, 0x00, 0xFF};

class HRFPictFile;
class HCDCodecImage;
class HCDPacket;

class HRFPictLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFPictFile;

    virtual ~HRFPictLineEditor  ();

    // Edition by block


    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t     pi_PosBlockX,
                               uint64_t     pi_PosBlockY,
                               const Byte*  pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFPictLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t             pi_Page,
                      unsigned short        pi_Resolution,
                      HFCAccessMode         pi_AccessMode);

private:

    HFCPtr<HRFPictFile>     m_pRasterFile;

    // Position of the file ptr for that editor
    uint64_t                m_ReadPosInFile;
    uint64_t                m_WritePosInFile;

    uint32_t               m_CurrentWriteLine;
    uint32_t               m_CurrentReadLine;

    // Codec
    HFCPtr<HCDCodecImage>   m_pCodec;

    size_t                  m_LineBufferSize;
    HArrayAutoPtr<Byte>     m_pReadLineBuffer;
    HArrayAutoPtr<Byte>     m_pWriteLineBuffer;
    HArrayAutoPtr<Byte>     m_pWriteLineBuffer2;

    uint32_t                m_RowBytes;

    const   uint32_t       m_BytesPerBlockWidth;
    uint32_t               m_SubsetWidth;
    uint32_t               m_SubsetHeight;

    const   uint32_t       m_ImageWidth;
    const   uint32_t       m_ImageHeigth;

    const   bool            m_ReorganizeLineColorRepresentation;


    void    ReorganizeColorsToRGB      (Byte*         pio_InOutBuffer,
                                        size_t          pi_BufferSize);
    void    ReorganizeColorsToPict     (const Byte*   pi_InBuffer,
                                        size_t          pi_InSize,
                                        Byte*         po_OutBuffer,
                                        size_t          pi_OutSize);

    bool   ReadPackedByteQty          (uint32_t*         po_pPackedByteQty);
    bool   WritePackedByteQty         (uint32_t        pi_rPackedByteQty);


    bool   SkipLine                   (uint32_t*         pio_pLineIterator);

    bool   ReadAndUnpackLine          (Byte*         po_OutBuffer,
                                        size_t          pi_OutSize);
    bool   PackAndWriteLine           (const Byte*   pi_InBuffer,
                                        size_t          pi_InSize);


    // Methods Disabled
    HRFPictLineEditor(const HRFPictLineEditor& pi_rObj);
    HRFPictLineEditor& operator=(const HRFPictLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

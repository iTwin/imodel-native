//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HCDCodecHMRGIF.h"
#include "HFCBinStream.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFGifFile;

class HRFGifLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFGifFile;

    // Constructor
    HRFGifLineEditor(HFCPtr<HRFRasterFile>  pi_rpRasterFile,
                     uint32_t              pi_Page,
                     uint16_t         pi_Resolution,
                     HFCAccessMode          pi_AccessMode);
    virtual ~HRFGifLineEditor();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              Byte*   po_pData) override;

    HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t       pi_PosBlockX,
                               uint64_t       pi_PosBlockY,
                               const Byte*    pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;
protected:

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    bool ReadFromFile(uint32_t&           pio_rLastValidIndex,
                      uint32_t&           pio_rMaxBufferSize,
                      HFCBinStream*     pi_pFile);

private:

    HFCPtr<HRFGifFile>    m_pRasterFile;

    // Number of line read
    uint32_t m_NumberOfLineRead;

    // Codec
    HFCPtr<HCDCodecHMRGIF>    m_pCodec;

    // Buffer that will contains compress data
    Byte* m_pCompressBuffer;

    // Maximum valid offset in the buffer
    uint32_t m_MaxOffsetInBuffer;

    // Position of the file ptr for that editor
    uint32_t m_PosInFile;

    // Maximum buffer size
    uint32_t m_BufferSize;

    // Minimum size to add to the buffer
    uint32_t m_MininumSizeToAdd;

    // End of file reach.
    bool   m_EndDataReach;

//        // Array of lines offset.
//        HArrayAutoPtr<UInt32> m_pLinesOffsetBuffer;

    // Methods Disabled
    HRFGifLineEditor(const HRFGifLineEditor& pi_rObj);
    HRFGifLineEditor& operator=(const HRFGifLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


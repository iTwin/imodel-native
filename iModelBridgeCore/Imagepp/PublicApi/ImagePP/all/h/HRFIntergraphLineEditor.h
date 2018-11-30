//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphLineEditor.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFIntergraphFile.h"
#include "HFCBuffer.h"
#include "HFCBinStream.h"
#include "HCDPacket.h"
// #define HRF_DEBUG_DUMPSTATONDISK

BEGIN_IMAGEPP_NAMESPACE
class HGFTileIDDescriptor;
class HCDPacketRLE;

class HRFIntergraphLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFIntergraphFile;

    HRFIntergraphLineEditor (HFCPtr<HRFRasterFile>  pi_rpRasterFile, uint32_t pi_Page, uint16_t pi_Resolution, HFCAccessMode pi_AccessMode,
                             HRFIntergraphFile::IntergraphResolutionDescriptor& pi_rIntergraphResolutionDescriptor);

    virtual         ~HRFIntergraphLineEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS ReadBlockRLE(uint64_t              pi_PosBlockX,
                                 uint64_t              pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>& pio_rpPacketRLE) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }

    virtual HSTATUS WriteBlockRLE(uint64_t              pi_PosBlockX,
                                  uint64_t              pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>& pi_rpPacketRLE) override;

protected:

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

private:
    void BuildJpegLumiChromaTable (double pi_QualityFactor, Byte* po_pLuminance,  Byte* po_pChroma);
    void InitializeJpegDecompTable(double pi_QualityFactor, Byte* po_pTileBuffer, uint32_t pi_DataSize);

    void ApplyLUTColorCorrection  (Byte* po_pData, uint32_t pi_pixelCount);

    // Members
    uint16_t m_BitPerPixel;

    uint64_t         m_CurrentReadLine;
    uint32_t         m_RasterOffset;
    uint32_t         m_ResSizeInBytes;
    uint32_t         m_WidthInByteToRead;
    uint32_t         m_PageIndex;

    HCDPacket       m_CompressPacket;

#ifdef  HRF_DEBUG_DUMPSTATONDISK
    int32_t*           m_DumpStatArray;
#endif

    HRFIntergraphFile::IntergraphResolutionDescriptor&
    m_IntergraphResolutionDescriptor;

    HFCBinStream*   m_pIntergraphFile;

#ifdef  HRF_DEBUG_DUMPSTATONDISK
    void            DumpStatOnDisk(void);
#endif

    // Methods Disabled
    HRFIntergraphLineEditor (const HRFIntergraphLineEditor& pi_rObj);
    HRFIntergraphLineEditor& operator= (const HRFIntergraphLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


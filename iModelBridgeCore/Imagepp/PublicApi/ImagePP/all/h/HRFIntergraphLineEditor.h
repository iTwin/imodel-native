//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    HRFIntergraphLineEditor
    (HFCPtr<HRFRasterFile>  pi_rpRasterFile,
     uint32_t              pi_Page,
     unsigned short         pi_Resolution,
     HFCAccessMode          pi_AccessMode,
     HRFIntergraphFile::IntergraphResolutionDescriptor& pi_rIntergraphResolutionDescriptor);

    virtual         ~HRFIntergraphLineEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


    virtual HSTATUS ReadBlockRLE(uint64_t              pi_PosBlockX,
                                 uint64_t              pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>& pio_rpPacketRLE,
                                 HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlockRLE(uint64_t              pi_PosBlockX,
                                  uint64_t              pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                  HFCLockMonitor const* pi_pSisterFileLock = 0) override;

protected:

    virtual void OnSynchronizedSharingControl();

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

private:
    void BuildJpegLumiChromaTable (double pi_QualityFactor, Byte* po_pLuminance,  Byte* po_pChroma);
    void InitializeJpegDecompTable(double pi_QualityFactor, Byte* po_pTileBuffer, uint32_t pi_DataSize);

    void ApplyLUTColorCorrection  (Byte* po_pData, uint32_t pi_pixelCount);

    // Members
    unsigned short m_BitPerPixel;

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


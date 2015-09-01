//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCalsLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HCDCodecCCITTFax4.h"
#include "HFCBinStream.h"
#include "HCDPacket.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDPacketRLE;
class HRFCalsFile;

class HRFCalsLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFCalsFile;

    virtual        ~HRFCalsLineEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;


    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS ReadBlockRLE(uint64_t              pi_PosBlockX,
                                 uint64_t              pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>& pio_rpPacketRLE,
                                 HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlockRLE(uint64_t                pi_PosBlockX,
                                  uint64_t                pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>&   pi_rpPacketRLE,
                                  HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFCalsLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t              pi_Page,
                      unsigned short       pi_Resolution,
                      HFCAccessMode         pi_AccessMode);
private:
    HFCBinStream*   m_pCalsFile;
    HFCPtr<HCDCodecCCITTFax4> m_pCodec;

    uint64_t        m_CurrentReadLine;
    uint32_t       m_WidthInByteToRead;
    Byte            m_BitPerPixel;

    uint32_t       m_RasterOffset;
    uint32_t       m_ResSizeInBytes;
    HCDPacket       m_CompressPacket;

    // Methods Disabled
    HRFCalsLineEditor(const HRFCalsLineEditor& pi_rObj);
    HRFCalsLineEditor& operator=(const HRFCalsLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


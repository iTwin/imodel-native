//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIG4StripEditor.h $
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

class HRFIG4StripEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFIG4File;

    virtual        ~HRFIG4StripEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t   pi_PosBlockX,
                              uint64_t   pi_PosBlockY,
                              Byte*      po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

//     virtual HSTATUS ReadBlockRLE(UInt64                 pi_PosBlockX,
//                                  UInt64                 pi_PosBlockY,
//                                  HFCPtr<HCDPacketRLE>&  pio_rpPacketRLE,
//                                  HFCLockMonitor const*  pi_pSisterFileLock = 0);

    virtual HSTATUS WriteBlock(uint64_t     pi_PosBlockX,
                               uint64_t     pi_PosBlockY,
                               const Byte*  pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0) override;

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFIG4StripEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t             pi_Page,
                      unsigned short        pi_Resolution,
                      HFCAccessMode         pi_AccessMode);
private:
    HFCBinStream*    m_pIG4File;


    HFCPtr<HCDCodecCCITT> m_pCodec;

    uint32_t        m_CurrentReadLine;
    uint32_t        m_UncompressedBufferSize;
    Byte             m_BitPerPixel;
    uint32_t        m_StripHeight;
    uint32_t        m_ImageHeight;

    HCDPacket        m_CompressPacket;

    // Methods Disabled
    HRFIG4StripEditor(const HRFIG4StripEditor& pi_rObj);
    HRFIG4StripEditor& operator=(const HRFIG4StripEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


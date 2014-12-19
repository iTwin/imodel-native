//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFLRDLineEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

#include "HFCBinStream.h"
#include "HCDPacket.h"
#include "HCDCodecLRDRLE.h"

class HRFLRDFile;

class HRFLRDLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFLRDFile;

    virtual        ~HRFLRDLineEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock (uint32_t     pi_PosBlockX,
                               uint32_t     pi_PosBlockY,
                               Byte*       po_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS          ReadBlock     (uint32_t                 pi_PosBlockX,
                                            uint32_t                 pi_PosBlockY,
                                            HFCPtr<HCDPacket>&       po_rpPacket,
                                            HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


    virtual HSTATUS WriteBlock(uint32_t     pi_PosBlockX,
                               uint32_t     pi_PosBlockY,
                               const Byte* pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS          WriteBlock    (uint32_t                 pi_PosBlockX,
                                            uint32_t                 pi_PosBlockY,
                                            const HFCPtr<HCDPacket>& pi_rpPacket,
                                            HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFLRDLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    HFCBinStream*   m_pLRDFile;

    HFCPtr<HCDCodecLRDRLE> m_pCodec;

    uint32_t        m_CurrentReadLine;
    uint32_t        m_WidthInByteToRead;
    Byte          m_BitPerPixel;

    uint32_t        m_RasterOffset;
    uint32_t        m_ResSizeInBytes;
    HCDPacket       m_CompressPacket;

    // Methods Disabled
    HRFLRDLineEditor(const HRFLRDLineEditor& pi_rObj);
    HRFLRDLineEditor& operator=(const HRFLRDLineEditor& pi_rObj);
    };


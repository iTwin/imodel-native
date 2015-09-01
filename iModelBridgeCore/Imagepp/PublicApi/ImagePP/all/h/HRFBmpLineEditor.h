//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFBmpLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFBmpFile;

class HRFBmpLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFBmpFile;

    virtual ~HRFBmpLineEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              Byte*                  po_pData,
                              HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t       pi_PosBlockX,
                               uint64_t       pi_PosBlockY,
                               const Byte*    pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0) override;

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
    HRFBmpLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:

    HFCPtr<HRFBmpFile>      m_pRasterFile;

    HArrayAutoPtr<Byte>   m_pLineBuffer;

    // Number of bits to use for line padding per row
    unsigned short m_PaddingBitsPerRow;

    // The number of bits per row in the buffer
    uint32_t m_BitsPerRow;

    // The number of bits per row what is realy used
    uint32_t m_UsedBitsPerRow;

    // 0 if bits per row not multiple of 8
    uint32_t m_ExactBytesPerRow;

    // Methods Disabled
    HRFBmpLineEditor(const HRFBmpLineEditor& pi_rObj);
    HRFBmpLineEditor& operator=(const HRFBmpLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


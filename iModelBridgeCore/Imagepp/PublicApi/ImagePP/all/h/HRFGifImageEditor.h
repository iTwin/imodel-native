//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGifImageEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HCDCodecHMRGIF.h"
#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFGifFile;

class HRFGifImageEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFGifFile;

    virtual ~HRFGifImageEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t    pi_PosBlockX,
                              uint64_t    pi_PosBlockY,
                              Byte*       po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
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
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFGifImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t             pi_Page,
                      unsigned short        pi_Resolution,
                      HFCAccessMode         pi_AccessMode);
private:

    HFCPtr<HRFGifFile>              m_pRasterFile;

    uint32_t m_Width;
    uint32_t m_Height;

    HAutoPtr<HRFResolutionEditor>   m_pLineEditor;
    uint32_t                       m_ExactBytesPerImageWidth;

    // Methods Disabled
    HRFGifImageEditor(const HRFGifImageEditor& pi_rObj);
    HRFGifImageEditor& operator=(const HRFGifImageEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


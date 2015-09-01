//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFJpegLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFJpegFile;

class HRFJpegLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFJpegFile;

    virtual ~HRFJpegLineEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t     pi_PosBlockX,
                              uint64_t     pi_PosBlockY,
                              Byte*        po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

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

    virtual void OnSynchronizedSharingControl();

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFJpegLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t              pi_Page,
                      unsigned short       pi_Resolution,
                      HFCAccessMode         pi_AccessMode);
private:
    // Methods Disabled
    HRFJpegLineEditor(const HRFJpegLineEditor& pi_rObj);
    HRFJpegLineEditor& operator=(const HRFJpegLineEditor& pi_rObj);

    HSTATUS                         ReadCMYKBlock (uint64_t pi_PosBlockX,
                                                   uint64_t pi_PosBlockY,
                                                   Byte*  po_pData,
                                                   HFCLockMonitor const* pi_pSisterFileLock);

    HSTATUS                         WriteCMYKBlock(uint64_t pi_PosBlockX,
                                                   uint64_t pi_PosBlockY,
                                                   const Byte* pi_pData,
                                                   HFCLockMonitor const* pi_pSisterFileLock);

    bool           m_IsCMYK;
    Byte*         m_pCMYKData;
    };
END_IMAGEPP_NAMESPACE


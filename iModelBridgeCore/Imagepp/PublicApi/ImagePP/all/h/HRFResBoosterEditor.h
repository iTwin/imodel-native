//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFResBoosterEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFResBoosterEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFResBoosterEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFRasterFileCache;

    // Constructor
    HRFResBoosterEditor(
        HFCPtr<HRFRasterFile> pi_rpRasterFile,
        uint32_t              pi_Page,
        unsigned short       pi_Resolution,
        HFCAccessMode         pi_AccessMode,
        HRFResolutionEditor*  pi_pBoosterResolutionEditor);


    virtual         ~HRFResBoosterEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlockRLE(uint64_t                 pi_PosBlockX,
                                 uint64_t                 pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>&    po_rpPacketRLE,
                                 HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                   pi_PosBlockX,
                               uint64_t                   pi_PosBlockY,
                               const HFCPtr<HCDPacket>&   pi_rpPacket,
                               HFCLockMonitor const*      pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlockRLE(uint64_t                pi_PosBlockX,
                                  uint64_t                pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>&   pi_rpPacketRLE,
                                  HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

protected:
    HAutoPtr<HRFResolutionEditor>   m_pBoosterResolutionEditor;

private:
    // Methods Disabled
    HRFResBoosterEditor(const HRFResBoosterEditor& pi_rObj);
    HRFResBoosterEditor& operator=(const HRFResBoosterEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


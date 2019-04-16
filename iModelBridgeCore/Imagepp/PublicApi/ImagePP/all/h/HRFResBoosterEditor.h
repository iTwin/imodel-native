//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
        uint16_t       pi_Resolution,
        HFCAccessMode         pi_AccessMode,
        HRFResolutionEditor*  pi_pBoosterResolutionEditor);


    virtual         ~HRFResBoosterEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override;

    virtual HSTATUS ReadBlockRLE(uint64_t                 pi_PosBlockX,
                                 uint64_t                 pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>&    po_rpPacketRLE) override;

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                   pi_PosBlockX,
                               uint64_t                   pi_PosBlockY,
                               const HFCPtr<HCDPacket>&   pi_rpPacket) override;

    virtual HSTATUS WriteBlockRLE(uint64_t                pi_PosBlockX,
                                  uint64_t                pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>&   pi_rpPacketRLE) override;

protected:
    HAutoPtr<HRFResolutionEditor>   m_pBoosterResolutionEditor;

private:
    // Methods Disabled
    HRFResBoosterEditor(const HRFResBoosterEditor& pi_rObj);
    HRFResBoosterEditor& operator=(const HRFResBoosterEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


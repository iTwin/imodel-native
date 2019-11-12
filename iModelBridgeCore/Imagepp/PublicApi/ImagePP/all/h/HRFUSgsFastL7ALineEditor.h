//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFUSgsFastL7AFile;

class HRFUSgsFastL7ALineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFUSgsFastL7AFile;

    virtual ~HRFUSgsFastL7ALineEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFUSgsFastL7ALineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                             uint32_t              pi_Page,
                             uint16_t       pi_Resolution,
                             HFCAccessMode         pi_AccessMode);
private:
    // Methods Disabled
    HRFUSgsFastL7ALineEditor(const HRFUSgsFastL7ALineEditor& pi_rObj);
    HRFUSgsFastL7ALineEditor& operator=(const HRFUSgsFastL7ALineEditor& pi_rObj);

    };
END_IMAGEPP_NAMESPACE


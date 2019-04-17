//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFTiffTileEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFTiffFile;
class HRFHMRFile;

class HRFHMRTileEditor : public HRFTiffTileEditor
    {
public:
    friend class HRFTiffFile;
    friend class HRFHMRFile;

    virtual         ~HRFHMRTileEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFHMRTileEditor
    (HFCPtr<HRFRasterFile>    pi_rpRasterFile,
     uint32_t                pi_Page,
     uint16_t           pi_Resolution,
     HFCAccessMode            pi_AccessMode,
     uint32_t                pi_PaddingLines);

private:

    // Methods Disabled
    HRFHMRTileEditor(const HRFHMRTileEditor& pi_rObj);
    HRFHMRTileEditor& operator=(const HRFHMRTileEditor& pi_rObj);

    uint32_t        m_PaddingLines;
    };
END_IMAGEPP_NAMESPACE

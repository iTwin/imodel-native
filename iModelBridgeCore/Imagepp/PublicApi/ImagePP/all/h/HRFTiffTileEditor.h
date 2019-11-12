//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFTiffFile;
class HRFGeoTiffFile;
class HRFiTiffFile;
class HRFHMRFile;

class HRFTiffTileEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFTiffFile;
    friend class HRFGeoTiffFile;
    friend class HRFiTiffFile;
    friend class HRFHMRFile;

    virtual                         ~HRFTiffTileEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override;

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;

protected:

    uint32_t        m_IndexOfPage;
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFTiffTileEditor
    (HFCPtr<HRFRasterFile> pi_rpRasterFile,
     uint32_t              pi_Page,
     uint16_t       pi_Resolution,
     HFCAccessMode         pi_AccessMode);

private:

    // Pointer to runs Compression/Decompression RLE1
    uint16_t*        m_pRuns;
    size_t          m_NumberOfRuns;

    // Methods Disabled
    HRFTiffTileEditor(const HRFTiffTileEditor& pi_rObj);
    HRFTiffTileEditor& operator=(const HRFTiffTileEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


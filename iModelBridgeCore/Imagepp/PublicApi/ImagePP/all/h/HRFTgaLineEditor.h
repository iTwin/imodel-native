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
//:Ignore
class HRFTgaFile;
//:End Ignore

/**----------------------------------------------------------------------------
 This class provides the interface to read and write raster data in the correct
 format for Tga file.
-----------------------------------------------------------------------------*/

class HRFTgaLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFTgaFile;

    virtual ~HRFTgaLineEditor  ();

    //:> Edition by Block
    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              Byte*   po_pData) override;

    HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const Byte*            pi_pData) override;

    HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }


protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFTgaLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     uint16_t       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    //:> Attributes
    HArrayAutoPtr<Byte>       m_pLineBuffer;
    uint32_t                    m_BytesPerLine;
    uint32_t                    m_RasterDataOffset;
    Byte                       m_AlphaChannelBits;

    //:> Methods Disabled
    HRFTgaLineEditor(const HRFTgaLineEditor& pi_rObj);
    HRFTgaLineEditor& operator=(const HRFTgaLineEditor& pi_rObj);

    };
END_IMAGEPP_NAMESPACE


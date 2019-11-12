//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFBilFile;

class HRFBilLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)
    friend class HRFBilFile;

    virtual ~HRFBilLineEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;


    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }

    virtual HSTATUS WriteBlock(uint64_t                pi_PosBlockX,
                               uint64_t                pi_PosBlockY,
                               const Byte*             pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFBilLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     uint16_t       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);

    HSTATUS Read24BitRgbBlock ( uint64_t                 pi_PosBlockX,
                                uint64_t                 pi_PosBlockY,
                                Byte*                   po_pData);

    HSTATUS Read48BitRgbBlock ( uint64_t                 pi_PosBlockX,
                                uint64_t                 pi_PosBlockY,
                                Byte*                   po_pData);

    HSTATUS Read8BitGrayBlock ( uint64_t                 pi_PosBlockX,
                                uint64_t                 pi_PosBlockY,
                                Byte*                   po_pData);

    HSTATUS Read16BitGrayBlock ( uint64_t                 pi_PosBlockX,
                                 uint64_t                 pi_PosBlockY,
                                 Byte*                   po_pData);

private:
    // Methods Disabled
    HRFBilLineEditor(const HRFBilLineEditor& pi_rObj);
    HRFBilLineEditor& operator=(const HRFBilLineEditor& pi_rObj);

    HFCPtr<HRFBilFile>      GetBilRasterFile();

    //static double GaussianLookupTable[255];

    uint32_t m_Offset;
    Byte*  m_pRedLineBuffer;
    Byte*  m_pGreenLineBuffer;
    Byte*  m_pBlueLineBuffer;
    uint32_t m_LineWidth;
    uint32_t m_nbBitsPerBandPerPixel;
    uint32_t m_nNbChannel;
    };
END_IMAGEPP_NAMESPACE


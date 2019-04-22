//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFSpotCAPFile;

class HRFSpotCAPLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFSpotCAPFile;

    virtual ~HRFSpotCAPLineEditor  ();

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


    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData) override;

    HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFSpotCAPLineEditor(   HFCPtr<HRFRasterFile> pi_rpRasterFile,
                            uint32_t              pi_Page,
                            uint16_t       pi_Resolution,
                            HFCAccessMode         pi_AccessMode);


private:
    // Methods Disabled
    HRFSpotCAPLineEditor(const HRFSpotCAPLineEditor& pi_rObj);
    HRFSpotCAPLineEditor& operator=(const HRFSpotCAPLineEditor& pi_rObj);

    HFCPtr<HRFSpotCAPFile>      m_pRasterFile;

    HSTATUS                 Read8BitGrayBlock ( uint32_t                 pi_PosBlockX,
                                                uint32_t                 pi_PosBlockY,
                                                Byte*                   po_pData);

    HSTATUS                 Read24BitRgbBlock ( uint32_t                 pi_PosBlockX,
                                                uint32_t                 pi_PosBlockY,
                                                Byte*                   po_pData);




    uint64_t m_Offset;
    uint32_t m_BandNumber;
    Byte*  m_pRedLineBuffer;
    Byte*  m_pGreenLineBuffer;
    Byte*  m_pBlueLineBuffer;
    uint32_t m_LineWidth;
    bool   m_isMsByteFirst;
    uint32_t m_nbBitsPerBandPerPixel;
    uint32_t m_nNbChannel;
    double m_RedBandScaling;
    double m_GreenBandScaling;
    double m_BlueBandScaling;
    };
END_IMAGEPP_NAMESPACE




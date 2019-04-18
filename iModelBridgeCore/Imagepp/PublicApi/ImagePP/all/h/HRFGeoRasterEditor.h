//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFGeoRasterFile.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFGeoRasterEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFGeoRasterFile;

    virtual         ~HRFGeoRasterEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFGeoRasterEditor  (HFCPtr<HRFRasterFile>      pi_rpRasterFile,
                         uint32_t                  pi_Page,
                         uint16_t             pi_Resolution,
                         HFCAccessMode              pi_AccessMode);


private:


    // optimization
    HFCPtr<HCDCodec>        m_pCodec;
    size_t                  m_BlockSizeInByte;

    Byte m_NbBytesToSwap;

    SDOGeoRasterWrapper*    m_pSDOGeoRasterWrapper;

    // Methods Disabled
    HRFGeoRasterEditor(const HRFGeoRasterEditor& pi_rObj);
    HRFGeoRasterEditor& operator=(const HRFGeoRasterEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE



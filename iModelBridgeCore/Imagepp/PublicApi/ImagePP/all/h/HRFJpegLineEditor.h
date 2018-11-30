//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFJpegLineEditor.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
                              Byte*        po_pData) override;

    HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t     pi_PosBlockX,
                               uint64_t     pi_PosBlockY,
                               const Byte*  pi_pData) override;

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
    HRFJpegLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t              pi_Page,
                      uint16_t       pi_Resolution,
                      HFCAccessMode         pi_AccessMode);
private:
    // Methods Disabled
    HRFJpegLineEditor(const HRFJpegLineEditor& pi_rObj);
    HRFJpegLineEditor& operator=(const HRFJpegLineEditor& pi_rObj);

    HSTATUS                         ReadCMYKBlock (uint64_t pi_PosBlockX,
                                                   uint64_t pi_PosBlockY,
                                                   Byte*  po_pData);

    HSTATUS                         WriteCMYKBlock(uint64_t pi_PosBlockX,
                                                   uint64_t pi_PosBlockY,
                                                   const Byte* pi_pData);

    bool           m_IsCMYK;
    Byte*         m_pCMYKData;
    };
END_IMAGEPP_NAMESPACE


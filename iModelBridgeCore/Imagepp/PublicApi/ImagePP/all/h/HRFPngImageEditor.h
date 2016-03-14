//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPngImageEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFPngFile;

class HRFPngImageEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFPngFile;

    virtual ~HRFPngImageEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t     pi_PosBlockX,
                              uint64_t     pi_PosBlockY,
                              Byte*        po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;

    virtual HSTATUS WriteBlock(uint64_t     pi_PosBlockX,
                               uint64_t     pi_PosBlockY,
                               const Byte*  pi_pData) override;
protected:

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFPngImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t              pi_Page,
                      uint16_t       pi_Resolution,
                      HFCAccessMode         pi_AccessMode);
private:
    // Members
    uint32_t m_NbInterlacePasses;
    uint32_t m_BytesByRow;

    // Methods Disabled
    HRFPngImageEditor(const HRFPngImageEditor& pi_rObj);
    HRFPngImageEditor& operator=(const HRFPngImageEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


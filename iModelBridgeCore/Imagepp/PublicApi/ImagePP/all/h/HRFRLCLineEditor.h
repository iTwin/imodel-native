//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFRLCLineEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

#include "HFCBinStream.h"
#include "HTIFFUtils.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRLCFile;

class HRFRLCLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFRLCFile;

    virtual        ~HRFRLCLineEditor  ();

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
                               uint64_t     pi_PosBlocY,
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
    HRFRLCLineEditor(HFCPtr<HRFRasterFile>  pi_rpRasterFile,
                     uint32_t        pi_Page,
                     uint16_t pi_Resolution,
                     HFCAccessMode   pi_AccessMode);
private:
    HFCBinStream*       m_pRLCFile;

    uint32_t            m_Width;
    uint32_t            m_BytesPerRow;
    uint32_t            m_CurrentLine;
    HTIFFByteOrdering   m_ByteOrdering;
    HArrayAutoPtr<uint16_t>
    m_pBuffer;

    // Methods Disabled
    HRFRLCLineEditor(const HRFRLCLineEditor& pi_rObj);
    HRFRLCLineEditor& operator=(const HRFRLCLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE





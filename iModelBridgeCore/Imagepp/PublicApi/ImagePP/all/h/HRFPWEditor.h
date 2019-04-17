//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#if defined(IPP_HAVE_PROJECTWISE_SUPPORT) 

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFPWRasterFile;
class HRFPWHandler;
class IHRFPWFileHandler;

class HRFPWEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFPWRasterFile;

    virtual         ~HRFPWEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket);

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFPWEditor
    (HFCPtr<HRFRasterFile>     pi_rpRasterFile,
     uint32_t                 pi_Page,
     uint16_t            pi_Resolution,
     HFCAccessMode             pi_AccessMode);


private:

    GUID                m_DocumentID;
    time_t              m_DocumentTimestamp;

    IHRFPWFileHandler*  m_pHandler;

    // Methods Disabled
    HRFPWEditor(const HRFPWEditor& pi_rObj);
    HRFPWEditor& operator=(const HRFPWEditor& pi_rObj);

    };

END_IMAGEPP_NAMESPACE

#endif

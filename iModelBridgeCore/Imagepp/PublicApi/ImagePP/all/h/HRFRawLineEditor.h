//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRawLineEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

class HRFRawFile;

class HRFRawLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFRawFile;

    virtual ~HRFRawLineEditor  ();

    // Edition by Block
    virtual HSTATUS                 ReadBlock (uint32_t                 pi_PosBlockX,
                                               uint32_t                 pi_PosBlockY,
                                               Byte*                   po_pData,
                                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

    virtual HSTATUS          ReadBlock     (uint32_t                 pi_PosBlockX,
                                            uint32_t                 pi_PosBlockY,
                                            HFCPtr<HCDPacket>&       po_rpPacket,
                                            HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS                 WriteBlock(uint32_t                 pi_PosBlockX,
                                               uint32_t                 pi_PosBlockY,
                                               const Byte*             pi_pData,
                                               HFCLockMonitor const*    pi_pSisterFileLock = 0);
    virtual HSTATUS          WriteBlock    (uint32_t                 pi_PosBlockX,
                                            uint32_t                 pi_PosBlockY,
                                            const HFCPtr<HCDPacket>& pi_rpPacket,
                                            HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFRawLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    // Methods Disabled
    HRFRawLineEditor(const HRFRawLineEditor& pi_rObj);
    HRFRawLineEditor& operator=(const HRFRawLineEditor& pi_rObj);

    };


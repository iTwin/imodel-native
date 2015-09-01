//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUSgsFastL7ALineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFUSgsFastL7AFile;

class HRFUSgsFastL7ALineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFUSgsFastL7AFile;

    virtual ~HRFUSgsFastL7ALineEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFUSgsFastL7ALineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                             uint32_t              pi_Page,
                             unsigned short       pi_Resolution,
                             HFCAccessMode         pi_AccessMode);
private:
    // Methods Disabled
    HRFUSgsFastL7ALineEditor(const HRFUSgsFastL7ALineEditor& pi_rObj);
    HRFUSgsFastL7ALineEditor& operator=(const HRFUSgsFastL7ALineEditor& pi_rObj);

    };
END_IMAGEPP_NAMESPACE


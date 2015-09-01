//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFDtedEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFGdalSupportedFileEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFDtedFile;


class HRFDtedEditor : public HRFGdalSupportedFileEditor
    {
public:
    DEFINE_T_SUPER(HRFGdalSupportedFileEditor)

    friend class HRFDtedFile;

    virtual ~HRFDtedEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFDtedEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                  uint32_t              pi_Page,
                  unsigned short       pi_Resolution,
                  HFCAccessMode         pi_AccessMode);

private:
    // Methods Disabled
    HRFDtedEditor(const HRFDtedEditor& pi_rObj);
    HRFDtedEditor& operator=(const HRFDtedEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

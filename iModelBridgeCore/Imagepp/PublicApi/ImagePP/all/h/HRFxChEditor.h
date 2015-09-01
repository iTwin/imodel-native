//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFxChEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFxChFile;

/** ---------------------------------------------------------------------------
    This class handles xCh raster file I/O operations.

    @see HRFRasterFile
    @see HRFxChEditor
    ---------------------------------------------------------------------------
 */
class HRFxChEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFxChFile;

    virtual       ~HRFxChEditor  ();

    //:> Edition by block
    virtual HSTATUS ReadBlock(uint64_t   pi_PosBlockX,
                              uint64_t   pi_PosBlockY,
                              Byte*      po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              HFCPtr<HCDPacket>&     po_rpPacket,
                              HFCLockMonitor const*  pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFxChEditor (HFCPtr<HRFRasterFile> pi_rpRasterFile,
                  uint32_t              pi_Page,
                  unsigned short       pi_Resolution,
                  HFCAccessMode         pi_AccessMode);
private:
    //:> Methods Disabled
    HRFxChEditor(const HRFxChEditor& pi_rObj);
    HRFxChEditor& operator=(const HRFxChEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

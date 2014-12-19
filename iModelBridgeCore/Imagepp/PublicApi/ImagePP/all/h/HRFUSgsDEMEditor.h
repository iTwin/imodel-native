//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUSgsDEMEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFGdalSupportedFileEditor.h"

class GDALRasterBand;

class HRFUSgsDEMEditor : public HRFGdalSupportedFileEditor
    {
public:
    DEFINE_T_SUPER(HRFGdalSupportedFileEditor)

    friend class HRFUSgsDEMFile;

    virtual ~HRFUSgsDEMEditor  ();

    // Edition by Block
    virtual HSTATUS                 ReadBlock (uint32_t                 pi_PosBlockX,
                                               uint32_t                 pi_PosBlockY,
                                               Byte*                   po_pData,
                                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

    virtual HSTATUS                 ReadBlock  (uint32_t                 pi_PosBlockX,
                                                uint32_t                 pi_PosBlockY,
                                                HFCPtr<HCDPacket>&       po_rpPacket,
                                                HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFUSgsDEMEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);

private:
    // Methods Disabled
    HRFUSgsDEMEditor(const HRFUSgsDEMEditor& pi_rObj);
    HRFUSgsDEMEditor& operator=(const HRFUSgsDEMEditor& pi_rObj);
    };
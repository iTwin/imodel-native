//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPngLineEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

class HRFPngFile;

class HRFPngLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFPngFile;

    virtual ~HRFPngLineEditor  ();

    // Edition by block
    virtual HSTATUS                 ReadBlock     (uint32_t     pi_PosBlockX,
                                                   uint32_t     pi_PosBlockY,
                                                   Byte*       po_pData,
                                                   HFCLockMonitor const*    pi_pSisterFileLock = 0);


    virtual HSTATUS                 ReadBlock     (uint32_t                 pi_PosBlockX,
                                                   uint32_t                 pi_PosBlockY,
                                                   HFCPtr<HCDPacket>&       po_rpPacket,
                                                   HFCLockMonitor const*    pi_pSisterFileLock = 0);


    virtual HSTATUS                 WriteBlock    (uint32_t                 pi_PosBlockX,
                                                   uint32_t                 pi_PosBlockY,
                                                   const HFCPtr<HCDPacket>& pi_rpPacket,
                                                   HFCLockMonitor const*    pi_pSisterFileLock = 0);
    virtual HSTATUS                 WriteBlock    (uint32_t     pi_PosBlockX,
                                                   uint32_t     pi_PosBlockY,
                                                   const Byte* pi_pData,
                                                   HFCLockMonitor const*    pi_pSisterFileLock = 0);

protected:

    virtual void OnSynchronizedSharingControl();

    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFPngLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    // Members
    uint32_t m_NbInterlacePasses;
    bool   m_FirstWriteInCreateMode;

    // Methods Disabled
    HRFPngLineEditor(const HRFPngLineEditor& pi_rObj);
    HRFPngLineEditor& operator=(const HRFPngLineEditor& pi_rObj);
    };


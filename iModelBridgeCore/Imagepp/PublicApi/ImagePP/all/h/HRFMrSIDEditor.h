//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFMrSIDEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#if defined(IPP_HAVE_MRSID_SUPPORT) 

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFMrSIDFile;
class HGFTileIDDescriptor;

class HRFMrSIDEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFMrSIDFile;

    virtual ~HRFMrSIDEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t     pi_PosBlockX,
                              uint64_t     pi_PosBlockY,
                              Byte*        po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t     pi_PosBlockX,
                               uint64_t     pi_PosBlockY,
                               const Byte*  pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFMrSIDEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                   uint32_t              pi_Page,
                   unsigned short       pi_Resolution,
                   HFCAccessMode         pi_AccessMode);
private:

    unsigned short                 m_ResNb;

    HAutoPtr<HGFTileIDDescriptor>   m_pTileIDDesc;

    // Methods Disabled
    HRFMrSIDEditor(const HRFMrSIDEditor& pi_rObj);
    HRFMrSIDEditor& operator=(const HRFMrSIDEditor& pi_rObj);
    };

END_IMAGEPP_NAMESPACE

#endif  // IPP_HAVE_MRSID_SUPPORT


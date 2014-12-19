//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSMemoryRLESurfaceDescriptor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSMemorySurfaceDescriptor
//-----------------------------------------------------------------------------
// General class for MemorySurfaces.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSMemoryBaseSurfaceDescriptor.h"
#include "HGFScanlineOrientation.h"
#include "HGSMemorySurfaceDescriptor.h"

class HCDPacketRLE;
class HRPPixelType;

class HGSMemoryRLESurfaceDescriptor : public HGSMemoryBaseSurfaceDescriptor
    {
    HDECLARE_CLASS_ID(1713, HGSMemoryBaseSurfaceDescriptor)

public:

    _HDLLg          HGSMemoryRLESurfaceDescriptor(uint32_t                          pi_Width,
                                                  uint32_t                          pi_Height,
                                                  const HFCPtr<HRPPixelType>&       pi_rpPixelType,
                                                  const HFCPtr<HCDPacketRLE>&       pi_rpPacket,
                                                  HGFSLO                            pi_SLO);

    _HDLLg virtual  ~HGSMemoryRLESurfaceDescriptor();

    virtual HGSSurfaceCapabilities*
    GetRequiredSurfaceCapabilities() const;

    virtual const HFCPtr<HCDCodec>&
    GetCodec() const override;

    _HDLLg const HFCPtr<HCDPacketRLE>&
    GetRLEPacket() const;
    void            SetRLEPacket(const HFCPtr<HCDPacketRLE>& pi_rpPacket);

protected:

private:

    HFCPtr<HCDPacketRLE>    m_pPacketRLE;

    // disabled methods
    HGSMemoryRLESurfaceDescriptor();
    HGSMemoryRLESurfaceDescriptor&
    operator=(const HGSMemoryRLESurfaceDescriptor& pi_rObj);
    bool             operator==(const HGSMemoryRLESurfaceDescriptor& pi_rObj) const;
    bool             operator!=(const HGSMemoryRLESurfaceDescriptor& pi_rObj);
    HGSMemoryRLESurfaceDescriptor(const HGSMemoryRLESurfaceDescriptor& pi_rObj);
    };


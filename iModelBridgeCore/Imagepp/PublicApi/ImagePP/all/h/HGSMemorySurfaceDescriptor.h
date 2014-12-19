//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSMemorySurfaceDescriptor.h $
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
#include "HRPPixelType.h"
#include "HCDPacket.h"
#include "HGFScanlineOrientation.h"

class HGSMemorySurfaceDescriptor : public HGSMemoryBaseSurfaceDescriptor
    {
    HDECLARE_CLASS_ID(1712, HGSMemoryBaseSurfaceDescriptor)

public:

    _HDLLg          HGSMemorySurfaceDescriptor(uint32_t                          pi_Width,
                                               uint32_t                          pi_Height,
                                               const HFCPtr<HRPPixelType>&       pi_rpPixelType,
                                               uint32_t                          pi_BytesPerRow);

    _HDLLg          HGSMemorySurfaceDescriptor(uint32_t                          pi_Width,
                                               uint32_t                          pi_Height,
                                               const HFCPtr<HRPPixelType>&       pi_rpPixelType,
                                               const HFCPtr<HCDPacket>&          pi_rpPacket,
                                               HGFSLO                            pi_SLO,
                                               uint32_t                          pi_BytesPerRow);

    _HDLLg virtual  ~HGSMemorySurfaceDescriptor();

    virtual HGSSurfaceCapabilities*
    GetRequiredSurfaceCapabilities() const override;

    _HDLLg const HFCPtr<HCDPacket>&
    GetPacket() const;
    void            SetPacket(const HFCPtr<HCDPacket>& pi_rpPacket);

    virtual const HFCPtr<HCDCodec>&
    GetCodec() const override;
protected:

private:

    HFCPtr<HCDPacket>       m_pPacket;

    // disabled methods
    HGSMemorySurfaceDescriptor();
    HGSMemorySurfaceDescriptor&
    operator=(const HGSMemorySurfaceDescriptor& pi_rObj);
    bool             operator==(const HGSMemorySurfaceDescriptor& pi_rObj) const;
    bool             operator!=(const HGSMemorySurfaceDescriptor& pi_rObj);
    HGSMemorySurfaceDescriptor(const HGSMemorySurfaceDescriptor& pi_rObj);
    };


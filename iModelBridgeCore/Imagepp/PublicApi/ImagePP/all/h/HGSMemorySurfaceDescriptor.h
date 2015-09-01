//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSMemorySurfaceDescriptor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_IMAGEPP_NAMESPACE
class HGSMemorySurfaceDescriptor : public HGSMemoryBaseSurfaceDescriptor
    {
    HDECLARE_CLASS_ID(HGSMemoryId_SurfaceDescriptor, HGSMemoryBaseSurfaceDescriptor)

public:

    // The constructor without a packet will delay the memory allocation until an HRASurface is created.
    HGSMemorySurfaceDescriptor(uint32_t                          pi_Width,
                               uint32_t                          pi_Height,
                               const HFCPtr<HRPPixelType>&        pi_rpPixelType,
                               uint32_t                          pi_BytesPerRow);

    HGSMemorySurfaceDescriptor(uint32_t                          pi_Width,
                               uint32_t                          pi_Height,
                               const HFCPtr<HRPPixelType>&        pi_rpPixelType,
                               const HFCPtr<HCDPacket>&           pi_rpPacket,
                               HGFSLO                             pi_SLO,
                               uint32_t                          pi_BytesPerRow);

    virtual  ~HGSMemorySurfaceDescriptor();

    const HFCPtr<HCDPacket>& GetPacket() const;

    void SetPacket(const HFCPtr<HCDPacket>& pi_rpPacket);

    virtual const HFCPtr<HCDCodec>& GetCodec() const override;
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

END_IMAGEPP_NAMESPACE
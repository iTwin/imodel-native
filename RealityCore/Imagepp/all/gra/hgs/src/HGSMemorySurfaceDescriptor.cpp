//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HGSMemorySurfaceDescriptor
//---------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGSMemorySurfaceDescriptor.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSMemorySurfaceDescriptor::HGSMemorySurfaceDescriptor
(uint32_t                       pi_Width,
 uint32_t                       pi_Height,
 const HFCPtr<HRPPixelType>&    pi_rpPixelType,
 uint32_t                       pi_BytesPerRow)

    : HGSMemoryBaseSurfaceDescriptor(pi_Width, pi_Height, pi_rpPixelType, HGF_UPPER_LEFT_HORIZONTAL, pi_BytesPerRow)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSMemorySurfaceDescriptor::HGSMemorySurfaceDescriptor
(uint32_t                       pi_Width,
 uint32_t                       pi_Height,
 const HFCPtr<HRPPixelType>&    pi_rpPixelType,
 const HFCPtr<HCDPacket>&       pi_rpPacket,
 HGFSLO                         pi_SLO,
 uint32_t                       pi_BytesPerRow)
    : HGSMemoryBaseSurfaceDescriptor(pi_Width, pi_Height, pi_rpPixelType, pi_SLO, pi_BytesPerRow)
    {
    HPRECONDITION(pi_rpPacket != 0);

    m_pPacket       = pi_rpPacket;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSMemorySurfaceDescriptor::~HGSMemorySurfaceDescriptor()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetPacket
//-----------------------------------------------------------------------------
const HFCPtr<HCDPacket>& HGSMemorySurfaceDescriptor::GetPacket() const
    {
    return m_pPacket;
    }

//-----------------------------------------------------------------------------
// public
// SetPacket
//-----------------------------------------------------------------------------
void HGSMemorySurfaceDescriptor::SetPacket(const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    m_pPacket = pi_rpPacket;
    }

//-----------------------------------------------------------------------------
// public
// GetPacket
//-----------------------------------------------------------------------------
const HFCPtr<HCDCodec>& HGSMemorySurfaceDescriptor::GetCodec() const
    {
    return m_pPacket->GetCodec();
    }

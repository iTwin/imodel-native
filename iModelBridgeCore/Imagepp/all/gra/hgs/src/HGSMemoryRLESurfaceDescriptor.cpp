//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HGSMemoryRLESurfaceDescriptor
//---------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGSMemoryRLESurfaceDescriptor.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <ImagePP/all/h/HCDPacketRLE.h>
#include <ImagePP/all/h/HCDCodecHMRRLE1.h>


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSMemoryRLESurfaceDescriptor::HGSMemoryRLESurfaceDescriptor
(uint32_t                       pi_Width,
 uint32_t                       pi_Height,
 const HFCPtr<HRPPixelType>&    pi_rpPixelType,
 const HFCPtr<HCDPacketRLE>&    pi_rpPacket,
 HGFSLO                         pi_SLO)
    : HGSMemoryBaseSurfaceDescriptor(pi_Width, pi_Height, pi_rpPixelType, pi_SLO, 8)
    {
    HPRECONDITION(pi_rpPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pi_rpPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));
    HPRECONDITION(pi_rpPacket != 0);
    HPRECONDITION(pi_rpPacket->GetCodec() != 0 ? pi_rpPacket->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID) : true);

    m_pPacketRLE = pi_rpPacket;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSMemoryRLESurfaceDescriptor::~HGSMemoryRLESurfaceDescriptor()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetPacket
//-----------------------------------------------------------------------------
const HFCPtr<HCDPacketRLE>& HGSMemoryRLESurfaceDescriptor::GetRLEPacket() const
    {
    return m_pPacketRLE;
    }

//-----------------------------------------------------------------------------
// public
// SetPacket
//-----------------------------------------------------------------------------
void HGSMemoryRLESurfaceDescriptor::SetRLEPacket(const HFCPtr<HCDPacketRLE>& pi_rpPacket)
    {
    m_pPacketRLE = pi_rpPacket;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
const HFCPtr<HCDCodec>& HGSMemoryRLESurfaceDescriptor::GetCodec() const
    {
    return (const HFCPtr<HCDCodec>&)m_pPacketRLE->GetCodec();
    }

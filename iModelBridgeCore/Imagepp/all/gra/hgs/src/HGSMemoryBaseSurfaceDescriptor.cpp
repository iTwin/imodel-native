//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HGSMemoryBaseSurfaceDescriptor
//---------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <ImagePP/all/h/HRPPixelType.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSMemoryBaseSurfaceDescriptor::HGSMemoryBaseSurfaceDescriptor
(uint32_t                       pi_Width,
 uint32_t                       pi_Height,
 const HFCPtr<HRPPixelType>&    pi_rpPixelType,
 HGFSLO                         pi_SLO,
 uint32_t                       pi_BytesPerRow)
    : HGSSurfaceDescriptor(pi_Width, pi_Height)
    {
    HPRECONDITION(pi_rpPixelType != 0);

    m_pPixelType    = pi_rpPixelType;
    m_SLO           = pi_SLO;
    m_BytesPerRow   = pi_BytesPerRow;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSMemoryBaseSurfaceDescriptor::~HGSMemoryBaseSurfaceDescriptor()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------
const HFCPtr<HRPPixelType>& HGSMemoryBaseSurfaceDescriptor::GetPixelType() const
    {
    return m_pPixelType;
    }

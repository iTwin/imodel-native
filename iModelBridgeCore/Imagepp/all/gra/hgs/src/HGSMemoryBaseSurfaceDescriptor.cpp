//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSMemoryBaseSurfaceDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HGSMemoryBaseSurfaceDescriptor
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <Imagepp/all/h/HRPPixelType.h>

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
// GetRequiredSurfaceCapabilities
//-----------------------------------------------------------------------------
HGSSurfaceCapabilities* HGSMemoryBaseSurfaceDescriptor::GetRequiredSurfaceCapabilities() const
    {
    // first call the parent method
    HGSSurfaceCapabilities* pCapabilities = HGSSurfaceDescriptor::GetRequiredSurfaceCapabilities();

    // add other capabilities
    pCapabilities->Add(new HGSSurfaceCapability(HGSSurfaceTypeAttribute(HGSSurfaceType::MEMORY)));
    pCapabilities->Add(new HGSSurfaceCapability(HGSSLOAttribute(m_SLO)));
    pCapabilities->Add(new HGSSurfaceCapability(HGSPixelTypeAttribute(m_pPixelType->GetClassID())));

    return pCapabilities;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------
const HFCPtr<HRPPixelType>& HGSMemoryBaseSurfaceDescriptor::GetPixelType() const
    {
    return m_pPixelType;
    }

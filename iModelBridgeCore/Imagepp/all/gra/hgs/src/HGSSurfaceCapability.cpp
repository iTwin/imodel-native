//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSSurfaceCapability.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSSurfaceCapability
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HGSSurfaceCapability.h>

//---------------------------------------------------------------------------------------
// class HGSSurfaceCapability
//---------------------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor

 @param pi_rAttribute
-----------------------------------------------------------------------------*/
HGSSurfaceCapability::HGSSurfaceCapability(const HGSSurfaceAttribute& pi_rAttribute)
    : HFCCapability()
    {
    m_pAttribute = pi_rAttribute.Clone();

    HPOSTCONDITION(m_pAttribute != 0);
    }


/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGSSurfaceCapability::~HGSSurfaceCapability()
    {
    }


/**----------------------------------------------------------------------------
 Clone
-----------------------------------------------------------------------------*/
HFCCapability* HGSSurfaceCapability::Clone() const
    {
    return new HGSSurfaceCapability(*m_pAttribute);
    }


/**----------------------------------------------------------------------------
 IsSameAs

 @param pi_rCapability
-----------------------------------------------------------------------------*/
bool HGSSurfaceCapability::IsSameAs(const HFCCapability& pi_rCapability) const
    {
    return m_pAttribute->IsSameAs(*((const HGSSurfaceCapability&)pi_rCapability).m_pAttribute);
    }


/**----------------------------------------------------------------------------
 Supports

 @param pi_rCapability
-----------------------------------------------------------------------------*/
bool HGSSurfaceCapability::Supports(const HFCCapability& pi_rpCapability) const
    {
    return IsSameAs(pi_rpCapability) &&
           m_pAttribute->Supports(*((const HGSSurfaceCapability&)pi_rpCapability).m_pAttribute);
    }



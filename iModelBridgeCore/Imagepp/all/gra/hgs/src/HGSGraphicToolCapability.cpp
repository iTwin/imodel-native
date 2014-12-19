//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSGraphicToolCapability.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSGraphicToolCapability.h>


//---------------------------------------------------------------------------------------
// Class : HGSGraphicToolCapability
//---------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSGraphicToolCapability::HGSGraphicToolCapability(const HGSGraphicToolAttribute& pi_rAttribute)
    : HFCCapability()
    {
    m_pAttribute = pi_rAttribute.Clone();
    HPOSTCONDITION(m_pAttribute != 0);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSGraphicToolCapability::~HGSGraphicToolCapability()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HFCCapability* HGSGraphicToolCapability::Clone() const
    {
    return new HGSGraphicToolCapability(*m_pAttribute);
    }

//-----------------------------------------------------------------------------
// public
// IsSameAs
//-----------------------------------------------------------------------------
bool HGSGraphicToolCapability::IsSameAs(const HFCCapability& pi_rCapability) const
    {
    return pi_rCapability.IsCompatibleWith(GetClassID()) &&
           m_pAttribute->IsSameAs(*((const HGSGraphicToolCapability&)pi_rCapability).m_pAttribute);
    }

//-----------------------------------------------------------------------------
// public
// Supports
//-----------------------------------------------------------------------------
bool HGSGraphicToolCapability::Supports(const HFCCapability& pi_rCapability) const
    {
    return IsSameAs(pi_rCapability) &&
           m_pAttribute->Supports(*((const HGSGraphicToolCapability&)pi_rCapability).m_pAttribute);
    }

//-----------------------------------------------------------------------------
// public
// GetAttribute
//-----------------------------------------------------------------------------
const HGSGraphicToolAttribute& HGSGraphicToolCapability::GetAttribute() const
    {
    return *m_pAttribute;
    }



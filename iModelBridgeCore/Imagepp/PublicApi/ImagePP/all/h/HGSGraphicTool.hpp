//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicTool.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicTool
//---------------------------------------------------------------------------------------

#include "HGSGraphicToolAttribute.h"

//-----------------------------------------------------------------------------
// public
// GetAttributes
//-----------------------------------------------------------------------------
inline const HGSGraphicToolAttributes& HGSGraphicTool::GetAttributes() const
    {
    return m_Attributes;
    }

//-----------------------------------------------------------------------------
// public
// Implementation
//-----------------------------------------------------------------------------
inline HGSGraphicToolImplementation* HGSGraphicTool::GetImplementation() const
    {
    return m_pImplementation;
    }

//-----------------------------------------------------------------------------
// public
// GetSurface
//-----------------------------------------------------------------------------
inline HFCPtr<HGSSurface> HGSGraphicTool::GetSurface() const
    {
    return m_pSurface;
    }

//-----------------------------------------------------------------------------
// public
// AddAttribute
//-----------------------------------------------------------------------------
inline void HGSGraphicTool::AddAttribute(const HGSGraphicToolAttribute& pi_rAttribute)
    {
    m_Attributes.Add(pi_rAttribute);
    }


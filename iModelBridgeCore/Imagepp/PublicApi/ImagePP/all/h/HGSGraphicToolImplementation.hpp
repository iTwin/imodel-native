//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolImplementation.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicToolImplementation
//---------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// GetSurfaceImplementation
//-----------------------------------------------------------------------------
inline HGSSurfaceImplementation* HGSGraphicToolImplementation::GetSurfaceImplementation() const
    {
    return m_pSurfaceImplementation;
    }

//-----------------------------------------------------------------------------
// public
// GetAttributes
//-----------------------------------------------------------------------------
inline const HGSGraphicToolAttributes* HGSGraphicToolImplementation::GetAttributes() const
    {
    return m_pAttributes;
    }

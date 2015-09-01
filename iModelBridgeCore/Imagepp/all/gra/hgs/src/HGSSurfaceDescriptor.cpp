//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSSurfaceDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSSurfaceDescriptor
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGSSurfaceDescriptor.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSSurfaceDescriptor::HGSSurfaceDescriptor(uint32_t pi_Width,
                                           uint32_t pi_Height)
    {
    m_Width         = pi_Width;
    m_Height        = pi_Height;
    m_DataWidth     = pi_Width;
    m_DataHeight    = pi_Height;
    m_OffsetX       = 0;
    m_OffsetY       = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSSurfaceDescriptor::~HGSSurfaceDescriptor()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetWidthToHeightUnitRatio
//-----------------------------------------------------------------------------
double HGSSurfaceDescriptor::GetWidthToHeightUnitRatio() const
    {
    // default
    return 1.0;
    }


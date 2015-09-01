//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelNeighbourhood.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class: HRPPixelNeighbourhood
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelNeighbourhood.h>

//-----------------------------------------------------------------------------
// public
// Default constructor.
//-----------------------------------------------------------------------------
HRPPixelNeighbourhood::HRPPixelNeighbourhood()
    {
    m_Width = 1;
    m_Height = 1;
    m_XOrigin = 0;
    m_YOrigin = 0;
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPPixelNeighbourhood::HRPPixelNeighbourhood(uint32_t pi_Width,
                                             uint32_t pi_Height,
                                             uint32_t pi_XOrigin,
                                             uint32_t pi_YOrigin)
    {
    HPRECONDITION(pi_XOrigin < pi_Width);
    HPRECONDITION(pi_YOrigin < pi_Height);

    // set the dimensions and origin of the Neighbourhood
    m_Width = pi_Width;
    m_Height = pi_Height;
    m_XOrigin = pi_XOrigin;
    m_YOrigin = pi_YOrigin;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor.
//-----------------------------------------------------------------------------
HRPPixelNeighbourhood::HRPPixelNeighbourhood(
    const HRPPixelNeighbourhood& pi_rNeighbourhood)
    {
    // copy the Neighbourhood
    m_Width = pi_rNeighbourhood.m_Width;
    m_Height = pi_rNeighbourhood.m_Height;
    m_XOrigin = pi_rNeighbourhood.m_XOrigin;
    m_YOrigin = pi_rNeighbourhood.m_YOrigin;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPPixelNeighbourhood::~HRPPixelNeighbourhood()
    {
    }

//-----------------------------------------------------------------------------
// public
// Equal operator
//-----------------------------------------------------------------------------
HRPPixelNeighbourhood& HRPPixelNeighbourhood::operator=(const HRPPixelNeighbourhood& pi_rObj)
    {
    // copy the Neighbourhood attributes
    m_Width = pi_rObj.m_Width;
    m_Height = pi_rObj.m_Height;
    m_XOrigin = pi_rObj.m_XOrigin;
    m_YOrigin = pi_rObj.m_YOrigin;

    return *this;
    }


//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelNeighbourhood.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRPPixelNeighbourhood
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// GetHeight
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelNeighbourhood::GetHeight() const
    {
    return(m_Height);
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelNeighbourhood::GetWidth() const
    {
    return(m_Width);
    }

//-----------------------------------------------------------------------------
// public
// GetXOrigin
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelNeighbourhood::GetXOrigin() const
    {
    return(m_XOrigin);
    }

//-----------------------------------------------------------------------------
// public
// GetYOrigin
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelNeighbourhood::GetYOrigin() const
    {
    return(m_YOrigin);
    }

//-----------------------------------------------------------------------------
// public
// IsUnity
//-----------------------------------------------------------------------------
inline bool HRPPixelNeighbourhood::IsUnity() const
    {
    return((m_Width == 1 && m_Height == 1));
    }

//-----------------------------------------------------------------------------
// public
// operator+
//-----------------------------------------------------------------------------
inline HRPPixelNeighbourhood HRPPixelNeighbourhood::operator+
(const HRPPixelNeighbourhood& pi_rNeighbourhood) const
    {
    return(HRPPixelNeighbourhood(m_Width  + pi_rNeighbourhood.m_Width - 1,
                                 m_Height + pi_rNeighbourhood.m_Height - 1,
                                 m_XOrigin + pi_rNeighbourhood.m_XOrigin,
                                 m_YOrigin + pi_rNeighbourhood.m_YOrigin));
    }
END_IMAGEPP_NAMESPACE

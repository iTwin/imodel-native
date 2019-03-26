//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DProjectiveGrid.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetDirectStep
//-----------------------------------------------------------------------------
inline double HGF2DProjectiveGrid::GetDirectStep() const
    {
    return(m_DirectStep);
    }

//-----------------------------------------------------------------------------
// GetInverseStep
//-----------------------------------------------------------------------------
inline double HGF2DProjectiveGrid::GetInverseStep() const
    {
    return(m_InverseStep);
    }


END_IMAGEPP_NAMESPACE
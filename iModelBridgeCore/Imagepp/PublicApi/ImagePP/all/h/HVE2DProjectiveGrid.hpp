//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DProjectiveGrid.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetDirectStep
//-----------------------------------------------------------------------------
inline double HVE2DProjectiveGrid::GetDirectStep() const
    {
    return(m_DirectStep);
    }

//-----------------------------------------------------------------------------
// GetInverseStep
//-----------------------------------------------------------------------------
inline double HVE2DProjectiveGrid::GetInverseStep() const
    {
    return(m_InverseStep);
    }

END_IMAGEPP_NAMESPACE

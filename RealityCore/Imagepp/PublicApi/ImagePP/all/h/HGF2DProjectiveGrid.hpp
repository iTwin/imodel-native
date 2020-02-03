//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
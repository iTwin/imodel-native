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

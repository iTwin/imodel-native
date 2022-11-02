/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::IsClosedVolume () const
    {
    return m_capped;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::IsClosedVolume () const
    {
    return m_capped || Angle::IsFullCircle (m_sweepAngle);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::IsClosedVolume () const
    {
    double phi0, phi1, z0, z1;
    return m_capped ||
        !GetSweepLimits (phi0, phi1, z0, z1, true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::IsClosedVolume () const
    {
    return m_capped;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::IsClosedVolume () const
    {
    return m_capped;
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::IsClosedVolume () const
    {
    if (m_capped || Angle::IsFullCircle (fabs (m_sweepAngle)))
        return m_baseCurve->IsAnyRegionType ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::IsClosedVolume () const
    {
    // hm .. just take the cap flag. This will be wrong if both end contours are degenerate (point)
    return m_capped;
    }


END_BENTLEY_GEOMETRY_NAMESPACE


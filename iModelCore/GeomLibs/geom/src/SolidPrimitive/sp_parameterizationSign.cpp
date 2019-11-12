/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static double signum (double a){return a < 0.0 ? -1.0 : 1.0;}
static double signum (double a, double b){return a * b < 0.0 ? -1.0 : 1.0;}
static double signum (double a, double b, double c){return a * b * c < 0.0 ? -1.0 : 1.0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DgnConeDetail::ParameterizationSign () const
    {
    return signum( m_vector0.TripleProduct (m_vector90, DVec3d::FromStartEnd (m_centerA, m_centerB)));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DgnTorusPipeDetail::ParameterizationSign () const
    {
    // m_vectorX and m_vectorY are always crossed to get right handed Z, so only the sweep angle affects anything.
    return signum (m_sweepAngle);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DgnSphereDetail::ParameterizationSign () const
    {
    return signum (
            m_localToWorld.Determinant (),
            m_latitudeSweep
            );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DgnBoxDetail::ParameterizationSign () const
    {
    return signum
            (
            m_vectorX.TripleProduct (m_vectorY, DVec3d::FromStartEnd (m_baseOrigin, m_topOrigin)),
            m_baseX,
            m_topX
            );
    }


END_BENTLEY_GEOMETRY_NAMESPACE


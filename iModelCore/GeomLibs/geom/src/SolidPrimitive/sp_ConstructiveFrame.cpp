/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_ConstructiveFrame.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    DPoint3d center;
    RotMatrix axes;
    double radiusA, radiusB, sweep;
    if (TryGetFrame (center, axes, radiusA, radiusB, sweep))
        {
        localToWorld.InitFrom (axes, center);
        if (worldToLocal.InverseOf (localToWorld))
            return true;
        }
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    DVec3d planeNormal = DVec3d::FromCrossProduct (m_vector0, m_vector90);
    RotMatrix axes;
    axes.InitFromColumnVectors (m_vector0, m_vector90, planeNormal);
    if (axes.SquareAndNormalizeColumns (axes, 0, 1))
        {
        localToWorld.InitFrom (axes, m_centerA);
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        return true;
        }
    
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    Transform localToWorldA, worldToLocalA;
    if (GetTransforms (localToWorldA, worldToLocalA))
        {
        RotMatrix axes;
        axes.InitFrom (localToWorldA);
        DPoint3d center;
        localToWorldA.GetTranslation (center);
        if (axes.SquareAndNormalizeColumns (axes, 0, 1))
            {
            localToWorld.InitFrom (axes, center);
            worldToLocal.InvertRigidBodyTransformation (localToWorld);
            return true;
            }
        }
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    Transform skewFrame;
    double ax, ay, bx, by;
    GetNonUniformTransform (skewFrame, ax, ay, bx, by);
    RotMatrix matrix;
    DPoint3d origin;
    skewFrame.GetMatrix (matrix);
    skewFrame.GetTranslation (origin);
    if (matrix.SquareAndNormalizeColumns (matrix, 0, 1))
        {
        localToWorld.InitFrom (matrix, origin);
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        return true;
        }
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }

static bool FlipYZForZPreference (TransformCR frame, DVec3dCR zVector, TransformR localToWorld, TransformR worldToLocal)
    {
    RotMatrix matrix;
    DPoint3d origin;
    frame.GetMatrix (matrix);
    frame.GetTranslation (origin);
    if (zVector.DotProductColumn (matrix, 2) < 0.0)
        matrix.ScaleColumns (1.0, -1.0, -1.0);
    if (matrix.SquareAndNormalizeColumns (matrix, 0, 1))
        {
        localToWorld.InitFrom (matrix, origin);
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        return true;
        }
    return false;
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    Transform frenet;
    if (m_baseCurve->GetAnyFrenetFrame (frenet))
        {
        if (FlipYZForZPreference (frenet, m_extrusionVector, localToWorld, worldToLocal))
            return true;
        }
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2015
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::TryGetZExtrusionFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    DPoint3d xyz0, xyz1;
    DVec3d unitX, unitY, unitZ;
    if (   m_baseCurve->GetStartEnd (xyz0, xyz1)
        && m_extrusionVector.GetNormalizedTriad (unitX, unitY, unitZ)
        )
        {
        localToWorld = Transform::FromOriginAndVectors (xyz0, unitX, unitY, unitZ);
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        return true;
        }

    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    Transform frenet;
    if (m_sectionCurves.size () > 0
        && m_sectionCurves[0]->GetAnyFrenetFrame (frenet))
        {
        DVec3d zVector;
        DPoint3d point0, point1;
        if (m_sectionCurves.size () > 1
            && m_sectionCurves[0]->GetStartPoint (point0)
            && m_sectionCurves[1]->GetStartPoint (point1)
            )
            {
            zVector = DVec3d::FromStartEnd (point0, point1);
            }
        else
            {
            frenet.GetMatrixColumn (zVector, 2);    // so no flip happens
            }
        if (FlipYZForZPreference (frenet, zVector, localToWorld, worldToLocal))
            return true;
        }
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::TryGetConstructiveFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    if (GetTransforms (localToWorld, worldToLocal))
        return true;
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }
END_BENTLEY_GEOMETRY_NAMESPACE



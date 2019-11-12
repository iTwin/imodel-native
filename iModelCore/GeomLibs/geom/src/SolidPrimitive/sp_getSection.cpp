/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr EllipseSection_ParallelToVector0(DEllipse3dCR ellipse, double yFraction)
    {
    if (yFraction < -1.0 || yFraction > 1.0)
        return ICurvePrimitivePtr ();
    double theta = asin (-1.0 + 2.0 * yFraction);
    if (yFraction < 0.0)
        theta = - theta;
    DSegment3d segment;
    ellipse.Evaluate (segment.point[0], Angle::Pi () - theta);
    ellipse.Evaluate (segment.point[1], theta);
    return ICurvePrimitive::CreateLine (segment);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr EllipseSection_ParallelToVector90(DEllipse3dCR ellipse, double xFraction)
    {
    if (xFraction < -1.0 || xFraction > 1.0)
        return ICurvePrimitivePtr ();
    double theta = acos (-1.0 + 2.0 * xFraction);
    DSegment3d segment;
    ellipse.Evaluate (segment.point[0], -theta);
    ellipse.Evaluate (segment.point[1], theta);
    return ICurvePrimitive::CreateLine (segment);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static ICurvePrimitivePtr CurveVectorSection(CurveVectorCR region, double fraction, bool verticalCut, TransformCP transform)
    {
    Transform localToWorld, worldToLocal;
    DRange3d range;
    CurveVectorPtr localCurves = region.CloneInLocalCoordinates
                (
                LOCAL_COORDINATE_SCALE_01RangeBothAxes,
                localToWorld, worldToLocal,
                range
                );
    DPlane3d sectionPlane;
    localToWorld.Multiply (sectionPlane.origin, fraction, fraction, 0.0);
    localToWorld.GetMatrixColumn (sectionPlane.normal, verticalCut ? 0 : 1);
    ICurvePrimitivePtr section = region.PlaneSection (sectionPlane, 0.0);
    if (NULL != transform)
        section->TransformInPlace (*transform);
    return section;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnConeDetail::GetConstantVSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {

    DEllipse3d ellipse;
    if (select.IsCap0 () && IsRealCap (0) && FractionToSection (0.0, ellipse))
        return EllipseSection_ParallelToVector0 (ellipse, fraction);
    else if (select.IsCap1 () && IsRealCap (1) && FractionToSection (1.0, ellipse))
        return EllipseSection_ParallelToVector0 (ellipse, fraction);
    else if (select.Is (0,0))
        {
        FractionToSection(fraction, ellipse);
        return ICurvePrimitive::CreateArc (ellipse);
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnConeDetail::GetConstantUSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {

    DEllipse3d ellipse;
    if (select.IsCap0 () && IsRealCap (0) && FractionToSection (0.0, ellipse))
        return EllipseSection_ParallelToVector90 (ellipse, fraction);
    else if (select.IsCap1 () && IsRealCap (1) && FractionToSection (1.0, ellipse))
        return EllipseSection_ParallelToVector90 (ellipse, fraction);
    else if (select.Is (0,0))
        {
        DEllipse3d ellipse0, ellipse1;
        FractionToSection(0.0, ellipse0);
        FractionToSection(1.0, ellipse1);
        DSegment3d segment;
        ellipse0.FractionParameterToPoint (segment.point[0], fraction);
        ellipse1.FractionParameterToPoint (segment.point[1], fraction);
        return ICurvePrimitive::CreateLine (segment);
        }
    return ICurvePrimitivePtr ();
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnSphereDetail::GetConstantVSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    DEllipse3d ellipse;
    if (select.IsCap0 () && IsRealCap (0))
        {
        ellipse = VFractionToUSectionDEllipse3d (0.0);
        return EllipseSection_ParallelToVector0 (ellipse, fraction);
        }
    else if (select.IsCap1 () && IsRealCap (1))
        {
        ellipse = VFractionToUSectionDEllipse3d (1.0);
        return EllipseSection_ParallelToVector0 (ellipse, fraction);
        }
    else if (select.Is (0,0))
        {
        return ICurvePrimitive::CreateArc (VFractionToUSectionDEllipse3d (fraction));
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnSphereDetail::GetConstantUSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    DEllipse3d ellipse;
    if (select.IsCap0 () && IsRealCap (0))
        {
        ellipse = VFractionToUSectionDEllipse3d (0.0);
        return EllipseSection_ParallelToVector90 (ellipse, fraction);
        }
    else if (select.IsCap1 () && IsRealCap (1))
        {
        ellipse = VFractionToUSectionDEllipse3d (1.0);
        return EllipseSection_ParallelToVector90 (ellipse, fraction);
        }
    else if (select.Is (0,0))
        {
        return ICurvePrimitive::CreateArc (UFractionToVSectionDEllipse3d (fraction));
        }
    return ICurvePrimitivePtr ();
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnTorusPipeDetail::GetConstantVSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    DEllipse3d ellipse;
    if (select.IsCap0 () && m_capped)
        {
        ellipse = VFractionToUSectionDEllipse3d (0.0);
        return EllipseSection_ParallelToVector0 (ellipse, fraction);
        }
    else if (select.IsCap1 () && m_capped)
        {
        ellipse = VFractionToUSectionDEllipse3d (1.0);
        return EllipseSection_ParallelToVector0 (ellipse, fraction);
        }
    else if (select.Is (0,0))
        {
        return ICurvePrimitive::CreateArc (VFractionToUSectionDEllipse3d (fraction));
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnTorusPipeDetail::GetConstantUSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    DEllipse3d ellipse;
    if (select.IsCap0 () && m_capped)
        {
        ellipse = VFractionToUSectionDEllipse3d (0.0);
        return EllipseSection_ParallelToVector90 (ellipse, fraction);
        }
    else if (select.IsCap1 () && m_capped)
        {
        ellipse = VFractionToUSectionDEllipse3d (1.0);
        return EllipseSection_ParallelToVector90 (ellipse, fraction);
        }
    else if (select.Is (0,0))
        {
        return ICurvePrimitive::CreateArc (UFractionToVSectionDEllipse3d (fraction));
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnBoxDetail::GetConstantVSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    BoxFaces cornerData;
    int singleIndex;
    cornerData.Load (*this);
    DPoint3d facePoints[5];
    if (   BoxFaces::TrySingleFaceIndex (select, singleIndex))
        {
        cornerData.Get5PointCCWFace (singleIndex, facePoints);
        DSegment3d segment;
        segment.point[0].Interpolate (facePoints[0], fraction, facePoints[3]);
        segment.point[1].Interpolate (facePoints[1], fraction, facePoints[2]);
        return ICurvePrimitive::CreateLine (segment);
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnBoxDetail::GetConstantUSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    BoxFaces cornerData;
    int singleIndex;
    cornerData.Load (*this);
    DPoint3d facePoints[5];
    if (   BoxFaces::TrySingleFaceIndex (select, singleIndex))
        {
        cornerData.Get5PointCCWFace (singleIndex, facePoints);
        DSegment3d segment;
        segment.point[0].Interpolate (facePoints[0], fraction, facePoints[1]);
        segment.point[1].Interpolate (facePoints[3], fraction, facePoints[1]);
        return ICurvePrimitive::CreateLine (segment);
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnExtrusionDetail::GetConstantVSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    if (select.IsCap0 () && m_capped)
        {
        return CurveVectorSection (*m_baseCurve, fraction, false, NULL);
        }
    else if (select.IsCap1 () && m_capped)
        {
        Transform transform = Transform::From (m_extrusionVector);
        return CurveVectorSection (*m_baseCurve, fraction, false, &transform);
        }
    else if (select.Index0 () == 0 && select.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)select.Index1 ());
        if (curveA.IsValid ())
            {
            ICurvePrimitivePtr curveB = curveA->CloneComponent (select.Index2 ());
            DVec3d partialExtrusionVector = DVec3d::FromScale (m_extrusionVector, fraction);
            Transform transform = Transform::From (partialExtrusionVector);
            curveB->TransformInPlace (transform);
            return curveB;
            }
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnExtrusionDetail::GetConstantUSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    if (select.IsCap0 () && m_capped)
        {
        return CurveVectorSection (*m_baseCurve, fraction, true, NULL);
        }
    else if (select.IsCap1 () && m_capped)
        {
        Transform transform = Transform::From (m_extrusionVector);
        return CurveVectorSection (*m_baseCurve, fraction, true, &transform);
        }
    else if (select.Index0 () == 0 && select.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)select.Index1 ());
        if (curveA.IsValid ())
            {
            DSegment3d segment;
            curveA->ComponentFractionToPoint ((int)select.Index2 (), fraction, segment.point[0]);
            segment.point[1].SumOf (segment.point[0], m_extrusionVector);
            return ICurvePrimitive::CreateLine (segment);
            }
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnRotationalSweepDetail::GetConstantVSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    if (select.IsCap0 () && m_capped)
        {
        return CurveVectorSection (*m_baseCurve, fraction, false, NULL);
        }
    else if (select.IsCap1 () && m_capped)
        {
        Transform transform;
        Transform derivativeTransform;
        if (GetVFractionTransform (1.0, transform, derivativeTransform))
            {
            return CurveVectorSection (*m_baseCurve, fraction, false, &transform);
            }
        }
    else if (select.Index0 () == 0 && select.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)select.Index1 ());
        if (curveA.IsValid ())
            {
            Transform transform;
            Transform derivativeTransform;
            ICurvePrimitivePtr curveB = curveA->CloneComponent (select.Index2 ());
            if (GetVFractionTransform (fraction, transform, derivativeTransform))
                {
                curveB->TransformInPlace (transform);
                return curveB;
                }
            }
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnRotationalSweepDetail::GetConstantUSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    if (select.IsCap0 () && m_capped)
        {
        return CurveVectorSection (*m_baseCurve, fraction, true, NULL);
        }
    else if (select.IsCap1 () && m_capped)
        {
        Transform transform;
        Transform derivativeTransform;
        if (GetVFractionTransform (1.0, transform, derivativeTransform))
            {
            return CurveVectorSection (*m_baseCurve, fraction, true, &transform);
            }
        }
    else if (select.Index0 () == 0 && select.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)select.Index1 ());
        if (   curveA.IsValid ())
            {
            DPoint3d point0;
            double axisFraction;
            curveA->ComponentFractionToPoint (select.Index2 (), fraction, point0);
            DEllipse3d arc;
            RotMatrix rotate90;
            m_axisOfRotation.ProjectPointUnbounded (arc.center, axisFraction, point0);
            arc.vector0 = DVec3d::FromStartEnd (arc.center, point0);
            rotate90 = RotMatrix::FromRotate90 (m_axisOfRotation.direction);
            rotate90.Multiply (arc.vector90, arc.vector0);
            arc.start = 0.0;
            arc.sweep = m_sweepAngle;
            return ICurvePrimitive::CreateArc (arc);
            }
        }
    return ICurvePrimitivePtr ();
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnRuledSweepDetail::GetConstantVSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    if (m_sectionCurves.size () == 0)
        {
        }
    else if (select.IsCap0 () && m_capped)
        {
        return CurveVectorSection (*m_sectionCurves.front(), fraction, false, NULL);
        }
    else if (select.IsCap1 () && m_capped)
        {
        return CurveVectorSection (*m_sectionCurves.back (), fraction, false, NULL);
        }
    else if (select.Index1 () >= 0)
        {
        size_t index0 = (size_t)select.Index0 ();
        size_t index1 = (size_t)select.Index1 ();
        if (index0 < m_sectionCurves.size () - 1)
            {
            ICurvePrimitivePtr curveA = m_sectionCurves[index0]->FindIndexedLeaf (index1);
            ICurvePrimitivePtr curveB = m_sectionCurves[index0+1]->FindIndexedLeaf (index1);
            if (curveA.IsValid () && curveB.IsValid ())
                {
                ICurvePrimitivePtr curveAB = ICurvePrimitive::CreateInterpolationBetweenCurves (*curveA, fraction, *curveB);
                return curveAB->CloneComponent (select.Index2 ());
                // need curve at fraction between curves
                }
            }
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr DgnRuledSweepDetail::GetConstantUSection (SolidLocationDetail::FaceIndices const &select, double fraction) const
    {
    if (m_sectionCurves.size () == 0)
        {
        }
    else if (select.IsCap0 () && m_capped)
        {
        return CurveVectorSection (*m_sectionCurves.front(), fraction, true, NULL);
        }
    else if (select.IsCap1 () && m_capped)
        {
        return CurveVectorSection (*m_sectionCurves.back (), fraction, true, NULL);
        }
    else if (select.Index1 () >= 0)
        {
        size_t index0 = (size_t)select.Index0 ();
        size_t index1 = (size_t)select.Index1 ();
        if (index0 < m_sectionCurves.size () - 1)
            {
            ICurvePrimitivePtr curveA = m_sectionCurves[index0]->FindIndexedLeaf (index1);
            ICurvePrimitivePtr curveB = m_sectionCurves[index0+1]->FindIndexedLeaf (index1);
            if (curveA.IsValid () && curveB.IsValid ())
                {
                DSegment3d segment;
                curveA->ComponentFractionToPoint ((int)select.Index2 (), fraction, segment.point[0]);
                curveB->ComponentFractionToPoint ((int)select.Index2 (), fraction, segment.point[1]);
                return ICurvePrimitive::CreateLine (segment);
                }
            }
        }
    return ICurvePrimitivePtr ();
    }

END_BENTLEY_GEOMETRY_NAMESPACE



/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_silhouette.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnConeDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    double      lambda;
    DVec3d      xAxis, yAxis, zAxis;
    DPoint3d    origin;
    Transform   frame;
    DCone3d     cone;

    // Pick larger radius end as xy plane of the coordinate frame...
    if (m_radiusA < m_radiusB)
        {
        origin = m_centerB;
        zAxis.DifferenceOf (m_centerA, m_centerB);
        xAxis.Scale (m_vector0, m_radiusB);
        yAxis.Scale (m_vector90, m_radiusB);
        lambda = m_radiusA / m_radiusB;
        }
    else
        {
        origin = m_centerA;
        zAxis.DifferenceOf (m_centerB, m_centerA);
        xAxis.Scale (m_vector0, m_radiusA);
        yAxis.Scale (m_vector90, m_radiusA);
        lambda = m_radiusB / m_radiusA;
        }

    frame.InitFromOriginAndVectors (origin, xAxis, yAxis, zAxis);
    bsiDCone3d_setFrameAndFraction (&cone, &frame, lambda, NULL);

    DPoint3d    trigPointBuffer[2];

    if (2 != bsiDCone3d_silhouetteAngles (&cone, trigPointBuffer, NULL, &eyePoint))
        return nullptr;
    DSegment3d segmentA, segmentB;
    bsiDCone3d_getRuleLine (&cone, &segmentA, trigPointBuffer[0].z);
    bsiDCone3d_getRuleLine (&cone, &segmentB, trigPointBuffer[1].z);
    auto curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    curves->push_back (ICurvePrimitive::CreateLine (segmentA));
    curves->push_back (ICurvePrimitive::CreateLine (segmentB));
    return curves;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnSphereDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    Transform localToWorld = this->m_localToWorld;
    Transform worldToLocal;
    if (worldToLocal.InverseOf (localToWorld))
        {
        DMap4d hMap;
        hMap.InitFromTransform (localToWorld, false);
        DEllipse4d hEllipse;
        DEllipse3d ellipse;
        bsiGeom_ellipsoidSilhouette (&hEllipse, nullptr, &hMap, &eyePoint);
        if (bsiDEllipse3d_initFromDEllipse4d (&ellipse, &hEllipse, -1))
            {
            CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
            cv->push_back (ICurvePrimitive::CreateArc (ellipse));
            return cv;
            }
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnBoxDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnTorusPipeDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnExtrusionDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnRotationalSweepDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnRuledSweepDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    return nullptr;
    }
END_BENTLEY_GEOMETRY_NAMESPACE


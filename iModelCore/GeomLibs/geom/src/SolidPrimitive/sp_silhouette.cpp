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

void cb_silhouettePoints (
HConic              *pConic,
DPoint3dP pPointArray,
int                 numPoint,
unsigned int        curveMask,
const RotatedConic  *pSurface,      /* => the rotated conic surface */
CurveVectorP         pCurves
)
    {
    if (numPoint > 1)
        pCurves->push_back (ICurvePrimitive::CreateLineString (pPointArray, numPoint));
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
CurveVectorPtr DgnTorusPipeDetail::SilhouetteCurves(DPoint4dCR eyePoint) const
    {
    DToroid3d toroid;
    DRange2d parameterRange;
    parameterRange.low.x = 0.0;
    parameterRange.high.x = m_sweepAngle;
    parameterRange.low.y = 0.0;
    parameterRange.high.y = Angle::TwoPi ();
    double minorRadiusRatio;
    double tolerance = 0.0001 * (fabs (m_majorRadius)  + fabs (m_minorRadius));
    DPoint3d center;
    RotMatrix axes;
    double radiusA, radiusB, sweep;
    if (TryGetFrame (center, axes, radiusA, radiusB, sweep)
        && DoubleOps::SafeDivideParameter (minorRadiusRatio, radiusB, radiusA))
        {
        axes.ScaleColumns (radiusA, radiusA, radiusB);
        Transform localToWorld;
        localToWorld.InitFrom (axes, center);
        bsiDToroid3d_set (&toroid, &localToWorld, minorRadiusRatio, &parameterRange);
        RotatedConic rc;
        bsiDToroid3d_getRotatedConic (&toroid, &rc);
        CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        HConic conics[2];
        int numConic;
        bool isEllipse, isSegment;
        DEllipse3d ellipse;
        DSegment3d segment;
        if (SUCCESS == bsiRotatedConic_interiorConicSilhouette (conics, &numConic, &rc, &eyePoint))
            {
            for (int i = 0; i < numConic; i++)
                {
                for (int j = 0; j < conics[i].coordinates.sectors.n; j++)
                    {
                    MSBsplineCurve bcurve;
                    if (SUCCESS == bspcurv_simplifyHConic (&bcurve, &ellipse, &isEllipse, &segment, &isSegment, &conics[i],
                        conics[i].coordinates.sectors.interval[j].minValue,
                        conics[i].coordinates.sectors.interval[j].maxValue
                        ))
                        {
                        bool useBcurve = true;
                        if (isEllipse)
                            {
                            curves->push_back (ICurvePrimitive::CreateArc (ellipse));
                            useBcurve = false;
                            }
                        if (isSegment)
                            {
                            curves->push_back (ICurvePrimitive::CreateLine (segment));
                            }
                        if (bcurve.GetIntNumPoles () > 0)
                            curves->push_back (ICurvePrimitive::CreateBsplineCurveSwapFromSource (bcurve));
                        else
                            {
                            bcurve.ReleaseMem ();
                            }
                        }
                    }
                }
            }
        else
            bsiRotatedConic_torusGeneralSilhouette (&rc, &eyePoint, (SilhouetteArrayHandler)cb_silhouettePoints, tolerance, curves.get ());
        if (curves->size () > 0)
            return curves;
        }
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


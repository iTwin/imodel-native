/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_silhouette.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static const DRange2d s_defaultParameterRange =
            {
                {-msGeomConst_pi, -msGeomConst_pi},
                { msGeomConst_pi,  msGeomConst_pi}
            };
/*---------------------------------------------------------------------------------**//**
* Initialize an toroid from full frame and range
* @param pTransform         => coordinate frame.  If NULL, default is applied.
* @param minorRadiusRatio   => radius of minor circles in the local coordinate system
*                               where major radius is 1.
* @param pRange             => parameter range.  If NULL, default is applied.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void               bsiDToroid3d_set

(
DToroid3dP pInstance,
TransformCP pFrame,
double          minorRadiusRatio,
DRange2dCP pParameterRange
)
    {
    bsiDToroid3d_setFrame (pInstance, pFrame);
    bsiDToroid3d_setNaturalParameterRange (pInstance, pParameterRange);
    pInstance->minorAxisRatio = minorRadiusRatio;
    }
/*---------------------------------------------------------------------------------**//**
* Return the range of the natural parameter for the active surface patch.
* @param pParam1Start => start value of natural parameter.
* @param pParam1End   => end value of natural parameter.
* @param pParam2Start => start value of natural parameter.
* @param pParam2End   => end value of natural parameter.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_getScalarNaturalParameterRange

(
DToroid3dCP pToroid,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
)
    {
    *pParam1Start = pToroid->parameterRange.low.x;
    *pParam1End   = pToroid->parameterRange.high.x;
    *pParam2Start = pToroid->parameterRange.low.y;
    *pParam2End   = pToroid->parameterRange.high.y;
    }


/*---------------------------------------------------------------------------------**//**
* Set the reference frame of the toroid.
* @param pFrame => coordinate frame.  null indicates an identity transformation.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void               bsiDToroid3d_setFrame

(
DToroid3dP pInstance,
TransformCP pFrame
)
    {
    if (pFrame)
        pInstance->frame = *pFrame;
    else
        pInstance->frame.InitIdentity ();
    }


/*---------------------------------------------------------------------------------**//**
* Set the parameter range of the toroid.
* @param pParameterRange => limits of longitude and latitude.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void               bsiDToroid3d_setNaturalParameterRange

(
DToroid3dP pInstance,
DRange2dCP pParameterRange
)
    {
    pInstance->parameterRange = pParameterRange ? *pParameterRange : s_defaultParameterRange;
    }

/*---------------------------------------------------------------------------------**//**
* Get the parameter range as start/sweep pairs.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDToroid3d_getScalarNaturalParameterSweep

(
DToroid3dCP pInstance,
double          *pTheta0,
double          *pThetaSweep,
double          *pPhi0,
double          *pPhiSweep
)
    {
    *pTheta0 = pInstance->parameterRange.low.x;
    *pThetaSweep = pInstance->parameterRange.high.x - *pTheta0;

    *pPhi0 = pInstance->parameterRange.low.y;
    *pPhiSweep = pInstance->parameterRange.high.y - *pPhi0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    curves = nullptr;
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
        return true;
    DSegment3d segmentA, segmentB;
    bsiDCone3d_getRuleLine (&cone, &segmentA, trigPointBuffer[0].z);
    bsiDCone3d_getRuleLine (&cone, &segmentB, trigPointBuffer[1].z);
    curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    curves->push_back (ICurvePrimitive::CreateLine (segmentA));
    curves->push_back (ICurvePrimitive::CreateLine (segmentB));
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    curves = nullptr;
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
            curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
            curves->push_back (ICurvePrimitive::CreateArc (ellipse));
            }
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    return false;
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
bool DgnTorusPipeDetail::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    curves = nullptr;
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
        axes.ScaleColumns (radiusA, radiusA, radiusA);
        Transform localToWorld;
        localToWorld.InitFrom (axes, center);
        bsiDToroid3d_set (&toroid, &localToWorld, minorRadiusRatio, &parameterRange);
        RotatedConic rc;
        bsiDToroid3d_getRotatedConic (&toroid, &rc);
        curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        HConic conics[10];
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
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2017
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::SilhouetteCurves(DPoint4dCR eyePoint, CurveVectorPtr &curves) const
    {
    return false;
    }
END_BENTLEY_GEOMETRY_NAMESPACE


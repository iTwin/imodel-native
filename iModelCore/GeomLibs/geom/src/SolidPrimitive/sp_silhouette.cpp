/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include "../DeprecatedFunctions.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static const DRange2d s_defaultParameterRange =
            {
                {-msGeomConst_pi, -msGeomConst_pi},
                { msGeomConst_pi,  msGeomConst_pi}
            };


Public bool               bsiDToroid3d_getRotatedConic
(
DToroid3dCP pInstance,
RotatedConic      *pConic
);
Public StatusInt    bsiRotatedConic_torusGeneralSilhouette
(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pEyePoint,     /* => eyepoint (nonzero weight) or view vector (zero weight) */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
);

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
Public void    bsiDToroid3d_getScalarNaturalParameterSweep

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
* Compute exact silhouette arcs.
* Return false if exact form is not known.
* @bsihdr                                                       EarlinLutz      04/18
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool bsiDToroid3d_addExactSilhouettes (DToroid3dCR instance, CurveVectorPtr &curves, DPoint4dCR eyePoint)
    {
    double fOuter = 1.0 + instance.minorAxisRatio;
    double fInner = 1.0 - instance.minorAxisRatio;
    double startRadians = instance.parameterRange.low.x;
    double sweepRadians = instance.parameterRange.high.x - instance.parameterRange.low.x;
    double startPhi = 0.0;
    double sweepPhi = Angle::TwoPi ();

    if (eyePoint.w != 0.0)
        return false;
    DVec3d viewDirection = DVec3d::From (eyePoint.x, eyePoint.y, eyePoint.z);
    DVec3d xVec, yVec, zVec;
    DPoint3d origin;
    instance.frame.GetOriginAndVectors (origin, xVec, yVec, zVec);
    if (   xVec.IsPerpendicularTo (viewDirection)
        && yVec.IsPerpendicularTo (viewDirection)
        && zVec.IsParallelTo (viewDirection))
        {
        // simple "top" view -- inside and outside of donut ..
        // NOTE: Toroid from TorusPipe has full phi range
        curves->Add (ICurvePrimitive::CreateArc (
                DEllipse3d::FromVectors (origin, xVec * fOuter, yVec * fOuter, startRadians, sweepRadians)));
        curves->Add (ICurvePrimitive::CreateArc (
                DEllipse3d::FromVectors (origin, xVec * fInner, yVec * fInner, startRadians, sweepRadians)));
        return true;
        }
    auto perpVector = DVec3d::FromCrossProduct (xVec, yVec);
    if (perpVector.IsPerpendicularTo (viewDirection))
        {
        // view parallel to xy plane.
        // theta circles at top and bottom . . .
        auto centerA = origin + zVec * instance.minorAxisRatio;
        auto centerB = origin - zVec * instance.minorAxisRatio;
        curves->Add (ICurvePrimitive::CreateArc (
                DEllipse3d::FromVectors (centerA, xVec, yVec, startRadians, sweepRadians)));
        curves->Add (ICurvePrimitive::CreateArc (
                DEllipse3d::FromVectors (centerB, xVec, yVec, startRadians, sweepRadians)));
        auto xDotView = xVec.DotProduct (viewDirection);
        auto yDotView = yVec.DotProduct (viewDirection);
        // cos(theta) * xDotView + sin(theta) * yDotView = 0
        auto theta = atan2 (-xDotView, yDotView);
        double c = cos (theta);
        double s = sin (theta);
        auto radiusC = xVec * c + yVec * s;
        auto centerC = origin + radiusC;
        auto centerD = origin - radiusC;
        auto vector90 = instance.minorAxisRatio * zVec;
        if (Angle::InSweepAllowPeriodShift (theta, startRadians, sweepRadians))
            curves->Add (ICurvePrimitive::CreateArc (
                DEllipse3d::FromVectors (centerC, radiusC * instance.minorAxisRatio, vector90, startPhi, sweepPhi)));
        if (Angle::InSweepAllowPeriodShift (theta + Angle::Pi (), startRadians, sweepRadians))
            curves->Add (ICurvePrimitive::CreateArc (
                DEllipse3d::FromVectors (centerD, radiusC * instance.minorAxisRatio, vector90, startPhi, sweepPhi)));
        return true;
        }
    return false;
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
        bvector<DEllipse3d> arcs;
        if (bsiDToroid3d_addExactSilhouettes (toroid, curves, eyePoint))
            {
            }
        else
            bsiRotatedConic_torusGeneralSilhouette (&rc, &eyePoint, (SilhouetteArrayHandler)cb_silhouettePoints, tolerance, curves.get ());
        }
    return curves->size () > 0;
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

#define MAX_TOROIDAL_POINT 400
#define MINIMUM_ANGLE_STEP 0.05

typedef struct
    {
    const RotatedConic      *pSurface;
    SilhouetteArrayHandler  handlerFunc;
    void                    *pUserData;
    } ToroidalOutputParams;

enum
    {
    TP_Break,
    TP_PhiLimit,
    TP_ThetaLimit,
    TP_ThetaStep,
    TP_PhiStep
    };

#define TP_BOUNDARY_POINT 0x00000001

typedef struct
    {
    DPoint3d thetaPoint;
    DPoint3d phiPoint;
    double sortCoordinate;
    int mask;
    } ToroidalPoint;

typedef struct _toroidalPointArray ToroidalPointArray;

typedef StatusInt (*ToroidalOutputFunction)(ToroidalPointArray *, void *);
struct _toroidalPointArray
    {
    int numPoint;
    ToroidalPoint pointPair[MAX_TOROIDAL_POINT];
    ToroidalOutputFunction outputFunction;
    void                *pOutputData;
    } ;

typedef enum
    {
    PR_EmptyAngle,
    PR_SweepAngle,
    PR_FullCircle
    } ParamIntervalType;

typedef struct
    {
    ParamIntervalType type;
    double          start;
    double          delta;
    } ParamInterval;
typedef struct
    {
    ParamInterval param[2];
    } ParamRange;
static const double s_toroidalQuadTol = 1.0e-12;
static const double s_toroidalFilterFraction = 0.1;  /* Filter away points closer than this fraction of angular tolerance */
// unused - static const double s_lineUnitCircleIntersectionTolerance = 1.0e-12;

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void jmdlParamInterval_initFromAngleLimits

(
ParamInterval   *pParam,
double          theta0,
double          theta1
)
    {
    double sweep = theta1 - theta0;
    if (Angle::IsFullCircle (sweep))
        {
        pParam->type = PR_FullCircle;
        pParam->start = theta0;
        pParam->delta = sweep > 0.0 ? msGeomConst_2pi : -msGeomConst_2pi;
        }
    else
        {
        pParam->type = PR_SweepAngle;
        pParam->start = theta0;
        pParam->delta = sweep;
        }
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void jmdlParamRange_initFromAngleLimits

(
ParamRange  *pRange,
double      theta0,
double      theta1,
double      phi0,
double      phi1
)
    {
    jmdlParamInterval_initFromAngleLimits (&pRange->param[0], theta0, theta1);
    jmdlParamInterval_initFromAngleLimits (&pRange->param[1], phi0, phi1);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static bool    jmdlParamInterval_containsPoint

(
const ParamInterval   *pParam,
double          value
)
    {
    switch (pParam->type)
        {
        case PR_FullCircle:
            return true;
        case PR_SweepAngle:
            return Angle::InSweepAllowPeriodShift (value, pParam->start, pParam->delta);
        case PR_EmptyAngle:
            return false;
        }
    return  false;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static double jmdlParamInterval_normalizePoint

(
const ParamInterval   *pParam,
double          value
)
    {
    switch (pParam->type)
        {
        case PR_FullCircle:
        case PR_SweepAngle:
            return pParam->start + pParam->delta * bsiTrig_normalizeAngleToSweep (value, pParam->start, pParam->delta);
        case PR_EmptyAngle:
            return value;
        }
    return  0.0;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static bool    jmdlParamRange_containsPoint

(
const ParamRange  *pRange,
double      value0,
double      value1
)
    {
    return    jmdlParamInterval_containsPoint (&pRange->param[0], value0)
           && jmdlParamInterval_containsPoint (&pRange->param[1], value1);
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void jmdlParamRange_normalizePoint

(
double      *pValue0,
double      *pValue1,
const ParamRange  *pRange
)
    {
    *pValue0 = jmdlParamInterval_normalizePoint (&pRange->param[0], *pValue0);
    *pValue1 = jmdlParamInterval_normalizePoint (&pRange->param[1], *pValue1);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static StatusInt       bsiRotatedConic_toroidalFormLimits

(
DPoint3dP pPointArray,       /* <= array of 0 to 4 singular angles, as cosine,sine, theta */
int             *pNumAngle,         /* <= number of singular points */
RotMatrixCP pCoff,                  /* => coefficient matrix */
bool    transpose           /* => true to transpose matrix */
)
    {
    StatusInt status = ERROR;
    double thetaQ[4];
    double cosQ[4];
    double sinQ[4];
    int numQ, i;

    int solutionStatus;

    if (!transpose)
        {
        solutionStatus = bsiMath_conicIntersectUnitCircle (cosQ, sinQ, thetaQ, &numQ,
                                    pCoff->form3d[0][2], pCoff->form3d[0][0], pCoff->form3d[0][1],
                                    pCoff->form3d[1][2], pCoff->form3d[1][0], pCoff->form3d[1][1],
                                    pCoff->form3d[2][2], pCoff->form3d[2][0], pCoff->form3d[2][1]
                                    );
        }
    else
        {
        solutionStatus = bsiMath_conicIntersectUnitCircle (cosQ, sinQ, thetaQ, &numQ,
                                    pCoff->form3d[2][0], pCoff->form3d[0][0], pCoff->form3d[1][0],
                                    pCoff->form3d[2][1], pCoff->form3d[0][1], pCoff->form3d[1][1],
                                    pCoff->form3d[2][2], pCoff->form3d[0][2], pCoff->form3d[1][2]
                                    );
        }

    if (solutionStatus >= 0)
        {
        *pNumAngle = numQ;
        for (i = 0; i < numQ; i++)
            {
            pPointArray[i].x = cosQ[i];
            pPointArray[i].y = sinQ[i];
            pPointArray[i].z = thetaQ[i];
            }
        status = SUCCESS;
        }
    else
        {
        *pNumAngle = 0;
        }

    return status;
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static StatusInt transformAndFlush /* Number of points left in local buffer at end (0 or 1) */

(
DPoint3dP pWorldBuffer, /* <=> world buffer */
DPoint4dP pLocalBuffer, /* <=> local buffer.  DATA IS OVERWRITTEN  */
int             *pNumPoint,     /* <=> point count */
int             recycleLastPoint,   /* => true if last point is to be recycled as 0'th */
const RotatedConic  *pSurface,      /* => the rotated conic surface */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
void                *pUserData      /* => arbitrary pointer */
)
    {
    DPoint4d savedPoint;
    StatusInt status = ERROR;
    int numPoint = *pNumPoint;

    if (numPoint > 0)
        {
        savedPoint = pLocalBuffer[numPoint-1];
        bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pLocalBuffer, pLocalBuffer, *pNumPoint);
        DPoint4d::NormalizeArrayWeights (pWorldBuffer, pLocalBuffer, numPoint);
        status = handlerFunc (NULL, pWorldBuffer, numPoint, RC_CURVEMASK_SMOOTH, pSurface, pUserData);

        if (recycleLastPoint)
            {
            pLocalBuffer[0] = savedPoint;
            numPoint = 1;
            }
        else
            {
            numPoint = 0;
            }
        }

    *pNumPoint = numPoint;
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_clear

(
ToroidalPointArray    *pArray
)
    {
    pArray->numPoint = 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int tp_compareSortCoordinate

(
const void *vpPoint0,
const void *vpPoint1
)
    {
    const ToroidalPoint *pPoint0 = (const ToroidalPoint*)vpPoint0;
    const ToroidalPoint *pPoint1 = (const ToroidalPoint*)vpPoint1;
    if (pPoint0->sortCoordinate < pPoint1->sortCoordinate)
        return -1;
    if (pPoint0->sortCoordinate > pPoint1->sortCoordinate)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_centralSort

(
ToroidalPointArray    *pArray,
double                  thetaMid,
double                  phiMid
)
    {
    int numPoint = pArray->numPoint;
    double phi, theta;
    int i;
    for (i = 0; i < numPoint; i++)
        {
        theta = pArray->pointPair[i].thetaPoint.z;
        phi   = pArray->pointPair[i].phiPoint.z;

        pArray->pointPair[i].sortCoordinate =
                    Angle::Atan2 (phi - phiMid, theta - thetaMid);
        }
    qsort (pArray->pointPair, numPoint, sizeof (ToroidalPoint),
        (int (*)(const void *, const void *))tp_compareSortCoordinate);
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_close

(
ToroidalPointArray    *pArray
)
    {
    ToroidalPoint point;
    if (pArray->numPoint > 0 && pArray->numPoint < MAX_TOROIDAL_POINT)
        {
        point = pArray->pointPair[0];
        pArray->pointPair[pArray->numPoint++] = point;
        }
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_setOutputFunction

(
ToroidalPointArray    *pArray,
ToroidalOutputFunction outputFunction,
void                    *pOutputData
)
    {
    pArray->outputFunction = outputFunction;
    pArray->pOutputData    = pOutputData;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static StatusInt    tpa_output

(
ToroidalPointArray    *pArray
)
    {
    return pArray->outputFunction (pArray, pArray->pOutputData);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static bool                tp_inRanges /* true if point is contained in both ranges */

(
ToroidalPoint         *pPoint,
const ParamRange            *pRange0,
const ParamRange            *pRange1
)
    {
    double theta = pPoint->thetaPoint.z;
    double phi   = pPoint->phiPoint.z;

    if (pRange0 && !jmdlParamRange_containsPoint (pRange0, theta, phi))
        return false;
    if (pRange1 && !jmdlParamRange_containsPoint (pRange1, theta, phi))
        return false;
    return true;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static StatusInt tpa_addTPoint

(
ToroidalPointArray    *pArray,
const ToroidalPoint         *pTPoint
)
    {
    if (pArray->numPoint < MAX_TOROIDAL_POINT)
        {
        pArray->pointPair[pArray->numPoint] = *pTPoint;
        pArray->numPoint++;
        return SUCCESS;
        }
    return ERROR;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int tp_compareTheta

(
const ToroidalPoint *pPoint0,
const ToroidalPoint *pPoint1
)
    {
    if (pPoint0->thetaPoint.z < pPoint1->thetaPoint.z)
        return -1;
    if (pPoint0->thetaPoint.z > pPoint1->thetaPoint.z)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void  tpa_filter

(
ToroidalPointArray  *pArray,
double          dTheta,         /* => filter tolerance for theta */
double          dPhi            /* => filter tolerance for phi */
)
    {
    int i, numKeep, numPoint;
    double ePhi, eTheta;
    double phi0, theta0;
    double phi1, theta1;

    numPoint = pArray->numPoint;

    if (dPhi > 0.0 || dTheta > 0.0 && numPoint > 2)
        {
        int lastI = numPoint - 1;
        numKeep = 1;

        theta0 = pArray->pointPair[0].thetaPoint.z;
        phi0   = pArray->pointPair[0].phiPoint.z;

        for (i = 1; i <= lastI;i++)
            {
            theta1 = pArray->pointPair[i].thetaPoint.z;
            phi1   = pArray->pointPair[i].phiPoint.z;
            eTheta = fabs (theta1 - theta0);
            ePhi = fabs (phi1 - phi0);
            if (  (pArray->pointPair[i].mask & TP_BOUNDARY_POINT)
                || eTheta >= dTheta
                || ePhi >= dPhi
                || i == lastI
                )
                {
                pArray->pointPair[numKeep] = pArray->pointPair[i];
                numKeep++;
                }
            theta0 = theta1;
            phi0   = phi1;
            }
        pArray->numPoint = numKeep;
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void  tpa_normalizeAndSortTheta

(
ToroidalPointArray  *pArray,
const ParamRange    *pRange,
double          dTheta,         /* => filter tolerance for theta */
double          dPhi            /* => filter tolerance for phi */
)
    {
    int i, numPoint;
    numPoint = pArray->numPoint;

    if (pRange)
        {
        for (i = 0; i < numPoint; i++)
            {
            jmdlParamRange_normalizePoint (
                            &pArray->pointPair[i].thetaPoint.z,
                            &pArray->pointPair[i].phiPoint.z,
                            pRange);
            }
        }
    qsort (pArray->pointPair, numPoint, sizeof(ToroidalPoint),
            (int (*)(const void*, const void*))tp_compareTheta
            );

    tpa_filter (pArray, dTheta, dPhi);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void  tpa_normalizeToAngleLimits

(
ToroidalPointArray  *pArray,
double              theta0,
double              theta1,
double              phi0,
double              phi1
)
    {
    int i;
    int numPoint = pArray->numPoint;
    ParamRange range;

    jmdlParamRange_initFromAngleLimits (&range, theta0, theta1, phi0, phi1);

    for (i = 0; i < numPoint; i++)
        {
        jmdlParamRange_normalizePoint (
                        &pArray->pointPair[i].thetaPoint.z,
                        &pArray->pointPair[i].phiPoint.z,
                        &range);
        }
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int         compareZ

(
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    if (pPoint0->z < pPoint1->z)
        return -1;
    if (pPoint0->z > pPoint1->z)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int     outputThetaPhi

(
ToroidalPointArray *pArray,
ToroidalOutputParams *pParams
)
    {
    const RotatedConic *pSurface = pParams->pSurface;
    //StatusInt status = SUCCESS;
    void  *pUserData  = pParams->pUserData;
    double sinPhi, cosPhi, sinTheta, cosTheta;
    ToroidalPoint *pPoint;
    int numPoint = pArray->numPoint;
    int i;
    double a = 1.0;
    double b = pSurface->hoopRadius;
    double rho;
    double thetaMin, thetaMax, phiMin, phiMax, phi1, theta1, phi0, theta0, thetaMid, phiMid;
    ParamRange surfaceRange;
    int numOut;

    DPoint4d point4d[MAX_TOROIDAL_POINT];
    DPoint3d point3d[MAX_TOROIDAL_POINT];

    jmdlParamRange_initFromAngleLimits
                    (
                    &surfaceRange,
                    pSurface->parameterRange.low.x,
                    pSurface->parameterRange.high.x,
                    pSurface->parameterRange.low.y,
                    pSurface->parameterRange.high.y
                    );

    phiMin = thetaMin = 1000.0;
    phiMax = thetaMax = -1000.0;

    numOut = 0;
    theta0 = thetaMid = phi0 = phiMid = 0.0;
    for (i = 0; i < numPoint; i++)
        {
        pPoint = &pArray->pointPair[i];
        cosTheta = pPoint->thetaPoint.x;
        sinTheta = pPoint->thetaPoint.y;
        cosPhi = pPoint->phiPoint.x;
        sinPhi = pPoint->phiPoint.y;
        rho = a + b * cosPhi;
        phi1 = pArray->pointPair[i].phiPoint.z;
        theta1 = pArray->pointPair[i].thetaPoint.z;

        if (i > 0)
            {
            thetaMid = 0.5 * (theta0 + theta1);
            phiMid   = 0.5 * (phi0 + phi1);
            }

        theta0 = theta1;
        phi0   = phi1;

        if (phi1 < phiMin)
            phiMin = phi1;
        if (phi1 > phiMax)
            phiMax = phi1;
        if (theta1 < thetaMin)
            thetaMin = theta1;
        if (theta1 > thetaMax)
            thetaMax = theta1;

        if (numOut > 0)
            {
            if (!jmdlParamRange_containsPoint (&surfaceRange, thetaMid, phiMid))
                {
                transformAndFlush (point3d, point4d, &numOut,
                                false,
                                pSurface,
                                pParams->handlerFunc,
                                pUserData);
                numOut = 0;
                }
            }


        point4d[numOut].x = cosTheta * rho;
        point4d[numOut].y = sinTheta * rho;
        point4d[numOut].z = b * sinPhi;
        point4d[numOut].w = 1.0;
        numOut++;
        }

    if (numOut > 0)
        transformAndFlush (point3d, point4d, &numOut,
                        false,
                        pSurface,
                        pParams->handlerFunc,
                        pUserData);


    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Output function for points that have been computed with phi
*  and theta swapped.  Swap them back and call the normal function.
+----------------------------------------------------------------------*/
static int     outputPhiTheta

(
ToroidalPointArray *pArray,
ToroidalOutputParams *pParams
)
    {
    int numPoint = pArray->numPoint;
    int i;
    DPoint3d tempPoint;


    for (i = 0; i < numPoint; i++)
        {
        tempPoint = pArray->pointPair[i].thetaPoint;
        pArray->pointPair[i].thetaPoint = pArray->pointPair[i].phiPoint;
        pArray->pointPair[i].phiPoint = tempPoint;
        }

    return outputThetaPhi (pArray, pParams);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Unbounded theta slice.
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_sliceToroidalFormAtTheta

(
ToroidalPoint   *pPoint,        /* <= 0, 1, or 2 toroidal point */
int             *pNumPoint,     /* <= number of points hit by the slice */
RotMatrixCP pMatrix,       /* => coefficient matrix */
double          theta
)
    {
    DPoint3d thetaPoint, coffPoint, phiPoint;
    double cosPhi[2], sinPhi[2];
    double phi;
    int i, numRoot;

    thetaPoint.x = coffPoint.x = cos (theta);
    thetaPoint.y = coffPoint.y = sin (theta);
    thetaPoint.z = theta;
    coffPoint.z = 1.0;
    pMatrix->Multiply (coffPoint);
    numRoot   = bsiMath_solveApproximateUnitQuadratic (
                                    &cosPhi[0], &sinPhi[0],
                                    &cosPhi[1], &sinPhi[1],
                                    coffPoint.z, coffPoint.x, coffPoint.y, s_toroidalQuadTol);
    *pNumPoint = 0;
    for (i = 0; i < numRoot; i++)
        {
        phi = Angle::Atan2 (sinPhi[i], cosPhi[i]);
        phiPoint.x = cosPhi[i];
        phiPoint.y = sinPhi[i];
        phiPoint.z = phi;
        pPoint[i].phiPoint = phiPoint;
        pPoint[i].thetaPoint = thetaPoint;
        *pNumPoint += 1;
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Unbounded phi slice.
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_sliceToroidalFormAtPhi

(
ToroidalPoint   *pPoint,        /* <= 0, 1, or 2 toroidal point */
int             *pNumPoint,     /* <= number of points hit by the slice */
RotMatrixCP pMatrix,       /* => coefficient matrix */
double          phi
)
    {
    DPoint3d thetaPoint, coffPoint, phiPoint;
    double cosTheta[2], sinTheta[2];
    double theta;
    int i, numRoot;

    phiPoint.x = coffPoint.x = cos (phi);
    phiPoint.y = coffPoint.y = sin (phi);
    phiPoint.z = phi;
    coffPoint.z = 1.0;
    pMatrix->MultiplyTranspose (coffPoint);
    numRoot   = bsiMath_solveApproximateUnitQuadratic (
                                    &cosTheta[0], &sinTheta[0],
                                    &cosTheta[1], &sinTheta[1],
                                    coffPoint.z, coffPoint.x, coffPoint.y, s_toroidalQuadTol);
    *pNumPoint = 0;
    for (i = 0; i < numRoot; i++)
        {
        theta = Angle::Atan2 (sinTheta[i], cosTheta[i]);
        thetaPoint.x = cosTheta[i];
        thetaPoint.y = sinTheta[i];
        thetaPoint.z = theta;
        pPoint[i].phiPoint = phiPoint;
        pPoint[i].thetaPoint = thetaPoint;
        *pNumPoint += 1;
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Find the 0,1, or 2 points where an isoline crosses the contour.
+----------------------------------------------------------------------*/
static int     tpa_addSliceAtPhi

(
ToroidalPointArray *pArray,
RotMatrixCP pMatrix,
double           phi,
ParamRange       *pRange0,
ParamRange       *pRange1,
int              mask
)
    {
    ToroidalPoint tPoint[2];
    int numPoint;
    int i;
    jmdlRotatedConic_sliceToroidalFormAtPhi (tPoint, &numPoint, pMatrix, phi);

    for (i = 0; i < numPoint; i++)
        {
        if (tp_inRanges (&tPoint[i], pRange0, pRange1))
            {
            tPoint[i].mask = mask;
            tpa_addTPoint (pArray, &tPoint[i]);
            }
        }
    return numPoint;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Find the 0,1, or 2 points where an isoline crosses the contour.
+----------------------------------------------------------------------*/
static int     tpa_addSliceAtTheta

(
ToroidalPointArray *pArray,
RotMatrixCP pMatrix,
double           theta,
ParamRange       *pRange0,
ParamRange       *pRange1,
int              mask
)
    {
    ToroidalPoint tPoint[2];
    int numPoint;
    int i;
    jmdlRotatedConic_sliceToroidalFormAtTheta (tPoint, &numPoint, pMatrix, theta);

    for (i = 0; i < numPoint; i++)
        {
        if (tp_inRanges (&tPoint[i], pRange0, pRange1))
            {
            tPoint[i].mask = mask;
            tpa_addTPoint (pArray, &tPoint[i]);
            }
        }
    return numPoint;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Interval is to be subdivided into steps numbered 0..lastIndex.
| If interval is tiny, lastIndex=0.                                     |
+----------------------------------------------------------------------*/
static void    gridSteps

(
double *pStep,
int    *pLastIndex,
double x0,
double x1,
double baseSize         /* Assumed nonzero */
)
    {
    static double s_singlePointRelTol = 1.0e-8;
    double delta = x1 - x0;

    if (baseSize < 0.0)
        baseSize = fabs (baseSize);

    if (fabs (delta) < s_singlePointRelTol)
        {
        *pLastIndex = 0;
        *pStep = baseSize;
        }
    else if (fabs (delta) <= baseSize)
        {
        *pLastIndex = 1;
        *pStep = delta;
        }
    else
        {
        *pLastIndex = 1 + (int) fabs(delta / baseSize);
        *pStep = delta / *pLastIndex;
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static double evalGridStep

(
double x0,
double x1,
int     index,
int    lastIndex,
double step
)
    {
    double x;
    if (index == 0)
        {
        x = x0;
        }
    else if (index == lastIndex)
        {
        x = x1;
        }
    else
        {
        x = x0 + index * step;
        }
    return x;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find zeros of a toroidal form within a band in which:
*  1) theta varies freely from theta0 to theta1
*  2) phi is single valued within the band limits phiA to phiB
*  3) phi values from phi0 to phi1 are kept
+----------------------------------------------------------------------*/
static StatusInt jmdlRotatedConic_toroidalBlock

(
RotMatrixCP pMatrix,
double      theta0,     /* Overall theta limit */
double      theta1,     /* Overall theta limit */
double      phi0,       /* Overall phi limit */
double      phi1,       /* Overall phi limit */
double      phiA,       /* Band phi limit */
double      phiB,       /* Band phi limit. ASSUMED phiB > phiA */
double      maxDTheta,
double      maxDPhi,
ToroidalOutputFunction outputFunc,
void        *pOutputData
)
    {

    StatusInt status = SUCCESS;
    ToroidalPointArray tPoints;

    ParamRange  patchRange;
    ParamRange  bandRange;
    //double      thetaSweep = theta1 - theta0;
    //double      phiSweep   = phi1   - phi0;
    double      theta, phi;
    double      step;

    int i, numInterval;


    if (maxDTheta <= MINIMUM_ANGLE_STEP)
        maxDTheta = MINIMUM_ANGLE_STEP;
    if (maxDPhi <= MINIMUM_ANGLE_STEP)
        maxDPhi = MINIMUM_ANGLE_STEP;

    jmdlParamRange_initFromAngleLimits (&patchRange, theta0, theta1, phi0, phi1);
    jmdlParamRange_initFromAngleLimits (&bandRange, theta0, theta1, phiA, phiB);

    tpa_clear (&tPoints);
    tpa_setOutputFunction (&tPoints, outputFunc, pOutputData);

    gridSteps (&step, &numInterval, theta0, theta1, maxDTheta);
    for (i = 0; i <= numInterval; i++)
        {
        theta = theta0 + i * step;
        if (i == numInterval)
            theta = theta1;             /* prevent roundoff problems */
        tpa_addSliceAtTheta (&tPoints, pMatrix, theta, &patchRange, &bandRange, 0);
        }

    gridSteps (&step, &numInterval, phiA, phiB, maxDPhi);
    for (i = 0; i <= numInterval; i++)
        {
        phi = phiA + i * step;
        if (i == numInterval)
            phi = phiB;         /* prevent roundoff problems */
        tpa_addSliceAtPhi (&tPoints, pMatrix, phi, &patchRange, &bandRange, 0);
        }

    tpa_normalizeAndSortTheta
                    (
                    &tPoints,
                    &patchRange,
                    s_toroidalFilterFraction * maxDTheta,
                    s_toroidalFilterFraction * maxDPhi
                    );

    status = tpa_output (&tPoints);
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find zeros of a toroidal form within a band in which:
*  1) theta varies freely from theta0 to theta1
*  2) phi is single valued within the band limits phiA to phiB
*  3) phi values from phi0 to phi1 are kept
+----------------------------------------------------------------------*/
static StatusInt jmdlRotatedConic_toroidalBand

(
RotMatrixCP pMatrix,
double      theta0,     /* Overall theta limit */
double      theta1,     /* Overall theta limit */
double      phi0,       /* Overall phi limit */
double      phi1,       /* Overall phi limit */
double      phiA,       /* Band phi limit */
double      phiB,       /* Band phi limit. ASSUMED phiB > phiA */
double      maxDTheta,
double      maxDPhi,
ToroidalOutputFunction outputFunc,
void        *pOutputData
)
    {

    StatusInt status = SUCCESS;
    ToroidalPoint breakPointM[2];
    ToroidalPoint breakPointA[2];
    ToroidalPoint breakPointB[2];
    double thetaBreak[6];
    int numBreakA, numBreakB, numBreak, numBreakM;
    double      thetaSweep = theta1 - theta0;
    //double      phiSweep   = phi1   - phi0;
    double      phiAB = phiB - phiA;
    double      theta, phiM0, phiM1;
    double      thetaC, thetaD, thetaM;
    double      smallAngle = bsiTrig_smallAngle ();

    int i;

    /* We know ...
    On a full torus with 'banded' contours: in each band the contours wander around theta with phi varying
    in the general shape of a sine curve --- one crossing of any constant theta line, two if any constant
    phi line.

    When dealing with a partial band, the partial bounds phiA and phiB can cut the contour in 4 points.
    Draw vertical (constant theta) lines at each of these points.
    Also find the crossings of
    */

    if (   SUCCESS == jmdlRotatedConic_sliceToroidalFormAtPhi (breakPointM, &numBreakM, pMatrix, phiA + 0.5 * phiAB)
        && numBreakM == 2
        && SUCCESS == jmdlRotatedConic_sliceToroidalFormAtPhi (breakPointA, &numBreakA, pMatrix, phiA)
        && SUCCESS == jmdlRotatedConic_sliceToroidalFormAtPhi (breakPointB, &numBreakB, pMatrix, phiB)
        )
        {
        /* The middle phi value has crossings. */
        numBreak = 0;
        thetaBreak[numBreak++] = theta0;
        thetaBreak[numBreak++] = theta1;
        for (i = 0; i < numBreakA; i++)
            {
            theta = breakPointA[i].thetaPoint.z;
            if (Angle::InSweepAllowPeriodShift (theta, theta0, thetaSweep))
                thetaBreak[numBreak++] = bsiTrig_adjustAngleToSweep (theta, theta0, thetaSweep);
            }

        for (i = 0; i < numBreakB; i++)
            {
            theta = breakPointB[i].thetaPoint.z;
            if (Angle::InSweepAllowPeriodShift (theta, theta0, thetaSweep))
                thetaBreak[numBreak++] = bsiTrig_adjustAngleToSweep (theta, theta0, thetaSweep);
            }


        bsiDoubleArray_sort (thetaBreak, numBreak, true);
        for (i = 1; i < numBreak; i++)
            {
            thetaC = thetaBreak[i-1];
            thetaD = thetaBreak[i];
            thetaM = 0.5 * (thetaC + thetaD);
            if (   fabs (thetaC - thetaD) > smallAngle
                && SUCCESS == jmdlRotatedConic_sliceToroidalFormAtTheta
                            (breakPointM, &numBreakM, pMatrix, thetaM)
                && numBreakM == 2
                )
                {
                phiM0 = breakPointM[0].phiPoint.z;
                phiM1 = breakPointM[1].phiPoint.z;
                if  (  Angle::InSweepAllowPeriodShift (phiM0, phiA, phiAB)
                    || Angle::InSweepAllowPeriodShift (phiM1, phiA, phiAB)
                    )
                    jmdlRotatedConic_toroidalBlock (
                            pMatrix,
                            thetaC,
                            thetaD,
                            phi0,
                            phi1,
                            phiA,
                            phiB,
                            maxDTheta,
                            maxDPhi,
                            outputFunc,
                            pOutputData);
                }
            }
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_processThetaBands

(
const RotatedConic    *pSurface,
RotMatrixCP pMatrix,       /* => toroidal form coefficients */
double                theta0,
double                theta1,
double                phi0,
double                phi1,
DPoint3dP pPhiLimit,     /* => phi limits. Must be dimensioned at least 3 larger to accomodate
surface and wraparound limits */
int                     numPhiLimit,    /* => number of limits */
ToroidalOutputFunction  outputFunc,      /* => output handler */
void                    *pOutputParams
)
    {
    static double bandTolerance = 0.05;
    double phiMid, phiA, phiB;
    int band;
    StatusInt bandStatus;

    double phiSweep, thetaSweep;

    thetaSweep  = theta1 - theta0;

    phiSweep    = phi1 - phi0;

    /* Two complete theta loops, each with restricted phi range */
    if (!Angle::IsFullCircle (phiSweep))
        {
        /* Only the internal band breaks matter */
        int k = 0;
        for (band = 0; band < numPhiLimit; band++)
            {
            double phiBand = bsiTrig_adjustAngleToSweep (pPhiLimit[band].z, phi0, phiSweep);
            if (Angle::InSweepAllowPeriodShift (phiBand, phi0, phiSweep))
                pPhiLimit[k++].z = bsiTrig_adjustAngleToSweep (pPhiLimit[band].z, phi0, phiSweep);
            }

        pPhiLimit[k++].z = phi0;
        pPhiLimit[k++].z = phi1;
        numPhiLimit = k;
        qsort (pPhiLimit, numPhiLimit, sizeof (DPoint3d),
                (int (*)(const void *, const void *))compareZ);
        }
    else
        {
        qsort (pPhiLimit, numPhiLimit, sizeof (DPoint3d),
                (int (*)(const void *, const void *))compareZ);
        pPhiLimit[numPhiLimit++].z = pPhiLimit[0].z + msGeomConst_2pi;
        }

    bandStatus = SUCCESS;
    for (band = 1; SUCCESS == bandStatus && band < numPhiLimit; band++)
        {
        phiA = pPhiLimit[band-1].z;
        phiB = pPhiLimit[band].z;
        phiMid = 0.5 * (phiA + phiB);
        if (Angle::InSweepAllowPeriodShift (phiMid, phi0, phiSweep))
            bandStatus = jmdlRotatedConic_toroidalBand
                            (
                            pMatrix,
                            theta0,
                            theta1,
                            phi0,
                            phi1,
                            phiA,
                            phiB,
                            bandTolerance, bandTolerance,
                            outputFunc,
                            pOutputParams
                            );
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static void fillCriticalAngles

(
double      *pAngle,
int         *pNumAngle,
DPoint3dP pPoint,           /* => candidate angles as cos,sin,theta */
int         numPoint,
double      angle0,         /* => patch limit */
double      angle1          /* => patch limit */
)
    {
    double sweep = angle1 - angle0;
    int i, numAngle;

    numAngle = 0;
    if (numPoint == 0)
        {
        /* Just fill in the patch limits */
        pAngle[numAngle++] = angle0;
        pAngle[numAngle++] = angle1;

        }
    else if (Angle::IsFullCircle (sweep))
        {
        /* Sort all limit angles, and add a wraparound */

        for (i = 0; i < numPoint; i++)
            pAngle[numAngle++] = pPoint[i].z;

        bsiDoubleArray_sort (pAngle, numAngle, true);
        pAngle[numAngle++] = pAngle[0] + msGeomConst_2pi;
        }
    else
        {
        /* Sort limit angles within patch limits */
        pAngle[numAngle++] = angle0;
        pAngle[numAngle++] = angle1;

        for (i = 0; i < numPoint; i++)
            {
            if (Angle::InSweepAllowPeriodShift (pPoint[i].z, angle0, sweep))
                pAngle[numAngle++] = pPoint[i].z;
            }

        bsiDoubleArray_sort (pAngle, numAngle, true);
        }
    *pNumAngle = numAngle;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static void tpa_addGrid

(
ToroidalPointArray  *pPointArray,
RotMatrixCP pMatrix,
double              startAngle,
double              endAngle,
int (*gridFunc)(ToroidalPointArray *, const RotMatrix *, double, ParamRange *, ParamRange *, int),
double              maxStepSize,
int                 mask
)
    {
    int i, numInterval;
    double step, angle;

    gridSteps (&step, &numInterval, startAngle, endAngle, maxStepSize);
    for (i = 0; i <= numInterval; i++)
        {
        angle = evalGridStep (startAngle, endAngle, i, numInterval, step);
        gridFunc (pPointArray, pMatrix, angle, NULL, NULL, mask);
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static int tpa_locateThetaBand /* Index of band containing ALL points */

(
ToroidalPointArray  *pPointArray,
double              *pThetaLimit,       /* => Limits of theta bands */
int                 numLimit            /* => one more than number of bands */
)
    {
    int band, i;
    int numPoint = pPointArray->numPoint;
    bool    allInBand;
    double thetaStart, thetaSweep, theta;

    for (band = 1; band < numLimit; band++)
        {
        thetaStart = pThetaLimit[band-1];
        thetaSweep = pThetaLimit[band] - thetaStart;
        allInBand = true;
        for (i = 0; i < numPoint && allInBand; i++)
            {
            theta = pPointArray->pointPair[i].thetaPoint.z;
            if (!Angle::InSweepAllowPeriodShift (theta, thetaStart, thetaSweep))
                {
                allInBand = false;
                }
            }
        if (allInBand)
            return band - 1;
        }
    return -1;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_processBubbles

(
const RotatedConic    *pSurface,
RotMatrixCP pMatrix,       /* => toroidal form coefficients */
double                theta0,
double                theta1,
double                phi0,
double                phi1,
DPoint3dP pThetaLimit,     /* => phi limits. Must be dimensioned at least 3 larger to accomodate
surface and wraparound limits */
int                   numThetaLimit,    /* => number of limits */
DPoint3dP pPhiLimit,       /* => phi limits. Must be dimensioned at least 3 larger to accomodate
surface and wraparound limits */
int                     numPhiLimit,    /* => number of limits */
ToroidalOutputFunction  outputFunc,     /* => output handler */
void                    *pOutputData
)
    {
    static double bandTolerance = 0.10;
    double phiCrit  [10];
    double thetaCrit[10];
    int numPhiCrit, numThetaCrit, j;
    double phiMid, thetaMid;
    ToroidalPointArray tPoints;
    ToroidalPoint breakPoint[4];
    int numBreak, thetaBand;
    double phiStart, phiEnd, thetaStart, thetaEnd;
    double smallAngle = bsiTrig_smallAngle ();

    double phiSweep, thetaSweep;
    double thetaBandSweep;
    double phiBandSweep;

    double maxDTheta = bandTolerance;
    double maxDPhi   = bandTolerance;
    thetaSweep  = theta1 - theta0;

    phiSweep    = phi1 - phi0;

    /* Get the raw bubble limits */
    fillCriticalAngles (thetaCrit, &numThetaCrit, pThetaLimit, numThetaLimit, 0.0, msGeomConst_2pi);
    fillCriticalAngles (phiCrit, &numPhiCrit, pPhiLimit, numPhiLimit, 0.0, msGeomConst_2pi);

    for (j = 1; j < numPhiCrit; j++)
        {
        phiStart = phiCrit[j-1];
        phiEnd   = phiCrit[j];
        phiBandSweep = phiEnd - phiStart;
        phiMid = 0.5 * (phiStart + phiEnd);
        jmdlRotatedConic_sliceToroidalFormAtPhi (breakPoint, &numBreak, pMatrix, phiMid);
        if (numBreak > 0 && fabs (phiStart - phiEnd) > smallAngle)
            {
            tpa_clear (&tPoints);
            tpa_setOutputFunction (&tPoints, outputFunc, pOutputData);

            tpa_addGrid (&tPoints, pMatrix, phiStart, phiEnd, tpa_addSliceAtPhi, maxDPhi, 0);

            if (0 <= (thetaBand = tpa_locateThetaBand (&tPoints, thetaCrit, numThetaCrit)))
                {
                thetaStart = thetaCrit[thetaBand];
                thetaEnd   = thetaCrit[thetaBand+1];
                thetaBandSweep = thetaEnd - thetaStart;
                thetaMid = 0.5 * (thetaStart + thetaEnd);

                tpa_addGrid (&tPoints, pMatrix,
                                    thetaStart, thetaEnd,
                                    tpa_addSliceAtTheta,
                                    maxDPhi,
                                    0);

                if (Angle::InSweepAllowPeriodShift (theta0, thetaStart, thetaBandSweep))
                    tpa_addSliceAtTheta (&tPoints, pMatrix, theta0, NULL, NULL, TP_BOUNDARY_POINT);
                if (Angle::InSweepAllowPeriodShift (theta1, thetaStart, thetaBandSweep))
                    tpa_addSliceAtTheta (&tPoints, pMatrix, theta1, NULL, NULL, TP_BOUNDARY_POINT);

                if (Angle::InSweepAllowPeriodShift (phi0, phiStart, phiBandSweep))
                    tpa_addSliceAtPhi (&tPoints, pMatrix, phi0, NULL, NULL, TP_BOUNDARY_POINT);
                if (Angle::InSweepAllowPeriodShift (phi1, phiStart, phiBandSweep))
                    tpa_addSliceAtPhi (&tPoints, pMatrix, phi1, NULL, NULL, TP_BOUNDARY_POINT);

                tpa_normalizeToAngleLimits (&tPoints, thetaStart, thetaEnd, phiStart, phiEnd);
                tpa_centralSort (&tPoints, thetaMid, phiMid);

                tpa_filter  (
                            &tPoints,
                            s_toroidalFilterFraction * maxDTheta,
                            s_toroidalFilterFraction * maxDPhi
                            );
                tpa_close (&tPoints);
                tpa_output (&tPoints);
                }
            }
        }



    return SUCCESS;
    }

#define MAX_SILHOUETTE_BUFFER 200
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Trace solution contours of the commonly occurring equation
*    (c s 1)[ acC acS ac1 ] (C) = 0
*           [ asC asS as1 ] (S)
*           [ a1C a1S a11 ] (1)
*  for c=cos(phi), s=sin(phi), C=cos(theta), S=sin(theta)
*  within the limits of the surface.
+----------------------------------------------------------------------*/
Public StatusInt    bsiRotatedConic_toroidalForm

(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
RotMatrixCP pMatrix,       /* => coefficient matrix */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
)
    {
    StatusInt status = ERROR;
    double a,b;
    double theta0, theta1, thetaSweep, phi0, phi1, phiSweep;

    DPoint3d thetaLimit[8], phiLimit[8];
    int numThetaLimit, numPhiLimit;
    ToroidalOutputParams outputParams;

    outputParams.pSurface   = pSurface;
    outputParams.handlerFunc = handlerFunc;
    outputParams.pUserData  = pUserData;

    if (!pSurface || RC_Torus != pSurface->type)
        return ERROR;

    /* Torus sizes, correspond to analysis on paper */
    a = 1.0;
    b = pSurface->hoopRadius;

    theta0 = pSurface->parameterRange.low.x;
    theta1 = pSurface->parameterRange.high.x;
    thetaSweep = theta1 - theta0;

    phi0 = pSurface->parameterRange.low.y;
    phi1 = pSurface->parameterRange.high.y;
    phiSweep = phi1 - phi0;


    bsiRotatedConic_toroidalFormLimits (thetaLimit, &numThetaLimit, pMatrix, false);
    bsiRotatedConic_toroidalFormLimits (phiLimit, &numPhiLimit, pMatrix, true);

    if (numThetaLimit == 0 && numPhiLimit == 4)
        {
        jmdlRotatedConic_processThetaBands (pSurface, pMatrix,
                                        theta0, theta1, phi0, phi1,
                                        phiLimit, numPhiLimit,
                                        (ToroidalOutputFunction)outputThetaPhi, &outputParams);
        }
    else if (numPhiLimit == 0 && numThetaLimit == 4)
        {
        RotMatrix transpose;
        transpose.TransposeOf (*pMatrix);
        /* Use the logic for theta bands, but with transposed matrix and all phi-theta references
            exchanged, and an output function that undoes the exchange. */
        jmdlRotatedConic_processThetaBands (pSurface, &transpose,
                                        phi0, phi1, theta0, theta1,
                                        thetaLimit, numThetaLimit,
                                        (ToroidalOutputFunction)outputPhiTheta, &outputParams);
        }
    else
        {
        jmdlRotatedConic_processBubbles (pSurface, pMatrix,
                                                theta0, theta1,
                                                phi0,   phi1,
                                                thetaLimit, numThetaLimit,
                                                phiLimit, numPhiLimit,
                                                (ToroidalOutputFunction)outputThetaPhi, &outputParams);
        }

    return status;
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute general silhouette curve of a torus.
+----------------------------------------------------------------------*/
Public StatusInt    bsiRotatedConic_torusGeneralSilhouette

(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pEyePoint,     /* => eyepoint (nonzero weight) or view vector (zero weight) */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
)
    {
    DPoint4d localEyePoint;
    double a,b;
    RotMatrix coffMatrix;

    if (!pSurface || RC_Torus != pSurface->type)
        return ERROR;
    bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, &localEyePoint, pEyePoint, 1);

    /* Torus sizes, correspond to analysis on paper */
    a = 1.0;
    b = pSurface->hoopRadius;

    coffMatrix.SetColumn (DVec3d::From (-localEyePoint.x, 0.0, 0.0), 0);
    coffMatrix.SetColumn (DVec3d::From (-localEyePoint.y, 0.0, 0.0), 1);
    coffMatrix.SetColumn (DVec3d::From (a * localEyePoint.w, -localEyePoint.z, b * localEyePoint.w), 2);

    return bsiRotatedConic_toroidalForm (pSurface, &coffMatrix,
                                handlerFunc, tolerance, pUserData);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Initialize with specified surface type and parameter range.
+----------------------------------------------------------------------*/
Public StatusInt bsiRotatedConic_initFrameAndSweep

(
RotatedConic        *pSurface,      /* <= intitialized surface */
RotatedConicType    type,           /* => Surface type. */
DMap4dCP pFrame,    /* => Placement frame.  If NULL, identity frame is used */
double      theta0,
double      dTheta,
double      alpha0,
double      dAlpha
)
    {
    memset (pSurface, 0, sizeof (RotatedConic));
    pSurface->type = type;;
    if (pFrame)
        {
        pSurface->rotationMap = *pFrame;
        }
    else
        {
        bsiDMap4d_initIdentity (&pSurface->rotationMap);
        }

    pSurface->parameterRange.low.x  = theta0;
    pSurface->parameterRange.high.x = theta0 + dTheta;
    pSurface->parameterRange.low.y  = alpha0;
    pSurface->parameterRange.high.y = alpha0 + dAlpha;
    pSurface->parameterRange.low.z = pSurface->parameterRange.high.z = 1.0;

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* Convert to rotated conic form.
* @indexVerb rotatedConic
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool               bsiDToroid3d_getRotatedConic
(
DToroid3dCP pInstance,
RotatedConic      *pConic
)
    {
    DMap4d hFrame;

    double theta0, dTheta, phi0, dPhi;
    bool    boolStat;

    /* Build a rotated conic. */
    boolStat = bsiDMap4d_initFromTransform (&hFrame, &pInstance->frame, false);

    if (boolStat)
        {
        bsiDToroid3d_getScalarNaturalParameterSweep (pInstance, &theta0, &dTheta, &phi0, &dPhi);
        bsiRotatedConic_initFrameAndSweep
                        (
                        pConic, RC_Torus, &hFrame,
                        theta0, dTheta,
                        phi0, dPhi
                        );
        pConic->hoopRadius = pInstance->minorAxisRatio;
        }
    return boolStat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE


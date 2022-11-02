/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| The cannonical form of an ellipse in this file is                     |
|     center  = center of the ellipse                                   |
|     vectorU = vector from center to '0 degree' point                  |
|     vectorV = vector from center to '90 degree' point                 |
|     theta0  = start angle (radians)                                   |
|     dtheta  = sweep angle (radians)                                   |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/internal2/GeomPrivateApi.h>

#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value

static double s_lineUnitCircleIntersectionTolerance = 1.0e-8;


/* We don't like it when hyperbolas go to infinity.  In fact, we don't even like
   getting close.   This is the default angular space tolerance zone that gets
   cut out around the singularities.    No attempt to get near machine precision here ..
   just stay away from it.
*/
static double s_defaultSingularCutAngle = 1.0e-3;

/*----------------------------------------------------------------------+
|                                                                       |
|Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/

/*======================================================================+
|                                                                       |
|   MajorPublic Code Section                                           |
|                                                                       |
+======================================================================*/


/*-----------------------------------------------------------------*//**
* @class DEllipse4d
* The DEllipse4d structure can represent an ellipse, parabola, or
* hyperbola.  We use the term "ellipse" to loosely cover all three
* of these because they are all just transformations of a unit circle.
*
* If C, U, and V are homogeneous points, the equation
*<pre>
*               X = C + cos(theta)*U + sin(theta)*V
*</pre>
* sweeps X along an 'elliptical' path in 4D space.  The term
* 'elliptical' is put in quotes because when X is normalized back to
* 3D the points may be an ellipse, hyperbola or parabola, depending on
* the weight
* part of the 4D point.
*
* If C has weight one and U,v have weight zero, the curve is
* an ellipse in the conventional sense.
*
* If these conventional ellipse points are altered by a 4x4 transform
* T, the transformed points
*<pre>
*               TX = TC + cos(theta)*TU + sin(theta)*TV
*</pre>
* is the equation of the transformed ellipse.  Hence one can transform
* the ellipse prior to stroking.
*
* An DEllipse4d structure stores the C, U, V vectors and a set of
* arc ranges.
* @indexVerb
* @bsimethod
+===============+===============+===============+===============+======*/



/*-----------------------------------------------------------------*//**
* Fill a homogenous ellipse from a cartesian ellipse.
* @param ellipseP <=> header to receive points
* @param centerP => ellipse center
* @param point0P => 0 degree vector
* @param point90P => 90 degree vector
* @param theta0 => start angle
* @param delta => sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromDEllipse3d

(
DEllipse4dP pInstance,
DEllipse3dCP pEllipse
)
    {
    bsiDEllipse4d_initFrom3dVectors
            (
            pInstance,
            &pEllipse->center,
            &pEllipse->vector0,
            &pEllipse->vector90,
            pEllipse->start,
            pEllipse->sweep
            );
    }


/*-----------------------------------------------------------------*//**
* Fill a homogenous ellipse from a 3D ellipse, using the input Z as weight and setting
*       output z to a specified constant.
* @param pEllipse => ellipse with x,y,w as x,y,z
* @param z => z to use in homogeneous form.
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromDEllipse3dXYW

(
DEllipse4dP pInstance,
DEllipse3dCP pEllipse,
double      z
)
    {

    bsiDEllipse4d_initFrom3dVectors
            (
            pInstance,
            &pEllipse->center,
            &pEllipse->vector0,
            &pEllipse->vector90,
            pEllipse->start,
            pEllipse->sweep
            );

    pInstance->center.w     = pInstance->center.z;
    pInstance->vector0.w    = pInstance->vector0.z;
    pInstance->vector90.w   = pInstance->vector90.z;

    pInstance->center.z     = z;
    pInstance->vector0.z    = z;
    pInstance->vector90.z   = z;

    }



/*-----------------------------------------------------------------*//**
* Fill in ellipse data from 3D coordinates
* @param ellipseP <=> header to receive points
* @param centerP => ellipse center
* @param point0P => 0 degree vector
* @param point90P => 90 degree vector
* @param theta0 => start angle
* @param delta => sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromDPoint4d

(
DEllipse4dP ellipseP,
DPoint4dCP centerP,
DPoint4dCP point0P,
DPoint4dCP point90P,
double          theta0,
double          delta
)

    {
    ellipseP->center = *centerP;

    bsiDPoint4d_addScaledDPoint4d (&ellipseP->vector0,  point0P,  centerP, -1.0);
    bsiDPoint4d_addScaledDPoint4d (&ellipseP->vector90, point90P, centerP, -1.0);

    bsiRange1d_clear ( &ellipseP->sectors );
    bsiRange1d_addArcSweep( &ellipseP->sectors, theta0, delta );
    }




/*-----------------------------------------------------------------*//**
* Fill in ellipse data from 3D coordinates
*
* @param ellipseP <=> header to receive points
* @param centerP => ellipse center
* @param vectorUP => 0 degree vector
* @param vectorVP => 90 degree vector
* @param theta0 => start angle
* @param delta => sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFrom3dVectors

(
DEllipse4dP ellipseP,
DPoint3dCP centerP,
DPoint3dCP vectorUP,
DPoint3dCP vectorVP,
double          theta0,
double          delta
)
    {
    bsiDPoint4d_copyAndWeight( &ellipseP->center,   centerP, 1.0);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector0,  vectorUP, 0.0);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector90, vectorVP, 0.0);
    bsiRange1d_clear ( &ellipseP->sectors );
    bsiRange1d_addArcSweep( &ellipseP->sectors, theta0, delta );
    }



/*-----------------------------------------------------------------*//**
* Fill in ellipse data from 4d coordinates of center, vector0, vector90
*
* @param pEllipse <=> header to receive points
* @param pCenter => ellipse center
* @param pVector0 => 0 degree vector
* @param pVector90 => 90 degree vector
* @param theta0 => start angle
* @param delta => sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFrom4dVectors

(
DEllipse4dP pEllipse,
DPoint4dCP pCenter,
DPoint4dCP pVector0,
DPoint4dCP pVector90,
double          theta0,
double          delta
)

    {
    pEllipse->center   = *pCenter;
    pEllipse->vector0  = *pVector0;
    pEllipse->vector90  = *pVector90;
    bsiRange1d_clear ( &pEllipse->sectors );
    bsiRange1d_addArcSweep( &pEllipse->sectors, theta0, delta );
    }


/*-----------------------------------------------------------------*//**
* Fill in ellipse data so the form of the ellipse is
*      P(theta) = P0 + cos(theta)*U + sin(theta)*V
* where P, P0, U, V are homogeneous points, with
*       P0 = (point[0], weight[0])  directly from given data.
*       U   = (point[1]point[0], weight[1])
*       V   = (point[2]point[1], weight[2])
*
* @param ellipseP <=> header to receive points
* @param pPointArray * @param pWeightArray * @param theta0 => start angle
* @param delta => sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromWeighted3dPoints

(
DEllipse4dP ellipseP,
DPoint3dCP pPointArray,
const double        *pWeightArray,
double        theta0,
double        delta
)
    {
    DPoint3d vectorU, vectorV;
    vectorU.DifferenceOf (pPointArray[1], pPointArray[0]);
    vectorV.DifferenceOf (pPointArray[2], pPointArray[0]);

    bsiDPoint4d_copyAndWeight( &ellipseP->center,   &pPointArray[0], pWeightArray[0]);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector0,  &vectorU,        pWeightArray[1] - pWeightArray[0]);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector90, &vectorV,        pWeightArray[2] - pWeightArray[0]);
    bsiRange1d_clear ( &ellipseP->sectors );
    bsiRange1d_addArcSweep( &ellipseP->sectors, theta0, delta );
    }



/**
           bsiDEllipse4d_initFrom3dPoints
* Fill in ellipse data from 3D coordinates


* @param ellipseP <=> header to receive points
* @param arrayP => ellipse center
* @param theta0 => start angle
* @param delta => sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFrom3dPoints

(
DEllipse4dP ellipseP,
DPoint3dCP arrayP,
double          theta0,
double          delta
)

    {
    DPoint3d vector0, vector90;
    vector0.DifferenceOf (arrayP[1], arrayP[0]);
    vector90.DifferenceOf (arrayP[2], arrayP[0]);
    bsiDPoint4d_copyAndWeight( &ellipseP->center, &arrayP[0], 1.0);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector0, &vector0, 0.0);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector90, &vector90, 0.0);
    if (delta > msGeomConst_2pi)
        delta = msGeomConst_2pi;
    bsiRange1d_setUncheckedArcSweep( &ellipseP->sectors, theta0, delta );
    }



/*-----------------------------------------------------------------*//**
* @param ellipseP <= header to receive points
* @param centerP => circle center
* @param normalP => plane normal
* @param radius => circle radius
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromCenterNormalRadius

(
DEllipse4dP ellipseP,
DPoint3dCP centerP,
DPoint3dCP normalP,
double          radius
)

    {
    DPoint3d uVector, vVector, wVector;

    normalP->GetNormalizedTriad (uVector, vVector, wVector);
    uVector.Scale (uVector, radius);
    vVector.Scale (vVector, radius);

    bsiDPoint4d_copyAndWeight( &ellipseP->center, centerP, 1.0);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector0, &uVector, 0.0);
    bsiDPoint4d_copyAndWeight( &ellipseP->vector90, &vVector, 0.0);

    bsiRange1d_clear ( &ellipseP->sectors );
    bsiRange1d_addArcSweep( &ellipseP->sectors, 0.0, msGeomConst_2pi);
    }






/*-----------------------------------------------------------------*//**
* @param pEllipse => ellipse to test
* @see
* @return true if the ellipse angular range is a full circle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_isFullEllipse

(
DEllipse4dCP pEllipse
)
    {
    return bsiRange1d_isFullCircle (&pEllipse->sectors);
    }



/*-----------------------------------------------------------------*//**
*  Evaluate the homogenous point on a homogeneous ellipse at given
* angle.
* @instance pHEllipse => ellipse to evaluate
* @param pPoint <= evaluted point
* @param theta => angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_evaluateDPoint4d

(
DEllipse4dCP pHEllipse,
DPoint4dP pPoint,
double      theta
)
    {
    double cosTheta, sinTheta;
    cosTheta = cos(theta);
    sinTheta = sin(theta);
    pPoint->SumOf(pHEllipse->center, pHEllipse->vector0, cosTheta, pHEllipse->vector90, sinTheta);
    }


/*-----------------------------------------------------------------*//**
*  Evaluate the homogeneous endpoints of the ellipse.
* @instance pInstance => ellipse to evaluate
* @param pStart <= start point
* @param pEnd   <= end point
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_evaluateDPoint4dEndPoints

(
DEllipse4dCP pInstance,
DPoint4dP pStartPoint,
DPoint4dP pEndPoint
)
    {
    double theta0, theta1;
    int n = pInstance->sectors.n;

    if (pInstance->sectors.n <= 0)
        {
        theta0 = theta1 = 0.0;
        }
    else
        {
        theta0 = pInstance->sectors.interval[0].minValue;
        theta1 = pInstance->sectors.interval[n].maxValue;

        bsiDEllipse4d_evaluateDPoint4d (pInstance, pStartPoint, theta0);
        bsiDEllipse4d_evaluateDPoint4d (pInstance, pEndPoint,   theta1);
        }
    }


/*-----------------------------------------------------------------*//**
* Compute an angle associated with a 4d point.   If the point is really on the ellipse, this
*   angle is the parameter of the point.  If the point is not on the ellipse, the angle minimizes
*   the 4D vector from ellipse points to the given point.
* @instance pEllipse => ellipse to evaluate
* @param    pPoint => point where angle is computed.
* @return computed angle.  0 is returned if the ellipse is degenerate or the point is at the
*           ellipse center.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_angleFromDPoint4d

(
DEllipse4dCP pEllipse,
DPoint4dCP pPoint
)
    {
    double UdotU, UdotV, VdotV;
    DPoint4d W;
    double UdotW, VdotW;
    double cosTheta, sinTheta, theta;
    bsiDPoint4d_addScaledDPoint4d (&W, pPoint, &pEllipse->center, -1.0);
    UdotU = pEllipse->vector0.DotProduct (pEllipse->vector0);
    UdotV = pEllipse->vector0.DotProduct (pEllipse->vector90);
    VdotV = pEllipse->vector90.DotProduct (pEllipse->vector90);
    UdotW = pEllipse->vector0.DotProduct (W);
    VdotW = pEllipse->vector90.DotProduct (W);

    if (bsiSVD_solve2x2 (&cosTheta, &sinTheta,
                        UdotU, UdotV,
                        UdotV, VdotV,
                        UdotW, VdotW))
        {
        theta = Angle::Atan2 (sinTheta, cosTheta);
        }
    else
        {
        theta = 0.0;
        }
    return theta;
    }


/*-----------------------------------------------------------------*//**
*  Evaluate the homogenous point on a homogeneous ellipse at given
* angle and reduce the point back to 3d.
* @instance pHEllipse => ellipse to evaluate
* @param    pPoint <= point on ellipse
* @param    theta => angle of evaluation.
* @see
* @return true if weight is nonzero and point was normalized.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_evaluateDPoint3d

(
DEllipse4dCP pHEllipse,
DPoint3dP pPoint,
double      theta
)
    {
    DPoint4d hPoint;
    static double minW = 1.0e-14;
    bool    boolStat;
    bsiDEllipse4d_evaluateDPoint4d (pHEllipse, &hPoint, theta);
    if (fabs (hPoint.w) <= minW)
        {
        pPoint->x = hPoint.x;
        pPoint->y = hPoint.y;
        pPoint->z = hPoint.z;
        boolStat = false;
        }
    else
        {
        pPoint->x = hPoint.x / hPoint.w;
        pPoint->y = hPoint.y / hPoint.w;
        pPoint->z = hPoint.z / hPoint.w;
        boolStat = true;
        }
    return boolStat;
    }



/*-----------------------------------------------------------------*//**
* Project pXYZ to the plane with center, 0, and 90 degree cartesian
* points.

* @instance pHEllipse => ellipse whose axes become 3d plane directions.
* @param pXYZNear <= nearest point
* @param pCoff0 <= coefficient on vector towards 0 degree point
* @param pCoff90 <= coefficient on vector towards 90 degree point
* @param pXYZ
* @see
* @return true if plane is well defined.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_projectPointTo3dPlane

(
DEllipse4dCP pHEllipse,
DPoint3dP pXYZNear,
double      *pCoff0,
double      *pCoff90,
DPoint3dCP pXYZ
)
    {
    DPoint3d point3d[3];
    DPoint4d point4d;

    pHEllipse->center.GetProjectedXYZ (*point3d);

    point4d.SumOf (pHEllipse->center, pHEllipse->vector0);
    point4d.GetProjectedXYZ (point3d[ 1]);

    point4d.SumOf (pHEllipse->center, pHEllipse->vector90);
    point4d.GetProjectedXYZ (point3d[ 2]);

    return bsiGeom_closestPointOnSkewedPlane (pXYZNear, pCoff0, pCoff90, point3d, pXYZ);
    }



/**

* @instance ellipseP => ellipse to be stroked
* @param n => default number of points on full ellipse
* @param nMax => max number of points on full ellipse
* @param tol => tolerance for stroking
* @see
* @return number of strokes required to reach the stroke tolerance
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_getStrokeCount

(
DEllipse4dCP ellipseP,
int             n,
int             nMax,
double          tol
)

    {
    DPoint4d hPoint0, hPoint90;
    DPoint3d point0, point90, center;
    double dx, dy, r0, r90, rmax;
    int nTol;

    if (tol > 0.0)
        {
        hPoint0.SumOf (ellipseP->center, ellipseP->vector0);
        hPoint90.SumOf (ellipseP->center, ellipseP->vector90);
        if (   hPoint0.GetProjectedXYZ (point0)
            && hPoint90.GetProjectedXYZ (point90)
            && ellipseP->center.GetProjectedXYZ (center)
            )
            {
            dx = hPoint0.x - center.x;
            dy = hPoint0.y - center.y;
            r0 = dx*dx + dy*dy;

            dx = hPoint90.x - center.x;
            dy = hPoint90.y - center.y;
            r90 = dx*dx + dy*dy;

            if (r0 > r90)
                {
                rmax = sqrt(r0);
                }
            else
                {
                rmax = sqrt(r90);
                }

            if (rmax > tol)
                {
                /* Raw taylor series says the angle is sqrt ( rmax/ (8 tol))
                */
                nTol = (int) ( sqrt( rmax / tol ) * msGeomConst_2pi );
                if (nTol < 4)
                    {
                    nTol = 4;
                    }
                else if (nTol > nMax)
                    {
                    nTol = nMax;
                    }
                n = nTol;
                }
            else
                {
                n = 4;
                }
            }
        }
    return n;
    }



/*-----------------------------------------------------------------*//**
* Find the sector index containing angle theta.
* @instance pHEllipse => Header to search
* @param theta = angle whose sector is required
* @see
* @return sector index containing angle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_findSectorIndex

(
DEllipse4dCP pHEllipse,
double  theta
)

    {
    int i;
    if (pHEllipse)
        {
        for (i = 0; i < pHEllipse->sectors.n; i++)
            {
            if (Angle::InSweepAllowPeriodShift
                        (
                        theta,
                        pHEllipse->sectors.interval[i].minValue,
                        pHEllipse->sectors.interval[i].maxValue - pHEllipse->sectors.interval[i].minValue
                        ))
                {
                return i;
                }
            }
        }
    return -1;
    }



/*-----------------------------------------------------------------*//**
* Evaluate an ellipse at a number of (cosine, sine) pairs, removing
* pairs whose corresponding angle is not in range.

* @instance pEllipse => ellipse to evaluate
* @param pPoint <= cartesian points
* @param pTrig => c,s pairs for evaluation
* @param numPoint => number of pairs
* @see
* @return int number of cartesian points returned.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_evaluateTrigPairs

(
DEllipse4dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
)
    {
    DPoint4d hPoint;
    int i, n;
    n = 0;
    for (i = 0; i < numPoint; i++)
        {
        hPoint.SumOf(pEllipse->center, pEllipse->vector0, pTrig[i].x, pEllipse->vector90, pTrig[i].y);
        if (hPoint.w != 0.0)
            {
            pPoint[n].x = hPoint.x / hPoint.w;
            pPoint[n].y = hPoint.y / hPoint.w;
            pPoint[n].z = hPoint.z / hPoint.w;
            n++;
            }
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
* Evaluate an ellipse at a number of (cosine, sine) pairs, removing
* pairs whose corresponding angle is not in range.

* @instance pEllipse => ellipse to evaluate
* @param pPoint <= cartesian points
* @param pTrig => c,s pairs for evaluation
* @param numPoint => number of pairs
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_testAndEvaluateTrigPairs

(
DEllipse4dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
)
    {
    int n = 0;
    int i;

    for (i = 0; i < numPoint; i++)
        {
        double theta = Angle::Atan2 (pTrig[i].y, pTrig[i].x);
        if (bsiRange1d_pointIsIn (&pEllipse->sectors, theta))
            {
            bsiDEllipse4d_evaluateTrigPairs (pEllipse, &pPoint[n], &pTrig[i], 1);
            n += 1;
            }
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
* Evaluate an ellipse at a number of (cosine, sine) pairs, removing
* pairs whose corresponding angle is not in range.

* @instance pEllipse => ellipse to evaluate
* @param pPoint <= cartesian points
* @param pTrig => c,s pairs for evaluation
* @param numPoint => number of pairs
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_testAndEvaluateAngles

(
DEllipse4dCP pEllipse,
DPoint4dP pPointOut,
double      *pAngleOut,
double      *pAngleIn,
int         numPoint
)
    {
    int n = 0;
    int i;

    for (i = 0; i < numPoint; i++)
        {
        double theta = pAngleIn[i];
        if (bsiRange1d_pointIsIn (&pEllipse->sectors, theta))
            {
            if (pPointOut)
                bsiDEllipse4d_evaluateDPoint4d (pEllipse, &pPointOut[n], theta);
            if (pAngleOut)
                pAngleOut[n] = pAngleIn[i];
            n += 1;
            }
        }
    return n;
    }




/*-----------------------------------------------------------------*//**
* Clears the sector count for an ellipse.
* @param pEllipse <= Ellipse to be cleared
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clearSectors

(
DEllipse4dP pEllipse
)

    {
    pEllipse->sectors.n = 0;
    }


/*-----------------------------------------------------------------*//**
* Set the sweep as a complete circle.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_setFullCircleSweep

(
DEllipse4dP pEllipse
)

    {
    pEllipse->sectors.n = 1;
    pEllipse->sectors.interval[0].minValue = 0.0;
    pEllipse->sectors.interval[0].maxValue = msGeomConst_2pi;
    }


/*-----------------------------------------------------------------*//**
*
* Add an interval with no test for min/max relationship
*
* @param minValue => new interval min.
* @param maxValue => new interval max
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiDEllipse4d_addUnorderedSector

(
DEllipse4dP pInstance,
double          minValue,
double          maxValue
)
    {
    bsiRange1d_addUnordered (&pInstance->sectors, minValue, maxValue);
    }


/*-----------------------------------------------------------------*//**
* Get the i'th sector angular range
* @instance pEllipse => ellipse whose angular range is queried.
* @param pStartAngle <= start angle
* @param pEndAngle <= end angle
* @param i => sector to read
* @see
* @return true if sector index is valid.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_getSector

(
DEllipse4dCP pEllipse,
double    *pStartAngle,
double    *pEndAngle,
int             i
)
    {
    bool    boolStat = false;
    if (pEllipse && 0 <= i && i < pEllipse->sectors.n)
        {
        *pStartAngle = pEllipse->sectors.interval[i].minValue;
        *pEndAngle = pEllipse->sectors.interval[i].maxValue;
        boolStat = true;
        }
    else
        {
        *pStartAngle = *pEndAngle = 0.0;
        }
    return boolStat;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlDEllipse4d_sortAndFilterAngles                        |
| Sort angular values, adding end angles at +- pi                       |
| NORET                                                                 |
+----------------------------------------------------------------------*/
static int jmdlDEllipse4d_sortAndAugmentAngles

(
double *pAngle,    /* <= array of n angles, with space for 2 more. */
int    n           /* => number of values */
)
    {
    int numSorted, i;
    int iMin;
    double minAngle;
    pAngle[n++] = -msGeomConst_pi;
    pAngle[n++] = msGeomConst_pi;
    for (numSorted = 0; numSorted < n; numSorted++)
        {
        minAngle = pAngle[numSorted];
        iMin = numSorted;
        for (i = numSorted + 1; i < n; i++)
            {
            if (pAngle[i] < minAngle)
                {
                iMin = i;
                minAngle = pAngle[i];
                }
            }
        if (iMin > numSorted)
            {
            pAngle[iMin] = pAngle[numSorted];
            pAngle[numSorted] = minAngle;
            }
        }
    return n;
    }

/*-----------------------------------------------------------------*//**
* Clips the sector data of the hyperbola to a plane.


* @param pEllipse => Ellipse to evaluate
* @param pPlaneTrigPoint => x,y=cosine and sine where plane is intersected.
* @param numPlaneIntersection => number of plane intersections
* @param pPlaneVector => Homogeneous plane point
* @param pTrigPoint => x,y=cosine and sine of angles. z = angle
* @param numAsymptote => number of singular points on the ellipse
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clipHyperbolicSectors

(
DEllipse4dP pEllipse,
DPoint3dCP pPlaneTrigPoint,
int     numPlaneIntersection,
DPoint4dCP pPlaneVector,
DPoint3dCP pTrigPoint,
int     numAsymptote
)
    {
    double angles[6];
    int i;
    double theta0, theta1, thetaMid, c, s, f;
    DPoint4d curvePoint;
    SmallSetRange1d clipSectors;
    SmallSetRange1d priorSectors;


    int numAngle = 0;
    int num0 = 0;
    int num1 = 0;

    bsiRange1d_clear (&clipSectors);
    for (i = 0; i < numAsymptote; i++)
        {
        angles[numAngle++] = pTrigPoint[i].z;
        }

    for (i = 0; i < numPlaneIntersection; i++)
        {
        angles[numAngle++] = Angle::Atan2 (pPlaneTrigPoint[i].y, pPlaneTrigPoint[i].x);
        }

    numAngle = jmdlDEllipse4d_sortAndAugmentAngles (angles, numAngle);

    for (i = 1; i < numAngle; i++)
        {
        theta0 = angles[i-1];
        theta1 = angles[i];
        thetaMid = 0.5 * (theta0 + theta1);
        c = cos (thetaMid);
        s = sin (thetaMid);
        curvePoint.SumOf(pEllipse->center, pEllipse->vector0, c, pEllipse->vector90, s);
        f = curvePoint.DotProduct (*pPlaneVector) * curvePoint.w;
        if (f < 0.0)
            {
            /* This sector is IN!!!*/
            bsiRange1d_addArcSweep( &clipSectors, theta0, theta1 - theta0);
            num0++;
            }
        else
            {
            num1++;
            }
        }
    priorSectors = pEllipse->sectors;
    bsiRange1d_intersect( &pEllipse->sectors, &clipSectors, &priorSectors );

    }

/**
           bsiDEllipse4d_evaluateAsymptotes
* Clips the sector data of the ellipse to a plane.


* @param pTrigPoint <= x,y=cosine and sine of angles. z= angle
* @param pEllipse => Ellipse to evaluate
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_evaluateAsymptotes

(
DPoint3dP pTrigPoint,
DEllipse4dCP pEllipse
)
    {
    double wC = pEllipse->center.w;
    double w0 = pEllipse->vector0.w;
    double w90 = pEllipse->vector90.w;
    int numRoot;
    int i;

    /* Quick test for obvious pure ellipse*/
    if (w0 * w0 + w90 * w90 < 0.99 * wC * wC)
        {
        numRoot = 0;
        }
    else
        {
        numRoot = bsiMath_solveApproximateUnitQuadratic (
                    &pTrigPoint[0].x, &pTrigPoint[0].y,
                    &pTrigPoint[1].x, &pTrigPoint[1].y,
                    wC, w0, w90,
                    s_lineUnitCircleIntersectionTolerance
                    );
        if (numRoot < 0)
            numRoot = 0;
        for (i = 0; i < numRoot; i++)
            pTrigPoint[i].z = Angle::Atan2 (pTrigPoint[i].y, pTrigPoint[i].x);
        }

    return numRoot;
    }




/*-----------------------------------------------------------------*//**
* Clips the sector data of the ellipse to a plane.


* @param ellipseP <= Ellipse to be tested
* @param trigPointP => trig values of clip sector
* @param planeP => plane vector
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clipSectors

(
DEllipse4dP ellipseP,
DPoint3dP trigPointP,
DPoint4dCP planeP
)
    {
    double c0 = trigPointP[0].x;
    double s0 = trigPointP[0].y;

    SmallSetRange1d clipSectors;
    SmallSetRange1d priorSectors;
    double delta;
    double dFdTheta = -s0 * ellipseP->vector0.DotProduct (*planeP)
                      +c0 * ellipseP->vector90.DotProduct (*planeP);

    /* Swap points if needed so that point 0 is the downward crossing under a positive-direction
       sweep through the angular range */
    if ( dFdTheta > 0.0 )
        {
        DPoint3d tempPoint;

        tempPoint = trigPointP[0];
        trigPointP[0] = trigPointP[1];
        trigPointP[1] = tempPoint;
        }

    delta = trigPointP[1].z - trigPointP[0].z;
    if (delta < 0.0)
        {
        delta += msGeomConst_2pi;
        }

    bsiRange1d_setArcSweep ( &clipSectors, trigPointP[0].z, delta );
    priorSectors = ellipseP->sectors;
    bsiRange1d_intersect( &ellipseP->sectors, &clipSectors, &priorSectors );
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlRange1d_volume                                      |
| Return the 'volume' of the range set, i.e. sum of interval lengths.   |
+----------------------------------------------------------------------*/
static double     jmdlRange1d_volume

(
SmallSetRange1dCP pRangeSet        /* => intervals to sum */
)
    {
    double volume = 0.0;
    int i;
    for (i = 0; i < pRangeSet->n; i++)
        {
        volume += fabs (pRangeSet->interval[i].maxValue - pRangeSet->interval[i].minValue);
        }
    return volume;
    }



/**
           bsiDEllipse4d_clipToPlanes
* Clips an ellipse to multiple planes.


* @instance ellipseP <= Ellipse to be tested
* @param clippedP <= Set true if clipped
* @param planeP => homogeneous plane equations
* @param nPlane => number of planes
* @param clipType => 0 for inside, 1 for outside
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clipToPlanes

(
DEllipse4dP ellipseP,
bool      *clippedP,
DPoint4dCP planeP,
int        nPlane,
int        clipType
)

    {
          int         i,nIntersection;
          DPoint3d    intersectionParameter[2];
          DPoint3d    asymptoteParameter[2];
    const DPoint4d    *currentPlaneP;
          double        hRef;
          double      h;

    int    numAsymptote;

    double v0 = 0.0;
    double v1;

    if (clippedP && !*clippedP)
        v0 = jmdlRange1d_volume (&ellipseP->sectors);

    if (ellipseP->sectors.n <= 0 || nPlane <= 0)
        {
        /* Leave the ellipse as is */
        }
    else if (clipType == 0)
        {
        numAsymptote = bsiDEllipse4d_evaluateAsymptotes (asymptoteParameter, ellipseP);
        for ( i = 0; ellipseP->sectors.n > 0 && i < nPlane; i++ )
            {
            currentPlaneP = planeP + i;
            nIntersection = bsiDEllipse4d_intersectPlane (
                        intersectionParameter,
                        &ellipseP->center,
                        &ellipseP->vector0,
                        &ellipseP->vector90,
                        currentPlaneP );

            if (numAsymptote == 0)
                {
                if ( nIntersection != 2 )
                    {
                    /* No intersections.  Any point classifies it all */
                    h = currentPlaneP->DotProduct (ellipseP->center);
                    hRef = s_lineUnitCircleIntersectionTolerance * bsiDPoint4d_maxAbs (currentPlaneP);
                    if ( h > hRef)
                       {
                       ellipseP->sectors.n = 0;
                       }
                    }
                else
                    {
                    bsiDEllipse4d_clipSectors ( ellipseP, intersectionParameter, currentPlaneP );
                    }
                }
            else if (numAsymptote > 0)
                {
                bsiDEllipse4d_clipHyperbolicSectors (
                            ellipseP, intersectionParameter, nIntersection,
                            currentPlaneP, asymptoteParameter, numAsymptote );

                }
            }
        }

    else if (clipType == 1)
        {
        /* Do an inside clip to find the parts we want to throw away: */
        SmallSetRange1d originalSectors = ellipseP->sectors;
        SmallSetRange1d insideSectors;
        bsiDEllipse4d_clipToPlanes (ellipseP, clippedP, planeP, nPlane, 0);
        insideSectors = ellipseP->sectors;
        /* And subtract them from the originals */
        bsiRange1d_subtract( &ellipseP->sectors, &originalSectors, &insideSectors );
        }

    if (clippedP && !*clippedP)
        {
        /* Find the new size and check if it is reduced ...*/
        v1 = jmdlRange1d_volume (&ellipseP->sectors);
        if (v1 < v0)
            *clippedP = true;
        }
    }




/*-----------------------------------------------------------------*//**
* Force the (homogeneous!!!!!!) 0 and 90 degree vectors to be
* perpendicular (as 4D vectors, not in projection).
*
* @param pNormalizedEllipse
* @param pEllipse
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_normalizeAxes

(
DEllipse4dP pNormalizedEllipse,
DEllipse4dCP pEllipse
)

    {
    double   dotUV, dotUU, dotVV;
    double   c, s, theta;
    double   ax, ay, a, tol;
    DPoint4d vector0, vector90;
    int i;

    dotUV = pEllipse->vector0.DotProduct (pEllipse->vector90);
    dotUU = pEllipse->vector0.DotProduct (pEllipse->vector0);
    dotVV = pEllipse->vector90.DotProduct (pEllipse->vector90);

    ay = dotUU - dotVV;
    ax = 2.0 * dotUV;
    a = dotUU + dotVV;
    tol = bsiTrig_smallAngle () * a;
    if (fabs (ax) < tol && fabs (ay) < tol)
        {
        *pNormalizedEllipse = *pEllipse;
        }
    else
        {
        bsiTrig_halfAngleFunctions (&c, &s, ay, ax);
        *pNormalizedEllipse = *pEllipse;
        vector0 = pEllipse->vector0;
        vector90 = pEllipse->vector90;
        pNormalizedEllipse->vector0.SumOf(vector0, c, vector90, s);
        pNormalizedEllipse->vector90.SumOf(vector0, -s, vector90, c);
        theta = Angle::Atan2 (s,c);
        for (i = 0; i < pNormalizedEllipse->sectors.n; i++)
            {
            pNormalizedEllipse->sectors.interval[i].minValue -= theta;
            pNormalizedEllipse->sectors.interval[i].maxValue -= theta;
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Set the ellipse so that it passes through the same points (same start, same intermediate path,
* same end) of the input ellipse, but has a positive sweep angle.
* This may require altering both the basis vectors and the angle limits.
* @param pEllipse => prior ellipse
* @param theta0 => start angle
* @param theta1 => end angle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initWithPositiveSweep

(
DEllipse4dP pInstance,
DEllipse4dCP pEllipse,
double        theta0,
double        theta1
)

    {
    double sweep = theta1 - theta0;
    *pInstance = *pEllipse;
    if (sweep > 0)
        {
        bsiRange1d_setUncheckedArcSweep (&pInstance->sectors, theta0, sweep);
        }
    else
        {
        pInstance->vector90.Negate(pInstance->vector90);
        bsiRange1d_setUncheckedArcSweep (&pInstance->sectors, -theta0, -sweep);
        }
    }


/**



* @param pHEllipse => ellipse to evaluate
* @param theta => angular parameter
* @see
* @return double
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_tangentMagnitude

(
DEllipse4dCP pHEllipse,
double      theta
)

    {
    DPoint4d hPoint, hTangent;
    double tx, ty, tz, dw, w;
    double s, c;

    s =  sin(theta);
    c =  cos(theta);

    hPoint.SumOf(pHEllipse->center, pHEllipse->vector0, c, pHEllipse->vector90, s);

    hTangent.SumOf(pHEllipse->vector0, -s, pHEllipse->vector90, c);

    dw = hTangent.w;
    w  = hPoint.w;

    tx = hTangent.x * w - hPoint.x * dw;
    ty = hTangent.y * w - hPoint.y * dw;
    tz = hTangent.z * w - hPoint.z * dw;

    return  sqrt (tx * tx + ty * ty + tz * tz) / (w * w);
    }




/*-----------------------------------------------------------------*//**
*
* Compute the signed arc length of a part of a conic section.
* (Negative sweep produces negative arc length, so the return from this
* can be negative.)
*
* @instance pEllipse => ellipse to measure
* @param    theta0  => start angle (parametric angle)
* @param    sweep   => sweep angel (parametric angle)
* @see
* @return double
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_arcLength

(
DEllipse4dCP pEllipse,
double      theta0,
double      sweep
)

    {
    return bsiIntegral_gauss (
                    (OmdlScalarFunction)bsiDEllipse4d_tangentMagnitude,
                    theta0, theta0 + sweep, msGeomConst_piOver2,
                    4, (void *)pEllipse);
    }



/*-----------------------------------------------------------------*//**
* Compute the 0, 1, or 2 angles at which a homogeneous ellipse is goes
* to infinity.
*
* @instance pHEllipse => Ellipse whose singular (asymptote) angles are to be computed.
* @param pAngleArray => 0, 1, or 2 singular angles
* @see
* @return number of singular angles
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_singularAngles

(
DEllipse4dCP pHEllipse,
double      *pAngleArray
)
    {
    double cosValue[2], sinValue[2];
    int n = bsiMath_solveApproximateUnitQuadratic (
                &cosValue[0], &sinValue[0],
                &cosValue[1], &sinValue[1],
                pHEllipse->center.w,
                pHEllipse->vector0.w,
                pHEllipse->vector90.w,
                s_lineUnitCircleIntersectionTolerance
                );
    int i;

    for (i = 0; i < n; i++)
        {
        pAngleArray[i] = Angle::Atan2 (sinValue[i], cosValue[i]);
        }
    return n;
    }



/*-----------------------------------------------------------------*//**
* Find the singular angles of the homogeneous ellipse, and clip out
* a small angular sector around each singular angle.
*
* @instance pHEllipse <=> Ellipse whose singular points are clipped
* @param cutAngle => size of cutout
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_removeSingularSectors

(
DEllipse4dP pHEllipse,
double      cutAngle
)

    {
    double singularAngle[2];
    int n = bsiDEllipse4d_singularAngles (pHEllipse, singularAngle);
    int i;
    SmallSetRange1d originalSectors, clipSectors;

    if (cutAngle <= 0.0)
        cutAngle = s_defaultSingularCutAngle;


    /* Small work item: If the singular angles are well separated and not near pi, you
        could do this subtract in one step.   Since it doesn't happen very often, just do
        separate steps:
    */
    for (i = 0; i < n; i++)
        {
        bsiRange1d_setArcSweep ( &clipSectors, singularAngle[i] - cutAngle, 2.0 * cutAngle );
        originalSectors = pHEllipse->sectors;
        bsiRange1d_subtract(&pHEllipse->sectors, &originalSectors, &clipSectors);
        }
    return n;
    }



/*-----------------------------------------------------------------*//**
* Compute the transfer matrix to normalize a weighted, uncentered
* ellipse into a centered cartesian ellipse.

* @param pMatrix <= transfer matrix
* @param pInverse <= its inverse.   Pass NULL if not needed.
* @param w0 => cosine weight
* @param w90 => sine weight
* @param wCenter => center weight
* @see
* @return true if weights define an angle change.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiEllipse_angularTransferMatrix

(
RotMatrixP pMatrix,
RotMatrixP pInverse,
double          w0,
double          w90,
double          wCenter
)
    {
    double mu;
    double wc2;
    double ww2;
    static double relTol = 1.0e-14;
    bool    boolStat = true;

    wc2 = wCenter * wCenter;
    ww2 = w0 * w0 + w90 * w90;
    mu = wc2 - ww2;
    if ( mu <= relTol * wc2)
        {
        /* It's a hyperbola or parabola*/
        pMatrix->InitIdentity ();
        if (pInverse)
            pInverse->InitIdentity ();
        return false;
        }
    else if (mu == wc2)     /* Yes, exact equality test -- if wc2 is small the squaring will */
                            /* wipe force its bits so far to the right they have no effect on the subtraction*/
        {
        /* It's already practically a circle.*/
        pMatrix->InitIdentity ();
        if (pInverse)
            pInverse->InitIdentity ();
        }
    else
        {
        double divW = 1.0 / sqrt (ww2);
        double bx = w0  * divW;
        double by = w90 * divW;
        double gamma = wc2 / mu;
        double rootGamma = sqrt (gamma);
        double centerScale = gamma / wCenter;

        pMatrix->SetColumn (0, gamma * bx, gamma * by, 0.0);
        pMatrix->SetColumn (1, rootGamma * -by, rootGamma * bx, 0.0);
        pMatrix->SetColumn (2, - centerScale * w0, - centerScale * w90, 1.0);
        if (pInverse)
            {
            pInverse->InverseOf (*pMatrix);
            }
        }
    return boolStat;
    }


/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Return the phi corresponding to theta.

* @param theta => known angle prior to transform
* @param pMatrix => transfer matrix.
* @param  => matrix M
* @param alpha => cosine coefficient
* @param beta => sine coefficient
* @see
* @return modified angle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_transferAngle

(
double          theta,
RotMatrixP pMatrix,

double          alpha,
double          beta
)
    {
    DPoint3d G;
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double wF       = 1.0 + alpha * cosTheta + beta * sinTheta;
    double phi;
    pMatrix->MultiplyComponents(G, cosTheta, sinTheta, wF);
    phi = atan2 (G.y, G.x);
    return phi;
    }

/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Replace all angles (theta) in an ellispe's stroke intervals by
* corresponding phi angles.

* @param pDest <=> Ellipse whose angles are corrected.
* @param pSource => source of angle data
* @param pMatrix => matrix M
* @param alpha => cosine coefficient
* @param beta => sine coefficient
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_transferAngles

(
DEllipse4dP pDest,
DEllipse4dCP pSource,
RotMatrixP pMatrix,
double          alpha,
double          beta
)
    {
    int i;
    double phi0, phi1, theta0, theta1, thetaMid, phiMid, dPhi;
    SmallSetRange1d sectors;

    sectors = pSource->sectors;
    bsiRange1d_clear (&pDest->sectors);
    for (i = 0; i < sectors.n; i++)
        {
        theta0 = sectors.interval[i].minValue;
        theta1 = sectors.interval[i].maxValue;
        if (Angle::IsFullCircle (theta1 - theta0))
            {
            bsiRange1d_setArcSweep (&pDest->sectors, 0.0, msGeomConst_2pi);
            return true;
            }
        else
            {
            thetaMid = 0.5 * (theta1 + theta0);
            phi0 = bsiDEllipse4d_transferAngle (theta0, pMatrix, alpha, beta);
            phi1 = bsiDEllipse4d_transferAngle (theta1, pMatrix, alpha, beta);
            dPhi = phi1 - phi0;
            phiMid = bsiDEllipse4d_transferAngle (thetaMid, pMatrix, alpha, beta);

            if ((phiMid - phi0) * (phi1 - phiMid) < 0.0)
                {
                if (dPhi > 0.0)
                    {
                    phi1 -= msGeomConst_2pi;
                    }
                else
                    {
                    phi1 +=  msGeomConst_2pi;
                    }
                }
            bsiRange1d_addUnordered (&pDest->sectors, phi0, phi1);
                }
        }
    return true;
    }



/*-----------------------------------------------------------------*//**
*
* Find new basis vectors with 0 weights on the U and V vectors, and unit
* on the C vector.  This computation is not possible if the curve is
* a hyperbola or parabola when projected to 3D.
*
* @instance pNormalized <= normalized form
* @param pWeighted => unnormalized form
* @see
* @return true if the curve really is an ellipse (i.e. not hyperbola or parabola)
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_normalizeWeights

(
DEllipse4dP pNormalized,
DEllipse4dCP pWeighted
)

    {
    double w0 = pWeighted->vector0.w;
    double w90 = pWeighted->vector90.w;
    double wCenter = pWeighted->center.w;
    RotMatrix   transferMatrix;
    RotMatrix   inverseTransferMatrix;
    bool    boolStat = bsiEllipse_angularTransferMatrix(
                                        &transferMatrix,
                                        &inverseTransferMatrix,
                                        w0,
                                        w90,
                                        wCenter
                                        );
    DPoint4d    vector0, vector90, center;

    if (boolStat)
        {
        double recip = 1.0 / wCenter;
        double alpha = w0 * recip;
        double beta  = w90 * recip;

        vector0.SumOf(pWeighted->vector0, recip, pWeighted->center, - alpha * recip);
        vector90.SumOf(pWeighted->vector90, recip, pWeighted->center, - beta * recip);
        center.Scale (pWeighted->center, recip);

        /* The transfer matrix is of the form*/
        /*          [ rxx  rxy  cx]*/
        /*          [ ryx  ryy  cy]*/
        /*          [  0    0    1]*/
        pNormalized->vector0.SumOf(vector0, transferMatrix.form3d[0][0], vector90, transferMatrix.form3d[1][0]);
        pNormalized->vector90.SumOf(vector0, transferMatrix.form3d[0][1], vector90, transferMatrix.form3d[1][1]);
        pNormalized->center.SumOf(center, vector0, transferMatrix.form3d[0][2], vector90, transferMatrix.form3d[1][2]);
        bsiDEllipse4d_transferAngles( pNormalized, pWeighted, &inverseTransferMatrix, w0 / wCenter, w90 / wCenter);
        }
    else
        {
        *pNormalized = *pWeighted;
        }
    return boolStat;
    }



/*-----------------------------------------------------------------*//**
*
* Negate vectors as needed so the center weight is positive (but retains absolute
* value)                                                                *
*
* @instance pNormalized <= copied and scaled ellipse
* @param pWeighted => original ellipse
* @see
* @return true unless weight is zero
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_absCenterWeight

(
DEllipse4dP pNormalized,
DEllipse4dCP pWeighted
)

    {
    bool    boolStat = true;
    *pNormalized = *pWeighted;

    if (pNormalized->center.w < 0.0)
        {
        pNormalized->center.Negate(pNormalized->center);
        pNormalized->vector0.Negate(pNormalized->vector0);
        pNormalized->vector90.Negate(pNormalized->vector90);
        }
    else if (pNormalized->center.w == 0.0)
        {
        boolStat = false;
        }

    return boolStat;
    }

/*-----------------------------------------------------------------*//**
*
* Shuffle the eigenvalues so that there are two eigevalues of one sign
*   followed by one of the other.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static bool    jmdlDEllipse4d_sortEllipticEigensystem

(
RotMatrixP pQ,
double   *pLambda
)
    {
    RotMatrix Q;
    bool    boolStat = false;
    double lambda[3];
    double bigVal, smallVal;
    static double s_relTol = 1.0e-10;
    int iPos[3], iNeg[3];
    int nPos, nNeg;
    int i;

    bigVal = fabs (pLambda[0]) + fabs (pLambda[1]) + fabs (pLambda[2]);

    smallVal = s_relTol * bigVal;

    if (   fabs (pLambda[0]) > smallVal
        && fabs (pLambda[1]) > smallVal
        && fabs (pLambda[2]) > smallVal
       )
        {

        nPos = nNeg = 0;

        for (i = 0; i < 3; i++)
            {
            if (pLambda[i] > 0.0)
                iPos[nPos++] = i;
            else
                iNeg[nNeg++] = i;
            lambda[i] = pLambda[i];
            }

        Q = *pQ;

        if (nPos == 1)
            {
            /* Copy back the two negatives,then the positive */
            pLambda[0] = lambda[iNeg[0]];
            pLambda[1] = lambda[iNeg[1]];
            pLambda[2] = lambda[iPos[0]];
            pQ->InitFromColumnVectors (
                    DVec3d::FromColumn (Q, iNeg[0]),
                    DVec3d::FromColumn (Q, iNeg[1]),
                    DVec3d::FromColumn (Q, iPos[0]));
            boolStat = true;
            }
        else if (nNeg == 1)
            {
            /* Copy back the two positives,then the negatives */
            pLambda[0] = lambda[iPos[0]];
            pLambda[1] = lambda[iPos[1]];
            pLambda[2] = lambda[iNeg[0]];
            pQ->InitFromColumnVectors (
                    DVec3d::FromColumn (Q, iPos[0]),
                    DVec3d::FromColumn (Q, iPos[1]),
                    DVec3d::FromColumn (Q, iNeg[0]));
            boolStat = true;
            }
        }
    return boolStat;

    }

/*-----------------------------------------------------------------*//**
*
* Initialize a (complete) homogeneous ellipse from its xy implicit form.
* The implicit form is
*       axx * x^2 + axy * x * y + ayy * y * y + ax * x + ay * y + a = 0
* The ellipse is placed with z=0.
*
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsifDEllipse4d_initFromImplicitCoefficients

(
DEllipse4dP pInstance,
double          axx,
double          axy,
double          ayy,
double          ax,
double          ay,
double          a
)
    {
    bool    boolStat = false;
    RotMatrix Q;        /* Eigenvectors */
    RotMatrix B;        /* Scaled basis */
    DPoint3d  lambda;   /* Eigenvalues */
    double lambdaA[3];
    RotMatrix A;

    A.InitFromRowValues (
                axx, axy * 0.5, ax * 0.5,
                axy * 0.5, ayy, ay * 0.5,
                ax * 0.5, ay * 0.5,  a);
    bsiRotMatrix_symmetricEigensystem (&Q, &lambda, &A);

    if (jmdlDEllipse4d_sortEllipticEigensystem (&Q, lambdaA))
        {
        lambda. Init (lambdaA[0], lambdaA[1], lambdaA[2]);
        B.ScaleColumns (Q,
                            1.0 / sqrt (fabs (lambda.x)),
                            1.0 / sqrt (fabs (lambda.y)),
                            1.0 / sqrt (fabs (lambda.z)));

        pInstance->center.Init(
                        B.form3d[0][2], B.form3d[1][2], 0.0, B.form3d[2][2]);

        pInstance->vector0.Init(
                        B.form3d[0][0], B.form3d[1][0], 0.0, B.form3d[2][0]);

        pInstance->vector90.Init(
                        B.form3d[0][1], B.form3d[1][1], 0.0, B.form3d[2][1]);

        bsiDEllipse4d_setFullCircleSweep (pInstance);
        boolStat = true;
        }

    return boolStat;
    }


/*-----------------------------------------------------------------*//**
* Extend a range to include the range of an ellipse.
* If the homogeneous weights have zeros (i.e. the cartesian image is a
* hyperbola or or parabola) the range is computed with a small angular
* sector clipped away from the singularity.

* @instance pRange <=> range extended by ellipse range.
* @param pHEllipse => Ellipse whose range is computed
* @see
* @indexVerb extend
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_extendRange

(
DRange3dP pRange,
DEllipse4dCP pHEllipse
)

    {
    DEllipse4d workEllipse;
    DPoint4d    point;
    int i;
    if (pHEllipse->sectors.n > 0)
        {
        workEllipse = *pHEllipse;
        bsiDEllipse4d_removeSingularSectors (&workEllipse, 0.0);
        /* Candidate points that can contribute to the range limits are:
            1) sector endpoints
            2) local extrema
        */
        if (bsiRange1d_isFullCircle (&workEllipse.sectors))
            {
            /* The DEllipse4d is a complete complete true ellipse.  Only local extrema need to
                be considered, and they do not need to have range tests applied. */

            bsiDEllipse4d_componentRange (&pRange->low.x, &pRange->high.x,
                    pHEllipse->center.x, pHEllipse->vector0.x, pHEllipse->vector90.x,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    NULL);

            bsiDEllipse4d_componentRange (&pRange->low.y, &pRange->high.y,
                    pHEllipse->center.y, pHEllipse->vector0.y, pHEllipse->vector90.y,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    NULL);

            bsiDEllipse4d_componentRange (&pRange->low.z, &pRange->high.z,
                    pHEllipse->center.z, pHEllipse->vector0.z, pHEllipse->vector90.z,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    NULL);

            }
        else
            {
            /* Test local extrema */
            bsiDEllipse4d_componentRange (&pRange->low.x, &pRange->high.x,
                    pHEllipse->center.x, pHEllipse->vector0.x, pHEllipse->vector90.x,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    &workEllipse.sectors);

            bsiDEllipse4d_componentRange (&pRange->low.y, &pRange->high.y,
                    pHEllipse->center.y, pHEllipse->vector0.y, pHEllipse->vector90.y,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    &workEllipse.sectors);

            bsiDEllipse4d_componentRange (&pRange->low.z, &pRange->high.z,
                    pHEllipse->center.z, pHEllipse->vector0.z, pHEllipse->vector90.z,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    &workEllipse.sectors);

            /* Test sector endpoints */
            for (i = 0; i < workEllipse.sectors.n; i++)
                {
                bsiDEllipse4d_evaluateDPoint4d (pHEllipse, &point, workEllipse.sectors.interval[i].minValue);
                bsiDRange3d_extendByDPoint4d (pRange, &point);
                bsiDEllipse4d_evaluateDPoint4d (pHEllipse, &point, workEllipse.sectors.interval[i].maxValue);
                bsiDRange3d_extendByDPoint4d (pRange, &point);
                }
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Extend a range to include the range of the xy parts of an ellipse.
* If the homogeneous weights have zeros (i.e. the cartesian image is a
* hyperbola or or parabola) the range is computed with a small angular
* sector clipped away from the singularity.

* @instance pRange <=> range extended by ellipse range.
* @param pHEllipse => Ellipse whose range is computed
* @see
* @indexVerb extend
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange2d_extendByDellipse4d

(
DRange2dP pRange,
DEllipse4dCP pHEllipse
)

    {
    DEllipse4d workEllipse;
    DPoint4d    point;
    int i;
    if (pHEllipse->sectors.n > 0)
        {
        workEllipse = *pHEllipse;
        bsiDEllipse4d_removeSingularSectors (&workEllipse, 0.0);
        /* Candidate points that can contribute to the range limits are:
            1) sector endpoints
            2) local extrema
        */
        if (bsiRange1d_isFullCircle (&workEllipse.sectors))
            {
            /* The DEllipse4d is a complete complete true ellipse.  Only local extrema need to
                be considered, and they do not need to have range tests applied. */

            bsiDEllipse4d_componentRange (&pRange->low.x, &pRange->high.x,
                    pHEllipse->center.x, pHEllipse->vector0.x, pHEllipse->vector90.x,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    NULL);

            bsiDEllipse4d_componentRange (&pRange->low.y, &pRange->high.y,
                    pHEllipse->center.y, pHEllipse->vector0.y, pHEllipse->vector90.y,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    NULL);

            }
        else
            {
            /* Test local extrema */
            bsiDEllipse4d_componentRange (&pRange->low.x, &pRange->high.x,
                    pHEllipse->center.x, pHEllipse->vector0.x, pHEllipse->vector90.x,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    &workEllipse.sectors);

            bsiDEllipse4d_componentRange (&pRange->low.y, &pRange->high.y,
                    pHEllipse->center.y, pHEllipse->vector0.y, pHEllipse->vector90.y,
                    pHEllipse->center.w, pHEllipse->vector0.w, pHEllipse->vector90.w,
                    &workEllipse.sectors);

            /* Test sector endpoints */
            for (i = 0; i < workEllipse.sectors.n; i++)
                {
                bsiDEllipse4d_evaluateDPoint4d (pHEllipse, &point, workEllipse.sectors.interval[i].minValue);
                bsiDRange2d_extendByDPoint4d (pRange, &point);
                bsiDEllipse4d_evaluateDPoint4d (pHEllipse, &point, workEllipse.sectors.interval[i].maxValue);
                bsiDRange2d_extendByDPoint4d (pRange, &point);
                }
            }
        }
    }
/*-----------------------------------------------------------------*//**
*
* Fill in conic data from the center and vectors of a DEllipse4d,
* with given angles.
*
* @instance pInstance <= initialized conic
* @param pSource    => source conic
* @param theta0     => start angle
* @param sweep      => sweep angle
*
* @indexVerb init
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromDEllipse4dSweep

(
DConic4dP pInstance,
DEllipse4dCP pSource,
double      theta0,
double      sweep
)
    {
    pInstance->center       = pSource->center;
    pInstance->vector0      = pSource->vector0;
    pInstance->vector90     = pSource->vector90;
    pInstance->start        = theta0;
    pInstance->sweep        = sweep;
    }

/*-----------------------------------------------------------------*//**
*
* Fill in conic data from the center and vectors of a DEllipse4d
* with angular range from a specified sector.  If sector index is invalid,
* make the DConic4d complete.
*
* @instance pInstance <= initialized conic
* @param pSource    => source conic
* @param sector     => sector index
*
* @indexVerb init
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromDEllipse4dSector

(
DConic4dP pInstance,
DEllipse4dCP pSource,
int         index
)
    {
    double start, end;
    if (bsiDEllipse4d_getSector (pSource, &start, &end, index))
        {
        bsiDConic4d_initFromDEllipse4dSweep (pInstance, pSource, start, end - start);
        }
    else
        bsiDConic4d_initFromDEllipse4dSweep (pInstance, pSource, 0.0, msGeomConst_2pi);
    }
/*---------------------------------------------------------------------------------**//**
* Project xy coordinates to the xy projection of a homogeneous ellipse.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_projectDPoint3dToDEllipse4dXY

(
DPoint4dP pProjection,
double      *pParam,
DPoint3dCP pPoint,
DEllipse4dCP pEllipse

)
    {
    RotMatrix   matrixB;
    RotMatrix   projectionConic;
    DPoint3d    xywPoint;
    double      sinTheta[4], cosTheta[4], theta[4], nearTheta[4];
    int         numTheta, numBoundedTheta, iNearPoint;
    DPoint4d    nearPoint[4];
    bool        boolStat = false;

    xywPoint.Init ( pPoint->x, pPoint->y, 1.0);

    bsiQCoff_xyEllipseRotMatrix (&matrixB, pEllipse);

    bsiQCoff_projectToEllipse (&projectionConic, &matrixB, &xywPoint);

    if (SUCCESS == bsiMath_implicitConicIntersectUnitCircle
                            (
                            cosTheta,
                            sinTheta,
                            theta,
                            &numTheta,
                            &projectionConic
                            )
        && numTheta > 0
        )
        {



        numBoundedTheta = bsiDEllipse4d_testAndEvaluateAngles
                                    (
                                    pEllipse,
                                    nearPoint,
                                    nearTheta,
                                    theta,
                                    numTheta
                                    );

        if (numBoundedTheta > 0)
            {
            iNearPoint = bsiGeom_closestXYDPoint4d (pPoint, nearPoint, numBoundedTheta);
            if (pParam)
                *pParam =  nearTheta[iNearPoint];
            if (pProjection)
                *pProjection = nearPoint[iNearPoint];
            boolStat = true;
            }
        }
    return boolStat;
    }

//--------------------------------------------------------------------------------------
// Conditional update range of one component of a homogeneous trig function.
// That is, find local extrema of
//   (x0 + x1 * cos(theta) + x2 * sin(theta)) / (w0 + w1 * cos(theta) + w2 * sin(theta))
// and augment minP, maxP if the angle of the extrema is 'in' the sector set.
// @bsimethod
//--------------------------------------------------------------------------------------
Public GEOMDLLIMPEXP void    bsiDEllipse4d_componentRange

(
double*         pMin,           /* <=> min coordiante of range box */
double*         pMax,           /* <=> max coordinate of range box */
double          x0,             /* => constant term of numerator */
double          x1,             /* => cosine term of numerator */
double          x2,             /* => sine term of numerator */
double          w0,             /* => constant term of numerator */
double          w1,             /* => cosine term of numerator */
double          w2,             /* => sine term of numerator */
SmallSetRange1dP pRangeSet    /* => range set with 'in' intervals. */
)
    {
    double alpha, beta, gamma;
    int numPole, i;
    double numerator, denominator, x;
    double cosine[2], sine[2];
    double angle;

    alpha   = x2 * w1 - x1 * w2;
    beta    = x2 * w0 - x0 * w2;
    gamma   = x0 * w1 - x1 * w0;

    numPole = bsiMath_solveApproximateUnitQuadratic (
                &cosine[0], &sine[0],
                &cosine[1], &sine[1],
                alpha, beta, gamma,
                s_lineUnitCircleIntersectionTolerance
                );

    if (numPole > 0)
        {
        for (i = 0; i < numPole; i++)
            {
            angle = Angle::Atan2 (sine[i], cosine[i]);
            if (!pRangeSet || bsiRange1d_pointIsIn (pRangeSet, angle))
                {
                numerator   = x0 + x1 * cosine[i] + x2 * sine[i];
                denominator = w0 + w1 * cosine[i] + w2 * sine[i];
                if (denominator != 0.0)
                    {
                    x = numerator / denominator;
                    FIX_MIN(x, *pMin);
                    FIX_MAX(x, *pMax);
                    }
                }
            }
        }
    else
        {
        /* degenerate equation.*/
        /* Plug in the point at 0 degrees as a dummy*/
        numerator = x0 + x1;
        denominator = w0 + w1;
        if (denominator != 0.0)
            {
            x = numerator / denominator;
            FIX_MIN(x, *pMin);
            FIX_MAX(x, *pMax);
            }
        }
    }

/*-----------------------------------------------------------------*//**
* Computes the silhouette ellipse of an ellipsoid under arbitrary
* DMap4d and viewpoint.
*
* @param pHEllipse <= silhouette ellipse/parabola/hyperbola
* @param pEllipsoidPoint => 4 defining points of the ellipsoid
* @param pHMap => further mapping
* @param pEyePoint => eyepoint
* @return false iff the eyeponit is inside the ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_ellipsoidSilhouette

(
DEllipse4dP pHEllipse,
DPoint3dCP pEllipsoidPoint,
DMap4dCP pHMap,
DPoint4dCP pEyePoint
)
    {
    Transform axisTransform; /* basis matrix defined by 4 ellipsoid points*/
    DMap4d axisMap;       /* Homogeneous form of basis matrix*/
    DMap4d BMap;                /* pHMap * axisMap -- the 'full' B matrix for the ellipsoid.*/
    bool    result = false;

    if (pEllipsoidPoint)
        {
        /* In its local coordinate system, the ellipsoid is a unit sphere centered at
            the origin.  Construct this mapping and concatentate it with the given DMap4d
            to give the full transformation of the 'ellipsoid' (which may go to infinity
            if the weight vanishes)
        */
        axisTransform.InitFrom4Points(pEllipsoidPoint[0], pEllipsoidPoint[1], pEllipsoidPoint[2], pEllipsoidPoint[3]);
        result = bsiDMap4d_initFromTransform (&axisMap, &axisTransform, false);

        if (pHMap)
            bsiDMap4d_multiply (&BMap, pHMap, &axisMap);
        else
            BMap = axisMap;
        }
    else
        {
        if (pHMap)
            BMap = *pHMap;
        else
            bsiDMap4d_initIdentity (&BMap);
        result = true;
        }


    if (result)
        {
        DPoint4d localEye;
        DMatrix4d Q, QT, BQT;
        double mag;

        static int indexVector[3] = {2, 0, 1};

        result = false;


        bsiDMatrix4d_multiply4dPoints (&BMap.M1, &localEye, pEyePoint, 1);
        /* Characteristic matrix for sphere negates the w component to turn the eye 'point'*/
        /* into an eye 'plane'.*/
        localEye.w = - localEye.w;

        /* Build orthgonal transformation which rotates z towards the eye.  After this*/
        /* rotation, the silhouette is a constant-z curve.*/
        if (bsiDMatrix3d_initSelectiveRotation (
                        (double*)&Q, &mag, 4, (double *)&localEye, indexVector, 3)
            && fabs (localEye.w) < mag
           )
            {
            DPoint4d vectorW;
            double sineThetaHat   = - localEye.w / mag;
            double cosineThetaHat = sqrt (1.0 - sineThetaHat * sineThetaHat);

            QT.TransposeOf (Q);
            BQT.InitProduct (BMap.M0, QT);

            BQT.GetColumn (pHEllipse->vector0, 0);
            BQT.GetColumn (pHEllipse->vector90, 1);
            BQT.GetColumn (vectorW, 2);
            BQT.GetColumn (pHEllipse->center, 3);
            pHEllipse->vector0.Scale (pHEllipse->vector0, cosineThetaHat);
            pHEllipse->vector90.Scale (pHEllipse->vector90, cosineThetaHat);
            bsiDPoint4d_addScaledDPoint4d (&pHEllipse->center, &pHEllipse->center,
                                            &vectorW, sineThetaHat);
            bsiRange1d_setArcSweep( &pHEllipse->sectors, 0.0, msGeomConst_2pi );

            result = true;
            }
        }

    return result;
    }


/*-----------------------------------------------------------------*//**
*
* @param pHEllipse => the silhouette curve.
* @param pHMap => mapping from local to model.  If NULL, all computations
*                 are local identity map
* @param pEyePoint => eyepoint, in model.  For perspective, from xyz, set w=1
*                     For flat view in direction xyz, set w=0
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiHyperboloid_silhouetteCurve

(
DEllipse4dP pHEllipse,
DMap4dCP pHMap,
DPoint4dCP pEyePoint
)
    {
    StatusInt status = ERROR;
    DPoint4d localEyePoint;
    DPoint4d H;     /* This is EXACTLY the H vector of EDL's writeup */

    /* The silhouette is the intersection of the hyperboloid with
        the plane whose 4d point representation is
                H = diag(1,1,-1,-1) * pHMap->M1 * eyePoint
    */
    if (pHMap)

        {
        bsiDMatrix4d_multiply4dPoints (&pHMap->M1, &localEyePoint, pEyePoint, 1
                                  );
        }
    else
        {
        localEyePoint = *pEyePoint;
        }

    H.Init(
                            localEyePoint.x,
                            localEyePoint.y,
                            -localEyePoint.z,
                            -localEyePoint.w
                            );

    pHEllipse->vector0.Init(   H.z, -H.w, -H.x,  H.y);
    pHEllipse->vector90.Init(  H.w,  H.z, -H.y, -H.x);
    pHEllipse->center.Init(    H.y, -H.x, -H.w,  H.z);

    /* This should fill in the whole ellipse */
    bsiRange1d_clear ( &pHEllipse->sectors ); bsiRange1d_addArcSweep( &pHEllipse->sectors, 0.0, msGeomConst_2pi) ; /* THISWAS a bool thrown away as a statement */

    if (pHMap)
        {
        bsiDMatrix4d_multiplyMatrixPoint (&pHMap->M0, &pHEllipse->center  , &pHEllipse->center  );
        bsiDMatrix4d_multiplyMatrixPoint (&pHMap->M0, &pHEllipse->vector0 , &pHEllipse->vector0 );
        bsiDMatrix4d_multiplyMatrixPoint (&pHMap->M0, &pHEllipse->vector90, &pHEllipse->vector90);
        }

    status = SUCCESS;
    return status;
    }


/*-----------------------------------------------------------------*//**
*
* @param pTrig0ut <= points that are IN the ellipse bounds.  May be same as input.
* @param pNumOut <= number of returned points
* @param pTrigIn <= points to test
* @param numIn => number of points to test
* @param pEllipse => bounding ellipse
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse4d_selectTrigPointsInBounds

(
DPoint3dP pTrig0ut,
int         *pNumOut,
DPoint3dCP pTrigIn,
int         numIn,
DEllipse4dCP pEllipse
)
    {
    int numOut = 0;
    int i;
    for (i = 0; i < numIn; i++)
        {
        if (bsiRange1d_pointIsIn (&pEllipse->sectors, pTrigIn[i].z))
            {
            pTrig0ut[numOut++] = pTrigIn[i];
            }
        }
    *pNumOut = numOut;
    }
#ifdef abc
/*-----------------------------------------------------------------*//**
 @instance pTransform => The transform
 @param pOutEllipse <= transformed ellipse
 @param pInEllipse => The untransformed ellipse
 @see
 @indexVerb
 @group DTransform3d
 @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDTransform3d_multiplyDEllipse4d

(
DTransform3dCP pTransform,
DEllipse4dP pOutEllipse,
DEllipse4dCP pInEllipse
)
    {
    if (pOutEllipse != pInEllipse)
        *pOutEllipse = *pInEllipse;
    bsiDTransform3d_multiplyDPoint4dArray (pTransform, &pOutEllipse->center  , &pOutEllipse->center, 1);
    bsiDTransform3d_multiplyDPoint4dArray (pTransform, &pOutEllipse->vector0 , &pOutEllipse->vector0, 1);
    bsiDTransform3d_multiplyDPoint4dArray (pTransform, &pOutEllipse->vector90, &pOutEllipse->vector90, 1);
    }
#endif
/*-----------------------------------------------------------------*//**
 @instance pTransform => The transform
 @param pOutEllipse <= transformed ellipse
 @param pInEllipse => The untransformed ellipse
 @see
 @indexVerb
 @group Transform
 @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDEllipse4d

(
TransformCP pTransform,
DEllipse4dP pOutEllipse,
DEllipse4dCP pInEllipse
)
    {
    if (pOutEllipse != pInEllipse)
        *pOutEllipse = *pInEllipse;
    pTransform->Multiply (&pOutEllipse->center, &pOutEllipse->center, 1);
    pTransform->Multiply (&pOutEllipse->vector0, &pOutEllipse->vector0, 1);
    pTransform->Multiply (&pOutEllipse->vector90, &pOutEllipse->vector90, 1);
    }



/*-----------------------------------------------------------------*//**
@nodoc DEllipse4d
@description Convert a homogeneous ellipse to cartesian.  Callers should beware of the following
 significant points:
<UL>
<LI>A homogeneous "ellipse" may appear as a hyperbola or parabola in xyz space.
   Hence the conversion can fail.
<LI>When the conversion succeeds, it is still a Very Bad Thing To Do numerically
   because a homogeneous ellipse with "nice" numbers can have very large center and axis
   coordinates.   It is always preferable to do calculations directly on the homogeneous
   ellipse if possible.
<LI>When the conversion succeeds, the axis may be non-perpendicular.  A subsequent call
   may be made to initWithPerpendicularAxes to correct this.
</UL>
 @param pEllipse <= initialized ellipse
 @param pSource => homogeneous ellipse
 @param sector  => angular sector index.  If out of bounds, a full ellipse is created.
 @return true if homogeneous parts allow reduction to simple ellipse. (false if the homogeneous
    parts are a parabola or hyperbola.)
 @group "DEllipse3d Initialization"
 @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_initFromDEllipse4d

(
DEllipse3dP pEllipse,
DEllipse4dCP pSource,
int           sector
)
    {
    DEllipse4d  normalizedSource;
    double      theta0, theta1;
    bool        funcStat = false;
    /* Try to eliminate the weights on the vectors of the source ellipse */
    if (bsiDEllipse4d_normalizeWeights (&normalizedSource, pSource))
        {
        funcStat = true;
        pEllipse->center.XyzOf (normalizedSource.center);
        pEllipse->vector0.XyzOf (normalizedSource.vector0);
        pEllipse->vector90.XyzOf (normalizedSource.vector90);

        if (bsiDEllipse4d_getSector (&normalizedSource, &theta0, &theta1, sector))
            {
            pEllipse->start = theta0;
            pEllipse->sweep = theta1 - theta0;
            }
        else
            {
            pEllipse->start = 0.0;
            pEllipse->sweep = msGeomConst_2pi;
            }
        }
    return funcStat;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

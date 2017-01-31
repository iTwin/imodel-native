/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dellipse3d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------*//**@doctext
 @group DEllipse3d
 A DEllipse3d defines an ellipse by its center and two (not-necessarily-
  perpendicular) basis vectors.  The parameterization of the ellipse is

<pre>
                X = center + vector0 * cos(theta)) *  vector90 * sin(theta)
</pre>

where
<ul>
<li>DPoint3d center = center of the ellipse</li>
<li>DPoint3d vector0 = vector from the center to the point parameterized
                    at 0 degrees.</li>
<li>DPoint3d vector90 = vector from the center to the point parameterized
                    at 90 degrees.</li>
<li>double start = start angle of parameterization.</li>
<li>double sweep = sweep angle of parameterization.</li>
</ul>
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_lineUnitCircleIntersectionTolerance = 1.0e-8;

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data.

 @param pEllipse <= initialized ellipse
 @param cx IN center x coordinate
 @param cy IN center y coordinate
 @param cz IN center z coordinate
 @param ux IN x part of 0 degree vector
 @param uy IN y part of 0 degree vector
 @param uz IN z part of 0 degree vector
 @param vx IN x part of 90 degree vector
 @param vy IN y part of 90 degree vector
 @param vz IN z part of 90 degree vector
 @param theta0 IN start angle in parameter space
 @param sweep IN sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_init

(
DEllipse3dP pEllipse,
double        cx,
double        cy,
double        cz,
double        ux,
double        uy,
double        uz,
double        vx,
double        vy,
double        vz,
double        theta0,
double        sweep
)
    {
    bsiDPoint3d_setXYZ (&pEllipse->center, cx, cy, cz);
    bsiDPoint3d_setXYZ (&pEllipse->vector0, ux, uy, uz);
    bsiDPoint3d_setXYZ (&pEllipse->vector90, vx, vy, vz);

    pEllipse->start        = theta0;
    pEllipse->sweep        = sweep;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from 2D major and minor axis lengths and the angle
   from the global to the local x-axis.

 @param pEllipse <= initialized ellipse
 @param cx      => center x coordinate
 @param cy      => center y coordinate
 @param cz      => z coordinate of all points on the ellipse
 @param rx      => radius along local x axis
 @param ry      => radius along local y axis
 @param thetaX  => angle from global x to local x
 @param theta0  => start angle in parameter space
 @param sweep   => sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromXYMajorMinor

(
DEllipse3dP pEllipse,
double          cx,
double          cy,
double          cz,
double          rx,
double          ry,
double          thetaX,
double          theta0,
double          sweep
)
    {
    double ux = cos (thetaX);
    double uy = sin (thetaX);

    bsiDPoint3d_setXYZ (&pEllipse->center,    cx, cy, cz);
    bsiDPoint3d_setXYZ (&pEllipse->vector0,   rx * ux, rx * uy, 0.0);
    bsiDPoint3d_setXYZ (&pEllipse->vector90, -ry * uy, ry * ux, 0.0);

    pEllipse->start        = theta0;
    pEllipse->sweep        = sweep;
    }



/*-----------------------------------------------------------------*//**
@description Convert a homogeneous ellipse to a Cartesian ellipse.
@remarks Callers should beware of the following significant points:
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
 @return false if the conic weights are zero anywhere, creating hyperbola or parabola which
    cannot be reduced to an ellipse.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_initFromDConic4d

(
DEllipse3dP pEllipse,
DConic4dCP pSource
)
    {
    DConic4d  normalizedSource;
    bool        funcStat = false;

    /* Try to eliminate the weights on the vectors of the source ellipse */
    normalizedSource = *pSource;
    if (   bsiDConic4d_isUnitWeighted (&normalizedSource)
        /* NEEDS WORK */
        /* || bsiDConic4d_initWithNormalizedWeights (&normalizedSource, pSource) */
       )
        {
        funcStat = true;
        bsiDPoint3d_getXYZ (&pEllipse->center,   &normalizedSource.center  );
        bsiDPoint3d_getXYZ (&pEllipse->vector0,  &normalizedSource.vector0 );
        bsiDPoint3d_getXYZ (&pEllipse->vector90, &normalizedSource.vector90);
        pEllipse->start = normalizedSource.start;
        pEllipse->sweep = normalizedSource.sweep;
        }
    return funcStat;
    }

/*-----------------------------------------------------------------*//**
@description Project the source ellipse to a plane.
@instance pSourceEllipse IN known ellipse.
@param pEllipse OUT initialized ellipse
@param pSweepDirection IN direction to project.  If NULL, the plane normal is used.
@param pPlane IN the target plane.
@return false if projection direction is parallel to the plane.  The result ellipse
    is then a copy of the source.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_sweepToPlane
(
DEllipse3dCP pSourceEllipse,
DEllipse3dP pEllipse,
DVec3dCP    pSweepDirection,
DPlane3dCP  pPlane
)
    {
    DVec3d direction = NULL != pSweepDirection ? *pSweepDirection : pPlane->normal;
    DRay3d centerRay;
    centerRay.origin = pSourceEllipse->center;
    centerRay.direction = direction;
    bool    boolstat = false;
    if (   bsiDRay3d_intersectDPlane3d (&centerRay, &pEllipse->center, NULL, pPlane)
        && bsiDVec3d_sweepToPlane (&pSourceEllipse->vector0,  &pEllipse->vector0,  NULL, pSweepDirection, pPlane)
        && bsiDVec3d_sweepToPlane (&pSourceEllipse->vector90, &pEllipse->vector90, NULL, pSweepDirection, pPlane)
       )
        {
        boolstat = true;
        }
    else
        *pEllipse = *pSourceEllipse;
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center, 0 degree, and 90 degree points.

 @param pEllipse <= initialized ellipse
 @param pCenter => ellipse center
 @param pPoint0 => 0 degree point
 @param pPoint90 => 90 degree point
 @param theta0 => start angle
 @param sweep => sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFrom3dPoints

(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pPoint0,
DPoint3dCP pPoint90,
double          theta0,
double          sweep
)

    {
    pEllipse->center = *pCenter;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&pEllipse->vector0, pPoint0, pCenter);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&pEllipse->vector90, pPoint90, pCenter);
    pEllipse->start        = theta0;
    pEllipse->sweep        = sweep;
    }


/*-----------------------------------------------------------------*//**
@description Initialize an elliptical arc from 3 points.

 @param pEllipse <= initialized ellipse
 @param pStart => start point
 @param pMiddle => mid point
 @param pEnd => end point
 @return true if the three points are valid, false if colinear.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initFrom3DPoint3dOnArc

(
DEllipse3dP pEllipse,
DPoint3dCP pStart,
DPoint3dCP pMiddle,
DPoint3dCP pEnd
)

    {
    DPoint3d normal;
    DPoint3d vector01, vector12;
    DPoint3d perp01, perp12;
    double param0, param1;
    DPoint3d center0, center1;
    DPoint3d mid01, mid12;
    DPoint3d endVector;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector01, pMiddle, pStart);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector12, pEnd, pMiddle);
    bsiDPoint3d_normalizedCrossProduct (&normal, &vector01, &vector12);
    bsiDPoint3d_crossProduct (&perp01, &vector01, &normal);
    bsiDPoint3d_crossProduct (&perp12, &vector12, &normal);
    bsiDPoint3d_addScaledDPoint3d (&mid01, pStart, &vector01, 0.5);
    bsiDPoint3d_addScaledDPoint3d (&mid12, pMiddle, &vector12, 0.5);

    if (bsiGeom_closestApproachOfRays
                (&param0, &param1, &center0, &center1, &mid01, &perp01, &mid12, &perp12))
        {
        pEllipse->center = center0;
        bsiDPoint3d_subtractDPoint3dDPoint3d (&pEllipse->vector0, pStart, &center0);
        bsiDPoint3d_crossProduct (&pEllipse->vector90, &normal, &pEllipse->vector0);
        pEllipse->start = 0.0;
        bsiDPoint3d_subtractDPoint3dDPoint3d (&endVector, pEnd, &center0);
        pEllipse->sweep = bsiDPoint3d_signedAngleBetweenVectors (
                        &pEllipse->vector0, &endVector, &normal);
        if (pEllipse->sweep < 0.0)
            pEllipse->sweep = bsiTrig_complementaryAngle (pEllipse->sweep);
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
@description Initialize a circlular arc from start point, end point, another vector which
  determines the plane, and the arc length.

 @param pEllipse <= initialized ellipse
 @param pStart => start point
 @param pEnd => end point
 @param arcLength => required arc length
 @param pPlaneVector => vector to be used to determine the plane of the
                    arc.  The plane is chosen so that it contains both the
                    start-to-end vector and the plane vector, and the arc bulge
                    is in the direction of the plane vector (rather than opposite).
 @return true if the arc length exceeds the chord length and the 2 points and plane vector
                determine a clear plane.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      01/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initArcFromChordAndArcLength

(
DEllipse3dP pEllipse,
DPoint3dCP pStart,
DPoint3dCP pEnd,
double                  arcLength,
DPoint3dCP pPlaneVector
)

    {
    bool    boolstat = false;
    double lambda;
    static double s_minPhi = 1.0e-3;
    double chordLength = bsiDPoint3d_distance (pStart, pEnd);
    double s_tol = bsiTrig_smallAngle ();
    static double s_formulaSplit = 0.95;

    if (chordLength < arcLength)
        {
        int count;
        static int maxCount = 20;
        double phi, dPhi;
        lambda = chordLength / arcLength;
        /* Newton-Raphson iteration for
                f(phi)  = sin(phi)-lambda * phi = 0
                f'(phi) = cos(phi)-lambda
            The line lambda*phi and the sine wave both pass
                through (0,0).   For 0 < lambda < 1, they
                have one additional root between 0 and pi.
        To get a starting guess:
            Approximate sin(theta) by a parabola
                q(theta) = -4 theta (theta-pi)/pi^2
            solve q(theta) - lambda theta = 0
            This has a root at 0 and one at
                theta = pi (1 - pi lambda / 4)
        For lambda safely away from one, this converges in typical NR fashion --
            half dozen iterations.   As lambda approaches one,
            the equation's two roots come together, and it is easily seen
            (experimentally) that the NR still converges, albeit slowly.
            Examples:
                lambda  count
                .99999  17
                .9797    8
                .9213    6
                .519     4
        For the small values, a better estimate comes from the small angle formula
         sin(x) = x - x^3/6; hence
            (1-lambda)x - x^3/6 = 0

        */
        if (lambda < s_formulaSplit)
            {
            phi = msGeomConst_pi * (1.0 - msGeomConst_pi * lambda * 0.25);
            }
        else
            phi = sqrt (6.0 * (1.0 - lambda));


        dPhi = 1.0;
        for (count = 0; count < maxCount && fabs (dPhi) > s_tol; count++)
            {
            if (phi > msGeomConst_2pi)
                phi = msGeomConst_2pi;
            if (phi < s_minPhi)
                phi = msGeomConst_piOver2;
            dPhi = (sin (phi) - lambda * phi) / ( cos(phi) - lambda);
            phi -= dPhi;
            }

        if (fabs (dPhi) <= s_tol)
            {
            DPoint3d chord, normal, binormal;
            DPoint3d center;
            DVec3d vector0, vector90;
            double b;
            bsiDPoint3d_subtractDPoint3dDPoint3d (&chord, pEnd, pStart);
            if (!bsiDPoint3d_areParallel (&chord, pPlaneVector)
                && bsiTrig_safeDivide
                    (
                    &b,
                    chordLength * cos (phi),
                    2.0 * sin (phi),
                    1.0))
                {
                bsiDPoint3d_normalizedCrossProduct (&normal, &chord, pPlaneVector);
                bsiDPoint3d_normalizedCrossProduct (&binormal, &normal, &chord);
                bsiDPoint3d_add2ScaledDPoint3d
                        (
                        &center,
                        pStart,
                        &chord, 0.5,
                        &binormal, -b);
                bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pStart, &center);
                bsiDPoint3d_crossProduct (&vector90, &vector0, &normal);
                pEllipse->center = center;
                pEllipse->vector0 = vector0;
                pEllipse->vector90 = vector90;
                pEllipse->start = 0.0;
                pEllipse->sweep = 2.0 * phi;
                boolstat = true;
                }
            }
        }
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
@description Initialize a circular arc from start point, start tangent, and end point.

 @param pEllipse <= initialized ellipse
 @param pStart => start point
 @param pTangent => start tangent
 @param pEnd => end point
 @return true if circular arc computed.   false if start, end and tangent are colinear.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initFromDPoint3dDPoint3dTangent

(
DEllipse3dP pEllipse,
DPoint3dCP pStart,
DPoint3dCP pTangent,
DPoint3dCP pEnd
)

    {
    DPoint3d normal;
    DPoint3d vector01;
    DPoint3d midPoint;
    DPoint3d perp0, perp01;
    double param0, param1;
    DPoint3d center0, center1;
    DPoint3d endVector;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector01, pEnd, pStart);
    bsiDPoint3d_normalizedCrossProduct (&normal, pTangent, &vector01);
    bsiDPoint3d_crossProduct (&perp01, &normal, &vector01);
    bsiDPoint3d_crossProduct (&perp0, &normal, pTangent);
    bsiDPoint3d_addScaledDPoint3d (&midPoint, pStart, &vector01, 0.5);

    if (bsiGeom_closestApproachOfRays
                (&param0, &param1, &center0, &center1, &midPoint, &perp01, pStart, &perp0))
        {
        pEllipse->center = center0;
        bsiDPoint3d_subtractDPoint3dDPoint3d (&pEllipse->vector0, pStart, &center0);
        bsiDPoint3d_crossProduct (&pEllipse->vector90, &normal, &pEllipse->vector0);
        pEllipse->start = 0.0;
        bsiDPoint3d_subtractDPoint3dDPoint3d (&endVector, pEnd, &center0);
        pEllipse->sweep = bsiDPoint3d_signedAngleBetweenVectors (
                        &pEllipse->vector0, &endVector, &normal);
        if (pEllipse->sweep < 0.0)
            pEllipse->sweep = bsiTrig_complementaryAngle (pEllipse->sweep);
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
@description Initialize a circular arc with given center and start, and with
 sweep so that the end point is near the given end. (Note that the circle
 will NOT pass directly through the endpoint itself if it is at a different
 distance from the center.)  The arc is always the smaller of the two
 possible parts of the full circle.

 @param pEllipse <= initialized ellipse
 @param pCenter => ellipse center
 @param pStart => start point
 @param pEnd => nominal end point
 @return false if the the three points are colinear.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initFromArcCenterStartEnd

(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pStart,
DPoint3dCP pEnd
)

    {
    DPoint3d normal;
    DVec3d vector0, vector90, vectorEnd;
    double cc, ss;
    double sweep;
    double cotangent;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pStart, pCenter);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorEnd, pEnd, pCenter);
    bsiDPoint3d_normalizedCrossProduct (&normal, &vector0, &vectorEnd);
    bsiDPoint3d_crossProduct (&vector90, &normal, &vector0);
    cc = bsiDPoint3d_dotProduct (&vectorEnd, &vector0);
    ss = bsiDPoint3d_dotProduct (&vectorEnd, &vector90);
    sweep = bsiTrig_atan2 (ss, cc);
    bsiDEllipse3d_initFrom3dVectors (pEllipse, pCenter, &vector0, &vector90, 0.0, sweep);
    /* If the input was degenerate, the normal vector will be zero, hence
        the sine will be zero, hence we can't divide. */
    return bsiTrig_safeDivide (&cotangent, cc, ss, 1.0);
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center and two basis vectors.

 @param pEllipse <= initialized ellipse
 @param pCenter => ellipse center
 @param pVector0 => 0 degree vector
 @param pVector90 => 90 degree vector
 @param theta0 => start angle
 @param sweep => sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFrom3dVectors

(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DVec3dCP pVector0,
DVec3dCP pVector90,
double          theta0,
double          sweep
)

    {
    pEllipse->center    = *pCenter;
    pEllipse->vector0   = *pVector0,
    pEllipse->vector90  = *pVector90;
    pEllipse->start        = theta0;
    pEllipse->sweep        = sweep;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center, 0 degree, and 90 degree points presented as an array of 3 points.

 @param pEllipse <= initialized ellipse
 @param pPointArray => ellipse center, 0 degree and 90 degree points
 @param theta0 => start angle
 @param sweep => sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFrom3dPointArray

(
DEllipse3dP pEllipse,
DPoint3dCP pPointArray,
double              theta0,
double              sweep
)

    {
    bsiDEllipse3d_initFrom3dPoints (pEllipse,
                pPointArray,
                pPointArray + 1,
                pPointArray + 2,
                theta0,
                sweep);
    }


/*-----------------------------------------------------------------*//**
@description Set angular parameters to have given start and end points.
@remarks If the given points are really on the ellipse, this does the expected thing.
@remarks If the given points are not on the ellipse, here's exactly what happens.
    The start/end points are placed on the original ellipse at the point where the ellipse intersects
    the plane formed by the ellipse axis and the given point.  This leaves the problem that the ellipse
    defines two paths from the given start to end. This is resolved as follows.  The ellipse's existing
    0 and 90 degree vectors define a coordinate system.  In that system, the short sweep from the 0
    degree vector to the 90 degree vector is considered "counterclockwise".
@remarks Beware that the relation of supposed start/end points to the ellipse is ambiguous.

 @param pEllipse IN OUT ellipse to update
 @param pStartPoint IN new start point
 @param pEndPoint IN new end point
 @param ccw    => true to force counterclockwise direction, false for clockwise.
 @return true if the ellipse axes are independent.  false if the ellipse is degenerate.
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiDEllipse3d_setStartEnd

(
DEllipse3dP pEllipse,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint,
bool        ccw
)

    {
    Transform inverseFrame;
    DPoint3d    point[2], paramPoint[2];
    double      theta0, theta1, sweep;
    bool        isInvertible;

    isInvertible = bsiDEllipse3d_getLocalFrameTransform (pEllipse, NULL, &inverseFrame);

    if (isInvertible)
        {
        point[0] = *pStartPoint;
        point[1] = *pEndPoint;

        bsiTransform_multiplyDPoint3dArray (&inverseFrame, paramPoint, point, 2);
        theta0 = bsiTrig_atan2 (paramPoint[0].y, paramPoint[0].x);
        theta1 = bsiTrig_atan2 (paramPoint[1].y, paramPoint[1].x);
        sweep  = theta1 - theta0;
        if (sweep < 0.0)
            {
            if (ccw)
                sweep += msGeomConst_2pi;
            }
        else
            {
            if (!ccw)
                sweep -= msGeomConst_2pi;
            }
        pEllipse->start = theta0;
        pEllipse->sweep = sweep;
        }
    return isInvertible;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center, x and y directions from columns
 0 and 1 of a RotMatrix, and scale factors to apply to x and and y directions.

 @param pEllipse <= initialized ellipse
 @param pCenter => ellipse center
 @param pMatrix => columns 0, 1 are ellipse directions (to be scaled by r0, r1)
 @param r0 => scale factor for column 0
 @param r1 => scale factor for column 1
 @param theta0 => start angle
 @param sweep => sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromScaledRotMatrix

(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
RotMatrixCP pMatrix,
double      r0,
double      r1,
double              theta0,
double              sweep
)
    {
    DVec3d vectorU, vectorV;
    bsiRotMatrix_getColumns (pMatrix, &vectorU, &vectorV, NULL);

    bsiDPoint3d_scale (&vectorU, &vectorU, r0);
    bsiDPoint3d_scale (&vectorV, &vectorV, r1);
    bsiDEllipse3d_initFrom3dVectors (pEllipse,
                pCenter,
                &vectorU,
                &vectorV,
                theta0,
                sweep);
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center and x and y directions as vectors with scale factors.

 @param pEllipse <= initialized ellipse
 @param pCenter => ellipse center
 @param pVector0 => 0 degree vector (e.g. major axis)
 @param pVector90 => 90 degree vector (e.g. minor axis)
 @param r0 => scale factor for vector 0
 @param r1 => scale factor for vector 90
 @param theta0 => start angle
 @param sweep => sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromScaledVectors

(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pVector0,
DPoint3dCP pVector90,
double      r0,
double      r1,
double              theta0,
double              sweep
)

    {
    DVec3d vectorU, vectorV;

    bsiDPoint3d_scale (&vectorU, pVector0, r0);
    bsiDPoint3d_scale (&vectorV, pVector90, r1);
    bsiDEllipse3d_initFrom3dVectors (pEllipse,
                pCenter,
                &vectorU,
                &vectorV,
                theta0,
                sweep);
    }

/*-----------------------------------------------------------------*//**
@description Adjust axis vectors so 0-degree vector is along true major axis.
@remarks This is similar to ~mbsiDEllipse3d_initWithPerpendicularAxes, which
       chooses the 0 degree vector <EM>closest</EM> to current 0 degrees, even if
       that is really the "minor" axis.  This function makes the further adjustment
       of bringing the larger axis to 0 degrees in the parameterization.
 @param pMajorMinorEllipse OUT modified ellipse
 @param pEllipse IN source ellipse.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      07/03
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initMajorMinor

(
DEllipse3dP pMajorMinorEllipse,
DEllipse3dCP pEllipse
)
    {
    DVec3d vector180;
    double rr0, rr90;

    bsiDEllipse3d_initWithPerpendicularAxes (pMajorMinorEllipse, pEllipse);
    rr0 = bsiDPoint3d_magnitudeSquared (&pMajorMinorEllipse->vector0);
    rr90 = bsiDPoint3d_magnitudeSquared (&pMajorMinorEllipse->vector90);
    if (rr90 > rr0)
        {
        /* Save the current 180 direction.  Move 0 to 90, 90 to the saved 180.   Adjust start angle. */
        bsiDPoint3d_negate (&vector180, &pMajorMinorEllipse->vector0);
        pMajorMinorEllipse->vector0 = pMajorMinorEllipse->vector90;
        pMajorMinorEllipse->vector90 = vector180;
        pMajorMinorEllipse->start -= msGeomConst_piOver2;
        }
    }



/*-----------------------------------------------------------------*//**
@description Extract major minor axis form of the ellipse.

 @param pEllipse => ellipse to query
 @param pCenter <= ellipse center
 @param pMatrix <= columns 0, 1 are normalized ellipse basis vectors, column 2 is their cross product
 @param pR0 <= scale factor for column 0
 @param pR1 <= scale factor for column 1
 @param pTheta0 <= start angle
 @param pSweep <= sweep angle
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_getScaledRotMatrix
(
DEllipse3dCP pEllipse,
DPoint3dP   pCenter,
RotMatrixP   pMatrix,
double      *pR0,
double      *pR1,
double      *pTheta0,
double      *pSweep
)
    {
    DVec3d xAxis, yAxis, zAxis;
    double r0, r1;
    DEllipse3d majorMinorEllipse;
    bsiDEllipse3d_initWithPerpendicularAxes (&majorMinorEllipse, pEllipse);
    r0 = bsiDPoint3d_normalize (&xAxis, &majorMinorEllipse.vector0);
    r1 = bsiDPoint3d_normalize (&yAxis, &majorMinorEllipse.vector90);
    bsiDPoint3d_crossProduct (&zAxis, &xAxis, &yAxis);

    if (pCenter)
        *pCenter = majorMinorEllipse.center;
    if (pMatrix)
        bsiRotMatrix_initFromColumnVectors (pMatrix, &xAxis, &yAxis, &zAxis);

    if (pR0)
        *pR0 = r0;
    if (pR1)
        *pR1 = r1;

    if (pTheta0)
        *pTheta0 = majorMinorEllipse.start;
    if (pSweep)
        *pSweep = majorMinorEllipse.sweep;
    }



/*-----------------------------------------------------------------*//**
@description Initialize a circle from center, normal and radius.
 @param pEllipse <= initialized ellipse
 @param pCenter => circle center
 @param pNormal => plane normal (NULL for 001)
 @param radius => circle radius
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromCenterNormalRadius

(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pNormal,
double          radius
)

    {
    DVec3d uVector, vVector, wVector;

    if (pNormal)
        {
        bsiDPoint3d_getNormalizedTriad(pNormal, &uVector, &vVector, &wVector);
        bsiDPoint3d_scale (&uVector, &uVector, radius);
        bsiDPoint3d_scale (&vVector, &vVector, radius);
        }
    else
        {
        bsiDPoint3d_setXYZ (&uVector, radius,  0.0,    0.0);
        bsiDPoint3d_setXYZ (&vVector, 0.0,     radius, 0.0);
        }

    bsiDEllipse3d_initFrom3dVectors
                    (
                    pEllipse,
                    pCenter,
                    &uVector,
                    &vVector,
                    0.0,
                    msGeomConst_2pi
                    );
    }


/*-----------------------------------------------------------------*//**
@description Test whether the ellipse is complete (2pi range).
 @param pEllipse => ellipse to query
 @return true if the ellipse is complete
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_isFullEllipse

(
DEllipse3dCP pEllipse
)
    {
    return bsiTrig_isAngleFullCircle (pEllipse->sweep);
    }


/*-----------------------------------------------------------------*//**
@description Set the ellipse sweep to a full 360 degrees (2pi radians), preserving direction of sweep.
@remarks Start angle is left unchanged.
 @param pEllipse <=> ellipse to change
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarldinLutz     12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_makeFullSweep

(
DEllipse3dP pEllipse
)
    {
    pEllipse->sweep = pEllipse->sweep >= 0.0 ? msGeomConst_2pi : -msGeomConst_2pi;
    }


/*-----------------------------------------------------------------*//**
@description Set the ellipse sweep to the complement of its current angular range.
@remarks Full ellipse is left unchanged.
 @param pEllipse <=> ellipse to change
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_complementSweep

(
DEllipse3dP pEllipse
)
    {
    if (!bsiDEllipse3d_isFullEllipse (pEllipse))
        pEllipse->sweep = bsiTrig_complementaryAngle (pEllipse->sweep);
    }



/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given parametric (angular) coordinate.
 @param pEllipse => ellipse to evaluate
 @param pPoint <= evaluated point
 @param theta => angle
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDPoint3d

(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
double      theta
)
    {
    double cosTheta, sinTheta;
    cosTheta = cos(theta);
    sinTheta = sin(theta);
    bsiDPoint3d_add2ScaledDPoint3d (
            pPoint,
            &pEllipse->center,
            &pEllipse->vector0, cosTheta,
            &pEllipse->vector90, sinTheta
            );
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given parametric (xy) coordinate.
 @param pEllipse => ellipse to evaluate
 @param pPoint <= evaluated point
 @param xx => local x coordinate: cos(theta)
 @param yy => local y coordinate: sin(theta)
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDPoint3dFromLocal

(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
double      xx,
double      yy
)
    {
    bsiDPoint3d_add2ScaledDPoint3d (
            pPoint,
            &pEllipse->center,
            &pEllipse->vector0, xx,
            &pEllipse->vector90, yy
            );
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given parametric (angular) coordinate.
 @param pEllipse => ellipse to evaluate
 @param pPoint <= evaluated point (unit weight)
 @param theta => angle
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDPoint4d

(
DEllipse3dCP pEllipse,
DPoint4dP pPoint,
double      theta
)
    {
    double cosTheta, sinTheta;
    DPoint3d point;
    cosTheta = cos(theta);
    sinTheta = sin(theta);
    bsiDPoint3d_add2ScaledDPoint3d (
            &point,
            &pEllipse->center,
            &pEllipse->vector0, cosTheta,
            &pEllipse->vector90, sinTheta
            );
    pPoint->x = point.x;
    pPoint->y = point.y;
    pPoint->z = point.z;
    pPoint->w = 1.0;
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse start and end points.
 @param pEllipse => ellipse to evaluate
 @param pStartPoint <= start point of ellipse
 @param pEndPoint  <= end point of ellipse
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateEndPoints

(
DEllipse3dCP pEllipse,
DPoint3dP pStartPoint,
DPoint3dP pEndPoint
)
    {
    bsiDEllipse3d_evaluateDPoint3d (pEllipse, pStartPoint, pEllipse->start);
    bsiDEllipse3d_evaluateDPoint3d (pEllipse, pEndPoint,   pEllipse->start + pEllipse->sweep  );
    }


/*-----------------------------------------------------------------*//**
@description  Compute the ellipse xyz point and derivatives at a given parametric (angular) coordinate.
 @param pEllipse => ellipse to evaluate
 @param pX <= (optional) point on ellipse
 @param pdX <= (optional) first derivative vector
 @param pddX <= (optional) second derivative vector
 @param theta => angle for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDerivatives

(
DEllipse3dCP pEllipse,
DPoint3dP pX,
DPoint3dP pdX,
DPoint3dP pddX,
double      theta
)
    {
    double cosTheta, sinTheta;
    DPoint3d vector;

    cosTheta = cos(theta);
    sinTheta = sin(theta);

    if (pX || pddX)
        {
        bsiDPoint3d_add2ScaledDPoint3d (
                &vector,
                NULL,
                &pEllipse->vector0, cosTheta,
                &pEllipse->vector90, sinTheta
                );
        if (pX)
            bsiDPoint3d_addDPoint3dDPoint3d (pX, &pEllipse->center, &vector);

        if (pddX)
            bsiDPoint3d_negate (pddX, &vector);
        }

    if (pdX)
        bsiDPoint3d_add2ScaledDPoint3d
                (
                pdX,
                NULL,
                &pEllipse->vector0,  -sinTheta,
                &pEllipse->vector90,  cosTheta
                );
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given fraction of the angular parametric range.
 @param pEllipse => ellipse to evaluate
 @param pX <= point on ellipse
 @param fraction => fractional parameter for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_fractionParameterToDPoint3d

(
DEllipse3dCP pEllipse,
DPoint3dP pX,
double      fraction
)
    {
    double theta = pEllipse->start + fraction * pEllipse->sweep;
    double cosTheta, sinTheta;

    if (pX)
        {
        cosTheta = cos(theta);
        sinTheta = sin(theta);
        bsiDPoint3d_add2ScaledDPoint3d (
                pX,
                &pEllipse->center,
                &pEllipse->vector0, cosTheta,
                &pEllipse->vector90, sinTheta
                );
        }
    }

/* Method(DEllipse3d,none,fractionParameterToDerivatives) */
/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point and derivatives at a given fraction of the angular parametric range.
 @param pEllipse => ellipse to evaluate
 @param pX <= (optional) point on ellipse
 @param pdX <= (optional) second derivative vector
 @param pddX <= (optional) second derivative vector
 @param fraction => fractional parameter for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_fractionParameterToDerivatives

(
DEllipse3dCP pEllipse,
DPoint3dP pX,
DPoint3dP pdX,
DPoint3dP pddX,
double      fraction
)
    {
    double theta = pEllipse->start + fraction * pEllipse->sweep;
    double a = pEllipse->sweep;

    bsiDEllipse3d_evaluateDerivatives (pEllipse, pX, pdX, pddX, theta);
    if (pdX)
        bsiDPoint3d_scale (pdX, pdX, a);
    if (pddX)
        bsiDPoint3d_scale (pddX, pddX, a * a);
    }


/*-----------------------------------------------------------------*//**
@description Compute ellipse xyz point and derivatives, returned as an array.
 @param pEllipse => ellipse to evaluate
 @param pX <= Array of ellipse point, first derivative, etc.  Must contain room for numDerivatives+1 points.  pX[i] = i_th derivative.
 @param numDerivative => number of derivatives (0 to compute just the xyz point)
 @param theta => angle for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDerivativeArray

(
DEllipse3dCP pEllipse,
DPoint3dP pX,
int         numDerivative,
double      theta
)
    {
    double cosTheta, sinTheta;
    DPoint3d vector;
    int i;

    cosTheta = cos(theta);
    sinTheta = sin(theta);

    if (pX || numDerivative >= 2)
        {
        bsiDPoint3d_add2ScaledDPoint3d (
                &vector,
                NULL,
                &pEllipse->vector0, cosTheta,
                &pEllipse->vector90, sinTheta
                );

        if (pX)
            bsiDPoint3d_addDPoint3dDPoint3d (pX, &pEllipse->center, &vector);

        for (i = 2; i <= numDerivative ; i += 2)
            {
            bsiDPoint3d_negateInPlace (&vector);
            pX[i] = vector;
            }
        }

    if (numDerivative >= 1)
        {
        bsiDPoint3d_add2ScaledDPoint3d
                (
                &vector,
                NULL,
                &pEllipse->vector0,  -sinTheta,
                &pEllipse->vector90,  cosTheta
                );
        pX[1] = vector;

        for (i = 3; i <= numDerivative ; i += 2 )
            {
            bsiDPoint3d_negateInPlace (&vector);
            pX[i] = vector;
            }
        }

    }


/*-----------------------------------------------------------------*//**
@description Convert a fractional parameter to ellipse parameterization angle.
 @param pEllipse => ellipse to evaluate
 @param fraction => fraction of angular range
 @return angular parameter
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse3d_fractionToAngle

(
DEllipse3dCP pEllipse,
double      fraction
)
    {
    return pEllipse->start + fraction * pEllipse->sweep;
    }



/*-----------------------------------------------------------------*//**
@description Compute the determinant of the Jacobian matrix for the transformation from local coordinates (cosine, sine) to global xy-coordinates.
 @param pEllipse => ellipse to query
 @return determinant of Jacobian.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse3d_determinantJXY

(
DEllipse3dCP pEllipse
)
    {
    return
          pEllipse->vector0.x * pEllipse->vector90.y
        - pEllipse->vector0.y * pEllipse->vector90.x;
    }


/*-----------------------------------------------------------------*//**
@description Get the coordinate frame for an ellipse.  X,Y axes are at 0 and 90 degrees.
 Z axis is perpendicular with magnitude equal to the geometric mean of the other two.
 @param pEllipse    => ellipse whose frame is computed.
 @param pFrame      <= transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
 @param pInverse    <= inverse of frame.  May be NULL.
 @return true if the requested frames were returned.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getLocalFrameTransform
(
DEllipse3dCP pEllipse,
TransformP pFrame,
TransformP pInverse
)
    {
    Transform frame;
    bool    myStat = true;
    DPoint3d zVector;
    bsiDPoint3d_geometricMeanCrossProduct
            (&zVector, &pEllipse->vector0, &pEllipse->vector90);

    bsiTransform_initFromOriginAndVectors (
                &frame,
                &pEllipse->center,
                &pEllipse->vector0,
                &pEllipse->vector90,
                &zVector
                );

    if (pFrame)
        *pFrame = frame;

    if (pInverse)
        myStat = bsiTransform_invertTransform (pInverse, &frame);

    return myStat;
    }




/*-----------------------------------------------------------------*//**
@description Get the coordinate frame for an ellipse.  X,Y axes are at 0 and 90 degrees.
 Z axis is perpendicular with magnitude equal to the geometric mean of the other two.
 @param pEllipse    => ellipse whose frame is computed.
 @param pFrame      <= transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
 @param pInverse    <= inverse of frame.  May be NULL.
 @return true if the requested frames were returned.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getLocalFrameAsTransform

(
DEllipse3dCP pEllipse,
TransformP   pFrame,
TransformP   pInverse
)
    {
    Transform frame;
    bool    myStat = true;
    DPoint3d zVector;
    bsiDPoint3d_geometricMeanCrossProduct
            (&zVector, &pEllipse->vector0, &pEllipse->vector90);

    bsiTransform_initFromOriginAndVectors (
                &frame,
                &pEllipse->center,
                &pEllipse->vector0,
                &pEllipse->vector90,
                &zVector
                );

    if (pFrame)
        *pFrame = frame;

    if (pInverse)
        myStat = bsiTransform_invertTransform (pInverse, &frame);

    return myStat;
    }



/*-----------------------------------------------------------------*//**
@description Compute the local coordinates of a point in the skewed coordinates of the ellipse, using
 only xy parts of both the ellipse and starting point.
@remarks This is equivalent to computing the intersection of the ellipse plane with a line through the point and
 parallel to the z axis, and returning the coordinates of the intersection relative to the
 skewed axes of the ellipse.
 @param pEllipse => ellipse to evaluate
 @param pLocalPoint <= evaluated point.  Coordinates x,y are multipliers for the ellipse axes.
                        Coordinate z is height of the initial point from the plane of the ellipse.
 @param pPoint => point to convert to local coordinates
 @return true if ellipse axes are independent.
 @group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_pointToXYLocal

(
DEllipse3dCP pEllipse,
DPoint3dP pLocalPoint,
DPoint3dCP pPoint
)
    {
    Transform frame, inverse;
    bool    myStat;
    myStat = bsiDEllipse3d_getXYLocalFrameAsTransform (pEllipse, &frame, &inverse);
    if (myStat)
        {
        bsiTransform_multiplyDPoint3dArray (&inverse, pLocalPoint, pPoint, 1);
        }
    return myStat;
}


/*-----------------------------------------------------------------*//**
@description Compute the angular position of the point relative to the ellipse's local coordinates.
@remarks If the point is on the ellipse, this is the inverse of evaluating the ellipse at the angle.
 @param pEllipse => ellipse definining angular space
 @param pPoint => point to evaluate
 @return angle in ellipse parameterization
 @group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDEllipse3d_pointToAngle

(
DEllipse3dCP pEllipse,
DPoint3dCP pPoint
)
    {
    Transform inverse;
    DPoint3d localPoint;
    double theta = 0.0;

    if (bsiDEllipse3d_getLocalFrameAsTransform (pEllipse, NULL, &inverse))
        {
        bsiTransform_multiplyDPoint3dArray (&inverse, &localPoint, pPoint, 1);
        theta = bsiTrig_atan2 (localPoint.y, localPoint.x);
        }
    return theta;
}


/*-----------------------------------------------------------------*//**
@description Project a point onto the plane of the ellipse.

 @param pEllipse => ellipse whose axes become 3d plane directions.
 @param pXYZNear <= projection of point onto ellipse plane
 @param pCoff0 <= coefficient on vector towards 0 degree point
 @param pCoff90 <= coefficient on vector towards 90 degree point
 @param pXYZ => point to project onto plane
 @return true if the plane is well defined.
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_projectPointToPlane

(
DEllipse3dCP pEllipse,
DPoint3dP pXYZNear,
double      *pCoff0,
double      *pCoff90,
DPoint3dCP pXYZ
)
    {
    return bsiGeom_closestPointOnSkewedVectors
                        (
                        pXYZNear,
                        pCoff0,
                        pCoff90,
                        &pEllipse->center,
                        &pEllipse->vector0,
                        &pEllipse->vector90,
                        pXYZ
                        );
    }


/*-----------------------------------------------------------------*//**
@description Compute an estimated number of points needed to stroke a full ellipse to within the given chord height tolerance.
@param pEllipse => ellipse to be stroked
 @param n => default number of points on full ellipse (used if tol <= 0.0)
 @param nMax => max number of points on full ellipse (minimum is 4)
 @param tol => tolerance for stroking
 @return number of strokes required on the full ellipse
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_getStrokeCount

(
DEllipse3dCP pEllipse,
int             n,
int             nMax,
double          tol
)

    {
    double r0, r90, rmax;

    if (tol > 0.0)
        {
        r0 = bsiDPoint3d_dotProduct (&pEllipse->vector0, &pEllipse->vector0);
        r90 = bsiDPoint3d_dotProduct (&pEllipse->vector90, &pEllipse->vector90);

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
            n = (int) (0.9999999 + msGeomConst_pi / acos (1.0 - tol / rmax));
            }
        else
            {
            n = 4;
            }
        }

    // restrict return value to [4,nMax]
    if (n < 4)
        {
        n = 4;
        }
    else if (n > nMax)
        {
        n = nMax;
        }

    return n;
    }



/*-----------------------------------------------------------------*//**
@description Evaluate an ellipse using given coefficients for the axes.
@remarks If the x,y components of the coefficients define a unit vector, the point is "on" the ellipse.
 @param pEllipse => ellipse to evaluate
 @param pPoint <= array of cartesian points
 @param pTrig => array of local coords (e.g., (cos, sin)).
 @param numPoint => number of pairs
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateTrigPairs

(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
)
    {
    int i;
    for (i = 0; i < numPoint; i++)
        {
        bsiDPoint3d_add2ScaledDPoint3d (&pPoint[i], &pEllipse->center,
                                &pEllipse->vector0, pTrig[i].x,
                                &pEllipse->vector90, pTrig[i].y);
        }
    return;
    }


/*-----------------------------------------------------------------*//**
@description Evaluate an ellipse at a number of (cosine, sine) pairs, removing
 pairs whose corresponding angle is not in range.

 @param pEllipse => ellipse to evaluate
 @param pPoint <= array of cartesian points
 @param pTrig => array of local coords
 @param numPoint => number of pairs
 @return number of points found to be in the angular range of the ellipse.
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_testAndEvaluateTrigPairs

(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
)
    {
    int n = 0;
    int i;

    for (i = 0; i < numPoint; i++)
        {
        double theta = bsiTrig_atan2 (pTrig[i].y, pTrig[i].x);
        if (bsiTrig_angleInSweep (theta, pEllipse->start, pEllipse->sweep))
            {
            bsiDEllipse3d_evaluateTrigPairs (pEllipse, &pPoint[n++], &pTrig[i], 1);
            }
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
@description Test if a specified angle is within the sweep of the ellipse.
 @param pEllipse => ellipse whose angular range is queried
 @param angle => angle (radians) to test
 @return true if angle is within the sweep angle of the elliptical arc.
@group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_angleInSweep

(
DEllipse3dCP pEllipse,
double      angle
)
    {
    return bsiTrig_angleInSweep (angle, pEllipse->start, pEllipse->sweep);
    }


/*-----------------------------------------------------------------*//**
@description Convert an angular parameter to a fraction of bounded arc length.
 @param pEllipse => ellipse whose angular range is queried
 @param angle      => angle (radians) to convert
 @return fractional parameter
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiDEllipse3d_angleToFraction

(
DEllipse3dCP pEllipse,
double      angle
)
    {
    return bsiTrig_normalizeAngleToSweep (angle, pEllipse->start, pEllipse->sweep);
    }


/*-----------------------------------------------------------------*//**
@description Get the start and end angles of the ellipse.
 @param pEllipse => ellipse whose angular range is queried
 @param pStartAngle <= start angle
 @param pEndAngle <= end angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getLimits

(
DEllipse3dCP pEllipse,
double    *pStartAngle,
double    *pEndAngle
)
    {
    if (pStartAngle)
        *pStartAngle = pEllipse->start;
    if (pEndAngle)
        *pEndAngle   = pEllipse->start + pEllipse->sweep;
    }


/*-----------------------------------------------------------------*//**
@description Get the start and sweep angles of the ellipse.
 @param pEllipse => ellipse whose angular range is queried.
 @param pStartAngle <= start angle
 @param pSweep <= sweep angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getSweep

(
DEllipse3dCP pEllipse,
double    *pStartAngle,
double    *pSweep
)
    {
    if (pStartAngle)
        *pStartAngle = pEllipse->start;
    if (pSweep)
        *pSweep      = pEllipse->sweep;
    }


/*-----------------------------------------------------------------*//**
@description Set the start and end angles of the ellipse.
 @param pEllipse <= ellipse whose angular range is changed
 @param startAngle => start angle
 @param endAngle   => end angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse3d_setLimits

(
DEllipse3dP pEllipse,
double    startAngle,
double    endAngle
)
    {
    pEllipse->start = startAngle;
    pEllipse->sweep = endAngle - startAngle;
    }


/*-----------------------------------------------------------------*//**
@description Set the start and sweep angles of the ellipse.
 @param pEllipse <= ellipse whose angular range is changed
 @param startAngle => start angle
 @param sweep      => sweep angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse3d_setSweep

(
DEllipse3dP pEllipse,
double    startAngle,
double    sweep
)
    {
    pEllipse->start = startAngle;
    pEllipse->sweep = sweep;
    }


/*-----------------------------------------------------------------*//**
@description Make a copy of the source ellipse, altering the axis vectors and angular limits so that
 the revised ellipse has perpendicular axes in the conventional major/minor axis form.
@remarks Inputs may be the same.
 @param pEllipse <= ellipse with perpendicular axes
 @param pSource => ellipse with unconstrained axes
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initWithPerpendicularAxes

(
DEllipse3dP pEllipse,
DEllipse3dCP pSource
)

    {
    double   dotUV, dotUU, dotVV;
    double   c, s, theta;
    double   ax, ay, a, tol;
    DPoint3d vector0, vector90;

    dotUV = bsiDPoint3d_dotProduct(&pSource->vector0,  &pSource->vector90);
    dotUU = bsiDPoint3d_dotProduct(&pSource->vector0,  &pSource->vector0 );
    dotVV = bsiDPoint3d_dotProduct(&pSource->vector90, &pSource->vector90);

    ay = dotUU - dotVV;
    ax = 2.0 * dotUV;
    a = dotUU + dotVV;
    tol = bsiTrig_smallAngle () * a;
    if (fabs (ax) < tol)
        {
        *pEllipse = *pSource;
        }
    else
        {
        bsiTrig_halfAngleFunctions (&c, &s, ay, ax);
        bsiTrig_rotate90UntilSmallAngle (&c, &s, c, s);
        *pEllipse = *pSource;
        /* Save the given axes in locals because the originals will be overwritten
            if the same ellipse is being used for input and output. */
        vector0 = pSource->vector0;
        vector90 = pSource->vector90;
        bsiDPoint3d_add2ScaledDPoint3d (&pEllipse->vector0, NULL,
                                    &vector0,   c,
                                    &vector90,  s);
        bsiDPoint3d_add2ScaledDPoint3d (&pEllipse->vector90, NULL,
                                    &vector0,  -s,
                                    &vector90,  c);
        theta = bsiTrig_atan2 (s,c);
        pEllipse->start -= theta;
        }
    }


/*-----------------------------------------------------------------*//**
@description Compute the range box of the ellipse in its major-minor axis coordinate system.
 Compute line segments that are the horizontal and vertical midlines in that system.
 Return those line segments ordered with the longest first, and return the shorter length.

@remarks The typical use of this is that if the shorter length is less than some tolerance the
 points swept out by the ellipse are the longer segment.  (But beware that the start and
 end points of the segment can be other than the start and end points of the ellipse.)

 @param pEllipse   => ellipse to analyze
 @param pLongSegment  <=   longer axis of local conic range box
 @param pShortSegment <=   shorter axis of local conic range box
 @return size of the shorter dimension
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double           bsiDEllipse3d_getMajorMinorRangeMidlines

(
DEllipse3dCP pEllipse,
DSegment3dP pLongSegment,
DSegment3dP pShortSegment
)

    {
    DEllipse3d majorMinorEllipse;
    DRange2d localRange;
    DSegment3d segment[2];
    double xBar, yBar;
    int iLong, iShort;
    double lengthSquared[2];

    bsiDEllipse3d_initWithPerpendicularAxes (&majorMinorEllipse, pEllipse);
    bsiDEllipse3d_getLocalRange (&majorMinorEllipse, &localRange);

    xBar = 0.5 * (localRange.low.x + localRange.high.x);
    yBar = 0.5 * (localRange.low.y + localRange.high.y);

    bsiDEllipse3d_evaluateDPoint3dFromLocal (&majorMinorEllipse, &segment[0].point[0], localRange.low.x, yBar);
    bsiDEllipse3d_evaluateDPoint3dFromLocal (&majorMinorEllipse, &segment[0].point[1], localRange.high.x, yBar);

    bsiDEllipse3d_evaluateDPoint3dFromLocal (&majorMinorEllipse, &segment[1].point[0], xBar, localRange.low.y);
    bsiDEllipse3d_evaluateDPoint3dFromLocal (&majorMinorEllipse, &segment[1].point[1], xBar, localRange.high.y);

    lengthSquared[0] = segment[0].LengthSquared ();//bsiDSegment3d_lengthSquared (&segment[0]);
    lengthSquared[1] = segment[1].LengthSquared ();//bsiDSegment3d_lengthSquared (&segment[1]);

    if (lengthSquared[0] >= lengthSquared[1])
        {
        iLong = 0;
        iShort = 1;
        }
    else
        {
        iLong = 1;
        iShort = 0;
        }

    if (pLongSegment)
        *pLongSegment = segment[iLong];

    if (pShortSegment)
        *pShortSegment = segment[iShort];

    return sqrt (lengthSquared[iShort]);
    }


/*-----------------------------------------------------------------*//**
@description Make a copy of the source ellipse, reversing the start and end angles.
@remarks Inputs may be the same.
 @param pEllipse   <= copied and reversed ellipse
 @param pSource => source ellipse
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initReversed

(
DEllipse3dP pEllipse,
DEllipse3dCP pSource
)
    {
    if (pEllipse != pSource)
        *pEllipse = *pSource;
    pEllipse->start = pEllipse->start + pEllipse->sweep;
    pEllipse->sweep = (-pEllipse->sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the magnitude of the tangent vector to the ellipse at the specified angle.
 @param pEllipse => ellipse to evaluate
 @param theta => angular parameter
 @return tangent magnitude
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse3d_tangentMagnitude

(
DEllipse3dCP pEllipse,
double      theta
)
    {
    DPoint3d tangent;
    double s, c;

    s =  sin(theta);
    c =  cos(theta);

    bsiDPoint3d_add2ScaledDPoint3d (&tangent,
                NULL,
                &pEllipse->vector0, -s,
                &pEllipse->vector90, c);

    return bsiDPoint3d_magnitude (&tangent);
    }

typedef struct
    {
    const DPoint4d *pPoleArray;
    int   order;
    int   count;
    } ArcLengthParams;

/*---------------------------------------------------------------------------------**//**
@description Compute the arc length integrand at given parameter.
 @bsimethod                                                     EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    cbArcLengthIntegrand

(
double  *pTangentMagnitude,
double  theta,
void    *vpEllipse
)
    {
    DEllipse3d  *pEllipse = (DEllipse3d *)(vpEllipse);

    *pTangentMagnitude = bsiDEllipse3d_tangentMagnitude (pEllipse, theta);
    return true;
    }

static double s_arcLengthRelTol = 1.0e-12;



/*---------------------------------------------------------------------------------**//**
@description Return arc length of ellipse.
@param pEllipse IN ellipse to integrate
@return arc length of ellipse.
@group "DEllipse3d Queries"
@bsimethod                                    Earlin.Lutz                     05/2006
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse3d_arcLength

(
DEllipse3dCP pEllipse
)
    {
    DEllipse3d ellipseMM;
    double a, b;
    bsiDEllipse3d_initMajorMinor (&ellipseMM, pEllipse);
    a = bsiDPoint3d_magnitude (&ellipseMM.vector0);
    b = bsiDPoint3d_magnitude (&ellipseMM.vector90);
    return bsiGeom_ellipseArcLength (a, b, ellipseMM.start, ellipseMM.sweep);
    }

/*-----------------------------------------------------------------*//**
@description Compute the signed arc length of the ellipse.
 Negative sweep produces negative arc length, so the return from this
 can be negative.
@param pEllipse IN ellipse.
 @return arc length of ellipse
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse3d_arcLengthAdaptive

(
DEllipse3dCP pEllipse
)

    {
    double length, error;
    DEllipse3d workEllipse = *pEllipse;
    int     count;
    double theta0 = pEllipse->start;
    double theta1 = pEllipse->start + pEllipse->sweep;
    double resultSign = 1.0;
    if (theta1 < theta0)
        {
        theta0 = theta1;
        theta1 = pEllipse->start;
        resultSign = -1.0;
        }
    bsiMath_recursiveNewtonCotes5 (&length, &error, &count,
                        theta0, theta1,
                        0.0, s_arcLengthRelTol,
                        cbArcLengthIntegrand, &workEllipse);

    bsiMath_recursiveNewtonCotesIntegration
                        (
                        &length,
                        &error,
                        &count,
                        &workEllipse,
                        cbArcLengthIntegrand,
                        5,
                        theta0, theta1,
                        theta0, theta1,
                        0.0, s_arcLengthRelTol
                        );
    return resultSign * length;
    /*
    return bsiIntegral_gauss (
                    (OmdlScalarFunction)bsiDEllipse3d_tangentMagnitude,
                    pEllipse->start,
                    pEllipse->start + pEllipse->sweep,
                    msGeomConst_piOver2,
                    4,
                    (void *)pEllipse);
    */
    }


/*---------------------------------------------------------------------------------**//**
@description Return the sweep angle corresponding to an arc length.
@remarks Negative returned sweep angle corresponds to arclength traversed in the opposite direction of the ellipse sweep.
@param pEllipse IN ellipse to integrate
@param arcLength IN arc length to invert
@return sweep angle
@group "DEllipse3d Queries"
@bsimethod                                    Earlin.Lutz                     06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse3d_inverseArcLength

(
DEllipse3dCP pEllipse,
double arcLength
)
    {
    DEllipse3d  ellipseMM;
    double      a, b, startRadians, sweepScale = 1.0;

    bsiDEllipse3d_initMajorMinor (&ellipseMM, pEllipse);
    a = bsiDPoint3d_magnitude (&ellipseMM.vector0);
    b = bsiDPoint3d_magnitude (&ellipseMM.vector90);
    startRadians = ellipseMM.start;

    if (arcLength < 0.0)
        {
        sweepScale = -1.0;
        arcLength = -arcLength;
        startRadians = -startRadians;
        }

    if (pEllipse->sweep < 0.0)
        {
        startRadians = -startRadians;
        }

    return sweepScale * bsiGeom_ellipseInverseArcLength (a, b, startRadians, arcLength);
    }


/*---------------------------------------------------------------------------------**//**
@description Compute the (signed) arc length between specified fractional parameters.
@remarks Fractions outside [0,1] return error.
 @param pEllipse => ellipse to measure.
 @param pArcLength <= computed arc length.  Negative if fraction1 < fraction0.
 @param fraction0 => start fraction for interval to measure
 @param fraction1 => end fraction for interval to measure
 @return true if the arc length was computed.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_fractionToLength

(
DEllipse3dCP pEllipse,
double      *pArcLength,
double      fraction0,
double      fraction1
)
    {
    DEllipse3d workEllipse = *pEllipse;
    double theta0, theta1;
    double error;
    int count;
    double s;
    if (fraction1 > fraction0)
        {
        theta0 = bsiDEllipse3d_fractionToAngle (pEllipse, fraction0);
        theta1 = bsiDEllipse3d_fractionToAngle (pEllipse, fraction1);
        s = 1.0;
        }
    else
        {
        theta0 = bsiDEllipse3d_fractionToAngle (pEllipse, fraction1);
        theta1 = bsiDEllipse3d_fractionToAngle (pEllipse, fraction0);
        s = -1.0;
        }
    bool    stat = bsiMath_recursiveNewtonCotesIntegration
                        (
                        pArcLength,
                        &error,
                        &count,
                        &workEllipse,
                        cbArcLengthIntegrand,
                        5,
                        pEllipse->start, pEllipse->start + pEllipse->sweep,
                        theta0, theta1,
                        0.0, s_arcLengthRelTol
                        );
    *pArcLength = fabs (*pArcLength) * s;
    return stat;
    }


/*-----------------------------------------------------------------*//**
@description Compute the xyz range limits of a 3D ellipse.
 @param pEllipse => ellipse whose range is determined
 @param pRange <= computed range
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_getRange

(
DEllipse3dCP pEllipse,
DRange3dP pRange
)
    {
    DPoint3d startPoint, endPoint;
    double theta0 = pEllipse->start;
    double sweep  = pEllipse->sweep;

    if (!bsiTrig_isAngleFullCircle (pEllipse->sweep))
        {
        bsiDEllipse3d_evaluateDPoint3d (pEllipse, &startPoint, theta0        );
        bsiDEllipse3d_evaluateDPoint3d (pEllipse, &endPoint,   theta0 + sweep);

        pRange->low = pRange->high = startPoint;

        FIX_MIN(  endPoint.x, pRange->low.x  );
        FIX_MIN(  endPoint.y, pRange->low.y  );
        FIX_MIN(  endPoint.z, pRange->low.z  );

        FIX_MAX(  endPoint.x, pRange->high.x );
        FIX_MAX(  endPoint.y, pRange->high.y );
        FIX_MAX(  endPoint.z, pRange->high.z );
        }
    else
        {
        pRange->low = pRange->high = pEllipse->center;
        }

    bsiEllipse_componentRange ( &pRange->low.x, &pRange->high.x,
                        pEllipse->center.x, pEllipse->vector0.x, pEllipse->vector90.x,
                        theta0, sweep);

    bsiEllipse_componentRange ( &pRange->low.y, &pRange->high.y,
                        pEllipse->center.y, pEllipse->vector0.y, pEllipse->vector90.y,
                        theta0, sweep);

    bsiEllipse_componentRange ( &pRange->low.z, &pRange->high.z,
                        pEllipse->center.z, pEllipse->vector0.z, pEllipse->vector90.z,
                        theta0, sweep);

    }


/*-----------------------------------------------------------------*//**
@description Compute the range of the ellipse in its own coordinate system..
@remarks This depends on the start and sweep angles but not the center or axis coordinates.
 @param pEllipse => ellipse whose range is determined
 @param pRange <= computed range
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_getLocalRange

(
DEllipse3dCP pEllipse,
DRange2dP pRange
)
    {
    DRange3d xyzRange;
    bsiDRange3d_initFromUnitArcSweep (&xyzRange, pEllipse->start, pEllipse->sweep);
    pRange->low.x = xyzRange.low.x;
    pRange->low.y = xyzRange.low.y;
    pRange->high.x = xyzRange.high.x;
    pRange->high.y = xyzRange.high.y;
    }


/*-----------------------------------------------------------------*//**
@description Find intersections of a (full) ellipse with a plane.
@remarks Return value n=1 is a single tangency point returned in pTrigPoints[0];
        n=2 is two simple intersections returned in pTrigPoints[0..1]
@remarks The three component values in pTrigPoints are:
<UL>
<LI>x == cosine of angle
<LI>y == sine of angle
<LI>z == angle in radians
</UL>
 @param pEllipse      => ellipse to intersect with plane
 @param pTrigPoints    <= 2 points: cosine, sine, theta values of plane intersection
 @param pPlane          => homogeneous plane equation
 @return The number of intersections, i.e. 0, 1, or 2
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectPlane

(
DEllipse3dCP pEllipse,
DPoint3dP pTrigPoints,
DPoint4dCP pPlane
)
    {
    double alpha = bsiDPoint4d_dotWeightedPoint ( pPlane, &pEllipse->center,   1.0);
    double beta  = bsiDPoint4d_dotWeightedPoint ( pPlane, &pEllipse->vector0, 0.0);
    double gamma = bsiDPoint4d_dotWeightedPoint ( pPlane, &pEllipse->vector90, 0.0);

    int n = bsiMath_solveApproximateUnitQuadratic (
                &pTrigPoints[0].x, &pTrigPoints[0].y,
                &pTrigPoints[1].x, &pTrigPoints[1].y,
                alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance );
    if ( n == 1 )
        {
        /* Take the on-circle solution */
        pTrigPoints[0].x = pTrigPoints[1].x;
        pTrigPoints[0].y = pTrigPoints[1].y;
        pTrigPoints[0].z = bsiTrig_atan2 ( pTrigPoints[0].y, pTrigPoints[0].x );
        }
    else if ( n == 2 )
        {
        pTrigPoints[0].z = bsiTrig_atan2 ( pTrigPoints[0].y, pTrigPoints[0].x );
        pTrigPoints[1].z = bsiTrig_atan2 ( pTrigPoints[1].y, pTrigPoints[1].x );
        }
    else
        {
        /* suppress the degenerate cases */
        n = 0;
        }
    return n;
    }



/*-----------------------------------------------------------------*//**
@description Test if the ellipse is circular.
@param pEllipse IN ellipse to test
@return true if circular
@group "DEllipse3d Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_isCircular

(
DEllipse3dCP pEllipse
)

    {
    double relTol = bsiTrig_smallAngle();
    double  dotUV, magU, magV;

    dotUV = bsiDPoint3d_dotProduct (&pEllipse->vector0, &pEllipse->vector90);
    magU = bsiDPoint3d_magnitude (&pEllipse->vector0);
    magV = bsiDPoint3d_magnitude (&pEllipse->vector90);

    /* Circular if the axes have the same magnitude and are perpendicular */
    if (fabs (magU - magV) < (magU + magV) * relTol)
        {
        if (fabs (dotUV) < magU * magV * relTol)
            return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
@description Test if the XY projection of the ellipse is circular.
@param pEllipse IN ellipse to test
@return true if circular
 @group "DEllipse3d Queries"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_isCircularXY

(
DEllipse3dCP pEllipse
)
    {
    double uu = bsiDPoint3d_dotProductXY (&pEllipse->vector0, &pEllipse->vector0);
    double uv = bsiDPoint3d_dotProductXY (&pEllipse->vector90, &pEllipse->vector0);
    double vv = bsiDPoint3d_dotProductXY (&pEllipse->vector90, &pEllipse->vector90);

    static double s_reltol = 1.0e-12;

    return    fabs (uv) < s_reltol * uu
            && fabs (uu - vv) < s_reltol * uu;
    }

int intersectXYEllipseFrames(DPoint3dP pCartesianPoints, DPoint3dP pEllipse0Params, DPoint3dP pEllipse1Params, TransformCP pFrame0, TransformCP pFrame1);


/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of two ellipses.
@remarks May return 0, 1, 2, 3 or 4 points.  Both ellipses are unbounded.
 @param pEllipse0      => ellipse to intersect with line.
 @param pCartesianPoints   <= cartesian intersection points.
 @param pEllipse0Params    <= array of coordinates relative to the first ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pEllipse1Params    <= array of coordinates relative to the second ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pEllipse1       => the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectXYDEllipse3d

(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Params,
DPoint3dP pEllipse1Params,
DEllipse3dCP pEllipse1
)
    {
    Transform       ellipse0Frame,  ellipse1Frame;
    RotMatrix       ellipse0Matrix, ellipse1Matrix;
    double ellipse0Condition, ellipse1Condition;
    int numUnbounded = 0;

    /* Condition number from dmatrix3d tells about skewing.
       We multiply by SIZE factor so that we favor using
       smaller ellipse as reference frame.
    */
    bsiDEllipse3d_getXYLocalFrameAsTransform (pEllipse0, &ellipse0Frame, NULL);
    ellipse0Matrix.InitFrom (ellipse0Frame);
    ellipse0Condition = bsiRotMatrix_conditionNumber (&ellipse0Matrix);
    ellipse0Condition *= sqrt (fabs (
            DVec3d::FromMatrixColumn (ellipse0Frame, 0).CrossProductXY (DVec3d::FromMatrixColumn (ellipse0Frame, 0))));

    bsiDEllipse3d_getXYLocalFrameAsTransform (pEllipse1, &ellipse1Frame, NULL);
    ellipse1Matrix.InitFrom (ellipse1Frame);
    ellipse1Condition = bsiRotMatrix_conditionNumber (&ellipse1Matrix);
    ellipse1Condition *= sqrt (fabs (
            DVec3d::FromMatrixColumn (ellipse1Frame, 0).CrossProductXY (DVec3d::FromMatrixColumn (ellipse1Frame, 0))));

    if (ellipse0Condition > ellipse1Condition)
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pEllipse0Params, pEllipse1Params,
                                &ellipse0Frame, &ellipse1Frame);
        }
    else
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pEllipse1Params,   pEllipse0Params,
                                &ellipse1Frame,    &ellipse0Frame);
        }
    return numUnbounded;
    }

int filterDualSweeps(DPoint3dP pCartesianPoints, DPoint3dP pEllipse0Coffs, double *pEllipse0Angle, DPoint3dP pEllipse1Coffs, double *pEllipse1Angle, DEllipse3dCP pEllipse0, DEllipse3dCP pEllipse1, DPoint3dCP pCartesianInPoint, DPoint3dCP pEllipse0InCoffs, DPoint3dCP pEllipse1InCoffs, int numUnBounded);



/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of two ellipses, with bounds applied.
@remarks May return 0, 1, 2, 3 or 4 points.
 @param pEllipse0      => ellipse to intersect with line.
 @param pCartesianPoints   <= cartesian intersection points.
 @param pEllipse0Coffs    <= array of coordinates relative to the first ellipse
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pEllipse0Angle <= array of angles on the first ellipse
 @param pEllipse1Coffs     <= array of coordinates relative to the second ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pEllipse1Angle <= array of angles on the other ellipse
 @param pEllipse1       => the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectXYDEllipse3dBounded

(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Coffs,
double        *pEllipse0Angle,
DPoint3dP pEllipse1Coffs,
double        *pEllipse1Angle,
DEllipse3dCP pEllipse1
)
    {
    DPoint3d cartesianPoint[4], ellipse0Coffs[4], ellipse1Coffs[4];
    int numUnBounded;
    int numBounded;

    numUnBounded = bsiDEllipse3d_intersectXYDEllipse3d (pEllipse0,
                            cartesianPoint, ellipse0Coffs, ellipse1Coffs, pEllipse1);

    numBounded = filterDualSweeps (
                        pCartesianPoints,
                        pEllipse0Coffs, pEllipse0Angle,
                        pEllipse1Coffs, pEllipse1Angle,
                        pEllipse0,
                        pEllipse1,
                        cartesianPoint, ellipse0Coffs, ellipse1Coffs, numUnBounded);
    return numBounded;
    }


/*-----------------------------------------------------------------*//**
@description Find "intersections" of two DEllipse3d.
@remarks Ellipses in space can pass very close to
 each other without intersecting, so some logic must be applied to define intersection
 more cleanly. The logic applied is to choose the more circular ellipse and apply the
 transformation which makes that one a unit circle, then intersect the xy projections of the
 transformations.

 @param pEllipse0           => ellipse to intersect with line.
 @param pCartesianPoints    <= cartesian intersection points.
 @param pEllipse0Params     <= array of coordinates relative to the first ellipse
                                For each point, (xy) are the cosine and sine of the
                                ellipse parameter, (z) is z distance from the plane of
                                of the ellipse.
 @param pEllipse1Params      <= array of coordinates relative to the second ellipse.
                                For each point, (xy) are the cosine and sine of the
                                ellipse parameter, (z) is z distance from the plane of
                                of the ellipse.
 @param pEllipse1            => the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDEllipse3d

(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Params,
DPoint3dP pEllipse1Params,
DEllipse3dCP pEllipse1
)
    {
    Transform ellipse0Frame, ellipse1Frame;
    double ellipse0Condition, ellipse1Condition;
    int numUnbounded = 0;

    bsiDEllipse3d_getLocalFrameAsTransform (pEllipse0, &ellipse0Frame, NULL);
    ellipse0Condition = RotMatrix::From (ellipse0Frame).ConditionNumber ();

    bsiDEllipse3d_getLocalFrameAsTransform (pEllipse1, &ellipse1Frame, NULL);
    ellipse1Condition = RotMatrix::From (ellipse1Frame).ConditionNumber ();

    if (ellipse0Condition > ellipse1Condition)
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pEllipse0Params, pEllipse1Params,
                                &ellipse0Frame, &ellipse1Frame);
        }
    else
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pEllipse1Params,   pEllipse0Params,
                                &ellipse1Frame,    &ellipse0Frame);
        }
    return numUnbounded;
    }


/*-----------------------------------------------------------------*//**
@description Intersect two ellipses as described in ~mbsiDEllipse3d_intersectSweptDEllipse3d, and
 filter out results not part of both ranges.

 @param pEllipse0      => ellipse to intersect with cylinder of second ellipse.
 @param pCartesianPoints   <= cartesian intersection points.
 @param pEllipse0Coffs    <= array of coordinates relative to the first ellipse
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pEllipse0Angle    <= array of angles on the first ellipse.
 @param pEllipse1Coffs     <= array of coordinates relative to the second ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pEllipse1Angle   <= array of angles on the other ellipse.
 @param pEllipse1       => the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDEllipse3dBounded

(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Coffs,
double        *pEllipse0Angle,
DPoint3dP pEllipse1Coffs,
double        *pEllipse1Angle,
DEllipse3dCP pEllipse1
)
    {
    DPoint3d cartesianPoint[4], ellipse0Coffs[4], ellipse1Coffs[4];
    int numUnBounded;
    int numBounded;

    numUnBounded = bsiDEllipse3d_intersectSweptDEllipse3d (pEllipse0,
                            cartesianPoint, ellipse0Coffs, ellipse1Coffs, pEllipse1);

    numBounded = filterDualSweeps (
                                    pCartesianPoints,
                                    pEllipse0Coffs, pEllipse0Angle,
                                    pEllipse1Coffs, pEllipse1Angle,
                                    pEllipse0, pEllipse1,
                                    cartesianPoint,
                                    ellipse0Coffs,
                                    ellipse1Coffs,
                                    numUnBounded);
    return numBounded;
    }



/*-----------------------------------------------------------------*//**
@description Find the closest point on a complete ellipse or its contained disk.
 @bsimethod                                                     EarlinLutz      12/97
@param pEllipse IN ellipse to search
@param pLocalX OUT local x coordinate.
@param pLocalY OUT local y coordinate.
@param pMinDistSquared OUT squared distance to closest point
@param pMinPoint OUT closest point
@param pSpacePoint IN space point
@return always true
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_closestPointOnDisk

(
DEllipse3dCP pEllipse,
double        *pLocalX,
double        *pLocalY,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pSpacePoint
)
    {
    double u, v;
    DPoint3d planePoint;
    // projection to plane can fail for colinear axes ...
    if (bsiGeom_closestPointOnSkewedVectors
                        (
                        &planePoint,
                        &u, &v,
                        &pEllipse->center,
                        &pEllipse->vector0,
                        &pEllipse->vector90,
                        pSpacePoint
                        )
        && u * u + v * v <= 1.0)
        {
        if (pMinPoint)
            *pMinPoint = planePoint;
        if (pLocalX)
            *pLocalX = u;
        if (pLocalY)
            *pLocalY = v;
        if (pMinDistSquared)
            *pMinDistSquared = pSpacePoint->Distance (planePoint);
        return true;
        }

    // This is always going to return true...
    DEllipse3d fullEllipse = *pEllipse;
    fullEllipse.MakeFullSweep ();
    double theta;
    if (bsiDEllipse3d_closestPointBounded (pEllipse, &theta, pMinDistSquared, pMinPoint, pSpacePoint))
        {
        if (pLocalX)
            *pLocalX = cos (theta);
        if (pLocalY)
            *pLocalY = sin (theta);
        return true;
        }
    return false;
    }




/*-----------------------------------------------------------------*//**
@description Apply a transformation to the source ellipse.
 @param pDest <= transformed ellipse
 @param pTransform => transformation to apply
 @param pSource => source ellipse
@group "DEllipse3d Transform"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse3d_multiplyTransformDEllipse3d

(
DEllipse3dP pDest,
TransformCP pTransform,
DEllipse3dCP pSource
)
    {
    bsiTransform_multiplyDPoint3dArray (pTransform, &pDest->center, &pSource->center, 1);
    bsiTransform_multiplyDPoint3dByMatrixPart (pTransform, &pDest->vector0, &pSource->vector0);
    bsiTransform_multiplyDPoint3dByMatrixPart (pTransform, &pDest->vector90, &pSource->vector90);
    pDest->start = pSource->start;
    pDest->sweep = pSource->sweep;
    }



/*-----------------------------------------------------------------*//**
* @description Gets parameters for a circular arc, to round a polygon's vertex.
*
* @param ellipsePts <=  3 points which define arc (center, 0 degree point = start point, 90 degree point)
* @param pEndPt <=  end point of arc
* @param pSweep <=  sweep angle
* @param pVertexPt => vertex point
* @param pPt1 => preceding vertex or rounding point
* @param pPt2 => succeeding vertex or rounding point
* @param radius => arc radius
* @return true if lines are non-parallel and long enough to allow fillet to be constructed.
* @group "Quadratic Geometry"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_filletPolygonVertex

(
DPoint3d* ellipsePts,
DPoint3d* pEndPt,
double*   pSweep,
const  DPoint3d* pVertexPt,
const  DPoint3d* pPt1,
const  DPoint3d* pPt2,
double    radius
)

    {
    DPoint3d    tanVec1, tanVec2, yVec, zVec;
    double      dist, maxDist1, maxDist2, angle, tangent, sweepAngle;
    const static double minimumAngle = 1e-12;

    maxDist1 = bsiDPoint3d_computeNormal (&tanVec1, pPt1, pVertexPt);
    maxDist2 = bsiDPoint3d_computeNormal (&tanVec2, pPt2, pVertexPt);

    angle = acos (bsiDPoint3d_dotProduct (&tanVec1, &tanVec2));
    sweepAngle = msGeomConst_pi - angle;

    if (angle < minimumAngle || sweepAngle < minimumAngle)
        return false;

    tangent = tan (angle * 0.5);

    if (tangent < minimumAngle || (dist = radius / tangent) > maxDist1 ||
            dist > maxDist2)
        return false;

    bsiDPoint3d_addScaledDPoint3d (&ellipsePts[1], pVertexPt, &tanVec1, dist);
    bsiDPoint3d_addScaledDPoint3d (pEndPt,  pVertexPt, &tanVec2, dist);
    bsiDPoint3d_crossProduct (&zVec, &tanVec1, &tanVec2);
    bsiDPoint3d_crossProduct (&yVec, &zVec, &tanVec1);
    bsiDPoint3d_normalizeInPlace (&yVec);
    bsiDPoint3d_addScaledDPoint3d (&ellipsePts[0], &ellipsePts[1], &yVec, radius);
    bsiDPoint3d_addScaledDPoint3d (&ellipsePts[2], &ellipsePts[0], &tanVec1, -radius);
    *pSweep = sweepAngle;
    return true;
    }


/*-----------------------------------------------------------------*//**
@description Construct a circular fillet arc from 3 points of the unfilleted corner.
 @param pEllipse      <= fillet arc
 @param pFilletStart  <= tangency point on first line.
 @param pFilletEnd    <= tangency point on second line.
 @param pPoint0       => first point of unfilleted chain
 @param pPoint1       => second (middle) point of unfilleted chain
 @param pPoint2       => third point of unfilleted chain
 @param radius      => radius of fillet
 @return true if fillet construted.
@group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool          bsiDEllipse3d_constructFillet

(
DEllipse3dP pEllipse,
DPoint3dP pFilletStart,
DPoint3dP pFilletEnd,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2,
double          radius
)
    {
    DPoint3d ellipsePoint[3];
    DPoint3d endPoint;
    double sweep;
    bool    myStat = bsiGeom_filletPolygonVertex (
                                ellipsePoint,
                                &endPoint,
                                &sweep,
                                pPoint1,
                                pPoint0,
                                pPoint2,
                                radius
                                );
    if (myStat)
        {
        if (pEllipse)
            bsiDEllipse3d_initFrom3dPoints (pEllipse,
                        &ellipsePoint[0], &ellipsePoint[1], &ellipsePoint[2],
                        0.0, sweep);
        if (pFilletStart)
            *pFilletStart = ellipsePoint[1];
        if (pFilletEnd)
            *pFilletEnd = endPoint;
        }
    else
        {
        /* How to initialize error ellipse? */
        if (pEllipse)
            bsiDEllipse3d_initFromCenterNormalRadius (pEllipse, pPoint1, NULL, radius);
        if (pFilletStart)
            *pFilletStart = *pPoint1;
        if (pFilletEnd)
            *pFilletEnd = *pPoint1;
        }
    return myStat;
    }

// START HERE.....................................

#ifdef QUARTIC_BEZIER
/*-----------------------------------------------------------------*//**
@description Return the (weighted) control points of a quartic bezier that
 represents the ellipse.
@remarks A quadratiic bezier can represent any arc of a circle, but
       it cannot wrap and cover the complete range.  The quartic
       can cover the complete circle with a single bezier span.
@param pEllipse => ellipse to query
@param pPoleArray <= array of 5 poles
@group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
void     bsiDEllipse3d_getQuarticBezierPoles

(
DEllipse3dCP pEllipse,
DPoint4dP pPoleArray
)
    {
    DConic4d conic;
    bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);
    bsiDConic4d_getQuarticBezierPoles (&conic, pPoleArray);
    }
#endif


/*-----------------------------------------------------------------*//**
@description Return the (weighted) control points of quadratic beziers which
   combine to represent the full conic section.

 @param pEllipse => ellipse to query
 @param pPoleArray <= array of poles of composite quadratic Bezier curve.  Array count depends on the angular extent.
 @param pCirclePoleArray <= array of corresponding poles which
            map the bezier polynomials back to the unit circle points (x,y,w)
            where x^2 + y^2 = w^2, but beware that w is not identically 1!  (Use this to map from bezier parameter back to angles.)
 @param pNumPole <= number of poles returned
 @param pNumSpan <= number of spans returned.  The pole indices for the first span are (always) 0,1,2; for the second span (if present),
                    2,3,4, and so on.
 @param maxPole => maximum number of poles desired.  maxPole must be at least
                5.  The circle is split into (maxPole - 1) / 2 spans.
                Beware that for 5 poles the circle is split into at most
                two spans, and there may be zero weights.   For 7 or more poles
                all weights can be positive.  The function may return fewer
                poles.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getQuadricBezierPoles

(
DEllipse3dCP pEllipse,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
int       *pNumPole,
int       *pNumSpan,
int       maxPole
)
    {
    DConic4d conic;
    bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);
    bsiDConic4d_getQuadricBezierPoles (&conic, pPoleArray, pCirclePoleArray, pNumPole, pNumSpan, maxPole);
    }


/*-----------------------------------------------------------------*//**
@description Return the (weighted) control points of a quadratic bezier
   for one sector of the arc.  Sector size is arc sweep divided by numSector.
   There are no checks for conventional sector size limits or whether the index
   is within 0..numSector-1.

 @param pEllipse => ellipse to query
 @param pPoleArray <= array of 3 poles.
 @param pCirclePoleArray <= array of 3 poles in unit circle space.
@param sectorIndex => integer index of sector.
@param numSector => number of sectors in overall arc.  The computed sector sweep is the total sweep divided
    by numSector.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getQuadricBezierSectorPoles
(
DEllipse3dCP pEllipse,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
int sector,
int numSector
)
    {
    DConic4d conic;
    bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);
    double sectorSweep = pEllipse->sweep / numSector;
    double theta0 = pEllipse->start + sector * sectorSweep;
    double theta2 = pEllipse->start + (sector + 1) * sectorSweep;
    double theta1 = 0.5 * (theta0 + theta2);
    double cc[3] = {cos(theta0), cos(theta1), cos(theta2)};
    double ss[3] = {sin(theta0), sin(theta1), sin(theta2)};
    double ww[3] = {1.0, cos (0.5 * sectorSweep), 1.0};
    if (pPoleArray)
        for (int i = 0; i < 3; i++)
            {
            bsiDPoint4d_add3ScaledDPoint4d (&pPoleArray[0],
                            NULL,
                            &conic.center, ww[i],
                            &conic.center, cc[i],
                            &conic.center, ss[i]
                            );
            }

    if (pCirclePoleArray)
        {
        for (int i = 0; i < 3; i ++)
            {
            pCirclePoleArray[i].x = cc[i];
            pCirclePoleArray[i].y = ss[i];
            pCirclePoleArray[i].z = ww[i];
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    LuHan      02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void    calcBiarcCenters

(
DPoint3dP        pCenter0,
DPoint3dP        pCenter1,
DPoint3dCP  pPoint0,
DPoint3dCP  pPoint1,
DPoint3dCP  pDir0,
DPoint3dCP  pDir1,
double          rad0,
double          rad1,
bool            isUnimodal
)
    {
    int         i0, i1, i0Min = 0, i1Min = 0;
    double      diff, dist, delta, deltaMin = DBL_MAX;
    DPoint3d    arcCenter0[2], arcCenter1[2];

    bsiDPoint3d_addScaledDPoint3d (arcCenter0, pPoint0, pDir0, rad0);
    bsiDPoint3d_addScaledDPoint3d (arcCenter0+1, pPoint0, pDir0, -rad0);
    bsiDPoint3d_addScaledDPoint3d (arcCenter1, pPoint1, pDir1, rad1);
    bsiDPoint3d_addScaledDPoint3d (arcCenter1+1, pPoint1, pDir1, -rad1);

    diff = isUnimodal ? fabs (rad0 - rad1) : fabs (rad0 + rad1);

    // TR #116288: take centers whose distance is closest to radius difference/sum;
    // previous logic tested three differences against a tolerance and failing that, assumed last was smallest,
    // but this failed to find smallest differences larger than the tolerance.
    for (i0 = 0; i0 < 2; i0++)
        {
        for (i1 = 0; i1 < 2; i1++)
            {
            dist = bsiDPoint3d_distance (&arcCenter0[i0], &arcCenter1[i1]);
            delta = fabs (dist - diff);
            if (delta < deltaMin)
                {
                i0Min = i0;
                i1Min = i1;
                deltaMin = delta;
                }
            }
        }

    *pCenter0 = arcCenter0[i0Min];
    *pCenter1 = arcCenter1[i1Min];
    }

/*-----------------------------------------------------------------*//**
@description Compute an array of 5 points such that points 0,1,2 and 2,3,4 define arcs which have a common tangent at point 2, and have
    specified start and end points and tangents.
@remarks At least one of the tangents should not be parallel to the vector between the points.
@remarks All inputs should be coplanar.
 @param pArcPoints <= array of 5 points
 @param pPoint0 => start point
 @param pPoint1 => end point
 @param pTangent0 => tangent at start
 @param pTangent1 => tangent at end
 @return true if the arcs are computed
@group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_compute3PointBiarcs

(
DPoint3dP pArcPoints,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dP pTangent0,
DPoint3dP pTangent1
)
    {
    bool        isUnimodal;
    double      radius0, radius1, magnitude, cosine0, cosine1, sine0, sine1, tmp, tmp0, tmp1;
    DPoint3d    chord, dir0, dir1, mid, center0, center1, normal, tangent0, tangent1;

    double      absTol, scaleSum = 0.0;
    static double s_relTol = 1.0e-10;
    scaleSum += bsiDPoint3d_dotProduct (pPoint0, pPoint1);
    scaleSum += bsiDPoint3d_dotProduct (pPoint1, pPoint1);
    absTol = s_relTol * sqrt (scaleSum);

    /* Compute the radii for arcs */
    bsiDPoint3d_subtractDPoint3dDPoint3d (&chord, pPoint1, pPoint0);
    magnitude = bsiDPoint3d_magnitude (&chord);
    bsiDPoint3d_normalizeInPlace (&chord);

    bsiDPoint3d_normalize (&tangent0, pTangent0);
    bsiDPoint3d_normalize (&tangent1, pTangent1);

    cosine0 = bsiDPoint3d_dotProduct (&chord, &tangent0);
    cosine1 = bsiDPoint3d_dotProduct (&chord, &tangent1);
    sine0 = sqrt (1.0 - cosine0 * cosine0);
    sine1 = sqrt (1.0 - cosine1 * cosine1);
    bsiDPoint3d_crossProduct (&dir0, &tangent0, &chord);
    bsiDPoint3d_crossProduct (&dir1, &tangent1, &chord);

    // TR #116288:
    // If tangent0 is (nearly) parallel to chord, then |dir0| ~ 0.
    // If this makes the below dot product just slightly negative, then since cosine0 ~ 1 and sine0 ~ 0,
    //  we have the possibility that tmp can be +-INF.
    // To rule this out, allow for some floating point slop in the negativity test below.
    // Using isUnimodal = false for the (near) tangency case avoids the bad intermediate value in computing the arc radii.
    // The same problem occurs (and is fixed by this change) if tangent1 is (nearly) parallel to chord.
    isUnimodal = bsiDPoint3d_dotProduct (&dir0, &dir1) < -mgds_fc_nearZero;
    if (isUnimodal)
        {
        tmp0 = 1.0 - cosine0;
        tmp1 = 1.0 - cosine1;

        if (!bsiTrig_safeDivide (&tmp, magnitude, sine0 * tmp1 + sine1 * tmp0, 0.0))
            return false;

        radius0 = tmp1 * tmp;
        radius1 = tmp0 * tmp;
        }
    else
        {
        if (!bsiTrig_safeDivide (&tmp, magnitude, sine0 + sine1 + sqrt (4.0 - (cosine0 + cosine1) * (cosine0 + cosine1)), 0.0))
            return false;

        radius1 = radius0 = tmp;
        }

    // TR #116288: avoid colinear tangent & chord---caller ensures one tangent is not colinear
    if (fabs (cosine0) < fabs (cosine1))
        bsiDPoint3d_crossProduct (&normal, &chord, &tangent0);
    else
        bsiDPoint3d_crossProduct (&normal, &chord, &tangent1);

    /* Compute the centers for the arcs */
    bsiDPoint3d_normalizedCrossProduct (&dir0, &tangent0, &normal);
    bsiDPoint3d_normalizedCrossProduct (&dir1, &tangent1, &normal);
    calcBiarcCenters (&center0, &center1, pPoint0, pPoint1, &dir0, &dir1, radius0, radius1, isUnimodal);

    /* Create points on arc */
    if (isUnimodal)
        {
        if (fabs (radius0 - radius1) < absTol)
            {
            bsiDPoint3d_interpolate (&dir0, pPoint0, 0.5, pPoint1);
            bsiDPoint3d_subtractDPoint3dDPoint3d (&dir0, &dir0, &center0);
            }
        else
            {
            if (radius1 >= radius0)
                bsiDPoint3d_subtractDPoint3dDPoint3d (&dir0, &center0, &center1);
            else
                bsiDPoint3d_subtractDPoint3dDPoint3d (&dir0, &center1, &center0);
            }
        bsiDPoint3d_normalizeInPlace (&dir0);
        bsiDPoint3d_addScaledDPoint3d (&pArcPoints[2], &center0, &dir0, radius0);
        }
    else
        {
        bsiDPoint3d_interpolate (&pArcPoints[2], &center0, 0.5, &center1);
        }

    pArcPoints[0] = *pPoint0;
    pArcPoints[4] = *pPoint1;
    bsiDPoint3d_interpolate (&mid, pPoint0, 0.5, &pArcPoints[2]);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&dir0, &mid, &center0);
    bsiDPoint3d_normalizeInPlace (&dir0);
    bsiDPoint3d_addScaledDPoint3d (&pArcPoints[1], &center0, &dir0, radius0);
    bsiDPoint3d_interpolate (&mid, pPoint1, 0.5, &pArcPoints[2]);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&dir0, &mid, &center1);
    bsiDPoint3d_normalizeInPlace (&dir0);
    bsiDPoint3d_addScaledDPoint3d (&pArcPoints[3], &center1, &dir0, radius1);

    return true;
    }


/*-----------------------------------------------------------------*//**
@description Initialize an ellipse from center, primary axis point, and additional pass-though point.
@param pEllipse OUT initialized ellipse
@param pCenter => center point of ellipse.
@param pPoint0 => point to appear at the zero degree point.   The ellipse must pass through
                this point as a major or minor axis point, i.e. its tangent must be perpendicular
                to the vector from the center to this point.
@param pPoint1 => additional pass-through point.
@return false if center, point0 and point1 are not independent, or if
    point1 is too far away from center to allow ellipse constrution.
@group "DEllipse3d Initialization"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initFromCenterMajorAxisPointAndThirdPoint

(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    double dot00, dot01, disc, root;
    double cc, ss, dss, axisRatio;
    double alpha0, alpha1;
    double sweep;
    DVec3d vector0, vector1, vector90;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pPoint0, pCenter);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, pPoint1, pCenter);

    dot00 = bsiDPoint3d_dotProduct (&vector0, &vector0);
    dot01 = bsiDPoint3d_dotProduct (&vector0, &vector1);

    disc = dot00 * dot00 - dot01 * dot01;
    if (disc <= 0.0)
        return false;
    root = sqrt (disc);
    /* dot00 must be strictly nonzero, and this division and acos are quite safe: */
    if (    !bsiTrig_safeDivide (&cc, dot01, dot00, 1.0)
        ||  !bsiTrig_safeDivide (&alpha0, dot00, root, 1.0)
        ||  !bsiTrig_safeDivide (&alpha1, dot01, root, 1.0))
        return false;

    sweep = acos (cc);
    /* sine could be calculated as sqrt (1-cc).  */
    ss = sin (sweep);
    if (!bsiTrig_safeDivide (&dss, 1.0, ss, 1.0))
        return false;

    /* vector0, vector1, and the 90 degree vector are related by the ellipse definition
        vector1 = vector0 * cos(sweep) + vector90 * sin(sweep).
        Rearrange for vector90 in terms of vector0 and vector1:
            vector90 = (vector1 - cos(sweep) * vector0) / sin(sweep)
        We know how to compute cos(sweep) = vector0.vector1 / vector0.vector0,
            and sin^2 = 1 - cos^2.  Inserting these dot products into
                 the vector90 expression and simplifying, we get
            vector90 =
                       (vector0.vector0 * vector1 - vector0.vector1 * vector0 )
                    /  sqrt (vector0.vector0 - vector0.vector1)
    */
    bsiDPoint3d_add2ScaledDPoint3d (&vector90, NULL, &vector1, alpha0, &vector0, - alpha1);

    bsiDEllipse3d_initFrom3dVectors (pEllipse, pCenter, &vector0, &vector90,
                    0.0, sweep);

    return bsiTrig_safeDivide (&axisRatio,
                bsiDPoint3d_magnitude (&vector0),
                bsiDPoint3d_magnitude (&vector90),
                1.0);
    }




/*-----------------------------------------------------------------*//**
@param ellipse IN ellipse
@return largest (absolute) coordinate or vector component.
@bsimethod                                     BentleySystems 08/09
+---------------+---------------+---------------+---------------+------*/
double GEOMDLLIMPEXP bsiDEllipse3d_maxAbs
(
DEllipse3dCP pInstance
)
    {
    double a = fabs (pInstance->center.x);
    double b;
    if ((b  = fabs (pInstance->center.y)) > a)
        a = b;
    if ((b  = fabs (pInstance->center.z)) > a)
        a = b;

    if ((b  = fabs (pInstance->vector0.z)) > a)
        a = b;
    if ((b  = fabs (pInstance->vector0.z)) > a)
        a = b;
    if ((b  = fabs (pInstance->vector0.z)) > a)
        a = b;

    if ((b  = fabs (pInstance->vector90.z)) > a)
        a = b;
    if ((b  = fabs (pInstance->vector90.z)) > a)
        a = b;
    if ((b  = fabs (pInstance->vector90.z)) > a)
        a = b;
    return a;
    }

static void AppendPoints (
bvector<DPoint3d>*pIntersectionPoints1,
bvector<DPoint3d>*pIntersectionPoints2,
DPoint3dCR xyz1,
DPoint3dCR xyz2
)
    {
    if (pIntersectionPoints1)
        pIntersectionPoints1->push_back (xyz1);
    if (pIntersectionPoints2)
        pIntersectionPoints2->push_back (xyz2);
    }

static double PointToAngleAssumePerpendicular
(
DEllipse3dCR ellipse,
DPoint3d xyz
)
    {
    DVec3d vector;
    vector.DifferenceOf (xyz, ellipse.center);
    double c = vector.DotProduct (ellipse.vector0);
    double s = vector.DotProduct (ellipse.vector90);
    return bsiTrig_atan2 (s, c);
    }

/*-----------------------------------------------------------------*//**
@description Test if two ellipses are circular and parallel.  If so, compute intersections of (unbounded) circles.
If not, return with no computed points.
Note that true/false return does not correspond to intersecting and non-intersecting.   Two ellipses that
intersect will return false with no intersections.   Two coplanar circles with no intersections return
true with no intersections.
@remark The order of the returned points is as per legacy microstation l_intsec.c.
@param [out] pIntersectionPoints1 xyz coordinates on ellipse1.  May not be NULL.
@param [out] pIntersectionPoints2 xyz coordinates on ellipse2   May be NULL.
@param [out] pAngles1 angular position on ellipse1. May be NULL.
@param [out] pAngles2 angular position on ellipse2. May be NULL.
@param [in] pEllipse1 first ellipse
@param [in] pEllipse2 second ellipse
@param [in] tangency tolerance.  If 0, a default will be supplied based on ellipse coordinates.
@return true if (a) both ellipses are cicular (by isCircular), 
   (b) the normals are parallel (by isParallelTo).
@bsimethod                                     BentleySystems 08/09
+---------------+---------------+---------------+---------------+------*/
bool    GEOMDLLIMPEXP bsiDEllipse3d_isParallelPlaneCircleCircleIntersect
(
bvector<DPoint3d>*pIntersectionPoints1,
bvector<DPoint3d>*pIntersectionPoints2,
bvector<double>*pAngles1,
bvector<double>*pAngles2,
DEllipse3dCP    pEllipse1,
DEllipse3dCP    pEllipse2,
double          tolerance
)
    {
    bool stat = false;
    if (tolerance <= 0.0)
        tolerance = bsiTrig_smallAngle () * bsiTrig_maxAbsDoubleDouble (
                            pEllipse1->MaxAbs (), pEllipse2->MaxAbs ());
    DVec3d normal1, normal2;
    normal1.NormalizedCrossProduct (pEllipse1->vector0, pEllipse1->vector90);
    normal2.NormalizedCrossProduct (pEllipse2->vector0, pEllipse2->vector90);

    if (    pEllipse1->IsCircular ()
        &&  pEllipse2->IsCircular ()
        &&  normal1.IsParallelTo (normal2))    // anitparallel passes ....
        {
        stat = true;
        DVec3d vector12, vector12InPlane, unit12InPlane, centerZ;
        vector12.DifferenceOf (pEllipse2->center, pEllipse1->center);
        double a = pEllipse1->vector0.Magnitude ();
        double b = pEllipse2->vector0.Magnitude ();
        double z = vector12.DotProduct (normal1);
        centerZ.Scale (normal1, z);
        vector12InPlane.SumOf (vector12,normal1, -z);
        unit12InPlane.Normalize (vector12InPlane);
        double d  = vector12InPlane.Magnitude ();
        double c0 = vector12.DotProduct (pEllipse1->vector0);
        double s0 = vector12.DotProduct (pEllipse1->vector90);
        double phi0 = bsiTrig_atan2 (s0, c0);
        DPoint3d xyz1, xyz2;

        if (d < tolerance)
            {
            // No intersections
            }
        else if (fabs (a + b - d) < tolerance)
            {
            xyz1.SumOf (pEllipse1->center,unit12InPlane, a);
            xyz2.SumOf (pEllipse2->center,unit12InPlane, -b);
            AppendPoints (pIntersectionPoints1, pIntersectionPoints2, xyz1, xyz2);
            }
        else if (fabs (a + d - b) < tolerance)
            {
            xyz1.SumOf (pEllipse1->center,unit12InPlane, -a);
            xyz2.SumOf (pEllipse2->center,unit12InPlane, -b);
            AppendPoints (pIntersectionPoints1, pIntersectionPoints2, xyz1, xyz2);
            }
        else if (fabs (b + d - a) < tolerance)
            {
            xyz1.SumOf (pEllipse1->center,unit12InPlane, a);
            xyz2.SumOf (pEllipse2->center,unit12InPlane, b);
            AppendPoints (pIntersectionPoints1, pIntersectionPoints2, xyz1, xyz2);
            }
        else if (d < a + b)
            {
            double alpha = (a * a - b * b + d * d) / (2.0 * d);
            if (fabs (alpha) < a)
                {
                double theta = acos (alpha / a);
                pEllipse1->Evaluate (xyz1, phi0 + theta);
                xyz2.SumOf (xyz1,centerZ);
                AppendPoints (pIntersectionPoints1, pIntersectionPoints2, xyz1, xyz2);
                pEllipse1->Evaluate (xyz1, phi0 - theta);
                xyz2.SumOf (xyz1,centerZ);
                AppendPoints (pIntersectionPoints1, pIntersectionPoints2, xyz1, xyz2);
                }
            }        

        if (pAngles1 != NULL || pAngles2 != NULL)
            {
            for (size_t i = 0; i < pIntersectionPoints1->size (); i++)
                {
                if (pAngles1 != NULL)
                    pAngles1->push_back (PointToAngleAssumePerpendicular (*pEllipse1, pIntersectionPoints1->at(i)));
                if (pAngles2 != NULL)
                    pAngles2->push_back (PointToAngleAssumePerpendicular (*pEllipse2, pIntersectionPoints2->at(i)));
                }
            }
        }

    return stat;
    }







/*-----------------------------------------------------------------*//**
@description Get the coordinate frame and inverse of an ellipse as viewed along the global z axis.
 @param pEllipse    => ellipse whose frame is computed.
 @param pFrame      <= transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
 @param pInverse    <= inverse of frame.  May be NULL.
 @return true if the requested frames were returned.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getXYLocalFrameAsTransform
(
DEllipse3dCP pEllipse,
TransformP pFrame,
TransformP pInverse
)
    {
    Transform frame;
    bool    myStat = true;
    frame.InitFromOriginAndVectors (
                    pEllipse->center,
                    pEllipse->vector0,
                    pEllipse->vector90,
                    DVec3d::From (0.0, 0.0, 1.0)
                    );

    if (pFrame)
        *pFrame = frame;

    if (pInverse)
        myStat = bsiTransform_invertTransform (pInverse, &frame);

    return myStat;
    }

// Modify an ellipse so that
//  1) one endpoint and its tangent are preserved.
//  2) sweep angle is preserved
//  3) the other endpoint moves a specified vector distance.
// @param [out] result transformed ellipse
// @param [in] source original ellipse
// @param [in] translation translation vector to apply.
// @param [in] movingEndIndex 0 to move startpoint, 1 to move endpoint
bool bsiDEllipse3d_translateEndPoint (DEllipse3dP result, DEllipse3dCP source, DVec3dCP translation, int movingEndIndex)
    {
    DVec3d vectorU0, vectorV0, vectorW0;
    DVec3d vectorU1, vectorV1, vectorW1;
    DPoint3d fixedPoint, movingPoint;
    double theta0 = source->start;
    double theta1 = source->start + source->sweep;
    if (movingEndIndex == 0)
        {
        bsiDEllipse3d_evaluateDerivatives (source, &fixedPoint, &vectorU0, NULL, theta1);
        bsiDEllipse3d_evaluateDerivatives (source, &movingPoint, NULL, NULL, theta0);
        }
    else
        {
        bsiDEllipse3d_evaluateDerivatives (source, &fixedPoint, &vectorU0, NULL, theta0);
        bsiDEllipse3d_evaluateDerivatives (source, &movingPoint, NULL, NULL, theta1);
        }
    vectorV0.DifferenceOf (movingPoint, fixedPoint);
    vectorW0.GeometricMeanCrossProduct (vectorU0, vectorV0);

    vectorU1 = vectorU0;
    vectorV1.SumOf (vectorV0,*translation);
    vectorW1.GeometricMeanCrossProduct (vectorU1, vectorV1);

    Transform frame0, frame1;
    frame0.InitFromOriginAndVectors (fixedPoint, vectorU0, vectorV0, vectorW0);
    frame1.InitFromOriginAndVectors (fixedPoint, vectorU1, vectorV1, vectorW1);
    Transform inverse0;
    if (inverse0.InverseOf(frame0))
        {
        Transform product;
        product.InitProduct (frame1, inverse0);
        bsiDEllipse3d_multiplyTransformDEllipse3d (result, &product, source);
        return true;
        }
    *result = *source;
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE

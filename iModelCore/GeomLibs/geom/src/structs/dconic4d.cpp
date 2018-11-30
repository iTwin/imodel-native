/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dconic4d.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*=================================================================================**//**
* @struct DConic4d
* A DConic4d is (depending on the weights in its DPoint4d points) an ellipse, hyperbola, or
* parabola.
*
* @fields
* @field DPoint4d center fixed point of parameterized form.
* @field DPoint4d vector0 difference (vector) from center to 0 degree point.
* @field DPoint4d vector90 difference (vector) from center to 90 degree point.
* @field double start parametric angle of start point.
* @field dobule sweep parametric angle of sweep angle.
* @endfields
* @bsistruct                                                    EarlinLutz      02/99
+===============+===============+===============+===============+===============+======*/
#include <bsibasegeomPCH.h>
#include "../DeprecatedFunctions.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// deprecated, body in refdsegment4d.cpp
void       bsiDSegment4d_getXYWImplicitDPoint4dPlane
(
DSegment4dCP pInstance,
DPoint4dP pPlaneCoffs
);
static double s_lineUnitCircleIntersectionTolerance = 1.0e-8;

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value


/*-----------------------------------------------------------------*//**
* Initialize the range from an arc of the unit circle
* @param theta0 => start angle
* @param sweep  => angular sweep
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromUnitArcSweep

(
DRange3dP pRange,
double          theta0,
double          sweep
)
    {
    double theta1 = theta0 + sweep;
    if (Angle::IsFullCircle (sweep))
        {
        pRange->InitFrom (-1.0, -1.0, 0.0, 1.0, 1.0, 0.0);
        }
    else
        {
        pRange->InitFrom (
                cos (theta0), sin (theta0), 0.0,
                cos (theta1), sin (theta1), 0.0);

        /* Force the range out to the axis extremes if they are in the sweep */
        if (Angle::InSweepAllowPeriodShift (0.0,              theta0, sweep))
            {
            pRange->high.x = 1.0;
            }

        if (Angle::InSweepAllowPeriodShift (msGeomConst_pi,  theta0, sweep))
            {
            pRange->low.x = -1.0;
            }

        if (Angle::InSweepAllowPeriodShift (msGeomConst_piOver2, theta0, sweep))
            {
            pRange->high.y = 1.0;
            }

        if (Angle::InSweepAllowPeriodShift (-msGeomConst_piOver2, theta0, sweep))
            {
            pRange->low.y = -1.0;
            }
        }

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
static bool     DEllipse3d_initFromDConic4d

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
        pEllipse->center.XyzOf (normalizedSource.center);
        pEllipse->vector0.XyzOf (normalizedSource.vector0);
        pEllipse->vector90.XyzOf (normalizedSource.vector90);
        pEllipse->start = normalizedSource.start;
        pEllipse->sweep = normalizedSource.sweep;
        }
    return funcStat;
    }
/*-----------------------------------------------------------------*//**
* @class DConic4d
* The DConic4d structure can represent an ellipse, parabola or hyperbola in 4d homogeneous space
*
* If C is a point ("center") and U and V are any vectors,
*<pre>
*               X = C + cos(theta)*U + sin(theta)*V
*</pre>
* sweeps X along an elliptic curve in homogeneous space. This may be a
* hyperbola, parabola, or ellipse in 3d.
*
* @bsihdr                                                       EarlinLutz      12/97
+===============+===============+===============+===============+======*/


/*-----------------------------------------------------------------*//**
* @description Fill in conic data.
*
* @param pConic <= initialized conic
* @param cx => center x coordinate
* @param cy => center y coordinate
* @param cz => center z coordinate
* @param ux => vector0 x coordinate
* @param uy => vector0 y coordinate
* @param uz => vector0 z coordinate
* @param vx => vector90 x coordinate
* @param vy => vector90 y coordinate
* @param vz => vector90 z coordinate
* @param theta0 => start angle in parameter space
* @param sweep => sweep angle in parameter space
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_init

(
DConic4dP pConic,
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
    pConic->center.Init( cx, cy, cz, 1.0);
    pConic->vector0.Init( ux, uy, uz, 0.0);
    pConic->vector90.Init( vx, vy, vz, 0.0);

    pConic->start        = theta0;
    pConic->sweep        = sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Fill in conic data from 2D major and minor axis lengths and the angle
*   from the global to the local x-axis.
*
* @param pConic <= initialized conic
* @param cx => center x coordinate
* @param cy => center y coordinate
* @param cz => z coordinate of all points on the conic
* @param rx => radius along local x axis
* @param ry => radius along local y axis
* @param thetaX => angle from global x-axis to local x-axis
* @param theta0 => start angle in parameter space
* @param sweep => sweep angle in parameter space
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromXYMajorMinor

(
DConic4dP pConic,
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

    pConic->center.Init( cx, cy, cz, 1.0);
    pConic->center.Init(  rx * ux, rx * uy, 0.0, 0.0);
    pConic->center.Init( -ry * uy, ry * ux, 0.0, 0.0);

    pConic->start        = theta0;
    pConic->sweep        = sweep;
    }




/*-----------------------------------------------------------------*//**
* @description Initialize a conic from its center, 0 degree, and 90 degree points.
*
* @param pConic <= initialized conic
* @param pCenter => conic center
* @param pPoint0 => 0 degree point
* @param pPoint90 => 90 degree point
* @param theta0 => start angle
* @param sweep => sweep angle
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom3dPoints

(
DConic4dP pConic,
DPoint3dCP pCenter,
DPoint3dCP pPoint0,
DPoint3dCP pPoint90,
double          theta0,
double          sweep
)

    {
    DPoint3d vector0, vector90;
    vector0.DifferenceOf (*pPoint0, *pCenter);
    vector90.DifferenceOf (*pPoint90, *pCenter);
    bsiDConic4d_initFrom3dVectors (pConic,
                    pCenter,
                    &vector0,
                    &vector90,
                    theta0,
                    sweep);
    }




/*-----------------------------------------------------------------*//**
* @description Initalize a conic from its homogeneous center, homogeneous vector to 0 degree point, and homogeneous vector to 90 degree point.
*
* @param pConic <= initialized conic
* @param pCenter => conic center
* @param pVector0 => 0 degree vector
* @param pVector90 => 90 degree vector
* @param theta0 => start angle
* @param sweep => sweep angle
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom4dVectors

(
DConic4dP pConic,
DPoint4dCP pCenter,
DPoint4dCP pVector0,
DPoint4dCP pVector90,
double          theta0,
double          sweep
)

    {
    pConic->center       = *pCenter;
    pConic->vector0      = *pVector0;
    pConic->vector90     = *pVector90;
    pConic->start        = theta0;
    pConic->sweep        = sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Initialize a conic from its center, 0 degree, and 90 degree points presented
* as an array of 3 points.
*
* @param pConic <= initialized conic.
* @param pPointArray => conic center, 0 degree and 90 degree points.
* @param theta0 => start angle
* @param sweep => sweep angle
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom3dPointArray

(
DConic4dP pConic,
DPoint3dCP pPointArray,
double              theta0,
double              sweep
)

    {
    bsiDConic4d_initFrom3dPoints (pConic,
                pPointArray,
                pPointArray + 1,
                pPointArray + 2,
                theta0,
                sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Set angular parameters to have given start and end points.
*
* @remarks If
* the given points are really on the conic, this does the expected thing.
* If they are not, here's exactly what happens: the start/end points are
* placed on the original conic at the point where the conic intersects
* the plane formed by the conic axis and the given point.
*
* @remarks This leaves the problem that the conic defines two paths from the
* given start to end. This is resolved as follows.  The conic's existing
* 0 and 90 degree vectors define a coordinate system.  In that system, the short
* sweep from the 0 degree vector to the 90 degree vector is considered "counterclockwise".
*
* @remarks Beware that the relation of supposed start/end points to the conic
*  is ambiguous
*
* @param pConic <=> conic to modify
* @param pStartPoint => start point to set
* @param pEndPoint => end point to set
* @param ccw    => true to force counterclockwise direction, false for clockwise.
* @return true if the conic's local to global transformation is invertible
* @group "DConic4d Modification"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiDConic4d_setStartEnd

(
DConic4dP pConic,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint,
bool        ccw
)
    {
    DMatrix4d inverseFrame;
    DPoint4d    point[2], paramPoint[2];
    double      theta0, theta1, sweep;
    bool        isInvertible;

    isInvertible = bsiDConic4d_getLocalFrame (pConic, NULL, &inverseFrame);

    if (isInvertible)
        {
        point[0].InitFrom (*pStartPoint, 1.0);
        point[1].InitFrom (*pStartPoint, 1.0);

        bsiDMatrix4d_multiply4dPoints (&inverseFrame, paramPoint, point, 2);
        theta0 = Angle::Atan2 (paramPoint[0].y, paramPoint[0].x);
        theta1 = Angle::Atan2 (paramPoint[1].y, paramPoint[1].x);
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
        pConic->start = theta0;
        pConic->sweep = sweep;
        }
    return isInvertible;
    }


/*-----------------------------------------------------------------*//**
* @description Initialize a conic from its center, x and y directions from columns
* 0 and 1 of a RotMatrix, scaled factors to apply to x and and y directions.
*
* @param pConic <= initialized conic.
* @param pCenter => conic center
* @param pMatrix => columns 0, 1 are conic directions (to be scaled by r0, r1)
* @param r0 => scale factor for column 0.
* @param r1 => scale factor for column 1.
* @param theta0 => start angle
* @param sweep => sweep angle
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromScaledRotMatrix

(
DConic4dP pConic,
DPoint3dCP pCenter,
RotMatrixCP pMatrix,
double      r0,
double      r1,
double              theta0,
double              sweep
)

    {
    DVec3d vectorU = DVec3d::FromColumn (*pMatrix, 0);
    DVec3d vectorV = DVec3d::FromColumn (*pMatrix, 1);
    vectorU.Scale (r0);
    vectorV.Scale (r0);

    bsiDConic4d_initFrom3dVectors (pConic,
                pCenter,
                &vectorU,
                &vectorV,
                theta0,
                sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Initialize a conic from its center, x and y directions from vectors
*   with scale factors.
*
* @param pConic <= initialized conic.
* @param pCenter => conic center
* @param pVector0 => 0 degree vector (e.g. major axis)
* @param pVector90 => 90 degree vector (e.g. minor axis)
* @param r0 => scale factor for vector 0
* @param r1 => scale factor for vector 90.
* @param theta0 => start angle
* @param sweep => sweep angle
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromScaledVectors

(
DConic4dP pConic,
DPoint3dCP pCenter,
DPoint3dCP pVector0,
DPoint3dCP pVector90,
double      r0,
double      r1,
double              theta0,
double              sweep
)

    {
    DPoint3d vectorU, vectorV;

    vectorU.Scale (*pVector0, r0);
    vectorV.Scale (*pVector90, r1);
    bsiDConic4d_initFrom3dVectors (pConic,
                pCenter,
                &vectorU,
                &vectorV,
                theta0,
                sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Initialize a conic from a circle.
* @param pConic <= initialized conic
* @param pCenter => circle center
* @param pNormal => plane normal
* @param radius => circle radius
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromCenterNormalRadius

(
DConic4dP pConic,
DPoint3dCP pCenter,
DVec3dCP pNormal,
double          radius
)

    {
    DVec3d uVector, vVector, wVector;

    if (pNormal)
        {
        pNormal->GetNormalizedTriad (uVector, vVector, wVector);
        uVector.Scale (uVector, radius);
        vVector.Scale (vVector, radius);
        }
    else
        {
        uVector.Init ( radius,  0.0,    0.0);
        uVector.Init ( 0.0,     radius, 0.0);
        }

    bsiDConic4d_initFrom3dVectors
                    (
                    pConic,
                    pCenter,
                    &uVector,
                    &vVector,
                    0.0,
                    msGeomConst_2pi
                    );
    }


/*-----------------------------------------------------------------*//**
* @description Tests whether the conic is complete (2pi range).
* @param pConic <= initialized conic.
* @return true if the conic is complete (2pi range)
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_isFullSweep

(
DConic4dCP pConic
)
    {
    return Angle::IsFullCircle (pConic->sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Set the ellipse sweep to a full 360 degrees (2pi radians).  Start angle is left unchanged.
* @param pConic <=> ellipse to change
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_makeFullSweep

(
DConic4dP pConic
)
    {
    pConic->sweep = msGeomConst_2pi;
    }


/*-----------------------------------------------------------------*//**
* @description Set the ellipse sweep the the complement of its current angular range.
* @remarks A full ellipse is left unchanged.
* @param pConic <=> ellipse to change
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_complementSweep

(
DConic4dP pConic
)
    {
    if (!bsiDConic4d_isFullSweep (pConic))
        pConic->sweep = bsiTrig_complementaryAngle (pConic->sweep);
    }



/*-----------------------------------------------------------------*//**
* @description Compute the conic xyz point at a given parametric (angular) coordinate.
* @param pConic => conic to evaluate
* @param pPoint <= evaluated point, projected back to 3D
* @param theta => angle
* @return true if normalization of the point succeeded
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDConic4d_angleParameterToDPoint3d

(
DConic4dCP pConic,
DPoint3dP pPoint,
double      theta
)
    {
    double cosTheta, sinTheta;
    DPoint4d homogeneousPoint;
    cosTheta = cos(theta);
    sinTheta = sin(theta);
    homogeneousPoint.SumOf(pConic->center, pConic->vector0, cosTheta, pConic->vector90, sinTheta);
    return homogeneousPoint.GetProjectedXYZ (*pPoint);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the conic point at a given parametric (angular) coordinate.
* @param pConic => conic to evaluate
* @param pPoint <= evaluated point
* @param theta => angle
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  bsiDConic4d_angleParameterToDPoint4d

(
DConic4dCP pConic,
DPoint4dP pPoint,
double      theta
)
    {
    double cosTheta, sinTheta;
    cosTheta = cos(theta);
    sinTheta = sin(theta);
    pPoint->SumOf(pConic->center, pConic->vector0, cosTheta, pConic->vector90, sinTheta);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the conic xyz point at a given parametric (angular) coordinate.
* @param pConic => conic to evaluate
* @param pPoint <= evaluated point
* @param xx => local x coordinate
* @param yy => local y coordinate
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_trigParameterToDPoint4d

(
DConic4dCP pConic,
DPoint4dP pPoint,
double      xx,
double      yy
)
    {
    pPoint->SumOf(pConic->center, pConic->vector0, xx, pConic->vector90, yy);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the conic start and end points.
* @param pConic => conic to evaluate
* @param pStartPoint <= start point of conic
* @param pEndPoint  <= end point of conic
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_getDPoint4dEndPoints

(
DConic4dCP pConic,
DPoint4dP pStartPoint,
DPoint4dP pEndPoint
)
    {
    bsiDConic4d_angleParameterToDPoint4d (pConic, pStartPoint, pConic->start);
    bsiDConic4d_angleParameterToDPoint4d (pConic, pEndPoint,   pConic->start + pConic->sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the conic start and end points (normalized).
* @param pConic => conic to evaluate
* @param pStartPoint <= start point of conic
* @param pEndPoint  <= end point of conic
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_getDPoint3dEndPoints

(
DConic4dCP pConic,
DPoint3dP pStartPoint,
DPoint3dP pEndPoint                  
)
    {
    bsiDConic4d_angleParameterToDPoint3d (pConic, pStartPoint, pConic->start);
    bsiDConic4d_angleParameterToDPoint3d (pConic, pEndPoint,   pConic->start + pConic->sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the conic point and derivatives at a given parametric (angular) coordinate.
* @param pConic => conic to evaluate
* @param pX <= (optional) point on conic
* @param pdX <= (optional) first derivative vector
* @param pddX <= (optional) second derivative vector
* @param theta => angle for evaluation
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_angleParameterToDPoint4dDerivatives

(
DConic4dCP pConic,
DPoint4dP pX,
DPoint4dP pdX,
DPoint4dP pddX,
double      theta
)
    {
    double cosTheta, sinTheta;
    DPoint4d vector;

    cosTheta = cos(theta);
    sinTheta = sin(theta);

    if (pX || pddX)
        {
        vector.SumOf(pConic->vector0, cosTheta, pConic->vector90, sinTheta);
        if (pX)
            pX->SumOf (pConic->center, vector);

        if (pddX)
            pddX->Negate(vector);
        }

    if (pdX)
        pdX->SumOf(pConic->vector0, -sinTheta, pConic->vector90, cosTheta);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the conic homogeneous point and derivatives at a given parametric (fractional) coordinate.
* @param pConic => conic to evaluate
* @param pX <= (optional) point on conic
* @param pdX <= (optional) first derivative vector
* @param pddX <= (optional) second derivative vector
* @param fraction => fractional parameter for evaluation
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_fractionParameterToDPoint4dDerivatives

(
DConic4dCP pConic,
DPoint4dP pX,
DPoint4dP pdX,
DPoint4dP pddX,
double      fraction
)
    {
    double theta = pConic->start + fraction * pConic->sweep;
    double a = pConic->sweep;

    bsiDConic4d_angleParameterToDPoint4dDerivatives (pConic, pX, pdX, pddX, theta);
    if (pdX)
        pdX->Scale (*pdX, a);
    if (pddX)
        pddX->Scale (*pddX, a * a);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the conic xyz point at a given parametric (angular) coordinate.
* @param pConic => conic to evaluate
* @param pX <= (optional) point on conic
* @param pdX <= (optional) first derivative vector
* @param pddX <= (optional) second derivative vector
* @param theta => angle for evaluation
* @group "DConic4d Parameterization"
* @return true if normalization of the point succeeded
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_angleParameterToDPoint3dDerivatives

(
DConic4dCP pConic,
DPoint3dP pX,
DPoint3dP pdX,
DPoint3dP pddX,
double      theta
)
    {
    DPoint4d X, dX, ddX;
    DPoint4d D1, D2, Z;
    double w, w2, wdw;
    double dw;
    double divw, divw2, divw4;
    bool    boolstat = false;

    if (pddX)
        {
        bsiDConic4d_angleParameterToDPoint4dDerivatives (pConic, &X, &dX, &ddX, theta);
        w = X.w;
        dw = dX.w;
        boolstat = DoubleOps::SafeDivide (divw, 1.0, w, 1.0);
        w2 = w * w;
        wdw = w * dw;
        divw2 = divw * divw;
        divw4 = divw2 * divw2;

        D2.WeightedDifferenceOf(ddX, X);
        D1.WeightedDifferenceOf(dX, X);
        Z.SumOf(D2, w2, D1, -2.0 * wdw);
        *pddX = Z. GetScaledXYZ (divw4);

        if (pdX)
            *pdX = D1. GetScaledXYZ (divw2);

        if (pX)
            *pX = X. GetScaledXYZ (divw);
        }
    else if (pdX)
        {
        bsiDConic4d_angleParameterToDPoint4dDerivatives (pConic, &X, &dX, NULL, theta);
        w = X.w;
        boolstat = DoubleOps::SafeDivide (divw, 1.0, w, 1.0);
        divw2 = divw * divw;

        D1.WeightedDifferenceOf(dX, X);

        *pdX = D1. GetScaledXYZ (divw2);

        if (pX)
            *pX = X. GetScaledXYZ (divw);

        }
    else if (pX)
        {
        boolstat = bsiDConic4d_angleParameterToDPoint3d (pConic, pX, theta);
        }
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
* @description Compute numDerivatives+1 points pX[i]= i'th derivative
* @param pConic => conic to evaluate
* @param pX <= Array of conic point, first derivative, etc.
* @param numDerivative => number of derivatives
* @param theta => angle for evaluation
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_angleParameterToDerivativeArray

(
DConic4dCP pConic,
DPoint4dP pX,
int         numDerivative,
double      theta
)
    {
    double cosTheta, sinTheta;
    DPoint4d vector;
    int i;

    cosTheta = cos(theta);
    sinTheta = sin(theta);

    if (pX || numDerivative >= 2)
        {
        vector.SumOf(pConic->vector0, cosTheta, pConic->vector90, sinTheta);

        if (pX)
            pX->SumOf (pConic->center, vector);

        for (i = 2; i <= numDerivative ; i += 2)
            {
            vector.Negate();
            pX[i] = vector;
            }
        }

    if (numDerivative >= 1)
        {
        vector.SumOf(pConic->vector0, -sinTheta, pConic->vector90, cosTheta);
        pX[1] = vector;

        for (i = 3; i <= numDerivative ; i += 2 )
            {
            vector.Negate();
            pX[i] = vector;
            }
        }

    }


/*-----------------------------------------------------------------*//**
* @description Convert a fractional parameter to conic parameterization angle.
* @param pConic => conic to evaluate
* @param fraction => fractional parameter
* @return angular parameter
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDConic4d_fractionToAngle

(
DConic4dCP pConic,
double      fraction
)
    {
    return  pConic->start + fraction * pConic->sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Get the center, 0 degree, and 90 homogeneous degree basis points of the conic.
* @param pConic     => conic
* @param pCenter    <= (optional) center point
* @param pVector0   <= (optional) 0 degree basis vector.
* @param pVector90  <= (optional) 90 degree basis vector.
* @group "DConic4d Local Coordinates"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_getDPoint4dBasis

(
DConic4dCP pConic,
DPoint4dP pCenter,
DPoint4dP pVector0,
DPoint4dP pVector90
)
    {
    if (pCenter)
        *pCenter    = pConic->center;
    if (pVector0)
        *pVector0   = pConic->vector0;
    if (pVector90)
        *pVector90  = pConic->vector90;
    }


/*-----------------------------------------------------------------*//**
* @description Get the coordinate frame for a conic.  X,Y, and W colums are the vector0, vector90, and
* center of the conic. Z column is out of plane.
* @param pConic      => conic whose frame is computed.
* @param pFrame         <= transformation from (cosine, sine, z, 1) coordinates to global.  May be NULL.
* @param pInverse       <= inverse of frame.  May be NULL.
* @return true if the requested frames were returned.
* @group "DConic4d Local Coordinates"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_getLocalFrame

(
DConic4dCP pConic,
DMatrix4dP pFrame,
DMatrix4dP pInverse
)
    {
#define QR_LOCAL_FRAME
#ifdef QR_LOCAL_FRAME
    DMatrix4d A, Q, R, QT, AI;
    DPoint4d q3;
    double p0, p1, p2, s;
    double es;
    double centerScale, weightScale, preScale, postScale;
    int i;

    /* Calculations work with first 3 columns.  */
    memset (&A, 0, sizeof (DMatrix4d));
    A.SetColumn (0, pConic->vector0);
    A.SetColumn (1, pConic->vector90);
    A.SetColumn (2, pConic->center);
    centerScale = fabs (pConic->center.x)
             + fabs (pConic->center.y)
             + fabs (pConic->center.z);
    weightScale = fabs (pConic->vector0.w)
                + fabs (pConic->vector90.w)
                + fabs (pConic->center.w);
    /* Wow --- really badly conditioned matrices due
        to weights around 1 and enormous coordinates.
        Prescale the last row up to coordinate range,
        and postscale back down.
    */
    if (   centerScale > weightScale
        && weightScale > 0.0
        )
        {
        preScale = centerScale / weightScale;
        postScale = weightScale / centerScale;
        for (i = 0; i < 4; i++)
            A.coff[3][i] *= preScale;
        }
    else
        preScale = postScale = 1.0;

    R = A;

    bsiLinAlg_givensQR ((double*)&QT, (double*)&R, 4, 3, 4, 1, 4, 1);
    Q.TransposeOf (QT);

#ifdef VERIFY
        {
        DMatrix4d QR, QQT, Diff;
        double maxDiff0;
        bool    isIdentity;
        QR.InitProduct (Q, R);
        bsiDMatrix4d_subtractDMatrix4d (&Diff, &QR, &A);
        maxDiff0 = bsiDMatrix4d_maxAbs (&Diff);
        QQT.InitProduct (Q, QT);
        isIdentity = QQT.IsIdentity ();
        isIdentity = QQT.IsIdentity ();
        }
#endif

    p0 = fabs (R.coff[0][0]);
    p1 = fabs (R.coff[1][1]);
    p2 = fabs (R.coff[2][2]);
    s  = p0 + p1 + p2;
    es = DBL_EPSILON * s;

    if (p0 <= es || p1 < es || p2 < es)
        {

        if (pFrame)
            pFrame->InitIdentity ();

        if (pInverse)
            pInverse->InitIdentity ();

        return false;
        }

    /* The last column of the orthogonal part becomes
        the last colum of the expanded A */

    Q.GetColumn (q3, 3);
    A.SetColumn (3, q3);
    R.SetColumn ( 3, 0.0, 0.0, 0.0, 1.0);

    AI = QT;
    bsiLinAlg_backSub ((double*)&AI, 4, 1, 4, (double*)&R, 4, 1, 4);

    if (preScale != 1.0)
        {
        for (i = 0; i < 4; i++)
            {
            A.coff[3][i] *= postScale;
            AI.coff[i][3] *= preScale;
            }
        }
#ifdef VERIFY
        {
        DMatrix4d RAI, Diff;
        double maxDiff0;
        RAI.InitProduct (R, AI);
        bsiDMatrix4d_subtractDMatrix4d (&Diff, &QT, &RAI);
        maxDiff0 = bsiDMatrix4d_maxAbs (&Diff);
        maxDiff0 = bsiDMatrix4d_maxAbs (&Diff);
        }
#endif

    /* Swap final columns to conform to usual interpretation
        with the out-of-plane part as z */

    A.SwapColumns (2, 3);
    AI.SwapRows (2, 3);

#ifdef VERIFY
        {
        DMatrix4d AAI;
        bool    isIdentity;
        AAI.InitProduct (A, AI);
        isIdentity = AAI.IsIdentity ();
        isIdentity = AAI.IsIdentity ();
        }
#endif

    if (pFrame)
        *pFrame = A;
    if (pInverse)
        *pInverse = AI;
    return true;
#else
    DPoint4d diff0, diff90;
    DPoint3d vec0, vec90, vecZ;
    DMatrix4d A, AI;
    /* This is an explicit scheme that works GREAT except.... when center.w=0 it
        failes.  GRRRRRRRRRRRR. */
    diff0.WeightedDifferenceOf(pConic->vector0, pConic->center);
    diff90.WeightedDifferenceOf(pConic->vector90, pConic->center);
    diff0.GetXYZ (vec0);
    diff90.GetXYZ (vec90);
    vecZ.NormalizedCrossProduct (vec0, vec90);
    A.SetColumn (0, pConic->vector0);
    A.SetColumn (1, pConic->vector90);
    A.SetColumn ( 2, vecZ.x, vecZ.y, vecZ.z, 0.0);
    A.SetColumn (3, pConic->center);

    if (!bsiDMatrix4d_invertQR (&AI, &A))
        {
        if (pFrame)
            pFrame->InitIdentity ();

        if (pInverse)
            pInverse->InitIdentity ();
        return false;
        }

    if (pFrame)
        *pFrame = A;
    if (pInverse)
        *pInverse = AI;

    return true;

#endif
    }

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Get the coordinate frame and inverse of a conic viewed along the z axis, with
* perspective effects.
* @param pConic      => conic whose frame is computed.
* @param pFrame         <= transformation from (cosine, sine, z) coordinates to global.  May be NULL.
* @param pInverse       <= inverse of frame.  May be NULL.
* @return true if the requested frames were returned.
* @group "DConic4d Local Coordinates"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_getXYLocalFrame

(
DConic4dCP pConic,
DTransform3dP pFrame,
DTransform3dP pInverse
)
    {
    DTransform3d frame;
    bool    myStat = true;
    frame.InitFromOriginAndVectors (pConic->center, pConic->vector0, pConic->vector90,
                        DVec3d::From (0.0, 0.0, 1.0));

    if (pFrame)
        *pFrame = frame;

    if (pInverse)
        myStat = bsiDTransform3d_invert (pInverse, &frame);

    return myStat;
    }
#endif


/*-----------------------------------------------------------------*//**
* @description Extract the xyw parts of each coordinate into a DEllipse3d.
* @param pConic      => conic to be reduced.
* @param pEllipse       <= reduced-dimension conic.
* @group "DConic4d Queries"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_getDConic3dXYW

(
DConic4dCP pConic,
DEllipse3dP pEllipse
)
    {
    pEllipse->start = pConic->start;
    pEllipse->sweep = pConic->sweep;
    pConic->center.GetXYW (pEllipse->center);
    pConic->vector0.GetXYW (pEllipse->vector0);
    pConic->vector90.GetXYW (pEllipse->vector90);
    }


/*-----------------------------------------------------------------*//**
* @description Extract the xyw parts of each coordinate into a RotMatrix.
* Column 0 is the 0 degree (cosine) vector.
* Column 1 is the 90 degree (sine) vector.
* Column 2 is the center.
* @param pConic      => conic to be reduced.
* @param pMatrix        <= matrix form
* @group "DConic4d Local Coordinates"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_getRotMatrixXYW
(
DConic4dCP  pConic,
RotMatrixP  pMatrix
)
    {
    DVec3d vec0, vec1, vec2;
    pConic->vector0.GetXYW (vec0);
    pConic->vector90.GetXYW (vec1);
    pConic->center.GetXYW (vec2);
    pMatrix->InitFromColumnVectors (vec0, vec1, vec2);
    }


/*-----------------------------------------------------------------*//**
* @description Extract the xyw parts of each coordinate into a RotMatrix, with separate translation.
* The ellipse is Translation * Basis * UnitCircle.
* @param pConic      => conic to be reduced.
* @param pTranslation   <= translation to center of ellipse
* @param pInverseTranslation <= translation back to origin.
* @param pMatrix        <= matrix form
* @return true if the center has nonzero weight.  If false, the basis matrix is returned without translation.
* @group "DConic4d Local Coordinates"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDConic4d_getTranslatedRotMatrixXYW

(
DConic4dCP pConic,
RotMatrixP pTranslation,
RotMatrixP pInverseTranslation,
RotMatrixP pMatrix
)
    {
    double a;
    bool    boolstat = false;
    RotMatrix basis, translate, inverseTranslate;
    bsiDConic4d_getRotMatrixXYW (pConic, &basis);
    translate.InitIdentity ();
    inverseTranslate = translate;

    if (DoubleOps::SafeDivide (a, 1.0, pConic->center.w, 1.0))
        {
        DVec3d vec;
        pConic->center.GetXYW (vec);
        translate.SetColumn (vec, 2);
        inverseTranslate.form3d[0][2] = -translate.form3d[0][2] * a;
        inverseTranslate.form3d[1][2] = -translate.form3d[1][2] * a;
        inverseTranslate.form3d[2][2] = a;
        basis.InitProduct (inverseTranslate, basis);
        boolstat = true;
        }
#define CHECK_INVERSE_not
#ifdef CHECK_INVERSE
        {
        RotMatrix product;
        product.InitProduct (translate, inverseTranslate);
        assert (product.IsIdentity ());
        }
#endif
#undef CHECK_INVERSE
    if (pTranslation)
        *pTranslation = translate;
    if (pInverseTranslation)
        *pInverseTranslation = inverseTranslate;
    if (pMatrix)
        *pMatrix = basis;
    return boolstat;
    }


#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Get the coordinate directions and inverse of a conic viewed along the z axis.
* Same as ~mbsiDConic4d_getXYLocalFrame but ignores conic center.
* @param pConic      => conic whose orientatioin is computed.
* @param pMatrix         => matrix from (cosine, sine, z) coordinates to global.  May be NULL.
* @param pInverse       => inverse of orientation.  May be NULL.
* @return true if the requested frames were returned.
* @group "DConic4d Local Coordinates"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDConic4d_getXYLocalOrientation

(
DConic4dCP pConic,
RotMatrixP pMatrix,
RotMatrixP pInverse
)
    {
    RotMatrix matrix;
    bool    myStat = true;

    matrix.SetColumnVectors (pConic->vector0, pConic->vector90, DVecd::From (0.0, 0.0, 1.0));

    if (pMatrix)
        *pMatrix = matrix;

    if (pInverse)
        myStat = pInverse->InverseOf (matrix);

    return myStat;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Compute the local coordinates of a point in the skewed coordinates of the conic, using
* only xy parts of both the conic and starting point.
*
* This is equivalent to computing the conic plane with a line through the point and
* parallel to the z axis, and returning the coordinates of the point relative to the
* skewed axes of the conic.
* @param pConic => conic to evaluate
* @param pLocalPoint <= evaluated point.  xy are multipliers for the conic axes.
*                       z is height of the initial point from the plane of the conic.
* @param pPoint => point to project intolocal coordinates
* @group "DConic4d projection
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_pointToXYLocal

(
DConic4dCP pConic,
DPoint3dP pLocalPoint,
DPoint3dCP pPoint
)
    {
    DTransform3d frame, inverse;
    bool    myStat;
    myStat = bsiDConic4d_getXYLocalFrame (pConic, &frame, &inverse);
    if (myStat)
        {
        bsiDTransform3d_multiplyDPoint3dArray (&inverse, pLocalPoint, pPoint, 1);
        }
    return myStat;
}
#endif


/*-----------------------------------------------------------------*//**
* @description Compute the angular position of the point relative to the conic's local
* coordinates.  If the point is on the conic, this is the inverse of
* evaluating the conic at the angle.
*
* @param pConic => conic definining angular space
* @param pPoint => point to evaluate
* @return angular parameter
* @group "DConic4d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDConic4d_DPoint3dToAngle

(
DConic4dCP pConic,
DPoint3dCP pPoint
)
    {
    DMatrix4d inverse;
    DPoint4d  localPoint;
    double theta = 0.0;

    if (bsiDConic4d_getLocalFrame (pConic, NULL, &inverse))
        {
        bsiDMatrix4d_multiplyWeightedDPoint3dArray (&inverse, &localPoint, pPoint, NULL, 1);
        theta = Angle::Atan2 (localPoint.y, localPoint.x);
        }
    return theta;
}


/*-----------------------------------------------------------------*//**
* @description Compute the angular position of the homogeneous point relative to the conic's local
* coordinates.  If the point is on the conic, this is the inverse of
* evaluating the conic at the angle.
*
* @param pConic => conic definining angular space
* @param pPoint => point to evaluate
* @return angular parameter
* @group "DConic4d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDConic4d_DPoint4dToAngle

(
DConic4dCP pConic,
DPoint4dCP pPoint
)
    {
    DMatrix4d inverse;
    DPoint4d  localPoint;
    double theta = 0.0;

    if (bsiDConic4d_getLocalFrame (pConic, NULL, &inverse))
        {
        bsiDMatrix4d_multiply4dPoints (&inverse, &localPoint, pPoint, 1);
        theta = Angle::Atan2 (localPoint.y, localPoint.x);
        }
    return theta;
}


/*-----------------------------------------------------------------*//**
* @description Project a point to the plane of the conic.

* @param pConic => conic whose axes become 3d plane directions.
* @param pXYZNear <= nearest point
* @param pCoff0 <= coefficient on vector towards 0 degree point
* @param pCoff90 <= coefficient on vector towards 90 degree point
* @param pXYZ => point to project onto plane
* @return true if the plane is well defined.
* @group "DConic4d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_projectPointToPlane

(
DConic4dCP pConic,
DPoint3dP pXYZNear,
double      *pCoff0,
double      *pCoff90,
DPoint3dCP pXYZ
)
    {
    DMatrix4d localMatrix, localInverse;
    DPoint4d localXYZW, worldXYZW;
    bool    boolStat = bsiDConic4d_getLocalFrame
                    (pConic, &localMatrix, &localInverse);

    if (boolStat)
        {
        double rw;
        bsiDMatrix4d_multiplyWeightedDPoint3dArray (&localInverse, &localXYZW, pXYZ, NULL, 1);
        localXYZW.z = 0.0;
        boolStat &= DoubleOps::SafeDivide (rw, 1.0, localXYZW.w, 1.0);
        if (pCoff0)
            *pCoff0 = localXYZW.x * rw;
        if (pCoff90)
            *pCoff90 = localXYZW.y * rw;
        localXYZW.w = 1.0;
        if (pXYZ)
            {
            bsiDMatrix4d_multiply4dPoints (&localMatrix, &worldXYZW, &localXYZW, 1);
            boolStat &= worldXYZW.GetProjectedXYZ (*pXYZNear);
            }
        }
    return boolStat;
    }

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @param pConic => conic to be stroked
* @param n => default number of points on full conic
* @param nMax => max number of points on full conic
* @param tol => tolerance for stroking
* @return number of strokes required on full conic
* @group "DConic4d Stroke"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_getStrokeCount

(
DConic4dCP pConic,
int             n,
int             nMax,
double          tol
)

    {
    double r0, r90, rmax;
    int nTol;

    if (tol > 0.0)
        {
        r0 = pConic->vector0.DotProduct (pConic->vector0);
        r90 = pConic->vector90.DotProduct (pConic->vector90);

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
    else
        n = 32;
    return n;
    }
#endif


/*-----------------------------------------------------------------*//**
* @description Evaluate a conic using given coefficients for the axes.  If the x,y components
* of the coefficients define a unit vector, the point is "on" the ellispe.
* @param pConic => conic to evaluate
* @param pPoint <= cartesian points
* @param pTrig => x component of each point multiplies the
*       0 degree vector, y component multiplies the 90 degree vector.
* @param numPoint => number of pairs
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_angleParameterToTrigPairs

(
DConic4dCP pConic,
DPoint4dP pPoint,
DPoint2dCP pTrig,
int       numPoint
)
    {
    int i;
    for (i = 0; i < numPoint; i++)
        {
        pPoint[i].SumOf(pConic->center, pConic->vector0, pTrig[i].x, pConic->vector90, pTrig[i].y);
        }
    return;
    }


/*-----------------------------------------------------------------*//**
* @description Evaluate a conic at a number of (cosine, sine) pairs, removing
* pairs whose corresponding angle is not in range.
*
* @param pConic => conic to evaluate
* @param pPoint <= cartesian points
* @param pTrig => c,s pairs for evaluation
* @param numPoint => number of pairs
* @return number of points found to be in the angular range of the conic.
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_testAndEvaluateTrigPairs

(
DConic4dCP pConic,
DPoint4dP pPoint,
DPoint2dCP pTrig,
int       numPoint
)
    {
    int n = 0;
    int i;

    for (i = 0; i < numPoint; i++)
        {
        double theta = Angle::Atan2 (pTrig[i].y, pTrig[i].x);
        if (Angle::InSweepAllowPeriodShift (theta, pConic->start, pConic->sweep))
            {
            bsiDConic4d_angleParameterToTrigPairs (pConic, &pPoint[n++], &pTrig[i], 1);
            }
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
* @description Tests whether the angle is in the conic's angular range.
* @return true if the angle is in the conic's angular range.
* @param pConic => conic to evaluate
* @param angle => angle to test
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDConic4d_angleInSweep

(
DConic4dCP pConic,
double      angle
)
    {
    return Angle::InSweepAllowPeriodShift (angle, pConic->start, pConic->sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Convert an angular parameter to fraction of bounded arc length.
* @param pConic => conic whose angular range is queried.
* @param angle      => angle to convert
* @return fractional parameter.
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiDConic4d_angleParameterToFraction

(
DConic4dCP pConic,
double      angle
)
    {
    double fraction0 = bsiTrig_normalizeAngleToSweep (angle, pConic->start, pConic->sweep);
    double fraction1;
    if (bsiDConic4d_isFullSweep (pConic)
        || fraction0 <= 1.0)
        return fraction0;
    /* Normalizer says we are "beyond" the end of a bounded arc.
        Check if we are closer to start than finish -- if so take the negative sweep */
    fraction1 = bsiTrig_normalizeAngleToSweep (angle, pConic->start, -pConic->sweep);
    if (1.0 + fraction1 < fraction0)
        return -fraction1;
    return fraction0;
    }


/*-----------------------------------------------------------------*//**
* @description Get the start and end angles of the conic.
* @param pConic => conic whose angular range is queried.
* @param pStartAngle <= start angle
* @param pEndAngle <= end angle
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_getLimits

(
DConic4dCP pConic,
double    *pStartAngle,
double    *pEndAngle
)
    {
    if (pStartAngle)
        *pStartAngle = pConic->start;
    if (pEndAngle)
        *pEndAngle   = pConic->start + pConic->sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Get the start and end angles of the conic.
* @param pConic => conic whose angular range is queried.
* @param pStartAngle <= start angle
* @param pSweep <= sweep angle
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_getSweep

(
DConic4dCP pConic,
double    *pStartAngle,
double    *pSweep
)
    {
    if (pStartAngle)
        *pStartAngle = pConic->start;
    if (pSweep)
        *pSweep      = pConic->sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Set the start and end angles of the conic.
* @param pConic <= conic whose angular range is changed
* @param startAngle => start angle
* @param endAngle   => end angle
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_setLimits

(
DConic4dP pConic,
double    startAngle,
double    endAngle
)
    {
    pConic->start = startAngle;
    pConic->sweep = endAngle - startAngle;
    }


/*-----------------------------------------------------------------*//**
* @description Set the start and sweep angles of the conic.
* @param pConic <= conic whose angular range is changed
* @param startAngle => start angle
* @param sweep      => sweep angle
* @group "DConic4d Parameter Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_setSweep

(
DConic4dP pConic,
double    startAngle,
double    sweep
)
    {
    pConic->start = startAngle;
    pConic->sweep = sweep;
    }

/*-----------------------------------------------------------------*//**
* Insertion sort for small arrays of doubles.
* @bsihdr                                                       EarlinLutz      04/00
+---------------+---------------+---------------+---------------+------*/
static int insertionSortDoubles

(
double *pX,
int    n
)
    {
    int numSorted, i;
    int iMin;
    double minX;
    for (numSorted = 0; numSorted < n; numSorted++)
        {
        minX = pX[numSorted];
        iMin = numSorted;
        for (i = numSorted + 1; i < n; i++)
            {
            if (pX[i] < minX)
                {
                iMin = i;
                minX = pX[i];
                }
            }
        if (iMin > numSorted)
            {
            pX[iMin] = pX[numSorted];
            pX[numSorted] = minX;
            }
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
* @bsihdr                                                       EarlinLutz      04/00
+---------------+---------------+---------------+---------------+------*/
static bool    selectiveRangeCheck

(
DRange3dCP pRange,
DPoint3dP pPoint,
bool        clipX,
bool        clipY,
bool        clipZ,
int         excludeAxis
)
    {
    if (clipX && excludeAxis != 0)
        {
        if ((pPoint->x - pRange->low.x) * (pPoint->x - pRange->high.x) > 0.0)
            return false;
        }

    if (clipY && excludeAxis != 1)
        {
        if ((pPoint->y - pRange->low.y) * (pPoint->y - pRange->high.y) > 0.0)
            return false;
        }

    if (clipZ && excludeAxis != 2)
        {
        if ((pPoint->z - pRange->low.z) * (pPoint->z - pRange->high.z) > 0.0)
            return false;
        }

    return true;
    }


/*-----------------------------------------------------------------*//**
* @description Test if the conic is in the pleasant form with center weight 1 and vector weights 0.
* @remarks In this case the conic is simply a conic.   If this test fails, the
* curve could still be a conic but requires reparameterization and transformation
* of weights.
* @param pConic => conic to be inspected
* @return true if the conic has center weight 1 and vector weights 0.
* @group "DConic4d Weights"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_isUnitWeighted

(
DConic4dCP pConic
)
    {
    static double tol = 1.0e-12;

    return     fabs (pConic->center.w - 1.0) < tol
            && fabs (pConic->vector0.w) < tol
            && fabs (pConic->vector90.w) < tol;
    }


/*-----------------------------------------------------------------*//**
* @description Test if the conic is elliptical, albeit possibly requiring tricky reweighting and reparameterization.
* @param pConic => conic to be inspected
* @group "DConic4d Circular"
* @return true if the conic is elliptical.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDConic4d_isEllipse

(
DConic4dCP pConic
)
    {
    double w0  = pConic->vector0.w;
    double w90 = pConic->vector90.w;
    double wc  = pConic->center.w;
    static double tol = 1.0e-12;
    double wcTol = wc * (1.0 - tol);
    return w0 * w0 + w90 * w90 < wcTol * wcTol;
    }

/*-----------------------------------------------------------------*//**
* @description Negate vectors as needed so the center weight is positive (but retains absolute value)
* @param pNormalized <= copied and scaled conic
* @param pWeighted => original conic
* @group "DConic4d Weights"
* @return true unless center weight is zero
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_absCenterWeight

(
DConic4dP pNormalized,
DConic4dCP pWeighted
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

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Make a copy of the source conic, altering the axis vectors and angular limits so that
* the revised conic has perpendicular axes in the conventional major/minor axis form.
* @remarks Inputs may be the same.
* @param pConic   <= conic with perpendicular axes.
* @param pSource        => conic with unconstrained axes.
* @group "DConic4d Major Minor Axes"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initWithPerpendicularAxes

(
DConic4dP pConic,
DConic4dCP pSource
)

    {
    double   dotUV, dotUU, dotVV;
    double   c, s, theta;
    double   ax, ay, a, tol;
    DPoint3d vector0, vector90;

    dotUV = pSource->vector0.DotProduct (pSource->vector90);
    dotUU = pSource->vector0.DotProduct (pSource->vector0);
    dotVV = pSource->vector90.DotProduct (pSource->vector90);

    ay = dotUU - dotVV;
    ax = 2.0 * dotUV;
    a = dotUU + dotVV;
    tol = bsiTrig_smallAngle () * a;
    if (fabs (ax) < tol && fabs (ay) < tol)
        {
        *pConic = *pSource;
        }
    else
        {
        bsiTrig_halfAngleFunctions (&c, &s, ay, ax);
        *pConic = *pSource;
        /* Save the given axes in locals because the originals will be overwritten
            if the same conic is being used for input and output. */
        vector0 = pSource->vector0;
        vector90 = pSource->vector90;
        pConic->vector0.SumOf(*NULL, vector0, c, vector90, s);
        pConic->vector90.SumOf(*NULL, vector0, -s, vector90, c);
        theta = Angle::Atan2 (s,c);
        pConic->start -= theta;
        }
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Compute the range box of the conic in its major-minor axis coordinate system.
* Compute line segments that are the horizontal and vertical midlines in that system.
* Return those line segments ordered with the longest first, and return the shorter length.
*
* @remarks The typical use of this is that if the shorter length is less than some tolerance the
* points swept out by the conic are the longer segment.  (But beware that the start and
* end points of the segment can be other than the start and end points of the conic.)
*
* @param pConic   => conic to analyze
* @param pLongSegment  <=   longer axis of local conic range box
* @param pShortSegment <=   shorter axis of local conic range box
* @return size of the shorter dimension
* @group "DConic4d Major Minor Axes"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double           bsiDConic4d_getMajorMinorRangeMidlines

(
DConic4dCP pConic,
DSegment3dP pLongSegment,
DSegment3dP pShortSegment
)

    {
    DConic4d majorMinorEllipse;
    DRange2d localRange;
    DSegment3d segment[2];
    double xBar, yBar;
    int iLong, iShort;
    double lengthSquared[2];

    bsiDConic4d_initWithPerpendicularAxes (&majorMinorEllipse, pConic);
    bsiDConic4d_getLocalRange (&majorMinorEllipse, &localRange);

    xBar = 0.5 * (localRange.low.x + localRange.high.x);
    yBar = 0.5 * (localRange.low.y + localRange.high.y);

    bsiDConic4d_trigParameterToDPoint4d (&majorMinorEllipse, &segment[0].point[0], localRange.low.x, yBar);
    bsiDConic4d_trigParameterToDPoint4d (&majorMinorEllipse, &segment[0].point[1], localRange.high.x, yBar);

    bsiDConic4d_trigParameterToDPoint4d (&majorMinorEllipse, &segment[1].point[0], xBar, localRange.low.y);
    bsiDConic4d_trigParameterToDPoint4d (&majorMinorEllipse, &segment[1].point[1], xBar, localRange.high.y);

    lengthSquared[0] = bsiDSegment3d_lengthSquared (&segment[0]);
    lengthSquared[1] = bsiDSegment3d_lengthSquared (&segment[1]);

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
#endif


/*-----------------------------------------------------------------*//**
* @description Make a copy of the source conic, reversing the start and end angles.
* @remarks Inputs may be the same.
* @param pConic   <= copied and reversed conic
* @param pSource        => source conic
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initReversed

(
DConic4dP pConic,
DConic4dCP pSource
)
    {
    if (pConic != pSource)
        *pConic = *pSource;
    pConic->start = pConic->start + pConic->sweep;
    pConic->sweep = (-pConic->sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the magnitude of the tangent vector to the conic at the specified angle.
* @param pConic => conic to evaluate
* @param theta => angular parameter
* @return tangent magnitude
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDConic4d_tangentMagnitude

(
DConic4dCP pConic,
double      theta
)
    {
    DPoint3d tangent;
    bsiDConic4d_angleParameterToDPoint3dDerivatives (pConic, NULL, &tangent, NULL, theta);
    return tangent.Magnitude ();
    }


/*-----------------------------------------------------------------*//**
* @description Compute a cartesian ellipse whose xyz coordinates (as evaluated at some
*       parameter angle theta) are a vector in the direction of the tangent to the
*       cartesian image of the given homogeneous conic.
* @remarks Informally, the direction of
*       this vector is tangent to the cartesian image, and its length is arbitrary.
*       Formally, the vector is the numerator part of the cartesian tangent
*       expression, X'w-Xw'.
* @param pConic     => conic to evaluate
* @param pTangentEllipse   <= computed ellipse
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_pseudoTangent

(
DConic4dCP pConic,
DEllipse3dP pTangentEllipse
)
    {
    pTangentEllipse->center.WeightedDifferenceOf(pConic->vector90, pConic->vector0);
    pTangentEllipse->vector0.WeightedDifferenceOf(pConic->vector90, pConic->center);
    pTangentEllipse->vector90.WeightedDifferenceOf(pConic->center, pConic->vector0);
    pTangentEllipse->start = pConic->start;
    pTangentEllipse->sweep = pConic->sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Compute a cartesian ellipse whose xyz coordinates (as evaluated at some
*       parameter angle theta) are a vector in the direction from a (cartesian projection of a)
*       fixed (eye) point to the (cartesian projection of the) conic.
* @remarks Informally, the
*       direction of this vector is from the eye to the ellipse, and its length is arbitrary.
*       Formally, the vector is the numerator part of the cartesian difference:
*       X(theta)/X.w(theta) - Eye/Eye.w
* @param pConic             => conic to evaluate
* @param pVectorEllipse     <= computed ellipse
* @param pEyePoint          => eye point
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_pseudoVector

(
DConic4dCP pConic,
DEllipse3dP pVectorEllipse,
DPoint4dCP pEyePoint
)
    {
    pVectorEllipse->center.WeightedDifferenceOf(pConic->center, *pEyePoint);
    pVectorEllipse->vector0.WeightedDifferenceOf(pConic->vector0, *pEyePoint);
    pVectorEllipse->vector90.WeightedDifferenceOf(pConic->vector90, *pEyePoint);
    pVectorEllipse->start = pConic->start;
    pVectorEllipse->sweep = pConic->sweep;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the arc length integrand at given parameter.
*
* @indexVerb
* @bsihdr                                                       EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    cbArcLengthIntegrand

(
double  *pTangentMagnitude,
double  theta,
void    *vpEllipse
)
    {
    DConic4d  *pEllipse = (DConic4d*)(vpEllipse);
    *pTangentMagnitude = bsiDConic4d_tangentMagnitude (pEllipse, theta);
    return true;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the signed arc length of the conic.
* @remarks Negative sweep produces negative arc length, so the return from this
* can be negative.
* @param pConic   => conic to evaluate
* @return arc length of conic
* @group "DConic4d Arc Length"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDConic4d_arcLength

(
DConic4dCP pConic
)

    {
    static double s_relTol = 1.0e-12;
    int count1;
    double nc5Length, nc5Error;
    DConic4d workEllipse = *pConic;
    /*
    int count0;
    double simpLength, simpError;
    bsiMath_recursiveSimpson (&simpLength, &simpError, &count0,
                        pConic->start, pConic->start + pConic->sweep,
                        0.0, s_relTol, cbArcLengthIntegrand, &workEllipse);
    */
    bsiMath_recursiveNewtonCotes5 (&nc5Length, &nc5Error, &count1,
                        pConic->start, pConic->start + pConic->sweep,
                        0.0, s_relTol, cbArcLengthIntegrand, &workEllipse);
    return nc5Length;
    /*
    return bsiIntegral_gauss (
                    (OmdlScalarFunction)bsiDConic4d_tangentMagnitude,
                    pConic->start,
                    pConic->start + pConic->sweep,
                    msGeomConst_piOver2,
                    4,
                    (void *)pConic);
    */
    }


/*-----------------------------------------------------------------*//**
* @description Compute the xyz range limits of a homogeneous conic.
* @remarks Warning: If there is an asymptote in range, results are unpredictable.
* @param pConic => conic whose range is determined
* @param pRange <= computed range
* @group "DConic4d Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_getRange

(
DConic4dCP pConic,
DRange3dP pRange
)
    {
    /* Candidate points that can contribute to the range limits are:
        1) sector endpoints
        2) local extrema
    */
    if (bsiDConic4d_isFullSweep (pConic))
        {
        /* The DConic4d is a complete complete true ellipse.    Only local extrema need to
            be considered, and they do not need to have angular range tests applied. */
        /* Take the center as an initial range limit. */
        pConic->center.GetProjectedXYZ (pRange->low);
        pRange->high = pRange->low;

        bsiTrig_extendComponentRange (&pRange->low.x, &pRange->high.x,
                pConic->center.x, pConic->vector0.x, pConic->vector90.x,
                pConic->center.w, pConic->vector0.w, pConic->vector90.w,
                0.0, 0.0, false);

        bsiTrig_extendComponentRange (&pRange->low.y, &pRange->high.y,
                pConic->center.y, pConic->vector0.y, pConic->vector90.y,
                pConic->center.w, pConic->vector0.w, pConic->vector90.w,
                0.0, 0.0, false);

        bsiTrig_extendComponentRange (&pRange->low.z, &pRange->high.z,
                pConic->center.z, pConic->vector0.z, pConic->vector90.z,
                pConic->center.w, pConic->vector0.w, pConic->vector90.w,
                0.0, 0.0, false);

        }
    else
        {
        DPoint3d startPoint, endPoint;
        bsiDConic4d_getDPoint3dEndPoints (pConic, &startPoint, &endPoint);

        /* Test endpoints */
        pRange->InitFrom(startPoint, endPoint);
        /* Test local extrema */
        bsiTrig_extendComponentRange (&pRange->low.x, &pRange->high.x,
                pConic->center.x, pConic->vector0.x, pConic->vector90.x,
                pConic->center.w, pConic->vector0.w, pConic->vector90.w,
                pConic->start, pConic->sweep, true);

        bsiTrig_extendComponentRange (&pRange->low.y, &pRange->high.y,
                pConic->center.y, pConic->vector0.y, pConic->vector90.y,
                pConic->center.w, pConic->vector0.w, pConic->vector90.w,
                pConic->start, pConic->sweep, true);

        bsiTrig_extendComponentRange (&pRange->low.z, &pRange->high.z,
                pConic->center.z, pConic->vector0.z, pConic->vector90.z,
                pConic->center.w, pConic->vector0.w, pConic->vector90.w,
                pConic->start, pConic->sweep, true);
        }
    }


/*-----------------------------------------------------------------*//**
* @description Compute the range of the conic in its own coordinate system.
* @remarks This depends on the start and sweep angles but not the center or axis coordinates.
* @param pConic => conic whose range is determined
* @param pRange <= computed range
* @group "DConic4d Range"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_getLocalRange

(
DConic4dCP pConic,
DRange2dP pRange
)
    {
    DRange3d xyzRange;
    bsiDRange3d_initFromUnitArcSweep (&xyzRange, pConic->start, pConic->sweep);
    pRange->low.x = xyzRange.low.x;
    pRange->low.y = xyzRange.low.y;
    pRange->high.x = xyzRange.high.x;
    pRange->high.y = xyzRange.high.y;
    }


/*-----------------------------------------------------------------*//**
* @description Find intersections of a (full) conic with a plane.
* @remarks Return value n=1 is a single tangency point returned in pTrigPoints[0];
*       n=2 is two simple intersections returned in pTrigPoints[0..1].
* @remarks The three component values in pTrigPoints are:
*<UL>
*<LI>x == cosine of angle
*<LI>y == sine of angle
*<LI>z == angle in radians
*</UL>
* @param pConic      => conic to intersect with plane
* @param pTrigPoints    <= 2 points: cosine, sine, theta values of plane intersection
* @param pPlane         => homogeneous plane equation
* @return The number of intersections, i.e. 0, 1, or 2.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectPlane

(
DConic4dCP pConic,
DPoint3dP pTrigPoints,
DPoint4dCP pPlane
)
    {
    double alpha = pPlane->DotProduct (pConic->center);
    double beta  = pPlane->DotProduct (pConic->vector0);
    double gamma = pPlane->DotProduct (pConic->vector90);

    double mag = sqrt (beta * beta + gamma * gamma);
    int n;


    if  (   mag == 0.0
        || !DoubleOps::SafeDivide (alpha, alpha, mag, 0.0)
        )
        return 0;
    beta /= mag;
    gamma /= mag;

    n = bsiMath_solveApproximateUnitQuadratic (
                &pTrigPoints[0].x, &pTrigPoints[0].y,
                &pTrigPoints[1].x, &pTrigPoints[1].y,
                alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance
                );
    if ( n == 1 )
        {
        /* Take the on-circle solution */
        pTrigPoints[0].x = pTrigPoints[1].x;
        pTrigPoints[0].y = pTrigPoints[1].y;
        pTrigPoints[0].z = Angle::Atan2 ( pTrigPoints[0].y, pTrigPoints[0].x );
        }
    else if ( n == 2 )
        {
        pTrigPoints[0].z = Angle::Atan2 ( pTrigPoints[0].y, pTrigPoints[0].x );
        pTrigPoints[1].z = Angle::Atan2 ( pTrigPoints[1].y, pTrigPoints[1].x );
        }
    else
        {
        /* suppress the degenerate cases */
        n = 0;
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
* @description Find the angles at which the conic becomes infinite.
* @param pConic => conic to evaluate
* @param pTrigPoints <= array of 0, 1 or 2  (cosine, sine, theta) triples.
* @group "DConic4d Asymptotes"
* @return Number of singular angles (0, 1 or 2).
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_singularAngles

(
DConic4dCP pConic,
DPoint3dP pTrigPoints
)

    {
    int n = bsiMath_solveApproximateUnitQuadratic (
                &pTrigPoints[0].x, &pTrigPoints[0].y,
                &pTrigPoints[1].x, &pTrigPoints[1].y,
                pConic->center.w,
                pConic->vector0.w,
                pConic->vector90.w,
                s_lineUnitCircleIntersectionTolerance
                );
    if ( n == 1 )
        {
        /* Take the on-circle solution */
        pTrigPoints[0].x = pTrigPoints[1].x;
        pTrigPoints[0].y = pTrigPoints[1].y;
        pTrigPoints[0].z = pTrigPoints[1].z = Angle::Atan2 ( pTrigPoints[0].x, pTrigPoints[0].y );
        }
    else if ( n == 2 )
        {
        pTrigPoints[0].z = Angle::Atan2 ( pTrigPoints[0].y, pTrigPoints[0].x );
        pTrigPoints[1].z = Angle::Atan2 ( pTrigPoints[1].y, pTrigPoints[1].x );
        }
    else
        {
        /* suppress the degenerate cases */
        n = 0;
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
* @description Build an array containing all angles at which the conic starts, ends, or
* has a simple intersection with a range box.
* @param pConic         => conic to evaluate
* @param pAngleArray    <= array of fractional parameters where crossings occur.
* @param pInOutArray    <= flag pInOutArray[i] indicates whether the interval from pAngleArray[i] to
*                               pAngleArray[i+1] is "inside" (nonzero) or "outside" (zero) the range limits.
* @param pNumOut        <= number of angles returned
* @param maxOut         => max allowed number of angles.  (Size of output arrays. Suggested size is 14)
* @param pRange         => clip range
* @param clipX          => true to enable x direction tests
* @param clipY          => true to enable y direction tests
* @param clipZ          => true to enable z direction tests
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      04/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void         bsiDConic4d_intersectDRange3d

(
DConic4dCP pConic,
double        *pAngleArray,
int           *pInOutArray,
int           *pNumOut,
int           maxOut,
DRange3dCP pRange,
bool          clipX,
bool          clipY,
bool          clipZ
)
    {
    const double *pCorner[2];
    const double  *pCenter, *pVector0, *pVector90;
    double fractionArray[20];
    double thetaArray[20];
    int i, corner, root, numRoot, numSaved;
    bool    clipSelect[3];
    DPoint2d trigPoints[2];
    DPoint4d hPoint;
    DPoint3d point;
    //double *pPoint = (double *)&point;
    double alpha, beta, gamma, theta, fraction;
    bool    goodPoint;

    clipSelect[0]   = clipX;
    clipSelect[1]   = clipY;
    clipSelect[2]   = clipZ;
    pCorner[0]      = (const double *)&pRange->low;
    pCorner[1]      = (const double *)&pRange->high;
    pCenter         = (const double *)&pConic->center;
    pVector0        = (const double *)&pConic->vector0;
    pVector90       = (const double *)&pConic->vector90;


    numSaved = 0;
    if (pRange->IsNull () || !(clipX || clipY || clipZ))
        {
        /* No intersections that matter. */
        }
    else
        {
        for (i = 0; i < 3; i++)
            {
            if (clipSelect[i])
                {
                for (corner = 0; corner < 2; corner++)
                    {
                    alpha = pCenter[i]   - pCorner [corner][i] * pCenter[3];
                    beta  = pVector0[i]  - pCorner [corner][i] * pVector0[3];
                    gamma = pVector90[i] - pCorner [corner][i] * pVector90[3];
                    numRoot = bsiMath_solveApproximateUnitQuadratic (
                                &trigPoints[0].x, &trigPoints[0].y,
                                &trigPoints[1].x, &trigPoints[1].y,
                                alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance );
                    if ( numRoot == 2 )
                        {
                        for (root = 0; root < numRoot; root++)
                            {
                            /* Clip against the other directons */
                            goodPoint = true;
                            hPoint.SumOf(pConic->center, pConic->vector0, trigPoints[root].x, pConic->vector90, trigPoints[root].y);

                            if (hPoint.GetProjectedXYZ (point))
                                {
                                if (selectiveRangeCheck (pRange, &point, clipX, clipY, clipZ, i))
                                    {
                                    theta = atan2 (trigPoints[root].y, trigPoints[root].x);
                                    fraction = bsiTrig_normalizeAngleToSweep (theta, pConic->start, pConic->sweep);
                                    if (0.0 < fraction && fraction < 1.0)
                                        fractionArray[numSaved++] = fraction;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    fractionArray[numSaved++] = 0.0;
    fractionArray[numSaved++] = 1.0;

    insertionSortDoubles (fractionArray, numSaved);

    for (i = 0; i < numSaved; i++)
        {
        thetaArray[i] = pConic->start + fractionArray[i] * pConic->sweep;
        }
    if (numSaved > maxOut)
        numSaved = maxOut;
    if (pAngleArray)
        memcpy (pAngleArray, thetaArray, numSaved * sizeof(double));
    if (pInOutArray)
        {
        for (i = 1; i < numSaved; i++)
            {
            theta = 0.5 * (thetaArray[i-1] + thetaArray[i]);
            pInOutArray[i-1] = 0;
            if (   bsiDConic4d_angleParameterToDPoint3d (pConic, &point, theta)
                && selectiveRangeCheck (pRange, &point, clipX, clipY, clipZ, -1)
                )
                {
                pInOutArray[i-1] = 1;
                }
            }
        }
    if (pNumOut)
        *pNumOut = numSaved;
    }




/*-----------------------------------------------------------------*//**
* @description Find the apparent intersections of (perspective) xy projections of a conic and line.
* @remarks May return 0, 1, or 2 points.  Both conic and line are unbounded
*
* @param pConic             => conic to intersect with line
* @param pConicPoints       <= points on conic
* @param pConicAngles       <= angles on conic
* @param pLinePoints        <= points on line
* @param pLineParams        <= parameters on line
* @param pSegment           => line to intersect with conic
* @return the number of intersections, i.e. 0, 1, or 2
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectDSegment4dXYW

(
DConic4dCP pConic,
DPoint4dP pConicPoints,
double        *pConicAngles,
DPoint4dP pLinePoints,
double        *pLineParams,
DSegment4dCP pSegment
)
    {
    DPoint4d planeCoffs;
    DPoint3d trigPoints[2];

    DPoint4d globalConicPoint;
    DPoint4d localConicPoint;
    int numAngle;
    int i;
    DMatrix4d worldToLocal;
    DSegment4d localSegment;
    DConic4d   localConic;

    bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, NULL, &pSegment->point[0]);
    localSegment.InitProduct (worldToLocal, *pSegment);
    bsiDConic4d_applyDMatrix4d (&localConic, &worldToLocal, pConic);

    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&localSegment, &planeCoffs);
    numAngle = bsiDConic4d_intersectPlane (&localConic, trigPoints, &planeCoffs);

    for (i = 0; i < numAngle; i++)
        {
        if (pConicAngles)
            pConicAngles[i] = trigPoints[i].z;

        globalConicPoint.SumOf(pConic->center, pConic->vector0, trigPoints[i].x, pConic->vector90, trigPoints[i].y);

        if (pConicPoints)
            pConicPoints[i] = globalConicPoint;

        if (pLinePoints || pLineParams)
            {
            double segmentParam;
            localConicPoint.SumOf(localConic.center, localConic.vector0, trigPoints[i].x, localConic.vector90, trigPoints[i].y);
            DPoint4d localLinePoint;
            localSegment.ProjectPointUnboundedCartesianXYW (localLinePoint, segmentParam, localConicPoint);
            if (pLineParams)
                pLineParams[i] = segmentParam;
            if (pLinePoints)
                pSegment->FractionParameterToPoint (pLinePoints[i], segmentParam);
            }
        }
    return numAngle;
    }

/*-----------------------------------------------------------------*//**
* Find the apparent intersections of (perspective) xy projections of two conics
* given as xyw matrices.  Conic 0 is assumed invertible.  Expected to be called with
* conic0 as the better condition number matrix.
*
* Columns of B are coefficients for (cos) (sin) (1)
*
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int intersectConicConic

(
DPoint4dP pConic0Points,
double        *pConic0Angles,
DPoint4dP pConic1Points,
double        *pConic1Angles,
DConic4dCP pConic0,
RotMatrixCP pB0,
RotMatrixCP pB0inv,
DConic4dCP pConic1,
RotMatrixCP pB1
)
    {
    RotMatrix C;
    double c1[4], s1[4], c0[4], s0[4], angle0[4], angle1[4];
    DPoint3d F;
    int numRoot;
    int i;
    int numInt;

    C.InitProduct (*pB0inv, *pB1);

    numRoot = bsiMath_conicIntersectUnitCircle
                            (
                            c1, s1, angle1, &numInt,
                            C.form3d[0][2], C.form3d[0][0], C.form3d[0][1],
                            C.form3d[1][2], C.form3d[1][0], C.form3d[1][1],
                            C.form3d[2][2], C.form3d[2][0], C.form3d[2][1]);

    if (numRoot < 0)
        numRoot = 0;

    /* Get the angles and trig values on conic0 */
    for (i = 0; i < numRoot; i++)
        {
        C.MultiplyComponents(F, c1[i], s1[i], 1.0);
        /* Back out the angles.  Just evaluate the trigs rather than worry
            about whether there is a divide by zero if you normalize from the
            nominal solution vector */
        if (F.z < 0.0)
            F.Negate();
        angle0[i] = Angle::Atan2 (F.y, F.x);
        c0[i] = cos (angle0[i]);
        s0[i] = sin (angle0[i]);

        if (pConic0Points)
            pConic0Points[i].SumOf(pConic0->center, pConic0->vector0, c0[i], pConic0->vector90, s0[i]);
        if (pConic0Angles)
            pConic0Angles[i] = angle0[i];

        if (pConic1Points)
            pConic1Points[i].SumOf(pConic1->center, pConic1->vector0, c1[i], pConic1->vector90, s1[i]);
        if (pConic1Angles)
            pConic1Angles[i] = angle1[i];

        }
    return numRoot;
    }

static void bsiDConic4d_evaluteTrigPointToArrays
(
DConic4dCR conic,
DPoint4dP  points,
double     *angles,
int        &count, // REQUIRED
DPoint3dCR    trigPoint
)
    {
    if (NULL != angles)
        angles[count] = trigPoint.z;
    if (NULL != points)
        points[count].SumOf (conic.center, conic.vector0, trigPoint.x, conic.vector90, trigPoint.y);
    count++;
    }

static bool bsiDConic4d_getPlane
(
DConic4dCP conic,
DPoint4dP plane
)
    {
    DVec3d vector0, vector90;
    vector0.WeightedDifferenceOf(conic->vector0, conic->center);
    vector90.WeightedDifferenceOf(conic->vector90, conic->center);
    return plane->InitPlaneFromDPoint4dDVec3dDVec3d (conic->center, vector0, vector90);
    }



/*-----------------------------------------------------------------*//**
* @description Find the apparent intersections of (perspective) xy projections of two conics.
* @remarks May return 0 to 4 points.  Both conics are unbounded.
* @param pConic         => first conic
* @param pConic0Points  <= points on conic0
* @param pConic0Angles  <= angles on conic0
* @param pConic1Points  <= points on conic1
* @param pConic1Angles  <= angles on conic1
* @param pConic1        => second conic
* @return the number of intersections, i.e. 0 to 4
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectDConic4dXYW

(
DConic4dCP pConic,
DPoint4dP pConic0Points,
double        *pConic0Angles,
DPoint4dP pConic1Points,
double        *pConic1Angles,
DConic4dCP pConic1
)
    {
    RotMatrix B0, B1;
    RotMatrix B0inv, B1inv;

    RotMatrix T0, T1, T0inv, T1inv;
    RotMatrix C;
    int numRoot = 0;
    double condition0, condition1;
    bool    bool0, bool1;

    bsiDConic4d_getTranslatedRotMatrixXYW (pConic, &T0, &T0inv, &B0);
    bsiDConic4d_getTranslatedRotMatrixXYW (pConic1, &T1, &T1inv, &B1);

    condition0 = B0.ConditionNumber ();
    condition1 = B1.ConditionNumber ();

    bool0 = B0inv.InverseOf (B0);
    bool1 = B1inv.InverseOf (B1);

    // If one conic is really large, bezier roots come out bad.
    //   In obvious cases look for the smaller ellipse to implicitize...
    DEllipse3d ellipse0, ellipse1;
    if (condition0 > 0.25 && condition1 > 0.25
        && DEllipse3d_initFromDConic4d (&ellipse0, pConic)
        && DEllipse3d_initFromDConic4d (&ellipse1, pConic1))
        {
        // Choose smaller ellipse as circle frame ...
        double size0 = ellipse0.vector0.Magnitude ()
                     + ellipse0.vector0.Magnitude ();
        double size1 = ellipse1.vector90.Magnitude ()
                     + ellipse1.vector90.Magnitude ();
        if (size0 > size1)
            {
            condition0 = condition1 + 1.0;
            }
        else
            {
            condition1 = condition0 + 1.0;
            }
        }

    if (bool0 || bool1)
        {
        /* The ellipses parameterizations are
                    X0 = T0 * B0 * F0
                    X1 = T1 * B1 * F1
            Equate X0 and X1 within scale factor, translating to origin of
                better conditioned axes.  For instance,
                    T0 * B0 * F0 ~ T1 * B1 * F1
                         B0 * F0 ~ (T0inv * T1) * B1 * F1
            and the final intersection is done wrt bases B0 and C = (T0inv * T1) * B1
        */
        if (condition0 > condition1)
            {
            C.InitProduct (T0inv, T1);
            C.InitProduct (C, B1);
            numRoot = intersectConicConic
                                    (
                                    pConic0Points,
                                    pConic0Angles,
                                    pConic1Points,
                                    pConic1Angles,
                                    pConic,
                                    &B0,
                                    &B0inv,
                                    pConic1,
                                    &C
                                    );
            }
        else
            {
            C.InitProduct (T1inv, T0);
            C.InitProduct (C, B0);
            numRoot = intersectConicConic
                                    (
                                    pConic1Points,
                                    pConic1Angles,
                                    pConic0Points,
                                    pConic0Angles,
                                    pConic1,
                                    &B1,
                                    &B1inv,
                                    pConic,
                                    &C
                                    );
            }
        }
    else
        {
        DPoint4d plane0, plane1;
        // Both conics are edge-on.
        // Intersect each with the plane of the other.
        // The intersection of the planes is towards the eye.
        // Every combination of an intersection angle on conic0 with one on conic1 is a visible intersection.
        if (   bsiDConic4d_getPlane (pConic, &plane0)
            && bsiDConic4d_getPlane (pConic1, &plane1))
            {
            DPoint3d trigPoints0[2], trigPoints1[2];
            int numIntersection0 = bsiDConic4d_intersectPlane (pConic, trigPoints0, &plane1);
            int numIntersection1 = bsiDConic4d_intersectPlane (pConic1, trigPoints1, &plane0);
            if (numIntersection0 > 0 && numIntersection1 > 0)
                {
                int numOut0 = 0;
                int numOut1 = 0;    // counts will increment together
                for (int i0 = 0; i0 < numIntersection0; i0++)
                    {
                    for (int i1 = 0; i1 < numIntersection1; i1++)
                        {
                        bsiDConic4d_evaluteTrigPointToArrays (*pConic,  pConic0Points, pConic0Angles, numOut0, trigPoints0[i0]);
                        bsiDConic4d_evaluteTrigPointToArrays (*pConic1, pConic1Points, pConic1Angles, numOut1, trigPoints1[i1]);
                        }
                    }
                numRoot = numOut0;
                }
            }
        }
    return numRoot;
    }

static double s_circularConicRelTol = 1.0e-12;

/*-----------------------------------------------------------------*//**
* @description Test if the conic is circular.
* @param pConic => conic to test
* @return true if circular
* @group "DConic4d Circular"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_isCircular

(
DConic4dCP pConic
)

    {
    double relTol = bsiTrig_smallAngle();

    double  dotUV, magU, magV;
    double wTol = s_circularConicRelTol  = fabs (pConic->center.w);

    if (    fabs (pConic->vector0.w) < wTol
        &&  fabs (pConic->vector90.w) < wTol
        )
        {
        dotUV = pConic->vector0.DotProductXYZ (pConic->vector90);
        magU = sqrt (pConic->vector0.DotProductXYZ (pConic->vector0));
        magV = sqrt (pConic->vector90.DotProductXYZ (pConic->vector90));

        /* Circular if the axes have the same magnitude and are perpendicular */
        if (fabs(magU - magV) < (magU + magV) * relTol)
            {
            if (fabs(dotUV) < magU * magV * relTol)
                return  true;
            }
        }
    return  false;
    }


/*-----------------------------------------------------------------*//**
* @description Test if the XY projection of the conic is circular.
* @param pConic => conic to test
* @return true if circular
* @group "DConic4d Circular"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDConic4d_isCircularXY

(
DConic4dCP pConic
)
    {
    DConic4d conic = *pConic;
    conic.center.z = conic.vector0.z = conic.vector90.z = 0.0;
    return bsiDConic4d_isCircular (&conic);
    }

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* Find the intersections of xy projections of two ellipses defined by coordinate
* frames.   Both are unbounded.   Prefers inverse of first frame.
* @return the number of intersections.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int intersectXYEllipseFrames

(
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Params,
DPoint3dP pEllipse1Params,
DTransform3dCP pFrame0,
DTransform3dCP pFrame1
)
    {
    DTransform3d inverseFrame;
    DTransform3d localFrame1;
    double cosValue[4], sinValue[4], thetaValue[4];
    double c1, s1;
    double c0, s0;
    double ax, bx, cx;
    double ay, by, cy;
    int numInt = 0;
    int i;

    if  (bsiDTransform3d_invert (&inverseFrame, pFrame0))
        {
        bsiDTransform3d_multiplyTransformTransform (&localFrame1, &inverseFrame, pFrame1);
        /* In the local coordinates of ellipse0, ellipse0 itself is a unit circle.
           Ellipse 1 is not.
        */
        ax = localFrame1.GetPointComponent (0);
        bx = localFrame1.GetFromMatrixByRowAndColumn[0][0];
        cx = localFrame1.GetFromMatrixByRowAndColumn[0][1];

        ay = localFrame1.GetPointComponent (1);
        by = localFrame1.GetFromMatrixByRowAndColumn[1][0];
        cy = localFrame1.GetFromMatrixByRowAndColumn[1][1];

        bsiMath_conicIntersectUnitCircle (cosValue, sinValue, thetaValue, &numInt,
                                            ax,  bx,  cx,
                                            ay,  by,  cy,
                                            1.0, 0.0, 0.0
                                            );

        for (i = 0; i < numInt; i++)
            {
            c1 = cosValue[i];
            s1 = sinValue[i];

            if  (pCartesianPoints)
                {
                pCartesianPoints[i] = DPoint3d::FromProduct (pFrame1, c1, s1, 0.0);
                }

            if  (pEllipse0Params)
                {
                c0 = ax + bx * c1 + cx * s1;
                s0 = ay + by * c1 + cy * s1;
                pEllipse0Params[i].Init ( c0, s0, 0.0);
                }

            if  (pEllipse1Params)
                {
                pEllipse1Params[i].Init ( c1, s1, 0.0);
                }

            }
        }
    return  numInt;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Find the intersections of xy projections of two ellipses
* @remarks Both ellipses are unbounded.
* @param pConic             => first conic
* @param pCartesianPoints   <= cartesian intersection points.
* @param pConicParams    <= array of coordinates relative to the instance
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pEllipseParams  <= array of coordinates relative to the conic.
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pEllipse       => second conic
* @return the number of intersections.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectXYDConic4d

(
DConic4dCP pConic,
DPoint3dP pCartesianPoints,
DPoint3dP pConicParams,
DPoint3dP pEllipseParams,
DConic4dCP pEllipse
)
    {
    DTransform3d instanceFrame, ellipseFrame;
    double instanceCondition, ellipseCondition;
    int numUnbounded = 0;

    bsiDConic4d_getXYLocalFrame (pConic, &instanceFrame, NULL);
    instanceCondition = instanceFrame.matrix.ConditionNumber ();

    bsiDConic4d_getXYLocalFrame (pEllipse, &ellipseFrame, NULL);
    ellipseCondition = ellipseFrame.matrix.ConditionNumber ();

    if  (instanceCondition > ellipseCondition)
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pConicParams, pEllipseParams,
                                &instanceFrame, &ellipseFrame);
        }
    else
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pEllipseParams,   pConicParams,
                                &ellipseFrame,    &instanceFrame);
        }
    return  numUnbounded;
    }
#endif

#ifdef NEEDS_WORK
/*---------------------------------------------------------------------------------**//**
* Remove points not contained in the sweep ranges of (both of) two ellipses.
* @bsihdr                                                       EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static int filterDualSweeps

(
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Coffs,
double        *pEllipse0Angle,
DPoint3dP pEllipse1Coffs,
double        *pEllipse1Angle,
DConic4dCP pEllipse0,
DConic4dCP pEllipse1,
DPoint3dCP pCartesianInPoint,
DPoint3dCP pEllipse0InCoffs,
DPoint3dCP pEllipse1InCoffs,
int           numUnBounded
)
    {
    int numBounded = 0;
    int i;
    double theta0, theta1;
    for (i = 0; i < numUnBounded ; i++)
        {
        theta0  = Angle::Atan2 (pEllipse0InCoffs[i].y, pEllipse0InCoffs[i].x);
        theta1  = Angle::Atan2 (pEllipse1InCoffs[i].y, pEllipse1InCoffs[i].x);

        if  (
                bsiDConic4d_angleInSweep (pEllipse0, theta0)
             && bsiDConic4d_angleInSweep (pEllipse1, theta1)
            )
            {
            if  (pCartesianPoints)
                pCartesianPoints[numBounded] = pCartesianInPoint[i];

            if  (pEllipse0Coffs)
                pEllipse0Coffs[numBounded]   = pEllipse0Coffs[i];

            if  (pEllipse0Angle)
                pEllipse0Angle[numBounded]   = theta0;

            if  (pEllipse1Coffs)
                pEllipse1Coffs[numBounded]    = pEllipse1Coffs[i];

            if  (pEllipse1Angle)
                pEllipse1Angle[numBounded]    = theta1;

            numBounded++;
            }
        }
    return numBounded;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Find the intersections of xy projections of two ellipses, with bounds applied.
* @param pConic      => first conic
* @param pCartesianPoints   <= cartesian intersection points.
* @param pConicCoffs    <= array of coordinates relative to the first conic
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pConicAngle    <= array of angles on first conic
* @param pEllipseCoffs  <= array of coordinates relative to the second conic.
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pEllipseAngle  <= array of angles on second conic
* @param pEllipse       => second conic
* @return the number of intersections.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectXYDConic4dBounded

(
DConic4dCP pConic,
DPoint3dP pCartesianPoints,
DPoint3dP pConicCoffs,
double        *pConicAngle,
DPoint3dP pEllipseCoffs,
double        *pEllipseAngle,
DConic4dCP pEllipse
)
    {
    DPoint3d cartesianPoint[4], ellipseCoffs[4], instanceCoffs[4];
    int numUnBounded;
    int numBounded;

    numUnBounded = bsiDConic4d_intersectXYDConic4d (pConic,
                            cartesianPoint, instanceCoffs, ellipseCoffs, pEllipse);

    numBounded = filterDualSweeps (
                        pCartesianPoints,
                        pConicCoffs, pConicAngle,
                        pEllipseCoffs, pEllipseAngle,
                        pConic,
                        pEllipse,
                        cartesianPoint, instanceCoffs, ellipseCoffs, numUnBounded);
    return  numBounded;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Find "intersections" of two DConic4d.   Ellipses in space can pass very close to
* each other without intersecting, so some logic must be applied to define intersection
* more cleanly. The logic applied is to choose the more circular conic and apply the
* transformation which makes that one a unit circle, then intersect the xy projections of the
* transformations.

* @param pConic         => first conic
* @param pCartesianPoints   <= cartesian intersection points.
* @param pConicParams    <= array of coordinates relative to the instance
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pEllipseParams     <= array of coordinates relative to the conic.
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pEllipse       => second conic
* @return the number of intersections.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectSweptDConic4d

(
DConic4dCP pConic,
DPoint3dP pCartesianPoints,
DPoint3dP pConicParams,
DPoint3dP pEllipseParams,
DConic4dCP pEllipse
)
    {
    DTransform3d instanceFrame, ellipseFrame;
    double instanceCondition, ellipseCondition;
    int numUnbounded = 0;

    bsiDConic4d_getLocalFrame (pConic, &instanceFrame, NULL);
    instanceCondition = instanceFrame.matrix.ConditionNumber ();

    bsiDConic4d_getLocalFrame (pEllipse, &ellipseFrame, NULL);
    ellipseCondition = ellipseFrame.matrix.ConditionNumber ();

    if  (instanceCondition > ellipseCondition)
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pConicParams, pEllipseParams,
                                &instanceFrame, &ellipseFrame);
        }
    else
        {
        numUnbounded = intersectXYEllipseFrames (
                                pCartesianPoints,
                                pEllipseParams,   pConicParams,
                                &ellipseFrame,    &instanceFrame);
        }
    return  numUnbounded;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Intersect two ellipses as described in ~mbsiDConic4d_intersectSweptDConic4d, and
* filter out results not part of both ranges.
*
* @param pConic         => first conic
* @param pCartesianPoints   <= cartesian intersection points

* @param pConicCoffs    <= array of coordinates relative to the first conic
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pConicAngle    <= array of angles on first conic
* @param pEllipseCoffs  <= array of coordinates relative to the second conic.
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pEllipseAngle  <= array of angles on second conic
* @param pEllipse       => second conic
* @return the number of intersections.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectSweptDConic4dBounded

(
DConic4dCP pConic,
DPoint3dP pCartesianPoints,
DPoint3dP pConicCoffs,
double        *pConicAngle,
DPoint3dP pEllipseCoffs,
double        *pEllipseAngle,
DConic4dCP pEllipse
)
    {
    DPoint3d cartesianPoint[4], ellipseCoffs[4], instanceCoffs[4];
    int numUnBounded;
    int numBounded;

    numUnBounded = bsiDConic4d_intersectSweptDConic4d (pConic,
                            cartesianPoint, instanceCoffs, ellipseCoffs, pEllipse);

    numBounded = filterDualSweeps (
                                    pCartesianPoints,
                                    pConicCoffs, pConicAngle,
                                    pEllipseCoffs, pEllipseAngle,
                                    pConic, pEllipse,
                                    cartesianPoint,
                                    instanceCoffs,
                                    ellipseCoffs,
                                    numUnBounded);
    return  numBounded;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Find "intersections" of a line and a DConic4d.   Curves in space can pass very close to
* each other without intersecting, so some logic must be applied to define intersection
* more cleanly. The logic applied is to compute the intersection of the line with
* the cylinder swept by the conic along its plane normal.
*
* @param pConic      => base conic for the cylinder.
* @param pPointArray   <= cartesian intersection points.
* @param pConicParams     <= array of coordinates relative to the instance
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pLineParams     <= array of parametric coordinates on the line.
* @param pSegment       => the (infinite) line
* @return the number of intersections.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectSweptDSegment3d

(
DConic4dCP pConic,
DPoint3dP pPointArray,
DPoint3dP pConicParams,
double        *pLineParams,
DSegment3dCP pSegment
)
    {
    int             i, numUnbounded = 0;
    double          ellipseCondition, lambda0[2], lambda1[2];
    DSegment3d      localSegment;
    DTransform3d    ellipseFrame, ellipseInverse;
    static double   s_conditionTol = 1.0e-8;

    bsiDConic4d_getLocalFrame (pConic, &ellipseFrame, &ellipseInverse);
    ellipseCondition = ellipseFrame.matrix.ConditionNumber ();

    if (ellipseCondition > s_conditionTol)
        {
        double aa, ab, bb;
        bsiDSegment3d_transform (&localSegment, &ellipseInverse, pSegment);
        aa = localSegment.point[0].DotProductXY (*(&localSegment.point[0])) - 1.0;
        ab = localSegment.point[0].DotProductXY (*(&localSegment.point[1])) - 1.0;
        bb = localSegment.point[1].DotProductXY (*(&localSegment.point[1])) - 1.0;

        numUnbounded = bsiMath_solveConvexQuadratic (lambda0, lambda1, aa, 2.0 * ab, bb);

        for (i = 0; i < numUnbounded; i++)
            {
            if (pLineParams)
                pLineParams[i] = lambda1[i];

            if (pConicParams)
                pConicParams[i].SumOf(*NULL, *(&localSegment.point[0]), lambda0[i], *(&localSegment.point[1]), lambda1[i]);
            if (pPointArray)
                pPointArray[i].SumOf(*NULL, *(&pSegment->point[0]), lambda0[i], *(&pSegment->point[1]), lambda1[i]);
            }
        }
    return  numUnbounded;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Find "intersections" of a line and a (bounded) DConic4d.   Curves in space can pass very close to
* each other without intersecting, so some logic must be applied to define intersection
* more cleanly. The logic applied is to compute the intersection of the line with
* the cylinder swept by the conic along its plane normal.
*
* @param pConic      => base conic for the cylinder.
* @param pPointArray   <= cartesian intersection points.
* @param pConicParams     => array of coordinates relative to the instance
*                           For each point, (xy) are the cosine and sine of the
*                           conic parameter, (z) is z distance from the plane of
*                           of the conic.
* @param pLineParams     => array of parametric coordinates on the line.
* @param pSegment       => the (infinite) line
* @return the number of intersections.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectSweptDSegment3dBounded

(
DConic4dCP pConic,
DPoint3dP pPointArray,
DPoint3dP pConicParams,
double        *pLineParams,
DSegment3dCP pSegment
)
    {
    DPoint3d pointArray[2];
    double lineParam[2];
    DPoint3d ellipseParam[2];
    int numBounded = 0;
    int i;
    static double s_lineTol = 1.0e-12;
    int numUnbounded = bsiDConic4d_intersectSweptDSegment3d
                                (
                                pConic,
                                pointArray,
                                ellipseParam,
                                lineParam,
                                pSegment
                                );

    for (i = 0; i < numUnbounded; i++)
        {
        if (lineParam[i] >= - s_lineTol && lineParam[i] <= 1.0 + s_lineTol)
            {
            double theta = Angle::Atan2 (ellipseParam[i].y, ellipseParam[i].x);
            if (bsiDConic4d_angleInSweep (pConic, theta))
                {
                if (pPointArray)
                    pPointArray[numBounded] = pointArray[i];

                if (pLineParams)
                    pLineParams[numBounded] = lineParam[i];

                if (pConicParams)
                    pConicParams[numBounded] = ellipseParam[i];

                numBounded++;
                }
            }
        }

    return  numBounded;
    }
#endif


/*-----------------------------------------------------------------*//**
* @description Project a point to the unbounded xy projection of the conic.
* @remarks May return up to 4 points and angles.
* @param pConic => conic to evaluate
* @param pXYZOut <= array of xyz coordinates of projections.
* @param pThetaOut <= array of parameter angles of projections.
* @param pPoint <= space point to project to curve.
* @return number of projected points.
* @group "DConic4d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  bsiDConic4d_projectDPoint3dXY

(
DConic4dCP pConic,
DPoint3dP pXYZOut,
double        *pThetaOut,
DPoint3dCP pPoint
)
    {
    RotMatrix   matrixB;
    RotMatrix   projectionConic;
    DPoint3d    xywPoint;
    double      sinTheta[4], cosTheta[4], theta[4];
    int         numTheta, numOut;
    //bool        boolStat = false;

    xywPoint.Init ( pPoint->x, pPoint->y, 1.0);

    bsiDConic4d_getRotMatrixXYW (pConic, &matrixB);

    bsiQCoff_projectToEllipse (&projectionConic, &matrixB, &xywPoint);

    numOut = 0;

    bsiMath_implicitConicIntersectUnitCircle (cosTheta, sinTheta, theta,
                    &numTheta, &projectionConic);
    if (numTheta > 0)
        {
        int i;
        for (i = 0; i < numTheta; i++)
            {
            if (pXYZOut)
                bsiDConic4d_angleParameterToDPoint3d (pConic, &pXYZOut[i], theta[i]);
            if (pThetaOut)
                pThetaOut[i] = theta[i];
            }
        numOut = numTheta;
        }
    return numOut;
    }


/*-----------------------------------------------------------------*//**
* @description Project a point to the xy proction of the conic, and apply sector bounds.
* @param pConic => conic to evaluate
* @param pXYZOut <= array of xyz coordinates of projections.
* @param pThetaOut <= array of parameter angles of projections.
* @param pPoint <= space point to project to curve.
* @return the number of bounded projection points
* @group "DConic4d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_projectDPoint3dXYBounded

(
DConic4dCP pConic,
DPoint3dP pXYZOut,
double        *pThetaOut,
DPoint3dCP pPoint
)
    {
    DPoint3d cartesianPoint[4];
    double   angle[4];
    int numBounded = 0;
    int i;
    int numUnBounded = bsiDConic4d_projectDPoint3dXY (pConic, cartesianPoint, angle, pPoint);

    for (i = 0; i < numUnBounded; i++)
        {
        if (Angle::InSweepAllowPeriodShift (angle[i], pConic->start, pConic->sweep))
            {
            if (pXYZOut)
                pXYZOut[numBounded] = cartesianPoint[i];
            if (pThetaOut)
                pThetaOut[numBounded] = angle[i];
            numBounded++;
            }
        }
    return numBounded;
    }


/*-----------------------------------------------------------------*//**
* @description Find the closest point on a bounded conic, considering both endpoints and perpendicular projections.
* @param pConic => conic to evaluate
* @param pMinAngle <= parameter at closest approach point
* @param pMinDistSquared <= squared distance to closest approach point
* @param pMinPoint <= closest approach point
* @param pPoint => point to project
* @return true if a closest point is returned
* @group "DConic4d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiDConic4d_closestPointXYBounded

(
DConic4dCP pConic,
double        *pMinAngle,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pPoint,
bool extend
)
    {
    DPoint3d cartesianPoint[6];
    double   angle[6];
    double theta;
    double distSquared, minDistSquared;
    int iMin;
    int numCandidate = 0;
    int i;

    numCandidate = bsiDConic4d_projectDPoint3dXY (pConic, cartesianPoint, angle, pPoint);

    if (!extend && !bsiDConic4d_isFullSweep (pConic))
        {
        theta = angle[numCandidate] = pConic->start;
        bsiDConic4d_angleParameterToDPoint3d (pConic, &cartesianPoint[numCandidate], theta);
        numCandidate++;

        theta = angle[numCandidate] = pConic->start + pConic->sweep;
        bsiDConic4d_angleParameterToDPoint3d (pConic, &cartesianPoint[numCandidate], theta);
        numCandidate++;
        }

    if (numCandidate > 0)
        {
        minDistSquared  = DBL_MAX; 
        iMin = 0;
        for (i = 0; i < numCandidate; i++)
            {
            if (!extend &&!bsiDConic4d_angleInSweep (pConic, angle[i]))
                continue;

            distSquared = pPoint->DistanceSquaredXY (cartesianPoint[i]);
            if (distSquared < minDistSquared)
                {
                minDistSquared = distSquared;
                iMin = i;
                }
            }
        if (pMinDistSquared)
            *pMinDistSquared    = minDistSquared;
        if (pMinPoint)
            *pMinPoint          = cartesianPoint[iMin];
        if (pMinAngle)
            *pMinAngle          = angle[iMin];
        return true;
        }
    return false;
    }

Public GEOMDLLIMPEXP bool         bsiDConic4d_closestPointXYBounded
(
DConic4dCP pConic,
double        *pMinAngle,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pPoint)
    {
    return bsiDConic4d_closestPointXYBounded (pConic, pMinAngle, pMinDistSquared, pMinPoint, pPoint, false);
    }



static const double s_unitCircleRelTol = 1.0e-12;

/*-----------------------------------------------------------------*//**
* @description Find points where lines from a (possibly infinite) eyepoint have a specified
*   angle relation with the conic's tangent.   The relation is expressed as cosine
*   and sine of the angle.
* @param pConic => conic to evaluate
* @param pAngleArray <= angle parameters of computed points.
* @param pNumAngle <= number of angles returned
* @param maxAngle  => max angles expected.
* @param pEye       => eyepoint.
* @param cosine => cosine of target angle.  0.0 for perpendicular, 1.0 for tangent
* @param sine   => sine of target angle.    1.0 for perpendicular, 0.0 for tangent
* @group "DConic4d Eye Point"
* @return always true
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiDConic4d_angularRelationFromDPoint4dEyeXYW

(
DConic4dCP pConic,
double        *pAngleArray,
int           *pNumAngle,
int           maxAngle,
DPoint4dCP pEye,
double        cosine,
double        sine
)
    {
    DEllipse3d ellipse1;
    DEllipse3d ellipse0;
    double theta[4];
    double cc[4], ss[4];
    int numTheta = 0;
    int i, j;

    bsiDConic4d_pseudoTangent (pConic, &ellipse1);

    if (pEye->w == 0.0)
        {
        DPoint3d eye;
        double aCenter, aVector0, aVector90;
        int numQuadSolution;
        eye.x = pEye->x;
        eye.y = pEye->y;
        eye.z = pEye->z;

        aCenter   = sine *   eye.DotProductXY (ellipse1.center)
                  - cosine * eye.CrossProductXY (ellipse1.center);

        aVector0  = sine *   eye.DotProductXY (ellipse1.vector0)
                  - cosine * eye.CrossProductXY (ellipse1.vector0);

        aVector90 = sine *   eye.DotProductXY (ellipse1.vector90)
                  - cosine * eye.CrossProductXY (ellipse1.vector90);

        numQuadSolution = bsiMath_solveApproximateUnitQuadratic
                                (
                                &cc[0], &ss[0], &cc[1], &ss[1],
                                aCenter, aVector0, aVector90,
                                s_unitCircleRelTol
                                );
        for (i = 0; i < numQuadSolution; i++)
            {
            theta[numTheta++] = atan2 (ss[i], cc[i]);
            }
        }
    else
        {
        RotMatrix coffMatrix;
        DPoint3d vector0[3];
        DPoint3d vector1[3];

        bsiDConic4d_pseudoVector  (pConic, &ellipse0, pEye);

        vector0[0] = ellipse0.vector0;
        vector0[1] = ellipse0.vector90;
        vector0[2] = ellipse0.center;

        vector1[0] = ellipse1.vector0;
        vector1[1] = ellipse1.vector90;
        vector1[2] = ellipse1.center;

        for (i = 0; i < 3; i++)
            {
            for (j = 0; j < 3; j++)
                {
                coffMatrix.form3d[i][j] =
                             sine *   vector0[i].DotProductXY (vector1[j])
                           - cosine * vector0[i].CrossProductXY (vector1[j]);
                }
            }
        bsiBezier_implicitConicIntersectUnitCircle (cc, ss, theta, &numTheta, NULL, NULL, &coffMatrix);
        }

    if (pAngleArray)
        for (i = 0; i < numTheta; i++)
            {
            pAngleArray[i] = theta[i];
            }
    if (pNumAngle)
        *pNumAngle = numTheta;

    return true;
    }


#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Find the intersections of xy projections of a conic and line.
* Both are unbounded.
* @param pConic      => conic to intersect with line.
* @param pCartesianPoints   <= cartesian intersection points.
* @param pLineParams        <= array of line parameters (0=start, 1=end)
* @param pEllipseCoffs      <= array of intersection coordinates in conic
*                               frame.   xy are cosine and sine of angles.
*                               z is z distance from plane of conic.
* @param pEllipseAngle      <= array of angles in conic parameter space.
* @param pIsTangency    <= true if the returned intersection is a tangency.
* @param pStartPoint    => line start
* @param pEndPoint      => line end
* @return the number of intersections after applying conic and line parameter limits.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_intersectXYLineBounded

(
DConic4dCP pConic,
DPoint3dP pCartesianPoints,
double        *pLineParams,
DPoint3dP pEllipseCoffs,
double        *pEllipseAngle,
bool          *pIsTangency,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint
)
    {
    int numIntersection;
    int numBounded;
    int i;
    DPoint3d cartesianPoint[2], ellipseCoff[2];
    double   ellipseAngle[2];
    double   lineParam[2];
    double angle;


    numIntersection = bsiDConic4d_intersectXYLine
                                (
                                pConic,
                                cartesianPoint,
                                lineParam,
                                ellipseCoff,
                                ellipseAngle,
                                pStartPoint,
                                pEndPoint
                                );

    if (pIsTangency)
        *pIsTangency = numIntersection == 1;

    numBounded = 0;
    for (i = 0; i < numIntersection ; i++)
        {
        if  (lineParam[i] >= 0.0 && lineParam[i] <= 1.0)
            {
            angle = ellipseAngle[i];
            if  (Angle::InSweepAllowPeriodShift (angle, pConic->start, pConic->sweep))
                {
                if  (pCartesianPoints)
                    pCartesianPoints[numBounded] = cartesianPoint[i];
                if  (pLineParams)
                    pLineParams[numBounded] = lineParam[i];
                if  (pEllipseCoffs)
                    pEllipseCoffs[numBounded] = ellipseCoff[i];
                if  (pEllipseAngle)
                    pEllipseAngle[numBounded] = angle;
                numBounded++;
                }
            }
        }
    return  numBounded;
    }
#endif

#ifdef NEEDS_WORK

/*-----------------------------------------------------------------*//**
* @description Compute area and swept angle as seen from given point.
* @param pConic => conic to evaluate
* @param pArea <= area
* @param pSweep <= angle swept
* @param pPoint => point from which to compute area/angle
* @group "DConic4d Sweep Properties"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_xySweepProperties

(
DConic4dCP pConic,
double        *pArea,
double        *pSweep,
DPoint3dCP pPoint
)
    {
    RotMatrix matrix, inverse;
    DPoint3d localVector, globalVector;
    DPoint3d centerToStart, centerToEnd, pointToStart, pointToEnd;
    DPoint3d centerToMid, midToStart, midToEnd;
    double midCross;
    double c, s, c0, s0, c1, s1, cMid, sMid;
    double thetaMid;
    double theta0 = pConic->start;
    double theta1 = pConic->start + pConic->sweep;
    double angle;
    double localArea;
    bool    inside;

    double detJ = bsiDConic4d_determinantJXY (pConic);

    globalVector.DifferenceOf (*pPoint, pConic->center);
    bsiDConic4d_getXYLocalOrientation (pConic, &matrix, &inverse);
    inverse.Multiply (&localVector, &globalVector, 1);

    c = localVector.x;
    s = localVector.y;

    inside = c * c + s * s < 1.0;

    c0 = cos (theta0);
    s0 = sin (theta0);

    c1 = cos (theta1);
    s1 = sin (theta1);

    if (pArea)
        {
        localArea = 0.5 * ( pConic->sweep
                      - DoubleOps::DeterminantXYXY (c0, s0, c,  s)
                      - DoubleOps::DeterminantXYXY (c,  s,  c1, s1)
                        );
        *pArea = localArea * detJ;
        }

    if (pSweep)
        {

        centerToStart.SumOf(*NULL, pConic->vector0, c0, pConic->vector90, s0);
        centerToEnd.SumOf(*NULL, pConic->vector0, c1, pConic->vector90, s1);
        pointToStart.DifferenceOf (centerToStart, globalVector);
        pointToEnd.DifferenceOf (centerToEnd, globalVector);

        angle = Angle::Atan2 (
                        pointToStart.CrossProductXY (pointToEnd),
                        pointToStart.DotProductXY (pointToEnd)
                        );

        /* Imagine contracting the elliptical arc towards the chord between the start and end points.
            If the viewpoint is within the region swept by the contracting arc,
            we have to flip the angle by a period.
        */
        if (inside)
            {
            thetaMid = theta0 + 0.5 * pConic->sweep;

            cMid = cos (thetaMid);
            sMid = sin (thetaMid);

            centerToMid.SumOf(*NULL, pConic->vector0, cMid, pConic->vector90, sMid);

            midToStart.DifferenceOf (centerToStart, centerToMid);
            midToEnd.DifferenceOf (centerToEnd, centerToMid);
            midCross = midToStart.CrossProductXY (midToEnd);

            if (angle > 0.0)
                {
                if (midCross > 0.0)
                    angle = angle - msGeomConst_2pi;
                }
            else
                {
                if (midCross < 0.0)
                    angle = msGeomConst_2pi + angle;
                }
            }
        *pSweep = angle;
        }
    }
#endif


/*-----------------------------------------------------------------*//**
* @description Apply a transformation to the source conic.
* @param pDest <= transformed conic
* @param pTransform => transformation to apply.
* @param pSource => source conic.
* @group "DConic4d Transform"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_applyTransform

(
DConic4dP pDest,
TransformCP pTransform,
DConic4dCP pSource
)
    {
    pTransform->Multiply (&pDest->center, &pSource->center, 1);
    pTransform->Multiply (&pDest->vector0, &pSource->vector0, 1);
    pTransform->Multiply (&pDest->vector90, &pSource->vector90, 1);
    pDest->start = pSource->start;
    pDest->sweep = pSource->sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Apply a transformation to the source conic.
* @param pDest <= transformed conic
* @param pTransform => transformation to apply.
* @param pSource => source conic.
* @group "DConic4d Transform"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_applyDMatrix4d

(
DConic4dP pDest,
DMatrix4dCP pTransform,
DConic4dCP pSource
)
    {
    bsiDMatrix4d_multiply4dPoints (pTransform, &pDest->center  , &pSource->center, 1);
    bsiDMatrix4d_multiply4dPoints (pTransform, &pDest->vector0 , &pSource->vector0, 1);
    bsiDMatrix4d_multiply4dPoints (pTransform, &pDest->vector90, &pSource->vector90, 1);
    pDest->start = pSource->start;
    pDest->sweep = pSource->sweep;
    }


/*-----------------------------------------------------------------*//**
* @description Initialize conic with axes subjected to a post transform.
* @param pConic <= transformed conic
* @param pSource => source conic
* @param pAxisTransform => transform for axes
* @group "DConic4d Transform"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initTransformedAxes

(
DConic4dP pConic,
DConic4dCP pSource,
RotMatrixCP pAxisTransform
)
    {
    DPoint4d center, vector0, vector90;
    DPoint4d outPoint[3];
    int i;

    /* Save the input */
    center   = pSource->center;
    vector0  = pSource->vector0;
    vector90 = pSource->vector90;

    for (i = 0; i < 3; i++)
        {
        outPoint[i].SumOf(vector0, pAxisTransform->form3d[0][i], vector90, pAxisTransform->form3d[1][i], center, pAxisTransform->form3d[2][i]);
        }

    pConic->vector0  = outPoint[0];
    pConic->vector90 = outPoint[1];
    pConic->center   = outPoint[2];
    }


/*-----------------------------------------------------------------*//**
* @description Initialize with angle space subjected to a homogeneous transformation.
* @param pConic <= transformed conic
* @param pSource => source conic
* @param pAngleTransform => transform for angles
* @group "DConic4d Transform"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initTransformedAngles

(
DConic4dP pConic,
DConic4dCP pSource,
RotMatrixCP pAngleTransform
)
    {
    double theta0, theta1, phi0,phi1;

    if (pConic != pSource)
        *pConic = *pSource;

    theta0 = pConic->start;
    theta1 = theta0 + pConic->sweep;
    bsiTrig_mapLimits (&phi0, &phi1, pAngleTransform, theta0, theta1);
    pConic->start = phi0;
    pConic->sweep = phi1 - phi0;
    }

// Compute tangent directions at start of each conic.
// If their dot products (mutliplied by both sweep angles) do not agree complement the sweep of conic1.
static void bsiDConic4d_matchStartDirections (DConic4dCR conic0, DConic4dR conic1)
    {
    DPoint3d xyz0, xyz1;
    DVec3d tangent0, tangent1;
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&conic0, &xyz0, &tangent0, NULL, conic0.start);
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&conic1, &xyz1, &tangent1, NULL, conic1.start);
    double sweepSign = conic0.sweep * conic1.sweep;
    if (sweepSign * tangent0.DotProduct (tangent1) < 0.0)
        bsiDConic4d_complementSweep (&conic1);
    }


/*-----------------------------------------------------------------*//**
* @description Construct a center, 0 degree, and 90 degree vector of the "conventional"
* classification of the homogeneous conic.
* @remarks Curve type is returned with one of the below values:
*<UL>
*</LI>*pCurveType = 1: The curve is the ellipse
*       C + U cos(theta) + V sin (theta). That is, the curve is a unit circle in the (C,U,V) coordinate system.
*</LI>*pCurveType = -1: The curve is the hyperbola
*       C + U * s + V /s, where s is a free parameter.  U and V give the asymptotic directions,
*   and C is the intersection of the asymptotes.  That is, the curve is the hyperbola y = 1/x in the C,U,V coordinate system.
*</LI>*pCurveType = 0: The curve is the parabola
*       C + U * s + V * s^2.  That is, the curve is the parabola y = x^2 in the C,U,V coordinate system.
*</UL>
*
* @param pConic => conic to evaluate
* @param pAxes <= A DConic4d structure which gives the axes in the restricted form indicated above.
* @param pBasis <= Matrix D so that the vectors [U,V,C]*D, when drawn as a homogeneous
*       "ellipse" driven by its unit circle forcing function, are the curve.
* @param pCurveType <= curve type as indicated above.
* @group "DConic4d Major Minor Axes"
* @return true if the sum of the absolute values of the weights of the conic vectors and center is positive.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_getCommonAxes

(
DConic4dCP pConic,
DConic4dP pAxes,
RotMatrixP pBasis,
int  *pCurveType
)
    {
    RotMatrix B, D, Q;
    double w;
    DConic4d axes;

    bool    boolStat = false;
    int curveType = 1;
    if (bsiTrig_factorConicWeights (&w, &B, &D, &Q, &curveType,
            pConic->vector0.w, pConic->vector90.w, pConic->center.w))
        {
        bsiDConic4d_initTransformedAxes   (&axes, pConic,    &B);
        axes.start = pConic->start;
        axes.sweep = pConic->sweep;
        bsiDConic4d_initTransformedAngles (&axes, &axes,        &Q);

        if (curveType == 1)
            {
            /* Ellipse.  Now that the weights are 0 on the axes, rotate
                the parameter space to make them perpendicular. */
            double c, s, theta;
            DPoint4d vector0, vector90;
            bsiTrig_constructOneSided4DJacobiRotation
                            (
                            &c,
                            &s,
                            &axes.vector0,
                            &axes.vector90
                            );
            vector0 = axes.vector0;
            vector90 = axes.vector90;
            axes.vector0.SumOf(vector0, c, vector90, s);
            axes.vector90.SumOf(vector0, -s, vector90, c);
            D.GivensRowOp (c, s, 0, 1);
            theta = Angle::Atan2 (s,c);
            axes.start -= theta;
            double a = Angle::TwoPi ();
            if (axes.sweep > a)
                axes.sweep -= a;
            if (axes.sweep < -a)
                axes.sweep += a;
            bsiDConic4d_matchStartDirections (*pConic, axes);
            }
        if (pBasis)
            *pBasis = D;

        if (pAxes)
            *pAxes = axes;
        if (pCurveType)
            *pCurveType = curveType;

        boolStat = true;
        }
    return boolStat;
    }




/*-----------------------------------------------------------------*//**
* @description Make a copy of the input curve, revising axes and angles so that
* the start angle is zero and the sweep is positive.
* @param pConicOut  <= output conic
* @param pConicIn   => input conic
* @group "DConic4d Parameterization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_initWithPositiveSweepFromZero

(
DConic4dP pConicOut,
DConic4dCP pConicIn
)
    {
    /* Revise axes so start=0 and sweep is positive. */
    DConic4d conic = *pConicIn;
    if (conic.start != 0.0 || conic.sweep < 0.0)
        {
        double c0 = cos (conic.start);
        double s0 = sin (conic.start);
        conic.vector0.SumOf(conic.vector0, c0, conic.vector90, s0);
        if (conic.sweep > 0.0)
            {
            conic.vector90.SumOf(conic.vector0, -s0, conic.vector90, c0);
            }
        else
            {
            conic.vector90.SumOf(conic.vector0, s0, conic.vector90, -c0);
            conic.sweep = - conic.sweep;
            }
        }
    *pConicOut = conic;
    }


/*------------------------------------------------------------------*//**
* @description Initialize a (complete) conic from coefficients of implicit form.
* @param pConic <= initialized conic
* @param axx => xx coefficient
* @param axy => xy coefficient
* @param axw => xw coefficient
* @param ayy => yy coefficient
* @param ayw => yw coefficient
* @param aww => ww coefficient
* @return integer indicating the classification of degenerate cases:
* <UL>
* <LI>0: (normal) conic
* <LI>1: (degenerate) pair of lines.  First line is any linear combinations of
*               center and vector0.  Second is any linear combinations of
*               center and vector1.  Center is common point of the two lines.
*               (note that zero-weight center is possible, i.e. parallel lines)
* <LI>2: (degenerate) single line.  The line is formed by linear combinations of
*               center and vector0.
* <LI>3: (degenerate) single point, returned as (center) in the conic.
* <LI>4: no solutions.
* </UL>
* @group "DConic4d Implicit"
* @bsihdr                                                       EarlinLutz      04/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  bsiDConic4d_classifyImplicitXYW

(
DConic4dP pConic,
double axx,
double axy,
double axw,
double ayy,
double ayw,
double aww
)
    {
    RotMatrix matrixA;
    RotMatrix matrixQ;
    RotMatrix matrixB;
    RotMatrix matrixQT;
    DPoint3d lambda;
    DPoint3d absLambda;
    double *pLambda;
    double gamma[3];    /* 1/ sqrt (abs(lambda[i])), or 1 if lambda is zero */

    double lambdaScale;
    double s_relTol = 1.0e-12;
    double tol;
    int numPositive, numNegative;
    int lastPositive, lastNegative, lastZero;
    int iCenter, iVector0, iVector90, i;
    int typeCode = 4;

    matrixA.InitFromRowValues (
                axx, 0.5 * axy, 0.5 * axw,
                0.5 * axy, ayy, 0.5 * ayw,
                0.5 * axw, 0.5 * ayw, aww);
    bsiRotMatrix_symmetricEigensystem (&matrixQ, &lambda, &matrixA);
    absLambda.Init ( fabs (lambda.x), fabs (lambda.y), fabs(lambda.z));
    lambdaScale = absLambda.MaxAbs ();
    tol = s_relTol * lambdaScale;

    pLambda = (double*)&lambda;
    numPositive = numNegative = 0;
    lastPositive = lastNegative = lastZero = -1;
    for (i = 0; i < 3; i++)
        {
        if (fabs (pLambda[i]) < tol)
            {
            lastZero = i;
            gamma[i] = 1.0;
            }
        else if (pLambda[i] > 0.0)
            {
            numPositive++;
            lastPositive = i;
            gamma[i] = 1.0 / sqrt (pLambda[i]);
            }
        else
            {
            numNegative++;
            lastNegative = i;
            gamma[i] = 1.0 / sqrt (-pLambda[i]);
            }
        }


    if (numNegative > numPositive)
        {
        int temp = numPositive;
        numPositive = numNegative;
        numNegative = temp;
        temp = lastPositive;
        lastPositive = lastNegative;
        lastNegative = temp;
        lambda.x = - lambda.x;
        lambda.y = - lambda.y;
        lambda.z = - lambda.z;
        }

    matrixQT.TransposeOf (matrixQ);
    matrixB.ScaleColumns (matrixQ, gamma[0], gamma[1], gamma[2]);

#ifdef CHECK_EIGENSYSTEM
        {
        RotMatrix QLambda, QLambdaQT;
        QLambda.ScaleColumns (matrixQ, lambda.x, lambda.y, lambda.z);
        QLambdaQT.InitProduct (QLambda, matrixQT);
        QLambdaQT.IsIdentity ();
        }
#endif

    memset (pConic, 0, sizeof (DConic4d));
    if (numPositive == 2 && numNegative == 1)
        {
        /* The usual case .... full conic */
        iCenter = lastNegative;
        iVector0  = (iCenter  + 1) % 3;
        iVector90 = (iVector0 + 1) % 3;

        pConic->center.Init(
                        matrixB.form3d[0][iCenter],   matrixB.form3d[1][iCenter],   0.0, matrixB.form3d[2][iCenter]);
        pConic->vector0.Init(
                        matrixB.form3d[0][iVector0],  matrixB.form3d[1][iVector0],  0.0, matrixB.form3d[2][iVector0]);
        pConic->vector90.Init(
                        matrixB.form3d[0][iVector90], matrixB.form3d[1][iVector90], 0.0, matrixB.form3d[2][iVector90]);

        pConic->start = 0.0;
        bsiDConic4d_makeFullSweep (pConic);
        typeCode = 0;
        }
    else if (numPositive == 2 && numNegative == 0)
        {
        iCenter = lastZero;
        pConic->center.Init(
                        matrixB.form3d[0][iCenter],   matrixB.form3d[1][iCenter],   0.0, matrixB.form3d[2][iCenter]);
        typeCode = 3;
        }
    else if (numPositive == 1 && numNegative == 1)
        {
        DPoint4d B0, B1;
        int i0, i1;
        /* Pair of lines */
        iCenter = lastZero;
        i0  = (iCenter  + 1) % 3;
        i1  = (i0 + 1) % 3;
        pConic->center.Init(
                        matrixB.form3d[0][iCenter],  matrixB.form3d[1][iCenter],  0.0, matrixB.form3d[2][iCenter]);
        B0.Init(
                        matrixB.form3d[0][i0],       matrixB.form3d[1][i0],       0.0, matrixB.form3d[2][i0]);
        B1.Init(
                        matrixB.form3d[0][i1],       matrixB.form3d[1][i1],       0.0, matrixB.form3d[2][i1]);
        pConic->vector0.SumOf(B0, B1, 1.0);
        pConic->vector90.SumOf(B0, B1, -1.0);
        typeCode = 1;
        }
    else if (numPositive == 1 && numNegative == 0)
        {
        /* Single line. */
        int i0, i1, i2;
        i0 = lastPositive;
        i1  = (i0 + 1) % 3;
        i2  = (i1 + 1) % 3;
        pConic->center.Init(
                        matrixB.form3d[0][i1],  matrixB.form3d[1][i1],  0.0, matrixB.form3d[2][i1]);
        pConic->vector0.Init(
                        matrixB.form3d[0][i2],  matrixB.form3d[1][i2],  0.0, matrixB.form3d[2][i2]);
        typeCode = 2;
        }
    else
        {
        typeCode = 4;
        }
    return typeCode;
    }


/*------------------------------------------------------------------*//**
* @description Initialize a (complete) conic from coefficients of implicit form.
* @param pConic <= initialized conic
* @param axx => xx coefficient
* @param axy => xy coefficient
* @param axw => xw coefficient
* @param ayy => yy coefficient
* @param ayw => yw coefficient
* @param aww => ww coefficient
* @return true if the coefficients are clearly non-singular.
* @group "DConic4d Implicit"
* @bsihdr                                                       EarlinLutz      04/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDConic4d_initFromImplicitXYW

(
DConic4dP pConic,
double axx,
double axy,
double axw,
double ayy,
double ayw,
double aww
)
    {
    int type = bsiDConic4d_classifyImplicitXYW (pConic,
                        axx, axy, axw, ayy, ayw, aww);
    return type == 0;
    }


/*-----------------------------------------------------------------*//**
* @description Return the (weighted) control points of a quartic bezier that represents the ellipse.
* @remarks A quadratic bezier can represent any arc of a circle, but
*       it cannot wrap and cover the complete range.  The quartic
*       can cover the complete circle with a single bezier span.
* @param pConic => conic to evaluate
* @param pPoleArray <= array of (exactly) 5 poles for the bezier
* @param pCirclePoleArray <= array of (exactly) 5 poles for a quadric
*               bezier giving (cosine,sine,weight) as a function of the bezier parameter.
* @group "DConic4d Bezier"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_getQuarticBezierPoles

(
DConic4dCP pConic,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray
)
    {
    double theta;
    int i;
    DPoint3d circlePoleArray[5];
    double stepAngle;
    double restrictedSweep = pConic->sweep;
    double c;
    if (fabs (restrictedSweep) > msGeomConst_2pi)
        restrictedSweep = restrictedSweep > 0.0 ? msGeomConst_2pi : -msGeomConst_2pi;
    stepAngle = restrictedSweep * 0.25;
    c = cos (stepAngle);

    if (Angle::IsFullCircle (restrictedSweep))
        {
        /* Force everything to dead-on values */
        circlePoleArray[0].z = circlePoleArray[4].z = 1.0;
        circlePoleArray[1].z = circlePoleArray[3].z = 0.0;
        circlePoleArray[2].z = 1.0 / 3.0;
        circlePoleArray[0].x = cos (pConic->start);
        circlePoleArray[0].y = sin (pConic->start);
        for (i = 1; i <= 4; i++)
            {
            circlePoleArray[i].x = -circlePoleArray[i-1].y;
            circlePoleArray[i].y =  circlePoleArray[i-1].x;
            }
        }
    else
        {
        circlePoleArray[0].z = circlePoleArray[4].z = 1.0;
        circlePoleArray[1].z = circlePoleArray[3].z = c;
        circlePoleArray[2].z = (2.0 + 4.0 * c * c) / 6.0;
        for (i = 0; i < 5; i++)
            {
            theta = pConic->start + i * stepAngle;
            circlePoleArray[i].x = cos (theta);
            circlePoleArray[i].y = sin (theta);
            }
        }

    if (pPoleArray)
        {
        for (i = 0; i < 5; i++)
            {
            pPoleArray[i].SumOf(pConic->center, circlePoleArray[i].z, pConic->vector0, circlePoleArray[i].x, pConic->vector90, circlePoleArray[i].y);
            }
        }

    if (pCirclePoleArray)
        {
        memcpy (pCirclePoleArray, circlePoleArray, 5 * sizeof (DPoint3d));
        }
    }

/*-----------------------------------------------------------------*//**
* @description Return 0, 1, or 2 parameter angles at which the (unbounded) conic goes to inifinity.
* @remarks Unlike ~mbsiDConic4d_singularAngles, this function uses a quick initial test for a pure ellipse (zero asymptotes).
* @param pConic => conic to evaluate
* @param pTrigPoint <= (x,y) = cosine and sine of angles; z = angle.  This array is assumed dimensioned to hold 2 outputs.
* @return number of asymptotes placed in pTrigPoint.
* @group "DConic4d Asymptotes"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDConic4d_evaluateAsymptotes

(
DConic4dCP pConic,
DPoint3dP pTrigPoint
)
    {
    double wC  = pConic->center.w;
    double w0  = pConic->vector0.w;
    double w90 = pConic->vector90.w;
    int numRoot;
    int i;

    /* Quick test for obvious pure ellipse*/
    if (w0 * w0 + w90 * w90 < 0.999999 * wC * wC)
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int compareZ

(
const void *vpPoint0,
const void *vpPoint1
)
    {
    const DPoint3d *pPoint0 = (const DPoint3d*)vpPoint0;
    const DPoint3d *pPoint1 = (const DPoint3d*)vpPoint1;

    if (pPoint0->z < pPoint1->z)
        return -1;
    if (pPoint0->z > pPoint1->z)
        return 1;
    return 0;
    }

/*-----------------------------------------------------------------*//**
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool    pointInDPlane4dIntersection

(
DPoint4dCP pTestPoint,
DPoint4dCP pPlaneCoffs,
int numPlane,
int clipType
)
    {
    int i;
    for (i = 0; i < numPlane; i++)
        {
        if (pTestPoint->DotProduct (pPlaneCoffs[i]) > 0.0)
            {
            return clipType == 0 ? false : true;
            }
        }
    /* Fall through if IN all planes... */
    return clipType == 0 ? true : false;
    }


/*-----------------------------------------------------------------*//**
* @description Clips a conic to multiple planes.
* @param pConic => conic to evaluate
* @param pClipped <= unused
* @param pOutputArray <= caller-allocated array to hold up to nPlane+3 conics.
*       (three extras allow for both start and end point occuring in an active section, and for asymptotes)
* @param pNumOut <= number of conics returned.  May be zero.
* @param maxOut => caller's array dimension
* @param pPlaneArray => homogeneous plane equations
* @param nPlane => number of planes.  "Negative" plane functions are "in".
* @param clipType => 0 for inside, 1 for outside
* @return true unless the output array is too small.
* @group "DConic4d Intersection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDConic4d_clipToPlanes

(
DConic4dCP pConic,
bool      *pClipped,
DConic4dP pOutputArray,
int       *pNumOut,
int       maxOut,
DPoint4dCP pPlaneArray,
int        nPlane,
int        clipType
)

    {
    int         i;
    DPoint3d   *pCriticalAngleArray;
    int         numCritical;
    int         numOut;
    double theta0 = pConic->start;
    double arcSweep = pConic->sweep;
    double theta1 = theta0 + arcSweep;
    bool    boolstat = true;

    /* Allocate breakpoint array to hold up to:
        .... 2 intersections per plane
        .... 2 asymptotes
        .... start, end angle of the input.
    */
    if (nPlane < 1)
        return false;
    pCriticalAngleArray = (DPoint3d*)_alloca ((2 * nPlane + 4) * sizeof(DPoint3d));
    numOut = 0;
    if ( nPlane <= 0)
        {
        if (numOut < maxOut)
            {
            pOutputArray[numOut] = *pConic;
            numOut += 1;
            }
        else
            boolstat = false;
        }
    else
        {
        numCritical = 0;
        for ( i = 0; i < nPlane; i++)
            {
            numCritical += bsiDConic4d_intersectPlane (
                        pConic,
                        &pCriticalAngleArray[numCritical],
                        &pPlaneArray[i]);
            }

        numCritical += bsiDConic4d_evaluateAsymptotes (pConic, &pCriticalAngleArray[numCritical]);

        if (numCritical > 0)
            {
            double thetaMid, dtheta;
            //int numAccept = 0;
            for (i = 0; i < numCritical; i++)
                {
                double rawAngle = pCriticalAngleArray[i].z;
                pCriticalAngleArray[i].z =
                    bsiTrig_adjustAngleToSweep (rawAngle, theta0, arcSweep);
                }

            pCriticalAngleArray[numCritical++].Init (
                                cos (theta0), sin(theta0), theta0);
            pCriticalAngleArray[numCritical++].Init (
                                cos (theta1), sin(theta1), theta1);
            qsort (pCriticalAngleArray, numCritical, sizeof (pCriticalAngleArray[0]), compareZ);

            for (i = 1; i < numCritical; i++)
                {
                thetaMid = (pCriticalAngleArray[i-1].z + pCriticalAngleArray[i].z) * 0.5;
                dtheta = pCriticalAngleArray[i].z - pCriticalAngleArray[i-1].z;
                if (Angle::InSweepAllowPeriodShift (thetaMid, theta0, arcSweep))
                    {
                    DPoint4d midPoint;
                    bsiDConic4d_angleParameterToDPoint4d (pConic, &midPoint, thetaMid);
                    if (pointInDPlane4dIntersection (&midPoint, pPlaneArray, nPlane, clipType))
                        {
                        if (numOut < maxOut)
                            {
                            pOutputArray[numOut] = *pConic;
                            pOutputArray[numOut].start = pCriticalAngleArray[i-1].z;
                            pOutputArray[numOut].sweep = pCriticalAngleArray[i].z - pCriticalAngleArray[i-1].z;
                            numOut++;
                            }
                        else
                            boolstat = false;
                        }
                    }
                }
            }
        }
    *pNumOut = numOut;
    return boolstat;
    }
Public GEOMDLLIMPEXP int bsiDConic4d_solveTangentsPerpendicularToVector
(
DConic4dP pConic,
double  *angles,   
DVec3dR vector
)
    {
    DEllipse3d tangentSpace;
    bsiDConic4d_pseudoTangent (pConic, &tangentSpace);
    double alpha = vector.DotProduct (tangentSpace.center);
    double beta  = vector.DotProduct (tangentSpace.vector0);
    double gamma = vector.DotProduct (tangentSpace.vector90);
    DPoint3d trigPoints[2];
    int n = bsiMath_solveApproximateUnitQuadratic (
            &trigPoints[0].x, &trigPoints[0].y,
            &trigPoints[1].x, &trigPoints[1].y,
            alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance);
    if ( n == 1 )
        {
        /* Take the on-circle solution */
        angles[0] = Angle::Atan2 (trigPoints[0].y, trigPoints[0].x);
        }
    else if ( n == 2 )
        {
        angles[0] = Angle::Atan2 (trigPoints[0].y, trigPoints[0].x);
        angles[1] = Angle::Atan2 (trigPoints[1].y, trigPoints[1].x);
        }
    else
        {
        /* suppress the degenerate cases */
        n = 0;
        }

    return n;
    }

Public GEOMDLLIMPEXP double bsiDConic4d_maxAbsUnnormalizedXYZ (DConic4dCR conic)
    {
    return DoubleOps::Max (conic.center.MaxAbsUnnormalizedXYZ (), conic.vector0.MaxAbsUnnormalizedXYZ (), conic.vector90.MaxAbsUnnormalizedXYZ ());
    }

Public GEOMDLLIMPEXP double bsiDConic4d_maxAbsWeight (DConic4dCR conic)
    {
    return DoubleOps::MaxAbs (conic.center.w, conic.vector0.w, conic.vector90.w);
    }

//! @return max absolute difference between weights of center, vector0, and vector 90.
Public GEOMDLLIMPEXP double bsiDConic4d_maxWeightDiff (DConic4dCR conicA, DConic4dCR conicB)
    {
    return DoubleOps::Max (conicA.center.MaxUnnormalizedXYZDiff (conicB.center), conicA.vector0.MaxUnnormalizedXYZDiff (conicB.vector0), conicA.vector90.MaxUnnormalizedXYZDiff (conicB.vector90));
    }

//! @return max absolute difference between corresponding xyz of center, vector0, and vector 90.
Public GEOMDLLIMPEXP double bsiDConic4d_maxUnnormalizedXYZDiff (DConic4dCR conicA, DConic4dCR conicB)
    {
    return DoubleOps::MaxAbs (
            conicA.center.w - conicB.center.w,
            conicA.vector0.w - conicB.vector0.w,
            conicA.vector90.w - conicB.vector90.w
            );
    }

END_BENTLEY_GEOMETRY_NAMESPACE

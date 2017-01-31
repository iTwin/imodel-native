/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/ddisk3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| A DDisk3d is a parameterization of a planar anulus.                   |
| This is useful for end caps on cones and cylinders.                   |
| The disk is a polar coordinates mapping in the xy plane of            |
| a coordinate frame.                                                   |
|                                                                       |
| The parameterization is                                               |
|   (r,theta,z) <====> (r*cos(theta), r*sin(theta), z)                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static DRange2d s_defaultParameterRange =
        {
            {0.0, -msGeomConst_pi},
            {1.0,  msGeomConst_pi},
        };


/*---------------------------------------------------------------------------------**//**
* Initialize all fields of the disk.
* @param pFrame => Transformation whose translation, x, y, and z columns are the
*                   center, 0-degree vector, 90-degree vector, and off-plane direction.
*                   if <code>null</code>, an identity is used.
* @param pRange     => parameter range in <code>(r, theta)</code> coordinates.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDDisk3d_set

(
DDisk3dP pDisk,
TransformCP pFrame,
DRange2dCP pRange
)
    {
    if (pFrame)
        pDisk->frame = *pFrame;
    else
        pDisk->frame.InitIdentity ();

    if (pRange)
        pDisk->parameterRange = *pRange;
    else
        {
        pDisk->parameterRange = s_defaultParameterRange;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Initialize the disk with given center, axis in z direction, reference radius, and parameter
* range.
* @param r      => reference radius.  A the unit circle of the local system maps
*                   to a circle of this radius in the xy plane.  The out-of-plane
*                   direction is construted with the same z axis length.
* @param pRange => parameter range.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDDisk3d_setCenterRadii

(
DDisk3dP pDisk,
DPoint3dP pCenter,
double    r,
DRange2dCP pRange
)
    {
    pDisk->frame.InitFrom (*pCenter);
    pDisk->frame.ScaleMatrixColumns (r, r, r);
    bsiDDisk3d_setParameterRange (pDisk, pRange);
    }


/*---------------------------------------------------------------------------------**//**
* Set the reference frame of the disk.
* @param pTransform => coordinate frame.  If NULL, default is applied.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDDisk3d_setFrame

(
DDisk3dP pInstance,
TransformCP pFrame
)
    {
    if (pFrame)
        pInstance->frame = *pFrame;
    else
        pInstance->frame.InitIdentity ();
    }


/*---------------------------------------------------------------------------------**//**
* Set the parameter range of the disk.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDDisk3d_setParameterRange

(
DDisk3dP pInstance,
DRange2dCP pParameterRange
)
    {
    pInstance->parameterRange = pParameterRange ? *pParameterRange : s_defaultParameterRange;
    }


/*---------------------------------------------------------------------------------**//**
* @param pFrame     <= coordinate frame.   The z axis is the disk axis.  The disk
*                       parameterization (polar coordinates) is in the xy plane.
* @param pRange     <= the conic parameter range.
* @indexVerb get
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDDisk3d_get

(
DDisk3dCP pDisk,
TransformP pFrame,
DRange2dP pRange
)
    {
    if (pFrame)
        *pFrame = pDisk->frame;

    if (pRange)
        *pRange = pDisk->parameterRange;
    }




/*---------------------------------------------------------------------------------**//**
* test if an angle is in the disk's parameter range.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDDisk3d_angleInRange

(
DDisk3dCP pInstance,
double          longitude
)
    {
    return bsiTrig_angleInSweep
                    (
                    longitude,
                    pInstance->parameterRange.low.y,
                    pInstance->parameterRange.high.y - pInstance->parameterRange.low.y
                    );
    }


/*---------------------------------------------------------------------------------**//**
* test if a radius in the disk's parameter range.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDDisk3d_radiusInRange

(
DDisk3dCP pInstance,
double          altitude
)
    {
    return bsiTrig_scalarInInterval
                    (
                    altitude,
                    pInstance->parameterRange.low.x,
                    pInstance->parameterRange.high.x
                    );
    }


/*---------------------------------------------------------------------------------**//**
* Convert a world cartesian point to the local cartesian system.
* @param pWorld <= world coordinates
* @param pLocal => coordinates in local frame of disk.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDDisk3d_localToWorld

(
DDisk3dCP pDisk,
DPoint3dP pWorld,
DPoint3dCP pLocal
)
    {
    pDisk->frame.Multiply (pWorld, pLocal, 1);
    }


/*---------------------------------------------------------------------------------**//**
* Convert a local cartesian point to the world coordinate system.
* @param pLocal <= coordinates in local frame
* @param pWorld => world coordinates
* @return true if the coordinate frame is invertible.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDDisk3d_worldToLocal

(
DDisk3dCP pDisk,
DPoint3dP pLocal,
DPoint3dCP pWorld
)
    {
    return  bsiTransform_solveDPoint3d (&pDisk->frame, pLocal, pWorld);
    }



/*---------------------------------------------------------------------------------**//**
* Convert a local cartesian point into the local polar coordinates.
*       The z part of this coordinate system is identical to the cartesian z.
*       of the conical coordinates.   This is faster than the full conical coordinates
*       calculation and has no potential divide by zero.
*       is safe from divide-by-zero.
* @param pTheta     <= angle in unit cylindrical coordiantes
* @param pZ         <= z coordinate in cylindrical coordinates
* @param pR         <= radius in cylindrical coordinates
* @return false if the local point is on the disk/cylinder axis.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDDisk3d_localToPolar

(
DDisk3dCP pDisk,
double    *pR,
double    *pTheta,
double    *pZ,
DPoint3dCP pPoint
)
    {

    if (pTheta)
        *pTheta = bsiTrig_atan2 (pPoint->y, pPoint->x);

    if (pZ)
        *pZ = pPoint->z;

    if (pR)
        *pR = sqrt (pPoint->x * pPoint->x + pPoint->y * pPoint->y);

    return pPoint->y != 0.0 || pPoint->x != 0.0;
    }


/*---------------------------------------------------------------------------------**//**
* Convert from parametric to cartesian, with the parametric coordinate given as
* radius and pre-evaluated sine/cosine values.
* @param pPoint <= point on the plane of the disk.
* @param radius => nominal radius in local polar coordinates.  (Actual radius may
*                   differ if cosine and sine values are scaled rather than true trig values.)
* @param cosTheta => cosine of angle.
* @param sinTheta => sine of angle.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDDisk3d_trigParameterToPoint

(
DDisk3dCP pDisk,
DPoint3dP pPoint,
double  radius,
double  cosTheta,
double  sinTheta
)
    {
    pDisk->frame.Multiply (*pPoint,
                radius * cosTheta,
                radius * sinTheta,
                0.0
                );
    }



/*---------------------------------------------------------------------------------**//**
* Intersect the disk with a line segment.
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the (r,theta,z) frame.   (Points on the
*                       xy plane.)
* @param pLineParameter <= array of 0, 1, or 2 parameters with respect to the line.
* @param DRay3d         => ray to intersect.
* @return number of intersections.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int                bsiDDisk3d_intersectDSegment3d

(
DDisk3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DSegment3dCP pSegment
)
    {
    DRay3d ray;
    bsiDRay3d_initFromDSegment3d (&ray, pSegment);
    return bsiDDisk3d_intersectDRay3d (pInstance, pXYZ, pUVW, pLineParameter, &ray);
    }


/*---------------------------------------------------------------------------------**//**
* Intersect the disk with a line segment.
* @param pXYZ   <= coordinates of cartesian intersection point. May be NULL.
* @param pUVW   <= intersection point in the local coordinate frame. (Use
*                       localToPolar to convert to (r,theta, z)).
* @param pLineParameter <= parameter on line.
* @return number of intersections.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDDisk3d_intersectDRay3d

(
DDisk3dCP pDisk,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double      *pLineParameter,
DRay3dCP pRay
)
    {
    bool    boolstat = false;
    DPoint3d  solutionPoint;
    double u, v, lambda;

    /* If the ray is not in the plane of the disk, the disk frame xy vectors and the
        ray direction are linearly independent.  (And the disk's out-of-plane vector
        is not involved in the problem.)

        A (3D) point in the disk plane is X = Ad + U*x + V*y
        A (3D0 point on the ray is X = Ar + lambda*W
        Equating all three coordinates
                [U V W]* [x y -lambda]' = Ar - Ad
    Solve this linear system and extract x, y, lambda:
    */

    RotMatrix matrix;
    pDisk->frame.GetMatrix (matrix);
    matrix.SetColumn (pRay->direction, 2);
    DVec3d rightHandSide = DVec3d::FromStartEnd (pDisk->frame, pRay->origin);
    boolstat = matrix.Solve (solutionPoint, rightHandSide);

    if (boolstat)
        {
        u = solutionPoint.x;
        v = solutionPoint.y;
        lambda = - solutionPoint.z;
        }
    else
        {
        /* Set arbitrary outputs. */
        u = v = lambda = 0.0;
        }

    if (pLineParameter)
        *pLineParameter = lambda;
    if (pXYZ)
        pDisk->frame.Multiply (*pXYZ, u, v, 0.0);

    if (pUVW)
        {
        pUVW->Init ( u, v, 0.0);
        }

    return boolstat;
    }



/*---------------------------------------------------------------------------------**//**
* Return a rule line at specified longitude (angle around disk).
* to the z range
* @param pSegment  <= ruling segment.
* @param theta => longitude angle (radians)
* @return true if theta is within the parameter range for the disk.
* @indexVerb ruleLines
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDDisk3d_getRuleLine

(
DDisk3dCP pDisk,
DSegment3dP pSegment,
double          theta
)
    {
    double              cosTheta = cos (theta);
    double              sinTheta = sin (theta);
    double              r0, r1, theta0, theta1;
    double              sweep;

    bsiDDisk3d_getScalarNaturalParameterRange (pDisk, &r0, &r1, &theta0, &theta1);
    sweep = theta1 - theta0;

    if (pSegment)
        {
        pDisk->frame.Multiply (*(&pSegment->point[0]), r0 * cosTheta, r0 * sinTheta, 0.0);
        pDisk->frame.Multiply (*(&pSegment->point[1]), r1 * cosTheta, r1 * sinTheta, 0.0);
        }

    return bsiTrig_angleInSweep (theta, theta0, sweep);
    }


/*---------------------------------------------------------------------------------**//**
* Return a cross section at given radius in the polar coordinate system.
* @param pEllipse <= ellipse at specified latitude.  0 and 90 degree vectors
*           are 0 and 90 degrees latitude.
* @param z      => altitude parameter
* @return true if the radius value is within the disk parameter range.
* @indexVerb ruleLines
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDDisk3d_getCrossSection

(
DDisk3dCP pDisk,
DEllipse3dP pEllipse,
double          r
)
    {
    DVec3d              vector0, vector90;
    DPoint3d    center;
    double              r0, r1, theta0, theta1, dr;
    double              sweep;

    bsiDDisk3d_getScalarNaturalParameterRange (pDisk, &r0, &r1, &theta0, &theta1);
    sweep = theta1 - theta0;

    dr = r1 - r0;
    pDisk->frame.GetMatrixColumn (vector0, 0);
    pDisk->frame.GetMatrixColumn (vector90, 0);
    pDisk->frame.GetTranslation (center);

    *pEllipse = DEllipse3d::FromScaledVectors (center, vector0, vector90, r0, r1,
                        theta0, sweep);

    return bsiTrig_scalarInSweep (r, r0, dr);
    }


/*---------------------------------------------------------------------------------**//**
* @param pPoint <= evaluated point
* @param radiusFraction => radial position, as a fraction of the patch radial range.
* @param angleFraction => angular position, as a fraction of the patch angle range.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDDisk3d_fractionParameterToDPoint3d

(
DDisk3dCP pDisk,
DPoint3dP pPoint,
double    radiusFraction,
double    angleFraction
)
    {
    double angle, radius;
    bsiDDisk3d_fractionParameterToNaturalParameter (pDisk, &radius, &angle, radiusFraction, angleFraction);
    return bsiDDisk3d_naturalParameterToDPoint3d (pDisk, pPoint, radius, angle);
    }


/*---------------------------------------------------------------------------------**//**
* @param pRadiusFraction <= radial position, as a fraction of the patch radial range.
* @param pAngleFraction  <= angular position, as a fraction of the patch angle range.
* @param pPoint         => evaluated point
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDDisk3d_dPoint3dToFractionParameter

(
DDisk3dCP pDisk,
double    *pThetaFraction,
double  *pZFraction,
DPoint3dCP pPoint
)
    {
    double theta, z;
    bool    stat = bsiDDisk3d_dPoint3dToNaturalParameter (pDisk, &theta, &z, pPoint);
    bsiDDisk3d_naturalParameterToFractionParameter (pDisk, pThetaFraction, pZFraction, theta, z);
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* @param pRadius => radius in xy plane.
* @param pAngle => angle around cross sectional circle
* @param radiusFraction => radial position, as a fraction of patch radius range.
* @param angleFraction => angular position, as a fraction of the patch angle range.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDDisk3d_fractionParameterToNaturalParameter

(
DDisk3dCP pDisk,
double    *pRadius,
double    *pAngle,
double    radiusFraction,
double    angleFraction
)
    {
    if (pRadius)
        *pRadius = pDisk->parameterRange.low.x +
                            radiusFraction * (pDisk->parameterRange.high.x - pDisk->parameterRange.low.x);

    if (pAngle)
        *pAngle = pDisk->parameterRange.low.y +
                            angleFraction * (pDisk->parameterRange.high.y - pDisk->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* @param pRadiusFraction => radius parameter as a fraction of the pach.
* @param pAngleFraction => natural angle parameter as fraction of the patch.
* @param radius => radius, in local coordinate frame.
* @param angle  => angular position
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDDisk3d_naturalParameterToFractionParameter

(
DDisk3dCP pDisk,
double    *pRadiusFraction,
double    *pAngleFraction,
double    radius,
double    angle
)
    {
    if (pRadiusFraction)
        bsiTrig_safeInverseLinearInterpolate (pRadiusFraction,
                                        radius,
                                        pDisk->parameterRange.low.x,
                                        pDisk->parameterRange.high.x,
                                        0.0);

    if (pAngleFraction)
        *pAngleFraction = bsiTrig_normalizeAngleToSweep
                                (
                                angle,
                                pDisk->parameterRange.low.y,
                                pDisk->parameterRange.high.y - pDisk->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* Test if the disk range is the full parameter space.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDDisk3d_isComplete

(
DDisk3dCP pInstance
)
    {
    return bsiDDisk3d_isParam1Complete (pInstance)
            && bsiDDisk3d_isParam2Complete (pInstance);
    }


/*---------------------------------------------------------------------------------**//**
* @return true if the disk angle parameter covers the complete circular range of the cross sections.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDDisk3d_isParam2Complete

(
DDisk3dCP pDisk
)
    {
    return bsiTrig_isAngleFullCircle (pDisk->parameterRange.high.y - pDisk->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* @return true if the radius range covers the nominal 0..1 complete range.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDDisk3d_isParam1Complete

(
DDisk3dCP pDisk
)
    {
    static double s_relTol = 1.0e-12;
    double nearOne = 1.0 - s_relTol;
    return  (pDisk->parameterRange.high.x >= nearOne && pDisk->parameterRange.low.x  < s_relTol)
        ||  (pDisk->parameterRange.low.x  >= nearOne && pDisk->parameterRange.high.x < s_relTol);
    }



/*---------------------------------------------------------------------------------**//**
* Get the reference frame of the disk.
*
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDDisk3d_getFrame

(
DDisk3dCP pInstance,
TransformP pFrame
)
    {
    *pFrame = pInstance->frame;
    }


/*---------------------------------------------------------------------------------**//**
* Get the disk's inverse coordinate, i.e. transform from the global space to
* a the system where the disk's circular rulings are circles around the origin.
* @param pInverseFrame <= inverse frame.
* @return true if the inverse is was computed.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDDisk3d_getInverseFrame

(
DDisk3dCP pInstance,
TransformP pInverseFrame
)
    {
    return pInverseFrame->InverseOf (pInstance->frame);
    }



/*---------------------------------------------------------------------------------**//**
* Evaluate the implicit function for the disk.
* @param pPoint => point where the implicit function is evaluated.
* @indexVerb implicit
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDDisk3d_implicitFunctionValue

(
DDisk3dCP pDisk,
DPoint3dCP pPoint
)
    {
    DVec3d vectorX, vectorY;
    DVec3d vector = DVec3d::FromStartEnd (pDisk->frame, *pPoint);
    pDisk->frame.GetMatrixColumn (vectorX, 0);
    pDisk->frame.GetMatrixColumn (vectorY, 1);
    return vector.TripleProduct (vectorX, vectorY);
    }


/*---------------------------------------------------------------------------------**//**
* @param pPoint <= evaluated point
* @param theta => angle around disk
* @param z     => fraction of z axis.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDDisk3d_naturalParameterToDPoint3d

(
DDisk3dCP pDisk,
DPoint3dP pPoint,
double  radius,
double  angle
)
    {
    double cosTheta = cos (angle);
    double sinTheta = sin (angle);

    pDisk->frame.Multiply (*pPoint,
                radius * cosTheta,
                radius * sinTheta,
                0.0
                );
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute a natural parameter of a given xyz point.  When the point is exactly on the
* surface, this function is the inverse of naturalParameterToPoint.   For a point off of the
* surface, this inversion should be predictable but may be other than a true projection.
* @param pRadius <= radial parameter
* @param pAngle <= angular parameter
* @param pPoint  => xyz coordinates
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiDDisk3d_dPoint3dToNaturalParameter

(
DDisk3dCP pDisk,
double  *pRadius,
double  *pAngle,
DPoint3dCP pPoint
)
    {
    DPoint3d localPoint;
    bool    boolstat = bsiDDisk3d_worldToLocal (pDisk, &localPoint, pPoint);
    if (boolstat)
        {
        boolstat = bsiDDisk3d_localToPolar (pDisk, pRadius, pAngle, NULL, &localPoint);
        }
    return boolstat;
    }



/*---------------------------------------------------------------------------------**//**
* Return the range of the natural parameter for the active surface patch.
* @param pRadius0 => start value of natural parameter.
* @param pRadius1 => end value of natural parameter.
* @param pAngle0  => start value of natural parameter.
* @param pAngle1  => end value of natural parameter.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDDisk3d_getScalarNaturalParameterRange

(
DDisk3dCP pDisk,
double    *pRadius0,
double    *pRadius1,
double    *pAngle0,
double    *pAngle1
)
    {
    *pRadius0  = pDisk->parameterRange.low.x;
    *pRadius1  = pDisk->parameterRange.high.x;
    *pAngle0   = pDisk->parameterRange.low.y;
    *pAngle1   = pDisk->parameterRange.high.y;
    }


/*---------------------------------------------------------------------------------**//**
* Get the parameter range of the disk.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDDisk3d_getNaturalParameterRange

(
DDisk3dCP pInstance,
DRange2dP pParameterRange
)
    {
    *pParameterRange = pInstance->parameterRange;
    }


/*---------------------------------------------------------------------------------**//**
* Get the parameter range as start/sweep pairs.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDDisk3d_getScalarNaturalParameterSweep

(
DDisk3dCP pInstance,
double          *pRadius0,
double          *pRadiusDelta,
double          *pAngle0,
double          *pAngleSweep
)
    {
    *pRadius0       = pInstance->parameterRange.low.x;
    *pRadiusDelta   = pInstance->parameterRange.high.x - *pRadius0;

    *pAngle0        = pInstance->parameterRange.low.y;
    *pAngleSweep    = pInstance->parameterRange.high.y - *pAngle0;
    }


/*---------------------------------------------------------------------------------**//**
* Return the range of the natural parameters for a complete surface.
* @param pRadius0 => start value of natural parameter.
* @param pRadius1 => end value of natural parameter.
* @param pAngle0  => start value of natural parameter.
* @param pAngle1  => end value of natural parameter.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDDisk3d_getCompleteNaturalParameterRange

(
DDisk3dCP pDisk,
double    *pRadius0,
double    *pRadius1,
double    *pAngle0,
double    *pAngle1
)
    {
    *pRadius0   =  0.0;
    *pRadius1   =  1.0;
    *pAngle0    = -msGeomConst_pi;
    *pAngle1    =  msGeomConst_pi;
    }


END_BENTLEY_GEOMETRY_NAMESPACE

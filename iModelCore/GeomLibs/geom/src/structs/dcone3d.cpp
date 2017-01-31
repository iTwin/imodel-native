/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dcone3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*=================================================================================**//**
* @struct DCone3d
* A DCone3d is a conical surface in 3D.
*
* The surface is parameterized in terms of angle theta and altitude z as
*
*<pre>
*               X = C + (1 +  z * (radiusFraction - 1)) * (U * cos(theta) + V * sin(theta)) + z * W
*</pre>
* where
*<ul>
*<li> C is the center of the base circle</li>
*<li> U is the vector from the base center to the 0-degree point on the base circle.</li>
*<li> V is the vector from the base center to the 90-degree point on the base circle. </li>
*<li> W is the axis vector.</li>
*<li> radiusFracton is the radius ofthe cross-section circle at z = 1.
*<ul>
*<li>For perfect cylinder, radiusFraction=1.</li>
*<li>For cone with apex at z = 1, radiusFraction=0.</li>
*<li>For cone with apex at z = Z, radiusFraction=1 - 1/z=(z-1)/z.</li>
*<li>For given radiusFraction, the apex is at z=1/(1-radiusFraction).</li>
*</ul>
*</li>
*</ul>
*
* The parameter range for a complete cone is
*<pre>
*                     -PI <= theta <= PI
*                       0 <= z <= 1
*</pre>
* @bsistruct                                                    EarlinLutz      09/00
+===============+===============+===============+===============+===============+======*/

static const DRange2d s_defaultParameterRange =
        {
            { -msGeomConst_pi, 0.0},
            {  msGeomConst_pi, 1.0},
        };



/*---------------------------------------------------------------------------------**//**
* @description Set all fields of the cone from arguments.
* @param pCone <= cone to initialize
* @param pFrame => Transformation whose translation, x, y, and z columns are the
*                   center, 0-degree vector, 90-degree vector, and axis direction.
*                   if <code>null</code>, an identity is used.
* @param radiusFraction => top circle radius divided by bottom circle radius.
* @param pRange     => parameter range in <code>(theta, phi)</code> coordinates.
* @group "DCone3d Initialization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDCone3d_set

(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
)
    {
    if (pFrame)
        pCone->frame = *pFrame;
    else
        pCone->frame.InitIdentity ();

    if (pRange)
        pCone->parameterRange = *pRange;
    else
        {
        pCone->parameterRange = s_defaultParameterRange;
        }

    pCone->radiusFraction = radiusFraction;
    }


/*---------------------------------------------------------------------------------**//**
* @description Set all fields of the cone from arguments.
* @param pCone <= cone to initialize
* @param pFrame => Transformation whose translation, x, y, and z columns are the
*                   center, 0-degree vector, 90-degree vector, and axis direction.
*                   if <code>null</code>, an identity is used.
* @param radiusFraction => top circle radius divided by bottom circle radius.
* @param pRange     => parameter range in <code>(theta, z)</code> coordinates.
* @group "DCone3d Initialization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDCone3d_setFrameAndFraction
(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
)
    {
    Transform dFrame;
    if (pFrame)
        dFrame = *pFrame;
    else
        dFrame.InitIdentity ();
    bsiDCone3d_set (pCone, &dFrame, radiusFraction, pRange);
    }


/*---------------------------------------------------------------------------------**//**
* @description Set the cone with given center, axis in z direction, and given top and bottom radii.
* The cone origin may be placed at either end of the cone, i.e. might not be
* at the given center.  This will be done to get a nonzero radius at the center.
* @param pCone <= cone to initialize
* @param pCenter => nominal origin of cone.
* @param r0     => radius at z=0
* @param r1     => radius at z=1
* @param h      => cone height.
* @param pRange => parameter range.
* @group "DCone3d Initialization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDCone3d_setCenterRadiiHeight

(
DCone3dP pCone,
DPoint3dP pCenter,
double    r0,
double    r1,
double    h,
DRange2dCP pRange
)
    {
    if (h == 0.0)
        h = 1.0;
    if (r0 == 0.0 && r1 == 0.0)
        {
        bsiDCone3d_setCenterRadiiHeight (pCone, pCenter, 1.0, 1.0, h, pRange);
        }

    if (r0 >= r1)
        {
        bsiTransform_initFromTranslation (&pCone->frame, pCenter);
        pCone->frame.ScaleMatrixColumns (r0, r0, h);
        pCone->radiusFraction = r1 / r0;
        bsiDCone3d_setParameterRange (pCone, pRange);
        }
    else
        {
        bsiTransform_initFromTranslation (&pCone->frame, pCenter);
        pCone->frame.ScaleMatrixColumns (r0, r0, h);
        pCone->radiusFraction = r1 / r0;
        if (pRange)
            {
            DRange2d range;
            range = *pRange;
            range.low.y = 1.0 - pRange->low.y;
            range.high.y = 1.0 - pRange->high.y;
            bsiDCone3d_setParameterRange (pCone, &range);
            }
        else
            {
            bsiDCone3d_setParameterRange (pCone, NULL);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Set the reference frame of the cone.
* @param pCone <= cone to modify
* @param pFrame => coordinate frame.  If NULL, default is applied.
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDCone3d_setFrame

(
DCone3dP pCone,
TransformCP pFrame
)
    {
    if (pFrame)
        pCone->frame = *pFrame;
    else
        pCone->frame.InitIdentity ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Set the parameter range of the cone.
* @param pCone <= cone to modify
* @param pParameterRange => parameter range to set
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDCone3d_setParameterRange

(
DCone3dP pCone,
DRange2dCP pParameterRange
)
    {
    pCone->parameterRange = pParameterRange ? *pParameterRange : s_defaultParameterRange;
    }


/*---------------------------------------------------------------------------------**//**
* @description Extract the cone definition.
* @param pCone => cone to evaluate
* @param pFrame     <= coordinate frame.   The z axis is the cone axis.  The intersection
*                           with the xy plane is a unit circle.
* @param pTaper     <= in local coordinates, the radius of the cone cross section (circle)
*                           at z = 1.
* @param pRange     <= the conic parameter range.
* @group "DCone3d Queries"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDCone3d_get

(
DCone3dCP pCone,
TransformP pFrame,
double          *pTaper,
DRange2dP pRange
)
    {
    if (pFrame)
        *pFrame = pCone->frame;
    if (pTaper)
        *pTaper = pCone->radiusFraction;
    if (pRange)
        {
        *pRange = pCone->parameterRange;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Test whether the longitude angle is in the cone's parameter range.
* @return true if the longitude is in range.
* @param pCone => cone to evaluate
* @param longitude => longitude angle to test
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDCone3d_longitudeInRange

(
DCone3dCP pCone,
double          longitude
)
    {
    return bsiTrig_angleInSweep
                    (
                    longitude,
                    pCone->parameterRange.low.x,
                    pCone->parameterRange.high.x - pCone->parameterRange.low.x
                    );
    }


/*---------------------------------------------------------------------------------**//**
* @description Test whether the altitude is in the cone's parameter range.
* @return true if the altitude is in range.
* @param pCone => cone to evaluate
* @param altitude => altitude to test
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDCone3d_altitudeInRange

(
DCone3dCP pCone,
double          altitude
)
    {
    return bsiTrig_scalarInInterval
                    (
                    altitude,
                    pCone->parameterRange.low.y,
                    pCone->parameterRange.high.y
                    );
    }


/*---------------------------------------------------------------------------------**//**
* @description Test whether the given local point lies within selective parametric extents of the cone.
* @return true if the local point is "in" the cone.
* @param pCone => cone to evaluate
* @param pUVW => local coordinates to test.
* @param applyAngleLimits => true to test angle of (x,y) part against angular bounds
*               (x part of parameter space bounds)
* @param applyAltitudeLimits => true to test local z coordinate against the altitude
*               bounds (y part of parameter space bounds)
* @param applyRadiusLimits => true to test (x,y) part against the radius
*               at the local point's z coordinate.
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDCone3d_isLocalDPoint3dInSelectiveBounds
(
DCone3dCP pCone,
DPoint3dCP pUVW,
bool            applyAngleLimits,
bool            applyAltitudeLimits,
bool            applyRadiusLimits
)
    {
    if (applyAltitudeLimits && !bsiTrig_scalarInInterval
                    (
                    pUVW->z,
                    pCone->parameterRange.low.y,
                    pCone->parameterRange.high.y
                    ))
        return false;

    if (applyRadiusLimits)
        {
        double r2point = pUVW->x * pUVW->x + pUVW->y * pUVW->y;
        double rcone = bsiDCone3d_heightToRadius (pCone, pUVW->z);
        double r2cone = rcone * rcone;
        if (r2point > r2cone)
            return false;
        }

    if (applyAngleLimits && (pUVW->x != 0.0 || pUVW->y != 0.0))
        {
        double theta = bsiTrig_atan2 (pUVW->y, pUVW->x);
        if (bsiTrig_angleInRange (theta, pCone->parameterRange.low.x, pCone->parameterRange.high.x))
            return false;
        }

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @description Convert a world cartesian point to the local cartesian system.
* @param pCone => cone to evaluate
* @param pWorld <= world coordinates
* @param pLocal => coordinates in local frame of cone.
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDCone3d_localToWorld

(
DCone3dCP pCone,
DPoint3dP pWorld,
DPoint3dCP pLocal
)
    {
    pCone->frame.Multiply (pWorld, pLocal, 1);
    }


/*---------------------------------------------------------------------------------**//**
* @description Convert a local cartesian point to the world coordinate system.
* @param pCone => cone to evaluate
* @param pLocal <= coordinates in local frame
* @param pWorld => world coordinates
* @group "DCone3d Local Coordinates"
* @return true if the local to world transformation was invertible.
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDCone3d_worldToLocal

(
DCone3dCP pCone,
DPoint3dP pLocal,
DPoint3dCP pWorld
)
    {
    return  bsiTransform_solveDPoint3d (&pCone->frame, pLocal, pWorld);
    }



/*---------------------------------------------------------------------------------**//**
* @description Convert a local cartesian point into the local reference cylindrical coordinates.
*       The theta and z parts of this coordinate system are identical to those
*       of the conical coordinates.   This is faster than the full conical coordinates
*       calculation and has no potential divide by zero.
*       is safe from divide-by-zero.
* @param pCone => cone to evaluate
* @param pTheta     <= angle in unit cylindrical coordiantes
* @param pZ         <= z coordinate in cylindrical coordinates
* @param pR         <= radius in cylindrical coordinates
* @param pPoint     => local Cartesian point
* @return false if the local point is on the cone/cylinder axis.
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDCone3d_localToCylindrical

(
DCone3dCP pCone,
double    *pTheta,
double    *pZ,
double    *pR,
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
* @description Convert a local cartesian point into the local conical coordinates.   This is
* more expensive than ~mbsiDCone3d_localToCylindrical; the additional expense produces
* a radius that is normalized to the cone radius, but does not affect the
* angle and altitude.
* @param pCone => cone to evaluate
* @param pTheta     <= angle in unit conical coordiantes
* @param pZ         <= z coordinate in conical coordinates
* @param pR         <= radius, as a multiple of the cone radius at this z.
*                       The radius is defined as zero everywhere on the constant-z
*                       plane through the apex.
* @param pPoint     => local Cartesian point
* @return true if the inversion is invertible (i.e. point not on axis or apex plane).
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDCone3d_localToConical

(
DCone3dCP pCone,
double    *pTheta,
double    *pZ,
double    *pR,
DPoint3dCP pPoint
)
    {
    double rCylinder, rCone;
    double mu = pCone->radiusFraction - 1.0;

    bool    cylinderOK = bsiDCone3d_localToCylindrical (pCone, pTheta, pZ, &rCylinder, pPoint);
    bool    radiusOK   = bsiTrig_safeDivide (&rCone, rCylinder, 1.0 + mu * pPoint->z, 0.0);
    if (pR)
        *pR = rCone;
    return cylinderOK && radiusOK;
    }


/*---------------------------------------------------------------------------------**//**
* @description Convert from parametric to cartesian, with the parametric coordinate given as trig values
* and height.
* @param pCone  => cone to evaluate
* @param pPoint <= Cartesian point
* @param cosTheta => cosine of cone angle parameter
* @param sinTheta => sine of cone angle parameter
* @param z      => cone altitude parameter
* @group "DCone3d Parameterization"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDCone3d_trigParameterToPoint

(
DCone3dCP pCone,
DPoint3dP pPoint,
double    cosTheta,
double    sinTheta,
double    z
)
    {
    double mu = pCone->radiusFraction - 1.0;
    double r = 1.0 + mu * z;

    pCone->frame.Multiply (*pPoint,
                r * cosTheta,
                r * sinTheta,
                z
                );
    }


/*---------------------------------------------------------------------------------**//**
* @description Intersect the cone with a line segment.
* @param pCone  => cone to evaluate
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the cone frame.   May be NULL.
* @param pLineParameter <= array of 0, 1, or 2 parameters with respect to the line.
* @param pSegment       => segment to intersect.
* @return number of intersections.
* @group "DCone3d Intersection"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDSegment3d

(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DSegment3dCP pSegment
)
    {
    DRay3d ray;
    bsiDRay3d_initFromDSegment3d (&ray, pSegment);
    return bsiDCone3d_intersectDRay3d (pCone, pXYZ, pUVW, pLineParameter, &ray);
    }

static double dotProductNegativeZ

(
DPoint3dCP pU,
DPoint3dCP pV
)
    {
    return pU->x * pV->x + pU->y * pV->y - pU->z * pV->z;
    }

/*---------------------------------------------------------------------------------**//**
* @description Intersect the unbounded cone surface with an unbounded line.
* @param pCone  => cone to evaluate
* @param pInverseFrame <= inverse coordinate frame. Caps are in z=0 and z=1 frames.
* @param pInverseOK    <= true if coordinate frame is invertible
* @param pLocalRay     <= ray transformed into local coordinates.
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the cone frame.   (Points on the
*                       unit sphere).  May be NULL.
* @param pLineParameter <= array of 0, 1, or 2 parameters with respect to the line.
* @param pRay => (unbounded) ray to intersect with cone.
* @return number of intersections.
* @group "DCone3d Intersection"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsiDCone3d_intersectDRay3dExt

(
DCone3dCP pCone,
TransformP pInverseFrame,
bool              *pInverseOK,
DRay3dP pLocalRay,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DRay3dCP pRay
)
    {
    //bool    boolstat = false;
    Transform inverseFrame;
    DRay3d  localRay;
    double aa, bb, cc;
    double mu;
    DRay3d scaledRay;
    double param[2];
    int numRoot = 0;
    int i;

    if (pInverseOK)
        *pInverseOK = false;

    if (bsiDCone3d_getInverseFrame (pCone, &inverseFrame))
        {
        if (pInverseOK)
            *pInverseOK = true;

        if (pInverseFrame)
            *pInverseFrame = inverseFrame;
        inverseFrame.Multiply (localRay, *pRay);

        if (pLocalRay)
            *pLocalRay = localRay;

        mu = pCone->radiusFraction - 1.0;


        /* Intersection condition is X^2 + Y^2 - (1 + mu Z)^2 = 0
            where X,Y,Z are local ray x, y, and z.
            X = x0 + s dx
            Y = y0 + s dy
            Z = z0 + s dz
            mu Z = mu z0 + s mu dz
            1 + mu Z = (1 + mu z0) + s (mu dz)
                     = w0 + s dw
            where w0 = 1 + mu z0
                  dw = mu dz
        */

        scaledRay = localRay;   /* x,y parts are good, z needs to be corrected. */
        scaledRay.origin.z = 1.0 + mu * localRay.origin.z;
        scaledRay.direction.z = mu * localRay.direction.z;

        aa = dotProductNegativeZ (&scaledRay.direction, &scaledRay.direction);

        bb = 2.0 * dotProductNegativeZ (&scaledRay.origin, &scaledRay.direction);

        cc = dotProductNegativeZ (&scaledRay.origin, &scaledRay.origin);

        numRoot = bsiMath_solveQuadratic (param, aa, bb, cc);

        if (numRoot > 0)
            {
            for (i = 0; i < numRoot; i++)
                {
                if (pLineParameter)
                    pLineParameter[i] = param[i];
                if (pXYZ)
                    bsiDRay3d_evaluateDPoint3d (pRay, &pXYZ[i], param[i]);
                if (pUVW)
                    bsiDRay3d_evaluateDPoint3d (&localRay, &pUVW[i], param[i]);
                }
            }
        }

    return numRoot;
    }


/*---------------------------------------------------------------------------------**//**
* @description Intersect the cone with a line.
* @param pCone  => cone to evaluate
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the cone frame.   (Points on the
*                       unit sphere).  May be NULL.
* @param pLineParameter <= array of 0, 1, or 2 parameters with respect to the line.
* @param pRay => (unbounded) ray to intersect with cone.
* @return number of intersections.
* @group "DCone3d Intersection"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDRay3d

(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DRay3dCP pRay
)
    {
    return bsiDCone3d_intersectDRay3dExt (pCone, NULL, NULL, NULL, pXYZ, pUVW, pLineParameter, pRay);
    }


/*---------------------------------------------------------------------------------**//**
* @description Intersect an (unbounded) ray with the angularly complete cone/cylinder surface patch
*   and (optionally) the cap ellipse surface patches.  ANGULAR LIMITS ARE NOT CONSIDERED.
* @param pCone  => cone to evaluate
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the cone frame.   May be NULL.
* @param pLineParameter <= array of 0, 1, or 2 parameters with respect to the line.
* @param pRay => (unbounded) ray to intersect with (capped) cone/cylinder.
* @param includeEndCaps => Whether to include the elliptical caps of the cone
* @return number of intersections.
* @group "DCone3d Intersection"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDRay3dBoundedComplete

(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DRay3dCP pRay,
bool              includeEndCaps
)
    {
    double   param[4];
    Transform inverseFrame;
    DRay3d  localRay;
    double testParam;
    DPoint3d localPoint;
    int i0, i1, i;
    bool    inverseOK;

    int numParam = bsiDCone3d_intersectDRay3dExt
                        (pCone, &inverseFrame, &inverseOK, &localRay, NULL, NULL, param, pRay);

    if (inverseOK)
        {
        /* z coordinate on the ray is origin + lambda * direction.
            Intersections with parameter space "y" values are potential cap entries.
        */
        if (includeEndCaps)
            {
            if (bsiTrig_safeDivide (&param[numParam],
                            pCone->parameterRange.low.y  - localRay.origin.z,
                            localRay.direction.z,
                            0.0))
                numParam++;

            if (bsiTrig_safeDivide (&param[numParam],
                            pCone->parameterRange.high.y - localRay.origin.z,
                            localRay.direction.z,
                            0.0))
                numParam++;

            /* The intersections with the cylinder and cap planes may be mixed in any order,
                and can have duplicate points (parameters) at edge intersections.
                There should only be one finite (nonzero length) "in" segment,
                possibly adjacent to 2 zero length segments at edge intersections.
                Test each segment, not paying attention to whether it has zero length,
                and record low and high parameter limits of "in" segments.
            */
            if (numParam >= 2)
                {
                bsiDoubleArray_sort (param, numParam, true);
                i0 = numParam;
                i1 = 0;
                for (i = 1; i < numParam; i++)
                    {
                    testParam = 0.5 * (param[i-1] + param[i]);
                    bsiDRay3d_evaluateDPoint3d (&localRay, &localPoint, testParam);
                    if (bsiDCone3d_isLocalDPoint3dInSelectiveBounds
                                    (pCone, &localPoint, false, true, true))
                        {
                        if (i0 == numParam)
                            i0 = i - 1;
                        i1 = i;
                        }
                    }

                if (i0 < i1)
                    {
                    if (pLineParameter)
                        {
                        pLineParameter[0] = param[i0];
                        pLineParameter[1] = param[i1];
                        }

                    if (pXYZ)
                        {
                        bsiDRay3d_evaluateDPoint3d (pRay, &pXYZ[0], param[i0]);
                        bsiDRay3d_evaluateDPoint3d (pRay, &pXYZ[1], param[i1]);
                        }

                    if (pUVW)
                        {
                        bsiDRay3d_evaluateDPoint3d (&localRay, &pUVW[0], param[i0]);
                        bsiDRay3d_evaluateDPoint3d (&localRay, &pUVW[1], param[i1]);
                        }

                    return 2;
                    }
                }
            }
        else
            {
            /* Uncapped case.  Accept 0, 1, or 2 axial points.*/
            int numOut = 0;
            for (i = 0; i < numParam; i++)
                {
                testParam = param[i];
                bsiDRay3d_evaluateDPoint3d (&localRay, &localPoint, testParam);
                if (bsiDCone3d_isLocalDPoint3dInSelectiveBounds
                                (pCone, &localPoint, false, true, false))
                    {
                    if (pLineParameter)
                        pLineParameter[numOut] = testParam;
                    if (pXYZ)
                        bsiDRay3d_evaluateDPoint3d (pRay, &pXYZ[numOut], testParam);
                    if (pUVW)
                        bsiDRay3d_evaluateDPoint3d (&localRay, &pUVW[numOut], testParam);
                    numOut++;
                    }
                }
            return numOut;
            }
        }
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Intersect an (unbounded) line with the angularly complete cone/cylinder patch and the (optional) planar caps.
* @param pCone  => cone to evaluate
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the cone frame.   May be NULL.
* @param pLineParameter <= array of 0,1, or 2 parameters with respect to the line.
* @param pSegment => (unbounded) line to intersect with (capped) cone/cylinder.
* @param includeEndCaps => Whether to include the elliptical caps of the cone
* @return number of intersections.
* @group "DCone3d Intersection"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDCone3d_intersectDSegment3dBoundedComplete

(
DCone3dCP pCone,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double            *pLineParameter,
DSegment3dCP pSegment,
bool              includeEndCaps
)
    {
    DRay3d ray;
    bsiDRay3d_initFromDSegment3d (&ray, pSegment);
    return bsiDCone3d_intersectDRay3dBoundedComplete
                (
                pCone,
                pXYZ,
                pUVW,
                pLineParameter,
                &ray,
                includeEndCaps
                );
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the cone's local radius at the specified local height.
* @param pCone  => cone to evaluate
* @param z => height in local coordinates
* @return the cone's radius in local coordinates
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDCone3d_heightToRadius

(
DCone3dCP pCone,
double          z
)
    {
    return 1.0 + (pCone->radiusFraction - 1.0) * z;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute angles for silhouettes of the cone with respect to a (possibly perspective) view transformation.
* @param pCone  => cone to evaluate
* @param pTrigPoint <= array where x,y are cosine, sine of
*                      silhouette angles. z is zero -- maybe a convenient
*                      place for the angles if you need to stash them
* @param pMap       => view transformation
* @param pEyePoint => eyepoint, in same coordinates.
*                     For perspective, from xyz, set w=1
*                     For flat view in direction xyz, set w=0
* @return number of silhouette angles.
* @group "DCone3d Silhouette"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiDCone3d_silhouetteAngles

(
DCone3dCP pCone,
DPoint3dP pTrigPoint,
DMap4dCP pMap,
DPoint4dCP pEyePoint
)
    {
    Transform Tinverse;
    int numOut = 0;
    DPoint4d cylinderEye, coneEye;
    double lambda = pCone->radiusFraction;
    double mu = 1.0 - lambda;
    static double s_coneSilhouetteTangencyTolerance = 1.0e-14;
    int i;


    if (Tinverse.InverseOf (pCone->frame))

        {
        cylinderEye = *pEyePoint;
        /* invert the eye as needed: */
        if (pMap)
            bsiDMatrix4d_multiply4dPoints (&pMap->M1, &cylinderEye, &cylinderEye, 1);
        bsiTransform_multiplyDPoint4dArray (&Tinverse, &cylinderEye, &cylinderEye, 1);

        /* Homogeneous transform to put cone apex at infinity */
        coneEye.x = cylinderEye.x;
        coneEye.y = cylinderEye.y;
        coneEye.z = cylinderEye.z;
        coneEye.w = cylinderEye.w - mu * cylinderEye.z;

        pTrigPoint[0].Zero ();
        pTrigPoint[1].Zero ();

        numOut = bsiMath_solveApproximateUnitQuadratic
                        (
                        &pTrigPoint[0].x, &pTrigPoint[0].y,
                        &pTrigPoint[1].x, &pTrigPoint[1].y,
                        -coneEye.w,
                        coneEye.x,
                        coneEye.y,
                        s_coneSilhouetteTangencyTolerance
                        );
        for (i = 0; i < numOut; i++)
            {
            pTrigPoint[i].z = atan2 (pTrigPoint[i].y, pTrigPoint[i].x);
            }

        }

    return numOut;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a rule line at specified longitude (angle around cone).
* to the z range
* @param pCone  => cone to evaluate
* @param pSegment  <= ruling segment.
* @param theta => longitude angle (radians)
* @return true if theta is within the parameter range for the cone.
* @group "DCone3d Rule Lines"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDCone3d_getRuleLine

(
DCone3dCP pCone,
DSegment3dP pSegment,
double          theta
)
    {
    double              cosTheta = cos (theta);
    double              sinTheta = sin (theta);
    double              r1 = pCone->radiusFraction;
    double              z0, z1, theta0, theta1;
    double              sweep;

    bsiDCone3d_getScalarNaturalParameterRange (pCone, &theta0, &theta1, &z0, &z1);
    sweep = theta1 - theta0;

    pCone->frame.Multiply (*(&pSegment->point[0]), cosTheta, sinTheta, z0);
    pCone->frame.Multiply (*(&pSegment->point[1]), r1 * cosTheta, r1 * sinTheta, z1);

    return bsiTrig_angleInSweep (theta, theta0, sweep);
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a cross section at given height along the cone axis.
* @param pCone  => cone to evaluate
* @param pEllipse <= full ellipse at specified latitude.  0 and 90 degree vectors
*           are 0 and 90 degrees latitude.
* @param z      => altitude parameter
* @return true if the altitude value is within the cone parameter range.
* @group "DCone3d Rule Lines"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDCone3d_getCrossSection

(
DCone3dCP pCone,
DEllipse3dP pEllipse,
double          z
)
    {
    DVec3d              vector0, vector90;
    DPoint3d            center;
    double              mu = pCone->radiusFraction - 1.0;
    double              r  = 1.0 + mu * z;
    double              z0, dz, theta0, dTheta;

    bsiDCone3d_getScalarNaturalParameterSweep (pCone, &theta0, &dTheta, &z0, &dz);

    pCone->frame.MultiplyMatrixOnly (vector0, r, 0.0, 0.0);
    pCone->frame.MultiplyMatrixOnly (vector90, 0.0, r, 0.0);
    pCone->frame.Multiply (center,   0.0, 0.0, z);

    bsiDEllipse3d_initFrom3dVectors (pEllipse, &center, &vector0, &vector90,
                        theta0, dTheta);

    return bsiTrig_scalarInSweep (z, z0, dz);
    }



/*---------------------------------------------------------------------------------**//**
* @description Compute a point on the cone given fractional parameters.
* @param pCone  => cone to evaluate
* @param pPoint <= evaluated point
* @param angleFraction => angular position, as a fraction of the patch angle range.
* @param zFraction => axial position, as a fraction of the patch z range.
* @group "DCone3d Parameterization"
* @return true
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDCone3d_fractionParameterToDPoint3d

(
DCone3dCP pCone,
DPoint3dP pPoint,
double    angleFraction,
double    zFraction
)
    {
    double theta, z;
    bsiDCone3d_fractionParameterToNaturalParameter (pCone, &theta, &z, angleFraction, zFraction);
    return bsiDCone3d_naturalParameterToDPoint3d (pCone, pPoint, theta, z);
    }


/*---------------------------------------------------------------------------------**//**
* @description Invert the point onto the cone and return fractional parameters.
* @param pCone  => cone to evaluate
* @param pThetaFraction  <= angular position, as a fraction of the patch angle range.
* @param pZFraction      <= axial position, as a fraction of the patch z range.
* @param pPoint         => evaluated point
* @group "DCone3d Parameterization"
* @return false if the local point is on the cone/cylinder axis or the local to world transformation is singular.
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDCone3d_dPoint3dToFractionParameter

(
DCone3dCP pCone,
double    *pThetaFraction,
double    *pZFraction,
DPoint3dCP pPoint
)
    {
    double theta, z;
    bool    stat = bsiDCone3d_dPoint3dToNaturalParameter (pCone, &theta, &z, pPoint);
    bsiDCone3d_naturalParameterToFractionParameter (pCone, pThetaFraction, pZFraction, theta, z);
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* @description Convert fractional parameters to natural parameters.
* @param pCone  => cone to evaluate
* @param pTheta => angle around cross sectional circle
* @param pZ     => height along (local) z axis
* @param thetaFraction => angular position, as a fraction of the patch angle range.
* @param zFraction => axial position, as a fraction of the patch z range.
* @group "DCone3d Parameterization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDCone3d_fractionParameterToNaturalParameter

(
DCone3dCP pCone,
double    *pTheta,
double    *pZ,
double    thetaFraction,
double    zFraction
)
    {
    if (pTheta)
        *pTheta = pCone->parameterRange.low.x +
                            thetaFraction * (pCone->parameterRange.high.x - pCone->parameterRange.low.x);

    if (pZ)
        *pZ = pCone->parameterRange.low.y +
                            zFraction * (pCone->parameterRange.high.y - pCone->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* @description Convert natural parameters to fractional parameters.
* @param pCone  => cone to evaluate
* @param pThetaFraction => angle parameter as a fraction of the patch.
* @param pZFraction => axis parameter as fraction of the patch.
* @param theta => natural angular position
* @param z => natural axial position
* @group "DCone3d Parameterization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDCone3d_naturalParameterToFractionParameter

(
DCone3dCP pCone,
double    *pThetaFraction,
double    *pZFraction,
double    theta,
double    z
)
    {
    if (pThetaFraction)
        *pThetaFraction = bsiTrig_normalizeAngleToSweep (theta, pCone->parameterRange.low.x,
                            pCone->parameterRange.high.x - pCone->parameterRange.low.x);

    if (pZFraction)
        bsiTrig_safeInverseLinearInterpolate (pZFraction,
                                        z,
                                        pCone->parameterRange.low.y,
                                        pCone->parameterRange.high.y,
                                        0.0);
    }


/*---------------------------------------------------------------------------------**//**
* @description Test whether the cone range is the full parameter space.
* @return true if the cone range is complete.
* @param pCone  =>  cone to evaluate
* @group "DCone3d Parameterization"
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDCone3d_isComplete

(
DCone3dCP pCone
)
    {
    return bsiDCone3d_isParam1Complete (pCone)
            && bsiDCone3d_isParam2Complete (pCone);
    }


/*---------------------------------------------------------------------------------**//**
* @description Test whether the cone angle parameter range covers the complete circular range of the cross sections.
* @return true if the cone angle parameter range is complete.
* @param pCone  =>  cone to evaluate
* @group "DCone3d Parameterization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDCone3d_isParam1Complete

(
DCone3dCP pCone
)
    {
    return bsiTrig_isAngleFullCircle (pCone->parameterRange.high.x - pCone->parameterRange.low.x);
    }


/*---------------------------------------------------------------------------------**//**
* @description Test whether the cone z range covers the complete axis of the local coordinate system.
* @return true if the z range is complete.
* @param pCone  =>  cone to evaluate
* @group "DCone3d Parameterization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDCone3d_isParam2Complete

(
DCone3dCP pCone
)
    {
    static double s_relTol = 1.0e-12;
    double nearOne = 1.0 - s_relTol;
    return  (pCone->parameterRange.high.y >= nearOne && pCone->parameterRange.low.y  < s_relTol)
        ||  (pCone->parameterRange.low.y  >= nearOne && pCone->parameterRange.high.y < s_relTol);
    }


/*---------------------------------------------------------------------------------**//**
* @description Get the reference frame of the cone.
* @param pCone      => cone to evaluate
* @param pFrame         <= reference frame
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDCone3d_getFrame

(
DCone3dCP pCone,
TransformP pFrame
)
    {
    *pFrame = pCone->frame;
    }



/*---------------------------------------------------------------------------------**//**
* @description Get the reference frame of the cone.
* @param pCone      => cone to evaluate
* @param pFrame         <= reference frame
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDCone3d_getTransform

(
DCone3dCP pCone,
TransformP pFrame
)
    {
    *pFrame = pCone->frame;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get the cone's inverse coordinate, i.e. transform from the global space to
* a the system where the base is a unit xy circle.
* @param pCone      => cone to evaluate
* @param pInverseFrame  <= inverse frame.
* @return true if the inverse is was computed.
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDCone3d_getInverseFrame

(
DCone3dCP pCone,
TransformP pInverseFrame
)
    {
    return pInverseFrame->InverseOf (pCone->frame);
    }


/*---------------------------------------------------------------------------------**//**
* @description Get the cone's inverse coordinate, i.e. transform from the global space to
* a the system where the base is a unit xy circle.
* @param pCone      => cone to evaluate
* @param pInverseFrame  <= inverse frame.
* @return true if the inverse is was computed.
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDCone3d_getInverseTransform

(
DCone3dCP pCone,
TransformP pInverseFrame
)
    {
    return pInverseFrame->InverseOf (pCone->frame);
    }



/*---------------------------------------------------------------------------------**//**
* @description Evaluate the implicit function for the cone.
* @param pCone  => cone to evaluate
* @param pPoint => point where the implicit function is evaluated.
* @return Value of implicit function (equals zero if point is on the cone)
* @group "DCone3d Implicit"
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDCone3d_implicitFunctionValue

(
DCone3dCP pCone,
DPoint3dCP pPoint
)
    {
    DPoint3d localPoint;
    double   mu = pCone->radiusFraction - 1.0;
    double f = 0.0;
    double r;

    if (bsiDCone3d_worldToLocal (pCone, &localPoint, pPoint))
        {
        r = 1.0 + mu * localPoint.z;
        f = localPoint.x * localPoint.x + localPoint.y * localPoint.y - r * r;
        }

    return f;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute a point on the cone given its parameters.
* @param pCone  => cone to evaluate
* @param pPoint <= evaluated point
* @param theta => angle around cone
* @param z     => fraction of z axis.
* @return true
* @group "DCone3d Parameterization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDCone3d_naturalParameterToDPoint3d

(
DCone3dCP pCone,
DPoint3dP pPoint,
double    theta,
double    z
)
    {
    double mu = pCone->radiusFraction - 1.0;
    double r = 1.0 + mu * z;
    double cosTheta = cos (theta);
    double sinTheta = sin (theta);

    pCone->frame.Multiply (*pPoint,
                r * cosTheta,
                r * sinTheta,
                z
                );
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute a natural parameter of a given xyz point.  When the point is exactly on the
* surface, this function is the inverse of naturalParameterToPoint.   For a point off the
* surface, this inversion should be predictable but need not be a true projection.
* @param pCone  => cone to evaluate
* @param pParam1 <= natural parameter
* @param pParam2 <= natural parameter
* @param pPoint  => xyz coordinates
* @return false if the local point is on the cone/cylinder axis or the local to world transformation is singular.
* @group "DCone3d Projection"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiDCone3d_dPoint3dToNaturalParameter

(
DCone3dCP pCone,
double    *pParam1,
double    *pParam2,
DPoint3dCP pPoint
)
    {
    DPoint3d localPoint;
    bool    boolstat = bsiDCone3d_worldToLocal (pCone, &localPoint, pPoint);
    if (boolstat)
        {
        boolstat = bsiDCone3d_localToCylindrical (pCone, pParam1, pParam2, NULL, &localPoint);
        }
    return boolstat;
    }



/*---------------------------------------------------------------------------------**//**
* @description Return the range of the natural parameter for the active surface patch.
* @param pCone  => cone to evaluate
* @param pParam1Start => start value of natural parameter.
* @param pParam1End   => end value of natural parameter.
* @param pParam2Start => start value of natural parameter.
* @param pParam2End   => end value of natural parameter.
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDCone3d_getScalarNaturalParameterRange

(
DCone3dCP pCone,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
)
    {
    *pParam1Start = pCone->parameterRange.low.x;
    *pParam1End   = pCone->parameterRange.high.x;
    *pParam2Start = pCone->parameterRange.low.y;
    *pParam2End   = pCone->parameterRange.high.y;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get the parameter range of the cone.
* @param pCone          => cone to evaluate
* @param pParameterRange    <= natural parameter range
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDCone3d_getNaturalParameterRange

(
DCone3dCP pCone,
DRange2dP pParameterRange
)
    {
    *pParameterRange = pCone->parameterRange;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get the parameter range as start/sweep pairs.
* @param pCone      => cone to evaluate
* @param pTheta0        <= start angle
* @param pThetaSweep    <= angle sweep
* @param pZ0            <= start altitude
* @param pZSweep        <= altitude sweep
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDCone3d_getScalarNaturalParameterSweep

(
DCone3dCP pCone,
double          *pTheta0,
double          *pThetaSweep,
double          *pZ0,
double          *pZSweep
)
    {
    *pTheta0 = pCone->parameterRange.low.x;
    *pThetaSweep = pCone->parameterRange.high.x - *pTheta0;

    *pZ0 = pCone->parameterRange.low.y;
    *pZSweep = pCone->parameterRange.high.y - *pZ0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the range of the natural parameters for a complete surface.
* @param pCone        => cone to evaluate
* @param pParam1Start <= start value of natural parameter.
* @param pParam1End   <= end value of natural parameter.
* @param pParam2Start <= start value of natural parameter.
* @param pParam2End   <= end value of natural parameter.
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDCone3d_getCompleteNaturalParameterRange

(
DCone3dCP pCone,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
)
    {
    *pParam1Start = -msGeomConst_pi;
    *pParam1End   =  msGeomConst_pi;
    *pParam1Start =  0.0;
    *pParam1End   =  1.0;
    }

/*---------------------------------------------------------------------------------**//**
@description Return intersections of a bezier curve with the cone.
@param pCone        => cone to evaluate
@param pIntersectionParam OUT intersection parameters on the bezier
@param pIntersectionXYZ OUT world coordinate intersection points
@param pIntersectionUVW OUT local frame intersection points
@param pNumConeIntersection OUT number of intersections on cone
@param pNumCapIntersection OUT number of intersections on cap.
@param maxIntersection IN size limit for all output arrays.
@param pBezierPoles IN bezier curve
@param order IN bezier order
@param coneSelect IN
    <list>
    <item>0 for no cone intersections.</item>
    <item>1 for cone intersections with unbounded cone.</item>
    <item>2 for cone intersections within 01 w limits.</item>
    <item>3 for cone intersections with bounded patch</item>
    </list>
@param capSelect IN
    <list>
    <item>0 for no cap intersections.</item>
    <item>1 for intersections with unbounded cap planes at 01 w.</item>
    <item>2 for intersections within bounded cap disks at 01 w.</item>
    <item>3 for intersections within bounded cap disks at bounded patch w limits.</item>
    </list>
@group "DCone3d Parameter Range"
@bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDCone3d_intersectBezierCurve
(
DCone3dCP pCone,
double *pIntersectionParam,
DPoint3d *pIntersectionXYZ,
DPoint3d *pIntersectionUVW,
int *pNumConeIntersection,
int *pNumCapIntersection,
int maxIntersection,
DPoint4dCP pWorldPoles,
int        order,
int        coneSelect,
int        capSelect
)
    {
    if (pNumConeIntersection)
        *pNumConeIntersection = 0;
    if (pNumCapIntersection)
        *pNumCapIntersection = 0;
    int num0 = 0;
    int num1 = 0;
    int num  = 0;
    Transform worldToLocal;
    if (!bsiDCone3d_getInverseTransform (pCone, &worldToLocal))
        return false;
    if (2 * order > MAX_BEZIER_ORDER)
        return false;

    DPoint4d localPoles[MAX_BEZIER_ORDER];
    double rootPoles[MAX_BEZIER_ORDER];
    DPoint3d rootUVW[MAX_BEZIER_ORDER];
    DPoint3d rootXYZ[MAX_BEZIER_ORDER];
    double roots[MAX_BEZIER_ORDER];
    double mu = 1.0 - pCone->radiusFraction;
    bsiTransform_multiplyDPoint4dArray (&worldToLocal, localPoles, pWorldPoles, order);
    if (coneSelect != 0)
        {
        // implicit equation of cone izzz....
        //   x^2 + y^2 = (1 - mu z)^2
        // with homogeneous coordinates in the bezier
        // X^2/W^2 + Y^2/W^2 = (1 - mu Z/W)^2
        // W^2 -2 * mu * Z * W + mu * mu * Z^2 - X^2 - Y^2 = 0
        // WW
        bsiBezier_univariateProduct (rootPoles, 0, 1,
                    (double*)localPoles, order, 3, 4,
                    (double*)localPoles, order, 3, 4);

        // - 2 mu Z W
        bsiBezier_accumulateUnivariateProduct (rootPoles, 0, 1, -2.0 * mu,
                    (double*)localPoles, order, 2, 4,
                    (double*)localPoles, order, 3, 4);
        bsiBezier_accumulateUnivariateProduct (rootPoles, 0, 1, mu * mu,
                    (double*)localPoles, order, 2, 4,
                    (double*)localPoles, order, 2, 4);

        bsiBezier_accumulateUnivariateProduct (rootPoles, 0, 1, -1.0,
                    (double*)localPoles, order, 0, 4,
                    (double*)localPoles, order, 0, 4);
        bsiBezier_accumulateUnivariateProduct (rootPoles, 0, 1, -1.0,
                    (double*)localPoles, order, 1, 4,
                    (double*)localPoles, order, 1, 4);

        int numConeRoot;

        int productOrder = 2 * order - 1;
        if (bsiBezier_univariateRoots (roots, &numConeRoot, rootPoles, productOrder)
            && numConeRoot > 0
            && numConeRoot < productOrder)
            {
            bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                (rootUVW, NULL, NULL, localPoles, order, roots, numConeRoot);
            if (NULL != pIntersectionXYZ)
                bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                    (rootXYZ, NULL, NULL, pWorldPoles, order, roots, numConeRoot);

            for (int i = 0; i < numConeRoot; i++)
                {
                DPoint3d uvw = rootUVW[i];
                bool bAccept = true;
                if (coneSelect == 2)
                    bAccept = uvw.z >= 0.0 && uvw.z <= 1.0;
                else if (coneSelect == 3)
                    bAccept =
                        bsiDCone3d_isLocalDPoint3dInSelectiveBounds (pCone, &uvw,
                                        true, false, true) ? true : false;

                if (bAccept && num < maxIntersection)
                    {
                    if (NULL != pIntersectionParam)
                        pIntersectionParam[num] = roots[i];
                    if (NULL != pIntersectionXYZ)
                        pIntersectionXYZ[num] = rootXYZ[i];
                    if (NULL != pIntersectionUVW)
                        pIntersectionUVW[num] = uvw;
                    num0++;
                    num++;
                    }
                }
            }
        }

    if (capSelect != 0)
        {
        double capW[2];
        if (capSelect == 3)
            {
            capW[0] = pCone->parameterRange.low.y;
            capW[1] = pCone->parameterRange.high.y;
            }
        else
            {
            capW[0] = 0.0;
            capW[1] = 1.0;
            }

        // Intersect with each of the two planes ...
        for (int k = 0; k < 2; k++)
            {
            // univariate bezier is (z(s)/w(s))=capW[k],
            //     z(s)=capW[k]*w(s)
            for (int i = 0; i < order; i++)
                rootPoles[i] = localPoles[i].z - localPoles[i].w * capW[k];

            int numCapRoot;

            if (   bsiBezier_univariateRoots (roots, &numCapRoot, rootPoles, order)
                && numCapRoot > 0
                && numCapRoot < order)
                {
                bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                    (rootUVW, NULL, NULL, localPoles, order, roots, numCapRoot);
                if (NULL != pIntersectionXYZ)
                    bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                        (rootXYZ, NULL, NULL, pWorldPoles, order, roots, numCapRoot);

                for (int i = 0; i < numCapRoot; i++)
                    {
                    DPoint3d uvw = rootUVW[i];
                    bool bAccept = true;
                    if (capSelect == 2 || capSelect == 3)
                        {
                        double r = bsiDCone3d_heightToRadius (pCone, capW[k]);
                        bAccept = uvw.x * uvw.x + uvw.y * uvw.y <= r * r;
                        }

                    if (bAccept && num < maxIntersection)
                        {
                        if (NULL != pIntersectionParam)
                            pIntersectionParam[num] = roots[i];
                        if (NULL != pIntersectionXYZ)
                            pIntersectionXYZ[num] = rootXYZ[i];
                        if (NULL != pIntersectionUVW)
                            pIntersectionUVW[num] = uvw;
                        num1++;
                        num++;
                        }
                    }
                }
            }
        }
    *pNumConeIntersection = num0;
    *pNumCapIntersection  = num1;
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

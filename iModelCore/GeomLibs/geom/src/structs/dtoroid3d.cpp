/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dtoroid3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @struct DToroid3d
* A DToroid3d structure defines a 3 dimensional toroid (possibly with elliptial
*       primary cross sections).
* A DToroid3d is a toroidal surface in 3d.
*
* The The parameterization of the surface is
*
*<pre>
*       X = C
*         + ( 1 + minorRadiusRatio * cos(phi)) * ( U * cos(theta) + V * sin(theta))
*         + sin(phi) * minorAxisRatio * W
*</pre>
* where
*<ul>
*<li> C is the center of the overall torus </li>
*<li> U is the vector from the center to the 0-degree point on the primary circle. </li>
*<li> V is the vector from the center to the 90-degree point on the primary circle. </li>
*<li> W is the reference out-of-plane vector.</li>
*<li> minorRadiusRatio is the radius of the minor hoop in the local coordinate system..</li>
*</ul>
*</pre>
*
* If the coordinate system is an identity matrix, the major radius is 1 and minor radius
*  is minorRadiusRatio.
*
* The center (C) and vectors (U,V,W) of the surface parameterization define a coordinate frame.
*
* @fields
* @field Transform frame Defining coordinate system.
* @field double minorAxisRatio size of minor (longitudinal) circle, as a fraction of
*           the major (latitudinal) circle.
* @field DRange2d parameterRange angular range a rectangular patch of the parameter space.
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static const DRange2d s_defaultParameterRange =
            {
                {-msGeomConst_pi, -msGeomConst_pi},
                { msGeomConst_pi,  msGeomConst_pi}
            };

/*---------------------------------------------------------------------------------**//**
* Force positive radii.  Changes are probably worthless for applications, but will prevent
* divide by zero.
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void forcePositiveRadii

(
double      *pMajorRadius,
double      *pMinorRadius,
double      majorRadius,
double      minorRadius
)
    {

    if (majorRadius <= 0.0 && minorRadius <= 0.0)
        {
        majorRadius = 1.0;
        minorRadius = 0.5;
        }
    else if (majorRadius <= 0.0)
        {
        majorRadius = 2.0 * minorRadius;
        }
    else if (minorRadius <= 0.0)
        {
        minorRadius = 0.5 * majorRadius;
        }

    if (pMajorRadius)
        *pMajorRadius = majorRadius;

    if (pMinorRadius)
        *pMinorRadius = minorRadius;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize an toroid from center, major and minor radii and optional parameter range.
* Orientation is set parallel to global axes.
* @param pCenter => toroid center.
* @param radius  => radius of toroid.
* @param pParameterRange => parameter range. If NULL, default is applied.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDToroid3d_setCenterRadii

(
DToroid3dP pInstance,
DPoint3dCP pCenter,
double          majorRadius,
double          minorRadius,
DRange2dCP pParameterRange
)
    {
    forcePositiveRadii (&majorRadius, &minorRadius, majorRadius, minorRadius);

    pInstance->frame.InitFromScaleFactors (majorRadius, majorRadius, majorRadius);
    if (nullptr != pCenter)
        pInstance->frame.SetTranslation (*pCenter);
    pInstance->minorAxisRatio = minorRadius / majorRadius;
    bsiDToroid3d_setNaturalParameterRange (pInstance, pParameterRange);
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
Public GEOMDLLIMPEXP void               bsiDToroid3d_set

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
* Set the reference frame of the toroid.
* @param pFrame => coordinate frame.  null indicates an identity transformation.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDToroid3d_setFrame

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
Public GEOMDLLIMPEXP void               bsiDToroid3d_setNaturalParameterRange

(
DToroid3dP pInstance,
DRange2dCP pParameterRange
)
    {
    pInstance->parameterRange = pParameterRange ? *pParameterRange : s_defaultParameterRange;
    }


/*---------------------------------------------------------------------------------**//**
* Convert a local cartesian point into the local toroidal coordinates.
* @param pTheta     <= angle in unit spherical coordiantes
* @param pPhi       <= z coordinate in spherical coordinates
* @param pR         <= radius from major circle, as a multiple of the minor circle radius.
* @return false if the local point is on the z axis or the major circle.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDToroid3d_localToToroidal

(
DToroid3dCP pToroid,
double    *pTheta,
double    *pPhi,
double    *pR,
DPoint3dCP pPoint
)
    {

    double rho2 = pPoint->x * pPoint->x + pPoint->y * pPoint->y;
    double rho = sqrt (rho2);
    double beta = rho - 1.0;
    double rr = beta * beta + pPoint->z * pPoint->z;


    if (pTheta)
        *pTheta = bsiTrig_atan2 (pPoint->y, pPoint->x);

    if (pPhi)
        {
        *pPhi = bsiTrig_atan2 (pPoint->z, beta);
        }

    if (pR)
        bsiTrig_safeDivide ( pR, sqrt (rr), pToroid->minorAxisRatio, 0.0);

    return rho2 != 0.0 && rr != 0.0;
    }



/*---------------------------------------------------------------------------------**//**
* test if a longitude angle is in the ellipsoid's parameter range.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDToroid3d_longitudeInRange

(
DToroid3dCP pInstance,
double      longitude
)
    {
    return bsiTrig_angleInSweep
                    (
                    longitude,
                    pInstance->parameterRange.low.x,
                    pInstance->parameterRange.high.x - pInstance->parameterRange.low.x
                    );
    }


/*---------------------------------------------------------------------------------**//**
* test if a latitude is in the ellipsoid's parameter range.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDToroid3d_latitudeInRange

(
DToroid3dCP pInstance,
double          latitude
)
    {
    return bsiTrig_angleInSweep
                    (
                    latitude,
                    pInstance->parameterRange.low.y,
                    pInstance->parameterRange.high.y - pInstance->parameterRange.low.y
                    );
    }


/*---------------------------------------------------------------------------------**//**
* Intersect the toroid with a line segment.
* @param pXYZ   <= array of 0 to 4 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0 to 4 coordinates of intersection points in the
*                       local coordinates of the toroid frame.   (Points on the
*                       unit sphere).  May be NULL.
* @param pLineParameter <= array of 0 to 4 parameters with respect to the line.
* @param DRay3d         => ray to intersect.
* @return number of intersections.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int                bsiDToroid3d_intersectDSegment3d

(
DToroid3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DSegment3dCP pSegment
)
    {
    RotatedConic hConic;
    DPoint4d     localPoint[4];
    DPoint3d    worldPoint[4];
    DPoint3d    lineParam[4];
    int         numIntersection = 0;
    int         i;

    bsiDToroid3d_getRotatedConic (pInstance, &hConic);

    if (SUCCESS == bsiRotatedConic_intersectLine
                            (
                            worldPoint, localPoint, lineParam,
                            &numIntersection,
                            &hConic,
                            pSegment->point
                            )
        && numIntersection > 0
        )
        {
        if (pXYZ)
            bsiDPoint3d_copyArray (pXYZ, worldPoint, numIntersection);

        if (pUVW)
            {
            for (i = 0; i < numIntersection; i++)
                {
                localPoint[i].GetProjectedXYZ (pUVW[i]);
                }
            }
        if (pLineParameter)
            {
            /* Simple parameter is second homogeneous parameter */
            for (i = 0; i < numIntersection; i++)
                {
                pLineParameter[i] = lineParam[i].y;
                }
            }
        }

    return numIntersection;
    }


/*---------------------------------------------------------------------------------**//**
* Intersect the toroid with a ray.   Beware that up to 4 intersection points may
* be returned.
* @param pXYZ   <= array of 0 to 4 (!!!) coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0 to 4 (!!!) coordinates of intersection points in the
*                       local coordinates of the toroid frame.   (Points on the
*                       unit sphere).  May be NULL.
* @param pLineParameter <= array of 0 to 4 (!!!) parameters with respect to the line.
* @return number of intersections.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int                bsiDToroid3d_intersectDRay3d

(
DToroid3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DRay3dCP pRay
)
    {
    DPoint4d poles[2];
    DPoint3d point0, point1;
    bsiDRay3d_evaluateEndPoints (pRay, &point0, &point1);
    poles[0].Init (point0, 1.0);
    poles[1].Init (point1, 1.0);
    int n0, n1;
    bsiDToroid3d_intersectBezierCurve (pInstance, pLineParameter,
                    pXYZ, pUVW, &n0, &n1, 4,
                    poles, 2, 1, 0, true);
    return n0;
    }


/*-----------------------------------------------------------------*//**
*
* Apply a transformation to the source ellipseoid.  Only affine parts of the homogeneous
* matrix are applied.
* @param pTransform => transformation to apply.
* @param pSource => source ellipse.
* @indexVerb transform
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid_applyAffineDMatrix4d

(
DToroid3dP pDest,
DMatrix4dCP pMatrix,
DToroid3dCP pSource
)
    {
    pMatrix->MultiplyAffine (pDest->frame, pSource->frame);
    pDest->parameterRange = pSource->parameterRange;
    }


/*---------------------------------------------------------------------------------**//**
* Return a (range restricted) meridian at specified longitude.
* @param pEllipse <= full ellipse at specified longitude.   0-degree vector is on
*           is on the equator.  90 degree vector is to the north pole.
* @param longitude => longitude angle (radians)
* @indexVerb ruleLines
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_getMeridian
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      longitude
)
    {
    DPoint3d    center;
    DVec3d      vector0;
    DVec3d      vector90;
    double              cosTheta = cos (longitude);
    double              sinTheta = sin (longitude);

    double b = pToroid->minorAxisRatio;

    pToroid->frame.MultiplyMatrixOnly (vector0, cosTheta * b, sinTheta * b,  0.0);
    pToroid->frame.MultiplyMatrixOnly (vector90, 0.0,         0.0,           b);
    pToroid->frame.Multiply (center,   cosTheta,     sinTheta,      0.0);

    bsiDEllipse3d_initFrom3dVectors (pEllipse, &center, &vector0, &vector90,
                pToroid->parameterRange.low.y,
                pToroid->parameterRange.high.y - pToroid->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* Return a (range restricted) meridian at specified longitude.
* @param pEllipse <= full ellipse at specified longitude.   0-degree vector is on
*           is on the equator.  90 degree vector is to the north pole.
* @param longitude => longitude angle (radians)
* @indexVerb ruleLines
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_getMeridianLocal
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      longitude
)
    {
    double              cosTheta = cos (longitude);
    double              sinTheta = sin (longitude);

    double b = pToroid->minorAxisRatio;
    pEllipse->Init
            (
            cosTheta, sinTheta, 0.0,
            cosTheta * b, sinTheta * b, 0.0,
            0, 0, b,
            pToroid->parameterRange.low.y,
            pToroid->parameterRange.high.y - pToroid->parameterRange.low.y);

    }


/*---------------------------------------------------------------------------------**//**
* Return a (range restricted) parallel at specified cos and sine coordinates in the minor circle space.
  (usual usage is cos=+-1, sin=+-1 to get "square duct" edges for loose range)
* @param pEllipse <= full ellipse at specified longitude.   0-degree vector is on
*           is on the equator.  90 degree vector is to the north pole.
* @param x "cosine like" coordinate in small circle plane.
* @param y "sine like" coordinate in small circle plane
* @indexVerb ruleLines
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_getParallelFromSmallCircleCoordinates
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      x,
double      y
)
    {
    DVec3d      vector0, vector90;
    DPoint3d            center;
    double              b = pToroid->minorAxisRatio;
    double              u = 1.0 + b * x;

    
    pToroid->frame.MultiplyMatrixOnly (vector0,  u,      0.0,      0.0);
    pToroid->frame.MultiplyMatrixOnly (vector90, 0.0,    u,        0.0);
    pToroid->frame.Multiply (center,   0.0,    0.0,      b * y);

    bsiDEllipse3d_initFrom3dVectors (pEllipse, &center, &vector0, &vector90,
                pToroid->parameterRange.low.x,
                pToroid->parameterRange.high.x - pToroid->parameterRange.low.x);
    }


/*---------------------------------------------------------------------------------**//**
* Return a (full) parallel of latitude.
* @param pEllipse <= full ellipse at specified latitude.  0 and 90 degree vectors
*           are 0 and 90 degrees latitude.
* @param latitude => latitude angle (radians)
* @indexVerb ruleLines
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_getParallel
(
DToroid3dCP pToroid,
DEllipse3dP pEllipse,
double      latitude
)
    {
    bsiDToroid3d_getParallelFromSmallCircleCoordinates (pToroid, pEllipse, cos(latitude), sin(latitude));
    }


/*---------------------------------------------------------------------------------**//**
* Convert from parametric to cartesian, with the parametric coordinate given as trig values.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDToroid3d_trigParameterToPoint

(
DToroid3dCP pToroid,
DPoint3dP pPoint,
double      cosTheta,
double      sinTheta,
double      cosPhi,
double      sinPhi
)
    {
    double              b = pToroid->minorAxisRatio;
    double              u = 1.0 + b * cosPhi;

    pToroid->frame.Multiply (*pPoint,
                cosTheta * u,
                sinTheta * u,
                sinPhi * b
                );
    }


/*---------------------------------------------------------------------------------**//**
* @param pPoint <= evaluated point
* @param thetaFraction => angular position, as a fraction of the patch longitude range.
* @param phiFraction => axial position, as a fraction of the patch latitude range.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDToroid3d_fractionParameterToDPoint3d

(
DToroid3dCP pToroid,
DPoint3dP pPoint,
double    thetaFraction,
double    phiFraction
)
    {
    double theta, phi;
    bsiDToroid3d_fractionParameterToNaturalParameter (pToroid, &theta, &phi, thetaFraction, phiFraction);
    return bsiDToroid3d_naturalParameterToDPoint3d (pToroid, pPoint, theta, phi);
    }


/*---------------------------------------------------------------------------------**//**
* @param thetaFraction  <= longitude, as a fraction of the patch longitude range
* @param phiFraction    <= latitude, as a fraction of the latitude range
* @param pPoint         => evaluated point
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDToroid3d_dPoint3dToFractionParameter

(
DToroid3dCP pToroid,
double    *pThetaFraction,
double  *pPhiFraction,
DPoint3dCP pPoint
)
    {
    double theta, phi;
    bool    stat = bsiDToroid3d_dPoint3dToNaturalParameter (pToroid, &theta, &phi, pPoint);
    bsiDToroid3d_naturalParameterToFractionParameter (pToroid, pThetaFraction, pPhiFraction, theta, phi);
    return stat;
    }



/*---------------------------------------------------------------------------------**//**
* @param pTheta => longitude
* @param pPhi   => latitude
* @param thetaFraction => longitude, as a fraction of the patch longitude range.
* @param phiFraction => latitude, as a fraction of the patch latitude range.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_fractionParameterToNaturalParameter

(
DToroid3dCP pToroid,
double    *pTheta,
double    *pPhi,
double    thetaFraction,
double    phiFraction
)
    {
    if (pTheta)
        *pTheta = pToroid->parameterRange.low.x +
                            thetaFraction * (pToroid->parameterRange.high.x - pToroid->parameterRange.low.x);

    if (pPhi)
        *pPhi = pToroid->parameterRange.low.y +
                            phiFraction * (pToroid->parameterRange.high.y - pToroid->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* @param pTheta => longitude
* @param pPhi => latitude
* @param theta  => longitude
* @param phi    => latitude
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_naturalParameterToFractionParameter

(
DToroid3dCP pToroid,
double    *pThetaFraction,
double    *pPhiFraction,
double    theta,
double    phi
)
    {
    if (pThetaFraction)
        *pThetaFraction = bsiTrig_normalizeAngleToSweep (theta, pToroid->parameterRange.low.x,
                            pToroid->parameterRange.high.x - pToroid->parameterRange.low.x);

    if (pPhiFraction)
        *pPhiFraction = bsiTrig_normalizeAngleToSweep (phi, pToroid->parameterRange.low.y,
                            pToroid->parameterRange.high.y - pToroid->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* Test if the ellipsoid range is the full parameter space.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDToroid3d_isComplete

(
DToroid3dCP pInstance
)
    {
    return bsiDToroid3d_isParam1Complete (pInstance)
            && bsiDToroid3d_isParam2Complete (pInstance);
    }


/*---------------------------------------------------------------------------------**//**
* @return true if the 1st natural parameter covers the complete range of the underlying
*           analytic surface.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDToroid3d_isParam1Complete

(
DToroid3dCP pToroid
)
    {
    return bsiTrig_isAngleFullCircle (pToroid->parameterRange.high.x - pToroid->parameterRange.low.x);
    }


/*---------------------------------------------------------------------------------**//**
* @return true if the 2nd natural parameter covers the complete range of the underlying
*           analytic surface.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDToroid3d_isParam2Complete

(
DToroid3dCP pToroid
)
    {
    return bsiTrig_isAngleFullCircle (pToroid->parameterRange.high.y - pToroid->parameterRange.low.y);
    }



/*---------------------------------------------------------------------------------**//**
* Get the reference frame of the ellipsoid.
* @param pFrame     <= coordinate frame with (0,0,0) at ellispoid center, (0,0,1) at
*                       is the out-of-plane vector, (1,0,0) and (0,1,0) at 0 and 90 degrees longitude
*                       on the major circle.
*
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDToroid3d_getFrame

(
DToroid3dCP pInstance,
TransformP pFrame
)
    {
    *pFrame = pInstance->frame;
    }


/*---------------------------------------------------------------------------------**//**
* Get the ellise's inverse coordinate, i.e. transform from the global space to
* a the system where the base is a unit xy circle.
* @param pInverseFrame <= inverse frame.
* @return true if the inverse is was computed.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDToroid3d_getInverseFrame

(
DToroid3dCP pInstance,
TransformP pInverseFrame
)
    {
    return pInverseFrame->InverseOf (pInstance->frame);
    }


/*---------------------------------------------------------------------------------**//**
* @description Get the cone's inverse coordinate, i.e. transform from the global space to
* a the system where the major circle is a unit xy circle.
* @param pToroid      => torus to evaluate
* @param pInverseFrame  <= inverse frame.
* @return true if the inverse is was computed.
* @group "DToroid3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDToroid3d_getInverseTransform

(
DToroid3dCP pToroid,
TransformP pInverseFrame
)
    {
    return pInverseFrame->InverseOf (pToroid->frame);
    }


/*---------------------------------------------------------------------------------**//**
* Convert a world cartesian point to the local cartesian system.
* @param pWorld <= world coordinates
* @param pLocal => coordinates in local frame of toroid.
* @return true if the local to world transformation was invertible.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDToroid3d_localToWorld

(
DToroid3dCP pInstance,
DPoint3dP pWorld,
DPoint3dCP pLocal
)
    {
    pInstance->frame.Multiply (pWorld, pLocal, 1);
    }


/*---------------------------------------------------------------------------------**//**
* Convert a local cartesian point to the world coordinate system.
* @param pLocal <= coordinates in local frame
* @param pWorld => world coordinates
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDToroid3d_worldToLocal

(
DToroid3dCP pInstance,
DPoint3dP pLocal,
DPoint3dCP pWorld
)
    {
    return  bsiTransform_solveDPoint3d (&pInstance->frame, pLocal, pWorld);
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate the implicit function for the ellipsoid.
* @param pPoint => point where the implicit function is evaluated.
* @indexVerb implicit
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDToroid3d_implicitFunctionValue

(
DToroid3dCP pToroid,
DPoint3dCP pPoint
)
    {

    DPoint3d localPoint;
    double rho2, r2;
    double q, c, f = 0.0;

    if (bsiDToroid3d_worldToLocal (pToroid, &localPoint, pPoint))
        {
        rho2 = localPoint.x * localPoint.x + localPoint.y * localPoint.y;
        r2   = rho2 + localPoint.z * localPoint.z;
        c    = 1.0 - pToroid->minorAxisRatio * pToroid->minorAxisRatio;
        q    = r2 + c;
        f    = q * q - 4.0 * rho2;
        }

    return f;
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate the implicit function for the ellipsoid.
* @param pPoint => point where the implicit function is evaluated, already in local coordinates
* @indexVerb implicit
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDToroid3d_implicitFunctionValueLocal
(
DToroid3dCP pToroid,
DPoint3dCP pLocalPoint
)
    {
    DPoint3d localPoint = *pLocalPoint;
    double rho2, r2;
    double q, c, f;

    rho2 = localPoint.x * localPoint.x + localPoint.y * localPoint.y;
    r2   = rho2 + localPoint.z * localPoint.z;
    c    = 1.0 - pToroid->minorAxisRatio * pToroid->minorAxisRatio;
    q    = r2 + c;
    f    = q * q - 4.0 * rho2;
    return f;
    }



/*---------------------------------------------------------------------------------**//**
* @param pPoint <= evaluated point
* @param theta  => longitude
* @param phi    => latitude
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDToroid3d_naturalParameterToDPoint3d

(
DToroid3dCP pToroid,
DPoint3dP pPoint,
double  theta,
double  phi
)
    {
    double cosTheta = cos (theta);
    double sinTheta = sin (theta);

    double cosPhi   = cos (phi);
    double sinPhi   = sin (phi);

    bsiDToroid3d_trigParameterToPoint
                (
                pToroid,
                pPoint,
                cosTheta,
                sinTheta,
                cosPhi,
                sinPhi
                );
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the spherical coordinates of a given xyz point.  When the point is exactly on the
* surface, this function is the inverse of naturalParameterToPoint.   For a point off of the
* surface, this inversion returns the parameter where the ellipsoid intersects a line
* to the center.  This is a true projection if the surface is a perfect sphere.
* @param pParam1 <= natural parameter
* @param pParam2 <= natural parameter
* @param pPoint  => xyz coordinates
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiDToroid3d_dPoint3dToNaturalParameter

(
DToroid3dCP pToroid,
double  *pParam1,
double  *pParam2,
DPoint3dCP pPoint
)
    {
    DPoint3d localPoint;
    bool    boolstat = bsiDToroid3d_worldToLocal (pToroid, &localPoint, pPoint);
    if (boolstat)
        {
        boolstat = bsiDToroid3d_localToToroidal (pToroid, pParam1, pParam2, NULL, &localPoint);
        }
    return boolstat;
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
* Get the parameter range of the ellipsoid.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDToroid3d_getNaturalParameterRange

(
DToroid3dCP pInstance,
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


/*---------------------------------------------------------------------------------**//**
* Return the range of the natural parameters for a complete surface.
* @param pParam1Start => start value of natural parameter.
* @param pParam1End   => end value of natural parameter.
* @param pParam2Start => start value of natural parameter.
* @param pParam2End   => end value of natural parameter.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDToroid3d_getCompleteNaturalParameterRange

(
DToroid3dCP pToroid,
double  *pParam1Start,
double  *pParam1End,
double  *pParam2Start,
double  *pParam2End
)
    {
    *pParam1Start = -msGeomConst_pi;
    *pParam1End   =  msGeomConst_pi;
    *pParam1Start = -msGeomConst_pi;
    *pParam1End   =  msGeomConst_pi;
    }

/*---------------------------------------------------------------------------------**//**
Copy point data to optional outputs.
* @bsihdr                                                       EarlinLutz      01/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     acceptPoint
(
DPoint3dP pXYZ,
DPoint3dP pUVW,
double *  pA,
int     &n,
int      max,
DPoint3dCR xyz,
DPoint3dCR uvw,
double     a
)
    {
    if (n < max)
        {
        if (NULL != pXYZ)
            pXYZ[n] = xyz;
        if (NULL != pUVW)
            pUVW[n] = uvw;
        if (NULL != pA)
            pA[n] = a;
        n++;
        return true;
        }
    return false;
    }

// local class to carry a mapping from external ray to working ray whose direction vector is
//   scaled to torus size.
struct MappedRay
{
DToroid3dCR rTorus;
DRay3dCR   rRay;
DRay3d     ray;
double     parentParameter0;
double     parentParameterDelta;

MappedRay (DToroid3dCR torus, DRay3dCR parentRay)
    : rTorus(torus),
        rRay(parentRay)
    {
    ray = parentRay;
    double targetSize = torus.frame.MatrixColumnMagnitude (0);
    double refSize    = ray.direction.Magnitude ();
    parentParameter0 = 0.0;
    bsiTrig_safeDivide (&parentParameterDelta, targetSize, refSize, 1.0);
    ray.direction.Scale (parentParameterDelta);
    }
// take parameters from the mapped ray space back to the original.
void DescaleParameters
(
double *pParam,
int     n
)
    {
    for (int i = 0; i < n; i++)
        {
        pParam[i] = parentParameter0 + parentParameterDelta * pParam[i];
        }
    }

};
/*---------------------------------------------------------------------------------**//**
* @description Intersect an (unbounded) ray with the (bounded) torus, considering the minor circles as
*   complete but considering angle range on the major circle.
* The result may include 4 points from the torus and 2 from caps.
* In perfect math, there can really only be 4 points.  However, if the line passes through
* a circle point on the end cap, there are coincident points from the end cap and torus intersections.
* This function does not try to detect this -- it just loads up the output arrays with up to 6 points.
* @param pCone  => torus to evaluate
* @param pXYZ   <= array of 0 to 6 (SIX) coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0 to 6 (SIX) coordinates of intersection points in the
*                       local coordinates of the cone frame.   May be NULL.
* @param pLineParameter <= array of 0 to 4 (SIX) parameters with respect to the line.
* @param pRay => (unbounded) ray to intersect with torus and caps
* @param includeEndCaps => Whether to include the elliptical caps
* @return number of intersections.
* @group "DTorus3d Intersection"
* @bsihdr                                                       EarlinLutz      01/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDTorus3d_intersectDRay3dCaps
(
DToroid3dCP pTorus,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double    *pLineParameter,
DRay3dCP pRay,
bool              includeEndCaps
)
    {
    DPoint4d poles[2];
    DPoint3d point0, point1;
    MappedRay scaledRay (*pTorus, *pRay);

    bsiDRay3d_evaluateEndPoints (&scaledRay.ray, &point0, &point1);
    poles[0].Init (point0, 1.0);
    poles[1].Init (point1, 1.0);
    int n0, n1;
    bsiDToroid3d_intersectBezierCurve (pTorus, pLineParameter,
                    pXYZ, pUVW, &n0, &n1, 6,
                    poles, 2, 2,
                    includeEndCaps ? 1 : 0,
                    true);
    scaledRay.DescaleParameters (pLineParameter, n0 + n1);
    return n0 + n1;
    }


/*---------------------------------------------------------------------------------**//**
* @description Test whether the given local point lies within selective parametric extents of the cone.
* @return true if the local point is "in" the cone.
* @param pCone => torus to evaluate
* @param pUVW => local coordinates to test.
* @param applyLongitudeLimits => true to test longitude
* @param applyLatitudeLimits => true to test latitude
* @param applyRadiusLimits => true to test for radius
* @group "DToroid3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDToroid3d_isLocalDPoint3dInSelectiveBounds
(
DToroid3dCP pTorus,
DPoint3dCP pUVW,
bool            applyLongitudeLimits,
bool            applyLatitudeLimits,
bool            applyRadiusLimits
)
    {
    double longitude, latitude, minorDistance;
    bsiDToroid3d_localToToroidal (pTorus, &longitude, &latitude, &minorDistance, pUVW);

    if (applyLongitudeLimits
            && !bsiDToroid3d_longitudeInRange (pTorus, longitude))
        return false;

    if (applyLatitudeLimits
            && !bsiDToroid3d_latitudeInRange (pTorus, latitude))
        return false;

    if (applyRadiusLimits && minorDistance <= 1.0)
        return false;

    return true;
    }



/*---------------------------------------------------------------------------------**//**
@description Return intersections of a bezier curve with the toroid.
@param pToroid        => toroid to evaluate
@param pIntersectionParam OUT intersection parameters on the bezier
@param pIntersectionXYZ OUT world coordinate intersection points
@param pIntersectionUVW OUT local frame intersection points
@param pNumToroidIntersection OUT number of intersections on toroid
@param pNumCapIntersection OUT number of intersections on cap.
@param maxIntersection IN size limit for all output arrays.
@param pBezierPoles IN bezier curve
@param order IN bezier order
@param toroidSelect IN
    <list>
    <item>0 for no toroid intersections.</item>
    <item>1 for toroid intersections with unbounded toroid.</item>
    <item>2 for toroid intersections with bounded patch</item>
    </list>
@param capSelect IN
    <list>
    <item>0 for no cap intersections.</item>
    <item>1 for intersections with cap disks at bounded patch limits.</item>
    </list>
@group "DToroid3d Parameter Range"
@bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDToroid3d_intersectBezierCurve
(
DToroid3dCP pToroid,
double *pIntersectionParam,
DPoint3d *pIntersectionXYZ,
DPoint3d *pIntersectionUVW,
int *pNumToroidIntersection,
int *pNumCapIntersection,
int maxIntersection,
DPoint4dCP pWorldPoles,
int        order,
int        toroidSelect,
int        capSelect,
bool       extendCurve
)
    {
    if (pNumToroidIntersection)
        *pNumToroidIntersection = 0;
    if (pNumCapIntersection)
        *pNumCapIntersection = 0;
    int num0 = 0;
    int num1 = 0;
    int num  = 0;
    Transform worldToLocal;
    if (!bsiDToroid3d_getInverseTransform (pToroid, &worldToLocal))
        return false;
    if (2 * order > MAX_BEZIER_ORDER)
        return false;

    DPoint4d localPoles[MAX_BEZIER_ORDER];
    double rootPoles[MAX_BEZIER_ORDER];
    double rho2Poles[MAX_BEZIER_ORDER];
    double w2Poles[MAX_BEZIER_ORDER];
    double qPoles[MAX_BEZIER_ORDER];
    DPoint3d rootUVW[MAX_BEZIER_ORDER];
    DPoint3d rootXYZ[MAX_BEZIER_ORDER];
    double roots[MAX_BEZIER_ORDER];
    bool    rootStat = false;
    bsiTransform_multiplyDPoint4dArray (&worldToLocal, localPoles, pWorldPoles, order);
    //double f0 = bsiDToroid3d_implicitFunctionValue (pToroid, (DPoint3d*)&pWorldPoles[0]);
    //double f1 = bsiDToroid3d_implicitFunctionValueLocal (pToroid, (DPoint3d*)&localPoles[0]);
    if (toroidSelect != 0)
        {
        // implicit equation of toroid izzz....
        //   c =
        //   (x^2 + y^2 + z^2 + C)^2 = 4 (x^2 + y^2)
        // with homogeneous coordinates in the bezier
        //   (x^2 + y^2+ z^2 + C w^2)^2 - 4 (x^2 + y^2)w^2 = 0
        // x^2
        double c    = 1.0 - pToroid->minorAxisRatio * pToroid->minorAxisRatio;
        int order2 = 2 * order - 1; // order (degree + 1!!) for squared parts
        int order4 = 4 * order - 3; // order (degree + 2!!) for quartic parts
        // w2
        bsiBezier_univariateProduct
                    (
                    w2Poles, 0, 1,
                    (double*)localPoles, order, 3, 4,
                    (double*)localPoles, order, 3, 4
                    );

        // rho^2 <== x^2 + y^2
        bsiBezier_univariateProduct
                    (rho2Poles, 0, 1,
                    (double*)localPoles, order, 0, 4,
                    (double*)localPoles, order, 0, 4);
        bsiBezier_accumulateUnivariateProduct
                    (rho2Poles, 0, 1, 1.0,
                    (double*)localPoles, order, 1, 4,
                    (double*)localPoles, order, 1, 4);

        // q == (x^2 + y^2 + z^2 + c w^2 )
        bsiBezier_univariateProduct
                    (qPoles, 0, 1,
                    (double*)localPoles, order, 0, 4,
                    (double*)localPoles, order, 0, 4);
        bsiBezier_accumulateUnivariateProduct
                    (qPoles, 0, 1, 1.0,
                    (double*)localPoles, order, 1, 4,
                    (double*)localPoles, order, 1, 4);
        bsiBezier_accumulateUnivariateProduct
                    (qPoles, 0, 1, 1.0,
                    (double*)localPoles, order, 2, 4,
                    (double*)localPoles, order, 2, 4);
        bsiBezier_accumulateUnivariateProduct
                    (qPoles, 0, 1, c,
                    (double*)localPoles, order, 3, 4,
                    (double*)localPoles, order, 3, 4);

        // q * w^2
        bsiBezier_univariateProduct (rootPoles, 0, 1,
                    qPoles, order2, 0, 1,
                    qPoles, order2, 0, 1);

        bsiBezier_accumulateUnivariateProduct (rootPoles, 0, 1, -4.0,
                    rho2Poles, order2, 0, 1,
                    w2Poles, order2, 0, 1);

        int numToroidRoot;

        if (extendCurve)
            rootStat = bsiBezier_univariateRootsExt (roots, &numToroidRoot, rootPoles, order4);
        else
            rootStat = bsiBezier_univariateRoots (roots, &numToroidRoot, rootPoles, order4);

        if (rootStat
            && numToroidRoot > 0
            && numToroidRoot < order4)
            {
            bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                (rootUVW, NULL, NULL, localPoles, order, roots, numToroidRoot);
            if (NULL != pIntersectionXYZ)
                bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                    (rootXYZ, NULL, NULL, pWorldPoles, order, roots, numToroidRoot);

            for (int i = 0; i < numToroidRoot; i++)
                {
                DPoint3d uvw = rootUVW[i];
                bool bAccept = true;

                if (toroidSelect == 2)
                    bAccept = bsiDToroid3d_isLocalDPoint3dInSelectiveBounds (pToroid, &uvw,
                                        true, true, false) ? true : false;

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

    if (capSelect != 0 && !bsiDToroid3d_isParam1Complete (pToroid))
        {
        DEllipse3d capEllipse[2];

        bsiDToroid3d_getMeridianLocal (pToroid, &capEllipse[0], pToroid->parameterRange.low.x);
        bsiDToroid3d_getMeridianLocal (pToroid, &capEllipse[1], pToroid->parameterRange.high.x);

        // Intersect with each of the two planes ...
        for (int k = 0; k < 2; k++)
            {
            // univariate bezier for this ellipse is (X - C).N = 0
            DVec3d normal;
            normal.NormalizedCrossProduct (*(&capEllipse[k].vector0), *(&capEllipse[k].vector90));
            DPoint3d center = capEllipse[k].center;
            for (int i = 0; i < order; i++)
                rootPoles[i] =
                                (localPoles[i].x - localPoles[i].w * center.x) * normal.x
                             +  (localPoles[i].y - localPoles[i].w * center.y) * normal.y
                             +  (localPoles[i].z - localPoles[i].w * center.z) * normal.z
                    ;

            int numRoot;
            if (extendCurve)
                rootStat = bsiBezier_univariateRootsExt (roots, &numRoot, rootPoles, order);
            else
                rootStat = bsiBezier_univariateRoots (roots, &numRoot, rootPoles, order);

            if (   rootStat
                && numRoot > 0
                && numRoot < order)
                {
                bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                    (rootUVW, NULL, NULL, localPoles, order, roots, numRoot);
                if (NULL != pIntersectionXYZ)
                    bsiBezierDPoint4d_evaluateDPoint3dArrayExt
                        (rootXYZ, NULL, NULL, pWorldPoles, order, roots, numRoot);

                for (int i = 0; i < numRoot; i++)
                    {
                    DPoint3d uvw = rootUVW[i];
                    bool bAccept = true;
                    double f = bsiDToroid3d_implicitFunctionValueLocal (pToroid, &rootUVW[i]);

                    bAccept = f <= 0.0;

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
    *pNumToroidIntersection = num0;
    *pNumCapIntersection  = num1;
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dellipsoid3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static const DRange2d s_defaultParameterRange =
                {
                    {-msGeomConst_pi, -msGeomConst_piOver2},
                    { msGeomConst_pi,  msGeomConst_piOver2},
                };


/*---------------------------------------------------------------------------------**//**
* Initialize an ellipsoid from center and radius and optional parameter range.
* Orientation is set parallel to global axes.
* @param pCenter => ellipsoid center.
* @param radius  => radius of ellipsoid.
* @param pParameterRange => parameter range. If NULL, default is applied.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_setCenterRadius

(
DEllipsoid3dP pInstance,
DPoint3dCP pCenter,
double          radius,
DRange2dCP pParameterRange
)
    {
    pInstance->frame.InitFromScaleFactors (radius, radius, radius);
    if (nullptr != pCenter)
        pInstance->frame.SetTranslation (*pCenter);
    bsiDEllipsoid3d_setNaturalParameterRange (pInstance, pParameterRange);
    }


/*---------------------------------------------------------------------------------**//**
* Initialize an ellipsoid from full frame and range
* @param pTransform => coordinate frame.  If NULL, default is applied.
* @param pRange => parameter range.  If NULL, default is applied.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_set

(
DEllipsoid3dP pInstance,
TransformCP pFrame,
DRange2dCP pParameterRange
)
    {
    bsiDEllipsoid3d_setFrame (pInstance, pFrame);
    bsiDEllipsoid3d_setNaturalParameterRange (pInstance, pParameterRange);
    }


/*---------------------------------------------------------------------------------**//**
* Set the reference frame of the ellipsoid.
* @param pTransform => coordinate frame.  If NULL, default is applied.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_setFrame

(
DEllipsoid3dP pInstance,
TransformCP pFrame
)
    {
    if (pFrame)
        pInstance->frame = *pFrame;
    else
        pInstance->frame.InitIdentity ();
    }


/*---------------------------------------------------------------------------------**//**
* Set the parameter range of the ellipsoid.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_setNaturalParameterRange

(
DEllipsoid3dP pInstance,
DRange2dCP pParameterRange
)
    {
    pInstance->parameterRange = pParameterRange ? *pParameterRange : s_defaultParameterRange;
    }



/*---------------------------------------------------------------------------------**//**
* Test if the ellipsoid range is the full parameter space.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_isFull

(
DEllipsoid3dCP pInstance
)
    {
    double dTheta = pInstance->parameterRange.high.x - pInstance->parameterRange.low.x;

    double phi0   = pInstance->parameterRange.high.y;
    double phi1   = pInstance->parameterRange.low.y;

    return bsiTrig_isAngleFullCircle (dTheta)
        && (   (phi1 >= msGeomConst_piOver2 && phi0 <= -msGeomConst_piOver2)
            || (phi1 <= msGeomConst_piOver2 && phi0 >= -msGeomConst_piOver2)
           );
    }


/*---------------------------------------------------------------------------------**//**
* Get the silhouette of the ellipse from a given homogeneous eyepoint.  This is a single,
* complete conic.  To get the (up to 4) fragments of the silhouette within the parameter range,
* use GraphicsPointArray method addSilhouette().
*
* @param pEllipse   <= silhouette ellipse.
* @param pMap       => additional transform to apply to the ellipsoid.
* @param pEyePoint  => eye point.
* @indexVerb silhouette
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_getSilhouette

(
DEllipsoid3dCP pInstance,
DConic4dP pEllipse,
DMap4dCP pMap,
DPoint4dCP pEyePoint
)
    {
    bool    boolstat = false;
    DMap4d map;
    DEllipse4d  dEllipse4d;
    if (bsiDMap4d_initFromTransform(&map, &pInstance->frame, false))
        {
        if (pMap)
            bsiDMap4d_multiply (&map, pMap, &map);
        boolstat = bsiGeom_ellipsoidSilhouette (&dEllipse4d, NULL, &map, pEyePoint);
        bsiDConic4d_initFromDEllipse4dSweep (pEllipse, &dEllipse4d, 0.0, msGeomConst_2pi);
        }
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Intersect the sphere with a line segment.
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the ellipsoid frame.   (Points on the
*                       unit sphere).  May be NULL.
* @param pLineParameter <= array of 0, 1, or 2 parameters with respect to the line.
* @param DRay3d         => ray to intersect.
* @return number of intersections.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int                bsiDEllipsoid3d_intersectDSegment3d

(
DEllipsoid3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DSegment3dCP pSegment
)
    {
    DRay3d ray;
    bsiDRay3d_initFromDSegment3d (&ray, pSegment);
    return bsiDEllipsoid3d_intersectDRay3d (pInstance, pXYZ, pUVW, pLineParameter, &ray);
    }


/*---------------------------------------------------------------------------------**//**
* Intersect the sphere with a line segment.
* @param pXYZ   <= array of 0, 1, or 2 coordinates of cartesian intersection points.
*                       May be NULL.
* @param pUVW   <= array of 0, 1, or 2 coordinates of intersection points in the
*                       local coordinates of the ellipsoid frame.   (Points on the
*                       unit sphere).  May be NULL.
* @param pLineParameter <= array of 0, 1, or 2 parameters with respect to the line.
* @return number of intersections.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int                bsiDEllipsoid3d_intersectDRay3d

(
DEllipsoid3dCP pInstance,
DPoint3dP pXYZ,
DPoint3dP pUVW,
double          *pLineParameter,
DRay3dCP pRay
)
    {
    //bool    boolstat = false;
    Transform inverseFrame;
    DRay3d  localRay;
    double aa, bb, cc;
    double param[2];
    int numRoot = 0;
    int i;
    if (bsiDEllipsoid3d_getInverseFrame (pInstance, &inverseFrame))
        {
        inverseFrame.Multiply (localRay, *pRay);
        cc = localRay.origin.DotProduct (localRay.origin) - 1.0;
        bb = 2.0 * localRay.origin.DotProduct (localRay.direction);
        aa = localRay.direction.DotProduct (localRay.direction);
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





/*-----------------------------------------------------------------*//**
*
* Apply a transformation to the source ellipseoid.  Only affine parts of the homogeneous
* matrix are applied.
* @param pTransform => transformation to apply.
* @param pSource => source ellipse.
* @indexVerb transform
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipsoid_applyAffineDMatrix4d

(
DEllipsoid3dP pDest,
DMatrix4dCP pMatrix,
DEllipsoid3dCP pSource
)
    {
    pMatrix->MultiplyAffine (pDest->frame, pDest->frame);
    pDest->parameterRange = pSource->parameterRange;
    }


/*---------------------------------------------------------------------------------**//**
* Convert a local cartesian point into the local spherical coordinates.
* @param pTheta     <= angle in unit spherical coordiantes
* @param pPhi       <= z coordinate in spherical coordinates
* @param pR         <= radius in spherical coordinates
* @return false if the local point is at the origin.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDEllipsoid3d_localToSpherical

(
DEllipsoid3dCP pEllipsoid,
double    *pTheta,
double    *pPhi,
double    *pR,
DPoint3dCP pPoint
)
    {
    double rho2 = pPoint->x * pPoint->x + pPoint->y * pPoint->y;
    double rho;

    if (pTheta)
        *pTheta = bsiTrig_atan2 (pPoint->y, pPoint->x);

    if (pPhi)
        {
        rho = sqrt (rho2);
        *pPhi = bsiTrig_atan2 (pPoint->z, rho);
        }

    if (pR)
        *pR = sqrt (rho2 + pPoint->z * pPoint->z);

    return rho2 != 0.0 || pPoint->z != 0.0;
    }


/*---------------------------------------------------------------------------------**//**
* Convert a world cartesian point to the local cartesian system.
* @param pWorld <= world coordinates
* @param pLocal => coordinates in local frame of ellipsoid.
* @return true if the local to world transformation was invertible.
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDEllipsoid3d_localToWorld

(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pWorld,
DPoint3dCP pLocal
)
    {
    pEllipsoid->frame.Multiply (pWorld, pLocal, 1);
    }


/*---------------------------------------------------------------------------------**//**
* Convert a local cartesian point to the world coordinate system.
* @param pLocal <= coordinates in local frame
* @param pWorld => world coordinates
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDEllipsoid3d_worldToLocal

(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pLocal,
DPoint3dCP pWorld
)
    {
    return  bsiTransform_solveDPoint3d (&pEllipsoid->frame, pLocal, pWorld);
    }



/*---------------------------------------------------------------------------------**//**
* Convert from parametric to cartesian, with the parametric coordinate given as trig values.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDEllipsoid3d_trigParameterToPoint

(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pPoint,
double      cosTheta,
double      sinTheta,
double      cosPhi,
double      sinPhi
)
    {
    pEllipsoid->frame.Multiply (*pPoint,
                cosTheta * cosPhi,
                sinTheta * cosPhi,
                sinPhi
                );
    }


/*---------------------------------------------------------------------------------**//**
* test if a longitude angle is in the ellipsoid's parameter range.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDEllipsoid3d_longitudeInRange

(
DEllipsoid3dCP pInstance,
double          longitude
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
Public GEOMDLLIMPEXP bool        bsiDEllipsoid3d_latitudeInRange

(
DEllipsoid3dCP pInstance,
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
* Return a meridian at specified longitude.  The angular sweep of the meridian is restricted
*   to the parameter range of the surface.
* @param pEllipse <= full ellipse at specified longitude.   0-degree vector is on
*           is on the equator.  90 degree vector is to the north pole.
* @param longitude => longitude angle (radians)
* @return true if the longitude is within the parameter range of the surface patch.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_getMeridian

(
DEllipsoid3dCP pEllipsoid,
DEllipse3dP pEllipse,
double      longitude
)
    {
    DPoint3d            center;
    DVec3d          vector0;
    DVec3d          vector90;

    double              cosTheta = cos (longitude);
    double              sinTheta = sin (longitude);
    double              theta0, dTheta;
    double              phi0,   dPhi;

    bsiDEllipsoid3d_getScalarNaturalParameterSweep
                    (
                    pEllipsoid,
                    &theta0, &dTheta,
                    &phi0, &dPhi
                    );

    pEllipsoid->frame.MultiplyMatrixOnly (vector0, cosTheta, sinTheta,  0.0);
    pEllipsoid->frame.MultiplyMatrixOnly (vector90, 0.0, 0.0, 1.0);
    pEllipsoid->frame.GetTranslation (center);
    bsiDEllipse3d_initFrom3dVectors (pEllipse, &center, &vector0, &vector90, phi0, dPhi);

    return bsiTrig_angleInSweep (longitude, theta0, dTheta);
    }


/*---------------------------------------------------------------------------------**//**
* Return a (full) parallel of latitude.
* @param pEllipse <= full ellipse at specified latitude.  0 and 90 degree vectors
*           are 0 and 90 degrees latitude.
* @param latitude => latitude angle (radians)
* @return true if the latitude is within the parameter range of the surface.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_getParallel

(
DEllipsoid3dCP pEllipsoid,
DEllipse3dP pEllipse,
double      latitude
)
    {
    double              cosPhi = cos (latitude);
    double              sinPhi = sin (latitude);
    DVec3d              vector0, vector90;
    DPoint3d    center;
    double              theta0, dTheta;
    double              phi0,   dPhi;

    bsiDEllipsoid3d_getScalarNaturalParameterSweep
                    (
                    pEllipsoid,
                    &theta0, &dTheta,
                    &phi0, &dPhi
                    );

    pEllipsoid->frame.MultiplyMatrixOnly (vector0, cosPhi, 0,0);
    pEllipsoid->frame.MultiplyMatrixOnly (vector90, 0, cosPhi, 0);
    pEllipsoid->frame.Multiply (center, 0, 0, sinPhi);

    bsiDEllipse3d_initFrom3dVectors (pEllipse, &center, &vector0, &vector90, theta0, dTheta);
    return bsiTrig_angleInSweep (latitude, phi0, dPhi);
    }

/*---------------------------------------------------------------------------------**//**
* @param pPoint <= evaluated point
* @param thetaFraction => angular position, as a fraction of the patch longitude range.
* @param phiFraction => axial position, as a fraction of the patch latitude range.
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_fractionParameterToDPoint3d

(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pPoint,
double    thetaFraction,
double    phiFraction
)
    {
    double theta, phi;
    bsiDEllipsoid3d_fractionParameterToNaturalParameter (pEllipsoid, &theta, &phi, thetaFraction, phiFraction);
    return bsiDEllipsoid3d_naturalParameterToDPoint3d (pEllipsoid, pPoint, theta, phi);
    }


/*---------------------------------------------------------------------------------**//**
* @param thetaFraction  <= longitude, as a fraction of the patch longitude range
* @param phiFraction    <= latitude, as a fraction of the latitude range
* @param pPoint         => evaluated point
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipsoid3d_dPoint3dToFractionParameter

(
DEllipsoid3dCP pEllipsoid,
double    *pThetaFraction,
double  *pPhiFraction,
DPoint3dCP pPoint
)
    {
    double theta, phi;
    bool    stat = bsiDEllipsoid3d_dPoint3dToNaturalParameter (pEllipsoid, &theta, &phi, pPoint);
    bsiDEllipsoid3d_naturalParameterToFractionParameter (pEllipsoid, pThetaFraction, pPhiFraction, theta, phi);
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_fractionParameterToNaturalParameter

(
DEllipsoid3dCP pEllipsoid,
double    *pTheta,
double    *pPhi,
double    thetaFraction,
double    phiFraction
)
    {
    if (pTheta)
        *pTheta = pEllipsoid->parameterRange.low.x +
                            thetaFraction * (pEllipsoid->parameterRange.high.x - pEllipsoid->parameterRange.low.x);

    if (pPhi)
        *pPhi = pEllipsoid->parameterRange.low.y +
                            phiFraction * (pEllipsoid->parameterRange.high.y - pEllipsoid->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* @param pTheta => longitude
* @param pPhi => latitude
* @param theta  => longitude
* @param phi    => latitude
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_naturalParameterToFractionParameter

(
DEllipsoid3dCP pEllipsoid,
double    *pThetaFraction,
double    *pPhiFraction,
double    theta,
double    phi
)
    {
    if (pThetaFraction)
        *pThetaFraction = bsiTrig_normalizeAngleToSweep (theta, pEllipsoid->parameterRange.low.x,
                            pEllipsoid->parameterRange.high.x - pEllipsoid->parameterRange.low.x);

    if (pPhiFraction)
        *pPhiFraction = bsiTrig_normalizeAngleToSweep (phi, pEllipsoid->parameterRange.low.y,
                            pEllipsoid->parameterRange.high.y - pEllipsoid->parameterRange.low.y);
    }


/*---------------------------------------------------------------------------------**//**
* Test if the ellipsoid range is the full parameter space.
*
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_isComplete

(
DEllipsoid3dCP pInstance
)
    {
    return bsiDEllipsoid3d_isParam1Complete (pInstance)
            && bsiDEllipsoid3d_isParam2Complete (pInstance);
    }


/*---------------------------------------------------------------------------------**//**
* @return true if the 1st natural parameter covers the complete range of the underlying
*           analytic surface.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_isParam1Complete

(
DEllipsoid3dCP pEllipsoid
)
    {
    return bsiTrig_isAngleFullCircle (pEllipsoid->parameterRange.high.x - pEllipsoid->parameterRange.low.x);
    }


/*---------------------------------------------------------------------------------**//**
* @return true if the 2nd natural parameter covers the complete range of the underlying
*           analytic surface.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_isParam2Complete

(
DEllipsoid3dCP pEllipsoid
)
    {
    return bsiTrig_isAngleFullCircle (2.0 * (pEllipsoid->parameterRange.high.y - pEllipsoid->parameterRange.low.y));
    }



/*---------------------------------------------------------------------------------**//**
* Get the reference frame of the ellipsoid.
*
* @indexVerb localCoordinates
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_getFrame

(
DEllipsoid3dCP pInstance,
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
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_getInverseFrame

(
DEllipsoid3dCP pInstance,
TransformP pInverseFrame
)
    {
    return pInverseFrame->InverseOf (pInstance->frame);
    }


/*---------------------------------------------------------------------------------**//**
* Evaluate the implicit function for the ellipsoid.
* @param pPoint => point where the implicit function is evaluated.
* @indexVerb implicit
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipsoid3d_implicitFunctionValue

(
DEllipsoid3dCP pEllipsoid,
DPoint3dCP pPoint
)
    {
    DPoint3d localPoint;
    double f = 0.0;

    if (bsiDEllipsoid3d_worldToLocal (pEllipsoid, &localPoint, pPoint))
        {
        f = localPoint.x * localPoint.x + localPoint.y * localPoint.y + localPoint.z * localPoint.z - 1.0;
        }

    return f;
    }


/*---------------------------------------------------------------------------------**//**
* @param pPoint <= evaluated point
* @param theta  => longitude
* @param phi    => latitude
* @indexVerb parameterization
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDEllipsoid3d_naturalParameterToDPoint3d

(
DEllipsoid3dCP pEllipsoid,
DPoint3dP pPoint,
double  theta,
double  phi
)
    {
    double cosTheta = cos (theta);
    double sinTheta = sin (theta);

    double cosPhi   = cos (phi);
    double sinPhi   = sin (phi);

    pEllipsoid->frame.Multiply (*pPoint,
                cosTheta * cosPhi,
                sinTheta * cosPhi,
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
Public GEOMDLLIMPEXP bool         bsiDEllipsoid3d_dPoint3dToNaturalParameter

(
DEllipsoid3dCP pEllipsoid,
double  *pParam1,
double  *pParam2,
DPoint3dCP pPoint
)
    {
    DPoint3d localPoint;
    bool    boolstat = bsiDEllipsoid3d_worldToLocal (pEllipsoid, &localPoint, pPoint);
    if (boolstat)
        {
        boolstat = bsiDEllipsoid3d_localToSpherical (pEllipsoid, pParam1, pParam2, NULL, &localPoint);
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_getScalarNaturalParameterRange

(
DEllipsoid3dCP pEllipsoid,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
)
    {
    *pParam1Start = pEllipsoid->parameterRange.low.x;
    *pParam1End   = pEllipsoid->parameterRange.high.x;
    *pParam2Start = pEllipsoid->parameterRange.low.y;
    *pParam2End   = pEllipsoid->parameterRange.high.y;
    }


/*---------------------------------------------------------------------------------**//**
* Get the parameter range of the ellipsoid.
*
* @indexVerb paramaterRange
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void               bsiDEllipsoid3d_getNaturalParameterRange

(
DEllipsoid3dCP pInstance,
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
Public GEOMDLLIMPEXP void    bsiDEllipsoid3d_getScalarNaturalParameterSweep

(
DEllipsoid3dCP pInstance,
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
Public GEOMDLLIMPEXP void     bsiDEllipsoid3d_getCompleteNaturalParameterRange

(
DEllipsoid3dCP pEllipsoid,
double  *pParam1Start,
double  *pParam1End,
double  *pParam2Start,
double  *pParam2End
)
    {
    *pParam1Start = -msGeomConst_pi;
    *pParam1End   =  msGeomConst_pi;
    *pParam1Start = -msGeomConst_piOver2;
    *pParam1End   =  msGeomConst_piOver2;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

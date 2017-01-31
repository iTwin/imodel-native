/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_silhouette.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
extern void mdlUtil_sortDoubles (double *, int, int);

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Public GEOMDLLIMPEXP Global variables                                             |
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


/*---------------------------------------------------------------------------------**//**
* Add a DEllipse3d to the array, optionally transforming by a DMap4d.
* @param pMatrix    => additional homogeneous matrix. May be null.
* @param pEllipse   => ellipse to add.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addDMap4dDEllipse3d
(
GraphicsPointArray  *pGPA,
const   DMap4d          *pMap,
const   DEllipse3d      *pEllipse
)
    {
    if (pMap)
        {
        jmdlGraphicsPointArray_addDEllipse3d (pGPA, pEllipse);
        }
    else
        {
        DConic4d conic;
        bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);
        /* Apply the map !!!*/
        jmdlGraphicsPointArray_addDConic4d (pGPA, &conic);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add the silhouette of an ellipsoid to the GraphicsPointArray.
* @param pEllipsoid => ellipsoid whose silhouette is calculated.
* @param pMap       => additional homogeneous mapping. May be null.
* @param pEyePoint  => homogeneous eyepoint.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDEllipsoid3dSilhouette
(
GraphicsPointArray  *pGPA,
const   DEllipsoid3d    *pEllipsoid,
const   DMap4d          *pMap,
const   DPoint4d        *pEyePoint
)
    {
    DConic4d conic;
    bsiDEllipsoid3d_getSilhouette (pEllipsoid, &conic, pMap, pEyePoint);
    jmdlGraphicsPointArray_addDConic4d (pGPA, &conic);
    }



/*---------------------------------------------------------------------------------**//**
* Add the silhouette of a cone to the GraphicsPointArray.
* @param pEllipsoid => ellipsoid whose silhouette is calculated.
* @param pMap       => additional homogeneous mapping. May be null.
* @param pEyePoint  => homogeneous eyepoint.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDCone3dSilhouette
(
GraphicsPointArray  *pGPA,
const   DCone3d         *pCone,
const   DMap4d          *pMap,
const   DPoint4d        *pEyePoint
)
    {
    DPoint3d trigPoint[2];
    DPoint3d point[2];
    DPoint4d hPoint[2];
    double   theta;
    double   z0, z1, theta0, theta1, sweep;
    double   cosPhi, sinPhi;
    int      i;

    int numSilhouette = bsiDCone3d_silhouetteAngles (pCone, trigPoint, pMap, pEyePoint);

    bsiDCone3d_getScalarNaturalParameterRange (pCone, &theta0, &theta1, &z0, &z1);
    sweep = theta1 - theta0;

    for (i = 0; i < numSilhouette; i++)
        {
        cosPhi = trigPoint[i].x;
        sinPhi = trigPoint[i].y;
        theta = bsiTrig_atan2 (sinPhi, cosPhi);

        if (bsiTrig_angleInSweep (theta, theta0, sweep))
            {
            bsiDCone3d_trigParameterToPoint (pCone, &point[0], cosPhi, sinPhi, z0);
            bsiDCone3d_trigParameterToPoint (pCone, &point[1], cosPhi, sinPhi, z1);

            if (!pMap)
                {
                jmdlGraphicsPointArray_addDPoint3d (pGPA, &point[0]);
                jmdlGraphicsPointArray_addDPoint3d (pGPA, &point[1]);
                }
            else
                {
                bsiDMatrix4d_multiplyWeightedDPoint3dArray (&pMap->M0, hPoint, point, NULL, 2);
                jmdlGraphicsPointArray_addDPoint4d (pGPA, &hPoint[0]);
                jmdlGraphicsPointArray_addDPoint4d (pGPA, &hPoint[1]);
                }
            jmdlGraphicsPointArray_markBreak (pGPA);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add the edges of a cone to the GraphicsPointArray.
* @param pCone => cone whose edges are added
* @param pMap       => additional homogeneous mapping. May be null.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDCone3dEdges
(
GraphicsPointArray  *pGPA,
const   DCone3d         *pCone,
const   DMap4d          *pMap
)
    {
    DConic4d conic;

    DPoint3d point[2];
    DPoint4d hPoint[2];
    DPoint3d center;
    DVec3d vectorX, vectorY, vectorZ;
    double   z0, z1, theta0, theta1, sweep, theta;
    double   cosPhi, sinPhi;
    bool     isFull;
    int i;

    bsiDCone3d_getScalarNaturalParameterRange (pCone, &theta0, &theta1, &z0, &z1);
    sweep = theta1 - theta0;

    isFull = bsiTrig_isAngleFullCircle (sweep);
    pCone->frame.GetOriginAndVectors (center, vectorX, vectorY, vectorZ);

    bsiDConic4d_initFrom3dVectors (&conic, &center, &vectorX, &vectorY,
                            theta0, sweep
                            );
    if (pMap)
        bsiDConic4d_applyDMatrix4d (&conic, &pMap->M0, &conic);

    jmdlGraphicsPointArray_addDConic4d (pGPA, &conic);
    jmdlGraphicsPointArray_markBreak (pGPA);

    center.SumOf (center, vectorZ);

    bsiDConic4d_initFromScaledVectors (&conic, &center, &vectorX, &vectorY,
                            pCone->radiusFraction,
                            pCone->radiusFraction,
                            theta0, sweep
                            );
    if (pMap)
        bsiDConic4d_applyDMatrix4d (&conic, &pMap->M0, &conic);

    jmdlGraphicsPointArray_addDConic4d (pGPA, &conic);
    jmdlGraphicsPointArray_markBreak (pGPA);

    if (!isFull)
        {
        for (i = 0, theta = theta0; i < 2; i++, theta = theta1)
            {
            cosPhi = cos (theta);
            sinPhi = sin (theta);

            bsiDCone3d_trigParameterToPoint (pCone, &point[0], cosPhi, sinPhi, z0);
            bsiDCone3d_trigParameterToPoint (pCone, &point[1], cosPhi, sinPhi, z1);

            if (!pMap)
                {
                jmdlGraphicsPointArray_addDPoint3d (pGPA, &point[0]);
                jmdlGraphicsPointArray_addDPoint3d (pGPA, &point[1]);
                }
            else
                {
                bsiDMatrix4d_multiplyWeightedDPoint3dArray (&pMap->M0, hPoint, point, NULL, 2);
                jmdlGraphicsPointArray_addDPoint4d (pGPA, &hPoint[0]);
                jmdlGraphicsPointArray_addDPoint4d (pGPA, &hPoint[1]);
                }
            jmdlGraphicsPointArray_markBreak (pGPA);
            }
        }

    }


/*---------------------------------------------------------------------------------**//**
* Add the edges of a cone to the GraphicsPointArray.
* @param pTorus => torus whose edgeds are added
* @param pMap       => additional homogeneous mapping. May be null.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDToroid3dEdges
(
GraphicsPointArray  *pGPA,
const   DToroid3d       *pToroid,
const   DMap4d          *pMap
)
    {
    bool            fullPhi,    fullTheta;
    double          dPhi,       dTheta;
    double          phi0,       phi1;
    double          theta0,     theta1;
    DEllipse3d      ellipse;

    bsiDToroid3d_getScalarNaturalParameterRange (pToroid, &theta0, &theta1, &phi0, &phi1);

    dTheta = theta1 - theta0;
    dPhi   = phi1   - phi0;

    fullPhi   = bsiTrig_isAngleFullCircle (dPhi);
    fullTheta = bsiTrig_isAngleFullCircle (dTheta);

    if (!fullTheta)
        {
        bsiDToroid3d_getMeridian (pToroid, &ellipse, theta0);
        bsiDEllipse3d_setLimits (&ellipse, phi0, phi1);
        jmdlGraphicsPointArray_addDMap4dDEllipse3d (pGPA, pMap, &ellipse);

        bsiDToroid3d_getMeridian (pToroid, &ellipse, theta1);
        bsiDEllipse3d_setLimits (&ellipse, phi0, phi1);
        jmdlGraphicsPointArray_addDMap4dDEllipse3d (pGPA, pMap, &ellipse);
        }

    if (!fullPhi)
        {
        bsiDToroid3d_getParallel (pToroid, &ellipse, phi0);
        bsiDEllipse3d_setLimits (&ellipse, theta0, theta1);
        jmdlGraphicsPointArray_addDMap4dDEllipse3d (pGPA, pMap, &ellipse);

        bsiDToroid3d_getParallel (pToroid, &ellipse, phi1);
        bsiDEllipse3d_setLimits (&ellipse, theta0, theta1);
        jmdlGraphicsPointArray_addDMap4dDEllipse3d (pGPA, pMap, &ellipse);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add rulings of a torus to the GraphicsPointArray.
* @param pEllipsoid => ellipsoid whose silhouette is calculated.
* @param pMap       => additional homogeneous mapping. May be null.
* @param pEyePoint  => homogeneous eyepoint.
* @param numMeridian => number of interior steps at constant theta.
* @param numParallel => number of interior steps at constant phi.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGPA_rulingLimits
(
int     *pIndex0,
int     *pIndex1,
int     numSpan,
bool    closed,
bool    includeEdges
)
    {
    if (closed)
        {
        *pIndex0 = 0;
        *pIndex1 = numSpan - 1;
        }
    else
        {
        if (includeEdges)
            {
            *pIndex0 = 0;
            *pIndex1 = numSpan;
            }
        else
            {
            *pIndex0 = 1;
            *pIndex1 = numSpan - 1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add rulings of a torus to the GraphicsPointArray.
* @param pEllipsoid => toroid whose silhouette is calculated.
* @param pMap       => additional homogeneous mapping. May be null.
* @param pEyePoint  => homogeneous eyepoint.
* @param numMeridian => number of interior steps at constant theta.
* @param numParallel => number of interior steps at constant phi.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDToroid3dRulings
(
GraphicsPointArray  *pGPA,
const   DToroid3d       *pToroid,
const   DMap4d          *pMap,
        int             numMeridian,
        int             numParallel,
        bool            includeEdges
)
    {
    int i;
    bool            fullPhi,    fullTheta;
    double          phi0,       theta0;
    double          phiSweep,   thetaSweep;
    DEllipse3d      ellipse;
    int i0, i1;

    double dPhi;
    double dTheta;

    if (numMeridian < 1)
        numMeridian = 2;

    if (numParallel < 1)
        numParallel = 2;

    bsiDToroid3d_getScalarNaturalParameterSweep (pToroid, &theta0, &thetaSweep, &phi0, &phiSweep);

    fullPhi   = bsiTrig_isAngleFullCircle (phiSweep);
    fullTheta = bsiTrig_isAngleFullCircle (thetaSweep);

    dPhi   = phiSweep / numParallel;
    dTheta = thetaSweep / numMeridian;

    jmdlGPA_rulingLimits (&i0, &i1, numParallel, includeEdges, fullPhi);
    for (i = i0; i <= i1; i++)
        {
        bsiDToroid3d_getParallel (pToroid, &ellipse, phi0 + i * dPhi);
        bsiDEllipse3d_setSweep (&ellipse, theta0, thetaSweep);
        jmdlGraphicsPointArray_addDMap4dDEllipse3d (pGPA, pMap, &ellipse);
        }

    jmdlGPA_rulingLimits (&i0, &i1, numMeridian, includeEdges, fullTheta);
    for (i = i0; i <= i1; i++)
        {
        bsiDToroid3d_getMeridian (pToroid, &ellipse, theta0 + i * dTheta);
        bsiDEllipse3d_setSweep (&ellipse, phi0, phiSweep);
        jmdlGraphicsPointArray_addDMap4dDEllipse3d (pGPA, pMap, &ellipse);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add the silhouette of a toroid to the GraphicsPointArray.
* @param pHConic => general conic intersection.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addHConic
(
GraphicsPointArray  *pGPA,
const   HConic          *pConic
)
    {
    switch (pConic->type)
        {
        case HConic_Ellipse:
            jmdlGraphicsPointArray_addDEllipse4d (pGPA, &pConic->coordinates);
            break;
        case HConic_Line:
            {
            jmdlGraphicsPointArray_addDPoint4d (pGPA, &pConic->coordinates.vector0);
            jmdlGraphicsPointArray_addDPoint4d (pGPA, &pConic->coordinates.vector90);
            jmdlGraphicsPointArray_markBreak (pGPA);
            break;
            }

        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int    jmdlGPA_cb_silhouetteHandler
(
      HConic        *pConic,        /* => single conic */
      DPoint3d      *pPointArray,   /* => polyline */
      int           numPoint,
      unsigned int  curveMask,      /* => bitmask for curve properties */
const RotatedConic  *pSurface,
GraphicsPointArray  *pGPA
)
    {
    if (pConic)
        jmdlGraphicsPointArray_addHConic (pGPA, pConic);

    if (numPoint > 1)
        {
        jmdlGraphicsPointArray_addDPoint3dArray (pGPA, pPointArray, numPoint);
        jmdlGraphicsPointArray_markBreak (pGPA);
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* Add the silhouette of a toroid to the GraphicsPointArray.
* @param pEllipsoid => ellipsoid whose silhouette is calculated.
* @param pMap       => additional homogeneous mapping. May be null.
* @param pEyePoint  => homogeneous eyepoint.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDToroid3dSilhouette
(
GraphicsPointArray  *pGPA,
const   DToroid3d       *pToroid,
const   DMap4d          *pMap,
const   DPoint4d        *pEyePoint
)
    {
    RotatedConic hConic;
    DMap4d hFrame;
    double theta0, dTheta, phi0, dPhi;
    HConic exactSilhouette[4];
    int numExactSilhouette;
    int i;
    /* Put out the inner and outer equator bands, and meridians
        at tighter spacing.  Since the silhouttes tend to go around the
        equator it visually comes out more balanced than the numbers suggest.
        (Cutting to 4 meridians might make sense too.)
    */
    static int numParallel = 2;
    static int numMeridian = 8;
    if (numParallel > 0 || numMeridian > 0)
        jmdlGraphicsPointArray_addDToroid3dRulings
                            (
                            pGPA,
                            pToroid,
                            pMap,
                            numMeridian,
                            numParallel,
                            true
                            );

    bsiDMap4d_initFromTransform (&hFrame, &pToroid->frame, false);

    if (pMap)
        bsiDMap4d_multiply (&hFrame, pMap, &hFrame);
    bsiDToroid3d_getScalarNaturalParameterSweep (pToroid, &theta0, &dTheta, &phi0, &dPhi);
    bsiRotatedConic_initFrameAndSweep (&hConic, RC_Torus, &hFrame, theta0, dTheta, phi0, dPhi);
    bsiRotatedConic_setHoopRadius (&hConic, pToroid->minorAxisRatio);

    if (SUCCESS == bsiRotatedConic_interiorConicSilhouette
                    (exactSilhouette, &numExactSilhouette, &hConic, pEyePoint))
        {
        for (i = 0; i < numExactSilhouette; i++)
            {
            jmdlGraphicsPointArray_addHConic (pGPA, &exactSilhouette[i]);
            }
        }
    else
        {
        double tolerance = 0.05;
        bsiRotatedConic_generalSilhouette
                            (
                            &hConic,
                            pEyePoint,
                            (SilhouetteArrayHandler)jmdlGPA_cb_silhouetteHandler,
                            tolerance,
                            pGPA
                            );

        }

    }

/*---------------------------------------------------------------------------------**//**
* Add the intersection of a plane and a rotated conic to the GraphicsPointArray.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionRotatedConicDPlane3d
(
GraphicsPointArray  *pGPA,
const   RotatedConic    *pRotatedConic,
const   DPlane3d        *pPlane
)
    {
    HConic conic[4];
    int numConic;
    DPoint4d hPlane;
    int i;

    bsiDPlane3d_getDPoint4d (pPlane, &hPlane);

    if (SUCCESS == bsiRotatedConic_intersectPlane (
                    conic, &numConic, pRotatedConic, &hPlane, true))
        {
        for (i = 0; i < numConic; i++)
            jmdlGraphicsPointArray_addHConic (pGPA, &conic[i]);
        }
    else
        {
        double tolerance = 0.05;
        bsiRotatedConic_generalIntersectPlane
                        (
                        pRotatedConic,
                        &hPlane,
                        (SilhouetteArrayHandler)jmdlGPA_cb_silhouetteHandler,
                        tolerance,
                        pGPA
                        );
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add the intersection of a toroid and plane to the GraphicsPointArray.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDToroid3dDPlane3d
(
GraphicsPointArray  *pGPA,
const   DToroid3d       *pToroid,
const   DPlane3d        *pPlane
)
    {
    RotatedConic rotConic;
    if (bsiDToroid3d_getRotatedConic (pToroid, &rotConic))
        {
        jmdlGraphicsPointArray_addIntersectionRotatedConicDPlane3d (pGPA, &rotConic,  pPlane);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add the intersection of a toroid and plane to the GraphicsPointArray.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDEllipsoid3dDPlane3d
(
GraphicsPointArray  *pGPA,
const   DEllipsoid3d    *pEllipsoid,
const   DPlane3d        *pPlane
)
    {
    RotatedConic rotConic;
    if (bsiDEllipsoid3d_getRotatedConic (pEllipsoid, &rotConic))
        {
        jmdlGraphicsPointArray_addIntersectionRotatedConicDPlane3d (pGPA, &rotConic, pPlane);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add the silhouette of a cone and plane to the GraphicsPointArray.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDCone3dDPlane3d
(
GraphicsPointArray  *pGPA,
const   DCone3d         *pCone,
const   DPlane3d        *pPlane
)
    {
    RotatedConic rotConic;
    if (bsiDCone3d_getRotatedConic (pCone, &rotConic))
        {
        jmdlGraphicsPointArray_addIntersectionRotatedConicDPlane3d (pGPA, &rotConic, pPlane);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add simple intersectin points of a rotated conic and DEllipse3d to the graphics point
* array.  Intersections are marked as points.   Each graphics point has the ellipse
* angle as its "a" value.
* @bsimethod                                                    EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionRotatedConicDEllipse3d
(
GraphicsPointArray  *pGPA,
const   RotatedConic    *pRotatedConic,
const   DEllipse3d      *pEllipse,
        bool            boundEllipse,
        bool            boundSurface
)
    {
#define MAX_INTERSECTION 8
    DPoint3d pointArray[8];
    double   angleArray[8];
    int numIntersection;
    GraphicsPoint gp;
    int i;

    if (SUCCESS == bsiRotatedConic_intersectDEllipse3d (
                                    pRotatedConic,
                                    pointArray,
                                    NULL,
                                    angleArray,
                                    &numIntersection,
                                    MAX_INTERSECTION,
                                    pEllipse,
                                    boundEllipse,
                                    boundSurface))
        {
        for (i = 0; i < numIntersection; i++)
            {
            bsiGraphicsPoint_init (&gp,
                                pointArray[i].x, pointArray[i].y, pointArray[i].z, 1.0,
                                angleArray[i], 0, 0);
            jmdlGraphicsPointArray_addGraphicsPoint (pGPA, &gp);
            jmdlGraphicsPointArray_markPoint (pGPA);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add simple intersection points of a DCone3d and DEllipse3d to the graphics point
* array.  Intersections are marked as points.
* @bsimethod                                                    EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDToroid3dDEllipse3d
(
GraphicsPointArray  *pGPA,
const   DToroid3d       *pToroid,
const   DEllipse3d      *pEllipse,
        bool            boundEllipse,
        bool            boundSurface
)
    {
    RotatedConic rotConic;
    if (bsiDToroid3d_getRotatedConic (pToroid, &rotConic))
        {
        jmdlGraphicsPointArray_addIntersectionRotatedConicDEllipse3d (pGPA, &rotConic,  pEllipse, boundEllipse, boundSurface);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add simple intersection points of a DEllipsoid3d and DEllipse3d to the graphics point
* array.  Intersections are marked as points.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDEllipsoid3dDEllipse3d
(
GraphicsPointArray  *pGPA,
const   DEllipsoid3d    *pEllipsoid,
const   DEllipse3d      *pEllipse,
        bool            boundEllipse,
        bool            boundSurface
)
    {
    RotatedConic rotConic;
    if (bsiDEllipsoid3d_getRotatedConic (pEllipsoid, &rotConic))
        {
        jmdlGraphicsPointArray_addIntersectionRotatedConicDEllipse3d (pGPA, &rotConic, pEllipse, boundEllipse, boundSurface);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add simple intersection points of a DCone3d and DEllipse3d to the graphics point
* array.  Intersections are marked as points.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDCone3dDEllipse3d
(
GraphicsPointArray  *pGPA,
const   DCone3d         *pCone,
const   DEllipse3d      *pEllipse,
        bool            boundEllipse,
        bool            boundSurface
)
    {
    RotatedConic rotConic;
    if (bsiDCone3d_getRotatedConic (pCone, &rotConic))
        {
        jmdlGraphicsPointArray_addIntersectionRotatedConicDEllipse3d (pGPA, &rotConic, pEllipse, boundEllipse, boundSurface);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add simple intersection points of a DCone3d and DEllipse3d to the graphics point
* array.  Intersections are marked as points.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDDisk3dDEllipse3d
(
GraphicsPointArray  *pGPA,
const   DDisk3d         *pDisk,
const   DEllipse3d      *pEllipse,
        bool            boundEllipse,
        bool            boundSurface
)
    {
    RotatedConic rotConic;
    if (bsiDDisk3d_getRotatedConic (pDisk, &rotConic))
        {
        jmdlGraphicsPointArray_addIntersectionRotatedConicDEllipse3d (pGPA, &rotConic, pEllipse, boundEllipse, boundSurface);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Find intersections of the polyline with a plane.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_addPolylinePlaneIntersections
(
      GraphicsPointArray    *pDest,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1,
const DPoint4d                  *pPlane,
      double                    tol
)
    {
    int i = i0;
    const GraphicsPoint *pGP = jmdlGraphicsPointArray_getConstPtr (pSource, 0);
    DPoint4d point0, point1, intersectionPoint;
    DPoint3d normalizedIntersectionPoint;
    double a0 = 0.0, a1 = 0.0;


    for (i = i0; i <= i1; i++)
        {
        point1 = pGP[i].point;
        a1 = bsiDPoint4d_dotProduct (&point1, pPlane);
        if (fabs (a1) <= tol)
            {
            jmdlGraphicsPointArray_addDPoint4d (pDest, &point1);
            }

	if (i > i0 && fabs (a0) > tol && fabs (a1) > tol && a0 * a1 < 0.0)
            {
            bsiDPoint4d_add2ScaledDPoint4d (&intersectionPoint, NULL, &point0, a1, &point1, -a0);
            if (bsiDPoint4d_normalize (&intersectionPoint,
                        &normalizedIntersectionPoint))
                {
                jmdlGraphicsPointArray_addDPoint3d (pDest, &normalizedIntersectionPoint);
                }
            }
        a0 = a1;
        point0 = point1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Find intersections of a conic with a plane.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_addDConic4dPlaneIntersections
(
      GraphicsPointArray    *pDest,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1,
const DPoint4d                  *pPlane,
      double                    tol
)
    {
    int iRead = i0;
    int k;
    DConic4d conic;
    DPoint3d trigPoint[4];
    int numIntersection;
    DPoint4d intersectionPoint;

    if (jmdlGraphicsPointArray_getDConic4d (pSource, &iRead, &conic, NULL, NULL, NULL, NULL))
        {
        numIntersection = bsiDConic4d_intersectPlane (&conic, trigPoint, pPlane);
        for (k = 0; k < numIntersection; k++)
            {
            if (bsiDConic4d_angleInSweep (&conic, trigPoint[k].z))
                {
                bsiDConic4d_trigParameterToDPoint4d (&conic, &intersectionPoint,
                                    trigPoint[k].x, trigPoint[k].y);
                jmdlGraphicsPointArray_addDPoint4d (pDest, &intersectionPoint);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Find intersections of a conic with a plane.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_addBezierPlaneIntersections
(
      GraphicsPointArray    *pDest,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1,
const DPoint4d                  *pPlane,
      double                    tol
)
    {
    int iRead = i0;
    int  numIntersection;
    bool    allOn;
    DPoint4d intersectionPoint[MAX_BEZIER_CURVE_ORDER];
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int numPole;
    int k;

    if (jmdlGraphicsPointArray_getBezier (pSource, &iRead,
                    poleArray, &numPole, MAX_BEZIER_CURVE_ORDER))
        {
        if (bsiBezierDPoint4d_intersectPlane (intersectionPoint, NULL, NULL, &numIntersection,
                        &allOn, poleArray, numPole, pPlane)
            && numIntersection > 0
            && !allOn
            )
            {
            for (k = 0; k < numIntersection; k++)
                {
                jmdlGraphicsPointArray_addDPoint4d (pDest, &intersectionPoint[k]);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Find intersections of a conic with a plane.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_addBezierPlaneIntersections
(
      GraphicsPointArray    *pDest,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1,
      DPoint4dCP                 poleArray,
      int                       numPole,
      DPoint4dCP                pPlane,
      double                    tol
)
    {
    int  numIntersection;
    bool    allOn;
    DPoint4d intersectionPoint[MAX_BEZIER_CURVE_ORDER];
    int k;

    if (bsiBezierDPoint4d_intersectPlane (intersectionPoint, NULL, NULL, &numIntersection,
                    &allOn, poleArray, numPole, pPlane)
        && numIntersection > 0
        && !allOn
        )
        {
        for (k = 0; k < numIntersection; k++)
            jmdlGraphicsPointArray_addDPoint4d (pDest, &intersectionPoint[k]);
        }
    }



/*---------------------------------------------------------------------------------**//**
* Add simple intersection points of a plane with all curves of the array.   Add them as
* isolated points in the destination array.
* @param pSource => array whose polylines and curves are interesected with the plane.
* @param pPlane => plane coordinates.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addIntersectionDPlane3d
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPlane3d                *pPlane
)
    {
    int i0, i1;
    int curveType;
    DPoint4d hPlane;
    double tol = 0.0;
    size_t tailIndex;
    bsiDPlane3d_getDPoint4d (pPlane, &hPlane);


    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (
                        pSource, &i1,
                        NULL, NULL,
                        &curveType, i0);
         i0 = i1 + 1)
        {
        if (curveType == 0)
            {
            jmdlGPA_addPolylinePlaneIntersections (pDest, pSource, i0, i1, &hPlane, tol);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) //  BSPLINE_CODED
            {
            jmdlGPA_addBezierPlaneIntersections (pDest, pSource, i0, i1, &hPlane, tol);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            jmdlGPA_addDConic4dPlaneIntersections (pDest, pSource, i0, i1, &hPlane, tol);
            }
        else if (pSource->IsBsplineCurve (i0, tailIndex))
            {
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int order;
            bool isNullInterval;
            double knot0, knot1;
            for (size_t spanIndex = 0;
                pSource->GetBezierSpanFromBsplineCurve (i0, spanIndex, poleArray, order,
                        MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
                {
                if (!isNullInterval)
                    {
                    jmdlGPA_addBezierPlaneIntersections (pDest, pSource, i0, i1, poleArray, order, &hPlane, tol);
                    }
                }
            }
        }
    }



typedef struct
    {
    bool     flag0;
    DPoint4d point0;
    DPoint4d tangent0;

    bool     flag1;
    DPoint4d point1;
    DPoint4d tangent1;

    const DPoint3d *pNormalArray;
    int      numNormal;
    bool     endPoints;
    bool     interiorPoints;
    double   tol;

    GraphicsPointArray *pDest;
    } ExtremaParams;

static DPoint3d s_defaultNormalArray[3] =
    {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0},
    };

/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_init
(
        ExtremaParams           *pParams,
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint3d                *pNormalArray,
        int                     numNormal,
        bool                    endPoints,
        bool                    interiorPoints
)
    {
    static double s_defaultRelTol = 1.0e-10;
    if (!pNormalArray)
        {
        pNormalArray = s_defaultNormalArray;
        if (numNormal > 3 || numNormal <1)
            numNormal = 3;
        }

    pParams->pNormalArray   = pNormalArray;
    pParams->numNormal      = numNormal;
    pParams->endPoints      = endPoints;
    pParams->interiorPoints = interiorPoints;

    pParams->flag0 = false;
    pParams->flag1 = false;
    pParams->pDest = pDest;
    pParams->tol = jmdlGraphicsPointArray_getMaxXYZComponent (pSource) * s_defaultRelTol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_addPoint
(
        ExtremaParams       *pParams,
const   DPoint4d            *pPoint
)
    {
    jmdlGraphicsPointArray_addDPoint4d (pParams->pDest, pPoint);
    }


/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_candidatePoint
(
        ExtremaParams       *pParams,
const   DPoint4d            *pPoint,
const   DPoint4d            *pTangent0,
const   DPoint4d            *pTangent1
)
    {
    int i;
    double a0, a1;
    for (i = 0; i < pParams->numNormal; i++)
        {
        a0 = bsiDPoint3d_dotProduct ((DPoint3d *)pTangent0, &pParams->pNormalArray[i]);
        a1 = bsiDPoint3d_dotProduct ((DPoint3d *)pTangent1, &pParams->pNormalArray[i]);
        if (a0 * a1 <= 0.0)
            {
            extrema_addPoint (pParams, pPoint);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_candidateStartPoint
(
        ExtremaParams       *pParams,
const   DPoint4d            *pPoint,
const   DPoint4d            *pTangent
)
    {
    bool    saveStart = true;
    if (pParams->endPoints)
        {
        if (pParams->flag1)
            {
            if (bsiDPoint4d_realDistance (pPoint, &pParams->point1) > pParams->tol)
                {
                /* The prior endpoint was a dangler. */
                extrema_addPoint (pParams, &pParams->point1);
                }
            else
                {
                extrema_candidatePoint (pParams, pPoint, &pParams->tangent1, pTangent);
                saveStart = false;
                }
            pParams->flag1 = false;
            }

        if (saveStart)
            {
            pParams->point0     = *pPoint;
            pParams->tangent0   = *pTangent;
            pParams->flag0      = true;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_flush
(
        ExtremaParams       *pParams
)
    {
    if (pParams->endPoints)
        {
        if (pParams->flag0)
            extrema_addPoint (pParams, &pParams->point0);
        if (pParams->flag1)
            extrema_addPoint (pParams, &pParams->point1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_CandidateEndPoint
(
        ExtremaParams       *pParams,
const   DPoint4d            *pPoint,
const   DPoint4d            *pTangent
)
    {
    if (pParams->endPoints)
        {
        if (pParams->flag0)
            {
            if (bsiDPoint4d_realDistance (pPoint, &pParams->point0) <= pParams->tol)
                {
                /* We have closed on the start point. Check only for tangent changes */
                extrema_candidatePoint (pParams, pPoint, pTangent, &pParams->tangent0);
                pParams->flag0 = pParams->flag1 = false;
                }
            else
                {
                pParams->point1     = *pPoint;
                pParams->tangent1   = *pTangent;
                pParams->flag1      = true;
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add extrema from a polyline.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_addPolyline
(
      ExtremaParams             *pParams,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1
)
    {
    int i = i0;
    const GraphicsPoint *pGP = jmdlGraphicsPointArray_getConstPtr (pSource, 0);
    DPoint4d tangent0, tangent1;

    if (i0 < i1)
        {
        bsiDPoint4d_subtractDPoint4dDPoint4d (&tangent0, &pGP[i0 + 1].point, &pGP[i0].point);
        extrema_candidateStartPoint (pParams, &pGP[i0].point, &tangent0);
        bsiDPoint4d_subtractDPoint4dDPoint4d (&tangent1, &pGP[i1].point, &pGP[i1 - 1].point);
        extrema_CandidateEndPoint (pParams, &pGP[i1].point, &tangent1);

        for (i = i0 + 1; i < i1; i++)
            {
            bsiDPoint4d_subtractDPoint4dDPoint4d (&tangent1, &pGP[i+1].point, &pGP[i].point);
            extrema_candidatePoint (pParams, &pGP[i].point, &tangent0, &tangent1);
            tangent0 = tangent1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add local extrema from a bezier.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_addConicInteriorPoints
(
      ExtremaParams         *pParams,
const DConic4d              *pConic
)
    {
    DPoint3d F, W, FcrossW;  /* Coefficient vectors */
    int numRoot;
    DPoint4d point;
    double cosine[2], sine[2];
    static double s_relTol = 1.0e-12;
    int i;

    int direction;
    for (direction = 0; direction < pParams->numNormal; direction++)
        {
        bsiDPoint3d_setXYZ (&F,
                    bsiDPoint3d_dotProduct (&pParams->pNormalArray[direction],
                                                (DPoint3d *)&pConic->vector0),
                    bsiDPoint3d_dotProduct (&pParams->pNormalArray[direction],
                                                (DPoint3d *)&pConic->vector90),
                    bsiDPoint3d_dotProduct (&pParams->pNormalArray[direction],
                                                (DPoint3d *)&pConic->center));

        bsiDPoint3d_setXYZ (&W, pConic->vector0.w, pConic->vector90.w, pConic->center.w);

        bsiDPoint3d_crossProduct (&FcrossW, &F, &W);

        numRoot = bsiMath_solveApproximateUnitQuadratic (
                                    &cosine[0], &sine[0],
                                    &cosine[1], &sine[1],
                                    FcrossW.z, FcrossW.x, FcrossW.y,
                                    s_relTol);
        for (i = 0; i < numRoot; i++)
            {
            double theta = bsiTrig_atan2(sine[i], cosine[i]);
            if (bsiDConic4d_angleInSweep (pConic, theta))
                {
                bsiDConic4d_trigParameterToDPoint4d (pConic, &point, cosine[i], sine[i]);
                extrema_addPoint (pParams, &point);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* Add extrema from a conic.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_addConic
(
      ExtremaParams             *pParams,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1
)
    {
    int iRead = i0;
    DConic4d conic;
    DPoint4d point, tangent;

    if (jmdlGraphicsPointArray_getDConic4d (pSource, &iRead, &conic, NULL, NULL, NULL, NULL))
        {

        bsiDConic4d_angleParameterToDPoint4dDerivatives
                                (
                                &conic,
                                &point, &tangent, NULL,
                                conic.start
                                );
        extrema_candidateStartPoint (pParams, &point, &tangent);

        bsiDConic4d_angleParameterToDPoint4dDerivatives
                                (
                                &conic,
                                &point, &tangent, NULL,
                                conic.start + conic.sweep
                                );
        extrema_CandidateEndPoint (pParams, &point, &tangent);
        if (pParams->interiorPoints)
            {
            extrema_addConicInteriorPoints (pParams, &conic);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add local extrema from a bezier.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_addBezierInteriorPoints
(
      ExtremaParams         *pParams,
const DPoint4d              *pPoleArray,
      int                   numPole
)
    {
    double  f[MAX_BEZIER_CURVE_ORDER];
    double  df[MAX_BEZIER_CURVE_ORDER];
    double  w[MAX_BEZIER_CURVE_ORDER];
    double  dw[MAX_BEZIER_CURVE_ORDER];
    double  wdf[MAX_BEZIER_ORDER];
    double  fdw[MAX_BEZIER_ORDER];
    double  g[MAX_BEZIER_ORDER];
    double  root[MAX_BEZIER_ORDER];
    DPoint4d pointAtRoot[MAX_BEZIER_ORDER];
    int     numDerivativePole = numPole - 1;
    int     numProductPole = (numPole - 1) + (numDerivativePole - 1) + 1;
    int     numRoot;
    double  a = numPole - 1;

    int i, direction;

    for (direction = 0; direction < pParams->numNormal; direction++)
        {
        /* Find zeros of df * w - f * dw,
            where f = polexyz dot normal, w is from the poles.
        */
        for (i = 0; i < numPole; i++)
            {
            w[i] = pPoleArray[i].w;
            f[i] = bsiDPoint3d_dotProduct (&pParams->pNormalArray[direction], (DPoint3d *)&pPoleArray[i]);
            }
        for (i = 0; i < numDerivativePole; i++)
            {
            df[i] = a * (f[i+1] - f[i]);
            dw[i] = a * (w[i+1] - w[i]);
            }

        bsiBezier_univariateProduct (wdf, 0, 1,
                            w,   numPole, 0, 1,
                            df,  numDerivativePole, 0, 1);

        bsiBezier_univariateProduct (fdw, 0, 1,
                            f,   numPole, 0, 1,
                            dw,  numDerivativePole, 0, 1);
        bsiBezier_subtractPoles (g, wdf, fdw, numProductPole, 1);

        bsiBezier_univariateRoots (root, &numRoot, g, numProductPole);

        if (numRoot > 0)
            {
            bsiBezierDPoint4d_evaluateArray (pointAtRoot, NULL, pPoleArray, numPole, root, numRoot);

            for (i = 0; i < numRoot; i++)
                extrema_addPoint (pParams, &pointAtRoot[i]);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add extrema from a bezier.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extrema_addBezier
(
      ExtremaParams             *pParams,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1
)
    {
    int iRead = i0;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int numPole;
    DPoint4d point[2];
    DPoint4d tangent[2];
    double param[2];
    param[0] = 0.0;
    param[1] = 1.0;


    if (jmdlGraphicsPointArray_getBezier (pSource, &iRead,
                    poleArray, &numPole, MAX_BEZIER_CURVE_ORDER))
        {
        bsiBezierDPoint4d_evaluateArray (point, tangent, poleArray, numPole, param, 2);
        extrema_candidateStartPoint (pParams, &point[0], &tangent[0]);
        extrema_CandidateEndPoint   (pParams, &point[1], &tangent[1]);

        if (pParams->interiorPoints)
            {
            extrema_addBezierInteriorPoints (pParams, poleArray, numPole);
            }
        }
    }

static void    extrema_addBezier
(
      ExtremaParams             *pParams,
GraphicsPointArrayCP pSource,
      int                       i0,
      int                       i1,
      DPoint4dCP            poleArray,
      int numPole
)
    {
    DPoint4d point[2];
    DPoint4d tangent[2];
    double param[2];
    param[0] = 0.0;
    param[1] = 1.0;

    bsiBezierDPoint4d_evaluateArray (point, tangent, poleArray, numPole, param, 2);
    extrema_candidateStartPoint (pParams, &point[0], &tangent[0]);
    extrema_CandidateEndPoint   (pParams, &point[1], &tangent[1]);

    if (pParams->interiorPoints)
        {
        extrema_addBezierInteriorPoints (pParams, poleArray, numPole);
        }
    }





/*---------------------------------------------------------------------------------**//**
* Find extrema of paths projected onto a direction vector.  Add them as isolated points
* in the destination array.
* @param pNormalArray => array of direction vectors.
* @param numNormal => number of directions.   If pNormalArray is null and numNormal is
*                   1, 2, or 3, the first numNormal principal directions (x, y, z) are used.
* @param endpoints => true to include dangling endpoints.
* @param interiorpoints => true to include interior points where curves become normal to the
*           direction vector.
* @param inflectionPoints => true to include inflection points of curves.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addExtrema
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint3d                *pNormalArray,
        int                     numNormal,
        bool                    endPoints,
        bool                    interiorPoints,
        bool                    inflectionsPoints
)
    {
    int i0, i1;
    int curveType;
    ExtremaParams params;
    size_t tailIndex;
    extrema_init (&params, pDest, pSource, pNormalArray, numNormal, endPoints, interiorPoints);

    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (
                        pSource, &i1,
                        NULL, NULL,
                        &curveType, i0);
         i0 = i1 + 1)
        {
        if (curveType == 0)
            {
            extrema_addPolyline (&params, pSource, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            extrema_addBezier (&params, pSource, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            extrema_addConic (&params, pSource, i0, i1);
            }
        else if (pSource->IsBsplineCurve (i0, tailIndex))
            {
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int order;
            bool isNullInterval;
            double knot0, knot1;
            for (size_t spanIndex = 0;
                pSource->GetBezierSpanFromBsplineCurve (i0, spanIndex, poleArray, order,
                        MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
                {
                if (!isNullInterval)
                    {
                    extrema_addBezier (&params, pSource, i0, i1, poleArray, order);
                    }
                }
            }


        }
    extrema_flush (&params);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/quadraticgeom.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_lineUnitCircleIntersectionTolerance = 1.0e-12;

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value

/*----------------------------------------------------------------------+
|                                                                       |
|   external variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/


/* MAP bsiEllipse_trigToTorus=Geom.trigToTorus ENDMAP */

/*-----------------------------------------------------------------*//**
* Given arrays of sin cos values distributed in theta and phi
* directions of a spherical coordinate system, construct 3d cartesian
* coordinates of a grid of points on the sphere.
*
* @param pPointArray <= Filled array of n1 * n2 points on the torus.
*                                     Theta varies fastest.
* @param pThetaArray => array where x,y coordinates are
*                                       costheta and sintheta
* @param n1 => number of point in pThetaPoint array
* @param pPhiArray => array where x,y coordinates are
*                                       cosphi and sinphi
* @param int            n2 number of points in pPhiPoint array
* @param double                 radius radius multiplier
* @param double         lambda           hoop radius multiplier
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiEllipse_trigToTorus

(
DPoint3dP pPointArray,
DPoint3dP pThetaArray,
int             n1,
DPoint3dP pPhiArray,
int             n2,
double          radius,
double          lambda
)

    {
    int i1, i2;
    double cosPhi, sinPhi, a;
    DPoint3d *pTheta, *pTorusPoint = pPointArray;

    for (i2 = 0; i2 < n2; i2++)
        {
        cosPhi = pPhiArray[i2].x;
        sinPhi = pPhiArray[i2].y;
        a = 1.0 + cosPhi * radius * lambda;
        for (i1 = 0, pTheta = pThetaArray;
             i1 < n1;
             i1++, pTheta++, pTorusPoint++
             )
            {
            pTorusPoint->x = a * pTheta->x;             /* = cos(theta)(1 + r * lambda * cos(phi)) */
            pTorusPoint->y = a * pTheta->y;             /* = sin(theta)(1 + r * lambda * cos(phi)) */
            pTorusPoint->z = radius * sinPhi;           /* = r*sin(phi) */
            }
        }
    }

/* MAP bsiEllipse_trigToConePseudoNormal=Geom.trigToConePseudoNormal ENDMAP */

/*-----------------------------------------------------------------*//**
* Given arrays of sin cos values distributed in theta, and the cone
* taper, construct components of a vector proportional to the normal
* of a unit cone.
*
* @param pPointArray <= Filled array of n1 * n2 points on the torus. Theta varies fastest.
* @param pThetaArray => array where x,y coordinates are costheta and sintheta
* @param n1 => number of points in pThetaPoint array
* @param n2 => number of alpha hoops
* @param lambda => taper factor
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiEllipse_trigToConePseudoNormal

(
DPoint3dP pPointArray,
DPoint3dP pThetaArray,
int             n1,
int             n2,
double          lambda
)
    {
    int i1, i2;
    DPoint3d *pTheta, *pPoint;
    double    zTerm = 1.0 - lambda;

    /* Explicit computation around i2=0 hoop*/
    for (i1 = 0, pPoint = pPointArray, pTheta = pThetaArray;
         i1 < n1;
         i1++, pPoint++, pTheta++)
        {
        pPoint->x = pTheta->x;
        pPoint->y = pTheta->y;
        pPoint->z = zTerm;
        }

    /* Pure copy to additional hoops*/
    for (i2 = 1; i2 < n2; i2++)
        {
        memcpy (pPointArray + i2 * n1, pPointArray, n1 * sizeof (DPoint3d));
        }
    }


/* MAP bsiEllipse_trigToCone=Geom.trigToCone ENDMAP */

/*-----------------------------------------------------------------*//**
* Given arrays of sin cos values distributed in theta direction, and
* taper in axial direction, in a conical coordiante system, construct
* 3d cartesian coordinates of a grid of points on the cone.
*
* @param pPointArray <= Filled array of n1 * n2 points on the torus.
*                                     Theta varies fastest.
* @param pThetaArray => array where x,y coordinates are
*                                       costheta and sintheta
* @param n1 => number of point in pThetaPoint array
* @param pAlphaArray => array where angular and axial multipliers
*                                       <alpha, radius + alpha lambda - 1>
* @param int            n2               number of points in pPhiPoint array
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiEllipse_trigToCone

(
DPoint3dP pPointArray,
DPoint3dP pThetaArray,
int             n1,
DPoint3dP pAlphaArray,
int             n2
)

    {
    int i1, i2;
    double alpha, rho;
    DPoint3d *pTheta, *pPoint;
    pPoint = pPointArray;

    for (i2 = 0; i2 < n2; i2++)
        {
        alpha = pAlphaArray[i2].x;
        rho   = pAlphaArray[i2].y;
        for (i1 = 0, pTheta = pThetaArray; i1 < n1; i1++, pTheta++, pPoint++)
            {
            pPoint->x = rho * pTheta->x;
            pPoint->y = rho * pTheta->y;
            pPoint->z = alpha;
            }
        }
    }

/* MAP bsiEllipse_axialStepsOnCone=Geom.axialStepsOnCone ENDMAP */

/*-----------------------------------------------------------------*//**
* Construct the axial parameter values on a cone.  The formula is
*   (x,y) = (alpha, rdius + alpha (lambda-1))
*
* @param pAlphaArray <= Filled array of numPoint points on the cone.
* @param alpha0 => lower alpha limit
* @param alpha1 => upper alpha limit
* @param numPoint => number of alpha points to compute
* @param radius => radius multiplier
* @param lambda => hoop radius multiplier
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiEllipse_axialStepsOnCone

(
DPoint3dP pAlphaArray,
double          alpha0,
double          alpha1,
int             numPoint,
double          radius,
double          lambda
)

    {
    int i;
    double alpha;
    double delta = numPoint > 1 ? (alpha1 - alpha0) / (double)(numPoint-1) : 0.0;
    for (i = 0; i < numPoint; i++)
        {
        alpha = alpha0 + i * delta;
        pAlphaArray[i].x = alpha;
        pAlphaArray[i].y = radius + alpha * (lambda - 1.0);
        }
    }

/* MAP bsiEllipse_trigToSphere=Geom.trigToSphere ENDMAP */

/*----------------------------------------------------------------------+
|FUNC           bsiEllipse_trigToSphere                                 |
| Given arrays of sin cos values distributed in theta and phi           |
| directions of a spherical coordinate system, construct 3d cartesian   |
| coordinates of a grid of points on the sphere.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsiEllipse_trigToSphere

(
DPoint3dP pSphereArray,  /* <= Filled array of n1 * n2 points on the sphere.
theta varies fastest
*/
DPoint3dP pThetaArray,   /* => array where x,y coordinates are
cos(theta) and sin(theta)

                                */
int             n1,             /* => number of point in pThetaPoint array */
DPoint3d        *pPhiArray,     /* => array where x,y coordinates are
                                        cos(phi) and sin(phi)
                                */
int             n2,             /* number of points in pPhiPoint array */
double          radius          /* radius multiplier */
)
    {
    int i1, i2;
    DPoint3d *pSphere = pSphereArray;
    double rCosPhi, rSinPhi;
    DPoint3d *pTheta;
    for (i2 = 0; i2 < n2; i2++)
        {
        rCosPhi = radius * pPhiArray[i2].x;
        rSinPhi = radius * pPhiArray[i2].y;
        for (i1 = 0, pTheta = pThetaArray;
             i1 < n1;
             i1++, pSphere++, pTheta++
            )
            {
            pSphere->x = pTheta->x * rCosPhi;   /* = r*cos(theta)*cos(phi) */
            pSphere->y = pTheta->y * rCosPhi;   /* = r*sin(theta)*cos(phi) */
            pSphere->z = rSinPhi;               /* = r*sin(phi) */
            }
        }
    }

/*----------------------------------------------------------------------+
|   name      hToroidComponentRange                                     |
| Return the range of a component (x,y,z) multiplied by spherical       |
| coordinates driving functions.                                        |
+----------------------------------------------------------------------*/
static StatusInt    hToroidComponentRange

(
double*         pXMin,           /* <= min coordiante of range box */
double*         pXMax,           /* <= max coordinate of range box */
double          x0,             /* <= center component */
double          u,              /* => primary plane vector component */
double          v,              /* => primary plane vector component */
double          w,              /* => component along axis through hole */
double          lambda,         /* => radius multiplier */
DRange3dCP pRange         /* => parameter space range box */
)
    {
    double r2 = u * u + v * v;
    double r = sqrt (r2);
    double rLambda = r * lambda;
    double rho = sqrt (rLambda * rLambda  + r2);
    int i,j;
    double x, xMin, xMax;
    double cPhi, sPhi;

    /* Remark: This assumes lambda < 1 (strictly).  There is a second case
        for lambda >= 1 */

    /* Remark: Range part is not implemented */
    xMin = xMax = 0.0;
    for (i = 0; i < 2; i++, r = -r, rLambda = -rLambda)
        {
        cPhi = rLambda / rho;
        sPhi = w / rho;
        for (j = 0; j < 2; j++, cPhi = -cPhi, sPhi = -sPhi)
            {
            x = r + cPhi * lambda * r + sPhi * w;
            if (x < xMin)
                {
                xMin = x;
                }
            else if (x > xMax)
                {
                xMax = x;
                }
            }
        }
    *pXMin = x0 + xMin;
    *pXMax = x0 + xMax;
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|FUNC           ellipseTrigRange                                     |
| Determine the sine and cosine of the angle at which the scalar        |
| equation cosCoff*cos(theta) + sinCoff*sin(theta) attains a min or max |
|                                                                       |
|NOTE This may be either a min or a max.  The other extrema is 180      |
| degrees away.                                                         |
|NORET                                                                  |
+----------------------------------------------------------------------*/
static double  ellipseTrigRange   /* <= value of extrema */

(
double*         cosineP,        /* <= cosine of extremal angle */
double*         sineP,          /* <= sine of extremal angle */
double          cosCoff,        /* => coefficient of cosine */
double          sinCoff         /* => coefficient of sine */
)
    {
    double r;
    r = sqrt ( cosCoff * cosCoff + sinCoff * sinCoff);
    if (r > 0.0)
        {
        *cosineP = cosCoff / r;
        *sineP   = sinCoff / r;
        }
    else
        {
        *cosineP = 1.0;
        *sineP   = 0.0;
        }
    return (*cosineP) * cosCoff + (*sineP) * sinCoff;
    }

/*----------------------------------------------------------------------+
|   name      hEllipsoidComponentRange                                  |
| Return the range of a component (x,y,z) multiplied by spherical       |
| coordinates driving functions.                                        |
+----------------------------------------------------------------------*/
static void    hEllipsoidComponentRange

(
double*         minP,           /* <= min coordiante of range box */
double*         maxP,           /* <= max coordinate of range box */
double          x0,             /* <= center component */
double          u,              /* => basis vector component multiplied by cos */
double          v,              /* => basis vector component multiplied by sin */
double          w               /* => component towards pole */
)
    {
    double cos1, sin1, cos2, sin2;
    double range1;
    double range2;

    range1 = ellipseTrigRange (&cos1, &sin1, u, v);
    range2 = fabs (ellipseTrigRange (&cos2, &sin2, range1, w));
    *minP = x0 - range2;
    *maxP = x0 + range2;
    }


/*-----------------------------------------------------------------*//**
* Evaluate an ellipse at a given angle.
*
* @param pPoint
* @param pPoint0
* @param vector1
* @param vector2
* @param theta
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiEllipse_evaluate

(
DPoint3dP pPoint,
DPoint3dCP pPoint0,
DPoint3dCP vector1,
DPoint3dCP vector2,
double      theta
)
    {
    double cosTheta, sinTheta;
    cosTheta = cos (theta);
    sinTheta = sin (theta);
    pPoint->x = pPoint0->x + cosTheta * vector1->x + sinTheta * vector2->x;
    pPoint->y = pPoint0->y + cosTheta * vector1->y + sinTheta * vector2->y;
    pPoint->z = pPoint0->z + cosTheta * vector1->z + sinTheta * vector2->z;
    }
#ifdef Compile_bsiDEllipse4d_rangeFrom3dPoints

/*-----------------------------------------------------------------*//**
* @nodoc
* @description Return the minmax points of the range box on an ellipse defined
*  by C, P0, P90 points
*
* @param minPointP <= min point of range box
* @param maxPointP <= max point of range box
* @param arrayP => ellipse center, P0, P90
* @param theta0 => start angle
* @param sweep => sweep angle
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_rangeFrom3dPoints

(
DPoint3dP        minPointP,
DPoint3dP        maxPointP,
DPoint3dCP arrayP,
double          theta0,
double          sweep
)

    {
    DPoint3d vector0, vector90, startPoint, endPoint;

    vector0.DifferenceOf (arrayP[1], arrayP[0]);
    vector90.DifferenceOf (arrayP[2], arrayP[0]);
    bsiEllipse_evaluate (&startPoint, &arrayP[0], &vector0, &vector90, theta0);
    bsiEllipse_evaluate (&endPoint, &arrayP[0], &vector0, &vector90, theta0 + sweep);
    *minPointP = *maxPointP = startPoint;

    FIX_MIN (endPoint.x, minPointP->x);
    FIX_MIN (endPoint.y, minPointP->y);
    FIX_MIN (endPoint.z, minPointP->z);
    FIX_MAX (endPoint.x, maxPointP->x);
    FIX_MAX (endPoint.y, maxPointP->y);
    FIX_MAX (endPoint.z, maxPointP->z);

    bsiEllipse_componentRange ( &minPointP->x, &maxPointP->x,
                        arrayP[0].x, vector0.x, vector90.x,
                        theta0, sweep);

    bsiEllipse_componentRange ( &minPointP->y, &maxPointP->y,
                        arrayP[0].y, vector0.y, vector90.y,
                        theta0, sweep);

    bsiEllipse_componentRange ( &minPointP->z, &maxPointP->z,
                        arrayP[0].z, vector0.z, vector90.z,
                        theta0, sweep);
    }
#endif

/*-----------------------------------------------------------------*//**
* @nodoc
* @description Computes the minmax points of the range box on a full ellipsoid
* @param rangeP <= range box
* @param arrayP => ellipsoid center, P0, P90, north pole
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiHEllipsoid_rangeFrom3dPoints

(
DRange3dP        rangeP,
DPoint3dCP arrayP
)

    {
    DPoint3d vector0, vector90, vectorNorth;

    vector0.DifferenceOf (arrayP[1], arrayP[0]);
    vector90.DifferenceOf (arrayP[2], arrayP[0]);
    vectorNorth.DifferenceOf (arrayP[3], arrayP[0]);

    hEllipsoidComponentRange ( &rangeP->low.x, &rangeP->high.x,
                        arrayP[0].x, vector0.x, vector90.x, vectorNorth.x);

    hEllipsoidComponentRange ( &rangeP->low.y, &rangeP->high.y,
                        arrayP[0].y, vector0.y, vector90.y, vectorNorth.y);

    hEllipsoidComponentRange ( &rangeP->low.z, &rangeP->high.z,
                        arrayP[0].z, vector0.z, vector90.z, vectorNorth.z);
    }


/*-----------------------------------------------------------------*//**
* @nodoc
* @description Computes the minmax points of the range box on a full toroid
* @param rangeP <= range box
* @param arrayP => ellipsoid center, P0, P90, north pole
* @param lambda => radius multiplier
* @param pParamRange => parameter space range
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiHToroid_rangeFrom3dPoints

(
DRange3dP        rangeP,
DPoint3dCP arrayP,
double    lambda,
DRange3dCP pParamRange
)

    {
    DPoint3d vector0, vector90, vectorNorth;

    vector0.DifferenceOf (arrayP[1], arrayP[0]);
    vector90.DifferenceOf (arrayP[2], arrayP[0]);
    vectorNorth.DifferenceOf (arrayP[3], arrayP[0]);

    hToroidComponentRange ( &rangeP->low.x, &rangeP->high.x,
                        arrayP[0].x, vector0.x, vector90.x, vectorNorth.x,
                        lambda, NULL);

    hToroidComponentRange ( &rangeP->low.y, &rangeP->high.y,
                        arrayP[0].y, vector0.y, vector90.y, vectorNorth.y,
                        lambda, NULL);

    hToroidComponentRange ( &rangeP->low.z, &rangeP->high.z,
                        arrayP[0].z, vector0.z, vector90.z, vectorNorth.z,
                        lambda, NULL);
    }


/*-----------------------------------------------------------------*//**
* @description Find intersections of a (full) unit sphere with a plane.
* @param pCenter <= Nearest plane point to origin.
*                   For tangency case return value 1 this is the tangency point.
*                   For full intersection case this is the circle center.
* @param pPoint1 <= 0 degree point end of U vector of circle
* @param pPoint2 <= 90 degree point end of V vector of circle
* @param pPlane => homogeneous plane coefficients
* @return 0 if no intersection, 1 if tangency point, 2 if full circle
* @group "Quadratic Geometry"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_intersectPlaneUnitSphere

(
DPoint3dP pCenter,
DPoint3dP pPoint1,
DPoint3dP pPoint2,
DPoint4dCP pPlane
)
    {
    //double alphaSquared = pPlane->w * pPlane->w;
    double gradSquared = pPlane->x * pPlane->x
                       + pPlane->y * pPlane->y
                       + pPlane->z * pPlane->z; /* squared distance to plane */
    double absDist, signedDist, squaredDist;
    static double epsilon = 1.0e-12;
    double onePlusEpsilon = 1.0 + epsilon;
    double oneMinusEpsilon = 1.0 - epsilon;
    DPoint3d uVector, vVector, wVector;
    int    resultType;
    if (gradSquared == 0.0)
        {
        resultType = 0;
        pCenter->Zero ();
        }
    else
        {
        DPoint3d normal;
        signedDist  = - pPlane->w / sqrt (gradSquared);
        absDist     = fabs (signedDist);
        squaredDist = absDist * absDist;
        normal.x = pPlane->x;
        normal.y = pPlane->y;
        normal.z = pPlane->z; bsiDPoint3d_getNormalizedTriad (&normal, &uVector, &vVector, &wVector) ; /* THISWAS a bool thrown away as a statement */
        pCenter->Scale (wVector, signedDist);
        if (absDist > onePlusEpsilon)
            {
            resultType = 0;
            }
        else if (absDist < oneMinusEpsilon)
            {
            double radius = sqrt (1.0 - squaredDist);
            resultType = 2;
            pPoint1->SumOf (*pCenter, uVector, radius);
            pPoint2->SumOf (*pCenter, vVector, radius);
            }
        else
            {
            resultType = 1;
            }
        }
    return resultType;
    }


/*-----------------------------------------------------------------*//**
@return the largest absolute value among 3 values.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static double maxAbs3

(
double xx,
double yy,
double zz
)
    {
    double aa = fabs (xx);
    yy = fabs (yy);
    if (yy > aa)
        aa = yy;
    zz = fabs (zz);
    if (zz > aa)
        aa = zz;
    return aa;
    }

/*-----------------------------------------------------------------*//**
* Compute a reference number from products of a plane with center, U, and V.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static double ellipsePlaneRefNum

(
DPoint4dCP pCenter,
DPoint4dCP pVectorU,
DPoint4dCP pVectorV,
DPoint4dCP pPlane
)
    {
    double aX = fabs (pPlane->x) * maxAbs3 (pCenter->x, pVectorU->x, pVectorV->x);
    double aY = fabs (pPlane->y) * maxAbs3 (pCenter->y, pVectorU->y, pVectorV->y);
    double aZ = fabs (pPlane->z) * maxAbs3 (pCenter->z, pVectorU->z, pVectorV->z);
    double aW = fabs (pPlane->w) * maxAbs3 (pCenter->w, pVectorU->w, pVectorV->w);
    return aX + aY + aZ + aW;
    }


/*-----------------------------------------------------------------*//**
* @nodoc
* @description Find intersections of a (full) ellipse with a plane.
* See quadeqn.c for description of degenerate cases.
* n=1 is a single tangency point returned in trigP[0]
* n=2 is two simple intersections returned in trigP[0..1]
* The three component values in trigP are:
* <UL>
* <LI>x == cosine of angle
* <LI>y == sine of angle
* <LI>z == angle in radians
* </UL>
* RETURNS The number of intersections, i.e. 0, 1, or 2
*
* @param trigP <= 2 points: cosine, sine, theta values of plane intersection
* @param centerP => ellipse center
* @param vectorUP => 0 degree vector
* @param vectorVP => 90 degree vector
* @param planeP => homogeneous plane equation
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_intersectPlane

(
DPoint3dP trigP,
DPoint4dCP centerP,
DPoint4dCP vectorUP,
DPoint4dCP vectorVP,
DPoint4dCP planeP
)
    {
    double alpha = bsiDPoint4d_dotProduct ( centerP, planeP );
    double beta  = bsiDPoint4d_dotProduct ( vectorUP, planeP );
    double gamma = bsiDPoint4d_dotProduct ( vectorVP, planeP );
    double coffTol = s_lineUnitCircleIntersectionTolerance * ellipsePlaneRefNum (centerP, vectorUP, vectorVP, planeP);
    int n = 0;

    if (fabs (alpha) + fabs (beta) + fabs (gamma) > coffTol)
        n = bsiMath_solveApproximateUnitQuadratic (
                &trigP[0].x, &trigP[0].y,
                &trigP[1].x, &trigP[1].y,
                alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance );

    if ( n == 1 )
        {
        /* solveApproximate returns distinct (but very close) points
            on the plane and ellipse.   Take the one on the ellipse:
        */
        trigP[0].x = trigP[1].x;
        trigP[0].y = trigP[1].y;
        trigP[0].z = bsiTrig_atan2 ( trigP[0].y, trigP[0].x );
        }
    else if ( n == 2 )
        {
        trigP[0].z = bsiTrig_atan2 ( trigP[0].y, trigP[0].x );
        trigP[1].z = bsiTrig_atan2 ( trigP[1].y, trigP[1].x );

        }
    else
        {
        /* suppress the degenerate cases */
        n = 0;
        }
    return n;
    }

/* MAP bsiCone_silhouetteAngles=Geom.silhouetteAngles ENDMAP */

/*-----------------------------------------------------------------*//**
*
* @param pTrigPoint <= array where x,y are cosine, sine of
*                      silhouette angles. z is zero -- maybe a convenient
*                      place for the angles if you need to stash them
* @param pConePoint => 4 defining points of the cone
* @param lambda => taper fraction
* @param pEyePoint => eyepoint, in same coordinates.
*                     For perspective, from xyz, set w=1
*                     For flat view in direction xyz, set w=0
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiCone_silhouetteAngles

(
DPoint3dP pTrigPoint,
DPoint3dCP pConePoint,
double            lambda,
DPoint4dCP pEyePoint
)
    {
    Transform T, Tinverse;
    bool    result = false;
    DPoint4d cylinderEye, coneEye;
    double mu = 1.0 - lambda;
    int n;
    bsiTransform_initFrom4Points (&T, &pConePoint[0], &pConePoint[1], &pConePoint[2], &pConePoint[3]);

    if (Tinverse.InverseOf (T))

        {
        /* Find the eyepoint relative the cylinder with the same base circle and axis: */
        bsiTransform_multiplyDPoint4dArray (&Tinverse, &cylinderEye, pEyePoint, 1);
        /* And in the cone system:*/
        coneEye.x = cylinderEye.x;
        coneEye.y = cylinderEye.y;
        coneEye.z = cylinderEye.z;
        coneEye.w = cylinderEye.w - mu * cylinderEye.z;

        pTrigPoint[0].Zero ();
        pTrigPoint[1].Zero ();

        n = bsiMath_solveApproximateUnitQuadratic (
                        &pTrigPoint[0].x, &pTrigPoint[0].y,
                        &pTrigPoint[1].x, &pTrigPoint[1].y,
                        -coneEye.w,
                        coneEye.x,
                        coneEye.y,
                        s_lineUnitCircleIntersectionTolerance
                        );

        if (n == 2)
            {
            result = true;
            }
        }

    return result;
    }




/*-----------------------------------------------------------------*//**
* @nodoc
* @description Return the intersection of homogeneous ellipse (i.e. ellipse,
* parabola, or hyperbola) with a cylinder, where the cylinder
* is defined by its (skewed) coordinate frame.  The cylinder is the
* set of all points that have x^2+y^2 = 1 in the skewed frame.
*
* @param pPointArray <= array of UP TO 4 points
* @param pEllipseCoord <= array of the cosine and sine values of the
* @param  solution points in the ellipse coordinates.
* @param pCylCoord <= array of the cylindrical coordinates cosine, sine, z
* @param  of the solution points in the cylinder system.
* @param pFrame => coordinate frame for the cylinder
* @param pEllipse => homogeneous basis points for ellipse.
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiCylinder_intersectHEllipse

(
DPoint3dP pPointArray,
DPoint2dP pEllipseCoord,

DPoint3dP pCylCoord,

TransformCP pFrame,
DEllipse4dP pEllipse
)
    {
    Transform inverseFrame;
    int numSolution;
    numSolution = 0;

    if (inverseFrame.InverseOf (*pFrame))

        {
        DEllipse4d localEllipse;
        DMatrix4d  inverseCylinder;
        RotMatrix matB;
        RotMatrix matBinv;

        bsiDMatrix4d_initFromTransform (&inverseCylinder, &inverseFrame);
        localEllipse = *pEllipse;
        /* Put the ellipse in local coordinates*/
        bsiDMatrix4d_multiplyMatrixPoint (&inverseCylinder, &localEllipse.center  , &pEllipse->center  );
        bsiDMatrix4d_multiplyMatrixPoint (&inverseCylinder, &localEllipse.vector0 , &pEllipse->vector0 );
        bsiDMatrix4d_multiplyMatrixPoint (&inverseCylinder, &localEllipse.vector90, &pEllipse->vector90);

        /* Compress out the z components of the ellipse*/
        bsiQCoff_xyEllipseRotMatrix (&matB, &localEllipse);

        if (bsiRotMatrix_invertRotMatrix(&matBinv, &matB))
            {
            RotMatrix matA0;
            RotMatrix temp;
            RotMatrix matBinvTr;
            RotMatrix matA1;
            DPoint2d localPoint[4];
            int numIntersection;

            bsiRotMatrix_scaleRows (&temp, &matBinv, 1.0, 1.0, -1.0);
            matBinvTr.TransposeOf (matBinv);
            matA0.InitProduct (matBinvTr, temp);
            matA1 = RotMatrix::FromScaleFactors (1.0, 1.0, -1.0);

            if (bsiQCoff_intersectQuadricSections (localPoint, &numIntersection, matA0, matA1))
                {
                /* localPoint contains intersection points in the xy plane of the cylinder frame.*/
                /* Move them back to the ellipse system and thence to the world*/
                DPoint3d frame0Point, frame1Point;
                DPoint3d cartesianResult;
                DPoint4d homogeneousResult;
                int i;
                for (i = 0; i < numIntersection; i++)
                    {
                    /* frame0 is the (w*cosine(theta),w*sine(theta),w) system for the cylinder.*/
                    /* frame1 is the (w*cosine(phi)  ,w*sine(phi)  ,w) system for the ellipse*/
                    /* The B matrix goes from frame1 to frame0.*/
                    frame0Point.Init ( localPoint[i].x, localPoint[i].y, 1.0);
                    matBinv.Multiply (&frame1Point, &frame0Point, 1);
                    /* and frame1 coordiantes are also coordinates on the master ellipse!!!*/
                    bsiDPoint4d_add3ScaledDPoint4d (&homogeneousResult, NULL,
                                                &pEllipse->vector0,  frame1Point.x,
                                                &pEllipse->vector90, frame1Point.y,
                                                &pEllipse->center,   frame1Point.z);
                    if (homogeneousResult.GetProjectedXYZ (cartesianResult))
                        {

                        if (pPointArray)
                            pPointArray [numSolution] = cartesianResult;

                        if (pEllipseCoord)
                            {
                            pEllipseCoord [numSolution].x = frame1Point.x / frame1Point.z;
                            pEllipseCoord [numSolution].y = frame1Point.y / frame1Point.z;
                            }

                        if (pCylCoord)
                            {
                            inverseFrame.Multiply (&pCylCoord [numSolution], &cartesianResult, 1);
                            }

                        numSolution++;
                        }
                    }
                }
            }
        }
    return numSolution;
    }


/*-----------------------------------------------------------------*//**
* @nodoc
* @description Extract a 3x3 matrix from 4D ellipse basis vectors.
* @param pMatrix <= extracted matrix
* @param pEllipse => homogeneous basis points for ellipse
* @param xIndex => index of coordinate x,y,z,w to be treated as X in matrix
* @param yIndex => index of coordinate x,y,z,w to be treated as Y in matrix
* @param wIndex => index of coordinate x,y,z,w to be treated as W in matrix
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_extract3x3
(
RotMatrixP   pMatrix,
DEllipse4dCP pEllipse,
int           xIndex,
int           yIndex,
int           wIndex
)
    {
    DVec3d columnX, columnY, columnZ;
    pMatrix->GetColumns (columnX, columnY, columnZ);
    bsiDPoint4d_selectDPoint3d (&pEllipse->vector0, &columnX, xIndex, yIndex, wIndex);
    bsiDPoint4d_selectDPoint3d (&pEllipse->vector90, &columnY, xIndex, yIndex, wIndex);
    bsiDPoint4d_selectDPoint3d (&pEllipse->center,   &columnZ, xIndex, yIndex, wIndex);
    }


/*-----------------------------------------------------------------*//**
* @nodoc
* @description Calculate the apparent intersection of two conic sections, where the first has a (callersupplied) inverse.
* @param pTrig0 <= 0 to 4 intersection points as cos, sin, theta triples in ellipse0 space
* @param pTrig1 <= 0 to 4 intersection points as cos, sin, theta triples in ellipse1 space
* @param pCount <= number of intersections.  May be 0 to 4
* @param pB0 => first ellipse matrix
* @param pB0inverse => and its inverse
* @param pB1 => second ellipse matrix
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_intersectPrepped2d

(
DPoint3dP pTrig0,
DPoint3dP pTrig1,
int           *pCount,
RotMatrixCP pB0,
RotMatrixCP pB0inverse,
RotMatrixCP pB1
)
    {
    StatusInt status = ERROR;
    double sinTheta1[4], cosTheta1[4], theta1[4];
    double c1, s1, t1;
    DPoint3d F0;
    int i;
    int numIntersection, numTangent;
    RotMatrix M;
    double r0, r02, w02;
    int k;
    static double s_tangentTolerance = 1.0e-4;
    static double s_duplicateRootTolerance = 1.0e-6;
    bool    duplicateRoot;
    *pCount = 0;

    M.InitProduct (*pB0inverse, *pB1);

    if (0 <= bsiMath_conicIntersectUnitCircle (cosTheta1, sinTheta1, theta1, &numIntersection,
                        M.form3d[0][2], M.form3d[0][0], M.form3d[0][1],
                        M.form3d[1][2], M.form3d[1][0], M.form3d[1][1],
                        M.form3d[2][2], M.form3d[2][0], M.form3d[2][1]))

        {
        for (i = 0; i < numIntersection; i++)
            {
            c1 = cosTheta1[i];
            s1 = sinTheta1[i];
            bsiRotMatrix_multiplyComponents (&M, &F0, c1, s1, 1.0);
            if (pTrig0)
                {
                pTrig0[i].Init ( F0.x, F0.y, bsiTrig_atan2 (F0.y, F0.x));
                }
            if (pTrig1)
                {
                pTrig1[i].Init ( c1, s1, theta1[i]);
                }
            }

        /* Also include near tangencies that are not close to existing roots. */
        if (   numIntersection < 4
            && 0 <= bsiMath_conicTangentUnitCircle (cosTheta1, sinTheta1, theta1, &numTangent,
                        M.form3d[0][2], M.form3d[0][0], M.form3d[0][1],
                        M.form3d[1][2], M.form3d[1][0], M.form3d[1][1],
                        M.form3d[2][2], M.form3d[2][0], M.form3d[2][1]))
            {
            for (i = 0; i < numTangent && numIntersection < 4; i++)
                {
                c1 = cosTheta1[i];
                s1 = sinTheta1[i];
                t1 = theta1[i];
                duplicateRoot = false;
                for (k = 0; k < numIntersection && !duplicateRoot; k++)
                    {
                    if (fabs (t1 - pTrig1[k].z) <= s_duplicateRootTolerance)
                        duplicateRoot = true;
                    }

                if (!duplicateRoot)
                    {
                    bsiRotMatrix_multiplyComponents (&M, &F0, c1, s1, 1.0);
                    r02 = F0.x * F0.x + F0.y * F0.y;
                    w02 = F0.z * F0.z;
                    if (fabs (r02 - w02)  < s_tangentTolerance * w02)
                        {
                        k = numIntersection++;
                        r0 = sqrt (r02);
                        if (pTrig0)
                            {
                            pTrig0[k].Init ( F0.x / r0, F0.y / r0, bsiTrig_atan2 (F0.y, F0.x));
                            }
                        if (pTrig1)
                            {
                            pTrig1[k].Init ( c1, s1, t1);
                            }
                        }
                    }
                }
            }
        *pCount = numIntersection;
        status = SUCCESS;

        }

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          intersectSingularProjections             |
|                                                                       |
| author        EarlinLutz                                  10/97       |
|                                                                       |
| Calculate the apparent intersection of two conic sections, where the  |
| first has a (caller-supplied) inverse.                                |
+----------------------------------------------------------------------*/
static int intersectSingularProjections

(
DPoint3dP pTrig0,        /* <= 0 to 4 intersection points as (cos, sin, theta) triples in ellipse0 space */
      DPoint3d      *pTrig1,        /* <= 0 to 4 intersection points as (cos, sin, theta) triples in ellipse1 space */
      int           *pCount,        /* <= number of intersections.  May be 0 to 4 */
const RotMatrix     *pB0,           /* => first ellipse matrix */
const RotMatrix     *pB1            /* => second ellipse matrix */
)
    {
    DVec3d nullVector[2];
    RotMatrix transpose[2];
    const RotMatrix *matrixPointer[2];
    DVec3d  planeVector;
    StatusInt status = ERROR;
    double cosine[2][2], sine[2][2], angle[2][2];
    int count[2];
    double alpha, beta, gamma;
    int i, j, k;
*pCount = 0;

    /* Find null spaces of both transposes.  We expect the null spaces to be 1D, i.e.
        each null space defines the line in which the (singular) ellipse is embedded */
    matrixPointer[0] = pB0;
    matrixPointer[1] = pB1;
    for (i = 0; i < 2; i++)
        {
        transpose[i].TransposeOf (*(matrixPointer[i]));
        RotMatrix rotationA, rotationB;
        DPoint3d scalePoint;
        transpose[i].FactorRotateScaleRotate (rotationA, scalePoint, rotationB);
        double tol = 1.0e-12 * scalePoint.x;
        if (fabs (scalePoint.z) <= tol && fabs (scalePoint.y) > tol)
            {
            nullVector[i] = DVec3d::FromRow (rotationB, 2);
            }
        else
            {
            return ERROR;
            }
        }

    /* Intersect each ellipse with the line of the other.
        Ellipse is: X = B * F       (F=[cos sin 1])
        Line is:    H dot X = 0     (H = line vector, i.e. null space of the other ellipse )
        intersection is: H dot B * F = 0
                H dot B0 cos + H dot B1 sin + H dot B2 = 0
     */
    for (i = 0; i < 2; i++)
        {
        j = 1 - i;
        planeVector = nullVector[i];

        alpha = planeVector.DotProductColumn (*matrixPointer[i], 2);
        beta  = planeVector.DotProductColumn (*matrixPointer[i], 0);
        gamma = planeVector.DotProductColumn (*matrixPointer[i], 1);
        count[i] = bsiMath_solveApproximateUnitQuadratic (
                        &cosine[i][0], &sine[i][0],
                        &cosine[i][1], &sine[i][1],
                        alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance);
        for (k = 0; k < count[i]; k++)
            {
            angle[i][k] = bsiTrig_atan2 (sine[i][k], cosine[i][k]);
            }
        }

    /* Each intersection I0 of ellipse0 with line1 combines with each intersection I1 of ellipse1 with line0
        to generate an output intersection */
    if (count[0] > 0 && count[1] > 0)
        {
        int numOut = 0;
        status = SUCCESS;
        for (i = 0; i < count[0]; i++)
            {
            for (j = 0; j < count[1]; j++)
                {
                pTrig0[numOut].Init ( cosine[0][i], sine[0][i], angle[0][i]);
                pTrig1[numOut].Init ( cosine[1][j], sine[1][j], angle[1][j]);
                numOut++;
                }
            }
        *pCount = numOut;
        }
    else if (count[0] >= 0 && count[1] >= 0)
        {
        /* The lines cross, but at least one of the ellipses is placed so that it does not intersect
            the other ellipse's line */
        status = SUCCESS;
        }
    else if (count[0] == -2 && count[1] == -2)
        {
        /* The lines are collinear */
        status = ERROR;
        }
    else
        {
        /* ??? */
        status = ERROR;
        }

    return status;
    }


/*-----------------------------------------------------------------*//**
* @nodoc
* @description Calculate the apparent intersection of two HEllipses, equating two
* indicated components and using a third as weight.
*
* @param pTrig0 <= 0 to 4 intersection points as cos, sin, theta triples in ellipse0 space
* @param pTrig1 <= 0 to 4 intersection points as cos, sin, theta triples in ellipse1 space
* @param pCount <= number of intersections.  May be 0 to 4
* @param pEllipse0 => homogeneous basis points for ellipse.
* @param pEllipse1 => homogeneous basis points for ellipse
* @param xIndex => index of coordinate x,y,z,w to be treated as X
* @param yIndex => index of coordinate x,y,z,w to be treated as Y
* @param wIndex => index of coordinate x,y,z,w to be treated as W
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiDEllipse4d_intersectUnboundedHEllipse2d

(
DPoint3dP pTrig0,
DPoint3dP pTrig1,
int           *pCount,
DEllipse4dCP pEllipse0,
DEllipse4dCP pEllipse1,
int           xIndex,
int           yIndex,
int           wIndex
)
    {
    RotMatrix B0, B1;
    RotMatrix B0inverse, B1inverse;
    double condition0, condition1;
    StatusInt status = ERROR;

    bsiDEllipse4d_extract3x3 (&B0, pEllipse0, xIndex, yIndex, wIndex);
    bsiDEllipse4d_extract3x3 (&B1, pEllipse1, xIndex, yIndex, wIndex);
    bsiRotMatrix_invertRotMatrixByOrthogonalFactors (&B0, &B0inverse, &condition0);
    bsiRotMatrix_invertRotMatrixByOrthogonalFactors (&B1, &B1inverse, &condition1);

    if (condition0 == 0.0 && condition1 == 0.0)

        {
        status = intersectSingularProjections (pTrig0, pTrig1, pCount, &B0, &B1);
        }
    else if (condition0 >= condition1)
        {
        status = bsiDEllipse4d_intersectPrepped2d (pTrig0, pTrig1, pCount, &B0, &B0inverse, &B1);
        }
    else if (condition1 > condition0)
        {
        status = bsiDEllipse4d_intersectPrepped2d (pTrig1, pTrig0, pCount, &B1, &B1inverse, &B0);
        }

    return status;
    }
#ifdef CompileAll
/*----------------------------------------------------------------------+
|                                                                       |
| name          circleProduct                                           |
|                                                                       |
| author        EarlinLutz                              10/97           |
|                                                                       |
| Form the product A^T sigma B where sigma = diag (1,1,-1)              |
+----------------------------------------------------------------------*/
static double circleProduct

(
DPoint3dP pA,
DPoint3dP pB
)
    {
    return pA->x * pB->x + pA->y * pB->y - pA->x * pB->x;
    }
#endif

/*-----------------------------------------------------------------*//**
* @nodoc
* @description Calculate the apparent intersection of a line and ellipse, using 2
* indicated components and using a third as weight.
*
* @param pTrig <= 0 to 2 intersection points as cos, sin, theta triples in ellipse space
* @param pLambda <= 0 to 4 intersection points as lambda0, lambda1, 0 triples in line space
* @param pCount <= number of intersections.  May be 0 to 4
* @param pEllipse => homogeneous basis points for ellipse.
* @param pPoint0 => line start point
* @param pPoint1 => line end point
* @param xIndex => index of coordinate x,y,z,w to be treated as X
* @param yIndex => index of coordinate x,y,z,w to be treated as Y
* @param wIndex => index of coordinate x,y,z,w to be treated as W
* @param nearMissTol => fraction of radius.  A near-miss will be considered a
*                       hit at the nearest point of the ellipse if
*                       the nearest approach is within this distance
*                       of the ellipse.
* @return number of inter
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiDEllipse4d_intersectUnboundedLine2d

(
DPoint3dP pTrig,
DPoint3dP pLambda,
int           *pCount,
DEllipse4dCP pEllipse,
DPoint4dCP pPoint0,
DPoint4dCP pPoint1,
int           xIndex,
int           yIndex,
int           wIndex,
double        nearMissTol
)
    {
    RotMatrix B;
    DVec3d point[2];
    DVec3d cross;
    DPoint3d solutionPoint;
    double cosTheta[2], sinTheta[2];
    double   alpha, beta, gamma;
    double a00, a01, a11, b0, b1;
    double lambda0, lambda1;
    int i;
    int numOut, numSolution;
    StatusInt status = ERROR;
    *pCount = numOut = 0;

    bsiDEllipse4d_extract3x3 (&B, pEllipse, xIndex, yIndex, wIndex);
    bsiDPoint4d_selectDPoint3d (pPoint0, &point[0], xIndex, yIndex, wIndex);
    bsiDPoint4d_selectDPoint3d (pPoint1, &point[1], xIndex, yIndex, wIndex);
    cross.CrossProduct (point[0], point[1]);

    //alpha = cross.DotProduct (column2);
    //beta  = cross.DotProduct (column0);
    //gamma = cross.DotProduct (column1);

    alpha = cross.DotProduct (DVec3d::FromColumn (B, 2));
    beta  = cross.DotProduct (DVec3d::FromColumn (B, 0));
    gamma = cross.DotProduct (DVec3d::FromColumn (B, 1));

    numSolution = bsiMath_solveApproximateUnitQuadratic (
                            &cosTheta[0], &sinTheta[0],
                            &cosTheta[1], &sinTheta[1],
                            alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance);

    if (numSolution == 0 && nearMissTol > 0.0)

        {
        double d = sqrt (beta * beta + gamma * gamma);
        if (fabs (d - fabs (alpha)) < nearMissTol * d)
            {
            numSolution = 1;
            cosTheta[0] = beta / d;
            sinTheta[0] = gamma / d;
            if (alpha > 0.0)
                {
                cosTheta[0] = - cosTheta[0];
                sinTheta[0] = - sinTheta[0];
                }
            }
        }

    if (numSolution > 0)
        {
        /* Possible optimizations:
            1) Prefactor the matrix
            2) Use QR instead of normal equations
        */
        a00 = point[0].DotProduct (point[0]);
        a01 = point[0].DotProduct (point[1]);
        a11 = point[1].DotProduct (point[1]);

        for (i = 0; i < numSolution; i++)
            {
            bsiRotMatrix_multiplyComponents (&B, &solutionPoint, cosTheta[i], sinTheta[i], 1.0);
            b0 = point[0].DotProduct (solutionPoint);
            b1 = point[1].DotProduct (solutionPoint);
            if (bsiSVD_solve2x2 (&lambda0, &lambda1, a00, a01, a01, a11, b0, b1))
                {
                pTrig[numOut].x = cosTheta[i];
                pTrig[numOut].y = sinTheta[i];
                pTrig[numOut].z = bsiTrig_atan2 (sinTheta[i], cosTheta[i]);
                pLambda[numOut].x = lambda0;
                pLambda[numOut].y = lambda1;
                numOut++;
                }
            }
        *pCount = numOut;
        status = SUCCESS;
        }
    return status;
    }


/*----------------------------------------------------------------------+
| name           bsiIntegral_gauss                              |
| author        EarlinLutz                              07/96           |
| Integrate a scalar function of one variable over a finite interval,   |
| using an nPoint gauss rule, subdividing the given interval into       |
| subintervals of at most dxMax.                                        |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bsiIntegral_gauss

(
OmdlScalarFunction  pFunction,
double          x0,
double          x1,
double          dxMax,
int             numPoint,
void            *pUserData
)
    {
    static double phi[] =
        {
        -0.861136311594053, -0.339981043584856, 0.339981043584856, 0.861136311594053
        };

    static double weight[] =
        {
        0.347854845137454, 0.652145154862546, 0.652145154862546, 0.347854845137454
        };

    int interval, i, numInterval;
    double xMid, dx, dxFull, xEval, sum;

    if (numPoint != 4)
        numPoint = 4;

    if (dxMax <= 0.0)
        {
        numInterval = 1;
        }
    else
        {
        numInterval = (int)(fabs (x1 - x0) / dxMax);
        if (numInterval < 1)
            numInterval = 1;
        }

    dxFull = (x1 - x0) / (double)numInterval;       /* Size of interval mapped to -1..1 in gauss space */
    dx     = 0.5 * dxFull;                          /* scale factor between gauss space and real space */
    sum = 0.0;

    for (interval = 0, xMid = x0 + dx;
         interval < numInterval;
         interval++, xMid += dxFull)
        {
        for (i = 0; i < numPoint; i++)
            {
            xEval = xMid + dx * phi[i];
            sum += weight[i] * pFunction (pUserData, xEval);
            }
        }
    return sum * dx;
    }


/*-----------------------------------------------------------------*//**
* @nodoc
* @param pOutEllipse <= transformed ellipse
* @param pInEllipse => untransformed ellipse
* @param pMatrix => 4x4 matrix
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_multiplyByHMatrix

(
DEllipse4dP pOutEllipse,
DEllipse4dCP pInEllipse,
DMatrix4dCP pMatrix
)

    {
    if (pOutEllipse != pInEllipse)
        *pOutEllipse = *pInEllipse;
    bsiDMatrix4d_multiplyMatrixPoint (pMatrix, &pOutEllipse->center  , &pOutEllipse->center  );
    bsiDMatrix4d_multiplyMatrixPoint (pMatrix, &pOutEllipse->vector0 , &pOutEllipse->vector0 );
    bsiDMatrix4d_multiplyMatrixPoint (pMatrix, &pOutEllipse->vector90, &pOutEllipse->vector90);
    }

/*-----------------------------------------------------------------*//**
@description Define a cylinder by axial points and radius.  Define a line
by two points.   Compute the line parameters at which the line pierces the cylinder
@param pPointBuffer OUT 0, 1, or two points of intersection.
@param pCylinderFractionBuffer OUT 0, 1, or 2 fractional parameters where the points of
        intersection project to the cylinder axis.
@param pLineFractionBuffer OUT 0, 1, or 2 fractional parameters with respect to the line.
@param pCylinderPoint0 IN point on axis of cylinder (at parameter 0)
@param pCylinderPoint1 IN point on axis of cylinder (at paramter 1)
@param radius   IN cylinder radius.
@param pLinePoint0 IN point on line (at parameter 0)
@param pLinePoint1 IN point on line (at parameter 1)
@return number of intersections.

@remark If the cylinder points are identical the surface reduces to a sphere of given radius.
@remark If the line points are identical the line recuces to a point.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_intersectCircularCylinderLine

(
DPoint3dP pPointBuffer,
double *pCylinderFractionBuffer,
double *pLineFractionBuffer,
DPoint3dCP pCylinderPoint0,
DPoint3dCP pCylinderPoint1,
double radius,
DPoint3dCP pLinePoint0,
DPoint3dCP pLinePoint1
)
    {
    DPoint3d vectorU; // UNIT along cylinder axis
    DPoint3d vectorV; // cylinder origin to intersection point.
    DPoint3d vectorQ; // cylinder origin to line start
    DPoint3d vectorR; // UNIT along line
    int numIntersection = 0;
    double aa, bb, cc;
    double scaleU, scaleR;
    double lineDistance[2];
    int i;
    /* UNIT vectors along line and cylinder */
    /* If either is degenerate, the respective vector is zero ... */
    scaleU = vectorU.NormalizedDifference (*pCylinderPoint1, *pCylinderPoint0);
    scaleR = vectorR.NormalizedDifference (*pLinePoint1, *pLinePoint0);
    vectorQ.DifferenceOf (*pLinePoint0, *pCylinderPoint0);

    if (scaleR == 0.0)
        {
        /* The line is just a point. Find out if it is inside or outside the cylinder.
           This will fall through the output computation with lots of zeros.
        */
        DPoint3d vectorW;
        double r;
        vectorW.SumOf (vectorQ, vectorU, -bsiDPoint3d_dotProduct (&vectorU,&vectorQ));
        r = vectorW.Magnitude ();
        if (r <= radius)
            {
            numIntersection = 1;
            lineDistance[0] = 0.0;
            }
        }
    else
        {
        /*  Take the cylinder start as origin.
            U is UNIT vector along the cylinder axis, V is (non-unit) vector from origin to some space point.
            The vector from (the tip of) V onto the ray through U is
                W = V - U.V U
            V is on the cylinder of radius r around the ray along U if
                W.W == r^2
            which expands to
                (V - U.V U).(V - U.V U) == r^2
                V.V -2 (U.V)^2 +(U.V)^2 U.U = r^2
            Since U is unit,
                V.V - (U.V) ^2 = r^2
            Let V move parametrically along the line, i.e.
                V = Q + s R
            We want
                (Q + sR).(Q + sR) - (U.Q + s U.R)^2 = r^2

                   s^2 R.R     + 2s Q.R        + Q.Q - r^2
                -  s^2 (U.R)^2 - 2s U.R U.Q    - (U.Q)^2 = 0
            Since R is unit vector, R.R=1
            The solution values are real distances.
        */

        double UdotR = vectorU.DotProduct (vectorR);
        double QdotR = vectorQ.DotProduct (vectorR);
        double UdotQ = vectorU.DotProduct (vectorQ);
        double QdotQ = vectorQ.DotProduct (vectorQ);
        aa = 1.0 - UdotR * UdotR;
        bb = 2.0 * (QdotR - UdotR * UdotQ);
        cc = QdotQ - radius * radius - UdotQ * UdotQ;
        numIntersection = bsiMath_solveQuadratic (lineDistance, aa, bb, cc);

        if (numIntersection == 2 && lineDistance[1] < lineDistance[0])
            {
            double q = lineDistance[0];
            lineDistance[0] = lineDistance[1];
            lineDistance[1] = q;
            }
        }

    /* Expand the (parametric) intersections to output form. */
    for (i = 0; i < numIntersection; i++)
        {
        vectorV.SumOf (vectorQ, vectorR, lineDistance[i]);

        if (pLineFractionBuffer)
            bsiTrig_safeDivide (&pLineFractionBuffer[i], lineDistance[i], scaleR, 0.0);

        if (pPointBuffer)
            pPointBuffer[i].SumOf (*pCylinderPoint0, vectorV);

        if (pCylinderFractionBuffer)
            bsiTrig_safeDivide (&pCylinderFractionBuffer[i],
                    vectorV.DotProduct (vectorU), scaleU, 0.0
                    );
        }

    return numIntersection;
    }

/*-----------------------------------------------------------------*//**
@description Define a cylinder by axial points and radius.  Define a line segment
by two points.   Compute the line parameters at which the bounded line segment is within the CAPPED
cylinder (a solid).
@param pPointBuffer OUT 0 or 2 points of intersection.
@param pLineFractionBuffer OUT 0 or 2 fractional parameters with respect to the segment.
@param pCylinderPoint0 IN point on axis of cylinder
@param pCylinderPoint1 IN point on axis of cylinder
@param radius   IN cylinder radius.
@param pLinePoint0 IN point on line (at parameter 0)
@param pLinePoint1 IN point on line (at parameter 1)
@return true if there is an intersection.

* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_intersectCappedCylinderBoundedSegment

(
DPoint3dP pPointBuffer,
double *pLineFractionBuffer,
DPoint3dCP pCylinderPoint0,
DPoint3dCP pCylinderPoint1,
double radius,
DPoint3dCP pSegmentPoint0,
DPoint3dCP pSegmentPoint1
)
    {
    double param0[2];
    double param1[2];
    double param2[2];
    double r2 = radius * radius;
    DVec3d segmentVector, cylinderVector;
    double segmentLength, cylinderLength;
    double h0, h1, dh;
    int numCylinderInt;

    cylinderLength = cylinderVector.NormalizedDifference (*pCylinderPoint1, *pCylinderPoint0);
    segmentLength = segmentVector.NormalizedDifference (*pSegmentPoint1, *pSegmentPoint0);

    if (segmentLength == 0.0 || cylinderLength == 0.0)
        return false;


    /* Find altitudes of segment end above cylinder base plane */
    h0 = pSegmentPoint0->DotDifference(*pCylinderPoint0, cylinderVector);
    h1 = pSegmentPoint1->DotDifference(*pCylinderPoint0, cylinderVector);

    if (h0 < 0.0 && h1 < 0.0)
        return false;
    if (h0 > cylinderLength && h1 > cylinderLength)
        return false;
    dh = h1 - h0;



    numCylinderInt = bsiGeom_intersectCircularCylinderLine
                (
                NULL, NULL, param0,
                pCylinderPoint0, pCylinderPoint1, radius,
                pSegmentPoint0, pSegmentPoint1
                );

    /* Get limits of intersection with cylinder. param0[0] > param0[1] is no intersection */
    if (numCylinderInt == 0)
        {
        /* Segment is entirely in or entirely out */
        DPoint3d nearPoint;
        nearPoint.SumOf (*pCylinderPoint0, cylinderVector, h0);
        if (nearPoint.DistanceSquared (*pSegmentPoint0) <= r2)
            {
            param0[0] = 0.0;
            param0[1] = 1.0;
            }
        else
            {
            param0[0] = 10000.0;
            param0[1] = -10000.0;
            }
        }
    else if (numCylinderInt == 1)
        {
        param0[1] = param0[0];
        }
    else
        {
        if (param0[1] < param0[0])
            {
            double q = param0[0];
            param0[0] = param0[1];
            param0[1] = q;
            }
        }


    /* convert distance above cylinder base plane to parameter along line */
    if (    bsiTrig_safeDivide (&param1[0], -h0, dh, 0.0)
        &&  bsiTrig_safeDivide (&param1[1], cylinderLength - h0, dh, 0.0))
        {
        if (param1[1] < param1[0])
            {
            double q = param1[0];
            param1[0] = param1[1];
            param1[1] = q;
            }
        }
    else
        {
        /* the line must be perpendicular to cylinder.  We've already deemed it between planes,
        so call it all in. */
        param1[0] = 0.0;
        param1[1] = 1.0;
        }

    /* Restrict to smaller interval... */
    param2[0] = param0[0] > param1[0] ? param0[0] : param1[0];
    param2[1] = param0[1] < param1[1] ? param0[1] : param1[1];

    if (param2[1] > 1.0)
        param2[1] = 1.0;
    if (param2[0] < 0.0)
        param2[0] = 0.0;

    if (param2[0] <= param2[1])
        {
        int i;
        for (i = 0; i < 2; i++)
            {
            if (pPointBuffer)
                pPointBuffer[i].Interpolate (*pSegmentPoint0, param2[i], *pSegmentPoint1);
            if (pLineFractionBuffer)
                pLineFractionBuffer[i] = param2[i];
            }
        return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
@description Define a sphere by center and radius.  Define a line segment
by two points.   Compute the line parameters at which the bounded line segment is within
sphere.
@param pPointBuffer OUT 0 or 2 points of intersection.
@param pLineFractionBuffer OUT 0 or 2 fractional parameters with respect to the segment.
@param pSphereCenter IN center of sphere
@param radius   IN cylinder radius.
@param pLinePoint0 IN point on line (at parameter 0)
@param pLinePoint1 IN point on line (at parameter 1)
@return true if there is an intersection.

* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_intersectSphereBoundedSegment

(
DPoint3dP pPointBuffer,
double *pLineFractionBuffer,
DPoint3dCP pSphereCenter,
double radius,
DPoint3dCP pSegmentPoint0,
DPoint3dCP pSegmentPoint1
)
    {
    DPoint3d vectorQ, vectorR;
    double QdotQ, RdotQ, RdotR;
    double radiusSquared = radius * radius;
    double aa, bb, cc;
    int numIntersection, i;
    double param[2];
    vectorQ.DifferenceOf (*pSegmentPoint0, *pSphereCenter);
    vectorR.DifferenceOf (*pSegmentPoint1, *pSegmentPoint0);
    QdotQ = vectorQ.DotProduct (vectorQ);
    RdotQ = vectorR.DotProduct (vectorQ);
    RdotR = vectorR.DotProduct (vectorR);

    if (    QdotQ < radiusSquared
       &&  pSphereCenter->DistanceSquared (*pSegmentPoint1) < radiusSquared
       )
        {
        if (pPointBuffer)
            {
            pPointBuffer[0] = *pSegmentPoint0;
            pPointBuffer[1] = *pSegmentPoint1;
            }
        if (pLineFractionBuffer)
            {
            pLineFractionBuffer[0] = 0.0;
            pLineFractionBuffer[1] = 1.0;
            }
        return true;
        }
    else
        {
        /* Vector from sphere center to point on line at parameter s is
        **    Q + sR
        ** Squared vector length matches r^2 at
        **    (Q + sR) dot (Q + sR) = r^2
        **    R dot R s^2 + 2 Q dot R s + Q dot Q = r^2
        **    R dot R s^2 + 2 Q dot R s + (Q dot Q - r^2) = 0
        */
        aa = RdotR;
        bb = 2.0 * RdotQ;
        cc = QdotQ - radius * radius;
        numIntersection = bsiMath_solveQuadratic (param, aa, bb, cc);
        /* Ensure that there are ALWAYS two parameter values. */
        if (numIntersection == 2)
            {
            if (param[1] < param[0])
                {
                double q = param[0];
                param[0] = param[1];
                param[1] = q;
                }
            }
        else if (numIntersection == 1)
            {
            param[1] = param[0];
            }
        else if (numIntersection == 0)
            {
            param[0] = 1.0;
            param[1] = 0.0;
            }

        if (param[1] > 1.0)
            param[1] = 1.0;

        if (param[0] < 0.0)
            param[0] = 0.0;

        if (param[1] >= param[0])
            {
            for (i = 0; i < 2; i++)
                {
                if (pPointBuffer)
                    pPointBuffer[i].SumOf (*pSegmentPoint0, vectorR, param[i]);
                if (pLineFractionBuffer)
                    pLineFractionBuffer[i] = param[i];
                }
            return true;
            }
        return false;
        }
    }


/*-----------------------------------------------------------------*//**
@description Define a line segment by two points.
Compute the line parameters at which the bounded line segment is within a planar halfspace
Output points parameters are sorted by the parameter.
@param pPointBuffer OUT 0 or 2 points of intersection.
@param pLineFractionBuffer OUT 0 or 2 fractional parameters with respect to the segment.
@param pPlanePoint IN point on plane
@param pPlaneVector IN vector perpendicular to plane.
@param bPositiveHalfspace IN true to consider positive halfspace, false for negative.
@param pSegmentPoint0 IN segment start
@param pSegmentPoint1 IN segment end
@return true if there is an intersection.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_intersectPlanarHalfspaceBoundedSegment

(
DPoint3dP pPointBuffer,
double *pLineFractionBuffer,
DPoint3dCP pPlanePoint,
DPoint3dCP pPlaneVector,
bool    bPositiveHalfspace,
DPoint3dCP pSegmentPoint0,
DPoint3dCP pSegmentPoint1
)
    {
    double h0 = pSegmentPoint0->DotDifference(*pPlanePoint, *((DVec3d const*) pPlaneVector));
    double h1 = pSegmentPoint1->DotDifference(*pPlanePoint, *((DVec3d const*) pPlaneVector));
    if (bPositiveHalfspace)
        {
        h0 = -h0;
        h1 = -h1;
        }

    /* We have flipped to consider only the negative half space. */
    if (h0 * h1 >= 0.0)
        {
        /* No crossings to consider.  Let average altitude decide all. */
        double hMid = 0.5 * (h0 + h1);
        if (hMid <= 0.0)
            {
            if (pPointBuffer)
                {
                pPointBuffer[0] = *pSegmentPoint0;
                pPointBuffer[1] = *pSegmentPoint1;
                }
            if (pLineFractionBuffer)
                {
                pLineFractionBuffer[0] = 0.0;
                pLineFractionBuffer[1] = 1.0;
                }
            return true;
            }
        return false;
        }
    else
        {
        /* There is a clear crossing.  (sign difference assures no divide by 0) */
        double param = -h0 / (h1 - h0);
        int i0 = h0 <= 0.0 ? 0 : 1;
        int i1 = i0 + 1;
        if (pPointBuffer)
            {
            DPoint3d points[3];
            points[0] = *pSegmentPoint0;
            points[1].Interpolate (*pSegmentPoint0, param, *pSegmentPoint1);
            points[2] = *pSegmentPoint1;
            pPointBuffer[0] = points[i0];
            pPointBuffer[1] = points[i1];
            }
        if (pLineFractionBuffer)
            {
            double fractions[3];
            fractions[0] = 0.0;
            fractions[1] = param;
            fractions[2] = 1.0;
            pLineFractionBuffer[0] = fractions[i0];
            pLineFractionBuffer[1] = fractions[i1];
            }
        return true;
        }
    }

/*-----------------------------------------------------------------*//**
@description Expand a 1-D range to union of prior and new range.
@param pbInitialized IN OUT indicates if prior initialized param/point data is input
@param params IN OUT parameter range.  Assumed ordered.
@param newParams IN parameter range.  Assumed ordered.

* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void extendInterval

(
bool    *pbInitialized,
double  params[2],
double newParams[2]
)
    {
    if (!*pbInitialized)
        {
        params[0] = newParams[0];
        params[1] = newParams[1];
        *pbInitialized = true;
        }
    else
        {
        if (newParams[0] < params[0])
            params[0] = newParams[0];
        if (newParams[1] > params[1])
            params[1] = newParams[1];
        }
    }

/*-----------------------------------------------------------------*//**
@description intersect a 1-D range to union of prior and new range.
@param pbInitialized IN OUT indicates if prior initialized param/point data is input
@param params IN OUT parameter range.  Assumed ordered.
@param newParams IN parameter range.  Assumed ordered.

* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void intersectInterval

(
bool    *pbInitialized,
double  params[2],
double newParams[2]
)
    {
    if (!*pbInitialized)
        {
        params[0] = newParams[0];
        params[1] = newParams[1];
        *pbInitialized = true;
        }
    else
        {
        if (newParams[0] > params[0])
            params[0] = newParams[0];
        if (newParams[1] < params[1])
            params[1] = newParams[1];
        if (params[0] > params[1])
            *pbInitialized = false;
        }
    }

/*-----------------------------------------------------------------*//**
@description Define a cylinder by axial points and radius.  Define a line segment
by two points.   Compute the line parameters at which the bounded line segment is within the CAPPED
cylinder (a solid), optionally using planar or spherical caps at each end.
@param pPointBuffer OUT 0 or 2 points of intersection.
@param pLineFractionBuffer OUT 0 or 2 fractional parameters with respect to the segment.
@param pCylinderPoint0 IN point on axis of cylinder
@param bSphericalCap0 IN true for spherical (false planar) cap.at cylinder start
@param pCylinderPoint1 IN point on axis of cylinder
@param bSphericalCap1 IN true for spherical (false planar) cap.at cylinder end
@param radius   IN cylinder radius.
@param pLinePoint0 IN point on line (at parameter 0)
@param pLinePoint1 IN point on line (at parameter 1)
@return true if there is an intersection.

* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_intersectSphericalCappedCylinderBoundedSegment

(
DPoint3dP pPointBuffer,
double *pLineFractionBuffer,
DPoint3dCP pCylinderPoint0,
bool     bSphericalCap0,
DPoint3dCP pCylinderPoint1,
bool     bSphericalCap1,
double radius,
DPoint3dCP pSegmentPoint0,
DPoint3dCP pSegmentPoint1
)
    {
    bool    bInitialized;
    double caseParams[2], compositeParams[2];
    DPoint3d cylinderVector;
    cylinderVector.DifferenceOf (*pCylinderPoint1, *pCylinderPoint0);

    bInitialized = bsiGeom_intersectCappedCylinderBoundedSegment
                (NULL, compositeParams, pCylinderPoint0, pCylinderPoint1, radius, pSegmentPoint0, pSegmentPoint1);

    if (bSphericalCap0)
        {
        if (bsiGeom_intersectSphereBoundedSegment (NULL, caseParams, pCylinderPoint0,
                            radius, pSegmentPoint0, pSegmentPoint1))
            {
            extendInterval (&bInitialized, compositeParams, caseParams);
            if (!bSphericalCap1)
                {
                /* chop away parts of the start sphere outside the end plane */
                if (bsiGeom_intersectPlanarHalfspaceBoundedSegment (NULL, caseParams,
                                    pCylinderPoint1, &cylinderVector, false,
                                    pSegmentPoint0, pSegmentPoint1))
                    intersectInterval (&bInitialized, compositeParams, caseParams);
                }
            }
        }

    if (bSphericalCap1)
        {
        if (bsiGeom_intersectSphereBoundedSegment (NULL, caseParams, pCylinderPoint1,
                            radius, pSegmentPoint0, pSegmentPoint1))
            {
            extendInterval (&bInitialized, compositeParams, caseParams);
            if (!bSphericalCap0)
                {
                /* chop away parts of the end sphere outside the start plane */
                if (bsiGeom_intersectPlanarHalfspaceBoundedSegment (NULL, caseParams,
                                    pCylinderPoint0, &cylinderVector, true,
                                    pSegmentPoint0, pSegmentPoint1))
                    intersectInterval (&bInitialized, compositeParams, caseParams);
                }
            }
        }

    if (!bInitialized)
        return false;

    if (pLineFractionBuffer)
        {
        pLineFractionBuffer[0] = compositeParams[0];
        pLineFractionBuffer[1] = compositeParams[1];
        }

    if (pPointBuffer)
        {
        pPointBuffer[0].Interpolate (*pSegmentPoint0, compositeParams[0], *pSegmentPoint1);
        pPointBuffer[1].Interpolate (*pSegmentPoint0, compositeParams[1], *pSegmentPoint1);
        }
    return true;
    }



/*-----------------------------------------------------------------*//**
* Initialize as sum of (possibly zero) prior matrix and explicit
* coefficients of a quadratic form.
*
* @instance  <= quadric coefficients
* @param pAB0 => prior quadric coefficients.  If NULL, assumed 0
* @param cxx => xx coefficient
* @param cxy => xy coefficient
* @param cyy => yy coefficient
* @param cx => x coefficient
* @param cy => y coefficient
* @param c1 => 1 coefficient
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiQCoff_loadCoefficients

(
RotMatrixP pInstance,
RotMatrixP pAB0,
double      cxx,
double      cxy,
double      cyy,
double      cx,
double      cy,
double      c1
)
    {
    if (pAB0)

        {
        pInstance->form3d[0][0] = pAB0->form3d[0][0] + cxx;
        pInstance->form3d[1][1] = pAB0->form3d[1][1] + cyy;
        pInstance->form3d[2][2] = pAB0->form3d[2][2] + c1;
        pInstance->form3d[0][1] = pInstance->form3d[1][0] = pAB0->form3d[0][1] + 0.5 * cxy;
        pInstance->form3d[0][2] = pInstance->form3d[2][0] = pAB0->form3d[0][2] + 0.5 * cx;
        pInstance->form3d[1][2] = pInstance->form3d[2][1] = pAB0->form3d[1][2] + 0.5 * cy;
        }
    else
        {
        pInstance->form3d[0][0] = cxx;
        pInstance->form3d[1][1] = cyy;
        pInstance->form3d[2][2] = c1;
        pInstance->form3d[0][1] = pInstance->form3d[1][0] = 0.5 * cxy;
        pInstance->form3d[0][2] = pInstance->form3d[2][0] = 0.5 * cx;
        pInstance->form3d[1][2] = pInstance->form3d[2][1] = 0.5 * cy;
        }
    }



/*-----------------------------------------------------------------*//**
* Compute B so X'BX = X'AX and B is symmetric.

* @param pA <= symmetric coefficients
* @param pB => nonsymmetric coefficients
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiQCoff_symmetrize

(
RotMatrixP pA,
RotMatrixCP pB
)
    {
    double bxy = 0.5 * (pB->form3d[0][1] + pB->form3d[1][0]);
    double bxz = 0.5 * (pB->form3d[0][2] + pB->form3d[2][0]);
    double byz = 0.5 * (pB->form3d[2][1] + pB->form3d[1][2]);

    pA->form3d[0][0] = pB->form3d[0][0];
    pA->form3d[1][1] = pB->form3d[1][1];
    pA->form3d[2][2] = pB->form3d[2][2];

    pA->form3d[0][1] = pA->form3d[1][0] = bxy;
    pA->form3d[0][2] = pA->form3d[2][0] = bxz;
    pA->form3d[1][2] = pA->form3d[2][1] = byz;
    }




/*-----------------------------------------------------------------*//**
* Compute a matrix A such that
*   A*(c s 1)' = H * B where
*  H is the matrix
* [ 0 -1 -s][bx]        [c]
* [ 1  0  c][by] == A * [s]
* [ s -c  0][bz]        [1]

* @param pA <= coefficient matrix
* @param pVecB => vector
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiQCoff_HOperator

(
RotMatrixP   pA,
DPoint3dCP pVecB
)
    {
    pA->InitFromColumnVectors (
            DVec3d::From (0.0,      pVecB->z,   -pVecB->y),
            DVec3d::From (-pVecB->z,     0.0,    pVecB->x),
            DVec3d::From (-pVecB->y,   pVecB->x,     0.0)
            );
    }




/*-----------------------------------------------------------------*//**
* Compute the matrix of a quadric section whose intersections with
* the unit circle are the cosine and sine of the angles where pPoint
* projects to the quadric.
* That is,
*   A = sym(D*B'* (I - QW') * B)
* Where sym is the symmetrizing operator and  B, D, Q, and W are things
* that need some explanation.

* @param pA <= matrix of new quadric section
* @param pB => matrix of existing quadric section
* @param pPoint => point being projected to matrix B.
* @param pPoint
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiQCoff_projectToEllipse

(
RotMatrixP pA,
RotMatrixCP pB,
DPoint3dCP pPoint
)
    {
    RotMatrix D, DT;
    RotMatrix E;
    DVec3d BtW;
    RotMatrix BT;
    RotMatrix product;
    BT.TransposeOf (*pB);

    BtW = DVec3d::FromColumn (BT, 2);

    bsiQCoff_HOperator (&D, &BtW);
    DT.TransposeOf (D);
    E.InitIdentity ();

    DVec3d col2 = DVec3d::FromColumn (E, 2);
    col2.DifferenceOf (col2, *pPoint);
    E.SetColumn (col2, 2);

    product.InitProduct (DT, BT);
    product.InitProduct (product, E);
    product.InitProduct (product, *pB);
    bsiQCoff_symmetrize (pA, &product);
    }


/*-----------------------------------------------------------------*//**
* Extract the 2d and w parts of an DEllipse4d into a 3x3 matrix B such
* that the (homogeneous xyw) points of the ellipse are stroked out by
* X = B * F, where F is the "vector" whose components are the functions
* cos(theta), sin(theta), and 1.

* @param pB <= matrix for the ellipse.
* @param pHEllipse => Full 3d ellipse
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiQCoff_xyEllipseMatrix
(
RotMatrixP pB,
DEllipse4dCP pHEllipse
)
    {
    pB->SetColumn (DVec3d::From (pHEllipse->vector0.x, pHEllipse->vector0.y, pHEllipse->vector0.w), 0);
    pB->SetColumn (DVec3d::From (pHEllipse->vector90.x, pHEllipse->vector90.y, pHEllipse->vector90.w), 1);
    pB->SetColumn (DVec3d::From (pHEllipse->center.x, pHEllipse->center.y, pHEllipse->center.w), 2);
    }

/*-----------------------------------------------------------------*//**
* Extract the 2d and w parts of an DEllipse4d into a 3x3 matrix B such
* that the (homogeneous xyw) points of the ellipse are stroked out by
* X = B * F, where F is the "vector" whose components are the functions
* cos(theta), sin(theta), and 1.

* @param pB <= matrix for the ellipse.
* @param pHEllipse => Full 3d ellipse
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiQCoff_xyEllipseRotMatrix
(
RotMatrixP pB,
DEllipse4dCP pHEllipse
)
    {
    *pB = RotMatrix::FromColumnVectors (
            DVec3d::From (pHEllipse->vector0.x,  pHEllipse->vector0.y,  pHEllipse->vector0.w),
            DVec3d::From (pHEllipse->vector90.x, pHEllipse->vector90.y, pHEllipse->vector90.w),
            DVec3d::From (pHEllipse->center.x,   pHEllipse->center.y,   pHEllipse->center.w)
            );
    }


/*-----------------------------------------------------------------*//**
* Find the angles at which an unbounded ellipse comes close to a point, measuring in
* projected space.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_closeApproachUnbounded2d

(
DPoint3dP pTrigPoint,
int                 *pNumApproach,
DEllipse4dCP pHEllipse,
int                 xIndex,
int                 yIndex,
int                 wIndex,
DPoint4dP pTestPoint
)
    {
    RotMatrix       matrixA, matrixB;
    DPoint3d        projectedPoint;
    int             numProjection;
    double          cosTheta[4], sinTheta[4], theta[4];
    bool            myStat = false;
    int             i;


    bsiDEllipse4d_extract3x3  (&matrixB, pHEllipse, xIndex, yIndex, wIndex);
    bsiDPoint4d_selectDPoint3d (pTestPoint, &projectedPoint, xIndex, yIndex, wIndex);
    bsiQCoff_projectToEllipse (&matrixA, &matrixB, &projectedPoint);

    *pNumApproach = 0;

    if (SUCCESS == bsiBezier_implicitConicIntersectUnitCircle (cosTheta, sinTheta, theta, &numProjection, NULL, NULL, &matrixA))
        {
        myStat = true;
        for (i = 0; i < numProjection; i++)
            {
            pTrigPoint[i].Init ( cosTheta[i], sinTheta[i], theta[i]);
            }
        *pNumApproach = numProjection;
        }
    return myStat;
    }

/*-----------------------------------------------------------------*//**
* @param pIntersections         <= up to 4 intersection points
* @param pNumIntersections      <= number of intersections
* @param pA                     => first conic section
* @param pB                     => second conic section
* @return true if intersections computed normally.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiQCoff_intersectQuadricSections
(
DPoint2dP pIntersections,
int         *pNumIntersections,
RotMatrixCR matrixA,
RotMatrixCR matrixB
)
    {
    RotMatrix CC, QQCC;
    DPoint3d  lambdaCC;
    DPoint3d  ref0, ref1, lineCoff[2];
    int line;
    double tol = 1.0e-14;
    bool    result = false;
    int order[3];
    double size;
    int num = 0;
    DPoint2d coff[4];
    int degree;
    DPoint2d complexRoot[4];    /* THESE COME OUT SHIFTED BY 1*/
    *pNumIntersections = 0;
    DVec3d Ax = DVec3d::FromColumn (matrixA, 0);
    DVec3d Ay = DVec3d::FromColumn (matrixA, 1);
    DVec3d Az = DVec3d::FromColumn (matrixA, 2);

    DVec3d Bx = DVec3d::FromColumn (matrixB, 0);
    DVec3d By = DVec3d::FromColumn (matrixB, 1);
    DVec3d Bz = DVec3d::FromColumn (matrixB, 2);

    coff[0].x = bsiDPoint3d_tripleProduct (&Ax, &Ay, &Az);

    coff[1].x = bsiDPoint3d_tripleProduct (&Ax, &Ay, &Bz)
              + bsiDPoint3d_tripleProduct (&Ax, &By, &Az)
              + bsiDPoint3d_tripleProduct (&Bx, &Ay, &Az);

    coff[2].x = bsiDPoint3d_tripleProduct (&Ax, &By, &Bz)
              + bsiDPoint3d_tripleProduct (&Bx, &Ay, &Bz)
              + bsiDPoint3d_tripleProduct (&Bx, &By, &Az);

    coff[3].x = bsiDPoint3d_tripleProduct (&Bx, &By, &Bz);

    coff[0].y = coff[1].y = coff[2].y = coff[3].y = 0.0;
    degree = 3;

    if (SUCCESS == bsiSolve_polynomialEquation (complexRoot, coff, degree, true))
        {
        int root;
        double complexTol = 1.0e-14;
        double scale;
        result = false;
        for (root = 1, scale = 0.0; root <= degree; root++)
            scale += complexRoot[root].Magnitude ();
        scale = complexTol * scale;

        for (root = 1; !result && root <= degree; root++)
            {
            double realPart = complexRoot[root].x;
            double complexPart = complexRoot[root].y;
            if (fabs(complexPart) < scale)
                {
                bsiRotMatrix_add2ScaledRotMatrix (&CC, &matrixA, &matrixB, realPart, NULL, 0.0);
                bsiRotMatrix_symmetricEigensystem (&QQCC, &lambdaCC, &CC);
                bsiRotMatrix_sortEigensystem (&QQCC, &lambdaCC, order);
                size = fabs (lambdaCC.x) + fabs (lambdaCC.y) + fabs(lambdaCC.z);
                if (fabs (lambdaCC.y) <= tol * size && lambdaCC.x < 0.0 && lambdaCC.z > 0.0)
                    {
                    double ax = sqrt (-lambdaCC.x);
                    double az = sqrt (lambdaCC.z);
                    DPoint2d lineInt[2];
                    int numLineInt;
                    int i;
                    DVec3d xCol = DVec3d::FromColumn (QQCC, 0);
                    DVec3d yCol = DVec3d::FromColumn (QQCC, 1);
                    DVec3d zCol = DVec3d::FromColumn (QQCC, 2);
                    bsiDPoint3d_add2ScaledDPoint3d (&ref0, NULL, &xCol, az, &zCol, ax);
                    bsiDPoint3d_add2ScaledDPoint3d (&ref1, NULL, &xCol, az, &zCol, -ax);
                    lineCoff[0].CrossProduct (ref0, yCol);
                    lineCoff[1].CrossProduct (ref1, yCol);

                    num = 0;
                    for (line = 0; line < 2; line++)
                        {
                        if (SUCCESS == bsiQCoff_intersectLine (lineInt, &numLineInt,
                                            &lineCoff[line], &matrixB))
                            {
                            for (i = 0; i < numLineInt; i++)
                                {
                                pIntersections[num++] = lineInt[i];
                                }
                            }
                        }
                    if (num > 0)
                        {
                        *pNumIntersections = num;
                        result = true;
                        }
                    }
                }
            }
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* @param pLineInt * @param pNumLineInt * @param pLineCoff * @param pQCoff * @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiQCoff_intersectLine

(
DPoint2dP pLineInt,
int                 *pNumLineInt,
DPoint3dCP pLineCoff,
RotMatrixCP pQCoff
)
    {
    RotMatrix LL;
    RotMatrix LLT;
    RotMatrix LTAL;

    double ax = pLineCoff->x;
    double ay = pLineCoff->y;
    double aw = pLineCoff->z;

    double s2 = ax * ax + ay * ay;
    double x0, y0, tx, ty;
    double aa, bb, cc, alpha[2];
    int i;
    *pNumLineInt = 0;
    if (s2 == 0.0)
        return ERROR;
    x0 =  - aw * ax / s2;
    y0 =  - aw * ay / s2;

    tx = -ay;
    ty = ax;
    LL.InitFromRowValues (
                tx, 0, x0,
                ty, 0, y0,
                0,  0, 1);

    LTAL.InitProduct (*pQCoff, LL);
    LLT.TransposeOf (LL);
    LTAL.InitProduct (LLT, LTAL);

    aa = LTAL.GetComponentByRowAndColumn(0,0);
    bb = 2.0 * LTAL.GetComponentByRowAndColumn (0,2);
    cc = LTAL.GetComponentByRowAndColumn (2, 2);
    *pNumLineInt = bsiMath_solveQuadratic (alpha, aa, bb, cc);

    for (i = 0; i < *pNumLineInt; i++)
        {
        pLineInt[i].x = x0 + alpha[i]* tx;
        pLineInt[i].y = y0 + alpha[i]* ty;
        }
    return SUCCESS;
    }



/*-----------------------------------------------------------------*//**
* Find the angles (on a conic) where the conic is
* tangent to a circle about the origin and through the conic point.
* (Not necessarily unit circle, but useful for seeking near approach
*   to unit circle.)
* This is equivalent to projecting the origin to the conic.
*
* @param pCosValue <= 0 to 4 cosine values
* @param pSinValue <= 0 to 4 sine values
* @param pThetaValue <= 0 to 4 angle values
* @param pNumInt <= number of tangencies
* @param centerx
* @param ux
* @param vx
* @param centery
* @param uy
* @param vy
* @param cenerw
* @param uw
* @param vw
* @return -1 if conic is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiMath_conicTangentUnitCircle

(
double          *pCosValue,
double          *pSinValue,
double          *pThetaValue,
int             *pNumInt,
double          centerx,
double          ux,
double          vx,
double          centery,
double          uy,
double          vy,
double          centerw,
double          uw,
double          vw
)
    {
    RotMatrix B;
    RotMatrix A;
    DPoint3d origin;
    B.form3d[0][0] = ux;
    B.form3d[1][0] = uy;
    B.form3d[2][0] = uw;

    B.form3d[0][1] = vx;
    B.form3d[1][1] = vy;
    B.form3d[2][1] = vw;

    B.form3d[0][2] = centerx;
    B.form3d[1][2] = centery;
    B.form3d[2][2] = centerw;
    origin.Zero ();
    bsiQCoff_projectToEllipse (&A, &B, &origin);
    return bsiBezier_implicitConicIntersectUnitCircle
                    (pCosValue, pSinValue, pThetaValue, pNumInt,
                    NULL, NULL, &A);
    }

/* MAP implicitConicIsUnitCircle=Geom.conicIntersectUnitCircle ENDMAP */

/*-----------------------------------------------------------------*//**
* This routine will find the intersection between a general conic
* and a unit circle. The conic is in the form of:
* x = centerx + ux * cos(theta) + vx*sin(theta)
* y = centery + uy * cos(theta) + vy*sin(theta)
* w = centerw + uw * cos(theta) + vw*sin(theta)
*   where centerx, centery, centerw, ux, uy, uw, vx, vy, vw are constants
*   and    PI < = theta < = PI
* A unit circle is x^2 + Y^2 = 1
* Return values: number of solutions found.
*               0: no intersection
*               -1: input error or polynomial solver failed.
*
* @param pCosValue <= 0 to 4 cosine values
* @param pSinValue <= 0 to 4 sine values
* @param pThetaValue <= 0 to 4 angle values
* @param pNumInt <= number of intersections
* @param centerx
* @param ux
* @param vx
* @param centery
* @param uy
* @param vy
* @param cenerw
* @param uw
* @param vw
* @return -1 if the conic is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiMath_conicIntersectUnitCircle

(
double          *pCosValue,
double          *pSinValue,
double          *pThetaValue,
int             *pNumInt,
double          centerx,
double          ux,
double          vx,
double          centery,
double          uy,
double          vy,
double          centerw,
double          uw,
double          vw
)
    {
    RotMatrix B;
    B.form3d[0][0] = ux;
    B.form3d[1][0] = uy;
    B.form3d[2][0] = uw;

    B.form3d[0][1] = vx;
    B.form3d[1][1] = vy;
    B.form3d[2][1] = vw;

    B.form3d[0][2] = centerx;
    B.form3d[1][2] = centery;
    B.form3d[2][2] = centerw;
    return bsiBezier_conicIntersectUnitCircle (pCosValue, pSinValue, pThetaValue, pNumInt,
                                    NULL, NULL, &B);
    }


/*-----------------------------------------------------------------*//**
* This routine finds the points of intersection between an implicit
* conic (specified by matrix A) X^AX = 0  and the unit circle
* x^2 + Y^2 = 1
* Returns  : number of intersections found.
*            -1: conic = circle or input error or polynomial solver failed.
*
* @param pCosValue <= x coordinates of intersections
* @param pSinValue <= y coordinates of intersections
* @param pThetaValue <= angular positions of intersections
* @param pNumInt <= number of intersections
* @param pCoefficientMatrix => matrix defining implicit conic
* @return 0 if success, nonzero if error
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bsiMath_implicitConicIntersectUnitCircle

(
double          *pCosValue,
double    *pSinValue,
double    *pThetaValue,
int       *pNumInt,
RotMatrixCP pCoefficientMatrix
)
    {
    return 0 == bsiBezier_implicitConicIntersectUnitCircle
                (pCosValue, pSinValue, pThetaValue, pNumInt, NULL, NULL, pCoefficientMatrix)
        ? ERROR : SUCCESS;
    }



// Let U0, U1, U2 be vectors from a common origin.
// A point is placed on each vector.
// P0 is        P0 = origin + s0 * U0
// P1 is        P1 = origin + s1 * U1
// P2 is        P2 = origin + s2 * U2
// For this analysis, assume U0, U1, U2 are UNIT vectors.
// (Implementation will scale as needed to accomodate non-unit vectors.)
//
// We want to find s0,s1,s2 so the triangle P0,P1,P2 has edge lengths PROPORTIONAL TO a01, a12, a02
// Distances (squared) are
//        dSquared (Pi,Pj)= (si*Ui - sj*Uj) DOT (si*Ui-sj*Uj)=si^2 Ui DOT Ui -2 si sj Ui Dot Uj + sj^2 Uj DOT Uj
// Note Ui DOT Ui = 1 always.
// Let s0 be 1 always -- when this solution is complete s0,s1,s2 can be scaled as needed
//    to achieve actual distances.
//
//   dsquared (P0,P1) = 1 - 2*U0 Dot U1 * s1 + s1^2
//   dsquared (P0,P2) = 1 - 2*U0 Dot U2 * s2 + s2^2
//   dsquared (P1,P2) = s1^2 - 2*U1 Dot U2 * s1 * s2 + s2^2
//
// We want the (squared) distances to be in ratios of a triangle whose edge lengths are (a01 :: a12 :: a02)
//   Let E = (a01/a12)^2
//       F = (a02/a12)^2
//       c01 = U0 DOT U1
//       c02 = U0 DOT U2
//       c12 = U1 DOT U2
//       
//  dsquared (P0,P1) = q1^2 dsquared(P1,P2)
// 1 - 2 c01 * s1 + s1^2 = E (s1^2 - 2 c12 * s1 * s2 + s2^2)
//  dsquared (P0,P2) = q2^2 dsquared(P1,P2)
// 1 - 2 c02 * s2 + s2^2 = F (s1^2 - 2 c12 * s1 * s2 + s2^2)
// Each of these is a CONIC SECTION in the s1,s2 plane.
// The 0 to 4 intersections are potential solutions. (Maybe some bogus ones due to squaring?)
// Assemble each as a 3x3 matrix quadratic form:
// [s1 s2 1] [1-E   E*c12 -c01][s1]  = 0
//           [E*c12  -E   0][s2]
//           [-c01    0     1][1]
// and
// [s1 s2 1] [-F   F*c12   0][s1]  = 0
//           [F*c12  1-F -c02][s2]
//           [0     -c02    1][1]
bool bsiGeom_matchTriangle
(
DVec3dCR U0,        // Vector along frustum edge 0.  (NOT required to be unit)
DVec3dCR U1,        // Vector along frustum edge 1.  (NOT required to be unit)
DVec3dCR U2,        // Vector along frustum edge 2.  (NOT required to be unit)
double a01Squared,  // sqaured length of target triangle along frustum face 01
double a12Squared,  // sqaured length of target triangle along frustum face 12
double a02Squared,  // sqaured length of target triangle along frustum face 02
bool restrictToPositiveSolutions,    // If TRUE, only solutions with positive s0,s1,s2 are returned.
DVec3d* solutions,  // Caller must allocate for at least 4 solutions (s0,s1,s2).
                    // The returned DVec3d have x,y,z as the respective mulitpliers for U0, U1, U2.
                    // Hence the actual solution triangle for solution [i] is the points
                    //      origin + U0 * solutions[i].x
                    //      origin + U1 * solutions[i].y
                    //      origin + U2 * solutions[i].z
int &numSolution
)
    {
    numSolution = 0;
    double E, F;
    DVec3d unitU0, unitU1, unitU2;
    double scale0 = unitU0.Normalize (U0);
    double scale1 = unitU1.Normalize (U1);
    double scale2 = unitU2.Normalize (U2);
    double descale0, descale1, descale2;
    if (   !bsiTrig_safeDivide (&E, a01Squared, a12Squared, 0.0)
        || !bsiTrig_safeDivide (&F, a02Squared, a12Squared, 0.0)
        || !bsiTrig_safeDivide (&descale0, 1.0, scale0, 0.0)
        || !bsiTrig_safeDivide (&descale1, 1.0, scale1, 0.0)
        || !bsiTrig_safeDivide (&descale2, 1.0, scale2, 0.0)
        )
        return false;
    double c01 = unitU0.DotProduct (unitU1);
    double c02 = unitU0.DotProduct (unitU2);
    double c12 = unitU1.DotProduct (unitU2);
    RotMatrix M01, M02;
    M01.InitFromRowValues
        (
        1-E,   E * c12, -c01,
        E*c12, -E,      0.0,
        -c01,  0.0,      1.0
        );

    M02.InitFromRowValues
        (
        -F,    F * c12, 0.0,
        F*c12, 1.0-F,   -c02,
        0.0,   -c02,     1.0
        );

    DPoint2d uv[4];
    int numQuadricSolution;
    if (!bsiQCoff_intersectQuadricSections (uv, &numQuadricSolution, M01, M02))
        return false;
    DVec3d Q0, Q1, Q2;
    double d12Squared, d01Squared, d02Squared;
    double placementFactor;
    for (int i = 0; i < numQuadricSolution; i++)
        {
#define CHECK_INTERSECTION_RESULTS        
#ifdef CHECK_INTERSECTION_RESULTS
        DVec3d uvw;
        uvw.Init (uv[i].x, uv[i].y, 1.0);
        double q01, q02;
        q01 = bsiRotMatrix_quadraticForm (&uvw, &M01, &uvw);
        q02 = bsiRotMatrix_quadraticForm (&uvw, &M02, &uvw);
#endif
        if (restrictToPositiveSolutions &&
            (uv[i].x <= 0.0 || uv[i].y <= 0.0))
            continue;
            
        Q0 = unitU0;
        Q1.Scale (unitU1, uv[i].x);
        Q2.Scale (unitU2, uv[i].y);
        d01Squared = Q0.DistanceSquared (Q1);
        d12Squared = Q1.DistanceSquared (Q2);
        d02Squared = Q0.DistanceSquared (Q2);
        if (bsiTrig_safeDivide (&placementFactor, sqrt (a12Squared), sqrt (d12Squared), 0.0))
            {
            solutions[numSolution++].Init (placementFactor * descale0, placementFactor * descale1 * uv[i].x, placementFactor * descale2 * uv[i].y);
            }
        }
    return true;
    }


END_BENTLEY_GEOMETRY_NAMESPACE

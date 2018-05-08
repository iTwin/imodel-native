/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_lastUsedFunctions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param pPlane <= initialized plane.
* @param pOrigin => origin point
* @param pNormal => normal vector
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_initFromOriginAndNormal

(
DPlane3dP pPlane,
DPoint3dCP pOrigin,
DVec3dCP pNormal
)
    {
    pPlane->origin = *pOrigin;
    pPlane->normal = *pNormal;
    }

/*-----------------------------------------------------------------*//**
* @description Return the plane as a DPoint4d.
* @param pPlane => plane structure with origin, normal
* @param pHPlane <= 4D plane coefficients
* @see bsiDPlane3d_initFromDPoint4d
* @group "DPlane3d Queries"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiDPlane3d_getDPoint4d

(
DPlane3dCP pPlane,
DPoint4dP pHPlane
)
    {
    bsiDPlane3d_getImplicitPlaneCoefficients
                            (
                            pPlane,
                            &pHPlane->x,
                            &pHPlane->y,
                            &pHPlane->z,
                            &pHPlane->w
                            );
    pHPlane->w = -pHPlane->w;
    }


/*-----------------------------------------------------------------*//**
* @description Convert the plane to implicit coeffcients ax+by+cz=d.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param pPlane => plane structure with origin, normal
* @param pA <= 4D plane x-coefficient
* @param pB <= 4D plane y-coefficient
* @param pC <= 4D plane z-coefficient
* @param pD <= 4D plane constant coefficient
* @see bsiDPlane3d_initFromImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_getImplicitPlaneCoefficients

(
DPlane3dCP pPlane,
double      *pA,
double      *pB,
double      *pC,
double      *pD
)
    {
    *pA = pPlane->normal.x;
    *pB = pPlane->normal.y;
    *pC = pPlane->normal.z;
    *pD = bsiDPoint3d_dotProduct (&pPlane->origin, &pPlane->normal);
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


/*-----------------------------------------------------------------*//**
* Search an array for the closest point, using only
* x and y components.  Useful for screenproximity
* tests between points at different depths.
*
* @param pPoint => fixed point for tests
* @param pArray => array of test points
* @param nPoint => number of points
* @see
* @return index of closest point
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_closestXYDPoint4d


(
DPoint3dCP pPoint,
DPoint4dCP pArray,
int             nPoint
)

    {
    int     iCurr, iMin;
    double  dCurr, dMin;

    iMin = -1;
    dMin = DBL_MAX;
    for (iCurr = 0; iCurr < nPoint; iCurr++)
        {
        if (bsiDPoint4d_realDistanceSquaredXY (&pArray[iCurr], &dCurr, pPoint)
            && (iMin == -1 || dCurr < dMin))
            {
            dMin = dCurr;
            iMin = iCurr;
            }
        }

    return iMin;
    }

/*----------------------------------------------------------------------------+
| BARYCENTRIC COORDINATE FUNCTIONS:
|
| For a given triangle T with vertices v0, v1, v2, every point q in the plane
| of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
| relative to T:
|
| q = b0 * v0 + b1 * v1 + b2 * v2,
| 1 = b0 + b1 + b2.
|
+----------------------------------------------------------------------------*/


static bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
double    *pArea,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    bool        status = true;
    double      denom, numer1, numer2;

    /*
    Let vectors v1 and v2 emanating from the plane's origin define triangle T,
    and let q be a point in the plane.  Then the system for finding the
    barycentric coordinates of q relative to T reduces from 3x3 to 2x2:
      -             - -    -   -     -      -            - -    -   -     -
      | 0 v1.x v2.x | | b0 |   | q.x |      | v1.x  v2.x | | b1 |   | q.x |
      | 0 v1.y v2.y | | b1 | = | q.y |  =>  | v1.y  v2.y | | b2 | = | q.y |
      | 1    1    1 | | b2 |   | 1   |      -            - -    -   -     -
      -             - -    -   -     -
    We use Cramer's Rule to solve this system for b1 and b2; b0 can be found
    by subtracting the sum of the other two coords from 1.0.
    */

    /* Calculate numerators of Cramer's Rule formulae */
    if (pOrigin)
        {
        /*
        Since barycoords are invariant under affine transformation, we can
        translate the triangle and point so that pOrigin is the origin.  This
        gives us the dimension reduction detailed above.
        */
        numer1 = (pPoint->x - pOrigin->x) * pVector2->y -
                 (pPoint->y - pOrigin->y) * pVector2->x;
        numer2 = (pPoint->y - pOrigin->y) * pVector1->x -
                 (pPoint->x - pOrigin->x) * pVector1->y;
        }
    else
        {
        numer1 = pPoint->x * pVector2->y - pPoint->y * pVector2->x;
        numer2 = pPoint->y * pVector1->x - pPoint->x * pVector1->y;
        }

    /*
    Calculate denominator of Cramer's Rule formulae.  On a good day, denom is
    twice the signed area of T.  On a bad day (i.e. when T is long and skinny)
    we get subtractive cancellation, but there's no way around it!
    */
    denom  = pVector1->x * pVector2->y - pVector2->x * pVector1->y;

    /* Return false and barycoords (1,0,0) if denom relatively close to zero */
    if (! bsiTrig_safeDivide (&pInstance->y, numer1, denom, 0.0))
        status = false;

    if (! bsiTrig_safeDivide (&pInstance->z, numer2, denom, 0.0))
        status = false;

    pInstance->x = 1.0 - pInstance->y - pInstance->z;

    if (pArea)
        *pArea = 2.0 * denom;
    return status;
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangle

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, NULL, &q, NULL, &v1, &v2);
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangle
(
DPoint3dR uvw,
DPoint3dR dUVWdX,
DPoint3dR dUVWdY,
double   &area,
DPoint2dCR point,
DPoint2dCR vertex0,
DPoint2dCR vertex1,
DPoint2dCR vertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = point.x   - vertex0.x;
    q.y  = point.y   - vertex0.y;
    v1.x = vertex1.x - vertex0.x;
    v1.y = vertex1.y - vertex0.y;
    v2.x = vertex2.x - vertex0.x;
    v2.y = vertex2.y - vertex0.y;
    double divArea;
    if (bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (&uvw, &area, &q, NULL, &v1, &v2)
        && DoubleOps::SafeDivide (divArea, 2.0, area, 0.0))
        {
        
        DVec2d edgeVector[3];
        edgeVector[0].DifferenceOf (vertex2, vertex1);  // vector opposite point0
        edgeVector[1].DifferenceOf (vertex0, vertex2);  // vector opposite point1
        edgeVector[2].DifferenceOf (vertex1, vertex0);  // vector opposite point2
        dUVWdX.Init (-edgeVector[0].y * divArea, -edgeVector[1].y * divArea, -edgeVector[2].y * divArea);
        dUVWdY.Init ( edgeVector[0].x * divArea,  edgeVector[1].x * divArea, edgeVector[2].x * divArea);
        return true;
        }
    uvw.Zero ();
    dUVWdX.Zero ();
    dUVWdY.Zero ();
    area = 0.0;
    return false;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the minimum distance from a point to a triangle.
* @instance pSpacePoint   <= point in space
* @param pVertex0       => vertex of T
* @param pVertex1       => vertex of T
* @param pVertex2       => vertex of T
* @param pClosePoint    <= projection of space point onto plane of triangle
* @param pBCoords       <= barycentric coordinates of closest point
* @return minimum distance
* @group "DPoint3d Barycentric"
* @bsihdr                                       EarlinLutz      10/04
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_minDistToTriangle

(
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pClosePoint,
DPoint3dP pBoundedUVW,
DPoint3dP pUnboundedUVW
)
    {
    DPoint3d planePoint;
    double s, t;
    DPoint3d xyz[3];
    static DPoint3d uvwCorner[3] =
        {
            {1,0,0},
            {0,1,0},
            {0,0,1}
        };

    /* Ugh.  Compute for each edge independently.  */
    DSegment3d testSeg[3];
    DPoint3d   testPoint[3];
    double     testParam[3];
    double     testDistanceSquared[3];

    int i, iMin, jMin;
    xyz[0] = *pVertex0;
    xyz[1] = *pVertex1;
    xyz[2] = *pVertex2;
    if (pUnboundedUVW)
        pUnboundedUVW->Zero ();
    
    if (    bsiGeom_closestPointOnSkewedPlane (&planePoint, &s, &t, xyz, pSpacePoint))
        {
        if (pUnboundedUVW)
            bsiDPoint3d_setXYZ (pUnboundedUVW, 1.0 - s - t, s, t);
        if (s >= 0.0
        &&  t >= 0.0
        &&  s + t <= 1.0)
            {
            if (pBoundedUVW)
                bsiDPoint3d_setXYZ (pBoundedUVW, 1.0 - s - t, s, t);
            if (pClosePoint)
                *pClosePoint = planePoint;
            return bsiDPoint3d_distance (&planePoint, pSpacePoint);
            }
        }

    bsiDSegment3d_initFromDPoint3d (&testSeg[0], pVertex0, pVertex1);
    bsiDSegment3d_initFromDPoint3d (&testSeg[1], pVertex1, pVertex2);
    bsiDSegment3d_initFromDPoint3d (&testSeg[2], pVertex2, pVertex0);

    for (i = 0; i < 3; i++)
        {
        bsiDSegment3d_projectPointBounded (&testSeg[i], &testPoint[i], &testParam[i], pSpacePoint);
        testDistanceSquared[i] = bsiDPoint3d_distanceSquared (&testPoint[i], pSpacePoint);
        }
    iMin = 0;
    if (testDistanceSquared[1] < testDistanceSquared[0])
        iMin = 1;
    if (testDistanceSquared[2] < testDistanceSquared[iMin])
        iMin = 2;

    jMin = iMin + 1;
    if (jMin == 3)
        jMin = 0;

    if (pClosePoint)
        *pClosePoint = testPoint[iMin];
    if (pBoundedUVW)
        bsiDPoint3d_interpolate (pBoundedUVW, &uvwCorner[iMin], testParam[iMin], &uvwCorner[jMin]);
    return sqrt (testDistanceSquared[iMin]);
    }

/*-----------------------------------------------------------------*//**
* @description Applies transformation to simplify the problem of finding the barycentric
* coordinates of a 3d point relative to a triangle.  Returned are the
* components of the new 2d problem.
*
* @instance pInstance   => point to find barycentric coords of
* @param pNewPoint      <= point in plane of new triangle with same barycoords
* @param pNewVector1    <= side of new triangle
* @param pNewVector2    <= side of new triangle
* @param pVector1       => side of old triangle
* @param pVector2       => side of old triangle
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
static void transformBarycentric3dSystemTo2d

(
DPoint3dCP pInstance,
DPoint2dP pNewPoint,
DPoint2dP pNewVector1,
DPoint2dP pNewVector2,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    /*
    Projecting the 3D point q onto the plane spanned by 3D vectors v1, v2
    (which form triangle T) is a least squares problem:

      [v1 v2] [b1 b2]^ = q

    The resulting normal equations (below) determine the barycoords b1, b2
    (corresponding to v1, v2) of the projection relative to T:
      -              - -    -   -      -
      | v1.v1  v1.v2 | | b1 | = | q.v1 |
      | v1.v2  v2.v2 | | b2 |   | q.v2 |
      -              - -    -   -      -

    or equivalently, [newVector1 newVector2] [b1 b2]^ = newPoint.

    This latter form shows that the 3D problem reduces to a 2D problem:
    finding the barycentric coordinates of newPoint relative to the
    triangle in the xy-plane spanned by vectors newVector1, newVector2.
    */
    pNewVector1->x =    pVector1->x * pVector1->x +
                        pVector1->y * pVector1->y +
                        pVector1->z * pVector1->z;
    pNewVector1->y =
    pNewVector2->x =    pVector1->x * pVector2->x +
                        pVector1->y * pVector2->y +
                        pVector1->z * pVector2->z;

    pNewVector2->y =    pVector2->x * pVector2->x +
                        pVector2->y * pVector2->y +
                        pVector2->z * pVector2->z;

    pNewPoint->x =     pInstance->x * pVector1->x +
                       pInstance->y * pVector1->y +
                       pInstance->z * pVector1->z;

    pNewPoint->y =     pInstance->x * pVector2->x +
                       pInstance->y * pVector2->y +
                       pInstance->z * pVector2->z;
    }



/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in space.
* Points p and r in space have the same barycentric coordinates relative to
* T if and only if they project to the same point q in the plane of T;
* then their barycentric coordinates relative to T are those of q.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in space
* @param pVertex0       => vertex of triangle T
* @param pVertex1       => vertex of triangle T
* @param pVertex2       => vertex of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint3d_fromBarycentricAndDPoint3dTriangle
* @return true if and only if the area of T is sufficiently large.
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint3dTriangle

(
DPoint3dP pInstance,
DPoint3dCP pPoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d        q, v1, v2;
    DPoint2d        newPoint, newV1, newV2;

    /*
    Translating by pVertex0 compresses the triangle definition from 3 points
    to 2 vectors while preserving barycentric coords.
    */
    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    q.z  = pPoint->z   - pVertex0->z;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v1.z = pVertex1->z - pVertex0->z;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;
    v2.z = pVertex2->z - pVertex0->z;

    /* decrement dimension of problem */
    transformBarycentric3dSystemTo2d (&q, &newPoint, &newV1, &newV2, &v1, &v2);

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, &newPoint, NULL, &newV1, &newV2);
    }
/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pOrigin, pVector1-pOrigin, pVector2-pOrigin)
* in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pArea          <= area of triangle.
* @param pPoint         => point in plane
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangleVectors
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                      DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors (pInstance, NULL, pPoint, pOrigin,
                    pVector1, pVector2);
    }
/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane with the given barycentric
* coordinates relative to triangle T (pVertex0, pVertex1, pVertex2).
*
* @instance pInstance   <= point with given barycoords relative to T
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @group "DPoint2d Barycentric"
* @bsihdr                                                               DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_fromBarycentricAndDPoint2dTriangle

(
DPoint2dP pInstance,
DPoint3dCP pBaryCoords,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    pInstance->x =  pBaryCoords->x * pVertex0->x +
                    pBaryCoords->y * pVertex1->x +
                    pBaryCoords->z * pVertex2->x;
    pInstance->y =  pBaryCoords->x * pVertex0->y +
                    pBaryCoords->y * pVertex1->y +
                    pBaryCoords->z * pVertex2->y;
    }

/*-----------------------------------------------------------------*//**
* @description Dot the plane normal with the vector from the plane origin to the point.
* @remarks If the plane normal is a unit vector, this is the true distance from the
*       plane to the point.  If not, it is a scaled distance.
* @param pPlane => plane to evaluate
* @param pPoint => point for evaluation
* @return dot product
* @group "DPlane3d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double  bsiDPlane3d_evaluate

(
DPlane3dCP pPlane,
DPoint3dCP pPoint
)
    {
    return bsiDPoint3d_dotDifference (pPoint, &pPlane->origin, &pPlane->normal);
    }
/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint3d
(
GraphicsPointArrayP pInstance,
const DPoint3d              *pPoint
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_init (&element,
                    pPoint->x, pPoint->y, pPoint->z, 1.0,
                    0.0, HPOINT_NORMAL, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Mark the break between disconnected line segments.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_markBreak
(
GraphicsPointArrayP         pInstance
)
    {
    if (pInstance && pInstance->vbArray_hdr.size () > 0)
        pInstance->vbArray_hdr.back ().mask |= HPOINT_MASK_BREAK;
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
* @bsihdr                                                       EarlinLutz      12/97
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
* @bsihdr                                                       EarlinLutz      12/97
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
        bsiRotMatrix_initIdentity (pMatrix);
        if (pInverse)
            bsiRotMatrix_initIdentity (pInverse);
        return false;
        }
    else if (mu == wc2)     /* Yes, exact equality test -- if wc2 is small the squaring will */
                            /* wipe force its bits so far to the right they have no effect on the subtraction*/
        {
        /* It's already practically a circle.*/
        bsiRotMatrix_initIdentity (pMatrix);
        if (pInverse)
            bsiRotMatrix_initIdentity (pInverse);
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
            bsiRotMatrix_invertRotMatrix (pInverse, pMatrix);
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
* @bsihdr                                                       EarlinLutz      12/97
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
    bsiRotMatrix_multiplyComponents (pMatrix, &G, cosTheta, sinTheta, wF);
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
* @bsihdr                                                       EarlinLutz      12/97
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
        if (bsiTrig_isAngleFullCircle (theta1 - theta0))
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
* @bsihdr                                                       EarlinLutz      12/97
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

        bsiDPoint4d_add2ScaledDPoint4d (&vector0,  NULL, &pWeighted->vector0,  recip, &pWeighted->center, - alpha * recip);
        bsiDPoint4d_add2ScaledDPoint4d (&vector90, NULL, &pWeighted->vector90, recip, &pWeighted->center, - beta * recip);
        bsiDPoint4d_scale               (&center,   &pWeighted->center,   recip);

        /* The transfer matrix is of the form*/
        /*          [ rxx  rxy  cx]*/
        /*          [ ryx  ryy  cy]*/
        /*          [  0    0    1]*/
        bsiDPoint4d_add2ScaledDPoint4d (&pNormalized->vector0,
                                            NULL,
                                            &vector0, transferMatrix.form3d[0][0],
                                            &vector90, transferMatrix.form3d[1][0]);
        bsiDPoint4d_add2ScaledDPoint4d (&pNormalized->vector90,
                                            NULL,
                                            &vector0, transferMatrix.form3d[0][1],
                                            &vector90, transferMatrix.form3d[1][1]);
        bsiDPoint4d_add2ScaledDPoint4d (&pNormalized->center,
                                            &center,
                                            &vector0, transferMatrix.form3d[0][2],
                                            &vector90, transferMatrix.form3d[1][2]);
        bsiDEllipse4d_transferAngles( pNormalized, pWeighted, &inverseTransferMatrix, w0 / wCenter, w90 / wCenter);
        }
    else
        {
        *pNormalized = *pWeighted;
        }
    return boolStat;
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
* @bsihdr                                       EarlinLutz      12/97
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
        bsiTransform_initFrom4Points (&axisTransform,
                        &pEllipsoidPoint[0],
                        &pEllipsoidPoint[1],
                        &pEllipsoidPoint[2],
                        &pEllipsoidPoint[3]);
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

            bsiDMatrix4d_transpose (&QT, &Q);
            bsiDMatrix4d_multiply (&BQT, &BMap.M0, &QT);

            bsiDMatrix4d_getColumnDPoint4d (&BQT, &pHEllipse->vector0 , 0);
            bsiDMatrix4d_getColumnDPoint4d (&BQT, &pHEllipse->vector90, 1);
            bsiDMatrix4d_getColumnDPoint4d (&BQT, &vectorW , 2);
            bsiDMatrix4d_getColumnDPoint4d (&BQT, &pHEllipse->center  , 3);
            bsiDPoint4d_scale (&pHEllipse->vector0 , &pHEllipse->vector0 , cosineThetaHat);
            bsiDPoint4d_scale (&pHEllipse->vector90, &pHEllipse->vector90, cosineThetaHat);
            bsiDPoint4d_addScaledDPoint4d (&pHEllipse->center, &pHEllipse->center,
                                            &vectorW, sineThetaHat);
            bsiRange1d_setArcSweep( &pHEllipse->sectors, 0.0, msGeomConst_2pi );

            result = true;
            }
        }

    return result;
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
 @bsimethod                                                     EarlinLutz      12/97
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
        bsiDPoint3d_getXYZ (&pEllipse->center,   &normalizedSource.center  );
        bsiDPoint3d_getXYZ (&pEllipse->vector0,  &normalizedSource.vector0 );
        bsiDPoint3d_getXYZ (&pEllipse->vector90, &normalizedSource.vector90);

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

/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_init

(
DRange2dP pRange        /* <= range to be initialized */
)

    {
    pRange->low.x = pRange->low.y = DBL_MAX;
    pRange->high.x = pRange->high.y = -DBL_MAX;
    }

/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange2d.init()
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromArray

(
DRange2dP pRange,
DPoint2dCP pPoint,
int             n
)

    {
    int i;
    DPoint2d *  minP = &pRange->low;
    DPoint2d *  maxP = &pRange->high;
    if (n < 1)
        {
        bsiDRange2d_init (pRange);
        }
    else
        {
        *minP = *maxP = pPoint[0];
        for (i=1; i<n; i++)
            {
            FIX_MINMAX ( pPoint[i].x, minP->x, maxP->x );
            FIX_MINMAX ( pPoint[i].y, minP->y, maxP->y );
            }
        }
    }

/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @see
* @indexVerb extrema
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDRange2d_getLargestCoordinate

(
DRange2dCP pRange
)

    {
     double     max;
     DPoint2d diagonal;

     max = fabs(pRange->low.x);
     FIX_MAX(fabs(pRange->high.x), max);
     FIX_MAX(fabs(pRange->low.y), max);
     FIX_MAX(fabs(pRange->high.y), max);

     diagonal.DifferenceOf (pRange->high, pRange->low);

     FIX_MAX(fabs(diagonal.x), max);
     FIX_MAX(fabs(diagonal.y), max);

     return max;

    }

END_BENTLEY_GEOMETRY_NAMESPACE

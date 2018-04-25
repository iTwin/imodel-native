/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_lastUsedFunctions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
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

END_BENTLEY_GEOMETRY_NAMESPACE

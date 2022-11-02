/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static DPoint3d ProjectPointOnRayXY
(
DPoint3dCR spacePoint,
DPoint3dCR linePoint,
DVec3dCR   lineVector
)
    {
    DVec3d spaceVector;
    spaceVector.DifferenceOf (spacePoint, linePoint);
    double uv = spaceVector.DotProductXY(lineVector);
    double uu = lineVector.DotProductXY(lineVector);
    double fraction;
    DoubleOps::SafeDivide (fraction, uv, uu, 0.0);
    DPoint3d result;
    result.SumOf (linePoint,lineVector, fraction);
    return result;
    }

static DPoint3d TangencyPointOnCircleXY
(
DPoint3dCR spacePoint,
DPoint3dCR circleCenter,
double    circleRadius,
double    tangencyRadius
)
    {
    tangencyRadius = fabs (tangencyRadius);
    circleRadius = fabs (circleRadius);
    // Tangency point between the circles is one of the two intersections of the circle and the line between the two centers. 
    DVec3d centerVector, direction;
    centerVector.DifferenceOf (circleCenter, spacePoint);
    centerVector.z = 0.0;
    double centerDistance = direction.Normalize (centerVector);
    if (centerDistance < bsiTrig_smallAngle () * (circleRadius + tangencyRadius))
        {
        direction.Init (1, 0, 0);
        }
    DPoint3d candidateA, candidateB;
    candidateA.SumOf (circleCenter,direction, circleRadius);
    candidateB.SumOf (circleCenter,direction, -  circleRadius);
    double rA = candidateA.DistanceXY (spacePoint);
    double rB = candidateB.DistanceXY (spacePoint);
    return (fabs (rA - tangencyRadius) < fabs (rB - tangencyRadius)) ? candidateA : candidateB;
    }

// Compute tangency points and append center, radius, and tangency points to output arrays.
// A,B,C are (variantly) lines and arcs, with line/arc distinguished by NULL pointers for line direction.
// If pDirectionA is passed it is a line.
// If pDirecitonA is not passed, it is a circle of radiusA.
// All output arrays are optional.
static bool RecordCenterAndTangencies
(
DPoint3dP pCenterArrayOut,
double *  pRadiusArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
DPoint3dP pTangentPointCOut,
int       &numOut,
int       maxOut,
DPoint3dCR center,
double     tangencyRadius,
DPoint3dCR pointA,
DVec3dCP   pDirectionA,
double     radiusA,
DPoint3dCR pointB,
DVec3dCP   pDirectionB,
double     radiusB,
DPoint3dCR pointC,
DVec3dCP   pDirectionC,
double     radiusC
)
    {
    if (numOut >= maxOut)
        return false;

    if (NULL != pCenterArrayOut)
        pCenterArrayOut[numOut] = center;

    if (NULL != pRadiusArrayOut)
        pRadiusArrayOut[numOut] = tangencyRadius;

    if (pTangentPointAOut)
        if (NULL != pDirectionA)
            pTangentPointAOut[numOut] = ProjectPointOnRayXY (center, pointA, *pDirectionA);
        else
            pTangentPointAOut[numOut] = TangencyPointOnCircleXY (center, pointA, radiusA, tangencyRadius);

    if (pTangentPointBOut)
        if (NULL != pDirectionB)
            pTangentPointBOut[numOut] = ProjectPointOnRayXY (center, pointB, *pDirectionB);
        else
            pTangentPointBOut[numOut] = TangencyPointOnCircleXY (center, pointB, radiusB, tangencyRadius);

    if (pTangentPointCOut)
        if (NULL != pDirectionC)
            pTangentPointCOut[numOut] = ProjectPointOnRayXY (center, pointC, *pDirectionC);
        else
            pTangentPointCOut[numOut] = TangencyPointOnCircleXY (center, pointC, radiusC, tangencyRadius);

    numOut++;
    return true;
    }



/*---------------------------------------------------------------------------------**//**
@description Initialize a conic for a locus of centers of circle tangent to 2 given circles.
   The conic can be an ellipse or hyperbola.
@param pConic OUT The initialized conic.
@param pCenterA IN center of first circle.
@param rA IN signed radius of first circle.
@param pCenterB IN center of second circle.
@param rB IN signed radius of second circle.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool bsiDConic4d_initSignedCircleTangentCenters
(
DConic4dP pConic,
DPoint3dP pCenterA,
double rA,  // SIGNED.
DPoint3dP pCenterB,
double rB   // SIGNED
)
    {
    DPoint3d origin;
    DPoint3d centerA = *pCenterA;
    DPoint3d centerB = *pCenterB;
    double d = centerA.Distance (centerB);
    origin.Interpolate (centerA, 0.5, centerB);
    double h = rA + rB;
    double disc = d * d - h * h;
    double hy = sqrt (fabs (disc));
    DVec3d xAxis, yAxis;
    bool boolstat = false;

    xAxis.NormalizedDifference(centerB, origin);

    yAxis = xAxis;
    yAxis.y =  xAxis.x;
    yAxis.x = -xAxis.y;
    double ax = h * 0.5;
    double ay = hy * 0.5;
    if (disc > 0.0)
        {
        DPoint4d hvec1, hvecSec, hvecTan;
        // The hyperbola is
        //  X = origin + xAxis * ax * sec (theta) + yAxis * ay * tan (theta)
        // X cos (theta) = origin * cos(theta) + xAxis * ay  + yAxis * ay * sin(theta)
        hvec1.Init(   origin.x, origin.y, origin.z, 1.0);
        hvecSec.Init( ax * xAxis.x, ax * xAxis.y,  ax * xAxis.z, 0.0);
        hvecTan.Init( ay * yAxis.x, ay * yAxis.y,  ay * yAxis.z, 0.0);
        bsiDConic4d_initFrom4dVectors (pConic, &hvecSec, &hvecTan, &hvec1, 0.0, msGeomConst_2pi);
        boolstat = true;
        }
    else if (disc < 0.0)
        {
        DPoint4d hvec1, hvecCos, hvecSin;
        // The ellipse is
        //  X = origin + xAxis * ax * cos (theta) + yAxis * ay * sin (theta)
        hvec1.Init(   origin.x, origin.y, origin.z, 1.0);
        hvecCos.Init( ax * xAxis.x, ax * xAxis.y,  ax * xAxis.z, 0.0);
        hvecSin.Init( ay * yAxis.x, ay * yAxis.y,  ay * yAxis.z, 0.0);
        bsiDConic4d_initFrom4dVectors (pConic, &hvec1, &hvecCos, &hvecSin, 0.0, msGeomConst_2pi);
        boolstat = true;
        }
    else
        {
        // Hmmmm... Circles already have contact at tangent ??
        boolstat = false;
        }
    bsiDConic4d_isFullSweep (pConic);
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
@description Generate 0, 1, or 2 conics for the locii of tangent circle centers.
@param pConicArray OUT array of conics --- hyperbola, ellipse depending on relative size and position
        of circles.  Caller MUST dimension this array to at least 2.
@param pNumConic OUT number of conics placed in output bufferr.
@param pCenterA IN center of first circle.
@param rA IN radius of first circle
@param pCenterB IN center of second circle.
@param rB IN radius of second circle.
@group "Tangent Circles"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_circleTTLociiOfCenters

(
DConic4dP pConicArray,
int      *pNumConic,
DPoint3dP pCenterA,
double   rA,
DPoint3dP pCenterB,
double   rB
)
    {
    DConic4d conic;
    *pNumConic = 0;

    if (bsiDConic4d_initSignedCircleTangentCenters (&conic, pCenterA, rA, pCenterB, rB))
        {
        pConicArray[*pNumConic] = conic;
        *pNumConic += 1;
        }

    if (rA != 0.0 && rB != 0.0)
        {
        if (bsiDConic4d_initSignedCircleTangentCenters (&conic, pCenterA, rA, pCenterB, -rB))
            {
            pConicArray[*pNumConic] = conic;
            *pNumConic += 1;
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* @description shuffle 3 centers and radii so that the center with largest total distance
   to other centers is first.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void shuffleCenters

(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
DPoint3dP pCenterArrayIn,
double   *pRadiusArrayIn,
int      *pExternalToInternal
)
    {
    double f[3];
    int id[3];
    int next[3] = {1,2,0};
    int pred[3] = {2,0,1};
    for (int i = 0; i < 3; i++)
        {
        id[i] = i;
        f[i] = pCenterArrayIn[i].Distance (pCenterArrayIn[next[i]])
             + pCenterArrayIn[i].Distance (pCenterArrayIn[pred[i]]);
        }
    
    for (int i = 0; i < 3; i++)
        {
        for (int j = i+1; j < 3; j++)
            {
            if (f[id[j]] > f[id[i]])
                {
                int a = id[j];
                id[j] = id[i];
                id[i] = a;
                }
            }
        }

    for (int i = 0; i < 3; i++)
        {
        pRadiusArrayOut[i] = pRadiusArrayIn[id[i]];
        pCenterArrayOut[i] = pCenterArrayIn[id[i]];
        pExternalToInternal[id[i]] = i;
        }
    
    }
/*---------------------------------------------------------------------------------**//**
* @description compute the centers of circles which are tangent to 3 given circles, under previously
*       verified condition that the input centers are colinear.
* @param pCenterArrayOut OUT array of centers of tangent circles.
* @param pRadiusArrayOut OUT array of radii of tangent circls
* @param pNumOut OUT number of tangent circles
* @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
        possible.
* @param pCenterArrayIn IN array of 3 known circle centers.
* @param pRadiusArrayIn IN array of 3 known radii.  Zero radius inputs are
        permitted.
* @group "Tangent Circles"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool bsiGeom_circleTTT_colinear_append_rotated

(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
DPoint3dP pTangentArrayAOut,
DPoint3dP pTangentArrayBOut,
DPoint3dP pTangentArrayCOut,
int      &numOut,
int      maxOut,
DPoint3dP pCenterArrayIn,
double   r0,
double   r1,
double   r2
)
    {
    DVec3d vec01, vec02;
    vec01.DifferenceOf (pCenterArrayIn[1], pCenterArrayIn[0]);
    vec02.DifferenceOf (pCenterArrayIn[2], pCenterArrayIn[0]);
    vec01.z = vec02.z = 0.0;
    double x1 = vec01.MagnitudeXY ();
    double x2 = vec02.MagnitudeXY ();
    if (vec01.DotProduct (vec02) < 0.0)
        x2 = - x2;
    DVec3d vectorU, vectorV;
    vectorU.Normalize (vec01);
    vectorV.UnitPerpendicularXY (vectorU);

    /*
    Measuring from center 0 in rotated system:
    x^2 + y^2  = (r+r0)^2
    (x-x1)^2 + y^2 = (r+r1)^2
    (x-x2)^2 + y^2 = (r+r2)^2
    -------------------------
    x^2 + y^2  = (r+r0)^2
    -2x1 x + x1^2 = (r+r1)^2 - (r+r0)^2
    -2x2 x + x2^2 = (r+r2)^2 - (r+r0)^2
    -------------------------
    x^2 + y^2  = (r+r0)^2
    -2x1 x = 2 (r1-r0) r + r1^2 - r0^2 - x1^2
    -2x2 x = 2 (r2-r0) r + r2^2 - r0^2 - x2^2
    -------------------------
    Solve
    -2x1 x - 2 (r1-r0) r = r1^2 - r0^2 - x1^2
    -2x2 x - 2 (r2-r0) r = r2^2 - r0^2 - x2^2
    */
    double ax = -2.0 * x1;
    double ar = -2.0 * (r1 - r0);
    double a  = r1 * r1 - r0 * r0 - x1 * x1;
    double bx = -2.0 * x2;
    double br = -2.0 * (r2 - r0);
    double b  = r2 * r2 - r0 * r0 - x2 * x2;
    double x, r;
    static double sRelTol = 1.0e-14;
    if (bsiSVD_solve2x2 (&x, &r,
            ax, ar,
            bx, br,
            a, b))
        {
        double dd = (r + r0) * (r + r0) - x * x;
        double tol = sRelTol * x * x;
        if (fabs (dd) < tol)
            dd = 0.0;
        if (dd >= 0.0)
            {
            double y = sqrt (dd);
            DPoint3d xyz0, xyz1;
            xyz0 = DPoint3d::FromSumOf (pCenterArrayIn[0], vectorU, x, vectorV,  y);
            xyz1 = DPoint3d::FromSumOf (pCenterArrayIn[0], vectorU, x, vectorV, -y);
            RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                          pTangentArrayAOut, pTangentArrayBOut, pTangentArrayCOut, numOut, maxOut,
                          xyz0, fabs (r),
                          pCenterArrayIn[0], NULL, r0,
                          pCenterArrayIn[1], NULL, r1,
                          pCenterArrayIn[2], NULL, r2);

            RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                          pTangentArrayAOut, pTangentArrayBOut, pTangentArrayCOut, numOut, maxOut,
                          xyz1, fabs (r),
                          pCenterArrayIn[0], NULL, r0,
                          pCenterArrayIn[1], NULL, r1,
                          pCenterArrayIn[2], NULL, r2);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description compute the centers of circles which are tangent to 3 given circles.
* @param pCenterArrayOut OUT array of centers of tangent circles.
* @param pRadiusArrayOut OUT array of radii of tangent circls
* @param pNumOut OUT number of tangent circles
* @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
        possible.
* @param pCenterArrayIn IN array of 3 known circle centers.
* @param pRadiusArrayIn IN array of 3 known radii.  Zero radius inputs are
        permitted.
* @group "Tangent Circles"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void bsiGeom_circleTTTCircleConstruction_thisOrder

(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
DPoint3dP pTangentAOut,
DPoint3dP pTangentBOut,
DPoint3dP pTangentCOut,
int      *pNumOut,
int      maxOut,
DPoint3dP pCenterArrayIn,
double   *pRadiusArrayIn
)
    {
    static double sDetTol = 1.0e-8;
//#define RECORD_INPUTS
#ifdef RECORD_INPUTS 
    static double sRadiusIn[3] = {0, 0, 0};
    static DPoint3d sCenterIn[3] = {{0,0}, {0,0}, {0,0}};
    static int sPrintTransitions = 0;
    static int sCallCount = 0;
    sCallCount++;
    // This provides a place to break ....
    if (   0 != memcmp (sRadiusIn, pRadiusArrayIn, 3 * sizeof (double))
        || 0 != memcmp (sCenterIn, pCenterArrayIn, 3 * sizeof (DPoint3d)))
        {
        for (int k = 0; k < 3; k++)
            {
            sRadiusIn[k] = pRadiusArrayIn[k];
            sCenterIn[k] = pCenterArrayIn[k];
            }
#ifdef DOPRINT
        if (sPrintTransitions)
            {
            printf (" TTT call %d\n", sCallCount);
            for (k = 0; k < 3; k++)
                {
                printf ("     (x %.15lg) (y %.15lg) (r %.15lg)\n",
                        sCenterIn[k].x, sCenterIn[k].x, sRadiusIn[k]);
                }
            }
#endif
        }
#endif
    double radiusSign[2] = {1.0, -1.0};

    // The first NONZERO radius can always be positive.
    // SUBSEQUENT nonzero radii must be considred both positive and negative.
    int n1 = 2;
    int n2 = 2;
    if (pRadiusArrayIn[0] == 0.0)
        {
        n1 = 1;
        if (pRadiusArrayIn[1] == 0.0)
            n2 = 1;
        }
    if (pRadiusArrayIn[1] == 0.0)
        n1 = 1;
    if (pRadiusArrayIn[2] == 0.0)
        n2 = 1;

    RotMatrix Minverse;

    DVec3d vector01, vector02;
    DVec3d vectorA, vectorB;
    double det, dot;
    int numOut = 0;

    if (pNumOut)
        *pNumOut = 0;

    double r0 = pRadiusArrayIn[0];
    DPoint3d xyz0 = pCenterArrayIn[0];
    vector01.DifferenceOf (pCenterArrayIn[1], pCenterArrayIn[0]);
    vector02.DifferenceOf (pCenterArrayIn[2], pCenterArrayIn[0]);

    det = vector01.CrossProductXY (vector02);
    dot = vector01.DotProduct (vector02);
    double a;
    if (fabs (det) <= sDetTol * fabs (dot) || !DoubleOps::SafeDivide (a, 1.0, det, 0.0))
        {
        for (int k1 = 0; k1 < n1; k1++)
            {
            double r1 = radiusSign[k1] * pRadiusArrayIn[1];
            for (int k2 = 0; k2 < n2; k2++)
                {
                double r2 = radiusSign[k2] * pRadiusArrayIn[2];
                bsiGeom_circleTTT_colinear_append_rotated (pCenterArrayOut, pRadiusArrayOut,
                        pTangentAOut, pTangentBOut, pTangentCOut, numOut, maxOut,
                        pCenterArrayIn, pRadiusArrayIn[0], r1, r2);
                if (pNumOut)
                    *pNumOut = numOut;
                }
            }
        return;
        }
    Minverse.InitFromRowValues (
                         vector02.y * a,  -vector01.y * a, 0.0,
                        -vector02.x * a,   vector01.x * a, 0.0,
                         0.0,          0.0,        1.0);
#define SQUARE(a) ((a) * (a))


    for (int k1 = 0; k1 < n1; k1++)
        {
        double r1 = radiusSign[k1] * pRadiusArrayIn[1];
        for (int k2 = 0; k2 < n2; k2++)
            {
            double r2 = radiusSign[k2] * pRadiusArrayIn[2];

            vectorA.Init (
                -0.5 * (SQUARE(r1) - SQUARE(vector01.x) - SQUARE(vector01.y)- SQUARE(r0)),
                -0.5 * (SQUARE(r2) - SQUARE(vector02.x) - SQUARE(vector02.y) - SQUARE(r0)),
                0.0);

            vectorB.Init (
                -(r1-r0),
                -(r2-r0),
                0.0);

            DVec3d vectorA1, b;
            vectorA1 = Minverse * vectorA;
            b = Minverse * vectorB;

            // quadratic equation coefficients ...
            double qa = SQUARE (b.x) + SQUARE(b.y) - 1.0;
            double qb = 2.0 * (vectorA1.x * b.x + vectorA1.y * b.y - r0);
            double qc = SQUARE (vectorA1.x) + SQUARE (vectorA1.y) - SQUARE (r0);
            double rr[2];
            int numSolution = bsiMath_solveQuadratic (rr, qa, qb, qc);
            if (numSolution == 2
                && fabs (rr[0] + rr[1]) < bsiTrig_smallAngle () * (fabs (rr[0]) + fabs(rr[1])))
                numSolution = 1;

            for (int i = 0; i < numSolution; i++)
                {
                DPoint3d xyz = xyz0;
                double r = rr[i];
                xyz.x += vectorA1.x + b.x * r;
                xyz.y += vectorA1.y + b.y * r;
                RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                            pTangentAOut, pTangentBOut, pTangentCOut, numOut, maxOut,
                            xyz, r,
                            pCenterArrayIn[0], NULL, pRadiusArrayIn[0],
                            pCenterArrayIn[1], NULL, pRadiusArrayIn[1],
                            pCenterArrayIn[2], NULL, pRadiusArrayIn[2]);
                }
            }
        }

    if (pNumOut)
        *pNumOut = numOut;
    }



//!@description compute the centers of circles which are tangent to 3 given circles
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pRadiusArrayOut OUT array of radii of tangent circls
//!@param pTangentAOut OUT array of tangency points on circle A
//!@param pTangentBOut OUT array of tangency points on circle B
//!@param pTangentCOut OUT array of tangency points on circle C
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!        possible.
//! @param [in] centerA center of first circle
//! @param [in] radiusA radius of first circle
//! @param [in] centerB center of second circle
//! @param [in] radiusB radius of second circle
//! @param [in] centerC center of second circle
//! @param [in] radiusC radius of second circle
Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleCircleCircleConstruction
(
DPoint3dP   pCenterArrayOut,
double   *  pRadiusArrayOut,
DPoint3dP   pTangentAOut,
DPoint3dP   pTangentBOut,
DPoint3dP   pTangentCOut,
int         &numOut,
int         maxOut,
DPoint3dCR  centerA,
double      radiusA,
DPoint3dCR  centerB,
double      radiusB,
DPoint3dCR  centerC,
double      radiusC
)
    {
    double rIn[3], rSort[3];
    DPoint3d cIn[3], cSort[3];
    int      externalToInternal[3];
    DPoint3d tangentSort[3][8];
    rIn[0] = radiusA;
    rIn[1] = radiusB;
    rIn[2] = radiusC;

    cIn[0] = centerA;
    cIn[1] = centerB;
    cIn[2] = centerC;
    
    shuffleCenters (cSort, rSort, cIn, rIn, externalToInternal);
    bsiGeom_circleTTTCircleConstruction_thisOrder (pCenterArrayOut, pRadiusArrayOut,
                        tangentSort[0], tangentSort[1], tangentSort[2],
                        &numOut, maxOut,
                        cSort, rSort);
    for (int k = 0; k < numOut; k++)
        {
        if (pTangentAOut != NULL)
            pTangentAOut[k] = tangentSort[externalToInternal[0]][k];
        if (pTangentBOut != NULL)
            pTangentBOut[k] = tangentSort[externalToInternal[1]][k];
        if (pTangentCOut != NULL)
            pTangentCOut[k] = tangentSort[externalToInternal[2]][k];
        }
    }



/*---------------------------------------------------------------------------------**//**
* @description compute the centers of circles which are tangent to 3 given circles.
*    Only xy parts of the inputs participate in the calculation.
* @param pCenterArrayOut OUT array of centers of tangent circles.
* @param pRadiusArrayOut OUT array of radii of tangent circls
* @param pNumOut OUT number of tangent circles
* @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
        possible.
* @param pCenterArrayIn IN array of 3 known circle centers.
* @param pRadiusArrayIn IN array of 3 known radii.  Zero radius inputs are
        permitted.
* @group "Tangent Circles"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleConstruction

(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
int      *pNumOut,
int      maxOut,
DPoint3dP pCenterArrayIn,
double   *pRadiusArrayIn
)
    {
    double rSort[3];
    DPoint3d cSort[3];
    int externalToInternal[3];
    shuffleCenters (cSort, rSort, pCenterArrayIn, pRadiusArrayIn, externalToInternal);
    bsiGeom_circleTTTCircleConstruction_thisOrder (pCenterArrayOut, pRadiusArrayOut, NULL, NULL, NULL, pNumOut, maxOut,
                    cSort, rSort);
    }


//!@description compute the centers of circles which are tangent to 2 given circles and a line
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pRadiusArrayOut OUT array of radii of tangent circls
//!@param pTangentAOut OUT array of tangency points on circle A
//!@param pTangentBOut OUT array of tangency points on circle B
//!@param pTangentCOut OUT array of tangency points on circle C
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!        possible.
//! @param [in] centerA center of first circle
//! @param [in] radiusA radius of first circle
//! @param [in] centerB center of second circle
//! @param [in] radiusB radius of second circle
//! @param [in] linePointC point on line
//! @param [in] lineDirectionC direction of line.
Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleCircleLineConstructionExt
(
DPoint3dP   pCenterArrayOut,
double   *  pRadiusArrayOut,
DPoint3dP   pTangentAOut,
DPoint3dP   pTangentBOut,
DPoint3dP   pTangentCOut,
int         &numOut,
int         maxOut,
DPoint3dCR  centerA,
double      radiusA,
DPoint3dCR  centerB,
double      radiusB,
DPoint3dCR  linePointC,
DVec3dCR    lineDirectionC
)
    {
/*--------------------------------------------------------------------------------------

(x-x0)^2 + (y-y0)^2 - (r + r0)^2 = 0

(x-x1)^2 + (y-y1)^2 - (r + r1)^2 = 0
    y = r       NOTE: Cannot arbitrarily change sign of r as is done in 3-circle case.
-------------------------------------------
x^2 -2 x0 x + x0^2 -2 y0 y + y0^2 - 2 r0 y - r0^2 = 0
x^2 -2 x1 x + x1^2 -2 y1 y + y1^2 - 2 r1 y - r1^2 = 0
-------------------------------------------
x^2 -2 x0 x + -2 (y0 + r0) y + a0= 0          a0 = x0^2 + y0^2 - r0^2
x^2 -2 x1 x + -2 (y1 + r1) y + a1= 0          a1 = x1^2 + y1^2 - r1^2
-------------------------------------------

(A0) x^2 + b0 x + c0 y + a0= 0          b0 = -2x0    c0 = -2 (y0 + r0)

(A1) x^2 + b1 x + c1 y + a1= 0          b1 = -2x1    c1 = -2 (y1 + r1)
-------------------------------------------
c1 x^2             c1 c0 y + c1 a0= 0
c0 x^2 + c0 b1 x + c0 c1 y + c0 a1= 0
---------------------------------------
Subtract first from second.

(c0 - c1) x^2 + (c0 b1 - c1 b0) x + (c0 a1 - c1 a0) = 0.
Solve for two x values.  Substitute in with largest c0, c1.
----------------------------------------------------------------------------------------*/

    static int rsign[2][4] = { {1,1,-1,-1},{1,-1,1,-1}};
    double aa[2], bb[2], cc[2], rr[2];
    DVec3d xVec, yVec, xyGlobal[2], xy[2];

    xVec = lineDirectionC;
    xVec.z = 0.0;
    xVec.Normalize ();
    yVec.UnitPerpendicularXY (xVec);
    xyGlobal[0].DifferenceOf (centerA, linePointC);
    xyGlobal[1].DifferenceOf (centerB, linePointC);

    for (int i = 0; i < 2; i++)
        {
        xy[i].x = xyGlobal[i].DotProduct (xVec);
        xy[i].y = xyGlobal[i].DotProduct (yVec);
        }

    numOut = 0;
    for (int m = 0; m < 4; m++)
        {
        rr[0] = rsign[0][m] * radiusA;
        rr[1] = rsign[1][m] * radiusB;

        for (int i = 0; i < 2; i++)
            {
            aa[i] = xy[i].x * xy[i].x + xy[i].y * xy[i].y - rr[i] * rr[i];
            bb[i] = -2.0 * xy[i].x;
            cc[i] = -2.0 * (xy[i].y + rr[i]);
            }
        int k = (fabs (cc[0]) > fabs (cc[1])) ? 0 : 1;

        double qa = cc[0] - cc[1];
        double qb = cc[0] * bb[1] - cc[1] * bb[0];
        double qc = cc[0] * aa[1] - cc[1] * aa[0];

        double xRoot[2], yRoot[2];
        int numRoot = bsiMath_solveQuadratic (xRoot, qa, qb, qc);

        for (int i = 0; i < numRoot; i++)
            {
            if (DoubleOps::SafeDivide (yRoot[i],
                        xRoot[i] * xRoot[i] + bb[k] * xRoot[i] + aa[k],
                        -cc[k], 0.0))
                {
                DPoint3d xyz;
                double tangentRadius = fabs (yRoot[i]);
                xyz = DPoint3d::FromSumOf (linePointC, xVec, xRoot[i], yVec, yRoot[i]);

                RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                            pTangentAOut, pTangentBOut, pTangentCOut, numOut, maxOut,
                            xyz, tangentRadius,
                            centerA, NULL, radiusA,
                            centerB, NULL, radiusB,
                            linePointC, &lineDirectionC, 0.0);
                }
            }
        }

    }


Public GEOMDLLIMPEXP void bsiGeom_circleTTTCircleCircleLineConstruction

(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
int      *pNumOut,
int      maxOut,
DPoint3dP pCenterArrayIn,
double   *pRadiusArrayIn,
DPoint3dP pLinePoint,
DVec3dP pLineDirection
)
    {
    return bsiGeom_circleTTTCircleCircleLineConstructionExt (pCenterArrayOut, pRadiusArrayOut,
            NULL, NULL, NULL,
            *pNumOut, maxOut,
            pCenterArrayIn[0], pRadiusArrayIn[0],
            pCenterArrayIn[1], pRadiusArrayIn[1],
            *pLinePoint, *pLineDirection);
    }
/*---------------------------------------------------------------------------------**//**
* @description compute the centers of circles which are tangent to a circle and 2 given lines
*    Only xy parts of the inputs participate in the calculation.
* @param pCenterArrayOut OUT array of centers of tangent circles.
* @param pRadiusArrayOut OUT array of radii of tangent circls
* @param pNumOut OUT number of tangent circles
* @param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
        possible.
* @param pCenterIn IN known circle center centers.
* @param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
* @param pLinePointA IN any point on line A.
* @param pLineDirectionA IN a vector in the direction of line A.
* @param pLinePointB IN any point on line B
* @param pLineDirectionB IN a vector in the direction of line B.
* @group "Tangent Circles"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_circleTTTLineLineCircleConstruction

(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
DPoint3dP pTangentPointCOut,
int      &numOut,
int      maxOut,
DPoint3dCR linePointA,
DVec3dCR   lineDirectionA,
DPoint3dCR linePointB,
DVec3dCR   lineDirectionB,
DPoint3dCR centerC,
double     radiusC
)
    {
/*--------------------------------------------------------------------------------------
Put origin at circle center.
A,B are line points.
M,N are line normals.
(X).(X) = (a +- r)^2 = 0
(X-A).M     = +-r
(X-B).N     = +-r
Need to consider 4 combinations of signs: (+++) (++-) (-++) (-+-) The other 4 are negations.
Write the linear part as
[M N]^ * X = [M.A N.B]^ + r Ei       where Ei is one of E0=[1 1]^   or  E1=[1 -1]^
Mutliply by inverse of matrix 
   X = F + r G
The quadratic part is
(F + rG).(F + rG) = (a +- r)^2
(F + rG).(F + rG) = a^2 +- 2ar + r^2
F.F + 2r G.F + r^2 G.G = a^2 +- 2ar + r^2
r^2 (1-G.G) + 2(+-a - G.F) + a^2-F.F = 0.
Solve with positive, negative branch.  Each generates 2 solutions to go back through Ei.
----------------------------------------------------------------------------------------*/

    double a = radiusC;
    numOut = 0;
    DVec3d orgA, orgB;
    DVec3d normalM, normalN;
    double r[2];
    int numR;
    DVec3d vecF, vecG, vecH;
    DPoint3d center;
    vecF.Zero ();
    vecG.Zero ();
    vecH.Zero ();
    normalM.UnitPerpendicularXY (lineDirectionA);
    normalN.UnitPerpendicularXY (lineDirectionB);
    orgA.DifferenceOf (linePointA, centerC);
    orgB.DifferenceOf (linePointB, centerC);
    double dotMA = orgA.DotProductXY(normalM);
    double dotNB = orgB.DotProductXY(normalN);

    if (normalM.IsParallelTo (normalN)
        || !bsiSVD_solve2x2 (
                &vecF.x, &vecF.y,
                normalM.x, normalM.y,
                normalN.x, normalN.y,
                dotMA, dotNB
                ))
        {
        // Lines are parallel.
        // Make a midline.  Half of line separation is the tangent circle radius.
        DPoint3d midLinePoint;
        DVec3d vectorAB, vectorCP;
        vectorAB.DifferenceOf (linePointB, linePointA);
        midLinePoint.SumOf (linePointA,vectorAB, 0.5);
        vectorCP.DifferenceOf (midLinePoint, centerC);
        double a1 = 0.5 * fabs (vectorAB.DotProductXY(normalN));
        // vector from centerC to midline point is vectorCP + s * lineDirectionA
        double coff_ss = lineDirectionA.DotProductXY(lineDirectionA);
        double coff_s  = 2.0 * vectorCP.DotProductXY(lineDirectionA);
        double coff_1  = vectorCP.DotProductXY(vectorCP);
        double targetRadius[2] = {radiusC + a1, radiusC - a1};
        int numPass = 2;
        // special case zero radius.
        if (radiusC == 0.0)
            {
            // the two target radii are negatives --- only one needs to be considered.
            numPass = 1;
            double dMax = DoubleOps::Max (
                  linePointA.Magnitude (),
                  linePointB.Magnitude (),
                  orgA.Magnitude (),
                  orgB.Magnitude ()
                  );
            static double s_relTol = 1.0e-12;
            // Is centerC on either line?
            bool centerKnown = false;
            if (fabs (dotMA) < s_relTol * dMax)
                {
                // Tangency is on lineA.  Move towards line B...
                center.SumOf (centerC, normalN, dotNB > 0.0 ? a1 : -a1);
                centerKnown = true;
                }
            else if (fabs (dotNB) < s_relTol * dMax)
                {
                // Tangency is on lineA.  Move towards line B...
                center.SumOf (centerC, normalM, dotMA > 0.0 ? a1 : -a1);
                centerKnown = true;
                }
            if (centerKnown)
                {
                RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                        pTangentPointAOut, pTangentPointBOut, pTangentPointCOut, numOut, maxOut,
                        center, a1,
                        linePointA, &lineDirectionA, 0.0,
                        linePointB, &lineDirectionB, 0.0,
                        centerC, NULL, radiusC
                        );
                numPass = 0;
                }
            }

        for (int i = 0; i < numPass; i++)
            {
            double s[2];
            int numS = bsiMath_solveQuadratic (s, coff_ss, coff_s, coff_1 - targetRadius[i] * targetRadius[i]);
            for (int j = 0; j < numS; j++)
                {
                center.SumOf (midLinePoint,lineDirectionA, s[j]);
                RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                        pTangentPointAOut, pTangentPointBOut, pTangentPointCOut, numOut, maxOut,
                        center, a1,
                        linePointA, &lineDirectionA, 0.0,
                        linePointB, &lineDirectionB, 0.0,
                        centerC, NULL, radiusC
                        );
                }
            }
        return;
        }

    for (int sign1 = -1; sign1 <= 2.; sign1 += 2)
        {
        if (!bsiSVD_solve2x2 (
                    &vecG.x, &vecG.y,
                    normalM.x, normalM.y,
                    normalN.x, normalN.y,
                    1.0, (double)sign1
                    ))
            return;

        for (int sign2 = -1; sign2 <= 2; sign2 += 2)
            {
            double coffA = 1.0 - vecG.DotProductXY(vecG);
            double coffB = 2.0 * ((double)sign2 * a - vecG.DotProductXY(vecF));
            double coffC = a * a - vecF.DotProductXY(vecF);
            numR = bsiMath_solveQuadratic (r, coffA, coffB, coffC);
            for (int i = 0; i < numR; i++)
                {
                center.SumOf (centerC,vecF, 1.0, vecG, r[i]);
                RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                        pTangentPointAOut, pTangentPointBOut, pTangentPointCOut, numOut, maxOut,
                        center, fabs (r[i]),
                        linePointA, &lineDirectionA, 0.0,
                        linePointB, &lineDirectionB, 0.0,
                        centerC, NULL, radiusC
                        );
                }
            }
        }
    }





//!@description compute the centers of circles which are tangent to 3 lines.
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pRadiusArrayOut OUT array of radii of tangent circls
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!      possible.
//!@param pCenterIn IN known circle center centers.
//!@param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
//!@param pLinePointA IN any point on line A.
//!@param pLineDirectionA IN a vector in the direction of line A.
//!@param pLinePointB IN any point on line B
//!@param pLineDirectionB IN a vector in the direction of line B.
//!@param pLinePointC IN any point on line C
//!@param pLineDirectionC IN a vector in the direction of line C.
Public GEOMDLLIMPEXP void bsiGeom_circleTTTLineLineLineConstruction
(
DPoint3dP pCenterArrayOut,
double   *pRadiusArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
DPoint3dP pTangentPointCOut,
int      &numOut,
int      maxOut,
DPoint3dCR linePointA,
DVec3dCR   lineDirectionA,
DPoint3dCR linePointB,
DVec3dCR   lineDirectionB,
DPoint3dCR linePointC,
DVec3dR    lineDirectionC
)
    {
/*--------------------------------------------------------------------------------------
(X-A).M     = a*r
(X-B).N     = b*r
(X-B).Q     = c*r
Typical expansion is
x*mx + y*my - a*r = A.M
Where a,b,c are combinations of {+1,-1}
Need to consider 4 combinations of signs: (+++) (++-) (+-+) (+--) The other 4 generate negated r as solution.
----------------------------------------------------------------------------------------*/

    numOut = 0;
    DPoint3d center;
    DVec3d orgA, orgB, orgC;
    DVec3d normalA, normalB, normalC;
    RotMatrix matrix;
    DVec3d rhs, xyr;


    normalA.UnitPerpendicularXY (lineDirectionA);
    normalB.UnitPerpendicularXY (lineDirectionB);
    normalC.UnitPerpendicularXY (lineDirectionC);
    orgA.DifferenceOf (linePointA, linePointC);
    orgB.DifferenceOf (linePointB, linePointC);
    orgC.Zero ();
    int signA = 1;
    for (int signB = -1; signB < 3; signB += 2)
        {
        for (int signC = -1; signC < 3; signC += 2)
            {
            matrix.InitFromRowValues (
                    normalA.x, normalA.y, (double)signA,
                    normalB.x, normalB.y, (double)signB,
                    normalC.x, normalC.y, (double)signC
                    );
            rhs.Init (orgA.DotProductXY(normalA), orgB.DotProductXY(normalB), orgC.DotProductXY(normalC));
            if (matrix.Solve (xyr, rhs) && numOut < maxOut)
                {
                center.Init (linePointC.x + xyr.x, linePointC.y + xyr.y, linePointC.z);
                RecordCenterAndTangencies (pCenterArrayOut, pRadiusArrayOut,
                        pTangentPointAOut, pTangentPointBOut, pTangentPointCOut, numOut, maxOut,
                        center, fabs (xyr.z),
                        linePointA, &lineDirectionA, 0.0,
                        linePointB, &lineDirectionB, 0.0,
                        linePointC, &lineDirectionC, 0.0
                        );
                }
            }
        }
    }


static bool RecordCenterTangentTangent
(
DPoint3dP pCenterArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
int      &numOut,
int      maxOut,
DPoint3dCR center,
DPoint3dCR tangentA,
DPoint3dCR tangentB
)
    {
    if (numOut < maxOut)
        {
        pCenterArrayOut[numOut] = center;
        pTangentPointAOut[numOut] = tangentA;
        pTangentPointBOut[numOut] = tangentB;
        numOut++;
        return true;
        }
    return false;
    }

//!@description compute the centers of circles which are tangent to a line and a circle, and have given radius
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pTangentPointAOut OUT array of tangencies on line A.
//!@param pTangentPointBOut OUt array of tangencies on circle B.
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!      possible.
//!@param pCenterIn IN known circle center centers.
//!@param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
//!@param linePointA IN any point on line A.
//!@param lineDirectionA IN a vector in the direction of line A.
//!@param circleCenterB IN center of circle B
//!@param radiusB IN radius of circle B
//!@param radiusC IN target circle radius.
Public GEOMDLLIMPEXP void bsiGeom_circleTTLineCircleConstruction
(
DPoint3dP pCenterArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
int      &numOut,
int      maxOut,
DPoint3dCR linePointA,
DVec3dCR   lineDirectionA,
DPoint3dCR circleCenterB,
double     radiusB,
double     radiusC
)
    {
    DVec3d perpA;
    perpA.UnitPerpendicularXY (lineDirectionA);
    DPoint3d offsetPointA[2];
    offsetPointA[0].SumOf (linePointA,perpA, radiusC);
    offsetPointA[1].SumOf (linePointA,perpA, -radiusC);
    double offsetRadiusB[2];
    offsetRadiusB[0] = radiusB + radiusC;
    offsetRadiusB[1] = radiusB - radiusC;

    numOut = 0;
    for (int iA = 0; iA < 2; iA++)
        {
        DVec3d vectorQ;
        vectorQ.DifferenceOf (offsetPointA[iA], circleCenterB);
        for (int iB = 0; iB < 2; iB++)
            {
            // (Q + s*U) dot (Q + s*U) = r*r
            // Q dot Q + 2 s U dot Q + s^2 U dot U - r*r = 0
            double r = offsetRadiusB[iB];
            double a = lineDirectionA.DotProductXY(lineDirectionA);
            double b = 2.0 * lineDirectionA.DotProductXY(vectorQ);
            double c = vectorQ.DotProductXY(vectorQ) - r * r;
            double lambda[2];
            int num = bsiMath_solveQuadratic (lambda, a, b, c);
            for (int i = 0; i < num && numOut < maxOut; i++)
                {
                pCenterArrayOut[numOut].SumOf (offsetPointA[iA],lineDirectionA, lambda[i]);
                pTangentPointAOut[numOut].SumOf (linePointA,lineDirectionA, lambda[i]);
                double fraction;
                double r1 = circleCenterB.Distance (pCenterArrayOut[numOut]);
                // Circles centered on this intersectio will have two radii tangent to circleA ... one should be exactly radiusC
                if (DoubleOps::SafeDivide (fraction,
                                fabs (fabs (r1-radiusB) - radiusC) < fabs (fabs (r1 + radiusB) - radiusC)
                                        ? radiusB : - radiusB,
                                r1, 0.0))
                    {
                    pTangentPointBOut[numOut].Interpolate (circleCenterB, fraction, pCenterArrayOut[numOut]);
                    numOut++;
                    }
                }
            }
        }
    }

static bool SelectTangency
(
DPoint3dCR center1,
double radius1,
DPoint3dCR center2,
double radius2,
DPoint3dR tangentPoint
)
    {
    DVec3d vector01;
    vector01.DifferenceOf (center2, center1);
    double d01 = vector01.MagnitudeXY ();
    //double a = radius1 + radius2;
    //double b = fabs (radius1 - radius2);
    double f;
    if (DoubleOps::SafeDivide (f, radius1, d01, 0.0))
        {
        DPoint3d tangentA, tangentB;
        tangentA.SumOf (center1,vector01, f);
        tangentB.SumOf (center1,vector01, -f);
        double dA = tangentA.DistanceXY (center2);
        double dB = tangentB.DistanceXY (center2);
        if (fabs (dA - radius2) < fabs (dB - radius2))
            tangentPoint = tangentA;
        else
            tangentPoint = tangentB;
        return true;
        }
    return false;
    }


//!@description compute the centers of circles which are tangent to a line and a circle, and have given radius
//!   Only xy parts of the inputs participate in the calculation.
//!@param pCenterArrayOut OUT array of centers of tangent circles.
//!@param pTangentPointAOut OUT array of tangencies on line A.
//!@param pTangentPointBOut OUt array of tangencies on circle B.
//!@param pNumOut OUT number of tangent circles
//!@param maxOut IN caller's dimension of pCenterArrayOut.   Up to 8 circles
//!      possible.
//!@param pCenterIn IN known circle center centers.
//!@param pRadiusIn IN circle radius.  Zero radius inputs are permitted.
//!@param centerA IN center of circle A
//!@param radiusA IN radius of circle A
//!@param centerB IN center of circle B
//!@param radiusB IN radius of circle B
//!@param radiusC IN target circle radius.
Public GEOMDLLIMPEXP void bsiGeom_circleTTCircleCircleConstruction
(
DPoint3dP pCenterArrayOut,
DPoint3dP pTangentPointAOut,
DPoint3dP pTangentPointBOut,
int      &numOut,
int      maxOut,
DPoint3dCR centerA,
double     radiusA,
DPoint3dCR centerB,
double     radiusB,
double     radiusC
)
    {
    radiusA = fabs (radiusA);
    radiusB = fabs (radiusB);
    double offsetRadiusB[2], offsetRadiusA[2];
    //double tangencyTol = s_tangencyFraction * (radiusA > radiusB ? radiusA : radiusB);
    offsetRadiusB[0] = radiusB + radiusC;
    offsetRadiusB[1] = radiusB - radiusC;
    offsetRadiusA[0] = radiusA + radiusC;
    offsetRadiusA[1] = radiusA - radiusC;
    DVec3d centerVector, perpVector;
    centerVector.DifferenceOf (centerB, centerA);
    perpVector.Init (-centerVector.y, centerVector.x, 0.0);
    double dAB = centerA.DistanceXY (centerB);
    numOut = 0;
    for (int iA = 0; iA < 2; iA++)
        {
        double rA = offsetRadiusA[iA];
        for (int iB = 0; iB < 2; iB++)
            {
            double rB = offsetRadiusB[iB];
            double alpha, f0, f1;
            if (DoubleOps::SafeDivide (alpha, rA * rA - rB * rB + dAB * dAB, 2.0 * dAB, 0.0)
                && DoubleOps::SafeDivide (f0, alpha, dAB, 0.0))
                {
                // special case tangency???
                double dPerp2 = rA * rA - alpha * alpha;
                if (dPerp2 >= 0.0 && DoubleOps::SafeDivide (f1, sqrt (dPerp2), dAB, 0.0))
                    {
                    DPoint3d centerC, tangentA, tangentB;
                    centerC.SumOf (centerA,centerVector, f0, perpVector, f1);
                    if (SelectTangency (centerA, radiusA, centerC, radiusC, tangentA)
                        && SelectTangency (centerB, radiusB, centerC, radiusC, tangentB))
                        RecordCenterTangentTangent (pCenterArrayOut, pTangentPointAOut, pTangentPointBOut, numOut, maxOut,
                                        centerC, tangentA, tangentB);
                    centerC.SumOf (centerA,centerVector, f0, perpVector, -f1);
                    if (SelectTangency (centerA, radiusA, centerC, radiusC, tangentA)
                        && SelectTangency (centerB, radiusB, centerC, radiusC, tangentB))
                        RecordCenterTangentTangent (pCenterArrayOut, pTangentPointAOut, pTangentPointBOut, numOut, maxOut,
                                        centerC, tangentA, tangentB);
                    }
                }
            }
        }       
    }

END_BENTLEY_GEOMETRY_NAMESPACE
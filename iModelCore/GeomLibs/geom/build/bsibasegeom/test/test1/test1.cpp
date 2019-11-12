/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Geom/GeomApi.h>
#include <printfuncs.h>
#ifdef confirmNamespaces
// Confirm that names are accessible bare (relying on USING) and fully qualified ..
static DPoint2d p1;
static ::Bentley::DPoint2d p2;
#include <string>
static string *s1;
static ::Bentley::string *s2;
#endif
static double s_pi = 4.0 * atan (1.0);
void test_bsiDEllipse3d_closestApproachDRay3d
(
)
    {
    DEllipse3d ellipse;
    ellipse.init (1,2,3,
                0.4, 0.3, 0.2,
                1.2, -0.3, 0.8,
                0.0, 2.0 * s_pi);
    DRay3d ray;
    DPoint3d rayOrigin = {0.1, 2, 0.5};
    DVec3d   rayTangent;
    rayTangent.init (0.3, 0.2, -0.5);
    bsiDRay3d_initFromDPoint3dTangent (&ray, &rayOrigin, &rayTangent);

    int numApproach;
    double angle[4];
    double fraction[4];
    DPoint3d xyzEllipse[4];
    DPoint3d xyzRay[4];

    numApproach = bsiDEllipse3d_closestApproachDRay3d (&ellipse,
                        angle, fraction, xyzEllipse, xyzRay, &ray);
    int i;
    for (i = 0; i < numApproach; i++)
        {
        DVec3d ellipseTangent;
        DVec3d rayTangent;
        bsiDRay3d_evaluateTangent (&ray, &rayTangent);
        bsiDEllipse3d_evaluateDerivatives (&ellipse,
                NULL, &ellipseTangent, NULL, angle[i]);
        DVec3d chord;
        chord.differenceOf (&xyzEllipse[i], &xyzRay[i]);
        double dot1 = chord.dotProduct (&ellipseTangent);
        double dot2 = chord.dotProduct (&rayTangent);
        checkDouble (dot1, 0.0, "chord.EllipseTangent");
        checkDouble (dot2, 0.0, "chord.RayTangent");
        }
    }


void test_bsiTransform_isMirror
(
)
    {
    Transform T0;
    DPoint3d origin;
    DVec3d normal;
    bsiTransform_initIdentity (&T0);
    checkBool (bsiTransform_isMirrorAboutPlane (&T0, &origin, &normal),
                    false,
                    "bsiTransform_isMirrorAboutPlane (I)");

    DPoint3d origin0 = {1,2,3};
    DVec3d normal0;
    bsiDVec3d_setXYZ (&normal0, 1,0,0);

    bsiTransform_initFromMirrorPlane (&T0, &origin0, &normal0);

    checkBool (bsiTransform_isMirrorAboutPlane (&T0, &origin, &normal),
                    true,
                    "bsiTransform_isMirrorAboutPlane (origin, normal)");

    DPoint3d origin1 = {1,2,3};
    DVec3d normal1;
    bsiDVec3d_setXYZ (&normal1, 0.3,0.2, 0.5);
    DVec3d normal2;
    DPoint3d origin2;
    bsiDVec3d_normalizeInPlace (&normal1);
    bsiTransform_initFromMirrorPlane (&T0, &origin1, &normal1);

    checkBool (bsiTransform_isMirrorAboutPlane (&T0, &origin, &normal),
                    true,
                    "bsiTransform_isMirrorAboutPlane (origin, normal)");
    T0.multiply (&origin2, &origin);
    checkDPoint3d (&origin, &origin2, "parsed origin");
    double a = bsiDVec3d_dotProduct (&normal, &normal1);
    bsiDVec3d_scale (&normal2, &normal, a);
    checkDVec3d (&normal1, &normal2, "parsed normal");
    }

static void test_RVDecomposition (int axisIndex, double degrees, int viewIndex)
    {
    RotMatrix matrixRV;
    double radians = bsiTrig_degreesToRadians (degrees);
    int axisIndex1, viewIndex1;
    double radians1;

    bsiRotMatrix_initRotationFromStandardView (&matrixRV, axisIndex, radians, viewIndex);
    checkBool (bsiRotMatrix_isRotationFromStandardView (&matrixRV, &axisIndex1, &radians1, &viewIndex1,
                        true, true, true),
                    true,
                    "isRotationFromStandardView");
    checkInt (axisIndex, axisIndex1, "axis index");
    checkDouble (radians, radians1, "rotation angle");
    checkInt (viewIndex, viewIndex1, "View index");
    }

#ifndef STANDARDVIEW_Top
#define     STANDARDVIEW_Top        1
#define     STANDARDVIEW_Bottom     2
#define     STANDARDVIEW_Left       3
#define     STANDARDVIEW_Right      4
#define     STANDARDVIEW_Front      5
#define     STANDARDVIEW_Back       6
#define     STANDARDVIEW_Iso        7
#define     STANDARDVIEW_RightIso   8
#endif


static void checkTriangleMinDist
(
char *pDescr,
DPoint3d *pTrianglePoints,  // 3 points of triangle
DPoint3d *pPlanePoint,    // Point on plane, will be shifted off plane.
DPoint3d *pClosePoint,    // asserted close point, by caller's construction
DVec3d   *pNormal,      // plane normal
double   a              // distance for offset
)
    {
    DPoint3d spacePoint, closePoint, closeBary, worldAtBary;
    char descr0[1024];
    char descr1[1024];
    sprintf (descr0, "Triangle Closest Point %s", pDescr);
    sprintf (descr1, "Tri minDist uvw->xyz %s", pDescr);

    spacePoint.sumOf (pPlanePoint, pNormal, a);

    bsiDPoint3d_minDistToTriangle (&spacePoint,
                &pTrianglePoints[0], &pTrianglePoints[1], &pTrianglePoints[2],
                &closePoint, &closeBary);
    checkDPoint3d (&closePoint, pClosePoint, descr0);

    bsiDPoint3d_fromBarycentricAndDPoint3dTriangle (&worldAtBary, &closeBary,
                &pTrianglePoints[0], &pTrianglePoints[1], &pTrianglePoints[2]);

    checkDPoint3d (&closePoint, &worldAtBary, descr1);
    }

static void test_triangles_go
(
double x0,
double y0,
double z0,
double x1,
double y1,
double z1,
double x2,
double y2,
double z2
)
    {
    DPoint3d worldPoints[3];

    worldPoints[0].init (x0, y0, z0);
    worldPoints[1].init (x1, y1, z1);
    worldPoints[2].init (x2, y2, z2);

    DVec3d normal;
    normal.crossProductToPoints (&worldPoints[0], &worldPoints[1], &worldPoints[2]);
    normal.normalize ();

    for (int edgeIndex = 0; edgeIndex < 3; edgeIndex++)
        {
        int i0 = edgeIndex;
        int i1 = (i0 + 1) % 3;
        int i2 = (i1 + 1) % 3;
        DPoint3d edgePoint;
        edgePoint.interpolate (&worldPoints[i0], 0.442, &worldPoints[i1]);
        DVec3d edgeVector, biVector;
        edgeVector.differenceOf (&worldPoints[i1], &worldPoints[i0]);
        biVector.crossProduct (&edgeVector, &normal);
        DPoint3d interiorPoint, exteriorPoint;
        interiorPoint.sumOf (&edgePoint, &biVector, -0.01);
        exteriorPoint.sumOf (&edgePoint, &biVector, 0.89);


        DVec3d tangent01, tangent02, outvec;
        tangent01.normalizedDifference (&worldPoints[i1], &worldPoints[i0]);
        tangent02.normalizedDifference (&worldPoints[i2], &worldPoints[i0]);
        outvec.sumOf (&tangent01, &tangent02);
        DPoint3d cornerPoint;
        cornerPoint.sumOf (&worldPoints[i0], &outvec, -2.0);

        checkTriangleMinDist ("Above Interior", worldPoints, &interiorPoint, &interiorPoint, &normal, 0.3);
        checkTriangleMinDist ("Above Edge", worldPoints, &edgePoint, &edgePoint, &normal, 0.3);
        checkTriangleMinDist ("Above Exterior", worldPoints, &exteriorPoint, &edgePoint, &normal, 0.3);
        checkTriangleMinDist ("Above Corner", worldPoints, &cornerPoint, &worldPoints[i0], &normal, 0.3);

        checkTriangleMinDist ("ON Interior", worldPoints, &interiorPoint, &interiorPoint, &normal, 0.0);
        checkTriangleMinDist ("ON Edge", worldPoints, &edgePoint, &edgePoint, &normal, 0.0);
        checkTriangleMinDist ("ON Exterior", worldPoints, &exteriorPoint, &edgePoint, &normal, 0.0);
        checkTriangleMinDist ("ON Corner", worldPoints, &cornerPoint, &worldPoints[i0], &normal, 0.0);

        checkTriangleMinDist ("Below Interior", worldPoints, &interiorPoint, &interiorPoint, &normal, -0.3);
        checkTriangleMinDist ("Below Edge", worldPoints, &edgePoint, &edgePoint, &normal, -0.3);
        checkTriangleMinDist ("Below Exterior", worldPoints, &exteriorPoint, &edgePoint, &normal, -0.3);
        checkTriangleMinDist ("Below Corner", worldPoints, &cornerPoint, &worldPoints[i0], &normal, -0.3);
        }
    }

static void test_triangles ()
    {
    test_triangles_go (0,0,0, 1,0,0, 0,1,0);
    test_triangles_go (1,2,3, -3.2, -3.1, 5.4, 0.3, 6.3, 2.4);
    }

static void test_MSViews ()
   {
    for (int i = 1; i <= 8; i++)
        {
        RotMatrix matrix;
        checkTrue (bsiDMatrix3d_getStandardRotation ((DMatrix3d *)&matrix, i),
                    "get standard view");
        checkTrue (bsiRotMatrix_isRigid (&matrix), "rigid");
        }

    test_RVDecomposition (0, 10.0, STANDARDVIEW_Top);
    test_RVDecomposition (1, 11.0, STANDARDVIEW_Top);
    test_RVDecomposition (2, 22.0, STANDARDVIEW_Top);

    test_RVDecomposition (1, 13.0, STANDARDVIEW_Front);
    test_RVDecomposition (2, 14.0, STANDARDVIEW_Front);

    test_RVDecomposition (0, 15.0, STANDARDVIEW_Right);
    test_RVDecomposition (2, 16.0, STANDARDVIEW_Right);

    test_RVDecomposition (0, 17.0, STANDARDVIEW_Left);
    test_RVDecomposition (2, 18.0, STANDARDVIEW_Left);

    test_RVDecomposition (0, 19.0, STANDARDVIEW_Back);
    test_RVDecomposition (2, 20.0, STANDARDVIEW_Back);

    test_RVDecomposition (1, 21.0, STANDARDVIEW_Bottom);
    test_RVDecomposition (2, 22.0, STANDARDVIEW_Bottom);

    test_RVDecomposition (2, 22.0, STANDARDVIEW_Iso);
    test_RVDecomposition (2, 23.0, STANDARDVIEW_RightIso);

    }

// ASSUME polygon on xy plane with positive area.
// Caller knows centroid.
void checkCentroid
(
DPoint3d *pXYZ,
int numXYZ,
double x0,
double y0,
double area0,
double perimeter0,
char const *pDescr
)
    {
    DPoint3d centroid0 = {x0, y0};
    DPoint3d centroid1;
    DVec3d normal0;
    normal0.init (0,0,1);
    DVec3d normal1;
    double perimeter1;
    double area1;
    double planeError1;

    checkTrue (bsiPolygon_centroidAreaPerimeter (pXYZ, numXYZ,
                &centroid1, &normal1, &area1, &perimeter1, &planeError1),
                pDescr);
    checkDPoint3d (&centroid0, &centroid1, "Centroid");
    checkDVec3d (&normal0, &normal1, "normal");
    checkDouble (area0, area1, "area");
    checkDouble (perimeter0, perimeter1, "perimeter");

    Transform transform;
    DPoint3d point0 = {2,3,4};
    DPoint3d point1 = {-3,2,1};
    double radians = 0.3;
    bsiTransform_initFromLineAndRotationAngle (&transform,
                    &point0, &point1, radians);
    DPoint3d image[100];
    DPoint3d centroid2;
    DVec3d normal2;
    bsiTransform_multiplyDPoint3dArray (&transform, image, pXYZ, numXYZ);
    bsiTransform_multiplyDPoint3d (&transform, &centroid2, &centroid0);
    bsiDVec3d_multiplyTransformDVec3d (&normal2, &transform, &normal0);
    // Rotation around line changes centroid and normal, area and perimeter unchanged.
    checkTrue (bsiPolygon_centroidAreaPerimeter (image, numXYZ, &centroid1, &normal1, &area1, &perimeter1, &planeError1),
                "rotated");
    checkDPoint3d (&centroid2, &centroid1, "Centroid");
    checkDVec3d (&normal2, &normal1, "normal");
    checkDouble (area0, area1, "area");
    checkDouble (perimeter0, perimeter1, "perimeter");

    }

void test_polygonCentroid ()
    {
    DPoint3d unitSquare [] =
        {
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {0,1,0}
        };

    checkCentroid (unitSquare, 4, 0.5, 0.5, 1.0, 4.0, "unit square");

    DPoint3d splitSquare [] =
        {
        {0,0,0},
        {1,0,0},
        {0,1,0},
        {1,0,0},
        {1,1,0},
        {0,1,0},
        {0,0,0}
        };

    checkCentroid (splitSquare, 7, 0.5, 0.5, 1.0, 4.0 + 2.0 * sqrt (2.0), "slit square");

    DPoint3d lpoints [] =
        {
        {0,0,0},
        {2,0,0},
        {2,1,0},
        {1,1,0},
        {1,2,0},
        {0,2,0},
        {0,0,0}
        };

    checkCentroid (lpoints, 7, 5.0/6.0, 5.0/6.0, 3.0, 8.0, "l blocks");
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);

    DVec3d direction;
    direction.init (1,2,3);
    DMatrix3d matrix;
    for (int axis = -6; axis < 7; axis++)
        bsiDMatrix3d_initFrom1Vector (&matrix, &direction, axis, true);

    DTransform3d transform;
    bsiDTransform3d_initFromRowValues (&transform,
            1,2,3,4,
            5,6,7,8,
            9,10,11,12
            );
    checkBool (bsiDTransform3d_isTranslate (&transform, NULL), false, "_isTranslate");

    DPoint3d unitSquare [] =
        {
            {0,0,0},
            {1,0,0},
            {1,1,0},
            {0,1,0}
        };
    int unitSquareCount = 4;

    DPoint3d spacePoint;
    DPoint3d closePoint;
    bsiDPoint3d_setXYZ (&spacePoint, -1.0, 0.5, 1.0);
    checkInt (1, bsiPolygon_closestPoint (&closePoint, unitSquare, unitSquareCount, &spacePoint),
                "bsiPolygon_closestPoint (OUT UP)");
    DPoint3d knownSolution;
    bsiDPoint3d_setXYZ(&knownSolution, 0.0, 0.5, 0.0);
    checkDPoint3d (&knownSolution, &closePoint, "midEdge");


    bsiDPoint3d_setXYZ (&spacePoint, -1.0, 2.5, 1.0);
    checkInt (1, bsiPolygon_closestPoint (&closePoint, unitSquare, unitSquareCount, &spacePoint),
                "bsiPolygon_closestPoint (OUTCORNER UP)");
    bsiDPoint3d_setXYZ(&knownSolution, 0.0, 1.0, 0.0);
    checkDPoint3d (&knownSolution, &closePoint, "corner");

    bsiDPoint3d_setXYZ (&spacePoint, 0.5, 0.5, 1.0);
    checkInt (2, bsiPolygon_closestPoint (&closePoint, unitSquare, unitSquareCount, &spacePoint),
                "bsiPolygon_closestPoint (IN UP)");
    bsiDPoint3d_setXYZ(&knownSolution, 0.5, 0.5, 0.0);
    checkDPoint3d (&knownSolution, &closePoint, "interior");

    bsiDPoint3d_setXYZ (&spacePoint, 0.5, 0.5, -1.0);
    checkInt (2, bsiPolygon_closestPoint (&closePoint, unitSquare, unitSquareCount, &spacePoint),
                "bsiPolygon_closestPoint (IN UP)");
    bsiDPoint3d_setXYZ(&knownSolution, 0.5, 0.5, 0.0);
    checkDPoint3d (&knownSolution, &closePoint, "interior");

    test_bsiDEllipse3d_closestApproachDRay3d ();

    test_bsiTransform_isMirror ();

    test_MSViews ();

    test_triangles ();

    test_polygonCentroid ();
    printf ("Hello world\n");
    return getExitStatus();
    }


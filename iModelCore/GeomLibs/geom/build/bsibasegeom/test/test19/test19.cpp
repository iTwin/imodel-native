/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <Geom/newton.h>
#include <printfuncs.h>
#include <stdlib.h>

void printPlane (DPlane3dCP pPlane, char *pName)
    {
    if (pName)
        printf ("%s\n", pName);
    double thetaXY = atan2 (pPlane->normal.y, pPlane->normal.x);
    printf (" (origin %lg %lg %lg) (normal %lg %lg %lg) (degreesXY %lg)\n",
                pPlane->origin.x,
                pPlane->origin.y,
                pPlane->origin.z,
                pPlane->normal.x,
                pPlane->normal.y,
                pPlane->normal.z,
                thetaXY * 45.0 / atan (1.0)
                );
    }

void printPolygon
(
DPoint3dCP pXYZ,
int n,
char *pName,
double expectedArea = 0.0
)
    {
    if (pName)
        printf ("%s\n", pName);
    int i0 = 0;
    double totalArea = 0.0;
    for (int i = 0; i <= n; i++)
        {
        if (i == n || bsiDPoint3d_isDisconnect (&pXYZ[i]))
            {
            if (i > i0)
                {
                DVec3d normal;
                DPoint3d origin;
                double area = bsiPolygon_polygonNormalAndArea (&normal, &origin, &pXYZ[i0], i - i0);
                totalArea += area;
                if (i < n)
                    printf (" %3d <disconnect> (normal %lg %lg %lg) (area %lg)\n", i,
                            normal.x, normal.y, normal.z, area);
                else
                    printf ("                  (normal %lg %lg %lg) (area %lg)\n",
                            normal.x, normal.y, normal.z, area);

                i0 = i + 1;
                }
            }
        else
            printf (" %3d (%lg %lg)\n", i, pXYZ[i].x, pXYZ[i].y);
        }
    if (expectedArea > 0.0)
        checkDouble (totalArea, expectedArea, "Polygon area");
    }

void testPolygonClip ()
    {
    DPoint3d square4[5] =
        {
            {-4,-4},
            {4,-4},
            {4,4},
            {-4,4},
            {-4,-4}
        };

    DPoint3d diamond4[5] =
        {
            {-4,0},
            {0,-4},
            {4,0},
            {0,4},
            {-4,0}
        };

    DPlane3d planeBox2[4];
    planeBox2[0].initFromOriginAndNormal ( 2, 0, 0,  1, 0, 0);
    planeBox2[1].initFromOriginAndNormal ( 0, 2, 0,  0, 1, 0);
    planeBox2[2].initFromOriginAndNormal (-2, 0, 0, -1, 0, 0);
    planeBox2[3].initFromOriginAndNormal (0, -2, 0,  0,-1, 0);

#define MAX_CLIP_OUT 40
    DPoint3d xyzClip[MAX_CLIP_OUT];
    int numClip, numLoop;
    printPolygon (square4, 5, "square4");
    for (int iPlane = 0; iPlane < 4; iPlane++)
        {
        printPlane (&planeBox2[iPlane], "Clip Plane");
        bsiPolygon_clipToPlane (xyzClip, &numClip, &numLoop, MAX_CLIP_OUT, square4, 5, &planeBox2[iPlane]);
        printPolygon (xyzClip, numClip, "postClip", 48.0);
        }

    DPlane3d planeDiamond4[4];
    double a = sqrt (0.5);
    planeDiamond4[0].initFromOriginAndNormal ( 4, 0, 0,  a, -a, 0);
    planeDiamond4[1].initFromOriginAndNormal ( 0, 4, 0,  a,  a, 0);
    planeDiamond4[2].initFromOriginAndNormal (-4, 0, 0, -a,  a, 0);
    planeDiamond4[3].initFromOriginAndNormal (0, -4, 0, -a, -a, 0);

    for (int iPlane = 0; iPlane < 4; iPlane++)
        {
        printPlane (&planeDiamond4[iPlane], "Clip Plane");
        bsiPolygon_clipToPlane (xyzClip, &numClip, &numLoop, MAX_CLIP_OUT, square4, 5, &planeDiamond4[iPlane]);
        printPolygon (xyzClip, numClip, "postClip");
        }

    }

void testPolygonClip_regular
(
double a0,  // Blank radius
int n0,  // blank count
double a1,  // cutter radius
int  n1,   // cutter count
int  k1     // cutter step skip.
)
    {
#define MAX_XYZ 1000
    DPoint3d xyz[2][MAX_XYZ];
    int iOut = 0;
    int n = 0;
    double dTheta = msGeomConst_2pi / (double)n0;
    for (int i = 0; i <= n0; i++)
        {
        double theta = i * dTheta;
        xyz[iOut][n++].init (a0 * cos (theta), a0 * sin (theta), 0.0);
        }

    DPlane3d cutter[MAX_XYZ];
    double dAlpha = msGeomConst_2pi / (double)n1;
    for (int iPlane = 0; iPlane < n1; iPlane++)
        {
        double alpha = k1 * iPlane * dAlpha;
        double cc = cos (alpha);
        double ss = sin (alpha);
        cutter[iPlane].initFromOriginAndNormal (a1 * cc, a1 * ss, 0, cc, ss, 0);
        }


    int numXYZ[2];
    numXYZ[0] = n0;
    int numLoop;
    printPolygon (xyz[iOut], numXYZ[iOut], "blank");
    for (int iPlane = 0; iPlane < n1; iPlane++)
        {
        int jOut = 1 - iOut;
        printPlane (&cutter[iPlane], "Clip Plane");
        bsiPolygon_clipToPlane (xyz[jOut], &numXYZ[jOut], &numLoop, MAX_XYZ, xyz[iOut], numXYZ[iOut], &cutter[iPlane]);
        printPolygon (xyz[jOut], numXYZ[jOut], "postClip");
        iOut = jOut;
        }
    }

void testPolygonClip_vertex ()
    {
    // Two times around a diamond
    DPoint3d xyz[9] =
        {
            { 1, 0, 0},
            { 0, 1, 0},
            {-1, 0, 0},
            { 0,-1, 0},
            { 1, 0, 0},
            { 0, 1, 0},
            {-1, 0, 0},
            { 0,-1, 0},
            { 1, 0, 0},
        };

    DPlane3d planeA;
    planeA.initFromOriginAndNormal (0,0,0, 0, 1, 0);
#define MAX_CLIP_OUT 40
    DPoint3d xyzClip[MAX_CLIP_OUT];
    int numClip, numLoop;
    int numIn = 5;
    for (int i0 = 0; i0 < 4; i0++)
        {
        printPlane (&planeA, "Clip Plane");
        DPoint3d *pXYZIn = &xyz[i0];
        printPolygon (pXYZIn, numIn, "preClip");
        bsiPolygon_clipToPlane (xyzClip, &numClip, &numLoop, MAX_CLIP_OUT, pXYZIn, 5, &planeA);
        printPolygon (xyzClip, numClip, "postClip");
        }
    }

void testNonConvex ()
    {
    DPoint3d xyzIn[9] =
        {
            {-4, -4, 0},
            {-2, -4, 0},
            {-2,  1, 0},
            { 2,  1, 0},
            { 2, -4, 0},
            { 4, -4, 0},
            { 4,  2, 0},
            {-4,  2, 0},
            {-4, -4, 0},
        };

    DPlane3d planeA, planeB;
    planeA.initFromOriginAndNormal (0,0,0, 0,  1, 0);
    planeB.initFromOriginAndNormal (0,0,0, 0, -1, 0);
#define MAX_CLIP_OUT 40
    DPoint3d xyzClip[MAX_CLIP_OUT];
    int numClip, numLoop;
    int numIn = 9;
    printPlane (&planeA, "Clip Plane UP");
    printPolygon (xyzIn, numIn, "indented preClip");
    bsiPolygon_clipToPlane (xyzClip, &numClip, &numLoop, MAX_CLIP_OUT, xyzIn, numIn, &planeA);
    printPolygon (xyzClip, numClip, "postClip BELOW");

    printPlane (&planeB, "Clip Plane DN");
    bsiPolygon_clipToPlane (xyzClip, &numClip, &numLoop, MAX_CLIP_OUT, xyzIn, numIn, &planeB);
    printPolygon (xyzClip, numClip, "postClip ABOVE");

    }


void testIsSimplePolyline ()
    {
    DPoint3d square[5] =
        {
            {1,0},
            {1,1},
            {0,1},
            {0,0},
            {1,0}
        };

    checkFalse (bsiDPoint3dArray_isSelfIntersectingXY (square, 5, true),  "IsSimple:Square");
    checkTrue  (bsiDPoint3dArray_isSelfIntersectingXY (square, 5, false), "IsSimple:SquareAsPolyline");

    DPoint3d bowTie[5] =
        {
            {0,0},
            {2,0},
            {0,2},
            {2,2},
            {0,0}
        };
    checkFalse (bsiDPoint3dArray_isSelfIntersectingXY (square, 5, true),  "IsSimple:BowTie");
    // Make two loops around ..
    DPoint3d xyz0[200];
    int numHigh = 5;

    for (int shiftIndex = 1; shiftIndex < numHigh; shiftIndex++)
        {
        int numXYZ = 0;
        for (int i = 0; i <= numHigh; i++)
            {
            xyz0[numXYZ++].init (i,1,0);
            }
        xyz0[shiftIndex].y = -0.5;
        xyz0[numXYZ++].init (numHigh, 0, 0);
        xyz0[numXYZ++].init (0, 0, 0);
        for (int i = 0; i < numXYZ; i++)
            xyz0[numXYZ+i] = xyz0[i];

        DPoint3d xyz[200];
        // Copy to single loop with each possible start point ..
        for (int k = 0; k < numXYZ; k++)
            {
            int n = 0;
            for (int i = 0; i < numXYZ; i++)
                xyz[n++] = xyz0[k+i];
            xyz[n++] = xyz[0];
            char message[200];
            sprintf (message, "IsSimple:wrapped shiftIndex %d start %d", shiftIndex, k);
            checkTrue (bsiDPoint3dArray_isSelfIntersectingXY (xyz, n, true),  message);
            }
        }
    }
int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testPolygonClip_vertex ();
    testPolygonClip ();
    testPolygonClip_regular (3, 4, 2, 7, 3);
    testPolygonClip_regular (3, 4, 2, 7, 1);
    testPolygonClip_regular (3, 4, 2, 8, 2);    // Force repeated use of same plane??

    testNonConvex ();

    testIsSimplePolyline ();
    return getExitStatus();
    }

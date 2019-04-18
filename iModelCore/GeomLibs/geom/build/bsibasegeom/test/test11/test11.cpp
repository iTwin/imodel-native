/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <stdlib.h>

void testXYConvex ()
    {
    DPoint3d square0[5] =
        {
          {0,0},
          {1,0},
          {1,1},
          {0,1},
          {0,0},
        };

    DPoint3d square1[5] =
        {
          {-1,-2},
          {1,0},
          {1,1},
          {0,1},
          {-1,-2},
        };

    DPoint3d xyz;
    xyz.init (0.3, 0.2, 0.0);
    checkTrue (bsiGeom_isXYPointInConvexPolygon (&xyz, square0, 5, 0), "IN point");
    checkTrue (bsiGeom_isXYPointInConvexPolygon (&xyz, square1, 5, 0), "IN point");
    xyz.init (-4, 1, 0);
    checkFalse (bsiGeom_isXYPointInConvexPolygon (&xyz, square0, 5, 0), "OUT point");
    checkFalse (bsiGeom_isXYPointInConvexPolygon (&xyz, square1, 5, 0), "OUT point");
    }

void testXYCircle (double xc, double yc, double r, int numPoints)
    {
    DPoint3d circlePoints[1000];
    double dTheta = 8.0 * atan (1.0) / numPoints;
    // Make an ngon on the circle.....
    for (int i = 0; i < numPoints; i++)
        {
        double theta = i * dTheta;
        circlePoints[i].init (xc + r * cos (theta), yc + r * sin (theta));
        }

    printf (" Convex tests (xc %g) (yc= %g) (r %g) %d\n", xc, yc, r, numPoints);
    DPoint3d center;
    center.init (xc, yc, 0.0);
    checkTrue (bsiGeom_isXYPointInConvexPolygon (&center, circlePoints, numPoints, 0), "Convex (circle) center");
    DPoint3d xyz;
    double rA = 0.5 * r;
    double rC = 2.0 * r;
    // "f" loop considers points on rays from the center out to points at fraction f along each edge of the ngon.
    for (double f = 0.0; f < 1.0; f += 0.24)
        {
        for (int i = 0; i < numPoints; i++)
            {
            double theta = ((double)i + f) * dTheta;
            int i1 = (i + 1) % numPoints;
            // rA is clearly inside ...
            xyz.init (xc + rA * cos (theta), yc + rA * sin (theta));
            checkTrue (bsiGeom_isXYPointInConvexPolygon (&xyz, circlePoints, numPoints, 0), "Convex, in");
            // rC is clearly inside ...
            xyz.init (xc + rC * cos (theta), yc + rC * sin (theta));
            checkFalse (bsiGeom_isXYPointInConvexPolygon (&xyz, circlePoints, numPoints, 0), "Convex, out");
            DPoint3d xyzEdge;
            // xyzEdge is exactly "on" the edge ...
            xyzEdge.interpolate (&circlePoints[i], f, &circlePoints[i1]);
            // Interpolate along ray from center to put a point just inside the edge ...
            xyz.interpolate (&center, 0.9999, &xyzEdge);
            checkTrue (bsiGeom_isXYPointInConvexPolygon (&xyz, circlePoints, numPoints, 0), "Convex, in near edge");
            // and another just outside the edge ...
            xyz.interpolate (&center, 1.0001, &xyzEdge);
            checkFalse (bsiGeom_isXYPointInConvexPolygon (&xyz, circlePoints, numPoints, 0), "Convex, out near edge");
            }
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testXYConvex ();
    testXYCircle (0.0, 0.0, 1.0, 4);
    testXYCircle (3.0, 4.5, 1.3, 12);
    return getExitStatus();
    }

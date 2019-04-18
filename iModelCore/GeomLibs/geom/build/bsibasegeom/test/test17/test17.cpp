/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <Geom/bezroot.fdf>
#include <Geom/bezeval.fdf>
#include <printfuncs.h>
#include <stdlib.h>


void testLineEllipse
(
double bx,
double by,
double nx,
double ny,
double planeDistance,
double ax,
double ay,
double ux,
double uy,
double vx,
double vy,
double ellipseDistance
)
    {
    DPlane3d plane;
    DEllipse3d ellipse;
    DPoint3d xyz[20];
    DPoint3d trigPoint[20];
    int numXYZ;
    double twoPi = 8.0 * atan (1.0);
    plane.initFromOriginAndNormal (bx, by, 0.0, nx, ny, 0.0);
    plane.normal.normalize ();
    ellipse.init (ax, ay, 0, ux, uy, 0, vx, vy, 0, 0.0, twoPi);
    bsiGeom_intersectOffsetEllipseOffsetPlane (xyz, trigPoint, &numXYZ, 20, &ellipse, ellipseDistance, &plane, planeDistance);
    printf ("\n\n");
    printf (" PLANE   (Bxy=%lg,%lg) (normal=%lg,%lg) (offset=%lg)\n",
                    plane.origin.x, plane.origin.y,
                    plane.normal.x, plane.normal.y,
                    planeDistance
                    );
   printf (" ELLIPSE (Cxy=%lg,%lg) (U=%lg,%lg) (V=%lg,%lg) (offset=%lg)\n",
                    ellipse.center.x, ellipse.center.y,
                    ellipse.vector0.x, ellipse.vector0.y,
                    ellipse.vector90.x, ellipse.vector90.y,
                    ellipseDistance
                    );

    for (int i = 0; i < numXYZ; i++)
        {
        DPoint3d ellipsePoint;
        ellipsePoint.sumOf (&ellipse.center,
                            &ellipse.vector0, trigPoint[i].x,
                            &ellipse.vector90, trigPoint[i].y);
        printDPoint3d ("center", &xyz[i], NULL);
        checkDouble (ellipsePoint.distance (&xyz[i]), fabs (ellipseDistance), "Distance to Ellipse");
        checkDouble (xyz[i].dotDifference (&plane.origin, &plane.normal),
                            planeDistance, "Altitude from plane");
        if (ellipseDistance != 0.0)
            {
            DVec3d ellipseDelta;
            DVec3d tangent;
            ellipseDelta.differenceOf (&xyz[i], &ellipsePoint);
            tangent.sumOf (NULL, &ellipse.vector0, - trigPoint[i].y, &ellipse.vector90, trigPoint[i].x);
            checkTrue (tangent.isPerpendicularTo (&ellipseDelta), "perpendicular at ellipse");
            }
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);

    selectTolerances (3);
    double hStep = 0.5;
    double rStep = 0.5;
    for (double h = -hStep; h < 1.5 * hStep; h += hStep)
        {
        for (double r = -rStep; r < 1.5 * rStep; r+= rStep)
            {
            for (double ay = 0.0; ay < 0.2; ay += 0.8)
                {
                testLineEllipse (
                    0.0, ay,
                    0.0, 1.0,
                    h,
                    0.0, 0.0,
                    3.0, 0.0,
                    0.0, 3.0,
                    r);
                }
            }
        }


    for (double h = -hStep; h < 1.5 * hStep; h += hStep)
        {
        for (double r = -rStep; r < 1.5 * rStep; r+= rStep)
            {
            for (double yFactor = 1.0; yFactor < 30.0; yFactor *= 2.5)
                {
                testLineEllipse (
                        0.1, 1.2,
                        0.5, 1.0,
                        h,
                        0.2, 1.0,
                        3.0, 0.1,
                        0.2, yFactor * 3.4,
                        r);
                }
            }
        }


    return getExitStatus();
    }

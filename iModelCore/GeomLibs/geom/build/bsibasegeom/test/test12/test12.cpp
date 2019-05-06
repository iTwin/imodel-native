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

void testTentVolume ()
    {
    DPoint3d square[5] =
        {
            {0,0,0},
            {1,0,0},
            {1,1,0},
            {0,1,0},
            {0,0,0},
        };

    DPoint3d quad[5] =
        {
            {0,0,0},
            {1,0.2,0},
            {1.1,0.6,0},
            {-4,1,0},
            {0,0,0},
        };


    DVec3d post;
    double h = 2.0;
    post.init (0,0,h);

    // Volume of a pyramid with height h over base with area A is Ah/3.
    // Unit square base ...
    double squareVol = bsiPolygon_tentVolume (square, 5, &post);
    checkDouble (squareVol, h / 3.0, "Unit Square Tent Volume");
    // Irregular quad base ...
    double quadArea = bsiGeom_getXYPolygonArea (quad, 5);
    double quadVol5 =  bsiPolygon_tentVolume (quad, 5, &post);
    checkDouble (quadVol5, h * quadArea / 3.0, "Tent Volume over quad");
    // Confirm that final point is optional ...
    double quadVol4 = bsiPolygon_tentVolume (quad, 4, &post);
    checkDouble (quadVol4, quadVol5, "Tent volume - final point");

    }

void testPerspective (double f)
    {
    double b = 1.0 / (1.0 - f);
    if (b < 1.1)
        return;
    DPoint3d eyePoint = {0,0,b};
    DPoint3d xyz0, xyz1, xyz2, xyz2z;

    for (double theta = 0.0; theta < 3.0; theta += 0.8)
        {
        xyz0.init (cos (theta), sin(theta), 1.0);
        for (double g = 0.5; g < 2.0; g+= 0.74)
            {
            xyz1.interpolate (&eyePoint, g, &xyz0);
            bsiGeom_applyPerspective (&xyz2, &xyz1, 1, f);
            xyz2z = xyz2;
            xyz2z.z = 0.0;
            DVec3d vec1, vec2;
            vec1.differenceOf (&xyz1, &eyePoint);
            vec2.differenceOf (&xyz2z, &eyePoint);
            checkTrue (vec1.isParallelTo (&vec2), "Pespective direction");
            }

        xyz0.init (cos(theta), sin(theta), 0.0);
        bsiGeom_applyPerspective (&xyz1, &xyz0, 1, f);
        checkDouble (0.0, xyz1.z, "Perspective z=0 maps to 0.");
        xyz0.init (cos(theta), sin(theta), 1.0);
        bsiGeom_applyPerspective (&xyz1, &xyz0, 1, f);
        checkDouble (1.0, xyz1.z, "Perspective z=1 maps to 1.");
        }
    }

void testPerspectivePlane
(
double ax,
double ay,
double az,
double ux,
double uy,
double uz,
double vx,
double vy,
double vz,
double f
)
    {
    Transform frame;
    frame.initFromRowValues
        (
        ux, vx, 0.0, ax,
        uy, vy, 0.0, ay,
        uz, vz, 0.0, az
        );

    DPoint3d xyz0[100], xyz1[100];
    int n = 0;
    for (double theta = 0.0; theta < 6; theta += 0.5)
        {
        frame.multiply (&xyz0[n++], cos (theta), sin(theta), 0.0);
        }

    bsiGeom_applyPerspective (xyz1, xyz0, n, f);
    for (int i = 3; i < n; i++)
        {
        // We expect each point to be in the plane of the first three.
        double f = xyz1[i].tripleProductToPoints (&xyz1[0], &xyz1[1], &xyz1[2]);
        checkDouble (f, 0.0, "Post-perspective planarity");
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testPerspective (0.8);
    testPerspective (0.5);
    testPerspectivePlane
                (
                1,2,0.3,
                0.2, 0.3, -0.2,
                -0.2, 0.3, 0.3,
                0.5);
    testTentVolume ();
    return getExitStatus();
    }

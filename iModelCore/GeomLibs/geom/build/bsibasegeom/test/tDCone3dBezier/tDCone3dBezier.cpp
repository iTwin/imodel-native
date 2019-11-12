/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <Geom/dspiral2dbase.h>
#include <printfuncs.h>
#include <stdlib.h>





int main (int argc, char **argv)
    {
    initErrorTracking (NULL, argc, argv);
    DCone3d cone0;
    // unit cylinder from z=0 to z=1
    bsiDCone3d_set (&cone0, NULL, 1.2, NULL);

    DPoint4d line0[2]
        = {
        { 2,   0, 0.4, 1.0},
        {-2, 0.0, 0.4, 1.0}
        };

    DPoint4d line1[2]
        = {
        { 0,0,0,1},
        { 0.2,0.2,2,0}
        };

    CGXmlWriter cgWriter = CGXmlWriter ();
    cgWriter.EmitDCone3d (cone0);
    cgWriter.EmitBezier (line0, 2);
    cgWriter.EmitBezier (line1, 2);

    DPoint3d intersectXYZ[100];
    DPoint3d intersectUVW[100];
    double   intersectParam[100];
    int numOnCone, numOnCap;
    bsiDCone3d_intersectBezierCurve (&cone0,
                intersectParam, intersectXYZ, intersectUVW, &numOnCone, &numOnCap, 20,
                line0, 2, 2, 2);
    //checkInt (numOnCone, 2, "Cone0.Line0 cone hits");
    //checkInt (numOnCap,  0, "Cone0.Line0 cap hits");
    cgWriter.EmitPoints (intersectXYZ, numOnCone + numOnCap);

    bsiDCone3d_intersectBezierCurve (&cone0,
                intersectParam, intersectXYZ, intersectUVW, &numOnCone, &numOnCap, 20,
                line1, 2, 2, 2);
    //checkInt (numOnCone, 0, "Cone0.Line1 cone hits");
    //checkInt (numOnCap,  2, "Cone0.Line1 cap hits");
    cgWriter.EmitPoints (intersectXYZ, numOnCone + numOnCap);

    DPoint4d line2[2]
        = {
        { 0,0,-0.5,1},
        { 3,0,2,1}
        };
    cgWriter.EmitBezier (line2, 2);

    bsiDCone3d_intersectBezierCurve (&cone0,
                intersectParam, intersectXYZ, intersectUVW, &numOnCone, &numOnCap, 20,
                line2, 2, 2, 2);
    //checkInt (numOnCone, 1, "Cone0.Line1 cone hits");
   // checkInt (numOnCap,  1, "Cone0.Line1 cap hits");
    cgWriter.EmitPoints (intersectXYZ, numOnCone + numOnCap);


    DPoint4d curve1[5]
        = {
        { 2,0,0,1},
        { -1,0,0,1},
        { -2,2,1,1},
        { 0,0,1,1},
        {0, 0, 2, 1}
        };
    int curve1Order = 5;
    cgWriter.EmitBezier (curve1, curve1Order);

    bsiDCone3d_intersectBezierCurve (&cone0,
                intersectParam, intersectXYZ, intersectUVW, &numOnCone, &numOnCap, 20,
                curve1, curve1Order, 2, 2);
    //checkInt (numOnCone, 1, "Cone0.Line1 cone hits");
    //checkInt (numOnCap,  1, "Cone0.Line1 cap hits");
    cgWriter.EmitPoints (intersectXYZ, numOnCone + numOnCap);



    //return getExitStatus();
    return 0;
    }
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <stdlib.h>

void testSingularFrustum ()
    {
    // This set of view vectors appeared in Microstation due to a faulty range which
    // forced the view origin to 1e18.  It was correctly flagged as singular by
    // the frustum map construction.
    // We experimentally find that the matrix becomes nonsingular when the "reasonable"
    // vectors are scaled by 1000.
    //
    DPoint3d origin = {
                9.4078394775920415e+018,
                9.4078394775918490e+018,
                -9.4078394775917322e+018
                };
    DPoint3d xExtent = {
                786.67263984292651,
                -786.67263984294755,
                -4.8170802391540608e-012
                };
    DPoint3d yExtent = {
                295.36000000004981,
                295.36000000003844,
                590.72000000010144
                };
    DPoint3d zExtent = {
                -1.8815678955184083e+019,
                -1.8815678955183698e+019,
                1.8815678955183469e+019
                };
    DMap4d map;
    StatusInt stat;
    stat = bsiDMap4d_initFromVectorFrustum (&map,
                    &origin, &xExtent, &yExtent, &zExtent, 1.0);
    checkBool (stat, false, "base case -- expect singular matrix");
    double f = 10.0;
    for (double s = 10.0; s < 2.0e10; s *= f)
        {
        xExtent.x *= f;
        xExtent.y *= f;
        yExtent.x *= f;
        yExtent.y *= f;
        stat = bsiDMap4d_initFromVectorFrustum (&map,
                    &origin, &xExtent, &yExtent, &zExtent, 1.0);
        if (s < 500.0)
            checkBool (stat, false, "scaled case -- expect singular matrix");
        else
            checkBool (stat, true, "scaled case -- expect invertible matrix");
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);

    //printStandardViews ();
    testSingularFrustum ();
    return getExitStatus();
    }

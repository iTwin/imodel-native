/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <stdlib.h>


void testPlaneTransformer
(
TransformR  transform,
DPlane3dR   plane0,
DPoint3dP   xyz0,
DPoint3dP   xyz1,
int         numTestPoint
)
    {
    RotMatrix matrix;
    DPlane3d plane1, plane2;
    Transform inverseTransform;
    matrix.initFrom (&transform);
    double d = matrix.determinant ();
    bool    bOrthogonal = matrix.isOrthogonal ();
    plane1.productOf (&transform, &plane0);
    inverseTransform.inverseOf (&transform);
    plane2.productOf (&inverseTransform, &plane1);
    checkDPoint3d (&plane0.origin, &plane2.origin, "plane transform -- roundtrip origin");
    checkDVec3d   (&plane0.normal, &plane2.normal, "plane transform -- roundtrip normal");

    plane1.normal.normalize ();
    transform.multiply (xyz1, xyz0, numTestPoint);

    double tol = 1e-8;
    int numPos = 0;
    int numNeg = 0;
    int numOn  = 0;
    int numOK = 0;
    for (int i = 0; i < numTestPoint; i++)
        {
        double a0 = plane0.evaluate (&xyz0[i]);
        double a1 = plane1.evaluate (&xyz1[i]);
        if (fabs (a0) < tol)
            a0 = 0.0;
        if (fabs (a1) < tol)
            a1 = 0.0;

        if (bOrthogonal)
            {
            if (fabs (a0 - a1) < tol)
                numOK++;
            }
        else if (a0 * a1 > 0.0)
            numOK++;
        else if (a0 * a1 == 0.0)
            {
            if (a0 == 0.0 && a1 == 0.0)
                numOK++;
            }

        if (a0 < 0)
            numNeg++;
        if (a0 > 0)
            numPos++;
        if (a0 == 0.0)
            numOn++;
        }

    printf (" numNeg = %d\n", numNeg);
    printf (" numPos = %d\n", numPos);
    printf (" numOn  = %d\n", numOn);
    printf (" determinant = %g\n", d);
    checkInt (numTestPoint, numOK, "Transformed plane preserves sign");
    }
void testPlaneTransformer ()
    {
    DPlane3d plane0;
    Transform transform;
#define NUMXYZ 10
    DPoint3d xyz0[NUMXYZ], xyz1[NUMXYZ];

    DPoint3d normals[] =
        {
        {1,0,0},
        {0,1,0},
        {0,0,1},
        {0.1, -0.5, 0.2},
        {3,2,1}
        };

    DPoint3d origins[] =
        {
        {0,0,0},
        {1,2,-1},
        {-2,1,2.54}
        };
    int numTestPoint = NUMXYZ;
    double dtheta = msGeomConst_2pi / numTestPoint;
    double r = 10.0;
    for (int i = 0; i < numTestPoint; i++)
        {
        xyz0[i].init (r * cos (i * dtheta), r * sin (i * dtheta), 0.0);
        }

    for (int normalIndex = 0; normalIndex < sizeof (normals) / sizeof (DVec3d); normalIndex++)
        {
        for (int originIndex = 0; originIndex < sizeof (origins) / sizeof (DVec3d); originIndex++)
            {

            plane0.initFromOriginAndNormal
                    (
                    origins[originIndex].x, origins[originIndex].y, origins[originIndex].z,
                    normals[normalIndex].x, normals[normalIndex].y, normals[normalIndex].z
                    );
            plane0.normal.normalize ();

            transform.initFromRowValues (
                    1, 0, 0,  1,
                    0, 1, 0,  2,
                    0, 0, 1, -3
                    );
            testPlaneTransformer (transform, plane0, xyz0,xyz1, numTestPoint);

            transform.initFromRowValues (
                    -1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0);
            testPlaneTransformer (transform, plane0, xyz0,xyz1, numTestPoint);

            transform.initFromRowValues (
                    0, 1, 0, 4,
                   -1, 0, 0, 2,
                    0, 0, 1, 1);
            testPlaneTransformer (transform, plane0, xyz0,xyz1, numTestPoint);

            transform.initFromRowValues (
                    1,2,3,2,
                    2,1,-2,1,
                    1,3,5,2);
            testPlaneTransformer (transform, plane0, xyz0,xyz1, numTestPoint);

            transform.initFromRowValues (
                    -1,2,3,5,
                    -2,1,-2,2,
                    -1,3,5,-1);
            testPlaneTransformer (transform, plane0, xyz0,xyz1, numTestPoint);
            }
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    selectTolerances (2);
    testPlaneTransformer ();
    return getExitStatus();
    }

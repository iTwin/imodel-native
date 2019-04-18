/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <printfuncs.h>
static double s_pi = 4.0 * atan (1.0);

// Standard view data as of July 12, 2005.
static RotMatrix s_standardViewMatrix[8] = {
    {   // Standard view matrix 1 (zero based 0)
         1.000000000000000,  0.00000000000000000,  0.00000000000000000,
         0.000000000000000,  1.00000000000000000,  0.00000000000000000,
         0.000000000000000,  0.00000000000000000,  1.00000000000000000,
    },
    {   // Standard view matrix 2 (zero based 1)
         1.000000000000000,  0.00000000000000000,  0.00000000000000000,
         0.000000000000000, -1.00000000000000000,  0.00000000000000000,
         0.000000000000000,  0.00000000000000000, -1.00000000000000000,
    },
    {   // Standard view matrix 3 (zero based 2)
         0.000000000000000, -1.00000000000000000,  0.00000000000000000,
         0.000000000000000,  0.00000000000000000,  1.00000000000000000,
        -1.000000000000000,  0.00000000000000000,  0.00000000000000000,
    },
    {   // Standard view matrix 4 (zero based 3)
         0.000000000000000,  1.00000000000000000,  0.00000000000000000,
         0.000000000000000,  0.00000000000000000,  1.00000000000000000,
         1.000000000000000,  0.00000000000000000,  0.00000000000000000,
    },
    {   // Standard view matrix 5 (zero based 4)
         1.000000000000000,  0.00000000000000000,  0.00000000000000000,
         0.000000000000000,  0.00000000000000000,  1.00000000000000000,
         0.000000000000000, -1.00000000000000000,  0.00000000000000000,
    },
    {   // Standard view matrix 6 (zero based 5)
        -1.000000000000000,  0.00000000000000000,  0.00000000000000000,
         0.000000000000000,  0.00000000000000000,  1.00000000000000000,
         0.000000000000000,  1.00000000000000000,  0.00000000000000000,
    },
    {   // Standard view matrix 7 (zero based 6)
         0.707106781186548, -0.70710678118654757,  0.00000000000000000,
         0.408248290463863,  0.40824829046386302,  0.81649658092772603,
        -0.577350269189626, -0.57735026918962573,  0.57735026918962573,
    },
    {   // Standard view matrix 8 (zero based 7)
         0.707106781186548,  0.70710678118654757,  0.00000000000000000,
        -0.408248290463863,  0.40824829046386302,  0.81649658092772603,
         0.577350269189626, -0.57735026918962573,  0.57735026918962573,
    },
    };
void printStandardViews ()
    {
    RotMatrix matrix;
    for (int viewId = 1; viewId <= 8; viewId++)
        {
        bsiDMatrix3d_getStandardRotation ((DMatrix3d*)&matrix, viewId);
        printf ("    {   // Standard view matrix %d (zero based %d)\n", viewId, viewId - 1);
        for (int i = 0; i < 3; i++)
            {
            printf ("      %20.15lf, %20.17lf, %20.17lf,\n",
                    matrix.form3d[i][0],
                    matrix.form3d[i][1],
                    matrix.form3d[i][2]);
            }
        printf ("    },\n");
        }
    }

void compareStandardViews ()
    {
    RotMatrix matrix, localMatrix;
    for (int viewId = 1; viewId <= 8; viewId++)
        {
        bsiDMatrix3d_getStandardRotation ((DMatrix3d*)&matrix, viewId);
        localMatrix = s_standardViewMatrix[viewId-1];
        checkRotMatrix (&matrix, &localMatrix, "bsiDMatrix3d_getStandardRotation vs static");
        }
    }

void checkMat (RotMatrix matrix, double thetaXA, double thetaYA, double thetaZA, double sA)
    {
    double thetaX, thetaY, thetaZ, s;
    checkTrue (bsiRotMatrix_isXRotationYRotationZRotationScale( &matrix, &thetaX, &thetaY, &thetaZ, &s),
                "isXYZs");
    checkDouble (thetaX, thetaXA, "isXYZs:thetaX");
    checkDouble (thetaY, thetaYA, "isXYZs:thetaY");
    checkDouble (thetaZ, thetaZA, "isXYZs:thetaZ");
    checkDouble (s, sA, "isXYZs:s");
    }

void checkMatrixDecomp ()
    {
    RotMatrix matrix;
    bsiRotMatrix_initFromRowValues (&matrix, 1,0,0, 0,1,0, 0,0,1);
    checkMat (matrix, 0.0, 0.0, 0.0, 1.0);
    bsiRotMatrix_initFromRowValues (&matrix, -1,0,0, 0,-1,0, 0,0,-1);
    checkMat (matrix, 0.0, 0.0, 0.0, -1.0);

    double alpha0 = -0.25;
    double alpha1 =  0.25;
    double dAlpha =  0.25;

    for (double alphaX = alpha0; alphaX <= alpha1; alphaX += dAlpha)
        {
        for (double alphaY = alpha0; alphaY <= alpha1; alphaY += dAlpha)
            {
            for (double alphaZ = alpha0; alphaZ <= alpha1; alphaZ += dAlpha)
                {
                for (double s = -3.0; s <= 3.0; s += 2.0)
                    {
                    bsiRotMatrix_rotateByPrincipleAngles (&matrix, NULL, alphaX, alphaY, alphaZ);
                    bsiRotMatrix_scaleColumns (&matrix, &matrix, s, s, s);
                    checkMat (matrix, alphaX, alphaY, alphaZ, s);
                    }
                }
            }
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);

    //printStandardViews ();
    compareStandardViews ();
    checkMatrixDecomp ();
    return getExitStatus();
    }

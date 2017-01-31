// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Geom/GeomApi.h>
#include <msgeomstructs.h>
#include <printfuncs.h>
static double s_pi = 4.0 * atan (1.0);


Private void test_RVDecomposition (int axisIndex, double degrees, int viewIndex)
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



Private void test_MSViews ()
   {
    for (int i = 1; i <= 8; i++)
        {
        DMatrix3d matrix0;
        RotMatrix matrix1;
        checkTrue (bsiDMatrix3d_getStandardRotation (&matrix0, i),
                    "get standard view");
        checkTrue (matrix0.isRigid (), "rigid");
        bsiRotMatrix_getStandardRotation (&matrix1, i);
        // OK, its ugly but true.
        // The DMatrix3d and RotMatrix data is copied in "memory" order from the same source
        // to the query destination.  So on a component basis they come out transposed ...
        for (int i = 0; i < 3; i++)
            {
            for (int j = 0; j < 3; j++)
                {
                double a0 = matrix0.getComponentByRowAndColumn (i, j);
                double a1 = matrix1.getComponentByRowAndColumn (j, i);
                checkDouble (a0, a1, "View Matrix entry");
                }
            }
        }

    }


int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    test_MSViews ();

    return getExitStatus();
    }


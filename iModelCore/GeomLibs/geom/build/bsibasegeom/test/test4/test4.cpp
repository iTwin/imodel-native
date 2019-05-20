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
#include <stdlib.h>


#include <ellipseFixup.cpp>


static double s_pi = 4.0 * atan (1.0);

Public void showDPoint3d
(
char *pName,
DPoint3d const *pXYZ
)
    {
    printf ("    DPoint3d %s = {%18.11lf, %18.11lf, %18.11lf};\n",
            pName, pXYZ->x, pXYZ->y, pXYZ->z);
    }

Public void showDouble
(
char *pName,
double a
)
    {
    printf ("    double %s = %22.14le\n",
            pName, a);
    }

void showInt
(
char *pName,
int a
)
    {
    printf ("    int %s = %d\n",
            pName, a);
    }

void showDoubleDelta
(
char *pName,
double a,
double aRef
)
    {
    printf ("    double %s = %22.14lf, %10.14lf\n",
            pName, a, a-aRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static void    roundTripDPoint3dToFloat
(
DPoint3d *pOut,
DPoint3d const *pIn,
void *pContext
)
    {
    float a;
    a = (float)pIn->x;
    pOut->x = (double)a;
    a = (float)pIn->y;
    pOut->y = (double)a;
    a = (float)pIn->z;
    pOut->z = (double)a;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static void    truncateDPoint3d001
(
DPoint3d *pOut,
DPoint3d const *pIn,
void *pContext
)
    {
    double factor = 1000;
    int a;

    a = (int) (pIn->x * factor);
    pOut->x = ((double)a)/factor;
    a = (int) (pIn->y * factor);
    pOut->y = ((double)a)/factor;
    a = (int) (pIn->z * factor);
    pOut->z = ((double)a)/factor;
    }


void testCircle3pts (char *pName, double ax, double bx, double cx, double ay, double by, double cy,
double dx,
double dy
)
    {
    DPoint3d pointA, pointB, pointC, pointB1, pointD;
    DPoint3d trueCenter, bestCenter;
    bsiDPoint3d_setXYZ (&pointA, ax, ay, 0.0);
    bsiDPoint3d_setXYZ (&pointB, bx, by, 0.0);
    bsiDPoint3d_setXYZ (&pointC, cx, cy, 0.0);
    bsiDPoint3d_setXYZ (&pointD, dx, dy, 0.0);
    DEllipse3d ellipseABC;

    bsiDEllipse3d_initFrom3DPoint3dOnArc (&ellipseABC, &pointA, &pointB, &pointC);
    printf ("// %sABC\n", pName);
    showDPoint3d ("pointA", &pointA);
    showDPoint3d ("pointB", &pointB);
    showDPoint3d ("pointC", &pointC);

    DPoint3d pointATRUNC, pointBTRUNC, pointCTRUNC;
    for (int numTest = 100; numTest < 12000; numTest *= 10)
        {
        printf ("\n FLOAT ");
        double centerTolFloat = 1.0e-6 * bsiDPoint3d_magnitude (&ellipseABC.vector0);
        double centerTol001 = 0.001;
        double tolMultiplier = 10.0;
        searchTruncatedCoordinateMidpointsToMatchCenter
                 (
                 &pointATRUNC, &pointBTRUNC, &pointCTRUNC,
                 &trueCenter, &bestCenter,
                 &pointA, &pointB, &pointC,
                 roundTripDPoint3dToFloat, NULL,
                 0.49, 0.51, numTest + 1, centerTolFloat * tolMultiplier
                 );
        printf ("\n 001 ");
        searchTruncatedCoordinateMidpointsToMatchCenter
                 (
                 &pointATRUNC, &pointBTRUNC, &pointCTRUNC,
                 &trueCenter, &bestCenter,
                 &pointA, &pointB, &pointC,
                 truncateDPoint3d001, NULL,
                 0.49, 0.51, numTest + 1, centerTol001 * tolMultiplier
                 );
        }




    showDPoint3d ("centerABC", &ellipseABC.center);

    bsiDEllipse3d_fractionParameterToDPoint3d (&ellipseABC, &pointB1, 0.5);
    DEllipse3d ellipse1, ellipseADC;

    bsiDEllipse3d_initFrom3DPoint3dOnArc (&ellipse1, &pointA, &pointB1, &pointC);
    printf ("// %sAB1C\n", pName);
    showDPoint3d ("pointB1", &pointB1);
    showDPoint3d ("centerAB1C", &ellipse1.center);

    bsiDEllipse3d_initFrom3DPoint3dOnArc (&ellipseADC, &pointA, &pointD, &pointC);
    printf ("// %sADC\n", pName);
    showDPoint3d ("pointD", &pointD);
    showDPoint3d ("centerADC", &ellipseADC.center);

    double rABC = bsiDPoint3d_magnitude (&ellipseABC.vector0);
    showDouble      ("Radius_ABC          ", rABC);
    showDoubleDelta ("distance_ABCCenter_B", bsiDPoint3d_distance (&ellipseABC.center, &pointD), rABC);
    showDoubleDelta ("Radius_AB1C         ", bsiDPoint3d_magnitude (&ellipse1.vector0), rABC);
    showDoubleDelta ("distance_ABCCenter_D", bsiDPoint3d_distance (&ellipseABC.center, &pointD), rABC);
    showDoubleDelta ("distance_ADCCenter_B", bsiDPoint3d_distance (&ellipseADC.center, &pointB), rABC);
    showDoubleDelta ("Radius_ADC          ", bsiDPoint3d_magnitude (&ellipseADC.vector0), rABC);

    showDouble (" Sweep_ABC ", ellipseABC.sweep);
    double t = tan (0.25 * ellipseABC.sweep);
    showDouble (" dRadius_dMidCoord", 1.0 / (t*t));
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);

    //printStandardViews ();
    testCircle3pts ("MS1", 155768.887, 156068.113, 155967.511, 387582.038, 387713.594, 387889.003,
                    156038.966, 387625.025);

    testCircle3pts ("MS2", 155562.525, 155964.932, 156284.794, 387685.22, 387961.23, 388167.592,
                    155920.254, 387931.504);

    testCircle3pts ("MS3", 156186.771, 156393.134, 156692.36, 387899.321, 388064.411, 388131.479,
                    156412.738, 388073.823);

    testCircle3pts ("MS2", 155562.52527255388, 155964.9325285443,
                    156284.794458254346, 387685.22032018349,
                    387961.237298129088750, 388167.5928323426192777,
                    155920.254, 387931.504);


    return getExitStatus();
    }

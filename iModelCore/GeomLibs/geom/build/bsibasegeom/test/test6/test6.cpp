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

void checkTangency
(
double r0,
double r1,
double r2,
int    index
)
    {
    double tol = 1.0e-8;

    r0 = fabs (r0);
    r1 = fabs (r1);
    r2 = fabs (r2);
    double e1 = fabs (r0 + r1 - r2);
    double e2 = fabs (fabs (r0 - r1) - r2);
    char message[128];
    sprintf (message, "Tangency Radii %d", index);
    if (e1 < e2)
        checkDouble (r0 + r1, r2, message);
    else
        checkDouble (fabs (r0 - r1), r2, message);
    }


void checkTangency (DPoint3d *pCenterIn, double r0, double r1, double r2)
    {
    double radiusIn[3];
    DPoint3d xyz[20];
    double   rr[20];
    int maxOut = 20;
    int numOut;
    radiusIn[0] = r0;
    radiusIn[1] = r1;
    radiusIn[2] = r2;
    bsiGeom_circleTTTCircleConstruction (xyz, rr, &numOut, maxOut, pCenterIn, radiusIn);
    printf ("\n CCC %lg %lg %lg\n", r0, r1, r2);
    for (int i = 0; i < numOut; i++)
        {
        double q0 = bsiDPoint3d_distance (&xyz[i], &pCenterIn[0]);
        double q1 = bsiDPoint3d_distance (&xyz[i], &pCenterIn[1]);
        double q2 = bsiDPoint3d_distance (&xyz[i], &pCenterIn[2]);
        checkTangency (q0, radiusIn[0], rr[i], i);
        checkTangency (q1, radiusIn[1], rr[i], i);
        checkTangency (q2, radiusIn[2], rr[i], i);
        }
    }

void testCCCTangent_onLine ()
    {
    DPoint3d centerIn[3];
    bsiDPoint3d_setXYZ (&centerIn[0], 0.0, 0.0, 0.0);
    bsiDPoint3d_setXYZ (&centerIn[1], 1.0, 0.0, 0.0);
    bsiDPoint3d_setXYZ (&centerIn[2], 2.0, 0.0, 0.0);
    checkTangency (centerIn, 0, 1.0, 0);
    checkTangency (centerIn, 0, 0.5, 0);
    checkTangency (centerIn, 0, 2.0, 0);

    checkTangency (centerIn, 0.2, 0, 0);

    checkTangency (centerIn, 0, 0.2, 0.1);


    checkTangency (centerIn, 0, 0, 0.2);

    checkTangency (centerIn, 0.1, 0.2, 0.1);
    checkTangency (centerIn, 0.05, 0.2, 0.1);

    bsiDPoint3d_setXYZ (&centerIn[0], 1.0, 0.0, 0.0);
    bsiDPoint3d_setXYZ (&centerIn[1], 5.0, 2.9, 0.0);
    bsiDPoint3d_interpolate (&centerIn[2], &centerIn[0], 0.38, &centerIn[1]);

    checkTangency (centerIn, 0.05, 0.2, 0.1);
    }

void testEllipseRadiusOfCurvature (double a, double b, int numStep = 8)
    {
    double thetaMax = 2.0 * atan (1.0);
    static double fTol = 1.0e-5;

    double rho0 = b * b / fabs (a);
    double rho1 = a * a / fabs (b);

    printf ("\n(a %12.6lg) (b %12.6lg)\n", a, b);
    printf ("(b*b/a = %12.8lg) (a*a/b %12.8lg)\n", rho0, rho1);
    double df = 1.0 / (double)numStep;
    for (double f = 0.0; f <= 1.0; f += df)
        {
        double theta = f * thetaMax;
        double rho = bsiGeom_ellipseRadiusOfCurvature (a, b, theta);
        double alpha;
        if (fabs (f) < fTol || fabs (f - 1.0) < fTol)
            selectTolerances (1);
        else
            selectTolerances (2);
        bool    bInverse = bsiGeom_ellipseInverseRadiusOfCurvature (&alpha, a, b, rho);
        printf ("(q %12.6lg) (rho %12.6lg) (mu %12.8lg) (diff %12.8le)\n", theta, rho, alpha, alpha - theta);
        checkDouble (alpha, theta, "inverse ellipse radius");
        selectTolerances (0);
        }

#ifdef abc
    for (double f = 0.0; f <= 1.0; f += 0.125)
        {
        double g = 0.5 + (1.0 - 1.0e-15) * (f - 0.5);
        double theta = g * thetaMax;
        double rho = ellipseRadiusOfCurvature (a, b, theta);
        double alpha;
        bool    bInverse = bsiGeom_ellipseInverseRadiusOfCurvature (&alpha, a, b, rho);
        printf ("(q %12.6lg) (rho %12.6lg) (mu %12.8lg) (diff %12.8le)\n", theta, rho, alpha, alpha - theta);
        checkDouble (alpha, theta, "inverse ellipse radius");
        }
#endif

    for (double f = -3.75; f <= 3.75; f += 0.5)
        {
        double rho = rho0 + f * (rho1 - rho0);
        double absrho = fabs (rho);
        double alpha;
        checkbool    (bsiGeom_ellipseInverseRadiusOfCurvature (&alpha, a, b, rho),
                        (absrho - rho0) * (absrho - rho1) <= 0.0,
                        "Inverse ROC - expect out of range");
        }

    }
int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    static bool sForceNumericalProblems = true;

    testCCCTangent_onLine ();
    if (sForceNumericalProblems)
        {
        testEllipseRadiusOfCurvature (104.234, 1.253);
        testEllipseRadiusOfCurvature (1.253, 104.234);
        testEllipseRadiusOfCurvature (1.253, 10.234);
        testEllipseRadiusOfCurvature (0.001, 0.0023);
        testEllipseRadiusOfCurvature (0.0023, -0.00253);
        testEllipseRadiusOfCurvature (1042.234, 1.253, 32);
        testEllipseRadiusOfCurvature (10421.234, 1.253, 64);
        testEllipseRadiusOfCurvature (104210.234, 1.253, 128);
        }

    testEllipseRadiusOfCurvature (2.0, 1.0);
    testEllipseRadiusOfCurvature (1.253, 4.234);
    return getExitStatus();
    }

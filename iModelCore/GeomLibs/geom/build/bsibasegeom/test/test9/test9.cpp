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

// Standard hyperbola function XY=1, where X and Y are pullbacks of x/w and y/w, hence xy = ww.
double fImplicitHyperbola (DVec3d &xyw)
    {
    double xx = xyw.x;
    double yy = xyw.y;
    double ww = xyw.z;
    return xx * yy - ww * ww;
    }

// Standard hyperbola function Y=X^2, where X and Y are pullbacks of x/w and y/w, hence yw=xx.
double fImplicitParabola (DVec3d &xyw)
    {
    double xx = xyw.x;
    double yy = xyw.y;
    double ww = xyw.z;
    if (ww == 0.0)
        return 0.0;
    return xx * xx - yy * ww;
    }


void testConic (DMatrix3d &matrix, double (*f)(DVec3d&), char *name)
    {
    double s0 = 0.0;
    double s1 = 0.0;
    for (double theta = 0.0; theta < 1.5; theta += 0.25)
        {
        DVec3d xyw;
        xyw.multiply (&matrix, cos(theta), sin(theta), 1);
        double fi = f(xyw);
        //printf ("%10.6lg %.5lg %.5lg %.5lg\n", theta, xyw.x / xyw.z, xyw.y / xyw.z, fi);
        double s0 = sqrt (fi * fi);
        double s1 = xyw.magnitude ();
        checkDouble (s0 / (1.0 + s1), 0.0, name);
        }

    }

void testConics ()
    {
    DMatrix3d A, B, Ainv;
    double diff;
    bsiTrig_getStandardHyperbola (&A, &Ainv);
    B.inverseOf (&A);
    diff = B.maxDiff (&Ainv);
    checkDouble (diff, 0.0, "Hyperbola matrix inverse");
    testConic (A, fImplicitHyperbola, "Hyperbola Implicit Function");
    bsiTrig_getStandardParabola (&A, &Ainv);
    B.inverseOf (&A);
    diff = B.maxDiff (&Ainv);
    checkDouble (diff, 0.0, "Parabola matrix inverse");
    testConic (A, fImplicitParabola, "Parabola Implicit Function");
    }

void testHalfAngles ()
    {
    static int numTestPerCircle = 12;
    double pi = 4.0 * atan (1.0);
    double step = 2.0 * pi / (double)numTestPerCircle;

    for (int i = 0; i < numTestPerCircle; i++)
        {
        double theta = -pi + i * step;
        double c0 = cos (theta);
        double s0 = sin (theta);
        double c1, s1;
        bsiTrig_halfAngleFunctions (&c1, &s1, c0, s0);
        double c2 = c1 * c1 - s1 * s1;
        double s2 = c1 * s1 + c1 * s1;
        checkDouble (c0, c2, "Half Angle cosine %.6g", theta);
        checkDouble (s0, s2, "Half Angle sine %.6g", theta);
        }
    }

void testRotateToSmallAngle ()
    {
    static int numTestPerCircle = 12;
    double pi = 4.0 * atan (1.0);
    double step = 2.0 * pi / (double)numTestPerCircle;

    for (int i = 0; i < numTestPerCircle; i++)
        {
        double theta = -pi + i * step;
        double c0 = cos (theta);
        double s0 = sin (theta);
        double c1, s1;
        bsiTrig_rotate90UntilSmallAngle (&c1, &s1, c0, s0);
        checkTrue (c1 > 0.0, "CCW90 to small angle c > 0 %.6g", theta);
        checkTrue (fabs (s1) < c1, "CCW90 to small angle abs(s) < abs(c) %.6g", theta);
        double dot = c0 * c1 + s0 * s1;
        checkDouble (fabs (fabs(dot) - 0.5), 0.5, "CCW90 to small angle abs(dot) - 0.5");
        }
    }

void testGivens ()
    {
    static int numTestPerCircle = 12;
    double pi = 4.0 * atan (1.0);
    double step = 2.0 * pi / (double)numTestPerCircle;

    for (double a = 0.5; a < 2.1; a *= 2.0)
        {
        for (int i = 0; i < numTestPerCircle; i++)
            {
            double theta = -pi + i * step;
            double x0 = a * cos (theta);
            double y0 = a * sin (theta);
            double c1, s1, x1, y1;
            bsiTrig_constructGivensWeights (&c1, &s1, x0, y0);
            bsiTrig_applyGivensWeights (&x1, &y1, x0, y0, c1, s1);
            checkDouble (fabs (x1), a, "Post-Givens x theta = %.6g", theta);
            checkDouble (y1, 0.0, "Post-Givens y theta = %.6g", theta);
            checkDouble (c1*c1 + s1 * s1, 1.0, "givens component magnitude = %.6g", theta);
            }
        }
    }

void testJacobi ()
    {
    DVec3d vectorU, vectorV, vectorW;
    DVec3d vectorU1, vectorV1, vectorW1;
    double c, s;
    vectorU.init (1,  0.05, 0);
    vectorV.init (0.1, 1, 0);
    bsiTrig_constructOneSided3DJacobiRotation (&c, &s, &vectorU, &vectorV);
    bsiDVec3d_applyGivensRotation (&vectorU1, &vectorV1, &vectorU, &vectorV, c, s);
    vectorW.crossProduct (&vectorU, &vectorV);
    vectorW1.crossProduct (&vectorU1, &vectorV1);
    printf (" Input  Angle %g\n", vectorU.angleTo (&vectorV));
    printf (" Result Angle %g\n", vectorU1.angleTo (&vectorV1));
    checkTrue (vectorU1.isPerpendicularTo (&vectorV1), "Jacobi rotates to perpendicular");
    checkDVec3d (&vectorW, &vectorW1, "Jacobi preserves cross product");
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testConics ();
    testHalfAngles ();
    testRotateToSmallAngle ();
    testGivens ();
    testJacobi ();
    return getExitStatus();
    }

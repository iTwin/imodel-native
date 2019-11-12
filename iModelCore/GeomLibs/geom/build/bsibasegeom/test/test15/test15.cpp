/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//#include <blocksolve.c>

#include <Geom/TriDiagonalSolver.h>

#include <mxspline.h>
#include <MXSplineUnitTest.cpp>

// Test demo for MX SPline.
void MXSplineCircleTest ()
    {
    double xArray[1000], yArray[1000];
    double pi = 4.0 * atan (1.0);
    double piOver2 = 2.0 * atan (1.0);
    //
    // Fit a portion of a circle with 10, 20, 40, 80 point splines.
    // Use combined bearing & radius end conditions
    int numPoint  = 10;
    for (int numPoint = 10; numPoint < 100; numPoint *= 2)
        {
        double theta0 = 0.2;
        double theta1 = 2.6;
        double dTheta = (theta1 - theta0) / (numPoint - 1);

        // Bearing direction for an arc is 90 degrees ahead of position ...
        double bearing0 = theta0 + piOver2;
        double bearing1 = theta1 + piOver2;
        double radius = 1.0;

        for (int i = 0; i < numPoint; i++)
            {
            double theta = theta0 + i * dTheta;
            xArray[i] = radius * cos (theta);
            yArray[i] = radius * sin (theta);
            }

        // Create end condition objects ...
        EndCondition2d ec0, ec1;
        double directionFactor = dTheta > 0.0 ? 1.0 : -1.0;
        ec0.SetDirectionRadians (bearing0);
        ec0.SetRadiusOfCurvature (radius);
        ec1.SetDirectionRadians (bearing1);
        ec1.SetRadiusOfCurvature (radius);

        MXSpline2d spline (ec0, ec1);

        printf (" MX SPline fit for data from circle\n");
        printf ("    (radius %g)\n", radius);
        printf ("    (theta0 %g) (bearing0 %g)\n", theta0, bearing0);
        printf ("    (theta1 %g) (bearing1 %g)\n", theta1, bearing1);
        printf ("    (numPoint %d)\n", numPoint);
        if (spline.FitWithArcLengthParameterization (xArray, yArray, numPoint, 1.0e-8))
            {
            //spline.Print (" MX Paramtric Spline Test Case");
            double t, x, y, mx, my;
            printf (" %3s %20s %20s %20s %20s %20s\n", "i", "t", "x", "y", "mx", "my");
            for (int i = 0; spline.GetSolutionPoint (i, t, x, y, mx, my); i++)
                {
                printf (" %3d %20.14g %20.14g %20.14g %20.14g %20.14g\n",
                            i, t, x, y, mx, my);
                }
            }
        }
    }

void DumpChainage (MXSpline2d &spline, char *pTitle)
    {
    printf (" ******** %s *********\n", pTitle);
    double t, x, y, mx, my, dx, dy;
    int numPoint = 0;
    double xAxisNorthBearing = 2.0 * atan(1.0);
    for (int i = 0; spline.GetSolutionPoint (i, t, x, y, mx, my); i++)
        numPoint++;

    printf (" %3s %20s %20s %20s %12s %20s\n", "i", "x", "y", "chainage", "bearing", "curvature");
    for (int i = 0; spline.GetSolutionPoint (i, t, x, y, mx, my); i++)
        {
        if (i < numPoint - 1)
            spline.EvalDerivativeInInterval (i, 0.0, dx, dy);
        else
            spline.EvalDerivativeInInterval (i-1, 1.0, dx, dy);
        double rc = pow (dx * dx + dy * dy, 1.5) / (dx * my - dy * mx);

        printf (" %3d %20.14g %20.14g %20.14g %12.8lf %20.14g\n",
                    i, x, y, t, xAxisNorthBearing - atan2 (dy, dx), rc);
        }
    }

void MXSplineChainageTest_1 ()
    {
    double xArray[4] = {200.0, 350.0, 450.0, 550.0};
    double yArray[4] = {650.0, 550.0, 400.0, 350.0};
    int numPoint = 4;
    double bearing0 = 0.0;
    double bearing1 = 0.0;
    double radius0 = -100.0;
    double radius1 =  100.0;

    // Create end condition objects ...
    EndCondition2d ec0, ec1;
    ec0.SetDirectionRadians (bearing0);
    ec0.SetRadiusOfCurvature (radius0);
    ec1.SetDirectionRadians (bearing1);
    ec1.SetRadiusOfCurvature (radius1);

    MXSpline2d spline (ec0, ec1);

    printf (" MX SPline fit for data from test case data --- BEARING PLUS CURVATURE \n");
    printf ("    (radius0 %g)\n", radius0);
    printf ("    (bearing0 %g)\n", bearing0);
    printf ("    (radius1 %g)\n", radius1);
    printf ("    (bearing1 %g)\n", bearing1);
    printf ("    (numPoint %d)\n", numPoint);
    if (spline.FitWithArcLengthParameterization (xArray, yArray, numPoint, 1.0e-8))
        {
        DumpChainage (spline, "BEARING PLUS CURVATURE");
        }

    }

void MXSplineChainageTest_2 ()
    {
    double xArray[4] = {200.0, 350.0, 450.0, 550.0};
    double yArray[4] = {650.0, 550.0, 400.0, 350.0};
    int numPoint = 4;
    double bearing0 = 0.0;
    double bearing1 = 0.0;
    double radius0 = -100.0;
    double radius1 =  100.0;

    // Create end condition objects ...
    EndCondition2d ec0, ec1;
    ec0.SetDirectionRadians (bearing0);
    ec1.SetDirectionRadians (bearing1);

    MXSpline2d spline (ec0, ec1);

    printf (" MX SPline fit for data from test case data -- BEARING ONLY\n");
    printf ("    (radius0 %g)\n", radius0);
    printf ("    (radius1 %g)\n", radius1);
    printf ("    (numPoint %d)\n", numPoint);
    if (spline.FitWithArcLengthParameterization (xArray, yArray, numPoint, 1.0e-8))
        {
        DumpChainage (spline, "BEARING ONLY");
        }

    }

#include <TriDiagonalSolver.h>
void tridiagonalTest ()
    {
    int numRow = 10;
    int numCol = 5;
    double yy[12][5];
    double xx[12][5];
    TriDiagonalSolver solver (numRow);
    for (int i = 0; i < numRow; i++)
        {
        double x = 0.1 * (double)i;
        solver.SetRow (i, -1.0 + 0.2 * sin(x), 2.5 + 0.5 * cos (x), -1.0 + 0.1 * sin (x + 1.0));
        xx[i][0] = x;
        xx[i][1] = 1.23 * x - 0.3;
        xx[i][2] = 1.4 * x * x + 2.3 * x - 0.3;
        xx[i][3] = (x + 2.0) * xx[i][2];
        xx[i][4] = (1.02 - x) * xx[i][3];
        }
    if (solver.MultiplyRowMajor ((double*)xx, (double*)yy, numRow, numCol)
        && solver.FactorInPlace ()
        && solver.SolveRowMajorInPlace ((double*)yy, numRow, numCol))
        {
        double ex = 0.0;
        double tol = 1.0e-12;
        printf ("\n\n TRIDIAGONAL SYSTEM ROUND TRIP\n");
        for (int i = 0; i < numRow; i++)
            {
            double dx = 0.0;
            printf ("%2d", i);
            for (int j = 0; j < numCol; j++)
                {
                dx = fabs (xx[i][j] - yy[i][j]);
                printf (" (%g %g)", xx[i][j], yy[i][j]);
                }
            if (dx > 1.0e-12)
                printf ("  (ERROR %g)", dx);
            if (dx > ex)
                ex = dx;
            printf ("\n");
            }

        if (ex < tol)
            printf ("\n\n(TRIDIAGONAL ROUND TRIP OK %g)\n", ex);
        else
            printf ("\n\n(TRIDIAGONAL ROUND TRIP ERROR %g) ************************* ERROR \n", ex);
        }
    }

int main (char **argv, int argc)
    {
    //MXSplineUnitTest ();
    //MXSplineCircleTest ();
    MXSplineChainageTest_1 ();
    MXSplineChainageTest_2 ();
    //tridiagonalTest ();
    return 0;
    }

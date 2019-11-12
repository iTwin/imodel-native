/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/




//===============================================================
// Interface (abstract virtual base class) for objects acting as generic function that can be evaluated
//   to generate test cases.
//===============================================================
class FunctionObject_double_to_double
{
public:
// Evalute function at x ....
virtual double Eval (double x) = 0;
// Evalute first derivative at x ....
virtual double EvalD1 (double x) = 0;
// Evalute second derivative at x ....
virtual double EvalD2 (double x) = 0;
// Print a summary of the functions ...
virtual void Print () = 0;
};

//===============================================================
// Function object for cubic polynomial.
//===============================================================
class Cubic : public FunctionObject_double_to_double
{
double ma0, ma1, ma2, ma3;
public:
    // Constructor: a0 + a1 x + a2 x^2 + a3 x^3
    Cubic(double a0, double a1, double a2, double a3)
        {
        ma0 = a0;
        ma1 = a1;
        ma2 = a2;
        ma3 = a3;
        }

double Eval   (double x) {return ma0 + x * (ma1 + x * (ma2 + x * ma3));}
double EvalD1 (double x) {return ma1 + x * (2.0 * ma2 + x * 3.0 * ma3);}
double EvalD2 (double x) {return 2.0 * ma2 + x * 6.0 * ma3;}

void Print ()    {printf (" (CUBIC %g + %g x + %g x^2 %g x^3)\n", ma0, ma1, ma2, ma3);}
};

//===============================================================
// Function object for cubic polynomial.
//===============================================================
class CosFunc : public FunctionObject_double_to_double
{
public:
    // Constructor:
    CosFunc()
        {
        }

double Eval   (double x) {return  cos (x);}
double EvalD1 (double x) {return -sin (x);}
double EvalD2 (double x) {return -cos (x);}

void Print ()    {printf (" (COSINE)\n");}
};

//===============================================================
// Function object for quartic polynomial.
//===============================================================
class Quartic : public FunctionObject_double_to_double
{
double ma0, ma1, ma2, ma3, ma4;
public:
   // Constructor: a0 + a1 x + a2 x^2 + a3 x^3 + z4 x^4
    Quartic (double a0, double a1, double a2, double a3, double a4)
        {
        ma0 = a0;
        ma1 = a1;
        ma2 = a2;
        ma3 = a3;
        ma4 = a4;
        }

double Eval   (double x) {return ma0 + x * (ma1 + x * (ma2 + x * (ma3 + x * ma4)));}
double EvalD1 (double x) {return ma1 + x * (2.0 * ma2 + x * (3.0 * ma3 + 4.0 * ma4));}
double EvalD2 (double x) {return 2.0 * ma2 + x * (6.0 * ma3 + 12.0 * x * ma4);}

void Print ()
    {
    printf (" (QUARTIC %g + %g x + %g x^2 + %g x^3 + %g x^4)\n", ma0, ma1, ma2, ma3, ma4);
    }
};

//===============================================================
// Object to hold tolerances and executed tests for near-zero values.
//===============================================================
class ZeroTester
{
public:
    double mAbsTol;
    double mRelTol;

ZeroTester (double absTol, double relTol)
    {
    mAbsTol = absTol;
    if (mAbsTol < 0.0)
        mAbsTol = 0.0;
    mRelTol = relTol;
    if (mRelTol < 0.0)
        mRelTol = 0.0;
    }

bool IsZero (double q)
    {
    return fabs (q) < mAbsTol;
    }

bool IsZero (double q, double qRefA, double qRefB)
    {
    q = fabs (q);
    if (q < mAbsTol)
        return true;
    if (q < mRelTol * fabs (qRefA))
        return true;
    if (q < mRelTol * fabs (qRefB))
        return true;
    return false;
    }

bool IsZero (double q, double qRef)
    {
    q = fabs (q);
    if (q < mAbsTol)
        return true;
    if (q < mRelTol * fabs (qRef))
        return true;
    return false;
    }

};



#define MAX_POINT 10
//===============================================================
// Do a 1D curve fit to points evaluated from a generic function object.
// Print out function and spline values at intermediate points.
// The x values are taken from a given array.
// The y values are evaluated from a function object.
// MULTIPLE fits can occur within a single call;  each fit uses one more
//   point out of the x array.
//===============================================================
void testFit (
// Function to be approximated
FunctionObject_double_to_double &func,
// Array of x values where function is interpolated
double *pXArray,
// SMALLEST number of points to interpolate ....
int minTestPoints,
// LARGEST number of points to interpolate ...
int maxTestPoints,
// End condition selector (same condition is used on both ends)
int endCondition,
// Test object for comparing second derivatives ....
ZeroTester &secondDerivativeTester,
// Big numbers generate more printout.
// Useful values are:
//  1 -- minimal print
//  2 -- add function values
//  90 -- spline coefficients
int printLevel = 2
)
    {
    MXSpline1d spline (0);
    printf ("\n\n CURVE FIT TESTS - end condition type %d\n", endCondition);
    printf ("\n\n    (printLevel %d)\n", printLevel);
    func.Print ();
    for (int n = minTestPoints; n <= maxTestPoints; n++)
        {
        if (n < 4 && endCondition == ECTypes::D12)
            continue;
        printf (" Curve Fit on %d points\n", n);
        double xArray[1000];
        double yArray[1000];
        // Build data points on n x values ...
        for (int i = 0; i < n; i++)
            {
            double x = pXArray[i];
            double y = func.Eval(x);
            if (printLevel > 1)
                printf (" (x %g) (y %g)\n", x, y);
            xArray[i] = x;
            yArray[i] = y;
            }

        bool solutionStat = false;
        double x0 = xArray[0];
        double x1 = xArray[n-1];
        if (endCondition == ECTypes::D2)
            {
            solutionStat = spline.SetupAndSolve
                            (
                            xArray, yArray, n,
                            0, false, func.EvalD2 (x0), true,
                            0, false, func.EvalD2 (x1), true
                            );
            }
        else if (endCondition == ECTypes::D1)
            {
            solutionStat = spline.SetupAndSolve
                            (
                            xArray, yArray, n,
                            func.EvalD1 (x0), true, 0, false,
                            func.EvalD1 (x1), true, 0, false
                            );
            }
        else if (endCondition == ECTypes::D12)
            {
            solutionStat = spline.SetupAndSolve
                            (
                            xArray, yArray, n,
                            func.EvalD1 (x0), true, func.EvalD2 (x0), true,
                            func.EvalD1 (x1), true, func.EvalD2 (x1), true
                            );
            int num1 = spline.GetCount ();
            int indexList[2];
            indexList[0] = 1;
            indexList[1] = num1 - 2;
            for (int ii = 0; ii < 2; ii++)
                {
                double xNew, yNew, mNew, yFunc;
                spline.GetPoint (indexList[ii], xNew, yNew, mNew);
                yFunc = func.Eval (xNew);
                printf (" MX Added Point for end condition (x %g) (Ytrue %g) (yspline %g) (diff %10.3le)\n",
                            xNew, yFunc, yNew, yFunc - yNew);
                }
            }
        if (!solutionStat)
            {
            printf ("   ********* TRIDIAG FAILED ************\n");
            }
        else
            {
            if (printLevel >= 90)
                spline.PrintCoffs (true);
            int newCount = spline.GetCount ();
            double dmMax = 0.0;
            if (printLevel > 1)
                printf ("Comparing exact nodal second derivative M to spline second derivative m\n");
            for (int i = 0; i < newCount; i++)
                {
                double x, y, m;
                spline.GetPoint (i, x, y, m);
                double m1 = func.EvalD2 (x);
                double dm = m - m1;
                if (fabs (dm) > dmMax)
                    dmMax = fabs (dm);
                if (printLevel > 1)
                    {
                    printf ("      %d (x %g) (M %g) (m %g)", i, x, m1, m);
                    if (!secondDerivativeTester.IsZero (dm, m, m1))
                         printf (" (D2_ERRROR %10.3e)", dm);
                    printf ("\n");
                    }
                }
            printf ("  ***** Max error in spline second derivative %10.3e\n", dmMax);
            }
        }
    }


// Fit a cubic to two points and the two second derivatives.
// Comapre spline values to function values.
void testEvaluation
(
// Function to evaluate.  If this is a cubic, we expect exact results...
FunctionObject_double_to_double &func,
// Start of evaluation interval
double x0,
// End of evaluation interval
double x1,
// number of points to evaluate in each interval ...
int numFraction,
// Toleranced zero tests ...
ZeroTester &zeroTester
)
    {
    MXSpline1d spline (2);
    double xArray[2], yArray[2];
    double y0 = func.Eval (x0);
    double y1 = func.Eval (x1);
    xArray[0] = x0;
    xArray[1] = x1;
    yArray[0] = y0;
    yArray[1] = y1;
    double m0 = func.EvalD2 (x0);
    double m1 = func.EvalD2 (x1);

    if (spline.SetupAndSolve
                    (
                    xArray, yArray, 2,
                    0.0, false, m0, true,
                    0.0, false, m1, true))
        {
        double df = 1.0 / (double)numFraction;
        printf ("\n EVAL tests\n");
        func.Print ();
        for (int i = 0; i <= (double)numFraction; i++)
            {
            double f = i * df;
            double x = x0 + f * (x1 - x0);
            double yFunc = func.Eval(x);
            double ySpline, dySpline;
            double dyFunc = func.EvalD1 (x);
            spline.EvalInInterval (0, f, ySpline);
            spline.EvalDerivativeInInterval (0, f, dySpline);
            double e0 = yFunc - ySpline;
            double e1 = dyFunc - dySpline;
            printf (" (x %g) (yFunc %g) (ySpline %g) (delta %10.3le)\n",
                            x, yFunc, ySpline, yFunc - ySpline);
            printf ("       (dyFunc %g) (dySpline %g) (delta %10.3le)\n",
                               dyFunc, dySpline, dyFunc - dySpline);
            if (   !zeroTester.IsZero (e0, dyFunc, dySpline)
                || !zeroTester.IsZero (e1, dyFunc, dySpline))
                printf ("                    ***** ERROR ****\n");

            }
        }
    }

//===============================================================
// Compute numPoints uniformly spaced points on a unit circle with mixed bearing/radius conditions.
// Print function evaluations within each interval.   We expect these to be close,
//  but not exact.
//===============================================================
void testCircleFit (int numPoints, double startAngle, double dTheta)
    {
    double xTest[1000];
    double yTest[1000];
    double r = 1.0;


    double theta0 = startAngle;
    double theta1 = theta0 + (numPoints - 1) * dTheta;

    for (int i = 0; i < numPoints; i++)
        {
        xTest[i] = r * cos(theta0 + i * dTheta);
        yTest[i] = r * sin(theta0 + i * dTheta);
        }

    double shift = 2.0 * atan (1.0);
    EndCondition2d ec0, ec1;
    double directionFactor = dTheta > 0.0 ? 1.0 : -1.0;
    ec0.SetDirectionRadians (theta0 + directionFactor * shift);
    ec0.SetRadiusOfCurvature (directionFactor * r);
    ec1.SetDirectionRadians (theta1 + directionFactor * shift);
    ec1.SetRadiusOfCurvature (directionFactor * r);
    MXSpline2d curve (ec0, ec1);
    double twoPi = 8.0 * atan(1.0);
    if (curve.FitWithArcLengthParameterization (xTest, yTest, numPoints, 1.0e-6))
        {
        int numFitPoints = curve.GetSolutionCount ();
        printf (" Curve fit to %d points on unit circle at spacing %g\n", numPoints, dTheta);
        printf (" Table of evaluated r and theta at fractional coordinates within intervals.\n");
        curve.Print (" SPLINE COEFFICIENTS");
        double drMax = 0.0;
        for (int i = 0; i < numFitPoints - 1; i++)
            {
            double x, y;
            double df = 0.25;
            for (double f = df; f + 0.f * df < 1.0; f += df)
                {
                curve.EvalInInterval (i, f, x, y);
                double ri = sqrt (x * x + y * y);
                double dri = fabs (ri - r);
                if (dri > drMax)
                    drMax = dri;
                double theta = atan2 (y, x);
                if (theta < 0.0)
                    theta += twoPi;
                printf (" (i %d) (f %g) (xy %g %g) (r theta %g %g)\n",
                            i, f, x, y,
                            ri,
                            theta
                            );
                }
            }
        printf (" Maximum error in radial position (drMax %g)\n", drMax);
        }
    }

void MXSplineUnitTest ()
    {

    //initErrorTracking (NULL, argc, argv);
    // Cubic coefficients ...
    double xxUnit [10] =
        {0,1,2,3,4, 4.2, 4.6, 5.2, 5.7, 7.8};

    double xxTight [10] =
        {
        0.1,
        0.2,
        0.3,
        0.4,
        0.42,
        0.46,
        0.52,
        0.57,
        0.78
        };

    ZeroTester highAccuracyTester (1.0e-12, 1.0e-15);

    // Compute spline fits to cubics.  These should be exact fits even
    // if the interplation points are widely spaced ....
    testFit (Cubic (1.0, 0.2, -0.3, 0.24), xxUnit, 3, 9, ECTypes::D2,  highAccuracyTester);
    testFit (Cubic (1.0, 1.0, 0.0, 0.0), xxUnit, 3, 4,   ECTypes::D2,  highAccuracyTester);
    testFit (Cubic (1.0, 1.0, 0.0, 0.0), xxUnit, 3, 4,   ECTypes::D1,  highAccuracyTester);
    testFit (Cubic (1.0, 1.0, 0.0, 0.0), xxUnit, 3, 4,   ECTypes::D12, highAccuracyTester);
    testFit (Cubic (1.0, 0.2, -0.3, 0.24), xxUnit, 3, 9, ECTypes::D1,  highAccuracyTester);
    testFit (Cubic (1.0, 0.2, -0.3, 0.24), xxUnit, 3, 9, ECTypes::D12, highAccuracyTester);


    // Compute cubic spline fits to quartics.  These are NOT exact.
    testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xxTight, 3, 4, ECTypes::D1,  highAccuracyTester, 1);
    testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xxTight, 3, 4, ECTypes::D1,  highAccuracyTester, 1);
    testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xxTight, 3, 4, ECTypes::D2,  highAccuracyTester, 1);
    testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xxTight, 3, 4, ECTypes::D12, highAccuracyTester, 1);

    testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xxTight, 3, 7, ECTypes::D1,  highAccuracyTester, 1);
    testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xxTight, 3, 7, ECTypes::D2,  highAccuracyTester, 1);
    testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xxTight, 3, 7, ECTypes::D12, highAccuracyTester, 1);

    #define MAX_ANGLES 25
    double xxTheta [MAX_ANGLES];
    int numAngles = MAX_ANGLES;
    double dTheta = 8.0 * atan (1.0) / (numAngles - 1);
    for (int i = 0; i < numAngles; i++)
        {
        xxTheta[i] = i * dTheta;
        }
    int nnn = (numAngles + 1) / 2;
    testFit (CosFunc (), xxTheta, nnn, nnn, ECTypes::D12, highAccuracyTester, 100);

    double xx[1000];
    // Compute cubic spline fits to a quartic, using 5, 10, 20, 40, 80 intervals between 0 and 1
    // We expect to see convergence ....
    for (int ecType = ECTypes::MinValid;
             ecType <= ECTypes::MaxValid;
             ecType ++)
        {
        for (int nInterval = 5; nInterval < 101; nInterval *= 2)
            {
            int numPoint = nInterval + 1;
            double dx = 1.0 / (double)nInterval;
            for (int j = 0; j < numPoint; j++)
                {
                xx[j] = j * dx;
                }
            testFit (Quartic (1.0, 0.2, -0.3, 0.24, 1.0), xx, numPoint, numPoint, ecType, highAccuracyTester, 1);
            }
        }

    // Confirm that evaluations exactly match cubics ...
    // Really simple cubics on [0,1] .....
    testEvaluation (Cubic (1,0,0,0), 0, 1, 4,    highAccuracyTester);
    testEvaluation (Cubic (0,1,0,0), 0, 1, 4,    highAccuracyTester);
    testEvaluation (Cubic (0,0,1,0), 0, 1, 4,    highAccuracyTester);
    testEvaluation (Cubic (0,0,0,1), 0, 1, 4,    highAccuracyTester);
    // Make the interval a little harder ....
    testEvaluation (Cubic (0,0,1,0), -1, 0, 4,   highAccuracyTester);
    testEvaluation (Cubic (0,0,0,1), -1, 0, 4,   highAccuracyTester);
    // All coefficients nonzero, and no zero/one interval boundaries ...
    testEvaluation (Cubic (-1,1,2,-1), -1, 2, 4, highAccuracyTester);

    // Test 2D curve fit to unit circle ....
    testCircleFit (10, 0.0, 0.1);
    testCircleFit (10, 0.0, 0.4);
    double pi = 4.0 * atan(1.0);
    testCircleFit (7, 0.0, pi / 6.0);
    testCircleFit (7, pi / 2, - pi / 6.0);
    }

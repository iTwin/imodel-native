/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <Geom/TriDiagonalSolver.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------
Spline calculations as done in MX documents.
MXSpline1d -- 1d spline, with D1, D2, and D12 end conditions.
MXSpline2d -- parametric xy xpline, based on two MXSpline2d
            with iterative solution for arc-length parameterization.
----------------------------------------------------------------------*/

// Enumeration of end conditions
class ECTypes
    {
    public:
        static const int D12 = 3;
        static const int D1 = 1;
        static const int D2 = 2;
        static const int MinValid = 1;
        static const int MaxValid = 3;
        static bool IsValidType (int i)  {return i >= MinValid && i <= MaxValid;}
    };

/*---------------------------------------------------------------------
One-Dimensional end conditions.
----------------------------------------------------------------------*/
class EndCondition1d
{
private:
    int ecType;
    double m;
    double slope;
public:
    double GetD1 () { return slope;}
    double GetD2 () { return m;}
    bool NeedsExtraPoint () {return ecType == ECTypes::D12;}
    bool GetD1Specified () {return ecType == ECTypes::D1 || ecType == ECTypes::D12;}
    bool GetD2Specified () {return ecType == ECTypes::D2 || ecType == ECTypes::D12;}
    bool ValidType () {return ECTypes::IsValidType (ecType);}
    void Init (int type, double d1, double d2)
            {
            ecType = type;
            slope = d1;
            m = d2;
            }

    void InitD1 (double slopeA) { Init (ECTypes::D1, slopeA, 0.0);}
    void InitD2 (double mA) { Init (ECTypes::D2, 0.0, mA);}
    void InitD12 (double slopeA, double mA) { Init (ECTypes::D12, slopeA, mA);}

};

/*---------------------------------------------------------------------
One-Dimensional spline
----------------------------------------------------------------------*/
class MXSpline1d
{
// Tridiagonal system
int mNumXY;
TriDiagonalSolver *pSolver;
double *pRHSCoffs;
double *pX;
double *pY;
double *pM;     // Second derivatives in solution

private:
    // Hide the copy constructor ...
    MXSpline1d () {}
public:
MXSpline1d (int numXY)
    {
    mNumXY = 0;
    pSolver = new TriDiagonalSolver (numXY);
    pRHSCoffs    = NULL;
    pX = NULL;
    pY = NULL;
    Allocate (numXY);
    }

void ReleaseMemory ()
    {
    if (mNumXY != 0)
        {
        pSolver->ReleaseMem ();
        delete [] pRHSCoffs;
        delete [] pX;
        delete [] pY;
        delete [] pM;
        mNumXY = 0;
        }
    }

~MXSpline1d ()
    {
    ReleaseMemory ();
    delete pSolver;
    }

// Return the actual count.  Due to insertion of artificial end condition points,
//  this may be different from the nominal point counts supplied in primary input.
int GetCount ()
    {
    return mNumXY;
    }

// Allocate memory for fitting n points.
void Allocate (int n)
    {
    if (n <= 0)
        {
        ReleaseMemory();
        return;
        }

    if (n != mNumXY)
        {
        ReleaseMemory();
        mNumXY = n;
        pRHSCoffs = new double[mNumXY];
        pSolver->Allocate(mNumXY);
        pX = new double[mNumXY];
        pY = new double[mNumXY];
        pM = new double[mNumXY];
        }

    memset (pRHSCoffs,    0, mNumXY * sizeof (double));
    memset (pX,           0, mNumXY * sizeof (double));
    memset (pY,           0, mNumXY * sizeof (double));
    memset (pM,           0, mNumXY * sizeof (double));
    }

// Return x, y and second derivative value at specified index.
bool GetPoint (int pointIndex, double &x, double &y, double &m)
    {
    if (pointIndex >= 0 && pointIndex < mNumXY)
        {
        x = pX[pointIndex];
        y = pY[pointIndex];
        m = pM[pointIndex];
        return true;
        }
    x = y = m = 0.0;
    return false;
    }

// Return the derivative with respect to global x at a point specified as
// a fraction within an interval.
bool EvalDerivativeInInterval (int pointIndex, double f, double &dydx)
    {
    if (pointIndex >= 0 && pointIndex < mNumXY - 1)
        {
        double g = 1.0 - f;
        double x0, x1, y0, y1, m0, m1;
        GetPoint (pointIndex, x0, y0, m0);
        GetPoint (pointIndex + 1, x1, y1, m1);
        double h = x1 - x0;
        double dydf = (y1 - y0)
               + (h * h / 6.0) * (m0 * (1.0 - 3.0 * g * g) + m1 * (3.0 * f * f - 1.0));
        dydx = dydf / h;
        return true;
        }
    dydx = 0.0;
    return false;
    }

bool EvalInInterval (int pointIndex, double f, double &y)
    {
    if (pointIndex >= 0 && pointIndex < mNumXY - 1)
        {
        double g = 1.0 - f;
        double x0, x1, y0, y1, m0, m1;
        GetPoint (pointIndex, x0, y0, m0);
        GetPoint (pointIndex + 1, x1, y1, m1);
        double h = x1 - x0;
        y = y0 + f * (y1 - y0)
               + (h * h / 6.0) * (m0 * g * (g * g - 1.0)  + m1 * f * (f * f - 1.0));
        return true;
        }
    y = 0.0;
    return false;
    }

void SetMatrixRow
(
int row,
double aLeft,
double aDiag,
double aRight,
double bVal
)
    {
    // exit silently if bad row ...
    if (row < 0 || row >= mNumXY)
        return;
    pSolver->SetRow (row, aLeft, aDiag, aRight);
    pRHSCoffs[row]    = bVal;
    }

void SetXY
(
int index,
double x,
double y
)
    {
    if (index >= 0 && index < mNumXY)
        {
        pX[index] = x;
        pY[index] = y;
        }
    }

void SetXY
(
double *pXi,
double *pYi,
int    numXY
)
    {
    Allocate (numXY);
    for (int i = 0; i < numXY; i++)
        SetXY (i, pXi[i], pYi[i]);
    }


// Setup the continuity conditions for interior points.
// Skip specified number of points at each end to allow for other
//  equations for end conditions.
void SetupStandardSecondDerivativeSystemInteriorRows
(
int numSkipStart = 1,
int numSkipEnd = 1
)
    {
    for (int i = numSkipStart; i < mNumXY - numSkipEnd; i++)
        {
        double hA = pX[i] - pX[i-1];
        double hB = pX[i+1] - pX[i];
        double dyA = pY[i] - pY[i-1];
        double dyB = pY[i+1] - pY[i];
        SetMatrixRow (i, hA / 6.0, (hA + hB) / 3.0, hB / 6.0,      dyB / hB - dyA / hA);
        }
    }

// Insert a second derivative condition at specified row.
void SetupSecondDerivativeCondition
(
int pointIndex,
double m
)
    {
    SetMatrixRow (pointIndex, 0.0, 1.0, 0.0, m);
    }

// Insert a second derivative condition at specified row.
void SetupFirstDerivativeCondition
(
int pointIndex,
double slope
)
    {
    if (pointIndex == 0)
        {
        // Enforce condition between pointIndex and successor ...
        double h = pX[1] - pX[0];
        double dy = pY[1] - pY[0];
        SetMatrixRow (0, 0.0, h / 3.0, h / 6.0, dy / h - slope);
        }
    else
        {
        // Enforce condition between pointIndex and predecessor ...
        double h = pX[pointIndex] - pX[pointIndex - 1];
        double dy = pY[pointIndex] - pY[pointIndex - 1];
        SetMatrixRow (pointIndex, h / 6.0, h / 3.0, 0.0, slope - dy / h);
        }
    }

// ASSUME x,y arrays are allocated with space for the artificial point !!!
void SetupDualEndCondition
(
int pointIndex,
double slope,
double m
)
    {
    SetupSecondDerivativeCondition (pointIndex, m);
    if (pointIndex == 0)
        {
        double xNew = (pX[0] + pX[2]) * 0.5;
        pX[1] = xNew;
        double hA = pX[1] - pX[0];
        double hB = pX[2] - pX[1]; // Same as hA
        double hC = pX[3] - pX[2];
        double dyAB = pY[2] - pY[0];
        double dyC = pY[3] - pY[2];
        SetMatrixRow (0, 0.0, 1.0, 0.0, m);
        SetMatrixRow (1, 0.0, hA, hA/6.0,
                        dyAB / hA - 2.0 * slope - 5.0 * hA * m / 6.0);
        SetMatrixRow (2, 0.0, (hB + hC)/3.0, hC / 6.0,
                        dyC / hC - dyAB / hB + slope + hB * m / 3.0);
        }
    else if (pointIndex == mNumXY - 1)
        {
        // n matches role of point count in 1-based indices ...
        int n = mNumXY - 1;
        double xNew = (pX[n] + pX[n - 2]) * 0.5;
        pX[n - 1] = xNew;
        double hA = pX[n    ] - pX[n - 1];
        double hB = pX[n - 1] - pX[n - 2];
        double hC = pX[n - 2] - pX[n - 3];

        double dyBA = pY[n] - pY[n - 2];
        double dyC  = pY[n - 2] - pY[n - 3];
        SetMatrixRow (n  , 0.0, 1.0, 0.0, m);
        SetMatrixRow (n-1,  hA / 6.0, hA, 0.0,
                        2.0 * slope - 5.0 * hA * m/  6.0 - dyBA / hA);
        SetMatrixRow (n-2, hC / 6.0, (hB + hC) / 3.0, 0.0,
                        hA / 3.0 * m - slope + dyBA / hA - dyC / hC);
        }
    }

// Full setup with mixed end conditions -- actual point indices may be adjusted for insertion
//   of dummy points for iterations.
bool SetupAndSolve
(
double *pXi,
double *pYi,
int    numXY,
double slope0,
bool bSetSlope0,
double m0,
bool   bSetM0,
double slope1,
bool bSetSlope1,
double m1,
bool bSetM1
)
    {
    if (!bSetM0 && !bSetSlope0)
        return false;
    if (!bSetM1 && !bSetSlope1)
        return false;

    int numExpanded = numXY;
    if (bSetM0 && bSetSlope0)
        numExpanded++;
    if (bSetM1 && bSetSlope1)
        numExpanded++;
    Allocate (numExpanded);
    int count = 0;
    SetXY (count++, pXi[0], pYi[0]);
    // Artificial point for iterations ...
    if (bSetM0 && bSetSlope0)
        SetXY (count++, (pXi[0] + pXi[1]) * 0.5, (pYi[0] + pYi[1]) * 0.5);

    for (int i = 1; i < numXY - 1; i++)
        SetXY (count++, pXi[i], pYi[i]);
    if (bSetM1 && bSetSlope1)
        SetXY (count++, (pXi[numXY-2] + pXi[numXY-1]) * 0.5, (pYi[numXY-2] + pYi[numXY-1]) * 0.5);
    SetXY (count++, pXi[numXY-1], pYi[numXY-1]);

    int numSkip0 = 1;
    if (bSetM0 && bSetSlope0)
        numSkip0 = 3;

    int numSkip1 = 1;
    if (bSetM1 && bSetSlope1)
        numSkip1 = 3;

    SetupStandardSecondDerivativeSystemInteriorRows (numSkip0, numSkip1);

    if (bSetM0 && bSetSlope0)
        SetupDualEndCondition (0, slope0, m0);
    else if (bSetM0)
        SetupSecondDerivativeCondition (0, m0);
    else if (bSetSlope0)
        SetupFirstDerivativeCondition (0, slope0);

    if (bSetM1 && bSetSlope1)
        SetupDualEndCondition (numExpanded - 1, slope1, m1);
    else if (bSetM1)
        SetupSecondDerivativeCondition (numExpanded - 1, m1);
    else if (bSetSlope1)
        SetupFirstDerivativeCondition (numExpanded - 1, slope1);

    if (!Solve ())
        return false;

    // Fixup the artifical y values ...
    double xk, yk, mk;
    if (bSetM0 && bSetSlope0)
        {
        GetPoint (1, xk, yk, mk);
        double hA = pX[1] - pX[0];
        pY[1] = pY[0] + slope0 * hA + hA * hA * (m0 / 3.0 + mk / 6.0);
        }

    if (bSetM1 && bSetSlope1)
        {
        GetPoint (numExpanded - 2, xk, yk, mk);
        double hA = pX[numExpanded - 1] - pX[numExpanded - 2];
        pY[numExpanded - 2] = pY[numExpanded - 1] - slope1 * hA + hA * hA * (m1 / 3.0 + mk / 6.0);
        }

    return true;
    }

bool SetupAndSolve
(
double *pXi,
double *pYi,
int     numXY,
EndCondition1d &ec0,
EndCondition1d &ec1
)
    {
    return SetupAndSolve (pXi, pYi, numXY,
                ec0.GetD1 (), ec0.GetD1Specified (),
                ec0.GetD2 (), ec0.GetD2Specified (),
                ec1.GetD1 (), ec1.GetD1Specified (),
                ec1.GetD2 (), ec1.GetD2Specified ()
                );
    }

bool Solve ()
    {
    for (int i = 0; i < mNumXY; i++)
        pM[i] = pRHSCoffs[i];
    if (pSolver->FactorInPlace ()
        && pSolver->SolveRowMajorInPlace (pM, mNumXY, 1))
        return true;
    return false;
    }

void PrintXYM (char const *pMessage, char const *pXName, char const *pYName)
    {
    printf (" %s (%d points)\n", pMessage, GetCount ());
    double x, y, m;
    for (int i = 0; GetPoint (i, x, y, m); i++)
        {
        GetPoint (i, x, y, m);
        printf (" (%s %20.12g) (%s %20.12g) (m %20.12g)\n", pXName, x, pYName, y, m);
        }
    }
void PrintCoffs (bool bCheck)
    {
    //pSolver->PrintCoffs ("MX Spline");
    }
};

class EndCondition2d
{
    bool   mbDirection;
    bool   mbRadius;
    double mDirectionRadians;
    double mRadius;

public:
    EndCondition2d ()
        {
        mbDirection = false;
        mbRadius = false;
        mRadius = 0.0;
        mDirectionRadians = 0.0;
        }

    bool IsDualCondition ()
        {
        return mbRadius && mbDirection;
        }

    bool GetDirection (double &ux, double &uy)
        {
        double radians = mDirectionRadians;
        ux = cos (radians);
        uy = sin (radians);
        return mbDirection;
        }

    bool GetInverseRadius (double &inverseRadius)
        {
        if (mRadius == 0.0)
            inverseRadius = 0.0;
        else
            inverseRadius = 1.0 / mRadius;
        return mbDirection;
        }

   void SetDirectionRadians (double radians)
        {
        mbDirection = true;
        mDirectionRadians = radians;
        }

    // Set radius.   Radius of zero is interpretted as straight line.
    void SetRadiusOfCurvature (double radius)
        {
        mbRadius = true;
        mRadius = radius;
        }

    bool Get1dEndConditions
        (
        EndCondition1d &ecX,
        EndCondition1d &ecY
        )
        {
        if (mbRadius && mbDirection)
            {
            double ux, uy;
            double inverseRadius;
            GetDirection (ux, uy);
            GetInverseRadius (inverseRadius);
            ecX.InitD12 (ux, -uy * inverseRadius);
            ecY.InitD12 (uy,  ux * inverseRadius);
            return true;
            }
        else if (mbDirection)
            {
            double ux, uy;
            GetDirection (ux, uy);
            ecX.InitD1 (ux);
            ecY.InitD1 (uy);
            return true;
            }
        return false;
        }

};

//===============================================================
// MX 2D Parametric spline fits.
//Methods are to be called in this sequence:
// STEP A) Constructor:    spline = MXSpline2d (StartCondition, EndCondition)
// STEP B) Point Input:    spline.FitWithArcLengthParameterization (xArray, yArray, numPoints, relTol)
// STEP C) Queries:
//              spline.GetSolutionCount () --- actual point count is INCREASED for mixed end conditions  !!!
//              spline.EvalInInterval (interval, fraction, x, y) --- Evaluate x,y at FRACTIONAL coordinate in
//                      interval.
//===============================================================
class MXSpline2d
{
private:
    MXSpline1d mxSpline;
    MXSpline1d mySpline;
    EndCondition2d ec2d[2];
    EndCondition1d ec1d[2][2];  // [startEnd, xy].   DERIVED FROM ec2d !!!
    MXSpline2d ();  // Prevent copying.

    int mMaxArcSteps;


    // Compute the (APPROXIMATE) arc length of a single interval.
    bool EvalIntervalArcLength (int pointIndex, double &arcLength, int numInterval)
        {
        #define MAX_SIMPSON_INTERVAL 200
        double t0, x0, m0;
        double t1, x1, m1;
        if (!mxSpline.GetPoint (pointIndex, t0, x0, m0)
            || !mxSpline.GetPoint (pointIndex + 1, t1, x1, m1))
            return false;
        double h = t1 - t0; // ASSUME mySpline has same "x" parameters !!!

        double tangentLength[2 * MAX_SIMPSON_INTERVAL + 1];
        if (numInterval > MAX_SIMPSON_INTERVAL)
            numInterval = MAX_SIMPSON_INTERVAL;
        int numEval = 2 * numInterval + 1;
        double df = 1.0 / (double)(numEval - 1);
        double dx, dy, f;
        for (int i = 0; i < numEval; i++)
            {
            f = i * df;
            if (   !mxSpline.EvalDerivativeInInterval (pointIndex, f, dx)
                || !mySpline.EvalDerivativeInInterval (pointIndex, f, dy))
                return false;
            tangentLength[i] = sqrt (dx * dx + dy * dy);
            }
        arcLength = 0.0;
        for (int i = 2; i < numEval; i += 2)
            {
            arcLength   +=       tangentLength[i-2]
                        +  4.0 * tangentLength[i-1]
                        +        tangentLength[i];
            }
        arcLength *= h / (6.0 * numInterval);
        return true;
        }

    // Recompute arc lengths from current spline.
    // Return the maximum change in any interval.
    double UpdateArcLengths (double *pT, int numXY)
        {
        double maxChange = 0.0;
        int lastInterval = numXY - 2;
        int k = 0;
        double oldT0 = pT[0];
        double oldT1;
        int numSimpsonInterval = 10;
        for (int i = 0; i <= lastInterval; i++, oldT0 = oldT1)
            {
            oldT1 = pT[i+1];
            int numPointsThisInterval = 1;
            if (   (i == 0 && ec2d[0].IsDualCondition ())
                || (i == lastInterval && ec2d[1].IsDualCondition ()))
                numPointsThisInterval = 2;
            double localLength = 0.0;
            double a;
            for (int subIntervalIndex = 0; subIntervalIndex < numPointsThisInterval; subIntervalIndex++)
                {
                EvalIntervalArcLength (k++, a, numSimpsonInterval);
                localLength += a;
                }
            double oldLocalLength = oldT1 - oldT0;
            double localChange = fabs (localLength - oldLocalLength);
            if (localChange > maxChange)
                maxChange = localChange;
            pT[i+1] = pT[i] + localLength;
            }
        return maxChange;
        }

    // Innitial curve fit with
    bool FitWithArcLengthIterations (double *pX, double *pY, int numXY, double relTol)
        {
        bool fitOK = false;
        if (ConvertEndConditions ())
            {
            // EDL March 2022 This was local pT same name as parameter.  Hence parameter is never used.
            // Using a local for the iteration makes sense.  But why is the parameter there at all?
            double *pTLocal = new double [numXY];
            // Initially use chord length as parameter ...
            pTLocal[0] = 0.0;
            for (int i = 1; i < numXY; i++)
                {
                double dx = pX[i] - pX[i-1];
                double dy = pY[i] - pY[i-1];
                pTLocal[i] = pTLocal[i-1] + sqrt (dx * dx + dy * dy);
                }
            double tMax = pTLocal[numXY-1];
            //bool stat = false;

            for (int arcStep = 0; arcStep < mMaxArcSteps; arcStep++)
                {
                if (   !mxSpline.SetupAndSolve (pTLocal, pX, numXY, ec1d[0][0], ec1d[1][0])
                    || !mySpline.SetupAndSolve (pTLocal, pY, numXY, ec1d[0][1], ec1d[1][1]))
                    break;
                //mxSpline.PrintXYM (" XXXXXX ");
                //mySpline.PrintXYM (" YYYYYY ");
                double arcChange = UpdateArcLengths (pTLocal, numXY);
                if (arcChange < relTol * tMax)
                    {
                    fitOK = true;
                    break;
                    }
                }
            delete [] pTLocal;
            }
        return fitOK;
        }

    // Convert from 2D end conditions to 1D conditions ....
    bool ConvertEndConditions ()
        {
        for (int i = 0; i < 2; i++)
            if (!ec2d[i].Get1dEndConditions (ec1d[i][0], ec1d[i][1]))
                return false;
        return true;
        }

public:
    // CONSTRUCTOR -- End conditions must be supplied right away.
    MXSpline2d (EndCondition2d &startCondition, EndCondition2d &endCondition)
        : mxSpline (0), mySpline(0)
        {
        ec2d[0] = startCondition;
        ec2d[1] = endCondition;
        mMaxArcSteps = 30;
        }

    // STEP B -- curve fit.
    bool FitWithArcLengthParameterization (double *pX, double *pY, int numXY, double relTol)
        {
        bool fitOK = false;
        if (ConvertEndConditions ())
            {
            fitOK = FitWithArcLengthIterations (pX, pY, numXY, relTol);
            }
        return fitOK;
        }
#ifdef MS_DPOINT2D_DEFINED
    bool FitWithArcLengthParameterization (DPoint2d *pXYArray, int numXY, double relTol)
        {
        bool fitOK;
        double *pX = new double [numXY];
        double *pY = new double [numXY];
        for (int i = 0; i < numXY; i++)
            {
            pX[i] = pXYArray[i].x;
            pY[i] = pXYArray[i].y;
            }
        fitOK = FitWithArcLengthParameterization (pX, pY, numXY, relTol);
        delete [] pX;
        delete [] pY;
        return fitOK;
        }
#endif
    int GetSolutionCount ()
        {
        return mxSpline.GetCount ();
        }

    bool EvalInInterval (int i, double f, double &x, double &y)
        {
        return mxSpline.EvalInInterval (i, f, x)
            && mySpline.EvalInInterval (i, f, y);
        }

    bool EvalDerivativeInInterval (int i, double f, double &dx, double &dy)
        {
        return mxSpline.EvalDerivativeInInterval (i, f, dx)
            && mySpline.EvalDerivativeInInterval (i, f, dy);
        }

    void Print (char const *pTitle)
        {
        printf (" %s\n", pTitle);
        mxSpline.PrintXYM (" Parametric fit", "arc length", "X");
        mySpline.PrintXYM (" Parametric fit", "arc length", "Y");
        double dx0, dy0;
        double dx1, dy1;
        EvalDerivativeInInterval (0, 0.0, dx0, dy0);
        EvalDerivativeInInterval (GetSolutionCount () - 2, 1.0, dx1, dy1);
        printf(" Start direction (%g,%g) (bearing %20.15g)\n", dx0, dy0, atan2 (dy0, dx0));
        printf("   End direction (%g,%g) (bearing %20.15g)\n", dx1, dy1, atan2 (dy1, dx1));
        }

    // Get arc length x,y and second derivatives
    bool GetSolutionPoint
    (
    int pointIndex,
    double &arcLength,
    // x coordinate
    double &x,
    // y coordinate
    double &y,
    // 2nd derivative of x
    double &mx,
    // 2nd derivative of y
    double &my
    )
        {
        return mxSpline.GetPoint (pointIndex, arcLength, x, mx)
            && mySpline.GetPoint (pointIndex, arcLength, y, my);
        }
};

END_BENTLEY_GEOMETRY_NAMESPACE
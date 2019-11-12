/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>

#include <Geom/DSpiral2d.h>
#include <Geom/DSpiral2dBase.h>

#define MAX_CLOTHOID_POINTS 2000

static double sMaxRadians = 0.08;

void StrokeAndPrint
(
DSpiral2d &spiral,
DSpiral2dBase &spiral1,
double theta0,
double K0,
double theta1,
double K1,
double xx,
double yy
)
    {
    DVec2d clothoidPoints[MAX_CLOTHOID_POINTS];
    DVec2d clothoidPoints1[MAX_CLOTHOID_POINTS];
    int numClothoidPoints, numClothoidPoints1;
    double errorBound = 0.0;
    if (   spiral.SetBearingAndCurvatureLimits (theta0, K0, theta1, K1)
        && spiral1.SetBearingAndCurvatureLimits (theta0, K0, theta1, K1))
        {
        printf (" SPIRAL (theta0 %lf) (K0 %lf) (beta0 %lf)\n", spiral.mTheta0, spiral.mCurvature0,
                            spiral.DistanceToGlobalAngle (0.0));
        printf ("        (theta1 %lf) (K1 %lf) (beta1 %lf)\n", spiral.mTheta1, spiral.mCurvature1,
                            spiral.DistanceToGlobalAngle (spiral.mLength));
        printf ("        (L %lf) (type %d)\n", spiral.mLength, spiral.mSpiralType);
        spiral.Stroke (spiral, sMaxRadians,
                    clothoidPoints, numClothoidPoints, MAX_CLOTHOID_POINTS, errorBound);
        DSpiral2dBase::Stroke (spiral1, sMaxRadians,
                    clothoidPoints1, numClothoidPoints1, MAX_CLOTHOID_POINTS, errorBound);
        printf ("PLACE SMARTLINE\n");
        printf ("xy=%.10lf,%.10lf\n", xx, yy);
        for (int i = 0; i < numClothoidPoints; i++)
            {
            double bearing = 0.0;
            if (i > 0)
                {
                double dx = clothoidPoints[i].x - clothoidPoints[i-1].x;
                double dy = clothoidPoints[i].y - clothoidPoints[i-1].y;
                bearing = atan2 (dy, dx);
                }
            printf ("xy=(%.10lf,%.10lf) (%.10lf,%.10lf) (%.12lf) (%.4lf)\n",
                    xx + clothoidPoints [i].x, yy + clothoidPoints [i].y,
                    xx + clothoidPoints1[i].x, yy + clothoidPoints1[i].y,
                    clothoidPoints[i].distance (&clothoidPoints1[i]), bearing

                    );
            }
        printf("                     finall error bound %8.2le\n", errorBound);
        }
    }


void PrintLineStrings (DSpiral2d &spiral, DSpiral2dBase &virtualSpiral)
    {
    double pi = 4.0 * atan (1.0);
    double betaMax =   pi;
    double K0 = -0.5;
    double theta0 = 0.0;
    double theta1 = 2.0 * pi;
    double xx = 0.0;
    double yy = 4.0;
    for (double sTheta = -1.0; sTheta < 1.5; sTheta += 2.0)
        for (double sK = -1.0; sK < 1.5; sK += 2.0)
            for (double K1 = 1.0; K1 < 1.1; K1 *= 2.0)
                {
//                double beta0 = sTheta * theta1;
//                double beta1 = theta0;
//                double beta2 = beta0 + 1.5 * (beta1 - beta0);
//                double H1 = sK * K1;
//                double H0 = H1 + 1.5 * (K0 - H1);
//                StrokeAndPrint (spiral, beta2, H1, beta0, H0, xx, yy);
                StrokeAndPrint (spiral, virtualSpiral, theta0, sK * K0, sTheta * theta1, sK * K1, xx, yy);
                //yy += 3.0;
                }
    }

static int sColorCurve     = 2;
static int sColorBox       = 0;
static int sColorGrid      = 32;
static int sStyleSolid     = 0;
static int sStyleLongDash  = 3;
static int sStyleShortDash = 5;

void emitColorStyle (int color, int style){printf ("ACTIVE COLOR %d\nACTIVE STYLE %d\n", color, style);}

void emitText (char *pString, double x, double y)
    {
    }

void emitPoint (double x, double y)
    {
    printf ("xy=%g,%g\n", x, y);
    }
void startPolyline (){printf ("PLACE SMARTLINE\n");}
void endPolyline   (){printf ("PLACE SMARTLINE\n");}

void makeBox (double x0, double y0, double a, double b)
    {
    startPolyline ();
    emitPoint (x0, y0);
    emitPoint (x0 + a, y0);
    emitPoint (x0 + a, y0 + b);
    emitPoint (x0, y0 + b);
    emitPoint (x0, y0);
    endPolyline ();
    }

struct BoxContext
{
double mXSize;
double mYSize;
double mMindF;
double mGridDF;
int    mNumSubGrid;
BoxContext (double xSize, double ySize, double mindF, double gridDF, int numSubGrid)
    {
    mXSize = xSize;
    mYSize = ySize;
    mMindF = mindF;
    mGridDF = gridDF;
    mNumSubGrid = numSubGrid;
    }

double mX0;
double mY0;
double mUMin, mUMax, mFMin, mFMax;

double UToX (double u)
    {
    return mX0 + mXSize * (u - mUMin) / (mUMax - mUMin);
    }

double FToY (double f)
    {
    return mY0 + mYSize * (f - mFMin) / (mFMax - mFMin);
    }

void emitUF (double u, double f)
    {
    emitPoint (UToX (u), FToY (f));
    }

void SetupMapping (double uMin, double uMax, double fMin, double fMax)
    {
    double fMid = 0.5 * (fMax + fMin);
    double a;
    if (fMax - fMin < mMindF)
        {
        a = mMindF * 0.55;
        }
    else
        {
        a = 0.55 * (fMax - fMin);
        }
    mFMin = fMid - a;
    mFMax = fMid + a;
    mUMin = uMin;
    mUMax = uMax;
    }

// Emit a box, grid lines, and a linestring.
void scaleAndPrint
(
char *pName,
double *pU0,    // Complete range for U data
int    numU0,
double *pU,     // Plotted U data
double *pF,     // Plotted V data
int numU,       // Number of U and F
double x0,      // lower left of box
double y0
)
    {
    mX0 = x0;
    mY0 = y0;
    double uMin, uMax, fMin, fMax;
    bsiDoubleArray_minMax (&uMin, &uMax, pU0, numU0);
    bsiDoubleArray_minMax (&fMin, &fMax, pF, numU);
    SetupMapping (uMin, uMax, fMin, fMax);
    emitColorStyle (sColorBox, sStyleSolid);
    makeBox (x0, y0, mXSize, mYSize);

    emitColorStyle (sColorCurve, sStyleSolid);
    startPolyline ();
    for (int i = 0; i < numU; i++)
        {
        emitUF (pU[i], pF[i]);
        }
    endPolyline ();

    int iF0 = (int)floor (mFMin / mGridDF);
    int iF1 = (int)ceil  (mFMax / mGridDF);
    int iFStep = 1;
    int numFStep = iF1 - iF0;
    while (numFStep / iFStep > 20)
        iFStep *= 10;
    if (numFStep / iFStep > 10)
        iFStep *= 5;
    double dU = mUMax - mUMin;
    int numPrimary = iF1 - iF0;
    int numSub = mNumSubGrid;
    if (iFStep > 1 || numSub * numPrimary > 50)
        {
        numSub = 1;
        }
    else
        {
        while (numSub * numPrimary > 20 && ((numSub & 0x01) == 0))
            {
            numSub /= 2;
            }
        if (numSub * numPrimary > 20)
            numSub = 1;
        }

    double eF  = mGridDF / (numSub > 0 ? numSub : 1);
    double setbackFraction = 0.02;
    for (int i = iF0; i <= iF1; i += iFStep)
        {
        double f0 = i * mGridDF;
        for (int j = 0; j < numSub; j++)
            {
            double f = f0 + j * eF;
            if (f > mFMin && f < mFMax)
                {
                startPolyline ();
                emitColorStyle (sColorGrid, j == 0 ? sStyleLongDash : sStyleShortDash);
                emitUF (mUMin + setbackFraction * dU, f);
                emitUF (mUMin + (1.0 - setbackFraction) * dU, f);
                endPolyline ();
                }
            }
        }
    }
};



void StepSizeTests (
        char *pName,
        DSpiral2dBase &spiral,
        double theta0,
        double K0,
        double theta1,
        double K1,
        double minStep,
        double maxStep,
        double stepFactor,
        double stepIncrement
        )
    {
    DVec2d uv[1000];
    spiral.SetBearingAndCurvatureLimits (theta0, K0, theta1, K1);
    printf ("** %s\n", pName);
    printf ("     Length %g\n", spiral.mLength);
    printf ("     theta0 %g    K0 %g\n", theta0, K0);
    printf ("     theta1 %g    K1 %g\n", theta1, K1);
    double error;
    int count;
    for (double stepSize = minStep;
            stepSize <= maxStep;
            stepSize = stepSize * stepFactor + stepIncrement)
        {
        DSpiral2dBase::Stroke (spiral, stepSize, uv, count, 1000, error);
        printf ("   (stepSize %.3g) (error %.2g)\n", stepSize, error);
        }
    }

static int sNumDistance = 32;
void PrintDistanceFunctions (
        char *pName,
        DSpiral2dBase &spiral,
        double theta0, double K0,
        double theta1, double K1,
        double x0,
        double y0
        )
    {
    double boxX = 1.0;
    double boxY = 1.0;
    BoxContext bc(boxX, boxY, 1.0, 1.0, 10);
    emitText (pName, x0, y0 + boxY * 1.1);
    spiral.SetBearingAndCurvatureLimits (theta0, K0, theta1, K1);
    double L = spiral.mLength;
    double fraction[1000];
    double curvature[1000];
    double theta[1000];
    double dK[1000];
    double ddK[1000];
    double fdK[1000];
    double fddK[1000];

    double beta = 4.0 * msGeomConst_pi;
    for (int i = 0; i <= sNumDistance; i++)
        {
        double u = i / (double)sNumDistance;
        fraction[i]  = u - sin (u * beta) / beta;
        double d = fraction[i] * L;
        curvature[i] = spiral.DistanceToCurvature (d);
        theta[i]     = spiral.DistanceToLocalAngle (d);
        }

    double h = 1.0 / sNumDistance;
    for (int i = 0; i < sNumDistance; i++)
        {
        fdK[i] = 0.5 * (fraction[i+1] + fraction[i]);
        dK[i] = (curvature[i+1] - curvature[i]) / (fraction[i+1] - fraction[i]);
        }
    for (int i = 0; i < sNumDistance - 1; i++)
        {
        fddK[i] = 0.5 * (fdK[i+1] + fdK[i]);
        ddK[i] = (dK[i+1] - dK[i]) / (fdK[i+1] - fdK[i]);
        }

    int numData = sNumDistance + 1;
    bc.scaleAndPrint ("Theta",  fraction, numData, fraction, theta,     numData,     x0, y0 + 4);
    bc.scaleAndPrint ("K",      fraction, numData, fraction, curvature, numData,     x0, y0 + 3);
    bc.scaleAndPrint ("dK",     fraction, numData, fdK,      dK,        numData - 1, x0, y0 + 2);
    bc.scaleAndPrint ("ddK",    fraction, numData, fddK,     ddK,       numData - 2, x0, y0 + 1);
    }

int main(int argc, char * argv[])
    {
    //initErrorTracking (NULL, argc, argv);
    int numStep = 12;

    int outputSelector = 8;
    static int sbPrintCurvatureCenters = 0;

    for (int i = 0; i < argc; i++)
        {
        int select = 0;
        if (   0 == strncmp (argv[i], "select=", 7))
            {
            if (1 == sscanf (argv[i] + 7, "%d", &select))
                outputSelector = select;
            }
        else if (   0 == strncmp (argv[i], "anglestep=", 10))
            {
            double a;
            if (1 == sscanf (argv[i] + 10, "%lf", &a))
                sMaxRadians = a;
            }
        }

    if (outputSelector & 0x01)
        {
        //PrintLineStrings (DSpiral2d_Bloss ());
        PrintLineStrings (DSpiral2d (SpiralClothoid), DSpiral2dClothoid ());
        }

    if (outputSelector & 0x02)
        {
        double pi = 4.0 * atan (1.0);
        double betaMax =   pi;
        double K0 = 0.0;
        double theta0 = 0.0;
        double theta1 = pi;
        DSpiral2d clothoid (SpiralClothoid);
        DSpiral2dClothoid clothoid1 = DSpiral2dClothoid();
        double yy = 0.0;
        double xx = 0.0;
        StrokeAndPrint (clothoid, clothoid1, 0.0, -0.5, 1.0,  1.0, xx, yy);
        yy += 3;
        StrokeAndPrint (clothoid,  clothoid1, 0.0,  1.0, 1.0,  2.0, xx, yy);
        yy += 3;
        StrokeAndPrint (clothoid,  clothoid1, 0.0,  1.0, 1.0, -2.0, xx, yy);
        }

    if (outputSelector & 0x04)
        {
        double theta0 = 0.0;
        double theta1 = 2.0;
        double K0     = 0.0;
        double K1     = 1.0;
        double yy = 0.0;
        double xx = 0.0;
        StrokeAndPrint (DSpiral2d (SpiralClothoid),  DSpiral2dClothoid (),       theta0, K0, theta1, K1, xx, yy);
        StrokeAndPrint (DSpiral2d (SpiralBloss),     DSpiral2dBloss (),          theta0, K0, theta1, K1, xx, yy);
        StrokeAndPrint (DSpiral2d (SpiralBiQuadratic),  DSpiral2dBiQuadratic (), theta0, K0, theta1, K1, xx, yy);
        }
#ifdef abc
    if (sbPrintCurvatureCenters)
        {
        for (double kappa = 0.125; kappa < 1.1; kappa *= sqrt (2.0))
            {
            double rho   = 1.0 / kappa;
            DVec2d centers[MAX_CLOTHOID_POINTS];
            int numCenter = 0;
            double maxBeta = 6.0 * atan (1.0);
            double myC = 0.1;
            double cFactor = 1.03;
            for (int circleCount = 0; circleCount < 151; circleCount++, myC *= cFactor)
                {
                clothoid = DClothoid2d (0.0, myC);
                t0 = 0.0;
                t1 = myC * kappa;
                double beta = t1 * t1 / (2.0 * myC);
                DClothoid2d::StrokeBetweenArcLengthPositions (clothoid, t0, t1, sMaxRadians, 4,
                            clothoidPoints, numClothoidPoints, MAX_CLOTHOID_POINTS, errorBound);

                DVec2d center, edge;
                edge = clothoidPoints[numClothoidPoints-1];
                center.x = edge.x - rho * sin (beta);
                center.y = edge.y + rho * cos (beta);
                centers[numCenter++] = center;

                if (circleCount % 50 == 0)
                    {
                    if (sbPrintCurvatureCenters & 0x02)
                        {
                        printf ("PLACE SMARTLINE\n");
                        int i0 = 0;
                        int maxOut = 100;
                        if (numClothoidPoints > maxOut)
                            i0 = numClothoidPoints - maxOut;
                        if (i0 == 0)
                            printf ("xy=0,0\n");
                        for (int i = i0; i < numClothoidPoints; i++)
                            {
                            printf ("xy=%.10lf,%.10lf\n", clothoidPoints[i].x, clothoidPoints[i].y);
                            }
                        }

                    if (sbPrintCurvatureCenters & 0x01)
                        {
                        printf ("PLACE CIRCLE\n");
                        printf ("xy=%.10lf,%.10lf\n", center.x, center.y);
                        printf ("xy=%.10lf,%.10lf\n", edge.x,   edge.y);
                        }
                    }
                }

            printf ("PLACE SMARTLINE\n");
            for (int i = 0; i < numCenter; i++)
                {
                printf ("xy=%.10lf,%.10lf\n", centers[i].x, centers[i].y);
                }
            }
        }


#endif
    if (outputSelector & 0x08)
        {
        double K0 = 0.0;
        double K1 = 1.0;
        double theta0 = 0.0;
        double theta1 = 2.0 * atan (1.0);
        double x0 = 0.0;
        double y0 = 0.0;
        double dX = 2.0;
        PrintDistanceFunctions ("clothoid", DSpiral2dClothoid (),
                theta0, K0, theta1, K1,
                x0, y0);
        x0 += dX;
        PrintDistanceFunctions ("BiQuadratic", DSpiral2dBiQuadratic (),
                theta0, K0, theta1, K1,
                x0, y0);
        x0 += dX;
        PrintDistanceFunctions ("Bloss", DSpiral2dBloss (),
                theta0, K0, theta1, K1,
                x0, y0);
        x0 += dX;
        PrintDistanceFunctions ("Cosine", DSpiral2dCosine (),
                theta0, K0, theta1, K1,
                x0, y0);
        x0 += dX;
        PrintDistanceFunctions ("Sine", DSpiral2dSine (),
                theta0, K0, theta1, K1,
                x0, y0);
        x0 += dX;
        PrintDistanceFunctions ("Viennese", DSPiral2dViennese (0.0, 0.0, 1.0),
                theta0, K0, theta1, K1,
                x0, y0);
        x0 += dX;

        PrintDistanceFunctions ("Viennese Roll", DSPiral2dWeightedViennese (1.0, 1.0, 1.0, 0.0, 1.0),
                theta0, K0, theta1, K1,
                x0, y0);
        x0 += dX;

        }

    if (outputSelector & 0x10)
        {
        double K0 = 0.0;
        double K1 = 0.5;
        double theta0 = 0.0;
        double theta1 = atan (1.0);
        double minStep = 0.02;
        double maxStep = 0.28;
        double stepFactor = 1.0;
        double stepIncrement = 0.02;
        StepSizeTests ("Clothoid",
                DSpiral2dClothoid (),
                theta0, K0, theta1, K1,
                minStep, maxStep, stepFactor, stepIncrement);
        StepSizeTests ("Viennese",
                DSPiral2dViennese (0.1, 1.0, 1.0),
                theta0, K0, theta1, K1,
                minStep, maxStep, stepFactor, stepIncrement);
        StepSizeTests ("BiQuadratic",
                DSpiral2dBiQuadratic (),
                theta0, K0, theta1, K1,
                minStep, maxStep, stepFactor, stepIncrement);
        StepSizeTests ("Bloss",
                DSpiral2dBloss (),
                theta0, K0, theta1, K1,
                minStep, maxStep, stepFactor, stepIncrement);


        }
    //return getExitStatus();
    return 0;
    }

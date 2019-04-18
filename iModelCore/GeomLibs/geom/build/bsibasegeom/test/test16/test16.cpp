/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <Geom/bezroot.fdf>
#include <Geom/bezeval.fdf>
#include <printfuncs.h>
#include <stdlib.h>


void checkRoots (double *pX, int nX, double *pY, int nY)
    {
    bsiDoubleArray_sort (pX, nX, true);
    bsiDoubleArray_sort (pY, nY, true);
    checkInt (nX, nY, "RootCounts");
    if (nX == nY)
        {
        double d, dMax = 0.0;
        double aMax = bsiDoubleArray_maxAbs (pX, nX);
        if (aMax < 1.0)
            aMax = 1.0;
        for (int i = 0; i < nX; i++)
            {
            d = fabs (pX[i] - pY[i]);
            if (d > dMax)
                dMax = d;
            checkDouble (pX[i] / aMax, pY[i] / aMax, "root");
            }
        printf (" max error = %10.4le maxRoot %10.4le  maxRel %10.4le\n", dMax, aMax, dMax / aMax);
        }
    }

// Multiply p(x) by (x-b)
int applyLinearFactor
(
double *pA,
int nCoffIn,
double b
)
    {
    for (int k = nCoffIn; k > 0; k--)
        pA[k] = pA[k-1];
    pA[0] = 0.0;
    for (int k = 0; k < nCoffIn; k++)
        pA[k] -= b * pA[k+1];
    return nCoffIn + 1;
    }

void testQuadratic (double a, double x0, double x1)
    {
    double c[3];
    int nCoff = 1;
    c[0] = a;
    nCoff = applyLinearFactor (c, nCoff, x0);
    nCoff = applyLinearFactor (c, nCoff, x1);
    double z[2];
    double zz[2];
    int nRoot = AnalyticRoots::SolveQuadric (c, z);
    zz[0] = x0;
    zz[1] = x1;
    checkRoots (zz, 2, z, nRoot);
    }

void testCubic(double a, double x0, double x1, double x2)
    {
    double c[10];
    int nCoff = 1;
    c[0] = a;
    nCoff = applyLinearFactor (c, nCoff, x0);
    nCoff = applyLinearFactor (c, nCoff, x1);
    nCoff = applyLinearFactor (c, nCoff, x2);
    double z[10];
    double zz[10];
    int nRoot = AnalyticRoots::SolveCubic (c, z);
    zz[0] = x0;
    zz[1] = x1;
    zz[2] = x2;
    checkRoots (zz, 3, z, nRoot);
    }

void testQuartic(double a, double x0, double x1, double x2, double x3)
    {
    double c[10];
    int nCoff = 1;
    c[0] = a;
    nCoff = applyLinearFactor (c, nCoff, x0);
    nCoff = applyLinearFactor (c, nCoff, x1);
    nCoff = applyLinearFactor (c, nCoff, x2);
    nCoff = applyLinearFactor (c, nCoff, x3);
    double z[10];
    double zz[10];
    int nRoot = AnalyticRoots::SolveQuartic (c, z);
    zz[0] = x0;
    zz[1] = x1;
    zz[2] = x2;
    zz[3] = x3;
    checkRoots (zz, 4, z, nRoot);
    }



void testAnalyticRoots ()
    {
    testQuadratic (1.0, 0.3, 0.5);
    testQuadratic (3.0, 0.3, 0.5);
    testCubic (1.0, 0.0, 1, -1);
    testCubic (2.0, 2.2, 3.2, -0.1);
    testCubic (-2.0, 2.2, 3.2, -0.1);
    testQuartic (1.0, 0, 1, 2, 3);
    testQuartic (1.2, -0.3, 1.4, 2.1, 3.9);
    testQuartic (1.0, 1.0e-3, 0.1, 100.0, -10.0);
    testQuartic (1.0, 1.3, 0.1, 100.0, -10.0);

    double cc0[4] = {2.0, 3.0, 0.0, 1.0};
    double z[4];
    int n = AnalyticRoots::SolveCubic (cc0, z);
    }

void testBezierSolution (double bezCoff[], int bezierOrder, double fy = 0.20)
    {
    double bezierRoot[10];
    double analyticRoot[10];
    int iLast = bezierOrder - 1;
    printf ("\nBezier versus Analytic roots order = %d\n", bezierOrder);
    for (int i = 0; i < bezierOrder; i++)
        printf ("       %lg\n", bezCoff[i]);
    double aMin, aMax;
    bsiDoubleArray_minMax (&aMin, &aMax, bezCoff, bezierOrder);
    double dy = 0.20 *  (aMax - aMin);
    double bMin, bMax;
    if (fy == 0.0)
        {
        bMin = 0.0;
        bMax = 0.1;
        dy = 1.0;
        }
    else
        {
        bMin = aMin - 1.2 * dy;
        bMax = aMax + 1.2 * dy;
        }
    for (double y = bMin; y <= bMax; y += dy)
        {
        int numAnalytic = -1;
        if (bezierOrder == 4)
                numAnalytic = AnalyticRoots::SolveBezierCubic
                            (
                            bezCoff[0], bezCoff[1], bezCoff[2], bezCoff[3], y,
                            analyticRoot, true);
        else if (bezierOrder == 3)
                numAnalytic = AnalyticRoots::SolveBezierQuadric
                            (
                            bezCoff[0], bezCoff[1], bezCoff[2], y,
                            analyticRoot, true);
        else if (bezierOrder == 5)
                numAnalytic = AnalyticRoots::SolveBezierQuartic
                            (
                            bezCoff[0], bezCoff[1], bezCoff[2], bezCoff[3], bezCoff[4], y,
                            analyticRoot, true);
        int numBezier;
        double shiftedBezier[10];
        for (int i = 0; i < bezierOrder; i++)
            shiftedBezier[i] = bezCoff[i] - y;
        bsiBezier_univariateRootsOptionalAnalytic (bezierRoot, &numBezier, shiftedBezier, bezierOrder, false);

        for (int i = 0; i < numAnalytic; i++)
            {
            double f;
            bsiBezier_evaluate (&f, bezCoff, bezierOrder, 1, analyticRoot[i]);
            checkDouble (f, y, "Analytic root into original bezier");
            }

        char message[1024];
        sprintf (message, "Bezier::Analytic root counts y=%g", y);
        checkInt (numBezier, numAnalytic, message);
        checkRoots (bezierRoot, numBezier, analyticRoot, numAnalytic);
        }
    }


void testBezierRoots ()
    {
    double bezierCoffs[5] = {-1.0, -0.5, 2.0, 2.4, 3.2};
    testBezierSolution (bezierCoffs, 3);
    testBezierSolution (bezierCoffs, 4);
    testBezierSolution (bezierCoffs, 5);
    double tripleRootCubic[4] = {-1,0,0,1};
    testBezierSolution (tripleRootCubic, 4);
    double singleRootCubic[4] = {-3,-1,1,3};
    testBezierSolution (tripleRootCubic, 4);

    double bigCoffs[5] = {-100.0, 400.0, -550.0, 375, -60.0};
    testBezierSolution (bigCoffs, 3);
    testBezierSolution (bigCoffs, 4);
    testBezierSolution (bigCoffs, 5);

    double lineCoffs[5] = { 1, 2, 3, 4, 5};
    testBezierSolution (lineCoffs, 3);
    testBezierSolution (lineCoffs, 4);
    testBezierSolution (lineCoffs, 5);

    double quadCoffs[5] = { 1, 0, 1, 4, 9};
    testBezierSolution (lineCoffs, 3);
    testBezierSolution (lineCoffs, 4);
    testBezierSolution (lineCoffs, 5);

    double cubicCoffs[5] = { -1, 0, 1, 8, 27};
    testBezierSolution (lineCoffs, 3);
    testBezierSolution (lineCoffs, 4);
    testBezierSolution (lineCoffs, 5);

    double cubicAtOne[4] = { 80000, 60000, 40000, 1.0e-2};
    testBezierSolution (cubicAtOne, 4, 0.0);

    double cubicAtOneA[4] = {
            -207.26307817223670,
            -138.17538544814903 ,
            -69.087692724093685,
            -1.4210854715202004e-014};
    testBezierSolution (cubicAtOneA, 4, 0.0);
    }

#undef ERROR
#include <windows.h>
void checkTimes (double *pCoffs, int order, int repCount)
    {
    double pRootsA[10];
    double pRootsB[10];

    int numA, numB;
    printf ("<timedAnalytic>\n");
    DWORD a0 = GetTickCount ();
    for (int i = 0; i < repCount; i++)
        bsiBezier_univariateRootsOptionalAnalytic (pRootsA, &numA, pCoffs, order, true);
    DWORD a1 = GetTickCount ();
    DWORD a01 = a1 - a0;
    printf ("  <ticks>%d</ticks>\n", a1 - a0);
    printf (" </timedAnalytic>\n");

    printf ("<timedBzier>\n");
    DWORD b0 = GetTickCount ();
    for (int i = 0; i < repCount; i++)
        bsiBezier_univariateRootsOptionalAnalytic (pRootsB, &numB, pCoffs, order, false);
    DWORD b1 = GetTickCount ();
    DWORD b01 = b1 - b0;
    printf ("  <ticks>%d</ticks>\n", b1 - b0);
    printf (" </timedBezier>\n");
    double ratio;
    bsiTrig_safeDivide (&ratio, (double)b01, (double)a01, 0.0);
    if (a01 > 0)
        printf ("                   <ratio>%.2lg</ratio>\n", ratio);
    }


void checkTimes (double *pCoff, int minOrder, int maxOrder, char *pDescription, int repCount)
    {
    printf ("\n<RootTimes> %s </RootTimes>\n", pDescription);
    for (int order = minOrder; order <= maxOrder; order++)
        {
        printf ("<degree>%d</degree>\n", order - 1);
        checkTimes (pCoff, order, repCount);
        }
    }

void checkTimes (int repCount)
    {
    double coff0[6] = {-4, -1, 1, 5, 9, 22};
    checkTimes (coff0, 4, 6, "Simple Root", repCount);
    double coff1[6] = {-10, 6, -1, -5, -12};
    checkTimes (coff1, 4, 6, "2 Roots", repCount);

    double coff2[6] = {1,2,3,5,6,7};
    checkTimes (coff2, 4, 6, "No Roots", repCount);
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    selectTolerances (3);
    testBezierRoots ();
    testAnalyticRoots ();
    selectTolerances (0);
    static int repCount = 400;
    checkTimes (repCount);
    return getExitStatus();
    }

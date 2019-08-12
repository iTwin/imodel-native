/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>

double extendRombergTable
(
double *pQ,
double qNew,
int    depth
)
    {
    double q0, q1, q2;
    double a = 4.0;
    q1 = qNew;
    for (int i = 0; i < depth; i++, a *= 4)
        {
        q0 = pQ[i];
        q2 = q1 + (q1 - q0) / (a - 1.0);
        pQ[i] = q1;
        q1 = q2;
        }
    pQ[depth] = q1;
    return q1;
    }

#define MAX_ROMBERG 1025
double bsiBezierDPoint3d_arcLengthRombergChords
(
DPoint3d *pXYZ,
int      order,
double refLength
)
    {
    DVec3d tangent;
    double chordLength[20];
    double extrapolation[20];
    DPoint3d xyz;
    static int maxDepth = 12;
    printf (" refLength %.16lg\n", refLength);

    double sum = 0.0;
    bsiBezierDPoint3d_evaluateDPoint3d (&xyz, &tangent, pXYZ, order, 0.0);
    sum += tangent.magnitude ();
    bsiBezierDPoint3d_evaluateDPoint3d (&xyz, &tangent, pXYZ, order, 1.0);
    sum += tangent.magnitude ();

    double q = extrapolation[0] = chordLength[0] = 0.5 * sum;
    double alpha, dq;
    int numPoint = 1;   // number to evaluate on first pass.
    double ds = 1.0;    // rectangle width for first pass.
    double dq0 = 1.0;
    double q0 = q;
    for (int depth = 1; depth < maxDepth; depth++)
        {
        double s0 = 0.5 * ds;
        sum = 0.0;
        for (int i = 0; i < numPoint; i++)
            {
            double s = s0 + i * ds;
            bsiBezierDPoint3d_evaluateDPoint3d (&xyz, &tangent, pXYZ, order, s);
            sum += tangent.magnitude ();
            }
        chordLength[depth] = 0.5 * (chordLength[depth-1] + sum * ds);
        double eSimple = chordLength[depth] - refLength;
        q = extendRombergTable (extrapolation, chordLength[depth], depth);
        double eRomberg = q - refLength;
        dq = q - q0;
        alpha = dq0 == 0.0 ? 0.0 : dq / dq0;
        printf ("    %.15lg (dq %.2le) (%.1le) (e0 %.1le) (eR %.1le)\n", q, dq, alpha, eSimple, eRomberg);
        dq0 = dq;
        q0 = q;
        numPoint *= 2;
        ds *= 0.5;
        }
    return q;
    }

void testBezierArcLength (char *pDescription, DPoint3dP pXYZ, int order, int minTerm, int maxTerm, int depth = 0)
    {
    DPoint4d xyzw[30];
    for (int i = 0; i < order; i++)
        xyzw[i].initFrom (&pXYZ[i], 1.0);
    double refLength = bsiBezierDPoint4d_arcLength (xyzw, order, 0.0, 1.0);
    printf ("\n%s Ref Length %lg\n", pDescription, refLength);

    bsiBezierDPoint3d_arcLengthRombergChords (pXYZ, order, refLength);

    for (int numTerm = minTerm; numTerm <= maxTerm; numTerm++)
        {
        double currLength = bsiBezierDPoint3d_arcLengthHermite (pXYZ, order, numTerm);
        double e1 = currLength - refLength;
        printf ("  (terms %d) (hermiteLength %.15lg) (e1 %.3lg)\n", numTerm, currLength, e1);
        if (depth > 1)
            {
            DPoint3d xyz[MAX_BEZIER_CURVE_ORDER];
            int numSub = 2;
            double eOld = e1;
            for (int currDepth = 0; currDepth < depth; currDepth++, numSub *= 2)
                {
                double aCurr = 0.0;
                double ds = 1.0 / (double)numSub;
                for (int i = 0; i < numSub; i++)
                    {
                    memcpy (xyz, pXYZ, order * sizeof (DPoint3d));
                    bsiBezierDPoint3d_subdivideToIntervalInPlace (xyz, order, i * ds, (i + 1) * ds);
                    double subLength = bsiBezierDPoint3d_arcLengthHermite (xyz, order, numTerm);
                    aCurr += subLength;
                    }
                double eCurr = aCurr - refLength;
                printf ("      (numSub %d) (hermiteLength %.15lg) (e %.3lg)\n", numSub, aCurr, eCurr);
                double ratio = eCurr == 0.0 ? 1.0 : fabs (eOld / eCurr);
                double logRatio = log (ratio) / log (2.0);
                printf ("      (ratio %lg) (ratioLog %lg)\n", ratio, logRatio);
                eOld = eCurr;
                }
            }
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    DPoint3d line01[2] = {{0,0,0},{1,0,0}};
    static int sNumTerm = 5;

    DPoint3d quadStretch03[3] = {{0,0,0},{1.0,0,0},{3,0,0}};
    testBezierArcLength ("Stretched Line", quadStretch03, 3, 1, sNumTerm, 2);


    testBezierArcLength ("Simple Line", line01, 2, 1, sNumTerm, 2);


    DPoint3d cornerA[3] = {{0,0,0},{1,0,0},{1,1,0}};
    testBezierArcLength ("Corner Quadratic", cornerA, 3, 1, sNumTerm, 2);

    DPoint3d gentle[3] = {{0,0,0},{2,1,0},{4,1,0}};
    testBezierArcLength ("Gentle Quadratic", gentle, 3, 1, sNumTerm, 2);

    DPoint3d cubic[4] = {{0,0,0}, {2,1,0}, {4,0.8,0}, {7,0,0}};
    testBezierArcLength ("Gentle Cubic", cubic, 4, 1, sNumTerm, 2);
    testBezierArcLength ("GentleCubic", cubic, 4, 1, sNumTerm, 4);

    DPoint3d sextic[7] = {{0,0,0}, {2,1,0}, {4,0.8,0}, {7,0,0},
                            {8,4,0}, {10,0,0}, {12,0,2}};
    testBezierArcLength ("Nasty degree 6", sextic, 7, 1, sNumTerm, 4);


    DPoint3d line15[15] =
        {
            {0,0,0},
            {1.1,0,0},
            {2,0,0},
            {3.2,0,0},
            {4.1,0,0},
            {5,0,0},
            {6,0,0},
            {7,0,0},
            {8,0,0},
            {9,0,0},
            {10,0,0},
            {11,0,0},
            {12,0,0},
            {13,0,0},
            {14,0,0}
        };

    for (int order = 2; order < 14; order++)
        {
        char message[1024];
        sprintf (message, "Stretched line %d\n", order);
        testBezierArcLength (message, line15, order, 2, 4, 10);
        }

    // Conclusions EDL July 6, 2007
    // Convergence with N terms at each end (i.e. 2N coffs determined in approximating bezier)
    // is about h^*(2*N).
    // In particular, with 4 terms at each end the reduction factor is about 256.
    return getExitStatus();
    }

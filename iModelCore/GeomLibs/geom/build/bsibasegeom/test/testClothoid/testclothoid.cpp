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
#include <malloc.h>

#include <Geom/dclothoid2d.h>

#define MAX_CLOTHOID_POINTS 2000

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    DVec2d clothoidPoints[MAX_CLOTHOID_POINTS];
    int numClothoidPoints;
    DClothoid2d clothoid(0.0, 1.0);
    double t0 = 1.0;
    double t1 = 4.0;

    int numStep = 12;
    double xy1[2], xy2[2], xy8[2], xyR[2];

    static double sMaxRadians = 0.08;
    double errorBound;
    static int sbCheckAccuracy = 0;
    static int sbPrintLinestrings = 1;
    static int sbPrintCurvatureCenters = 0;

    BSIQuadraturePoints gauss[4];
    gauss[0].InitGauss(1);
    gauss[1].InitGauss(2);
    gauss[2].InitGauss(3);
    gauss[3].InitGauss(4);

    if (sbCheckAccuracy)
        {
        // EDL Sept 26 very close to machine prcision with step size 0.03
        //    (in angle space -- not distance) and 3 point gauss rule.
        for (int numGauss = 1; numGauss < 5; numGauss++)
            {
            int numSubStep = 2;
            double expectedFactor = pow ((double)numSubStep, gauss[numGauss - 1].GetConvergencePower ());
            printf ("\n\n  ======numGauss  %d  == numSubStep %d == expected factor %5.0lf ==========\n",
                        numGauss, numSubStep, expectedFactor);
            xy1[0] = xy1[1] = 0.0;
            xy2[0] = xy2[1] = 0.0;
            xy8[0] = xy8[1] = 0.0;
            for (int step = 0; step < numStep; step++)
                {
                double dt = (t1 - t0) / numStep;
                double a0 =  t0 + step * dt;
                double a1 = t0 + (step + 1) * dt;
                gauss[numGauss-1].AccumulateWeightedSums (clothoid, a0, a1, xy1, 1);
                gauss[numGauss-1].AccumulateWeightedSums (clothoid, a0, a1, xy2, numSubStep);
                gauss[3].AccumulateWeightedSums (clothoid, a0, a1, xy8, 16);
                double e1 = xy8[0] - xy1[0];
                double e2 = xy8[0] - xy2[0];
                double f = 1.0 / (expectedFactor - 1.0);
                xyR[0] = xy2[0] + f * (xy2[0] - xy1[0]);
                xyR[1] = xy2[1] + f * (xy2[1] - xy1[1]);
                double eR = xy8[0] - xyR[0];
                double f12 = e2 != 0.0 ? fabs (e1/e2) : 0.0;
                if (step == numStep - 1)
                    printf (" %d %5.2lg    %20.15lg,%20.15lg  e:[%.1le,%.1le,%.1e] %.6lf @ %lg\n", step, a1,
                        xy8[0], xy1[0],
                        fabs (e1), fabs (e2), fabs (eR),
                        f12,
                        clothoid.GlobalHeadingRadiansAtDistance (a1));
                }
            DVec2d dxy;
            DClothoid2d::StrokeBetweenArcLengthPositions (clothoid, t0, t1, sMaxRadians, numGauss,
                        clothoidPoints, numClothoidPoints, MAX_CLOTHOID_POINTS, errorBound);
            double error = dxy.distance ((DVec2d*)xy8);
            dxy = clothoidPoints[numClothoidPoints - 1];
            printf (" f = %20.15lg, %20.15lg   e = %.1le   E=%le\n",
                                dxy.x, dxy.y, error, errorBound);
            }
        }

    if (sbPrintLinestrings)
        {
        double pi = 4.0 * atan (1.0);
        double betaMax =   pi;
        for (double myC = -9.0; myC < 10.0; myC+= 1.0)
            {
            if (fabs (myC) < 1.0e-3)
                continue;
            clothoid = DClothoid2d (0.0, myC);
            for (double s = -1; s < 2; s += 2)
                {
                t0 = 0.0;
                t1 = clothoid.SignedDistanceAtSignedLocalHeadingRadians (s * betaMax);
                DClothoid2d::StrokeBetweenArcLengthPositions (clothoid, t0, t1, sMaxRadians, 4,
                            clothoidPoints, numClothoidPoints, MAX_CLOTHOID_POINTS, errorBound);
                printf ("PLACE SMARTLINE\n");
                printf ("xy=0,0\n");
                for (int i = 0; i < numClothoidPoints; i++)
                    {
                    printf ("xy=%.10lf,%.10lf\n", clothoidPoints[i].x, clothoidPoints[i].y);
                    }
                }
            }
        }

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




    return getExitStatus();
    }

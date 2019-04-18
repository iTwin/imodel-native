/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <msgeomstructs.h>
#include <printfuncs.h>


int main (int argc, char** argv)
    {
    initErrorTracking (NULL, argc, argv);
    double testSeconds[] = {25433.2, 2593.7, 0.5, -235.3, 1.2345678, 0.0};
    int angleSplits[3] = {60,60, 0};
    int angleDenom[5] = {4,16,10, 100, 1000};
    int numDenom = 5;
    int digits[20];
    double residual;
    bool      bNegative;

    for (int i = 0;;i++)
        {
        double seconds = testSeconds[i];
        double degrees0 = seconds / 3600;

        for (int k = 0; k < numDenom; k++)
            {
            angleSplits[2] = angleDenom[k];
            bsiTrig_splitDoubleToUnitizedDigits (degrees0, &bNegative, digits, &residual, angleSplits, 3, 0.0);
            double degrees1 = bsiTrig_mergeUnitizedDigits (bNegative, digits, residual, angleSplits, 3);
            checkDouble (degrees0, degrees1, "DDD MM SS (rem 1/%lf)", (double)angleDenom[k]);
            double seconds1 = digits[0] * 3600
                            + digits[1] * 60 + digits[2]
                            + (digits[3] + residual) / (double)angleSplits[2];
            if (bNegative)
                seconds1 = - seconds1;
            checkDouble (seconds, seconds1, "seconds (rem 1/%.0lf)", (double)angleDenom[k]);
            }
        if (seconds == 0.0)
            break;
        }
#define INTERACTIVEnot
#ifdef INTERACTIVE
    double degrees, minutes, seconds;
    while (scanf ("%lf %lf %lf", &degrees, &minutes, &seconds) == 3)
        {
        double angle = degrees + minutes / 60.0 + seconds / 3600.0;
        // integerized split with various denominators ...
        for (int k = 0; k < 5; k++)
            {
            angleSplits[2] = angleDenom[k];
            bsiTrig_splitDoubleToUnitizedDigits (angle, &bNegative, digits, &residual, angleSplits, 3, 0.0);
            double angle1 = bsiTrig_mergeUnitizedDigits (bNegative, digits, residual, angleSplits, 3);
            printf ("(LIB) %d dg %d min  %d %d/%d sec (residual %22.16le)\n",
                        digits[0], digits[1], digits[2], digits[3], angleSplits[2], residual);
            printf ("            angle0 %20.15lf angle1 %20.15lf diff %.2le\n\n", angle, angle1, angle - angle1);
            }
        }
#endif
    getExitStatus();
    }
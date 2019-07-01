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


void test1_DRange2d ()
    {
    DRange2d rangeA, rangeB, rangeC, rangeD;
    rangeA.init ();
    rangeB.init ();
    checkDRange2d (&rangeA, &rangeB, "init");
    rangeA.initFrom (1,2,2.5,4.3);
    checkDRange2d (&rangeA, &rangeA, "initFrom (xyxy)");
    checkTrue  (rangeB.isNull (), "DRange2d.isNull()");
    checkFalse (rangeA.isNull (), "DRange2d.isNull()");

    rangeA.initFrom (1,2,3,5);
    checkDouble (rangeA.extentSquared (), 13.0, "DRange2d.extentSquared");
    checkDouble (rangeA.area (), 6.0, "DRange2d.area");
    checkFalse (rangeA.isPoint (), "DRange2d.isPoint ()");

    checkFalse (rangeA.isEmpty (), "DRange2d.isEmpty()");
    rangeA.initFrom (1,2);
    checkTrue (rangeA.isPoint (), "DRange2d.isPoint ()");
    checkFalse (rangeA.isEmpty (), "DRange2d.isEmpty()");

    rangeA.high.x -= 0.2;
    checkTrue (rangeA.isEmpty (), "DRange2d.isEmpty() on x");
    rangeA.initFrom (1,2);
    rangeA.high.x -= 0.2;
    checkTrue (rangeA.isEmpty (), "DRange2d.isEmpty() on y");

    DPoint2d pointQ, pointR;
    pointQ.x = 3.0;
    pointQ.y = 4.0;
    pointR.x = 1.0;
    pointR.y = 2.3;
    rangeA.initFrom (&pointQ);
    rangeB.initFrom(pointQ.x, pointQ.y);
    checkDRange2d (&rangeA, &rangeB, "DRange2d::initFrom(x,y)==initFrom(point)");

    rangeA.initFrom(&pointQ, &pointR);
    rangeB.initFrom (&pointR, &pointQ);
    checkDRange2d (&rangeA, &rangeB, "DRange2d::initFrom (P,Q) order");

    rangeC.initFrom (pointQ.x, pointQ.y, pointR.x, pointR.y);
    rangeD.initFrom (pointR.x, pointR.y, pointQ.x, pointQ.y);
    checkDRange2d (&rangeC, &rangeD, "DRange2d::initFrom(x,y,x,y) order");
    checkDRange2d (&rangeA, &rangeC, "DRange2d::initFrom");

    rangeC.init ();
    rangeD.init ();

    pointQ.x += 3.1;
    pointQ.y += 0.2;
    rangeC.initFrom (&pointQ);
    rangeD.initFrom( &pointQ, &pointQ);
    checkDRange2d (&rangeC, &rangeD, "DRange2d init from same point");

    for (double theta0 = -3; theta0 < 4.0; theta0 += 1.0)
        {
        for (double sweep = -6.25; sweep < 6.28; sweep += 1.5)
            {
            rangeA.initFromUnitArcSweep (theta0, sweep);
            rangeB.init ();
            int numStep = 32;
            double df = 1.0 / (double)numStep;
            for (int i = 0; i <= numStep; i++)
                {
                double f = i * df;
                double theta = theta0 + f * sweep;
                double x = cos (theta);
                double y = sin (theta);
                rangeB.extend (x, y);
                }
            double areaA = rangeA.area ();
            double areaB = rangeB.area ();
            checkTrue (rangeB.isContained (&rangeA), "Arc Containment");
            checkTrue (areaB <= areaA && areaB > 0.90 * areaA,
                    "unit arc sweep range");
            }
        }
    }

void test2_DRange2d ()
    {
    double xx[5] = {1,2,3,4,5};
    double yy[5] = {1.5, 2.5, 3.5, 4.5, 5.5};

    DRange2d range;

    range.initFrom (xx[1], yy[1], xx[3], yy[3]);

    for (int i = 0; i < 5; i++)
        {
        for (int j = 0; j < 5; j++)
            {
            char message1[128];
            char message2[128];
            char message3[128];
            sprintf (message1, "Contains(x,y)%d%d", i, j);
            sprintf (message2, "Contains(xy)%d%d", i, j);
            sprintf (message3, "Contains(xyz)%d%d", i, j);
            DPoint2d xy;
            xy.x = xx[i];
            xy.y = yy[j];
            DPoint3d xyz;
            xyz.x = xx[i];
            xyz.y = yy[j];
            xyz.z = 0.0;
            if (   i >= 1
                && i <= 3
                && j >= 1
                && j <= 3
                )
                {
                checkTrue  (range.contains (xx[i], yy[j]), message1);
                checkTrue  (range.contains (&xy),  message2);
                checkTrue  (range.contains (&xyz), message3);
                }
            else
                {
                checkFalse (range.contains (xx[i], yy[j]), message1);
                checkFalse  (range.contains (&xy),  message2);
                checkFalse  (range.contains (&xyz), message3);
                }
            }
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    test1_DRange2d ();
    test2_DRange2d ();
    return getExitStatus();
    }

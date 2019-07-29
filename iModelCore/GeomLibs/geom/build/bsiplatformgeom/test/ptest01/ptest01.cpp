/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <migeomstructs.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>
#include <platformgeometry.h>
using namespace BSIG;

void exerciseCCI (
    char *pNameA,
    Curve3dP curveA,
    char *pNameB,
    Curve3dP curveB,
    int expectedPoints     = -1,
    int expectedIntervals  = -1
    )
    {
    VectorDPoint3dDoublePair intPoints;
    VectorDPoint3dDoubleQuad intIntervals;

    printDatum ("CurveA", pNameA, "",NULL);
    printDatum ("CurveB", pNameB, "",NULL);
    Curve3dCurve3d::AddXYIntersections (curveA, 0, curveB, 0, intPoints, intIntervals);

    if (expectedPoints >= 0)
        checkInt (expectedPoints, intPoints.size (), "intersect count");
    if (expectedIntervals >= 0)
        checkInt (0, intIntervals.size (), "interval count");

    for (unsigned int i = 0; i < intPoints.size (); i++)
        {
        DPoint3d pointA, pointB;
        double fractionA, fractionB;
        EntityP entityA, entityB;
        intPoints[i].Get (entityA, pointA, fractionA, entityB, pointB, fractionB);
        DPoint3d pointA1, pointB1;
        curveA->FractionToPoint (pointA1, fractionA);
        curveB->FractionToPoint (pointB1, fractionB);
        checkDPoint3d (&pointA, &pointA1, "Intersection on A");
        checkDPoint3d (&pointB, &pointB1, "Intersection on B");
        pointA.z = pointB.z;
        checkDPoint3d (&pointA, &pointB, "Intersection Match");
        }
    }


void exerciseCurve (char *pName, Curve3dP curve, int numEdge = 4)
    {
    DRange3d range;
    curve->GetRange (range);
    int rangeErrors = 0;
    printDatum ("exerciseCurve", pName, "",NULL);

    VectorDPoint3dDVec3dDouble strokePoints;
    curve->AddStrokes (strokePoints, 0.0, 0.0, 0.0, numEdge + 1, -1);
    for (unsigned int i = 0; i < strokePoints.size (); i++)
        {
        DPoint3d xyz = strokePoints[i].point;
        DPoint3d xyz1;
        double f = strokePoints[i].a;
        curve->FractionToPoint (xyz1, f);
        checkDPoint3d (&xyz, &xyz1, "Eval at stroke point");
        if (!bsiDRange3d_isDPoint3dContained (&range, &xyz))
            {
            rangeErrors++;
            checkTrue (false, "Curve Range Error");
            printDPoint3d ("PointNotInRange", &xyz);
            }
        }

    if (rangeErrors > 0)
        {
        printDPoint3d ("range.low", &range.low);
        printDPoint3d ("range.high", &range.high);
        }
    double length = curve->Length ();
    printDouble ("Length", length);
    double distanceA, distanceB;
    double sMid = 0.3243;
    checkTrue (curve->SignedDistanceBetweenFractions (0.0, sMid, 0, distanceA), "Partial Distance 0.0, 0.5");
    checkTrue (curve->SignedDistanceBetweenFractions (sMid, 1.0, 0, distanceB), "Partial Distance 0.5, 1.0");
    selectTolerances (3);
    checkDouble (distanceA + distanceB, length, "partial length sums to length");
    printDouble ("PartialLengthSum - Length", distanceA + distanceB - length);
    selectTolerances (0);
    }

int main(int argc, char * argv[])
    {
    static int sDoNonUniformNurbs = 0;
    initErrorTracking (NULL, argc, argv);

    Curve3dFactoryP factory = BSIG::bsiPlatform_getFactory()->GetCurveFactory ();
    DPoint3d point0, point1;
    point0.init (1,2,3);
    point1.init (3,4,6);
    Curve3dP segment =  factory -> NewSegment (point0, point1);
    checkDouble (segment->Length (), point0.distance (&point1), "Length");

    DPoint3d bezPoint[3] =
        {
            {0,0,0},
            {1,1,0},
            {2,0,0}
        };
    Curve3dP bezier =  factory-> NewBezier (bezPoint, 3);
    exerciseCurve ("BEZIER", bezier);
    exerciseCurve ("SEGMENT", segment);


    DPoint3d nurbsPoint[5] =
        {
            {0,0,0},
            {1,1,0},
            {2,0,0},
            {3,-1,0},
            {4,0,0}
        };
    Curve3dP nurbsA = factory->NewNurbs (nurbsPoint, NULL, 5, 3, false, 0);
    exerciseCurve ("Uniform Nurbs", nurbsA, 15);

    {
    double twoPi = 8.0 * atan (1.0);
    DPoint3d pointA0, pointA1, pointB0, pointB1;
    pointA0.init (0,-10,0);
    pointA1.init (0,20,0);
    pointB0.init (-1,0.25,0);
    pointB1.init (1,0.0,0);
    Curve3dP segmentA = factory->NewSegment (pointA0, pointA1);
    Curve3dP segmentB = factory->NewSegment (pointB0, pointB1);
    exerciseCCI("Line", segmentA, "Line", segmentB, 1, 0);

    DEllipse3d circle4;
    DEllipse3d ellipseX6Y2;
    bsiDEllipse3d_init (&circle4, 0,0,0, 4,0,0, 0,4,0, 0.0, twoPi);
    bsiDEllipse3d_init (&ellipseX6Y2, 0,0,0, 6,0,0, 0,2,0, 0.0, twoPi);
    Curve3dP ellipseA = factory->NewEllipse (circle4);
    Curve3dP ellipseB = factory->NewEllipse (ellipseX6Y2);
    exerciseCCI ("Line", segmentA, "Ellipse", ellipseA, 2, 0);
    exerciseCCI ("Ellipse", ellipseA, "Ellipse", ellipseB, 4, 0);
    }

    if (sDoNonUniformNurbs)
        {
        double knots[7] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
        DPoint3d poles[4] =
            {
            {0,0,0},
            {0,1,0},
            {1,1,0},
            {1,0,0},
            };
        Curve3dP nurbsB = factory->NewNurbs (poles, NULL, 4, knots, 7, 0, 3, false);
        exerciseCurve ("NonUniform NURBS", nurbsB);
        }
    return getExitStatus();
    }

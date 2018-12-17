
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <stdlib.h>


void testProjections
(
double ax, double ay, double az,
double bx, double by, double bz,
double cx, double cy, double cz
)
    {
    DSegment3d seg1;
    seg1.init (ax, ay, az, bx, by ,bz);
    DPoint3d spacePoint = {cx, cy, cz};
    DPoint3d xyzPoint, xyPoint;
    DPoint3d xyzPoint1, xyPoint1;
    double   xyzFraction, xyFraction;
    seg1.projectPoint (&xyzPoint, &xyzFraction, &spacePoint);
    seg1.projectPointXY (&xyPoint, &xyFraction, &spacePoint);

    seg1.fractionParameterToPoint (&xyPoint1, xyFraction);
    seg1.fractionParameterToPoint (&xyzPoint1, xyzFraction);

    DVec3d xyzVector, xyVector, lineVector;
    xyzVector.differenceOf (&xyzPoint, &spacePoint);
    xyVector.differenceOf (&xyPoint, &spacePoint);
    seg1.fractionParameterToTangent (NULL, &lineVector, 0.0);

    checkDPoint3d (&xyzPoint, &xyzPoint1, "xyz projection on line");
    checkDPoint3d (&xyPoint, &xyPoint1, "xy projection on line");
    double scalingShift = xyzVector.magnitude() + xyVector.magnitude () + lineVector.magnitude ();
    checkDouble (scalingShift, scalingShift + xyzVector.dotProduct (&lineVector), "xyz perp");
    checkDouble (scalingShift, scalingShift + xyVector.dotProductXY (&lineVector), "xy perp");
    checkFalse (xyVector.isParallelTo (&xyzVector), "xyz, xy differ");
    checkDoubleLessThan (spacePoint.distance (&xyzPoint1), spacePoint.distance (&xyPoint1),
            "space distance < xy distance");
    double df = 0.01;
    for (int i = 1; i < 5; i++)
        {
        for (int k = -1; k <= 1; k+= 2)
            {
            DPoint3d xyzi;
            double e = i * k * df;
            double fxyz = xyzFraction + e;
            seg1.fractionParameterToPoint (&xyzi, fxyz);
            checkDoubleLessThan (spacePoint.distance (&xyzPoint1), spacePoint.distance (&xyzi), "mindistxyx < xyz distance at df %lf", e);

            double fxy = xyFraction + e;
            seg1.fractionParameterToPoint (&xyzi, fxy);
            checkDoubleLessThan (spacePoint.distanceXY (&xyPoint1), spacePoint.distance (&xyzi), "mindistxy < xy distance at df %lf", e);
            }
        }
    }

void testProjections ()
    {
    testProjections (1,2,3,-4,2,12, 4, 9,12);
    testProjections (3,3,6, -4, 2,5, 69, 23, -8);
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testProjections ();
    return getExitStatus();
    }

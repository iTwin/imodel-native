// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <printfuncs.h>
static double s_pi = 4.0 * atan (1.0);


bool    bsiDEllipse3d_alignedRangeXXX
(
DEllipse3dCP ellipse,
TransformP   frame,
DRange3dP     range
)
    {
    DEllipse3d mmEllipse, localEllipse;
    double r0, r1, start, sweep;
    DPoint3d center;
    RotMatrix axes;
    bool    stat = false;
    Transform worldToLocal, localToWorld;
    static double sSweepFactor = 1.00001;
    DVec3d diagonal;
    DPoint3d worldLowerLeft;
    DRange3d localRange;

    bsiDEllipse3d_initMajorMinor (&mmEllipse, ellipse);
    // Tentatively work in the centered major-minor axes ...
    mmEllipse.getScaledRotMatrix (&center, &axes, &r0, &r1, &start, &sweep);

    if (fabs (sweep) < msGeomConst_pi * sSweepFactor)
        {
        // align to start-end chord ...
        DPoint3d startPoint, endPoint;
        ellipse->evaluateEndPoints (&startPoint, &endPoint);
        DVec3d xVec, yVec, zVec;
        axes.getColumn (&zVec, 2);
        xVec.normalizedDifference (&endPoint, &startPoint);
        yVec.normalizedCrossProduct (&zVec, &xVec);
        localToWorld.initFromOriginAndVectors (&startPoint, &xVec, &yVec, &zVec);
        }
    else
        {
        localToWorld.initFrom (&axes, &center);
        }

    stat = worldToLocal.inverseOf (&localToWorld);
    if (!stat)
        {
        worldToLocal.initIdentity ();
        localEllipse = *ellipse;
        }

    localEllipse.productOf (&worldToLocal, ellipse);
    localEllipse.getRange (&localRange);
    // Shift transform so origin is at lower left of range ...
    diagonal.differenceOf (&localRange.high, &localRange.low);
    range->low.zero ();
    range->high.init (diagonal.x, diagonal.y, diagonal.z);
    localToWorld.multiply (&worldLowerLeft, &localRange.low);
    *frame = localToWorld;
    frame->setTranslation (&worldLowerLeft);
    return stat;
    }

void show (DEllipse3dR ellipse, TransformR transform, DRange3dR range)
    {
    CGXmlWriter cgWriter = CGXmlWriter ();
    DPoint3d localPoints[100];
    DPoint3d worldPoints[100];
    int n;
    localPoints[0] = localPoints[4] = range.low;
    localPoints[1] = range.low;
    localPoints[1].x = range.high.x;
    localPoints[2] = range.high;
    localPoints[3] = range.high;
    localPoints[3].x = range.low.x;
    n = 5;
    cgWriter.EmitEllipticArc (ellipse);
    transform.multiply (worldPoints, localPoints, n);
    cgWriter.EmitLinestring (worldPoints, n);
    }

void check (double cx, double cy, double cz,
    double ux, double uy, double uz,
    double vx, double vy, double vz,
    double start, double sweep)
    {
    DEllipse3d ellipseA;
    DRange3d rangeA;
    Transform frameA, inverseA;
    ellipseA.init (cx, cy, cz,
            ux, uy, uz,
            vx, vy, vz,
            start, sweep);
    ellipseA.alignedRange (&frameA, &inverseA, &rangeA);
    show (ellipseA, frameA, rangeA);
    }
int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    check (0,0,0, 2,0,0, 0,1,0, -msGeomConst_piOver2, msGeomConst_pi);
    for (double a = 0.5; a < 5.0; a+= 0.5)
        check (1 + 10 * a,2,2, 1,2,5, -2,3,.2, 0.3, a);

    return getExitStatus();
    }

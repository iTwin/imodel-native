// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <stdlib.h>

void testDVec2d ()
    {
    DVec2d vecA;
    DVec2d vecB;
    double dx = 1.0;
    double dy = 2.0;
    double ex, ey;
    vecA.init (dx, dy);
    vecA.getComponents (&ex, &ey);

    checkDouble (dx, ex, "DVec2d::getComponents x");
    checkDouble (dy, ey, "DVec2d::getComponents y");

    vecB = DVec2d::FromXY (dx, dy);

    checkDVec2d (&vecA, &vecB, "DVec2d::FromXY, .init");

    checkDouble (dx, vecA.getComponent (0),      "DVec2d::getComponent(%g)", 0);
    checkDouble (dy, vecA.getComponent (1),      "DVec2d::getComponent(%g)", 1);

    checkDouble (dx, vecA.dotProduct (1.0, 0.0), "DVec2d::dotProduct (1,0)");
    checkDouble (dy, vecA.dotProduct (0.0, 1.0), "DVec2d::dotProduct (0,1)");

    for (int i = -4; i < 4; i += 2)
        {
        vecA.init (0,0);
        vecA.setComponent (dx, 0);
        checkDouble (dx, vecA.getComponent (i), "DVec2d::getComponent(%g)", i);
        checkDouble (0.0, vecA.getComponent (i+1), "DVec2d::getComponent(%g)", i);
        vecA.init (0,0);
        vecA.setComponent (dy, 1);
        checkDouble (0.0, vecA.getComponent (i), "DVec2d::getComponent(%g)", i);
        checkDouble (dy, vecA.getComponent (i+1), "DVec2d::getComponent(%g)", i);
        }

    vecA.init (dx, dy);

    double a2 = dx * dx + dy * dy;
    double a1 = sqrt (a2);
    double b1 = vecA.magnitude ();
    double b2 = vecA.magnitudeSquared ();
    checkDouble (a1, b1, "DVec2d::magnitude ()");
    checkDouble (a2, b2, "DVec2d::magnitudeSquared");

    checkDouble (0.0, vecA.crossProduct (&vecA), "DVec2d U cross U");
    vecB.unitPerpendicular (&vecA);
    checkDouble (1.0, vecB.magnitude (), "UnitPerpendicular mag");

    vecB.init (3.0, -2.123123);
    double magA = vecA.magnitude ();
    double magB = vecB.magnitude ();
    double thetaAB = vecA.angleTo (&vecB);
    double dp = vecA.dotProduct (&vecB);
    double cp = vecA.crossProduct (&vecB);
    checkDouble (magA * magB * cos (thetaAB), dp, "DotProduct");
    checkDouble (magA * magB * sin (thetaAB), cp, "CrossProduct");

    double cp2 = vecA.crossProductSquared (&vecB);
    checkDouble (cp2, cp* cp, "crossProductSquared");

    }

void testSums ()
    {
    DVec2d vecA, vecB, vecC, vecA1, vecB2;
    vecA = DVec2d::FromXY (1,2);
    vecB = DVec2d::FromXY (3.5, 4.5);
    vecC.sumOf (&vecA, &vecB);
    vecA1.differenceOf (&vecC, &vecB);

    DPoint2d pointA, pointC;
    pointA.setComponents (vecA.x, vecA.y);
    pointC.setComponents (vecC.x, vecC.y);
    vecB2.differenceOf (&pointC, &pointA);

    checkDVec2d (&vecA, &vecA1, "C=A+B;D=C-B; D==A?");
    checkDVec2d (&vecB, &vecB2, "C.AsPoint-A.AsPoint");

    checkDouble (vecA.distance (&vecC), vecB.magnitude (), "Distance::Magnitude");
    checkDouble (vecA.distanceSquared (&vecC), vecB.magnitudeSquared (), "Distance::Magnitude");

    checkTrue (vecA.isEqual (&vecA), "A==A");
    checkFalse (vecA.isEqual (&vecB), "A!=B");

    double tol = 0.001;
    checkTrue (vecA.isEqual (&vecA, tol), "A==A");
    checkFalse (vecA.isEqual (&vecB, tol), "A!=B");

    DVec2d vecAA;

    vecAA = DVec2d::FromXY (vecA.x + tol * 0.5, vecA.y);
    checkTrue (vecA.isEqual (&vecAA, tol), "A==A(x+e)");

    vecAA = DVec2d::FromXY (vecA.x, vecA.y + tol * 0.5);
    checkTrue (vecA.isEqual (&vecAA, tol), "A==A(y+e)");

    vecAA = DVec2d::FromXY (vecA.x + tol * 0.5, vecA.y - 0.5 * tol);
    checkTrue (vecA.isEqual (&vecAA, tol), "A==A(x+=e, y-=e)");

    vecAA = DVec2d::FromXY (vecA.x + tol * 2.5, vecA.y - 0.5 * tol);
    checkFalse (vecA.isEqual (&vecAA, tol), "A==A(x+=2.5e, y-=e)");

    DVec2d vecC1;
    vecC1 = vecA;
    vecC1.add (&vecB);
    checkDVec2d (&vecC, &vecC1, "C:=A+B; C1:=A; C1+=B;");
    DVec2d vecB1 = vecC;
    vecB1.subtract (&vecA);

    checkDVec2d (&vecB, &vecB1, "[A+B]-A==B");

    DVec2d vecBs;
    double s = 1.234;
    DVec2d vecD, vecD1;
    vecD.sumOf (&vecA, &vecB, s);
    vecBs.scale (&vecB, s);
    vecD1.sumOf (&vecA, &vecBs);
    checkDVec2d (&vecD, &vecD1, "A+B*s = A + (B*s)");

    DVec2d vecD2;
    vecD2.interpolate (&vecA, s, &vecC);
    checkDVec2d (&vecD, &vecD2, "interpolate (A, s,[A+B])== A+sB");

    DVec2d vecE, vecEt;
    double t = 0.234;
    DVec2d vecF, vecF1;
    vecF = vecA;
    vecE = DVec2d::FromXY (-2.3, 5.242342);
    vecF.sumOf (&vecA, &vecB, s, &vecE, t);
    vecF1 = vecA;
    vecEt.scale (&vecE, t);
    vecF1.add (&vecBs);
    vecF1.add (&vecEt);
    checkDVec2d (&vecF, &vecF1, "A+Bs+Et");
    DVec2d vecG = DVec2d::FromXY (34.2, 352.1);
    DVec2d vecGu;
    double u = -0.23;
    vecGu.scale (&vecG, u);
    vecF1.add (&vecGu);
    vecF.sumOf (&vecA, &vecB, s, &vecE, t, &vecG, u);
    checkDVec2d (&vecF, &vecF1, "A+Bs+Et+Gu");
    }

void testAngles ()
    {
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testDVec2d ();
    testSums ();
    testAngles ();
    return getExitStatus();
    }

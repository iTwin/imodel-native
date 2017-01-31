// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <Geom/newton.h>
#include <printfuncs.h>
#include <stdlib.h>


void testEllipseIntersection
(
double aSize,
double offset0,
double offset1
)
    {
    DEllipse3d ellipse0, ellipse1;
    ellipse0.init (0.0, 0.0, 0.0,
                   2.0 * aSize, 0.0, 0.0,
                   0.0, aSize, 0.0,
                    0.0, msGeomConst_2pi);
    ellipse1.init (aSize, aSize, 0.0,
                   aSize, 0.0, 0.0,
                   0.0, aSize, 0.0,
                    0.0, msGeomConst_2pi);
    Function_DEllipse3d_AngleToXY evaluator0(ellipse0), evaluator1(ellipse1);
    double theta0 = 1.5;
    double theta1 = 3.1;
    NewtonIterationsRRToRR newton(1.0e-14);

    newton.RunNewtonDifference (theta0, theta1,
                evaluator0, evaluator1);

    Function_DEllipse3dOffset_AngleToXY evaluator0Offset (ellipse0, offset0);
    Function_DEllipse3dOffset_AngleToXY evaluator1Offset (ellipse1, offset1);
    double alpha0 = theta0;
    double alpha1 = theta1;
    newton.RunNewtonDifference (alpha0, alpha1, evaluator0Offset, evaluator1);
    DPoint3d xyz0, xyz1;
    ellipse0.evaluate (&xyz0, alpha0);
    ellipse1.evaluate (&xyz1, alpha1);
    checkDouble (fabs (offset0), xyz0.distance (&xyz1), "Offset::Ellipse distance");

    newton.RunNewtonDifference (alpha0, alpha1, evaluator0Offset, evaluator1Offset);
    double f0, g0, f1, g1;
    evaluator0Offset.EvaluateRToRR (alpha0, f0, g0);
    evaluator1Offset.EvaluateRToRR (alpha1, f1, g1);
    checkDouble (f0, f1, "Offset::Offset x");
    checkDouble (g0, g1, "Offset::Offset y");
    }

void testEllipseIntersection
(
double cxA, double cyA,
double uxA, double uyA,
double vxA, double vyA,
double offsetA,
double thetaA,
double cxB, double cyB,
double uxB, double uyB,
double vxB, double vyB,
double offsetB,
double thetaB
)
    {
    DEllipse3d ellipseA, ellipseB;
    ellipseA.init (cxA, cyA, 0.0,
                   uxA, uyA, 0.0,
                   vxA, vyA, 0.0,
                    0.0, msGeomConst_2pi);
    ellipseB.init (cxB, cyB, 0.0,
                   uxB, uyB, 0.0,
                   vxB, vyB, 0.0,
                   0.0, msGeomConst_2pi);

    Function_DEllipse3dOffset_AngleToXY evaluatorA (ellipseA, offsetA);
    Function_DEllipse3dOffset_AngleToXY evaluatorB (ellipseB, offsetB);
    NewtonIterationsRRToRR newton(1.0e-14);
    checkTrue (
            newton.RunNewtonDifference (thetaA, thetaB, evaluatorA, evaluatorB),
            "Newton Converged");
    double fA, gA, fB, gB;
    evaluatorA.EvaluateRToRR (thetaA, fA, gA);
    evaluatorB.EvaluateRToRR (thetaB, fB, gB);
    checkDouble (fA, fB, "Offset::points x");
    checkDouble (gA, gB, "Offset::points y");
    }



int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    selectTolerances (2);
    testEllipseIntersection (1.0, 0.1, 0.13);
    testEllipseIntersection (
                0,0,        3, -1,  1.0 / 3.0, 1.0, -0.1, 1.0,
                4,0,        2, 0,   0,3,             0.1, 3.0);




    testEllipseIntersection (1000.0, 10.0, 13.0);
    testEllipseIntersection (1000.0, -10.0, 13.0);
    testEllipseIntersection (1000.0, -10.0, -13.0);

    testEllipseIntersection (
                -9.7186960130929947, 19.190218150615692,
                130.54176865809941, -199.35867869557180,
                99.341723346077060,  65.049810482258948,
                -53.388856515194256,
                -0.88521407691074883,   // thetaA
                -4.4703483581542969e-008, 0.00000000000000000,
                280.94566358090185, 0.00000000000000000,
                7.1054273576010019e-015, 167.66183392390428,
                -53.388856515194256,
                -1.4647309966355109
                );



    return getExitStatus();
    }

/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include <testHarness.h>
#include <Geom/XYRangeTree.h>
//#include <Geom/Polynomials.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
#ifdef TestPolygonAccelerator
// @param [in] a0 = polygon value at x = 0;
// @param [in] a1 = polygon value at x = 1/order;
// @param [in] a2 = polygon value at x = 2/order;
// @param [in] order = bezier order.  (Assumed 2 or more.
// @param [out] fractionA = fractional postion where [a0,a1] segment intersects y=0.
// @param [out] fractionB = fractional position where [b0,b1] segment intersects y=0, where b0 and b1 are the altitudes of [a0,a1] and [a1,a2] at (fraction/(order - 1))
// @param [out] xB = x coordinate of the fractionB crossing.
bool TrySecondaryPolygonRoot (double a0, double a1, double a2, int order, double &fractionA, double &fractionB, double &xB)
    {
    fractionA = fractionB = 0.0;
    if (DoubleOps::SafeDivide (fractionA, -a0, a1 - a0, 0.0))
        {
        double uA = fractionA / (double) (order - 1);
        double b0 = DoubleOps::Interpolate (a0, uA, a1);
        double b1 = DoubleOps::Interpolate (a1, uA, a2);
        if (DoubleOps::SafeDivide (fractionB, -b0, b1 - b0, 0.0))
            {
            xB = DoubleOps::Interpolate (uA, fractionB,  (1 + uA)) / (double) (order - 1);
            return true;
            }
        }
    xB = 0.0;
    return false;
    }

double Bez2 (double u, double a0, double a1, double a2)
    {
    double v = 1.0 - u;
    return v * v * a0 + 2.0 * u * v * a1 + u * u * a2;
    }

void Subdivide
(
Polynomial::Bezier::Order3 const &parent,
Polynomial::Bezier::Order3 &left,
Polynomial::Bezier::Order3 &right,
double f
)
    {
    double b01 = DoubleOps::Interpolate (parent.coffs[0], f, parent.coffs[1]);
    double b12 = DoubleOps::Interpolate (parent.coffs[1], f, parent.coffs[2]);
    double b012 = DoubleOps::Interpolate (b01, f, b12);
    left = Polynomial::Bezier::Order3 (parent.coffs[0], b01, b012);
    right = Polynomial::Bezier::Order3 (b012, b12, parent.coffs[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bezier,PolygonAccelerator)
    {
    double a1 = 0.5; 
    int degree = 2;
    for (double a0 = -100.0; a0 < -0.001; a0 /= 2.0)
        {
        printf ("\n(a0 %g) (a1 %g)\n", a0, a1);
        for (double a2 = 0.0; a2 < 100.0; a2 = 2.0 * a2 + 1.0)
            {
            Polynomial::Bezier::Order3 bezier (a0, a1, a2);
            double roots[2];
            int numRoots = bezier.Roots (0.0, roots, true);
            Check::True (numRoots>0, "roots found");
            double fractionA, fractionB, xB;
            TrySecondaryPolygonRoot (a0, a1, a2, 3, fractionA, fractionB, xB);
            double yA = Bez2 (fractionA / (double)degree, a0, a1, a2);
            double yB = Bez2 (xB, a0, a1, a2);
            double q;
            DoubleOps::SafeDivide (q, yB, yA, DBL_MAX);
            printf ("(a %g) (fA %g) (fB %g) (uB %g ~ %9.2le) (r0 %g) (yA %g) (yb %g) (yb/ya %g)\n",
                        a2, fractionA, fractionB, xB, xB - roots[0], roots[0], yA, yB, q);
            Polynomial::Bezier::Order3 left, right;
            Subdivide (bezier, left, right, xB);
            double fractionC, fractionD, xD;
            if (left.coffs[0] * left.coffs[1] <= 0)
                {
                if (TrySecondaryPolygonRoot (left.coffs[0], left.coffs[1], left.coffs[2], 3,
                        fractionC, fractionD, xD))
                    {
                    //varunused double uD = xD * xB;
                    printf (" Left01 (fractionC %g) (fractionD %g) (xD %.15g) (yD %8.3le)\n",
                                fractionC, fractionD, xD, left.Evaluate (xD));
                    }
                }

            if (left.coffs[1] * left.coffs[2] <= 0)
                {
                if (TrySecondaryPolygonRoot (left.coffs[2], left.coffs[1], left.coffs[0], 3,
                        fractionC, fractionD, xD))
                    {
                    double uD = (1.0 - xD) * xB;
                    printf (" Left12 (fractionC %g) (fractionD %g) (xD %.15g ~ %9.2le) (yD %8.3le)\n",
                                fractionC, fractionD, uD,
                                uD - roots[0], bezier.Evaluate (uD));
                    }
                }

            }
        }
    }
#endif    
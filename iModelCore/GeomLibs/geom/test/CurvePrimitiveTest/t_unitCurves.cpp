/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

/* unused - static int s_noisy = 0;*/

#if defined(BENTLEY_WIN32)
static bool s_readFile = 0;
#endif

void TestTrackingCurveOffset (char const *message, ICurvePrimitiveR curve0, double d, double expectedLength, bool isCurved)
    {
    Check::StartScope (message);
    ICurvePrimitivePtr curve1 = ICurvePrimitive::CreateTrackingCurveXY (&curve0, d);
    for (double f : bvector<double> {0,0.2, 0.5, 0.9})
        {
        DRay3d xyz0 = curve0.FractionToPointAndUnitTangent (f);
        DRay3d xyz1 = curve1->FractionToPointAndUnitTangent (f);
        Check::Near (fabs (d), xyz0.origin.Distance (xyz1.origin), "Offset Curve point");
        Check::Near (xyz0.direction, xyz1.direction, "Offset curve unit tangent");
        }


    auto options = IFacetOptions::CreateForCurves ();
    static double s_defaultMaxEdgeLength = 4.0;
    options->SetMaxEdgeLength (s_defaultMaxEdgeLength);
    bvector<DPoint3d> stroke0, stroke1, stroke1A;
    curve0.AddStrokes (stroke0, *options);
    curve1->AddStrokes (stroke1, *options);
    auto strokeLengthSum1 = PolylineOps::SumSegmentLengths (stroke1);

    static double s_strokeLengthFraction = 0.45;
    //Check::Size (stroke0.size (), stroke1.size (), "Match strokes between base and offset");

    options->SetMaxEdgeLength (s_strokeLengthFraction * strokeLengthSum1.Max ());
    //Check::True (strokeLengthSum.Max () < 1.42 * s_defaultMaxEdgeLength);   // allow some fluff in max edge length!!!
    curve1->AddStrokes (stroke1A, *options);    // should have more points !!!
    double length1;
    curve1->Length (length1);


    auto strokeLengthSum1A = PolylineOps::SumSegmentLengths (stroke1A);
    double strokeLength1 = strokeLengthSum1.Sum ();
    double strokeLength1A = strokeLengthSum1A.Sum ();
    static double s_lengthFactor = 1.0 + Angle::SmallAngle ();
    if (Check::True (curve1->Length (length1), "Evaluate length"))
        {
        if (expectedLength > 0.0)
            Check::Near (expectedLength, length1, "Offset length");

        Check::True (strokeLength1 <= length1 * s_lengthFactor, "strokeLength < length");
        Check::True (strokeLength1A <= length1 * s_lengthFactor, "strokeLength < length");
        if (isCurved)
            {
            // finer stroking approaches length from below ..
            Check::True (strokeLength1 < strokeLength1A, "tighter stroke length");
            Check::True (strokeLength1A < length1, "strokeLength < length");
            }
        }
    
    Check::EndScope ();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive, Tracking)
    {
    double r0 = 10.0;
    double dr = 1.0;
    double radians = Angle::DegreesToRadians (90.0);
    auto curve0 = ICurvePrimitive::CreateArc
                    (
                    DEllipse3d::From (0,0,0, r0,0,0, 0,r0,0, 0.0, radians)
                    );
    TestTrackingCurveOffset ("Arc offset outside", *curve0, dr, radians * (r0 + dr), true);
    TestTrackingCurveOffset ("Arc offset inside", *curve0, -dr, radians * (r0 - dr), true);

    auto curve1 = ICurvePrimitive::CreateLine
                    (
                    DSegment3d::From (0,0,0, r0,0,0)
                    );
    TestTrackingCurveOffset ("Line offset outside", *curve1, dr, r0, false);
    TestTrackingCurveOffset ("Line offset inside", *curve1, -dr, r0, false);

    auto curve2 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
                (
                DSpiral2dBase::TransitionType_Clothoid,
                0.0, 0.0,
                100.0, 500.0,
                Transform::FromIdentity (),
                0.0, 1.0
                );
    TestTrackingCurveOffset ("Spiral right", *curve2, dr, 0.0, true);
    
    }
    
   // reltolR = relative tolerance for fraction of R.
void EvaluateClothoidTerms(double s, double L, double R, bvector<double> &terms, uint32_t minTerm, uint32_t maxTerm, double relTolR)
    {
    if (maxTerm > 32)
        maxTerm = 32;
    double tol = relTolR * R;
    terms.clear ();
    // here q is angle  q = s * s / (2 L R)
    // angle = s2 * A
    // generic series term is   Qk =  q^k/ k! = (1/k!) * s^(2k) * A^k
    // Hence Qk integrates to   Uk = (1/ (2k + 1)(1/k!) * s* s ^2k / A^k = (s/(2k+1) * Wk
    //     and W(k+1) = s * s * Ak * Wk / (k+1) = B * Wk / (k+1)
    terms.push_back (s);
    double W = s;
    double B = s * s / (2.0 * L * R);
    double Q;
    uint32_t numAccept = 0;
    for (double k = 1; k < maxTerm; k++)
        {
        W = B * W / (double)(k);
        Q = W / (2.0 * k + 1.0);
        terms.push_back (Q);
        if (Q < tol)
            numAccept++;
        if (numAccept > 1 && k >= minTerm)
            return;
        }
    }

void SumClothoidTerms(bvector<double> &terms, double &x, double &y, double &lastXTerm, double &lastYTerm)
    {
    x = 0.0;
    y = 0.0;
    // sum in reverse order ...
    ptrdiff_t ix, iy;
    size_t n = terms.size ();
    if (n & 0x01)
        {
        ix = n - 1;
        iy = n - 2;
        }
    else
        {
        iy = n - 1;
        ix = n - 2;
        }
    double sx = ix & 0x02 ? -1.0 : 1.0;
    double sy = iy & 0x02 ? -1.0 : 1.0;
    lastXTerm = terms[ix];
    lastYTerm = terms[iy];

    for (ptrdiff_t i = ix; i >= 0; i -= 2, sx *= -1.0)
        x += sx * terms[i];
    for (ptrdiff_t i = iy; i >= 0; i -= 2, sy *= -1.0)
        y += sy * terms[i];
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClothoidSeries, BareTerms)
    {
    double L = 100.0;
    bvector<double>terms;
    double x, y, dx, dy;
    for (double relTol : {1.0e-9, 1.0e-12, 1.0e-13, 1.0e-14, 1.0e-15})
        {
        double rxMax = 0.0;
        double ryMax = 0.0;
        for (double rFactor : {4.0, 10.0, 20.0})
            {
            double R = rFactor * L;
            auto spiral = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
                (
                DSpiral2dBase::TransitionType_Clothoid,
                0.0, 0.0,
                L, R,
                Transform::FromIdentity (),
                0.0, 1.0
                );
            printf ("\n Series Clothoid R = %lg   L = %lg  relTol = %.2le\n", R, L, relTol);
            for (double f : { 0.0, 0.1, 0.2, 0.7, 0.8, 0.9, 1.0 })
                {
                EvaluateClothoidTerms (f * L, L, R, terms, 2, 20, relTol);
                SumClothoidTerms (terms, x, y, dx, dy);
                DPoint3d xyz;
                spiral->FractionToPoint(f, xyz);
                double ex, ey;
                DoubleOps::SafeDivide (ex, xyz.x - x, x, 0.0);
                DoubleOps::SafeDivide (ey, xyz.y - y, y, 0.0);
                printf (" (%d)  (x %.14g    %8.1le %8.1le)      (y %.14g    %8.1le  %8.1le)\n", (int) terms.size (), x, dx, ex, y, dy, ey);
                Check::LessThanOrEqual(ex, relTol);
                Check::LessThanOrEqual(ey, relTol);
                rxMax = DoubleOps::MaxAbs (ex, rxMax);
                ryMax = DoubleOps::MaxAbs(ey, ryMax);
                }
            }
        printf ("      relTol %le   rxMax %le     ryMax=%le\n\n", relTol, rxMax, ryMax);
        }
    }

/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

/* unused - static int s_noisy = 0;*/

#if defined(BENTLEY_WIN32)
static bool s_readFile = 0;
#endif

void TestTrackingCurveOffset (char const *message, ICurvePrimitiveR curve0, double d, double expectedLength, bool isCurved, double distanceRefFactor = 1.0)
    {
    Check::StartScope (message);
    ICurvePrimitivePtr curve1 = ICurvePrimitive::CreateTrackingCurveXY (&curve0, d);
    for (double f : bvector<double> {0,0.2, 0.5, 0.9})
        {
        DRay3d xyz0 = curve0.FractionToPointAndUnitTangent (f);
        DRay3d xyz1 = curve1->FractionToPointAndUnitTangent (f);
        Check::Near (fabs (d), xyz0.origin.Distance (xyz1.origin), "Offset Curve point", distanceRefFactor);
        Check::Near (xyz0.direction, xyz1.direction, "Offset curve unit tangent", distanceRefFactor);
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
            Check::Near (expectedLength, length1, "Offset length", distanceRefFactor);

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
* @bsimethod
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
    auto refDist = 1.0;
    TestTrackingCurveOffset ("Spiral right", *curve2, dr, 0.0, true, refDist);
    
    }
#ifdef TestClothoidSeres
// EDL June 5, 2020 This works, but is not applied anywhere.
// series- based clothoid rocks.    
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
* @bsimethod
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
        printf (" series relTol %lg\n", relTol);
        printf(" (f) (terms) (x lastTerm errorX) (y lastTerm errorY\n");
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
            auto placement = spiral->GetSpiralPlacementCP ();
            printf ("\n Series Clothoid R = %lg   L = %lg  relTol = %.2le\n", R, L, relTol);
            for (double f : { 0.0, 0.1, 0.2, 0.7, 0.8, 0.9, 1.0 })
                {
                EvaluateClothoidTerms (f * L, L, R, terms, 2, 20, relTol);
                SumClothoidTerms (terms, x, y, dx, dy);
                DPoint3d xyz;
                spiral->FractionToPoint(f, xyz);
                DVec2d delta;
                double errorBound;
                DSpiral2dBase::Stroke (*(placement->spiral),
                        0.0, f, 0.01, delta, errorBound);
                double ex, ey;
                DoubleOps::SafeDivide (ex, delta.x - x, x, 0.0);
                DoubleOps::SafeDivide (ey, delta.y - y, y, 0.0);

                // do output and error tests using only the DSpiral2dBase::Stroke.
                // On imodel tree with CurvePrimitiveSpiralCurve1 implementation, the curve evaluations
                //   used to be just as good.
                // But on production code on connect, length is computed from the bspline and is less precise, so ignore it.
                printf (" (%.14lg) (%d)  (x %.14g    %8.1le %8.1le)      (y %.14g    %8.1le  %8.1le)\n", 
                        f, (int) terms.size (),
                        x, dx, ex, y, dy, ey);
                Check::LessThanOrEqual(ex, relTol);
                Check::LessThanOrEqual(ey, relTol);
                rxMax = DoubleOps::MaxAbs (ex, rxMax);
                ryMax = DoubleOps::MaxAbs(ey, ryMax);
                }
            }
        printf ("      relTol %le   rxMax %le     ryMax=%le\n\n", relTol, rxMax, ryMax);
        }
    }
#endif

// old-style DGNJS for two civil alignments that are mostly the same but the second one has had mirror effects.
// (And they ARE different towards the end)
static char const* oldHzStr = R"({"DgnCurveVector":{"Member":[{"LineSegment":{"endPoint":[714568.61199878599,2140508.7679090519,0.0],"startPoint":[714569.32312496693,2140509.4709734549,0.0]}},{"CircularArc":{"placement":{"origin":[714956.39506909880,2140116.5382746095,0.0],"vectorX":[-0.70306440306569562,0.71112618088625956,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":551.56123481987436,"startAngle":0.0,"sweepAngle":1.5830225619769440}},{"LineSegment":{"endPoint":[714544.64748139505,2140484.0330950925,0.0],"startPoint":[714557.92448169715,2140497.9055342581,0.0]}},{"CircularArc":{"placement":{"origin":[714608.36324397475,2140423.0521681691,0.0],"vectorX":[-0.83032888598297028,0.55727366805033896,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":88.195078374680676,"startAngle":9.8761333406896590,"sweepAngle":-9.8761333406896590}},{"LineSegment":{"endPoint":[714449.05337149801,2140343.9446971165,0.0],"startPoint":[714535.13232279872,2140472.2009629989,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":56.132537378711518,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":60.334227876336385,"StartRadius":-373.38001524013157,"placement":{"origin":[714419.66321602417,2140297.7526394082,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[714744.10331986775,2140112.9520582431,0.0],"vectorX":[-0.75155248850420842,-0.65967329567228328,0.0],"vectorZ":[-0.0,0.0,-1.0]},"radius":373.38001524005739,"startAngle":70.940733345898323,"sweepAngle":-70.940733345898323}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":131.27496122187694,"EndRadius":-373.38001524013134,"SpiralType":"Clothoid","StartBearing":135.47665171950180,"StartRadius":0.0,"placement":{"origin":[714501.57302075916,2139827.3104554252,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[714510.78873388912,2139818.2468143445,0.0],"startPoint":[714501.57302075916,2139827.3104554252,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":135.47665171906394,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":131.27496122143975,"StartRadius":358.13998476013012,"placement":{"origin":[714547.31864683609,2139780.5194593584,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[714278.15765005699,2139544.2640752997,0.0],"vectorX":[0.68690907021064385,-0.72674337235529640,0.0],"vectorZ":[-0.0,0.0,1.0]},"radius":358.13998476008959,"startAngle":87.889030947284468,"sweepAngle":-87.889030947284468}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":43.385930274319087,"EndRadius":358.13998476013006,"SpiralType":"Clothoid","StartBearing":39.184239776694895,"StartRadius":0.0,"placement":{"origin":[714484.28529044439,2139249.8236597180,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[714203.95345824875,2139021.3189692753,0.0],"startPoint":[714484.28529044439,2139249.8236597180,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":39.184239777028409,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":34.982549279402598,"StartRadius":358.13998475998812,"placement":{"origin":[714162.44961636770,2138989.1441504993,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[713957.11832249945,2139282.5778030376,0.0],"vectorX":[-0.60785368591774402,-0.79404905170663886,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":358.13998475958715,"startAngle":72.417021039617353,"sweepAngle":-72.417021039617353}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":-37.434471760175299,"EndRadius":358.13998475998812,"SpiralType":"Clothoid","StartBearing":-41.636162257801118,"StartRadius":0.0,"placement":{"origin":[713699.33224160771,2139032.1180279083,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[713193.10174446821,2139482.1421789881,0.0],"startPoint":[713699.33224160771,2139032.1180279083,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":-41.636162258295180,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":-37.434471760672707,"StartRadius":-373.38001524034013,"placement":{"origin":[713151.30643921834,2139517.5065663452,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[712924.34602070670,2139221.0245193178,0.0],"vectorX":[-0.47231906196096468,0.88142765086438857,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":373.38001524016670,"startAngle":65.619409226107464,"sweepAngle":-65.619409226107464}},{"TransitionSpiral":{"ActiveEndFraction":0.0,"ActiveStartFraction":1.0,"EndBearing":28.184937466225609,"EndRadius":-373.38001524034001,"SpiralType":"Clothoid","StartBearing":32.386627963848092,"StartRadius":0.0,"placement":{"origin":[712701.05537447985,2139521.9453702969,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[712307.16613997531,2139272.1045934879,0.0],"startPoint":[712701.05537447985,2139521.9453702969,0.0]}},{"CircularArc":{"placement":{"origin":[712354.40604563407,2139197.6279987688,0.0],"vectorX":[-0.53562972600218572,0.84445295702177714,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":88.195078363831030,"startAngle":0.0,"sweepAngle":9.8761333416004220}},{"LineSegment":{"endPoint":[712280.88114558987,2139249.9843186028,0.0],"startPoint":[712295.09206342942,2139262.8983853357,0.0]}},{"CircularArc":{"placement":{"origin":[712651.82353683561,2138841.7913028770,0.0],"vectorX":[-0.69271969642459719,0.72120692050576851,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":551.56123477838503,"startAngle":1.5830225620719582,"sweepAngle":-1.5830225620719582}},{"LineSegment":{"endPoint":[712269.02499879990,2139238.8883627853,0.0],"startPoint":[712269.74620572035,2139239.5810824819,0.0]}}],"boundaryType":1}})";
static char const* newHzStr = R"({"DgnCurveVector":{"Member":[{"LineSegment":{"endPoint":[714568.61199878599,2140508.7679090519,0.0],"startPoint":[714569.32312496693,2140509.4709734549,0.0]}},{"CircularArc":{"placement":{"origin":[714956.39506909880,2140116.5382746095,0.0],"vectorX":[-0.70306440306569562,0.71112618088625956,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":551.56123481987436,"startAngle":0.0,"sweepAngle":1.5830225619630758}},{"LineSegment":{"endPoint":[714544.64748139505,2140484.0330950925,0.0],"startPoint":[714557.92448169715,2140497.9055342581,0.0]}},{"CircularArc":{"placement":{"origin":[714608.36324397475,2140423.0521681691,0.0],"vectorX":[-0.72244124903261664,0.69143231172414332,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":88.195078374906799,"startAngle":0.0,"sweepAngle":9.8761333407934639}},{"LineSegment":{"endPoint":[714449.05337149778,2140343.9446971165,0.0],"startPoint":[714535.13232279872,2140472.2009629989,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":-119.66577212391208,"EndRadius":373.38001524013157,"SpiralType":"Clothoid","StartBearing":-123.86746262153694,"StartRadius":0.0,"placement":{"origin":[714449.05337149778,2140343.9446971165,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[714744.10331986775,2140112.9520582431,0.0],"vectorX":[-0.86892734104887726,0.49493966902818393,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":373.38001524006910,"startAngle":0.0,"sweepAngle":70.940733345889470}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":-44.523348280227623,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":-48.725038777852482,"StartRadius":373.38001524013123,"placement":{"origin":[714463.48864025646,2139866.6432330515,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[714510.78873388947,2139818.2468143450,0.0],"startPoint":[714501.57302075916,2139827.3104554252,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":-48.725038778553497,"EndRadius":-358.13998476013012,"SpiralType":"Clothoid","StartBearing":-44.523348280929319,"StartRadius":0.0,"placement":{"origin":[714510.78873388947,2139818.2468143450,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[714278.15765005699,2139544.2640752997,0.0],"vectorX":[0.75155248850412748,0.65967329567237565,0.0],"vectorZ":[0.0,0.0,-1.0]},"radius":358.13998476001211,"startAngle":0.0,"sweepAngle":87.889030947284468}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":-140.81576022367960,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":-136.61406972605539,"StartRadius":-358.13998476013023,"placement":{"origin":[714524.16725399403,2139283.9882149994,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[714203.95345824875,2139021.3189692753,0.0],"startPoint":[714484.28529044439,2139249.8236597180,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":-145.01745072059231,"EndRadius":-358.13998475998812,"SpiralType":"Clothoid","StartBearing":-140.81576022296650,"StartRadius":0.0,"placement":{"origin":[714203.95345824875,2139021.3189692753,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[713957.11832249945,2139282.5778030376,0.0],"vectorX":[0.57332691854019646,-0.81932670191883950,0.0],"vectorZ":[0.0,-0.0,-1.0]},"radius":358.13998475959272,"startAngle":0.0,"sweepAngle":72.417021039617353}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":138.36383774248401,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":142.56552824010984,"StartRadius":-358.13998475998818,"placement":{"origin":[713739.42161268869,2138998.1970877605,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[713334.70741273311,2139356.2588688987,0.0],"startPoint":[713699.33224160771,2139032.1180279083,0.0]}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":140.87459909614290,"EndRadius":624.84124968249932,"SpiralType":"Clothoid","StartBearing":138.36383774222358,"StartRadius":0.0,"placement":{"origin":[713334.70741273311,2139356.2588688987,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"CircularArc":{"placement":{"origin":[712898.96842399985,2138907.3071029913,0.0],"vectorX":[0.63101978963658456,0.77576673368158844,0.0],"vectorZ":[0.0,-0.0,1.0]},"radius":624.84124968236142,"startAngle":0.0,"sweepAngle":69.001267513679210}},{"TransitionSpiral":{"ActiveEndFraction":1.0,"ActiveStartFraction":0.0,"EndBearing":-147.61337203684104,"EndRadius":0.0,"SpiralType":"Clothoid","StartBearing":-150.12413339076036,"StartRadius":624.84124968249921,"placement":{"origin":[712587.72090582433,2139449.1110986709,0.0],"vectorX":[1.0,0.0,0.0],"vectorZ":[0.0,0.0,1.0]}}},{"LineSegment":{"endPoint":[712307.16613997531,2139272.1045934879,0.0],"startPoint":[712541.05711306818,2139420.4597585634,0.0]}},{"CircularArc":{"placement":{"origin":[712354.40604563407,2139197.6279987688,0.0],"vectorX":[-0.53562972600218572,0.84445295702177714,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":88.195078363831030,"startAngle":0.0,"sweepAngle":9.8761333416989725}},{"LineSegment":{"endPoint":[712280.88114558987,2139249.9843186028,0.0],"startPoint":[712295.09206342942,2139262.8983853357,0.0]}},{"CircularArc":{"placement":{"origin":[712651.82353683561,2138841.7913028770,0.0],"vectorX":[-0.67253165715075958,0.74006835503894730,0.0],"vectorZ":[0.0,0.0,1.0]},"radius":551.56123477855738,"startAngle":0.0,"sweepAngle":1.5830225620885936}},{"LineSegment":{"endPoint":[712269.02499879990,2139238.8883627853,0.0],"startPoint":[712269.74620572035,2139239.5810824819,0.0]}}],"boundaryType":1}})";

TEST(Spiral, CommonPartsWithReversal)
    {
    bvector<IGeometryPtr> oldGeometry, newGeometry;
    static int s_maxVolume = 1000;
    if (   Check::True (BentleyGeometryJson::TryJsonStringToGeometry(oldHzStr, oldGeometry))
        && Check::True(BentleyGeometryJson::TryJsonStringToGeometry(newHzStr, newGeometry))
            && oldGeometry.size() == 1
            && newGeometry.size() == 1)
        {
        auto oldCV = oldGeometry.front ()->GetAsCurveVector ();
        auto newCV = newGeometry.front ()->GetAsCurveVector();
        if (Check::True (oldCV.IsValid (), "oldCV")
            && Check::True (newCV.IsValid (), "newCV"))
            {
            Check::SaveTransformed (*oldCV);
            Check::SaveTransformed (*newCV);
            auto pathA = CurveVectorWithDistanceIndex::Create();
            auto pathB = CurveVectorWithDistanceIndex::Create();
            pathA->SetPath(oldCV);
            pathB->SetPath(newCV);
            bvector<PathLocationDetailPair> intervalsA, intervalsB;
            CurveVectorWithDistanceIndex::FindCommonSubPaths(*pathA, *pathB, intervalsA, intervalsB, true, false);
            
            auto oldNoisy = Check::SetMaxVolume(s_maxVolume);
            size_t count = 0;
            for (size_t i = 0; i < intervalsA.size(); i++)
                {
                if (s_maxVolume > 0)
                    {
                    printf("\n** %s *******\n", intervalsA[i].GetTagA() == 0 ? "split" : "shared");
                    auto dA0 = intervalsA[i].DetailA().DistanceFromPathStart();
                    auto dA1 = intervalsA[i].DetailB().DistanceFromPathStart();
                    auto dB0 = intervalsB[i].DetailA().DistanceFromPathStart();
                    auto dB1 = intervalsB[i].DetailB().DistanceFromPathStart();
                    Check::PrintIndent(2);
                    Check::Print(intervalsA[i].DetailA());
                    Check::Print(intervalsA[i].DetailB());
                    Check::Print (dA1 - dA0);
                    Check::PrintIndent(2);
                    Check::Print(intervalsB[i].DetailA());
                    Check::Print(intervalsB[i].DetailB());
                    Check::Print(dB1 - dB0);
/*
                    Check::PrintIndent (4);
                    Check::Print (dA0);
                    Check::Print(dA1);
                    Check::Print (dA1 - dA0);
                    Check::PrintIndent(4);
                    Check::Print(dB0);
                    Check::Print(dB1);
                    Check::Print(dB1 - dB0);
*/
                    }
                if (intervalsA[i].GetTagA() != 0)
                    count++;
                }
            oldNoisy = Check::SetMaxVolume(oldNoisy);
            bvector<PathLocationDetailPair> intervalsA1, intervalsA2;
            CurveVectorWithDistanceIndex::FindCommonSubPaths(*pathA, *pathA, intervalsA1, intervalsA2, true, false);
            bvector<PathLocationDetailPair> intervalsB1, intervalsB2;
            CurveVectorWithDistanceIndex::FindCommonSubPaths(*pathB, *pathB, intervalsB1, intervalsB2, true, false);
            // For this test case . . .we know exactly 3 curves fail between A and B.
            Check::Size (count + 3, oldCV->size (), "expect missing pieces comparing forward to reversed dataset");
            Check::Size(intervalsA1.size(), oldCV->size(), "expect full match comparing forward to itself");
            Check::Size(intervalsB1.size(), oldCV->size(), "expect full match comparing reverse to itself");
            }
        }
    Check::ClearGeometry("Spiral.CommonPartsWithReversal");
    }

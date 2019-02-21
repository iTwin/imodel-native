//
//
#include "testHarness.h"
#include "SampleGeometry.h"
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

static bool s_noisy = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, BinomialCoefficients)
    {
    int maxRow = 80;    // see bezeval.cpp
    // hm.. we expect "doubles" to be exact for binomial coefficients up to row 59.
    int lastExactRow = 59;
    for (int k = 1; k <= maxRow; k++)     // row k
        {
        Check::True (bsiBezier_getBinomialCoefficient (0, k) == 1.0, "Left 1");
        Check::True (bsiBezier_getBinomialCoefficient (k, k) == 1.0, "Right 1");
        for (int i = 1; i < k; i++)
            {
            double b0 = bsiBezier_getBinomialCoefficient (i-1,k-1) + bsiBezier_getBinomialCoefficient (i,k-1);
            double b1 = bsiBezier_getBinomialCoefficient (i,k);
            if (k <= lastExactRow)
                Check::True (b0 == b1, "Binomial Coffs EXACT");
            else
                Check::Near (b0, b1, "High order binomial coffs (approximate)");
            }
        }
    printf ("Binomial comparison to row %d\n", maxRow);
    }


void Exercise (MSBsplineCurveCR curve)
    {
    DVec3d derivatives[100];
    int order = curve.GetIntOrder ();
    double knotA, knotB;
    int indexA, indexB;
    double tolerance;
    curve.GetKnotRange (knotA, knotB, indexA, indexB, tolerance);
    curve.ComputeDerivatives (derivatives, order - 1, 0.5 * (knotA + knotB));


    BCurveSegment segment;
    for (size_t spanIndex = 0; curve.GetBezier (segment, spanIndex); spanIndex++)
        {
        if (s_noisy)
            printf ("(span %d) (segment range %g %g)\n", (int)spanIndex, segment.FractionToKnot (0.0), segment.FractionToKnot (1.0));
         if (!segment.IsNullU ())
            {
            for (double segmentFraction = 0.0; segmentFraction <= 1.0; segmentFraction += 0.25)
                {
                DPoint3d segmentPoint, curvePoint;
                double knot = segment.FractionToKnot (segmentFraction);
                if (s_noisy)
                    printf ("    (segmentFraction %g) (curveKnot %g)\n", segmentFraction, knot);
                segment.FractionToPoint (segmentPoint, segmentFraction);
                double curveFraction = curve.KnotToFraction (knot);
                curve.FractionToPoint (curvePoint, curveFraction);
                Check::Near (curvePoint, segmentPoint, "curve, segment evaluations");
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, Create1)
    {
    bvector<DPoint3d> points;
points.push_back (DPoint3d::From(-5767.07, 2719.07, 13817.60));
points.push_back (DPoint3d::From(-5767.07, 2719.07, 13809.24));
points.push_back (DPoint3d::From(-5778.20, 2730.21, 13795.10));
points.push_back (DPoint3d::From(-5796.03, 2480.26, 13792.52));
points.push_back (DPoint3d::From(-5803.27, 2755.27, 13796.70));
points.push_back (DPoint3d::From(-5810.50, 2762.50, 13800.88));
points.push_back (DPoint3d::From(-5817.19, 2769.19, 13817.60));
points.push_back (DPoint3d::From(-5810.50, 2762.50, 13834.32));
points.push_back (DPoint3d::From(-5803.27, 2755.27, 13838.50));
points.push_back (DPoint3d::From(-5796.03, 2748.03, 13842.68));
points.push_back (DPoint3d::From(-5778.21, 2730.21, 13840.10));
points.push_back (DPoint3d::From(-5767.07, 2719.07, 13825.96));
points.push_back (DPoint3d::From(-5767.07, 2719.07, 13817.60));
// 13 points
bvector<double> weights;
weights.push_back (1.0);
weights.push_back (0.83333333333333326);
weights.push_back (0.72222222222222221);
weights.push_back (0.83333333333333326);
weights.push_back (1.0);
weights.push_back (0.83333333333333348);
weights.push_back (0.7222222222222221);
weights.push_back (0.83333333333333326);
weights.push_back (1.0);
weights.push_back (0.83333333333333326);
weights.push_back (0.72222222222222221);
weights.push_back (0.83333333333333326);
weights.push_back (1.0);
// 13 weights
bvector<double>knots;
knots.push_back (0.0);
knots.push_back (0.0);
knots.push_back (0.0);
knots.push_back (0.11111111111111116);
knots.push_back (0.22222222222222221);
knots.push_back (0.33333333333333337);
knots.push_back (0.33333333333333337);
knots.push_back (0.44444444444444442);
knots.push_back (0.55555555555555558);
knots.push_back (0.66666666666666674);
knots.push_back (0.66666666666666674);
knots.push_back (0.77777777777777779);
knots.push_back (0.88888888888888884);
knots.push_back (1.0);
knots.push_back (1.0);
knots.push_back (1.0);
// 16 knots

MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (points, &weights, &knots, 3, false, false);
Exercise (*curve);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, Create2)
{
    bvector<DPoint3d> points;
points.push_back (DPoint3d::From(58.7375, 78.5876, 19.05));
points.push_back (DPoint3d::From(58.7375, 84.9376, 19.05));
points.push_back (DPoint3d::From(58.7375, 84.9376, -19.05));
points.push_back (DPoint3d::From(58.7375, 78.5876, -19.05));
points.push_back (DPoint3d::From(58.7375, 78.5876, 19.05));
// 5 points

bvector<double>knots;
knots.push_back (0.0);
knots.push_back (0.0);
knots.push_back (0.0714285714285714);
knots.push_back (0.5);
knots.push_back (0.5714285714285714);
knots.push_back (1.0);
knots.push_back (1.0);
// 7 knots

MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder(points, NULL, &knots, 2, false, false);

Exercise (*curve);

}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, PeriodicEvaluation)
{
    bvector<DPoint3d> points;
points.push_back (DPoint3d::From ( 0, 0,0));
points.push_back (DPoint3d::From (10,0,0));
points.push_back (DPoint3d::From (10,10,0));
points.push_back (DPoint3d::From ( 0,10,0));
// 4 points

bvector<double>knots;
knots.push_back (-0.75);
knots.push_back (-0.50);
knots.push_back (-0.25);
knots.push_back ( 0.00);
knots.push_back ( 0.25);
knots.push_back ( 0.50);
knots.push_back ( 0.75);
knots.push_back ( 1.00);
knots.push_back ( 1.25);
knots.push_back ( 1.50);
knots.push_back ( 1.75);
// 11 knots

MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder(points, NULL, &knots, 4, true, false);
Exercise (*curve);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, Circle)
{
MSBsplineCurve curve;
DEllipse3d ellipse;
ellipse.Init (0,0,0,    10,0,0, 0,10,0,   0.0, Angle::TwoPi ());
curve.InitFromDEllipse3d (ellipse);

Exercise (curve);
curve.ReleaseMem ();
}

double Integrate_ArcLength_xSquaredOver2 (double x)
    {
    double r = sqrt (1 + x * x);
    return 0.5 * (x * r + log (x + r));
    }


// Create degree 2 spline for the curve y=x^2 from x=0 to x=1 but knots specified by the caller.
MSBsplineCurvePtr CreateXSqaured (double knot0, double knot1)
    {
    bvector<DPoint3d> poles;
    poles.push_back (DPoint3d::From (0,0,0));
    poles.push_back (DPoint3d::From (0.5, 0,0));
    poles.push_back (DPoint3d::From (1,0.5,0));
    bvector<double> knots;
    knots.push_back (knot0);
    knots.push_back (knot0);
    knots.push_back (knot0);
    knots.push_back (knot1);
    knots.push_back (knot1);
    knots.push_back (knot1);
    return MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, &knots, 3, false, true);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, ArcLengthMappings)
    {
    double k0 = 2.0;
    double k1 = 6.0;
    MSBsplineCurvePtr curve1 = CreateXSqaured (k0, k1);
    
    int numAdd = 6;
    static double s_extraFraction[] = {0.25, 0.4,0.1, 0.78, 0.397, 0.667, 0.94, 0.5};
    for (int i = 0; i < numAdd; i++)
        {
        for (double f = 0.0; f <= 1.0; f += 0.5)
            {
            DPoint3d xyz;
            double x = f;
            double y = x * x * 0.5;
            curve1->FractionToPoint (xyz, f);
            Check::Near (x, xyz.x, "construction x");
            Check::Near (y, xyz.y, "consturction y");
            double trueLength = Integrate_ArcLength_xSquaredOver2 (x);
            double fractionalLength = curve1->LengthBetweenFractions (0.0, f);
            double knotLength = curve1->LengthBetweenKnots (curve1->FractionToKnot (0.0), curve1->FractionToKnot (f));
                            
            Check::Near (trueLength, fractionalLength, "LengthBetweenFractions");
            Check::Near (trueLength, knotLength, "LengthBetweenKnots");
            
            if (f > 0.1 && f < 0.9)
                {
                double f0 = 0.5 * f;
                double f1 = 0.5 * (f + 1);
                double trueDelta0 = Integrate_ArcLength_xSquaredOver2 (f0)
                                - Integrate_ArcLength_xSquaredOver2 (f);
                double trueDelta1 = Integrate_ArcLength_xSquaredOver2 (f1)
                                - Integrate_ArcLength_xSquaredOver2 (f);
                                
                double g0, g1;
                double delta0, delta1;
                Check::True(curve1->FractionAtSignedDistance (f, trueDelta0, g0, delta0));
                Check::True(curve1->FractionAtSignedDistance (f, trueDelta1, g1, delta1));
                Check::Near (g0, f0);
                Check::Near (g1, f1);
                }
            }
        curve1->AddKnot (curve1->FractionToKnot (s_extraFraction[i]), 1);
        }
    }
    
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, AdvanceAndRetreat)
    {
    MSBsplineCurvePtr curve1 = CreateXSqaured (2.0, 6.0);

    int numAdd = 6;
    static double s_extraFraction[] = {0.25, 0.4,0.1, 0.78, 0.397, 0.667, 0.94, 0.5};
    bvector<DRange1d> ranges;
    ranges.push_back (DRange1d::From (0.0, 1.0));
    ranges.push_back (DRange1d::From (0.0, 0.15));
    ranges.push_back (DRange1d::From (0.0, 0.1));
    ranges.push_back (DRange1d::From (0.4, 0.45));
    ranges.push_back (DRange1d::From (0.25, 0.5));
    ranges.push_back (DRange1d::From (0.75, 1.0));
    BCurveSegment segment;
    for (int i = 0; i < numAdd; i++)
        {
        for (size_t j = 0; j < ranges.size (); j++)
            {
            DRange1d fractionRange = ranges[j];
            DRange1d knotRange = DRange1d::From (curve1->FractionToKnot (fractionRange.low), curve1->FractionToKnot (fractionRange.high));
            size_t numAdvance = 0;
            size_t numRetreat = 0;
            double advanceSize = 0.0;
            double retreatSize = 0.0;
            for (size_t select = 0; curve1->AdvanceToBezierInKnotInterval (segment, select, knotRange);)
                {
                advanceSize += segment.UMax () - segment.UMin ();
                numAdvance++;
                }
                
            for (size_t select = curve1->GetTailBezierSelect (); curve1->RetreatToBezierInKnotInterval (segment, select, knotRange);)
                {
                retreatSize += segment.UMax () - segment.UMin ();
                numRetreat++;
                }
            Check::Size (numAdvance, numRetreat, "Advance and retreat interval search");
            Check::Near (advanceSize, retreatSize, "Advance and retreat knot support");
            }
        curve1->AddKnot (curve1->FractionToKnot (s_extraFraction[i]), 1);
        }
    }

/*--------------------------------------------------------------------------------**//**
// @return true if a,b are effectively the same weight
+--------------------------------------------------------------------------------------*/
bool SameWeight (double a, double b)
    {
    static double s_weightTol = 1.0e-14;
    return fabs (a - b) < s_weightTol;
    }
/*--------------------------------------------------------------------------------**//**
// update max distance between vector and center parts of two ellipses.
+--------------------------------------------------------------------------------------*/
void UpdateEllipseDiffs
(
double &maxVectorDiff,
double &maxCenterDiff,
DEllipse3d const &ellipse0,
DEllipse3d const &ellipse1
)
    {
    double dx = ellipse0.center.Distance (ellipse1.center);
    double du = ellipse0.vector0.Distance (ellipse1.vector0);
    double dv = ellipse0.vector90.Distance (ellipse1.vector90);
    if (dx > maxCenterDiff)
        maxCenterDiff = dx;
    if (du > maxVectorDiff)
        maxVectorDiff = du;
    if (dv > maxVectorDiff)
        maxVectorDiff = dv;
    }
    
/*--------------------------------------------------------------------------------**//**
// Convert msbsplineCurve to major/minor DEllipse3d.
// @return false if any fussy detail is different from the usual way an ellipse is converted to bspline curve.
// @param [in] curve source bspline curve.
// @param [out] ellipse (if possible) ellipse form.
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool bspcurv_curveToDEllipse3d (MSBsplineCurveCR curve, DEllipse3dR ellipse)
    {
    DEllipse3d sectorEllipse[10];
    int numSector = curve.params.numPoles / 2;
    if (numSector > 10)
        return false;
    if (curve.params.numPoles != 2 * numSector + 1)
        return false;
    if (curve.params.order != 3)
        return false;
    if (curve.weights == NULL)
        return false;
    // each sector should have weights (1,w,1) where w is cosine of half the sector angle ....
    for (int i = 0; i < numSector; i++)
        {
        int k0 = 2 * i;
        if (!SameWeight (curve.weights[k0], 1.0))
            return false;
        if (!SameWeight (curve.weights[k0], 1.0))
            return false;
        double w = curve.weights[k0+ 1];
        if (w <= 0.1 || w >= 1.0)
            return false;
        if (!SameWeight (w, curve.weights[1]))
            return false;
        }

    double w = curve.weights[1];
    double beta = acos (w);     // half the sector angle
    double theta = 2.0 * beta;  // full sector angle
    double maxDiff0 = 0.0;
    double maxDiff1 = 0.0;

    for (int sector = 0; sector < numSector; sector++)
        {
        int k0 = 2 * sector;
        RotMatrix circlePoles, inverseCircle;
        double alpha0 = sector * theta;     // sector start parameter angle advances by theta each sector
        double alpha1 = alpha0 + beta;
        double alpha2 = (sector + 1) * theta;
        // unit circle points with weights ...
        circlePoles.InitFromRowValues (
            cos (alpha0),     cos (alpha1),     cos(alpha2),
            sin (alpha0),     sin (alpha1),     sin (alpha2),
            1.0,              w,                1.0
            );
        DPoint4d hpole[3];
        DPoint4d epole[3];
        inverseCircle.InverseOf (circlePoles);
        for (int k = 0; k < 3; k++)
            {
            int kp = k0 + k;
            hpole[k].SetComponents (curve.poles[kp].x, curve.poles[kp].y, curve.poles[kp].z, curve.weights[kp]);
            }
        DPoint4d zero;
        zero.Zero ();
        for (int i = 0; i < 3; i++)
            epole[i].SumOf (zero,
                            hpole[0], inverseCircle.form3d[0][i],
                            hpole[1], inverseCircle.form3d[1][i],
                            hpole[2], inverseCircle.form3d[2][i]
                            );
        // WE EXPECT epole to be 2 simple vectors and a simple point...
        if (!SameWeight (epole[0].w, 0.0))
            return false;
        if (!SameWeight (epole[1].w, 0.0))
            return false;
        if (!SameWeight (epole[2].w, 1.0))
            return false;
        sectorEllipse[sector].Init (
            epole[2].x, epole[2].y, epole[2].z,
            epole[0].x, epole[0].y, epole[0].z,
            epole[1].x, epole[1].y, epole[1].z,
            alpha0, theta);
        if (sector > 0)
            UpdateEllipseDiffs (maxDiff0, maxDiff1, sectorEllipse[0], sectorEllipse[sector]);
        }
    double bigNum = sectorEllipse[0].center.MaxAbs ()
                  + sectorEllipse[0].vector0.MaxAbs ()
                  + sectorEllipse[0].vector90.MaxAbs ();
    static double s_relTol = 1.0e-10;
    double tol = bigNum * s_relTol;
    if (maxDiff0 > tol || maxDiff1 > tol)
        return false;
    DEllipse3d ellipseA = sectorEllipse[0];
    ellipseA.sweep = numSector * theta;    // Expand to cover all sectors.
    ellipse.InitWithPerpendicularAxes (ellipseA);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, EllipseRoundTrip)
    {
    for (double theta0 = 0.0; theta0 < 3.0; theta0 += 0.974)
        {
        for (double f = 0.5; f <= 2.0; f += 0.5)
            {
            DEllipse3d ellipse0 = DEllipse3d::From (0,0,0, 2,0,0, 0,1,0, theta0, f * Angle::Pi());
            MSBsplineCurvePtr bcurve = MSBsplineCurve::CreatePtr ();
            bcurve->InitFromDEllipse3d (ellipse0);
            DEllipse3d ellipse1;
            Check::True (bspcurv_curveToDEllipse3d (*bcurve, ellipse1), "BCurve to Ellipse");
            Check::Near (ellipse0.center, ellipse1.center, "Round trip center");
            if (bcurve->params.numPoles > 3)
                {
                bcurve->poles[bcurve->params.numPoles / 2].x += 1.0;
                Check::False (bspcurv_curveToDEllipse3d (*bcurve, ellipse1), "Altered BCurve to Ellipse");
                }
            for (double g = 0.0; g <= 1.0; g += 0.15)
                {
                DPoint3d point0, point1;
                ellipse0.FractionParameterToPoint (point0, g);
                ellipse1.FractionParameterToPoint (point1, g);
                Check::Near (point0, point1, "Fractional point match");
                }
            }
        }
    }
    
void checkCurve (char const *title, MSBsplineCurveCR curve, MSBsplineSurfaceCR surface,
    double u0, double v0, double u1, double v1, int n)
    {
    DPoint3d surfaceXYZ, curveXYZ;
    for (int i = 0; i <= n; i++)
        {
        double s = i / (double)n;
        double u = DoubleOps::Interpolate (u0, s, u1);
        double v = DoubleOps::Interpolate (v0, s, v1);
        surface.EvaluatePoint (surfaceXYZ, u, v);
        curve.FractionToPoint (curveXYZ, s);
        char message[1000];
        sprintf (message, "%s (s=%g) (uv %g %g) (surfaceXYZ %g,%g,%g) (curveXYZ %g,%g,%g)\n",
                    title, s,
                    u, v,
                    surfaceXYZ.x, surfaceXYZ.y, surfaceXYZ.z,
                    curveXYZ.x, curveXYZ.y, curveXYZ.z
                    );
        if (!Check::Near (curveXYZ, surfaceXYZ, message))
            break;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, Extract)
    {
    bvector<DPoint3d>poles;
    int numJ = 6;
    int numI = 5;
    double aI = 0.2;
    double aJ = 0.4;
    int uOrder = 3;
    int vOrder = 4;
    double q0I = 0.21;
    double q0J = 0.15;
    for (int j = 0; j < numJ; j++)
        for (int i = 0; i < numI; i++)
            {
            poles.push_back (DPoint3d::From ((double)i, (double)j, sin(q0I + aI * i) * sin (q0J + aJ * j)));
            }

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder
        (
        poles, NULL,
        NULL, uOrder, numI, false,
        NULL, vOrder, numJ, false,
        true                
        );
    int numTest = 20;
    MSBsplineCurvePtr uEdge0 = surface->GetPolygonRowAsCurve     (0);
    MSBsplineCurvePtr uEdge1 = surface->GetPolygonRowAsCurve    (-1);
    
    MSBsplineCurvePtr vEdge0 = surface->GetPolygonColumnAsCurve (0);
    MSBsplineCurvePtr vEdge1 = surface->GetPolygonColumnAsCurve (-1);
    
    checkCurve ("uEdge0", *uEdge0, *surface, 0,0, 1,0, numTest);
    checkCurve ("uEdge1", *uEdge1, *surface, 0,1, 1,1, numTest);
    checkCurve ("vEdge0", *vEdge0, *surface, 0,0, 0,1, numTest);
    checkCurve ("vEdge1", *vEdge1, *surface, 1,0, 1,1, numTest);
    
    double delta = 0.125;
    for (double q = 0.0; q <= 1.0; q += delta)
        {
        MSBsplineCurvePtr uEdge = surface->GetIsoVCurve (q);
        checkCurve ("uEdge", *uEdge, *surface, 0, q, 1,q, numTest);
        MSBsplineCurvePtr vEdge = surface->GetIsoUCurve (q);
        checkCurve ("vEdge", *vEdge, *surface, q, 0, q, 1, numTest);
        }
    }
    



double s_01band = 1.0e-9;
double Avoid01 (double u)
    {
    if (fabs (u) < s_01band)
        u = s_01band;
    else if (fabs (u-1) < s_01band)
        u = 1.0 - s_01band;
    return u;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, PlanarBilinear)
    {
    double q0I = 0.5;
    double q0J = 0.1;
    // This is planar !!!
    MSBsplineSurfacePtr planarSurface = SurfaceWithSinusoidalControlPolygon (2, 2, 3, 4, q0I, 0.0, q0J, 0.0);
    // This is not !!!
    MSBsplineSurfacePtr curvedSurface = SurfaceWithSinusoidalControlPolygon (2, 2, 3, 4, q0I, 0.22, q0J, 0.1);

    Check::True (planarSurface->IsPlanarBilinear (), "planar bilinear true case");
    Check::False (curvedSurface->IsPlanarBilinear (), "planar bilinear false case");

    DBilinearPatch3d patch0 (DPoint3d::From (0,0,0),
          DPoint3d::From (1,0,0),
          DPoint3d::From (0,1,0),
          DPoint3d::From (1,3,0));

    DBilinearPatch3d patch1 (DPoint3d::From (0,0,0),
          DPoint3d::From (1,0,0),
          DPoint3d::From (0,1,0),
          DPoint3d::From (1,1,0.01));


    Check::True (patch0.IsPlanar (), "Planar patch");
    Check::False (patch1.IsPlanar (), "NonPlanar patch");
    }
static double s_rayParameterToleranceShift = 1.0e1;   // allow lots digits more than usual for tolerance ..

void CheckRayIntersectSurface (MSBsplineSurfaceCR surface, DPoint2dCR uvA)
    {
    DRay3d rayA;
    bvector<DPoint3d> xyzB, xyzC;
    bvector<double> rayParamB, rayParamC;
    bvector<DPoint2d> uvB, uvC;
    surface.EvaluatePointAndUnitNormal (rayA, uvA.x, uvA.y);
    DRay3d rayB = DRay3d::FromOriginAndVector (rayA.origin, DVec3d::From (0,0,1));
    DRay3d rayC = rayB;
    double shift = 0.4;
    rayC.origin = rayB.FractionParameterToPoint (-shift);
    surface.IntersectRay (xyzB, rayParamB, uvB, rayB);
    surface.IntersectRay (xyzC, rayParamC, uvC, rayC);
    //  -- confirm xy
    if (Check::True (xyzB.size () > 0, "vertical ray to z=f(x,y) surface.  Expect 1 hit")
      && Check::Size (xyzB.size (), xyzC.size (), "ray reparameterization preserves root counts"))
        {
        double dMinB = DBL_MAX;
        double dMinC = DBL_MAX;
        for (size_t i = 0; i < xyzB.size (); i++)
            {
            Check::Near (DPoint2d::From (rayA.origin.x, rayA.origin.y),
                            DPoint2d::From (xyzB[i].x, xyzB[i].y),
                  "xy part of hit even if z not iterated!!!");
            Check::Near (DPoint2d::From (rayA.origin.x, rayA.origin.y),
                            DPoint2d::From (xyzC[i].x, xyzC[i].y),
                  "xy part of hit even if z not iterated!!!");
            dMinB = DoubleOps::Min (dMinB, fabs (rayParamB[i] - 0.0));
            dMinC = DoubleOps::Min (dMinC, fabs (rayParamC[i] - shift));
            }
        Check::Near (s_rayParameterToleranceShift, s_rayParameterToleranceShift + dMinB, "rayB should intersection patch at ray origin");
        Check::Near (s_rayParameterToleranceShift, s_rayParameterToleranceShift + dMinC, "rayC should intersection patch at ray parameter shift");
        }
    }



void CheckClosestPoint (MSBsplineSurfaceCR surface, DPoint2dCR uvA)
    {
    DRay3d rayA;
    surface.EvaluatePointAndUnitNormal (rayA, uvA.x, uvA.y);
    DPoint3d xyzA = rayA.origin;
    //Check::PushTolerance (ToleranceSelect_Loose);
    for (double offsetDistance = 1.0e-10; offsetDistance < 1.0e-3; offsetDistance *= 100.0)
        {
        Check::StartScope ("offsetDistance", offsetDistance);
        DPoint3d xyzB = rayA.FractionParameterToPoint (offsetDistance);
        DPoint2d uvC;
        DPoint3d xyzC;
        surface.ClosestPoint(xyzC, uvC, xyzB);
        Check::Near (xyzA, xyzC, "closest point");
        Check::Near (uvA, uvC, "closest point");
        Check::EndScope ();
        }
    //Check::PopTolerance ();
    }


void CheckPatch (MSBsplineSurfaceCR surface, size_t numUVTest)
    {
    for (size_t i = 0; i <= numUVTest; i++)
        {
        double u = i / (double)numUVTest;
        Check::StartScope ("u", u);
        for (size_t j = 0; j <= numUVTest; j++)
            {
            double v = j / (double)numUVTest;
            Check::StartScope ("v", v);
            DPoint2d uvA = DPoint2d::From (Avoid01(u), Avoid01(v));
            CheckRayIntersectSurface (surface, uvA);
            CheckClosestPoint (surface, uvA);
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, SinusoidalPatch0)
    {
    int numJ = 6;
    int numI = 5;
    double aI = 0.2;
    double aJ = 0.4;
    int uOrder = 3;
    int vOrder = 4;
    double q0I = 0.21;
    double q0J = 0.15;
    //numI = uOrder;
    //numJ = vOrder;
    MSBsplineSurfacePtr surface = SurfaceWithSinusoidalControlPolygon (uOrder, vOrder, numI, numJ, q0I, aI, q0J, aJ);
    CheckPatch (*surface, 4);
    }

#ifdef TestClosestPointPerformance
// Remark: 9/21/13
// ClosestPointBlanketTime has option to call bsprsurf_minDistToSurface.
// But that function now calls the ClosestPoint method.  So it's meaningless to compare.
// But the timing is still useful for checking optimizing activety in ClosestPoint.
// method == 0==>ClosestPoint
// method == 1==bsprsurf
double  ClosestPointBlanketTime
(
MSBsplineSurfaceCR surface,
double normalDist,
size_t numUVTest, 
size_t numCalls,
double tolerance
)
    {
    DPoint3d surfacePoint;
    DPoint2d uv;
    clock_t clock0 = clock ();
    for (size_t i = 0; i <= numUVTest; i++)
        {
        double u = i / (double)numUVTest;
        for (size_t j = 0; j <= numUVTest; j++)
            {
            double v = j / (double)numUVTest;
            DRay3d ray;
            surface.EvaluatePointAndUnitNormal (ray, u, v);
            DPoint3d spacePoint = ray.FractionParameterToPoint (normalDist);
            for (size_t k = 0; k < numCalls; k++)
                {
                surface.ClosestPoint (surfacePoint, uv, spacePoint);
                }
            }
        }
    clock_t clock1 = clock ();
    return (double)(clock1 - clock0);
    }

void PatchTiming (char * name, MSBsplineSurfaceCR surface, size_t numUVTest, size_t numCall, double normalDist)
    {
    printf ("*** Patch timing %s\n", name);
    printf ("  order (%d,%d)\n", surface.GetUOrder (), surface.GetVOrder ());
    printf ("  poles (%d,%d)\n", surface.GetNumUPoles (), surface.GetNumVPoles ());
    for (int select = 0; select < 6; select++)
        {
        bspsurf_setTimerControl (select,0);
        printf ("\n TIMER SELECT %d\n", select);
        printf (" %8.1lf   ClosestPoint\n", ClosestPointBlanketTime (surface, normalDist, numUVTest, numCall, 0.0));
        printf (" %8.1lf   ClosestPoint\n", ClosestPointBlanketTime (surface, normalDist, numUVTest, numCall, 0.0));
        }
    }


static size_t s_numI = 7;//17;
static size_t s_numJ = 8;//13;
static size_t s_numUVTest = 3;
static size_t s_numCall = 1;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, SinusoidalPatchTiming)
    {
    double aI = 0.2;
    double aJ = 0.8;
    int uOrder = 3;
    int vOrder = 4;
    double q0I = 0.21;
    double q0J = 0.15;
    //numI = uOrder;
    //numJ = vOrder;
    MSBsplineSurfacePtr surface = SurfaceWithSinusoidalControlPolygon (uOrder, vOrder, s_numI, s_numJ, q0I, aI, q0J, aJ);
    PatchTiming ("sinusoid", *surface, s_numUVTest, s_numCall, 0.01);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, HyperbolicPatchTiming)
    {
    double u1 = 1.2;
    double v1 = 1.4;
    double x1 = 1.1;
    double y1 = 1.2;
    double z1 = 1.0;
    for (size_t uOrder = 2; uOrder < 6; uOrder++)
        for (size_t vOrder = 2; vOrder < 6; vOrder++)
          {
          MSBsplineSurfacePtr surface = HyperbolicGridSurface (uOrder, vOrder, s_numI, s_numJ, x1, y1, z1, u1, v1);
          PatchTiming ("hyperbolic", *surface, s_numUVTest, s_numCall, 0.01);
          }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, BilinearPatch1)
    {
    MSBsplineSurfacePtr surface = SimpleBilinearPatch (1.0, 1.0, 1.0);
    CheckPatch (*surface, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, BilinearPatch2)
    {
    MSBsplineSurfacePtr surface = SimpleBilinearPatch (2.0, 2.0, 1.0);
    CheckPatch (*surface, 4);
    }

    
static int s_printBeziers = 0;    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, MakeBezier)
    {
    for (int order = 2; order < 7; order++)
        {
        for (int numPoles = order, step = 1; numPoles < 4 * order; numPoles += (step++))
            {
            double dtheta = Angle::Pi () / (numPoles - 1);
            bvector<DPoint3d>poles;
            for (int i = 0; i < numPoles; i++)
                {
                double theta = i * dtheta;
                poles.push_back (DPoint3d::From (cos(theta), sin (theta), 0.0));
                }
            MSBsplineCurvePtr bspline = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, order, false, true);
            MSBsplineCurve bezierCurve;
            Check::True (SUCCESS == bspcurv_makeBezier (&bezierCurve, bspline.get ()), "MakeBezier");
            int degree = order - 1;
            int spans  = numPoles - degree;
            if (Check::Int (1 + spans * degree, bezierCurve.params.numPoles, "Bezier pole count"))
                {
                if (s_printBeziers)
                    {
                    printf ("<bspline order= \"%d\" numPoles=\"%d\">\n", order, numPoles);
                    for (int i0 = 0; i0 + order - 1 < bezierCurve.params.numPoles; i0 += order - 1)
                        {
                        printf ("<bezier>\n");
                        for (int i = 0; i < order; i++)
                            {
                            DPoint3d xyz = bezierCurve.poles[i0 + i];
                            printf (" %#.17g,%#.17g,%#.17g\n", xyz.x, xyz.y, xyz.z);
                            }
                        printf ("</bezier>\n");
                        }
                    printf ("</bspline>");
                    }
                }
            bezierCurve.ReleaseMem ();                
            }
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,KnotSearch)
    {
    double knots[] =
        {
        0,0,0,
        1,2,2,3,
        4,4,4
        };
    int order = 3;
    int numKnots = sizeof (knots) / sizeof(double);
    int numPoles = numKnots - order;
    for (double  t = -0.5; t < 5.0; t += 0.5)
        {
        int spanIndex;
        bspknot_findSpan (&spanIndex, knots, numPoles, order, false, t);
        if (s_noisy)
            printf ("(t %g) (spanIndex %d) (knots %g, %g)\n", t, spanIndex, knots[spanIndex], knots[spanIndex+1]);
        if ( t > knots[order-1] && t <= knots[numKnots - order + 1])
            {
            Check::True (t > knots[spanIndex], "open left");
            Check::True (t <= knots[spanIndex + 1], "closed right");
            }
        }
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, MatchCurveKnots)
    {
    MSBsplineSurfacePtr surface = SimpleBilinearPatch (2.0, 2.0, 1.0);
    bvector<DPoint3d> point2;
    point2.push_back (DPoint3d::From (0.0, 0.0, 0.0));
    point2.push_back (DPoint3d::From (1.0, 0.0, 0.0));
    point2.push_back (DPoint3d::From (2.0, 0.0, 0.0));
    point2.push_back (DPoint3d::From (3.0, 0.0, 0.0));
    MSBsplineCurvePtr   curve2 = MSBsplineCurve::CreateFromPolesAndOrder (point2, NULL, NULL, 3, false, false);
    bvector<DPoint3d> point0;
    point0.push_back (DPoint3d::From (0,0,0));
    point0.push_back (DPoint3d::From (1,0,0));
    MSBsplineCurvePtr   curve0 = MSBsplineCurve::CreateFromPolesAndOrder (point0, NULL, NULL, 2, false, false);
    size_t numU0 = surface->GetNumUKnots ();
    size_t numV0 = surface->GetNumVKnots ();
    // These should be noop?
    bspsurf_matchCurveParams (surface.get (), curve0.get (), 0);
    bspsurf_matchCurveParams (surface.get (), curve0.get (), 1);
    size_t numU1 = surface->GetNumUKnots ();
    size_t numV1 = surface->GetNumVKnots ();
    Check::Size (numU0, numU1, "U knots should be unchanged");
    Check::Size (numV0, numV1, "V knots should be unchanged");
    // This should insert 0.5 as single, 0 and 1 for degree elevation
    bspsurf_matchCurveParams (surface.get (), curve2.get (), 0);
    size_t numU2 = surface->GetNumUKnots ();
    size_t numV2 = surface->GetNumVKnots ();
    Check::Size (numU1 + 3, numU2, "U knots should be incremented");
    Check::Size (numV1, numV2, "V knots should be unchanged");
    
        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline,KnotByDistance)
    {
    bvector<DPoint3d> points;
    for (int power = 1; power < 4; power++)
        {
        points.clear ();
        for (size_t i = 0; i < 11; i++)
            points.push_back (DPoint3d::From (pow((double)i, power), 0, 0));
        double knots[30];
        for (int order = 2; order < 6; order++)
            {
            for (int i = 0; i < 20; i++)
                knots[i] = 0.0;
            mdlBspline_computeChordLengthKnotVector (knots, &points[0], nullptr, (int)points.size (), order, false);
            }
        }
    }


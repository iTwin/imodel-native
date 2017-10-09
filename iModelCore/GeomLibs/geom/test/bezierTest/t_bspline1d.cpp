#include "testHarness.h"

void exerciseBspline1d (bvector<double> &yValues, bool noisy = false)
    {
    bvector<DPoint3d> xyz;

    for (double y : yValues)
        {
        xyz.push_back (DPoint3d::From (y, y, (double)xyz.size ()));
        }
    if (noisy)
        {
        for (double t : yValues)
            {
            GEOMAPI_PRINTF ("  %16.4f\n", t);
            }
        }

    for (int order = 2; order <= 4; order++)
        {        
        double du = 1.0 / (double)(yValues.size () - (order - 1) );        
        size_t lastIndex = yValues.size () - 1;
        if (noisy)
            {
            GEOMAPI_PRINTF ("Bspline1d order %d (poles %d) (knot step %g)\n", order, (int)yValues.size (), du);
            GEOMAPI_PRINTF ("(t_dt %16.5f %18.8f\n", yValues[0], (yValues[1] - yValues[0]) / du);
            GEOMAPI_PRINTF ("(t_dt %16.5f %18.8f\n", yValues[lastIndex], (yValues[lastIndex] - yValues[lastIndex - 1]) / du);
            }
        Bspline1d spline;
        spline.Populate (&yValues[0], yValues.size (), order);
        MSBsplineCurvePtr xyzSpline = MSBsplineCurve::CreateFromPolesAndOrder (xyz, NULL, NULL, order, false, true);
        double df = 1.0 / 32.0;
        bvector<double> yValues1, dY1;
        for (double f = 0.0; f <= 1.0; f += df)
            {
            double t, dt;
            DPoint3d xyz1;
            DVec3d dxyz1;
            spline.FractionToValue (t, dt, f);
            xyzSpline->FractionToPoint (xyz1, dxyz1, f);

            if (noisy)
                GEOMAPI_PRINTF ("(f_t_dt %16.8f (1d %16.8f %16.8f)  (3d %16.8f %16.8f)\n", f, t, dt, xyz1.x, dxyz1.x);
            yValues1.push_back (t);
            Check::Near (t, xyz1.x);
            dY1.push_back (dt);
            Check::Near (dt, dxyz1.x);
            }
        }
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline1d, Raman2)
    {
    bvector<double> time
        {
        0.000,
        2.000,
        4.000,
        6.000,
        8.000
        };
    exerciseBspline1d (time);
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline1d, Raman1)
    {
    bvector<double> time
        {
        0.000,
        0.077,
        0.154,
        0.231,
        0.308,
        0.385,
        0.462,
        0.538,
        0.615,
        0.692,
        0.769,
        0.846,
        0.923,
        1.000
        };
    exerciseBspline1d (time);
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline1d, ProfileCurve0)
  {

  bvector<double> altitude
    {
    1,
    1,  // intermediate for linear section
    1,
    1,  // intermediate point of parabolic section
    2
    };
  bvector<double> knots
    {
    0,        // open clamp
    0,0,      // triple knot to clamp
    3,3,    // double to enforce passthrough at start of parabola
    5,5,    //  end of parabola
    5        // close clamp
    };
  Bspline1d profile;
  profile.Populate(altitude, knots);
  for (double d = 0.0; d <= 5.0; d += 0.25)
    {
    Check::PrintIndent (2);
    Check::Print (DPoint2d::From (d, profile.KnotToValue (d)), "xy");
    }
  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline1d, ProfileCurve1)
  {
  // y=1+x through first section, length 4
  // then transition to parabola 
  bvector<double> altitude
    {
    1,
    3,  // intermediate for linear section
    5,
    6,  // intermediate point of parabolic section
    8
    };

  bvector<double> distance
    {
    0,
    2,
    4,
    5,
    6
    };


  bvector<double> knots
    {
    0,        // open clamp
    0, 0,      // triple knot to clamp
    4, 4,    // double to enforce passthrough at start of parabola
    6, 6,    //  end of parabola
    6        // close clamp
    };
  Bspline1d profileCurve;
  profileCurve.Populate(altitude, knots);
  Bspline1d distanceCurve;
  distanceCurve.Populate(distance, knots);
  for (double d = 0.0; d <= 6.0; d += 0.25)
    {
    double z = profileCurve.KnotToValue(d);
    Check::PrintIndent (2);
    Check::Print (d, "x");
    Check::Print (z, "z");
    Check::Print (z-(1.0+d), "z-(1+d)");
    Check::Print (distanceCurve.KnotToValue (d));
    }
  }

#ifdef NOISY_CRITICAL_KNOTS
bool FindMaximalBasisFunctionKnots (bvector<double> const&knots, size_t order, bvector<double> &criticalKnots)
    {
    GEOMAPI_PRINTF ("Greville Knot Basis Evaluations (order %d)\n", (int)order);
    double      b[MAX_BSORDER], dB[MAX_BSORDER], ddB[MAX_BSORDER], maxKnot;
    static int s_maxNewton = 6;
    static double s_tol = 1.0e-12;
    size_t degree = (size_t) (order - 1);
    int numKnots = (int)knots.size ();
    maxKnot = knots[numKnots - order];
    size_t numPoles = numKnots - order;
    size_t knot0 = 1;   // leftmost knot of first Greville average.
    for (size_t basisFunctionIndex = 0; basisFunctionIndex < numPoles; basisFunctionIndex++)
        {
        double s = 0.0;
        for (size_t k = 0; k < degree; k++)
            s += knots[knot0 + basisFunctionIndex + k];
        double knotValue = s / (double)degree;
        int left;
        bsputil_knotToBlendingFuncs (b, dB, &left, &knots[0], knotValue, maxKnot, (int)order, 0);
        GEOMAPI_PRINTF (" (basisIndex %d knot %.17g leftBasis %d)\n", (int)basisFunctionIndex, knotValue, left);
        for (size_t i = 0; i < order; i++)
            {
            GEOMAPI_PRINTF ("   (%d %.16g %.17g)\n", (int)i, b[i], dB[i]);
            }
        size_t iMax = 0;
        for (size_t i = 0; i < order; i++)
            if (b[i] > b[iMax])
                iMax = i;
        if (basisFunctionIndex > 0 && basisFunctionIndex + 1 < numPoles)
            {
            double x = knotValue;
            // run some newton steps looking for max ...
            for (int newtonStep = 0; newtonStep < s_maxNewton; newtonStep++)
                {
                bsputil_blendingsForSecondPars (b, dB, ddB, &left, &knots[0], x, maxKnot, (int)order, 0);
                auto dx = DoubleOps::ValidatedDivideParameter (dB[iMax], ddB[iMax], 0.0);
                x -= dx;
                GEOMAPI_PRINTF("     (iMax %d (b %.17g %.17g) %.17g %.3le)\n", iMax, b[iMax], dB[iMax], x, dx.Value ());
                if (fabs (dx) < s_tol)
                    break;
                }
            }
        } 
    }
#else
void FindMaximalBasisFunctionKnots (bvector<double> const &knots, size_t order, bvector<double> &criticalKnots)
    {
    criticalKnots.clear ();
    double      b[MAX_BSORDER], dB[MAX_BSORDER], ddB[MAX_BSORDER], maxKnot;
    static int s_maxNewton = 6;
    static double s_tol = 1.0e-12;
    size_t degree = (size_t) (order - 1);
    int numKnots = (int)knots.size ();
    maxKnot = knots[numKnots - order];
    size_t numPoles = numKnots - order;
    size_t knot0 = 1;   // leftmost knot of first Greville average.
    for (size_t basisFunctionIndex = 0; basisFunctionIndex < numPoles; basisFunctionIndex++)
        {
        double s = 0.0;
        for (size_t k = 0; k < degree; k++)
            s += knots[knot0 + basisFunctionIndex + k];
        double knotValue = s / (double)degree;
        int left0;
        bsputil_knotToBlendingFuncs (b, dB, &left0, &knots[0], knotValue, maxKnot, (int)order, 0);
        size_t iMax = 0;
        for (size_t i = 0; i < order; i++)
            if (b[i] > b[iMax])
                iMax = i;
        if (basisFunctionIndex > 0 && basisFunctionIndex + 1 < numPoles)
            {
            double x = knotValue;
            // run some newton steps looking for max ...
            for (int newtonStep = 0; newtonStep < s_maxNewton; newtonStep++)
                {
                int left;
                bsputil_blendingsForSecondPars (b, dB, ddB, &left, &knots[0], x, maxKnot, (int)order, 0);
                int kk = (int)iMax + left0 - left;
                auto dx = DoubleOps::ValidatedDivideParameter (dB[kk], ddB[kk], 0.0);
                x -= dx;
                if (fabs (dx) < s_tol)
                    break;
                }
            knotValue = x;
            }
        criticalKnots.push_back (knotValue);
        } 
    }
#endif
static size_t s_dump = 1;
bool ConvertToClampedBspline (
bvector<double> const & knots,  //!< @param [in] knots clamped knots.
size_t order,                   //!< @param [in] order bspline order.
bvector<DPoint3d>const &xyz,    //!< @param [in] xyz interpolation points
bvector<double> &criticalKnots, //!< @param [out] critical knot values
bvector<DPoint3d> &poleXYZ      //!< @param [out] computed poles
)
    {
    bool ok = true;
    ok &= Check::Size (xyz.size () + order, knots.size (), "Confirm excess knots over poles");
    size_t lastKnotIndex = knots.size () - 1;
    for (size_t i = 1; i < order; i++)
        {
        ok &= Check::Near (knots[0], knots[i], "confirm left clamp");
        ok &= Check::Near (knots.back (), knots[lastKnotIndex - i], "confirm right clamp");
        }

    FindMaximalBasisFunctionKnots (knots, order, criticalKnots);
    if (s_dump > 2)
        {
        GEOMAPI_PRINTF("(CriticalKnots\n");
        for (size_t i = 0; i < criticalKnots.size (); i++)
            {
            GEOMAPI_PRINTF ("   %.17g", criticalKnots[i]);
            if (((i+1) % 6) == 0)
                GEOMAPI_PRINTF ("\n");
            }
        GEOMAPI_PRINTF("\n");
        }
    double      b[MAX_BSORDER], db[MAX_BSORDER];
    int leftKnotIndex;
    double maxKnot = knots[lastKnotIndex - order + 1];
    size_t numPoles = xyz.size ();
    // Matrix A has bandwidth {order-1}  !!!
    RowMajorMatrix A (numPoles, numPoles);
    RowMajorMatrix B (numPoles, 3);

    // RowMajorBandedMatrix matrix (poles.size (), (size_t)order - 1);
    for (size_t i = 0; i < criticalKnots.size (); i++)
        {
        double knotValue = criticalKnots[i];
        bsputil_blendingsForSecondPars (b, nullptr, nullptr, &leftKnotIndex, &knots[0], knotValue, maxKnot, (int)order, 0);
        int leftPoleIndex = leftKnotIndex - (int)order;    // shift from knot index to pole index
        B.At (i, 0) = xyz[i].x;
        B.At (i, 1) = xyz[i].y;
        B.At (i, 2) = xyz[i].z;
        for (size_t k = 0; k < order; k++)
            A.At (i, leftPoleIndex + k) = b[k];
        }

    if (s_dump > 100)
        {
        GEOMAPI_PRINTF ("MaxBasisValue Interpolation Matrix\n");
        RowMajorMatrix::Print (A);
        }
#ifdef abc
    if (!LinearAlgebra::SolveInplaceGaussPartialPivot (A, B))
        return false;
#else
    double condition;
    if (!LinearAlgebra::SolveInplaceGaussFullPivot (A, B, condition))
        return false;
    Check::PrintHeading ("ConvertToClamped");
    Check::Print ((int)numPoles, "NumPoles");
    Check::Print ((int)order, "order");
    Check::Print (condition, "condition");
#endif
    poleXYZ.clear ();
    for (size_t i = 0; i < numPoles; i++)        
        poleXYZ.push_back (DPoint3d::From (B.At (i,0), B.At (i,1), B.At (i,2)));

    // RowMajorBandedMatrix matrix (poles.size (), (size_t)order - 1);
    for (size_t i = 0; i < criticalKnots.size (); i++)
        {
        double knotValue = criticalKnots[i];
        bsputil_blendingsForSecondPars (b, db, nullptr, &leftKnotIndex, &knots[0], knotValue, maxKnot, (int)order, 0);
        int leftPoleIndex = leftKnotIndex - (int)order;    // shift from knot index to pole index
        DPoint3d xyzB = DPoint3d::FromZero ();
        DVec3d  dxyzB;
        dxyzB.Zero ();
        for (size_t k = 0; k < order; k++)
            {
            DVec3d xyzAsVector = DVec3d::From (poleXYZ[leftPoleIndex+k]);
            xyzB.x += b[k] * xyzAsVector.x;
            xyzB.y += b[k] * xyzAsVector.y;
            xyzB.z += b[k] * xyzAsVector.z;


            dxyzB.x += db[k] * xyzAsVector.x;
            dxyzB.y += db[k] * xyzAsVector.y;
            dxyzB.z += db[k] * xyzAsVector.z;
            }
        Check::Near (xyz[i], xyzB, "Confirm interpolation");
        if (s_dump > 2)
            GEOMAPI_PRINTF (" (xyz %g, %g, %g) (xyzB %g %g %g) (dxyzB %g,%g,%g)\n",
                        xyz[i].x, xyz[i].y, xyz[i].z,
                        xyzB.x, xyzB.y, xyzB.z,
                        dxyzB.x, dxyzB.y, dxyzB.z
                        );
        }

    return true;
    }

void TestMaximalBasisFunctions (bvector<double> const &knots, size_t order, bvector<double> &criticalKnots)
    {
    bvector<DPoint3d> xyz, xyzPoles;
    for (size_t i = 0; i + order < knots.size (); i++)
        {
        double a = (double)i;
        xyz.push_back (DPoint3d::From (a, a * a, a * a * a));
        }
    ConvertToClampedBspline (knots, order, xyz, criticalKnots, xyzPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve, CriticalKnotsHighOrder)
    {

    bvector<double> criticalKnots;

    for (size_t order = 3; order < 14; order++)  // EDL -- bad things happen at order 8.   Bad matrix condition.
        {
        bvector<DPoint3d> xyz;
        bvector<DPoint3d> xyzPoles;
        bvector<double> knots;
        for (size_t i = 0; i < order; i++)
            knots.push_back (0.0);
        for (size_t i = 0; i < order; i++)
            knots.push_back (1.0);
        for (size_t i = 0; i < order; i++)
            {
            //double a = (double)i/ (double)(order - 1);
            double a = (double)i;
            xyz.push_back (DPoint3d::From (a, a*a, pow (a, (int)(order - 1))));
            }
        /*bool stat =*/ ConvertToClampedBspline (knots, order, xyz, criticalKnots, xyzPoles);
        bvector<DPoint3d> xyzCritical, xyzCriticalPoles;
        xyz.clear ();
        for (size_t i = 0; i < order; i++)
            {
            double a = criticalKnots[i];
            xyz.push_back (DPoint3d::From (a, a*a, pow (a, (int)(order - 1))));
            }
        /*bool stat1 =*/ ConvertToClampedBspline (knots, order, xyz, criticalKnots, xyzCriticalPoles);
        }

    bvector<double> knotQuadratic {0,0,0,  1,2,3,4,5, 6,6,6};
    TestMaximalBasisFunctions (knotQuadratic, 3, criticalKnots);


    bvector<double> knotCubic {0,0,0,0,  1,2,3,5,5.5, 7,7,7,7};
    TestMaximalBasisFunctions (knotCubic, 4, criticalKnots);


    bvector<double> knotQuartic {0, 0,0,0,0,  1,2,3,5,5.5, 7,7,7,7, 7};
    TestMaximalBasisFunctions (knotQuartic, 5, criticalKnots);

    bvector<double> knotCubicU {0,0,0,0,   1,2,3,4,5,6,7,  8,8,8,8};
    TestMaximalBasisFunctions (knotCubicU, 4, criticalKnots);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,CriticalKnotsCubicManyPoles)
    {
    bvector<double> criticalKnots;
    bvector<size_t> interiorCounts {0,1,2,4,6,10,20,60};
    for (size_t numInteriorKnots : interiorCounts)
        {
        size_t order = 4;
        bvector<DPoint3d> xyz;
        bvector<DPoint3d> xyzPoles;
        bvector<double> knots;
        double a = 0.0;
        for (size_t i = 0; i < order; i++)
            knots.push_back (0.0);
        for (size_t i = 0; i < numInteriorKnots; i++)
            {
            a++;
            knots.push_back (a);
            }
        a++;
        for (size_t i = 0; i < order; i++)
            knots.push_back (a);
        size_t numPoles = knots.size () - order;
        for (size_t i = 0; i < numPoles; i++)
            {
            double f = (double)i;
            xyz.push_back (DPoint3d::From (f, f*f, f * f * f));
            }
        /*bool stat =*/ ConvertToClampedBspline (knots, order, xyz, criticalKnots, xyzPoles);
        }


    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,KnotSpread)
    {

    for (size_t order = 3; order < 5; order++)
        {
        GEOMAPI_PRINTF("\nKnotSpread Tests for order %d\n", (int)order);
        for (size_t numInteriorKnots = 0; numInteriorKnots < order + 2; numInteriorKnots++)
            {
            bvector<double> criticalKnots;
            bvector<DPoint3d> xyz;
            bvector<DPoint3d> xyzPoles;
            bvector<double> knots;
            double a = 0.0;
            for (size_t i = 0; i < order; i++)
                knots.push_back (0.0);
            for (size_t i = 0; i < numInteriorKnots; i++)
                {
                a++;
                knots.push_back (a);
                }
            a++;
            for (size_t i = 0; i < order; i++)
                knots.push_back (a);
            size_t numPoles = knots.size () - order;
            for (size_t i = 0; i < numPoles; i++)
                {
                double f = (double)i;
                xyz.push_back (DPoint3d::From (f, f*f, f * f * f));
                }
            /*bool stat =*/ ConvertToClampedBspline (knots, order, xyz, criticalKnots, xyzPoles);
            Check::Print (knots, "Knots");
            Check::Print (criticalKnots, "CriticalKnots");
            }
        }
    }

MSBsplineCurvePtr ConvertToClampedBspline
(
size_t order,                   //!< @param [in] order bspline order.
bvector<DPoint3d>const &xyz    //!< @param [in] xyz interpolation points
)
    {
    size_t numPoints = xyz.size ();
    if (numPoints < order)
        return nullptr;
    if (order < 2)
        return nullptr;

    bvector<double> knots;
    double df = 1.0 / (double)(numPoints - 1);
    for (size_t i = 0; i < order; i++)
        knots.push_back (0.0);
    for (size_t i = 0; i + 1 < numPoints; i++)
        knots.push_back (i * df);
    for (size_t i = 0; i < order; i++)
        knots.push_back (1.0);
    bvector<DPoint3d> poles;
    bvector<double> criticalKnots;
    ConvertToClampedBspline (knots, order, xyz, criticalKnots, poles);
    return MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, &knots,(int)order, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,KnotSpread1)
    {
    bvector<double>criticalSpreadKnots;
    double a = 2.0/3.0;
    bvector<double> spreadKnots {0,0,0,0, 3 * a, 5 * a, 6 * a};
    for (size_t i = 0; i < 4; i++)
        spreadKnots.push_back (spreadKnots.back () + 1.0);
    double b = spreadKnots.back ();
    spreadKnots.push_back (b + a);
    spreadKnots.push_back (b + 3*a);
    spreadKnots.push_back (b + 6 * a);
    double c = spreadKnots.back ();
    spreadKnots.push_back (c);
    spreadKnots.push_back (c);
    spreadKnots.push_back (c);

    FindMaximalBasisFunctionKnots (spreadKnots, 4, criticalSpreadKnots);
    Check::Print (spreadKnots, "Knots");
    Check::Print (criticalSpreadKnots, "CriticalKnots");
    }

struct BsplineBasisFunctions
{
bvector<double> const &m_knots;

BsplineBasisFunctions (bvector<double> const &knots) : m_knots(knots) {}

double Evaluate (double x, size_t i, size_t windowWidth, size_t activeIntervalIndex)
    {
    double knot0 = m_knots[i];
    double knotK = m_knots[i+windowWidth];
    if (windowWidth == 1)
        {
        if (i == activeIntervalIndex)
            return 1.0;
        else
            return 0.0;
        }
    double d0 = m_knots[i+windowWidth-1] - knot0;
    double d1 = knotK - m_knots[i+1];
    double f = 0.0;
    if (d0 > 0.0)
        {
        double b0 = Evaluate (x, i, windowWidth-1, activeIntervalIndex);
        f += b0 * (x - knot0) / d0;
        }
    if (d1 > 0.0)
        {
        double b1 = Evaluate (x, i+1, windowWidth-1, activeIntervalIndex);
        f += b1 * (knotK - x) / d1;
        }
    return f;
    }
double GrevilleKnot (size_t i, size_t k)
    {
    double s = 0.0;
    for (size_t kk = k + 1; kk < i + k; kk++)
        s += m_knots[kk];
    return s / (double)(k - 1);
    }    
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline1d,BasisFunctions)
{
static int s_noisy = 0;
size_t numInterval = 3;
// double dxEval = 0.125; **** UNUSED VARIABLE ****
size_t numEvalPerInterval = 4;
bvector<double> clampStepVector {0.0};
for (double clampStep : clampStepVector)
    {
    for (size_t order = 2; order < 5; order++)
        {
        bvector<double> knots;
        size_t numClamp = order;
        double da0 = clampStep;
        double da  = 1.0;
        double da1 = clampStep;

        double a = - ((double)order - 1.0) * da0;
        for (size_t i = 0; i + 1 < numClamp; i++, a+= da0)
            knots.push_back (a);
        for (size_t i = 0; i < numInterval; i++, a += da)
            knots.push_back (a);
        knots.push_back (a + 0.25 * da); // nonuniform knots here !!
        a += da;
        for (size_t i = 0; i < numInterval; i++, a += da)
            knots.push_back (a);

        knots.push_back (a);
        for (size_t i = 0; i + 1 < numClamp; i++)
            {
            a += da1;
            knots.push_back (a);
            }
        Check::Print (knots, "knots");
        BsplineBasisFunctions B(knots);
        // Evaluate each basis function horizontally over all of its intervals ...
        for (size_t iLeft = 0; iLeft + order < knots.size (); iLeft++)
            {
            bvector<DPoint2d> xy;
            DRange2d range;
            range.Init ();
            for (size_t interval = 0; interval < order; interval++)
                {
                double k0 = knots[iLeft + interval];
                double k1 = knots[iLeft + interval + 1];
                if (k0 < k1)
                    {
                    for (size_t i = 1; i <= numEvalPerInterval; i++)
                        {
                        double f = (double)i / (double)numEvalPerInterval;
                        double x = DoubleOps::Interpolate (k0, f, k1);
                        double y = B.Evaluate (x, iLeft, order, iLeft + interval);
                        xy.push_back (DPoint2d::From (x, y));
                        range.Extend (x,y);
                        }
                    }
                }

            Check::True (range.low.y >= 0.0 && range.high.y <= 1.0);
            int numReversal = 0;
            for (size_t i = 0; i + 2 < xy.size (); i++)
                {
                double dy01 = xy[i+1].y - xy[i].y;
                double dy12 = xy[i+2].y - xy[i+1].y;
                if (dy01 * dy12 <= 0.0)
                    numReversal++;
                }
#define CheckBasisFuncs
#ifdef CheckBasisFuncs
            if (!Check::True (numReversal <= 1, "Composite basis function has single maximum")
                || s_noisy > 0)
                {
                GEOMAPI_PRINTF ("\n order %d BasisFunction %d\n", (int)order, (int)iLeft);
                Check::Print (xy, "   xy");
                }
#endif

            }
        bvector<double> basisFunctions;
        // Evaluate all basis functions vertically in each single interval.
        for (size_t intervalIndex = order - 1; intervalIndex + order < knots.size (); intervalIndex++)
            {
            double k0 = knots[intervalIndex];
            double k1 = knots[intervalIndex+1];
            if (k0 < k1)
                {
                for (size_t i = 0; i <= numEvalPerInterval; i++)
                    {
                    double f = (double)i / (double)numEvalPerInterval;
                    double x = DoubleOps::Interpolate (k0, f, k1);
                    double sum = 0.0;
                    basisFunctions.clear ();
                    for (size_t kB = 0; kB < order; kB++)
                        {
                        double y = B.Evaluate (x, intervalIndex - kB, order, intervalIndex);
                        basisFunctions.push_back (y);
                        sum += y;
                        Check::True (y >= 0.0 && y <= 1.0);
                        }
                    Check::Near (1.0, sum);
                    }
                }
            }



        }
    }
}

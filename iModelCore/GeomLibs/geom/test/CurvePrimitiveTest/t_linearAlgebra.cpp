#include "testHarness.h"
#include "Bentley/BeNumerical.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (LinearAlgebra, IdentitySolve)
    {
    for (size_t i = 1; i < 5; i++)
        {
        RowMajorMatrix A (i, i);
        RowMajorMatrix RHS (i, 1);
        A.SetIdentity ();
        bvector<double> x0 (i);
        DoubleOps::SetSequential (x0, 3.0, 7.0);
        bvector<double> x1, b0;
        A.Multiply (x0, b0);
        RHS.SetColumn (0, b0);
        if (Check::True (LinearAlgebra::SolveInplaceGaussPartialPivot (A, RHS), "Gauss solver completes"))
            {
            RHS.GetColumn (0, x1);
            double d = DoubleOps::MaxAbsDiff (x0, x1);
            Check::Near (0.0, d, "I*X = B simple solution");
            }
        }
    }



void Print (RowMajorMatrix &A)
    {
    size_t period = 4;
    for (size_t i = 0; i < A.NumRows (); i++)
        {
        printf (" %2d:", (int)i);
        for (size_t j = 0; j < A.NumColumns (); j++)
            {
            printf (" %g", A.At (i,j));
            if ((j % period) + 1 == period
                && j + 1 != A.NumColumns ()
                )
                printf ("\n   ");
            }
        printf ("\n");
        }
    }

// Make an identity matrix of size n.
// Add a * sin(q0(i) * sin(q1(j)) to each entry, where q0(i) and q1(j) are sines of various angles controlled by q0 and q1.
// Make a solution vector X0 with known values (3, 10, 17 ...)
// From B=A*X0
// Solve A*X1 = B for an X1 (which ideally exactly matches X0).  Return the max difference between X0 an X1
double TestSineShifted (size_t n, double a, double q0, double dq0, double q1, double dq1, bool printIt, int method, double &aMax)
    {
    RowMajorMatrix A (n, n);
    RowMajorMatrix RHS (n, 2);
    bvector<double> shift0 (n), shift1 (n);
    bvector<double> b0, b1, x0 (n), x1, b1Q;
    DoubleOps::SetSequential (shift0, q0, dq0);
    DoubleOps::ApplyFunction (shift0, sin, shift0);
    DoubleOps::SetSequential (shift1, q1, dq1);
    DoubleOps::ApplyFunction (shift1, sin, shift1);

    A.SetIdentity ();
    A.AddScaledOuterProductInPlace (a, shift0, shift1);
    aMax = DoubleOps::MaxAbs (A);
    if (printIt)
        Print (A);

    DoubleOps::SetSequential (x0, 3.0, 7.0);
    A.Multiply (x0, b0);
    DoubleOps::ApplyFunction (b0, sin, b1);
    RHS.SetColumn (0, b0);
    RHS.SetColumn (1, b1);
    double condition;
    double d = DBL_MAX;
    double d1 = DBL_MAX;
    bool stat = false;
    RowMajorMatrix A1 = A;
    if (method == 0)
        {
        stat = Check::True (LinearAlgebra::SolveInplaceGaussPartialPivot (A, RHS), "Gauss solver completes");
        }
    else if (method == 1 || method == 2)
        {
        stat = Check::True (LinearAlgebra::SolveInplaceGaussFullPivot (A, RHS, condition), "Invert full swap");
        }

    if (stat)
        {
        RHS.GetColumn (0, b0);
        RHS.GetColumn (1, x1);
        A1.Multiply (x1, b1Q);
        d1 = DoubleOps::MaxAbsDiff (b1, b1Q);   // This error is on the b side.
        d = DoubleOps::MaxAbsDiff (x0, b0);
        }
    
    d /= DoubleOps::MaxAbs (x0);
    return d;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (LinearAlgebra, ShiftedSolve)
    {
    size_t nMax = 100;
    size_t nFactor = 1;
    double factorA = 4.5;
    static bool s_doPrint = false;
    for (double a = 0.1; a < 200.0; a *= factorA)
        {
        if (s_doPrint)
            printf (" **** SineShifted Gauss solutions (a %g)\n", a);
        for (size_t i = 1; i < nMax; i += (i < 5 ? 1 : nFactor * i))
            {
            double q0 = 0.01;
            double dq0 = 0.012;   // shift0 is all small positive sines
            double q1 = 1.58;     // shift1 jumps around and changes sign.
            double dq1 = 2.0;
            Check::StartScope ("Cosine Shifted ", i);
            double aPartial, aFull;
            double ePartial = TestSineShifted (i, a, q0, dq0, q1, dq1, s_doPrint, 0, aPartial);
            double eFull    = TestSineShifted (i, a, q0, dq0, q1, dq1, s_doPrint, 2, aFull);
            double mu;
            DoubleOps::SafeDivide (mu, eFull, ePartial, 10000.0);
            if (s_doPrint)
                printf ("  (n %d) (eMaxPartial %#.2g)  (eMaxFull %#.2g) (mu %g)\n", (int)i, ePartial, eFull, mu);
            // experience says the errors get big after 200 .....
            if (i < 200)
                {
                Check::Near (1.0, 1.0 + ePartial, "PartialPivot error in X");
                Check::Near (1.0, 1.0 + eFull, "FullPivot error in X");
                }
            Check::EndScope ();
            }
          }
    }
double ShowNeighbors (CharCP name, double x, bool doPrint)
    {
    double y = DBL_MAX;
    double z = BeNumerical::BeNextafter(x,y);
    double e = z-x;
    if (doPrint)
        printf ("%s   (x %#.17g)   (after(x) %#.17g)  (epsAfter %g)\n", name,
            x, z, e);
    return e;
    }

// caller precomputes c = b-a (hence forcing it to be stored, which might not happen if done within this function?)
// interpolate with both formulas.  Print difference between interpolees, and machine precision of c.
double CheckInterpolation (double a, double b, double c, double f, bool adaptiveFormula, bool printIt)
    {
    double c1 = a + f * c;
    if (adaptiveFormula && f > 0.5)
        c1 = b + (f - 1.0) * c;
    double c2 = (1.0-f) * a + f * b;
    double e = fabs (c2 - c1);
    double u = fabs (BeNumerical::BeNextafter (c, DBL_MAX) - c);
    if (printIt)
        {
        printf (" (f %#.17g)  (c1 %#.17g) (c2 (%#.17g) (diff %#.2g) (u(c) %#.2g) (diff/u(c) %#.2g)",
              f, c1, c2, e, u, e / u);
        if (adaptiveFormula)
            printf (" ** ADAPTIVE");
        printf ("\n");
        }
    return e;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(LinearAlgebra,Epsilon0)
    {
    static bool s_doPrint = false;
    // fussy factoids to demonstrate:
    // The local granularity around (500) is 4 times the local granularity around (-85).
    //    (because the two powerOf2 transitions 128 and 256 between the two absolute values)
    // Their difference (585) is beyond the next powerOf2 beak at 512.
    double b = -85;
    double a = 500;
    b = a;
    double ea = ShowNeighbors ("a", a, s_doPrint);
    double eb = ShowNeighbors ("b", b, s_doPrint);
    double c = b - a;
    double ec = ShowNeighbors ("c=b-a", c, s_doPrint);
    ShowNeighbors ("a+c", a+c, s_doPrint);
    ShowNeighbors ("b-c", b-c, s_doPrint);
    if (s_doPrint)
        printf ("  (eb/ea %g) (ec/eb %g)\n", eb/ea, ec/eb);
    Check::Near (ea, 4.0 * eb, "85 vs 500 granularity");
    Check::Near (ec, 2.0 * eb, "granularity of 585 vs 500");
    static bool showAll = false;
    for (double factorA = 1.0; factorA < 1.1e12; factorA *= 10.0)
        {
        double aa = factorA * a;
        c = b - aa;
        if (s_doPrint)
            printf (" (a %#.17g)  (b %#.17g) (c %#.17g)\n", aa, b, c);
        // printf 2 shots for confirmation...
        CheckInterpolation (aa, b, c, 0.999999, false, showAll);
        CheckInterpolation (aa, b, c, 0.999999, true, showAll);
        for (int i = 0; i < 2; i++)
            {
            bool adaptive = i != 0;
            for (double f = 0.0; f <= 1.0; f += 0.125)
              CheckInterpolation (aa, b, c, f, adaptive, showAll);
            double fFactor = sqrt (sqrt (10.0));
            int numMatch = 0;
            int numTest = 0;
            for (double f = 0.5; f > 1.0e-15; f /= fFactor)
              {
              if (0.0 == CheckInterpolation (aa, b, c, (1.0 - f), adaptive, showAll))
                  numMatch++;
              if (0.0 == CheckInterpolation (aa, b, c, f, adaptive, showAll))
                  numMatch++;
              numTest += 2;
              }
          if (s_doPrint)
              printf ("   ** (numMatch %d) (numTest %d)\n", numMatch, numTest);
          }
      }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(LinearAlgebra, acos)
    {
    static bool s_doPrint = false;
    double arg = 1.0;
    for (int i = 0; i < 10; i++, arg = BeNumerical::BeNextafter (arg, 0.0))
        {
        if (s_doPrint)
            printf (" acos (%#.17g) = %#.17g       (1.-arg=%#.17g)\n", arg, acos (arg), 1.0 -arg);
        }

    double q = 1.0 - BeNumerical::BeNextafter (1.0, 0.0);
    for (int i = 0; i < 32 && q <= 2.0; i++, q *= 4)
        {
        double c = cos(q);
        double s = sin(q);
        double qc = acos (c);
        double qs = asin(s);
        double qt = atan2 (s, c);
        if (s_doPrint)
            printf (" (q %#.17g) (c=cos(q) %#.17g) (s=sin(q) %#.17g) (acos(c) %#.17g) (asin(s) %#.17g), (atan2(s,c) %#.17g)\n",
                  q, c, s, qc, qs, qt);
        }
    }

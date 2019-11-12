/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include <testHarness.h>
#include <Geom/XYRangeTree.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

static int s_noisy = 0;
void ApplyLinearFactor (bvector <double> &product, double root, double slope)
    {
    double a[2];
    a[0] = - root * slope;
    a[1] = (1.0 - root) * slope;
    int inputOrder = (int) product.size();
    if (inputOrder ==  0)
        {
        product.push_back (a[0]);
        product.push_back (a[1]);
        }
    else
        {
        double newProduct[MAX_BEZIER_ORDER];
        bsiBezier_univariateProduct (
                    newProduct, 0, 1,
                    product.data (), inputOrder, 0, 1,
                    a, 2, 0, 1);
        product.clear ();
        for (int i = 0; i < inputOrder + 1; i++)
            product.push_back (newProduct[i]);
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(UnivariateRoots, RegularRoots)
    {
    bvector <double> product;
    bvector <double> knownRoots;
    static double s_targetError = 1.0e-14;
    bvector<double>maxErrorVector;
    static int s_maxTestOrder = 8;    // above 10 tickles interesting problems.
    for (int order = 2; order <= s_maxTestOrder; order++)
        {
        //varunused double slope = 1.0;
        maxErrorVector.clear ();
        for (int numRoots = 0; numRoots < order; numRoots++)
            {
            double rootStep = 1.0 / order;
            int numExteriorRoots = order - 1 - numRoots;
            double firstRoot = rootStep / 2.0 - numExteriorRoots * rootStep;
            // root grid is firstRoot + i * rootStep
            product.clear ();
            knownRoots.clear ();
            for (int i = 0; i < order - 1; i++)
                {
                double root = firstRoot + i * rootStep;
                knownRoots.push_back (root);
                ApplyLinearFactor (product, root, 1.0 / (1.0 - root));
                }
            double roots[MAX_BEZIER_ORDER];
            int numRootsFound;
            bsiBezier_univariateRoots (roots, &numRootsFound, product.data (), order);
            if (order < 15) // NEEDS WORK -- debug high order !!
                Check::Int (numRoots, numRootsFound);
            double maxError = 0.0;
            for (size_t i = 0; i < (size_t)numRoots; i++)
                {
                double thisError = knownRoots[numExteriorRoots + i] - roots[i];
                maxError = DoubleOps::MaxAbs (maxError, thisError);
                }
            if (order < 10)
                Check::True (maxError < s_targetError);
            maxErrorVector.push_back (maxError);
            }
        if (Check::PrintDeepStructs ())
            {
            printf ("(order %d (maxErr", order);
            for (size_t i = 0; i < maxErrorVector.size (); i++)
                {
                double e = maxErrorVector[i];
                printf (" %5.1le", e);
                if ((i + 1) % 10 == 0)
                    printf ("\n     ");
                }
            printf (")\n");
            }
        }
    }
// Evaluate a hermite cubic at the midpoint of its interval . . 
double HermiteMidpoint
(
double f0,
double f1,
double df0, // derivative (wrt x) at start of interval
double df1, // derivative (wrt x) at end of interval
double h    // interval length (in x)
)
    {
    return 0.5 * (f0 + f1) + 0.125 * (df0 - df1) * h;
    }
double ratio (double a, double b)
    {
    if (b != 0.0)
        return a/b;
    return 0.0;
    }
double smallAngleVersine (double theta)
    {
    double s = sin (theta);
    double c = cos (theta);
    return s * s / (1.0 + c);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Trig,Hermite)
    {
    double theta0 = 0.0;
    double c0 = cos(theta0);
    double s0 = sin(theta0);
    double v0 = smallAngleVersine (theta0);
    bvector<double> eCos, eSin, eVer;
    for (double h = 0.1; h > 1e-5; h *= 0.5)
        {
        double theta1 = theta0 + h;
        double thetaA = 0.5 * (theta0 + theta1);
        double c1 = cos(theta1);
        double s1 = sin(theta1);
        double v1 = smallAngleVersine (theta1);
        double cA = cos(thetaA);
        double sA = sin(thetaA);
        double vA = smallAngleVersine (thetaA);
        double hcA = HermiteMidpoint (c0, c1, -s0, -s1, h);
        double ec = cA - hcA;
        double hsA = HermiteMidpoint (s0, s1, c0, c1, h);
        double es = sA - hsA;
        double hvA = HermiteMidpoint (v0, v1, s0, s1, h);
        double ev = vA - hvA;
        eCos.push_back (ec);
        eSin.push_back (es);
        eVer.push_back (ev);
        if (s_noisy)
            printf ("(h %g) (cosineError %g) (sineError %g) (versine error %g)\n", h, ec, es, ev);
        }

    bvector<double> qCos, qSin, qVer;
    size_t numCheck = 6;    // Check reduction ratios only while values are significantly nonzero
    DRange1d cosFactor (15.0, 17.0);    // We expect cosine convergence around 16
    DRange1d sinFactor (30.0, 34.0);    // We expect sine convergence around 17
    DRange1d verFactor (15.0, 17.0);
    for (size_t i = 1; i < eCos.size (); i++)
        {
        qCos.push_back (ratio(eCos[i-1], eCos[i]));
        qSin.push_back (ratio (eSin[i - 1], eSin[i]));
        qVer.push_back (ratio (eVer[i-1], eVer[i]));
        if (i <= numCheck)
            {
            Check::Contains (cosFactor, qCos.back ());
            Check::Contains (sinFactor, qSin.back ());
            Check::Contains (verFactor, qVer.back ());
            }
        }
    if (s_noisy)
        {
        Check::Print (qCos, "Cosine Reductions");
        Check::Print (qSin, "SineReductions");
        Check::Print (qVer, "VersineReductions");
        }
    }

#ifdef TestExperimentalConvexHullRootFinder
// string of points on convex hull.
// x is POINT INDEX -- integers 0,1
// y is coefficient
// z is slope
struct HullString
{
size_t n;
DPoint3d xy[MAX_BEZIER_ORDER];
HullString () {n = 0;}
void Clear () {n = 0;}

void Push (double x, double y, double z)
    {
    if (n < MAX_BEZIER_ORDER)
        {
        xy[n].x = x;
        xy[n].y = y;
        xy[n].z = z;
        n++;
        }
    }

double SlopeFromHullPoint (ptrdiff_t k, double x, double y)
    {
    return (y - xy[k].y) / (x - xy[k].x);
    }
double ExtrapolateFinalHullSlope (double x)
    {
    return xy[n-1].y + (x-xy[n-1].x) * xy[n-1].z;
    }
// k is clamped to [1,n-1]
double EvaluateHullEdge (size_t k, double x)
    {
    if (n == 0)
        return 0.0;
    else if (k == 0)
        return EvaluateHullEdge (1, x);
    else if (k >= n)
        return EvaluateHullEdge (n-1, x);
    else
        return xy[k].y + (x-xy[k].x) * xy[k].z;
    }
void Pop ()
    {
    if (n > 0)
        n--;
    }

void PushUpperHullPoint (double x, double y)
    {
    if (n == 0)
        {
        Push (x, y, 0.0);
        }
    else
        {
        // eliminate hull points covered by the new line ...
        while (n > 1)
            {
            double extrapolatedY = ExtrapolateFinalHullSlope (x);
            if (extrapolatedY <= y)
                {
                n--;
                }
            else
                {
                double newSlope = SlopeFromHullPoint (n-1, x, y);
                Push (x,y, newSlope);
                return;
                }
            }
        // Fall out with n == 1.
        Push (x,y, SlopeFromHullPoint (0, x, y));
        }
    }
void NegateYZ ()
    {
    for (size_t i = 0; i < n; i++)
        {
        xy[i].y = -xy[i].y;
        }
    }

bool ValidatePointBelow (double x, double y)
    {
    for (size_t i = 1; i < n; i++)
        {
        double yHull = EvaluateHullEdge (i, x);
        if (yHull < y)
            return false;
        }
    return true;
    }

bool ValidatePointAbove (double x, double y)
    {
    for (size_t i = 1; i < n; i++)
        {
        double yHull = EvaluateHullEdge (i, x);
        if (yHull > y)
            return false;
        }
    return true;
    }
void Print (char *name)
    {
    printf ("(%s ", name);
    for (size_t i = 0; i < n; i++)
        {
        printf ("(%lg %lg)", xy[i].x, xy[i].y);
        }
    printf (")\n");
    }

void UpdateCrossings (DRange1dR range)
    {
    for (int k = 1; k < (int)n; k++)
        {
        double y0 = xy[k-1].y;
        double y1 = xy[k].y;
        double p = y0 * y1;
        if (p > 0.0)
            {
            // no crossing here
            }
        else if (p < 0.0)
            {
            // simple crossing between
            double f = -y0 / (y1 - y0);
            range.Extend (xy[k-1].x + f * (xy[k].x - xy[k-1].x));
            }
        else if (y0 != 0.0) // y1 must be zero
            {
            range.Extend (xy[k].x);
            }
        else if (y1 != 0.0) // y0 must be zero
            {
            range.Extend (xy[k-1].x);
            }
        else
            {
            // both are zero
            range.Extend (xy[k].x);
            range.Extend (xy[k-1].x);
            }
        }
    }

};

struct BezHull
{
size_t order;
double coffs[MAX_BEZIER_ORDER];
size_t numRoots;
double roots[MAX_BEZIER_ORDER];
// Hull points have x coordinates as INTEGERS.
HullString upperHull;
HullString lowerHull;

BezHull ()
    {
    order = 0;
    }

BezHull (bvector<double> const &coffIn)
    {
    order = coffIn.size ();
    if (order > MAX_BEZIER_ORDER)
        order = MAX_BEZIER_ORDER;
    for (size_t i = 0; i < order; i++)
        coffs[i] = coffIn[i];
    }
void PrintCoffs (char * name)
    {
    printf ("(%s", name);
    for (size_t i = 0; i < order; i++)
        {
        printf (" %lg", coffs[i]);
        }
    printf (")\n");
    }

void SubdivideLeft (double u)
    {
    double v = 1.0 - u;
    for (size_t level = 1; level < order; level++)
        {
        for (size_t k = order-1; k >= level; k--)
            {
            coffs[k] = v * coffs[k-1] + u * coffs[k];
            }
        }
    }

double Evaluate (double u) const
    {
    double a[MAX_BEZIER_ORDER];
    if (order > 1)
        {
        double v = 1.0 - u;
        // First level gets its own loop to load up the a[] array.
        for (int i = 1; i < (int)order; i++)
            a[i-1] = v * coffs[i-1] + u * coffs[i];
        // remaining levels in place like SubdivideRight.
        for (int level = 2; level < (int)order; level++)
            {
            for (int i = 0; i < (int)order - level; i++)
                {
                a[i] = v * a[i] + u * a[i+1];
                }
            }
        return a[0];
        }
    else if (order == 1)
        {
        return coffs[0];
        }
    return 0.0;
    
    }
void SubdivideRight (double u)
    {
    double v = 1.0 - u;
    for (size_t level = 1; level < order; level++)
        {
        for (size_t k = 0; k < order - level; k++)
            {
            coffs[k] = v * coffs[k] + u * coffs[k + 1];
            }
        }
    }

void DeflateLeft ()
    {
    int rowA = (int)order - 1;
    int rowB = (int)order - 2;
    for (int i  = 0; i < (int)order - 1; i++)
        coffs[i] = coffs[i+1] * bsiBezier_getBinomialCoefficient (i+1, rowA) / bsiBezier_getBinomialCoefficient (i, rowB);
    order--;
    }
void DeflateRight ()
    {
    int rowA = (int)order - 1;
    int rowB = (int)order - 2;
    // coffs[0] gets multiplied and divided by 1, start at 1 in loop.
    for (int i  = 1; i < (int)order - 1; i++)
        coffs[i] *= bsiBezier_getBinomialCoefficient (i, rowA) / bsiBezier_getBinomialCoefficient (i, rowB);
    order--;
    }

// right is input and output upper part.
// left is output lower part
void SubdivideRight (double left[], double right[], size_t n, double u)
    {
    double v = 1.0 - u;
    for (size_t step = 1; step < n; step++)
        {
        left[step - 1] = right[0];
        for (size_t k = 0; k < n - step; k++)
            {
            right[k] = v * right[k] + u * right[k+1];
            }
        }
    left[n - 1] = right[0];
    }

bool FindFirstCrossing (double a[], size_t n, double u)
    {
    u = 0.0;
    if (a[0] == 0.0)
        return true;
    for (size_t i = 1; i < n; i++)
        {
        if (a[i-1] * a[i] <= 0.0)
            {
            double dy = a[i] - a[i-1];
            u = (i - 1 + (-a[i] / dy)) / (double)(n - 1);
            return true;
            }
        }
    return false;
    }
bool CautiousNewton (bool fromRight, double &u)
    {
    double a[MAX_BEZIER_ORDER];
    double b[MAX_BEZIER_ORDER];
    double u0 = 0.0;
    double u1 = 1.0;
    if (fromRight)
        {
        for (size_t i = 0; i < order; i++)
            b[i] = coffs[i];
        u0 = 1.0;
        u1 = 0.0;        
        }
     else
        {
        for (size_t i = 0; i < order; i++)
            b[i] = coffs[order - 1 - i];
        }


    // negate if needed to make consistent signs ....
    if (b[0] > 0.0)
        {
        for (size_t i = 0; i < order; i++)
            b[i] = -b[i];
        }

    // Do not attempt if no crossing in first polygon segment.
    static size_t s_maxSteps = 15;
    for (size_t steps = 0; steps < s_maxSteps; steps++)
        {
        double s;
        if (b[0] == 0.0)
            {
            u = fromRight ? 1.0 - u0 : u0;
            return true;
            }
        if (!FindFirstCrossing (b, order, s))
            return false;
        SubdivideRight (a, b, order, s);
                
        }
    return false;
    }
void Differentiate ()
    {
    if (order <= 1)
        {
        order = 1;
        coffs[0] = 0.0;
        }
    else
        {
        double factor = (double)(order - 1);
        for (size_t i = 1; i < order; i++)
            {
            coffs[i-1] = factor * (coffs[i] - coffs[i-1]);
            }
        order--;
        }
    }
void Clear ()
    {
    order = 0;
    lowerHull.Clear ();
    upperHull.Clear ();
    }

void Push (double a)
    {
    if (order < MAX_BEZIER_ORDER)
        coffs[order++] = a;
    }

void FormHull ()
    {
    upperHull.Clear ();
    lowerHull.Clear ();
    for (size_t i = 0; i < order; i++)
        {
        upperHull.PushUpperHullPoint ((double)i, coffs[i]);
        lowerHull.PushUpperHullPoint ((double)i, -coffs[i]);
        }
    lowerHull.NegateYZ ();
    }

DRange1d HullCrossings ()
    {
    DRange1d range = DRange1d ();
    upperHull.UpdateCrossings (range);
    lowerHull.UpdateCrossings (range);
    return range;
    }
bool ValidateHull ()
    {
    bool ok = true;
    for (size_t i = 0; i < order; i++)
        {
        ok |= upperHull.ValidatePointBelow ((double)i, coffs[i]);
        ok |= lowerHull.ValidatePointAbove ((double)i, coffs[i]); 
        }
    return ok;
    }
};
// input uMin = prior min crossing.
// Compute the crossing for new pole a.
// Ask seg for its min crorssing and verify expected behavior.
void AddAndCheck (BezHull &seg, double a, double &uMin)
    {
    seg.Push (a);
    seg.FormHull ();
    printf ("coffs: ");
    for (size_t i = 0; i < seg.order; i++)
        printf (" %lg", seg.coffs[i]);
    printf ("\n");
    seg.upperHull.Print ("Upper Hull");
    seg.lowerHull.Print ("Lower Hull");
    Check::True (seg.ValidateHull (), "Convex Hull Properties");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, Test0)
    {
    for (double s = 1.0; s > -2.0; s -= 2.0)
        {
        BezHull bezier;
        bezier.Push (s * -1.0);
        double uMin = DBL_MAX;
        AddAndCheck (bezier, s * 0.0, uMin);
        AddAndCheck (bezier, s * -0.5, uMin);
        AddAndCheck (bezier, s * 0.5, uMin);
        AddAndCheck (bezier, s * 10.0, uMin);
        AddAndCheck (bezier, s * 10.0, uMin);
        AddAndCheck (bezier, s * 1000.0, uMin);
        }
    }


void Solve (BezHull const &functionIn, bvector<double> &roots, char * name)
    {
    roots.clear ();
    double leftRoots[MAX_BEZIER_ORDER];
    double rightRoots[MAX_BEZIER_ORDER];
    int numLeftRoots = 0;
    int numRightRoots = 0;
    BezHull function = functionIn;  // This is updated in place
    function.PrintCoffs (name);
    if (function.order <= 1)
        return;
    double u0 = 0.0;
    double u1 = 1.0;
    for (size_t steps = 0; steps < 100; steps++)
        {
        if (function.coffs[0] == 0.0
            && function.coffs[function.order - 1] == 0.0)
            break;
        function.FormHull ();
        DRange1d interval = function.HullCrossings ();
        if (interval.IsNull ())
            {
            printf ("(no roots)\n");
            break;
            }
        double f0, f1;
        interval.GetLowHigh (f0, f1);
        double denominator = (double) (function.order - 1);
        f0 /= denominator;
        f1 /= denominator;
        double v0 = u0 + f0 * (u1 - u0); 
        double v1 = u0 + f1 * (u1 - u0);
        if (f1 <= f0 || v1 <= v0)
            {
            leftRoots[numLeftRoots++] = v0;
            printf ("Hull converged to point f(%22.16le) = %8.3le\n", v0, functionIn.Evaluate (v0));
            break;
            }
        else
            {
            double g = (f1 - f0) / (1.0 - f0);
            double pLeft = function.coffs[0];
            size_t last = function.order - 1;
            double pRight = function.coffs[last];
            function.SubdivideRight (f0);
            function.SubdivideLeft (g);
            if (pLeft *function.coffs[0] <= 0.0 || u0 == v0)
                {
                leftRoots[numLeftRoots++] = u0;
                printf ("Hull converged at left f(%22.16le) = %8.3le\n", u0, functionIn.Evaluate (u0));
                function.coffs[0] = 0.0;
                function.DeflateLeft ();
                steps = 0;
                }
            if (pRight * function.coffs[last] <= 0.0 || u1 == v1)
                {
                rightRoots[numRightRoots++] = u1;
                printf ("Hull converged at right f(%22.16le) = %8.3le\n", u1, functionIn.Evaluate (u1));
                function.coffs[last] = 0.0;
                function.DeflateRight ();
                steps = 0;
                }
            u0 = v0;
            u1 = v1;
            printf ("(%20.15le,%20.15le) ", u0, u1);
            function.PrintCoffs ("");
            }
        }
    for (int i = 0; i < numLeftRoots; i++)
        roots.push_back (leftRoots[i]);
    for (int i = numRightRoots - 1; i >= 0; i--)
        {
        roots.push_back (rightRoots[i]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, TestSymmetricCubic)
    {
    bvector<double> roots;
    BezHull function;
    function.Push (-1);
    function.Push (1);
    function.Push (1);
    function.Push (-1);
    Solve (function, roots, "cubic hump, symmetric coefficients (really parabola)");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, Test1)
    {
    BezHull function;
    bvector<double> roots;
    function.Push (-1);
    function.Push (1);
    function.Push (1);
    function.Push (-1);
    function.Push (-3);
    function.Push (6.0);
    Solve (function, roots, "Multiple Roots expected");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, Test2)
    {
    BezHull function;
    bvector<double> roots;
    function.Push (-1);
    function.Push (1);
    function.Push (2);
    function.Push (3);
    Solve (function, roots, "Single Root Expected");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, Test3)
    {
    BezHull function;
    bvector<double> roots;
    function.Push (-1);
    function.Push (100);
    function.Push (-0.1);
    function.Push (0.1);
    printf ("SUCCESSIVE DERIVATIVES\n");
    while (function.order > 1)
        {
        char title[1024];
        sprintf (title, "(order %d)\n", function.order);
        function.PrintCoffs (title);
        BezHull function1 = function;
        Solve (function1, roots, title);
        function.Differentiate ();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, Test4)
    {
    BezHull function;
    bvector<double> roots;
    function.Push (-1);
    function.Push (100);
    function.Push (-0.1);
    function.Push (0.1);
    function.Push (-10.0);
    printf ("SUCCESSIVE DERIVATIVES\n");
    while (function.order > 1)
        {
        char title[1024];
        sprintf (title, "(order %d)\n", function.order);
        function.PrintCoffs (title);
        BezHull function1 = function;
        Solve (function1, roots, title);
        function.Differentiate ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, Test5)
    {
    BezHull function;
    bvector<double> roots;
    function.Push (1);
    function.Push (10);
    function.Push (10);
    function.Push (3);
    printf ("SUCCESSIVE DERIVATIVES\n");
    while (function.order > 1)
        {
        char title[1024];
        sprintf (title, "(order %d)\n", function.order);
        BezHull function1 = function;
        Solve (function1, roots, title);
        function.Differentiate ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, Test6)
    {
    BezHull function;
    bvector<double> roots;
    function.Push (-2413.2);
    function.Push (1203.6);
    function.Push (-123.6);
    Solve (function, roots, "Quadratic with large coffs");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BezierHull, RegularRoots)
    {
    bvector <double> product;
    bvector <double> knownRoots;
    static double s_targetError = 1.0e-14;
    bvector<double>maxErrorVector;
    static int maxOrder = 10;
    for (int order = 2; order < maxOrder; order++)
        {
        //varunused double slope = 1.0;
        maxErrorVector.clear ();
        for (int numRoots = 0; numRoots < order; numRoots++)
            {
            double rootStep = 1.0 / order;
            int numExteriorRoots = order - 1 - numRoots;
            double firstRoot = rootStep / 2.0 - numExteriorRoots * rootStep;
            // root grid is firstRoot + i * rootStep
            product.clear ();
            knownRoots.clear ();
            for (int i = 0; i < order - 1; i++)
                {
                double root = firstRoot + i * rootStep;
                knownRoots.push_back (root);
                ApplyLinearFactor (product, root, 1.0 / (1.0 - root));
                }
            double roots[MAX_BEZIER_ORDER];
            int numRootsFound;
            bsiBezier_univariateRoots (roots, &numRootsFound, product.data (), order);
            BezHull function (product);
            bvector<double> hullRoots;
            Solve (product, hullRoots, "comparison");
            int numHullRoots = (int) hullRoots.size ();
            if (order < 10) // NEEDS WORK -- debug high order !!
                {
                Check::Int (numRoots, numRootsFound);
                Check::Int (numRoots, numHullRoots);
                }
            double maxError = 0.0;
            for (size_t i = 0; i < (size_t)numRoots; i++)
                {
                double thisError = knownRoots[numExteriorRoots + i] - roots[i];
                maxError = DoubleOps::MaxAbs (maxError, thisError);
                }
            if (order < 10)
                Check::True (maxError < s_targetError);
            maxErrorVector.push_back (maxError);
            }
        printf ("(order %d (maxErr", order);
        for (size_t i = 0; i < maxErrorVector.size (); i++)
            {
            double e = maxErrorVector[i];
            printf (" %5.1le", e);
            if ((i + 1) % 10 == 0)
                printf ("\n     ");
            }
        printf (")\n");
        }
    }
#endif
#define TestBezierIncrementalRootFinder
#ifdef TestBezierIncrementalRootFinder
struct NewtonStepEvaluator
{
double m_maxStep;
double m_maxTrapezoidExtrapolationFraction;
double m_maxQuadraticFraction;
NewtonStepEvaluator (double maxStep, double maxTrapezoidalExtrapolationFraction, double maxQuadraticFraction)
    : m_maxTrapezoidExtrapolationFraction (maxTrapezoidalExtrapolationFraction),
      m_maxStep (maxStep),
      m_maxQuadraticFraction (maxQuadraticFraction)
    {
    }

//!<ul>
//!<li>0 if no step is accepted
//!<li>1 if standard newton is accepted
//!<li>2 if extrapolated step is accepted
//!</ul>
int TrapezoidalStepFrom2Derivatives
(
double fA,      // bezier coff at current iterate.
double dfA,      // first derivative
double ddfA,      // second derivative
double &standardNewtonStep,
double &extrapolatedStep,
double &acceptedStep
)
    {
    acceptedStep = extrapolatedStep = standardNewtonStep = 0.0;
    if (DoubleOps::SafeDivide (standardNewtonStep, -fA, dfA, 0.0)
        && standardNewtonStep < m_maxStep)
        {
        acceptedStep = extrapolatedStep = standardNewtonStep;
        double dfB = dfA + standardNewtonStep * ddfA;   // approximate derivative at (u + standardNewtonStep)
        if (DoubleOps::SafeDivide (extrapolatedStep, -2.0 * fA, dfA + dfB, 0.0) // step using trapezoid rule (!!!) for evolving function
            && fabs (extrapolatedStep - standardNewtonStep ) <= m_maxTrapezoidExtrapolationFraction * fabs (standardNewtonStep))
            {
            acceptedStep = extrapolatedStep;
            return 2;
            }
        return 1;
        }
    return 0;
    }
int QuadraticStep
(
double fA,      // bezier coff at current iterate.
double dfA,      // first derivative
double ddfA,      // second derivative
double &standardNewtonStep,
double &quadraticNewtonStep,
double &acceptedStep
)
    {
    acceptedStep = quadraticNewtonStep = standardNewtonStep = 0.0;
    if (DoubleOps::SafeDivide (standardNewtonStep, -fA, dfA, 0.0)
        && standardNewtonStep < m_maxStep)
        {
        acceptedStep = quadraticNewtonStep = standardNewtonStep;
        Polynomial::Power::Degree2 quadratic (fA, dfA, 0.5 * ddfA);
        double roots[2];
        int numRoots = quadratic.RealRoots (roots);
        if (numRoots == 2)
            {
            quadraticNewtonStep = roots[0];
            if (fabs (roots[1]) <= fabs (quadraticNewtonStep))
                quadraticNewtonStep = roots[1];
            if (fabs (quadraticNewtonStep - standardNewtonStep) < m_maxQuadraticFraction * fabs (standardNewtonStep))
                {
                acceptedStep = quadraticNewtonStep;
                return 2;
                }
            }
        return 1;
        }
    return 0;
    }
int HouseholderStep
(
double fA,      // bezier coff at current iterate.
double dfA,      // first derivative
double ddfA,      // second derivative
double &standardNewtonStep,
double &householderStep,
double &acceptedStep
)
    {
    acceptedStep = householderStep = standardNewtonStep = 0.0;
    if (DoubleOps::SafeDivide (standardNewtonStep, -fA, dfA, 0.0)
        && standardNewtonStep < m_maxStep)
        {
        acceptedStep = householderStep = standardNewtonStep;
        double correctionRatio;
        if (DoubleOps::SafeDivide (correctionRatio, 0.5 * standardNewtonStep * ddfA, dfA, 0.0))
            {
            if (fabs (correctionRatio) < m_maxQuadraticFraction)
                {
                acceptedStep = householderStep = standardNewtonStep * (1.0 - correctionRatio);
                return 2;
                }
            }
        return 1;
        }
    return 0;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bezier,IncrementalRoots)
    {
    NewtonStepEvaluator newton (0.5, 0.1, 0.5);
    //auto coffs = bvector<double>  {0,1,2,4,5,8, 12};
    bvector<double> coffs;
    for (double radians = 0.0; radians < 3.0; radians += 0.45)
        coffs.push_back (cos (radians));        // put bezier points on cosine -- finite spacing so monotone downward from 0 to pi.
    double startDiffA = 0.15;
    double startDiffB = 0.02;
    int maxIteration = 10;
    int order = (int)coffs.size ();
    double abstol = 1.0e-15;
    // unused -- static int s_stepSelect = 0;

    for (double startDiff : bvector<double> {-startDiffA, -startDiffB, startDiffA, startDiffB})
        {
        Check::PrintIndent (0);
        Check::PrintIndent (0);
        Check::Print (startDiff, "newton iterations with starting error");
        bvector<bvector<int>> counters;
        for (double root = 0.1; root <= 0.91; root += 0.1)
            {
            Check::PrintIndent (0);
            counters.push_back (bvector<int> ());
            for (int methodSelect : bvector<int> {0, 1, 2, 3})
                {
                Check::PrintIndent (0);
                Check::Print (methodSelect, "************* newton variant");
                double f, df, ddf;
                bsiBezier_functionAndDerivativeExt (&f, &df, &ddf, &coffs[0], order, 1, root);
                double froot = f;
                Check::PrintHeading ("RootTest");
                Check::Print (root, "root");
                Check::Print (froot, "f(root)");
                double start = root + startDiff;
                double dx0, dx1, dx, x;
                x = start;
                bvector<double> errors;
                int numIterations = 1;
                for (; numIterations < maxIteration; numIterations++)
                    {
                    bsiBezier_functionAndDerivativeExt (&f, &df, &ddf, &coffs[0], order, 1, x);
                    double g = f - froot;
                    errors.push_back (x-root);
                    int stepCode = 0;
                    if (methodSelect == 2)
                        stepCode = newton.QuadraticStep (g, df, ddf, dx0, dx1, dx);
                    else if (methodSelect == 3)
                        stepCode = newton.HouseholderStep (g, df, ddf, dx0, dx1, dx);
                    else
                        stepCode = newton.TrapezoidalStepFrom2Derivatives (g, df, ddf, dx0, dx1, dx);
                    if (stepCode == 0)
                        {
                        Check::PrintIndent (3);
                        Check::Print (numIterations, "FAIL Iterations");
                        break;
                        }
                    else
                        {
                        Check::PrintIndent (0);
                        Check::Print (stepCode, "code");
                        Check::Print (x-root, "x-root");
                        Check::Print (g, "g");
                        //Check::Print (df, "df");
                        //Check::Print (ddf, "ddf");
                        //Check::Print (dx0, "dx0");
                        //Check::Print (dx1, "dx1");
                        if (fabs (dx0) > 1.0e-14)
                            Check::PrintE3 ((dx1 - dx0)/ dx0, "q/dx0");
                        {
                        double z0 = x + dx0;
                        double z1 = x + dx1;
                        bsiBezier_functionAndDerivativeExt (&f, &df, &ddf, &coffs[0], order, 1, z0);
                        double h0 = f - froot;  // function value at usual newton step
                        bsiBezier_functionAndDerivativeExt (&f, &df, &ddf, &coffs[0], order, 1, z1);
                        double h1 = f - froot;  // function value at corrected step
                        if (fabs (h0) > 1.0e-16)
                            Check::PrintE3 (h1/h0, "h1/h0");
                        }
                        x += methodSelect == 0 ? dx0 : dx;
                        if (fabs (dx) < abstol)
                            {
                            Check::PrintIndent (3);
                            Check::Print (numIterations, "ACCEPT iterations");
                            Check::PrintE3 (x - root, "error after final step");
                            break;
                            }
                        }
                    }
                counters.back ().push_back (numIterations);
                if (errors.size () >= 3)
                    {
                    for (size_t i = 2; i < errors.size (); i++)
                        {
                        double e0 = fabs (errors[i-2]);
                        double e1 = fabs (errors[i-1]);
                        double e2 = fabs (errors[i]);
                        if (e1 < e0 && e2 < e1 && 0.0 < e2)
                            {
                            Check::PrintIndent (2);
                            double q01 = e1 / e0;
                            double q12 = e2 / e1;
                            double alpha = log (q12) / log (q01);
                            Check::PrintE3(alpha, "alpha");   // order of convergence?
                            }
                        }
                    }
                }

            Check::PrintIndent (0);
            for (auto &c :counters)
                {
                Check::PrintIndent (2);
                for (auto &n : c)
                    {
                    Check::Print (n,"");
                    }
                }
            }
        }
    }
// CONCLUSIONS Nov 11 2016
// What is the method?
// 1) get 2 derivatives from function.
// 2) Naive (?) 2nd order Newton uses those to define parabola, take near parabola root as iterate.
// 2a) I don't want to do that messiness with picking roots.
// 3) from papers (http://ictp.acad.ro/jnaat/journal/article/view/1049/1094), in lieu of solving the
//     quadratic, somehow use first derivatives at two points to get better root.
//     (Call this the trapezoid formula)
// 4) (Earlin's variant) take the derivative of just the quadratic at the usual newton point.
// 5) Use that derivative (not a true derivative) in the trapezoid formula.
// 6) Works great!!! cubic convergence !!! No fussing with multiple roots of quadratic !!!
//
// Nov 14
// Add true quadratic case -- use f,df,ddf to define quadratic
// 1) Even faster -- "typical" iterations counts for (1st order newton, trapezoidal from 2nd derivative, true 2nd derivative) are (5,4,3) or (4,3,3)
// 2) This seems "predictable" -- the trapezoid uses 'some' of the 2nd derivative but not completely.
// 3) How does the trapezoidal logic avoid the quadratic term at all?
//
// Nov 23 2016
// Householder formula from http://www.sztaki.hu/~bozoki/oktatas/nemlinearis/SebahGourdon-Newton.pdf
//
#endif
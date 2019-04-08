//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL


template <typename T>
void CheckCurveAccess (MSBsplineCurveCP curve, bvector<DPoint3d> &pointA, bvector<double> &weightA)
    {
    T n = (T) pointA.size ();
    for (T i = 0; i < n; i++)
        {
        T j = n - 1 - i;
        Check::Near (pointA[i], curve->GetPole (i), "GetPole");
        Check::Near (weightA[i], curve->GetWeight (i), "GetWeight");
        Check::Near (pointA[i], curve->GetReversePole (j), "GetReversePole");
        Check::Near (weightA[i], curve->GetReverseWeight (j), "GetReverseWeight");
        
        DPoint3d pointB = DPoint3d::FromScale (pointA[i], 1.0 / weightA[i]);
        Check::Near (pointB, curve->GetUnWeightedPole (i), "GetUnweightedPole");        
        
        DPoint3d pointC = DPoint3d::FromScale (pointA[j], 1.0 / weightA[j]);
        Check::Near (pointC, curve->GetUnWeightedPole (j), "GetReverseUnweightedPole");
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,PoleAccess)
    {
    bvector<DPoint3d> pointA;
    bvector<double> weightA;
    for (double theta = 0.0; theta < 5.0; theta += 0.2349)
        {
        pointA.push_back (DPoint3d::From (cos(theta), sin(theta), theta));
        weightA.push_back (1.0 + 0.001 * theta);
        }
    size_t order = 3;
    bvector<double> knotA;
    for (size_t i = 0; i < pointA.size () + order; i++)
        knotA.push_back ((double)i);

    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (pointA, &weightA, &knotA, 3, false, true);
    
    CheckCurveAccess<size_t> (curve.get (), pointA, weightA);
    CheckCurveAccess<int> (curve.get (), pointA, weightA);


    for (size_t i = 0; i < knotA.size (); i++)
        Check::Near (knotA[i], curve->GetKnot(i), "GetKnot");

    bvector<DPoint3d> pointB;
    bvector<double> weightB;
    bvector<double> knotB;
    curve->GetPoles (pointB);
    curve->GetKnots (knotB);
    curve->GetWeights (weightB);
    Check::Near (pointA, pointB, "Bulk Query poles");
    Check::Near (knotA, knotB, "Bulk Query knots");
    Check::Near (weightA, weightB, "Bulk Query weights");
    }

TEST(BsplineSurface, PoleAccess)
    {
    int uOrder = 3;
    int vOrder = 4;
    size_t numI = 5;
    size_t numJ = 7;
    double weight = 1.1;
    // (chuckle) start at nonzero origin so weighted point 0 at 000 does not match unweighted 000.
    auto surfaceA = SurfaceWithSinusoidalControlPolygon (
                uOrder, vOrder,
                numI, numJ,
                0.1, 0.4,
                0.4, 0.8, 0.0);
    auto surfaceB = SurfaceWithSinusoidalControlPolygon (
                uOrder, vOrder,
                numI, numJ,
                0.1, 0.4,
                0.4, 0.8, weight);
    for (auto surface : {surfaceA, surfaceB})
        {
        Check::Bool (surface.get () == surfaceB.get (), surface->HasWeights ());
        int uKnotsInt = surface->GetIntNumUKnots ();
        int vKnotsInt = surface->GetIntNumVKnots ();
        size_t uKnots = surface->GetNumUKnots ();
        size_t vKnots = surface->GetNumVKnots ();

        Check::Int (uKnotsInt, (int)uKnots, "u knot queries");
        Check::Int (vKnotsInt, (int)vKnots, "u knot queries");

        Check::Int (uKnotsInt, surface->GetIntNumUPoles () + surface->GetIntUOrder ());
        Check::Int (vKnotsInt, surface->GetIntNumVPoles () + surface->GetIntVOrder ());

        Check::Size (numI * numJ, surface->GetNumPoles ());
        Check::Int (surface->GetIntNumPoles (), (int)surface->GetNumPoles ());

        bvector<bool> manyBools {true, false, true, false};

        for (bool b : manyBools)
            {
            surface->SetSurfaceDisplay (b);
            Check::Bool (b, surface->GetSurfaceDisplay ());
            surface->SetPolygonDisplay (b);
            Check::Bool (b, surface->GetPolygonDisplay ());
            }

        for (size_t j = 0; j < numJ; j++)
            {
            for (size_t i = 0; i < numI; i++)
                {
                DPoint3d xyz = surface->GetPole (i, j);
                DPoint3d xyz1 = xyz + DVec3d::From (1,2,3);
                surface->SetPole (i, j, xyz1);
                DPoint3d xyz2 = surface->GetPole (i, j);
                Check::Exact (xyz1, xyz2);
                Check::Exact (surface->GetPole (i,j), surface->GetPole ((int)i, (int)j));
                size_t k = j * numI + i;
                DPoint3d xyz3 = surface->GetPole (k);
                Check::Exact (xyz3, xyz2);
                surface->SetPole (k, xyz);
                Check::Exact (surface->GetPole (k), surface->GetPole ((int)k));

                DPoint3d xyz4 = surface->GetPole (i, j);
                Check::Exact (xyz4, xyz);
                }
            }
        }

    surfaceB->UnWeightPoles ();
    // xyz parts are now equal (other than bit losss in division)
    for (size_t i = 0; i < numI * numJ; i++)
        Check::Near (surfaceA->GetPole (i), surfaceB->GetPole (i));
    surfaceB->WeightPoles ();
    for (size_t i = 0; i < numI * numJ; i++)
        {
        Check::True(surfaceA->GetPole (i).AlmostEqual (surfaceB->GetUnWeightedPole (i)));
        }
// change all weights . .
    for (size_t j = 0; j < numJ; j++)
        {
        for (size_t i = 0; i < numI; i++)
            {
            size_t k = i + numI * j;
            double e = k / 247.0;
            double w = surfaceB->GetWeight (k);
            double we = w + e;
            surfaceB->SetWeight (k, we);
            Check::ExactDouble (surfaceB->GetWeight ((int)k), we);
            Check::ExactDouble (we, surfaceB->GetWeight (k));
            surfaceB->SetWeight ((int)k, w);
            Check::ExactDouble (w, surfaceB->GetWeight (k));
            }
        }
    }

static bool s_printAllKnots = false;
void PrintKnots (char const *name, double const *values, int n, int *counts = NULL)
    {
    if (!s_printAllKnots)
        return;
    printf ("\n<%s>", name);
    for (int i = 0; i <n; i++)
        {
        printf ("\n     %.4g", values[i]);
        if (NULL != counts)
            printf ("  %d", counts[i]);
       }
    printf ("\n</%s>", name);
    }


void TestBasisFunctions (MSBsplineCurveCR curve)
    {
    size_t order = curve.GetOrder ();
    size_t numKnots = curve.GetNumKnots ();
    size_t rightKnot;
    double b[MAX_ORDER];
    double db[MAX_ORDER];
    for (size_t i = order - 1; i + order < numKnots; i++)
        {
        // i is the left index of a knot interval.
        // skip if trivial ..
        if (!MSBsplineCurve::AreSameKnots (curve.GetKnot (i), curve.GetKnot (i + 1)))
            {
            double u = 0.5 * (curve.GetKnot (i) + curve.GetKnot (i+1));
            curve.KnotToBlendFunctions (b, db, rightKnot, u);
            Check::Size (i+1, rightKnot, "knot search");
            Check::Near (1.0, DoubleOps::Sum (b, (int)order), "basis functions sum to 1");
            Check::Near (0.0, DoubleOps::Sum (db, (int)order), "basis function derivatives sum to 0");
            }
        }
    };

 void TestDistinctKnots (int order, bool closed)
    {
    double knotA[100], knotB[100];
    int    countA[100], countB[100];
    bvector<DPoint3d> poles;
    poles.push_back (DPoint3d::From (0,0,0));
    poles.push_back (DPoint3d::From (1,0,0));
    poles.push_back (DPoint3d::From (1,1,0));
    poles.push_back (DPoint3d::From (0,1,0));
    poles.push_back (DPoint3d::From (0,2,0));
    poles.push_back (DPoint3d::From (0,3,0));
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, order, closed, true);
    
    int numTotal = curve->NumberAllocatedKnots ();
    int numA, numB;
    if (s_printAllKnots)
        printf ("\n<Curve order=\"%d\"  closed=\"%s\" numPoles=\"%d\">\n", order, closed ? "true" : "false", curve->NumberAllocatedPoles ());
    PrintKnots ("KnotsInCurve", curve->GetKnotCP (), numTotal);
    bspknot_getKnotMultiplicity (knotA, countA, &numA, const_cast <double*> (curve->GetKnotCP ()), curve->NumberAllocatedPoles (), order, closed, 1.0e-9);
    PrintKnots ("distinctA", knotA, numA, countA);
    bsputil_getKnotMultiplicityExt (knotB, countB, &numB, const_cast <double*> (curve->GetKnotCP ()), curve->NumberAllocatedPoles (), order, closed, 1.0e-9);
    PrintKnots ("distinctB", knotB, numB, countB);
    TestBasisFunctions (*curve);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST(BsplineCurve,DistinctKnot)
    {
    TestDistinctKnots (4, true);
    TestDistinctKnots (4, false);
    }    
    
    
double doubleFunc (size_t i)
    {
    return (double)i * 2.0 + 17.5;
    }


bool ScopedArrayTest (size_t n)
    {
    bvector<double> data;
    ScopedArray <double> scopedData0 (n);
    for (size_t i = 0; i < n; i++)
        {
        data.push_back (doubleFunc (i));
        scopedData0.GetData ()[i] = data.back ();
        }
    ScopedArray <double> scopedData1 (n, &data[0]);
    // YES -- exact equality tests -- they really should match.
    for (size_t i = 0; i < n; i++)
        {
        if (!Check::True (data[i] == scopedData1.GetData ()[i], "scoped data with copy-in"))
            return false;
        if (!Check::True (data[i] == scopedData0.GetData ()[i], "scoped data with caller copy"))
            return false;
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ScopedArray, Allocations)
    {
    for (size_t i = 1; i < 20000; i += 10 * i)
        ScopedArrayTest (i);
    }
    
#ifdef TestScopedRectangularMatrix
static int s_noisy = 0;
template <typename T>
struct ScopedRectangularArray : ScopedArray<T>
{
protected:
size_t m_numRow, m_numColumn;
T m_defaultValue;
public:
ScopedRectangularArray (size_t numRow, size_t numColumn, T const defaultValue)
  : m_numRow (numRow),
    m_numColumn (numColumn),
    m_defaultValue (defaultValue),
    ScopedArray<T> (numRow * numColumn)
    {
    T* data = this->GetData ();
    for (size_t i = 0; i < numRow * numColumn; ++i)
        data[i] = defaultValue;
    }

size_t GetNumColumn () const { return m_numColumn;}
size_t GetNumRow () const { return m_numRow;}
size_t GetNumEntry () const { return m_numRow * m_numColumn;}

bool CopyFrom (ScopedRectangularArray const &source)
    {
    if (m_numRow != source.m_numRow)
        return false;
    if (m_numColumn != source.m_numColumn)
        return false;
    size_t n = GetNumEntry ();
    T *pA = GetData ();
    T const *pB = source.GetDataCP ();
    for (size_t i = 0; i < n; i++)
        pA[i] = pB[i];
    return true;
    }

T * GetRowP (size_t j)
    {
    assert (j < m_numRow);
    return GetData () + j * m_numColumn;
    }

T * GetColumnP (size_t j)
    {
    assert (j < m_numColumn);
    return GetData () + j;
    }

T const * GetRowCP (size_t i) const
    {
    assert (i < m_numRow);
    return GetDataCP () + i * m_numColumn;
    }

T const * GetColumnCP (size_t i) const
    {
    assert (i < m_numColumn);
    return GetDataCP () + i;
    }

bool TryGetIndex (size_t i, size_t j, size_t &index) const
    {
    if (i < m_numRow && j < m_numColumn)
        {
        index = j + m_numColumn * i;
        return true;
        }
    if (i > m_numRow)
        i = m_numRow - 1;
    if (j > m_numColumn)
        j = m_numColumn - 1;
    index = i + m_numColumn * j;
    return false;
    }

bool TryGetAt (size_t i, size_t j, T &data) const
    {
    size_t index;
    if (TryGetIndex (i, j, index))
        {
        data = this->GetDataCP () [index];
        return true;
        }
    data = m_defaultValue;
    return false;
    }

bool TrySetAt (size_t i, size_t j, T const &data)
    {
    size_t index;
    if (TryGetIndex (i, j, index))
        {
        GetData () [index] = data;
        return true;
        }
    return false;
    }

T& TryGetAt (size_t i, size_t j)
    {
    size_t index;
    if (TryGetIndex (i, j, index))
        return GetData () [index];
    return defaultValue;
    }

};

struct ScopedDMatrix;
void Print (char *, ScopedDMatrix const &, int minNoisy = 1);
struct ScopedDMatrix : ScopedRectangularArray<double>
{
ScopedDMatrix (size_t numRow, size_t numColumn)
  : ScopedRectangularArray <double> (numRow, numColumn, 0.0)
    {
    }

void Zero ()
    {
    size_t n = GetNumEntry ();
    double *data = GetData ();
    for (size_t i = 0; i < n; i++)
        data[i] = 0.0;
    }

void InitIdentity ()
    {
    size_t n = GetNumEntry ();
    double *data = GetData ();
    for (size_t i = 0; i < n; i++)
        data[i] = 0.0;
    size_t diagonalStep = GetNumColumn () + 1;
    for (size_t k = 0; k < n; k += diagonalStep)
        data[k] = 1.0;
    }

double MaxAbsDiff (ScopedDMatrix const &matrixB) const
    {
    size_t n = GetNumEntry ();
    double const *data = GetDataCP ();
    double const *dataB = matrixB.GetDataCP ();
    if (n != matrixB.GetNumEntry ())
        return DBL_MAX;
    double a = 0.0;
    for (size_t i = 0; i < n; i++)
        a = DoubleOps::MaxAbs (a, fabs (dataB[i] - data[i]));
    return a;
    }

bool TrySet (size_t i0, size_t iStep, size_t j0, size_t jStep)
    {
    }

double RowDotColumn (
size_t rowA,
ScopedDMatrix const &dataB,
size_t columnB
) const
    {
    double sum = 0.0;
    size_t n = GetNumColumn ();
    assert (n == dataB.GetNumRow ());
    if (n != dataB.GetNumRow ())
        return sum;
    size_t stepB = dataB.GetNumColumn ();
    double const *pA = GetRowCP (rowA);
    double const *pB = dataB.GetColumnCP (columnB);
    for (size_t i = 0, k = 0; i < n; i++, k += stepB)
        {
        sum += pA[i] * pB[k];
        }
    return sum;
    }

bool  IndexOfMaxAbsInColumnTail (size_t startRow, size_t column, size_t &row, double &maxAbs) const
    {
    row = startRow;
    maxAbs = 0.0;
    size_t numRow = GetNumRow ();
    if (row >= numRow)
        return false;
    size_t NumColumn = GetNumColumn ();
    if (column >= NumColumn)
        return false;
    double const *pA = GetColumnCP (column) + startRow * NumColumn;
    maxAbs = fabs (*pA);
    for (size_t i = startRow + 1;
          pA += NumColumn, i < NumColumn;   // That's a comma so the first one steps away from startRow
          i++)
        {
        double a = fabs (*pA);
        if (a > maxAbs)
            {
            maxAbs = a;
            row = i;
            }
        }
    return true;
    }

// Set each  B[pivot][j] = B[pivot][j] / A[pivot][pivot]
// Subtract   B[numActiveRow][j] * A[0..numActiveRow-1][columnA] from columns of B
static bool  BacksubStep
(
ScopedDMatrix const &dataA,
size_t pivot,
ScopedDMatrix &dataB
)
    {
    assert (pivot < dataA.GetNumRow ());
    assert (pivot < dataB.GetNumRow ());
    size_t numColumnB = dataB.GetNumColumn ();
    size_t stepA = dataA.GetNumColumn ();
    size_t stepB = numColumnB;
    double a;
    if (!dataA.TryGetAt (pivot, pivot, a))
        return false;
    double *pB1 = dataB.GetRowP (pivot);
    for (size_t columnB = 0; columnB < numColumnB; columnB++)
        {
        double b = pB1[columnB];
        double x;
        if (!DoubleOps::SafeDivide (x, b, a, 0.0))
            return false;
        pB1[columnB] = x;
        double const *pA = dataA.GetColumnCP (pivot);
        double *pB = dataB.GetColumnP (columnB);
        for (size_t i = 0; i < pivot; i++, pA += stepA, pB += stepB)
            *pB -= (*pA) * x;
        }
    return true;
    }

void SwapRows (size_t row0, size_t row1)
    {
    size_t NumColumn = GetNumColumn ();
    double *p0 = GetRowP (row0);
    double *p1 = GetRowP (row1);
    for (size_t i = 0; i < NumColumn; i++, p0++, p1++)
        {
        std::swap (*p0, *p1);
        }
    }
// For rows i > pivotRow and columns J > pivotColumn
//     data[i][j] += factor * data[i][pivotColumn] * data[pivotRow][j]
//
void LowerRightGaussUpdate
(
size_t pivotRow,
size_t pivotColumn,
double factor,
ScopedDMatrix &matrixB
)
    {
    double *pA0 = GetRowP (pivotRow);
    double *pB0 = matrixB.GetRowP (pivotRow);
    size_t numRow = GetNumRow ();
    size_t numColumn = GetNumColumn ();
    size_t numColumnB = matrixB.GetNumColumn ();
    if (pivotColumn + 1 >= numColumn)
        return;
    for (size_t i = pivotRow + 1; i < numRow; i++)
        {
        double *pA1 = GetRowP (i);
        double c = factor * pA1[pivotColumn];
        double *pB1 = matrixB.GetRowP (i);
        if (c != 0.0)
            {
            for (size_t j = pivotColumn + 1; j < numColumn; j++)
                pA1[j] += c * pA0[j];
            for (size_t j = 0; j < numColumnB; j++)
                pB1[j] += c * pB0[j];
            }
        }
    }

// INPLACE in both A and B.
static bool GaussianElimination (ScopedDMatrix &matrixA, ScopedDMatrix &matrixB)
    {
    size_t n = matrixA.GetNumRow ();
    assert (n == matrixA.GetNumColumn ());
    assert (n == matrixB.GetNumRow ());
    double a, diva;
    for (size_t pivot = 0; pivot + 1 < n; pivot++)
        {
        size_t k;
        double aMax;
        matrixA.IndexOfMaxAbsInColumnTail (pivot, pivot, k, aMax);
        if (k != pivot)
            {
            matrixA.SwapRows (pivot, k);
            matrixB.SwapRows (pivot, k);
            if (s_noisy > 100)
                printf (" (Swap %d %d)\n", (int)pivot, (int)k);
            }
        matrixA.TryGetAt (pivot, pivot, a);
        if (!DoubleOps::SafeDivide (diva, 1.0, a, 0.0))
            return false;
        matrixA.LowerRightGaussUpdate (pivot, pivot, -diva, matrixB);
        if (s_noisy > 10)
          {
          Print ("LU update", matrixA);
          Print ("L$B update", matrixB);
          }
        }
    for (size_t i = n; i > 0;)
        {
        i--;
        if (!BacksubStep (matrixA, i, matrixB))
            return false;
        }
    return true;
    }

static bool Multiply (ScopedDMatrix const &matrixA, ScopedDMatrix const &matrixB, ScopedDMatrix &matrixC)
    {
    size_t numRowA = matrixA.GetNumRow ();
    size_t numColumnA = matrixA.GetNumColumn ();
    size_t numRowB = matrixB.GetNumRow ();
    size_t numColumnB = matrixB.GetNumColumn ();
    if (numColumnA != numRowB)
        return false;
    if (numRowA != matrixC.GetNumRow ())
        return false;
    if (numColumnB != matrixC.GetNumColumn ())
        return false;
    for (size_t i = 0; i < numRowA; i++)
        for (size_t j = 0; j < numColumnB; j++)
            matrixC.TrySetAt (i, j, matrixA.RowDotColumn (i, matrixB, j));
    return true;
    }
};

void Print (char *name, ScopedDMatrix const &A, int minNoisy)
    {
    if (s_noisy < minNoisy)
        return;
    printf ("\n<%s> (%d,%d)\n", name, (int)A.GetNumRow (), (int)A.GetNumColumn ());
    for (size_t i = 0; i < A.GetNumRow (); i++)
        {
        printf ("\n  ");
        for (size_t j = 0; j < A.GetNumColumn (); j++)
            {
            double a;
            A.TryGetAt (i, j, a);
            printf (" %g", a);
            }
        }
    printf ("\n</%s>\n", name);
    }

void Print (char *name, RotMatrixCR &A)
    {
    printf ("\n<%s>\n", name);
    for (size_t i = 0; i < 3; i++)
        {
        printf ("\n  ");
        for (size_t j = 0; j < 3; j++)
            {
            printf (" %g", A.form3d[i][j]);
            }
        }
    printf ("\n</%s>\n", name);
    }


void HelloMatrix (size_t n)
    {
    double a = 2.0;
    double b = 3.5;
    size_t numColumn = 2 * n;
    bool ok = true;
    for (size_t columnShift = 0; ok && columnShift <= n; columnShift++)
        {
        // A has a diagonal starting at [0,columnShift] ...
        ScopedDMatrix A (n, numColumn);
        ScopedDMatrix B (numColumn, numColumn);

        for (size_t i = 0; i < n; i++)
            A.TrySetAt (i,i + columnShift, 1.0 + a * i);
        for (size_t i = 0; i < numColumn; i++)
            B.TrySetAt (i,i, 1.0 + b * i);
        if (s_noisy)
            {
            printf (" (n %d) (shift %d)\n", (int)n, (int)columnShift);
            Print ("A", A);
            Print ("B", B);
            }
        for (size_t i = 0; ok && i < n; i++)
            {
            for (size_t k = 0; ok && k < numColumn; k++)
                {
                double f = (1 + a * i) * (1 + b * k);
                double dot = A.RowDotColumn (i, B, k);
                if (columnShift + i == k)
                    {
                    ok &= Check::Near (f, dot, "AB diagonal term");
                    }
                else
                    ok &= Check::Near (0.0, dot);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ScopedRectangular,HelloWorld)
    {
    HelloMatrix (1);
    HelloMatrix (2);
    HelloMatrix (5);
    HelloMatrix (8);

    }

void CopyToScopedArray (RotMatrixCR source, ScopedDMatrix &dest)
    {
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            dest.TrySetAt (i, j, source.form3d[i][j]);
    }

bool CheckInverse3 (RotMatrixCR rmA)
    {
    RotMatrix rmB;
    RotMatrix rmI;
    rmI.InitIdentity ();
    rmB.InverseOf (rmA);

    ScopedDMatrix A(3,3);
    ScopedDMatrix B(3,3);
    CopyToScopedArray (rmA, A);
    CopyToScopedArray (rmI, B);
    if (s_noisy)
        {
        Print ("A", A);
        Print ("RotMatrix A", rmA);
        }
    ScopedDMatrix::GaussianElimination (A, B);
    if (s_noisy)
        {
        Print ("Ainv", B);
        Print ("RotMatrix Ainv", rmB);
        }
    for (size_t i = 0; i < 3; i++)
        {
        for (size_t j = 0; j < 3; j++)
            {
            double b;
            Check::True (B.TryGetAt (i, j, b), "access inverse");
            Check::Near (rmB.form3d[i][j], b, "inverse entry");
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ScopedRectangular,Gaussian3)
    {
    CheckInverse3 (RotMatrix::FromRowValues (
                    3,2,1,
                    1,4,-1,
                    0.4,0.2,1
                    ));
    CheckInverse3 (RotMatrix::FromRowValues (
                    1,4,-1,
                    3,2,1,
                    0.4,0.2,1
                    ));

    CheckInverse3 (RotMatrix::FromRowValues (
                    1,4,-1,
                    0.4,0.2,1,
                    3,2,1
                    ));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ScopedRectangular,Gaussian1)
    {
    ScopedDMatrix A(1,1);
    ScopedDMatrix B(1,3);
    double a = 5.0;
    A.TrySetAt (0,0, a);
    for (size_t j = 0; j < 3; j++)
        B.TrySetAt (0,j, a * j);
    Check::True (ScopedDMatrix::GaussianElimination (A, B), "1x1 gaussian");
    for (size_t j = 0; j < 3; j++)
        {
        double x;
        B.TryGetAt (0, j, x);
        Check::Near (x, (double)j, "1x1 gaussian solution");
        }
    }

// test inversion with
// a = diagonal value
// b = off diagonal value
// n = matrix dimension
// numBelow = number of diagonals below the main diagonal
// numAblve = number of diagonals above the main diagonal
bool TestDiagonalInverse (double a, double b, ptrdiff_t n, ptrdiff_t numBelow, ptrdiff_t numAbove)
    {
    ScopedDMatrix A (n, n);
    ScopedDMatrix A0 (n, n);
    ScopedDMatrix B (n, n);
    ScopedDMatrix C (n, n);
    ScopedDMatrix I (n, n);
    A.Zero ();
    for (ptrdiff_t i = 0; i < n; i++)
        {
        for (ptrdiff_t shift = -numBelow; shift <= numAbove; shift++)
            {
            if (shift == 0)
                A.TrySetAt (i, i, a);
            else
                A.TrySetAt (i, shift+i, b);   // Let TrySetAt fail when the shift runs out of the array
            }
        }

    B.InitIdentity ();
    I.InitIdentity ();
    Print ("A", A, 5);
    Print ("B", B, 5);
    A0.CopyFrom (A);
    ScopedDMatrix::GaussianElimination (A, B);
    ScopedDMatrix::Multiply (A0, B, C);
    Print ("LU", A, 5);
    Print ("Ainv", B, 5);
    Print ("C", C, 5);
    Print ("I", I, 5);
    double e = C.MaxAbsDiff (I);
    return Check::Near (1.0, 1.0 + e, "inversion round trip error");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ScopedRectangular, MediumInverse)
    {
    // strongly diagonal test matrices. ...
    TestDiagonalInverse (1.0, 0.1, 5, 1, 0);
    TestDiagonalInverse (1.0, 0.1, 5, 1, 1);
    TestDiagonalInverse (1.0, 0.1, 9, 2, 2);
    TestDiagonalInverse (1.0, 0.1, 20, 4, 6);
    //s_noisy = 101;
    // weak diagonal test matrix ...
    TestDiagonalInverse (1.0, 5.1, 3, 1, 1);
    }
#endif    
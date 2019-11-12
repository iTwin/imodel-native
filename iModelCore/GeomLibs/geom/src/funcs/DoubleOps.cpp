/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_smallCoordinateRelTol = 1.0e-10;
static double s_smallMetricDistance = 1.0e-6;
static double s_floatCoordinateRelTol = 2.0e-6;
double DoubleOps::SmallCoordinateRelTol () {return s_smallCoordinateRelTol;}
double DoubleOps::SmallMetricDistance () {return s_smallMetricDistance;}
double DoubleOps::FloatCoordinateRelTol () {return s_floatCoordinateRelTol;}

double DoubleOps::DeterminantXYXY (double x0, double y0, double x1, double y1)
    {
    return x0 * y1 - x1 * y0;
    }
double DoubleOps::PreciseSum (double * data, size_t n)
    {
    if (n == 0)
        return 0.0;
    double sum = data[0];
    double c = 0.0;
    double y, t;
     for (size_t i = 1; i < n; i++)
        {
        y = data[i] - c;
        t = sum + y;
        c = (t - sum) - y;
        sum = t;
        } 
    return sum;
    }

double DoubleOps::PreciseSum (double a, double b, double c)
    {
    double data[3];
    data[0] = a;
    data[1] = b;
    data[2] = c;
    return DoubleOps::PreciseSum (data, 3);
    }

double DoubleOps::PreciseSum (double a, double b, double c, double d)
    {
    double data[4];
    data[0] = a;
    data[1] = b;
    data[2] = c;
    data[3] = d;
    return DoubleOps::PreciseSum (data, 4);
    }

double DoubleOps::Sum (double * data, int n)
    {
    double s = 0.0;
    for (int i = 0; i < n; i++)
        s += data[i];
    return s;
    }

double DoubleOps::SumAbs (double * data, int n)
    {
    double s = 0.0;
    for (int i = 0; i < n; i++)
        s += fabs (data[i]);
    return s;
    }

double DoubleOps::Sum (bvector<double> const &data)
    {
    double s = 0.0;
    for (size_t i = 0, n = data.size (); i < n; i++)
        s += data[i];
    return s;
    }

double DoubleOps::SumAbs (bvector<double> const &data)
    {
    double s = 0.0;
    for (size_t i = 0, n = data.size (); i < n; i++)
        s += fabs (data[i]);
    return s;
    }

double DoubleOps::Interpolate(double  dataA, double fraction, double dataB)
    {
    return dataA + fraction * (dataB - dataA);
    }

ValidatedDouble DoubleOps::InverseInterpolate(double  dataA, double value, double dataB)
    {
    if (AlmostEqual (dataA, dataB))
        return ValidatedDouble (0.0, false);
    return ValidatedDouble ((value - dataA) / (dataB - dataA), true);
    }

double DoubleOps::Max (double a1, double a2)
    {
    return a1 > a2 ? a1 : a2;
    }

double DoubleOps::Max (double a1, double a2, double a3)
    {
    double a = a1;
    if (a2 > a)
        a = a2;
    if (a3 > a)
        a = a3;
    return a;
    }

double DoubleOps::Max (double a1, double a2, double a3, double a4)
    {
    double a = a1;
    if (a2 > a)
        a = a2;
    if (a3 > a)
        a = a3;
    if (a4 > a)
        a = a4;
    return a;
    }

double DoubleOps::MaxAbs (double a1, double a2)
    {
    a1 = fabs (a1);
    a2 = fabs (a2);
    return a1 > a2 ? a1 : a2;
    }

double DoubleOps::MaxAbs (double a1, double a2, double a3)
    {
    a1 = fabs (a1);
    a2 = fabs (a2);
    a3 = fabs (a3);
    double a = a1;
    if (a2 > a)
        a = a2;
    if (a3 > a)
        a = a3;
    return a;
    }

double DoubleOps::MaxAbs (double a1, double a2, double a3, double a4)
    {
    a1 = fabs (a1);
    a2 = fabs (a2);
    a3 = fabs (a3);
    a4 = fabs (a4);
    double a = a1;
    if (a2 > a)
        a = a2;
    if (a3 > a)
        a = a3;
    if (a4 > a)
        a = a4;
    return a;
    }

double DoubleOps::MaxAbs (double a1, double a2, double a3, double a4, double a5, double a6)
    {
    a1 = fabs (a1);
    a2 = fabs (a2);
    a3 = fabs (a3);
    a4 = fabs (a4);
    a5 = fabs (a5);
    a6 = fabs (a6);
    double a = a1;
    if (a2 > a)
        a = a2;
    if (a3 > a)
        a = a3;
    if (a4 > a)
        a = a4;
    if (a5 > a)
        a = a5;
    if (a6 > a)
        a = a6;
    return a;
    }

//! update evolving minmax f values with associated x values.
void DoubleOps::UpdateMinMax (double x, double f, double &xMin, double &fMin, double &xMax, double &fMax)
    {
    if (f < fMin)
        {
        xMin = x;
        fMin = f;
        }
    if (f > fMax)
        {
        xMax = x;
        fMax = f;
        }
    }

//! return sqrt (a*a + b*b + c*c)
double DoubleOps::Magnitude (double a, double b, double c)
    {
    return sqrt (a*a + b*b + c*c);
    }

//! return sqrt (a*a + b*b)
double DoubleOps::Magnitude (double a, double b)
    {
    return sqrt (a*a + b*b);
    }

//! return -1,0,1 for comparison, but with toleranced equal case
int DoubleOps::TolerancedComparison(double a, double b)
    {
    if (AlmostEqual (a, b))
        return 0;
    if (a < b)
        return -1;
    return 1;
    }


bool DoubleOps::AlmostEqualFraction (double a, double b)
    {
    return fabs (b-a) < Angle::SmallAngle ();
    }

bool DoubleOps::ClearlyIncreasingFraction (double a, double b)
    {
    return b > a + Angle::SmallAngle ();
    }

bool DoubleOps::ClearlyIncreasing (double a, double b, double tol)
    {
    return b > a + tol;
    }

void DoubleOps::UpdateMax (double &evolvingMax, double testValue)
    {
    if (testValue > evolvingMax)
        evolvingMax = testValue;
    }

double DoubleOps::Min (double a1, double a2)
    {
    return a1 < a2 ? a1 : a2;
    }

double DoubleOps::Min (double a1, double a2, double a3)
    {
    double a = a1;
    if (a2 < a)
        a = a2;
    if (a3 < a)
        a = a3;
    return a;
    }

double DoubleOps::Min (double a1, double a2, double a3, double a4)
    {
    double a = a1;
    if (a2 < a)
        a = a2;
    if (a3 < a)
        a = a3;
    if (a4 < a)
        a = a4;
    return a;
    }

double DoubleOps::MinAbs (double a1, double a2)
    {
    a1 = fabs (a1);
    a2 = fabs (a2);
    return a1 < a2 ? a1 : a2;
    }

double DoubleOps::MinAbs (double a1, double a2, double a3)
    {
    a1 = fabs (a1);
    a2 = fabs (a2);
    a3 = fabs (a3);
    double a = a1;
    if (a2 < a)
        a = a2;
    if (a3 < a)
        a = a3;
    return a;
    }

double DoubleOps::MinAbs (double a1, double a2, double a3, double a4)
    {
    a1 = fabs (a1);
    a2 = fabs (a2);
    a3 = fabs (a3);
    a4 = fabs (a4);
    double a = a1;
    if (a2 < a)
        a = a2;
    if (a3 < a)
        a = a3;
    if (a4 < a)
        a = a4;
    return a;
    }

bool DoubleOps::IsExact01 (double a0, double a1)
    {
    return a0 == 0.0 && a1 == 1.0;
    }

bool DoubleOps::IsIn01 (double x)
    {
    return x >= 0.0 && x <= 1.0;
    }

bool DoubleOps::IsAlmostIn01 (double x)
    {
    return (x >= 0.0 && x <= 1.0)
        || AlmostEqualFraction (x, 0.0)
        || AlmostEqualFraction (x, 1.0);
    }

bool DoubleOps::IsIn01OrExtension (double x, bool extend0, bool extend1)
    {
    if (x < 0.0)
        return extend0;
    if (x > 1.0)
        return extend1;
    return true;
    }


bool DoubleOps::IsIn01 (double x, double y)
    {
    return x >= 0.0 && x <= 1.0
        && y >= 0.0 && y <= 1.0;
    }
bool DoubleOps::IsIn01 (DPoint2dCR xy)
    {
    return xy.x >= 0.0 && xy.x <= 1.0
        && xy.y >= 0.0 && xy.y <= 1.0;
    }

bool DoubleOps::IsIn01 (DPoint3dCR xyz)
    {
    return xyz.x >= 0.0 && xyz.x <= 1.0
        && xyz.y >= 0.0 && xyz.y <= 1.0
        && xyz.z >= 0.0 && xyz.z <= 1.0;
    }


double DoubleOps::ClampFraction (double s)
    {
    if (s < 0.0)
        return 0.0;
    if (s > 1.0)
        return 1.0;
    return s;
    }

double DoubleOps::Clamp (double s, double a, double b)
    {
    if (s < a)
        return a;
    if (s > b)
        return b;
    return s;
    }



void DoubleOps::ClampDirectedFractionInterval (double &t0, double &t1)
    {
    if (t0 <= t1)
        {
        if (t0 > 1.0)
            {
            t0 = t1 = 1.0;
            return;
            }
        if (t1 < 0.0)
            {
            t0 = t1 = 0.0;
            return;
            }
        t0 = t0 < 0.0 ? 0.0 : t0;
        t1 = t1 > 1.0 ? 1.0 : t1;
        }
    else    // t1 < t0 (strictly) -- this can only recurse once.
        ClampDirectedFractionInterval (t1, t0);
    }


static double s_safeDenominatorRatio_Parameter        = 1.0e-10;
static double s_safeDenominatorRatio_Distance        = 1.0e-15;
static double s_safeDenominatorRatio_DistanceSquared = 1.0e-30;

bool DoubleOps::SafeDivide (double &result, double numerator, double denominator, double defaultResult, double fraction)
    {
    if (fabs (denominator) > fraction * fabs (numerator))
        {
        result = numerator / denominator;
        return true;
        }
    result = defaultResult;
    return false;
    }

bool DoubleOps::LinearTransform (double a0, double a1, double b0, double b1, double &c, double &d)
    {
    double da = a1 - a0;
    double db = b1 - b0;
    bool stat = SafeDivide (d, db, da, 0.0);
    c = b0 - a0 * d;
    return stat;
    }

bool DoubleOps::ChooseSafeDivideParameter
(
double &result,
double numerator0,
double denominator0,
double numerator1,
double denominator1,
double defaultResult
)
    {
    if (fabs (denominator0) >= fabs (denominator1))
        return SafeDivideParameter (result, numerator0, denominator0, defaultResult);
    else
        return SafeDivideParameter (result, numerator1, denominator1, defaultResult);
    }

bool DoubleOps::SafeDivideParameter (double &result, double numerator, double denominator, double defaultResult)
    {
    return SafeDivide (result, numerator, denominator, defaultResult, s_safeDenominatorRatio_Parameter);
    }

bool DoubleOps::SafeDivideDistance (double &result, double numerator, double denominator, double defaultResult)
    {
    return SafeDivide (result, numerator, denominator, defaultResult, s_safeDenominatorRatio_Distance);
    }

bool DoubleOps::SafeDivideDistanceSquared (double &result, double numerator, double denominator, double defaultResult)
    {
    return SafeDivide (result, numerator, denominator, defaultResult, s_safeDenominatorRatio_DistanceSquared);
    }

ValidatedDouble DoubleOps::ValidatedDivideDistance (double a, double b, double defaultResult)
    {
    return ValidatedDivide(a, b, defaultResult, s_safeDenominatorRatio_Distance);
    }

ValidatedDouble DoubleOps::ValidatedDivideDistanceSquared (double a, double b, double defaultResult)
    {
    return ValidatedDivide(a, b, defaultResult, s_safeDenominatorRatio_DistanceSquared);
    }

ValidatedDouble DoubleOps::ValidatedDivideParameter (double a, double b, double defaultResult)
    {
    return ValidatedDivide(a, b, defaultResult, s_safeDenominatorRatio_Parameter);
    }

ValidatedDouble DoubleOps::ValidatedDivide (double a, double b, double defaultResult, double minRelativeDivisor)
    {
    double result;
    bool stat = SafeDivide(result, a, b, defaultResult, minRelativeDivisor);
    return ValidatedDouble(result, stat);
    }

ValidatedDVec2d DoubleOps::ValidatedDivideAndDifferentiate (double f, double df, double g, double dg)
    {
    auto q = ValidatedDivide (f,g);
    auto dq = ValidatedDivide (df * g - f * dg, g * g);
    if (q.IsValid () && dq.IsValid ())
        return DVec2d::From (q, dq);
    return ValidatedDVec2d ();
    }

bool DoubleOps::WithinTolerance (double a1, double a2, double abstol)
    {
    return fabs (a1 - a2) <= fabs (abstol);
    }

bool DoubleOps::WithinTolerance (double a1, double a2, double abstol, double reltol)
    {
    return fabs (a1 - a2) <= fabs (abstol) + fabs (reltol) * Max (a1, a2);
    }

double DoubleOps::ComputeTolerance (double maxSize, double absTol, double relTol)
    {
    if (absTol <= 0.0)
        absTol = Angle::SmallAngle ();
    if (relTol <= 0.0)
        relTol = Angle::SmallAngle ();
    return absTol + relTol * fabs (maxSize);
    }

double DoubleOps::Max (bvector <double> &values)
    {
    if (values.size () == 0)
        return 0.0;

    bvector<double>::const_iterator i = values.begin ();
    double a = *i;
    for (i++; i != values.end (); ++i)
        {
        if (a < *i)
            a = *i;
        }
    return a;
    }

double DoubleOps::Min (bvector <double> &values)
    {
    if (values.size () == 0)
        return 0.0;

    bvector<double>::const_iterator i = values.begin ();
    double a = *i;
    for (i++; i != values.end (); ++i)
        {
        if (a > *i)
            a = *i;
        }
    return a;
    }

double DoubleOps::MaxAbs (bvector <double> &values)
    {
    if (values.size () == 0)
        return 0.0;

    bvector<double>::const_iterator i = values.begin ();
    double a = fabs (*i);
    for (i++; i != values.end (); ++i)
        {
        double b = fabs (*i);
        if (a < b)
            a = b;
        }
    return a;
    }

double DoubleOps::MinAbs (bvector <double> &values)
    {
    if (values.size () == 0)
        return 0.0;

    bvector<double>::const_iterator i = values.begin ();
    double a = fabs (*i);
    for (i++; i != values.end (); ++i)
        {
        double b = fabs (*i);
        if (a > b)
            a = b;
        }
    return a;
    }





double DoubleOps::Max (double const *values, size_t n)
    {
    if (n == 0)
        return 0.0;

    double a = values[0];
    for (size_t i = 1; i < n; ++i)
        {
        if (a < values[i])
            a = values[i];
        }
    return a;
    }

double DoubleOps::Min (double const *values, size_t n)
    {
    if (n == 0)
        return 0.0;

    double a = values[0];
    for (size_t i = 1; i < n; ++i)
        {
        if (a > values[i])
            a = values[i];
        }
    return a;
    }

double DoubleOps::MaxAbs (double const *values, size_t n)
    {
    if (n == 0)
        return 0.0;

    double a = fabs (values[0]);
    for (size_t i = 1; i < n; ++i)
        {
        double b = fabs (values[i]);
        if (a < b)
            a = b;
        }
    return a;
    }

double DoubleOps::MinAbs (double const *values, size_t n)
    {
    if (n == 0)
        return 0.0;

    double a = fabs (values[0]);
    for (size_t i = 1; i < n; ++i)
        {
        double b = fabs (values[i]);
        if (a > b)
            a = b;
        }
    return a;
    }


void DoubleOps::ScaleArray (double *values, size_t n, double a)
    {
    if (values != NULL)
        {
        for (size_t i = 0; i < n; ++i)
            values[i] *= a;
        }
    }
void DoubleOps::ScaleArray (bvector <double> &values, double a)
    {
    for (size_t i = 0, n = values.size (); i < n; i++)
        values[i] *= a;
    }


double Rounding::Round (double value, Rounding::RoundingMode mode, double lowerValue, double upperValue)
    {
    if (lowerValue > upperValue)
        return Round (value, mode, upperValue, lowerValue);

    double roundValue;
    switch (mode)
        {
        case RoundingMode_Down:
            roundValue = value < upperValue ? lowerValue : upperValue;            
            break;
        case RoundingMode_Up:
            roundValue = value > lowerValue ? upperValue : lowerValue;            
            break;
        default:
            roundValue = (value - lowerValue) > (upperValue - value) ? upperValue : lowerValue;
        }
    return roundValue;
    }

bool DoubleOps::UpperBound (double const *pData, size_t n, double searchValue, size_t &index)
    {
    if (0 == n)
        return false;

    size_t i;
    for (i=0; i<n; i++)
        {
        if (pData[i] > searchValue)
            break;
        }
    index = i;

    return true;
    }

bool DoubleOps::LowerBound (double const *pData, size_t n, double searchValue, size_t &index)
    {
    if (0 == n)
        return false;

    size_t i;
    for (i=0; i<n; i++)
        {
        if (!(pData[i] < searchValue))
            break;
        }
    index = i;

    return true;
    }

bool DoubleOps::BoundingValues (bvector<double>const &data, double searchValue,  size_t &lowerIndex, double &lowerBound, double &upperBound)
    {
    return BoundingValues (&data[0], data.size (), searchValue, lowerIndex, lowerBound, upperBound);
    }

bool DoubleOps::BoundingValues (double const *pData, size_t n, double searchValue, size_t &lowerIndex, double &lowerBound, double &upperBound)
    {
    if (0 == n)
        {
        lowerIndex = 0;
        lowerBound = upperBound = 0.0;
        return false;
        }
    
    size_t index;
    UpperBound (pData, n, pData[0], index);
    if (1 == n || index == n)
        return false;

    if (searchValue <= pData[0])
        {/*index = index;*/}
    else if (searchValue >= pData[n-1])
        LowerBound (pData, n, pData[n-1], index);
    else
        UpperBound (pData, n, searchValue, index);

    lowerIndex = index - 1;
    lowerBound = pData[lowerIndex];
    upperBound = pData[lowerIndex+1];

    return true;
    }

bool DoubleOps::BoundingValues (size_t n, double searchValue, size_t &lowerIndex, double &lowerBound, double &upperBound)
    {
    if (1 >= n)
        {
        lowerIndex = 0;
        lowerBound = upperBound = 0.0;
        return false;
        }

    if (searchValue <= 0.0)
        {
        lowerIndex = 0;
        lowerBound = 0.0;
        upperBound = 1.0/(n-1);
        }
    else if (searchValue >= 1.0)
        {
        lowerIndex = n - 2;
        lowerBound = 1.0 - 1.0/(n-1);
        upperBound = 1.0;
        }
    else
        {
        lowerIndex = int (searchValue*(n-1));
        lowerBound = double (lowerIndex)/(n-1);
        upperBound = double (lowerIndex + 1)/(n-1);
        }

    return true;
    }

void DoubleOps::Swap (double &dataA, double &dataB)
    {
    double dataZ = dataA;
    dataA = dataB;
    dataB = dataZ;
    }

static bool util_descendDoubles_std (double const &dataA, double const &dataB)
    {
    return dataA > dataB;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlUtil_sortDoubles                                     |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void mdlUtil_sortDoubles
(    
double *doubles,        /* <=> array of Doubles to be sorted (in place) */
int    numDoubles,      /* => number of Doubles to sort */
int    ascend           /* => true for ascending order */
)    
    {
    if (ascend)
        std::sort (doubles, doubles + numDoubles);
    else
        std::sort (doubles, doubles + numDoubles, util_descendDoubles_std);
    }
extern void mdlUtil_sortDoubles (double *, int, int);
void DoubleOps::Sort (double *data, size_t count, bool ascending)
    {
    if (count > 1)
        mdlUtil_sortDoubles (data, (int) count, ascending ? true : false);
    }
void DoubleOps::Sort (bvector<double> &data, bool ascending)
    {
    if (data.size () > 1)
        mdlUtil_sortDoubles (&data[0], (int) data.size (), ascending ? true : false);
    }

void DoubleOps::SortTail (bvector<double> &data, size_t i0, bool ascending)
    {
    if (data.size () > i0)
        mdlUtil_sortDoubles (&data[i0], (int) data.size (), ascending ? true : false);
    }

double DoubleOps::Hypotenuse (double a, double b)
    {
    return sqrt (a * a + b * b);
    }

double DoubleOps::Hypotenuse (double a, double b, double c)
    {
    return sqrt (a * a + b * b + c * c);
    }

//! snap to zero if within tolerance.
double DoubleOps::SnapZero (double a, double abstol)
    {
    return fabs (a) < abstol ? 0.0 : a;
    }

//! Return -1,0,1 according to sign, with 0 case subject to tolerance.
int DoubleOps::TolerancedSign (double a, double abstol)
    {
    if (a > abstol)
        return 1;
    if (a < -abstol)
        return -1;
    return 0;
    }

void DoubleOps::MovingAverages(bvector<double> &dest, bvector<double> const &source, size_t blockSize, size_t numSkip0, size_t numSkip1)
    {
    dest.clear ();
    size_t n = source.size ();
    if (blockSize + numSkip0 + numSkip1 > n)
        return;
    if (blockSize < 1)
        return;
    if (blockSize == 1)
        {
        for (size_t i = numSkip0; i + numSkip1 < n; i++)
            dest.push_back (source[i]);
        return;
        }

    for (size_t i0 = numSkip0, n = source.size (); i0 + blockSize + numSkip1 <= n; i0++)
        {
        double sum = source[i0];
        for (size_t i = 1; i < blockSize; i++)
            sum += source[i0+i];
        dest.push_back (sum / (double)blockSize);
        }
    }
void DoubleOps::MovingAverages
(
bvector<double> &dest,
double a0,
double a1,
size_t numOut,
size_t blockSize,
size_t numCopy0,
size_t numCopy1
)
    {
    dest.clear();
    if (blockSize > numOut || numCopy0 < 1 || numCopy1 < 1)
        return;

    ptrdiff_t numInterval = (ptrdiff_t)numOut + (ptrdiff_t)blockSize - 2 - ((ptrdiff_t)numCopy0 - 1) - ((ptrdiff_t)numCopy1 - 1);
    if (numInterval < 1)
        return;
    double da = (a1 - a0) / (double)numInterval;
    size_t lastStartIndex = numCopy0 - 1;                   // last index of an a0 value
    size_t firstTailIndex = lastStartIndex + numInterval;     // first index of an a1 value
    for (size_t i0 = 0; i0 < numOut; i0++)
        {
        double sum = 0.0;
        for (size_t i = i0; i < i0 + blockSize; i++)
            {
            if (i <= lastStartIndex)
                sum += a0;
            else if (i >= firstTailIndex)
                sum += a1;
            else
                sum += a0 + (i - lastStartIndex) * da;
            }
        dest.push_back(sum / (double)blockSize);
        }
    }


double DoubleOps::ChebyshevPoint (size_t i, size_t n)
    {
    return cos (Angle::Pi () * (2 * i + 1) / (2 * (double)n));
    }

void DoubleOps::ChebyshevPoints
(
bvector<double> &dest,
size_t numChebyshev,
bool addLimitPoints,
size_t numCopyLeft,
size_t numCopyRight
)
    {
    dest.clear();
    if (numChebyshev < 2)
        return;

    size_t iStart, iEnd;  // loop controls for the non-replicated values
    double cA, cB;  // actual end values -- these will be replicated
    if (addLimitPoints)
        {
        cA = -1.0;
        cB = 1.0;
        iStart = 0;
        iEnd = numChebyshev;
        // leave iStart and end alone
        }
    else
        {
        cA = ChebyshevPoint(0, numChebyshev);
        cB = ChebyshevPoint (numChebyshev - 1, numChebyshev);
        // We've grabbed the first and last as cA and cB -- reset the integer limits for evaluation calls.
        iStart = 1;
        iEnd = numChebyshev - 1;
        }
    for (size_t i = 0; i < numCopyLeft; i++)
        dest.push_back (cA);
    for (size_t i = iStart; i < iEnd; i++)
        dest.push_back (ChebyshevPoint (i, numChebyshev));
    for (size_t i = 0; i < numCopyRight; i++)
        dest.push_back (cB);
    }

void DoubleOps::LinearMapInPlace (bvector<double> &data, double a, double b)
    {
    for (auto &x : data)
        x = a + b * x;
    }

void DoubleOps::LinearMapFrontBackToInterval (bvector<double> &data, double a0, double a1)
    {
    if (data.empty ())
        return;
    // y = a0 + (x-x0) * (a1-a0)/(x1-x0)
    double f;
    double x0 = data.front ();
    DoubleOps::SafeDivide (f, a1-a0, data.back () - data.front (), 1.0);
    for (auto &x : data)
        x = a0 + (x- x0) * f;
    }


void DoubleOps::MinusOnePlus1LinearMapInPlace (bvector<double> &data, double aMinus, double aPlus)
    {
    LinearMapInPlace (data, 0.5 * (aMinus + aPlus), 0.5 * (aPlus - aMinus));
    }

bool DoubleOps::Normalize (bvector<double> &data, double a0, double a1)
    {
    double b;
    if (!DoubleOps::SafeDivide (b, 1.0, a1 - a0, 0.0))
        return false;
    for (size_t i = 0; i < data.size (); i++)
        data[i] = (data[i] - a0) * b;
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_ascendDoubles                                      |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_ascendDoubles

(
const double *num1,         /* => Doubles to compare */
const double *num2
)
    {
    if (*num1 < *num2)
        return (-1);
    else if (*num1 > *num2)
        return (1);
    else
        return (0);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          util_descendDoubles                                     |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_descendDoubles

(
const double    *num1,      /* => Doubles to compare */
const double    *num2
)
    {
    if (*num1 > *num2)
        return (-1);
    else if (*num1 < *num2)
        return (1);
    else
        return (0);
    }

/*---------------------------------------------------------------------------------**//**
*
* @param pDest      <=> array of doubles
* @param num    => number of doubles
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDoubleArray_sort

(
double *doubles,        /* <=> array of Doubles to be sorted (in place) */
int    numDoubles,      /* => number of Doubles to sort */
int    ascend           /* => true for ascending order */
)
    {
    qsort (doubles, numDoubles, sizeof(double),
                        ascend
                            ? (int (*)(const void *,const void *))util_ascendDoubles
                            : (int (*)(const void *,const void *))util_descendDoubles);
    }


END_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static const double s_nullRange_low = DBL_MAX;
static const double s_nullRange_high = -DBL_MAX;

//! @description constructor for a NULL range.
DRange1d::DRange1d ()
    {
    InitNull ();
    }

void DRange1d::InitNull ()
    {
    low  = s_nullRange_low;
    high = s_nullRange_high;
    }

//! @description Explicit constructor.  Values are NOT checked.
DRange1d::DRange1d (double _low, double _high)
    :   low  (_low),
        high (_high)
    {
    }

//! @description Return a range with explicit (possibly reversed) low and high.
DRange1d DRange1d::FromLowHigh (double _low, double _high)
    {
    return DRange1d (_low, _high);
    }


//! @description Return a range with explicit (possibly reversed) low and high.
DRange1d DRange1d::FromAltitudes (bvector<DPoint3d> &points, DPlane3dCR plane)
    {
    DRange1d range = DRange1d::NullRange ();
    for (size_t i = 0, n = points.size (); i < n; i++)
        range.Extend (plane.Evaluate (points[i]));
    return range;
    }

//! @description Return a range with explicit (possibly reversed) low and high.
DRange1d DRange1d::FromAltitudes (DPoint3dCP points, size_t n, DPlane3dCR plane)
    {
    DRange1d range = DRange1d::NullRange ();
    for (size_t i = 0; i < n; i++)
        range.Extend (plane.Evaluate (points[i]));
    return range;
    }

//! @description copy constructor.
DRange1d::DRange1d (DRange1dCR source)
    {
    low = source.low;
    high = source.high;
    }

//! @description Return a range which satisfies IsNull ()
DRange1d DRange1d::NullRange ()
    {
    return DRange1d ();
    }
DRange1d DRange1d::InfiniteRange () {return DRange1d (-DBL_MAX, DBL_MAX);}
DRange1d DRange1d::ZeroAndPositiveRange () {return DRange1d ( 0.0, DBL_MAX);}
DRange1d DRange1d::ZeroAndNegativeRange () {return DRange1d (-DBL_MAX, 0.0);}

bool DRange1d::IsInfinitePositive () const {return high == DBL_MAX;}
bool DRange1d::IsInfiniteNegative () const {return low == -DBL_MAX;}
bool DRange1d::IsDoublyInfinite () const {return IsInfinitePositive () && IsInfiniteNegative ();}

//! @description Return a range containing a single value.
DRange1d DRange1d::From (double value)
    {
    return DRange1d (value, value);
    }

//! @description Return a (sorted) range of 2 values.
DRange1d DRange1d::From (double valueA, double valueB)
    {
    if (valueA >= valueB)
        return DRange1d (valueB, valueA);
    else
        return DRange1d (valueA, valueB);
    }

//! @description Return a (sorted) range of 3 values.
DRange1d DRange1d::From (double valueA, double valueB, double valueC)
    {
    double max = valueA;
    if (valueB > max)
        max = valueB;
    if (valueC > max)
        max = valueC;
    double min = valueA;
    if (valueB < min)
        min = valueB;
    if (valueC < min)
        min = valueC;
    return DRange1d (min, max);
    }

//! @description Return a (sorted) range of an array of values.
DRange1d DRange1d::From (double *values, size_t n)
    {
    DRange1d range = DRange1d::NullRange ();
    range.Extend (values, n);
    return range;
    }

//! @description Test if the range is exactly the same as the null ranges returned
//! by DRange1d::NullRange ().
bool DRange1d::IsNull () const
    {
    return      low == s_nullRange_low
           &&   high == s_nullRange_high;
    }


//! @description Return {MAX(0, high - low)}
//! The DRange1::NullRange returns 0.
//! A single-point interval returns 0.
//! An interval with {high < low} returns 0.
//! Normal case returns {high - low}
double GEOMDLLIMPEXP DRange1d::Length () const
    {
    if (high > low)
        return high - low;
    return 0.0;
    }

//! @description Test if the range contains a given value.
bool DRange1d::Contains (double a) const
    {
    return a >= low && a <= high;
    }

//! @description Extend to include a value.
void DRange1d::Extend (double valueA)
    {
    if (high < low)
        {
        high = low = valueA;
        }
    else
        {
        if (valueA < low)
            low = valueA;
        if (valueA > high)
            high = valueA; 
        }
    }
//! @description Extend to include 2 values.
void DRange1d::Extend (double valueA, double valueB)
    {
    Extend (valueA);
    Extend (valueB);
    }

//! @description Extend to include array of values.
void DRange1d::Extend (double *data, size_t n)
    {
    for (size_t i = 0; i < n; i++)
        Extend (data[i]);
    }

//! @description Extend to include array of values.
void DRange1d::Extend (bvector <double> &data)
    {
    size_t n = data.size ();
    for (size_t i = 0; i < n; i++)
        Extend (data[i]);
    }


//! @description Extend to include (union) another (possibly null!!) range.
void DRange1d::Extend (DRange1dCR other)
    {
    if (!other.IsEmpty ())
        {
        Extend (other.low);
        Extend (other.high);
        }
    }


//! @description Restrict to overlap with another (possibly null!!) range.
void DRange1d::IntersectInPlace (DRange1dCR other)
    {
    if (other.low > low)
        low = other.low;
    if (other.high < high)
        high = other.high;
    if (high < low)
        InitNull ();
    }

//! @description return the (possibly null) intersection of two ranges.
DRange1d DRange1d::FromIntersection (DRange1dCR rangeA, DRange1dCR rangeB)
    {
    double aLow, aHigh, bLow, bHigh;
    if (rangeA.GetLowHigh (aLow, aHigh)
        && rangeB.GetLowHigh (bLow, bHigh))
        {
        double low = aLow   < bLow  ? bLow : aLow;
        double high = aHigh < bHigh ? aHigh : bHigh;
        if (low <= high)
            return DRange1d (low, high);
        }
    return NullRange ();
    }

//! @description Compute intersection of the instance with rangeB.  Return the intersection
//!    as fractions of the instance.
//! @return true if the fractional intersection is more than single point.  In false case, fractionalIntersection
//!     always null range -- no effort to catch single point case.
bool DRange1d::StrictlyNonEmptyFractionalIntersection (DRange1dCR other, DRange1dR fractionalIntersection)
    {
    double newLow = low < other.low ? other.low : low;
    double newHigh = high > other.high ? other.high : high;
    if (newLow < newHigh)
        {
        // Strict less than makes division safe. (high - low is at least as large as newHigh - newLow)
        double a = 1.0 / (high - low);
        fractionalIntersection = DRange1d ( (newLow - low) * a, (newHigh - low) * a);
        return true;
        }
    fractionalIntersection.InitNull ();
    return false;
    }

bool DRange1d::GetLowHigh (double &a, double &b) const
    {
    if (IsEmpty ())
        return false;
    if (low < high)
        {
        a = low;
        b = high;
        }
    else
        {
        a = high;
        b = low;
        }
    return true;
    }

double DRange1d::Low ()  const{return low;}
double DRange1d::High () const{return high;}

//! @description return the (possibly null) union of two ranges.
DRange1d DRange1d::FromUnion (DRange1dCR rangeA, DRange1dCR rangeB)
    {
    DRange1d range = rangeA;
    if (!rangeB.IsNull ())
        range.Extend (rangeB.low, rangeB.high);
    return range;
    }

//! @description Test if the range has {low < high}, i.e. there are no x for which {low <= x && x <= high}
bool GEOMDLLIMPEXP DRange1d::IsEmpty () const
    {
    return low > high;
    }

//! @description Test if the range has {high >= low}, i.e. is empty or just one point.
bool GEOMDLLIMPEXP DRange1d::IsEmptyOrSinglePoint() const
    {
    return low >= high;
    }

//! @description Test if the range is a single point.
bool GEOMDLLIMPEXP DRange1d::IsSinglePoint () const
    {
    return high == low;
    }

//! @description Test if the range is a single point.
bool GEOMDLLIMPEXP DRange1d::IsSinglePoint (double value) const
    {
    return high == value && low == value;
    }

//! @description Test if the range has {high > low}, i.e. has a non-empty set of points with properly sorted lower and upper limit.
bool GEOMDLLIMPEXP DRange1d::IsPositiveLength () const
    {
    return high > low;
    }


//! @description Test if the instance range is a (possibly complete, possibly empty) subset of {other} range.
bool DRange1d::IsSubsetOf (DRange1dCR other) const
    {
    if (high < low)    // empty set is subset of everything
        return true;
    return low >= other.low && high <= other.high;
    }

//! @description Test if the instance range is a (possibly complete, but not empty) subset of {other} range.
bool DRange1d::HasNonEmptyIntersectionWith (DRange1dCR other) const
    {
    return high >= low && (low <= other.high && high <= other.low);
    }

//! @description Test if the instance range has a nonzero-length (more than single point) intersection with {other}.
bool DRange1d::HasPositiveLengthIntersectionWith (DRange1dCR other) const
    {
    return high > low && low < other.high && high > other.low;
    }


//! @description return the largest coordinate in the range.
double DRange1d::MaxAbs (double defaultValueForNullRange) const
    {
    if (this->IsNull ())
        return defaultValueForNullRange;
    double a = fabs (low);
    double b = fabs (high);
    return a > b ? a : b;
    }

//! @description Test if equal intervals in point set sense. 
//! Any pair of empty intervals (even if different low and high) are equal.
bool DRange1d::IsEqualInterval (DRange1dCR other) const
    {
    if (low <= high)
        return low == other.low && high == other.high;
    return other.low > other.high;
    }

//! @description Direct equality test for low and high parts.
bool DRange1d::IsEqualLowHigh (DRange1dCR other) const
    {
    return low == other.low && high == other.high;
    }

//! @description Toleranced equality test for low and high parts.
bool DRange1d::IsSameMinMax(DRange1dCR other, double absTol) const
    {
    if (low <= high && other.low <= other.high)
        return fabs (low-other.low) <= absTol && fabs (high-other.high) <= absTol;
    return false;
    }
//! If non empty, shift endpoints by (-tol, +tol).
//! No change if empty !!!
void DRange1d::ExtendBySignedShift (double tol)
    {
    if (high >= low)
        {
        high += tol;
        low  -= tol;
        }
        
    }


//! @description map fractional coordinate to real. Returns false if {IsEmpty()}
bool DRange1d::FractionToDouble (double fraction, double &x, double defaultReturnX) const
    {
    if (IsEmpty ())
        {
        x = defaultReturnX;
        return false;
        }
    x = low + fraction * (high - low);
    return true;
    }

//! @description map real to fraction. Returns false if {IsEmptyOrSinglePoint ()}
bool DRange1d::DoubleToFraction (double x, double &fraction, double defaultReturnX) const
    {
    if (IsEmptyOrSinglePoint ())
        {
        fraction = defaultReturnX;
        return false;
        }
    fraction = (x - low) / (high - low);
    return true;
    }

//! @description return a tolerance computed as
//!    {absTol + localRelTol * Extent() + globalRelTol * MaxAbs ()}
double DRange1d::GetTolerance (double absTol, double localRelTol, double globalRelTol)
    {
    double tol = absTol;
    if (IsNull())
        return tol;
    if (localRelTol > 0.0)
        tol += localRelTol * fabs (high - low);
    if (globalRelTol > 0.0)
        tol += globalRelTol * MaxAbs ();
    return tol;
    }

static bool cb_compareForSortLow (DRange1dCR pointA, DRange1dCR pointB)
    {
    if (pointA.low < pointB.low)
        return true;
    if (pointA.low > pointB.low)
        return false;
    return pointA.high < pointB.high;
    }

//! @description Sort on low values.
void DRange1d::SortLowInPlace (bvector<DRange1d> &data)
    {
    std::sort (data.begin (), data.end (), cb_compareForSortLow);
    }

//! @description Combine intervals so there are no overlaps.
void DRange1d::SimplifyInPlace (bvector<DRange1d> &data)
    {
    SortLowInPlace (data);
    size_t n = data.size ();
    for (size_t i = 0; i < n; i++)
        {
        double b = data[i].high;
        size_t j = i;
        while (j + 1 < n && data[j + 1].low <= b)
            {
            if (data[j+1].high > b)
                b = data[j+1].high;
            j++;
            }
        data[i].high = b;
        i = j + 1;
        }
    }

//! @description Return the encompassing single range.
DRange1d DRange1d::FromExtent (bvector <DRange1d> &data)
    {
    DRange1d range;
    range.InitNull ();
    for (size_t i = 0, n = data.size (); i < n; i++)
        {
        if (data[i].high >= data[i].low)
            {
            range.Extend (data[i].high);
            range.Extend (data[i].low);
            }
        }
    return range;
    }

//! @description Return the overall intersection.
DRange1d DRange1d::FromIntersection (bvector <DRange1d> &data)
    {
    size_t n = data.size ();
    if (n == 0)
        return NullRange ();
    DRange1d range = data[0];
    for (size_t i = 0; i < n && range.high >= range.low; i++)
        {
        range.IntersectInPlace (data[i]);
        }
    return range;
    }

//! @description Intersect each interval in dataA with the single range dataB.  Append non-empty results to dataOut.
void DRange1d::AppendClips (bvector <DRange1d> &dataA, DRange1dCR clipper, bvector<DRange1d> &dataOut)
    {
    if (clipper.IsEmpty ())
        return;
    for (size_t i = 0, n = dataA.size (); i < n; i++)
        {
        DRange1d result = FromIntersection (dataA[i], clipper);
        if (!result.IsEmpty ())
            dataOut.push_back (result);
        }
    }

//! @description Intersect each interval in dataA with the single range dataB.  Append non-empty results to dataOut.
void DRange1d::ClipInPlace (bvector <DRange1d> &dataA, DRange1dCR clipper)
    {
    size_t numOut = 0;
    if (!clipper.IsEmpty ())
        {
        for (size_t i = 0, n = dataA.size (); i < n; i++)
            {
            DRange1d result = FromIntersection (dataA[i], clipper);
            if (!result.IsEmpty ())
                dataA[numOut++] = result;
            }
        }
    dataA.resize (numOut);
    }


// if [max(a0, a1), b] is non-empty, push it on dataOut.
// return b
static void appendInterval (double a, double b, bvector<DRange1d> &dataOut)
    {
    if (a < b)
        {
        if (dataOut.size () > 0 && dataOut.back ().high >= a)
            dataOut.back ().Extend (b);
        else
            dataOut.push_back (DRange1d::From (a,b));
        }
    }
enum class PlaceInRangeArray
    {
    RIGHT_OF_ALL = 0,
    INTERIOR = 1,
    GAP_BEFORE = 2
    };
struct RangeWalker {
    bvector<DRange1d> &m_data;
    PlaceInRangeArray position;
    size_t index;
    // The next x that will be encountered moving left to right . .
    // In "no more points" state this is DBL_MAX
    double x;
    RangeWalker(bvector<DRange1d> &data) : m_data(data)
        {
        SetGapBefore (0);
        }

    void SetRightOfAll ()
        {
        position = PlaceInRangeArray::RIGHT_OF_ALL;
        x = DBL_MAX;
        index = m_data.size ();
        }
    void SetInterior(size_t k)
        {
        if (k < m_data.size ())
            {
            index = k;
            x = m_data[k].high;
            position = PlaceInRangeArray::INTERIOR;
            }
        else
            SetRightOfAll ();
        }
    void SetGapBefore(size_t k)
        {
        if (k < m_data.size())
            {
            index = k;
            x = m_data[k].low;
            position = PlaceInRangeArray::GAP_BEFORE;
            }
        else
            SetRightOfAll();
        }
    // Return true if the walker is positioned within an interval .
    bool IsInterior () { return position == PlaceInRangeArray::INTERIOR;}
    // Return true if the walker has one or more points available (including the current point)
    bool HasMorePoints (){ return position != PlaceInRangeArray::RIGHT_OF_ALL;}
    void Advance()
        {
        switch (position)
            {
            case PlaceInRangeArray::GAP_BEFORE:
                {
                SetInterior (index);
                break;
                }
            case PlaceInRangeArray::INTERIOR:
                {
                index++;
                SetGapBefore (index);
                break;
                }
            case PlaceInRangeArray::RIGHT_OF_ALL:
                {
                break;
                }
            }
        }
    };

//! @description return classification of all overlap intervals.  Output may NOT be the same as either input.
void ClassifySorted(bool (*classify) (bool a, bool b), bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut)
    {
    dataOut.clear();
    RangeWalker walkerA (dataA);
    RangeWalker walkerB(dataB);
    double low, high;
    if (walkerA.x <= walkerB.x)
        {
        low = walkerA.x;
        walkerA.Advance ();
        }
    else
        {
        low = walkerB.x;
        walkerB.Advance ();
        }

    while (walkerA.HasMorePoints () || walkerB.HasMorePoints())
        {
        bool accept = classify (walkerA.IsInterior(), walkerB.IsInterior());
        if (walkerA.x <= walkerB.x)
            {
            high = walkerA.x;
            walkerA.Advance ();
            }
        else
            {
            high = walkerB.x;
            walkerB.Advance();
            }
        if (accept)
            appendInterval (low, high, dataOut);
        low = high;
        }
    }

void DRange1d::ParitySorted(bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut)
    {
    ClassifySorted (DRange1d::BooleanXOR, dataA, dataB, dataOut);
    }

void DRange1d::IntersectSorted(bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut)
    {
    ClassifySorted(DRange1d::BooleanAND, dataA, dataB, dataOut);
    }

void DRange1d::UnionSorted(bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut)
    {
    ClassifySorted(DRange1d::BooleanOR, dataA, dataB, dataOut);
    }

void DRange1d::DifferenceSorted(bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut)
    {
    ClassifySorted(DRange1d::BooleanANotB, dataA, dataB, dataOut);
    }

bool DRange1d::IsIncreasing (bvector<DRange1d> &data, bool allowZeroLength, bool allowZeroGap)
    {
    size_t n = data.size ();
    double a;
    for (size_t i = 0; i < n; i++)
        {
        if (i > 0)
            {
            a = data[i].low - data[i-1].high;
            if (a < 0.0)
                return false;
            if (!allowZeroGap && a == 0.0)
                return false;
            }
        a = data[i].high - data[i].low;
        if (a < 0.0)
            return false;
        if (!allowZeroLength && a == 0.0)
            return false;
        }
    return true;
    }

//! @description Sum the interval lengths
double DRange1d::LengthSum (bvector <DRange1d> &data)
    {
    double s = 0.0;
    for (size_t i = 0, n = data.size (); i < n; i++)
        s += data[i].Length ();
    return s;
    }


//! Update a bounding interval of a line based on the variation of that line within one dimension.
//! The ray parameterized {x = x0+s*dxdx}
//! The rayInterval is an {s} range that is live.  This is usually initialized to InfiniteRange before clipping.
//! {xA,xB} is an interval of the {x} space.  (xA and xB are not required to be sorted.}
//! @param [in,out] rayInterval the live parameter interval of the ray.
//! @param [in] x0 ray start coordinate in this dimension
//! @param [in] dxds Rate of change of ray in this dimension.
//! @param [in] xA x limit
//! @param [in] xB x limit.
bool DRange1d::UpdateRay1dIntersection
(
double  x0,
double  dxds,
double xA,
double xB
)
    {
    double s0, s1;
    if (IsEmpty ())
        return false;

    if (dxds != 0.0)
        {
        s0 = (xA  - x0) / dxds;
        s1 = (xB - x0) / dxds;
        if (s1 < s0)
            {
            double temp = s0;
            s0 = s1;
            s1 = temp;
            }

        if (s0 > low)
            low = s0;
        if (s1 < high)
            high = s1;
        }
    else
        {
        if ((x0 - xA) * (x0 - xB) > 0.0)
            {
            InitNull ();
            }
        }

    if (IsEmpty ())
        {
        InitNull ();
        return false;
        }
    return true;
    }

ValidatedDouble DRange1d::FractionToDouble (double fraction) const
    {
    if (IsEmpty ())
        return ValidatedDouble (0.0, false);
    return ValidatedDouble (DoubleOps::Interpolate (low, fraction, high), true);
    }

bool DRange1d::BooleanAND(bool a, bool b) { return a && b;}
//! function (typically passed as a parameter) for boolean "OR" of 2 parameters.
bool DRange1d::BooleanOR(bool a, bool b) { return a || b; }
//! function (typically passed as a parameter) for boolean "XOR" of 2 parameters.
bool DRange1d::BooleanXOR(bool a, bool b) { return a != b; }
//! function (typically passed as a parameter) for boolean "ANotB" of 2 parameters.
bool DRange1d::BooleanANotB(bool a, bool b) { return a && !b;}
//! @description return true if any of the ranges in the array contains x
bool DRange1d::AnyRangeContains(bvector<DRange1d> const &data, double x)
    {
    for (auto &range : data)
        {
        if (range.Contains (x))
            return true;
        }
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE

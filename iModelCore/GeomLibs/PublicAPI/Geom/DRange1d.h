/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/DRange1d.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/**
A 1d interval with low and high values.

Useful typedefs for DRange1d
\code
    typdedef struct const &DRange1d DRange1dCR;
    typdedef struct &DRange1d DRange1dR;
    typdedef struct const *DRange1d DRange1dCP;
    typdedef struct *DRange1d DRange1dP;
\endcode


 If you look closely at "names" for special intervals, there are several tricky distinctions:
 <ul>
 <li>{high > low} The "usual" interval with a strictly positive length.  You can test for this with {IsPositiveLength ()}
 <li>{high == low} A single point interval.  This has something int it (one point!) and is NOT empty.   You can test for this with {IsSinglePoint()} 
 <li>{high < low} The set of points x with {low < x < high} is empty. You can test for this with {IsEmpty()}
 <li>The initialization value NullInterval -- this is a unique value. You can test for this with {IsNull()}   The NullInterval is empty, but not all empty intervals are
        equal to the NullInterval if you compare the low and high values!!
 </ul>

 Be aware that if the application needs to treat a single point interval as "empty" for its purposes, the
 use of "empty" will be what the methods call {IsEmptyOrSinglePoint()}

@ingroup GROUP_Geometry
*/
struct DRange1d
{

//! low point of box
double low;
//! high point of box
double high;

#ifdef __cplusplus
    
public:
//! @description constructor for a NULL range.
GEOMDLLIMPEXP DRange1d ();

//! @description Constructor from explicit low and high.
GEOMDLLIMPEXP DRange1d (double low, double high);

//! @description Return a range with explicit (possibly reversed) low and high.
static GEOMDLLIMPEXP DRange1d FromLowHigh (double low, double high);

//! @description copy constructor.
GEOMDLLIMPEXP DRange1d (DRange1dCR source);

//! @description Return a range which satisfies IsNull ()
static GEOMDLLIMPEXP DRange1d NullRange ();

//! @description Return a complete range (-DBL_MAX to DBL_MAX)
static GEOMDLLIMPEXP DRange1d InfiniteRange ();
//! @description Return a complete 0 and positive range (0 to DBL_MAX)
static GEOMDLLIMPEXP DRange1d ZeroAndPositiveRange ();
//! @description Return a complete 0 and negative range (-DBL_MAX to 0)
static GEOMDLLIMPEXP DRange1d ZeroAndNegativeRange ();


//! @description Set this range to the {NullRange}
GEOMDLLIMPEXP void InitNull ();

//! @description Return a range containing a single value.
static GEOMDLLIMPEXP DRange1d From (double value);

//! @description Return a (sorted) range of 2 values.
static GEOMDLLIMPEXP DRange1d From (double valueA, double valueB);

//! @description Return a (sorted) range of 3 values.
static GEOMDLLIMPEXP DRange1d From (double valueA, double valueB, double valueC);

//! @description Return a (sorted) range of an array of values.
static GEOMDLLIMPEXP DRange1d From (double *values, size_t n);

//! @description Return a (sorted) range of altitudes of points.
static GEOMDLLIMPEXP DRange1d FromAltitudes (bvector<DPoint3d> &points, DPlane3dCR plane);

//! @description Return a (sorted) range of altitudes of points.
static GEOMDLLIMPEXP DRange1d FromAltitudes (DPoint3dCP points, size_t n, DPlane3dCR plane);

//! @description Test if the range is exactly the same as the null ranges returned by NullRange ().
bool GEOMDLLIMPEXP IsNull () const;

//! @description Test if the range has {low > high}, i.e. there are no x for which {low <= x && x <= high}
bool GEOMDLLIMPEXP IsEmpty () const;

//! @description Test if the range has {high >= low}, i.e. is empty or just one point.
bool GEOMDLLIMPEXP IsEmptyOrSinglePoint () const;

//! @description Test if the range is a single point.
bool GEOMDLLIMPEXP IsSinglePoint () const;

//! @description Test if the range is a particular single point
bool GEOMDLLIMPEXP IsSinglePoint (double value) const;
//! @description Test if the range has {high > low}, i.e. has a non-empty set of points with properly sorted lower and upper limit.
bool GEOMDLLIMPEXP IsPositiveLength () const;

//! @description Test if the range has the largest possible {high}
bool GEOMDLLIMPEXP IsInfinitePositive () const;

//! @description Test if the range has the most negative possible {low}
bool GEOMDLLIMPEXP IsInfiniteNegative () const;

//! @description Test if the range has the most negative {low} and most positive {high}
bool GEOMDLLIMPEXP IsDoublyInfinite () const;

//! Get the low and high limits (unchecked).
//! @return true if {b >= a}, i.e. the interval is single point or postive length.
//! (i.e. any empty interval returns false, with whatever limit data is present)
bool GEOMDLLIMPEXP GetLowHigh (double &a, double &b) const;

//! Get the low limit (unchecked).
double GEOMDLLIMPEXP Low () const;
//! Get the low limit (unchecked).
double GEOMDLLIMPEXP High () const;

//! @description Return {MAX(0, high - low)}
//! The DRange1::NullRange returns 0.
//! A single-point interval returns 0.
//! An interval with {high < low} returns 0.
//! Normal case returns {high - low}
double GEOMDLLIMPEXP Length () const;

//! @description Test if the range contains a given value.
bool GEOMDLLIMPEXP Contains (double a) const;

//! @description Extend to include a value.
void GEOMDLLIMPEXP Extend (double valueA);
//! @description Extend to include 2 values.
void GEOMDLLIMPEXP Extend (double valueA, double valueB);
//! @description Extend to include an array of values.
void GEOMDLLIMPEXP Extend (double *values, size_t count);

//! @description Extend to include an array of values.
void GEOMDLLIMPEXP Extend (bvector<double> &values);

//! @description Extend to include (union) another (possibly null!!) range.
void GEOMDLLIMPEXP Extend(DRange1dCR other);


//! If non empty, shift endpoints by (-tol, +tol).
//! No change if empty !!!
void GEOMDLLIMPEXP ExtendBySignedShift (double tol);

//! @description Restrict to overlap with another (possibly null!!) range.
void GEOMDLLIMPEXP IntersectInPlace (DRange1dCR other);

//! @description return the (possibly null) intersection of two ranges.
static GEOMDLLIMPEXP DRange1d FromIntersection (DRange1dCR rangeA, DRange1dCR rangeB);
//! @description return the (possibly null) union of two ranges.
static GEOMDLLIMPEXP DRange1d FromUnion (DRange1dCR rangeA, DRange1dCR rangeB);

//! @description Compute intersection of the instance with rangeB.  Return the intersection
//!    as fractions of the instance.
//! @return true if the fractional intersection is more than single point.
bool GEOMDLLIMPEXP StrictlyNonEmptyFractionalIntersection (DRange1dCR rangeB, DRange1dR fractionalIntersection);

//! @description Test if the instance range is a (possibly complete, possibly empty) subset of {other} range.
bool GEOMDLLIMPEXP IsSubsetOf (DRange1dCR other) const;
//! @description Test if the instance range is a (possibly complete, but not empty) subset of {other} range.
bool GEOMDLLIMPEXP HasNonEmptyIntersectionWith (DRange1dCR other) const;
//! @description Test if the instance range has a positive-length (more than single point) intersection with {other}.
bool GEOMDLLIMPEXP HasPositiveLengthIntersectionWith (DRange1dCR other) const;

//! @description Test if the instnace range is a (possibly complete) subset of {other} range.
bool GEOMDLLIMPEXP IsContainedIn (DRange1dCR other) const;
//! @description return the largest coordinate (absolute value) in the range.
double GEOMDLLIMPEXP MaxAbs (double defaultValeForNullRange = 0.0) const;

//! @description Test if equal intervals in point set sense. 
//! Any pair of empty intervals (even if different low and high) are equal.
bool GEOMDLLIMPEXP IsEqualInterval (DRange1dCR other) const;

//! @description Direct equality test for low and high parts.
bool GEOMDLLIMPEXP IsEqualLowHigh (DRange1dCR other) const;

//! @description test for toleranced equality of (min max of) low and high.
bool GEOMDLLIMPEXP IsSameMinMax (DRange1dCR other, double absTol) const;
//! @description test for toleranced equality of low to low and high to high 
//  (i.e. two ranges with reversed low and high return false here, true for {IsSameMinMax}
bool GEOMDLLIMPEXP IsSameLowHigh (DRange1dCR other, double relTol) const;

//! @description map fractional coordinate to real. Returns false if null range.
bool GEOMDLLIMPEXP FractionToDouble (double fraction, double &x, double defaultReturnX = 0.0) const;
//! @description map real to fraction. Returns false if null range or single point.
bool GEOMDLLIMPEXP DoubleToFraction (double x, double &fraction, double defaultReturnX = 0.0) const;
//! map fractional coordinate to double.  Indicate invalid if the range is empty.
ValidatedDouble GEOMDLLIMPEXP FractionToDouble (double fraction) const;

//! @description return a tolerance computed as
//!    {absTol + localRelTol * Extent() + globalRelTol * MaxAbs ()}
double GEOMDLLIMPEXP GetTolerance (double absTol = 1.0e-14, double localRelTol = 1.0e-14, double globalRelTol = 0.0);

//! @description Sort on low values.
static GEOMDLLIMPEXP void SortLowInPlace (bvector<DRange1d> &data);
//! @description Combine intervals so there are no overlaps.
static GEOMDLLIMPEXP void SimplifyInPlace (bvector<DRange1d> &data);

//! @description Return the encompassing single range.
static GEOMDLLIMPEXP DRange1d FromExtent (bvector <DRange1d> &data);
//! @description Return the overall intersection.
static GEOMDLLIMPEXP DRange1d FromIntersection (bvector <DRange1d> &data);

//! @description Intersect each range in dataA with clipper.  Append non-empty results to dataOut.
//! Note that if the same array is passed for dataA and dataOut, all input contents will remain.  (Use {ClipInPlace} to get compaction)
static GEOMDLLIMPEXP void AppendClips (bvector <DRange1d> &dataA, DRange1dCR clipper, bvector<DRange1d> &dataOut);

//! @description Intersect each range in dataA with clipper.  Retain non-empty result parts.
static GEOMDLLIMPEXP void ClipInPlace (bvector <DRange1d> &dataA, DRange1dCR clipper);

//! @description Intersect intervals in two pre-sorted sets.  Output may NOT be the same as either input.
static GEOMDLLIMPEXP void IntersectSorted (bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut);

//! @description Union of intervals in two pre-sorted sets.  Output may NOT be the same as either input.
static GEOMDLLIMPEXP void UnionSorted (bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut);

//! @description Intersect intervals in two pre-sorted sets.  Output may NOT be the same as either input.
static GEOMDLLIMPEXP void DifferenceSorted (bvector <DRange1d> &dataA, bvector <DRange1d> &dataB, bvector <DRange1d> &dataOut);

//! @description Test for increasing relationships, optionally allowing zero length.
static GEOMDLLIMPEXP bool IsIncreasing (bvector <DRange1d> &data, bool allowZeroLength = false, bool allowZeroGaps = false);

//! @description Sum the interval lengths
static GEOMDLLIMPEXP double LengthSum (bvector <DRange1d> &data);

//! Update a bounding interval of a line based on the variation of that line within one dimension.
//! The ray parameterized {x = x0+s*dxdx}
//! The instance is an {s} range that is live.  This is usually initialized to InfiniteRange before clipping.
//! {xA,xB} is an interval of the {x} space.  (xA and xB are not required to be sorted.}
//! @param [in] x0 ray start coordinate in this dimension
//! @param [in] dxds Rate of change of ray in this dimension.
//! @param [in] xA x limit
//! @param [in] xB x limit.
//! @return true if interval is nonempty at return.
bool GEOMDLLIMPEXP UpdateRay1dIntersection
(
double  x0,
double  dxds,
double xA,
double xB
);



#endif

};
END_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/DPoint3dOps.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <Bentley/BeTimeUtilities.h>
//! @file  DPoint3dOps.h Class wrapper for static utiliity functions on data structures built around DPoint2d: VectorOps, DPoint2dOps, DVec3dOps, DVec2dOps, DoubleOps, DPoint3dOps, and PolylineOps
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// To be included from GeomApi.h !!!
//!
//! @description class wrapper for static utiliity functions on data structures built around DPoint2d.
//!
template <typename T>
//! Templated base class for operations on arrays of DPoint2d, DPoint3d, DVec2d, DVec3d double.
//!  (This class cannot be instantiated outside the geometry library -- the derived class implementations are exported.)
//! @ingroup BentleyGeom_Operations
struct VectorOps
{
//!
//! @description Copy from source to destination. destination is cleared first. Either array may be null.
//!
public:
static GEOMDLLIMPEXP void Copy (bvector<T> *dest, bvector<T> const *source);

static GEOMDLLIMPEXP void Copy (bvector<T> *dest, T const *source, size_t count);

//! append contiguous points to the bvector<T>.  return first index 

static GEOMDLLIMPEXP size_t Append (bvector<T> *dest, T const *source, size_t count);

//! append source to dest, return return first index 

static GEOMDLLIMPEXP size_t Append (bvector<T> *dest, bvector<T> const *source);

//! size with pointer check 

static GEOMDLLIMPEXP size_t Size (bvector<T> *dest);

//! Add with checked array pointer. Return its index.

static GEOMDLLIMPEXP size_t Append (bvector<T> *dest, T const &data);

//! append a disconnect.
static GEOMDLLIMPEXP void AppendDisconnect (bvector<T> *dest);

//! Append an exact closure point (if not already present)
// If already closed within tolerance, enforce exact-bitwise equality
static GEOMDLLIMPEXP void AppendClosure (bvector<T> &dest, double tolerance = 1.0e-14);

//! Set with checked array pointer and index 
static GEOMDLLIMPEXP bool Set (bvector<T> *dest, T const &data, size_t index);

//! Get with checked array pointer and index 
static GEOMDLLIMPEXP bool Get (bvector<T> *dest, T &data, size_t index);

//! Add uniformly spaced interpolated points between limits.
//! @param [inout] dest vector to receive values
//! @param [in] first first value.  If {includeFirst} is true, this point is added explicitly.
//! @param [in] last last value
//! @param [in] count number of values to add AFTER the optional first point.
//! @param [in] includeFirst true to include the first point directly.
//! @remark When inserting points  on segments of a polyline, settig includeFirst false is useful to prevent replication of intermediate vertices.
static GEOMDLLIMPEXP void AppendInterpolated (bvector<T> &dest, T const &first, T const &last, size_t count, bool includeFirst = true);

//! Interpolate between T values in dataA and dataB.
//! @param [inout] dest returned with min(dataA.size{}, dataB.size()) interpolated values.
//! @param [in] dataA first input array
//! @param [in] f interpolation fraction
//! @param [in] dataB second input array
static GEOMDLLIMPEXP void InterpolateAll (bvector<T> &dest, bvector <T> const &dataA, double f, bvector <T> const &dataB);

//! Copy data to the heap as allocated by BSIBaseGeom::Malloc.
//! @param [in] source source data.
//! @return NULL if empty array, otherwise pointer to allocated and filled heap memory.
static GEOMDLLIMPEXP T * MallocAndCopy (bvector<T> &source);

//! Inplace removal of near-duplicate points.
//! @param [inout] data soure data.  count is reduced when points are eliminated.
//! @param [in] tolerance for comparison via AlmostEqual (a,b,tolerance)
static GEOMDLLIMPEXP void Compress (bvector<T> &data, double tolerance);

//! Inplace removal of near-duplicate points via Compress, followed by removal of trailing points that match source[0]
//! @param [inout] data soure data.
//! @param [in] tolerance for comparison via AlmostEqual (a,b,tolerance)
static GEOMDLLIMPEXP void CompressCyclic (bvector<T> &data, double tolerance);

//! Inplace removal of near-duplicate points.
//! @param [inout] source soure data.
//! @param [inout] dest array to be cleared and loaded.
//! @param [in] tolerance for comparison via AlmostEqual (a,b,tolerance)
static GEOMDLLIMPEXP void Compress (bvector<T> const &source, bvector<T> &dest, double tolerance);
//!
//! @description Reverse all points in the vector.
//!
public:
static GEOMDLLIMPEXP void Reverse (bvector<T>& xyz);

//! @description return the largest absolute coordinate in the array.
public: static double GEOMDLLIMPEXP LargestCoordinate (bvector<T> const &data);

//! @description return the largest absolute coordinate in the array.
public: static double GEOMDLLIMPEXP LargestCoordinate (T const *data, size_t count);

//!
//! @description return a tolerance as an absolute tolerance plus relative tolerance times largest coordinate.
//!
public: static double GEOMDLLIMPEXP Tolerance (bvector<T> const &data, double absTol, double relTol);

//!
//! @description return a tolerance as an absolute tolerance plus relative tolerance times largest coordinate.
//!
public: static double GEOMDLLIMPEXP Tolerance (T const *data, size_t count, double absTol, double relTol);

//! @description Test if all members are exactly equal.
public: static bool GEOMDLLIMPEXP Equal (bvector<T> const &dataA, bvector<T> const &dataB);

//! @description Test if all members are exactly equal.
public: static bool GEOMDLLIMPEXP Equal (T const *dataA, size_t countA, T const *dataB, size_t countB);

//! Search for [i0..] for index i1 at which the array value is not AlmostEqual to the given value.
public: static bool GEOMDLLIMPEXP FindNotAlmostEqualAtOrAfter (bvector<T>const &data, T const &baseValue, size_t i0, size_t &i1, T &value);
//! Search for [i0-1..(downward)] for index i1 at which the array value is not AlmostEqual to the given value.
public: static bool GEOMDLLIMPEXP FindNotAlmostEqualBefore (bvector<T>const &data, T const &baseValue, size_t i0, size_t &i1, T &value);

//! Near equality test with default Angle::SmallAngle absolute and relative tolerances.
public: static bool GEOMDLLIMPEXP AlmostEqual (T const &valueA, T const &valueB);



//! Near equality test.
//! @param [in] valueA first value
//! @param [in] valueB second value
//! @param [in] tolerance Allowable variation.  If zero or negative, default to AlmostEqual(valueA, valueB)
public: static bool GEOMDLLIMPEXP AlmostEqual (T const &valueA, T const &valueB, double tolerance);

//! Near equality test.
//! @param [in] valueA first array.
//! @param [in] numA first count
//! @param [in] valueB second array
//! @param [in] numB second count
//! @param [in] tolerance Allowable variation.  If zero or negative, default to AlmostEqual(valueA, valueB)
public: static bool GEOMDLLIMPEXP AlmostEqual
(
T const *valueA,
size_t numA,
T const *valueB,
size_t numB,
double tolerance);


//! Find the index of the value most distance from baseValue. Return SIZE_MAX if empty array.
public: static size_t GEOMDLLIMPEXP MostDistantIndex (bvector<T>const &data, T const &baseValue);
//! Find the index of the value most distance from baseValue. Return SIZE_MAX if empty array.
public: static size_t GEOMDLLIMPEXP MostDistantIndex (T const *data, size_t n, T const &baseValue);


public: static void ReverseArrayInPlace (T *data, int n)
    {
    for (int i = 0, j = n - 1; i < j; i++, j--)
        std::swap (data[i], data[j]);
    }
};

//! @ingroup BentleyGeom_Operations
//! @description Operations on arrays of DPoint2d (bvector and contiguous buffer).
struct DPoint2dOps : VectorOps <DPoint2d>
{
private: DPoint2dOps (){};
public:

//! append a point to the bvector<T> 

static GEOMDLLIMPEXP void AppendXY (bvector<DPoint2d> *dest, double x, double y);

//! Append xy parts.
static GEOMDLLIMPEXP void AppendXY (bvector<DPoint2d> &dest, bvector<DPoint3d> const &source);

//!
//! @description Find nearly-identical points.  Pack the coordinates down.
//! @param [in] xyzIn array of points.   Coordinates are shuffled and array resized.
//! @param [out] xyzOut optional array to receive packed points.  May NOT alias xyzIn.
//! @param [out] oldIndexToPackedIndex For each original index, indicates the point's position in the resized array.
//! @param [in] absTol optional absolute tolerance.   Negative value requests default.  Zero is an allowable non-default value.
//! @param [in] relTol optional relative tolerance.   Negative value requests default.  Zero is an allowable non-default value.
//!
public:
static GEOMDLLIMPEXP size_t Cluster
(
bvector<DPoint2d> const &xyzIn,
bvector<DPoint2d> *xyzOut,
bvector<size_t>&oldIndexToPackedIndex,
double absTol = -1.0,
double relTol = -1.0
);

//!
//! Multiply each (non-disconnect) point in place by a transform.
//! @param [out]  xyz array of points, may have disconnects
//! @param [in] transform  transform to apply.
//!
public:
static GEOMDLLIMPEXP void Multiply
(
bvector<DPoint2d> *xyz,
TransformCR transform
);

public:
//! Return indices where min and max y of lexical sort order occur.
static bool LexicalXExtrema (int &iMin, int &iMax, DPoint2dCP points, int nPoint);
public:
//! Return indices where min and max y of lexical sort order occur.
static bool LexicalYExtrema (int &iMin, int &iMax, DPoint2dCP points, int nPoint);
};

//! @ingroup BentleyGeom_Operations
//! @description Operations on arrays of DVec3d (bvector and contiguous buffer).
struct DVec3dOps : VectorOps <DVec3d>
{
private: DVec3dOps (){};
public:

//! append a point to the bvector<T> 

static GEOMDLLIMPEXP void AppendXYZ (bvector<DVec3d> *dest, double x, double y, double z = 0.0);


//!
//! @description Find nearly-identical points.  Pack the coordinates down.
//! @param [in] xyzIn array of points.   Coordinates are shuffled and array resized.
//! @param [out] xyzOut optional array to receive packed points.  May NOT alias xyzIn.
//! @param [out] oldIndexToPackedIndex For each original index, indicates the point's position in the resized array.
//! @param [in] absTol optional absolute tolerance.   Negative value requests default.  Zero is an allowable non-default value.
//! @param [in] relTol optional relative tolerance.   Negative value requests default.  Zero is an allowable non-default value.
//!
public:
static GEOMDLLIMPEXP size_t Cluster
(
bvector<DVec3d> const &xyzIn,
bvector<DVec3d> *xyzOut,
bvector<size_t>&oldIndexToPackedIndex,
double absTol = -1.0,
double relTol = -1.0
);

//! Find the min and max distances and their index positions.
public: static bool GEOMDLLIMPEXP MinMaxAngle (bvector<DVec3d> const &dataA, bvector<DVec3d> const &dataB,
            double &minAngle, size_t &minIndex,
            double &maxAngle, size_t &maxIndex
            );
};
//! @ingroup BentleyGeom_Operations
//! @description Operations on arrays of DVec2d (bvector and contiguous buffer).
struct DVec2dOps : VectorOps <DVec2d>
{
private: DVec2dOps (){};
public:

//! append a vector to the bvector<T> 

static GEOMDLLIMPEXP void AppendXY (bvector<DVec2d> *dest, double x, double y);
};
//! @description Operations on arrays of doubles (bvector and contiguous buffer).
//! Also contains various comparison and computation functions unique to doubles.
//! @ingroup BentleyGeom_Operations
struct DoubleOps : VectorOps <double>
{
public:

private: DoubleOps ();
public:
//! min of 2 candidates
static GEOMDLLIMPEXP double Min (double a1, double a2);
//! min of 3 candidates
static GEOMDLLIMPEXP double Min (double a1, double a2, double a3);
//! min of 4 candidates
static GEOMDLLIMPEXP double Min (double a1, double a2, double a3, double a4);

//! min absolute value of 2 candidates
static GEOMDLLIMPEXP double MinAbs (double a1, double a2);
//! min absolute value of 3 candidates
static GEOMDLLIMPEXP double MinAbs (double a1, double a2, double a3);
//! min absolute value of 4 candidates
static GEOMDLLIMPEXP double MinAbs (double a1, double a2, double a3, double a4);
//! Convert a double to a float, being sure the float is strictly less than the double.
static float DoubleToFloatRoundLeft (double d);
//! Convert a double to a float, being sure the float is strictly greater than the double.
static float DoubleToFloatRoundRight (double d);
//! min of 2 candidates
static GEOMDLLIMPEXP double Max (double a1, double a2);
//! min of 3 candidates
static GEOMDLLIMPEXP double Max (double a1, double a2, double a3);
//! min of 4 candidates
static GEOMDLLIMPEXP double Max (double a1, double a2, double a3, double a4);

//! min absolute value of 2 candidates
static GEOMDLLIMPEXP double MaxAbs (double a1, double a2);
//! min absolute value of 3 candidates
static GEOMDLLIMPEXP double MaxAbs (double a1, double a2, double a3);
//! min absolute value of 4 candidates
static GEOMDLLIMPEXP double MaxAbs (double a1, double a2, double a3, double a4);

//! min absolute value of 6 candidates
static GEOMDLLIMPEXP double MaxAbs (double a1, double a2, double a3, double a4, double a5, double a6);

//! update an evolving max value.
static GEOMDLLIMPEXP void UpdateMax (double &evolvingMax, double testValue);

//! Test if two doubles are equal within tolerance.
static GEOMDLLIMPEXP bool WithinTolerance (double a1, double a2, double abstol);

//! snap to zero if within tolerance.
static GEOMDLLIMPEXP double SnapZero (double a, double abstol);

//! Return -1,0,1 according to sign, with 0 case subject to tolerance.
static GEOMDLLIMPEXP int TolerancedSign (double a, double abstol);

//! Test if two doubles are equal within tolerance.
static GEOMDLLIMPEXP bool WithinTolerance (double a1, double a2, double abstol, double reltol);
//! Return a tolerance appropriate for coordinates up to maxSize
//! @param [in] maxSize large number for range of values being compared.
//! @param [in] abstol smallest allowed tolerance.  If 0 or negative, a small default is used.
//! @param [in] reltol a realtive tolerance.  If 0 or negative, a small default is used.
static GEOMDLLIMPEXP double ComputeTolerance (double maxSize, double abstol, double reltol);

static GEOMDLLIMPEXP double Max (bvector <double> &values);
static GEOMDLLIMPEXP double Min (bvector <double> &values);
static GEOMDLLIMPEXP double MaxAbs (bvector <double> &values);
static GEOMDLLIMPEXP double MinAbs (bvector <double> &values);

static GEOMDLLIMPEXP double Max (double const *values, size_t n);
static GEOMDLLIMPEXP double Min (double const *values, size_t n);
static GEOMDLLIMPEXP double MaxAbs (double const *values, size_t n);
static GEOMDLLIMPEXP double MinAbs (double const *values, size_t n);

static GEOMDLLIMPEXP void ScaleArray (double *values, size_t n, double a);
static GEOMDLLIMPEXP void ScaleArray (bvector <double> &values, double a);
//! Clamp a single fraction.
static GEOMDLLIMPEXP double ClampFraction (double s);

//! Clamp two fractions to 0..1 respecting directional relationship.
static GEOMDLLIMPEXP void ClampDirectedFractionInterval (double &t0, double &t1);

//! Clamp x so it is between a and b.  a is assumed less than or equal to b.
static GEOMDLLIMPEXP double Clamp (double x, double a, double b);
//! Bitwise equality test for exact 01 interval references...
static GEOMDLLIMPEXP bool IsExact01 (double a0, double a1);

//! Test if x is between 0 and 1 inclusive.
static GEOMDLLIMPEXP bool IsIn01 (double x);
//! Test if x is between 0 and 1 inclusive, with AlmostEqualFraction at each end.
static GEOMDLLIMPEXP bool IsAlmostIn01 (double x);

//! Test if x is between 0 and 1 inclusive.
static GEOMDLLIMPEXP bool IsIn01OrExtension (double x, bool extend0, bool extend1);

//! Test if x,y are both between 0 and 1 inclusive.
static GEOMDLLIMPEXP bool IsIn01 (double x, double y);

//! Test if x,y are both between 0 and 1 inclusive.
static GEOMDLLIMPEXP bool IsIn01 (DPoint2dCR xy);

//! Test if x,y,z are all between 0 and 1 inclusive.
static GEOMDLLIMPEXP bool IsIn01 (DPoint3dCR xyz);

//! return sqrt (a^2 + b^2)
static GEOMDLLIMPEXP double Hypotenuse (double a, double b);
//! return sqrt (a^2 + b^2 + c^2)
static GEOMDLLIMPEXP double Hypotenuse (double a, double b, double c);
//! return sqrt (a^2 + b^2 + c^2 + d^2)
static GEOMDLLIMPEXP double Hypotenuse (double a, double b, double c, double d);

//! return determinant 
static GEOMDLLIMPEXP double DeterminantXYXY (double x0, double y0, double x1, double y1);
//! Attempt to divide {numerator/denominator}
//! Return false if result is larger than 10 digits.
//! This is a very comfortable upper limit for parameter values (fractions and angles)
static GEOMDLLIMPEXP bool SafeDivideParameter (double &result, double numerator, double denominator, double defaultResult = 0.0);

//! Attempt to divide either of {numerator0/denominator0} or (numerator1/denominator1)
//! Return false if result is larger than 10 digits.
//! This is a very comfortable upper limit for parameter values (fractions and angles)
static GEOMDLLIMPEXP bool ChooseSafeDivideParameter
(
double &result,
double numerator0,
double denominator0,
double numerator1,
double denominator1,
double defaultResult = 0.0
);



//! Attempt to divide {numerator/denominator}
//! Return false if result is more than 15 digits.
//! (Distances larger than 15 digits are highly dangerous.  Even 10 digit distances is pretty dangerous, but they happen.)
//! @remark This is deprecated. Use ValidatedDivideDistance
static GEOMDLLIMPEXP bool SafeDivideDistance (double &result, double numerator, double denominator, double defaultResult = 0.0);

//! Attempt to divide {numerator/denominator}
//! @param [in] a numerator
//! @param [in] b denominator
//! @param [in] defaultResult value to return (in the ValidatedDouble) when the actual quotient distance is more than 15 digits
//! @return ValidatedDouble with quotient (or defaultResult)
static GEOMDLLIMPEXP ValidatedDouble ValidatedDivideDistance(double a, double b, double defaultResult = 0.0);

//! Attempt to divide {numerator/denominator}
//! @param [in] a numerator
//! @param [in] b denominator
//! @param [in] defaultResult value to return (in the ValidatedDouble) when the actual quotient distance is more than 30 digits
//! @return ValidatedDouble with quotient (or defaultResult)
static GEOMDLLIMPEXP ValidatedDouble ValidatedDivideDistanceSquared (double a, double b, double defaultResult = 0.0);

//! Attempt to divide {f/g}.  Also divide {(df*g - f*dg) / g^2}, i.e. form deriviative
//! @return ValidatedDVec2d with (x,y) = (f/g, (df*g-f*dg) / g^2)

static GEOMDLLIMPEXP ValidatedDVec2d ValidatedDivideAndDifferentiate
(
double f,   //!< [in] numerator
double df,  //!< [in] numerator derivative
double g,   //!< [in] denominator
double dg   //!< [in] denomiinator derivative
);


//! Attempt to divide {numerator/denominator}
//! @param [in] a numerator
//! @param [in] b denominator
//! @param [in] defaultResult value to return (in the ValidatedDouble) when the actual quotient is more than 10 digits.
//! @return ValidatedDouble with quotient (or defaultResult)
static GEOMDLLIMPEXP ValidatedDouble ValidatedDivideParameter (double a, double b, double defaultResult = 0.0);

//! Attempt to divide {numerator/denominator}
//! @param [in] a numerator
//! @param [in] b denominator
//! @param [in] defaultResult value to return (in the ValidatedDouble) when the {b < minRelativeDivisor * a}
//! @param [in] minRelativeDivisor smallest allowed divisor for 1.0.   Effectively, the largest allowed result is (1/minRelativeDivisor)
//! @return ValidatedDouble with quotient (or defaultResult)
static GEOMDLLIMPEXP ValidatedDouble ValidatedDivide (double a, double b, double defaultResult = 0.0, double minRelativeDivisor = 1.0e-15);



//! Attempt to divide {numerator/denominator}
//! Return false if result is larger than 30 digits.
//! This should be used only when the result is known to be a squared distance.
static GEOMDLLIMPEXP bool SafeDivideDistanceSquared (double &result, double numerator, double denominator, double defaultResult = 0.0);

//! Attempt to divide {numerator/denominator}
//! Return false if result is larger than than 1/safeDivideFraction.
//! Most users should call specific functions SafeDivideDistance, SafeDivideParameter, SafeDivideDistanceSquared which provide
//! appropriate fractions.
static GEOMDLLIMPEXP bool SafeDivide (double &result, double numerator, double denominator, double defaultResult = 0.0, double smallFraction = 1.0e-15);

//! Compute a linear transformation f(x) = c + d * x so that a0 maps to b0 and a1 maps to b1.
//! @return false if a0, a1 are nearly identical.
static GEOMDLLIMPEXP bool LinearTransform (double a0, double a1, double b0, double b1, double &c, double &d);
//! Find the Index of value in pData(sorted!!!) which is greater than searchValue.
static GEOMDLLIMPEXP bool UpperBound (double const *pData, size_t n, double searchValue, size_t &index);
//! Find the Index of value in pData(sorted!!!) which is greater or than equal to searchValue.
static GEOMDLLIMPEXP bool LowerBound (double const *pData, size_t n, double searchValue, size_t &index);
//! Search a sorted array for a bracketing interval, preferably
//!     lowerBound &le searchValue &lt upperBound (note the lessThanOrEqual and lessThan usage).
//! Exceptions: 1) If the searchValue exactly matches an internal value, the returned lowerBound is the searchValue and the search interval is "to the right" -- the searchValue and a larger upperBound
//!    2) If the search value is less than the start data, return the first non-trivial data interval -- i.e. skip over multiple copies of the start value.
//!    3) If the search value is greater than or equal to the final data, return the last non-trivial data interval -- i.e. skip backwards over copies of the last value.
//! @param [in] pData sorted data (e.g. bspline knots)
//! @param [in] n data count
//! @param [in] searchValue value to be bracketed
//! @param [out] lowerIndex index of lowerBound.
//! @param [out] lowerBound left interval bound.
//! @param [out] upperBound right interval bound.
//! @return false if the array has only one value.  Note that the out-of-bounds cases (but within a good array) return true and can
//!      be detected by testing searchValue against the lower and upper bounds.
static GEOMDLLIMPEXP bool BoundingValues (double const *pData, size_t n, double searchValue, size_t &lowerIndex, double &lowerBound, double &upperBound);
static GEOMDLLIMPEXP bool BoundingValues (bvector<double>const &data, double searchValue,  size_t &lowerIndex, double &lowerBound, double &upperBound);
static GEOMDLLIMPEXP bool BoundingValues (size_t n, double searchValue, size_t &lowerIndex, double &lowerBound, double &upperBound);
//! sort flat array of doubles, optionally descending.
static GEOMDLLIMPEXP void Sort (double *data, size_t count, bool ascending = true);
static GEOMDLLIMPEXP void Sort (bvector<double> &data, bool ascending = true);
//! Sort entries starting at i0.
static GEOMDLLIMPEXP void SortTail (bvector<double> &data, size_t i0, bool ascending = true);

//! Interpolate between values
static GEOMDLLIMPEXP double Interpolate (double dataA, double fraction, double dataB);
//! Return the fraction such that Interplate (dataA, fraction, dataB) equals value.
//! (Return 0 marked invalid if dataA and dataB are AlmostEqual)
static GEOMDLLIMPEXP ValidatedDouble InverseInterpolate (double dataA, double value, double dataB);
static GEOMDLLIMPEXP void Swap (double &dataA, double &dataB);

//! update evolving minmax f values with associated x values.
static GEOMDLLIMPEXP void UpdateMinMax (double x, double f, double &xMin, double &fMin, double &xMax, double &fMax);

//! return sqrt (a*a + b*b + c*c)
static GEOMDLLIMPEXP double Magnitude (double a, double b, double c);
//! return sqrt (a*a + b*b)
static GEOMDLLIMPEXP double Magnitude (double a, double b);
//! Near-equality test knowing range 0..1 for values...
static GEOMDLLIMPEXP bool AlmostEqualFraction (double a, double b);

//! Return -1,0,1 for less than, AlmostEqual, greater than
static GEOMDLLIMPEXP int TolerancedComparison (double a, double b);

//! test knowing range 0..1 for values...
static GEOMDLLIMPEXP bool ClearlyIncreasingFraction (double a, double b);

//! test with tolerance for equality 
static GEOMDLLIMPEXP bool ClearlyIncreasing (double a, double b, double tol);

static GEOMDLLIMPEXP double Sum (double *data, int n);
static GEOMDLLIMPEXP double Sum (bvector<double> const &data);
static GEOMDLLIMPEXP double SumAbs (double *data, int n);
static GEOMDLLIMPEXP double SumAbs (bvector<double> const &data);

//! Sum using Kanane's high precision correction sequence.  This takes
//! roughly 4 times as long as the simple sum but may be significantly more accurate.
static GEOMDLLIMPEXP double PreciseSum (double *data, size_t n);
static GEOMDLLIMPEXP double PreciseSum (double a, double b, double c);
static GEOMDLLIMPEXP double PreciseSum (double a, double b, double c, double d);

//! replace each value by its fractional position between a0 and a1.
//! return false and leave unchanged if a0==a1.
static GEOMDLLIMPEXP bool Normalize (bvector<double> &data, double a0, double a1);

/// <summary> make dest[i] = source[index[i]];</summary>
/// <remarks>index.size () determines the number moved</remarks>
static GEOMDLLIMPEXP void CopyToIndex (bvector<double> const &source, bvector<size_t> const &index, bvector<double> &dest);
/// <summary> make dest[index[i]] = source[i];</summary>
/// <remarks>index.size () determines the number moved</remarks>
static GEOMDLLIMPEXP void CopyFromIndex (bvector<double> const &source, bvector<size_t> const &index, bvector<double> &dest);
static GEOMDLLIMPEXP void SetZeros (bvector<double> &dest);
static GEOMDLLIMPEXP void SetSequential (bvector<double> &dest, double a0 = 0.0, double delta = 1.0);
static GEOMDLLIMPEXP double MaxAbsDiff (bvector<double> const &x, bvector<double> const &y);
static GEOMDLLIMPEXP void ApplyFunction (bvector<double> const &x, double (*f)(double), bvector <double> &fOfx);

//! Compute moving averages of blockSize consecutive values, optionally skipping some leading and trailing values.
//!<ul>
//!<li>If (numSkip0 + blockSize + numSkip1) is larger than soruce.size (), the dest array is returned empty.
//!<li>If blockSize is 0, the dest array is returned empty.
//!</ul>
static GEOMDLLIMPEXP void MovingAverages
(
bvector<double> &dest,              //!< [out] averaged values.
bvector<double> const &source,      //!< [in] values to average
size_t blockSize,                   //!< [in] number of consecutive values to be averaged
size_t numSkip0 = 0,                    //!< [in] number of leading values to skip.
size_t numSkip1 = 0                     //!< [in] number of trailing values to skip.
);

//! Construct moving averages of blockSize consecutive values of uniformly spaced original values.
//! Optionally have mutliple copies of first and last original values.
//!<ul>
//!<li>dest is returned empty if any count problem appears.
//!</ul>
static GEOMDLLIMPEXP void MovingAverages
(
bvector<double> &dest,   //!< [out] averaged values.
double value0,          //!< [in] first reference value
double value1,          //!< [in] last reference value.
size_t numOut,          //!< [in] number of averaged values to compute
size_t blockSize,       //!< [in] number of consecutive values to be averaged
size_t numCopy0 = 1,    //!< [in] number of copies of first value. Must be 1 or more
size_t numCopy1 = 1     //!< [in] number of copies of final value. Must be 1 or more
);

//! Constructs an array of Chebyshev points, optionally adding -1 and +1 end values.
static GEOMDLLIMPEXP void ChebyshevPoints
(
    bvector<double> &dest,   //!< [out] averaged values.
    size_t numChebyshev,          //!< [in] number of ChebyshevPoints to evaluate.   limit points and replication are additional.
    bool addLimitPoints,      //!< [in] if true, replicated points are -1 and 1.  If false, they are the actual Chebyshev points
    size_t numCopyLeft = 1,     //!< [in] number of copies of the left point.
    size_t numCopyRight = 1     //!< [in] number of copies of the right ponit
    );
//! replace each data[i] by (a + b * data[i]);
static GEOMDLLIMPEXP void LinearMapInPlace (bvector<double> &data, double a, double b);
//! map (-1,1) to (aMinus,aPlus) in place
static GEOMDLLIMPEXP void MinusOnePlus1LinearMapInPlace (bvector<double> &data, double aMinus, double aPlus);
//! map first point to a0, last to a1, others proportionally
static GEOMDLLIMPEXP void LinearMapFrontBackToInterval(bvector<double> &data, double a0, double a1);

//! Return the ith of n chebyshev points, cos(PI * (2i+1)/(2n)), starting at i=0
static GEOMDLLIMPEXP double ChebyshevPoint (size_t i, size_t n);
//! Relative tolerance for coordinate tests.  This is 1e-10, and is coarser than Angle::SmallAngle.
public: static GEOMDLLIMPEXP double SmallCoordinateRelTol ();
//! Return a distance (1.0e-6) that is essentially zero for typical metric calculation.
public: static GEOMDLLIMPEXP double SmallMetricDistance ();
//! Relative tolerance for coordinate tests with floats.  This is 2.0e-6.
public: static GEOMDLLIMPEXP double FloatCoordinateRelTol ();
};


//!
//! @description Operations on arrays of DPoint2d (bvector and contiguous buffer).
//!
//! @ingroup BentleyGeom_Operations
struct DPoint3dOps : VectorOps <DPoint3d>
{
private:
    DPoint3dOps (); // No instances.
public:

//! append a point to the bvector<T> 

static GEOMDLLIMPEXP void AppendXYZ (bvector<DPoint3d> *dest, double x, double y, double z = 0.0);

//! Append xy points with 0 z.
static GEOMDLLIMPEXP void AppendXY0 (bvector<DPoint3d> &dest, bvector<DPoint2d> const &source);

//!
//! @description Find nearly-identical points.  Pack the coordinates down.
//! @param [in] xyzIn array of points.   Coordinates are shuffled and array resized.
//! @param [out] xyzOut optional array to receive packed points.  May NOT alias xyzIn.
//! @param [out] oldIndexToPackedIndex For each original index, indicates the point's position in the resized array.
//! @param [in] absTol optional absolute tolerance.   Negative value requests default.  Zero is an allowable non-default value.
//! @param [in] relTol optional relative tolerance.   Negative value requests default.  Zero is an allowable non-default value.
//!
public:
static GEOMDLLIMPEXP size_t Cluster
(
bvector<DPoint3d> const &xyzIn,
bvector<DPoint3d> *xyzOut,
bvector<size_t>&oldIndexToPackedIndex,
double absTol = -1.0,
double relTol = -1.0
);



//!
//! Multiply each (non-disconnect) point in place by a transform.
//! @param [out]  xyz array of points, may have disconnects
//! @param [in] transform  transform to apply.
//!
public:
static GEOMDLLIMPEXP void Multiply
(
bvector<DPoint3d> *xyz,
TransformCR transform
);

//!
//! Add a vector to each element
//! @param [out]  xyz array of points, may have disconnects
//! @param [in] delta vector to add.
//!
public:
static GEOMDLLIMPEXP void Add
(
bvector<DPoint3d> &xyz,
DVec3dCR delta
);



//! Find the min and max distances and their index positions.
public: static bool GEOMDLLIMPEXP MinMaxDistance (bvector<DPoint3d> const &xyzA, bvector<DPoint3d> const &xyzB,
            double &minDistance, size_t &minIndex,
            double &maxDistance, size_t &maxIndex
            );


//!
//! @description search for closest point in array (NO implied curve or lines between points).
//! @return false if no points in the vector.
//!
public: static bool GEOMDLLIMPEXP ClosestPoint (bvector<DPoint3d> const &xyz, DPoint3dCR spacePoint, size_t &closestIndex, double &minDist);

//!
//! @description search for closest point in any array (NO implied curve or lines between points).
//! @return false if no points in the vector.
//!
public: static bool GEOMDLLIMPEXP ClosestPoint (bvector<bvector<DPoint3d>> const &xyz,
    DPoint3dCR spacePoint, size_t &outerIndex, size_t &innerIndex, double &minDist);



//!
//! @description search for farthest point from unbounded ray
//! @return false if no points in the vector.
//!
public: static bool GEOMDLLIMPEXP MaxDistanceFromUnboundedRay (bvector<DPoint3d> const &xyz, DRay3dCR ray, size_t &index, double &maxDistance);



//!
//! @description search for closest point in array (NO implied curve or lines between points).
//! Measure in XY only (after optional transform)
//! @return false if no points in the vector.
//!
public: static bool GEOMDLLIMPEXP ClosestPointXY (bvector<DPoint3d> const &xyz, DMatrix4dCP worldToLocal, DPoint3dCR spacePoint, size_t &closestIndex, double &minDist);


public:
//!
//! @param [in] pXYZIn Source array, possibly with disconnects.
//! @return range of points in the array.
//!
static GEOMDLLIMPEXP DRange3d Range (bvector<DPoint3d> const *pXYZIn);

public:
//!
//! @param [in] pXYZIn Source array
//! @param [in] worldToLocal transform to apply (nondestructive)
//! @return local coordinates range of points in the array.
//!
static GEOMDLLIMPEXP DRange3d Range (bvector<DPoint3d> const *pXYZIn, TransformCR worldToLocal);

//! @return the range of points projected to the parameter space of a ray.
public: static GEOMDLLIMPEXP DRange1d ProjectedParameterRange (bvector <DPoint3d> const &points, DRay3dCR ray);


//! Find the largest coordinate in array, ignoreing z.
static GEOMDLLIMPEXP double LargestXYCoordinate (DPoint3dCP points, size_t n);

//! Return the number of disconnect points in the array.
static GEOMDLLIMPEXP size_t CountDisconnects (bvector<DPoint3d> &points);

//! Inplace removal of disconnects.
//! @param [inout] data soure data.  count is reduced when points are eliminated.
static GEOMDLLIMPEXP void CompressDisconnects (bvector<DPoint3d> &data);

//!
//! @param [in] points Source array
//! @param [out] centroid centroid of the points.
//! @param [out] axes principal axes.
//! @param [out] moments second moments wrt x,y,z -- sums of (yy+zz,xx+zz,xx+yy)
//! @remarks axes are orderd so x is largest moment, y next, z smallest.
//! @return true if 1 or more points supplied.
//!
static GEOMDLLIMPEXP bool PrincipalAxes (bvector<DPoint3d> const &points, DVec3dR centroid, RotMatrixR axes, DVec3dR moments);

//!
//! @param [in] points Source array
//! @param [out] localToWorld local frame with with centroid at origin.
//! @param [out] worldToLocal transform into local frame.
//! @param [out] moments second moments wrt x,y,z -- sums of (yy+zz,xx+zz,xx+yy)
//! @return true if 1 or more points supplied.
//!
static GEOMDLLIMPEXP bool PrincipalAxes (bvector<DPoint3d> const &points, TransformR localToWorld, TransformR worldToLocal, DVec3dR moments);

//!
//! @param [in] pointsA First source array
//! @param [in] pointsB Second source array
//! @param [out] localToWorld local frame with with centroid at origin.
//! @param [out] worldToLocal transform into local frame.
//! @param [out] moments second moments wrt x,y,z -- sums of (yy+zz,xx+zz,xx+yy)
//! @return true if 1 or more points supplied.
//!
static GEOMDLLIMPEXP bool PrincipalAxes (
        bvector<DPoint3d> const &pointsA,
        bvector<DPoint3d> const &pointsB,
        TransformR localToWorld, TransformR worldToLocal, DVec3dR moments);

//!
//! @param [in] points Source array.  Each point is normalized for use in the sums.
//! @param [out] centroid centroid of the points.
//! @param [out] axes principal axes.
//! @param [out] moments second moments wrt x,y,z -- sums of (yy+zz,xx+zz,xx+yy)
//! @return true if 1 or more points supplied.
//!
static GEOMDLLIMPEXP bool PrincipalAxes (bvector<DPoint4d> const &points, DVec3dR centroid, RotMatrixR axes, DVec3dR moments);


//!
//! @param [in] pointsA First source array
//! @param [in] pointsB Second source array
//! @param [out] centroid centroid of the points.
//! @param [out] axes principal axes.
//! @param [out] moments second moments wrt x,y,z -- sums of (yy+zz,xx+zz,xx+yy)
//! @return true if 1 or more points supplied.
//!
static GEOMDLLIMPEXP bool PrincipalAxes (bvector<DPoint3d> const &pointsA, bvector<DPoint3d> const &pointsB, DVec3dR centroid, RotMatrixR axes, DVec3dR moments);

//!
//! @param [out] result Compressed points (subset of input points)
//! @param [in] source Input points
//! @param [in] chordTolerance tolerance.
//!
static GEOMDLLIMPEXP void CompressByChordError (bvector<DPoint3d>& result, bvector<DPoint3d> const& source, double chordTolerance);

//! @param [in] points source array
//! @param [out] originWithExtentVectors transform with
//!<pre>
//!<ul>
//!<li>axes in the direction of principal axes.
//!<li>origin at lower left of a tight (principal axes) bounding box.
//!<li>x,y,z column vectors are full extent along the bounding box.
//!<li>z column is the smallest column.
//!<li>z column has positive z if possible.
//!<li>x column has positive x if possible.
//!</pre>
static GEOMDLLIMPEXP bool    PrincipalExtents (bvector<DPoint3d> const &points, TransformR originWithExtentVectors);


//! @param [in] points source array
//! @param [out] originWithExtentVectors transform with
//!<pre>
//!<ul>
//!<li>axes in the direction of principal axes.
//!<li>origin at lower left of a tight (principal axes) bounding box.
//!<li>x,y,z column vectors are full extent along the bounding box.
//!<li>z column is the smallest column.
//!<li>z column has positive z if possible.
//!<li>x column has positive x if possible.
//!</pre>
//! @param [out] localToWorld rigid transform from local to world
//! @param [out] worldToLocal rigid transform from world to local
static GEOMDLLIMPEXP bool    PrincipalExtents (bvector<DPoint3d> const &points, TransformR originWithExtentVectors, TransformR localToWorld, TransformR worldToLocal);


//! Convert local coordinates range data to "lower left corner" extent box transform.
//! @return false if localToWorld is not orthogonal.
static GEOMDLLIMPEXP bool LocalRangeToOrderedExtents
(
TransformCR localToWorld, //!< [in] local coordinate frame (assumed rigid)
DRange3dCR localRange,    //!< [in] range cube in local coordinates
TransformR extentTransform,  //!< [out] transform with origin at lower left of range, xyz columns as full extent of the ranges, z the smallest direction, others shuffled to favor z and then x being positive.
TransformR centroidAlLocalToWorld,  //!< [out] rigid frame, same origin as localToWorld
TransformR centroidalWorldToLocal,   //!< [out] inverse of rigid frame.
DRange3dR  centroidalRange     //!< [out] range in sorted system.
);

//! Accumulate (points[i]-origin) into moments sums.
//! The summed terms are [xx xy xz x], [yx yy yz y], [zx zy zz z], [x y z 1]
//! @param [in] origin reference point.
//! @param [in] points array of points.
static GEOMDLLIMPEXP DMatrix4d MomentSums (DPoint3dCR origin, bvector<DPoint3d> const &points);
//! Accumulate products xx xy etc into a moment sum matrix.
//! Only the diagonal and upper triangle are accumulated.
static GEOMDLLIMPEXP void AccumulateToMomentSumUpperTriangle
(
DMatrix4dR sums,    //!< [in,out] evolving sums
DPoint3dCR origin,  //!< [in] origin for xyz vectors
DPoint3dCR point    //!< [in] point for accumulation
);
};
//! @description Operations in which an array of points is understood to be connected as a polyline (but not closed as a polygon).
//! @ingroup BentleyGeom_Operations
struct PolylineOps
{
private:
    PolylineOps (); // No instances.

//! Set the transition from m*n search to range heap for PolylinePolyline searches.
//! Exported primarily for testing, not expected to be called by apps.
public: static GEOMDLLIMPEXP void SetPolylinePolylineHeapTrigger (size_t mn);

//! convert segment index and fraction along segment to overall fraction of polyline.
public: static GEOMDLLIMPEXP double SegmentFractionToPolylineFraction (size_t segmentIndex, size_t numSegment, double segmentFraction);

//! convert fraction along total polyline to segment and local fraction.
//!  (Each segment of polyline is mapped to the same fraction step regardless of
//!   physical length)
//! @param [in] numVertex vertices in polyline
//! @param [in] fraction global fraction.
//! @param [out] segmentIndex
//! @param [out] numSegment number of segments
//! @param [out] segmentFraction fraction within segment
//! @param [out] isExtrapolated true if input fraction is outside 0..1
public: static GEOMDLLIMPEXP bool PolylineFractionToSegmentData
(
size_t numVertex,
double fraction,
size_t &segmentIndex,
size_t &numSegment,
double &segmentFraction,
bool &isExtrapolated
);

//! Test if two global fractions map to the same segment within a polyline
public: static GEOMDLLIMPEXP bool PolylineFractionsOnSameSegment
(
double f0,
double f1,
size_t numVertex
);

//! return the total length of the polyline.  Optionally add closure edge.  disconnects allowed.
public: static GEOMDLLIMPEXP double Length (bvector<DPoint3d> const &xyz, bool addClosure = false);
//! return the total length of polylines.  Optionally add closure edge. to each
public: static GEOMDLLIMPEXP double Length (bvector< bvector<DPoint3d> > const &xyz, bool addClosure = false);
//! return the total length of polylines, with worldToLocal applied to each vector.  Optionally add closure edge. to each
public: static GEOMDLLIMPEXP double Length (RotMatrixCP worldToLocal, bvector<DPoint3d> const &xyz, bool addClosure = false);

//! return statistical data (sum, min, max ..) for length of segments 
public: static GEOMDLLIMPEXP UsageSums SumSegmentLengths (bvector<DPoint3d> const &xyz);

//! return the total length of polyline with step index.  Optionally add closure edge.   Weights optional.
public: static GEOMDLLIMPEXP double Length (DPoint3dCP xyz, double const *weight, ptrdiff_t step, size_t n, bool addClosure = false);



// return  the total of (absolute) angles at vertices of polyline with step index.  Optionally add closure edge.   Weights optional.
//! @param [in] xyz points.
//! @param [in] weight optional weights.
//! @param [in] step step between points
//! @param [in] count number of points
//! @param [in] addClosure true to wrap
//! @param [in] minEdgeLength edges shorter than this are not considered.
public: static GEOMDLLIMPEXP double SumAbsoluteAngles (DPoint3dCP xyz, double const *weight, ptrdiff_t step, size_t count, bool addClosure, double minEdgeLength = 0.0);

//! Search for closest point on edge. Optionally add closure edge.
public: static GEOMDLLIMPEXP bool ClosestPoint
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        double &fraction,
        DPoint3dR curvePoint
        );

//! Search for closest point on edge. Optionally add closure edge.
public: static GEOMDLLIMPEXP bool ClosestPoint
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        double &fraction,
        DPoint3dR curvePoint,
        size_t &edgeIndex,
        size_t &numEdge,
        double &edgeFraction,
        bool extend0,
        bool extend1
        );

//! Search for closest point on edge, using xy coordinates after optional projection. Optionally add closure edge.
public: static GEOMDLLIMPEXP bool ClosestPointXY
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        DMatrix4dCP worldToLocal,
        double &globalFraction,
        DPoint3dR curvePoint,
        size_t &edgeIndex,
        size_t &numEdge,
        double &edgeFraction,
        double &xyDistance
        );

//! Search for closest point on edge, using xy coordinates after optional projection. Optionally add closure edge.
public: static GEOMDLLIMPEXP bool ClosestPointXY
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        DMatrix4dCP worldToLocal,
        double &globalFraction,
        DPoint3dR curvePoint,
        size_t &edgeIndex,
        size_t &numEdge,
        double &edgeFraction,
        double &xyDistance,
        bool extend0,
        bool extend1
        );


//! Find (pairs of) points of closest approach between two linestrings.
//! For parallel or coincident intervals, an arbitrary point is returned to represent the interval.
//! Candidates separated by more than maxDistance are not considered.
public: static bool GEOMDLLIMPEXP AddCloseApproaches
(
bvector<DPoint3d> const &xyzA,
bvector<DPoint3d> const &xyzB,
bvector< CurveLocationDetail> &locationA,
bvector< CurveLocationDetail> &locationB,
double maxDistance = DBL_MAX
);

public: static bool GEOMDLLIMPEXP AddCloseApproaches
(
bvector<DPoint3d> const &xyzA,
bvector<double> const *paramA,
bvector<DPoint3d> const &xyzB,
bvector<double> const *paramB,
bvector< CurveLocationDetail> &locationA,
bvector< CurveLocationDetail> &locationB,
double    maxDist
);

//! Compute intersections among linestrings.
//! @param [in] xyzA points on linestring A
//! @param [in] paramA optional parameters (e.g. distance along) for linestring A
//! @param [in] xyzB points on linestring B
//! @param [in] paramB optional parameters (e.g. distance along) for linestring B
//! @param [out] locationA returned intersections, with location information relative to linestringA.
//! @param [out] locationB returned intersections, with location information relative to linestringB.
//! @param [in] maxDist tolerance to consider "near hits" as intersections.  Send 0 to only receive intersections.
//!               Send a "near zero" number to find instances where a vertex of one linestring is near but not on an edge of the other.
public: static bool GEOMDLLIMPEXP CollectIntersectionsAndCloseApproachesXY
(
bvector<DPoint3d> const &xyzA,
bvector<double> const *paramA,
bvector<DPoint3d> const &xyzB,
bvector<double> const *paramB,
bvector< CurveLocationDetail> &locationA,
bvector< CurveLocationDetail> &locationB,
double    maxDist
);

//! Find the closest approach between polylines.   If multiple approaches have the same minimum distance,
//!  one is arbitrarily chosen.
public: static bool GEOMDLLIMPEXP ClosestApproach
(
bvector<DPoint3d> const &xyzA,
bvector<DPoint3d> const &xyzB,
CurveLocationDetailR locationA,
CurveLocationDetailR locationB
);

//! Both inputs must have increasing X.
//! Return pairs with every (xyzA,xyzB) pair with distinct x.
public: static GEOMDLLIMPEXP void CollectZPairsForOrderedXPoints (
bvector<DPoint3d>const &pointA,
bvector<DPoint3d> const &pointB,
bvector<CurveLocationDetailPair> &pairs
);



//! return true if there are any intersections within the linestring.
public: static GEOMDLLIMPEXP bool IsSelfIntersectingXY (bvector<DPoint3d> const &xyz, bool addClosure = false);
//! return true if there are any intersections within the linestring.
public: static GEOMDLLIMPEXP bool IsSelfIntersectingXY (DPoint3d *points, size_t n, bool addClosure = false);
//! test if points are form a rectangle.
public: static GEOMDLLIMPEXP bool IsRectangle (DPoint3dCP points, size_t n,
    TransformR localToWorld,
    TransformR worldToLocal,
    bool requireClosurePoint = true
    );

//! return true if the 4 edges between points 01,12,23,30 are perpendicular.
public: static GEOMDLLIMPEXP bool Are4EdgesPerpendicular (DPoint3dCP points);

//! test if points form a rectangle.
public: static GEOMDLLIMPEXP bool IsRectangle (bvector<DPoint3d> const &points,
    TransformR localToWorld,
    TransformR worldToLocal,
    bool requireClosurePoint = true);

//! Return the length of the segment starting at given index.
//! Return 0 if index is invalid.
public: static GEOMDLLIMPEXP double SegmentLength (bvector<DPoint3d> const &points, size_t index);
//! Return the length of the segment starting at given index.
//! Return 0 if index is invalid.
public: static GEOMDLLIMPEXP double SegmentLength (RotMatrixCP worldToLocal, bvector<DPoint3d> const &points, size_t index);

//! Return the point at fractional position along a specified edge.
//! Invalid edge indices are clamped.
public: static GEOMDLLIMPEXP DPoint3d SegmentFractionToPoint (bvector<DPoint3d> const &points, size_t index, double segmentFraction);
public: static GEOMDLLIMPEXP ValidatedDRay3d PolylineFractionToRay (bvector<DPoint3d> const &points, double polylineFraction);

//! Search (forward or reverse, according to sign of requested distance).
public: static GEOMDLLIMPEXP bool FractionAtSignedDistanceFromFraction
(
bvector<DPoint3d> const &xyz,
double startFraction,
double signedDistance,
double &endFraction,
size_t &endSegmentIndex,
double &segmentFraction,
double &actualDistance
);

//! Search (forward or reverse, according to sign of requested distance).
public: static GEOMDLLIMPEXP bool FractionAtSignedDistanceFromFraction
(
RotMatrixCP worldToLocal,
bvector<DPoint3d> const &xyz,
double startFraction,
double signedDistance,
double &endFraction,
size_t &endSegmentIndex,
double &segmentFraction,
double &actualDistance
);

//! Return distance (along polyline) between fractional positions.
//! Distance from large fraction to smaller fraction is negative distance.
public: static GEOMDLLIMPEXP double SignedDistanceBetweenFractions
(
bvector <DPoint3d> const & points,
double fraction0,
double fraction1
);

//! Return distance (along polyline) between fractional positions.
//! Distance from large fraction to smaller fraction is negative distance.
public: static GEOMDLLIMPEXP double SignedDistanceBetweenFractions
(
RotMatrixCP worldTolocal,
bvector <DPoint3d> const & points,
double fraction0,
double fraction1
);

public: static GEOMDLLIMPEXP bool FractionToFrenetFrame
(
bvector <DPoint3d> const & points,
double f,
TransformR frame
);

//! Search (forward or reverse, according to sign of requested distance).
public: static GEOMDLLIMPEXP void CopyBetweenFractions
(
bvector <DPoint3d> const &points,
bvector<DPoint3d> &dest,
double fractionA,
double fractionB
);

public: static void CopyBetweenFractions
(
bvector <DPoint3d> const &points,
bvector<CurveLocationDetail> &dest,
double fractionA,
double fractionB
);
//! return polyline length and centroid of the "wire".
//! Empty polyline returns false with length 0 and centroid 000.
//! Single point polylien returns true with length 0 and centroid at the single point.
//! @param [in] points polyline points.
//! @param [out] length summed lengths.
//! @param [out] centroid centroid of wire.
//! @param [in] fraction0 start fraction for active interval.
//! @param [in] fraction1 end fraction for active interval.
public: static GEOMDLLIMPEXP bool WireCentroid
(
bvector <DPoint3d> const &points,
double &length,
DPoint3dR centroid,
double fraction0 = 0.0,
double fraction1 = 1.0
);

//! @return the range of points projected to the parameter space of a ray.
public: static GEOMDLLIMPEXP DRange1d ProjectedParameterRange (bvector <DPoint3d> const &points, DRay3dCR ray, double fraction0, double fraction1);

//! push_back xyz to the point array, but possibly ignore if it duplicates points.back ()
//! @param points array to receive point
//! @param [in] xyz new point
//! @param [in] forceIncludeThisPoint if true, include this point even if it is a duplicate.
public: static GEOMDLLIMPEXP void AddContinuationStartPoint (bvector<DPoint3d>&points, DPoint3dCR xyz, bool forceIncludeThisPoint);

public: static GEOMDLLIMPEXP bool AddStrokes
(
bvector <DPoint3d> const &points,
bvector <DPoint3d> & strokes, 
IFacetOptionsCR options,
bool includeStartPoint = true,
double startFraction = 0.0,
double endFraction = 1.0
);

public: static GEOMDLLIMPEXP bool AddStrokes
(
DEllipse3dCR arc,
bvector <DPoint3d> & strokes, 
IFacetOptionsCR options,
bool includeStartPoint = true,
double startFraction = 0.0,
double endFraction = 1.0
);

//! Add xyz,fraction,tangent for this polyline to the arrays.
//!<ul>
//!<li>interior vertices of the polyline are present twice -- once with incoming tangent, once with outgoing.
//!</ul>
public: static GEOMDLLIMPEXP bool AddStrokes
(
bvector <DPoint3d> const &points,
DPoint3dDoubleUVCurveArrays &strokes, 
IFacetOptionsCR options,
double startFraction = 0.0,
double endFraction = 1.0,
ICurvePrimitiveCP curve = nullptr
);


public: static GEOMDLLIMPEXP size_t GetStrokeCount
(
bvector <DPoint3d> const &points,
IFacetOptionsCR options,
double startFraction = 0.0,
double endFraction = 1.0
);

//! Inplace compression of points to eliminate colinear points.
//! @param [in,out] points points to compress
//! @param [in] absTol absolute tolerance
//! @param [in] eliminateOverdraw if false, a 180 turn point is included in the output.  If true, the doubled line due to the
//!     180 degree turn is elmiinated.  (Hence the range of the compressed polygon can be smaller)
//! @param [in] closed If false, the first point always remains even if it is "within" colinear first and last segments.
//!       If true, this point can be eliminated.
//! @param [in] xyOnly if true, use only xy coordinates in comparisons.
public: static GEOMDLLIMPEXP void CompressColinearPoints (bvector<DPoint3d> &points, double absTol, bool eliminateOverdraw, bool closed, bool xyOnly = false);


//! Simple linestring offset -- miter joints, no loop removal.
static GEOMDLLIMPEXP bool OffsetLineStringXY
(
bvector<DPoint3d> &out,        //!< offset point.
bvector<DPoint3d> const &in,  //!< input points
double offsetDistance,        //!< signed offset distance
bool periodic = false,        //!< true to make offset joint for closure
double maxMiterRadians = 1.58 //!< largest angle of turn for an outside miter.  Suggested value is no more than
                              //!             Angle:DegreesToRadians (95.0)
);

//! Simple linestring offset -- miter joints, no loop removal.
static GEOMDLLIMPEXP bool OffsetLineString
(
bvector<DPoint3d> &out,        //!< offset point.
bvector<DPoint3d> const &in,  //!< input points
double offsetDistance,        //!< signed offset distance
DVec3dCR planeNormal,         //!< normal towards eye
bool periodic = false,        //!< true to make offset joint for closure
double maxMiterRadians = 1.58 //!< largest angle of turn for an outside miter.  Suggested value is no more than
                              //!             Angle:DegreesToRadians (95.0).  Large miter angles will create sharp
                              //!             joints to miter points far away from the vertex.
);

//! Build triangles that advance along two linestrings.
//! Selection of whether to advance on linestringA or linestringB is based on one-step lookahead -- the better aspect ratio of the two choices is chosen.
//! This is expected to be called in planar configurations where the construction guarantees there are no orientation flips.
static GEOMDLLIMPEXP void GreedyTriangulationBetweenLinestrings
(
bvector<DPoint3d> const & linestringA,   //!< [in] first linestring
bvector<DPoint3d> const &linestringB,    //!< [in] second linestring
bvector<DTriangle3d> &triangles,         //!< [out] triangle coordinates
bvector<int> *oneBasedABIndex = nullptr  //!< [out] one-based index positive index if from linestringA, negative if from linestringB
);

//! Build triangles that advance along two linestrings.
//! Selection of whether to advance on linestringA or linestringB is based on one-step lookahead -- the better aspect ratio of the two choices is chosen.
//! This is expected to be called in planar configurations where the construction guarantees there are no orientation flips.
static GEOMDLLIMPEXP void GreedyTriangulationBetweenLinestrings
(
bvector<DPoint3d> const & linestringA,   //!< [in] first linestring
bvector<DPoint3d> const &linestringB,    //!< [in] second linestring
bvector<DTriangle3d> &triangles,         //!< [out] triangle coordinates
bvector<int> *oneBasedABIndex,  //!< [out] one-based index positive index if from linestringA, negative if from linestringB
Angle planarityAngle       //!< [in] angle to use for lookahead for approximately coplanar candidates.
);


//! If segment chains to the final chain, add its endpoint to the chain.  Otherwise start a new chain.
static GEOMDLLIMPEXP void AppendToChains (bvector<bvector<DPoint3d>> &chains, DSegment3dCR segment);

//! push a point on the back, but skip of "AlmostEqual"
static GEOMDLLIMPEXP void AddPointIfDistinctFromBack (bvector<DPoint3d> &points, DPoint3d xyz);
//! If final point is "AlmostEqual" to first, make it exact.   If not, push first point to close.
static GEOMDLLIMPEXP void EnforceClosure(bvector<DPoint3d> &points);
//! Starting at initialIndex, pack out points that are AlmostEqual.  (if initialIndex is greater than 0, it is compared to [initialIndex-1])
static GEOMDLLIMPEXP void PackAlmostEqualAfter (bvector<DPoint3d> &points, size_t initialCount);

//! Make a copy of points in an array, starting at specified index and wrapping around.
//! Consecutive AlmostEqual points are eliminated.
//! A closure point is added if needed.
static GEOMDLLIMPEXP void CopyCyclicPointsFromStartIndex
(
bvector<DPoint3d> const &source,    //!< [in] given points.
size_t startIndex,                  //!< [in] index of first point to copy.
bvector<DPoint3d> &dest             //!< [out] copied points.
);
};



enum class PlanePolygonSSICode
{
Unknown = 0,
Transverse = 1,
Coincident = 2
};

//! @description Operations in which an array of points is understood to be connected as a closed polygon.
//! @ingroup BentleyGeom_Operations
struct PolygonOps
{
private:
    PolygonOps (); // No instances.

//!
//! @description Sum 0.5 times the cross product vectors from first point to all edges.
//!  If the polygon is planar, the magnitude of this vector is the area and its direction is the plane normal.
//!
public: static GEOMDLLIMPEXP DVec3d AreaNormal (bvector<DPoint3d> const &xyz);


//!
//! @description Sum 0.5 times the cross product vectors from first point, using only n points.  (n at most xyz.size ())
//!  If the polygon is planar, the magnitude of this vector is the area and its direction is the plane normal.
//!
public: static GEOMDLLIMPEXP DVec3d AreaNormal (bvector<DPoint3d> const &xyz, size_t n);


//!
//! @description test if the polygon is convex.
//!
public: static GEOMDLLIMPEXP bool IsConvex (bvector<DPoint3d> const &xyz);


//!
//! Triangulate a single xy polygon.  Triangulation preserves original
//!   indices.
//! @param [out] pIndices array of indices.  Each face is added
//!           as one-based indices, followed by a 0 (terminator).
//!           Interior edges are optionally negated.
//! @param [out] pExteriorLoopIndices array of indices of actual outer loops. (optional)
//!           (These are clockwise loops as viewed.)
//! @param [out] pXYZOut output points.  The first numPoint points are exactly
//!           the input points.   If necessary and permitted, additional
//!           xyz are added at crossings.  In the usual case in which crossings
//!           are not expected, this array may be NULL.
//! @param [in] pXYZIn array of polygon points.
//! @param [in] xyTolerance tolerance for short edges on input polygon.
//! @param [in] maxPerFace number of edges allowed per face.
//! @param [in] signedOneBasedIndices if true, output indices are 1 based, with 0 as separator.
//!           If false, indices are zero based wtih -1 as separator.
//! @param [in] addVerticesAtCrossings true if new coorinates can be added to pXYZOut
//! @return false if nonsimple polygon.
//!
public:
static GEOMDLLIMPEXP bool FixupAndTriangulateLoopsXY
(
bvector<int>        *pIndices,
bvector<int>        *pExteriorLoopIndices,
bvector<DPoint3d>   *pXYZOut,
bvector<DPoint3d>   *pXYZIn,
double                  xyTolerance,
int                     maxPerFace,
bool                    signedOneBasedIndices,
bool                    addVerticesAtCrossings
);

//!
//! Triangulate a single space polygon as projected in caller-supplied coordinate frame.
//!  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
//! @param [out] pIndices  array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
//!                                   and whether or not interior edges are negated.
//! @param [out] pExteriorLoopIndices array giving vertex sequence for each exterior loop.
//! @param [out] pXYZOut  array of output points, or NULL to disallow adding points at crossings.
//! @param [in] localToWorld local to world transformation
//! @param [in] worldToLocal world to local transformation
//! @param [in] pXYZIn  array of input polygon points.
//! @param [in] xyTolerance  tolerance for short edges on input polygon.
//! @param [in] bSignedOneBasedIndices  if false, output indices are 0-based, with -1 as separator.  If true, indices are 1-based, and
//!                                   interior edges are negated, with 0 as separator.
//! @return false if nonsimple polygon.
//!
public:
static GEOMDLLIMPEXP bool FixupAndTriangulateProjectedLoops
(
bvector<int>*       pIndices,
bvector<int>*       pExteriorLoopIndices,
bvector<DPoint3d>*  pXYZOut,
TransformCR             localToWorld,
TransformCR             worldToLocal,
bvector<DPoint3d>   *pXYZIn,
double                  xyTolerance,
bool                    bSignedOneBasedIndices
);

//! <ul>
//! <li>Apply worldToLocal transform.
//! <li>Triangulate as viewed in that plane
//! <li>Apply localToWorld transform to the triangles.
//! </ul>
//! @return false if triangulation issues.
static GEOMDLLIMPEXP bool FixupAndTriangulateProjectedLoops
(
bvector<bvector<DPoint3d>> const &loops,    //!< [in] array of loops
TransformCR localToWorld,                   //!< [in] transform for converting xy data to world
TransformCR worldToLocal,                   //!< [in] transform for converting world data to xy
bvector<DTriangle3d> &triangles             //!< [out] triangle coordinates.
);
//!
//! Triangulate a single space polygon.  Best effort to handle non-planar polygons.  Optionally handle self-intersecting polygons.
//! @param [out] pIndices  array of output indices.  The value of bSignedOneBasedIndices determines the separator of each face loop
//!                                   and whether or not interior edges are negated.
//! @param [out] pExteriorLoopIndices array giving vertex sequence for each exterior loop.
//! @param [out] pXYZOut  array of output points, or NULL to disallow adding points at crossings.
//! @param [out] localToWorld  local to world transformation
//! @param [out] worldToLocal  world to local transformation
//! @param [in] pXYZIn  array of input polygon points.
//! @param [in] xyTolerance  tolerance for short edges on input polygon.
//! @param [in] bSignedOneBasedIndices  if false, output indices are 0-based, with -1 as separator.  If true, indices are 1-based, and
//!                                   interior edges are negated, with 0 as separator.
//! @return false if nonsimple polygon.
//!
public:
static GEOMDLLIMPEXP bool FixupAndTriangulateSpaceLoops
(
bvector<int>*       pIndices,
bvector<int>*       pExteriorLoopIndices,
bvector<DPoint3d>*  pXYZOut,
TransformR              localToWorld,
TransformR              worldToLocal,
bvector<DPoint3d>*  pXYZIn,
double                  xyTolerance,
bool                    bSignedOneBasedIndices
);

//!
//! Triangulate a multiple space loops as viewed in an internally-chosen xy plane.
//! @param [out] triangleIndices array giving vertex sequence for each exterior loop.
//! @param [out] exteriorLoopIndices  array of output points, or NULL to disallow adding points at crossings.
//! @param [out] xyzOut output coordinates.  May have additional points (at self intersections)
//! @param [out] localToWorld  local to world transformation
//! @param [out] worldToLocal  world to local transformation
//! @param [in] loops  array of arrays of points traversing loops.
//! @return false if nonsimple polygon.
//!
public:
static GEOMDLLIMPEXP bool FixupAndTriangulateSpaceLoops
(
bvector<int>        &triangleIndices,
bvector<int>        &exteriorLoopIndices,
bvector<DPoint3d>   &xyzOut,
TransformR              localToWorld,
TransformR              worldToLocal,
bvector<bvector<DPoint3d>> const &loops
);

//! Triangulate a multiple space loops as viewed in an internally-chosen xy plane.
static GEOMDLLIMPEXP bool FixupAndTriangulateSpaceLoops
(
bvector<bvector<DPoint3d>> const &loops,
bvector<DTriangle3d> &triangles
);
//!
//! Compute a local to world transformation for a polygon (disconnects allowed)
//! Favor first polygon CCW for upwards normal.
//! Favor the first edge as x direction.
//! Favor first point as origin.
//! If unable to do that, use GPA code which will have random relation of origin and x direction.
//! @param [in] pXYZIn  polygon data.
//! @param [out] localToWorld  coordinate frame
//! @param [out] worldToLocal  inverse
//!
public:
static GEOMDLLIMPEXP bool CoordinateFrame
(
bvector<DPoint3d>   *pXYZIn,
TransformR              localToWorld,
TransformR              worldToLocal
);

//!
//! Compute a local to world transformation for a polygon (disconnects allowed)
//! Favor first polygon CCW for upwards normal.
//! Favor the first edge as x direction.
//! Favor first point as origin.
//! If unable to do that, use GPA code which will have random relation of origin and x direction.
//! @param [in] pXYZIn polygon points.
//! @param [in] numXYZ number of points.
//! @param [out] localToWorld  coordinate frame
//! @param [out] worldToLocal  inverse
//!
public:
static GEOMDLLIMPEXP bool CoordinateFrame
(
DPoint3dCP       pXYZIn,
size_t          numXYZ,
TransformR      localToWorld,
TransformR      worldToLocal
);

public:
static GEOMDLLIMPEXP bool CoordinateFrame
(
DPoint3dCP       pXYZIn,
size_t          numXYZ,
TransformR              localToWorld,
TransformR              worldToLocal,
enum LocalCoordinateSelect selector
);



//!
//! @description Adjust the triangulation indices returned by ~mvu_triangulateSpacePolygon to match the orientation of the
//!       original polygon.
//! @remarks Although the triangles returned by ~mvu_triangulateSpacePolygon will have consistent orientation, they are not
//!       guaranteed to have the same orientation as the input polygon.
//! @remarks The original polygon is ASSUMED to be non-self-intersecting.  This function may incorrectly decide to reorient
//!       the triangulation if the original polygon is self-intersecting.
//! @param [out] indices array of triangulation indices, formatted as per bSignedOneBasedIndices
//! @param [out] pbReversed true if orientation was reversed; false if orientation unchanged (can be NULL).
//! @param [in] bSignedOneBasedIndices If true, indices are one-based, interior edges are negated, and 0 is the separator;
//!                                  if false, indices are 0-based and -1 is the separator.
//! @return false if invalid index array
//!
public:
static GEOMDLLIMPEXP bool  ReorientTriangulationIndices
(
bvector<int>    &indices,
bool                *pbReversed,
bool                bSignedOneBasedIndices
);

//! Return the of the polygon's projection to the xy plane.
//! Disconnects are allowed.   Areas of each loop are added without test for containment or xy orientation.
//! Enter holes with opposite orientation from their containing loop.
public: static GEOMDLLIMPEXP double AreaXY (bvector<DPoint3d> const &xyz);
//! Return the of the polygon's projection to the xy plane.
//! Disconnects are allowed.   Areas of each loop are added without test for containment or xy orientation.
//! Enter holes with opposite orientation from their containing loop.
public: static GEOMDLLIMPEXP double AreaXY (DPoint3dCP pXYZ, size_t numXYZ);

//! Return the of the polygon's area.
//! Disconnects are allowed.   Areas of each loop are added without test for containment or xy orientation.
//! Enter holes with opposite orientation from their containing loop.
public: static GEOMDLLIMPEXP double Area (bvector<DPoint2d> const &xy);
//! Return the of the polygon's area.
//! Disconnects are allowed.   Areas of each loop are added without test for containment or xy orientation.
//! Enter holes with opposite orientation from their containing loop.
public: static GEOMDLLIMPEXP double Area (DPoint2dCP pXY, size_t numXY);
//! Return true if the xy parts of point xyz project into the xy parts of a triangle with vertices point0, point1, point2.
public: static GEOMDLLIMPEXP bool IsPointInOrOnXYTriangle (DPoint3dCR xyz, DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2);
//! Return true if the xy parts of point xyz project into the xy parts of a convex polygon
public: static GEOMDLLIMPEXP bool IsPointInConvexPolygon (DPoint2dCR xy, DPoint2dCP points, int numPoint, int sense = 0);
//! Classify a point with respect to a polygon.
//! @param point => point to test
//! @param polygonPoints => polygon points.  Last point does not need to be duplicated.
//! @param tol => tolerance for "on" detection. May be zero.
//! @return 1 if point is "in" by parity, 0 if "on", -1 if "out", -2 if nothing worked.
public: static GEOMDLLIMPEXP int PointPolygonParity
(
DPoint2dCR point,
bvector<DPoint2d> const &polygonPoints,
double      tol
);

//! Classify a point with respect to a polygon.
//! @param point => point to test
//! @param points => polygon points.  Last point does not need to be duplicated.
//! @param numPoint => number of points
//! @param tol => tolerance for "on" detection. May be zero.
//! @return 1 if point is "in" by parity, 0 if "on", -1 if "out", -2 if nothing worked.
public: static GEOMDLLIMPEXP int PointPolygonParity
(
DPoint2dCR point,
DPoint2d const *points,
size_t numPoint,
double      tol
);

//! Return the 3D polygon centroid, normal and area.
//! Disconnects are allowed.   Areas of each loop are added without test for containment or xy orientation.
//! Enter holes with opposite orientation from their containing loop.
public: static GEOMDLLIMPEXP bool CentroidNormalAndArea (bvector<DPoint3d> const &xyz, DPoint3dR centroid, DVec3dR normal, double &area);
//! Return the 3D polygon centroid, normal and area.
//! Disconnects are allowed.   Areas of each loop are added without test for containment or xy orientation.
//! Enter holes with opposite orientation from their containing loop.
public: static GEOMDLLIMPEXP bool CentroidNormalAndArea (DPoint3dCP pXYZ, size_t numXYZ, DPoint3dR centroid, DVec3dR normal, double &area);

//! Return the centroid and area of the polygon.
//! @return false if zero area.
public: static GEOMDLLIMPEXP bool CentroidAndArea (bvector<DPoint2d> &points, DPoint2dR centroid, double &area);

//! @return true if the polygon has well defined area, and if so compute products of moment integrals
//! [xx xy xz x; xy yy yz y; xz yz zz z; x y z 1]
//! @param [in] xyz polygon points
//! @param [in] origin origin for moments
//! @param [out] products integrated moments.
public: static GEOMDLLIMPEXP bool SecondAreaMomentProducts (bvector<DPoint3d> const &xyz, DPoint3dCR origin, DMatrix4dR products);

//! Search for a triangle which has an interior point intersection with a ray.
//! The triangles considered all have the polygon start point as one vertex and a polygon edge as opposite edge.
//! If the ray strikes an even number of triangles, it is considered non-intersecting.
//! If the ray strikes an odd number of triangles, the first triangle hit is the live one for localUVW
//! @param [in] pXYZ coordinates for triangles.
//! @param [in] numXYZ number of points.
//! @param [in] ray search ray.
//! @param [out] xyz hit point.
//! @param [out] triangleFractions barycentric coordinates within triangle.
//! @param [out] rayFraction coordinate along ray 
//! @param [out] edgeBaseIndex base index of far edge of triangle.
public: static GEOMDLLIMPEXP bool PickTriangleFromStart
    (
    DPoint3dCP pXYZ,
    size_t numXYZ,
    DRay3dCR ray,
    DPoint3dR xyz,
    DPoint3dR triangleFractions,
    double &rayFraction,
    size_t &edgeBaseIndex
    );

//! Compute the area normal of the polygon.  If its dot product with positiveDirection is negative, reverse the points.
//! @return true if a normal flip was applied.
public: static GEOMDLLIMPEXP bool ReverseForPreferedNormal (bvector<DPoint3d> &xyz, DVec3dCR positiveDirection);
//! Search for a triangle where uv is an interior point.
//! The triangles considered all have the polygon start point as one vertex and a polygon edge as opposite edge.
//! @param [in] xyPoints coordinates for triangles.
//! @param [in] n number of points.
//! @param [in] xy search point.
//! @param [out] edgeBaseIndex base index of far edge of triangle.
//! @param [out] uvw barycentric coordinates within triangle.
//! @param [out] duvwdX derivative of uvw wrt x of the search point.
//! @param [out] duvwdY derivative of uvw wrt y of the search point.
public: static GEOMDLLIMPEXP bool PickTriangleFromStart
    (
    DPoint2dCP xyPoints, size_t n, DPoint2dCR xy,
    size_t &edgeBaseIndex,
    DPoint3dR uvw, DPoint3dR duvwdX, DPoint3dCR duvwdY
    );

//! Collect data for a polygon crossing a plane.
//! The "trueCrossing" data is always guaranteed to have an even number of entries.
//! @param [in] points polygon points.  (Start end repeat NOT expected)
//! @param [in] plane 
//! @param [in] touchTolerance tolerance for detecting touches.
//! @param [out] trueCrossings array with an entry for each place that there is a true upwards or downwards crossing.
//!<pre>
//!<ul>
//!<li>The index is the start point index for the edge.
//!<li>The fraction is the fraction within the edge.
//!<li>An "on" vertex with adjacent vertices on opposites sides appears with fraction 0 for the outgoing edge.
//!<li>If one or more segments are "on" and have prior and following vertices on opposite sides, the "crossing"
//!    is recorded as the final "on" vertex.
//!</ul> 
//!</pre>   
//! @param [out] touchData optional details about exact "on" points.   The tolerance is used for recording this points.
//! @param [out] altitudeLimits min and max signed altitudes.
//! @return
//!<pre>
//!<ul>
//!<li> SSICode::Unknown -- less than 2 points.
//!<li> SSICode::AllTouching
public: static GEOMDLLIMPEXP PlanePolygonSSICode PlaneIntersectionPoints
(
bvector<DPoint3d> const &points,
DPlane3dCR plane,
double touchTolerance,
bvector<CurveLocationDetail> &trueCrossings,
bvector<CurveLocationDetail> *touchData,
DRange1d &altitudeLimits
);
    
};


//! Implement the classic "Union Find" algorithm in bvector<size_t>
//! Begin with n distinct sequential integers 0..(n-1).
//! Application decides that various pairs (i,j) are "equlivalent", transitively.
//! Application ultimately needs to know one identifier for each of the (fewer0 equivalance classes.
//!
//! Use pattern (starting with an empty clusterData vector) is usually distinct 3 phases:
//!  1) Create trivial cluster data.
//!  2) Announce equivalence pairs.
//!  3) Retrive final representatives of clusters.
//! Late appearance of new cluster ids is possible, but be aware that assigned final representatives can change.
//! The algorithm is extraordinarily fast (no more than 5 averagte lookups per entry).
//! Phase 3 searches can alter the internal structure of the vector but do not change final representatives.
//!<code>
//! clusterData.clear ();
//! 
//! for (...)
//!     {
//!     .... index = UnionFind::NewClusterIndex (clusterData);
//!     }
//! for (...)
//!     {
//!     UnionFind::MergeClusters (clusterData, index0, index1);
//!     }
//! for (...)
//!     {
//!     size_t clusterId = UnionFind::FindClusterRoot (clusterData, seedIndex);
//!     }
//!</code>
//! Once FindClusterRoot has been executed from every index, every index points directly to its representative (root)
//! and further FindClusterRoot calls execute only a single lookup.
//!
//! Under normal usage there are no errors -- every lookup or merge returns another valid index.
//! If an invalid index is supplied to a lookup or merge, the search terminates and returns that index.
//! (There is no attempt to classify or fixup from bad inputs -- just recognize that they are bad and quit)
//! @ingroup BentleyGeom_Operations

struct UnionFind
{
//! Simple bounds check in an index.
//! This is called frequently in the algorithm, but in proper use it never fails --
//!  all transitions lead to safe places.
static bool GEOMDLLIMPEXP IsValidClusterIndex (bvector <size_t> &clusterData, size_t index);

//! Create a new cluster, i.e. new entry at end of array pointing to itself as parent.
static size_t GEOMDLLIMPEXP NewClusterIndex (bvector<size_t> &clusterData);

//! Create numAdd new clusters.  Return the total number of clusters.
static size_t GEOMDLLIMPEXP AddClusters (bvector<size_t> &clusterData, size_t numAdd);

//! Announce that the clusters that contains index0 are ultimately the same cluster.
static size_t GEOMDLLIMPEXP MergeClusters (bvector<size_t> &clusterData, size_t index0, size_t index1);

//! Walk from start to root.   All entries along the way are fixed up to point directly to the root.
static size_t GEOMDLLIMPEXP FindClusterRoot (bvector<size_t> &clusterData, size_t start);

};

//! Embed a double ended queue<T> in a bvector<T>
//!<ul>
//!<li>The dequeue's logical start value a can appear at any m_i0 within the bvector.
//!<li>successive entries proceed to the right and wrap as needed.
//!<li>In a wrapped state, the dequeue layout is [c..dXXXXXXXa..b]
//!<ul>
//!<li>a..b c..d is the logical dequeue contents.
//!<li>XXXXXXX is unused array entries.
//!</ul>
//!<li>m_i0 indexes to the "a" position.
//!<li>m_i1=m_i0+n where n is the number of entries in a..b c..d.
//!<li>Hence m_i1 can be beyond the physical array limit
//!<li>When phyical size is reached, it is doubled and appropriate data updates occur.
//!</ul>

template <typename T>
struct DoubleEndedQueue
{

private:
bvector<T> m_entries;
// The dequeue entries are in indices i0 <= i < i1, considered cyclically.
size_t      m_i0;   // Always 0 <= m_i0 < m_entries.size ();
size_t      m_i1;   // Always m_i0 <= m_i1 <= m_i0 + m_entries.size ();
T m_defaultValue;

size_t CyclicPhysicalIndex (size_t i)  const
    {
    if (i < m_entries.size ())
        return i;
    else
        return i % m_entries.size ();
    }


public:
//! Clear the dequeue
void Clear (){m_i0 = m_i1 = 0;}
//! Initialize with a specified default value to be returned for invalid accesses.
DoubleEndedQueue (T defaultValue)
    :
    m_entries(10),
    m_defaultValue (defaultValue)
    {
    Clear ();
    }
//! @return true if the dequeue is empty.
bool IsEmpty () const {return m_i1 <= m_i0;}
//! Ensure there is capacity for 1 additional insertion to the logical dequeue.
//! Returns with no change if there is already capacity
//! Doubles the physical array size if there is not already capacity.
void MakeRoomForInsertion ()
    {
    size_t n0 = m_i1 - m_i0;
    size_t n1 = m_entries.size ();
    if (n0 < n1)
        return;
    // (and strictly, n0==n1 -- the > case cannot happen)
    // the array is packed with entries a..b c..d organized physically as [c..d a..b]
    // Copy all entries of the physical aray to the back, so its contents are [c..d a..b c..d a..b]
    m_entries.reserve (2 * n0);
    for (size_t i = 0; i < n0; i++)
        m_entries.push_back (m_entries[i]);
    // m_i0 already points at the first a
    // m_i1 = m_i0 + n0 is already correct, but now with no wraparound happening.
    // so there is nothing more to do!!!
    }
//! @return the number of entries in the dequeue.
size_t Size () const { return m_i1 - m_i0;}
//! Add value to the dequeue at the right
void AddRight (T value)
    {
    MakeRoomForInsertion ();
    m_entries[CyclicPhysicalIndex (m_i1)] = value;
    m_i1++;
//     if (s_printHullSteps)
//         printf ("   PushLeft  %3d\n", (int)value);
    }
//! Add value to the dequeue at the left
void AddLeft (T value)
    {
    MakeRoomForInsertion ();
//     if (s_printHullSteps)
//         printf ("   PushLeft  %3d\n", (int)value);
    if (m_i0 == 0)
        {
        size_t n = m_entries.size ();
        m_i0 += n;
        m_i1 += n;
        }
    m_i0--;
    m_entries[m_i0] = value;
    }
//! return the right entry in the dequeue
T GetRight () const
    {
    if (IsEmpty ())
        return m_defaultValue;
    return m_entries[CyclicPhysicalIndex(m_i1)];
    }

//! return the left entry in the dequeue
T GetLeft () const
    {
    if (IsEmpty ())
        return m_defaultValue;
    return m_entries[m_i0];
    }

//! return the i'th entry counting cyclically from the left
T GetCyclicFromLeft (ptrdiff_t k) const
    {
    size_t n = m_i1 - m_i0;
    if (n == 0)
        return m_defaultValue;
    if (k < 0)
        k = n - ((-k) % n);
    if (k >= (ptrdiff_t)n)
        k = (k % n);
    if ((ptrdiff_t)m_i0 + k >= (ptrdiff_t)m_i1)
        return m_defaultValue;
    size_t i = m_i0 + k;
    return m_entries[CyclicPhysicalIndex(i)];
    }

//! return the i'th entry counting from right to left.
T GetFromRight (size_t k) const
    {
    if (IsEmpty ())
        return m_defaultValue;
    if (m_i0 + k >= m_i1)
        return m_defaultValue;
    size_t i = m_i1 - 1     - k;
    return m_entries[CyclicPhysicalIndex(i)];
    }

//! return the k'th entry counting from left to right
T GetFromLeft (size_t k) const
    {
    if (IsEmpty ())
        return m_defaultValue;
    size_t i = m_i0 + k;
    if (i < m_i1)
        return m_entries[CyclicPhysicalIndex(i)];
    return m_defaultValue;
    }

//! remove the right-most entry
void PopRight ()
    {
    if (!IsEmpty ())
        {
//         if (s_printHullSteps)
//             printf ("   PopRight %3d\n", (int)GetRight ());
        if (m_i1 > m_i0)
            m_i1--;
        }
    }

//! remove the left-most entry.
void PopLeft ()
    {
    if (!IsEmpty ())
        {
//         if (s_printHullSteps)
//             printf ("   PopLeft  %3d\n", (int)GetLeft ());
        m_i0++;
        if (m_i0 >= m_entries.size ())
            {
            m_i0 = 0;
            m_i1 -= m_entries.size ();
            }
        }
    }
//! Test if two queues have identical logical entries.
//! (Physical layouts -- i.e. m_i0,m_i1 can be different!!)
bool operator == (DoubleEndedQueue<T> const &other) const
    {
    size_t n = Size ();
    if (n != other.Size ())
        return false;
    for (size_t i = 0; i < n; i++)
        if (GetFromLeft (i) != other.GetFromLeft (i))
            return false;
    return true;
    }
};

//! Implementation of a priorityQueue (sometimes called a heap).
//!<ul>
//!<li>Values can be inserted and removed in any order.
//!<li>The priority queue structure is organized so that it is efficient to remove the "smallest" element at any time.
//!<li>The heap structure (within a bvector<Entry>) is a binary tree.
//! <ul>
//! <li>The tree is always "packed" in the array -- there are no holes.
//! <li>m_heap.front() is the minimum value entry.
//! <li>The children of m_heap[i] are m_heap[2i+1] and m_heap[2*i+2]
//! <li>m_heap[i] always has smaller value than either of its children.
//! <li>This relationship guarantees that overally cost of inserting and removing n values (with arbitrary ordering of insert and remove) is O(n * log(n))
//! </ul>
//!</ul> 
template <typename T>
struct MinimumValuePriorityQueue
    {
    struct Entry
        {
        double m_value;
        T m_data;
        Entry (T data, double value) : m_data (data), m_value (value) {}
        };
    private:
    bvector<Entry> m_heap;

    size_t Parent (size_t i)
        {
        if (i > 0)
            return (i - 1) / 2;
        return 0;       // root is its own parent !?!?!
        }
    size_t LeftChild (size_t i) { return 2 * i + 1; }
    size_t RightChild (size_t i) { return 2 * i + 2; }
    public: size_t Size () const { return m_heap.size ();}
    public:
    void Clear (){m_heap.clear ();}
    //! insert an entry
    void Insert (T data, double value)
        {
        //! place at the very back of the array -- this is unlikely to be its final position!!!
        Entry entry (data, value);
        m_heap.push_back (entry);
        size_t child = m_heap.size () - 1;
        //! bubble up until the heap conditions are satisfied.
        while (child > 0)
            {
            size_t parent = Parent (child);
            if (m_heap[parent].m_value < m_heap[child].m_value)
                break;
            std::swap (m_heap[parent], m_heap[child]);
            child = parent;
            }
        }
    public:
    // ASSUME heap has data .. use IsEmpty () to pretest !!!
    double MinValue (){return m_heap.front ().m_value;}
    T MinData (){return m_heap.front ().m_data;}
    bool IsEmpty (){return m_heap.empty ();}


    //! Remove the root (minimum sort value) entry
    bool RemoveMin (T &data, double &value)
        {
        size_t n = m_heap.size ();
        if (n == 0)
            return false;
        data = m_heap.front ().m_data;
        value = m_heap.front ().m_value;
        // fill the hole at the root with the very last array entry.
        m_heap.front () = m_heap.back ();
        m_heap.pop_back ();
        size_t parent = 0;
        // push the new value back down until the sort condition is again in effect.
        for (;;)
            {
            // swap with the smaller child and continue down ....
            size_t child = LeftChild (parent);
            if (child >= n)
                break;
            size_t rightChild = child + 1;
            if (rightChild < n && m_heap[rightChild].m_value < m_heap[child].m_value)
                child = rightChild;
            if (m_heap[child].m_value < m_heap[parent].m_value)
                {
                std::swap (m_heap[child], m_heap[parent]);
                parent = child;
                }
            else
                break;
            }
        return true;
        }

    bool RemoveMin ()
        {
        T data;
        double value;
        return removeMin (data, value);
        }
    //! verify the heap structure .
    bool Validate ()
        {
        size_t errors = 0;
        size_t n = m_heap.size ();
        for (size_t i = 0; i < n; i++)
            {
            size_t i1 = LeftChild (i);
            size_t i2 = RightChild (i);
            if (i1 < n && m_heap[i1].m_value < m_heap[i].m_value)
                errors++;
            if (i2 < n && m_heap[i2].m_value < m_heap[i].m_value)
                errors++;
            }
        return errors == 0;
        }
    };

//! Pair of arrays of corresponding xyz and double values.
//!<ul>
//!<li> The arrays are public (m_xyz and m_f).
//!<li> Search methods treat the double array as "fractions" -- sorted and in the interval [0..1]
//!<li> These are "white box" structures -- semantics of all arrays may vary among use cases.
//!</ul>
struct DPoint3dDoubleArrays
{
bvector<DPoint3d> m_xyz;
bvector<double>   m_f;

DPoint3dDoubleArrays (){}

//! Append to each array
GEOMDLLIMPEXP void AppendXF (DPoint3dCR xyz, double f);
//! Append xyz to its array, 
//! If there are prior fractions, add deltaF to the last.
//! If not, make the begin () fraction 0.0;
GEOMDLLIMPEXP void AppendXdeltaF (DPoint3dCR xyz, double deltaF);

//!Search the fraction array for an interval containing f.
//!<ul>
//!<li>This assumes the fractions are sorted
//!<li>When f is within the range of the fractions, f0 and f1 are surrounding values.
//!<li>When f is outside the range of the fractions, f0 and f1 are the appropriate boundary interval.
//!</ul>
GEOMDLLIMPEXP bool SearchBracketPoints (double f, size_t &i0, double &f0, DPoint3dR xyz0, size_t &i1, double &f1, DPoint3dR xyz1) const;

//! Return the range of the points.
GEOMDLLIMPEXP DRange3d GetRange () const;

//! Return the range of the points under a transform.
GEOMDLLIMPEXP DRange3d GetRange (TransformCR transform) const;
//! Reverse the order of the xyz and F arrays.
//! Optionally change each fraction value (f) to (1-f) so it remains sorted and 0 to 1.
//! This is virtual so derived classes can reverse additional arrays.
GEOMDLLIMPEXP void ReverseXF (bool reverseFAs01Fraction);
//! Fill with xyz and fraction for uniform fraction steps on an arc.
GEOMDLLIMPEXP void Stroke (DEllipse3dCR arc, size_t numPoints);
//! Fill with xyz and fraction for uniform fraction steps on an segment.
GEOMDLLIMPEXP void Stroke (DSegment3dCR segment, size_t numPoints);

//! Single-step construct and stroke
GEOMDLLIMPEXP DPoint3dDoubleArrays (DEllipse3dCR arc, size_t numPoints);
//! Single-step construct and stroke
GEOMDLLIMPEXP DPoint3dDoubleArrays (DSegment3dCR segment, size_t numPoints);
};

//! DPoint3dDoubleArrays with additional markup:
//!<ul>
//!<ul>m_uv -- array of 2d (uv, xy) data
//!<ul>m_vectorU -- typically curve tangent or surface partial derivative
//!<ul>m_vectorB -- typically curve secondary vector or surface partial derivative
//!</ul>
struct DPoint3dDoubleUVArrays : DPoint3dDoubleArrays
{
bvector<DPoint2d> m_uv;
bvector<DVec3d>   m_vectorU;
bvector<DVec3d>   m_vectorV;

//!<ul>
//!<li>Invoke the base class ReverseXF with reverseFAs01Fraction.
//!<li>Reverse all other arrays, optionally negating vectors
//!</ul>
//!
GEOMDLLIMPEXP void ReverseXFUV (bool reverseFAs01Fraction, bool negateVectorU, bool negateVectorV);


};

//! DPoint3dDoubleUVArrays with additional markup:
//!<ul>
//!<ul>m_curve -- array of curve primitive pointer (not ref counted!!!)
//!</ul>
struct DPoint3dDoubleUVCurveArrays : DPoint3dDoubleUVArrays
{
bvector<ICurvePrimitive *>  m_curve;

//!<ul>
//!<li>Invoke the base class ReverseXFYV with with passthrough args
//!<li>Reverse curves, optionally negating vectors
//!</ul>
//!
GEOMDLLIMPEXP void ReverseXFUVC (bool reverseFAs01Fraction, bool negateVectorU, bool negateVectorV);
//!<ul>
//!<li> Reverse in range index0&lt;=index&lt;index1
//!<li> Any array for which any part of the index range is out of bounds is skipped
//!</ul>
GEOMDLLIMPEXP void ReverseXFUVC (size_t index0, size_t index1, bool reverseFAs01Fraction, bool negateVectorU, bool negateVectorV);


//! Add data to respective arrays.
void Add (DPoint3dCR xyz, double f, DVec3dCR vectorU, ICurvePrimitive* curve)
    {
    m_xyz.push_back (xyz);
    m_f.push_back (f);
    m_vectorU.push_back (vectorU);
    m_curve.push_back (curve);
    }
void Add (ValidatedDRay3d const &ray, double f, ICurvePrimitive* curve)
    {
    if (ray.IsValid ())
        {
        m_xyz.push_back (ray.Value ().origin);
        m_f.push_back (f);
        m_vectorU.push_back (ray.Value ().direction);
        m_curve.push_back (curve);
        }
    }

//! Add data in respective arrays.  Return false if insertion index is invalid for any array.
bool Insert (size_t i, DPoint3dCR xyz, double f, DVec3dCR vectorU, ICurvePrimitive* curve)
    {
    bool ok = true;
    if (i <= m_xyz.size ())
        m_xyz.insert (m_xyz.begin () + i, xyz);
    else
        ok = false;
    if (i <= m_xyz.size ())
        m_f.insert (m_f.begin () + i, f);
    else
        ok = false;
    if (i <= m_vectorU.size ())
        m_vectorU.insert (m_vectorU.begin () + i, vectorU);
    else
        ok = false;
    if (i <= m_curve.size ())
        m_curve.insert (m_curve.begin () + i, curve);
    else
        ok = false;
    return ok;
    }

};

/*__PUBLISH_SECTION_END__*/

//! class to be called for many polygons to insert points along edges.
//! Usage patterns:
//! <ul>
//! <li>Create context:     InsertPointsInPolylineContext context (tolerance)
//! <li> for each of (many) polygons, call context.InsertPointsInPolyline (source, dest, spacePoints);
struct InsertPointsInPolylineContext
{
// point with sort key for sorting along edges.
  struct DPoint3dDouble
    {
    DPoint3d xyz;
    double   a;
    GEOMDLLIMPEXP DPoint3dDouble (DPoint3dCR _xyz, double _a) : xyz(_xyz), a (_a) {}
    GEOMDLLIMPEXP bool operator < (DPoint3dDouble const &dataB) const
        {
        return this->a < dataB.a;
        }
    };

double m_tolerance;
bvector<DPoint3dDouble> m_interiorPoints;
//! Constructor: retain tolerance
GEOMDLLIMPEXP InsertPointsInPolylineContext (double tolerance);
//! Test for points within tolerance
GEOMDLLIMPEXP bool AlmostEqual (DPoint3dCR pointA, DPoint3dCR pointB) const;
//! Fill the m_interiorPoints array with <x,y,z,fraction> for all space points that are close to a
//!   strict interior point of the segment.
GEOMDLLIMPEXP void CollectInteriorPointsAlongEdge (DSegment3dCR segment, bvector<DPoint3d> const &spacePoints);
//! For each space point which is close to the interior of a segemnt of the polyline, insert the space point
//! into the dest polyline.
GEOMDLLIMPEXP void InsertPointsInPolyline (bvector<DPoint3d> const &source, bvector<DPoint3d> &dest, bvector<DPoint3d> const &spacePoints);
};

/*__PUBLISH_SECTION_START__*/

END_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define ROUND_FLOAT_AWAY_FROM_ZERO 1.000000119
#define ROUND_FLOAT_TOWARDS_ZERO   0.9999999404
#define FLOAT_COORDINATE_RELTOL 2.0e-6

float DoubleOps::DoubleToFloatRoundLeft (double d)
    {
    float f = (float) d;
    return (f<d) ? f : (float) (d * (d<0 ? ROUND_FLOAT_AWAY_FROM_ZERO : ROUND_FLOAT_TOWARDS_ZERO));
    }

float DoubleOps::DoubleToFloatRoundRight (double d)
    {
    float f = (float) d;
    return (f>d) ? f : (float) (d * (d<0 ? ROUND_FLOAT_TOWARDS_ZERO : ROUND_FLOAT_TOWARDS_ZERO));
    }
bool FRange3d::IsNull () const
    {
    return low.x == FLT_MAX
        && low.y == FLT_MAX
        && low.z == FLT_MAX
        && high.x == -FLT_MAX
        && high.y == -FLT_MAX
        && high.z == -FLT_MAX
        ;
    }

bool FRange3d::HasAnyReversedLowHigh () const
        {
        return high.x < low.x
            || high.y < low.y
            || high.z < low.z;
        }

FRange3d FRange3d::NullRange ()
    {
    return FRange3d::from (
             FLT_MAX,  FLT_MAX,  FLT_MAX,
            -FLT_MAX, -FLT_MAX, -FLT_MAX
            );
    }

void FRange3d::InitNull ()
    {
    init (
             FLT_MAX,  FLT_MAX,  FLT_MAX,
            -FLT_MAX, -FLT_MAX, -FLT_MAX
            );
    }

FRange3d FRange3d::From (DRange3dCR dRange)
    {
    if (dRange.IsEmpty ())
        return NullRange ();
    return FRange3d::from (
        FPoint3d::FromRoundLeft (dRange.low),
        FPoint3d::FromRoundRight (dRange.high)
        );
    }

FPoint3d FRange3d::Low  () const {return low;}
FPoint3d FRange3d::High () const {return high;}
// -- FROM with DPoint3d inputs
FRange3d FRange3d::From (DPoint3dCR point)
    {
    auto range = NullRange ();
    range.Extend (point);   // due to directional rounding, this is generally NOT a single point!!!
    return range;
    }

FRange3d FRange3d::From (DPoint3dCR pointA, DPoint3dCR pointB)
    {
    auto range = NullRange ();
    range.Extend (pointA);
    range.Extend (pointB);
    return range;
    }
FRange3d FRange3d::From (bvector<DPoint3d> const &points)
    {
    DRange3d rangeD = DRange3d::From (points);
    return FRange3d::From (rangeD);
    }

// -- FROM with FPoint3d inputs
FRange3d FRange3d::From (FPoint3dCR point)
    {
    return from (point, point);
    }

FRange3d FRange3d::From (FPoint3dCR pointA, FPoint3dCR pointB)
    {
    auto range = from (pointA, pointA);
    range.Extend (pointB);
    return range;
    }

FRange3d FRange3d::From (bvector<FPoint3d> const &points)
    {
    auto range = NullRange ();
    for (auto &p: points)
        range.Extend (p);
    return range;
    }

static bool Intersect1dInterval (float &low, float &high, float lowA, float highA, float lowB, float highB)
    {
    low = std::max (lowA, lowB);
    high = std::min (highA, highB);
    return low <= high;
    }

FRange3d FRange3d::FromIntersection (FRange3dCR rangeA, FRange3dCR rangeB)
    {
    if (rangeA.IsNull ())
        return NullRange ();
    if (rangeB.IsNull ())
        return NullRange ();
    FRange3d result;
    if (    !Intersect1dInterval (result.low.x, result.high.x, rangeA.low.x, rangeB.high.x, rangeB.low.x, rangeB.high.x)
        ||  !Intersect1dInterval (result.low.y, result.high.y, rangeA.low.y, rangeB.high.y, rangeB.low.y, rangeB.high.y)
        ||  !Intersect1dInterval (result.low.z, result.high.z, rangeA.low.z, rangeB.high.z, rangeB.low.z, rangeB.high.z)
        )
        return NullRange ();
    return result;
    }


// extend low, high with a new double value, using directional double-to-float
inline void FixLowHigh (float &low, float a, float &high)
    {
    if (a < low)
        low = a;
    if (a > high)
        high = a;
    }

// extend low, high with a new double value, using directional double-to-float
inline void FixLowHighD (float &low, double x, float &high)
    {
    float a = DoubleOps::DoubleToFloatRoundLeft (x);
    if (a < low)
        low = a;
    float b = DoubleOps::DoubleToFloatRoundRight (x);
    if (b > high)
        high = b;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Extend (DPoint3dCR point)
    {
    FixLowHighD (low.x, point.x, high.x);
    FixLowHighD (low.y, point.y, high.y);
    FixLowHighD (low.z, point.z, high.z);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Extend (double a)
    {
    if (!IsNull ())
        {
        low.x = DoubleOps::DoubleToFloatRoundLeft ((double)low.x - a);
        low.y = DoubleOps::DoubleToFloatRoundLeft ((double)low.y - a);
        low.z = DoubleOps::DoubleToFloatRoundLeft ((double)low.z - a);

        high.x = DoubleOps::DoubleToFloatRoundLeft ((double)high.x + a);
        high.y = DoubleOps::DoubleToFloatRoundLeft ((double)high.y + a);
        high.z = DoubleOps::DoubleToFloatRoundLeft ((double)high.z + a);

        if (a < 0 && HasAnyReversedLowHigh ())
            InitNull ();
        }
    }


/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Extend (DPoint3dCR pointA, DPoint3dCR pointB)
    {
    Extend (pointA);
    Extend (pointB);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Extend (bvector<DPoint3d> const &points)
    {
    for (auto &xyz : points)
        Extend (xyz);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Extend (FPoint3dCR point)
    {
    FixLowHigh (low.x, point.x, high.x);
    FixLowHigh (low.y, point.y, high.y);
    FixLowHigh (low.z, point.z, high.z);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Extend (FPoint3dCR pointA, FPoint3dCR pointB)
    {
    Extend (pointA);
    Extend (pointB);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Extend (bvector<FPoint3d> const &points)
    {
    for (auto &xyz : points)
        Extend (xyz);
    }

void FRange3d::Extend (FRange3dCR other)
    {
    if (!other.IsNull ())
        {
        Extend (other.low);
        Extend (other.high);
        }
    }

FRange3d FRange3d::FromUnion (FRange3dCR rangeA, FRange3dCR rangeB)
    {
    if (rangeA.IsNull ())
        return rangeB;
    if (rangeB.IsNull ())
        return rangeA;
    FRange3d u = rangeA;
    u.Extend (rangeB.low);
    u.Extend (rangeB.high);
    return u;
    }

bool FRange3d::IsSinglePoint () const
    {
    if (IsNull ())
        return false;
    return low.x == high.x && low.y == high.y && low.z == high.z;
    }

double FRange3d::Volume () const
    {
    if (IsNull ())
        return false;
    return XLength () * YLength () * ZLength ();
    }
bool FRange3d::AreAllSidesLongerThan (double a)
    {
    if (IsNull())
        return false;
    if (low.x + a >= high.x)
        return false;
    if (low.y + a >= high.y)
        return false;
    if (low.z + a >= high.z)
        return false;
    return true;
    }

bool FRange3d::IntersectsWith (FRange3dCR other, bool trueForExactTouch) const
    {
    if (trueForExactTouch)
        {
        if (low.x > other.high.x)
            return false;
        if (other.low.x > high.x)
            return false;

        if (low.y > other.high.y)
            return false;
        if (other.low.y > high.y)
            return false;
        
        if (low.z > other.high.z)
            return false;
        if (other.low.z > high.z)
            return false;
        }
    else
        {
        if (low.x >= other.high.x)
            return false;
        if (other.low.x >= high.x)
            return false;

        if (low.y >= other.high.y)
            return false;
        if (other.low.y >= high.y)
            return false;
        
        if (low.z >= other.high.z)
            return false;
        if (other.low.z >= high.z)
            return false;
        }
    return true;
    }

//=============================================================================================
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double FRange3d::XLength () const {double a = high.x - low.x; return a > 0.0 ? a : 0.0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double FRange3d::YLength () const {double a = high.y - low.y; return a > 0.0 ? a : 0.0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double FRange3d::ZLength () const {double a = high.z - low.z; return a > 0.0 ? a : 0.0;}
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double FRange3d::DiagonalDistance () const
    {
    return IsNull () ? 0.0 : low.Distance (high);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double FRange3d::DiagonalDistanceXY () const
    {
    return IsNull () ? 0.0 : low.DistanceXY (high);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FVec3d FRange3d::DiagonalVector () const
    {
    return IsNull () ? FVec3d::From (0,0,0) : FVec3d::FromStartEnd (low, high);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FVec3d FRange3d::DiagonalVectorXY () const
    {
    FVec3d diagonal = DiagonalVector ();
    diagonal.z = 0.0;
    return diagonal;
    }


static double DistanceSquaredOutsideDirecteFRange1d (double x, double low, double high)
    {
    if (high <= low)
        return 0.0;
    double d = 0.0;        
    if (x < low)
        d = low-x;
    else if (x > high)
        d = x - high;
    else
        return 0.0;
    return d * d;
    }

static double DistanceSquaredBetweenIntervals
(
double aLow,
double aHigh,
double bLow,
double bHigh
)
    {
    double d;
    if (aHigh < bLow)
        {
        d = bLow - aHigh;
        return d * d;
        }
    else if (bHigh < aLow)
        {
        d = aLow - bHigh;
        return d * d;
        }
    return 0.0;

    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double FRange3d::DistanceOutside (DPoint3dCR xyz) const
    {
    double d = DistanceSquaredOutsideDirecteFRange1d (xyz.x, low.x, high.x)
             + DistanceSquaredOutsideDirecteFRange1d (xyz.y, low.y, high.y)
             + DistanceSquaredOutsideDirecteFRange1d (xyz.z, low.z, high.z);
    return sqrt (d);             
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double FRange3d::DistanceSquaredOutside (DPoint3dCR xyz) const
    {
    double d = DistanceSquaredOutsideDirecteFRange1d (xyz.x, low.x, high.x)
             + DistanceSquaredOutsideDirecteFRange1d (xyz.y, low.y, high.y)
             + DistanceSquaredOutsideDirecteFRange1d (xyz.z, low.z, high.z);
    return d;
    }


double FRange3d::DistanceSquaredTo (FRange3dCR other) const
    {
    return DistanceSquaredBetweenIntervals (low.x, high.x, other.low.x, other.high.x)
        + DistanceSquaredBetweenIntervals (low.y, high.y, other.low.y, other.high.y)
        + DistanceSquaredBetweenIntervals (low.z, high.z, other.low.z, other.high.z);
    }

bool FRange3d::IsSmallDistance (double a, bool nullRangeResult) const
    {
    if (IsNull ())
        return nullRangeResult;
    return fabs (a) < DoubleOps::FloatCoordinateRelTol () * DoubleOps::Max (1.0, LargestCoordinate ());
    }
/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
bool FRange3d::IsAlmostZeroX () const {return IsSmallDistance (high.x - low.x);}
bool FRange3d::IsAlmostZeroY () const {return IsSmallDistance (high.y - low.y);}
bool FRange3d::IsAlmostZeroZ () const {return IsSmallDistance (high.z - low.z);}
bool FRange3d::IsAlmostZeroXYZ () const {return IsSmallDistance (DoubleOps::MaxAbs (high.x - low.x, high.y - low.y, high.z - low.z));}



/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @bsimethod
+----------------------------------------------------------------------*/
double FRange3d::LargestCoordinate () const
    {
    if (IsNull ())
        return 0.0;
    FVec3d diagonal = FVec3d::FromStartEnd (low, high);
    return DoubleOps::Max
        ( low.MaxAbs (), high.MaxAbs(), diagonal.MaxAbs ());
    }

double FRange3d::MaxAbs () const
    {
    if (IsNull ())
        return 0.0;
    return DoubleOps::Max( low.MaxAbs (), high.MaxAbs());
    }


/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @bsimethod
+----------------------------------------------------------------------*/
double FRange3d::LargestCoordinateXY () const
    {
    if (IsNull ())
        return 0.0;
    FVec3d diagonal = FVec3d::FromStartEnd (low, high);
    return DoubleOps::Max (
                DoubleOps::MaxAbs (low.x, low.y, high.x, high.y),
                fabs (diagonal.x),
                fabs (diagonal.y));
    }



/*-----------------------------------------------------------------*//**
*
* Generates an 8point box around around a range cube.  Point ordering is
* maintained from the cube.
*
* @param [out] box array of 8 points of the box
* @bsimethod
+----------------------------------------------------------------------*/
void FRange3d::Get8Corners (bvector<FPoint3d> &corners) const
    {
    corners.clear ();
    FPoint3d minmax[2];
    int ix,iy,iz,i;
    minmax[0] = low;
    minmax[1] = high;
    i = 0;
    for( iz = 0; iz < 2; iz++ )
        for ( iy = 0; iy < 2; iy++ )
            for ( ix = 0; ix < 2; ix++ )
                {
                corners.push_back (FPoint3d::From (minmax[ix].x, minmax[iy].y, minmax[iz].z ));
                i++;
                }
    }

bool FRange3d::IsContained (FPoint3dCR point) const
    {
    return      point.x >= low.x
             && point.y >= low.y
             && point.z >= low.z
             && point.x <= high.x
             && point.y <= high.y
             && point.z <= high.z;
    }

bool FRange3d::IsContainedXY (FPoint3dCR point) const
    {
    return      point.x >= low.x
             && point.y >= low.y
             && point.x <= high.x
             && point.y <= high.y;
    }

bool FRange3d::IsContained (double ax, double ay, double az) const
    {
    return  (   ax >= this->low.x
             && ay >= this->low.y
             && az >= this->low.z
             && ax <= this->high.x
             && ay <= this->high.y
             && az <= this->high.z);
    }

bool FRange3d::IsEqual (FRange3dCR range1) const
    {
    return
           this->low.x  == range1.low.x
        && this->low.y  == range1.low.y
        && this->low.z  == range1.low.z
        && this->high.x == range1.high.x
        && this->high.y == range1.high.y
        && this->high.z == range1.high.z
        ;
    }

bool FRange3d::IsEqual (FRange3dCR range1, double tolerance) const
    {
    int numNull = 0;
    if (IsNull ())
        numNull ++;
    if (range1.IsNull ())
        numNull++;
    if (numNull == 2)
        return true;
    if (numNull == 1)
        return false;
    return
           fabs (this->low.x  - range1.low.x ) <= tolerance
        && fabs (this->low.y  - range1.low.y ) <= tolerance
        && fabs (this->low.z  - range1.low.z ) <= tolerance
        && fabs (this->high.x - range1.high.x) <= tolerance
        && fabs (this->high.y - range1.high.y) <= tolerance
        && fabs (this->high.z - range1.high.z) <= tolerance
        ;
   }


END_BENTLEY_GEOMETRY_NAMESPACE

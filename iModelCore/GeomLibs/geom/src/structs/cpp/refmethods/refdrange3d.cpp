/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

#define MSVECTOR_
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
 @instance pRange IN range to query
 @return the largest individual coordinate value among (a) range min point,
 (b) range max point, and (c) range diagonal vector.
 @group "DRange3d Queries"
 @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDRange3d_getLargestCoordinate

(
DRange3dCP pRange
)

    {
     double     max;

     if (pRange->IsNull ())
            return 0.0;

     max = fabs(pRange->low.x);
     FIX_MAX(fabs(pRange->high.x), max);
     FIX_MAX(fabs(pRange->low.y), max);
     FIX_MAX(fabs(pRange->high.y), max);
     FIX_MAX(fabs(pRange->low.z), max);
     FIX_MAX(fabs(pRange->high.z), max);
     auto diagonal = pRange->high - pRange->low;

     FIX_MAX(fabs(diagonal.x), max);
     FIX_MAX(fabs(diagonal.y), max);
     FIX_MAX(fabs(diagonal.z), max);

     return max;

    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange3d::XLength () const {double a = high.x - low.x; return a > 0.0 ? a : 0.0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange3d::YLength () const {double a = high.y - low.y; return a > 0.0 ? a : 0.0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange3d::ZLength () const {double a = high.z - low.z; return a > 0.0 ? a : 0.0;}
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange3d::DiagonalDistance () const
    {
    return IsNull () ? 0.0 : low.Distance (high);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange3d::DiagonalDistanceXY () const
    {
    return IsNull () ? 0.0 : low.DistanceXY (high);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DVec3d DRange3d::DiagonalVector () const
    {
    return IsNull () ? DVec3d::From (0,0,0) : DVec3d::FromStartEnd (low, high);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DVec3d DRange3d::DiagonalVectorXY () const
    {
    DVec3d diagonal = DiagonalVector ();
    diagonal.z = 0.0;
    return diagonal;
    }


static double DistanceSquaredOutsideDirectedRange1d (double x, double low, double high)
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
double DRange3d::DistanceOutside (DPoint3dCR xyz) const
    {
    double d = DistanceSquaredOutsideDirectedRange1d (xyz.x, low.x, high.x)
             + DistanceSquaredOutsideDirectedRange1d (xyz.y, low.y, high.y)
             + DistanceSquaredOutsideDirectedRange1d (xyz.z, low.z, high.z);
    return sqrt (d);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange3d::DistanceSquaredOutside (DPoint3dCR xyz) const
    {
    double d = DistanceSquaredOutsideDirectedRange1d (xyz.x, low.x, high.x)
             + DistanceSquaredOutsideDirectedRange1d (xyz.y, low.y, high.y)
             + DistanceSquaredOutsideDirectedRange1d (xyz.z, low.z, high.z);
    return d;
    }


double DRange3d::DistanceSquaredTo (DRange3dCR other) const
    {
    return DistanceSquaredBetweenIntervals (low.x, high.x, other.low.x, other.high.x)
        + DistanceSquaredBetweenIntervals (low.y, high.y, other.low.y, other.high.y)
        + DistanceSquaredBetweenIntervals (low.z, high.z, other.low.z, other.high.z);
    }

/*-----------------------------------------------------------------*//**
* @description Initializes a range cube with (inverted) large positive and negative
* values.
+----------------------------------------------------------------------*/
DRange3d DRange3d::NullRange ()

    {
     DRange3d range;
     range.Init ();
     return range;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsAlmostZeroZ () const
    {
    double maxAbs = LargestCoordinate ();
    double dz = high.z - low.z;
    return DoubleOps::AlmostEqual (maxAbs, maxAbs + dz);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsAlmostZeroY () const
    {
    double maxAbs = LargestCoordinate ();
    double d = high.y - low.y;
    return DoubleOps::AlmostEqual (maxAbs, maxAbs + d);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsAlmostZeroX () const
    {
    double maxAbs = LargestCoordinate ();
    double d = high.x - low.x;
    return DoubleOps::AlmostEqual (maxAbs, maxAbs + d);
    }



/*-----------------------------------------------------------------*//**
* @description Initializes the range to contain the single given point.
* @param [in] point  the point
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (DPoint3dCR point)
    {
     DRange3d range;
     range.InitFrom (point);
     return range;
    }

/*-----------------------------------------------------------------*//**
* @description Initializes the range to contain the two given points.
* @param [in] point0  first point
* @param [in] point1  second point
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (DPoint3dCR point0, DPoint3dCR point1)
    {
     DRange3d range;
     range.InitFrom (point0, point1);
     return range;
    }

/*-----------------------------------------------------------------*//**
* @description Initializes the range to contain two points given as components.
* Minmax logic is applied to the given points.
* @param [in] x0  first x
* @param [in] y0  first y
* @param [in] z0  first z
* @param [in] x1  second x
* @param [in] y1  second y
* @param [in] z1  second z
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (double x0, double y0, double z0, double x1, double y1, double z1)
    {
     DRange3d range;
     range.InitFrom (x0, y0, z0, x1, y1, z1);
     return range;
    }

/*-----------------------------------------------------------------*//**
* Initialize the range.InitFrom a single point given by components
* @param [in] x  x coordinate
* @param [in] y  y coordinate
* @param [in] z  z coordinate
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (double x, double y, double z)
    {
     DRange3d range;
     range.InitFrom (x, y, z);
     return range;
    }

/*-----------------------------------------------------------------*//**
* @description Initialize the range.InitFrom given min and max in all directions.
* Given values will be swapped if needed.
* @param [in] v0 min (or max)
* @param [in] v1 max (or min)
+----------------------------------------------------------------------*/
DRange3d DRange3d::FromMinMax (double v0, double v1)
    {
     DRange3d range;
     range.InitFromMinMax (v0, v1);
     return range;
    }

/*-----------------------------------------------------------------*//**
* @description Initializes a range to contain three given points.
* @param [in] point0  first point
* @param [in] point1  second point
* @param [in] point2  third point
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2)
    {
     DRange3d range;
     range.InitFrom (point0, point1, point2);
     return range;
    }

/*-----------------------------------------------------------------*//**
* @description Initializes a range cube with (inverted) large positive and negative
* values.
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Init ()
    {
    this->low.x = this->low.y = this->low.z =  DBL_MAX;
    this->high.x = this->high.y = this->high.z = -DBL_MAX;
    }

#define PANIC_SIZE 1.0e100


/*-----------------------------------------------------------------*//**
* @description Check if the range is exactly the same as the null ranges of a just-initialized
* range.
* @param [in] pRange  range to test.
* @return true if the range is null.
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsNull () const
    {
    if (
           this->low.x == DBL_MAX
        && this->low.y == DBL_MAX
        && this->low.z ==  DBL_MAX
        && this->high.x == -DBL_MAX
        && this->high.y == -DBL_MAX
        && this->high.z == -DBL_MAX
        )
        {
        return true;
        }
    else if (
           fabs(this->low.x) < PANIC_SIZE
        && fabs(this->low.y) < PANIC_SIZE
        && fabs(this->low.z) < PANIC_SIZE
        && fabs(this->high.x) < PANIC_SIZE
        && fabs(this->high.y) < PANIC_SIZE
        && fabs(this->high.z) < PANIC_SIZE
        )
        {
        return false;
        }
    else
        {
        /* It doesn't match the definition of a null, but it is definitely not normal.
         This is a good place for a breakpoint!!*/
//      throwException ("BadRange");
        return true;
        }
    }


/*-----------------------------------------------------------------*//**
* @description returns 0 if the range is null (Range3dIsNull), otherwise
*       sum of squared axis extents.
* @param [in] pRange  range to test
* @return squared magnitude of the diagonal vector.
* @bsimethod
+----------------------------------------------------------------------*/
double DRange3d::ExtentSquared () const
    {
    double dx, dy, dz;
    if (this->IsNull ())
        return  0.0;

    dx = (this->high.x - this->low.x);
    dy = (this->high.y - this->low.y);
    dz = (this->high.z - this->low.z);

    return  ((dx*dx) + (dy*dy) + (dz*dz));
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRange3d::IsEmpty () const
    {
    return
            high.x < low.x
        ||  high.y < low.y
        ||  high.z < low.z
        ;
    }


/*-----------------------------------------------------------------*//**
*
* @return true if high is equal to low in every direction.
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsPoint () const
    {
    return
            this->high.x == this->low.x
        &&  this->high.y == this->low.y
        &&  this->high.z == this->low.z
        ;
    }


/*-----------------------------------------------------------------*//**
* returns product of axis extents.  No test for zero or negative axes.
* @bsimethod
+----------------------------------------------------------------------*/
double DRange3d::Volume () const
    {
    if (this->IsNull ())
        return 0.0;
    return
            (this->high.x - this->low.x)
        *   (this->high.y - this->low.y)
        *   (this->high.z - this->low.z)
        ;
    }


/*-----------------------------------------------------------------*//**
* @description Initializes the range to contain the single given point.
* @param [out]pRange  the initialized range.
* @param [in] point  the point
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (DPoint3dCR point)
    {
    low = high = point;
    }


/*-----------------------------------------------------------------*//**
* @vbdescription Initializes the range to contain the two given points.
* @param [out]pRange  initialzied range.
* @param [in] point0  first point
* @param [in] point1  second point
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (DPoint3dCR point0, DPoint3dCR point1)
    {
    low = high = point0;
    Extend (point1);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (FRange3dCR fRange)
    {
    // ah, maybe you'd like to just copy the xyz values.
    // but the NullRange values are different.
    // Even when not null, do a careful Extend() to ensure validity
    auto result = NullRange ();
    if (!fRange.IsNull ())
        {
        result.Extend (DPoint3d::From (fRange.low));
        result.Extend (DPoint3d::From (fRange.high));
        }
    return result;
    }

/*-----------------------------------------------------------------*//**
* @vbdescription Initializes the range to contain two points given as components.
* Minmax logic is applied to the given points.
* @param [in] x0  first x
* @param [in] y0  first y
* @param [in] z0  first z
* @param [in] x1  second x
* @param [in] y1  second y
* @param [in] z1  second z
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (double x0, double y0, double z0, double x1, double y1, double z1)
    {
    InitFrom (x0, y0, z0);
    Extend (x1, y1,z1);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRange1d DRange3d::GetComponentDRange1d (int index) const
    {
    index = Angle::Cyclic3dAxis (index);
    double a, b;
    if (index == 0)
        {
        a = low.x;
        b = high.x;
        }
    else if (index == 1)
        {
        a = low.y;
        b = high.y;
        }
    else
        {
        a = low.z;
        b = high.z;
        }
    if (a <= b)
        return DRange1d (a,b);

    return DRange1d ();
    }

DRange1d DRange3d::GetCornerRange (DRay3dCR ray) const
    {
    DRange1d range = DRange1d ();
    DPoint3d corners[8];
    Get8Corners (corners);
    for (int i = 0; i < 8; i++)
        range.Extend (ray.DirectionDotVectorToTarget (corners[i]));
    return range;
    }

DRange1d DRange3d::GetCornerRange (DPlane3dCR plane) const
    {
    DRange1d range = DRange1d ();
    DPoint3d corners[8];
    Get8Corners (corners);
    for (int i = 0; i < 8; i++)
        range.Extend (plane.Evaluate (corners[i]));
    return range;
    }

/*-----------------------------------------------------------------*//**
* Initialize the range from a single point given by components
* @param [in] x0  x coordinate
* @param [in] y0  y coordinate
* @param [in] z0  z coordinate
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (double x, double y, double z)
    {
    low.x = x;
    low.y = y;
    low.z = z;
    high = low;
    }


/*-----------------------------------------------------------------*//**
* @description Initialize the range from given min and max in all directions.
* Given values will be swapped if needed.
* @param [in] v0 min (or max)
* @param [in] v1 max (or min)
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::InitFromMinMax (double v0, double v1)
    {
    double a, b;

    if  (v0 <= v1)
        {
        a = v0;
        b = v1;
        }
    else
        {
        a = v1;
        b = v0;
        }

    this->low.x  = a;
    this->low.y  = a;
    this->low.z  = a;

    this->high.x = b;
    this->high.y = b;
    this->high.z = b;
    }


/*-----------------------------------------------------------------*//**
* @vbdescription Initializes a range to contain three given points.
* @param [out]pRange  initialzied range.
* @param [in] point0  first point
* @param [in] point1  second point
* @param [in] point2  third point
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::InitFrom (DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2)
    {
    low = high = point0;
    Extend (point1);
    Extend (point2);
    }


/*-----------------------------------------------------------------*//**
* @description Extend each axis by the given distance on both ends of the range.
* @param [in] extend  distance to extend
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (double extend)
    {
    if (!IsNull ())
        {
        this->low.x -= extend;
        this->low.y -= extend;
        this->low.z -= extend;

        this->high.x += extend;
        this->high.y += extend;
        this->high.z += extend;
        }
    }


#ifndef MinimalRefMethods


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRange3d::Extend (DEllipse3dCR ellipse)
    {
    DRange3d ellipseRange;
    ellipse.GetRange (ellipseRange);
    Extend (ellipseRange);
    }

#endif




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRange3d::ExtendBySweep (DVec3dCR vector)
    {
    if (low.x <= high.x)
        {
        if (vector.x > 0.0)
            high.x += vector.x;
        else
            low.x += vector.x;
        }

    if (low.y <= high.y)
        {
        if (vector.y > 0.0)
            high.y += vector.y;
        else
            low.y += vector.y;
        }

    if (low.z <= high.z)
        {
        if (vector.z > 0.0)
            high.z += vector.z;
        else
            low.z += vector.z;
        }
    }



/*-----------------------------------------------------------------*//**
* @description Extends the coordinates of the range cube points in pRange so as
* to include the single additional point point.
* @param [in] point  new point to be included in the range.
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (DPoint3dCR point)
    {
    if (!point.IsDisconnect ())
        {
        FIX_MINMAX (point.x, low.x, high.x);
        FIX_MINMAX (point.y, low.y, high.y);
        FIX_MINMAX (point.z, low.z, high.z);
        }
    }


/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (FPoint3dCR point)
    {

    FIX_MINMAX ((double)point.x, low.x, high.x);
    FIX_MINMAX ((double)point.y, low.y, high.y);
    FIX_MINMAX ((double)point.z, low.z, high.z);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (FPoint3dCR pointA, FPoint3dCR pointB)
    {
    Extend (pointA);
    Extend (pointB);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (bvector<FPoint3d> const &points)
    {
    for (auto &xyz : points)
        Extend (xyz);
    }


/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (FPoint3dCR point)
    {
    return From (point.x, point.y, point.z);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (FPoint3dCR pointA, FPoint3dCR pointB)
    {
    return From (pointA.x, pointA.y, pointA.z, pointB.x, pointB.y, pointB.z);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (bvector<FPoint3d> const &points)
    {
    DRange3d range = NullRange ();
    for (auto &xyz : points)
        range.Extend (xyz.x, xyz.y, xyz.z);
    return range;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
DRange3d DRange3d::From (bvector<DSegment3d> const &segments)
    {
    DRange3d range = NullRange ();
    for (auto &segment : segments)
        {
        range.Extend (segment.point[0]);
        range.Extend (segment.point[1]);
        }
    return range;
    }

/*-----------------------------------------------------------------*//**
* @description Extends the coordinates of the range cube points in pRange so as
* to include the single additional weighted point.
* @param [in] point  new point to be included in the range.
* @param [in] weight  weight.   Point coordinates are divided by the weight.
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (DPoint3dCR point, double weight)
    {
    DPoint4d pointw;
    pointw.Init (point, weight);
    Extend (pointw);
    }


/*-----------------------------------------------------------------*//**
*
* @description Extends the coordinates of the range cube points in pRange so as
* to include the single additional point at x,y,z.
* @param [in,out] pRange range to be extended
* @param [in] x extended range coordinate
* @param [in] y extended range coordinate
* @param [in] z extended range coordinate
* @param
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (double x, double y, double z)
    {
    if (x != DISCONNECT && y != DISCONNECT && z != DISCONNECT)
        {
        FIX_MINMAX (x, low.x, high.x);
        FIX_MINMAX (y, low.y, high.y);
        FIX_MINMAX (z, low.z, high.z);
        }
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the (normalized image of) the given 4D point.
*
* @param [in] point4d new point to be included in minmax ranges
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (DPoint4dCR point4d)
    {
    double divw;
    if (DoubleOps::SafeDivide (divw, 1.0, point4d.w, 0.0))
        {
        double coord;
        coord = point4d.x * divw;
        FIX_MINMAX (coord, this->low.x, this->high.x);
        coord = point4d.y * divw;
        FIX_MINMAX (coord, this->low.y, this->high.y);
        coord = point4d.z * divw;
        FIX_MINMAX (coord, this->low.z, this->high.z);
        }
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points to
* include the range cube range1P.
*
* @param [in] range1 second range
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Extend (DRange3dCR range1)
    {
    if (!range1.IsNull ())
        {
        Extend (range1.low);
        Extend (range1.high);
        }
    }


/*-----------------------------------------------------------------*//**
* @description returns the union of two ranges.
* @param [in] range0  first range
* @param [in] range1  second range
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::UnionOf (DRange3dCR range0, DRange3dCR range1)
    {
    *this = range0;
    Extend (range1);
    }


/*-----------------------------------------------------------------*//**
* Compute the intersection of two ranges.  If any direction has no intersection
*       the result range is initialized to a null range.  (Zero thickness
*       intersection is null.)
* @param [in] range1 first range
* @param [in] range2 second range
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::IntersectionOf (DRange3dCR range1, DRange3dCR range2)
    {
    this->low.x = range1.low.x > range2.low.x ? range1.low.x : range2.low.x;
    this->low.y = range1.low.y > range2.low.y ? range1.low.y : range2.low.y;
    this->low.z = range1.low.z > range2.low.z ? range1.low.z : range2.low.z;
    this->high.x = range1.high.x < range2.high.x ? range1.high.x : range2.high.x;
    this->high.y = range1.high.y < range2.high.y ? range1.high.y : range2.high.y;
    this->high.z = range1.high.z < range2.high.z ? range1.high.z : range2.high.z;

    if (   this->low.x >= this->high.x
        || this->low.y >= this->high.y
        || this->low.z >= this->high.z
        )
        Init ();
    }


/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
DRange3d DRange3d::FromIntersection (DRange3dCR range1, DRange3dCR range2, bool zeroExtentsAreValid)
    {
    DRange3d out;
    out.low.x = range1.low.x > range2.low.x ? range1.low.x : range2.low.x;
    out.low.y = range1.low.y > range2.low.y ? range1.low.y : range2.low.y;
    out.low.z = range1.low.z > range2.low.z ? range1.low.z : range2.low.z;
    out.high.x = range1.high.x < range2.high.x ? range1.high.x : range2.high.x;
    out.high.y = range1.high.y < range2.high.y ? range1.high.y : range2.high.y;
    out.high.z = range1.high.z < range2.high.z ? range1.high.z : range2.high.z;

    if (zeroExtentsAreValid)
        {
        if (   out.low.x > out.high.x
            || out.low.y > out.high.y
            || out.low.z > out.high.z
            )
            out.Init ();
        }
    else
        {
        if (   out.low.x >= out.high.x
            || out.low.y >= out.high.y
            || out.low.z >= out.high.z
            )
            out.Init ();
        }

    return out;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
DRange3d DRange3d::FromUnion (DRange3dCR range1, DRange3dCR range2)
    {
    DRange3d out = range1;
    out.Extend (range2);
    return out;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static bool intersectInterval
(
double &result0,
double &result1,
double a0,
double a1,
double b0,
double b1
)
    {
    if (a0 <= a1 && b0 <= b1)
        {
        result0 = a0 < b0 ? b0 : a0;
        result1 = a1 < b1 ? a1 : b1;
        if (result0 <= result1)
            return true;
        }
    result0 = DBL_MAX;
    result1 = -DBL_MAX;
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRange3d::IntersectIndependentComponentsOf (DRange3dCR range1, DRange3dCR range2)
    {
    intersectInterval (low.x, high.x, range1.low.x, range1.high.x, range2.low.x, range2.high.x);
    intersectInterval (low.y, high.y, range1.low.y, range1.high.y, range2.low.y, range2.high.y);
    intersectInterval (low.z, high.z, range1.low.z, range1.high.z, range2.low.z, range2.high.z);
    }




/*-----------------------------------------------------------------*//**
* @description Test if the first range is contained in the second range.
* @param [in] pInnerRange  candidate inner range.
* @param [in] outerRange  candidate outer range.
* @return true if the inner range is a (possibly improper) subset of the outer range.
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsContained (DRange3dCR outerRange) const
    {
    return  (this->low.x >= outerRange.low.x
             && this->low.y >= outerRange.low.y
             && this->low.z >= outerRange.low.z
             && this->high.x <= outerRange.high.x
             && this->high.y <= outerRange.high.y
             && this->high.z <= outerRange.high.z);    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRange3d::IsContained (DPoint3dCR point) const
    {
    return      point.x >= low.x
             && point.y >= low.y
             && point.z >= low.z
             && point.x <= high.x
             && point.y <= high.y
             && point.z <= high.z;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRange3d::IsContained (DPoint3dCR point, int numDimensions) const
    {
    if (point.x < low.x)
        return false;
    if (point.x > high.x)
        return false;
    if (numDimensions < 2)
        return true;

    if (point.y < low.y)
        return false;
    if (point.y > high.y)
        return false;
    if (numDimensions < 3)
        return true;

    if (point.z < low.z)
        return false;
    if (point.z > high.z)
        return false;
    return true;
    }







/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRange3d::IsContainedXY (DPoint3dCR point) const
    {
    return      point.x >= low.x
             && point.y >= low.y
             && point.x <= high.x
             && point.y <= high.y;
    }



/*-----------------------------------------------------------------*//**
* @description Test if a point given as x,y,z is contained in a range.
* @param [in] Range  candidate containing range.
* @param [in] x  x coordinate
* @param [in] y  y coordinate
* @param [in] z  z coordinate
* @return true if the point is in (or on boundary of)
* @indexVerb containment
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsContained (double ax, double ay, double az) const
    {
    return  (   ax >= this->low.x
             && ay >= this->low.y
             && az >= this->low.z
             && ax <= this->high.x
             && ay <= this->high.y
             && az <= this->high.z);
    }


/*-----------------------------------------------------------------*//**
* Test if two ranges are exactly equal.
* @param [in] pRange0  first range
* @param [in] range1  second range
* @param [in] tolerance  toleranc to be applied to each component
* @return true if ranges are identical in all components.
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsEqual (DRange3dCR range1) const
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


/*-----------------------------------------------------------------*//**
* Test if two ranges are equal within a tolerance applied componentwise.
* @param [in] pRange0  first range
* @param [in] range1  second range
* @param [in] tolerance  toleranc to be applied to each component
* @return true if ranges are within tolerance in all components.
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsEqual (DRange3dCR range1, double tolerance) const
    {
    return
           fabs (this->low.x  - range1.low.x ) <= tolerance
        && fabs (this->low.y  - range1.low.y ) <= tolerance
        && fabs (this->low.z  - range1.low.z ) <= tolerance
        && fabs (this->high.x - range1.high.x) <= tolerance
        && fabs (this->high.y - range1.high.y) <= tolerance
        && fabs (this->high.z - range1.high.z) <= tolerance
        ;    }


/*-----------------------------------------------------------------*//**
*
* Test if the given range is a proper subset of outerRange, using only xy parts
*
* @param [in] outerRange outer range
* @return true if the given range is a proper subset of
*   outerRange.
* @indexVerb containment
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IsStrictlyContainedXY (DRange3dCR outerRange) const
    {
    return      this->low.x  > outerRange.low.x
             && this->low.y  > outerRange.low.y
             && this->high.x < outerRange.high.x
             && this->high.y < outerRange.high.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlDRange3d_restrictIntervalToMinMax

(
double          *pStart,
double          *pEnd,
double          start0,
double          end0,
double          min1,
double          max1
)

    {
    if (start0 < min1)
                {
                *pStart = min1;
                }
    else if (start0 > max1)
                {
                *pStart = max1;
                }
    else
                {
                *pStart = start0;
                }

    if (end0 < min1)
                {
                *pEnd = min1;
                }
    else if (end0 > max1)
                {
                *pEnd = max1;
                }
    else
                {
                *pEnd = end0;
                }

    }


/*-----------------------------------------------------------------*//**
*
* Returns a range which is the intersection of two ranges.  The first
* range is treated as a signed range, i.e. decreasing values from low
* to high are a nonempty range, and the output will maintain the
* direction.
* In a direction where there is no overlap, pRange high and low values
* are identical and are at the limit of pRange1 that is nearer to the
* values in range0.
* (Intended use: range0 is the 'actual' stroking range of a surface
*   i.e. may go 'backwards'.  pRange1 is the nominal full surface range,
*   i.e. is known a priori to be 'forwards'.  The clipping restricts
*   unreliable range0 to the nominal surface range pRange1.
* range0 and pRange may be the same address.  minMax must be different.
*
* @param [in] range0 range to be restricted
* @param [in] minMax allowable minmax range.  Assumed to have low < high
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::RestrictToMinMax (DRange3dCR range0, DRange3dCR minMax)
    {
    jmdlDRange3d_restrictIntervalToMinMax
            (
            &this->low.x, &this->high.x,
            range0.low.x, range0.high.x,
            minMax.low.x, minMax.high.x
            );

    jmdlDRange3d_restrictIntervalToMinMax
            (
            &this->low.y, &this->high.y,
            range0.low.y, range0.high.y,
            minMax.low.y, minMax.high.y
            );

    jmdlDRange3d_restrictIntervalToMinMax
            (
            &this->low.z, &this->high.z,
            range0.low.z, range0.high.z,
            minMax.low.z, minMax.high.z
            );
    }


/*-----------------------------------------------------------------*//**
* @description scale a range about its center point.
* @param [out]pResultRange  scaled range.
* @param [in] rangeIn  original range
* @param [in] scale  scale factor
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::ScaleAboutCenter (DRange3dCR rangeIn, double scale)
    {
    DRange3d result = rangeIn;
    if (!result.IsNull())
        {
        double f = 0.5 * (1.0 + scale);
        result.high.Interpolate (rangeIn.low, f, rangeIn.high);
        result.low.Interpolate (rangeIn.high, f, rangeIn.low);
        }
    *this = result;
    }


/*-----------------------------------------------------------------*//**
*
* Extract the 6 bounding planes for a range cube.
*
* @param [out] originArray array of plane origins
* @param [out] normalArray array of plane normals
* @bsimethod
+----------------------------------------------------------------------*/
void DRange3d::Get6Planes (DPoint3dP originArray, DPoint3dP normalArray) const
    {
    DRange3d minMaxRange;
    minMaxRange.InitFrom (this->low, this->high);   // fix reverse direction?
    originArray[0] = originArray[1] = originArray[2] = minMaxRange.low;
    originArray[3] = originArray[4] = originArray[5] = minMaxRange.high;

    normalArray[0].Init (-1.0, 0.0, 0.0);
    normalArray[3].Init ( 1.0, 0.0, 0.0);

    normalArray[1].Init ( 0.0, -1.0, 0.0);
    normalArray[4].Init ( 0.0,  1.0, 0.0);

    normalArray[2].Init ( 0.0, 0.0, -1.0);
    normalArray[5].Init ( 0.0, 0.0,  1.0);
    }


/*-----------------------------------------------------------------*//**
*
* Return the index of the axis with largest absolute range.
*
* @indexVerb extrema
* @bsimethod
+----------------------------------------------------------------------*/
int DRange3d::IndexOfMaximalAxis () const
    {
    double a, aMax;
    int    index = 0;
    aMax = fabs (high.x - low.x);

    a = fabs (high.y - low.y);
    if (a > aMax)
        {
        aMax = a;
        index = 1;
        }

    a = fabs (high.z - low.z);
    if (a > aMax)
        {
        aMax = a;
        index = 2;
        }

    return index;
    }


/*-----------------------------------------------------------------*//**
*
* Compute the intersection of a range cube and a ray.
*
* If there is not a finite intersection, both params are set to 0 and
* and both points to point0.
*
* @param [out] param0 ray parameter where cube is entered
* @param [out] param1 ray parameter where cube is left
* @param [out] point0 entry point
* @param [out] point1 exit point
* @param [in] start start point of ray
* @param [in] direction direction of ray
* @return true if non-empty intersection.
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IntersectRay (double &param0, double &param1, DPoint3dR point0, DPoint3dR point1, DPoint3dCR start, DPoint3dCR direction) const
    {
    bool    boolStat;
    /* save points in case of duplicate pointers by caller */
    DRange1d rayRange = DRange1d::InfiniteRange ();
    if ( rayRange.UpdateRay1dIntersection (
                start.x, direction.x,
                low.x, high.x
                )
        && rayRange.UpdateRay1dIntersection  (
                start.y, direction.y,
                low.y, high.y
                )
        && rayRange.UpdateRay1dIntersection (
                start.z, direction.z,
                low.z, high.z
                )
        )
        {
        param0 = rayRange.low;
        param1 = rayRange.high;
        boolStat = true;
        }
    else
        {
        param0 = 0.0;
        param1 = 0.0;
        boolStat = false;
        }

    point0.SumOf (start, direction, param0);
    point1.SumOf (start, direction, param1);

    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
*
* Compute the intersection of a range cube and a ray.
*
* If there is not a finite intersection, both params are set to 0 and
* and the output segment consists of only the start point.
* @param [out] param0 ray parameter where cube is entered
* @param [out] param1 ray parameter where cube is left
* @param [out] clipped clipped segment
* @param
* @return true if non-empty intersection.
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IntersectBounded (double &param0, double &param1, DSegment3dR clipped, DSegment3dCR segment) const
    {
    DVec3d direction = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
    DPoint3d point0, point1;
    bool unboundedStat = IntersectRay (param0, param1, point0, point1,
                segment.point[0], direction);
    bool boolstat = false;
    if (unboundedStat)
        {
        if (param1 > 1.0)
            param1 = 1.0;
        if (param0 < 0.0)
            param0 = 0.0;

        if (param1 > param0)
            {
            boolstat = true;
            }
        }

    if (!boolstat)
        {
        param0 = param1 = 0.0;
        }

    clipped.point[0].SumOf (segment.point[0], direction, param0);
    clipped.point[1].SumOf (segment.point[0], direction, param1);
    return boolstat;
    }


/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @bsimethod
+----------------------------------------------------------------------*/
double DRange3d::LargestCoordinate () const
    {
    if (IsNull ())
        return 0.0;
    DVec3d diagonal = DVec3d::FromStartEnd (low, high);
    return DoubleOps::Max
        ( low.MaxAbs (), high.MaxAbs(), diagonal.MaxAbs ());
    }

double DRange3d::MaxAbs () const
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
double DRange3d::LargestCoordinateXY () const
    {
    if (IsNull ())
        return 0.0;
    DVec3d diagonal = DVec3d::FromStartEnd (low, high);
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
void DRange3d::Get8Corners (DPoint3dP box) const
    {
    DPoint3d minmax[2];
    int ix,iy,iz,i;
    minmax[0] = low;
    minmax[1] = high;
    i = 0;
    for( iz = 0; iz < 2; iz++ )
        for ( iy = 0; iy < 2; iy++ )
            for ( ix = 0; ix < 2; ix++ )
                {
                box[i].Init (minmax[ix].x, minmax[iy].y, minmax[iz].z );
                i++;
                }
    }


/*-----------------------------------------------------------------*//**
*
* Compute the intersection of given range with another range and return the
* extentSquared of the intersection range.
*
* @param [in] range2 second range
* @return extentSquared() for the intersection range.
* @indexVerb intersection
* @bsimethod
+----------------------------------------------------------------------*/
double DRange3d::IntersectionExtentSquared (DRange3dCR range2) const
    {
    DRange3d overlapRange;
    overlapRange.low.x = this->low.x > range2.low.x ? this->low.x : range2.low.x;
    overlapRange.low.y = this->low.y > range2.low.y ? this->low.y : range2.low.y;
    overlapRange.low.z = this->low.z > range2.low.z ? this->low.z : range2.low.z;
    overlapRange.high.x = this->high.x < range2.high.x ? this->high.x : range2.high.x;
    overlapRange.high.y = this->high.y < range2.high.y ? this->high.y : range2.high.y;
    overlapRange.high.z = this->high.z < range2.high.z ? this->high.z : range2.high.z;

    return  overlapRange.ExtentSquared ();
    }



/*-----------------------------------------------------------------*//**
*
* Test if two ranges have strictly non-null overlap (intersection)
*
* @param [in] pRange1 first range
* @param [in] range2 second range
* @return true if ranges overlap, false if not.
* @indexVerb intersection
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::IntersectsWith (DRange3dCR range2) const
    {
    if (this->low.x > range2.high.x
        || this->low.y > range2.high.y
        || this->low.z > range2.high.z
        || range2.low.x > this->high.x
        || range2.low.y > this->high.y
        || range2.low.z > this->high.z)
        return  false;
    else
        return  true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRange3d::IntersectsWith (DRange3dCR other, int numDimensions) const
    {
    if (low.x > other.high.x || high.x < other.low.x)
        return false;
    if (numDimensions < 2)
        return true;

    if (low.y > other.high.y || high.y < other.low.y)
        return false;
    if (numDimensions < 3)
        return true;

    if (low.z > other.high.z || high.z < other.low.z)
        return false;
    return true;

    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DRange3d::IntersectsWith (DRange3dCR other, double gapSize, int numDimensions) const
    {
    if (gapSize == DBL_MAX)
        {
        return !IsEmpty () && !other.IsEmpty ();
        }

    if (low.x > other.high.x + gapSize || high.x + gapSize < other.low.x)
        return false;
    if (numDimensions < 2)
        return true;

    if (low.y > other.high.y  + gapSize|| high.y  + gapSize< other.low.y)
        return false;
    if (numDimensions < 3)
        return true;

    if (low.z > other.high.z  + gapSize|| high.z  + gapSize < other.low.z)
        return false;
    return true;

    }




/*-----------------------------------------------------------------*//**
*
* Test if a modification of the given (instance) range would have a different
* touching relationship with outerRange.
*
* @remark This may only be meaningful in context of range tree tests where
*   some prior relationship among ranges is known to apply.
*
* @param [in] newRange candidate for modified range relationship.
* @param [in] pOuterRnage containing range
* @return true if touching condition occurs.
* @indexVerb extend
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange3d::MoveChangesIntersection (DRange3dCR newRange, DRange3dCR outerRange) const
    {
    if ((this->low.x != newRange.low.x) && (this->low.x == outerRange.low.x))
        return  true;

    if ((this->low.y != newRange.low.y) && (this->low.y == outerRange.low.y))
        return  true;

    if ((this->low.z != newRange.low.z) && (this->low.z == outerRange.low.z))
        return  true;

    if ((this->high.x != newRange.high.x) && (this->high.x == outerRange.high.x))
        return  true;

    if ((this->high.y != newRange.high.y) && (this->high.y == outerRange.high.y))
        return  true;

    if ((this->high.z != newRange.high.z) && (this->high.z == outerRange.high.z))
        return  true;

    return  false;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3d DRange3d::LocalToGlobal (double xFraction, double yFraction, double zFraction) const
    {
    return DPoint3d::FromXYZ (
        DoubleOps::Interpolate (low.x, xFraction, high.x),
        DoubleOps::Interpolate (low.y, yFraction, high.y),
        DoubleOps::Interpolate (low.z, zFraction, high.z));
    }

//!
//!
//! Generates 6 planes for the faces of the box.
//!
//! @param [out] planes array of 6 planes
//! @param [in] normalLength scale factor for plane normals.  1.0 is outward unit normals, -1.0 is inward unit normals
//!
void DRange3d::Get6Planes (DPlane3d planes[6], double normalLength) const
    {
    planes[0] = DPlane3d::FromOriginAndNormal (low.x, low.y, low.z, -normalLength, 0, 0);
    planes[1] = DPlane3d::FromOriginAndNormal (low.x, low.y, low.z, 0, -normalLength, 0);
    planes[2] = DPlane3d::FromOriginAndNormal (low.x, low.y, low.z, 0, 0, -normalLength);
    planes[3] = DPlane3d::FromOriginAndNormal (high.x, high.y, high.z, normalLength, 0, 0);
    planes[4] = DPlane3d::FromOriginAndNormal (high.x, high.y, high.z, 0, normalLength, 0);
    planes[5] = DPlane3d::FromOriginAndNormal (high.x, high.y, high.z, 0, 0, normalLength);
    }

//!
//!
//! Generates 12 edges.
//!
void DRange3d::GetEdges (bvector<DSegment3d> &edges) const
    {
    edges.clear ();
    if (IsNull ())
        return;
    DPoint3d corners[8];
    Get8Corners (corners);
    edges.push_back (DSegment3d::From (corners[0], corners[1]));
    edges.push_back (DSegment3d::From (corners[2], corners[3]));
    edges.push_back (DSegment3d::From (corners[4], corners[5]));
    edges.push_back (DSegment3d::From (corners[6], corners[7]));

    edges.push_back (DSegment3d::From (corners[0], corners[2]));
    edges.push_back (DSegment3d::From (corners[1], corners[3]));
    edges.push_back (DSegment3d::From (corners[4], corners[6]));
    edges.push_back (DSegment3d::From (corners[5], corners[7]));

    edges.push_back (DSegment3d::From (corners[0], corners[4]));
    edges.push_back (DSegment3d::From (corners[1], corners[5]));
    edges.push_back (DSegment3d::From (corners[2], corners[6]));
    edges.push_back (DSegment3d::From (corners[3], corners[7]));
    }
END_BENTLEY_GEOMETRY_NAMESPACE

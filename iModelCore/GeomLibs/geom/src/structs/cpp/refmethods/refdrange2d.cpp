/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

#define PANIC_SIZE 1.0e100


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange2d::XLength () const {double a = high.x - low.x; return a > 0.0 ? a : 0.0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DRange2d::YLength () const {double a = high.y - low.y; return a > 0.0 ? a : 0.0;}

/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param
+----------------------------------------------------------------------*/
DRange2d DRange2d::NullRange
(
)
    {
    DRange2d range;
    range.Init ();
    return range;
    }

/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the single given point.
*
+----------------------------------------------------------------------*/
DRange2d DRange2d::From
(
DPoint2dCR      point
)
    {
    DRange2d range;
    range.InitFrom (point);
    return range;
    }

/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the 3d range xy
*
+----------------------------------------------------------------------*/
DRange2d DRange2d::From (DRange3dCR source)
    {
    DRange2d range = DRange2d::NullRange ();
    if (!source.IsNull ())
        {
        range.Extend (source.low);
        range.Extend (source.high);
        }
    return range;
    }

/*-----------------------------------------------------------------*//**
*
* Return the range of an array.
*
+----------------------------------------------------------------------*/
DRange2d DRange2d::From (bvector<DPoint3d> const &data)
    {
    DRange2d range;
    range.Init ();
    for (auto &item : data)
        range.Extend (item);
    return range;
    }

/*-----------------------------------------------------------------*//**
* Initializes the range to contain the two given points.
* @param [in] point0 first point
* @param [in] point1 second point
+----------------------------------------------------------------------*/
DRange2d DRange2d::From
(
DPoint2dCR      point0,
DPoint2dCR      point1
)
    {
    DRange2d range;
    range.InitFrom (point0, point1);
    return range;
    }

/*-----------------------------------------------------------------*//**
* Initializes the range to contain two points given as components.
* Minmax logic is applied to the given points.
* @param [in] x0 first x
* @param [in] y0 first y
* @param [in] x1 second x
* @param [in] y1 second y
+----------------------------------------------------------------------*/
DRange2d DRange2d::From
(
double          x0,
double          y0,
double          x1,
double          y1
)
    {
    DRange2d range;
    range.InitFrom (x0, y0, x1, y1);
    return range;
    }

/*-----------------------------------------------------------------*//**
* Initialize the range from a single point given by components
* @param [in] x x coordinate
* @param [in] y y coordinate
+----------------------------------------------------------------------*/
DRange2d DRange2d::From
(
double          x,
double          y
)
    {
    DRange2d range;
    range.InitFrom (x, y);
    return range;
    }

/*-----------------------------------------------------------------*//**
* Initialize the range from an arc of the unit circle
* @param [in] theta0 start angle
* @param [in] sweep angular sweep
+----------------------------------------------------------------------*/
DRange2d DRange2d::FromUnitArcSweep
(
double          theta0,
double          sweep
)
    {
    DRange2d range;
    range.InitFromUnitArcSweep (theta0, sweep);
    return range;
    }

/*-----------------------------------------------------------------*//**
* Initialize the range to contain the three given points.
* @param [in] point0 first point
* @param [in] point1 second point
* @param [in] point2 third point
* @param
+----------------------------------------------------------------------*/
DRange2d DRange2d::From
(
DPoint2dCR      point0,
DPoint2dCR      point1,
DPoint2dCR      point2
)
    {
    DRange2d range;
    range.InitFrom (point0, point1, point2);
    return range;
    }

/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange2d.init()
*
* @param [in] point array of points to search
* @param [in] n number of points in array
+----------------------------------------------------------------------*/
DRange2d DRange2d::From
(
DPoint2dCP      point,
int             n
)
    {
    DRange2d range;
    range.InitFrom (point, n);
    return range;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRange2d DRange2d::From (bvector<DPoint2d> const &points)
    {
    DRange2d range = DRange2d::NullRange ();
    if (points.size () > 0)
        range.Extend (&points[0], (int)points.size ());
    return range;
    }

/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the range of the xy parts of
* the array of 3D points.
*
* @param [in] point array of points to search
* @param [in] n number of points in array
+----------------------------------------------------------------------*/
DRange2d DRange2d::From
(
DPoint3dCP      point,
int             n
)
    {
    DRange2d range;
    range.InitFrom (point, n);
    return range;
    }



/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Init
(
/* <= range to be initialized */

)
    {
    low.x = low.y = DBL_MAX;
    high.x = high.y = -DBL_MAX;
    }


/*-----------------------------------------------------------------*//**
*
* Check if the range is exactly the same as the null ranges returned
* by bsiDRange2d_init.  (Note that ranges with other values with low > high
* are not necessarily null by this condition.)
*
* @see bsiDRange2d_isEmpty
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IsNull
(

) const
    {
    if (
           low.x == DBL_MAX
        && low.y == DBL_MAX
        && high.x == -DBL_MAX
        && high.y == -DBL_MAX
        )
        {
        return true;
        }
    else if (
           fabs(low.x) < PANIC_SIZE
        && fabs(low.y) < PANIC_SIZE
        && fabs(high.x) < PANIC_SIZE
        && fabs(high.y) < PANIC_SIZE
        )
        {
        return false;
        }
    else
        {
        /* It doesn't match the definition of a null, but it is definitely not normal.
         This is a good place for a breakpoint!!*/
        return true;
        }
    }


/*-----------------------------------------------------------------*//**
* @return 0 if null range (as decided by ~mbsiDRange2d_isNull), otherwise
*       sum of squared axis extents.
* @see bsiDRange2d_isNull
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
double DRange2d::ExtentSquared
(

) const
    {
    double dx, dy;
    if (IsNull ())
        return  0.0;

    dx = (high.x - low.x);
    dy = (high.y - low.y);

    return  dx*dx + dy*dy;
    }


/*-----------------------------------------------------------------*//**
*
* Test if high component is (strictly) less than low in any direction.
* Note that equal components do not indicate empty.
*
* returns true if any low component is less than the corresponding high component
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IsEmpty
(

) const
    {
    return
            high.x < low.x
        ||  high.y < low.y
        ;
    }


/*-----------------------------------------------------------------*//**
*
* @return true if high is less than or equal to low in every direction.
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IsPoint
(

) const
    {
    return
            high.x == low.x
        &&  high.y == low.y
        ;
    }


/*-----------------------------------------------------------------*//**
* returns product of axis extents.  No test for zero or negative axes.
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
double DRange2d::Area
(

) const
    {
    return
            (high.x - low.x)
        *   (high.y - low.y)
        ;
    }


/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the single given point.
*
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFrom
(

DPoint2dCR point

)
    {
    low = high = point;
    }


/*-----------------------------------------------------------------*//**
* Initializes the range to contain the two given points.
* @param [in] point0 first point
* @param [in] point1 second point
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFrom
(

DPoint2dCR point0,
DPoint2dCR point1

)
    {
    low = high = point0;

    FIX_MINMAX (point1.x, low.x, high.x);
    FIX_MINMAX (point1.y, low.y, high.y);
    }


/*-----------------------------------------------------------------*//**
* Initializes the range to contain two points given as components.
* Minmax logic is applied to the given points.
* @param [in] x0 first x
* @param [in] y0 first y
* @param [in] x1 second x
* @param [in] y1 second y
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFrom
(

double          x0,
double          y0,
double          x1,
double          y1

)
    {
    low.x = x0;
    low.y = y0;
    high = low;

    FIX_MINMAX (x1, low.x, high.x);
    FIX_MINMAX (y1, low.y, high.y);
    }


/*-----------------------------------------------------------------*//**
* Initialize the range from a single point given by components
* @param [in] x0 x coordinate
* @param [in] y0 y coordinate
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFrom
(

double          x,
double          y

)
    {
    low.x = high.x = x;
    low.y = high.y = y;
    }


/*-----------------------------------------------------------------*//**
* Initialize the range from an arc of the unit circle
* @param [in] theta0 start angle
* @param [in] sweep angular sweep
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFromUnitArcSweep
(
double          theta0,
double          sweep
)
    {
    double theta1 = theta0 + sweep;
    if (Angle::IsFullCircle (sweep))
        {
        InitFrom (-1.0, -1.0, 1.0, 1.0);
        }
    else
        {
        DPoint2d testPoint;

        testPoint.x = cos (theta0);
        testPoint.y = sin (theta0);
        low = high = testPoint;
        Extend (cos (theta1), sin (theta1));

        /* Force the range out to the axis extremes if they are in the sweep */
        if (Angle::InSweepAllowPeriodShift (0.0,              theta0, sweep))
            high.x = 1.0;

        if (Angle::InSweepAllowPeriodShift (msGeomConst_pi,  theta0, sweep))
            low.x = -1.0;

        if (Angle::InSweepAllowPeriodShift (msGeomConst_piOver2, theta0, sweep))
            high.y = 1.0;

        if (Angle::InSweepAllowPeriodShift (-msGeomConst_piOver2, theta0, sweep))
            low.y = -1.0;
        }
    }


/*-----------------------------------------------------------------*//**
* Initialize the range to contain the three given points.
* @param [in] point0 first point
* @param [in] point1 second point
* @param [in] point2 third point
* @param
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFrom
(

DPoint2dCR point0,
DPoint2dCR point1,
DPoint2dCR point2

)
    {
    low = high = point0;

    FIX_MINMAX (point1.x, low.x, high.x);
    FIX_MINMAX (point1.y, low.y, high.y);

    FIX_MINMAX (point2.x, low.x, high.x);
    FIX_MINMAX (point2.y, low.y, high.y);
    }


/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange2d.init()
*
* @param [in] point array of points to search
* @param [in] n number of points in array
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFrom
(

DPoint2dCP pPoint,
        int             n

)
    {
    if (n < 1)
        {
        Init ();
        }
    else
        {
        low = high = pPoint[0];
        for (int i = 1; i < n; i++)
            {
            FIX_MINMAX ( pPoint[i].x, low.x, high.x );
            FIX_MINMAX ( pPoint[i].y, low.y, high.y );
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the range of the xy parts of
* the array of 3D points.
*
* @param [in] point array of points to search
* @param [in] n number of points in array
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::InitFrom (DPoint3dCP pPoint, int n)
    {
    if (n < 1)
        {
        Init ();
        }
    else
        {
        low.Init (pPoint[0]);
        high = low;
        for (int i = 1; i < n; i++)
            {
            FIX_MINMAX ( pPoint[i].x, low.x, high.x );
            FIX_MINMAX ( pPoint[i].y, low.y, high.y );
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Extend each axis by the given distance on both ends of the range.
*
* @param [in] extend distance to extend
* @indexVerb extend
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Extend (double extend)
    {
    low.x -= extend;
    low.y -= extend;

    high.x += extend;
    high.y += extend;
    }


/*-----------------------------------------------------------------*//**
*
* Extends the coordinates of the range cube points in pRange so as
* to include the single additional point point.
*
* @param [in] point new point to be included in the range.
* @indexVerb extend
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Extend
(

DPoint2dCR point

)
    {
    FIX_MINMAX (point.x, low.x, high.x);
    FIX_MINMAX (point.y, low.y, high.y);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DRange2d::Extend (DPoint3dCR point)
    {
    FIX_MINMAX (point.x, low.x, high.x);
    FIX_MINMAX (point.y, low.y, high.y);
    }



/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the single additional point at x,y.
*
* @param [in,out] pRange range to be extended
* @param [in] x extended range coordinate
* @param [in] y extended range coordinate
* @param
* @indexVerb extend
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Extend
(

double      x,
double      y

)
    {
    FIX_MINMAX (x, low.x, high.x);
    FIX_MINMAX (y, low.y, high.y);
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the (normalized image of) the xy projection of the 4D point.
*
* @param [in] point4d new point to be included in minmax ranges
* @indexVerb extend
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Extend
(

DPoint4dCR point4d

)
    {
    if (point4d.w != 0.0)
        {
        double coord;
        coord = point4d.x / point4d.w;
        FIX_MINMAX (coord, low.x, high.x);
        coord = point4d.y / point4d.w;
        FIX_MINMAX (coord, low.y, high.y);
        }
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include range of an array of points.
*
* @param [in] array new points to be included in minmax ranges
* @param [in] n number of points
* @indexVerb extend
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Extend
(

DPoint2dCP pPoint,
        int             n

)
    {
    for (; n-- > 0; pPoint++)
        {
        if (!pPoint->IsDisconnect ())
            {
            FIX_MINMAX (pPoint->x, low.x, high.x);
            FIX_MINMAX (pPoint->y, low.y, high.y);
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points to
* include the range cube range1P.
*
* @param [in] range1 second range
* @indexVerb extend
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Extend
(
DRange2dCR range1
)
    {
    if (!range1.IsNull ())
        {
        Extend (range1.low);
        Extend (range1.high);
        }
    }


/*-----------------------------------------------------------------*//**
* Compute the intersection of two ranges and test if it is nonempty.
*
* @param [in] range1 first range
* @param [in] range2 second range
* @return same result as checkOverlap(range1,range2).
* @indexVerb intersect
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IntersectionOf
(
DRange2dR range1,
DRange2dR range2
)
    {
    if (range1.IntersectsWith (range2))
        {
        low.x = range1.low.x > range2.low.x ? range1.low.x : range2.low.x;
        low.y = range1.low.y > range2.low.y ? range1.low.y : range2.low.y;
        high.x = range1.high.x < range2.high.x ? range1.high.x : range2.high.x;
        high.y = range1.high.y < range2.high.y ? range1.high.y : range2.high.y;

        return true;
        }
    Init ();
    return false;
    }


/*-----------------------------------------------------------------*//**
*
* Form the union of two ranges.
*
* @param [in] range1 first range.
* @param [in] range2 second range.
* @indexVerb union
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::UnionOf
(

DRange2dCR range1,
DRange2dCR range2

)
    {
    low.x = range1.low.x < range2.low.x ? range1.low.x : range2.low.x;
    low.y = range1.low.y < range2.low.y ? range1.low.y : range2.low.y;
    high.x = range1.high.x > range2.high.x ? range1.high.x : range2.high.x;
    high.y = range1.high.y > range2.high.y ? range1.high.y : range2.high.y;
    }


/*-----------------------------------------------------------------*//**
*
* Test if the given range is a (possible improper) subset of outerRange.
*
* @param [in] outerRange outer range
* @return true if the given range is a (possibly improper) subset of
*   outerRange.
* @indexVerb containment
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IsContained
(

DRange2dCR outerRange

) const
    {
    return      low.x >= outerRange.low.x
             && low.y >= outerRange.low.y
             && high.x <= outerRange.high.x
             && high.y <= outerRange.high.y;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    restrictIntervalToMinMax

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
* @indexVerb intersection
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::RestrictToMinMax
(
DRange2dCR range0,
DRange2dCR minMax
)
    {
    restrictIntervalToMinMax
            (
            &low.x, &high.x,
            range0.low.x, range0.high.x,
            minMax.low.x, minMax.high.x
            );

    restrictIntervalToMinMax
            (
            &low.y, &high.y,
            range0.low.y, range0.high.y,
            minMax.low.y, minMax.high.y
            );
    }

/*-----------------------------------------------------------------*//**
* @description scale a range about its center point.
* @param [out]pResultRange  scaled range.
* @param [in] rangeIn  original range
* @param [in] scale  scale factor
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::ScaleAboutCenter (DRange2dCR rangeIn, double scale)
    {
    DRange2d result = rangeIn;
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
* Extract the 4 bounding lines for a range rectangle, in origin normal form
*
* @param [out] originArray array of line origins
* @param [out] normalArray array of plane normals. Directions down, left, right, up.
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Get4Lines
(

DPoint2dP originArray,
DPoint2dP normalArray

) const
    {
    DRange2d minMaxRange;

    minMaxRange.InitFrom (low, high);
    originArray[0] = originArray[1] =  minMaxRange.low;
    originArray[2] = originArray[3] =  minMaxRange.high;


    normalArray[0].Init ( -1.0, 0.0);
    normalArray[2].Init (  1.0, 0.0);

    normalArray[1].Init (  0.0, -1.0);
    normalArray[3].Init (  0.0,  1.0);
    }


/*-----------------------------------------------------------------*//**
*
* Return the index of the axis with largest absolute range.
*
* @indexVerb extrema
* @bsimethod
+----------------------------------------------------------------------*/
int DRange2d::IndexOfMaximalAxis
(

) const
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
* @indexVerb intersect
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IntersectRay
(
double      &param0,
double      &param1,
DPoint2dR point0,
DPoint2dR point1,
DPoint2dCR start,
DPoint2dCR direction
) const
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
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @indexVerb extrema
* @bsimethod
+----------------------------------------------------------------------*/
double DRange2d::LargestCoordinate () const
    {
    double     max;
    DPoint2d diagonal;

    max = fabs(low.x);
    FIX_MAX(fabs(high.x), max);
    FIX_MAX(fabs(low.y), max);
    FIX_MAX(fabs(high.y), max);

    diagonal.DifferenceOf (high, low);

    FIX_MAX(fabs(diagonal.x), max);
    FIX_MAX(fabs(diagonal.y), max);

    return max;
    }


/*-----------------------------------------------------------------*//**
*
* Generates an 8point box around around a range cube.  Point ordering is by
* "x varies fastest" --- 00, 10, 01, 11 for the unit range.
* @param [out] box array of 4 points of the box
* @indexVerb boxCorners
* @bsimethod
+----------------------------------------------------------------------*/
void DRange2d::Get4Corners
(

DPoint2dP pBoxPoints

) const
    {
    pBoxPoints[0] = pBoxPoints[1] = low;
    pBoxPoints[2] = pBoxPoints[3] = high;
    pBoxPoints[1].x = pBoxPoints[3].x;
    pBoxPoints[2].x = pBoxPoints[0].x;
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
double DRange2d::IntersectionExtentSquared
(

DRange2dCR range2

) const
    {
    DRange2d overlapRange;
    overlapRange.low.x = low.x > range2.low.x ? low.x : range2.low.x;
    overlapRange.low.y = low.y > range2.low.y ? low.y : range2.low.y;
    overlapRange.high.x = high.x < range2.high.x ? high.x : range2.high.x;
    overlapRange.high.y = high.y < range2.high.y ? high.y : range2.high.y;

    return  overlapRange.ExtentSquared ();
    }


/*-----------------------------------------------------------------*//**
*
* Test if two ranges have strictly non-null overlap (intersection)
*
* @param [in] pRange1 first range
* @param [in] range2 second range
* @return true if ranges overlap, false if not.
* @indexVerb intersectioin
* @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IntersectsWith (DRange2dCR range2) const
    {
    if (   low.x > range2.high.x
        || low.y > range2.high.y
        || range2.low.x > high.x
        || range2.low.y > high.y)
        return  false;
    else
        return  true;
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
bool DRange2d::MoveChangesIntersection
(
DRange2dCR newRange,
DRange2dCR outerRange
) const
    {
    if ((low.x != newRange.low.x) && (low.x == outerRange.low.x))
        return  true;

    if ((low.y != newRange.low.y) && (low.y == outerRange.low.y))
        return  true;

    if ((high.x != newRange.high.x) && (high.x == outerRange.high.x))
        return  true;

    if ((high.y != newRange.high.y) && (high.y == outerRange.high.y))
        return  true;

    return  false;
    }


/*-----------------------------------------------------------------*//**
 @description Test if a point is contained in a range.
 @param [in] pRange  candidate containing range.
 @param [in] point  point to test. (z is ignored)
 @return true if the point is in (or on boundary of)
 @group "DRange2d Queries"
 @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::Contains (DPoint3dCR point) const
    {
    return  (   point.x >= low.x
             && point.y >= low.y
             && point.x <= high.x
             && point.y <= high.y
             );
    }


/*-----------------------------------------------------------------*//**
 @description Test if a point is contained in a range.
 @param [in] pRange  candidate containing range.
 @param [in] point  point to test. (z is ignored)
 @return true if the point is in (or on boundary of)
 @group "DRange2d Queries"
 @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::Contains (DPoint2dCR point) const
    {
    return  (   point.x >= low.x
             && point.y >= low.y
             && point.x <= high.x
             && point.y <= high.y
             );
    }


/*-----------------------------------------------------------------*//**
 @description Test if a point given as x,y,z is contained in a range.
 @param [in] pRange  candidate containing range.
 @param [in] x  x coordinate
 @param [in] y  y coordinate
 @param [in] z  z coordinate
 @return true if the point is in (or on boundary of)
 @group "DRange2d Queries"
 @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::Contains (double x, double y) const
    {
    return  (   x >= low.x
             && y >= low.y
             && x <= high.x
             && y <= high.y
             );
    }


/*-----------------------------------------------------------------*//**
 @description Test if two ranges are exactly equal.
 @param [in] pRange0  first range
 @param [in] range1  second range
 @param [in] tolerance  toleranc to be applied to each component
 @return true if ranges are identical in all components.
 @group "DRange2d Queries"
 @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IsEqual  (DRange2dCR range1) const
    {
    return
           low.x  == range1.low.x
        && low.y  == range1.low.y
        && high.x == range1.high.x
        && high.y == range1.high.y
        ;
    }


/*-----------------------------------------------------------------*//**
 @description Test if two ranges are equal within a tolerance applied componentwise.
 @param [in] pRange0  first range
 @param [in] range1  second range
 @param [in] tolerance  toleranc to be applied to each component
 @return true if ranges are within tolerance in all components.
 @group "DRange2d Queries"
 @bsimethod
+----------------------------------------------------------------------*/
bool DRange2d::IsEqual
(
DRange2dCR range1,
double tolerance

) const
    {
    return
           fabs (low.x  - range1.low.x ) <= tolerance
        && fabs (low.y  - range1.low.y ) <= tolerance
        && fabs (high.x - range1.high.x) <= tolerance
        && fabs (high.y - range1.high.y) <= tolerance
        ;
    }


//! map a fractional point to the range coordinates. (0,0) is low point, (1,1) is high point.
//! @param [in] fraction fractional coordinates
//! @param [out] xy computed coordinates.
//! @return false if range is null range.
bool DRange2d::TryFractionsToRangePoint (DPoint2dCR fraction, DPoint2dR xy) const
    {
    double dx = high.x - low.x;
    double dy = high.y - low.y;
    if (dx >= 0.0 && dy >= 0.0)
        {
        xy.x = low.x + fraction.x * dx;
        xy.y = low.y + fraction.y * dy;
        return true;
        }
    xy.Zero ();
    return false;
    }

//! map a range point to the fractional coordinates. (0,0) is low point, (1,1) is high point.
//! @param [out] fraction fractional coordinates
//! @param [in] xy computed coordinates.
//! @return false if range is null range or single point.
bool DRange2d::TryRangePointToFractions (DPoint2dCR xy, DPoint2dR fractions) const
    {
    double dx = high.x - low.x;
    double dy = high.y - low.y;
    double u, v;
    if (   DoubleOps::SafeDivide (u, xy.x - low.x, dx, 0.0)
        && DoubleOps::SafeDivide (v, xy.y - low.y, dy, 0.0))
        {
        fractions.x = u;
        fractions.y = v;
        return true;
        }
    fractions.Zero ();
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE

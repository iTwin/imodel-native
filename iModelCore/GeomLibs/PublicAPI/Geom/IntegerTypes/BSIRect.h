/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/IntegerTypes/BSIRect.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Geom/msgeomstructs_typedefs.h>
#include <Geom/IntegerTypes/Point.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! Integer x,y rectangle coordinates.
//! A constructor cannot be added to BSIRect because BSIRects are included in structs
//! that are, in turn, included in unions. A constructor results in error C2620:
//! member 'abc' of union 'xyz' has user-defined constructor or non-trivial default constructor
//! So when including a BSIRect in a struct or class, you must initialize it
//! using BSIRect.Init() or some type of memset.
struct BSIRect
{
    Point2d      origin;        //!< The rectangle origin.
    Point2d      corner;        //!< The rectangle corner.

#if defined (__cplusplus)
    //! Initialize by direct copy to points.  This does NOT enforce ordering.
    void Init(int xOrg, int yOrg, int xHigh, int yHigh)
        {
        origin.x = xOrg;
        origin.y = yOrg;
        corner.x = xHigh;
        corner.y = yHigh;
        }

    //! Initialize by direct copy to points.  This does NOT enforce ordering.
    void Init(Point2d const* low, Point2d const* high)
        {
        origin = *low;
        corner = *high;
        }

    static BSIRect From(int xOrg, int yOrg, int xHigh, int yHigh) {BSIRect rect; rect.Init(xOrg,yOrg,xHigh,yHigh); return rect;}
    static BSIRect From(Point2d const* low, Point2d const* high) {BSIRect rect; rect.Init(low,high); return rect;}

    //! Return the origin x coordinate.
    int Left() const {return origin.x;}
    //! Return the origin y coordinate. (Called "Top" name because screen rectangles start at top left and advance downward.)
    int Top() const {return origin.y;}
    //! Return the corner x coordinate.
    int Right() const {return corner.x;}
    //! Return the corner y coordinate. (Called "Bottom" because screen rectangles advance downward.)
    int Bottom() const {return corner.y;}

    //! Return corner.x - origin.x + 1, i.e. pixel width with start end considered inclusive.
    int Width() const {return (corner.x - origin.x) + 1;}
    //! Return corner.y - origin.y + 1, i.e. pixel height with start end considered inclusive.
    int Height() const {return (corner.y - origin.y) + 1;}
    //! Return Width() * Height ()
    int Area() const {return Width() * Height();}
    //! Return Width () / Height ()
    double Aspect() const {return ((double) Width() / (double) Height());}

    //! limit this rectangle so it is contained in "other"
    GEOMDLLIMPEXP void Limit(BSIRect const* other);

    //! inset rectangle by delta x, y
    GEOMDLLIMPEXP void Inset(int deltaX, int deltaY);

    //! make rectangle larger
    GEOMDLLIMPEXP void Expand(int delta);

    //! offset rectangle by x and y values
    GEOMDLLIMPEXP void Offset(int dx, int dy);

    //! form union of this rectangle with another rectangle
    GEOMDLLIMPEXP void Union(BSIRect const* other);

    //! are two rectangles exactly equal?
    GEOMDLLIMPEXP bool IsEqual(BSIRect const* other) const;

    //! determine whether a point is inside of this rectangle
    GEOMDLLIMPEXP bool PointInside(Point2d const&) const;

    //! determine whether this rectangle overlaps another one. Optionally return the overlapping rect
    GEOMDLLIMPEXP bool Overlap(BSIRect* overlap, BSIRect const* other) const;

    //! determine whether this rectangle is completely within a "container" rectangle.
    GEOMDLLIMPEXP bool IsContained(BSIRect const* container) const;
#endif
};

END_BENTLEY_GEOMETRY_NAMESPACE

DEFINE_GEOM_STRUCT1(BSIRect)

/*__PUBLISH_SECTION_END__*/


/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/FRange3d.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
//! Low and high points of the diagonal of a range box.
//! In null state (FRange3d::NullRange ()), the low and high values are at reversed (FLT_MAX,-FLT_MAX)
struct GEOMDLLIMPEXP FRange3d
    {
friend struct DRange3d;
private:
    FPoint3d low;
    FPoint3d high;

    // direct initialization, no test for expected left-right ordering
    void init (float x0, float y0, float z0, float x1, float y1, float z1)
        {
        low.x = x0;
        low.y = y0;
        low.z = z0;
        high.x = x1;
        high.y = y1;
        high.z = z1;
        }
    // direct initialization, no test for expected left-right ordering
    static FRange3d from (float x0, float y0, float z0, float x1, float y1, float z1)
        {
        FRange3d result;
        result.low.x = x0;
        result.low.y = y0;
        result.low.z = z0;
        result.high.x = x1;
        result.high.y = y1;
        result.high.z = z1;
        return result;
        }

    // direct initialization, no test for expected left-right ordering
    static FRange3d from (FPoint3dCR _low, FPoint3dCR _high)
        {
        FRange3d result;
        result.low = _low;
        result.high = _high;
        return result;
        }
    // in DRange3d, this is "IsEmpty ()", which may be different from "IsNull"  But FRange3d hopes that "empty but not IsNull" cannot happen with private low and high.
    bool HasAnyReversedLowHigh () const;
public:
    //! Return the high point of the range diagonal.
    FPoint3d Low () const;
    //! Return the low point of the range diagonal.
    FPoint3d High () const;
    //! Test if this is completely identical to the value returned by FRange3d::NullRange ()
    bool IsNull () const;
    //! Return an initialized range with no content.
    static FRange3d NullRange ();
    //! (Re)initialize as a null range.
    void InitNull ();
    //! Convert a DRange3d to FRange3d, rounding to ensure the interior volume is not reduced by the double-to-float.
    static FRange3d From (DRange3dCR dRange);

    //! Return the intersection of ranges.
    static FRange3d FromIntersection (FRange3dCR rangeA, FRange3dCR rangeB);
    //! Return the "union" range -- min and max of low, high from the two ranges.
    static FRange3d FromUnion (FRange3dCR rangeA, FRange3dCR rangeB);

    static FRange3d From (DPoint3dCR point);
    static FRange3d From (DPoint3dCR pointA, DPoint3dCR pointB);
    static FRange3d From (bvector<DPoint3d> const &points);

    static FRange3d From (FPoint3dCR point);
    static FRange3d From (FPoint3dCR pointA, FPoint3dCR pointB);
    static FRange3d From (bvector<FPoint3d> const &points);

    void Extend (DPoint3dCR point);
    void Extend (DPoint3dCR pointA, DPoint3dCR pointB);
    void Extend (bvector<DPoint3d> const &points);

    void Extend (FPoint3dCR point);
    void Extend (FRange3dCR other);
    void Extend (FPoint3dCR pointA, FPoint3dCR pointB);
    void Extend (bvector<FPoint3d> const &points);
    //! Extend by (possibly signed!!!) a in all dimensions.
    //! A NullRange is unaffected.
    //! If a is negative and any dimension becomes empty, the entire range becomes NullRange;
    void Extend (double a);
    //! Return a point at fractional position on the diagonal.  Returns 000 if range is null.
    FPoint3d InterpolateDiagonal (double fraction) const;
    //! Return a point with interpolated position on each axis.   Returns 000 if range is null.
    FPoint3d LocalToGlobal (double xFraction, double yFraction, double zFraction) const;
    //! If non-null, return volume.  NullRange returns 0 volume.
    double Volume () const;
    double XLength () const;
    double YLength () const;
    double ZLength () const;
    double DiagonalDistance () const;
    double DiagonalDistanceXY () const;
    FVec3d DiagonalVector () const;
    FVec3d DiagonalVectorXY () const;
    double DistanceOutside (DPoint3dCR xyz) const;
    double DistanceSquaredOutside (DPoint3dCR xyz) const;
    double DistanceSquaredTo (FRange3dCR other) const;

    bool IsAlmostZeroZ () const;
    bool IsAlmostZeroY () const;
    bool IsAlmostZeroX () const;
    bool IsAlmostZeroXYZ () const;

    double LargestCoordinate () const;
    double MaxAbs () const;
    double LargestCoordinateXY () const;
    void Get8Corners (bvector<FPoint3d> &corners) const;

    bool IsContained (FPoint3dCR point) const;
    bool IsContainedXY (FPoint3dCR point) const;
    bool IsContained (double ax, double ay, double az) const;
    bool IsEqual (FRange3dCR range1) const;
    bool IsEqual (FRange3dCR range1, double tolerance) const;


    bool IsSinglePoint () const;
    bool AreAllSidesLongerThan (double a);
    bool IntersectsWith (FRange3dCR other, bool trueForExactTouch) const;
    //! test if a is a small distance relative to the size of this range.
    //! For null range, alwyas return the second arg.
    bool IsSmallDistance (double x, bool nullRangeResult = true) const;
};

END_BENTLEY_GEOMETRY_NAMESPACE

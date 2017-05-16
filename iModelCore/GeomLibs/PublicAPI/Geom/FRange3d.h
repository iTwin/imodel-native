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

public:
    //! Return the high point of the range diagonal.
    FPoint3d Low () const;
    //! Return the low point of the range diagonal.
    FPoint3d High () const;
    //! Test if this is completely identical to the value returned by FRange3d::NullRange ()
    bool IsNull () const;
    //! Return an initialized range with no content.
    static FRange3d NullRange ();
    //! Convert a DRange3d to FRange3d, rounding to ensure the interior volume is not reduced by the double-to-float.
    static FRange3d From (DRange3dCR dRange);


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
    void Extend (FPoint3dCR pointA, FPoint3dCR pointB);
    void Extend (bvector<FPoint3d> const &points);


};

END_BENTLEY_GEOMETRY_NAMESPACE

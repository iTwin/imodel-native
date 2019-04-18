/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#ifndef FVec3d_H_
#define FVec3d_H_

BEGIN_BENTLEY_NAMESPACE
/**
3d vector coordinates.

Note that in addition to the methods here FPoint3d and FVec3d have a full set of operator overloads.

@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP FVec3d
{
public:
//! x coordinate
float x;
//! y coordinate
float y;
//! z coordinate
float z;
private:
// private members expected to be inlined
// Downcasts to float appear only in (a) these inline methods and (b) SetComponent.
    static FVec3d from (double xx, double yy, double zz)
        {
        FVec3d xyz;
        xyz.x = (float)xx;
        xyz.y = (float)yy;
        xyz.z = (float)zz;
        return xyz;
        }
    void init (double xx, double yy, double zz)
        {
        x = (float)xx;
        y = (float)yy;
        z = (float)zz;
        }
    static FVec3d from (DVec3d in)
        {
        FVec3d xyz;
        xyz.x = (float)in.x;
        xyz.y = (float)in.y;
        xyz.z = (float)in.z;
        return xyz;
        }


public:
#ifdef __cplusplus

//! Return a FVec3d with given xyz.
static FVec3d From (double x, double y, double z = 0.0);

//! Return a FVec3d from DVec3d
static FVec3d From (DVec3dCR xyz);

//! Return a FVec3d from xy parts of given point, with z from optional parameter. (And z of the input point is ignored)
static FVec3d FromXY (FVec3dCR xy, double z = 0.0);

//! Return a FVec3d from xy parts of given point, with z from optional parameter. (And z of the input point is ignored)
static FVec3d FromXY (DVec3dCR xy, double z = 0.0);

//! Return a FVec3d with xyz = 0.
static FVec3d FromZero();
//! Return a FVec3d with xyz = 1.
static FVec3d FromOne();


//! @description unit X vector
static FVec3d UnitX (){return FVec3d::From (1,0,0);}
//! @description unit Y vector
static FVec3d UnitY() { return FVec3d::From(0, 1, 0);}
//! @description unit Z vector
static FVec3d UnitZ() { return FVec3d::From (0, 0, 1);}

//! @description Returns the unnormalized (full length) DVec3d between start and end.
//! @param [in] start start point
//! @param [in] end   end point
static FVec3d FromStartEnd (DPoint3dCR start, DPoint3dCR end);

//! @description Returns the unnormalized (full length) DVec3d between start and end.
//! @param [in] start start point
//! @param [in] end   end point
static FVec3d FromStartEnd (FPoint3dCR start, FPoint3dCR end);
//! Unit vector from start towards end.  This is valid if the (pre normalized) length is at least DoubleOps::SmallMetricDistance
static ValidatedFVec3d FromStartEndNormalized (FPoint3dCR start, FPoint3dCR end);
//! Unit vector from start towards end.  This is valid if the (pre normalized) length is at least DoubleOps::SmallMetricDistance
static ValidatedFVec3d FromStartEndNormalized (DPoint3dCR start, DPoint3dCR end);

//! Dot product
double DotProduct (FVec3dCR other) const;
//! Cross product
static FVec3d FromCrossProduct (FVec3dCR vector0, FVec3dCR vector1);

//! Dot product, using only XY parts
double DotProductXY (FVec3dCR other) const;
//! Cross product, using only XY parts
double CrossProductXY (FVec3dCR other) const;


//! Vector magnitude, promoting components to double and accumulating as double
double Magnitude () const;
//! Squared vector magnitude, promoting components to double and accumulating as double
double MagnitudeSquared () const;

//! maximum absolute value among x,y,z parts.
double MaxAbs () const;

//! bitwise equality test
bool IsEqual (FVec3dCR range1) const;
//! toleranced equality test.
bool IsEqual (FVec3dCR other, double tolerance) const;


//! return true 3D angle between vectors.   Because there is no clear "up" direction, this is 0 or positive, and less than 180 degrees.
Angle AngleTo (FVec3dCR other) const;
//! return (signed) angle from this vector to the other, as viewed in xy direction.
Angle AngleToXY (FVec3dCR other) const;
//! return (signed) angle from this vector to the other, as measured in the plane perpendicular to the planeUpVector
Angle PlanarAngleTo (FVec3dCR other, FVec3dCR planeUpVector) const;
//! return the (signed) angle from this vector to the other.  This is a full 3D angle in the plane of the two vectors, with the
//! orientationVector selecting which side is positive.
Angle SignedAngleTo (FVec3dCR other, FVec3dCR orientationVector) const;
//! return the smaller angle between the vectors extended in both directions.  This is no larger than 90 degrees.
Angle SmallerUnorientedAngleTo (FVec3dCR other) const;
//! return the absolute difference of the angle the vectors are away from perpendicular.
Angle AngleFromPerpendicular (FVec3dCR other) const;
//! Test for parallel vectors.
bool IsParallelTo (FVec3dCR other) const;
bool IsPositiveParallelTo (FVec3dCR other) const;
bool IsPerpendicularTo (FVec3dCR other) const;
#endif // __cplusplus

};

END_BENTLEY_NAMESPACE

#endif // FVec3d_H_

/*__PUBLISH_SECTION_END__*/


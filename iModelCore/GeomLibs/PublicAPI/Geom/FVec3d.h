/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/FVec3d.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#ifndef FVec3d_H_
#define FVec3d_H_

BEGIN_BENTLEY_NAMESPACE
/**
3d point coordinates.

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
static DVec3d UnitX (){return DVec3d::From (1,0,0);}
//! @description unit Y vector
static DVec3d UnitY() { return DVec3d::From(0, 1, 0);}
//! @description unit Z vector
static DVec3d UnitZ() { return DVec3d::From (0, 0, 1);}

//! @description Returns the unnormalized (full length) DVec3d between start and end.
//! @param [in] start start point
//! @param [in] end   end point
static FVec3d FromStartEnd (DPoint3dCR start, DPoint3dCR end);

//! @description Returns the unnormalized (full length) DVec3d between start and end.
//! @param [in] start start point
//! @param [in] end   end point
static FVec3d FromStartEnd (FPoint3dCR start, FPoint3dCR end);

//! Swap contents of instance, other.
//! @param [in,out] other second point.
void Swap (FVec3dR other);

//! Dot product, using FLOAT ONLY (no promotion to double for higher accuracy sums)
float FDotProduct (FVec3dCR other) const;
//! Cross prodcut, using FLOAT ONLY (no promotion to double for higher accuracy sums)
FVec3d FCrossProduct (FVec3dCR other) const;

//! Dot product, promoting to double
double DotProduct (FVec3dCR other) const;

#endif // __cplusplus

};

END_BENTLEY_NAMESPACE

#endif // FVec3d_H_

/*__PUBLISH_SECTION_END__*/


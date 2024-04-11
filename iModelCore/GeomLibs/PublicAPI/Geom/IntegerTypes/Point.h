/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Geom/msgeomstructs_typedefs.h>

DEFINE_GEOM_STRUCT1 (Point2d)
DEFINE_GEOM_STRUCT1 (Point3d)

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! An integer 2d point, useful for screen pixel references, but not for real geometry
struct Point2d
    {
    int32_t x;
    int32_t y;
#if defined (__cplusplus)
    void Init (int xIn, int yIn) {x  = xIn; y=yIn;}
    bool IsSame (Point2dCR other) const {return x==other.x && y==other.y;}
    static Point2d From(int x, int y) {Point2d val; val.x=x; val.y=y; return val;}
#endif
    };

//! An integer 3d point, useful for screen pixel references, but not for real geometry
struct Point3d
    {
    int32_t     x;
    int32_t     y;
    int32_t     z;
#if defined (__cplusplus)
    void Init(int xIn, int yIn, int zIn) { x = xIn; y = yIn; z = zIn;}
    bool IsSame(Point2dCR other) const { return x == other.x && y == other.y; }
    static Point3d From(int x, int y, int z)
        { Point3d val; val.x = x; val.y = y; val.z = z; return val; }
#endif
    };

END_BENTLEY_GEOMETRY_NAMESPACE

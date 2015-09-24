//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/PointTypes.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/PointTypes.h>

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* Stored types integrity checks
+---------------+---------------+---------------+---------------+---------------+------*/
HSTATICASSERT(2*sizeof(double) == sizeof(Point2d64f));
HSTATICASSERT((2*sizeof(double) + 4*sizeof(uint8_t)) == sizeof(Point2d64fR8G8B8I8));
HSTATICASSERT((2*sizeof(double) + sizeof(uint32_t)) == sizeof(Point2d64fG32));

HSTATICASSERT(3*sizeof(double) == sizeof(Point3d64f));
HSTATICASSERT((3*sizeof(double) + 4*sizeof(uint8_t)) == sizeof(Point3d64fR8G8B8I8));
HSTATICASSERT((3*sizeof(double) + sizeof(uint32_t)) == sizeof(Point3d64fG32));
HSTATICASSERT(4*sizeof(double) == sizeof(Point3d64fM64f));
HSTATICASSERT((4*sizeof(double) + sizeof(uint32_t)) == sizeof(Point3d64fM64fG32));


bool Point2dEqual  (double lx, double ly,
                    double rx, double ry)
    {
    return HNumeric<double>::EQUAL_EPSILON(lx, rx) &&
           HNumeric<double>::EQUAL_EPSILON(ly, ry);
    }

bool Point2dLess   (double lx, double ly,
                    double rx, double ry)
    {
    if (lx < rx) // left less than right
        return true;
    else if (rx < lx) // left greater than right
        return false;

    return ly < ry;
    }

bool Point3dEqual  (double lx, double ly, double lz,
                    double rx, double ry, double rz)
    {
    return HNumeric<double>::EQUAL_EPSILON(lx, rx) &&
           HNumeric<double>::EQUAL_EPSILON(ly, ry) &&
           HNumeric<double>::EQUAL_EPSILON(lz, rz);
    }

bool Point3dLess   (double lx, double ly, double lz,
                    double rx, double ry, double rz)
    {
    if (lx < rx) // left less than right
        return true;
    else if (rx < lx) // left greater than right
        return false;
    if (ly < ry) // left less than right
        return true;
    else if (ry < ly) // left greater than right
        return false;

    return lz < rz;
    }

bool operator== (const Point2d64f& lhs, const Point2d64f& rhs)
    {
    return PointTrait<Point2d64f>::Equal(lhs, rhs);
    }

bool operator< (const Point2d64f& lhs, const Point2d64f& rhs)
    {
    return PointTrait<Point2d64f>::Less(lhs, rhs);
    }


bool operator== (const Point2d64fR8G8B8I8& lhs, const Point2d64fR8G8B8I8& rhs)
    {
    return PointTrait<Point2d64fR8G8B8I8>::Equal(lhs, rhs) &&
           lhs.r == rhs.r &&
           lhs.g == rhs.g &&
           lhs.b == rhs.b &&
           lhs.i == rhs.i;
    }

bool operator< (const Point2d64fR8G8B8I8& lhs, const Point2d64fR8G8B8I8& rhs)
    {
    if (PointTrait<Point2d64fR8G8B8I8>::Less(lhs, rhs))
        return true;
    else if (PointTrait<Point2d64fR8G8B8I8>::Less(rhs, lhs))
        return false;

    return std::lexicographical_compare(&lhs.r, &lhs.r + 4, &rhs.r, &rhs.r + 4); // NTERAY: Optimize??
    }


bool operator== (const Point2d64fG32& lhs, const Point2d64fG32& rhs)
    {
    return PointTrait<Point2d64fG32>::Equal(lhs, rhs) &&
           lhs.g == rhs.g;
    }

bool operator< (const Point2d64fG32& lhs, const Point2d64fG32& rhs)
    {
    if (PointTrait<Point2d64fG32>::Less(lhs, rhs))
        return true;
    else if (PointTrait<Point2d64fG32>::Less(rhs, lhs))
        return false;

    return lhs.g < rhs.g;
    }



bool operator== (const Point3d64f& lhs, const Point3d64f& rhs)
    {
    return PointTrait<Point3d64f>::Equal(lhs, rhs);
    }

bool operator< (const Point3d64f& lhs, const Point3d64f& rhs)
    {
    return PointTrait<Point3d64f>::Less(lhs, rhs);
    }


bool operator== (const Point3d64fR8G8B8I8& lhs, const Point3d64fR8G8B8I8& rhs)
    {
    return PointTrait<Point3d64fR8G8B8I8>::Equal(lhs, rhs) &&
           lhs.r == rhs.r &&
           lhs.g == rhs.g &&
           lhs.b == rhs.b &&
           lhs.i == rhs.i;
    }

bool operator< (const Point3d64fR8G8B8I8& lhs, const Point3d64fR8G8B8I8& rhs)
    {
    if (PointTrait<Point3d64fR8G8B8I8>::Less(lhs, rhs))
        return true;
    else if (PointTrait<Point3d64fR8G8B8I8>::Less(rhs, lhs))
        return false;

    return std::lexicographical_compare(&lhs.r, &lhs.r + 4, &rhs.r, &rhs.r + 4); // NTERAY: Optimize??
    }



bool operator== (const Point3d64fG32& lhs, const Point3d64fG32& rhs)
    {
    return PointTrait<Point3d64fG32>::Equal(lhs, rhs) &&
           lhs.g == rhs.g;
    }

bool operator< (const Point3d64fG32& lhs, const Point3d64fG32& rhs)
    {
    if (PointTrait<Point3d64fG32>::Less(lhs, rhs))
        return true;
    else if (PointTrait<Point3d64fG32>::Less(rhs, lhs))
        return false;

    return lhs.g < rhs.g;
    }

bool operator== (const Point3d64fM64f& lhs, const Point3d64fM64f& rhs)
    {
    return PointTrait<Point3d64fM64f>::Equal(lhs, rhs) &&
           HNumeric<double>::EQUAL_EPSILON(lhs.m, rhs.m);
    }

bool operator< (const Point3d64fM64f& lhs, const Point3d64fM64f& rhs)
    {
    if (PointTrait<Point3d64fM64f>::Less(lhs, rhs))
        return true;
    else if (PointTrait<Point3d64fM64f>::Less(rhs, lhs))
        return false;

    return lhs.m < rhs.m;
    }

bool operator== (const Point3d64fM64fG32& lhs, const Point3d64fM64fG32& rhs)
    {
    return PointTrait<Point3d64fM64fG32>::Equal(lhs, rhs) &&
           HNumeric<double>::EQUAL_EPSILON(lhs.m, rhs.m) &&
           lhs.g == rhs.g;
    }

bool operator< (const Point3d64fM64fG32& lhs, const Point3d64fM64fG32& rhs)
    {
    if (PointTrait<Point3d64fM64fG32>::Less(lhs, rhs))
        return true;
    else if (PointTrait<Point3d64fM64fG32>::Less(rhs, lhs))
        return false;

    if (lhs.m < rhs.m)
        return true;
    else if (rhs.m < lhs.m)
        return false;

    return lhs.g < rhs.g;
    }


} //End namespace IDTMFile
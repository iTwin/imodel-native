//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/PointTypes.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


/*---------------------------------------------------------------------------------**//**
* @description  Helper mix-in to be used as a base for specific point trait
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct Point2dTraitMixin
    {
    typedef T                           point_type;

    typedef double                      coordinate_type;

    static const size_t                 dimensions_quantity = 2;

    static const bool                   can_hold_x = true;
    static const bool                   can_hold_y = true;
    static const bool                   can_hold_z = false;

    // Undefined Create. User need to implement one for his specific point type.

    static coordinate_type              GetX                       (const point_type&       pt) {
        return pt.x;
        }
    static coordinate_type              GetY                       (const point_type&       pt) {
        return pt.y;
        }
    static coordinate_type              GetZ                       (const point_type&       pt) {
        return 0;
        }

    static void                         SetX                       (point_type&             pt,
                                                                    coordinate_type         value) {
        pt.x = value;
        }
    static void                         SetY                       (point_type&             pt,
                                                                    coordinate_type         value) {
        pt.y = value;
        }
    static void                         SetZ                       (point_type&             pt,
                                                                    coordinate_type         value) {
        /*Do nothing*/
        }

    static bool                         Equal                      (const point_type&       lhs,
                                                                    const point_type&       rhs)
        {
        return Point2dEqual(lhs.x, lhs.y, rhs.x, rhs.y);
        }

    static bool                         Less                       (const point_type&       lhs,
                                                                    const point_type&       rhs)
        {
        return Point2dLess(lhs.x, lhs.y, rhs.x, rhs.y);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  Helper mix-in to be used as a base for specific point trait
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct Point3dTraitMixin
    {
    typedef T                           point_type;

    typedef double                      coordinate_type;

    static const size_t                 dimensions_quantity = 3;

    static const bool                   can_hold_x = true;
    static const bool                   can_hold_y = true;
    static const bool                   can_hold_z = true;

    // Undefined Create. User need to implement one for his specific point type.

    static double                       GetX                       (const point_type&       pt) {
        return pt.x;
        }
    static double                       GetY                       (const point_type&       pt) {
        return pt.y;
        }
    static double                       GetZ                       (const point_type&       pt) {
        return pt.z;
        }

    static void                         SetX                       (point_type&             pt,
                                                                    double                  value) {
        pt.x = value;
        }
    static void                         SetY                       (point_type&             pt,
                                                                    double                  value) {
        pt.y = value;
        }
    static void                         SetZ                       (point_type&             pt,
                                                                    double                  value) {
        pt.z = value;
        }

    static bool                         Equal                      (const point_type&       lhs,
                                                                    const point_type&       rhs)
        {
        return Point3dEqual(lhs.x, lhs.y, lhs.z, rhs.x, rhs.y, rhs.z);
        }

    static bool                         Less                       (const T&                lhs,
                                                                    const T&                rhs)
        {
        return Point3dLess(lhs.x, lhs.y, lhs.z, rhs.x, rhs.y, rhs.z);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  Helper mix-in to be used as a base for specific PointGroupIdTrait.
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct PointGroupId32TraitMixin
    {
    typedef T                           point_type;

    typedef uint32_t                   type;
    static const bool                   can_hold = true;

    // TDORAY: Do we default to 0 or max? 0 will then need to be reserved for no group id...
    static type                         GetDefault                 () {
        return 0;
        }

    static type                         Get                        (const point_type&       pt) {
        return pt.g;
        }
    static void                         Set                        (point_type&             pt,
                                                                    type                    value) {
        pt.g = value;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  Helper mix-in to be used as a base for specific PointSignificanceTrait.
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct PointSignificance64fTraitMixin
    {
    typedef T                           point_type;

    typedef double                      type;
    static const bool                   can_hold = true;

    // TDORAY: What is the real default for significance??
    static type                         GetDefault                 () {
        return 0.0;
        }

    static type                         Get                        (const point_type&       pt) {
        return pt.m;
        }
    static void                         Set                        (point_type&             pt,
                                                                    type                    value) {
        pt.m = value;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 2D point class with a RGBI color.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointTrait<Point2d64f> : public Point2dTraitMixin<Point2d64f>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y};
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point2d64f>
    {
    static const PointTypeID                value = POINT_TYPE_XYf64;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 2D point class with a RGBI color.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointTrait<Point2d64fR8G8B8I8> : public Point2dTraitMixin<Point2d64fR8G8B8I8>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y, 0, 0, 0, 0}; // NTERAY: Is that correct default value for color components?
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point2d64fR8G8B8I8>
    {
    static const PointTypeID                value = POINT_TYPE_XYf64RGBIi8;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 2D point class with a group id that can be used to access
*               metadata common for this group.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointGroupIdTrait<Point2d64fG32> : public PointGroupId32TraitMixin<Point2d64fG32> {};

template <> struct PointTrait<Point2d64fG32> : public Point2dTraitMixin<Point2d64fG32>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y, PointGroupIdTrait<point_type>::GetDefault()};
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point2d64fG32>
    {
    static const PointTypeID                value = POINT_TYPE_XYf64Gi32;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointTrait<Point3d64f> : public Point3dTraitMixin<Point3d64f>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y, z};
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point3d64f>
    {
    static const PointTypeID                value = POINT_TYPE_XYZf64;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a RGBI color.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointTrait<Point3d64fR8G8B8I8> : public Point3dTraitMixin<Point3d64fR8G8B8I8>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y, z, 0, 0, 0, 0}; // NTERAY: Is that correct default value for color components?
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point3d64fR8G8B8I8>
    {
    static const PointTypeID                value = POINT_TYPE_XYZf64RGBIi8;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a group id that can be used to access
*               metadata common for this group.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointGroupIdTrait<Point3d64fG32> : public PointGroupId32TraitMixin<Point3d64fG32> {};

template <> struct PointTrait<Point3d64fG32> : public Point3dTraitMixin<Point3d64fG32>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y, z, PointGroupIdTrait<point_type>::GetDefault()};
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point3d64fG32>
    {
    static const PointTypeID                value = POINT_TYPE_XYZf64Gi32;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a m dimension that represents the
*               significance level of the point relatively to his surrounding.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointSignificanceTrait<Point3d64fM64f> : public PointSignificance64fTraitMixin<Point3d64fM64f> {};

template <> struct PointTrait<Point3d64fM64f> : public Point3dTraitMixin<Point3d64fM64f>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y, z, PointSignificanceTrait<point_type>::GetDefault()};
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point3d64fM64f>
    {
    static const PointTypeID                value = POINT_TYPE_XYZMf64;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a m dimension that represents the
*               significance level of the point relatively to his surrounding and
*               a G (group id) that can be used to access metadata common for this
*               group.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct PointSignificanceTrait<Point3d64fM64fG32> : public PointSignificance64fTraitMixin<Point3d64fM64fG32> {};
template <> struct PointGroupIdTrait<Point3d64fM64fG32> : public PointGroupId32TraitMixin<Point3d64fM64fG32> {};

template <> struct PointTrait<Point3d64fM64fG32> : public Point3dTraitMixin<Point3d64fM64fG32>
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
        {
        point_type pt = {x, y, z,
                         PointSignificanceTrait<point_type>::GetDefault(),
                         PointGroupIdTrait<point_type>::GetDefault()
                        };
        return pt;
        }
    };

template <> struct PointTypeIDTrait<Point3d64fM64fG32>
    {
    static const PointTypeID                value = POINT_TYPE_XYZMf64Gi32;
    };

    template <> struct PointTypeIDTrait<int32_t>
        {
        static const PointTypeID                value = POINT_TYPE_INTEGER;
        };
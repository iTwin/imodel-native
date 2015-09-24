//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/PointTypes.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Listing of all supported IDTM point types.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
enum PointTypeID
    {
    POINT_TYPE_XYZf64,
    POINT_TYPE_XYZf64RGBIi8,
    POINT_TYPE_XYZf64Gi32,
    POINT_TYPE_XYZMf64,
    POINT_TYPE_XYZMf64Gi32,
    POINT_TYPE_XYf64,
    POINT_TYPE_XYf64RGBIi8,
    POINT_TYPE_XYf64Gi32,
    POINT_TYPE_INTEGER,
    POINT_TYPE_QTY,
    POINT_TYPE_INVALID = POINT_TYPE_QTY,
    POINT_TYPE_NONE = POINT_TYPE_QTY,
    };


#ifdef _WIN32
#pragma pack(push, IDTMFileIdent, 4)
#else
#pragma pack(push, 4)
#endif


/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 2D point class.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point2d64f
    {
    double                                  x, y;
    };

 bool                                 operator==             (const Point2d64f&           lhs,
                                                                    const Point2d64f&           rhs);
 bool                                 operator<              (const Point2d64f&           lhs,
                                                                    const Point2d64f&           rhs);

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 2D point class with a RGBI color.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point2d64fR8G8B8I8
    {
    double                                  x, y;
    uint8_t                                r, g, b, i;
    };

 bool                                 operator==             (const Point2d64fR8G8B8I8&   lhs,
                                                                    const Point2d64fR8G8B8I8&   rhs);
 bool                                 operator<              (const Point2d64fR8G8B8I8&   lhs,
                                                                    const Point2d64fR8G8B8I8&   rhs);

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 2D point class with a group id that can be used to access
*               metadata common for this group.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point2d64fG32
    {
    double                                  x, y;
    uint32_t                               g;
    };


 bool                                 operator==             (const Point2d64fG32&        lhs,
                                                                    const Point2d64fG32&        rhs);
 bool                                 operator<              (const Point2d64fG32&        lhs,
                                                                    const Point2d64fG32&        rhs);

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point3d64f
    {
    double                                  x, y, z;
    };

 bool                                 operator==             (const Point3d64f&           lhs,
                                                                    const Point3d64f&           rhs);
 bool                                 operator<              (const Point3d64f&           lhs,
                                                                    const Point3d64f&           rhs);

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a RGBI color.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point3d64fR8G8B8I8
    {
    double                                  x, y, z;
    uint8_t                                r, g, b, i;
    };

 bool                                 operator==             (const Point3d64fR8G8B8I8&   lhs,
                                                                    const Point3d64fR8G8B8I8&   rhs);
 bool                                 operator<              (const Point3d64fR8G8B8I8&   lhs,
                                                                    const Point3d64fR8G8B8I8&   rhs);

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a group id that can be used to access
*               metadata common for this group.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point3d64fG32
    {
    double                                  x, y, z;
    uint32_t                               g;
    };

 bool                                 operator==             (const Point3d64fG32&        lhs,
                                                                    const Point3d64fG32&        rhs);
 bool                                 operator<              (const Point3d64fG32&        lhs,
                                                                    const Point3d64fG32&        rhs);

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a m dimension that represents the
*               significance level of the point relatively to his surrounding.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point3d64fM64f
    {
    double                                  x, y, z;
    double                                  m;
    };

 bool                                 operator==             (const Point3d64fM64f&       lhs,
                                                                    const Point3d64fM64f&       rhs);
 bool                                 operator<              (const Point3d64fM64f&       lhs,
                                                                    const Point3d64fM64f&       rhs);

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D point class with a m dimension that represents the
*               significance level of the point relatively to his surrounding and
*               a G (group id) that can be used to access metadata common for this
*               group.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point3d64fM64fG32
    {
    double                                  x, y, z;
    double                                  m;
    uint32_t                               g;
    };


 bool                                 operator==             (const Point3d64fM64fG32&    lhs,
                                                                    const Point3d64fM64fG32&    rhs);

 bool                                 operator<              (const Point3d64fM64fG32&    lhs,
                                                                    const Point3d64fM64fG32&    rhs);

#ifdef _WIN32
#pragma pack(pop, IDTMFileIdent)
#else
#pragma pack(pop)
#endif



 bool                             Point2dEqual               (double lx, double ly,
                                                                    double rx, double ry);

 bool                             Point2dLess                (double lx, double ly,
                                                                    double rx, double ry);

 bool                             Point3dEqual               (double lx, double ly, double lz,
                                                                    double rx, double ry, double rz);

 bool                             Point3dLess                (double lx, double ly, double lz,
                                                                    double rx, double ry, double rz);

/*---------------------------------------------------------------------------------**//**
* @description  Compile time mapping between point types and point types ids.
* @bsiclass                                                  Raymond.Gauthier   1/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
struct PointTypeIDTrait
    {
    static const PointTypeID            value = POINT_TYPE_INVALID;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Give generic accessors to 3 coordinates x, y, z and a way of
*               comparing points types based only on coordinates. This trait is
*               mandatory to implement for new point types.
* @bsiclass                                                  Raymond.Gauthier   1/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
struct PointTrait
    {
    typedef PointType                   point_type;

    typedef double                      coordinate_type;
    static const size_t                 dimensions_quantity = 0;

    static const bool                   can_hold_x = false;
    static const bool                   can_hold_y = false;
    static const bool                   can_hold_z = false;

    static point_type                   Create                     (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z)
       {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif
        return point_type();
        }

    static coordinate_type              GetX                       (const point_type&       pt) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        }

    static coordinate_type              GetY                       (const point_type&       pt) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        }

    static coordinate_type              GetZ                       (const point_type&       pt) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        }


    static void                         SetX                       (const point_type&       pt,
                                                                    coordinate_type         value) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        }

    static void                         SetY                       (const point_type&       pt,
                                                                    coordinate_type         value) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        }
    static void                         SetZ                       (const point_type&       pt,
                                                                    coordinate_type         value) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        }

    static bool                         Equal                      (const point_type&       lhs,
                                                                    const point_type&       rhs) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        return false;
        }

    static bool                         Less                       (const point_type&       lhs,
                                                                    const point_type&       rhs) 
        {
#if defined (ANDROID) || defined (__APPLE__)
	//DM-Android
#elif defined (_WIN32)
        HSTATICASSERT(0);
#endif        
        return false;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  Trait for generically accessing group id for a point. This trait is
*               optional to implement for new point types. can_hold attribute specifies
*               whether a point type actually can hold a group id.
* @bsiclass                                                  Raymond.Gauthier   1/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
struct PointGroupIdTrait
    {
    typedef PointType                   point_type;

    typedef uint32_t                   type;
    static const bool                   can_hold = false;

    // TDORAY: Do we default to 0 or max? 0 will then need to be reserved for no group id...
    static type                         GetDefault                 () {
        return 0;
        }

    static type                         Get                        (const point_type&       pt) {
        return GetDefault();
        }
    static void                         Set                        (const point_type&       pt,
                                                                    type                    value) {
        /*Do nothing*/
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  Trait for generically accessing significance for a point. This trait is
*               optional to implement for new point types. can_hold attribute specifies
*               whether a point type actually can hold a significance.
* @bsiclass                                                  Raymond.Gauthier   1/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
struct PointSignificanceTrait
    {
    typedef PointType                   point_type;

    typedef double                      type;
    static const bool                   can_hold = false;

    // TDORAY: What is the real default for significance??
    static type                         GetDefault                 () {
        return 0.0;
        }


    static type                         Get                        (const point_type&       pt) {
        return GetDefault();
        }
    static void                         Set                        (const point_type&       pt,
                                                                    type                    value) {
        /*Do nothing*/
        }
    };

// TDORAY: Implement a rgb trait when needed...
// TDORAY: Implement a i trait when needed...


#include <STMInternal/Storage/IDTMFileDirectories/PointTypes.hpp>


} //End namespace IDTMFile
//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGFSpatialIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once


#include <ImagePP/all/h/HGF2DLiteExtent.h>
#include <ImagePP/all/h/HGF3DCoord.h>
#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HGF2DPosition.h>
#include <ImagePP/all/h/HGF3DExtent.h>
#include <ImagePP/all/h/HPMPooledVector.h>

#define MAX_NUM_SUBNODES 8

/** -----------------------------------------------------------------------------

    CopyVector<> class

    This template class provides a way to normalize the vector interface.
    for anytype of container. For performance reason, some vector implementations
    allow for a direct copy of many contained objects. In this case it is possible
    to overload the present class for use in the spatial index.

    -----------------------------------------------------------------------------
*/
template<class CONTAINER> class CopyVector
    {
public:
    static void Copy(CONTAINER* destination, CONTAINER* source)
        {
        for (size_t index= 0 ; index < source->size() ; index++)
            {
            destination->push_back (source->operator[](index));
            }
        }
    };


template<class DataType> class CopyVector<HPMPooledVector<DataType> >
    {
public:
    static void Copy(HPMPooledVector<DataType>* destination, HPMPooledVector<DataType>* source)
        {
        destination->push_back(source);
        }
    };

// For use with spatial index
template<class DataType> class CopyVector<HFCPtr<HPMPooledVector<DataType> > >
    {
public:
    static void Copy(HFCPtr<HPMPooledVector<DataType> > destination, HFCPtr<HPMPooledVector<DataType> > source)
        {
        destination->push_back(source);
        }
    };

template<class DataType> class CopyVector<HPMStoredPooledVector<DataType> >
    {
public:
    static void Copy(HPMStoredPooledVector<DataType>* destination, HPMPooledVector<DataType>* source)
        {
        destination->push_back(source);
        }
    };

template<class DataType> class CopyVector<HFCPtr<HPMStoredPooledVector<DataType> > >
    {
public:
    static void Copy(HFCPtr<HPMStoredPooledVector<DataType> > destination, 
                     HFCPtr<HPMPooledVector<DataType> > source)
        {
        destination->push_back(source);
        }
    };




class UnknownPointFamily {};

/** -----------------------------------------------------------------------------
    Trait that enables developing custom families of point types with similar
    semantics. New family of types can be added by specializing or partially
    specializing the family trait. UnknownPointFamily is returned when no
    family is found for the specified point type.
    e.g.:
    class IDTMFilePointFamily {};
    template <> struct PointFamilyTrait<Point3d64f>       { typedef IDTMFilePointFamily type; };
    template <> struct PointFamilyTrait<Point3d64fM64f>   { typedef IDTMFilePointFamily type; };
    -> Create a new family of point for idtm points.
    or
    class PtrPointFamily {};
    template <typename POINT> struct PointFamilyTrait<POINT*> { typedef PtrPointFamily type; }
    -> Create a new family of point for pointer to points.

    -----------------------------------------------------------------------------
*/
template <class POINT> struct PointFamilyTrait
    {
    typedef UnknownPointFamily type;
    };


/** -----------------------------------------------------------------------------

    PointOp<> class

    This template class provides a way to normalize the point class interface.
    Since the point index has the point type as template argument and all
    point classes have their own individiual interfaces, the spatial index
    makes use of the PointOp<> template class. The default implementation
    fits properly the HGF3DCoord<double> interface. Other point types need simply
    overload the template class and code the appropriate operation.

    Partial specialization are also possible for a whole family of point by
    specifying the point family for which the template is specialized.
    e.g.:
    template <class POINT> struct PointOp<POINT, IDTMFilePointFamily>
    -> Partially specialize point op for the whole IDTMFilePointFamily.

    -----------------------------------------------------------------------------
*/
template <class POINT, class POINT_FAMILY = typename PointFamilyTrait<POINT>::type> struct PointOp
    {
    static double GetX(const POINT& point) {
        return point.GetX();
        }
    static double GetY(const POINT& point) {
        return point.GetY();
        }
    static double GetZ(const POINT& point) {
        return point.GetZ();
        }
    static void   SetX(POINT& point, double x) {
        point.SetX(x);
        }
    static void   SetY(POINT& point, double y) {
        point.SetY(y);
        }
    static void   SetZ(POINT& point, double z) {
        point.SetZ(z);
        }
    static POINT  Create(double x, double y, double z) {
        return POINT(x, y, z);
        }
    static bool   AreEqual(const POINT& point1, const POINT& point2) {
        return point1 == point2;
        }
    };


/** -----------------------------------------------------------------------------

    ExtentOp<> class

    This template class provides a way to normalize the extent class interface.
    Since the point index has the extent type as template argument and all
    extent classes have their own individiual interfaces, the spatial index
    makes use of the ExtentOp<> template class. The default implementation
    fits properly the HGF3DExtent<double> interface. Other extent types need simply
    overload the template class and code the appropriate operation.

    Notice the the HGF2DTemplateExtent<> specialization is also provided.

    -----------------------------------------------------------------------------
*/
template <class EXTENT> class ExtentOp
    {
public:
    static double GetXMin(const EXTENT& extent) {
        return extent.GetXMin();
        }
    static double GetXMax(const EXTENT& extent) {
        return extent.GetXMax();
        }
    static double GetYMin(const EXTENT& extent) {
        return extent.GetYMin();
        }
    static double GetYMax(const EXTENT& extent) {
        return extent.GetYMax();
        }
    static double GetZMin(const EXTENT& extent) {
        return extent.GetZMin();
        }
    static double GetZMax(const EXTENT& extent) {
        return extent.GetZMax();
        }
    static void   SetXMin(EXTENT& extent, double xMin) {
        extent.SetXMin(xMin);
        }
    static void   SetXMax(EXTENT& extent, double xMax) {
        extent.SetXMax(xMax);
        }
    static void   SetYMin(EXTENT& extent, double yMin) {
        extent.SetYMin(yMin);
        }
    static void   SetYMax(EXTENT& extent, double yMax) {
        extent.SetYMax(yMax);
        }
    static void   SetZMin(EXTENT& extent, double zMin) {
        extent.SetZMin(zMin);
        }
    static void   SetZMax(EXTENT& extent, double zMax) {
        extent.SetZMax(zMax);
        }
    static double GetWidth(const EXTENT& extent) {
        return extent.GetWidth();
        }
    static double GetHeight(const EXTENT& extent) {
        return extent.GetHeight();
        }
    static double GetThickness(const EXTENT& extent) {
        return extent.GetThickness();
        }
    static bool   Overlap (const EXTENT& extent1, const EXTENT& extent2) {
        return extent1.Overlaps(extent2);
        }
    static bool   InnerOverlap (const EXTENT& extent1, const EXTENT& extent2) {
        return extent1.InnerOverlaps(extent2);
        }
    static bool   OutterOverlap (const EXTENT& extent1, const EXTENT& extent2) {
        return extent1.OutterOverlaps(extent2);
        }
    static EXTENT Create(double xMin, double yMin, double xMax, double yMax) {
        return EXTENT(xMin, yMin, xMax, yMax);
        }
    static EXTENT Create(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax) {
        return EXTENT(xMin, yMin, zMin, xMax, yMax, zMax);
        }
    static EXTENT MergeExtents(const EXTENT& extent1, const EXTENT& extent2)
        {
        return Create(MIN(GetXMin(extent1), GetXMin(extent2)),
                      MIN(GetYMin(extent1), GetYMin(extent2)),
                      MIN(GetZMin(extent1), GetZMin(extent2)),
                      MAX(GetXMax(extent1), GetXMax(extent2)),
                      MAX(GetYMax(extent1), GetYMax(extent2)),
                      MAX(GetZMax(extent1), GetZMax(extent2)));
        }

    };


#if (0)
template <typename T1, typename T2> class ExtentOp<HGF2DTemplateExtent<T1, T2> >
    {
public:
    static double GetXMin(const HGF2DTemplateExtent<T1, T2>& extent) {
        return extent.GetXMin();
        }
    static double GetXMax(const HGF2DTemplateExtent<T1, T2>& extent) {
        return extent.GetXMax();
        }
    static double GetYMin(const HGF2DTemplateExtent<T1, T2>& extent) {
        return extent.GetYMin();
        }
    static double GetYMax(const HGF2DTemplateExtent<T1, T2>& extent) {
        return extent.GetYMax();
        }
    static double GetZMin(const HGF2DTemplateExtent<T1, T2>& extent) {
        return 0.0;
        }
    static double GetZMax(const HGF2DTemplateExtent<T1, T2>& extent) {
        return 0.0;
        }
    static void   SetXMin(HGF2DTemplateExtent<T1, T2>& extent, double xMin) {
        extent.SetXMin(xMin);
        }
    static void   SetXMax(HGF2DTemplateExtent<T1, T2>& extent, double xMax) {
        extent.SetXMax(xMax);
        }
    static void   SetYMin(HGF2DTemplateExtent<T1, T2>& extent, double yMin) {
        extent.SetYMin(yMin);
        }
    static void   SetYMax(HGF2DTemplateExtent<T1, T2>& extent, double yMax) {
        extent.SetYMax(yMax);
        }
    static void   SetZMin(HGF2DTemplateExtent<T1, T2>& extent, double zMin) {
        /*Do nothing*/
        }
    static void   SetZMax(HGF2DTemplateExtent<T1, T2>& extent, double zMax) {
        /*Do nothing*/
        }
    static double GetWidth(const HGF2DTemplateExtent<T1, T2>& extent) {
        return extent.GetWidth();
        }
    static double GetHeight(const HGF2DTemplateExtent<T1, T2>& extent) {
        return extent.GetHeight();
        }
    static double GetThickness(const HGF2DTemplateExtent<T1, T2>& extent) {
        return 0.0;
        }
    static bool   Overlap (const HGF2DTemplateExtent<T1, T2>& extent1, const HGF2DTemplateExtent<T1, T2>& extent2) {
        return extent1.Overlaps(extent2);
        }
    static bool   InnerOverlap (const HGF2DTemplateExtent<T1, T2>& extent1, const HGF2DTemplateExtent<T1, T2>& extent2) {
        return extent1.InnerOverlaps(extent2);
        }
    static bool   OutterOverlap (const HGF2DTemplateExtent<T1, T2>& extent1, const HGF2DTemplateExtent<T1, T2>& extent2) {
        return extent1.OutterOverlaps(extent2);
        }
    static HGF2DTemplateExtent<T1, T2> Create(double xMin, double yMin, double xMax, double yMax) {
        return HGF2DTemplateExtent<T1, T2>(xMin, yMin, xMax, yMax);
        }
    static HGF2DTemplateExtent<T1, T2> Create(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax) {
        return HGF2DTemplateExtent<T1, T2>(xMin, yMin, xMax, yMax);
        }
    static HGF2DTemplateExtent<T1, T2> MergeExtents(const HGF2DTemplateExtent<T1, T2>& extent1, const HGF2DTemplateExtent<T1, T2>& extent2)
        {
        return Create(MIN(GetXMin(extent1), GetXMin(extent2)),
                      MIN(GetYMin(extent1), GetYMin(extent2)),
                      MIN(GetZMin(extent1), GetZMin(extent2)),
                      MAX(GetXMax(extent1), GetXMax(extent2)),
                      MAX(GetYMax(extent1), GetYMax(extent2)),
                      MAX(GetZMax(extent1), GetZMax(extent2)));
        }
    };

#endif

template <typename DataType> class ExtentOp<HGF3DExtent<DataType> >
    {
public:
    static double GetXMin(const HGF3DExtent<DataType>& extent) {
        return extent.GetXMin();
        }
    static double GetXMax(const HGF3DExtent<DataType>& extent) {
        return extent.GetXMax();
        }
    static double GetYMin(const HGF3DExtent<DataType>& extent) {
        return extent.GetYMin();
        }
    static double GetYMax(const HGF3DExtent<DataType>& extent) {
        return extent.GetYMax();
        }
    static double GetZMin(const HGF3DExtent<DataType>& extent) {
        return extent.GetZMin();
        }
    static double GetZMax(const HGF3DExtent<DataType>& extent) {
        return extent.GetZMax();
        }
    static void   SetXMin(HGF3DExtent<DataType>& extent, double xMin) {
        extent.SetXMin(xMin);
        }
    static void   SetXMax(HGF3DExtent<DataType>& extent, double xMax) {
        extent.SetXMax(xMax);
        }
    static void   SetYMin(HGF3DExtent<DataType>& extent, double yMin) {
        extent.SetYMin(yMin);
        }
    static void   SetYMax(HGF3DExtent<DataType>& extent, double yMax) {
        extent.SetYMax(yMax);
        }
    static void   SetZMin(HGF3DExtent<DataType>& extent, double zMin) {
        extent.SetZMin(zMin);
        }
    static void   SetZMax(HGF3DExtent<DataType>& extent, double zMax) {
        extent.SetZMax(zMax);
        }
    static double GetWidth(const HGF3DExtent<DataType>& extent) {
        return extent.GetWidth();
        }
    static double GetHeight(const HGF3DExtent<DataType>& extent) {
        return extent.GetHeight();
        }
    static double GetThickness(const HGF3DExtent<DataType>& extent) {
        return extent.GetThickness();
        }
    static bool   Overlap (const HGF3DExtent<DataType>& extent1, const HGF3DExtent<DataType>& extent2) {
        return extent1.Overlaps(extent2);
        }
    static bool   InnerOverlap (const HGF3DExtent<DataType>& extent1, const HGF3DExtent<DataType>& extent2) {
        return extent1.InnerOverlaps(extent2);
        }
    static bool   OutterOverlap (const HGF3DExtent<DataType>& extent1, const HGF3DExtent<DataType>& extent2) {
        return extent1.OuterOverlaps(extent2);
        }
    static HGF3DExtent<DataType> Create(double xMin, double yMin, double xMax, double yMax) {
        return HGF3DExtent<DataType>(xMin, yMin, 0.0, xMax, yMax, 0.0);
        }
    static HGF3DExtent<DataType> Create(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax) {
        return HGF3DExtent<DataType>(xMin, yMin, zMin, xMax, yMax, zMax);
        }
    static HGF3DExtent<DataType> MergeExtents(const HGF3DExtent<DataType>& extent1, const HGF3DExtent<DataType>& extent2)
        {
        return Create(MIN(GetXMin(extent1), GetXMin(extent2)),
                      MIN(GetYMin(extent1), GetYMin(extent2)),
                      MIN(GetZMin(extent1), GetZMin(extent2)),
                      MAX(GetXMax(extent1), GetXMax(extent2)),
                      MAX(GetYMax(extent1), GetYMax(extent2)),
                      MAX(GetZMax(extent1), GetZMax(extent2)));
        }
    };


/** -----------------------------------------------------------------------------

    ExtentPointOp<> class

    This template class provides a way to normalize the point/extent interaction
    class interface.
    Since the index has the point type and extent template argument and all
    point and extent classes have their own individiual interfaces, the point index
    makes use of the ExtentPointOp<> template class.

    -----------------------------------------------------------------------------
*/
template <class EXTENT, class POINT> class ExtentPointOp
    {
public:

    static bool IsPointOutterIn3D(const EXTENT& extent, const POINT& point)
        {
        HASSERT(0);
        return false;
        }

    static bool IsPointOutterIn2D(const EXTENT& extent, const POINT& point)
        {
        return extent.IsPointOutterIn (point);
        }
    };


template <class DataType, class POINT> class ExtentPointOp<HGF3DExtent<DataType>, POINT>
    {
public:

    static bool IsPointOutterIn3D(const HGF3DExtent<DataType>& extent, const POINT& point)
        {
        HASSERT(0);
        return false;//extent.IsPointIn2D (point, HNumeric<DataType>::GLOBAL_EPSILON(), false);
        }

    static bool IsPointOutterIn2D(const HGF3DExtent<DataType>& extent, const POINT& point)
        {
        return extent.IsPointIn2D (point, HNumeric<DataType>::GLOBAL_EPSILON(), false);
        }
    };


class UnknownSpatialFamily {};

class PointSpatialFamily {};

/** -----------------------------------------------------------------------------
    Trait that automatically infer the spatial object's type family. For the
    moment, spatial families includes: PointSpatialFamily and UnknownSpatialFamily.
    User can also develop his own point family (see PointFamilyTrait doc).
    -----------------------------------------------------------------------------
*/
template <class SPATIAL> struct SpatialFamilyTrait
    {
    // Infer whether a spatial is of the point spatial family based on the point family
    // trait.

    template <class POINT_FAMILY>
    struct SpatialFromPointFamily                       {
        typedef PointSpatialFamily type;
        };
    template <>
    struct SpatialFromPointFamily<UnknownPointFamily>   {
        typedef UnknownSpatialFamily type;
        };

    typedef typename SpatialFromPointFamily<typename PointFamilyTrait<SPATIAL>::type>::type type;
    };


/** -----------------------------------------------------------------------------

    SpatialOp<> class

    This template class provides a way to normalize the spatial class interface.
    Since the spatial index has the spatial type as template argument and all
    spatial classes have their own individiual interfaces, the spatial index
    makes use of the SpatialOp<> template class.

    Partial specializations are provided for spatial pointers pointed by use the
    the HFCPtr<> class
    Specialisation is provided for a 3D Point (HGF3DPoint<>)

    Partial specialization are also possible for a whole family of spatial by
    specifying the spacial family for which the template is specialized.
    e.g.:
    template <class SPATIAL, class POINT>
    class SpatialOp<SPATIAL, POINT, MyExtentType, PointSpatialFamily>
    -> Partially specialize spatial op for the whole PointSpatialFamily and
       for a specific user defined extent type. This kind of specialization
       enable usage of PointOp trait the retrieve the spacial object's
       properties and develop algorithms applicable on points.

    -----------------------------------------------------------------------------
*/
template <class SPATIAL, class POINT, class EXTENT, class SPATIAL_FAMILY = typename SpatialFamilyTrait<SPATIAL>::type> class SpatialOp
    {
public:
    static EXTENT GetExtent(const SPATIAL& spatial) {
        return spatial.GetExtent();
        }
    static bool IsPointIn2D (const SPATIAL& spatial, const POINT& point) {
        return spatial.IsPointIn(point);
        }
    static bool IsSpatialInExtent2D (const SPATIAL& spatial, const EXTENT& extent)
        {
        EXTENT spatialExtent = SpatialOp<SPATIAL, POINT, EXTENT>::GetExtent(spatial);
        POINT Origin = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMin(spatialExtent), ExtentOp<EXTENT>::GetYMin(spatialExtent), ExtentOp<EXTENT>::GetZMin(spatialExtent));
        POINT Corner = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMax(spatialExtent), ExtentOp<EXTENT>::GetYMax(spatialExtent), ExtentOp<EXTENT>::GetZMax(spatialExtent));
        return (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(spatialExtent, Origin) && ExtentPointOp<EXTENT, POINT>::IsPointOutterIn(spatialExtent, Corner));
        }
    static size_t GetPointCount (const SPATIAL& spatial)
        {
        return 1;
        }
    };

template <class SPATIAL, class POINT, class EXTENT> class SpatialOp<HFCPtr<SPATIAL>, POINT, EXTENT>
    {
public:
    static EXTENT GetExtent(const HFCPtr<SPATIAL>& spatial) {
        return spatial->GetExtent();
        }
    static bool IsPointIn2D (const HFCPtr<SPATIAL>& spatial, const POINT& point) {
        return spatial->IsPointIn(point);
        }
    static bool IsSpatialInExtent2D (const HFCPtr<SPATIAL>& spatial, const EXTENT& extent)
        {
        EXTENT spatialExtent = SpatialOp<HFCPtr<SPATIAL>, POINT, EXTENT>::GetExtent(spatial);
        POINT Origin = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMin(spatialExtent),
                                               ExtentOp<EXTENT>::GetYMin(spatialExtent),
                                               ExtentOp<EXTENT>::GetZMin(spatialExtent));
        POINT Corner = PointOp<POINT>::Create (ExtentOp<EXTENT>::GetXMax(spatialExtent),
                                               ExtentOp<EXTENT>::GetYMax(spatialExtent),
                                               ExtentOp<EXTENT>::GetZMax(spatialExtent));
        return (ExtentPointOp<EXTENT, POINT>::IsPointOutterIn2D(extent, Origin) && ExtentPointOp<EXTENT, POINT>::IsPointOutterIn(extent, Corner));
        }
    static size_t GetPointCount (const HFCPtr<SPATIAL>& spatial)
        {
        return 1;
        }

    };



template <> class SpatialOp<HGF3DCoord<double>, HGF3DCoord<double>, HGF2DTemplateExtent<double, HGF3DCoord<double> > >
    {

public:
    static  HGF2DTemplateExtent<double, HGF3DCoord<double> > GetExtent(const HGF3DCoord<double> spatialObject)
        {
        return  HGF2DTemplateExtent<double, HGF3DCoord<double> >(spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetX(), spatialObject.GetY());
        }

    static bool IsPointIn2D(const HGF3DCoord<double> spatialObject, HGF3DCoord<double> pi_rCoord)
        {
        return spatialObject.IsEqualTo2D (pi_rCoord);
        }

    static bool IsSpatialInExtent2D (const HGF3DCoord<double>& spatial, const  HGF2DTemplateExtent<double, HGF3DCoord<double> >& extent)
        {
        return extent.IsPointIn (spatial);
        }
    static size_t GetPointCount (const HGF3DCoord<double>& spatial)
        {
        return 1;
        }

    };



template <> class SpatialOp<HGF3DCoord<double>, HGF3DCoord<double>, HGF3DExtent<double> >
    {

public:
    static  HGF3DExtent<double> GetExtent(const HGF3DCoord<double> spatialObject)
        {
        return  HGF3DExtent<double>(spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetZ(), spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetZ());
        }

    static bool IsPointIn2D(const HGF3DCoord<double> spatialObject, HGF3DCoord<double> pi_rCoord)
        {
        return spatialObject.IsEqualTo2D (pi_rCoord);
        }

    static bool IsSpatialInExtent2D (const HGF3DCoord<double>& spatial, const HGF3DExtent<double>& extent)
        {
        return extent.IsPointIn2D (spatial);
        }
    static size_t GetPointCount (const HGF3DCoord<double>& spatial)
        {
        return 1;
        }


    };



/** -----------------------------------------------------------------------------

    The index node header is the base header for any spatial index nodes control. Since the
    spatial index node receives the actual node header type in parameter, the actual
    node header can be another however ALL members using the same name and type must be defined
    and be public for the provided node header. It is usually a lot easier to
    inherit any actual node header from the provided HGFIndexNodeHeader.
    -----------------------------------------------------------------------------
*/
template <typename EXTENT> class HGFIndexNodeHeader
    {
public:

    size_t      m_SplitTreshold;            // Holds the split treshold
    bool        m_IsLeaf;                   // Indicates if the node is a leaf
    bool        m_IsBranched;               // Indicates if the node is branched. This field is basically only used between node content load time and
    // unsplit sub-node construction as the presence of the unsplit sub-node is the indication
    // of split or unsplit parentage.
    bool        m_IsUnSplitSubLevel;        // Important control variable ... indicates if the node is the sub-node of an unsplit parent
    EXTENT      m_nodeExtent;               // The extent of the node (this extent is always defined immediately at the creation of the node)
    EXTENT      m_contentExtent;            // The extent of the content of the node ... this is not the same as the extent of the node.
    bool        m_contentExtentDefined;     // Indicates if the content extent has been initialised. Even if node is empty the content extent
    // can have been initialized since this extent includes sub-nodes content.
    size_t      m_level;                    // The level depth of the node in the index tree.
    bool        m_balanced;                 // Control variable that indicates if the tree must be balanced
    size_t      m_numberOfSubNodesOnSplit;  // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.

    bool        m_totalCountDefined;         // Indicates if the total count of objects in node and subnode is up to date
    uint64_t   m_totalCount;                // This value indicates the total number of points in node all recursively all sub-nodes.
    };


/** -----------------------------------------------------------------------------

    The index header is the base header for any spatial index control. Since the
    spatial index receives the actual index header type in parameter, the actual
    index header can be another however ALL members using the same name and type must be defined
    and be public for the provided header type. It is usually a lot easier to
    inherit any actual index header from the provided HGFSpatialIndexHeader.
    -----------------------------------------------------------------------------
*/
template <class EXTENT> class HGFSpatialIndexHeader
    {
public:

    size_t                  m_SplitTreshold;                // Holds the split treshold
    EXTENT                  m_MaxExtent;                    // Indicates the maximum extent if the spatial index is extent limited
    bool                    m_HasMaxExtent;                 // indicated if the index is extent limited.

    bool                    m_balanced;                     // Control variable that indicates if the tree must be balanced
    size_t                  m_numberOfSubNodesOnSplit;      // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.
    };



/** -----------------------------------------------------------------------------

    This class implements a spatial index node. It implements all required
    fucntionality required of a spatial index node.
    The class requires 6 template arguments which allow full configuration of the
    payload for the data. It is recommended to inherit from this node class
    a more specialised node as this current implementation is highly configurable but provides
    a far too complex interface.

    Template arguments are:
    SPATIAL - The type of spatial objects to index. The SPATIAL can be a pointer
              or the spatial object itself, given the appropriate SpatialOp<>
              specialisation is also provided.
    POINT   - The type of the point used in the index. The purpose of providing
              configuration for the point was initially to allow use of other
              point type than those present in Image++.
    EXTENT  - The type of the extent used in the index and its interface. The purpose of providing
              configuration for the extent was initially to allow use of other
              extent type than those present in Image++. Since the extent is used in the interface
              it also dictates the actual implementation returned by the API. Use
              of a 3D extent type is recommended yet not mandatory
              If the extent is 3D in nature then the node will maintain the Z dimension in
              one of two ways depending if the node is part of a quadtree or an octtree.
              If the node is part of an octtree then the Z dimension will behave exactly as for the other
              two dimensions. If the node is part of a quadtree then the Z dimension will be maintained
              as the bound of the node content and will increase accordingly to this content.

    CONTAINER - The type of the container used by the index. The container type must comform
                to a very limited subset of the STL std::vector<> interface. The
                container type must implement:
                 push_back()
                 operator[]
                 clear()
                 size()
                The index node INHERITS from the container and thus the container API is also
                available for the node intances. HOWEVER as the node recognises the ability for a
                node to be delay loaded, the external user of a node SHALL make sure that the node is
                loaded using (IsLoaded()) method and loading it if required using (Load()) prior to using
                the container API if this container does not implement this behavior.
                For example, if the container is a std::vector<SPATIAL> then this implementation
                does not call if (!IsLoaded()) Load(); prior to operation. It is recommended to use
                a container that calls if (!IsLoaded()) Load(); before all operations

    SELFNODEPTR - This selfnode pointer type MUST be an HFCPtr<> or define a typedef POINTED_TYPE
                for the class it points to. The template argument allows setting the
                index node API to take and return fully scoped descendants of itself
                The SELFNODEPTR class must be a pointer to a descendant of the present index node template class
                making the class be dependent on itself. The compiler does (for some unknown reason)
                support such self dependency by apparently resolving the final instanciation after the
                first compiling stages, allowing such weird behavior.

    NODEHEADER - The type of the node header. The node header will be declared as a protected
                member of this type allowing decendent classes to use a mode complete node header
                that should inherit from the HGFIndexNodeHeader class or define the exact same
                members.




    -----------------------------------------------------------------------------
*/
template <class SPATIAL, class POINT, class EXTENT, class CONTAINER, class SELFNODEPTR, class NODEHEADER> class HGFIndexNode : public CONTAINER, public HFCShareableObject<HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER> >
    {


private:
    SELFNODEPTR REALTHIS() const
        {
        return static_cast<SELFNODEPTR::POINTED_TYPE*>(const_cast<HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>*>(this));
        }


public:

    /**----------------------------------------------------------------------------
     Default Constructor

    -----------------------------------------------------------------------------*/
    HGFIndexNode(bool propagateDataDown);

    /**----------------------------------------------------------------------------
     Constructor for node based upon extent .

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.
                               Additional conditions can prevent temporarily or
                               permanently a split; refer to index documentation
                               for details.

     @param pi_rExtent - The extent of the node

     @param balanced - If true the node is part of a balanced tree. Balancing will
                       be enforced.

     @param dummyParam, dummyParam2, dummyParam3 Dummy parameters that must be provided.
                      These parameters have been made necessary by the inability of the
                      compiler to differentiate correctly between bool, pointers and integers
                      in construction overloads. Here the balanced parameter was confused
                      with pointers which caused serious runtime errors to occur.
    -----------------------------------------------------------------------------*/
    HGFIndexNode(size_t pi_SplitTreshold,
                 const EXTENT& pi_rExtent,
                 bool balanced, bool propagateDataDown, bool dummyParam2, bool dummyParam3);

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              Additional conditions can prevent temporarily or
                              permanently a split; refer to index documentation
                              for details.

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
    -----------------------------------------------------------------------------*/
    HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER> (size_t pi_SplitTreshold,
                                                                              const EXTENT& pi_rExtent,
                                                                              const SELFNODEPTR& pi_rpParentNode);

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              Additional conditions can prevent temporarily or
                              permanently a split; refer to index documentation
                              for details.

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node

     @param IsUnsplitSubLevel - This parameter indicates to the created node wether
     it is an unsplit sub node. In this case, it knowns that its parent is not
     split and upon requiring a split, the parent should be requested to do it.
    -----------------------------------------------------------------------------*/
    HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER> (size_t pi_SplitTreshold,
                                                                              const EXTENT& pi_rExtent,
                                                                              const SELFNODEPTR& pi_rpParentNode,
                                                                              bool IsUnsplitSubLevel);

    /**----------------------------------------------------------------------------
     Copy constructor - This constructor may only be called when the
     given node has no parent

     @param pi_rNode - Reference to node to duplicate. This node must have no parent
    -----------------------------------------------------------------------------*/
    HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER> (const HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>& pi_rNode);

    /**----------------------------------------------------------------------------
     Alternate Copy constructor ... equivalent but parent node is provided as parameter

     @param pi_rNode - Reference to node to duplicate.

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER>(const HGFIndexNode& pi_rNode,
                                                                             const HFCPtr<HGFIndexNode>& pi_rpParentNode);


    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    virtual ~HGFIndexNode<SPATIAL, POINT, EXTENT, CONTAINER, SELFNODEPTR, NODEHEADER> ();

    /**----------------------------------------------------------------------------
     Clone
     These methods create a duplicate of the self node of the same type. The node
     has no parent and no children except for the CloneChild() which automatically
     sets the parent relationship. The CloneUnsplitChild creates a child node
     that is an unsplit additional level in the index
    -----------------------------------------------------------------------------*/
    virtual SELFNODEPTR Clone () const = 0;
    virtual SELFNODEPTR Clone (const EXTENT& newNodeExtent) const = 0;
    virtual SELFNODEPTR CloneChild (const EXTENT& newNodeExtent) const = 0;
    virtual SELFNODEPTR CloneUnsplitChild (const EXTENT& newNodeExtent) const = 0;


    /**----------------------------------------------------------------------------
     Indicates that the node has been modified. For storable nodes this
     implies that the node should be restored.
     In addition indicates filtering must be reperformed.

     This method simply implements a way to discard cached pre-computed parameters
     that may have become invalid.

     There is no IsDirty() method nor permanent dirty state.

     NOTE: This method was originally part of the HPMPooledVector implementation
     that used to be the ancester CONTAINER of the original point index. If
     the CONTAINER type does contain a SetDirty() then the present implementation
     will override still not call the ancester method, rendering this CONTAINER
     dirty state and interface inoperative. If the CONTAINER does contain
     a SetDirty() method then a new descendant class of the present node class must implement
     a SetDirty() methos that calls both the present implementation and
     the CONTAINER implementation.
    -----------------------------------------------------------------------------*/
    virtual void SetDirty(bool dirty) const; // Intentionaly const ... only mutable members are modified

    /**----------------------------------------------------------------------------
     Sets the parent node ..

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    virtual void SetParentNode(const SELFNODEPTR& pi_rpParentNode);

    /**----------------------------------------------------------------------------
     Returns the parent node

     @return reference to the parent node
    -----------------------------------------------------------------------------*/
    const SELFNODEPTR& GetParentNode() const;


    /**----------------------------------------------------------------------------
     Sets the split treshold value for node

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.
    -----------------------------------------------------------------------------*/
    virtual void SetSplitTreshold(size_t pi_SplitTreshold);

    /**----------------------------------------------------------------------------
     Returns the split treshold value for node

    -----------------------------------------------------------------------------*/
    size_t GetSplitTreshold() const;

    /**----------------------------------------------------------------------------
     Returns the number of subnodes on split. This number can be 4 or 8

    -----------------------------------------------------------------------------*/
    size_t GetNumberOfSubNodesOnSplit() const;

    /**----------------------------------------------------------------------------
     Indicates if the node requires a balanced quadtree
    -----------------------------------------------------------------------------*/
    bool IsBalanced() const;


    /**----------------------------------------------------------------------------
     Changes an unbalanced index to a balanced index. If the index is already balanced
     nothing will occur otherwise the balancing will immediately be performed.
    -----------------------------------------------------------------------------*/
    void Balance(size_t depth);

    /**----------------------------------------------------------------------------
     Changes a balanced index to an unbalanced index. This function only sets the
     balance field of the node and subnodes to false. The topology of the index is not
     modified.
    -----------------------------------------------------------------------------*/
    void Unbalance();

    /**----------------------------------------------------------------------------
     Indicates if the node propagates data toward nodes upon addition or not.
    -----------------------------------------------------------------------------*/
    bool PropagatesDataDown() const;

    /**----------------------------------------------------------------------------
     Changes an Propagate Data Down setting. Changing this value will not provoke a
     propagation down of the data currnetly located in the parent nodes but deactivating
     this propagation will prevent data to immediately propagate data. Newly added
     data will remain in upper parent nodes till required.
    -----------------------------------------------------------------------------*/
    void SetPropagateDataDown(bool propagate);

    /**----------------------------------------------------------------------------
     This method provoques the propagation of data down immediately.
     Note that even if data propagation is off this function can be called. It will
     not change the PropagatesDataDown() value.
    -----------------------------------------------------------------------------*/
    virtual void PropagateDataDownImmediately(bool propagateRecursively = true);


    /**----------------------------------------------------------------------------
     Returns the depth level of a node in the quadtree
    -----------------------------------------------------------------------------*/
    size_t GetLevel() const;

    /**----------------------------------------------------------------------------
     Increases the level by one ... this increase will propagate to all subnodes.
    -----------------------------------------------------------------------------*/
    virtual void IncreaseLevel();

    /**----------------------------------------------------------------------------
     Returns the highest depth level of a node sub-quadtree
    -----------------------------------------------------------------------------*/
    size_t GetDepth() const;

    /**----------------------------------------------------------------------------
     Indicates if node is leaf

     @return true if node is a leaf
    -----------------------------------------------------------------------------*/
    bool IsLeaf() const;

    /**----------------------------------------------------------------------------
     Returns node extent

     @return reference to extent of node
    -----------------------------------------------------------------------------*/
    const EXTENT& GetNodeExtent() const;

    /**----------------------------------------------------------------------------
     Returns node content extent

     @return reference to extent of content of node
    -----------------------------------------------------------------------------*/
    const EXTENT& GetContentExtent() const;

    /**----------------------------------------------------------------------------
     Returns the number of objects in node and sub-nodes added

     @return Total number of objects in node and sub-nodes
    -----------------------------------------------------------------------------*/
    uint64_t GetCount() const;

    /**----------------------------------------------------------------------------
     Returns the number of levels for which there is splits leafward. This number
     can be used to identify the densest subnode of a node. The split level
     is the deepest level below the node for which there is still splitting.

     @return Split depth
    -----------------------------------------------------------------------------*/
    size_t GetSplitDepth() const;

    /**----------------------------------------------------------------------------
     Returns the sub-node no split of the quadtree.

     @return Sub-node no split
    -----------------------------------------------------------------------------*/
	SELFNODEPTR GetSubNodeNoSplit() const;

    /**----------------------------------------------------------------------------
     Indicates if the node or one of its sub-nodes contains data.

     @return true if node and sub-nodes are empty
    -----------------------------------------------------------------------------*/
    bool IsEmpty() const;

    /**----------------------------------------------------------------------------
     This method advises that one of the subnode storage block ID has changed
     as a result of reallocation, destruction or initial store.

     @param p_subNode - Pointer to sub node for which the ID changed
    -----------------------------------------------------------------------------*/
    virtual void AdviseSubNodeIDChanged(const SELFNODEPTR& p_subNode);


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node if it is contained in the extent
     or if the extent of the node can be extended.

     @param Pointer to spatial object to add a reference to in the index

     @param ExtentFixed IN indicates if the extent can be increased to include given
             object. To be meaningfull, the node must be the first and only node of the tree
             which means it is a leaf and has no parent, otherwise this parameter
             value is ignored.

     @return True if the object could be added and false otherwise.
    -----------------------------------------------------------------------------*/
    bool AddConditional (const SPATIAL& pi_rpSpatialObject, bool ExtentFixed);

    /**----------------------------------------------------------------------------
     Adds a list of reference to spatial object in node if it can. The method will
     add the objects that fit into the node starting at the first object till an object does
     not fit. The method will not continue into the list to add spatial objects that would
     have been included in the node but will stop at the first occurence that does not fit into
     the node.

     @param Pointer to spatial object array to add a reference to in the index

     @param startSpatialIndex IN the first index in the array of spatial
               objects to add (Previous objects are assumed to have already been added)

     @param countSpatials IN The number of spatial object in array

     @param ExtentFixed IN indicates if the extent can be increased to include given
             object. To be meaningfull, the node must be the first and only node of the tree
             which means it is a leaf and has no parent, otherwise this parameter
             value is ignored.

     @return The index of the last added spatial object. If the returned value
             is countSpatials then all spatial objects of the array have been added.

    -----------------------------------------------------------------------------*/
    virtual size_t AddArrayConditional (SPATIAL* spatialArray, size_t startSpatialIndex, size_t countSpatials, bool ExtentFixed);

    /**----------------------------------------------------------------------------
     Adds a list of spatial objects unconditionaly. All object must be satisfactory
     (be included in the node extent) for this node.

     @param spatialArray, Pointer to spatial object array to add a reference to in the index

     @param countSpatial IN The number of spatial objects in the array.
    -----------------------------------------------------------------------------*/
    virtual bool AddArrayUnconditional(SPATIAL* spatialArray, size_t countSpatial);


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index

     @param true if the object could be added and false otherwise.
    -----------------------------------------------------------------------------*/
    virtual bool Add(const SPATIAL& pi_rpSpatialObject);

    /**----------------------------------------------------------------------------
     Removes a reference to spatial object in node.

     @param Pointer to spatial object to remove reference to in the index

     @param true if the object has been found and removed and false otherwise.
    -----------------------------------------------------------------------------*/
    virtual bool Remove(const SPATIAL& pi_rpSpatialObject);


    /**----------------------------------------------------------------------------
     Gets a list of objects potentially located inside given extent. The method does
     not perform precise geometric calculations to determine presence of spatial object
     in given extent. It simply compares the extents of spatial object and given extent
     to see if they overlap. This implies that some or many spatial objects may be located
     completely outside of the given extent.

     The node will search within subnodes.

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects which are potentially located in
                              the given extent

    -----------------------------------------------------------------------------*/
    virtual size_t GetIn(const EXTENT& pi_rExtent,
                         list<SPATIAL>& pio_rListOfObjects) const;

    /**----------------------------------------------------------------------------
     Clears all spatial objects that are completely included in given shape.

     @param pi_shapeToClear IN The shape that defines the area to clear.
    -----------------------------------------------------------------------------*/
//        virtual bool Clear(HFCPtr<HVEShape> pi_shapeToClear);

    /**----------------------------------------------------------------------------
     Loads the present tile if delay loaded.
    -----------------------------------------------------------------------------*/
    virtual void Load() const; // Intentionaly const as only mutable members are modified

    /**----------------------------------------------------------------------------
     Unloads the present node if delay loaded.
    -----------------------------------------------------------------------------*/
    virtual void Unload() const; // Intentionaly const as only mutable members are modified

    /**----------------------------------------------------------------------------
     This method indicates if the node is loaded or not.
    -----------------------------------------------------------------------------*/
    bool IsLoaded() const;


    /**----------------------------------------------------------------------------
     This method indicates that a split node just occured. For a balanced index
     the maximum level depth of each branch must be equal. The method will propagate
     this increase in depth toward sub-nodes and parent node except if it is the
     calling node. The method will of course result in an increase of the depth by
     split or unsplit sub-node generation.

     @param initiator Pointer to node that calls this function. This node will not be called
     during the message propagation thus preventing infinite loops.

     @param quadTreeDepth IN the final depth of the quadtree. This value is used for
             balanced quadtrees
    -----------------------------------------------------------------------------*/
    virtual void PropagateSplitNode(SELFNODEPTR initiator, size_t quadTreeDepth);

    /**----------------------------------------------------------------------------
     This method pushes the root down, by creating a single sub-node and pushing
     all spatials down to this subnode. The process is repeated till the desired provided
     target level. This method is only called for balanced quadtrees

     @param targetLevel IN The level depth desired.
    -----------------------------------------------------------------------------*/
    virtual void PushNodeDown(size_t targetLevel);

    /**----------------------------------------------------------------------------
     This method is called by a non-split subnode to its parent. The purpose
     is to advise that a split should have been performed, yet such a split cannot be performed
     at an unsplit node level. The non-split node calls his parent that if this parent is
     also unsplit will propagate the event to its parent, up until the initial unsplit node
     is reached.
     This initial unsplit node will remember that it must split and will split at the earliest
     possible moment. Typically, this call occurs during batch addition of spatial objects
     into the node. The addition causes the split treshold to be reached but the split
     is delayed till the control can be given back to the initial unsplit node.

     This method is only used for unsplit nodes thus will only be called for
     balaced quadtrees
    -----------------------------------------------------------------------------*/
    virtual void AdviseDelayedSplitRequested() const;

    /**----------------------------------------------------------------------------
     This method indicates if the node is destroyed. Typically a destroyed node
     is a node that used to be an unsplit sub-node that got orphaned
     following a subsequent split. The destroyed state reminds this node
     that it must not attempt to store itself upon destruction.
    -----------------------------------------------------------------------------*/
    virtual bool IsDestroyed() const;

    /**----------------------------------------------------------------------------
     This method destroys a node. Typically a destroyed node
     is a node that used to be an unsplit sub-node that got orphaned
     following a subsequent split. The destroyed state reminds this node
     that it must not attempt to store itself upon destruction.

     The very first step of the Destroy method is to sever the relation with its
     parent node. The next one is to destroy all sub-nodes. Then the node
     content is cleared and then any additional steps preventing this node
     to be loaded is performed.
    -----------------------------------------------------------------------------*/
    virtual bool Destroy();

    /**----------------------------------------------------------------------------
     This method causes a split of the node to occur. This method can only be
     called on a leaf node or the parent of an unsplit subnode.
     The optional parameter indicates if the split event must be propagated
     to the whole tree for index balancing purposes.
    -----------------------------------------------------------------------------*/
    virtual void SplitNode(bool propagateSplit = true);



    // Did tried to make this not public but it is accessed by HGFSpatialIndex which has some unknown and undefinable template arguments by the node so
    // it cannot be made friend either.
    SELFNODEPTR m_apSubNodes[MAX_NUM_SUBNODES];
    mutable NODEHEADER m_nodeHeader;         // The node header. Contains permanent control data.


    virtual void               ValidateInvariants() const
        {
        ValidateInvariantsSoft();

#ifdef __HMR_DEBUG
        // We only check invariants if the node is loaded ...
        if (IsLoaded() && !m_destroyed)
            {

            // The following verifications  require that subnodes or parent nodes be loaded
            if (!m_nodeHeader.m_IsLeaf)
                {
                if ((m_pSubNodeNoSplit != NULL) && (!m_pSubNodeNoSplit->IsLoaded()))
                    return;

                if (m_pSubNodeNoSplit == NULL)
                    {
                    for (size_t indexNodes = 0 ; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                        {
                        if (!m_apSubNodes[indexNodes]->IsLoaded())
                            return;
                        }
                    }
                }

            if (m_pParentNode != NULL)
                {
                if (!m_pParentNode->IsLoaded())
                    return;

                }

            if (m_nodeHeader.m_totalCountDefined)
                {
                // The total number of points in a leaf node must be the node content itself
                HASSERT(!m_nodeHeader.m_IsLeaf || (m_nodeHeader.m_totalCount == this->size()));

                // If the node is not leaf and is unsplit then total count must be equal to self size + unsplit child
                HASSERT(m_nodeHeader.m_IsLeaf || (m_pSubNodeNoSplit == 0) || (m_nodeHeader.m_totalCount == this->size() + m_pSubNodeNoSplit->m_nodeHeader.m_totalCount));

                // If the node is not leaf and is unsplit then total count must be equal to self size + unsplit child
                if (!m_nodeHeader.m_IsLeaf && m_pSubNodeNoSplit == 0)
                    {
                    uint64_t effectiveTotalCount = this->size();
                    for (size_t indexNodes = 0 ; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                        {
                        effectiveTotalCount += m_apSubNodes[indexNodes]->m_nodeHeader.m_totalCount;
                        }
                    HASSERT(m_nodeHeader.m_totalCount == effectiveTotalCount);
                    }
                }


            // Either case the node is an unsplit sublevel which implies it must have a parent and this parent may node be a leaf and the subnode no split must
            // point to this node. OR the node is not an unsplit sublevel which implies that either it has no parent or the parent is not a leaf and does not
            // point to an unsplit node.
            HASSERT ((m_nodeHeader.m_IsUnSplitSubLevel && m_pParentNode != NULL && !m_pParentNode->m_nodeHeader.m_IsLeaf && (m_pParentNode->m_pSubNodeNoSplit == this)) ||
                     (!m_nodeHeader.m_IsUnSplitSubLevel && ((m_pParentNode == NULL) || (!m_pParentNode->m_nodeHeader.m_IsLeaf && (m_pParentNode->m_pSubNodeNoSplit == NULL)))));

            // Balancing must be homogenous throughout the quadtree
            if (!m_nodeHeader.m_IsLeaf)
                {
                if (m_pSubNodeNoSplit != NULL)
                    {
                    HASSERT (m_pSubNodeNoSplit->m_nodeHeader.m_balanced == m_nodeHeader.m_balanced);
                    }
                else
                    {
                    for (size_t indexNodes = 0 ; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                        {
                        HASSERT (m_apSubNodes[indexNodes]->m_nodeHeader.m_balanced == m_nodeHeader.m_balanced);
                        }
                    }
                }
            if (m_pParentNode != NULL)
                {
                HASSERT (m_pParentNode->m_nodeHeader.m_balanced == m_nodeHeader.m_balanced);
                }



            }

#endif
        }


    // This invariant validation is called when the nodes are reconfiguring. It does not validate the node topology.
    virtual void               ValidateInvariantsSoft() const
        {
#ifdef __HMR_DEBUG
        // We only check invariants if the node is loaded ...
        if (IsLoaded())
            {

            // If the node is a leaf, then subnodes must not exist
            HASSERT(!m_nodeHeader.m_IsLeaf || ((m_pSubNodeNoSplit == 0) && (m_apSubNodes[0] == 0) && (m_apSubNodes[1] == 0) && (m_apSubNodes[2] == 0) && (m_apSubNodes[3] == 0)));

            // If the node is not a leaf, then subnodes must exist
            HASSERT(m_nodeHeader.m_IsLeaf || (m_pSubNodeNoSplit != 0) || ((m_apSubNodes[0] != 0) && (m_apSubNodes[1] != 0) && (m_apSubNodes[2] != 0) && (m_apSubNodes[3] != 0)));


            // If the node is a leaf, then split treshold must not be attained
            // This condition cannot be true anymore as split can be delayed.
            // HASSERT(!m_nodeHeader.m_IsLeaf || this->size() < m_nodeHeader.m_SplitTreshold);


            // The content extent must fit completely into the node extent at all times (if defined).
            if (m_nodeHeader.m_contentExtentDefined)
                {
                HASSERT(ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_contentExtent) >= ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent));
                HASSERT(ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_contentExtent) <= ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent));
                HASSERT(ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_contentExtent) >= ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent));
                HASSERT(ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_contentExtent) <= ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));
                }

            // Make sure that the subnodes fallows some rules
            if (!m_nodeHeader.m_IsLeaf)
                {
                if (m_pSubNodeNoSplit != NULL)
                    {
                    if (m_pSubNodeNoSplit->IsLoaded())
                        {
                        HASSERT(m_nodeHeader.m_balanced == m_pSubNodeNoSplit->m_nodeHeader.m_balanced);

                        HASSERT(ExtentOp<EXTENT>::GetXMin(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent));
                        HASSERT(ExtentOp<EXTENT>::GetXMax(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent));
                        HASSERT(ExtentOp<EXTENT>::GetYMin(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent));
                        HASSERT(ExtentOp<EXTENT>::GetYMax(m_pSubNodeNoSplit->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));
                        }

                    }
                else
                    {
                    for (size_t indexNodes = 0 ; indexNodes < m_nodeHeader.m_numberOfSubNodesOnSplit ; indexNodes++)
                        {
                        if (m_apSubNodes[indexNodes]->IsLoaded())
                            {
                            HASSERT(m_nodeHeader.m_balanced == m_apSubNodes[indexNodes]->m_nodeHeader.m_balanced);
                            HASSERT(ExtentOp<EXTENT>::GetXMin(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetXMin(m_nodeHeader.m_nodeExtent));
                            HASSERT(ExtentOp<EXTENT>::GetXMax(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetXMax(m_nodeHeader.m_nodeExtent));
                            HASSERT(ExtentOp<EXTENT>::GetYMin(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) >= ExtentOp<EXTENT>::GetYMin(m_nodeHeader.m_nodeExtent));
                            HASSERT(ExtentOp<EXTENT>::GetYMax(m_apSubNodes[indexNodes]->m_nodeHeader.m_nodeExtent) <= ExtentOp<EXTENT>::GetYMax(m_nodeHeader.m_nodeExtent));
                            }

                        }
                    }
                }

            // If the count of points is zero then the memory must be cleared.
            }

#endif
        }





protected:



    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes pointer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
    virtual void SetSubNodes(SELFNODEPTR pi_apSubNodes[], size_t numSubNodes);

    virtual void SetNodeExtent(const EXTENT& extent);



    SELFNODEPTR m_pParentNode;      // Parent node
    size_t 		m_numSubNodes;           

    SELFNODEPTR m_pSubNodeNoSplit;  // Pointer to non-split sub-node. If NULL then node is either a leaf or has split sub-nodes
    mutable bool m_loaded;
    mutable bool m_destroyed;


    // The following members are currently used for invariant validation purposes
    // Indicates when a node cannot be split. The most likely reason is that the coordinates extremes are very close
    // to the maximum number of digits.
    HDEBUGCODE(mutable bool m_unspliteable;)
    // The following member indicates that one of its sub-node or sub-node of sub-node is unspliteable
    // Since in the case of an unspliteable node then number of points can exceed the number of
    // allowed treshold points, after filtering the parent can likewise exceed this number
    // This member is set upon filtering only. If the node has never been filtered then it is meaningless.
    HDEBUGCODE(mutable bool m_parentOfAnUnspliteableNode;)

    //Cached here but should eventually be in the header
    mutable int32_t m_NbObjects;     // The total number of object in and over(in parent nodes (within tile extent)) this node.

    mutable bool m_DelayedSplitRequested;    // Control variable. Indicates a split is requested for the initial parent of un-split sub-nodes.

    bool m_delayedDataPropagation;



    };



/** -----------------------------------------------------------------------------

    This class implements a spatial index default node. It implements a concrete
    HGFIndexNode by implementing the various Clone() methods and setting the
    Node type to self. This sub-nodes does not implement additional behavior.

    This class is suitable for general use of spatial objects but may be not
    efficient enough for storing mass amount of points where some HGFIndexNode
    should propably be overridden.

    -----------------------------------------------------------------------------
*/
template <class SPATIAL, class POINT, class EXTENT> class HGFSpatialIndexDefaultNode : public HGFIndexNode<SPATIAL, POINT, EXTENT, vector<SPATIAL>, HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> >, HGFIndexNodeHeader<EXTENT> >
    {
public:
    /**----------------------------------------------------------------------------
     Constructor for node based upon extent only.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.
                               Additional conditions can prevent temporarily or
                               permanently a split; refer to index documentation
                               for details.

     @param pi_rExtent - The extent of the node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexDefaultNode(size_t pi_SplitTreshold,
                               const EXTENT& pi_rExtent,
                               bool balanced,
                               bool propagateDataDown)
        : HGFIndexNode<SPATIAL, POINT, EXTENT, vector<SPATIAL>, HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> >, HGFIndexNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, balanced, propagateDataDown, false, false)
        {
        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.
                               Additional conditions can prevent temporarily or
                               permanently a split; refer to index documentation
                               for details.

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> (size_t pi_SplitTreshold,
                                                        const EXTENT& pi_rExtent,
                                                        const HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> >& pi_rpParentNode)
        : HGFIndexNode<SPATIAL, POINT, EXTENT, vector<SPATIAL>, HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> >, HGFIndexNodeHeader<EXTENT> >(pi_SplitTreshold, pi_rExtent, pi_rpParentNode)
        {
        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Copy constructor - This constructor may only be called when the
     given node has no parent

     @param pi_rNode - Reference to node to duplicate. This node must have no parent
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> (const HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT>& pi_rNode)
        : HGFIndexNode<SPATIAL, POINT, EXTENT, vector<SPATIAL>, HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> >, HGFIndexNodeHeader<EXTENT> >(pi_rNode)
        {
        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Create
    These methods create a duplicate of the self node of the same type. The node
    has no parent and no children except for the Clonechild() which automatically
    sets the parent relationship. The CloneUnsplitChild creates a child node
    that is an unsplit additional level in the index
    -----------------------------------------------------------------------------*/
    virtual HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> > Clone () const
        {
        return new HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT>(GetSplitTreshold(), GetExtent(), IsBalanced(), PropagatesDataDown());
        }
    virtual HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> > Clone (const EXTENT& newNodeExtent) const
        {
        return new HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, IsBalanced(), PropagatesDataDown());
        }
    virtual HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> > CloneChild (const EXTENT& newNodeExtent) const
        {
        return new HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT>* >(this));
        }
    virtual HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> > CloneUnsplitChild (const EXTENT& newNodeExtent) const
        {
        return new HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT>(GetSplitTreshold(), newNodeExtent, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT>* >(this));
        }

    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    virtual ~HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> () {}


protected:
    void               ValidateInvariants() const
        {
        HGFIndexNode<SPATIAL, POINT, EXTENT, vector<SPATIAL>, HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, POINT, EXTENT> >, HGFIndexNodeHeader<EXTENT> >::ValidateInvariants();
        }

    };









/** -----------------------------------------------------------------------------

    The following classes implement a spatial index. The index is based on a quadtree
    (or octtree) structure. The architecture of the index makes it so that a single
    function needs to be overloaded in order to change the spliting algorithm that
    ultimately dictates if the spatial index has 4 or 8 sub-nodes and what is their
    location into space.

    In order to be the most general, the spatial container type is undecided, yet this
    container must comply with a simplified STL std::vector interface a few of these
    function can be overriden if needed.

    The spatial index struture derives from a prototype specifically designed for managing
    DTM (Digital Terrain Model) points which usually contain 2.5D set of points.
    The spatial index structure recognises the necessity of providing various options
    for various needs. The base structure of the index is a quadtree and it can be
    configured and used as a plain quadtree, where the spatials indexed will always be
    stored at a leaf of the quadtree index.


    INDEX STRUCTURE
    The index structure is based mainly on the quadtree architecture, however some configurations
    change this structure in some case. When the index is populated with spatials, the
    spatials fill index nodes. Initially there is a single node which gets filled and
    the extent of the node content increases adequately to include spatials added. The extent
    is maintained square to prevent the construction of long and thin node extents that could result from
    ordered spatial sets provided as strip (such as resulting from reading a stripped DEM raster elevation
    file). When the number of spatials in the node reaches some split treshold amount of spatials,
    the node is split and the points of the split node are distributed to the created sub-nodes, initially
    completely emptying the split node (subsequent filtering may repopulate back the new parent node eventually).
    Once the alignement is set and the root node has split, it is still possible to add spatials outside
    the root node extent, but this will simply trigger the creation of a new root node and the initial root node
    will become one of the subnodes. The new root extent alignement is based upon the initial root extent
    and the location depend on the extent of the spatial being added.

    Once the first root node is split, the extent of all nodes cannot be modified arbitrarily,
    and the node alignement becomes fixed. The split function can be overloaded and configure the
    following behaviors. The SplitNode() method must create any number of sub-nodes but the
    union of the extent of the subnodes must cover exactly the same area as the split nodes. Usually
    the sub-node extents will be disjoint one from another, yet no ill effect were identified about a possible
    overlapping sub-node extents. The default implementation will split node in equal size sub-nodes
    thus splitting the original extent in two in both dimensions (or three dimensions if octal split is used).

    The index recognises the concept of LEVEL. The level is the depth within the index from the root. Thus the root level is always
    0, its sub-nodes are at level 1, and so on for the sub-nodes of the sub-nodes. It is possible to impose the
    balancing of the index. If the index is balanced, then the index will guarantee that the depth of the leaf nodes
    will be the same for the whole tree, regardless of the density or size of spatial objects and the split treshold. This means that
    if a leaf is being split as a result of containing more spatial objects than allowed by the split treshold
    then it will first split, then propagate a message through all the index nodes about the final
    depth required. All leaf nodes that do not satisfy this depth level will increase their depth typically through non-splitting.

    The concept of index balancing result from the necessity to homogenize object density when spatial objects are points
    across a same level. This does not necessarily mean that the amount of spatial objectss between nodes of a same
    level are similar, on the contrary. It means that the  density of a node can be related by a predictable
    rule to the density of the leaf nodes. The filtering process which is not part of the base index can become important to
    understand the various effects of index balancing but this concerns the point index descendant class.
    For example, assuming the filtering process will takes all
    points of a sub-node and populate the parent node with 1 node out of every four points in the subnodes. If the
    subnodes have high density compared to other sub-nodes of the same level then the parent node will also be high density relatively
    to the nodes at this same parent node level.

    Splitting nodes in 4 sub-nodes just to increase the level depth, is not an economical
    structure in matter of points per node, for this reason the index will increase level by simply
    creating a single and unique subnode. In this case the index remains balanced yet deviates significantly from the
    normal quadtree structure. The result greatly minimizes the amount of nodes in the index yet provides the same
    spatial indexing and balancing capabilities. It is the optimal structure if index balancing is required.
    Note that as the index is populated, it may happen
    that a leaf node that happens to be the single sub-node of a node will have to split as the amount of
    points has reached the split treshold. In this case, the node will NOT be split, It will instead trigger and
    event to its parent node, for this parent node to be split instead. If this occurs then the single-sub-node will be replaced
    by 4 sub-nodes. Since this process will occur during point addition, the process may be temporarily delayed
    after the addition process has completed, resulting in a node having momentarily more points than normally allowed.
    Notice that if the single node subdivision is many levels deep, the split process will always occur at the most
    rootward node that has not been split.

    There are extremely rare occurences of data located within the limits of double floating point representation (15 or so digits)
    the node splitting cannot be performed as the node extent would be null-sized. In such case node splitting will not occur and the
    node will be designated as "unspliteable". In this case the node content may far exceed the treshold limit. This mechansim should accomodate
    strange datasets that have, by error or intent, numerous (thousands!?) of spatial objects at the same location.

    MEMORY MANAGEMENT

    The index does not provide any memory management proper, yet the fact the container type is given as parameters allows
    for the use of a memory managed container to be used.


    PERMANENT STORAGE AND DATA RETRIEVAL

    The index does not provide any permanent storage of storage data retreival proper, yet the fact the container type is given as parameters allows
    for the use of a persistance enabled container to be used. The index node may provide help method such as
    IsLoaded() and Load() which, by default, do nothing. For persistancy storage, the methods may be overridden as appropriate.


    INDEX MODIFICATION

    The index implementation was initally based on the assumption that no data modification will occur
    except for the addition of new spatial obejcts. Of course this assumption can only be considered temporary as the purpose of have data
    to manage is to manage this data. Spatial object addition and removal, as well as modification will occur. For this
    reason the index provides a full interface for such operation. As modification of data (including object addition) may
    have a significant impact on the data after the, some possible

    -----------------------------------------------------------------------------
*/
template<class SPATIAL, class POINT, class EXTENT, class NODE, class INDEXHEADER> class HGFSpatialIndex : public HFCShareableObject<HGFSpatialIndex<SPATIAL, POINT, EXTENT, NODE, INDEXHEADER>>
    {

public:

    // Primary methods
    /**----------------------------------------------------------------------------------------------
     Constructor for this class. The split threshold is used to indicate the maximum
     amount of spatial objects to be indexed by an index node after which the node is
     split.

        @param pi_SplitTreshold IN The maximum number of items per spatial index
                                   node after which the node may be split.

        @param balanced IN If true the index will be balanced.

        @param exampleNode IN This node is provided as an example of the actual node type
        to be used for the index nodes. The parameters of the actual node implementation
        that are external to the spatial index must be set in the example node exactly how
        they are meant to be in the actual nodes. This example node for example can be used
        for storable nodes (storage is a behavior external to the index) to contain pointers to
        their store object structure.

    -------------------------------------------------------------------------------------------------*/
    HGFSpatialIndex(size_t SplitTreshold, bool balanced, bool propagatesDataDown, HFCPtr<NODE> exampleNode);
//                            HGFSpatialIndex(EXTENT const & pi_rMaxExtent, size_t SplitTreshold, bool balanced, HFCPtr<NODE> exampleNode);
    virtual             ~HGFSpatialIndex();


    /**----------------------------------------------------------------------------
     Indicates if the data is propagated toward the leaves immediately or if it is
     delayed till the most appropariate moment (re-filtering for example)

     @return true if the data is immediately propagated towards the leaves.

    -----------------------------------------------------------------------------*/
    bool                PropagatesDataDown() const;

    /**----------------------------------------------------------------------------
     Changes an Propagate Data Down setting. Changing this value will not provoke a
     propagation down of the data currently located in the parent nodes but deactivating
     this propagation will prevent data to immediately propagate data. Newly added
     data will remain in upper parent nodes till required.
    -----------------------------------------------------------------------------*/
    void SetPropagateDataDown(bool propagate);

    /**----------------------------------------------------------------------------
     This method provoques the propagation of data down immediately.
     Note that even if data propagation is off this function can be called. It will
     not change the PropagatesDataDown() value.

     This emthod may result in an increase of the depth of the index as propagating
     data down may result in new node splitting as a result of split treshold being
     attained.
    -----------------------------------------------------------------------------*/
    virtual void PropagateDataDownImmediately();

    /**----------------------------------------------------------------------------
     Indicates if the node is part of a balanced quadtree

     @return true if the quadtree is balanced and false otherwise.

    -----------------------------------------------------------------------------*/
    bool                IsBalanced() const;
    /**----------------------------------------------------------------------------
     Changes an unbalanced index to a balanced index. If the index is already balanced
     nothing will occur otherwise the balancing will immediately be performed.
    -----------------------------------------------------------------------------*/
    void                Balance();

    /**----------------------------------------------------------------------------
     Returns the number of subnodes on split. This number can be 4 or 8

    -----------------------------------------------------------------------------*/
    size_t              GetNumberOfSubNodesOnSplit() const;

    /**----------------------------------------------------------------------------
     Returns the highest depth level of the quadtree

     @return The highest depth level

    -----------------------------------------------------------------------------*/
    size_t              GetDepth() const;

    /**----------------------------------------------------------------------------
     Returns the root node of the quadtree

     @return The root node  
     
    -----------------------------------------------------------------------------*/
    HFCPtr<NODE>        GetRootNode() const;

    /**----------------------------------------------------------------------------
     This method adds a spatial object in the spatial index

     @param pi_rpSpatialObject IN The spatial object to index.
     No check are performed to verify that the spatial object is already indexed or not.
     If the spatial object is already indexed then it will be indexed twice.

     @return true if item is added and false otherwise.
    -----------------------------------------------------------------------------*/
    virtual bool        Add(const SPATIAL& pi_pSpatialObject);

    /**----------------------------------------------------------------------------
     Adds a list of reference to spatial object in index. The method will
     No check are performed to verify that the spatial object is already indexed or not.
     If the spatial object is already indexed then it will be indexed twice.

     @param Pointer to spatial object array to add a reference to in the index

     @param countSpatials IN The number of spatial object in array


     @return true if items are added and false otherwise.

    -----------------------------------------------------------------------------*/
    virtual bool        AddArray (SPATIAL* spatialsArray, size_t countOfSpatials);

    /**----------------------------------------------------------------------------
     Removes the specified object from the index

     @return true if object was removed and false otherwise.

    -----------------------------------------------------------------------------*/
    virtual bool        Remove(const SPATIAL& pi_pSpatialObject);

//        virtual bool        Clear(HFCPtr<HVEShape> shapeToClear);
    /**----------------------------------------------------------------------------
     Gets a list of objects potentially located inside given extent. The method does
     not perform precise geometric calculations to determine presence of spatial object
     in given extent. It simply compares the extents of spatial object and given extent
     to see if they overlap. This implies that some or many spatial objects may be located
     completely outside of the given extent.

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects which are potentially located in
                              the given extent

    -----------------------------------------------------------------------------*/
    virtual size_t      GetIn(const EXTENT& pi_rExtent,
                              list<SPATIAL>& pio_rListOfObjects) const;

    /**----------------------------------------------------------------------------
     Gets the effective limiting outter extent. This extent is the node extent
     of the root node.

     @return Returns the extent of spatial index.
    -----------------------------------------------------------------------------*/
    EXTENT              GetIndexExtent() const;
    /**----------------------------------------------------------------------------
     Gets the effective limiting outter extent of the content.

     @return Returns the extent of the content of the spatial index.
    -----------------------------------------------------------------------------*/
    EXTENT              GetContentExtent() const;

    /**----------------------------------------------------------------------------
     Returns the total number of objects in index

     @return The number of objects

    -----------------------------------------------------------------------------*/
    uint64_t            GetCount() const;

    /**----------------------------------------------------------------------------
     Indicates if the index is empty

     @return true if the index is empty and false otherwise

    -----------------------------------------------------------------------------*/
    bool                IsEmpty() const;

    /**----------------------------------------------------------------------------
     Returns the split treshold value for index. This treshold value is the
     maximum number of items idexed by a single node before splitting occurs.

     @return The treshold value.

    -----------------------------------------------------------------------------*/
    size_t              GetSplitTreshold() const;

protected:

    HGFSpatialIndex(const HGFSpatialIndex&   pi_rSpatialIndex);

    HGFSpatialIndex&
    operator=(const HGFSpatialIndex& pi_rObj);


    virtual void        PushRootDown(const EXTENT& pi_rObjectExtent);
    void                SetSplitTreshold(size_t pi_SplitTreshold);

    bool                HasMaxExtent () const;
    EXTENT              GetMaxExtent () const;

    virtual void                ValidateInvariants() const
        {

#ifdef __HMR_DEBUG

//            if (m_pRootNode != NULL)
//                HASSERT(m_pRootNode->GetDepth() == m_pRootNode->GetSplitDepth());
        // Notice that even if we have strong aggregation we do not check invariants of root node
#endif
        };


protected:

    HFCPtr<NODE>            m_pExampleNode;

    HFCPtr<NODE>            m_pRootNode;

    INDEXHEADER             m_indexHeader;

    bool                    m_indexHeaderDirty;

    bool                    m_propagatesDataDown;

    };


#include "HGFSpatialIndex.hpp"


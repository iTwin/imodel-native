//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/ISMDataStore.hpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



#include <../STM/IScalableMeshDataStore.h>
/*#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <ImagePP/all/h/HGFSpatialIndex.h>
#include <ImagePP/all/h/HVEDTMLinearFeature.h>
#include <ImagePP/all/h/HGFPointTileStore.h>*/
#include <BeJsonCpp/BeJsonUtilities.h>
#include "SMSQLiteFile.h"
USING_NAMESPACE_IMAGEPP

namespace IDTMFile
    {
    typedef HFCAccessMode                   AccessMode;
    typedef uint32_t NodeID;
    typedef uint32_t FeatureType;
    inline uint32_t       GetNullNodeID               () {
        return std::numeric_limits<uint32_t>::max();
        };

    struct FeatureHeader
        {
        typedef uint32_t                     feature_type;
        typedef uint32_t                     group_id_type;
        typedef uint32_t                     index_type;
        typedef uint32_t                     size_type;

        static group_id_type         GetNullID();

        feature_type                        type;
        index_type                          offset;     // In points
        size_type                           size;       // In points
        group_id_type                       groupId;    // Reference to the metadata for this feature
        };
    };
//#if 0
class IDTMFilePointFamily {};

/*template <> struct PointFamilyTrait<DPoint3d>       {
    typedef IDTMFilePointFamily type;
    };*/

template <class POINT> struct PointOp
    {
    static double GetX(const POINT& point) {
        return point.x;
        }
    static double GetY(const POINT& point) {
        return point.y;
        }
    static double GetZ(const POINT& point) {
        return point.z;
        }
    static void   SetX(POINT& point, double x) {
        point.x = x;
        }
    static void   SetY(POINT& point, double y) {
        point.y = y;
        }
    static void   SetZ(POINT& point, double z) {
        point.z = z;
        }
    static POINT  Create(double x, double y, double z) {
        return POINT(x, y, z);
        }
    static bool   AreEqual(const POINT& point1, const POINT& point2) {
        return point1 == point2;
        }
    };


template<> bool    PointOp < DPoint3d >::AreEqual(const DPoint3d& point1, const DPoint3d& point2)
    {
    return point1.DistanceSquared(point2) < 1e-8;
    };

template<> DPoint3d   PointOp < DPoint3d >::Create(double x, double y, double z)
    {
    return DPoint3d::From(x, y, z);
    }

    template<class EXTENT> class ExtentOp
        {
        };

template <> class ExtentOp <DRange3d>
{
public:
    static double GetXMin(const DRange3d& extent) { return extent.low.x; }
    static double GetXMax(const DRange3d& extent) { return extent.high.x; }
    static double GetYMin(const DRange3d& extent) { return extent.low.y; }
    static double GetYMax(const DRange3d& extent) { return extent.high.y; }
    static double GetZMin(const DRange3d& extent) { return extent.low.z; }
    static double GetZMax(const DRange3d& extent) { return extent.high.z; }
    static void   SetXMin(DRange3d& extent, double xMin) { extent.low.x = xMin; }
    static void   SetXMax(DRange3d& extent, double xMax) { extent.high.x = xMax; }
    static void   SetYMin(DRange3d& extent, double yMin) { extent.low.y = yMin; }
    static void   SetYMax(DRange3d& extent, double yMax) { extent.high.y = yMax; }
    static void   SetZMin(DRange3d& extent, double zMin) { extent.low.z = zMin; }
    static void   SetZMax(DRange3d& extent, double zMax) { extent.high.z = zMax; }
    static double GetWidth(const DRange3d& extent) { return extent.XLength(); }
    static double GetHeight(const DRange3d& extent) { return extent.YLength(); }
    static double GetThickness(const DRange3d& extent) { return extent.ZLength(); }
    static bool   Contains2d(const DRange3d& extent1, const DRange3d& extent2) { return extent2.IsStrictlyContainedXY(extent1); }
    static bool   Overlap(const DRange3d& extent1, const DRange3d& extent2) { return extent1.IntersectsWith(extent2); }
    static bool   InnerOverlap(const DRange3d& extent1, const DRange3d& extent2) {
        return extent1.high.x > extent2.low.x &&
            extent1.low.x < extent2.high.x &&
            extent1.high.y > extent2.low.y &&
            extent1.low.y < extent2.high.y &&
            extent1.high.z > extent2.low.z &&
            extent1.low.z < extent2.high.z;
        }
    static bool   OutterOverlap(const DRange3d& extent1, const DRange3d& extent2)
        {
        return extent1.high.x >= extent2.low.x &&
            extent1.low.x <= extent2.high.x &&
            extent1.high.y >= extent2.low.y &&
            extent1.low.y <= extent2.high.y &&
            extent1.high.z >= extent2.low.z &&
            extent1.low.z <= extent2.high.z;
        }
    __forceinline static DRange3d Create(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax)
    {
    return DRange3d::From(xMin, yMin, zMin, xMax, yMax, zMax);
        }
    static DRange3d Create(double xMin, double yMin, double xMax, double yMax)
        {
        return DRange3d::From(xMin, yMin, 0.0, xMax, yMax, 0.0);
        }

    __forceinline static DRange3d MergeExtents(const DRange3d& extent1, const DRange3d& extent2)
        {
        DRange3d ext = extent1;
        ext.Extend(extent2);
        return ext;
        }

    };

    template <class EXTENT, class POINT> class ExtentPointOp
        {};


template <class POINT> class ExtentPointOp<DRange3d, POINT>
    {
public:


    static bool IsPointOutterIn3D(const DRange3d& extent, const POINT& point)
        {
        return (extent.low.x <= PointOp<POINT>::GetX(point) && extent.high.x >= PointOp<POINT>::GetX(point)) &&
               (extent.low.y <= PointOp<POINT>::GetY(point) && extent.high.y >= PointOp<POINT>::GetY(point)) &&
               (extent.low.z <= PointOp<POINT>::GetZ(point) && extent.high.z >= PointOp<POINT>::GetZ(point));
        }


    static bool IsPointOutterIn2D(const DRange3d& extent, const POINT& point)
        {
        return extent.IsContainedXY(point);
        }
    };

template<class SPATIAL, class POINT, class EXTENT> class SpatialOp
    {
    };

template <> class SpatialOp <DPoint3d, DPoint3d, DRange3d>
    {
    public:
        static DRange3d GetExtent(const DPoint3d& spatial)
            {
            return DRange3d::From(spatial);
            }
        static bool IsPointIn2D(const DPoint3d& spatial, const DPoint3d& point)
            {
            return spatial.AlmostEqualXY(point);
            }

        static bool IsSpatialInExtent2D(const DPoint3d& spatial, const DRange3d& extent)
            {
            return extent.IsContainedXY(spatial);
            }

        static bool IsSpatialInExtent3D(const DPoint3d& spatial, const DRange3d& extent)
            {
            return extent.IsContained(spatial);
            }


        static size_t GetPointCount(const DPoint3d& spatial)
            {
            return 1;
            }
    };
#if 0
template <> class SpatialOp<HFCPtr<HVEDTMLinearFeature>, HGF3DCoord<double>, HGF3DExtent<double> >
    {
private:
    typedef HGF3DExtent<double>   Extent_t;
    typedef HFCPtr<HVEDTMLinearFeature>                         Spatial_t;
    typedef HGF3DCoord<double>                                  Coord_t;

public:
    static Extent_t GetExtent(const Spatial_t& spatialObject)
        {
        typedef HGF3DExtent<double> SpatialExtent_t;
        const SpatialExtent_t& r3dExtent = spatialObject->GetExtent();

        return ExtentOp<Extent_t>::Create(ExtentOp<SpatialExtent_t>::GetXMin(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetYMin(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetZMin(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetXMax(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetYMax(r3dExtent),
                                          ExtentOp<SpatialExtent_t>::GetZMax(r3dExtent));
        }

    static bool IsPointIn2D(const Spatial_t& spatialObject, const Coord_t& coord)
        {
        return spatialObject->IsPointOn2D(coord);
        }

    static bool IsSpatialInExtent2D (const Spatial_t& spatialObject, const Extent_t& extent)
        {
        const Extent_t& spatialExtent = SpatialOp<Spatial_t, Coord_t, Extent_t>::GetExtent(spatialObject);

        return ((ExtentOp<Extent_t>::GetXMin(spatialExtent) >= ExtentOp<Extent_t>::GetXMin(extent)) &&
                (ExtentOp<Extent_t>::GetYMin(spatialExtent) >= ExtentOp<Extent_t>::GetYMin(extent)) &&
                (ExtentOp<Extent_t>::GetXMax(spatialExtent) <= ExtentOp<Extent_t>::GetXMax(extent)) &&
                (ExtentOp<Extent_t>::GetYMax(spatialExtent) <= ExtentOp<Extent_t>::GetYMax(extent)));

        }

     static bool IsSpatialInExtent3D (const Spatial_t& spatialObject, const Extent_t& extent)
        {
        const Extent_t& spatialExtent = SpatialOp<Spatial_t, Coord_t, Extent_t>::GetExtent(spatialObject);

        return ((ExtentOp<Extent_t>::GetXMin(spatialExtent) >= ExtentOp<Extent_t>::GetXMin(extent)) &&
                (ExtentOp<Extent_t>::GetYMin(spatialExtent) >= ExtentOp<Extent_t>::GetYMin(extent)) &&
                (ExtentOp<Extent_t>::GetZMin(spatialExtent) >= ExtentOp<Extent_t>::GetZMin(extent)) &&
                (ExtentOp<Extent_t>::GetXMax(spatialExtent) <= ExtentOp<Extent_t>::GetXMax(extent)) &&
                (ExtentOp<Extent_t>::GetYMax(spatialExtent) <= ExtentOp<Extent_t>::GetYMax(extent)) &&
                (ExtentOp<Extent_t>::GetZMax(spatialExtent) <= ExtentOp<Extent_t>::GetZMax(extent)));      
        }

    static size_t GetPointCount (const Spatial_t& spatial)
        {
        return spatial->GetSize();
        }


    };

#endif

#if 0
template <> class ExtentOp <IDTMFile::Extent3d64f>
{
public:
    static double GetXMin(const IDTMFile::Extent3d64f& extent) {return extent.xMin;}
    static double GetXMax(const IDTMFile::Extent3d64f& extent) {return extent.xMax;}
    static double GetYMin(const IDTMFile::Extent3d64f& extent) {return extent.yMin;}
    static double GetYMax(const IDTMFile::Extent3d64f& extent) {return extent.yMax;}
    static double GetZMin(const IDTMFile::Extent3d64f& extent) {return extent.zMin;}
    static double GetZMax(const IDTMFile::Extent3d64f& extent) {return extent.zMax;}
    static void   SetXMin(IDTMFile::Extent3d64f& extent, double xMin) {extent.xMin = xMin;}
    static void   SetXMax(IDTMFile::Extent3d64f& extent, double xMax) {extent.xMax = xMax;}
    static void   SetYMin(IDTMFile::Extent3d64f& extent, double yMin) {extent.yMin = yMin;}
    static void   SetYMax(IDTMFile::Extent3d64f& extent, double yMax) {extent.yMax = yMax;}
    static void   SetZMin(IDTMFile::Extent3d64f& extent, double zMin) {extent.zMin = zMin;}
    static void   SetZMax(IDTMFile::Extent3d64f& extent, double zMax) {extent.zMax = zMax;}
    static double GetWidth(const IDTMFile::Extent3d64f& extent) {return (extent.xMax - extent.xMin);}
    static double GetHeight(const IDTMFile::Extent3d64f& extent) {return (extent.yMax - extent.yMin);}
    static double GetThickness(const IDTMFile::Extent3d64f& extent) {return extent.zMax - extent.zMin;}
    static bool   Contains2d (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return (extent1.xMax >= extent2.xMax) && (extent1.xMin <= extent2.xMin) && (extent1.yMax >= extent2.yMax) && (extent1.yMin <= extent2.yMin);}                
    static bool   Overlap (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent1.xMin, extent1.yMin, extent1.xMax, extent1.yMax).Overlaps (HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent2.xMin, extent2.yMin, extent2.xMax, extent2.yMax));}
    static bool   InnerOverlap (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent1.xMin, extent1.yMin, extent1.xMax, extent1.yMax).InnerOverlaps (HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent2.xMin, extent2.yMin, extent2.xMax, extent2.yMax));}
    static bool   OutterOverlap (const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2) {return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent1.xMin, extent1.yMin, extent1.xMax, extent1.yMax).OutterOverlaps (HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent2.xMin, extent2.yMin, extent2.xMax, extent2.yMax));}
    __forceinline static IDTMFile::Extent3d64f Create(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax) 
    {
        IDTMFile::Extent3d64f dummy;
        dummy.xMin = xMin;
        dummy.yMin = yMin;
        dummy.zMin = zMin;
        dummy.xMax = xMax;
        dummy.yMax = yMax;
        dummy.zMax = zMax;
        return dummy;
        }
    static IDTMFile::Extent3d64f Create(double xMin, double yMin, double xMax, double yMax)
        {
        IDTMFile::Extent3d64f dummy;
        dummy.xMin = xMin;
        dummy.yMin = yMin;
        dummy.zMin = 0.0;
        dummy.xMax = xMax;
        dummy.yMax = yMax;
        dummy.zMax = 0.0;
        return dummy;
        }

    __forceinline static IDTMFile::Extent3d64f MergeExtents(const IDTMFile::Extent3d64f& extent1, const IDTMFile::Extent3d64f& extent2)
        {
        return Create(min(GetXMin(extent1), GetXMin(extent2)),
                      min(GetYMin(extent1), GetYMin(extent2)),
                      min(GetZMin(extent1), GetZMin(extent2)),
                      max(GetXMax(extent1), GetXMax(extent2)),
                      max(GetYMax(extent1), GetYMax(extent2)),
                      max(GetZMax(extent1), GetZMax(extent2)));
        }

    };



template <class POINT> class ExtentPointOp<IDTMFile::Extent3d64f, POINT>
    {
public:


    static bool IsPointOutterIn3D(const IDTMFile::Extent3d64f& extent, const POINT& point)
        {
        return (extent.xMin <= PointOp<POINT>::GetX(point) && extent.xMax >= PointOp<POINT>::GetX(point)) &&
               (extent.yMin <= PointOp<POINT>::GetY(point) && extent.yMax >= PointOp<POINT>::GetY(point)) &&
               (extent.zMin <= PointOp<POINT>::GetZ(point) && extent.zMax >= PointOp<POINT>::GetZ(point));
        }


    static bool IsPointOutterIn2D(const IDTMFile::Extent3d64f& extent, const POINT& point)
        {
        return HGF2DTemplateExtent<double, HGF3DCoord<double> >(extent.xMin,
                                                                extent.yMin,
                                                                extent.xMax,
                                                                extent.yMax).IsPointOutterIn (HGF3DCoord<double>(PointOp<POINT>::GetX(point),
                                                                        PointOp<POINT>::GetY(point),
                                                                        PointOp<POINT>::GetZ(point)));
        }
    };


template <class SPATIAL, class POINT> class SpatialOp<SPATIAL, POINT, IDTMFile::Extent3d64f, PointSpatialFamily>
    {
public:
    static  IDTMFile::Extent3d64f GetExtent(const SPATIAL& spatial)
        {
        IDTMFile::Extent3d64f toto;
        toto.xMin = PointOp<SPATIAL>::GetX(spatial);
        toto.xMax = PointOp<SPATIAL>::GetX(spatial);
        toto.yMin = PointOp<SPATIAL>::GetY(spatial);
        toto.yMax = PointOp<SPATIAL>::GetY(spatial);
        toto.zMin = PointOp<SPATIAL>::GetZ(spatial);
        toto.zMax = PointOp<SPATIAL>::GetZ(spatial);

        return  toto;
        }

    static bool IsPointIn2D(const SPATIAL& spatial, const POINT& pi_rCoord)
        {
        return HGF3DCoord<double>(PointOp<SPATIAL>::GetX(spatial),
                                  PointOp<SPATIAL>::GetY(spatial),
                                  PointOp<SPATIAL>::GetZ(spatial)).IsEqualTo2D(HGF3DCoord<double>(PointOp<POINT>::GetX(pi_rCoord),
                                                                               PointOp<POINT>::GetY(pi_rCoord),
                                                                               PointOp<POINT>::GetZ(pi_rCoord)));
        }

    __forceinline static bool IsSpatialInExtent2D (const SPATIAL& spatial, const  IDTMFile::Extent3d64f& extent)
        {
        return ((PointOp<SPATIAL>::GetX(spatial) >= extent.xMin) &&
                (PointOp<SPATIAL>::GetX(spatial) <= extent.xMax) &&
                (PointOp<SPATIAL>::GetY(spatial) >= extent.yMin) &&
                (PointOp<SPATIAL>::GetY(spatial) <= extent.yMax));
        }

    __forceinline static bool IsSpatialInExtent3D (const SPATIAL& spatial, const  IDTMFile::Extent3d64f& extent)
        {
        return ((PointOp<SPATIAL>::GetX(spatial) >= extent.xMin) &&
                (PointOp<SPATIAL>::GetX(spatial) <= extent.xMax) &&
                (PointOp<SPATIAL>::GetY(spatial) >= extent.yMin) &&
                (PointOp<SPATIAL>::GetY(spatial) <= extent.yMax) &&
                (PointOp<SPATIAL>::GetZ(spatial) >= extent.zMin) &&
                (PointOp<SPATIAL>::GetZ(spatial) <= extent.zMax));
        }

    static size_t GetPointCount (const SPATIAL& spatial)
        {
        return 1;
        }

    };
#endif

    /** -----------------------------------------------------------------------------

    The index node header is the base header for any spatial index nodes control. Since the
    spatial index node receives the actual node header type in parameter, the actual
    node header can be another however ALL members using the same name and type must be defined
    and be public for the provided node header. It is usually a lot easier to
    inherit any actual node header from the provided HGFIndexNodeHeader.
    -----------------------------------------------------------------------------
    */
    template <typename EXTENT> class SMIndexNodeHeader
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
            uint64_t      m_totalCount;                // This value indicates the total number of points in node all recursively all sub-nodes.
            bool        m_arePoints3d;               //Indicates if the node contains 3D points or 2.5D points only. 
            bool        m_isTextured;               // Indicates if the node contains Texture or not


            //INFORMATION NOT PERSISTED
            struct RLC3dPoints
                {
                RLC3dPoints(size_t startIndex, size_t length)
                    {
                    m_startIndex = startIndex;
                    m_length = length;
                    }

                size_t m_startIndex;
                size_t m_length;
                };

            vector<RLC3dPoints> m_3dPointsDescBins;   //Determine if a group of points is 3D or not. 
        };


    /** -----------------------------------------------------------------------------

    The index header is the base header for any spatial index control. Since the
    spatial index receives the actual index header type in parameter, the actual
    index header can be another however ALL members using the same name and type must be defined
    and be public for the provided header type. It is usually a lot easier to
    inherit any actual index header from the provided HGFSpatialIndexHeader.
    -----------------------------------------------------------------------------
    */
    template <class EXTENT> class SMSpatialIndexHeader
        {
        public:

            size_t                  m_SplitTreshold;                // Holds the split treshold
            EXTENT                  m_MaxExtent;                    // Indicates the maximum extent if the spatial index is extent limited
            bool                    m_HasMaxExtent;                 // indicated if the index is extent limited.

            bool                    m_balanced;                     // Control variable that indicates if the tree must be balanced
            bool                    m_textured;
            bool                    m_singleFile;
            size_t                  m_numberOfSubNodesOnSplit;      // Control value that hold either 4 or 8 to indicate if a quadtree or octtree is used.
            size_t                  m_depth;                        // Cached (maximum) number of levels in the tree.
            size_t                  m_terrainDepth;                 //Maximum number of LODs for terrain(mesh) data, set at generation time
            bool                    m_isTerrain;
        };

#define MAX_NEIGHBORNODES_COUNT 26

template <typename EXTENT> class SMPointNodeHeader : public SMIndexNodeHeader<EXTENT>
    {
public:
   
    HPMBlockID  m_parentNodeID; //Required when loading 
    vector<HPMBlockID>  m_apSubNodeID;
    HPMBlockID  m_SubNodeNoSplitID;
    bool        m_filtered;    
    
    //NEEDS_WORK_SM - m_meshed ?
    size_t      m_nbFaceIndexes;
    size_t        m_nbUvIndexes;
    size_t        m_nbTextures;
    HPMBlockID  m_graphID;
    std::vector<HPMBlockID>  m_textureID;
    HPMBlockID  m_uvID;

    //NEEDS_WORK_SM - should not be a vector.
    std::vector<HPMBlockID>  m_ptsIndiceID;
    std::vector<HPMBlockID>  m_uvsIndicesID;
    size_t      m_numberOfMeshComponents;
    int*        m_meshComponents = nullptr;
    size_t m_nodeCount;


    std::vector<HPMBlockID> m_apNeighborNodeID[MAX_NEIGHBORNODES_COUNT];    
    bool               m_apAreNeighborNodesStitched[MAX_NEIGHBORNODES_COUNT];

    std::vector<HPMBlockID> m_clipSetsID;

    ~SMPointNodeHeader()
         {
           if (nullptr != m_meshComponents) delete[] m_meshComponents;
         }

    SMPointNodeHeader<EXTENT>& operator=(const SQLiteNodeHeader& nodeHeader)
        {
        m_arePoints3d = nodeHeader.m_arePoints3d;
        m_isTextured = nodeHeader.m_isTextured;
        m_contentExtentDefined = nodeHeader.m_contentExtentDefined;
        m_contentExtent = ExtentOp<EXTENT>::Create(nodeHeader.m_contentExtent.low.x, nodeHeader.m_contentExtent.low.y, nodeHeader.m_contentExtent.low.z,
                                                   nodeHeader.m_contentExtent.high.x, nodeHeader.m_contentExtent.high.y, nodeHeader.m_contentExtent.high.z);
        m_nodeExtent = ExtentOp<EXTENT>::Create(nodeHeader.m_nodeExtent.low.x, nodeHeader.m_nodeExtent.low.y, nodeHeader.m_nodeExtent.low.z,
                                                nodeHeader.m_nodeExtent.high.x, nodeHeader.m_nodeExtent.high.y, nodeHeader.m_nodeExtent.high.z);
        if (nodeHeader.m_graphID != SQLiteNodeHeader::NO_NODEID) m_graphID = HPMBlockID(nodeHeader.m_graphID);
        m_filtered = nodeHeader.m_filtered;
        m_level = nodeHeader.m_level;
        m_nbFaceIndexes = nodeHeader.m_nbFaceIndexes;
        m_nbTextures = nodeHeader.m_nbTextures;
        m_nbUvIndexes = nodeHeader.m_nbUvIndexes;
        m_numberOfMeshComponents = nodeHeader.m_numberOfMeshComponents;
        m_meshComponents = nodeHeader.m_meshComponents;
        m_numberOfSubNodesOnSplit = nodeHeader.m_numberOfSubNodesOnSplit;
        if (nodeHeader.m_parentNodeID != SQLiteNodeHeader::NO_NODEID) m_parentNodeID = HPMBlockID(nodeHeader.m_parentNodeID);
        else m_parentNodeID = IDTMFile::GetNullNodeID();
        if (nodeHeader.m_SubNodeNoSplitID != SQLiteNodeHeader::NO_NODEID) m_SubNodeNoSplitID = HPMBlockID(nodeHeader.m_SubNodeNoSplitID);
        if (nodeHeader.m_uvID != SQLiteNodeHeader::NO_NODEID) m_uvID = HPMBlockID(nodeHeader.m_uvID);
        m_totalCountDefined = nodeHeader.m_totalCountDefined;
        m_totalCount = nodeHeader.m_totalCount;
        m_nodeCount = nodeHeader.m_nodeCount;
        m_SplitTreshold = nodeHeader.m_SplitTreshold;
        m_clipSetsID.resize(nodeHeader.m_clipSetsID.size());
        for (auto& id : m_clipSetsID) if (nodeHeader.m_clipSetsID[&id - &m_clipSetsID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_clipSetsID[&id - &m_clipSetsID.front()]);
        m_textureID.resize(nodeHeader.m_textureID.size());
        for (auto& id : m_textureID) if (nodeHeader.m_textureID[&id - &m_textureID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_textureID[&id - &m_textureID.front()]);
        m_ptsIndiceID.resize(nodeHeader.m_ptsIndiceID.size());
        for (auto& id : m_ptsIndiceID) if (nodeHeader.m_ptsIndiceID[&id - &m_ptsIndiceID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_ptsIndiceID[&id - &m_ptsIndiceID.front()]);
        m_uvsIndicesID.resize(nodeHeader.m_uvsIndicesID.size());
        for (auto& id : m_uvsIndicesID) if (nodeHeader.m_uvsIndicesID[&id - &m_uvsIndicesID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_uvsIndicesID[&id - &m_uvsIndicesID.front()]);
        m_apSubNodeID.resize(nodeHeader.m_apSubNodeID.size());
        for (auto& id : m_apSubNodeID) if (nodeHeader.m_apSubNodeID[&id - &m_apSubNodeID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_apSubNodeID[&id - &m_apSubNodeID.front()]);
        for (size_t i = 0; i < 26; ++i)
            {
            for (auto& id : nodeHeader.m_apNeighborNodeID[i])
                if (id != SQLiteNodeHeader::NO_NODEID)
                    m_apNeighborNodeID[i].push_back(HPMBlockID(id));
            }

        return *this;
        }

    operator SQLiteNodeHeader()
        {
        SQLiteNodeHeader header;
        header.m_arePoints3d = m_arePoints3d;
        header.m_isTextured = m_isTextured;
        header.m_contentExtentDefined = m_contentExtentDefined;
        header.m_contentExtent = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_contentExtent), ExtentOp<EXTENT>::GetYMin(m_contentExtent), ExtentOp<EXTENT>::GetZMin(m_contentExtent),
                                                ExtentOp<EXTENT>::GetXMax(m_contentExtent), ExtentOp<EXTENT>::GetYMax(m_contentExtent), ExtentOp<EXTENT>::GetZMax(m_contentExtent));
        header.m_nodeExtent = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeExtent),
                                             ExtentOp<EXTENT>::GetXMax(m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeExtent));
        header.m_graphID = m_graphID.IsValid() ? m_graphID.m_integerID : -1;
        header.m_filtered = m_filtered;
        header.m_level = m_level;
        header.m_nbFaceIndexes = m_nbFaceIndexes;
        header.m_nbTextures = m_nbTextures;
        header.m_nbUvIndexes = m_nbUvIndexes;
        header.m_numberOfMeshComponents = m_numberOfMeshComponents;
        header.m_meshComponents = m_meshComponents;
        header.m_numberOfSubNodesOnSplit = m_numberOfSubNodesOnSplit;
        header.m_parentNodeID = m_parentNodeID.IsValid() && m_parentNodeID != IDTMFile::GetNullNodeID() ? m_parentNodeID.m_integerID : -1;
        header.m_SubNodeNoSplitID = m_SubNodeNoSplitID.IsValid() && m_SubNodeNoSplitID != IDTMFile::GetNullNodeID() ? m_SubNodeNoSplitID.m_integerID : -1;
        header.m_uvID = m_uvID.IsValid() ? m_uvID.m_integerID : -1;
        header.m_totalCountDefined = m_totalCountDefined;
        header.m_totalCount = m_totalCount;
        header.m_SplitTreshold = m_SplitTreshold;
        header.m_clipSetsID.resize(m_clipSetsID.size());
        header.m_nodeCount = m_nodeCount;
        for (auto& id : m_clipSetsID) header.m_clipSetsID[&id - &m_clipSetsID.front()] = id.IsValid() && id != IDTMFile::GetNullNodeID() ? id.m_integerID : -1;
        header.m_textureID.resize(m_textureID.size());
        for (auto& id : m_textureID) header.m_textureID[&id - &m_textureID.front()] = id.IsValid() && id != IDTMFile::GetNullNodeID() ? id.m_integerID : -1;
        header.m_ptsIndiceID.resize(m_ptsIndiceID.size());
        for (auto& id : m_ptsIndiceID) header.m_ptsIndiceID[&id - &m_ptsIndiceID.front()] = id.IsValid() && id != IDTMFile::GetNullNodeID() ? id.m_integerID : -1;
        header.m_uvsIndicesID.resize(m_uvsIndicesID.size());
        for (auto& id : m_uvsIndicesID) header.m_uvsIndicesID[&id - &m_uvsIndicesID.front()] = id.IsValid() && id != IDTMFile::GetNullNodeID() ? id.m_integerID : -1;
        header.m_apSubNodeID.resize(m_apSubNodeID.size());
        for (auto& id : m_apSubNodeID) header.m_apSubNodeID[&id - &m_apSubNodeID.front()] = id.IsValid() && id != IDTMFile::GetNullNodeID() ? id.m_integerID : -1;
        if (header.m_SubNodeNoSplitID != -1) header.m_apSubNodeID[0] = header.m_SubNodeNoSplitID;
        for (size_t i = 0; i < 26; ++i)
            {
            header.m_apNeighborNodeID[i].resize(m_apNeighborNodeID[i].size());
            for (auto& id : m_apNeighborNodeID[i]) header.m_apNeighborNodeID[i][&id - &m_apNeighborNodeID[i].front()] = id.IsValid() && id != IDTMFile::GetNullNodeID() ? id.m_integerID : -1;
            }
        return header;
        }
    };


template <class EXTENT> class SMPointIndexHeader: public SMSpatialIndexHeader<EXTENT>
    {
public:

    HPMBlockID              m_rootNodeBlockID;

    SMPointIndexHeader<EXTENT>& operator=(const SQLiteIndexHeader& indexHeader)
        {
        if (indexHeader.m_rootNodeBlockID != SQLiteNodeHeader::NO_NODEID) m_rootNodeBlockID = HPMBlockID(indexHeader.m_rootNodeBlockID);
        m_balanced = indexHeader.m_balanced;
        m_depth = indexHeader.m_depth;
        m_terrainDepth = indexHeader.m_terrainDepth;
        m_HasMaxExtent = indexHeader.m_HasMaxExtent;
        m_MaxExtent = ExtentOp<EXTENT>::Create(indexHeader.m_MaxExtent.low.x, indexHeader.m_MaxExtent.low.y, indexHeader.m_MaxExtent.low.z,
                                               indexHeader.m_MaxExtent.high.x, indexHeader.m_MaxExtent.high.y, indexHeader.m_MaxExtent.high.z);
        m_numberOfSubNodesOnSplit = indexHeader.m_numberOfSubNodesOnSplit;
        m_singleFile = indexHeader.m_singleFile;
        m_SplitTreshold = indexHeader.m_SplitTreshold;
        m_textured = indexHeader.m_textured;
        m_isTerrain = indexHeader.m_isTerrain;
        return *this;
        }


    operator SQLiteIndexHeader()
        {
        SQLiteIndexHeader header;
        header.m_rootNodeBlockID = m_rootNodeBlockID.IsValid() ? m_rootNodeBlockID.m_integerID : -1;
        header.m_balanced = m_balanced;
        header.m_depth = m_depth;
        header.m_terrainDepth = m_terrainDepth;
        header.m_HasMaxExtent = m_HasMaxExtent;
        header.m_MaxExtent = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_MaxExtent), ExtentOp<EXTENT>::GetYMin(m_MaxExtent), ExtentOp<EXTENT>::GetZMin(m_MaxExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_MaxExtent), ExtentOp<EXTENT>::GetYMax(m_MaxExtent), ExtentOp<EXTENT>::GetZMax(m_MaxExtent));
        header.m_numberOfSubNodesOnSplit = m_numberOfSubNodesOnSplit;
        header.m_singleFile = m_singleFile;
        header.m_SplitTreshold = m_SplitTreshold;
        header.m_textured = m_textured;
        header.m_isTerrain = m_isTerrain;
        return header;
        }
    };



template <class POINT, class EXTENT> class SMPointTileStore: public IScalableMeshDataStore<POINT, SMPointIndexHeader<EXTENT>, SMPointNodeHeader<EXTENT>>
    {
public:
    SMPointTileStore() {};
    virtual ~SMPointTileStore() {};

    virtual uint64_t GetNextID() const
        {
        //assert(false); // Not implemented!
        return -1;
        }
    };

template <typename POINT, typename EXTENT> class SMPointTaggedTileStore : public SMPointTileStore<POINT, EXTENT>// , public HFCShareableObject<SMPointTileStore<POINT, EXTENT> >
    {
protected:
   // typedef IDTMFile::PointTileHandler<POINT> TileHandler;
   //     typedef IDTMFile::BTreeIndexHandler IndexHandler;

  //  typedef typename TileHandler::PointArray PointArray;

 /*   static IDTMFile::NodeID ConvertBlockID (const HPMBlockID& blockID)
        {
        return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
        }

    static IDTMFile::SubNodesTable::value_type ConvertChildID (const HPMBlockID& childID)
        {
        return static_cast<IDTMFile::SubNodesTable::value_type>(childID.m_integerID);
        }

    static IDTMFile::NeighborNodesTable::value_type ConvertNeighborID (const HPMBlockID& neighborID)
        {
        return static_cast<IDTMFile::NeighborNodesTable::value_type>(neighborID.m_integerID);
        }*/

public:
    // Constructor / Destroyer
    
   /* SMPointTaggedTileStore(IDTMFile::File::Ptr openedDTMFile, bool compress, size_t layerID = 0)
            :   m_filteringDir(0),
                m_layerID(layerID),
            m_compress(compress),            
            m_DTMFile(openedDTMFile)
        {
        //if (m_DTMFile == NULL)
        //    throw;
        }

    virtual ~SMPointTaggedTileStore ()
        {        
        m_DTMFile = NULL;
        }*/

    // New function
    /*virtual bool HasSpatialReferenceSystem()
        {
        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());
                    
        IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);
            
        if (NULL == layerDir)
            return false;

        return layerDir->HasWkt();
        }


    // New function
    virtual std::string GetSpatialReferenceSystem()
        {
        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);
        if (NULL == layerDir)
            return false;

        size_t  destinationBuffSize = WString(layerDir->GetWkt().GetCStr()).GetMaxLocaleCharBytes();
        char*  valueMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(valueMBS, layerDir->GetWkt().GetCStr(),destinationBuffSize);


        return string(valueMBS);
        }*/

    // ITileStore interface
    /*virtual void Close ()
        {        
        m_DTMFile = NULL;
        }*/

    virtual bool StoreMasterHeader (SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {
      /*  if (m_DTMFile == NULL)
            return false;

        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (NULL == m_tileHandler)
            {
            IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);

            if (NULL == layerDir)
                {
                layerDir = m_DTMFile->GetRootDir()->AddLayerDir ();
                HASSERT(NULL != layerDir);
                if (NULL == layerDir)
                    return false;

                m_layerID = layerDir->GetIndex();
                //HASSERT(0 == m_layerID);
                }

            IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(MASS_POINT_FEATURE_TYPE);
            if (NULL == featureDir)
                {
                //HASSERT(0 == layerDir->CountPointOnlyUniformFeatureDirs());

                // No Point dir ... we create one
                featureDir = layerDir->CreatePointsOnlyUniformFeatureDir(MASS_POINT_FEATURE_TYPE,
                                                                         IDTMFile::PointTypeIDTrait<POINT>::value,
                                                                         (m_compress) ? HTGFF::Compression::Deflate::Create() :
                                                                         HTGFF::Compression::None::Create());

                HASSERT(NULL != featureDir);
                if (NULL == featureDir)
                    return false;
                }


            m_tileHandler = TileHandler::CreateFrom(featureDir->GetPointDir());

            m_indexHandler = IndexHandler::CreateFrom(featureDir->GetSpatialIndexDir());
            if (NULL == m_indexHandler)
                {
                m_indexHandler = IndexHandler::CreateFrom(featureDir->CreateSpatialIndexDir(IndexHandler::Options(true, // TDORAY: Set correct value for is progressive
                    indexHeader!= NULL ? indexHeader->m_SplitTreshold : 0)));
                HASSERT(NULL != m_indexHandler);
                if (NULL == m_indexHandler)
                    return false;
                }

            m_filteringDir = featureDir->GetFilteringDir();
            if (NULL == m_filteringDir)
                {
                // TDORAY: Match real filter type. Ask alain where to find this information.
                m_filteringDir = featureDir->CreateFilteringDir(IDTMFile::DumbFilteringHandler::Options());
                HASSERT(NULL != m_filteringDir);
                if (NULL == m_filteringDir)
                    return false;
                }
            }
        HASSERT(NULL != m_tileHandler);
        HASSERT(NULL != m_indexHandler);
        HASSERT(NULL != m_filteringDir);
        if (indexHeader != NULL)
            {
            // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
            m_indexHandler->SetBalanced(indexHeader->m_balanced);
            m_indexHandler->SetDepth((uint32_t)indexHeader->m_depth);

            m_indexHandler->SetSingleFile(indexHeader->m_singleFile);

            if (indexHeader->m_rootNodeBlockID.m_integerInitialized)
                m_indexHandler->SetTopNode(ConvertBlockID(indexHeader->m_rootNodeBlockID));
            else
                m_indexHandler->SetTopNode(IDTMFile::GetNullNodeID());
            }
            */
        return true;
        }

    virtual size_t LoadMasterHeader (SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
        {            
        /*if (m_DTMFile == NULL)
            return 0;

        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        if (NULL == m_tileHandler)
            {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir (m_layerID);
            if (NULL == layerDir)
                return 0;

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(MASS_POINT_FEATURE_TYPE);
            if (NULL == featureDir)
                return 0;

            m_tileHandler = TileHandler::CreateFrom(featureDir->GetPointDir());

            m_indexHandler = IndexHandler::CreateFrom(featureDir->GetSpatialIndexDir());
            if (NULL == m_indexHandler)
                return 0;

            m_filteringDir = featureDir->GetFilteringDir();
            if (NULL == m_filteringDir)
                return 0;

            }
        HASSERT(NULL != m_tileHandler);
        HASSERT(NULL != m_indexHandler);
        HASSERT(NULL != m_filteringDir);
        if (indexHeader != NULL)
            {
            // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
            indexHeader->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
            indexHeader->m_balanced = m_indexHandler->IsBalanced();

            indexHeader->m_depth = m_indexHandler->GetDepth();

            indexHeader->m_singleFile = m_indexHandler->IsSingleFile();

            if (m_indexHandler->GetTopNode() != IDTMFile::GetNullNodeID())
                indexHeader->m_rootNodeBlockID = m_indexHandler->GetTopNode();
            else
                indexHeader->m_rootNodeBlockID = HPMBlockID();
            }*/
        return headerSize;
        }

    // New interface
    virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData)
        {
        /* HPRECONDITION(m_tileHandler != NULL);
         //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

         PointArray MyArray(DataTypeArray, countData);

         IDTMFile::NodeID newNodeID;

         std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

         if (!m_tileHandler->AddPoints(newNodeID, MyArray))
         {
         HASSERT(!"Write failed!");
         //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
         throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
         }

         return HPMBlockID(newNodeID);*/return 0;
        }

    virtual HPMBlockID StoreBlock (POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
      /*  HPRECONDITION(m_tileHandler != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::SubNodesTable::GetNoSubNodeID())
            return StoreNewBlock (DataTypeArray, countData);

        PointArray arrayOfPoints (DataTypeArray, countData);

        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        if (!m_tileHandler->SetPoints(ConvertBlockID(blockID), arrayOfPoints))
            {
            HASSERT(!"Write failed!");
            //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
            throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
            }*/

        return blockID;
        }

    virtual size_t GetBlockDataCount(HPMBlockID blockID) const
        {
        /*HPRECONDITION(m_tileHandler != NULL);
        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());
        return m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));*/ 
        return 0;

        }


    virtual size_t StoreHeader (SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
      /*  HPRECONDITION(m_tileHandler != NULL);
        HPRECONDITION(m_indexHandler != NULL);
        HPRECONDITION(m_filteringDir != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        if (header->m_contentExtentDefined)
            {

            }

        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        m_tileHandler->GetDir().SetResolution (ConvertBlockID(blockID), (IDTMFile::ResolutionID)header->m_level);
        m_filteringDir->SetFiltered (ConvertBlockID(blockID), header->m_filtered);                

        if (header->m_parentNodeID.IsValid())
            {
            m_indexHandler->EditParentNode(ConvertBlockID(blockID)) = ConvertBlockID(header->m_parentNodeID);
            }
        else
            {
            m_indexHandler->EditParentNode(ConvertBlockID(blockID)) = IDTMFile::GetNullNodeID();
            }

        IDTMFile::SubNodesTable& rChildren = m_indexHandler->EditSubNodes(ConvertBlockID(blockID));
        
        size_t nbChildren = header->m_IsLeaf || (!header->m_IsBranched  && !header->m_SubNodeNoSplitID.IsValid())? 0 : (!header->m_IsBranched ? 1 : header->m_numberOfSubNodesOnSplit);


        rChildren.SetNbVarData((uint32_t)nbChildren);

        IDTMFile::NodeInfo* childNodeInfo = m_indexHandler->EditSubNodesVarData(rChildren, nbChildren);

        assert(childNodeInfo != 0 || rChildren.GetNbVarData() == 0);

        if (rChildren.GetNbVarData() > 0)
            {
            if (nbChildren > 1)
                {
                for (size_t childInd = 0; childInd < nbChildren; childInd++)
                    {
                    childNodeInfo[childInd].m_nodePos = (uint8_t)childInd;
                    childNodeInfo[childInd].m_nodeId = ConvertChildID(header->m_apSubNodeID[childInd]);
                    }
                }
            else
                {
                childNodeInfo[0].m_nodePos = 0;
                childNodeInfo[0].m_nodeId = header->m_SubNodeNoSplitID.IsValid() ? ConvertChildID(header->m_SubNodeNoSplitID) : ConvertChildID(header->m_apSubNodeID[0]);
                }
            }
       
        IDTMFile::NeighborNodesTable& rNeighbors = m_indexHandler->EditNeighborNodes(ConvertBlockID(blockID));        

        size_t nbNeighbors = 0; 

        for (int neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd ++)
            {       
            rNeighbors.m_AreNeighborTilesStitched[neighborPosInd] = header->m_apAreNeighborNodesStitched[neighborPosInd];
            nbNeighbors += header->m_apNeighborNodeID[neighborPosInd].size();            
            }

        rNeighbors.SetNbVarData((uint32_t)nbNeighbors);
        
        IDTMFile::NodeInfo* neighborNodeInfo = m_indexHandler->EditNeighborNodesVarData (rNeighbors);
            
        assert(neighborNodeInfo != 0 || rNeighbors.GetNbVarData() == 0);

        size_t neighborInfoInd = 0; 

        if (rNeighbors.GetNbVarData() > 0)
            {        
            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {       
                for (size_t neighborInd = 0; neighborInd < header->m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
                    {
                    neighborNodeInfo[neighborInfoInd].m_nodePos = (uint8_t)neighborPosInd;
                    neighborNodeInfo[neighborInfoInd].m_nodeId = ConvertNeighborID(header->m_apNeighborNodeID[neighborPosInd][neighborInd]);                                     
                    neighborInfoInd++;
                    }                                     
                }
            }        

        IDTMFile::Extent3d64f& rExtent = m_tileHandler->GetDir().EditExtent (ConvertBlockID(blockID));
        rExtent.xMin = ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent);
        rExtent.yMin = ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent);
        rExtent.zMin = ExtentOp<EXTENT>::GetZMin(header->m_nodeExtent);
        rExtent.xMax = ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent);
        rExtent.yMax = ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent);
        rExtent.zMax = ExtentOp<EXTENT>::GetZMax(header->m_nodeExtent);


        if (header->m_contentExtentDefined)
            {
            IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->EditContentExtent (ConvertBlockID(blockID));
            rContentExtent.xMin = ExtentOp<EXTENT>::GetXMin(header->m_contentExtent);
            rContentExtent.yMin = ExtentOp<EXTENT>::GetYMin(header->m_contentExtent);
            rContentExtent.zMin = ExtentOp<EXTENT>::GetZMin(header->m_contentExtent);
            rContentExtent.xMax = ExtentOp<EXTENT>::GetXMax(header->m_contentExtent);
            rContentExtent.yMax = ExtentOp<EXTENT>::GetYMax(header->m_contentExtent);
            rContentExtent.zMax = ExtentOp<EXTENT>::GetZMax(header->m_contentExtent);
            }
        else
            {
            IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->EditContentExtent (ConvertBlockID(blockID));
            rContentExtent.xMin = 0.0;
            rContentExtent.yMin = 0.0;
            rContentExtent.zMin = 0.0;
            rContentExtent.xMax = 0.0;
            rContentExtent.yMax = 0.0;
            rContentExtent.zMax = 0.0;
            }


        m_indexHandler->SetTotalPointCount(ConvertBlockID(blockID), header->m_totalCount);    
        m_indexHandler->SetArePoints3d(ConvertBlockID(blockID), header->m_arePoints3d);  

        //why was this commented?
       // assert(header->m_3dPointsDescBins.size() <= USHORT_MAX);
       // m_indexHandler->SetNb3dPointsBins(ConvertBlockID(blockID), header->m_3dPointsDescBins.size());  
        
        
        
        m_indexHandler->SetMeshIndexesCount(ConvertBlockID(blockID), header->m_nbFaceIndexes);
        if (header->m_graphID.IsValid())
            {
            m_indexHandler->EditGraphBlockID(ConvertBlockID(blockID)) = ConvertBlockID(header->m_graphID);
            }
        else
            {
            m_indexHandler->EditGraphBlockID(ConvertBlockID(blockID)) = IDTMFile::GetNullNodeID();
            }

        IDTMFile::PtsIndicesTable& rPtsIndices = m_indexHandler->EditPtsIndices(ConvertBlockID(blockID));
        rPtsIndices.SetNbVarData((int)header->m_ptsIndiceID.size());
        IDTMFile::NodeID* ptsIndiceInfo = m_indexHandler->EditPtsIndicesVarData(rPtsIndices, header->m_ptsIndiceID.size());
        for (auto& id : header->m_ptsIndiceID)
        //if(header->m_ptsIndiceID.IsValid())
            {
            if (id.IsValid())
                {
            //m_indexHandler->EditPtsIndiceBlockID(ConvertBlockID(blockID)) = ConvertBlockID(header->m_ptsIndiceID);
                    ptsIndiceInfo[&id - &header->m_ptsIndiceID.front()] = ConvertBlockID(id);
                }
            else
                {
            //m_indexHandler->EditPtsIndiceBlockID(ConvertBlockID(blockID)) = IDTMFile::GetNullNodeID();
                    ptsIndiceInfo[&id - &header->m_ptsIndiceID.front()] = IDTMFile::GetNullNodeID();
                }
            }

        IDTMFile::UVsIndicesTable& rUVsIndices = m_indexHandler->EditUVsIndices(ConvertBlockID(blockID));
        rUVsIndices.SetNbVarData((int)header->m_uvsIndicesID.size());
        IDTMFile::NodeID* uvsIndicesInfo = m_indexHandler->EditUVsIndicesVarData(rUVsIndices, header->m_uvsIndicesID.size());
        for (auto& id : header->m_uvsIndicesID)
            //if(header->m_ptsIndiceID.IsValid())
        {
            if (id.IsValid())
            {
                //m_indexHandler->EditPtsIndiceBlockID(ConvertBlockID(blockID)) = ConvertBlockID(header->m_ptsIndiceID);
                uvsIndicesInfo[&id - &header->m_uvsIndicesID.front()] = ConvertBlockID(id);
            }
            else
            {
                //m_indexHandler->EditPtsIndiceBlockID(ConvertBlockID(blockID)) = IDTMFile::GetNullNodeID();
                uvsIndicesInfo[&id - &header->m_uvsIndicesID.front()] = IDTMFile::GetNullNodeID();
            }
        }

        
//        for (auto& id : header->m_uvID)
            {
            if (header->m_uvID.IsValid())
                {
                m_indexHandler->EditUVBlockID(ConvertBlockID(blockID)) = ConvertBlockID(header->m_uvID);
                //uvInfo[&id - &header->m_uvID.front()] = ConvertBlockID(id);
                }
            else
                {
                m_indexHandler->EditUVBlockID(ConvertBlockID(blockID)) = IDTMFile::GetNullNodeID();
                //uvInfo[&id - &header->m_uvID.front()] = IDTMFile::GetNullNodeID();
                }
            }
       
        IDTMFile::TexturesTable& rTextures = m_indexHandler->EditTextures(ConvertBlockID(blockID));
        rTextures.SetNbVarData((int)header->m_textureID.size());
        IDTMFile::NodeID* textureInfo = m_indexHandler->EditTexturesVarData(rTextures, header->m_textureID.size());
        for(auto& id : header->m_textureID)
        {
        if (id.IsValid())
        {
            textureInfo[&id - &header->m_textureID.front()] = ConvertBlockID(id);
        }
        else
        {
            textureInfo[&id - &header->m_textureID.front()] = IDTMFile::GetNullNodeID();
        }
        }

        m_indexHandler->SetMeshComponentsCount(ConvertBlockID(blockID), header->m_numberOfMeshComponents);
        if (header->m_numberOfMeshComponents > 0)
            {
            IDTMFile::MeshComponentsList& list = m_indexHandler->EditMeshComponents(ConvertBlockID(blockID));
            int* allComponents = m_indexHandler->EditMeshComponentsVarData(list, header->m_numberOfMeshComponents);
            memcpy(allComponents, header->m_meshComponents, header->m_numberOfMeshComponents * sizeof(int));
            }
        IDTMFile::ClipSetsList &clipSets = m_indexHandler->EditClipSets(ConvertBlockID(blockID));
        if (header->m_clipSetsID.size() > 0)
            {
            IDTMFile::NodeID* pClipSetInfo = m_indexHandler->EditClipSetsVarData(clipSets, header->m_clipSetsID.size());
            for (size_t i = 0; i < header->m_clipSetsID.size(); ++i) pClipSetInfo[i] = ConvertNeighborID(header->m_clipSetsID[i]);
            }
      //  else
            clipSets.SetNbVarData((uint32_t)header->m_clipSetsID.size());

        return 1;
        }

    virtual size_t LoadHeader (SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
        {
        HPRECONDITION(m_tileHandler != NULL);
        HPRECONDITION(m_indexHandler != NULL);
        HPRECONDITION(m_filteringDir != NULL);

        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        const IDTMFile::Extent3d64f& rExtent = m_tileHandler->GetDir().GetExtent (ConvertBlockID(blockID));
        ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, rExtent.xMin);
        ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, rExtent.yMin);
        ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, rExtent.zMin);
        ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, rExtent.xMax);
        ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, rExtent.yMax);
        ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, rExtent.zMax);

        header->m_level = m_tileHandler->GetDir().GetResolution (ConvertBlockID(blockID));
        header->m_filtered = m_filteringDir->IsFiltered (ConvertBlockID(blockID));
        header->m_parentNodeID = m_indexHandler->GetParentNode(ConvertBlockID(blockID));   

        const IDTMFile::SubNodesTable& rChildren = m_indexHandler->GetSubNodes(ConvertBlockID(blockID));
        header->m_numberOfSubNodesOnSplit = rChildren.GetNbVarData();
        header->m_apSubNodeID.resize(header->m_numberOfSubNodesOnSplit);
        if (rChildren.GetNbVarData() > 0)
            {

            const IDTMFile::NodeInfo* pChildrenInfo(m_indexHandler->GetSubNodesVarData(rChildren));

            for (size_t indexNodes = 0; indexNodes < (size_t)rChildren.GetNbVarData(); indexNodes++)
                {
                header->m_apSubNodeID[pChildrenInfo[indexNodes].m_nodePos]= pChildrenInfo[indexNodes].m_nodeId;
                }
            header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];
            }

        const IDTMFile::NeighborNodesTable& rNeighbors = m_indexHandler->GetNeighborNodes(ConvertBlockID(blockID));

        for (int neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
            {       
            header->m_apAreNeighborNodesStitched[neighborPosInd] = rNeighbors.m_AreNeighborTilesStitched[neighborPosInd];            
            }        

        if (rNeighbors.GetNbVarData() > 0)
            {
            const IDTMFile::NodeInfo* pNeighborNodeInfo(m_indexHandler->GetNeighborNodesVarData (rNeighbors));                    
                       
            for (size_t indexNodes = 0 ; indexNodes < (size_t)rNeighbors.GetNbVarData(); indexNodes++)
                {                
                assert(pNeighborNodeInfo[indexNodes].m_nodePos < MAX_NEIGHBORNODES_COUNT);

                header->m_apNeighborNodeID[pNeighborNodeInfo[indexNodes].m_nodePos].push_back(pNeighborNodeInfo[indexNodes].m_nodeId);                        
                }           
            }
               
        header->m_IsLeaf = (!header->m_apSubNodeID[0].IsValid());
        header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID.size() > 1 && header->m_apSubNodeID[1].IsValid());

        header->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
         header->m_balanced = m_indexHandler->IsBalanced();


        const IDTMFile::Extent3d64f& rContentExtent = m_indexHandler->GetContentExtent (ConvertBlockID(blockID));
        ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, rContentExtent.xMin);
        ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, rContentExtent.yMin);
        ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, rContentExtent.zMin);
        ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, rContentExtent.xMax);
        ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, rContentExtent.yMax);
        ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, rContentExtent.zMax);

        header->m_contentExtentDefined =  (rContentExtent.xMin != 0.0 || rContentExtent.xMax != 0.0 || rContentExtent.yMin != 0.0 ||
                                           rContentExtent.yMax != 0.0 || rContentExtent.zMin != 0.0 || rContentExtent.zMax != 0.0);

        if (header->m_contentExtentDefined)
            {
            }

        header->m_totalCountDefined = true;
        header->m_totalCount = m_indexHandler->GetTotalPointCount(ConvertBlockID(blockID));
        header->m_arePoints3d = m_indexHandler->GetArePoints3d(ConvertBlockID(blockID));        

            

        header->m_nbFaceIndexes = (size_t)m_indexHandler->GetMeshIndexesCount(ConvertBlockID(blockID));
        header->m_graphID = m_indexHandler->GetGraphBlockID(ConvertBlockID(blockID));
        header->m_uvID = m_indexHandler->GetUVBlockID(ConvertBlockID(blockID));


        const IDTMFile::PtsIndicesTable& rPtsIndices = m_indexHandler->GetPtsIndices(ConvertBlockID(blockID));
        header->m_ptsIndiceID.resize(rPtsIndices.GetNbVarData());
        if (rPtsIndices.GetNbVarData() > 0)
            {

            const IDTMFile::NodeID* pPtsIndiceInfo(m_indexHandler->GetPtsIndicesVarData(rPtsIndices));

            for (size_t ptsIndiceID = 0; ptsIndiceID < (size_t)rPtsIndices.GetNbVarData(); ptsIndiceID++)
                {
                header->m_ptsIndiceID[ptsIndiceID] = pPtsIndiceInfo[ptsIndiceID] != IDTMFile::GetNullNodeID() ? pPtsIndiceInfo[ptsIndiceID] : HPMBlockID();
                }
            }
        else
            {
            header->m_ptsIndiceID.resize(1);
            header->m_ptsIndiceID[0] = HPMBlockID();
            }

        const IDTMFile::UVsIndicesTable& rUvsIndices = m_indexHandler->GetUVsIndices(ConvertBlockID(blockID));
        header->m_uvsIndicesID.resize(rUvsIndices.GetNbVarData());
        if (rUvsIndices.GetNbVarData() > 0)
        {

            const IDTMFile::NodeID* pUvsIndicesInfo(m_indexHandler->GetUVsIndicesVarData(rUvsIndices));

            for (size_t uvsIndicesID = 0; uvsIndicesID < (size_t)rUvsIndices.GetNbVarData(); uvsIndicesID++)
            {
                header->m_uvsIndicesID[uvsIndicesID] = pUvsIndicesInfo[uvsIndicesID] != IDTMFile::GetNullNodeID() ? pUvsIndicesInfo[uvsIndicesID] : HPMBlockID();
            }
        }
        else
        {
            header->m_uvsIndicesID.resize(1);
            header->m_uvsIndicesID[0] = HPMBlockID();
        }

        const IDTMFile::TexturesTable& rTextures = m_indexHandler->GetTextures(ConvertBlockID(blockID));
        header->m_textureID.resize(rTextures.GetNbVarData());
        if (rTextures.GetNbVarData() > 0)
            {

            const IDTMFile::NodeID* pTextureInfo(m_indexHandler->GetTexturesVarData(rTextures));

            for (size_t textureID = 0; textureID < (size_t)rTextures.GetNbVarData(); textureID++)
                {
                header->m_textureID[textureID] = pTextureInfo[textureID] != IDTMFile::GetNullNodeID() ? pTextureInfo[textureID] : HPMBlockID();;
                }
            }
        else
            {
            header->m_textureID.resize(1);
            header->m_textureID[0] = HPMBlockID();
            }

        if (ConvertBlockID(header->m_graphID) == IDTMFile::GetNullNodeID()) header->m_graphID = HPMBlockID();
        if (ConvertBlockID(header->m_uvID) == IDTMFile::GetNullNodeID()) header->m_uvID = HPMBlockID();
        header->m_numberOfMeshComponents = (size_t)m_indexHandler->GetMeshComponentsCount(ConvertBlockID(blockID));
                        
        if (header->m_numberOfMeshComponents > 0)
            {
            const IDTMFile::MeshComponentsList& list = m_indexHandler->GetMeshComponents(ConvertBlockID(blockID));
            header->m_meshComponents = new int[header->m_numberOfMeshComponents];

            const int* allComponents = m_indexHandler->GetMeshComponentsVarData(list);
            memcpy(header->m_meshComponents, allComponents, header->m_numberOfMeshComponents * sizeof(int));
            }

        const IDTMFile::ClipSetsList &clipSets = m_indexHandler->GetClipSets(ConvertBlockID(blockID));
        header->m_clipSetsID.resize(clipSets.GetNbVarData());
        if (header->m_clipSetsID.size() > 0)
            {
            const IDTMFile::NodeID* pClipSetInfo(m_indexHandler->GetClipSetsVarData(clipSets));
            for(size_t i =0; i < header->m_clipSetsID.size(); ++i) header->m_clipSetsID[i] = pClipSetInfo[i];
            }*/
        return 1;
        }


    virtual size_t LoadBlock (POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
       /* HPRECONDITION(m_tileHandler != NULL);

        PointArray arrayOfPoints;
        arrayOfPoints.WrapEditable(DataTypeArray, 0, maxCountData);

        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
            throw; //TDORAY: Do something else*/

        return 1;
        }

    virtual bool DestroyBlock (HPMBlockID blockID)
        {
       /* HPRECONDITION(m_tileHandler != NULL);
        //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

        std::lock_guard<std::recursive_mutex> lck (m_DTMFile->GetFileAccessMutex());

        return m_tileHandler->RemovePoints(ConvertBlockID(blockID));*/
        return true;
        }

   // static const IDTMFile::FeatureType MASS_POINT_FEATURE_TYPE = 0;

protected:
   /* const IDTMFile::File::Ptr& GetFileP () const
        {
        return m_DTMFile;
        }

    IDTMFile::File::Ptr m_DTMFile;
    typename TileHandler::Ptr m_tileHandler;
    IndexHandler::Ptr m_indexHandler;
    IDTMFile::FilteringDir* m_filteringDir;
    size_t m_layerID;*/

private:

    bool m_compress;
    };

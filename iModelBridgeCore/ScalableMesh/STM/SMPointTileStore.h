//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMPointTileStore.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



#include <../STM/IScalableMeshDataStore.h>
/*#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/ISMStore.h>
#include <ImagePP/all/h/HGFSpatialIndex.h>
#include <ImagePP/all/h/HVEDTMLinearFeature.h>
#include <ImagePP/all/h/HGFPointTileStore.h>*/
#include <BeJsonCpp/BeJsonUtilities.h>
#include "SMSQLiteFile.h"
USING_NAMESPACE_IMAGEPP

namespace ISMStore
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

    /** -----------------------------------------------------------------------------

    The index node header is the base header for any spatial index nodes control. Since the
    spatial index node receives the actual node header type in parameter, the actual
    node header can be another however ALL members using the same name and type must be defined
    and be public for the provided node header. It is usually a lot easier to
    inherit any actual node header from the provided HGFIndexNodeHeader.
    -----------------------------------------------------------------------------
    */
    template <typename EXTENT> class SMPointNodeHeaderBase
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

template <typename EXTENT> class SMPointNodeHeader : public SMPointNodeHeaderBase<EXTENT>
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
        else m_parentNodeID = ISMStore::GetNullNodeID();
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
        header.m_parentNodeID = m_parentNodeID.IsValid() && m_parentNodeID != ISMStore::GetNullNodeID() ? m_parentNodeID.m_integerID : -1;
        header.m_SubNodeNoSplitID = m_SubNodeNoSplitID.IsValid() && m_SubNodeNoSplitID != ISMStore::GetNullNodeID() ? m_SubNodeNoSplitID.m_integerID : -1;
        header.m_uvID = m_uvID.IsValid() ? m_uvID.m_integerID : -1;
        header.m_totalCountDefined = m_totalCountDefined;
        header.m_totalCount = m_totalCount;
        header.m_SplitTreshold = m_SplitTreshold;
        header.m_clipSetsID.resize(m_clipSetsID.size());
        header.m_nodeCount = m_nodeCount;
        for (auto& id : m_clipSetsID) header.m_clipSetsID[&id - &m_clipSetsID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
        header.m_textureID.resize(m_textureID.size());
        for (auto& id : m_textureID) header.m_textureID[&id - &m_textureID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
        header.m_ptsIndiceID.resize(m_ptsIndiceID.size());
        for (auto& id : m_ptsIndiceID) header.m_ptsIndiceID[&id - &m_ptsIndiceID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
        header.m_uvsIndicesID.resize(m_uvsIndicesID.size());
        for (auto& id : m_uvsIndicesID) header.m_uvsIndicesID[&id - &m_uvsIndicesID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
        header.m_apSubNodeID.resize(m_apSubNodeID.size());
        for (auto& id : m_apSubNodeID) header.m_apSubNodeID[&id - &m_apSubNodeID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
        if (header.m_SubNodeNoSplitID != -1) header.m_apSubNodeID[0] = header.m_SubNodeNoSplitID;
        for (size_t i = 0; i < 26; ++i)
            {
            header.m_apNeighborNodeID[i].resize(m_apNeighborNodeID[i].size());
            for (auto& id : m_apNeighborNodeID[i]) header.m_apNeighborNodeID[i][&id - &m_apNeighborNodeID[i].front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
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


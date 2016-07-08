#pragma once

#include "ISMDataStore.h"

////MST_TS
#include "..\SMSQLiteFile.h"



////MST_TS
#include <ImagePP\h\ImageppAPI.h>
#include <ImagePP\h\ImagePPClassId.h>
#include <ImagePP\all\h\HCDPacket.h>
#include <ImagePP\all\h\HCDCodecZlib.h>
#include <ImagePP\all\h\HFCAccessMode.h>

 bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
        HCDPacket& pi_compressedPacket)
    {
        HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

        // initialize codec
        HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_uncompressedPacket.GetDataSize());
        pi_compressedPacket.SetBufferOwnership(true);
        size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
        pi_compressedPacket.SetBuffer(new Byte[compressedBufferSize], compressedBufferSize * sizeof(Byte));
        const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(Byte), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(Byte));
        pi_compressedPacket.SetDataSize(compressedDataSize);

        return true;
    }

    bool LoadCompressedPacket(const HCDPacket& pi_compressedPacket,
        HCDPacket& pi_uncompressedPacket)
    {
        HPRECONDITION(pi_compressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

        // initialize codec
        HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_compressedPacket.GetDataSize());
       /* pi_uncompressedPacket.SetBufferOwnership(true);
        pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));*/
        const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
        pi_uncompressedPacket.SetDataSize(compressedDataSize);

        return true;
    }


    
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
////////MST_TS

template <class EXTENT> class SMIndexMasterHeader 
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
        HPMBlockID              m_rootNodeBlockID;

        SMIndexMasterHeader<EXTENT>& operator=(const SQLiteIndexHeader& indexHeader);
        
        operator SQLiteIndexHeader();        
    };


#define MAX_NEIGHBORNODES_COUNT 26

    /** -----------------------------------------------------------------------------

The index node header is the base header for any spatial index nodes control. Since the
spatial index node receives the actual node header type in parameter, the actual
node header can be another however ALL members using the same name and type must be defined
and be public for the provided node header. It is usually a lot easier to
inherit any actual node header from the provided HGFIndexNodeHeader.
-----------------------------------------------------------------------------
*/
template <typename EXTENT> class SMIndexNodeHeaderBase
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
        uint64_t    m_totalCount;                // This value indicates the total number of points in node all recursively all sub-nodes.
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

template <typename EXTENT> class SMIndexNodeHeader : public SMIndexNodeHeaderBase<EXTENT>
    {
    public:
   
        HPMBlockID          m_parentNodeID; //Required when loading 
        vector<HPMBlockID>  m_apSubNodeID;
        HPMBlockID          m_SubNodeNoSplitID;
        bool                m_filtered;    
    
        //NEEDS_WORK_SM - m_meshed ?
        size_t      m_nbFaceIndexes;
        size_t      m_nbUvIndexes;
        size_t      m_nbTextures;
        HPMBlockID  m_graphID;
        std::vector<HPMBlockID>  m_textureID;
        HPMBlockID  m_uvID;

        //NEEDS_WORK_SM - should not be a vector.
        std::vector<HPMBlockID>  m_ptsIndiceID;
        std::vector<HPMBlockID>  m_uvsIndicesID;
        size_t      m_numberOfMeshComponents;
        int*        m_meshComponents = nullptr;
        size_t      m_nodeCount;

        std::vector<HPMBlockID> m_apNeighborNodeID[MAX_NEIGHBORNODES_COUNT];    
        bool                    m_apAreNeighborNodesStitched[MAX_NEIGHBORNODES_COUNT];

        std::vector<HPMBlockID> m_clipSetsID;

        ~SMIndexNodeHeader();
             
        SMIndexNodeHeader<EXTENT>& operator=(const SQLiteNodeHeader& nodeHeader);
            
        operator SQLiteNodeHeader();
           
        };


template <class EXTENT> class SMSQLiteStore : public ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>
    {
    private : 

        SMSQLiteFilePtr m_smSQLiteFile;

    public : 
    
        SMSQLiteStore(SMSQLiteFilePtr database);
            
        virtual ~SMSQLiteStore();
                    
        virtual uint64_t GetNextID() const override;
            
        virtual void Close() override;
            
        virtual bool StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t StoreHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;
            
        virtual size_t LoadHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;            
    };


template <class POINT, class EXTENT> class SMSQLiteNodePointStore : public ISMNodeDataStore<POINT, SMIndexNodeHeader<EXTENT>> 
    {
    private:

        SMSQLiteFilePtr m_smSQLiteFile;
    
    public:
              
        SMSQLiteNodePointStore(SMIndexNodeHeader<EXTENT>* nodeHeader,/* ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>* dataStore,*/ SMSQLiteFilePtr& smSQLiteFile);
            
        virtual ~SMSQLiteNodePointStore();
              
        virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData) override;
            
        virtual HPMBlockID StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;
            
        virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        virtual bool DestroyBlock(HPMBlockID blockID) override;            
    };
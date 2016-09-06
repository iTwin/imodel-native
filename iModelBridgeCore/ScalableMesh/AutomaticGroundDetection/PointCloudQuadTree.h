/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PointCloudQuadTree.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once

#include <DgnPlatform\DgnPlatform.r.h>
#include <DgnPlatform\ElementHandle.h>
#include <DgnPlatform\XAttributeHandler.h>
#include <RmgrTools\Tools\DataExternalizer.h>

#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\ITransactionHandler.h>
#include <DgnPlatform\XAttributeHandler.h>

#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudChannel.h>
#include <DgnPlatform\PointCloudClipHandler.h>
USING_NAMESPACE_BENTLEY_POINTCLOUD
#include <PointCloud\PointCloudDataQuery.h>
#include <ScalableMesh\AutomaticGroundDetection\GroundDetectionManager.h>


USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct quadSeed;
class PointCloudQuadTree;
class PointCloudQuadNode;
class ProgressBarMonitorForTree;

typedef RefCountedPtr<quadSeed>                    QuadSeedPtr;
typedef RefCountedPtr<PointCloudQuadTree>          PointCloudQuadTreePtr;
typedef RefCountedPtr<PointCloudQuadNode>          PointCloudQuadNodePtr;


struct quadSeed : public BENTLEY_NAMESPACE_NAME::RefCountedBase
    {
    protected : 
    
        quadSeed();
        quadSeed(std::vector<DPoint3d> seedPoints_, PointCloudQuadNode* tile_);

    public :

        std::vector<DPoint3d> seedPoints;
        PointCloudQuadNode* tile; // Pointer PointCloudQuadNode
        double density;
        double trustworthiness;
        std::vector<int> groundPts;

        static QuadSeedPtr Create() {return new quadSeed();}
        
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/

class PointCloudQuadTreeData
    {
    public:
        PointCloudQuadTreeData()
            : m_maxDepth(5),
            m_sizeBound(50),
            m_maxDataSize(400000),
            m_triangleIndexes(NULL),
            m_elHandle(NULL),
            m_points(NULL),
            m_bb(NULL)
            {
            }
        DPoint3d*               m_bb;
        std::vector<DPoint3d>*  m_points;
        std::vector<int>*       m_triangleIndexes;
        int                     m_maxDepth;
        size_t                  m_maxDataSize;
        unsigned                m_sizeBound;
        EditElementHandleP      m_elHandle;

    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
/*
* Class Node
* A node is a branch of quadTree
* save regions (children of the node) and parent node
* save seeds in this node
* can return points in this node (query)
*/
class PointCloudQuadNode : public RefCountedBase
    {
    public:
        static PointCloudQuadNodePtr Create(DPoint3d* bb, PointCloudQuadTree* tree, PointCloudQuadNode* parent, int depth, bool init = false);

        std::vector<size_t>&    findTries(DPoint3d& searchPoint);
        std::vector<size_t>&    findTriesParent();
        PointCloudQuadNode*     findNode(DPoint3dCR searchPoint);
        inline int              findRegion(DPoint3dCR point);
        void                    addTriangleToRegion(DPoint3d* triangle, size_t startIdx);
        void                    addToRegion(size_t startIdx, int region);
        //creates a subnode
        int                     createSubRegion(int currentDepth, size_t dataSize);
        int                     createSubRegion(int currentDepth, size_t dataSize, std::vector<QuadSeedPtr>& seeds, size_t& lastLoadedSeedIdx);
        void                    SetDataSize(size_t dataSize) {m_dataSize = dataSize;}
        size_t                  GetDataSize() {return m_dataSize;}
        int                     GetDepth() {return m_depth;}
        //computes all descendants that have seeds
        bool                    seedSearch(std::vector<QuadSeedPtr>& seeds);
        //compute seeds for this node
        void                    fetchSeedsFromQuadTree(QuadSeedPtr& seed);
        void                    drawNode(EditElementHandleR elHandle);
        void                    visitUnmarkedNodes(std::vector<QuadSeedPtr>& seeds);
        bool                    getMarked() {return m_marked;}
        void                    setMarked(bool marked) {m_marked = marked;}
        void                    setSubreeMark();
        size_t                  getAllDataPoints(std::vector<DPoint3d>& pointsBuffer);
        size_t                  getAllDataPoints(IPointCloudDataQueryPtr& query, IPointCloudQueryBuffersPtr& queryBuffer, IPointCloudChannelVector& queryChannels);
        void                    createTrianglesRegions();

        //manages a local cache of the point cloud data (for thread safety)
        void                    loadData(bool loadClassif = false);
        void                    saveClassification();
        void                    unloadData();

        bool operator<(const PointCloudQuadNode* other) const
            {
            return uid < other->uid;
            }

        DPoint3d                m_corner[2];        // Boxing box of node
        DPoint3d                m_pivot;            // center of node
        PointCloudQuadTree*     m_tree;
        PointCloudQuadNode*     m_parent;           //back pointer to parent
        PointCloudQuadNodePtr   m_region[4];
        size_t                  m_dataSize;         // Number of data in this node
        std::vector<size_t>     m_tries;            // Index of triangles in this node
        char                    m_bitRegions;
        int                     m_depth;
        bool                    m_marked;
        long                    m_ptsGood;          // For debug
        long                    m_ptsError;         // For debug
        long                    m_ptsGround;        // For debug
        long                    m_ptsMiss;          // For debug
        long                    m_ptsGroundGlobal;  // for debug
        long                    m_ptsTotal;         // for debug
        long                    m_ptsGround2;       // for debug

        size_t                  m_nOfPointsRead;
        DPoint3d*               m_pXyzBuffer;
        UChar*                  m_pFilterBuffer;
        UChar*                  m_pChannelBuffer;
        IPointCloudQueryBuffersPtr m_buffers;
        IPointCloudDataQueryPtr m_query;
        std::atomic<bool>                    isReady; //has the node data been cached yet, used during filtering by worker threads
        enum{ SW=0, SE, NW, NE};

        const int uid;

    protected:
        PointCloudQuadNode(DPoint3d* bb, PointCloudQuadTree* tree, PointCloudQuadNode* parent, int depth, bool init = false);
        ~PointCloudQuadNode();

        void findTriangleIntersection(DPoint3d* triangle, size_t startIdx);
        void findIntersectionWithX(DPoint3d& direction, DPoint3d* triangle, size_t startIdx, int k);
        void findIntersectionWithY(DPoint3d& direction, DPoint3d* triangle, size_t startIdx, int k);

    private:
        const size_t MAX_N_LOADED_TILES = 50;
        static int newID;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
/*
* QuadTree class
* Save Seeds and seeds triangulation
* Save nodes
* Have an spatial index for seed triangulation
* can find all triangles in a node
*/

class PointCloudQuadTree : public RefCountedBase
    {
    friend PointCloudQuadNode;

    public:
        static PointCloudQuadTreePtr Create(PointCloudQuadTreeData* data, std::vector<QuadSeedPtr>& seeds, ProgressReportR report, float density = 1.0);
        static PointCloudQuadTreePtr Create(PointCloudQuadTreeData* data, ProgressReportR report, float density = 1.0);

        //finds all seeds for the tree, IGroundDetectionProgressListener will be called after a block of seeds has been generated
        bool                    createSeeds(std::vector<QuadSeedPtr>& seeds, double distanceThreshold, double minDistance, ProgressReportR report);
        std::vector<size_t>&    findTries(DPoint3d& searchPoint);
        const int*              getBitVector() { return BITVECTOR; }
        void                    drawTree(EditElementHandleR elHandle);
        void                    visitUnmarkedNodes(std::vector<QuadSeedPtr>& seeds);
        void                    setTriangles(std::vector<DPoint3d>& points, std::vector<int>& indices);
        DPoint3d                GetPoint(int idx) {return m_points.at(idx);}
        int                     GetTriangleIndexe(size_t idx) { return m_triangleIndexes->at(idx); }
        double                  GetEpsilon() {return D_EPSILON_2;}
        size_t                  GetMaxDataSize(){return m_maxDataSize;}
        void                    setSeedsPoints(std::vector<DPoint3d>& seedsPoints, std::vector<QuadSeedPtr>& seeds);
        //exclude seeds that are locally but not globally fitting the criteria, e.g. points on roofs when there are no ground points in the tile.
        void                    excludeOutlierSeeds(std::vector<QuadSeedPtr>& seeds);

    private:
        PointCloudQuadTree(PointCloudQuadTreeData* data, std::vector<QuadSeedPtr>& seeds, ProgressReportR report, float density = 1.0);
        PointCloudQuadTree(PointCloudQuadTreeData* data, ProgressReportR report, float density = 1.0);
        ~PointCloudQuadTree();
        void                    createTree(PointCloudQuadTreeData* data, ProgressReportR report);
        void                    createTreeAndSeeds(PointCloudQuadTreeData* data, std::vector<QuadSeedPtr>& seeds, ProgressReportR report);
        void                    setSize(int val) {m_maxSize = val;}
        void                    setMaxDepth(int val) {m_maxDepth = val;}
        int                     getMaxDepth() {return m_maxDepth;}
        size_t                  getMaxSize() {return m_maxSize;}
        EditElementHandleP      getElHandle() {return m_elHandle;}
        PointCloudQuadNode*     findNode(DPoint3dCR searchPoint);

        PointCloudQuadNodePtr   m_root;                 // First node
        double                  m_nbNodeFetched;
        std::vector<DPoint3d>        m_points;
        std::vector<int>*       m_triangleIndexes;
        EditElementHandleP      m_elHandle;
        size_t                  m_maxSize;
        size_t                  m_maxDataSize;
        int                     m_maxDepth;
        size_t                  m_dataSize;
        static const double     D_EPSILON_2;
        static const int        BITVECTOR[];
        float m_densityVal = 1.0;

    };

/*
* Add triangle ID in node
*/
inline void PointCloudQuadNode::addToRegion(size_t startIdx, int region)
    {
    if(!(m_bitRegions & m_tree->getBitVector()[region]))
        {
        m_region[region]->m_tries.push_back(startIdx);
        m_bitRegions |= m_tree->getBitVector()[region];
        }
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE

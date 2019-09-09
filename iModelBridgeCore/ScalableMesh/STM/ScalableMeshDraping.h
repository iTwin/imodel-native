/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshDraping.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/04/20 12:32:17 $
|     $Author: Elenie.Godzaridis $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <queue>
#include <future>
#include <stdint.h>
#include <set>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshDraping : IDTMDraping
    {
    private:
        IScalableMesh* m_scmPtr;
        Transform m_transform;
        Transform m_UorsToStorage;
        DTMAnalysisType m_type = DTMAnalysisType::Precise;

        size_t m_levelForDrapeLinear;

        bvector<IScalableMeshNodePtr> m_nodeSelection;
        bvector<CurveVectorPtr> m_regionRestrictions;


        DTMStatusInt DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedTypeP, DPoint3dCR point, const DMatrix4d& w2vMap);


        size_t ComputeLevelForTransform(const DMatrix4d& w2vMap);

        void QueryNodesBasedOnParams(bvector<IScalableMeshNodePtr>& nodes, const DPoint3d& testPt, const IScalableMeshNodeQueryParamsPtr& params, const IScalableMeshPtr& targetedMeshPtr);

    protected:

        virtual DTMStatusInt _DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedTypeP, DPoint3dCR point) override;


        virtual DTMStatusInt _DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) override;

        virtual bool _DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector) override;
       
        virtual bool _ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint) override;

        virtual bool _IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) override;
        virtual bool _IntersectRay(bvector<DTMRayIntersection>& pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) override;

    public:
        ScalableMeshDraping(IScalableMeshPtr scMesh);
        virtual ~ScalableMeshDraping(){}
        void SetTransform(TransformR transform)
            {
            m_transform = transform;
            m_UorsToStorage.InverseOf(m_transform);
            }

        void SetAnalysisType(DTMAnalysisType type)
            {
            m_type = type;
            }

        DTMAnalysisType GetAnalysisType()
        {
            return m_type;
        }
            

            void ClearNodes()
                {
                m_nodeSelection.clear();
                }

        void SetRegionRestrictions(const bvector<CurveVectorPtr>& regions)
        {
            m_regionRestrictions = regions;
        }

        void ClearRegionRestrictions()
        {
            m_regionRestrictions.clear();
        }
    };

struct MeshTraversalStep
    {
    IScalableMeshNodePtr linkedNode;
    DPoint3d startPoint;
    bool hasEndPoint;
    DPoint3d endPointOfLastProjection;
    int currentSegment;
    bool fetchNeighborsOnly;
    IScalableMeshNodePtr nodeRef;
    char direction;
    MTGNodeId lastEdge;
    };

typedef std::function<void(MeshTraversalStep)> NodeCallback;
struct MeshTraversalQueue
    {
    private:
        size_t m_levelForDrapeLinear;
        std::queue<MeshTraversalStep> m_nodesRemainingToDrape;
        std::map<double, MeshTraversalStep> m_collectedNodes;
        const DPoint3d* m_polylineToDrape;
        size_t m_numPointsOnPolyline = 0;
        IScalableMesh*   m_scm = nullptr;
        MeshTraversalStep m_currentStep;
        std::set<int64_t> allVisitedNodes;
        //The ID of the plane that is shared with the next node in the direction of the line.
        // 0 = right, 1 = left, 2 = bottom, 3 = top 
        unsigned char m_intersectionWithNextNode = 255;
        DPoint3d    m_endOfLineInNode; //intersection between polyline and the first of the node's boundaries it hits
		Transform m_reproTransform;
		bool m_isReprojected;


        void                                 ComputeDirectionOfNextNode(MeshTraversalStep& start);
        IScalableMeshNodePlaneQueryParamsPtr GetPlaneQueryParam(size_t depth, size_t segmentId);
        

    public:

        std::map<int64_t, std::future<DTMStatusInt>> m_nodesToLoad;

        MeshTraversalQueue(const DPoint3d* line, int nPts, size_t levelForDrapeLinear, Transform reprojectionTransform=Transform::FromIdentity()) :m_polylineToDrape(line), m_numPointsOnPolyline((size_t)nPts), m_levelForDrapeLinear(levelForDrapeLinear)
            {
			m_isReprojected = !reprojectionTransform.IsIdentity();
			if (m_isReprojected)
				m_reproTransform = reprojectionTransform;
		};
        void UseScalableMesh(IScalableMesh* ptr)
            {
            m_scm = ptr;
            }
        void SetLastEdge(MTGNodeId edge)
            {
            m_currentStep.lastEdge = edge;
            }

        size_t GetNumberOfNodes()
            {
            return m_nodesRemainingToDrape.size();
            }
        static const size_t ALL_CHUNKS = (size_t)-1;
        void GetNodes(bvector < MeshTraversalStep>& nodes, size_t numberOfChunks)
            {
            size_t n = 0;
            while (n < numberOfChunks && !m_nodesRemainingToDrape.empty())
                {
                nodes.push_back(m_nodesRemainingToDrape.front());
                m_nodesRemainingToDrape.pop();
                ++n;
                }
            }

        void CollectAll();

        void CollectAll(const bvector<IScalableMeshNodePtr>& inputNodes);


        bool TryStartTraversal(bool& needProjectionToFindFirstTriangle, int segment);
        bool HasNodesToProcess();
        bool NextAlongElevation();
        bool NextAlongDirection();
        MeshTraversalStep& Step();
        bool SetStartPoint(DPoint3d pt);
        void Clear();
        bool CollectIntersects(bool& findTriangleAlongRay);
        bool CollectAlongElevation(MeshTraversalStep& start, NodeCallback c);
            bool CollectAlongDirection(MeshTraversalStep& start, NodeCallback c);

    };

struct Tile3dTM :public RefCounted<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM>, IDTMDraping
    {
    private:
        const IScalableMeshNodePtr m_node;
        DRange3d m_range;

    protected:
///#ifdef VANCOUVER_API
        virtual DTMStatusInt _DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedTypeP, DPoint3dCR point) override
            {
            return DTM_ERROR;
            }
/*#else
        virtual DTMStatusInt _DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point) override
            {
            return DTM_ERROR;
            }
#endif*/

        virtual DTMStatusInt _DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) override;

        virtual bool _DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector) override
            {
            return DTM_ERROR;
            }

        virtual bool _ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
            {
            return false;
            }

        virtual bool _IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) override
            {
            return DTM_ERROR;
            }

        virtual bool _IntersectRay(bvector<DTMRayIntersection>& pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) override
            {
            return DTM_ERROR;
            }

        virtual IDTMDrapingP     _GetDTMDraping() override
            {
            return this;
            }

        virtual IDTMDrainageP    _GetDTMDrainage() override
            {
            return nullptr;
            }

        virtual IDTMContouringP  _GetDTMContouring() override
            {
            return nullptr;
            }

        virtual int64_t _GetPointCount() override
            {
            return (int64_t)m_node->GetPointCount();
            }

        virtual DTMStatusInt _GetRange(DRange3dR range) override
            {
            range = m_node->GetContentExtent();
            return DTM_SUCCESS;
            }

        virtual BcDTMP _GetBcDTM() override
            {
            return nullptr;
            }

        virtual DTMStatusInt _GetBoundary(DTMPointArray& ret) override
            {
            return DTM_ERROR;
            }

        virtual DTMStatusInt _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints) override
            {
            return DTM_ERROR;
            }

        virtual DTMStatusInt _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback) override
            {
            return DTM_ERROR;
            }

        virtual DTMStatusInt _GetTransformDTM(DTMPtr& transformedDTM, TransformCR transformation) override
            {
            return DTM_ERROR;
            }

        virtual bool _GetTransformation(TransformR transformation) override
            {
            return false;
            }

        virtual IDTMVolumeP _GetDTMVolume() override
            {
            return nullptr;
            }

        virtual DTMStatusInt _ExportToGeopakTinFile(WCharCP fileNameP, TransformCP transformation) override
            {
            return DTM_ERROR;
            }

    public:
        Tile3dTM(IScalableMeshNodePtr node): m_node(node)
            {}

        static DTMPtr Create(IScalableMeshNodePtr node)
            {
            return new Tile3dTM(node);
            }
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE

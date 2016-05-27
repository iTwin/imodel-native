/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshDraping.h $
|    $RCSfile: ScalableMeshDraping.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/04/20 12:32:17 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <queue>

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

        DTMStatusInt DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point, const DMatrix4d& w2vMap);

        size_t ComputeLevelForTransform(const DMatrix4d& w2vMap);

        void QueryNodesBasedOnParams(bvector<IScalableMeshNodePtr>& nodes, const DPoint3d& testPt, const IScalableMeshNodeQueryParamsPtr& params);

    protected:
        virtual DTMStatusInt _DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point) override;

        virtual DTMStatusInt _DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) override;
        //virtual DTMStatusInt _FastDrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) override;

        virtual bool _DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector) override;
        //virtual bool _FastDrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector) override;
        
        virtual bool _ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint) override;
    public:
        ScalableMeshDraping(IScalableMeshPtr scMesh);
        void SetTransform(TransformR transform)
            {
            m_transform = transform;
            m_UorsToStorage = m_transform.ValidatedInverse();
            }

        void SetAnalysisType(DTMAnalysisType type)
            {
            m_type = type;
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
        std::set<__int64> allVisitedNodes;
        //The ID of the plane that is shared with the next node in the direction of the line.
        // 0 = right, 1 = left, 2 = bottom, 3 = top 
        unsigned char m_intersectionWithNextNode = 255;
        DPoint3d    m_endOfLineInNode; //intersection between polyline and the first of the node's boundaries it hits

        void ComputeDirectionOfNextNode(MeshTraversalStep& start);

    public:
        MeshTraversalQueue(const DPoint3d* line, int nPts, size_t levelForDrapeLinear) :m_polylineToDrape(line), m_numPointsOnPolyline((size_t)nPts), m_levelForDrapeLinear(levelForDrapeLinear)
            {};
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

END_BENTLEY_SCALABLEMESH_NAMESPACE

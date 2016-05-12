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
    protected:
        virtual DTMStatusInt _DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedType, DPoint3dCR point) override;
        virtual DTMStatusInt _DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) override;
    public:
        ScalableMeshDraping(IScalableMeshPtr scMesh);
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
        MeshTraversalQueue(const DPoint3d* line, int nPts) :m_polylineToDrape(line), m_numPointsOnPolyline((size_t)nPts)
            {};
        void UseScalableMesh(IScalableMesh* ptr)
            {
            m_scm = ptr;
            }
        void SetLastEdge(MTGNodeId edge)
            {
            m_currentStep.lastEdge = edge;
            }
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

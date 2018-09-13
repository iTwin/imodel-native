#include "ScalableMeshPCH.h"
#include "ImagePPHeaders.h"
#include "ScalableMeshDraping.h"
#include "ScalableMeshQuadTreeQueries.h"
#include <queue>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
bool s_civilDraping = true;
struct SMDrapedLine;
struct SMDrapedPoint : RefCounted<IDTMDrapedLinePoint>
    {
    private: 
        DPoint3d m_pt;
        DTMDrapedLinePtr m_lineRef;
    protected:
        virtual DTMStatusInt _GetPointCoordinates(DPoint3d& coordP) const override
            {
            coordP = m_pt;
            return DTMStatusInt::DTM_SUCCESS;
            }
        virtual double _GetDistanceAlong() const override;
        virtual DTMDrapedLineCode _GetCode() const override
            {
            return DTMDrapedLineCode::Tin;
            }

        SMDrapedPoint(const DPoint3d& val, DTMDrapedLinePtr line)
            {
            m_pt = val;
            m_lineRef = line;
            }
    public:
        static SMDrapedPoint* Create(const DPoint3d& val, DTMDrapedLinePtr line)
            {
            return new SMDrapedPoint(val, line);
            }
    };

struct SMDrapedLine : RefCounted<IDTMDrapedLine>
    {
    private:
        bvector<DPoint3d> m_linePts;
        bvector<double> m_distancePts;
		size_t m_maxPtOnLineIdx, m_minPtOnLineIdx;

    protected:
        virtual DTMStatusInt _GetPointByIndex(DTMDrapedLinePointPtr& ret, unsigned int index) const override
            {
            if (index >= m_linePts.size()) return DTMStatusInt::DTM_ERROR;
            ret = SMDrapedPoint::Create(m_linePts[index], DTMDrapedLinePtr((IDTMDrapedLine*)this));
            return DTMStatusInt::DTM_SUCCESS;
            }

//#ifdef VANCOUVER_API
        virtual DTMStatusInt _GetPointByIndex(DPoint3dR ptP, double* distanceP, DTMDrapedLineCode* codeP, unsigned int index) const override
//#else
//        virtual DTMStatusInt _GetPointByIndex(DPoint3dP ptP, double* distanceP, DTMDrapedLineCode* codeP, unsigned int index) const override
//#endif
            {
            if (index >= m_linePts.size()) return DTMStatusInt::DTM_ERROR;
//#ifdef VANCOUVER_API
            ptP = m_linePts[index];
//#else
//           *ptP = m_linePts[index];
//#endif
            if (distanceP != nullptr) *distanceP = m_distancePts[index];
            if (codeP != nullptr) *codeP = index <= m_maxPtOnLineIdx && index >= m_minPtOnLineIdx ? DTMDrapedLineCode::Tin : DTMDrapedLineCode::External;
            return DTMStatusInt::DTM_SUCCESS;
            }
        virtual unsigned int _GetPointCount() const override
            {
            return (unsigned int)m_linePts.size();
            }
        SMDrapedLine(const bvector<DPoint3d>& line, double distance, size_t maxPtOnLineIdx, size_t minPtOnLineIdx)
            {
            m_linePts = line;
			if (maxPtOnLineIdx == 0) m_maxPtOnLineIdx = line.size()-1;
			else m_maxPtOnLineIdx = maxPtOnLineIdx;
			m_minPtOnLineIdx = minPtOnLineIdx;
            m_distancePts.resize(m_linePts.size());
			double distUntil = 0;
            auto distIt = m_distancePts.begin();
            for (auto it = m_linePts.begin(); it != m_linePts.end(); distIt++, it++)
                {
                if (it != m_linePts.begin())
                    {
                    distUntil += DVec3d::FromStartEnd(*(it - 1), *it).MagnitudeXY();
                    }

                *distIt = distUntil;
                }

            }

    public:
        static SMDrapedLine* Create(const bvector<DPoint3d>& line, double distance =0.0, size_t maxPtOnLineIdx=0, size_t minPtOnLineIdx =0)
            {
            return new SMDrapedLine(line, distance, maxPtOnLineIdx, minPtOnLineIdx);
            }
        double GetDistanceUntil(const DPoint3d& pt) const
            {
            auto it = m_linePts.begin();
            bool foundPt = false;
            double dist = 0.0;
            if (it->x == pt.x && it->y == pt.y && it->z == pt.z) return dist;
            while (++it != m_linePts.end() && !foundPt)
                {
                dist += DVec3d::FromStartEnd(*(it - 1), *it).Magnitude();
                if (it->x == pt.x && it->y == pt.y && it->z == pt.z) foundPt = true;
                }
            return dist;
            }

    };

double SMDrapedPoint::_GetDistanceAlong() const
    {
    return dynamic_cast<SMDrapedLine*>(m_lineRef.get())->GetDistanceUntil(m_pt);
    }


IScalableMeshNodePlaneQueryParamsPtr MeshTraversalQueue::GetPlaneQueryParam(size_t depth, size_t segmentId)
{
	IScalableMeshNodePlaneQueryParamsPtr paramsLine = IScalableMeshNodePlaneQueryParams::CreateParams();
	paramsLine->SetLevel(depth);

	DPoint3d pointOnDirection, origin;
	DVec3d drapeDirection = DVec3d::From(0, 0, -1);
	m_reproTransform.Multiply(origin, m_polylineToDrape[segmentId]);

	pointOnDirection.SumOf(origin, drapeDirection);
    bool    inverseOf(TransformCP pIn);

#ifdef VANCOUVER_API
    Transform t; 
    t.inverseOf(&m_reproTransform);
#else
    Transform t = m_reproTransform.ValidatedInverse();
#endif
	t.Multiply(pointOnDirection, pointOnDirection);

	DPlane3d targetCuttingPlane = DPlane3d::From3Points(m_polylineToDrape[segmentId], m_polylineToDrape[segmentId + 1], pointOnDirection);
	paramsLine->SetPlane(targetCuttingPlane);

	return paramsLine;
}


void MeshTraversalQueue::CollectAll(const bvector<IScalableMeshNodePtr>& inputNodes)
    {
    bvector<bool> skipNode(inputNodes.size(), false);

    bmap<int64_t,bset<int64_t>> parents;
    bmap<int64_t, bset<size_t>> children;
    size_t minLevel = INT_MAX;
    size_t maxLevel = 0;

    for (auto&node : inputNodes)
    {
        minLevel = std::min(minLevel, node->GetLevel());
        maxLevel = std::max(maxLevel, node->GetLevel());
        IScalableMeshNodePtr nodeIter = node;
        while (nodeIter->GetLevel() > 0)
        {
            nodeIter = nodeIter->GetParentNode();
            parents[nodeIter->GetLevel()].insert(nodeIter->GetNodeId());
            children[nodeIter->GetNodeId()].insert(&node - inputNodes.data());
        }
    }

    for (auto&node : inputNodes)
    {
        if (parents[node->GetLevel()].count(node->GetNodeId()) > 0)
        {
            for (auto&index : children[node->GetNodeId()])
                skipNode[index] = true;
        }
    }

    for (size_t segment = 0; segment < m_numPointsOnPolyline - 1; segment++)
        {
        DRay3d ray = DRay3d::FromOriginAndVector(m_polylineToDrape[segment], DVec3d::FromStartEndNormalize(m_polylineToDrape[segment], m_polylineToDrape[segment + 1]));

        DRange3d range;
        m_scm->GetRange(range);
        bvector<IScalableMeshNodePtr> outNodes;
        for (auto& node : inputNodes)
            {
            if (skipNode[&node - inputNodes.data()])
                continue;
			if (!m_isReprojected)
			{
            ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, DRange3d> query(range,
                                                                                   node->GetLevel(),
                                                                                   ray,
                                                                                   true,
                                                                                   DVec3d::FromStartEnd(m_polylineToDrape[segment], m_polylineToDrape[segment + 1]).Magnitude(),
                                                                                   true,
                                                                                   ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, DRange3d>::RaycastOptions::ALL_INTERSECT);
            node->RunQuery(query, outNodes);
			}
			else
                {   
				IScalableMeshNodePlaneQueryParamsPtr paramsLine(GetPlaneQueryParam(m_levelForDrapeLinear, segment));

                ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<DPoint3d, DRange3d> query(range, node->GetLevel(), paramsLine->GetPlane(), paramsLine->GetDepth());

                node->RunQuery(query, outNodes);
                }             
            }

        for (auto& node : outNodes)
            {
            if (node.get() == nullptr) continue;
            MeshTraversalStep step;
            step.currentSegment = (int)segment;
            step.startPoint = m_polylineToDrape[segment];
            step.linkedNode = node;
            m_nodesRemainingToDrape.push(step);
            }
        }
    }

void MeshTraversalQueue::CollectAll()
    {
    for (size_t segment = 0; segment < m_numPointsOnPolyline - 1; segment++)
        {
		bvector<IScalableMeshNodePtr> nodes;
		if(!m_isReprojected)
			{ 
			IScalableMeshNodeQueryParamsPtr paramsLine = IScalableMeshNodeQueryParams::CreateParams();
			paramsLine->SetLevel(m_levelForDrapeLinear);
			paramsLine->SetDirection(DVec3d::FromStartEndNormalize(m_polylineToDrape[segment], m_polylineToDrape[segment + 1]));
			paramsLine->SetDepth(DVec3d::FromStartEnd(m_polylineToDrape[segment], m_polylineToDrape[segment + 1]).Magnitude());
			paramsLine->Set2d(true);

			IScalableMeshNodeRayQueryPtr queryLine = m_scm->GetNodeQueryInterface();
			if (queryLine->Query(nodes, &m_polylineToDrape[segment], NULL, 0, paramsLine) != SUCCESS)
				{
				continue;
				}
			}       
		else
		    {
            IScalableMeshNodePlaneQueryParamsPtr paramsLine(GetPlaneQueryParam(m_levelForDrapeLinear, segment));

			IScalableMeshMeshQueryPtr queryLine = m_scm->GetMeshQueryInterface(MESH_QUERY_PLANE_INTERSECT);
			if (queryLine->Query(nodes, NULL, 0, paramsLine) != SUCCESS)
			    {
				continue;
			    }
		    }

		for (auto& node : nodes)
		    {
			if (node.get() == nullptr) continue;
			MeshTraversalStep step;
			step.currentSegment = (int)segment;
			step.startPoint = m_polylineToDrape[segment];
			step.linkedNode = node;
			if (m_nodesToLoad.count(step.linkedNode->GetNodeId()) == 0)
			    {
				m_nodesToLoad.insert(std::make_pair(step.linkedNode->GetNodeId(), std::async([](MeshTraversalStep& step)
				    {
					if (step.linkedNode->ArePoints3d() || step.linkedNode->GetBcDTM() != nullptr)
						return DTM_SUCCESS;
					return DTM_ERROR;
				    }, step)));
			    }
			m_nodesRemainingToDrape.push(step);
		    }
        }


    }

bool MeshTraversalQueue::TryStartTraversal(bool& needProjectionToFindFirstTriangle, int segment)
    {
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    IScalableMeshNodeRayQueryPtr query = m_scm->GetNodeQueryInterface();
    params->SetLevel(m_levelForDrapeLinear);

    MeshTraversalStep firstStep;
    firstStep.currentSegment = segment;
    firstStep.startPoint = m_polylineToDrape[segment];
    firstStep.hasEndPoint = false;
    firstStep.fetchNeighborsOnly = false;
    firstStep.nodeRef = nullptr;
    firstStep.direction = -1;
    DPoint3d* ptr = new DPoint3d();
    *ptr = firstStep.startPoint;
    needProjectionToFindFirstTriangle = false;
    if (query->Query(firstStep.linkedNode, ptr, NULL, 0, params) != SUCCESS)
        {
        //2d ray query to find node along line segment
        IScalableMeshNodeQueryParamsPtr paramsLine = IScalableMeshNodeQueryParams::CreateParams();
        paramsLine->SetLevel(m_levelForDrapeLinear);
        paramsLine->SetDirection(DVec3d::FromStartEndNormalize(m_polylineToDrape[segment], m_polylineToDrape[segment+1]));
        paramsLine->SetDepth(DVec3d::FromStartEnd(m_polylineToDrape[segment], m_polylineToDrape[segment+1]).Magnitude());
        paramsLine->Set2d(true);

        IScalableMeshNodeRayQueryPtr queryLine = m_scm->GetNodeQueryInterface();
        if (queryLine->Query(firstStep.linkedNode, ptr, NULL, 0, paramsLine) != SUCCESS || firstStep.linkedNode.get() == nullptr) return false;
        DRange3d nodeRange = firstStep.linkedNode->GetContentExtent();
        DRange2d nodeRange2d = DRange2d::From(DPoint2d::From(nodeRange.low.x, nodeRange.low.y), DPoint2d::From(nodeRange.high.x, nodeRange.high.y));
        DPoint2d pt1 = DPoint2d::From(m_polylineToDrape[segment].x, m_polylineToDrape[segment].y);
        DPoint2d pt2 = DPoint2d::From(m_polylineToDrape[segment+1].x - m_polylineToDrape[segment].x, m_polylineToDrape[segment+1].y - m_polylineToDrape[segment].y);
        DPoint2d intersect;

        if (!bsiDRange2d_intersectRay(&nodeRange2d, NULL, NULL, &intersect, NULL, &pt1, &pt2)) return false;
        firstStep.startPoint.x = intersect.x;
        firstStep.startPoint.y = intersect.y;
        firstStep.startPoint.z = nodeRange.high.z;
        needProjectionToFindFirstTriangle = true;
        }
    delete ptr;
    if (firstStep.linkedNode.get() == nullptr) return false;
    m_nodesRemainingToDrape.push(firstStep);
    return true;
    }

bool MeshTraversalQueue::HasNodesToProcess()
    {
    return m_nodesRemainingToDrape.size() > 0;
    }

bool MeshTraversalQueue::NextAlongElevation()
    {
    /*bool addedNeighbors = false;
    bvector<IScalableMeshNodePtr> neighbors = m_currentStep.linkedNode->GetNeighborAt(0, 0, m_currentStep.direction);
    if (neighbors.size() == 0 && m_currentStep.direction == -1)
        {
        m_currentStep.direction = 1;
        if (m_currentStep.nodeRef != nullptr) neighbors = m_currentStep.nodeRef->GetNeighborAt(0, 0, m_currentStep.direction);
        else neighbors = m_currentStep.linkedNode->GetNeighborAt(0, 0, m_currentStep.direction);
        }
    if (neighbors.size() > 0) addedNeighbors = true;
    else if (m_currentStep.nodeRef != nullptr)
        {
        addedNeighbors = true;
        MeshTraversalStep nextNode = { m_currentStep.nodeRef, m_currentStep.startPoint,
            m_currentStep.hasEndPoint, m_currentStep.endPointOfLastProjection, m_currentStep.currentSegment, true, IScalableMeshNodePtr(nullptr), (char)-1 };
        m_nodesRemainingToDrape.push(nextNode);
        }
    for (auto nextPtr : neighbors)
        {
        MeshTraversalStep nextNode = { nextPtr, m_currentStep.startPoint,
            m_currentStep.hasEndPoint, m_currentStep.endPointOfLastProjection, m_currentStep.currentSegment, false,
            m_currentStep.nodeRef != nullptr ? m_currentStep.nodeRef : m_currentStep.linkedNode, m_currentStep.direction };
        m_nodesRemainingToDrape.push(nextNode);
        }
    return addedNeighbors;*/
    return CollectAlongElevation(m_currentStep, [this] (MeshTraversalStep s) { this->m_nodesRemainingToDrape.push(s); });
    }

bool MeshTraversalQueue::CollectAlongElevation(MeshTraversalStep& start, NodeCallback c)
    {
    bool addedNeighbors = false;
    bvector<IScalableMeshNodePtr> neighbors = start.linkedNode->GetNeighborAt(0, 0, m_currentStep.direction);
    if (neighbors.size() == 0 && start.direction == -1)
        {
        start.direction = 1;
        if (start.nodeRef != nullptr) neighbors = start.nodeRef->GetNeighborAt(0, 0, start.direction);
        else neighbors = start.linkedNode->GetNeighborAt(0, 0, start.direction);
        }
    if (neighbors.size() > 0) addedNeighbors = true;
    else if (start.nodeRef != nullptr)
        {
        addedNeighbors = true;
        MeshTraversalStep nextNode = { start.nodeRef, start.startPoint,
            start.hasEndPoint, start.endPointOfLastProjection, start.currentSegment, true, IScalableMeshNodePtr(nullptr), (char)-1, -1 };
        c(nextNode);
        }
    for (auto nextPtr : neighbors)
        {
        MeshTraversalStep nextNode = { nextPtr, start.startPoint,
            start.hasEndPoint, start.endPointOfLastProjection, start.currentSegment, false,
            start.nodeRef != nullptr ? start.nodeRef : start.linkedNode, start.direction, -1 };
        c(nextNode);
        }
    return addedNeighbors;
    }


bool NodeMeshIntersectsRay(bool& extentIntersectsRay, MeshTraversalStep& step,  IScalableMeshNodePtr& node, IScalableMeshMeshPtr& meshP, int* triangle, const DPoint3d* line, bool& findTriangleAlongRay, MTGNodeId edge=-1)
    {
    DRay3d ray = DRay3d::FromOriginAndVector(step.startPoint, DVec3d::From(0, 0, -1)); //what would we do if we need to use different draping directions?
    bool withinMesh = false;
    DSegment3d segClipped;
    DRange1d fraction;
    extentIntersectsRay = true;
    if (node->GetPointCount() > 4 && !step.fetchNeighborsOnly)
        {
        DRay3d toTileInterior = DRay3d::FromOriginAndVector(step.startPoint, DVec3d::FromStartEnd(line[step.currentSegment], line[step.currentSegment + 1]));
        //NEEDSWORK_SM: would be better to just use a 2d clipping function here (if 2.5d) or project the ray and extent along drape direction (if 3d)
        toTileInterior.direction.z = 0; //regardless of how the lines were drawn, we are assuming they're projected straight-on. Not sure what's to be done for 3D.
        toTileInterior.origin.z = node->GetContentExtent().low.z;
        if (!findTriangleAlongRay && !node->GetContentExtent().IsContained(step.startPoint, 2)) findTriangleAlongRay = true;
        if (!findTriangleAlongRay && !ray.ClipToRange(node->GetContentExtent(), segClipped, fraction))
            {
            extentIntersectsRay = false;
            return false;
            }
        else if (findTriangleAlongRay && !toTileInterior.ClipToRange(node->GetContentExtent(), segClipped, fraction))
            {
            //it is possible that the ray intersects the right tile, but there is no mesh over the intersection. If that is the case, 
            //then we need to find and add the neighbors to the list. If it is not, we can skip over this tile.
            toTileInterior.origin.z = node->GetNodeExtent().low.z;
            if (!toTileInterior.ClipToRange(node->GetNodeExtent(), segClipped, fraction))
                {
                extentIntersectsRay = false;
                return false;
                }
            }
        else
            {

            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            flags->SetLoadGraph(true);
            meshP = node->GetMesh(flags);
            if (!findTriangleAlongRay) withinMesh = meshP->FindTriangleForProjectedPoint(triangle, step.startPoint, !node->ArePoints3d());
            if (findTriangleAlongRay || !withinMesh)
                {
                withinMesh = meshP->FindTriangleAlongRay(triangle, toTileInterior, edge);
                }
            }
        }
        return withinMesh;
    }

bool MeshTraversalQueue::CollectIntersects(bool& findTriangleAlongRay)
    {
    std::vector<MeshTraversalStep> elevation, direction;
    m_collectedNodes.clear();
    m_currentStep.direction = -1;
   // DPoint3d pt = m_currentStep.startPoint;
   // SetStartPoint(m_currentStep.lastPoint);
    if (m_currentStep.linkedNode->ArePoints3d()) CollectAlongElevation(m_currentStep, [&elevation] (MeshTraversalStep s) { elevation.push_back(s); });
    CollectAlongDirection(m_currentStep, [&direction] (MeshTraversalStep s) { direction.push_back(s); });
    std::vector<MeshTraversalStep> furtherAlongElevation, furtherAlongDirection;
    while (elevation.size() > 0)
        {
        for (auto it = elevation.begin(); it != elevation.end(); ++it)
            {
            int triangle[3] = { -1, -1, -1 };
            DRay3d ray = DRay3d::FromOriginAndVector(it->startPoint, DVec3d::FromStartEnd(m_polylineToDrape[it->currentSegment], m_polylineToDrape[it->currentSegment + 1]));
            DSegment3d segClipped;
            DRange1d fraction;
            bool extentIntersectsRay = false;
            IScalableMeshMeshPtr meshP;
            if (NodeMeshIntersectsRay(extentIntersectsRay, *it, it->linkedNode, meshP, triangle, m_polylineToDrape, findTriangleAlongRay) && it->linkedNode->GetNodeId() != m_currentStep.linkedNode->GetNodeId() /*&& allVisitedNodes.count(it->linkedNode->GetNodeId()) == 0*/)
                {
                DPoint3d triPoints[3];
                for (size_t i = 0; i < 3; ++i) triPoints[i] = meshP->EditPoints()[triangle[i] - 1];
                DRange3d triRange = DRange3d::From(triPoints, 3);
                triRange.low.z = triRange.high.z = 0;
                ray.origin.z = ray.direction.z = 0;
                ray.ClipToRange(triRange, segClipped, fraction);
                m_collectedNodes.insert(make_pair(fraction.low, *it));
                }
            if (!it->fetchNeighborsOnly && extentIntersectsRay) CollectAlongElevation(*it, [&furtherAlongElevation] (MeshTraversalStep s) { furtherAlongElevation.push_back(s); });
            }
        elevation.clear();
        elevation = furtherAlongElevation;
        furtherAlongElevation.clear();
        }
    while (direction.size() > 0)
        {
        for (auto it = direction.begin(); it != direction.end(); ++it)
            {
            auto& step = *it;
            int triangle[3] = { -1, -1, -1 };
            DRay3d ray = DRay3d::FromOriginAndVector(it->startPoint, DVec3d::FromStartEnd(m_polylineToDrape[step.currentSegment], m_polylineToDrape[step.currentSegment + 1]));
            DSegment3d segClipped;
            DRange1d fraction;
            bool extentIntersectsRay = false;
            IScalableMeshMeshPtr meshP;
            ray.direction.z = 0;
            ray.origin.z = step.linkedNode->GetNodeExtent().low.z;
            ray.ClipToRange(step.linkedNode->GetNodeExtent(), segClipped, fraction);
            if (NodeMeshIntersectsRay(extentIntersectsRay, step, step.linkedNode, meshP, triangle, m_polylineToDrape, findTriangleAlongRay))
                {
                DPoint3d triPoints[3];
                for (size_t i = 0; i < 3; ++i) triPoints[i] = meshP->EditPoints()[triangle[i] - 1];
                DRange3d triRange = DRange3d::From(triPoints[0], triPoints[1], triPoints[2]);
                ray = DRay3d::FromOriginAndVector(it->startPoint, DVec3d::FromStartEnd(m_polylineToDrape[step.currentSegment], m_polylineToDrape[step.currentSegment + 1]));
                ray.direction.z = 0;
                ray.origin.z = 0;
                triRange.low.z = 0;
                triRange.high.z = 0;
                if (ray.ClipToRange(triRange, segClipped, fraction) && it->linkedNode->GetNodeId() != m_currentStep.linkedNode->GetNodeId() /*&& allVisitedNodes.count(it->linkedNode->GetNodeId()) == 0*/)
                m_collectedNodes.insert(make_pair(fraction.low, step));
                }
            else if (m_collectedNodes.size() == 0 || m_collectedNodes.begin()->first > fraction.high)
                {
                m_intersectionWithNextNode = 255;
                CollectAlongDirection(*it, [&furtherAlongDirection] (MeshTraversalStep s) { furtherAlongDirection.push_back(s); });
                }
            }
        direction.clear();
        if (m_collectedNodes.size() == 0) direction = furtherAlongDirection;
        furtherAlongDirection.clear();
        }
  //  SetStartPoint(pt);
    if (!s_civilDraping)
        {
        DRay3d ray = DRay3d::FromOriginAndVector(m_currentStep.startPoint, DVec3d::FromStartEnd(m_polylineToDrape[m_currentStep.currentSegment], m_polylineToDrape[m_currentStep.currentSegment + 1]));
        DSegment3d segClipped;
        DRange1d fraction;
        bool extentIntersectsRay = false;
        IScalableMeshMeshPtr meshP;
        int triangle[3] = { -1, -1, -1 };
        if (NodeMeshIntersectsRay(extentIntersectsRay, m_currentStep, m_currentStep.linkedNode, meshP, triangle, m_polylineToDrape, findTriangleAlongRay, m_currentStep.lastEdge))
            {
            DPoint3d triPoints[3];
            for (size_t i = 0; i < 3; ++i)
                {
                triPoints[i] = meshP->EditPoints()[triangle[i] - 1];
                triPoints[i].z = 0;
                }
            DRange3d triRange = DRange3d::From(triPoints, 3);
            triRange.low.z = triRange.high.z = 0;
            ray.origin.z = ray.direction.z = 0;
            double maxParam = DBL_MIN;
            if (ray.ClipToRange(triRange, segClipped, fraction))
                {
                double param, param2;
                DSegment3d edge1 = DSegment3d::From(triPoints[0], triPoints[1]);
                DRay3d triEdge1 = DRay3d::From(edge1);
                DSegment3d edge2 = DSegment3d::From(triPoints[1], triPoints[2]);
                DRay3d triEdge2 = DRay3d::From(edge2);
                DSegment3d edge3 = DSegment3d::From(triPoints[2], triPoints[0]);
                DRay3d triEdge3 = DRay3d::From(edge3);
                if (bsiDRay3d_closestApproach(&param, &param2, NULL, NULL, &ray, &triEdge1) && param2 > 0 && param2 < 1 && param>maxParam) maxParam = param;
                if (bsiDRay3d_closestApproach(&param, &param2, NULL, NULL, &ray, &triEdge2) && param2 > 0 && param2 < 1 && param>maxParam) maxParam = param;
                if (bsiDRay3d_closestApproach(&param, &param2, NULL, NULL, &ray, &triEdge3) && param2 > 0 && param2 < 1 && param>maxParam) maxParam = param;
                if (maxParam > 1.0e-8) m_collectedNodes.insert(make_pair(fraction.low, m_currentStep));
                }
            }
        }
    if(m_collectedNodes.size() > 0) m_nodesRemainingToDrape.push(m_collectedNodes.begin()->second);
    return m_collectedNodes.size() > 0;
    }

void MeshTraversalQueue::ComputeDirectionOfNextNode(MeshTraversalStep& start)
    {
    double param = -1;
    DRange3d ext = start.linkedNode->GetNodeExtent();
    DPlane3d right = DPlane3d::From3Points(DPoint3d::From(ext.high.x, ext.high.y, ext.high.z), DPoint3d::From(ext.high.x, ext.low.y, ext.high.z), DPoint3d::From(ext.high.x, ext.high.y, ext.low.z));
    DPlane3d left = DPlane3d::From3Points(DPoint3d::From(ext.low.x, ext.high.y, ext.high.z), DPoint3d::From(ext.low.x, ext.low.y, ext.high.z), DPoint3d::From(ext.low.x, ext.high.y, ext.low.z));
    DPlane3d nearPlane = DPlane3d::From3Points(DPoint3d::From(ext.high.x, ext.low.y, ext.high.z), DPoint3d::From(ext.low.x, ext.low.y, ext.high.z), DPoint3d::From(ext.high.x, ext.low.y, ext.low.z));
    DPlane3d farPlane = DPlane3d::From3Points(DPoint3d::From(ext.high.x, ext.high.y, ext.high.z), DPoint3d::From(ext.low.x, ext.high.y, ext.high.z), DPoint3d::From(ext.high.x, ext.high.y, ext.low.z));


    ext.high.x += 0.00001;
      ext.low.x -= 0.00001;
      ext.low.y -= 0.00001;
      ext.high.y += 0.00001;
    //compute last intersection of polyline with extent of node (XY)
      for (size_t pt = start.currentSegment; pt < m_numPointsOnPolyline - 1; pt++)
        {
        double lowestParam = DBL_MAX;
        DSegment3d seg = DSegment3d::From(m_polylineToDrape[pt], m_polylineToDrape[pt + 1]);
        if (ext.IsContained(m_polylineToDrape[pt + 1], 2))
            {
            if (pt == m_numPointsOnPolyline - 2) m_endOfLineInNode = m_polylineToDrape[m_numPointsOnPolyline - 1]; //line ends in node
            continue;
            }
        if (seg.Intersect(m_endOfLineInNode, param, right) && param >= 0.0 && param <= 1.0 && ext.IsContained(m_endOfLineInNode, 2))
            {
            if (lowestParam <= 1.0 && param > lowestParam)
                {
                m_intersectionWithNextNode = 0;
                break;
                }
            if (lowestParam > 1.0) m_intersectionWithNextNode = 0;
            lowestParam = param;
            }
        if (seg.Intersect(m_endOfLineInNode, param, left) && param >= 0.0 && param <= 1.0 && ext.IsContained(m_endOfLineInNode, 2))
            {
            if (lowestParam <= 1.0 && param > lowestParam)
                {
                m_intersectionWithNextNode = 1;
                break;
                }
            if (lowestParam > 1.0) m_intersectionWithNextNode = 1;
            lowestParam = param;
            }
        if (seg.Intersect(m_endOfLineInNode, param, nearPlane) && param >= 0.0 && param <= 1.0 && ext.IsContained(m_endOfLineInNode, 2))
            {
            if (lowestParam <= 1.0 && param > lowestParam)
                {
                m_intersectionWithNextNode = 2;
                break;
                }
            if (lowestParam > 1.0) m_intersectionWithNextNode = 2;
            lowestParam = param;
            }
        if (seg.Intersect(m_endOfLineInNode, param, farPlane) && param >= 0.0 && param <= 1.0 && ext.IsContained(m_endOfLineInNode, 2))
            {
            if (lowestParam <= 1.0 && param > lowestParam)
                {
                m_intersectionWithNextNode = 3;
                break;
                }
            if (lowestParam > 1.0) m_intersectionWithNextNode = 3;
            lowestParam = param;
            }
        if (lowestParam <= 1.0) break;
        }
    }

bool MeshTraversalQueue::NextAlongDirection()
    {
    /*if (m_intersectionWithNextNode == 255) ComputeDirectionOfNextNode();
    if (m_intersectionWithNextNode != 255 && !bsiDPoint3d_pointEqualTolerance(&m_endOfLineInNode, &m_polylineToDrape[m_numPointsOnPolyline - 1], 1e-10))
        {
        char relativeX = (m_intersectionWithNextNode == 0 ? 1 : (m_intersectionWithNextNode == 1 ? -1 : 0));
        char relativeY = (m_intersectionWithNextNode == 3 ? 1 : (m_intersectionWithNextNode == 2 ? -1 : 0));
        bvector<IScalableMeshNodePtr> neighbors = m_currentStep.linkedNode->GetNeighborAt(relativeX, relativeY, 0);
        for (auto nextPtr : neighbors)
            {
            MeshTraversalStep nextNode = { nextPtr, m_currentStep.startPoint,
                m_currentStep.hasEndPoint, m_currentStep.endPointOfLastProjection, m_currentStep.currentSegment, false, IScalableMeshNodePtr(nullptr), m_currentStep.direction };
            m_nodesRemainingToDrape.push(nextNode);
            }
        return neighbors.size() > 0;
        }
    else return false;*/
    std::vector<MeshTraversalStep> neighbors;
    std::map<double, MeshTraversalStep> neighborsDistance;
    bool success = CollectAlongDirection(m_currentStep, [&neighbors] (MeshTraversalStep s) { neighbors.push_back(s); });
    std::vector<MeshTraversalStep>  nNeighborsWithNoMeshIntersect;
    std::vector<MeshTraversalStep>  nNeighborsWithNoExtentIntersect;
    for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
        {
        auto& step = *it;
        int triangle[3] = { -1, -1, -1 };
        DRay3d ray = DRay3d::FromOriginAndVector(it->startPoint, DVec3d::FromStartEnd(m_polylineToDrape[step.currentSegment], m_polylineToDrape[step.currentSegment + 1]));
        DSegment3d segClipped;
        DRange1d fraction;
        bool extentIntersectsRay = false;
        IScalableMeshMeshPtr meshP;
        ray.direction.z = 0;
        ray.origin.z = step.linkedNode->GetNodeExtent().low.z;
        ray.ClipToRange(step.linkedNode->GetNodeExtent(), segClipped, fraction);
        bool findTriangleAlongRay = true;
        bool sameNode = step.linkedNode->GetNodeExtent().IsContained(m_currentStep.linkedNode->GetNodeExtent());
        if (s_civilDraping && it->linkedNode->GetNodeId() != m_currentStep.linkedNode->GetNodeId()) neighborsDistance.insert(make_pair(fraction.low, step));
        else if (!s_civilDraping && NodeMeshIntersectsRay(extentIntersectsRay, step, step.linkedNode, meshP, triangle, m_polylineToDrape, findTriangleAlongRay,sameNode?m_currentStep.lastEdge:-1))
            {
            DPoint3d triPoints[3];
            for (size_t i = 0; i < 3; ++i) triPoints[i] = meshP->EditPoints()[triangle[i] - 1];
            DRange3d triRange = DRange3d::From(triPoints[0], triPoints[1], triPoints[2]);
            ray = DRay3d::FromOriginAndVector(it->startPoint, DVec3d::FromStartEnd(m_polylineToDrape[step.currentSegment], m_polylineToDrape[step.currentSegment + 1]));
            ray.direction.z = 0;
            ray.origin.z = 0;
            triRange.low.z = 0;
            triRange.high.z = 0;
            if (ray.ClipToRange(triRange, segClipped, fraction) && it->linkedNode->GetNodeId() != m_currentStep.linkedNode->GetNodeId())
                neighborsDistance.insert(make_pair(fraction.low, step));
            else
                {
                nNeighborsWithNoMeshIntersect.push_back(step);
                }
            }
        else{
            nNeighborsWithNoExtentIntersect.push_back(step);
            }
        }
    for (auto it = neighborsDistance.begin(); it != neighborsDistance.end(); ++it)
        {
        m_nodesRemainingToDrape.push(it->second);
        }
    for (auto& step : nNeighborsWithNoMeshIntersect) m_nodesRemainingToDrape.push(step);
    for (auto& step : nNeighborsWithNoExtentIntersect) m_nodesRemainingToDrape.push(step);
    return success && neighbors.size() > 0;
    }

bool MeshTraversalQueue::CollectAlongDirection(MeshTraversalStep& start, NodeCallback c)
    {
    if (m_intersectionWithNextNode == 255) ComputeDirectionOfNextNode(start);
    if (m_intersectionWithNextNode != 255 && !bsiDPoint3d_pointEqualTolerance(&m_endOfLineInNode, &m_polylineToDrape[m_numPointsOnPolyline - 1], 1e-10))
        {
        char relativeX = (m_intersectionWithNextNode == 0 ? 1 : (m_intersectionWithNextNode == 1 ? -1 : 0));
        char relativeY = (m_intersectionWithNextNode == 3 ? 1 : (m_intersectionWithNextNode == 2 ? -1 : 0));
        bvector<IScalableMeshNodePtr> neighbors = start.linkedNode->GetNeighborAt(relativeX, relativeY, 0);
        for (auto nextPtr : neighbors)
            {
            MeshTraversalStep nextNode = { nextPtr, start.startPoint,
                start.hasEndPoint, start.endPointOfLastProjection, start.currentSegment, false, IScalableMeshNodePtr(nullptr), start.direction, -1 };
            c(nextNode);
            }
        return neighbors.size() > 0;
        }
    else return false;
    }

void MeshTraversalQueue::Clear()
    {
    m_nodesRemainingToDrape = std::queue<MeshTraversalStep>();
    }

MeshTraversalStep& MeshTraversalQueue::Step()
    {
    m_currentStep = m_nodesRemainingToDrape.front();
    m_nodesRemainingToDrape.pop();
    m_intersectionWithNextNode = 255;
    allVisitedNodes.insert(m_currentStep.linkedNode->GetNodeId());
    return m_currentStep;
    }

bool MeshTraversalQueue::SetStartPoint(DPoint3d pt)
    {
    m_currentStep.startPoint = pt;
    return true;
    }

bool IntersectRay3D(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint, IScalableMeshNodePtr& target)
    {
    DRay3d ray = DRay3d::FromOriginAndVector(testPoint, direction);
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
       flags->SetSaveToCache(true);
    flags->SetPrecomputeBoxes(true);
    auto meshP = target->GetMesh(flags);
    if (meshP != nullptr) return meshP->IntersectRay(pointOnDTM, ray);
    return false;
    }

// Get all intesection with ray in an ordered vector (closest -> fardest)
bool IntersectRay3D(bvector<DTMRayIntersection>& pointsOnDTM, DVec3dCR direction, DPoint3dCR testPoint, IScalableMeshNodePtr& target)
    {
    DRay3d ray = DRay3d::FromOriginAndVector(testPoint, direction);
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
    flags->SetSaveToCache(true);
    flags->SetPrecomputeBoxes(true);
    auto meshP = target->GetMesh(flags);
    if (meshP != nullptr) return meshP->IntersectRay(pointsOnDTM, ray); // intersected points are ordered from closest to fardest
    return false;
    }

bool ScalableMeshDraping::_IntersectRay(bvector<DTMRayIntersection>& pointsOnDTM, DVec3dCR direction, DPoint3dCR testPoint)
    {
    bvector<DTMRayIntersection> AllHits;

    DPoint3d transformedPt = testPoint;
    m_UorsToStorage.Multiply(transformedPt);
    DPoint3d startPt = transformedPt;
    DPoint3d endPt = DPoint3d::FromSumOf(testPoint, direction);
    m_UorsToStorage.Multiply(endPt);
    DVec3d newDirection = DVec3d::FromStartEndNormalize(transformedPt, endPt);

    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    IScalableMeshNodeRayQueryPtr query = m_scmPtr->GetNodeQueryInterface();
    if (m_type == DTMAnalysisType::Fast || m_type == DTMAnalysisType::ViewOnly) //other modes use full resolution
        {
        params->SetLevel(std::min((size_t)5, m_scmPtr->GetTerrainDepth()));
        m_scmPtr->GetCurrentlyViewedNodes(m_nodeSelection); // final list will be a subset of m_nodeSelection
        }
    else if (m_scmPtr->IsTerrain()) params->SetLevel(m_scmPtr->GetTerrainDepth());
    bvector<IScalableMeshNodePtr> nodes;
    params->SetDirection(newDirection);
    QueryNodesBasedOnParams(nodes, startPt, params, m_scmPtr);
    m_nodeSelection.clear();

    if (m_type == DTMAnalysisType::ViewOnly && nodes.empty()) //not in view, only do a range intersect in this mode
        {
        DRange3d totalBox;
        m_scmPtr->GetRange(totalBox);
        DRay3d ray = DRay3d::FromOriginAndVector(startPt, newDirection);
        DSegment3d segClipped;
        DRange1d fraction;
        DPoint3d pointOnDTM;
        if (ray.ClipToRange(totalBox, segClipped, fraction))
            {
            if (fraction.low < 0)
                {
                pointOnDTM = segClipped.point[1];//ray starts within box
                if (fraction.high < 0) //ray completely outside box
                    return false;
                }
            else pointOnDTM = segClipped.point[0];
            m_transform.Multiply(pointOnDTM);
            DTMRayIntersection rayInter;
            rayInter.point = pointOnDTM;
            rayInter.rayFraction = (pointOnDTM - testPoint).Magnitude();
            pointsOnDTM.push_back(rayInter); // we add only this one
            return true;
            }
        return false;
        }
    else if (nodes.empty()) 
        QueryNodesBasedOnParams(nodes, startPt, params, m_scmPtr);

    bvector<bool> clips;
    bool ret = false;
    for (auto& node : nodes)
        {
        if (!node->ArePoints3d() && (newDirection.x == 0 && newDirection.y == 0))
            {
            BcDTMPtr dtmP = node->GetBcDTM();
            DPoint3d interP = DPoint3d::From(0,0,0);
            if (dtmP != nullptr && dtmP->GetDTMDraping()->IntersectRay(interP, newDirection, transformedPt))
                {
                DTMRayIntersection RayInter;
                RayInter.point = interP;
                RayInter.hasNormal = false;
                RayInter.rayFraction = (interP - transformedPt).Magnitude();
                AllHits.push_back(RayInter);
                ret = true;
                }
            }
        else if (IntersectRay3D(AllHits, newDirection, transformedPt, node))
            {
            ret = true; // we have at least one
            }
        }

    // transform back to world and sort the hits
    for (auto &hit : AllHits)
    {
        // transform the normals in world
        if (hit.hasNormal)
        {
            DPoint3d startDir = hit.point;
            DPoint3d endDir = DPoint3d::FromSumOf(startDir, hit.normal);
            m_transform.Multiply(startDir);
            m_transform.Multiply(endDir);
            hit.normal = DVec3d::FromStartEndNormalize(startDir, endDir);
        }
        // transform the hit point
        m_transform.Multiply(hit.point);
    }

    // Sort by fraction
    DTMIntersectionCompare Comparator;
    std::sort(AllHits.begin(), AllHits.end(), Comparator);

    if (ret && !m_regionRestrictions.empty())
        {
        ret = false;
        for (auto hit : AllHits)
            {
            bool bInRegion = true;
            for (auto& region : m_regionRestrictions)
                {
                if ((region->PointInOnOutXY(hit.point) == CurveVector::InOutClassification::INOUT_Out && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer) ||
                    (region->PointInOnOutXY(hit.point) == CurveVector::InOutClassification::INOUT_In && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner))
                    {
                    bInRegion = false; // this hit is outside 
                    break;
                    }
                }
            if (bInRegion)
                {
                DTMRayIntersection rayInter;
                pointsOnDTM.push_back(rayInter); // add the point in the output vector
                ret = true; // we have at least a hit
                }
            }
        }
    else if (ret) // No restrictions, we keep all the hits
        {
        pointsOnDTM.insert(pointsOnDTM.end(), AllHits.begin(), AllHits.end()); // insert, in case vector is not empty
        }
    return ret;
    }

bool ScalableMeshDraping::_IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint)
    {    
    DPoint3d transformedPt = testPoint;
    m_UorsToStorage.Multiply(transformedPt);
    DPoint3d startPt = transformedPt;

    DPoint3d endPt = DPoint3d::FromSumOf(testPoint, direction);
    m_UorsToStorage.Multiply(endPt);
    DVec3d newDirection = DVec3d::FromStartEndNormalize(transformedPt, endPt);


    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    IScalableMeshNodeRayQueryPtr query = m_scmPtr->GetNodeQueryInterface();
    if (m_type == DTMAnalysisType::Fast || m_type == DTMAnalysisType::ViewOnly) //other modes use full resolution
        {
        params->SetLevel(std::min((size_t)5, m_scmPtr->GetTerrainDepth()));
        m_scmPtr->GetCurrentlyViewedNodes(m_nodeSelection);
        }
    else if (m_scmPtr->IsTerrain()) params->SetLevel(m_scmPtr->GetTerrainDepth());
    bvector<IScalableMeshNodePtr> nodes;
    params->SetDirection(newDirection);
    QueryNodesBasedOnParams(nodes, startPt, params, m_scmPtr);
    m_nodeSelection.clear();
    if (m_type == DTMAnalysisType::ViewOnly && nodes.empty()) //not in view, only do a range intersect in this mode
    {
        DRange3d totalBox;
        m_scmPtr->GetRange(totalBox);
        DRay3d ray = DRay3d::FromOriginAndVector(startPt, newDirection);
        DSegment3d segClipped;
        DRange1d fraction;
        if (ray.ClipToRange(totalBox, segClipped, fraction))
        {
            if (fraction.low < 0)
            {
                pointOnDTM = segClipped.point[1];//ray starts within box
                if (fraction.high < 0) //ray completely outside box
                    return false;
            }
            else pointOnDTM = segClipped.point[0];
            m_transform.Multiply(pointOnDTM);
            return true;
        }
        return false;
    }
    else if (nodes.empty()) QueryNodesBasedOnParams(nodes, startPt, params, m_scmPtr);
    bvector<bool> clips;
    bool ret = false;
    for (auto& node : nodes)
        {
        if (!node->ArePoints3d())
            {
            BcDTMPtr dtmP = node->GetBcDTM();
            if (dtmP != nullptr && dtmP->GetDTMDraping()->IntersectRay(pointOnDTM, newDirection, transformedPt))
                {
                m_transform.Multiply(pointOnDTM);
                ret = true;
                break;
                }
            }
        else if (IntersectRay3D(pointOnDTM, newDirection, transformedPt, node))
            {
            m_transform.Multiply(pointOnDTM);
            ret =true;
            break;
            }
        }


        if (ret && !m_regionRestrictions.empty())
        {
            for (auto& region : m_regionRestrictions)
            {

                if ((region->PointInOnOutXY(pointOnDTM) == CurveVector::InOutClassification::INOUT_Out && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer) ||
                    (region->PointInOnOutXY(pointOnDTM) == CurveVector::InOutClassification::INOUT_In && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner))
                {
                    pointOnDTM.z = 0.0;
                    ret = false;
                }
            }
        }
        return ret;
    }

bool ScalableMeshDraping::_ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
    {
    //bvector<bvector<DPoint3d>> coverages;
    IScalableMeshPtr targetedMesh = m_scmPtr;
   // m_scmPtr->GetAllCoverages(coverages);
   // if (!coverages.empty()) targetedMesh = m_scmPtr->GetTerrainSM();
    DPoint3d transformedPt = testPoint;
    m_UorsToStorage.Multiply(transformedPt);
    DPoint3d startPt = transformedPt;
    DPoint3d endPt;
    DPoint3d pt;
    DMatrix4d invW2vMap;


    invW2vMap.QrInverseOf(w2vMap);
    w2vMap.MultiplyAndRenormalize(&pt, &transformedPt, 1);
    pt.z -= 100;
    invW2vMap.MultiplyAndRenormalize(&endPt, &pt, 1);
    if (startPt.DistanceSquaredXY(endPt) < 1e-4)
        {
        double elevation = 0.0;
        
//#ifdef VANCOUVER_API
        int drapedTypeP =0;
        if (DTM_SUCCESS != DrapePoint(&elevation, NULL, NULL, NULL, drapedTypeP, startPt, w2vMap))
//#else
//        if (DTM_SUCCESS != DrapePoint(&elevation, NULL, NULL, NULL, NULL, startPt, w2vMap))
//#endif
            {
            return false;
            }
        pointOnDTM = startPt;
        pointOnDTM.z = elevation;
        m_transform.Multiply(pointOnDTM);
        return true;
        }
    else
        {
        DVec3d vecDirection;
        vecDirection.DifferenceOf(endPt, startPt);
        IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
        IScalableMeshNodeRayQueryPtr query = targetedMesh->GetNodeQueryInterface();
        params->SetLevel(ComputeLevelForTransform(w2vMap));
        bvector<IScalableMeshNodePtr> nodes;
        params->SetDirection(vecDirection);

        targetedMesh->GetCurrentlyViewedNodes(m_nodeSelection);
        QueryNodesBasedOnParams(nodes, startPt, params, targetedMesh);
        m_nodeSelection.clear();
        bvector<bool> clips;
        for (auto& node : nodes)
            {
            BcDTMPtr dtmP = node->GetBcDTM();
            if (dtmP != nullptr && dtmP->GetDTMDraping()->ProjectPoint(pointOnDTM, w2vMap, transformedPt))
                {
                m_transform.Multiply(pointOnDTM);
                return true;
                }
            }
        return false;
        }
    }

void ScalableMeshDraping::QueryNodesBasedOnParams(bvector<IScalableMeshNodePtr>& nodes, const DPoint3d& testPt, const IScalableMeshNodeQueryParamsPtr& params, const IScalableMeshPtr& targetedMeshPtr)
    {
    if (m_nodeSelection.empty())
        {
        IScalableMeshNodeRayQueryPtr query = targetedMeshPtr->GetNodeQueryInterface();
        query->Query(nodes, &testPt, NULL, 0, params);
        }
    else
        {
        DRay3d ray = DRay3d::FromOriginAndVector(testPt, params->GetDirection());

        DRange3d range;
        targetedMeshPtr->GetRange(range);
        ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, DRange3d> query(range,
            targetedMeshPtr->GetTerrainDepth(),
            ray,
            params->Get2d(),
            params->GetDepth(),
            params->GetUseUnboundedRay(),
            ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, DRange3d>::RaycastOptions::ALL_INTERSECT);

        size_t numberOfNodes = 0;
        clock_t startClock = clock();
        for (auto& node : m_nodeSelection)
            {
            numberOfNodes++;
            if (numberOfNodes % 30 == 0 && m_type == DTMAnalysisType::ViewOnly && !nodes.empty())
            {
                double endTime = 1.0*(clock() - startClock) / CLOCKS_PER_SEC;
                if(endTime > 0.0005)
                    return;
            }
            query.SetLevel(node->GetLevel());                                                                 
            node->RunQuery(query, nodes);
            }
        }
    }
static bool s_drapeAlongTurnOff = false;
static bool s_zeroTranslation = true;
static bool s_tryDoublePts = true;


bool ScalableMeshDraping::_DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector)
    {
	if (s_drapeAlongTurnOff)
		return false;

	//bvector<bvector<DPoint3d>> coverages;
	IScalableMeshPtr targetedMesh = m_scmPtr;
	// m_scmPtr->GetAllCoverages(coverages);
	//if (!coverages.empty()) targetedMesh = m_scmPtr->GetTerrainSM();

	if (m_type == DTMAnalysisType::Fast)
	{
		m_levelForDrapeLinear = std::min((size_t)5, targetedMesh->GetTerrainDepth());
		targetedMesh->GetCurrentlyViewedNodes(m_nodeSelection);
	}
	DVec3d vecDirection = DVec3d::FromXYAngleAndMagnitude(directionOfVector, 1);
	vecDirection.z = slopeOfVector;

	IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
	params->SetUseUnboundedRay(false);
	IScalableMeshNodeRayQueryPtr query = targetedMesh->GetNodeQueryInterface();
	params->SetLevel(m_levelForDrapeLinear);

	bvector<IScalableMeshNodePtr> nodes;

	if (m_scmPtr->IsCesium3DTiles())
	{
		if (s_tryDoublePts)
		{
			DPoint3d origin = DPoint3d::From(0, 0, 0);
			DPoint3d endPts = DPoint3d::From(vecDirection.x, vecDirection.y, vecDirection.z);

			Transform dirTransformD(m_UorsToStorage);

			dirTransformD.Multiply(origin);
			dirTransformD.Multiply(endPts);

			DVec3d vecDir2(DVec3d::FromStartEnd(origin, endPts));
			vecDir2.Normalize();
			vecDir2 = vecDir2;
		}

		Transform dirTransform(m_UorsToStorage);

		if (s_zeroTranslation)
		{
			dirTransform.form3d[0][3] = 0;
			dirTransform.form3d[1][3] = 0;
			dirTransform.form3d[2][3] = 0;
		}

		dirTransform.Multiply(vecDirection);
		vecDirection.Normalize();


	}

	params->SetDirection(vecDirection);
	DPoint3d depthVal = DPoint3d::From(1000, 1000, 1000);
	params->SetDepth(depthVal.x);
	DPoint3d transformedPt = point;
	m_UorsToStorage.Multiply(transformedPt);


	QueryNodesBasedOnParams(nodes, transformedPt, params, targetedMesh);
    bvector<bool> clips;
    bool ret = false;
    for (auto& node : nodes)
        {
        if (!node->ArePoints3d() && !m_scmPtr->IsCesium3DTiles())
            {
            BcDTMPtr dtmP = node->GetBcDTM();
            if (dtmP == nullptr) continue;
            assert(dtmP->GetPointCount() < 4 || dtmP->GetTrianglesCount() > 0);
            DRange3d tmBox;
            dtmP->GetRange(tmBox);
            DRay3d ray = DRay3d::FromOriginAndVector(transformedPt, vecDirection);
            DSegment3d seg;
            DRange1d fraction;
            DPoint3d pt = transformedPt;
            if (!tmBox.IsContainedXY(pt) && ray.ClipToRange(tmBox, seg, fraction))
                {
                pt = ray.FractionParameterToPoint(fraction.low);
                }
            
                if (dtmP != nullptr && dtmP->GetDTMDraping()->DrapeAlongVector(endPt, slope, aspect, triangle, drapedType, pt, directionOfVector, slopeOfVector))
                    {
                    if (endPt != nullptr)
                        {
                        //if (node->GetContentExtent().IsContained(*endPt))
                                {
                                m_transform.Multiply(*endPt);
                                ret = true;
                                break;
                                }
                        }
                    }
                
            }
        else
            {
            *drapedType = 0;
            if (IntersectRay3D(*endPt, vecDirection, transformedPt, node))
                {
                *drapedType = 3;
                m_transform.Multiply(*endPt);
                ret = true;
                break;
                }
            }
        }

    if (ret && !m_regionRestrictions.empty())
    {
        for (auto& region : m_regionRestrictions)
        {

            if ((region->PointInOnOutXY(*endPt) == CurveVector::InOutClassification::INOUT_Out && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer) ||
                (region->PointInOnOutXY(*endPt) == CurveVector::InOutClassification::INOUT_In && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner))
            {
                endPt->z = 0.0;
                ret = false;
            }
        }
    }
    return ret;
    }


size_t ScalableMeshDraping::ComputeLevelForTransform(const DMatrix4d& w2vMap)
    {
    if (w2vMap.IsIdentity()) return m_scmPtr->GetTerrainDepth();
    DRange3d range;
    m_scmPtr->GetRange(range);
    Transform w2vTrans;
    w2vTrans.InitFrom(w2vMap);
    w2vTrans.Multiply(range.low);
    w2vTrans.Multiply(range.high);
    DRange3d newRange = DRange3d::From(range.low, range.high);
    double maxLength = std::max(std::max(newRange.XLength(), newRange.YLength()), newRange.ZLength());
    size_t targetLevel = std::min(m_scmPtr->GetTerrainDepth(), (size_t)(ceil(log(maxLength / 20.0) / log(4))));
    return  targetLevel;
    }

//#ifdef VANCOUVER_API
DTMStatusInt ScalableMeshDraping::DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedTypeP, DPoint3dCR point, const DMatrix4d& w2vMap)
//#else
//DTMStatusInt ScalableMeshDraping::DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point, const DMatrix4d& w2vMap)
//#endif
    {
   // bvector<bvector<DPoint3d>> coverages;
    IScalableMeshPtr targetedMesh = m_scmPtr;
   // m_scmPtr->GetAllCoverages(coverages);
   // if (!coverages.empty()) targetedMesh = m_scmPtr->GetTerrainSM();
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    IScalableMeshNodeRayQueryPtr query = targetedMesh->GetNodeQueryInterface();
    params->SetLevel(ComputeLevelForTransform(w2vMap));
    if (m_type == DTMAnalysisType::Fast)
        {
        params->SetLevel(std::min((size_t)5, targetedMesh->GetTerrainDepth()));
        targetedMesh->GetCurrentlyViewedNodes(m_nodeSelection);
        }

    DVec3d direction = DVec3d::From(0, 0, -1);
    bvector<IScalableMeshNodePtr> nodes;
    DVec3d origDirection = direction;

    if (m_scmPtr->IsCesium3DTiles())
    {
        if (s_tryDoublePts)
        {
            DPoint3d origin = DPoint3d::From(0, 0, 0);
            DPoint3d endPts = DPoint3d::From(direction.x, direction.y, direction.z);

            Transform dirTransformD(m_UorsToStorage);

            dirTransformD.Multiply(origin);
            dirTransformD.Multiply(endPts);

            DVec3d vecDir2(DVec3d::FromStartEnd(origin, endPts));
            vecDir2.Normalize();
            vecDir2 = vecDir2;
        }

        Transform dirTransform(m_UorsToStorage);

        if (s_zeroTranslation)
        {
            dirTransform.form3d[0][3] = 0;
            dirTransform.form3d[1][3] = 0;
            dirTransform.form3d[2][3] = 0;
        }

        dirTransform.Multiply(direction);
        direction.Normalize();


    }

    params->SetDirection(direction);
    DPoint3d transformedPt = point;
    m_UorsToStorage.Multiply(transformedPt);
    params->SetUseUnboundedRay(true);
    QueryNodesBasedOnParams(nodes, transformedPt, params, targetedMesh);



    if (nodes.empty())
        {
//#ifdef VANCOUVER_API
        drapedTypeP = 0;
//#else
//       *drapedTypeP = 0;
//#endif
        return DTM_SUCCESS;
        }
    IScalableMeshNodePtr node = nodes.front();
    DTMStatusInt result;
    if (!node->ArePoints3d() && !m_scmPtr->IsCesium3DTiles())
        {
        BcDTMPtr bcdtm = node->GetBcDTM();
        if (bcdtm == nullptr)
            {
//#ifdef VANCOUVER_API
            drapedTypeP = 0;
//#else
//            *drapedTypeP = 0;
//#endif
            return DTM_SUCCESS;
            }
        result = bcdtm->GetDTMDraping()->DrapePoint(elevationP, slopeP, aspectP, triangle, drapedTypeP, transformedPt);
        if (elevationP != nullptr)
            {
            transformedPt.z = *elevationP;
            m_transform.Multiply(transformedPt);
            *elevationP = transformedPt.z;
            }
        }
    else
        {
        DPoint3d pointOnDTM;
        DVec3d otherDirection = direction;
        otherDirection.Negate();
        if (IntersectRay3D(pointOnDTM, direction, transformedPt, node)
            || IntersectRay3D(pointOnDTM, otherDirection, transformedPt, node))
            {
            transformedPt = pointOnDTM;
            m_transform.Multiply(transformedPt);
            *elevationP = transformedPt.z;
            result = DTM_SUCCESS;
            }
        else result = DTM_ERROR;
        }


    if (result == DTM_SUCCESS && !m_regionRestrictions.empty())
    {
        for (auto& region : m_regionRestrictions)
        {

            if ((region->PointInOnOutXY(transformedPt) == CurveVector::InOutClassification::INOUT_Out && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer) ||
                (region->PointInOnOutXY(transformedPt) == CurveVector::InOutClassification::INOUT_In && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner))
            {
                *elevationP = 0.0;
                result = DTM_ERROR;
            }
        }
    }
    return result;
    }

//#ifdef VANCOUVER_API
DTMStatusInt ScalableMeshDraping::_DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedTypeP, DPoint3dCR point)
//#else
//DTMStatusInt ScalableMeshDraping::_DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point)
//#endif
    {
    Transform t = Transform::FromIdentity();
    DMatrix4d identityTrans = DMatrix4d::From(t);
    return DrapePoint(elevationP, slopeP, aspectP, triangle, drapedTypeP, point, identityTrans);
    }

int PickLineSegmentForProjectedPoint(DPoint3dCP line, int nPts, int beginning, DPoint3dCR pt)
    {

    for (int i = beginning; i < nPts - 1; ++i)
        {
        DSegment3d seg = DSegment3d::From(line[i], line[i + 1]);
        double param;
        DPoint3d closestPt;
        seg.ProjectPointXY(closestPt, param, pt);
        if (fabs(closestPt.x - pt.x) < 1e-6 &&fabs(closestPt.y - pt.y) < 1e-6)
            {
            return i;
            }
        }
    return beginning;
    }

double DrapeLine3d(bvector<DPoint3d>& pts, const IScalableMeshNodePtr& node, DPoint3d firstPt, DPoint3d secondPt, Transform myTransform=Transform::FromIdentity(), DVec3d direction = DVec3d::From(0, 0, -1))
    {
    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
    auto meshP = node->GetMesh(flags);
    if (meshP.get() == nullptr) return DBL_MAX;
    bvector<DSegment3d> allSegments;
    if (firstPt.AlmostEqualXY(secondPt))
        {
        DPoint3d pt;
        if (IntersectRay3D(pt, direction, firstPt, const_cast<IScalableMeshNodePtr&>(node)))
            pts.push_back(pt);
        return 0;
        }

	if (!myTransform.IsIdentity())
	{
		meshP->SetTransform(myTransform);
		myTransform.Multiply(firstPt);
		myTransform.Multiply(secondPt);
	}
    DPlane3d planeFromSegment = DPlane3d::From3Points(firstPt, secondPt, DPoint3d::FromSumOf(firstPt, direction));
    meshP->CutWithPlane(allSegments, planeFromSegment);

    bmap<double, DSegment3d> orderedSegments;

    DSegment3d origSeg = DSegment3d::From(firstPt, secondPt);
    double params[2];

    for (auto& seg : allSegments)
        {
        origSeg.PointToFractionParameter(params[0], seg.point[0]);
        origSeg.PointToFractionParameter(params[1], seg.point[1]);
        if (params[0] > params[1])
            {
            DPoint3d tmp;
            double tmpParam;
            tmp = seg.point[0];
            seg.point[0] = seg.point[1];
            seg.point[1] = tmp;

            tmpParam = params[0];
            params[0] = params[1];
            params[1] = tmpParam;
            }
        if (params[0] > -1e-8 && params[1] < 1 + 1e-8)
            {
            orderedSegments.insert(make_bpair(params[0], seg));
            }
        else //need to clip the segment
            {
            if (params[0] <= -1e-8 && params[1] <= -1e-8) continue;
            if (params[0] >= 1 + 1e-8 && params[1] >= 1 + 1e-8) continue;
            if (params[0] <= -1e-8)
                {
                DPoint3d pt, tmpPt;
                double tmpParam;
                DSegment3d projectSegment = DSegment3d::FromOriginAndDirection(firstPt, direction);
                seg.ClosestApproachUnbounded(params[0], tmpParam, pt, tmpPt, seg, projectSegment);
                seg.point[0] = pt;
				origSeg.PointToFractionParameter(params[0], seg.point[0]);
                }
            if (params[1] >= 1 + 1e-8)
                {
                DPoint3d pt, tmpPt;
                double tmpParam;
                DSegment3d projectSegment = DSegment3d::FromOriginAndDirection(secondPt, direction);
                seg.ClosestApproachUnbounded(params[1], tmpParam, pt, tmpPt, seg, projectSegment);
                seg.point[1] = pt;
				origSeg.PointToFractionParameter(params[1], seg.point[1]);
                }
            orderedSegments.insert(make_bpair(params[0], seg));
            }
        }
    for (auto it = orderedSegments.begin(); it != orderedSegments.end(); ++it)
        {
        if (pts.empty() || pts.back().DistanceSquaredXY(it->second.point[0]) > 1e-8)
            pts.push_back(it->second.point[0]);
        if (pts.empty() || pts.back().DistanceSquaredXY(it->second.point[1]) > 1e-8)
            pts.push_back(it->second.point[1]);
        }
    if (orderedSegments.empty()) return DBL_MAX;

	if (!myTransform.IsIdentity())
	    {
		Transform invTransform = myTransform;
		invTransform.InverseOf(myTransform);
		for (auto& pt : pts)
			invTransform.Multiply(pt);
	    }

	//faster computation if direction is z
	if (direction.x == 0 && direction.y == 0)
		return  orderedSegments.begin()->first*firstPt.DistanceXY(secondPt);
	else
	    {
		DVec3d dirX = DVec3d::FromStartEndNormalize(firstPt, secondPt);
		DVec3d dirY = DVec3d::FromCrossProduct(dirX, direction);
		Transform projectToXY = Transform::FromOriginAndVectors(firstPt, dirX, dirY, direction);
		DPoint3d firstPtTrans, secondPtTrans;

		projectToXY.Multiply(firstPtTrans, firstPt);
		projectToXY.Multiply(secondPtTrans, secondPt);

		return orderedSegments.begin()->first*firstPtTrans.DistanceXY(secondPtTrans);
	    }
    }

struct Location
    {
    size_t idxFirst;
    size_t idxLast;
    int segment;
    size_t chunkId;

    Location()
        {}
    Location(size_t first, size_t last, int seg) : idxFirst(first), idxLast(last), segment(seg) {}
    };

//#define DEACTIVATE_PARALLEL_DRAPE

DTMStatusInt ScalableMeshDraping::_DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints)
    {

    IScalableMeshPtr targetedMesh = m_scmPtr;

    if (m_type == DTMAnalysisType::Fast)
        {
        m_levelForDrapeLinear = std::min((size_t)5, targetedMesh->GetTerrainDepth());//params->GetLevel();
        targetedMesh->GetCurrentlyViewedNodes(m_nodeSelection);
        }
    bvector<bvector<DPoint3d>> drapedPointsTemp(numPoints);
    //Trying to find point to start drape

    bvector<DPoint3d> transformedLine(numPoints);
    memcpy(&transformedLine[0], pts, numPoints*sizeof(DPoint3d));

    m_UorsToStorage.Multiply(&transformedLine[0], numPoints);
    MeshTraversalQueue queue(&transformedLine[0], numPoints, m_levelForDrapeLinear, m_scmPtr->IsCesium3DTiles()? m_scmPtr->GetReprojectionTransform() : Transform::FromIdentity());
    queue.UseScalableMesh(targetedMesh.get());
    IScalableMeshMeshPtr meshP = NULL;
    if (!m_nodeSelection.empty()) queue.CollectAll(m_nodeSelection);
    else queue.CollectAll();
    size_t nOfNodesToSplit = queue.GetNumberOfNodes();

#ifdef DEACTIVATE_PARALLEL_DRAPE
    size_t chunkSize = nOfNodesToSplit;
#else
    size_t chunkSize = nOfNodesToSplit < 8 ? 1 : nOfNodesToSplit / 8;
#endif

    bvector<bvector<MeshTraversalStep>> stepData;

#ifndef DEACTIVATE_PARALLEL_DRAPE
    for (size_t i = 0; i < 8; ++i)
#endif
        {
        bvector<MeshTraversalStep> chunks;
        queue.GetNodes(chunks, chunkSize);
        stepData.push_back(chunks);
        }

    if (queue.HasNodesToProcess())
        {
        bvector<MeshTraversalStep> chunks;
        queue.GetNodes(chunks, MeshTraversalQueue::ALL_CHUNKS);
        stepData.push_back(chunks);
        }

    drapedPointsTemp.resize(stepData.size());

    std::vector<bvector<bpair<double, Location>>> initDistances(stepData.size());
    std::vector<std::future<DTMStatusInt>> drapeResults;
    for (auto& chunk : stepData)
        {
        drapeResults.push_back(std::async([] (bvector<DPoint3d>* outPts, const bvector<MeshTraversalStep>& nodesToDrape, bvector<bpair<double, Location>>* initDistanceList, const bvector<DPoint3d>& line, Transform reproTransform, bool isCesium)
            {
            DTMStatusInt retval = DTM_ERROR;
            initDistanceList->reserve(nodesToDrape.size());
            auto lineWithDistances = SMDrapedLine::Create(line, 0);
            for (auto& node : nodesToDrape)
                {
                double initDistance = DBL_MAX;
                if (!node.linkedNode->ArePoints3d() && !isCesium)
                    {
                    BcDTMPtr dtmPtr = node.linkedNode->GetBcDTM();
                    DTMDrapedLinePtr drapeForTile;
                    size_t nAdded = 0;
                    if (dtmPtr != nullptr)
                        {   
                        DTMStatusInt status = dtmPtr->GetDTMDraping()->DrapeLinear(drapeForTile, &line[0] + node.currentSegment, (int)2);
                        for (size_t i = 0; status == DTM_SUCCESS && i < drapeForTile->GetPointCount(); ++i)
                            {
                            retval = DTM_SUCCESS;
                            DTMDrapedLineCode code;
                            DPoint3d pt;
                            double distance;
                            GET_POINT_AT_INDEX(drapeForTile, pt, &distance, &code, (unsigned int)i);
                            if (code == DTMDrapedLineCode::Tin || code == DTMDrapedLineCode::OnPoint || code == DTMDrapedLineCode::Breakline || code == DTMDrapedLineCode::Edge)
                                {
                                if (initDistance == DBL_MAX) initDistance = DVec3d::FromStartEnd(node.startPoint, pt).MagnitudeXY();
                                outPts->push_back(pt);
                                ++nAdded;
                                }
                            if (nAdded != 0 && code == DTMDrapedLineCode::External) break;
                            }

                        double absoluteDist = initDistance;
                        DPoint3d pt;
                        double segmentDist;
                        GET_POINT_AT_INDEX(lineWithDistances, pt, &segmentDist, NULL, (unsigned int)node.currentSegment);
                        absoluteDist += segmentDist;
                        if (nAdded > 0)
                            initDistanceList->push_back(make_bpair(absoluteDist, Location(outPts->size() - nAdded, outPts->size(), node.currentSegment)));
                        }
                    }
                else
                    {
                    bvector<DPoint3d> pts;

					DVec3d direction = DVec3d::From(0, 0, -1);


                    double dist = DrapeLine3d(pts, node.linkedNode, line[node.currentSegment], line[node.currentSegment + 1], reproTransform,direction);
                  
                    if (pts.size() > 0) retval = DTM_SUCCESS;
                    outPts->insert(outPts->end(), pts.begin(), pts.end());

                    double absoluteDist = dist;
                    DPoint3d pt;
                    double segmentDist;
                    GET_POINT_AT_INDEX(lineWithDistances, pt, &segmentDist, NULL, (unsigned int)node.currentSegment);
                    absoluteDist += segmentDist;
                    if (pts.size() > 0)
                        initDistanceList->push_back(make_bpair(absoluteDist, Location(outPts->size() - pts.size(), outPts->size(), node.currentSegment)));
                    }
                }
            return retval;
            }, &drapedPointsTemp[&chunk - &stepData.front()], chunk, &initDistances[&chunk - &stepData.front()], transformedLine, m_scmPtr->IsCesium3DTiles()? m_scmPtr->GetReprojectionTransform(): Transform::FromIdentity(), m_scmPtr->IsCesium3DTiles()));
        }
    bvector<DPoint3d> drapedLine;
    bmap<double, Location> orderedList;

    for (auto& result : drapeResults)
        {
        if (result.get() == DTM_SUCCESS)
            {
            for (auto& dist : initDistances[&result - &drapeResults.front()])
                {
                Location loc = dist.second;
                loc.chunkId = &result - &drapeResults.front();

                orderedList.insert(make_bpair(dist.first, loc));
                }
            }
        }

    for (auto& finalOrder : orderedList)
        {
        for (size_t ptPosition = finalOrder.second.idxFirst; ptPosition < finalOrder.second.idxLast; ++ptPosition) drapedLine.push_back(drapedPointsTemp[finalOrder.second.chunkId][ptPosition]);
        }

    if(drapedLine.size() > 0) m_transform.Multiply(&drapedLine[0],(int) drapedLine.size());



    if (!m_regionRestrictions.empty())
    {
        for (auto& region : m_regionRestrictions)
        {
            for (auto& pt : drapedLine)
            {
                if ((region->PointInOnOutXY(pt) == CurveVector::InOutClassification::INOUT_Out && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer) ||
                    (region->PointInOnOutXY(pt) == CurveVector::InOutClassification::INOUT_In && region->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner))
                {
                    pt.z = 0.0;
                }
            }
        }
    }

	size_t minDrapedPos = 0;
	size_t maxDrapedPos = drapedLine.size() - 1;
	if (!orderedList.empty())
	{
		//check for non-draped segments
		if (orderedList.begin()->second.segment > 0)
			for (size_t i = 0; i < orderedList.begin()->second.segment; ++i)
			{
				drapedLine.insert(drapedLine.begin() + i, pts[i]);
				minDrapedPos = i;
			}

		maxDrapedPos = drapedLine.size() - 1;

		if (orderedList.rbegin()->second.segment+1 <= numPoints - 2)
			for (size_t i = orderedList.rbegin()->second.segment+1; i <= numPoints - 2; ++i)
			{
				drapedLine.insert(drapedLine.end(), pts[i+1]);
			}

		//check for non-draped begin/end within the first/last segment
		int firstSeg = orderedList.begin()->second.segment;
		int lastSeg = orderedList.rbegin()->second.segment;

		DSegment3d first = DSegment3d::From(pts[firstSeg], pts[firstSeg + 1]);
		double param1, param2;
		DPoint3d pt1, pt2;
		DPoint3d retroProjectPt = DPoint3d::FromSumOf(drapedLine[minDrapedPos], DVec3d::From(0, 0, 1));
		DSegment3d intersectFirst = DSegment3d::From(drapedLine[minDrapedPos], retroProjectPt);
		first.ClosestApproachUnbounded(param1, param2, pt1, pt2, first, intersectFirst);
		first.PointToFractionParameter(param1, pt1);
		if (param1 > 1e-6)
		{
			drapedLine.insert(drapedLine.begin() + minDrapedPos, pts[firstSeg]);
			minDrapedPos++;
			maxDrapedPos++;
		}

		DSegment3d last = DSegment3d::From(pts[lastSeg], pts[lastSeg + 1]);
		DPoint3d retroProjectPtLast = DPoint3d::FromSumOf(drapedLine[maxDrapedPos], DVec3d::From(0, 0, 1));
		DSegment3d intersectLast = DSegment3d::From(drapedLine[maxDrapedPos], retroProjectPtLast);
		last.ClosestApproachUnbounded(param1, param2, pt1, pt2, last, intersectLast);
		last.PointToFractionParameter(param1, pt1);
		if (param1 <1-1e-6)
		{
			drapedLine.insert(drapedLine.begin() + maxDrapedPos+1, pts[lastSeg+1]);
		}
	}

    ret = SMDrapedLine::Create(drapedLine, orderedList.empty()? 0 : orderedList.begin()->first, maxDrapedPos, minDrapedPos);
    return DTMStatusInt::DTM_SUCCESS;
    }

ScalableMeshDraping::ScalableMeshDraping(IScalableMeshPtr scMesh) : m_scmPtr(scMesh.get()), m_transform(Transform::FromIdentity()), m_UorsToStorage(Transform::FromIdentity()),
m_levelForDrapeLinear(scMesh->GetTerrainDepth())
    {}

DTMStatusInt Tile3dTM::_DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints)
    {
   bvector<DPoint3d> drapedLine;
    for (size_t i = 0; i < numPoints-1; ++i)
        {
        bvector<DPoint3d> lineTemp;
        DrapeLine3d(lineTemp, m_node, pts[i], pts[i + 1]);
        drapedLine.insert(drapedLine.end(),lineTemp.begin(), lineTemp.end());
        }
    ret = SMDrapedLine::Create(drapedLine, 0);
    return DTM_SUCCESS;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE

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

    protected:
        virtual DTMStatusInt _GetPointByIndex(DTMDrapedLinePointPtr& ret, unsigned int index) const override
            {
            if (index >= m_linePts.size()) return DTMStatusInt::DTM_ERROR;
            ret = SMDrapedPoint::Create(m_linePts[index], DTMDrapedLinePtr((IDTMDrapedLine*)this));
            return DTMStatusInt::DTM_SUCCESS;
            }
        virtual DTMStatusInt _GetPointByIndex(DPoint3dP ptP, double* distanceP, DTMDrapedLineCode* codeP, unsigned int index) const override
            {
            if (index >= m_linePts.size()) return DTMStatusInt::DTM_ERROR;
            *ptP = m_linePts[index];
            if (distanceP != nullptr) *distanceP = m_distancePts[index];
            if (codeP != nullptr) *codeP = DTMDrapedLineCode::Tin;
            return DTMStatusInt::DTM_SUCCESS;
            }
        virtual unsigned int _GetPointCount() const override
            {
            return (unsigned int)m_linePts.size();
            }
        SMDrapedLine(bvector<DPoint3d>& line, double distance)
            {
            m_linePts = line;
            m_distancePts.resize(m_linePts.size());
            double distUntil = distance;
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
        static SMDrapedLine* Create(bvector<DPoint3d>& line, double distance =0.0)
            {
            return new SMDrapedLine(line, distance);
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

void MeshTraversalQueue::CollectAll(const bvector<IScalableMeshNodePtr>& inputNodes)
    {
    for (size_t segment = 0; segment < m_numPointsOnPolyline - 1; segment++)
        {
        DRay3d ray = DRay3d::FromOriginAndVector(m_polylineToDrape[segment], DVec3d::FromStartEndNormalize(m_polylineToDrape[segment], m_polylineToDrape[segment + 1]));

        DRange3d range;
        m_scm->GetRange(range);
        bvector<IScalableMeshNodePtr> outNodes;
        for (auto& node : inputNodes)
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
        IScalableMeshNodeQueryParamsPtr paramsLine = IScalableMeshNodeQueryParams::CreateParams();
        paramsLine->SetLevel(m_levelForDrapeLinear);
        paramsLine->SetDirection(DVec3d::FromStartEndNormalize(m_polylineToDrape[segment], m_polylineToDrape[segment + 1]));
        paramsLine->SetDepth(DVec3d::FromStartEnd(m_polylineToDrape[segment], m_polylineToDrape[segment + 1]).Magnitude());
        paramsLine->Set2d(true);
        bvector<IScalableMeshNodePtr> nodes;

        IScalableMeshNodeRayQueryPtr queryLine = m_scm->GetNodeQueryInterface();
        if (queryLine->Query(nodes, &m_polylineToDrape[segment], NULL, 0, paramsLine) == SUCCESS)
            {
            for (auto& node : nodes)
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
            bvector<bool> clips;
            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            flags->SetLoadGraph(true);
            meshP = node->GetMesh(flags,clips);
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

bool ScalableMeshDraping::_IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint)
    {
    DPoint3d transformedPt = testPoint;
    m_UorsToStorage.Multiply(transformedPt);
    DPoint3d startPt = transformedPt;

    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    IScalableMeshNodeRayQueryPtr query = m_scmPtr->GetNodeQueryInterface();
    params->SetLevel(m_scmPtr->GetTerrainDepth());
    bvector<IScalableMeshNodePtr> nodes;
    params->SetDirection(direction);
    m_scmPtr->GetCurrentlyViewedNodes(m_nodeSelection);
    QueryNodesBasedOnParams(nodes, startPt, params);
    m_nodeSelection.clear();
    bvector<bool> clips;
    for (auto& node : nodes)
        {
        BcDTMPtr dtmP = node->GetBcDTM();
        if (dtmP != nullptr && dtmP->GetDTMDraping()->IntersectRay(pointOnDTM, direction, transformedPt))
            {
            m_transform.Multiply(pointOnDTM);
            return true;
            }
        }
        return false;
    }

bool ScalableMeshDraping::_ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
    {
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
        if (DTM_SUCCESS != DrapePoint(&elevation, NULL, NULL, NULL, NULL, startPt, w2vMap))
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
        IScalableMeshNodeRayQueryPtr query = m_scmPtr->GetNodeQueryInterface();
        params->SetLevel(ComputeLevelForTransform(w2vMap));
        bvector<IScalableMeshNodePtr> nodes;
        params->SetDirection(vecDirection);

        m_scmPtr->GetCurrentlyViewedNodes(m_nodeSelection);
        QueryNodesBasedOnParams(nodes, startPt, params);
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

void ScalableMeshDraping::QueryNodesBasedOnParams(bvector<IScalableMeshNodePtr>& nodes, const DPoint3d& testPt, const IScalableMeshNodeQueryParamsPtr& params)
    {
    if (m_nodeSelection.empty())
        {
        IScalableMeshNodeRayQueryPtr query = m_scmPtr->GetNodeQueryInterface();
        query->Query(nodes, &testPt, NULL, 0, params);
        }
    else
        {
        DRay3d ray = DRay3d::FromOriginAndVector(testPt, params->GetDirection());

        DRange3d range;
        m_scmPtr->GetRange(range);
        for (auto& node : m_nodeSelection)
            {
            ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, DRange3d> query(range,
                                                                                   node->GetLevel(),
                                                                                   ray,
                                                                                   params->Get2d(),
                                                                                   params->GetDepth(),
                                                                                   params->GetUseUnboundedRay(),
                                                                                   ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, DRange3d>::RaycastOptions::ALL_INTERSECT);
            node->RunQuery(query, nodes);
            }
        }
    }

bool ScalableMeshDraping::_DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector)
    {
    if (m_type == DTMAnalysisType::Fast)
        {
        m_levelForDrapeLinear = std::min((size_t)5, m_scmPtr->GetTerrainDepth());
        m_scmPtr->GetCurrentlyViewedNodes(m_nodeSelection);
        }
    DVec3d vecDirection = DVec3d::FromXYAngleAndMagnitude(directionOfVector, 1);
    vecDirection.z = slopeOfVector;
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    params->SetUseUnboundedRay(false);
    IScalableMeshNodeRayQueryPtr query = m_scmPtr->GetNodeQueryInterface();
    params->SetLevel(m_levelForDrapeLinear);

    bvector<IScalableMeshNodePtr> nodes;
    params->SetDirection(vecDirection);
    DPoint3d depthVal = DPoint3d::From(1000, 1000, 1000);
    m_UorsToStorage.Multiply(depthVal);
    params->SetDepth(depthVal.x);
    DPoint3d transformedPt = point;
    m_UorsToStorage.Multiply(transformedPt);

    QueryNodesBasedOnParams(nodes, transformedPt, params);
    bvector<bool> clips;
    bool ret = false;
    for (auto& node : nodes)
        {
        BcDTMPtr dtmP = node->GetBcDTM();
        assert(dtmP->GetPointCount() < 4 || dtmP->GetTrianglesCount() > 0);
        if (dtmP != nullptr && dtmP->GetDTMDraping()->DrapeAlongVector(endPt, slope, aspect, triangle, drapedType, transformedPt, directionOfVector, slopeOfVector))
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

DTMStatusInt ScalableMeshDraping::DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point, const DMatrix4d& w2vMap)
    {
    IScalableMeshNodeQueryParamsPtr params = IScalableMeshNodeQueryParams::CreateParams();
    IScalableMeshNodeRayQueryPtr query = m_scmPtr->GetNodeQueryInterface();
    params->SetLevel(ComputeLevelForTransform(w2vMap));

    IScalableMeshNodePtr node;
    DPoint3d transformedPt = point;
    m_UorsToStorage.Multiply(transformedPt);
    if (query->Query(node, &transformedPt, NULL, 0, params) != SUCCESS)
        {
        if (drapedTypeP != nullptr) *drapedTypeP = 0;
        return DTM_SUCCESS;
        }
    BcDTMPtr bcdtm = node->GetBcDTM();
    if (bcdtm == nullptr)
        {
        if (drapedTypeP != nullptr) *drapedTypeP = 0;
        return DTM_SUCCESS;
        }
    DTMStatusInt result = bcdtm->GetDTMDraping()->DrapePoint(elevationP, slopeP, aspectP, triangle, drapedTypeP, transformedPt);
    if (elevationP != nullptr)
        {
        transformedPt.z = *elevationP;
        m_transform.Multiply(transformedPt);
        *elevationP = transformedPt.z;
        }
    return result;
    }

DTMStatusInt ScalableMeshDraping::_DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int* drapedTypeP, DPoint3dCR point)
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

DTMStatusInt ScalableMeshDraping::_DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints)
    {
    if (m_type == DTMAnalysisType::Fast)
        {
        m_levelForDrapeLinear = std::min((size_t)5, m_scmPtr->GetTerrainDepth());//params->GetLevel();
        m_scmPtr->GetCurrentlyViewedNodes(m_nodeSelection);
        }
    bvector<bvector<DPoint3d>> drapedPointsTemp(numPoints);
    //Trying to find point to start drape

    bvector<DPoint3d> transformedLine(numPoints);
    memcpy(&transformedLine[0], pts, numPoints*sizeof(DPoint3d));
    m_UorsToStorage.Multiply(&transformedLine[0], numPoints);
    MeshTraversalQueue queue(&transformedLine[0], numPoints, m_levelForDrapeLinear);
    queue.UseScalableMesh(m_scmPtr);
    IScalableMeshMeshPtr meshP = NULL;
    if (!m_nodeSelection.empty()) queue.CollectAll(m_nodeSelection);
    else queue.CollectAll();
    size_t nOfNodesToSplit = queue.GetNumberOfNodes();
    size_t chunkSize = nOfNodesToSplit < 8 ? 1 : nOfNodesToSplit / 8;
    bvector<bvector<MeshTraversalStep>> stepData;
    for (size_t i = 0; i < 8; ++i)
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
    std::vector<double> initDistances(stepData.size());
    std::vector<std::future<DTMStatusInt>> drapeResults;
    for (auto& chunk : stepData)
        {
        drapeResults.push_back(std::async([] (bvector<DPoint3d>* outPts, const bvector<MeshTraversalStep>& nodesToDrape, double* initDistance, const bvector<DPoint3d>& line)
            {
            DTMStatusInt retval = DTM_ERROR;
            *initDistance = DBL_MAX;
            for (auto& node : nodesToDrape)
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
                        drapeForTile->GetPointByIndex(&pt, &distance, &code, (unsigned int)i);
                        if (code == DTMDrapedLineCode::Tin || code == DTMDrapedLineCode::OnPoint || code == DTMDrapedLineCode::Breakline || code == DTMDrapedLineCode::Edge)
                            {
                            if (*initDistance == DBL_MAX) *initDistance = distance;
                            outPts->push_back(pt);
                            ++nAdded;
                            }
                        if (nAdded != 0 && code == DTMDrapedLineCode::External) break;
                        }
                    }
                }
            return retval;
            }, &drapedPointsTemp[&chunk - &stepData.front()], chunk, &initDistances[&chunk - &stepData.front()], transformedLine));
        }
    bvector<DPoint3d> drapedLine;
    for (auto& result : drapeResults)
        {
        if(result.get() == DTM_SUCCESS)
            for (auto pt : drapedPointsTemp[&result-&drapeResults.front()]) drapedLine.push_back(pt);
        }
    if(drapedLine.size() > 0) m_transform.Multiply(&drapedLine[0],(int) drapedLine.size());
    ret = SMDrapedLine::Create(drapedLine, initDistances[0]);
    return DTMStatusInt::DTM_SUCCESS;
#if 0
    if (!queue.TryStartTraversal(findTriangleAlongRay, 0))
        {
        bvector<DPoint3d> line(numPoints);
        memcpy(&line[0], pts, numPoints*sizeof(DPoint3d));
        ret = SMDrapedLine::Create(line);
        return DTMStatusInt::DTM_SUCCESS;
        }
    while (queue.HasNodesToProcess())
        {
        MeshTraversalStep& startNode = queue.Step();
        DRay3d ray = DRay3d::FromOriginAndVector(startNode.startPoint, DVec3d::From(0,0,-1)); //what would we do if we need to use different draping directions?
        bool withinMesh = false;
        int triangle[3] = { -1, -1, -1 };
        //MTGNodeId triangleEdge = -1;
        DSegment3d segClipped;
        DRange1d fraction;
        if (startNode.linkedNode->GetPointCount() > 4 && !startNode.fetchNeighborsOnly)
            {
            DRay3d toTileInterior = DRay3d::FromOriginAndVector(startNode.startPoint, DVec3d::FromStartEnd(pts[startNode.currentSegment], pts[startNode.currentSegment + 1]));
            //NEEDSWORK_SM: would be better to just use a 2d clipping function here (if 2.5d) or project the ray and extent along drape direction (if 3d)
            toTileInterior.direction.z = 0; //regardless of how the lines were drawn, we are assuming they're projected straight-on. Not sure what's to be done for 3D.
            toTileInterior.origin.z = startNode.linkedNode->GetContentExtent().low.z;
            if (!findTriangleAlongRay && !startNode.linkedNode->GetContentExtent().IsContained(startNode.startPoint, 2)) findTriangleAlongRay = true;
            if (!findTriangleAlongRay && !ray.ClipToRange(startNode.linkedNode->GetContentExtent(), segClipped, fraction)){
                while(!queue.HasNodesToProcess() && startNode.currentSegment < numPoints-2) queue.TryStartTraversal(findTriangleAlongRay,++startNode.currentSegment); //try to find nodes until we have no more line segments left.
                continue;
            }
            else if (findTriangleAlongRay && !toTileInterior.ClipToRange(startNode.linkedNode->GetContentExtent(), segClipped, fraction))
                {
                //it is possible that the ray intersects the right tile, but there is no mesh over the intersection. If that is the case, 
                //then we need to find and add the neighbors to the list. If it is not, we can skip over this tile.
                toTileInterior.origin.z = startNode.linkedNode->GetNodeExtent().low.z;
                if (!toTileInterior.ClipToRange(startNode.linkedNode->GetNodeExtent(), segClipped, fraction))
                {
                while(!queue.HasNodesToProcess() && startNode.currentSegment < numPoints - 2) queue.TryStartTraversal(findTriangleAlongRay, ++startNode.currentSegment);
                    continue;
                }
            }
            else
                {
                if (!s_civilDraping)
                    {
                    bvector<bool> clips;
                    IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
                    flags->SetLoadGraph(true);
                    meshP = startNode.linkedNode->GetMesh(flags, clips);
                    if (!findTriangleAlongRay) withinMesh = meshP->FindTriangleForProjectedPoint(triangle, startNode.startPoint, !startNode.linkedNode->ArePoints3d());
                    if (findTriangleAlongRay || !withinMesh)
                        {
                        withinMesh = meshP->FindTriangleAlongRay(triangle, toTileInterior);
                        }
                    }
                }
            
        }
        //bool addedNeighbors = false;
        //no intersection found with triangle mesh; look down
        //This algorithm looks first (if 3D) for neighbors above or below the current node. (If 2d, the octree region is flattened, so that all data should be in the bottom-most node,
        //so that there is no need to look up or down). Specifically, we first look below the node (general case would be in the direction of projection), then above (same direction opposite
        // orientation). When doing that we preserve a reference to the "original" node that prompted the search so that it is the basis for the next pass.
        // When all elevations are unsuccessfully exhausted, look if there's a reference, and push it with a "last pass" marker.On those references, neighbors in drape direction should
        // be pursued. This means we will skip all empty nodes in drape direction, if applicable, and find the next node that has a mesh to drape on.
       /* if (!withinMesh)
            {
            if (startNode.linkedNode->ArePoints3d() && !startNode.fetchNeighborsOnly)
                {
                addedNeighbors = queue.NextAlongElevation();
                }               
            if (addedNeighbors) continue;
            }*/
  
        //no intersection found with triangle mesh; add relevant neighbors
        if (!s_civilDraping &&!withinMesh)
            {
            //also add neighbors in the direction of the line
            //if (!addedNeighbors) queue.NextAlongDirection();
            queue.CollectIntersects(findTriangleAlongRay);
            while(!queue.HasNodesToProcess() && startNode.currentSegment < numPoints-2) queue.TryStartTraversal(findTriangleAlongRay,++startNode.currentSegment);
            continue;
            }
        queue.Clear();
        findTriangleAlongRay = false;
        DPoint3d endPoint;
        MTGNodeId edge = -1;// = triangle;
        //begin greedy drape
        if (!s_civilDraping)
            {
            if (ERROR == meshP->ProjectPolyLineOnMesh(endPoint, drapedPointsTemp, &transformedLine[0], (int)numPoints, &startNode.currentSegment, triangle, startNode.startPoint, edge))
                {
                //can't find triangle
                DRay3d toTileInterior = DRay3d::FromOriginAndVector(startNode.startPoint, DVec3d::FromStartEnd(pts[startNode.currentSegment], pts[startNode.currentSegment + 1]));
                if (meshP->FindTriangleAlongRay(triangle, toTileInterior))
                    {
                    if (ERROR == meshP->ProjectPolyLineOnMesh(endPoint, drapedPointsTemp, &transformedLine[0], (int)numPoints, &startNode.currentSegment, triangle, startNode.startPoint, edge))
                        {
                        return DTMStatusInt::DTM_ERROR;
                        }
                    }
                else
                    {
                    endPoint = startNode.startPoint; //possible hole, try next node
                    }
                }
            }
        else
            {
            BcDTMPtr dtmPtr = startNode.linkedNode->GetBcDTM();
            DTMDrapedLinePtr drapeForTile;
            size_t nAdded = 0;
            if (dtmPtr != nullptr)
                {
                DTMStatusInt status = dtmPtr->GetDTMDraping()->DrapeLinear(drapeForTile, &transformedLine[0] + startNode.currentSegment, numPoints - startNode.currentSegment);
                for (size_t i = 0; status == DTM_SUCCESS && i < drapeForTile->GetPointCount(); ++i)
                    {
                    DTMDrapedLineCode code;
                    DPoint3d pt;
                    drapeForTile->GetPointByIndex(&pt, NULL, &code, (unsigned int)i);
                    if (code == DTMDrapedLineCode::Tin || code == DTMDrapedLineCode::OnPoint || code == DTMDrapedLineCode::Breakline || code == DTMDrapedLineCode::Edge)
                        {
                        startNode.currentSegment = PickLineSegmentForProjectedPoint(&transformedLine[0], numPoints, startNode.currentSegment, pt);
                        drapedPointsTemp[startNode.currentSegment].push_back(pt);
                        ++nAdded;
                        }
                    if (nAdded != 0 && code == DTMDrapedLineCode::External) break;
                    }
                }
            if(nAdded > 0) endPoint = drapedPointsTemp[startNode.currentSegment].back();
            else
                {
                queue.CollectIntersects(findTriangleAlongRay);
                while (!queue.HasNodesToProcess() && startNode.currentSegment < numPoints - 2) queue.TryStartTraversal(findTriangleAlongRay, ++startNode.currentSegment);
                continue;
                }
            }
        if (startNode.currentSegment < numPoints - 1 || pow(DRay3d::FromOriginAndVector(transformedLine[numPoints - 1], DVec3d::From(0, 0, -1)).DirectionDotVectorToTarget(endPoint), 2) != (DVec3d::FromStartEnd(transformedLine[numPoints - 1], endPoint)).MagnitudeSquared())
            {
            DRange3d ext = startNode.linkedNode->GetNodeExtent();
            //drape did not reach end of node, but line still has segment. Try to find current segment in neighbor instead
            if ((!startNode.hasEndPoint && ext.IsContained(endPoint, 2)) || startNode.hasEndPoint && !bsiDPoint3d_pointEqualTolerance(&endPoint, &startNode.endPointOfLastProjection, 0.000001))
                {
                if (startNode.currentSegment >= numPoints - 1) break;
                findTriangleAlongRay = true;
                }
            queue.SetStartPoint(endPoint);
            queue.SetLastEdge(edge);
            //bool addedNodes = false;
            if (findTriangleAlongRay && startNode.linkedNode->ArePoints3d()) //again for 3d we need to check the top and bottom neighbors
                {
                //addedNodes = queue.NextAlongElevation();
                queue.CollectIntersects(findTriangleAlongRay);
                }
            else//if (!addedNodes)
                {
                queue.NextAlongDirection();
                }
            }
        while(!queue.HasNodesToProcess() && startNode.currentSegment < numPoints-2) queue.TryStartTraversal(findTriangleAlongRay,++startNode.currentSegment);
        }

#if 0
    DrapeUsingPlaneIntersect(pts, numPoints, drapedPointsTemp, m_scmPtr);
        vector<DPoint3d> drapedLine;
       for (size_t i = 0; i < drapedPointsTemp.size();i++)
        for (auto pt : drapedPointsTemp[i]) drapedLine.push_back(pt);
       if(drapedLine.size() > 0)SortPointsOnLine(drapedLine);
#endif
       bvector<DPoint3d> drapedLine;
       for (size_t i = 0; i < drapedPointsTemp.size(); i++)
           for (auto pt : drapedPointsTemp[i]) drapedLine.push_back(pt);
       m_transform.Multiply(&drapedLine[0],(int) drapedLine.size());
       ret = SMDrapedLine::Create(drapedLine);
        return DTMStatusInt::DTM_SUCCESS;
#endif
    }

ScalableMeshDraping::ScalableMeshDraping(IScalableMeshPtr scMesh) : m_scmPtr(scMesh.get()), m_transform(Transform::FromIdentity()), m_UorsToStorage(Transform::FromIdentity()),
m_levelForDrapeLinear(scMesh->GetTerrainDepth())
    {}

END_BENTLEY_SCALABLEMESH_NAMESPACE

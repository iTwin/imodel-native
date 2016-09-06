/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/DrapeOnGraph.cpp $
|    $RCSfile: DrapeOnGraph.cpp,v $
|   $Revision: 1.0 $
|       $Date: 2015/08/07 12:30:58 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "DrapeOnGraph.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
const double ScalableMeshGraphDraping::POINT_TOLERANCE = 1.0e-8; //in meters

bool ScalableMeshGraphDraping::FollowPolylineOnGraph(bvector<bvector<DPoint3d>>& projectedPoints, DPoint3d& endPt, MTGNodeId& triangleStartEdge, int* segment, DPoint3d startPt)
    {
   // bool stopProgression = false;
    MTGNodeId currentTriangle = triangleStartEdge;
    int triangleVertexIds[3];
    DPoint3d triangleVertices[3];
    if (!GetTriangle(triangleVertices, triangleVertexIds, currentTriangle)) return false;
    DVec3d directionOfFirstSegment = DVec3d::FromStartEndNormalize(m_linePoints[*segment], m_linePoints[*segment + 1]);
    DPoint3d currentVertex = ProjectPointOnTriangle(startPt, triangleVertices, directionOfFirstSegment);
    if (*segment >= m_nPointsInLine - 1) return true;
    projectedPoints[*segment].push_back(currentVertex);
    m_lastVtx1 = -1;
    m_lastVtx2 = -1;
   /* DVec3d currentLineSegmentDirection = DVec3d::FromStartEnd(m_linePoints[*segment], m_linePoints[*segment + 1]);
    currentLineSegmentDirection.Normalize();
    DRay3d ray = DRay3d::FromOriginAndVector(m_linePoints[*segment + 1], m_drapeDirection);*/

    ScalableMeshGraphDrapingProcess process(projectedPoints, segment, currentVertex, triangleStartEdge, m_graphP, m_linePoints, m_drapeDirection, m_nPointsInLine);
    while (process.CanContinue())
        {
        if (!GetTriangle(triangleVertices, triangleVertexIds, process.GetCurrentTriangleEdge())) assert(false && "WRONG TRIANGLE IDS");
        DRay3d toNextPoint = GetDirectionInTrianglePlane(process.GetCurrentVertex(), process.GetCurrentLineSegmentDirection(), triangleVertices);
        bool intersectFound = false;
        DPoint3d intersectPt = { 0, 0, 0 };
        double param, lastParam =-1;
        DPoint3d lastPt;
        int intersectedEdge = -1;
        if (!FindIntersectionOfRayAndTriangleEdges(intersectedEdge, intersectPt, param, lastPt, lastParam, toNextPoint, triangleVertices, triangleVertexIds))
            {
            if (!FindIntersectionOfRayAndTriangleVertices(intersectedEdge, intersectPt, param, toNextPoint, triangleVertices, triangleVertexIds))
                for (size_t i = 0; i < 3 && !intersectFound; i++)
                    {
                    DSegment3d triEdge = DSegment3d::From(triangleVertices[i], triangleVertices[(i + 1) % 3]);
                    DRay3d edgeRay = DRay3d::From(triEdge);
                    double param2;
                    DPoint3d intersectPt2;
                    if (bsiDRay3d_closestApproach(&param, &param2, &intersectPt2, &intersectPt, &toNextPoint, &edgeRay))
                        {
                        if ((m_lastVtx1 == triangleVertexIds[i] || m_lastVtx1 == triangleVertexIds[(i + 1) % 3]) && (m_lastVtx2 == triangleVertexIds[i] || m_lastVtx2 == triangleVertexIds[(i + 1) % 3])) continue;
                        if (param2 < 0 || param2 > 1 || param < -POINT_TOLERANCE) continue;
                        intersectFound = true;
                        intersectedEdge = (int)i;
                        }
                    }
            else intersectFound = true;
            }
        else intersectFound = true;
        //assert(intersectFound);
//        DRay3d toEdgeRay;
        if (!intersectFound)
            {
            if (m_3d) //we need to at least try look in the tiles above and below
                {
                triangleStartEdge = process.GetCurrentTriangleEdge();
                process.SavePoint();
                break;
                }
            m_lastVtx1 = -1;
            m_lastVtx2 = -1;
            int inter = 0;
            inter = process.FindNextTriangleEdgeInDirection(m_vertices, m_nVertices);
            if (process.CanContinue())
                {
                process.SavePoint();
                continue;
                }
            else continue;
            }
        if (fabs(param) > 0.000001)
            {
            DPoint3d projectedEnd;
            if (process.EndOfSegmentReached(projectedEnd, intersectPt))
                {
                m_lastVtx1 = -1;
                m_lastVtx2 = -1;
                process.NextSegment();
                continue;
                }
            else if (lastParam != param && lastParam != -1 && !process.LastSegment()) process.SavePoint(lastPt);
            }
        else if (process.SegmentEndsOnCurrentVertex())
            {
                m_lastVtx1 = -1;
                m_lastVtx2 = -1;
                process.NextSegment();
                continue;
            }
        //next triangle
        m_lastVtx1 = triangleVertexIds[(intersectedEdge) % 3];
        m_lastVtx2 = triangleVertexIds[(intersectedEdge + 1) % 3];

        process.NextTriangle(intersectedEdge);
        process.SetCurrentVertex(intersectPt);
        process.SavePoint();
        if (!GetTriangle(triangleVertices, triangleVertexIds, process.GetCurrentTriangleEdge())) assert(false && "WRONG TRIANGLE IDS");
        if (IsExteriorEdge(process.GetCurrentTriangleEdge()) ||//reached mesh exterior -- is it a hole?
            ((m_lastVtx1 != triangleVertexIds[0] && m_lastVtx1 != triangleVertexIds[1] && m_lastVtx1 != triangleVertexIds[2]) && (m_lastVtx2 != triangleVertexIds[0] && m_lastVtx2 != triangleVertexIds[1] && m_lastVtx2 != triangleVertexIds[2]))) //this should not happen, temp fix
            {
            if (m_3d) //we need to at least try look in the tiles above and below
                {
                triangleStartEdge = process.GetCurrentTriangleEdge();
                break;
                }
            m_lastVtx1 = -1;
            m_lastVtx2 = -1;
            if (process.FindNextTriangleEdgeInDirection(m_vertices, m_nVertices) != 1)
                {
                m_graphP->TryGetLabel(process.GetCurrentTriangleEdge(), 0, m_lastVtx1);
                m_graphP->TryGetLabel(m_graphP->FSucc(process.GetCurrentTriangleEdge()), 0, m_lastVtx2);
                }
            if (process.GetCurrentTriangleEdge() != -1 && !GetTriangle(triangleVertices, triangleVertexIds, process.GetCurrentTriangleEdge())) assert(false && "WRONG TRIANGLE IDS");
            if (process.GetCurrentTriangleEdge() == -1) break;
            process.SavePoint();
            }
        }
    endPt = process.GetCurrentVertex();
    return true;
    }


bool ScalableMeshGraphDraping::GetTriangle(DPoint3d* vertices, int* indices, const MTGNodeId& firstEdge)
    {
    if (!m_graphP->TryGetLabel(firstEdge, 0, indices[0]) ||
        !m_graphP->TryGetLabel(m_graphP->FSucc(firstEdge), 0, indices[1]) ||
        !m_graphP->TryGetLabel(m_graphP->FSucc(m_graphP->FSucc(firstEdge)), 0, indices[2])) return false;
    for (size_t i = 0; i < 3; ++i) vertices[i] = m_vertices[indices[i] - 1];
    return true;
    }

DPoint3d ScalableMeshGraphDraping::ProjectPointOnTrianglePlane(const DPoint3d& pt, const DPoint3d* triangle, DPoint3d* barycentric)
    {
    DRay3d ray = DRay3d::FromOriginAndVector(pt, m_drapeDirection);
    DPoint3d projectedPt;
    bsiDRay3d_intersectTriangle(&ray, &projectedPt, barycentric, NULL, triangle);
    return projectedPt;
    }


DPoint3d ScalableMeshGraphDraping::ProjectPointOnTriangle(const DPoint3d& pt, const DPoint3d* triangle, const DVec3d& direction)
    {
    DPoint3d bary;
    DPoint3d pointInTrianglePlane = ProjectPointOnTrianglePlane(pt, triangle, &bary);
    bool isPointWithinTriangle = bary.x >= -1.0e-6f
        && bary.x <= 1.0&& bary.y >= -1.0e-6f && bary.y <= 1.0 && bary.z >= -1.0e-6f && bary.z <= 1.0;
    if (isPointWithinTriangle) return pointInTrianglePlane;
    else //we need to project the point onto the closest triangle edge
        {
        DRay3d toTriangle = GetDirectionInTrianglePlane(pointInTrianglePlane, direction, triangle);
        DPoint3d intersectPt = { 0, 0, 0 };
        double param, lastParam = -1;
        DPoint3d lastPt;
        int intersectedEdge = -1;
        int ids[3] = { -1, -2, -3 };
        if (!FindIntersectionOfRayAndTriangleEdges(intersectedEdge, intersectPt, param, lastPt, lastParam, toTriangle, triangle, ids))
            {
            bool result = FindIntersectionOfRayAndTriangleVertices(intersectedEdge, intersectPt, param, toTriangle, triangle, ids);
            if(!result) assert(false && "Wrong initial triangle");
            }
        return intersectPt;
        }
    }

DRay3d ScalableMeshGraphDraping::GetDirectionInTrianglePlane(const DPoint3d& startPoint, const DVec3d& direction, const DPoint3d* triangle)
    {
    DPoint3d pt2;
    pt2.SumOf(startPoint, direction);
    DRay3d toTriPlane = DRay3d::FromOriginAndVector(pt2, m_drapeDirection);
    bsiDRay3d_intersectTriangle(&toTriPlane, &pt2, NULL, NULL, triangle);
    DVec3d projDirection = DVec3d::FromStartEnd(startPoint, pt2);
   return DRay3d::FromOriginAndVector(startPoint, projDirection);
    }

bool ScalableMeshGraphDraping::FindIntersectionOfRayAndTriangleEdges(int& intersectedEdge, DPoint3d& intersectPt, double& param, DPoint3d& lastPt, double& lastParam, const DRay3d& toNextPoint, const DPoint3d* triangle, const int* indices,IntersectionType intersectType)
    {
    bool intersectFound = false;
    lastParam = -1;
    size_t i = 0;
    for (; i < 3 && !intersectFound; i++)
        {
        DSegment3d triEdge = DSegment3d::From(triangle[i], triangle[(i + 1) % 3]);
        DRay3d edgeRay = DRay3d::From(triEdge);
        double param2;
        DPoint3d intersectPt2;
        if (bsiDRay3d_closestApproach(&param, &param2, &intersectPt2, &intersectPt, &toNextPoint, &edgeRay))
            {
            if ((m_lastVtx1 == indices[i] || m_lastVtx1 == indices[(i + 1) % 3]) && (m_lastVtx2 == indices[i] || m_lastVtx2 == indices[(i + 1) % 3])) continue;
            if (param2 < 0 || param2 > 1 || param < POINT_TOLERANCE) continue;
            if (m_lastVtx1 != -1 && m_lastVtx2 != -1) intersectFound = true;
            else if (lastParam == -1)
                {
                lastPt = intersectPt;
                lastParam = param;
                intersectedEdge = (int)i;
                }
            else if ((intersectType == INTERSECTION_LAST && lastParam > param) || (intersectType == INTERSECTION_FIRST && lastParam < param))
                {
                intersectPt = lastPt;
                param = lastParam;
                intersectFound = true;
                i = intersectedEdge;
                }
            else intersectFound = true;
            }
        }
    if (intersectFound) intersectedEdge = (int)i-1;
    if (!intersectFound && lastParam != -1)
        {
        intersectFound = true;
        param = lastParam;
        intersectPt = lastPt;
        }
    return intersectFound;
    }

bool ScalableMeshGraphDraping::FindIntersectionOfRayAndTriangleVertices(int& intersectedEdge, DPoint3d& intersectPt, double& param, const DRay3d& toNextPoint, const DPoint3d* triangle, const int* indices, IntersectionType intersectType)
    {
    bool intersectFound = false;
    for (size_t i = 0; i < 3; ++i)
        {
        if (bsiDPoint3d_pointEqualTolerance(&toNextPoint.origin, &triangle[i], POINT_TOLERANCE))
            {
            intersectFound = true;
            //compare the two edges to pick the one that is in direction of ray
            DPoint3d pt, pt2;
            double paramA, paramB;
            toNextPoint.ProjectPointUnbounded(pt, paramA, triangle[(i + 1) % 3]);
            toNextPoint.ProjectPointUnbounded(pt2, paramB, triangle[(i + 2) % 3]);
            intersectPt = toNextPoint.origin;
            param = 0;
            if (m_lastVtx1 == indices[(i + 2) % 3] || m_lastVtx2 == indices[(i + 2) % 3] || (m_lastVtx1 != indices[(i + 1) % 3] && m_lastVtx2 != indices[(i + 1) % 3] && (intersectType == INTERSECTION_LAST && paramA > paramB) || (intersectType == INTERSECTION_FIRST && paramA < paramB)))
                {
                intersectedEdge = (int)i;
                if (bsiDPoint3d_pointEqualTolerance(&pt, &triangle[(i + 1) % 3], POINT_TOLERANCE)) //second point also on ray
                    if (paramA > 0) intersectPt = pt;
                }
            else
                {
                intersectedEdge = (int)(i + 2) % 3;
                if (bsiDPoint3d_pointEqualTolerance(&pt2, &triangle[(i + 2) % 3], POINT_TOLERANCE)) //second point also on ray
                    if (paramB > 0) intersectPt = pt2;
                }
            break;
            }
        }
    return intersectFound;
    }


bool ScalableMeshGraphDrapingProcess::EndOfSegmentReached(DPoint3d& projectedEnd, const DPoint3d& intersectPt)
    {
    DSegment3d toTriEdge = DSegment3d::From(m_currentVertex, intersectPt);
    DRay3d toEdgeRay = DRay3d::From(toTriEdge);
    double param;
    DPoint3d projectedEnd2;
    bool endOfSegment = (bsiDRay3d_closestApproach(&param, NULL, &projectedEnd, &projectedEnd2, &toEdgeRay, &m_endOfCurrentSegment) && param >= ScalableMeshGraphDraping::POINT_TOLERANCE && param <= 1 + ScalableMeshGraphDraping::POINT_TOLERANCE);
    if (endOfSegment) SetCurrentVertex(projectedEnd);
    return endOfSegment;
    }

void ScalableMeshGraphDrapingProcess::NextSegment()
    {
    m_outputPoints[*m_currentSegment].push_back(m_currentVertex);
    ++(*m_currentSegment);
    if (*m_currentSegment == m_nPointsInLine - 1)
        {
        m_stopProgression = true; //reached the end of the line
        }
    else
        {
        m_directionOfCurrentSegment = DVec3d::FromStartEnd(m_line[*m_currentSegment], m_line[*m_currentSegment + 1]);
        m_directionOfCurrentSegment.Normalize();
        m_endOfCurrentSegment = DRay3d::FromOriginAndVector(m_line[*m_currentSegment + 1], DVec3d::From(0, 0, -1));
        }
    }

void ScalableMeshGraphDrapingProcess::NextTriangle(int edgeToCross)
    {
    if (edgeToCross == 0) m_currentTriangleEdge = m_graphP->EdgeMate(m_currentTriangleEdge);
    else if (edgeToCross == 1) m_currentTriangleEdge = m_graphP->EdgeMate(m_graphP->FSucc(m_currentTriangleEdge));
    else m_currentTriangleEdge = m_graphP->EdgeMate(m_graphP->FSucc(m_graphP->FSucc(m_currentTriangleEdge)));
    }

void ScalableMeshGraphDrapingProcess::SavePoint()
    {
    m_outputPoints[*m_currentSegment].push_back(m_currentVertex);
    }

void ScalableMeshGraphDrapingProcess::SavePoint(DPoint3d& point)
    {
    m_outputPoints[*m_currentSegment].push_back(point);
    }

bool ScalableMeshGraphDrapingProcess::SegmentEndsOnCurrentVertex()
    {
    DPoint3d projectedEnd;
    double param;
    return (m_endOfCurrentSegment.ProjectPointUnbounded(projectedEnd, param, m_currentVertex) && bsiDPoint3d_pointEqualTolerance(&projectedEnd, &m_currentVertex, 10 * ScalableMeshGraphDraping::POINT_TOLERANCE));
    }

int ScalableMeshGraphDrapingProcess::FindNextTriangleEdgeInDirection(const DPoint3d* vertices, size_t nVertices)
    {
    DRay3d toEdgeRay = DRay3d::FromOriginAndVector(m_currentVertex, m_directionOfCurrentSegment);
    int lastSegment = *m_currentSegment;
    int inter = 0;
    DPoint3d intersectPt;
    if (!FindNextTriangleOnRay(m_currentTriangleEdge, intersectPt, m_currentSegment, m_graphP, toEdgeRay, m_line, vertices, m_outputPoints, &inter, nVertices, false))
        {
        m_stopProgression = true;
        }
    else if (*m_currentSegment != lastSegment && *m_currentSegment < m_nPointsInLine - 1)
        {
        m_directionOfCurrentSegment = DVec3d::FromStartEnd(m_line[*m_currentSegment], m_line[*m_currentSegment + 1]);
        m_directionOfCurrentSegment.Normalize();
        m_endOfCurrentSegment = DRay3d::FromOriginAndVector(m_line[*m_currentSegment + 1], DVec3d::From(0, 0, -1));
        toEdgeRay.origin = intersectPt;
        toEdgeRay.direction = m_directionOfCurrentSegment;
        m_currentTriangleEdge = -1;
        if (!FindNextTriangleOnRay(m_currentTriangleEdge, intersectPt, m_currentSegment, m_graphP, toEdgeRay, m_line, vertices, m_outputPoints, &inter, nVertices, false))
            {
            m_stopProgression = true;
            }
        else m_currentVertex = intersectPt;
        }
    else m_currentVertex = intersectPt;
    if (*m_currentSegment == m_nPointsInLine - 1)
        {
        m_stopProgression = true;
        m_currentTriangleEdge = -1;
        }
    return inter;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE

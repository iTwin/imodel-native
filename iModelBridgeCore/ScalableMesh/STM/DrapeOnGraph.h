/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/DrapeOnGraph.h $
|    $RCSfile: DrapeOnGraph.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/08/07 12:30:58 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include "ScalableMesh\ScalableMeshGraph.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class ScalableMeshGraphDrapingProcess
    {
    private:
        bvector<bvector<DPoint3d>>& m_outputPoints;
        int* m_currentSegment;
        DPoint3d m_currentVertex;
        DPoint3d m_lastVertex;

        MTGNodeId m_currentTriangleEdge;
        bool m_stopProgression = false;

        DRay3d m_endOfCurrentSegment;
        DVec3d m_directionOfCurrentSegment;

        MTGGraph* m_graphP;
        const DPoint3d* m_line;
        const DVec3d m_drapeDirection;
        size_t m_nPointsInLine;

    public:
        ScalableMeshGraphDrapingProcess(bvector<bvector<DPoint3d>>& outputPoints, int* segment, const DPoint3d& startVertex, const MTGNodeId& startTriangle, MTGGraph* graphP, const DPoint3d* line, DVec3d direction, size_t nPts)
            : m_outputPoints(outputPoints), m_currentSegment(segment), m_currentVertex(startVertex), m_currentTriangleEdge(startTriangle), m_graphP(graphP), m_line(line), m_drapeDirection(direction), m_nPointsInLine(nPts)
            {
            m_directionOfCurrentSegment = DVec3d::FromStartEnd(m_line[*m_currentSegment], m_line[*m_currentSegment + 1]);
            m_directionOfCurrentSegment.Normalize();
            m_endOfCurrentSegment = DRay3d::FromOriginAndVector(m_line[*m_currentSegment + 1], DVec3d::From(0, 0, -1));
            }

        bool CanContinue() { return !m_stopProgression && *m_currentSegment < m_nPointsInLine -1; }
        void NextSegment();
        void NextTriangle(int edgeToCross);
        void SavePoint();
        void SavePoint(DPoint3d& point);
        bool EndOfSegmentReached(DPoint3d& projectedEnd, const DPoint3d& intersectPt);
        bool SegmentEndsOnCurrentVertex();
        bool LastSegment() { return *m_currentSegment == m_nPointsInLine - 1; }

        int FindNextTriangleEdgeInDirection(const DPoint3d* vertices, size_t nVertices);
        void SetCurrentVertex(DPoint3d& pt) { m_lastVertex = m_currentVertex;  m_currentVertex = pt; }

        DVec3d GetCurrentLineSegmentDirection() { return m_directionOfCurrentSegment; }
        DPoint3d GetCurrentVertex()  { return m_currentVertex; }
        MTGNodeId GetCurrentTriangleEdge() { return m_currentTriangleEdge; }
    };

class ScalableMeshGraphDraping
    {
    private:

        MTGGraph* m_graphP;
        const DPoint3d* m_linePoints;
        size_t m_nPointsInLine;
        const DPoint3d* m_vertices;
        size_t m_nVertices;
        bool m_3d; //whether there is any 3d data (if not, several assumptions can be made)
        DVec3d m_drapeDirection;


        int m_lastVtx1, m_lastVtx2; //vertices making up last edge crossed

        enum IntersectionType
            {
            INTERSECTION_LAST = 0,
            INTERSECTION_FIRST
            };

        bool GetTriangle(DPoint3d* vertices, int* indices, const MTGNodeId& firstEdge);
        DPoint3d ProjectPointOnTrianglePlane(const DPoint3d& pt, const DPoint3d* triangle, DPoint3d* barycentric = nullptr);
        DPoint3d ProjectPointOnTriangle(const DPoint3d& pt, const DPoint3d* triangle, const DVec3d& direction);
        DRay3d GetDirectionInTrianglePlane(const DPoint3d& startPoint, const DVec3d& direction, const DPoint3d* triangle);
        bool IsExteriorEdge(MTGNodeId edgeId) { return m_graphP->GetMaskAt(edgeId, MTG_EXTERIOR_MASK) || FastCountNodesAroundFace(m_graphP, edgeId) != 3; };

        bool FindIntersectionOfRayAndTriangleEdges(int& intersectedEdge, DPoint3d& intersectPt, double& param, DPoint3d& lastPt, double& lastParam, const DRay3d& toNextPoint, const DPoint3d* triangle, const int* indices, IntersectionType intersectType = INTERSECTION_LAST);
        bool FindIntersectionOfRayAndTriangleVertices(int& intersectedEdge, DPoint3d& intersectPt, double& param, const DRay3d& toNextPoint, const DPoint3d* triangle, const int* indices, IntersectionType intersectType = INTERSECTION_LAST);


    public:
        static const double POINT_TOLERANCE;
        ScalableMeshGraphDraping(MTGGraph* graph, bool use3d = false) : m_graphP(graph), m_3d(use3d), m_drapeDirection(DVec3d::From(0, 0, -1)) {};
        void SetPolyLine(const DPoint3d* linePts, size_t nPts)
            {
            m_linePoints = linePts;
            m_nPointsInLine = nPts;
            };
            
        void SetVertices(const DPoint3d* vertices, size_t nVertices)
            {
            m_vertices = vertices;
            m_nVertices = nVertices;
            };

        bool FollowPolylineOnGraph(bvector<bvector<DPoint3d>>& projectedPoints, DPoint3d& endPt, MTGNodeId& triangleStartEdge, int* segment, DPoint3d startPt);
       
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE

#pragma once
#include <Mtg/MtgStructs.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

bool IsOutsideEdge(MTGGraph* graphP, MTGNodeId id);

void CreateGraphFromIndexBuffer(MTGGraph* graph, const long* buffer, int count, int pointCount, bvector<int>& componentContours, const DPoint3d* points);

void MergeGraphs(MTGGraph * destGraphP, std::vector<DPoint3d>& destPoints, MTGGraph * srcGraphP, std::vector<DPoint3d>& inPoints, DPoint3d minCorner, DPoint3d maxCorner, std::vector<int>& pointToDestPointsMap, bvector<int>& componentContours);

void ExtractFaceIndexListFromGraph(std::vector<int>& faceIndexBuffer, MTGGraph* graphP);

void AddFacesToGraph(MTGGraph* destGraphP, std::vector<int>& faces, std::vector<std::vector<std::pair<int, MTGNodeId>>*>& existingEdges, std::vector<int>& indexMapping, std::vector<DPoint3d>& pointsInGraph, bvector<int>& componentContours);

MTGNodeId FindFaceInGraph(MTGGraph* graphP, int vertex1, int vertex2, int vertex3);

bool FindNextTriangleOnRay(MTGNodeId& edge, DPoint3d& lastVertex, int* segment, MTGGraph* graphP, DRay3d toEdgeRay, const DPoint3d* linePts, const DPoint3d* points, bvector<bvector<DPoint3d>>& projectedPoints, int* nIntersect, const size_t nPoints, bool is3d = false);

bool FollowPolylineOnGraph(MTGGraph* graphP, DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, MTGNodeId& triangleStartEdge, int* segment, const DPoint3d* linePoints, int nLinePts, DPoint3d startPt, const size_t nPoints, bool is3d = false);

void GetGraphExteriorBoundary(MTGGraph* graphP, bvector<bvector<DPoint3d>>& bound, const DPoint3d* points);

void RemovePolygonInteriorFromGraph(MTGGraph* graphP, bvector<DPoint3d>& polygon, const DPoint3d* points, DPoint3d& minCorner, DPoint3d& maxCorner);

void RemoveTrianglesWithinExtent(MTGGraph* graphP, const DPoint3d* points, DPoint3d& minCorner, DPoint3d& maxCorner);

void GetFaceDefinition(MTGGraph* graphP, int* outTriangle, MTGNodeId& edge);

END_BENTLEY_SCALABLEMESH_NAMESPACE
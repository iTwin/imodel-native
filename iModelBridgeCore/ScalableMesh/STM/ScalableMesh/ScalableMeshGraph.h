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

void GetGraphExteriorBoundary(MTGGraph* graphP, bvector<bvector<DPoint3d>>& bound, const DPoint3d* points, bool pruneTriangleInnerLoops = false);

void RemovePolygonInteriorFromGraph(MTGGraph* graphP, bvector<DPoint3d>& polygon, const DPoint3d* points, DPoint3d& minCorner, DPoint3d& maxCorner);

void RemoveTrianglesWithinExtent(MTGGraph* graphP, const DPoint3d* points, DPoint3d& minCorner, DPoint3d& maxCorner);

void GetFaceDefinition(MTGGraph* graphP, int* outTriangle, MTGNodeId& edge);

void ExtractMeshIndicesFromGraph(std::vector<int32_t>& indices, MTGGraph* graphP);

size_t FastCountNodesAroundFace(MTGGraph* graphP, MTGNodeId id);

void PrintGraph(Utf8String path, Utf8String name, MTGGraph* graphP);

size_t CountExteriorFaces(MTGGraph* graphP);

void UntieLoopsFromPolygon(bvector<DPoint3d>& polygon);

bool RemoveKnotsFromGraph(MTGGraph* graphP, std::vector<DPoint3d>& ptsToModify);

void PrintGraphWithPointInfo(Utf8String path, Utf8String name, MTGGraph* graphP, const DPoint3d* pts, const size_t npts);

void ResolveUnmergedBoundaries(MTGGraph * graphP);

void RecomputeExterior(MTGGraph * graphP);

void RemoveOverhangsFromPolygon(bvector<DPoint3d>& polygon);

bool HasOverlapWithNeighborsXY(MTGGraph* graphP, MTGNodeId boundaryId, const DPoint3d* pts);

void TagFeatureEdges(MTGGraph* graphP, DTMFeatureType tagValue, size_t featureSize, const int32_t* featureData, bool tagEnds = true);

void ReadFeatureTags(MTGGraph * graphP, std::vector<int>& pointToDestPointsMap, bvector<bvector<int32_t>>& features, std::map<int,int>& componentForPoints);

void ReadFeatureTags(MTGGraph * graphP, std::vector<int>& pointToDestPointsMap, bvector<bvector<int32_t>>& features);

struct TaggedEdge
    {
    int tag;
    int vtx1;
    int vtx2;
    };

void ReadFeatureEndTags(MTGGraph * graphP, std::vector<int>& pointToDestPointsMap, bvector<TaggedEdge>& featureEdges);

END_BENTLEY_SCALABLEMESH_NAMESPACE
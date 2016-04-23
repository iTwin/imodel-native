/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Edits/ClipUtilities.h $
|    $RCSfile: ClipUtilities.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/08 15:28:13 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <ScalableMesh/IScalableMesh.h>
#include "DifferenceSet.h"
#include <TerrainModel\TerrainModel.h>
#include <array>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
class ScalableMeshMesh;
struct DifferenceSetWithTracking : public DifferenceSet
    {
    bvector<int32_t> splitFaces;
    public:
        void ApplySetMerge(DifferenceSetWithTracking& d, int firstIndx, const DPoint3d* vertices);
        void ResolveConflicts(bvector<bvector<int32_t>>& mergedFaces, const DPoint3d* triangle, bvector<int32_t>& conflictFaces, const DPoint3d* vertices);
    };

void PrintDiff(std::string& s, DifferenceSet& d, const DPoint3d* oldVert);
bool GridBasedIntersect(const bvector<DPoint3d>& pts, const DRange3d& ptsBox, const DRange3d& box);

class FaceToUVMap
    {
    bmap<int32_t, bmap<int32_t, std::array<bpair<int32_t, std::array<DPoint2d, 3>>,2>>> map;
    public:
        DRange3d range;
        FaceToUVMap(DRange3d r=DRange3d()): range(r) {};
        void ReadFrom(const int32_t* indices, const int32_t* uvIndices, const DPoint2d* uvCoords, size_t nIndices);
        void AddFacetOnEdge(const int32_t* indices, const DPoint2d* uvCoords, const int32_t* edge);
        void RemoveFacetFromEdge(const int32_t* indices, const int32_t* edge);
        void AddFacet(const int32_t* indices, const DPoint2d* uvCoords);
        void RemoveFacet(const int32_t* indices);
        bool GetFacet(const int32_t* indices, DPoint2d* uvCoords);
        bool SplitFacet(DPoint2d& newUv, const int32_t newPt, const int32_t* indices, std::function<DPoint2d(std::array<DPoint2d, 3>, const int32_t*, int32_t)> interpolateCallback);
        bool SplitEdge(DPoint2d* newUvs, const int32_t newPt, const int32_t* indices, const int32_t* pts, std::function<DPoint2d(std::array<DPoint2d, 3>, const int32_t*, int32_t)> interpolateCallback);
        bool SplitFacetOrEdge(DPoint2d& newUv, const int32_t newPt, const int32_t* indices, std::function<DPoint2d(std::array<DPoint2d, 3>, const int32_t*, int32_t)> interpolateCallback, bool onEdge);
    };

class Clipper
    {
    private:
    const DPoint3d* m_vertexBuffer;
    const size_t m_nVertices;
    const int32_t* m_indexBuffer;
    const size_t m_nIndices;
    const DPoint2d* m_uvBuffer;
    const int32_t* m_uvIndices;
    DRange3d m_range;

    enum PointClassification
        {
        UNKNOWN =0,
        OUTSIDE,
        INSIDE,
        ON,
        };
    DifferenceSet TriangulateAroundHole(const DPoint3d* triangle, const vector<vector<int32_t>>& lines, const vector<DPoint3d>& pts);
    DifferenceSet Triangulate(const vector<DPoint3d>& pts, const vector<vector<int32_t>>& lines, const DPoint3d* triangle, const bool* triangleMask, const vector<vector<int32_t>>& triangleIntersects);
    void ClipVertices(DifferenceSet& d, bvector<bool>& removed, const DPoint3d* poly, size_t polySize);
    void ClassifyPointForTriangle(PointClassification* classif, const DPoint3d* pt, const DPoint3d* triangle);
    DifferenceSet ClipTriangle(const DPoint3d* triangle, const bool* triangleMask, const DPoint3d* poly, size_t polySize, DRange3d polyExt);
    DifferenceSetWithTracking ClipConvexPolygon2DCustom(const DPoint3d* poly, size_t polySize);
    DifferenceSetWithTracking ClipPolygon2DDTM(const DPoint3d* poly, size_t polySize);
    void TagUVsOnPolyface(PolyfaceHeaderPtr& poly, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr, FaceToUVMap& faceToUVMap);
    DTMInsertPointCallback GetInsertPointCallback(FaceToUVMap& faceToUVMap, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& ptr);

    public:
        Clipper(const DPoint3d* vertexBuffer, size_t nVertices, const int32_t* indexBuffer, size_t nIndices, DRange3d extent, const DPoint2d* uvBuffer = nullptr, const int32_t* uvIndices = nullptr) :
            m_vertexBuffer(vertexBuffer), m_nVertices(nVertices), m_indexBuffer(indexBuffer), m_nIndices(nIndices), m_range(extent), m_uvBuffer(uvBuffer), m_uvIndices(uvIndices)
            {}

        DifferenceSet ClipNonConvexPolygon2D(const DPoint3d* poly, size_t polySize);
        DifferenceSetWithTracking ClipConvexPolygon2D(const DPoint3d* poly, size_t polySize);
        DifferenceSet ClipSeveralPolygons(bvector<bvector<DPoint3d>>& polygons);
        void MakeDTMFromIndexList(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtm);
        bool GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons);
        bool GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons, bvector<bpair<double, int>>& metadata, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr);


    };
template<class POINT, class EXTENT> void ClipMeshToNodeRange(vector<int>& faceIndexes, vector<POINT>& nodePts, bvector<DPoint3d>& pts, EXTENT& contentExtent, DRange3d& nodeRange, ScalableMeshMesh* meshP);
void print_polygonarray(std::string& s, const char* tag, DPoint3d* polyArray, int polySize);


//void BuildSkirtMeshesForPolygonSet(bvector<bvector<PolyfaceHeaderPtr>>& skirts, bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons, DRange3d& nodeRange);
END_BENTLEY_SCALABLEMESH_NAMESPACE
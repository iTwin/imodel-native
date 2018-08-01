/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Edits/ClipUtilities.h $
|    $RCSfile: ClipUtilities.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/08 15:28:13 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <ScalableMesh/IScalableMesh.h>
#include "DifferenceSet.h"
#include <TerrainModel/TerrainModel.h>
#include <array>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
class ScalableMeshMesh;


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
    DRange3d m_nodeRange;
	size_t m_widthOfTexData;
	size_t m_heightOfTexData;

    enum PointClassification
        {
        UNKNOWN =0,
        OUTSIDE,
        INSIDE,
        ON,
        };


    void TagUVsOnPolyface(PolyfaceHeaderPtr& poly, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr, FaceToUVMap& faceToUVMap, bmap<int32_t, int32_t>& mapOfIndices);
    DTMInsertPointCallback GetInsertPointCallback(FaceToUVMap& faceToUVMap, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& ptr);

    public:
        Clipper(const DPoint3d* vertexBuffer, size_t nVertices, const int32_t* indexBuffer, size_t nIndices, DRange3d extent, DRange3d nodeRange, const DPoint2d* uvBuffer = nullptr, const int32_t* uvIndices = nullptr) :
            m_vertexBuffer(vertexBuffer), m_nVertices(nVertices), m_indexBuffer(indexBuffer), m_nIndices(nIndices), m_range(extent), m_nodeRange(nodeRange), m_uvBuffer(uvBuffer), m_uvIndices(uvIndices), m_widthOfTexData(1024), m_heightOfTexData(1024)
            {}

		void SetTextureDimensions(size_t width, size_t height)
		{
			m_widthOfTexData = width;
			m_heightOfTexData = height;
		}

        void MakeDTMFromIndexList(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtm);
        bool GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons);
        bool GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons, bvector<bpair<double, int>>& metadata, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr);


    };


template<class POINT, class EXTENT> void ClipMeshToNodeRange(std::vector<int>& faceIndexes, std::vector<POINT>& nodePts, bvector<DPoint3d>& pts, EXTENT& contentExtent, DRange3d& nodeRange, ScalableMeshMesh* meshP);
template<class POINT, class EXTENT> void ClipMeshToNodeRange(std::vector<int>& faceIndexes, std::vector<POINT>& nodePts, bvector<DPoint3d>& pts, bvector<DPoint2d>& uvs, EXTENT& contentExtent, DRange3d& nodeRange, ScalableMeshMesh* meshP);

void print_polygonarray(std::string& s, const char* tag, DPoint3d* polyArray, int polySize);

BENTLEY_SM_EXPORT bool GetRegionsFromClipPolys3D(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons, const PolyfaceQuery* meshP);
BENTLEY_SM_EXPORT bool GetRegionsFromClipVector3D(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<size_t>& polyfaceIndices, ClipVectorCP clip, const PolyfaceQuery* meshP, const bvector<bool>& isMask);
//void BuildSkirtMeshesForPolygonSet(bvector<bvector<PolyfaceHeaderPtr>>& skirts, bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons, DRange3d& nodeRange);


class MeshClipper
{
public:
    enum class RegionResult
    {
        Success,
        ClippingNotComputed,
        NoData,
        NoIntersectionWithClips,
        NoSelection,
        InsufficientRegionFilterArguments,
        NoRegionsMatchingSelection,
        MoreThanOneRegionMatchingSelection
    };

    enum class RegionFilter
    {
        Boundary,
        Mask,
        BoundaryOrExterior,
        MaskOrExterior,
        Exterior,
        All
    };

    enum class RegionFilterMode
    {
        IncludeSelected,
        ExcludeSelected,
        SelectedIDs,
        ExcludeSelectedIDs
    };

private:

    //Source data
    const DPoint3d* m_vertexBuffer;
    size_t m_nVertices;
    const int32_t* m_indexBuffer;
    size_t m_nIndices;
    const DPoint2d* m_uvBuffer;
    const int32_t* m_uvIndices;
    DRange3d m_range;
    DRange3d m_nodeRange;
    size_t m_widthOfTexData;
    size_t m_heightOfTexData;
    bool m_is25dData;
    PolyfaceQuery const* m_sourceData;
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmData;

    //Clipper object geometries
    struct ClipInfo
    {
        enum struct Type
        {
            Vector,
            Polygon
        };
        Type type;
        uint64_t id;
        virtual bool isClipMask() {
            return true;
        }
    };

    struct ClipVectorInfo :ClipInfo
    {
        ClipVectorCP clip = nullptr;
        bvector<bool> arePrimitivesMasks;
        DRange3d clipExt;
        ClipVectorInfo() { type = Type::Vector; }
        virtual bool isClipMask() {
            bool isMask = true;
            for (auto& mask : arePrimitivesMasks)
                if (!mask) isMask = false;
            return isMask;
        }
    };

    struct ClipPolyInfo : ClipInfo
    {
        bvector<DPoint3d> pts;
        bool isMask;
        ClipPolyInfo() { type = Type::Polygon; }
        virtual bool isClipMask() { return isMask; }
    };

    bvector<ClipVectorPtr> vectorDefs;
    bvector<ClipVectorInfo> allVectors;
    bvector<ClipPolyInfo> allPolys;
    bvector<ClipInfo*> orderedClipList;

    //results of computation
    bool wasClipped;
    struct ClippedRegion
    {
        uint64_t id;
        bvector<PolyfaceHeaderPtr> meshes;
        bool isExterior;
    };

    bvector<ClippedRegion> computedRegions;
    bvector<ClippedRegion*> selectedRegions;
    RegionResult selectionError;
    

    //private functions
    enum PointClassification
    {
        UNKNOWN = 0,
        OUTSIDE,
        INSIDE,
        ON,
    };
    void TagUVsOnPolyface(PolyfaceHeaderPtr& poly, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr, FaceToUVMap& faceToUVMap, bmap<int32_t, int32_t>& mapOfIndices);
    DTMInsertPointCallback GetInsertPointCallback(FaceToUVMap& faceToUVMap, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& ptr);

    void MakeDTMFromIndexList(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtm);
    bool GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons);
    bool GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr);

    void OrderClipGeometryList();

    bool HasOnlyPolygons();
    void GetClipsAsPolygons(bvector<bvector<DPoint3d>>& outPolygons);
    void GetClipsAsVectors(bvector<ClipVectorPtr>& outVectors);
    void GetClipsAsSingleVector(ClipVectorPtr& outVector);

    RegionResult GetInOrOutRegion(PolyfaceHeaderPtr& mesh, const bool getInside);

    ClipInfo* FindMatchingClip(uint64_t id);
public:

    BENTLEY_SM_EXPORT MeshClipper();
    BENTLEY_SM_EXPORT void SetSourceMesh(const PolyfaceQuery* meshSourceData, bool is25dData = false);
    void SetTextureDimensions(size_t width, size_t height)
    {
        m_widthOfTexData = width;
        m_heightOfTexData = height;
    }

    BENTLEY_SM_EXPORT void SetClipGeometry(const bvector<uint64_t>& ids, const bvector<bvector<DPoint3d>>& polygons);
    BENTLEY_SM_EXPORT void SetClipGeometry(const bvector<uint64_t>& ids, const bvector<bvector<DPoint3d>>& polygons, const bvector<bool>& polygonIsMask);
    BENTLEY_SM_EXPORT void SetClipGeometry(const bmap<size_t, uint64_t>& idsForPrimitives, ClipVectorCP clip);
    BENTLEY_SM_EXPORT void SetClipGeometry(const bmap<size_t, uint64_t>& idsForPrimitives, ClipVectorCP clip, const bmap<size_t, bool>& isMaskForEachPrimitive);
    BENTLEY_SM_EXPORT void SetClipGeometry(uint64_t id, ClipVectorCP clip, bool isMask = true);
    BENTLEY_SM_EXPORT void ClearClipGeometry();

    BENTLEY_SM_EXPORT void SetMaskInfo(uint64_t id, bool isMask);
    BENTLEY_SM_EXPORT bool IsClipMask(uint64_t id);

    BENTLEY_SM_EXPORT void SetMeshExtents(DRange3d extentOfData, DRange3d extentOfTexture);

    BENTLEY_SM_EXPORT void ComputeClip();



    BENTLEY_SM_EXPORT RegionResult GetRegions(bvector<uint64_t>& ids, bvector<bvector<PolyfaceHeaderPtr>>& polyfaces);
    BENTLEY_SM_EXPORT RegionResult GetExteriorRegion(PolyfaceHeaderPtr& mesh);
    BENTLEY_SM_EXPORT RegionResult GetInteriorRegion(PolyfaceHeaderPtr& mesh);
    BENTLEY_SM_EXPORT bool WasClipped();

    BENTLEY_SM_EXPORT void SelectRegions(RegionFilter filter, RegionFilterMode mode);
    BENTLEY_SM_EXPORT void SelectRegions(RegionFilter filter, RegionFilterMode mode, bvector<uint64_t>& idsFilter);
    BENTLEY_SM_EXPORT RegionResult GetSelectedRegion(PolyfaceHeaderPtr& mesh);
    BENTLEY_SM_EXPORT  bvector<bpair<uint64_t, PolyfaceHeaderPtr>>&& GetSelectedRegions(RegionResult& result);
    BENTLEY_SM_EXPORT void ClearSelection();

};

END_BENTLEY_SCALABLEMESH_NAMESPACE
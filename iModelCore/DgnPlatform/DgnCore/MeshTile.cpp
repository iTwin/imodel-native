/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MeshTile.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <Geom/XYZRangeTree.h>
#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

#if defined (BENTLEYCONFIG_OPENCASCADE)
#include <DgnPlatform/DgnBRep/OCBRep.h>
#endif

#if defined(BENTLEYCONFIG_OS_WINDOWS)
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_RENDER

BEGIN_UNNAMED_NAMESPACE
    static TileGenerator::IProgressMeter s_defaultProgressMeter;
END_UNNAMED_NAMESPACE

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct RangeTreeNode
{
    TileGeometryPtr m_geometry;

    RangeTreeNode(IGeometryR geom, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR opts, bool isCurved, DgnDbR db)
        : m_geometry(TileGeometry::Create(geom, tf, range, elemId, params, opts, isCurved, db)) { }
    RangeTreeNode(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR opts, DgnDbR db)
        : m_geometry(TileGeometry::Create(solid, tf, range, elemId, params, opts, db)) { }
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct FreeLeafDataTreeHandler : XYZRangeTreeHandler
{
    virtual bool ShouldContinueAfterLeaf (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override
        {
        delete reinterpret_cast <RangeTreeNode*> (pLeaf->GetData());
        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryCache::TileGeometryCache(TransformCR tfFromDgn) : m_tree(XYZRangeTreeRoot::Allocate()), m_transformFromDgn(tfFromDgn), m_nextTextureId(0)
    {
    m_transformToDgn.InverseOf(m_transformFromDgn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryCache::~TileGeometryCache()
    {
    FreeLeafDataTreeHandler handler;
    m_tree->Traverse(handler);

    XYZRangeTreeRoot::Free(m_tree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileGeometryCache::GetRange() const
    {
    return m_tree->Range();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureCPtr TileDisplayParams::QueryTexture(DgnDbR db) const
    {
    JsonRenderMaterial mat;
    if (!m_materialId.IsValid() || SUCCESS != mat.Load(m_materialId, db))
        return nullptr;

    auto texMap = mat.GetPatternMap();
    DgnTextureId texId;
    if (!texMap.IsValid() || !(texId = texMap.GetTextureId()).IsValid())
        return nullptr;

    return DgnTexture::QueryTexture(texId, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Image TileGeometryCache::TextureImage::Load(TileDisplayParamsCR params, DgnDbR db)
    {
    DgnTextureCPtr tex = params.QueryTexture(db);
    return tex.IsValid() ? Image(tex->GetImageSource()) : Image();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryCache::ResolveTexture(TileDisplayParamsCR params, DgnDbR db)
    {
    if (m_textures.end() != m_textures.find(params))
        return;

    TextureImagePtr image;
    Image renderImage = TextureImage::Load(params, db);
    if (renderImage.IsValid())
        image = new TextureImage(std::move(renderImage), m_nextTextureId++);

    m_textures.Insert(params, image);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryCache::TextureImage const* TileGeometryCache::GetTextureImage(TileDisplayParamsCR params) const
    {
    auto found = m_textures.find(params);
    BeAssert(m_textures.end() != found);

    return m_textures.end() != found ? found->second.get() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileMesh::GetTriangleRange(TriangleCR triangle) const
    {
    return DRange3d::From (m_points.at (triangle.m_indices[0]), 
                           m_points.at (triangle.m_indices[1]),
                           m_points.at (triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d TileMesh::GetTriangleNormal(TriangleCR triangle) const
    {
    return DVec3d::FromNormalizedCrossProductToPoints (m_points.at (triangle.m_indices[0]), 
                                                       m_points.at (triangle.m_indices[1]),
                                                       m_points.at (triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMesh::HasNonPlanarNormals() const
    {
    if (m_normals.empty())
        return false;

    for (auto& triangle : m_triangles)
        if (!m_normals.at (triangle.m_indices[0]).IsEqual (m_normals.at (triangle.m_indices[1])) ||
            !m_normals.at (triangle.m_indices[0]).IsEqual (m_normals.at (triangle.m_indices[2])) ||
            !m_normals.at (triangle.m_indices[1]).IsEqual (m_normals.at (triangle.m_indices[2])))
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId elemId)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(point);
    m_elementIds.push_back(elemId);

    if (nullptr != normal)
        m_normals.push_back(*normal);

    if (nullptr != param)
        m_uvParams.push_back(*param);

    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::VertexKey::Comparator::operator()(VertexKey const& lhs, VertexKey const& rhs) const
    {
    static const double s_normalTolerance = 1.0E-6;
    static const double s_paramTolerance  = 1.0E-6;

    COMPARE_VALUES (lhs.m_elementId, rhs.m_elementId);

    COMPARE_VALUES_TOLERANCE (lhs.m_point.x, rhs.m_point.x, m_tolerance);
    COMPARE_VALUES_TOLERANCE (lhs.m_point.y, rhs.m_point.y, m_tolerance);
    COMPARE_VALUES_TOLERANCE (lhs.m_point.z, rhs.m_point.z, m_tolerance);

    if (lhs.m_normalValid != rhs.m_normalValid)
        return rhs.m_normalValid;

    if (lhs.m_normalValid)
        {
        COMPARE_VALUES_TOLERANCE (lhs.m_normal.x, rhs.m_normal.x, s_normalTolerance);
        COMPARE_VALUES_TOLERANCE (lhs.m_normal.y, rhs.m_normal.y, s_normalTolerance);
        COMPARE_VALUES_TOLERANCE (lhs.m_normal.z, rhs.m_normal.z, s_normalTolerance);
        }

    if (lhs.m_paramValid != rhs.m_paramValid)
        return rhs.m_paramValid;

    if (lhs.m_paramValid)
        {
        COMPARE_VALUES_TOLERANCE (lhs.m_param.x, rhs.m_param.x, s_paramTolerance);
        COMPARE_VALUES_TOLERANCE (lhs.m_param.y, rhs.m_param.y, s_paramTolerance);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshBuilder::TriangleKey::TriangleKey(TriangleCR triangle)
    {
    // Could just use std::sort - but this should be faster?
    if (triangle.m_indices[0] < triangle.m_indices[1])
        {
        if (triangle.m_indices[0] < triangle.m_indices[2])
            {
            m_sortedIndices[0] = triangle.m_indices[0];
            if (triangle.m_indices[1] < triangle.m_indices[2])
                {
                m_sortedIndices[1] = triangle.m_indices[1];
                m_sortedIndices[2] = triangle.m_indices[2];
                }
            else
                {
                m_sortedIndices[1] = triangle.m_indices[2];
                m_sortedIndices[2] = triangle.m_indices[1];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle.m_indices[2];
            m_sortedIndices[1] = triangle.m_indices[0];
            m_sortedIndices[2] = triangle.m_indices[1];
            }
        }
    else
        {
        if (triangle.m_indices[1] < triangle.m_indices[2])
            {
            m_sortedIndices[0] = triangle.m_indices[1];
            if (triangle.m_indices[0] < triangle.m_indices[2])
                {
                m_sortedIndices[1] = triangle.m_indices[0];
                m_sortedIndices[2] = triangle.m_indices[2];
                }
            else
                {
                m_sortedIndices[1] = triangle.m_indices[2];
                m_sortedIndices[2] = triangle.m_indices[0];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle.m_indices[2];
            m_sortedIndices[1] = triangle.m_indices[1];
            m_sortedIndices[2] = triangle.m_indices[0];
            }
        }
    BeAssert (m_sortedIndices[0] < m_sortedIndices[1]);
    BeAssert (m_sortedIndices[1] < m_sortedIndices[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::TriangleKey::operator<(TriangleKey const& rhs) const
    {
    COMPARE_VALUES (m_sortedIndices[0], rhs.m_sortedIndices[0]);
    COMPARE_VALUES (m_sortedIndices[1], rhs.m_sortedIndices[1]);
    COMPARE_VALUES (m_sortedIndices[2], rhs.m_sortedIndices[2]);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(PolyfaceVisitorR visitor, DgnElementId elemId, bool doVertexClustering, bool duplicateTwoSidedTriangles)
    {
    BeAssert(3 == visitor.Point().size());

    Triangle newTriangle(!visitor.GetTwoSided());
    bvector<DPoint2d> params;

    // ###TODO: If have material: Material::ComputeUVParams()

    params = visitor.Param();
    bool haveNormals = !visitor.Normal().empty();
    for (size_t i = 0; i < 3; i++)
        {
        VertexKey vertex(visitor.Point().at(i), haveNormals ? &visitor.Normal().at(i) : nullptr, params.empty() ? nullptr : &params.at(i), elemId);
        newTriangle.m_indices[i] = doVertexClustering ? AddClusteredVertex(vertex) : AddVertex(vertex);
        }

    BeAssert(m_mesh->Params().empty() || m_mesh->Params().size() == m_mesh->Points().size());
    BeAssert(m_mesh->Normals().empty() || m_mesh->Normals().size() == m_mesh->Points().size());

    AddTriangle(newTriangle);
    ++m_triangleIndex;

    if (visitor.GetTwoSided() && duplicateTwoSidedTriangles)
        {
        Triangle dupTriangle(false);
        for (size_t i = 0; i < 3; i++)
            {
            size_t reverseIndex = 2 - i;
            DVec3d reverseNormal;
            if (haveNormals)
                reverseNormal.Negate(visitor.Normal().at(reverseIndex));

            VertexKey vertex(visitor.Point().at(reverseIndex), haveNormals ? &reverseNormal : nullptr, params.empty() ? nullptr : &params.at(reverseIndex), elemId);
            dupTriangle.m_indices[i] = doVertexClustering ? AddClusteredVertex(vertex) : AddVertex(vertex);
            }

        AddTriangle(dupTriangle);
        ++m_triangleIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyline (bvector<DPoint3d>const& points, DgnElementId elemId, bool doVertexClustering)
    {
    TilePolyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, elemId);

        newPolyline.m_indices.push_back (doVertexClustering ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }
    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(TriangleCR triangle, TileMeshCR mesh)
    {
    Triangle newTriangle(triangle.m_singleSided);
    for (size_t i = 0; i < 3; i++)
        {
        uint32_t index = triangle.m_indices[i];
        VertexKey vertex(*mesh.GetPoint(index), mesh.GetNormal(index), mesh.GetParam(index), mesh.GetElementId(index));
        newTriangle.m_indices[i] = AddVertex(vertex);
        }

    m_mesh->AddTriangle(newTriangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(TriangleCR triangle)
    {
    if (triangle.IsDegenerate())
        return;

    TriangleKey key(triangle);

    if (m_triangleSet.insert(key).second)
        m_mesh->AddTriangle(triangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddVertex(VertexKey const& vertex)
    {
    return m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddClusteredVertex(VertexKey const& vertex)
    {
    auto found = m_vertexMap.find(vertex);
    if (m_vertexMap.end() != found)
        return found->second;

    auto index = AddVertex(vertex);
    m_vertexMap[vertex] = index;
    return index;
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct ComputeFacetCountTreeHandler : XYZRangeTreeHandler
{
    DRange3d                m_range;
    size_t                  m_facetCount;

    ComputeFacetCountTreeHandler (DRange3dCR range) : m_range (range), m_facetCount (0) { }

    virtual bool ShouldRecurseIntoSubtree (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior) override { return pInterior->Range().IntersectsWith (m_range); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool ShouldContinueAfterLeaf (XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override
    {   
    DRange3d        intersection;
                                 
    intersection.IntersectionOf (pLeaf->Range(), m_range);

    if (!intersection.IsNull())
        m_facetCount += (size_t) ((reinterpret_cast <RangeTreeNode*> (pLeaf->GetData()))->m_geometry->GetFacetCountDensity() * intersection.Volume());

    return true;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::ComputeSubTiles(bvector<DRange3d>& subTileRanges, TileGeometryCacheR geometryCache, DRange3dCR range, size_t maxFacetsPerSubTile)
    {
    bvector<DRange3d> bisectRanges;
    DVec3d diagonal = range.DiagonalVector();

    if (diagonal.x > diagonal.y && diagonal.x > diagonal.z)
        {
        double bisectValue = (range.low.x + range.high.x) / 2.0;

        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, range.low.z, bisectValue, range.high.y, range.high.z));
        bisectRanges.push_back (DRange3d::From (bisectValue, range.low.y, range.low.z, range.high.x, range.high.y, range.high.z));
        }
    else if (diagonal.y > diagonal.z)
        {
        double bisectValue = (range.low.y + range.high.y) / 2.0;

        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, range.low.z, range.high.x, bisectValue, range.high.z));
        bisectRanges.push_back (DRange3d::From (range.low.x, bisectValue, range.low.z, range.high.x, range.high.y, range.high.z));
        }
    else
        {
        double bisectValue = (range.low.z + range.high.z) / 2.0;

        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, range.low.z, range.high.x, range.high.y, bisectValue));
        bisectRanges.push_back (DRange3d::From (range.low.x, range.low.y, bisectValue, range.high.x, range.high.y, range.high.z));
        }

    for (auto& bisectRange : bisectRanges)
        {
        ComputeFacetCountTreeHandler    treeHandler (bisectRange);

        geometryCache.GetTree().Traverse(treeHandler);
        if (treeHandler.m_facetCount < maxFacetsPerSubTile)
            {
            if (treeHandler.m_facetCount != 0)
                subTileRanges.push_back (bisectRange);
            }
        else
            {
            ComputeSubTiles (subTileRanges, geometryCache, bisectRange, maxFacetsPerSubTile);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::ComputeTiles(TileGeometryCacheR geometryCache, double chordTolerance, size_t maxPointsPerTile)
    {
    static const size_t s_depthLimit = 0xffff;
    static const double s_targetChildCount = 5.0;

    ComputeFacetCountTreeHandler handler(m_range);
    geometryCache.GetTree().Traverse(handler);

    if (handler.m_facetCount < maxPointsPerTile)
        {
        m_tolerance = chordTolerance;
        }
    else if (m_depth < s_depthLimit)
        {
        bvector<DRange3d> subRanges;
        size_t targetChildFacetCount = static_cast<size_t>(static_cast<double>(handler.m_facetCount) / s_targetChildCount);
        size_t siblingIndex = 0;

        ComputeSubTiles(subRanges, geometryCache, m_range, targetChildFacetCount);
        for (auto& subRange : subRanges)
            {
            double childTolerance = pow(subRange.Volume() / maxPointsPerTile, 1.0 / 3.0);
            m_children.push_back(TileNode(subRange, m_depth+1, siblingIndex++, childTolerance, this));
            }

        for (auto& child : m_children)
            child.ComputeTiles(geometryCache, chordTolerance, maxPointsPerTile);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double TileNode::GetMaxDiameter(double tolerance) const
    {
    static double s_maxPixelRatio = 0.25;
    return m_range.DiagonalDistance() / (tolerance * s_maxPixelRatio);
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct MeshBuilderKey
{
    TileDisplayParamsCP m_params;
    bool                m_hasNormals;
    bool                m_hasFacets;

    MeshBuilderKey() : m_params(nullptr), m_hasNormals(false), m_hasFacets (false) { }
    MeshBuilderKey(TileDisplayParamsCR params, bool hasNormals, bool hasFacets) : m_params(&params), m_hasNormals(hasNormals), m_hasFacets (hasFacets) { }

    bool operator<(MeshBuilderKey const& rhs) const
        {
        BeAssert(nullptr != m_params && nullptr != rhs.m_params);
        if (m_hasNormals != rhs.m_hasNormals)
            return !m_hasNormals;

        if (m_hasFacets != rhs.m_hasFacets)
            return !m_hasFacets;

        return *m_params < *rhs.m_params;
        }
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct GatherGeometryCacheHandler : XYZRangeTreeHandler
{
    TileGeometryList&   m_geometry;
    DRange3d            m_range;

    GatherGeometryCacheHandler(TileGeometryList& geom, DRange3dCR range) : m_geometry(geom), m_range(range) { }
    virtual bool ShouldRecurseIntoSubtree(XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior) override { return pInterior->Range().IntersectsWith(m_range); }
    virtual bool ShouldContinueAfterLeaf(XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override
        {
        if (pLeaf->Range().IntersectsWith(m_range))
            {
            auto rtn = reinterpret_cast<RangeTreeNode*>(pLeaf->GetData());
            m_geometry.push_back(rtn->m_geometry);
            }

        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static IFacetOptionsPtr createTileFacetOptions(double chordTolerance)
    {
    IFacetOptionsPtr opts = IFacetOptions::Create();

    opts->SetChordTolerance(chordTolerance);
    opts->SetMaxPerFace(3);
    opts->SetCurvedSurfaceMaxPerFace(3);
    opts->SetParamsRequired(true);
    opts->SetNormalsRequired(true);

    return opts;
    }

typedef bmap<MeshBuilderKey, TileMeshBuilderPtr> MeshBuilderMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshList TileNode::GenerateMeshes(TileGeometryCacheR geometryCache, double tolerance, TileGeometry::NormalMode normalMode, bool twoSidedTriangles) const
    {
    static const double s_minRangeBoxSize = 0.5;
    static const double s_vertexToleranceRatio = 1.0;
    static const double s_decimateThresholdPixels = 50.0;
    static const size_t s_maxGeometryIdCount = 0xffff;

    double vertexTolerance = tolerance * s_vertexToleranceRatio;
    IFacetOptionsPtr facetOptions = createTileFacetOptions(tolerance);
    size_t  geometryCount = 0;

    MeshBuilderMap builderMap;
    TileGeometryList geometries;

    GatherGeometryCacheHandler handler(geometries, m_range);
    geometryCache.GetTree().Traverse(handler);

    std::sort(geometries.begin(), geometries.end(), [&](TileGeometryPtr const& lhs, TileGeometryPtr const& rhs) { return lhs->GetFacetCountDensity() < rhs->GetFacetCountDensity(); });

    for (auto& geometry : geometries)
        {
        DRange3dCR geometryRange = geometry->GetRange();
        double rangePixels = geometryRange.DiagonalDistance() / tolerance;
        if (rangePixels < s_minRangeBoxSize)
            continue;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

        CurveVectorPtr      strokes;
        PolyfaceHeaderPtr   polyface;
        
        if (!(strokes = geometry->GetStrokedCurve (tolerance)).IsValid() &&
            !(polyface = geometry->GetPolyface(tolerance, normalMode)).IsValid())
            continue;

        TileDisplayParamsCP     displayParams = &geometry->GetDisplayParams();
        TileMeshBuilderPtr      meshBuilder;
        MeshBuilderKey          key(*displayParams, polyface.IsValid() && nullptr != polyface->GetNormalIndexCP(), polyface.IsValid());
        auto                    found = builderMap.find(key);

        if (builderMap.end() != found)
            meshBuilder = found->second;
        else
            builderMap[key] = meshBuilder = TileMeshBuilder::Create(displayParams, &geometryCache.GetTransformToDgn(), vertexTolerance);

        bool isContained = geometryRange.IsContained(m_range);
        bool doVertexClustering = rangePixels < s_decimateThresholdPixels;

        ++geometryCount;
        bool maxGeometryCountExceeded = geometryCount > s_maxGeometryIdCount;
        if (polyface.IsValid())
            {
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                {
                if (isContained || m_range.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
                    {
                    DgnElementId elemId;
                    if (!maxGeometryCountExceeded)
                        elemId = geometry->GetElementId();

                    meshBuilder->AddTriangle (*visitor, elemId, doVertexClustering, twoSidedTriangles);
                    }
                }
            }
        if (strokes.IsValid())
            {
            for (auto& curvePrimitive : *strokes)
                {
                bvector<DPoint3d> const* lineString = curvePrimitive->GetLineStringCP ();

                if (nullptr == lineString)
                    {
                    BeAssert (false);
                    continue;
                    }
                DgnElementId elemId;
                if (!maxGeometryCountExceeded)
                    elemId = geometry->GetElementId();

                meshBuilder->AddPolyline (*lineString, elemId, doVertexClustering);
                }
            }
        }

    TileMeshList meshes;
    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back (builder.second->GetMesh());

    // ###TODO: statistics: record empty node...

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshPtr TileNode::GetRangeMesh(DRange3dCR range, TileGeometryCacheR geometryCache) const
    {
    DPoint3d baseOrigin = range.low,
             topOrigin  = DPoint3d::From(range.low.x, range.low.y, range.high.z);

    double xSize = range.high.x - range.low.x,
           ySize = range.high.y - range.low.y;

    auto facetOptions = createTileFacetOptions(1.0E-4);
    auto polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);

    DgnBoxDetail boxDetail(baseOrigin, topOrigin, DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), xSize, ySize, xSize, ySize, true);
    auto solidPrimitive = ISolidPrimitive::CreateDgnBox(boxDetail);
    polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);

    auto meshBuilder = TileMeshBuilder::Create(nullptr, &geometryCache.GetTransformToDgn(), 1.0E-6);
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyfaceBuilder->GetClientMeshR()); visitor->AdvanceToNextFace(); /**/)
        meshBuilder->AddTriangle(*visitor, DgnElementId(), true, false);

    return meshBuilder->GetMesh();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshPtr TileNode::GetDefaultMesh(TileGeometryCacheR geometryCache) const
    {
    return GetRangeMesh(m_range, geometryCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshPtr TileNode::GetRangeMesh(TileGeometryCacheR geometryCache) const
    {
    DPoint3d rangeCenter = DPoint3d::FromInterpolate(m_range.low, 0.5, m_range.high);
    DRange3d range = DRange3d::From(rangeCenter);

    range.Extend(1.0E-2);

    return GetRangeMesh(range, geometryCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TileNode::GetNodeCount() const
    {
    size_t count = 1;
    for (auto const& child : m_children)
        count += child.GetNodeCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TileNode::GetMaxDepth() const
    {
    size_t maxChildDepth = 0;
    for (auto const& child : m_children)
        {
        size_t childDepth = child.GetMaxDepth();
        maxChildDepth = std::max(maxChildDepth, childDepth);
        }

    return 1 + maxChildDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::GetTiles(TileNodePList& tiles)
    {
    tiles.push_back(this);
    for (auto& child : m_children)
        child.GetTiles(tiles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileNodePList TileNode::GetTiles()
    {
    TileNodePList tiles;
    GetTiles(tiles);
    return tiles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, bool isCurved, DgnDbR db)
    : m_params(params), m_transform(tf), m_range(range), m_elementId(elemId), m_isCurved(isCurved), m_hasTexture(params.QueryTexture(db).IsValid())
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometry::SetFacetCount(size_t numFacets)
    {
    m_facetCount = numFacets;
    double rangeVolume = m_range.Volume();
    m_facetCountDensity = (0.0 != rangeVolume) ? static_cast<double>(m_facetCount) / rangeVolume : 0.0;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveTileGeometry : TileGeometry
{
private:
    IGeometryPtr        m_geometry;

    PrimitiveTileGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry)
        {
        FacetCounter counter(facetOptions);
        SetFacetCount(counter.GetFacetCount(geometry));
        }

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) override;
    virtual CurveVectorPtr _GetStrokedCurve(double chordTolerance) override;
public:
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
        {
        return new PrimitiveTileGeometry(geometry, tf, range, elemId, params, facetOptions, isCurved, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct SolidKernelTileGeometry : TileGeometry
{
private:
    ISolidKernelEntityPtr   m_entity;
    BeMutex                 m_mutex;

    SolidKernelTileGeometry(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR facetOptions, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, SolidKernelUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid)
        {
        FacetCounter counter(facetOptions);
        SetFacetCount(counter.GetFacetCount(solid));
        }

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) override;
    virtual CurveVectorPtr _GetStrokedCurve(double) override { return nullptr; }
public:
    static TileGeometryPtr Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR facetOptions, DgnDbR db)
        {
        return new SolidKernelTileGeometry(solid, tf, range, elemId, params, facetOptions, db);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
    {
    return PrimitiveTileGeometry::Create(geometry, tf, range, elemId, params, facetOptions, isCurved, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, IFacetOptionsR facetOptions, DgnDbR db)
    {
    return SolidKernelTileGeometry::Create(solid, tf, range, elemId, params, facetOptions, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PrimitiveTileGeometry::_GetPolyface(IFacetOptionsR facetOptions)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    if (polyface.IsValid())
        {
        if (!HasTexture())
            polyface->ClearParameters(false);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return polyface;
        }

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    CurveVectorPtr curveVector = m_geometry->GetAsCurveVector();
    ISolidPrimitivePtr solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    if (curveVector.IsValid())
        polyfaceBuilder->AddRegion(*curveVector);
    else if (solidPrimitive.IsValid())
        polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    else if (bsplineSurface.IsValid())
        polyfaceBuilder->Add(*bsplineSurface);

    polyface = polyfaceBuilder->GetClientMeshPtr();
    if (polyface.IsValid())
        polyface->Transform(GetTransform());

    return polyface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  PrimitiveTileGeometry::_GetStrokedCurve (double chordTolerance)
    {
    CurveVectorPtr  curveVector = m_geometry->GetAsCurveVector();

    if (!curveVector.IsValid() || curveVector->IsAnyRegionType())
        return nullptr;

    IFacetOptionsPtr    facetOptions = CreateFacetOptions (chordTolerance, NormalMode::Never);
    CurveVectorPtr      strokedCurveVector = curveVector->Stroke (*facetOptions);

    if (strokedCurveVector.IsValid())
        strokedCurveVector->TransformInPlace (GetTransform());
            
    return strokedCurveVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr SolidKernelTileGeometry::_GetPolyface(IFacetOptionsR facetOptions)
    {
#if defined (BENTLEYCONFIG_OPENCASCADE)
    // Cannot process the same solid entity simultaneously from multiple threads...
    BeMutexHolder lock(m_mutex);
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(*m_entity);
    auto polyface = nullptr != shape ? OCBRep::IncrementalMesh(*shape, facetOptions) : nullptr;
    if (polyface.IsValid())
        {
        polyface->SetTwoSided(ISolidKernelEntity::EntityType_Solid != m_entity->GetEntityType());
        polyface->Transform(Transform::FromProduct(GetTransform(), m_entity->GetEntityTransform()));
        }

    return polyface;
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TileGeometry::GetPolyface(double chordTolerance, NormalMode normalMode)
    {
    auto facetOptions = CreateFacetOptions(chordTolerance, normalMode);
    auto polyface = _GetPolyface(*facetOptions);

    return polyface.IsValid() && 0 != polyface->GetPointCount() ? polyface : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr TileGeometry::CreateFacetOptions(double chordTolerance, NormalMode normalMode) const
    {
    auto facetOptions = createTileFacetOptions(chordTolerance / m_transform.ColumnXMagnitude());

    bool normalsRequired = false;
    switch (normalMode)
        {
        case NormalMode::Always:    normalsRequired = true; break;
        case NormalMode::CurvedSurfacesOnly:    normalsRequired = m_isCurved; break;
        }

    facetOptions->SetNormalsRequired(normalsRequired);
    facetOptions->SetParamsRequired(HasTexture());

    return facetOptions;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TileGeometryProcessor : IGeometryProcessor
{
    Transform                       m_dgnToTarget;
    XYZRangeTreeRootP               m_rangeTree;
    ViewControllerR                 m_view;
    DRange3d                        m_range;
    IFacetOptionsR                  m_facetOptions;
    TileGenerator::IProgressMeter&  m_progressMeter;
    TileGeometryCacheR              m_geometryCache;
    IFacetOptionsPtr                m_targetFacetOptions;
    DgnElementId                    m_curElemId;

    TileGeometryProcessor(ViewControllerR view, TileGeometryCacheR geometryCache, XYZRangeTreeRootP rangeTree, TransformCR dgnToTarget, IFacetOptionsR facetOptions, TileGenerator::IProgressMeter& progressMeter)
        : m_dgnToTarget(dgnToTarget), m_rangeTree(rangeTree), m_view(view), m_range(DRange3d::NullRange()), m_facetOptions(facetOptions),
          m_progressMeter(progressMeter), m_geometryCache(geometryCache), m_targetFacetOptions(facetOptions.Clone())
        {
        m_targetFacetOptions->SetChordTolerance(facetOptions.GetChordTolerance() * dgnToTarget.ColumnXMagnitude());
        }

    virtual IFacetOptionsP _GetFacetOptionsP() override { return &m_facetOptions; }

    bool ProcessGeometry(IGeometryR geometry, bool isCurved, SimplifyGraphic& gf);

    virtual bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    virtual bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    virtual bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessBody(ISolidKernelEntityCR solid, SimplifyGraphic& gf) override;
    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override {return UnhandledPreference::Facet;}

    virtual UnhandledPreference _GetUnhandledPreference(ISolidKernelEntityCR, SimplifyGraphic&) const override
        {
        return UnhandledPreference::Facet;
        }

    virtual void _OutputGraphics(ViewContextR context) override;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::ProcessGeometry(IGeometryR geom, bool isCurved, SimplifyGraphic& gf)

    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return true;    // ignore and continue

    Transform tf = Transform::FromProduct(m_dgnToTarget, gf.GetLocalToWorldTransform());
    tf.Multiply(range, range);
    m_range.Extend(range);

    TileDisplayParams displayParams(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());
    m_geometryCache.ResolveTexture(displayParams, m_view.GetDgnDb());
    m_rangeTree->Add(new RangeTreeNode(geom, tf, range, m_curElemId, displayParams, *m_targetFacetOptions, isCurved, m_view.GetDgnDb()), range);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf)
    {
    if (curves.IsAnyRegionType() && !curves.ContainsNonLinearPrimitive())
        return false;   // process as facets.

    CurveVectorPtr clone = curves.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return ProcessGeometry(*geom, false, gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) 
    {
    bool hasCurvedFaceOrEdge = prim.HasCurvedFaceOrEdge();
    if (!hasCurvedFaceOrEdge)
        return false;   // Process as facets.

    ISolidPrimitivePtr clone = prim.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return ProcessGeometry(*geom, hasCurvedFaceOrEdge, gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) 
    {
    MSBsplineSurfacePtr clone = MSBsplineSurface::CreatePtr();
    clone->CopyFrom(surface);
    IGeometryPtr geom = IGeometry::Create(clone);

    bool isCurved = (clone->GetUOrder() > 2 || clone->GetVOrder() > 2);
    return ProcessGeometry(*geom, isCurved, gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) 
    {
    PolyfaceHeaderPtr clone = polyface.Clone();
    if (!clone->IsTriangulated())
        clone->Triangulate();

    clone->Transform(Transform::FromProduct(m_dgnToTarget, gf.GetLocalToWorldTransform()));

    DRange3d range = clone->PointRange();
    m_range.Extend(range);

    TileDisplayParams displayParams(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());
    m_geometryCache.ResolveTexture(displayParams, m_view.GetDgnDb());

    IGeometryPtr geom = IGeometry::Create(clone);
    m_rangeTree->Add(new RangeTreeNode(*geom, Transform::FromIdentity(), range, m_curElemId, displayParams, *m_targetFacetOptions, false, m_view.GetDgnDb()), range);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessBody(ISolidKernelEntityCR solid, SimplifyGraphic& gf) 
    {
#define MESHTILE_FACET_BODIES
#if !defined(MESHTILE_FACET_BODIES)
    ISolidKernelEntityPtr clone = const_cast<ISolidKernelEntityP>(&solid);
    DRange3d range = clone->GetEntityRange();

    Transform localTo3mx = Transform::FromProduct(m_dgnToTarget, gf.GetLocalToWorldTransform());
    Transform solidTo3mx = Transform::FromProduct(localTo3mx, clone->GetEntityTransform());

    solidTo3mx.Multiply(range, range);
    m_range.Extend(range);

    TileDisplayParams displayParams(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());
    m_geometryCache.ResolveTexture(displayParams, m_view.GetDgnDb());
    auto rangeTreeNode = new RangeTreeNode(*clone, localTo3mx, range, m_curElemId, displayParams, *m_targetFacetOptions, m_view.GetDgnDb());
    m_rangeTree->Add(rangeTreeNode, range);

    return true;
#else
    // ###TODO: There's a threading issue in OpenCascade - TileGeometry::GetPolyface() is going to produce access violations in EnsureNormalConsistency() when called from another thread.
    // If we call it here on the main thread when creating the range tree, no such problems.
    return false;
#endif
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct ModelAndCategorySet : VirtualSet
{
private:
    DgnModelIdSet const&    m_models;
    DgnCategoryIdSet const& m_categories;

    virtual bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        BeAssert(2 == nVals);
        return m_models.Contains(DgnModelId(vals[0].GetValueUInt64())) && m_categories.Contains(DgnCategoryId(vals[1].GetValueUInt64()));
        }
public:
    ModelAndCategorySet(ViewControllerCR view) : m_models(view.GetViewedModels()), m_categories(view.GetViewedCategories()) { }

    bool IsEmpty() const { return m_models.empty() || m_categories.empty(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::_OutputGraphics(ViewContextR context)
    {
    // ###TODO? The dependencies between ViewContext, ViewController, and DgnViewport are pretty tangled up...
    // e.g., ViewController::DrawView() takes a ViewContext, but requires that the ViewContext has a DgnViewport...
    // DgnViewport requires a Target which requires a Device which requires a Window...
    // ViewContext::CookGeometryParams() requires a viewport in order to apply the view controller's appearance overrides...
    // m_view.DrawView(context);

    ModelAndCategorySet vset(m_view);
    if (vset.IsEmpty())
        return;

    DgnDbR db = m_view.GetDgnDb();
    context.SetDgnDb(db);

    static const Utf8CP s_ecsql3d = "SELECT ECInstanceId FROM " DGN_SCHEMA(DGN_CLASSNAME_GeometricElement3d) " WHERE InVirtualSet(?, ModelId, CategoryId)",
                        s_ecsql2d = "SELECT ECInstanceId FROM " DGN_SCHEMA(DGN_CLASSNAME_GeometricElement2d) " WHERE InVirtualSet(?, ModelId, CategoryId)";

    bool is2d = nullptr != dynamic_cast<ViewController2d const*>(&m_view);
    auto stmt = db.GetPreparedECSqlStatement(is2d ? s_ecsql2d : s_ecsql3d);

    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (m_progressMeter._WasAborted())
            break;

        m_curElemId = stmt->GetValueId<DgnElementId>(0);
        context.VisitElement(m_curElemId, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::TileGenerator(TransformCR transformFromDgn, TileGenerator::IProgressMeter* progressMeter) 
    : m_geometryCache(transformFromDgn), m_progressMeter(nullptr != progressMeter ? *progressMeter : s_defaultProgressMeter)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::LoadGeometry(ViewControllerR view, double toleranceInMeters)
    {
    m_progressMeter._SetTaskName(TaskName::CollectingGeometry);
    m_progressMeter._IndicateProgress(0, 1);

    IFacetOptionsPtr facetOptions = createTileFacetOptions(toleranceInMeters);
    TileGeometryProcessor processor(view, m_geometryCache, &m_geometryCache.GetTree(), m_geometryCache.GetTransformFromDgn(), *facetOptions, m_progressMeter);
    
    StopWatch timer(true);

    GeometryProcessor::Process(processor, view.GetDgnDb());

    m_statistics.m_collectionTime = timer.GetCurrentSeconds();

    if (m_progressMeter._WasAborted())
        return Status::Aborted;

    m_progressMeter._IndicateProgress(1, 1);

    return processor.m_range.IsNull() ? Status::NoGeometry : Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::GenerateTiles(TileNodeR root, DRange3dCR range, double leafTolerance, size_t maxPointsPerTile)
    {
    double tolerance = pow(range.Volume()/maxPointsPerTile, 1.0/3.0);
    root = TileNode(range, 0, 0, tolerance, nullptr);
    root.ComputeTiles(m_geometryCache, leafTolerance, maxPointsPerTile);
    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::CollectTiles(TileNodeR root, ITileCollector& collector)
    {
    m_progressMeter._SetTaskName(TaskName::CreatingTiles);

    // Enqueue all tiles for processing on the IO thread pool...
    bvector<TileNode*> tiles = root.GetTiles();
    m_statistics.m_tileCount = tiles.size();
    m_statistics.m_tileDepth = root.GetMaxDepth();

    auto numTotalTiles = static_cast<uint32_t>(tiles.size());
    BeAtomic<uint32_t> numCompletedTiles;

#if !defined(MESHTILE_SINGLE_THREADED)
    auto threadPool = &BeFolly::IOThreadPool::GetPool();
    for (auto& tile : tiles)
        folly::via(threadPool, [&]()
                {
                // Once the tile tasks are enqueued we must process them...do nothing if we've already aborted...
                auto status = m_progressMeter._WasAborted() ? TileGenerator::Status::Aborted : collector._AcceptTile(*tile);
                ++numCompletedTiles;
                return status;
                });

    // Spin until all tiles complete, periodically notifying progress meter
    // Note that we cannot abort any tasks which may still be 'pending' on the thread pool...but we can skip processing them if the abort flag is set
    static const uint32_t s_sleepMillis = 1000.0;
    StopWatch timer(true);
    do
        {
        m_progressMeter._IndicateProgress(numCompletedTiles, numTotalTiles);
        BeThreadUtilities::BeSleep(s_sleepMillis);
        }
    while (numCompletedTiles < numTotalTiles);
#else
    StopWatch timer(true);
    for (auto& tile : tiles)
        {
        collector._AcceptTile(*tile);
        ++numCompletedTiles;
        }
#endif

    m_statistics.m_tileCreationTime = timer.GetCurrentSeconds();

    m_progressMeter._IndicateProgress(numTotalTiles, numTotalTiles);

    return m_progressMeter._WasAborted() ? Status::Aborted : Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGenerator::SplitMeshToMaximumSize(TileMeshList& meshes, TileMeshR mesh, size_t maxPoints)
    {
    auto const& points = mesh.Points();
    if (points.size() <= maxPoints)
        {
        meshes.push_back(&mesh);
        return;
        }

    bvector<DRange3d> subRanges;
    ComputeSubRanges(subRanges, points, maxPoints, DRange3d::From(points));
    for (auto const& subRange : subRanges)
        {
        auto meshBuilder = TileMeshBuilder::Create(mesh.GetDisplayParams(), nullptr, 1.0E-6);
        for (auto const& triangle : mesh.Triangles())
            if (subRange.IntersectsWith(mesh.GetTriangleRange(triangle)))
                meshBuilder->AddTriangle(triangle, mesh);

        meshes.push_back(meshBuilder->GetMesh());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGenerator::ComputeSubRanges(bvector<DRange3d>& subRanges, bvector<DPoint3d> const& points, size_t maxPoints, DRange3dCR range)
    {
    size_t pointCount = 0;
    DPoint3d centroid = DPoint3d::FromZero();

    for (auto const& point : points)
        {
        if (range.IsContained(point))
            {
            ++pointCount;
            centroid.Add(point);
            }
        }

    if (pointCount < maxPoints)
        {
        subRanges.push_back(range);
        }
    else
        {
        centroid.Scale(1.0 / static_cast<double>(pointCount));

        DVec3d diagonal = range.DiagonalVector();
        if (diagonal.x > diagonal.y && diagonal.x > diagonal.z)
            {
            ComputeSubRanges (subRanges, points, maxPoints, DRange3d::From (range.low.x, range.low.y, range.low.z, centroid.x, range.high.y,  range.high.z));
            ComputeSubRanges (subRanges, points, maxPoints, DRange3d::From (centroid.x, range.low.y, range.low.z, range.high.x, range.high.y, range.high.z));
            }
        else if (diagonal.y > diagonal.z)
            {
            ComputeSubRanges (subRanges, points, maxPoints, DRange3d::From (range.low.x, range.low.y, range.low.z, range.high.x, centroid.y, range.high.z));
            ComputeSubRanges (subRanges, points, maxPoints, DRange3d::From (range.low.x, centroid.y, range.low.z, range.high.x, range.high.y, range.high.z));
            }
        else
            {
            ComputeSubRanges (subRanges, points, maxPoints, DRange3d::From (range.low.x, range.low.y, range.low.z, range.high.x, range.high.y, centroid.z));
            ComputeSubRanges (subRanges, points, maxPoints, DRange3d::From (range.low.x, range.low.y, centroid.z, range.high.x, range.high.y, range.high.z));
            }
        }
    }


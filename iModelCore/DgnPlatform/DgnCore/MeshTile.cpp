/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MeshTile.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

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

    RangeTreeNode(IGeometryR geom, TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, IFacetOptionsR opts, bool isCurved)
        : m_geometry(TileGeometry::Create(geom, tf, range, elemId, params, opts, isCurved)) { }
    RangeTreeNode(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, IFacetOptionsR opts)
        : m_geometry(TileGeometry::Create(solid, tf, range, elemId, params, opts)) { }
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
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryCache::ResolveTexture(GraphicParamsCR params)
    {
    TextureImageKey key(params);
    if (m_textures.end() != m_textures.find(key))
        return;

    TextureImagePtr image;

    // ###TODO: MaterialManager::GetSimpleTextureImage()...

    m_textures.Insert(key, image);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryCache::TextureImage const* TileGeometryCache::GetTextureImage(GraphicParamsCR params) const
    {
    TextureImageKey key(params);
    auto found = m_textures.find(key);
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

    // ###TODO: Materials...

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
    GraphicParamsCP     m_params;
    bool                m_hasNormals;

    MeshBuilderKey() : MeshBuilderKey(nullptr, false) { }
    MeshBuilderKey(GraphicParamsCP params, bool hasNormals) : m_params(params), m_hasNormals(hasNormals) { }

    bool operator<(MeshBuilderKey const& rhs) const
        {
        if (m_hasNormals != rhs.m_hasNormals)
            return !m_hasNormals;

        auto lParams = m_params,
             rParams = rhs.m_params;

        if (lParams == rParams)
            return false;
        else if (nullptr == lParams || nullptr == rParams)
            return nullptr == lParams;

        if (lParams->GetFillColor().GetValue() != rParams->GetFillColor().GetValue())
            return lParams->GetFillColor().GetValue() < rParams->GetFillColor().GetValue();
        else if (lParams->GetMaterial() != rParams->GetMaterial())
            return lParams->GetMaterial() < rParams->GetMaterial();

        return false;
        }
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     06/2016
+===============+===============+===============+===============+===============+======*/
struct GatherGeometryCacheHandler : XYZRangeTreeHandler
{
    TileGeometryList    m_geometry;
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

        PolyfaceHeaderPtr polyface = geometry->GetPolyface(tolerance, normalMode);
        if (polyface.IsNull())
            continue;

        GraphicParamsCP graphicParams = &geometry->GetGraphicParams();
        TileMeshBuilderPtr meshBuilder;
        MeshBuilderKey key(graphicParams, nullptr != polyface->GetNormalIndexCP());
        auto found = builderMap.find(key);
        if (builderMap.end() != found)
            meshBuilder = found->second;
        else
            builderMap[key] = meshBuilder = TileMeshBuilder::Create(graphicParams, &geometryCache.GetTransformToDgn(), vertexTolerance);

        bool isContained = geometryRange.IsContained(m_range);
        bool doVertexClustering = rangePixels < s_decimateThresholdPixels;

        ++geometryCount;
        bool maxGeometryCountExceeded = geometryCount > s_maxGeometryIdCount;
        for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
            {
            if (isContained || m_range.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
                {
                DgnElementId elemId;
                if (!maxGeometryCountExceeded)
                    elemId = geometry->GetElementId();

                meshBuilder->AddTriangle(*visitor, elemId, doVertexClustering, twoSidedTriangles);
                }
            }
        }

    TileMeshList meshes;
    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back(builder.second->GetMesh());

    // ###TODO: statistics...
    // if (meshes.empty())
    //     statistics.m_emptyNodeCount++;

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
TileGeometry::TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, bool isCurved)
    : m_params(params), m_transform(tf), m_range(range), m_elementId(elemId), m_type(Type::Empty), m_isCurved(isCurved), m_isInstanced(false), m_solidEntity(nullptr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometry::Init(IGeometryR geom, IFacetOptionsR options)
    {
    geom.AddRef();
    m_geometry = &geom;
    m_facetCount = FacetCountUtil::GetFacetCount(geom, options);

    double rangeVolume = m_range.Volume();
    m_facetCountDensity = (0.0 != rangeVolume) ? static_cast<double>(m_facetCount) / rangeVolume : 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometry::Init(ISolidKernelEntityR solid, IFacetOptionsR options)
    {
    // ###TODO: Solids...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::~TileGeometry()
    {
    auto solid = GetSolidEntity();
    auto geom = GetGeometry();
    if (nullptr != solid)
        solid->Release();
    else if (nullptr != geom)
        geom->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, IFacetOptionsR facetOptions, bool isCurved)
    {
    TileGeometryPtr tileGeom = new TileGeometry(tf, range, elemId, params, isCurved);
    tileGeom->Init(geometry, facetOptions);
    return tileGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, GraphicParamsCR params, IFacetOptionsR facetOptions)
    {
    TileGeometryPtr tileGeom = new TileGeometry(tf, range, elemId, params, false);
    tileGeom->Init(solid, facetOptions);
    return tileGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometry::HasTexture() const
    {
    return false; // ###TODO: Materials...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TileGeometry::GetPolyface(double chordTolerance, NormalMode normalMode)
    {
    auto geometry = GetGeometry();
    auto solid = nullptr != geometry ? GetSolidEntity() : nullptr;
    if (nullptr == geometry && nullptr == solid)
        return nullptr;

    PolyfaceHeaderPtr polyface = nullptr != geometry ? geometry->GetAsPolyfaceHeader() : nullptr;
    if (polyface.IsValid())
        {
        if (!HasTexture())
            polyface->ClearParameters(false);

        BeAssertOnce(m_transform.IsIdentity()); // Polyfaces are transformed during collection.
        return polyface;
        }

    auto preTesselated = m_tesselations.find(chordTolerance);
    if (m_tesselations.end() != preTesselated)
        return preTesselated->second;

    IFacetOptionsPtr facetOptions = CreateFacetOptions(chordTolerance, normalMode);

    if (nullptr != solid)
        {
        // ###TODO: Solids...
        }
    else
        {
        BeAssert(nullptr != geometry);
        IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);

        CurveVectorPtr curveVector = geometry->GetAsCurveVector();
        ISolidPrimitivePtr solidPrimitive = curveVector.IsNull() ? geometry->GetAsISolidPrimitive() : nullptr;
        MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? geometry->GetAsMSBsplineSurface() : nullptr;

        if (curveVector.IsValid())
            polyfaceBuilder->AddRegion(*curveVector);
        else if (solidPrimitive.IsValid())
            polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
        else if (bsplineSurface.IsValid())
            polyfaceBuilder->Add(*bsplineSurface);

        polyface = polyfaceBuilder->GetClientMeshPtr();
        if (polyface.IsValid())
            polyface->Transform(m_transform);
        }

    if (polyface.IsNull() || 0 == polyface->GetPointCount())
        return nullptr;

    if (m_isInstanced)
        m_tesselations[chordTolerance] = polyface;  // ###TODO: This appears never to be set...

    return polyface;
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
TileGenerator::Status TileGenerator::LoadGeometry(DgnViewportR vp, double toleranceInMeters)
    {
    return Status::NoGeometry; // ###TODO...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::GenerateTiles(TileNodeR root, DRange3dCR range, double leafTolerance, size_t maxPointsPerTile)
    {
    return Status::NoGeometry; // ###TODO...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::CollectTiles(TileNodeR root, ITileCollector& collector)
    {
    return Status::NoGeometry; // ###TODO...
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
        auto meshBuilder = TileMeshBuilder::Create(mesh.GetGraphicParams(), nullptr, 1.0E-6);
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


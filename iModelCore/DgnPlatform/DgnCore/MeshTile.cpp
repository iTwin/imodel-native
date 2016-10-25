/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MeshTile.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>
#include <folly/futures/Future.h>
#include <Geom/XYZRangeTree.h>
#if defined (BENTLEYCONFIG_OPENCASCADE) 
#include <DgnPlatform/DgnBRep/OCBRep.h>
#endif

#if defined(BENTLEYCONFIG_OS_WINDOWS)
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_RENDER

BEGIN_UNNAMED_NAMESPACE
static ITileGenerationProgressMonitor   s_defaultProgressMeter;
static UnconditionalTileGenerationFilter s_defaultFilter;

struct RangeTreeNode
{
    // ###TODO: On 64-bit hardware, don't allocate a node just to hold a 64-bit integer...
    DgnElementId    m_elementId;

    RangeTreeNode(DgnElementId elemId) : m_elementId(elemId) { }
};

static const int    s_splitCount         = 3;       // 3 splits per parent (oct-trees).
static const double s_minRangeBoxSize    = 0.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh
static const size_t s_maxGeometryIdCount = 0xffff;  // Max batch table ID - 16-bit unsigned integers
static const double s_minToleranceRatio = 100.0;

static Render::GraphicSet s_unusedDummyGraphicSet;

#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
static const Utf8CP s_geometrySource3dECSql = "SELECT CategoryId,GeometryStream,Yaw,Pitch,Roll,Origin,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE ECInstanceId=?";
#else
static const Utf8CP s_geometrySource3dNativeSql =
    "SELECT CategoryId,GeometryStream,Yaw,Pitch,Roll,Origin_X,Origin_Y,Origin_Z,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z FROM "
    BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHERE ElementId=?";
#endif

END_UNNAMED_NAMESPACE

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerationCache::TileGenerationCache(Options options) : m_tree(XYZRangeTreeRoot::Allocate()), m_options(options),
    m_dbMutex(BeSQLite::BeDbMutex::MutexType::Recursive)
    {
    // Caller will populate...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGenerationCache::AddCachedGeometry(DgnElementId elementId, TileGeometryList&& geometry) const
    {
    if (WantCacheGeometry())
        {
        BeMutexHolder lock(m_mutex);

        m_geometry.Insert(elementId, geometry);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGenerationCache::GetCachedGeometry(TileGeometryList& geometry, DgnElementId elementId) const
    {
    if (WantCacheGeometry())
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_geometry.find(elementId);
        if (m_geometry.end() != iter)
            {
            if (geometry.empty())
                geometry = iter->second;
            else
                geometry.insert(geometry.end(), iter->second.begin(), iter->second.end());

            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySourceCP TileGenerationCache::AddCachedGeometrySource(std::unique_ptr<GeometrySource>& source, DgnElementId elemId) const
    {
    if (!WantCacheGeometrySources())
        return source.get();

    BeMutexHolder lock(m_mutex);

    // May already exist in cache...if so we've moved from it and it will be destroyed...otherwise it's now owned by cache
    m_geometrySources.insert(GeometrySourceMap::value_type(elemId, std::move(source)));

    // Either way, we know an now exists in cache for this element
    auto existing = GetCachedGeometrySource(elemId);
    BeAssert(nullptr != existing);
    return existing;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySourceCP TileGenerationCache::GetCachedGeometrySource(DgnElementId elemId) const
    {
    if (!WantCacheGeometrySources())
        return nullptr;

    BeMutexHolder lock(m_mutex);
    auto iter = m_geometrySources.find(elemId);
    return m_geometrySources.end() != iter ? iter->second.get() : nullptr;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct FreeLeafDataTreeHandler : XYZRangeTreeHandler
{
    virtual bool ShouldContinueAfterLeaf(XYZRangeTreeRootP pRoot, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override
        {
        delete reinterpret_cast<RangeTreeNode*>(pLeaf->GetData());
        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerationCache::~TileGenerationCache()
    {
    FreeLeafDataTreeHandler handler;
    m_tree->Traverse(handler);

    XYZRangeTreeRoot::Free(m_tree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileGenerationCache::GetRange() const
    {
    return GetTree().Range();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGenerationCache::Populate(DgnDbR db, ITileGenerationFilterR filter)
    {
    // ###TODO_FACET_COUNT: Assumes 3d spatial view for now...
    static const Utf8CP s_sql =
        "SELECT r.ECInstanceId,r.MinX,r.MinY,r.MinZ,r.MaxX,r.MaxY,r.MaxZ "
        "FROM " BIS_SCHEMA(BIS_CLASS_SpatialIndex) " AS r JOIN " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g ON (g.ECInstanceId = r.ECInstanceId)";

    auto stmt = db.GetPreparedECSqlStatement(s_sql);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto elemId = stmt->GetValueId<DgnElementId>(0);
        if (!filter.AcceptElement(elemId))
            continue;

        DRange3d elRange = DRange3d::From(stmt->GetValueDouble(1), stmt->GetValueDouble(2), stmt->GetValueDouble(3),
                stmt->GetValueDouble(4), stmt->GetValueDouble(5), stmt->GetValueDouble(6));

        m_tree->Add(new RangeTreeNode(elemId), elRange);
        }
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
TileDisplayParams::TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams) : m_fillColor(nullptr != graphicParams ? graphicParams->GetFillColor().GetValue() : 0x00ffffff), m_ignoreLighting (false)
        {
        if (nullptr != geometryParams)
            m_materialId = geometryParams->GetMaterialId();
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::operator<(TileDisplayParams const& rhs) const
    {
    COMPARE_VALUES (m_fillColor, rhs.m_fillColor);
    COMPARE_VALUES (m_materialId.GetValueUnchecked(), rhs.m_materialId.GetValueUnchecked());
    // No need to compare textures -- if materials match then textures must too.

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource TileTextureImage::Load(TileDisplayParamsCR params, DgnDbR db)
    {
    DgnTextureCPtr tex = params.QueryTexture(db);
    return tex.IsValid() ? tex->GetImageSource() : ImageSource();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileDisplayParams::ResolveTextureImage(DgnDbR db) const
    {
    if (m_textureImage.IsValid())
        return;

    ImageSource renderImage  = TileTextureImage::Load(*this, db);

    if (renderImage.IsValid())
        m_textureImage = TileTextureImage::Create(std::move(renderImage));
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
uint32_t TileMesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, BeInt64Id entityId)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(point);
    m_entityIds.push_back(entityId);

    if (nullptr != normal)
        m_normals.push_back(*normal);

    if (nullptr != param)
        m_uvParams.push_back(*param);

    m_validIdsPresent |= (entityId.IsValid());
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::VertexKey::Comparator::operator()(VertexKey const& lhs, VertexKey const& rhs) const
    {
    static const double s_normalTolerance = .1;     
    static const double s_paramTolerance  = .1;

    COMPARE_VALUES (lhs.m_entityId, rhs.m_entityId);

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
void TileMeshBuilder::AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, BeInt64Id entityId, bool doDecimate, bool duplicateTwoSidedTriangles)
    {
    auto const&       points = visitor.Point();
    BeAssert(3 == points.size());

    if (doDecimate)
        {
        DVec3d      cross;

        cross.CrossProductToPoints (points.at(0), points.at(1), points.at(2));
        if (cross.MagnitudeSquared() < m_areaTolerance)
            return;
        }

    Triangle                newTriangle(!visitor.GetTwoSided());
    bvector<DPoint2d>       params = visitor.Param();

    if (!params.empty() &&
        (m_material.IsValid() || (materialId.IsValid() && SUCCESS == m_material.Load (materialId, dgnDb))))
        {
        auto const&         patternMap = m_material.GetPatternMap();
        bvector<DPoint2d>   computedParams;

        if (patternMap.IsValid() &&
            SUCCESS == patternMap.ComputeUVParams (computedParams, visitor))
            params = computedParams;
        }
            
    bool haveNormals = !visitor.Normal().empty();
    for (size_t i = 0; i < 3; i++)
        {
        VertexKey vertex(points.at(i), haveNormals ? &visitor.Normal().at(i) : nullptr, params.empty() ? nullptr : &params.at(i), entityId);
        newTriangle.m_indices[i] = doDecimate ? AddClusteredVertex(vertex) : AddVertex(vertex);
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

            VertexKey vertex(points.at(reverseIndex), haveNormals ? &reverseNormal : nullptr, params.empty() ? nullptr : &params.at(reverseIndex), entityId);
            dupTriangle.m_indices[i] = doDecimate ? AddClusteredVertex(vertex) : AddVertex(vertex);
            }

        AddTriangle(dupTriangle);
        ++m_triangleIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyline (bvector<DPoint3d>const& points, BeInt64Id entityId, bool doDecimate)
    {
    TilePolyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, entityId);

        newPolyline.m_indices.push_back (doDecimate ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }
    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyface (PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, BeInt64Id entityId, bool twoSidedTriangles)
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        AddTriangle(*visitor, materialId, dgnDb, entityId, false, twoSidedTriangles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddVertex(VertexKey const& vertex)
    {
    auto found = m_unclusteredVertexMap.find(vertex);
    if (m_unclusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_entityId);
    m_unclusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddClusteredVertex(VertexKey const& vertex)
    {
    auto found = m_clusteredVertexMap.find(vertex);
    if (m_clusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_entityId);
    m_clusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WString TileNode::GetNameSuffix() const
    {
    WString suffix;

    if (nullptr != m_parent)
        {
        suffix = WPrintfString(L"%02d", static_cast<int>(m_siblingIndex));
        for (auto parent = m_parent; nullptr != parent->GetParent(); parent = parent->GetParent())
            suffix = WPrintfString(L"%02d", static_cast<int>(parent->GetSiblingIndex())) + suffix;
        }

    return suffix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void setSubDirectoryRecursive (TileNodeR tile, WStringCR subdirectory)
    {
    tile.SetSubdirectory (subdirectory);
    for (auto& child : tile.GetChildren())
        setSubDirectoryRecursive(*child, subdirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus TileNode::GenerateSubdirectories (size_t maxTilesPerDirectory, BeFileNameCR dataDirectory)
    {
    if (GetNodeCount () < maxTilesPerDirectory)
        return BeFileNameStatus::Success;
        
    for (auto& child : m_children)
        {
        if (child->GetNodeCount() < maxTilesPerDirectory)
            {
            BeFileName  childDataDirectory = dataDirectory;
            WString     subdirectoryName = L"Tile"  + child->GetNameSuffix();

            childDataDirectory.AppendToPath (subdirectoryName.c_str());
            BeFileNameStatus  status;
            if (BeFileNameStatus::Success != (status = BeFileName::CreateNewDirectory (childDataDirectory)))
                return status;

            setSubDirectoryRecursive (*child, subdirectoryName);
            }
        else
            {
            child->GenerateSubdirectories (maxTilesPerDirectory, dataDirectory);
            }
        }
    return BeFileNameStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
WString TileNode::GetRelativePath (WCharCP rootName, WCharCP extension) const
    {
    WString     relativePath;

    BeFileName::BuildName (relativePath, nullptr, m_subdirectory.empty() ? nullptr : m_subdirectory.c_str(), (rootName + GetNameSuffix()).c_str(), extension);

    return relativePath;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static IFacetOptionsPtr createTileFacetOptions(double chordTolerance)
    {
    static double       s_defaultAngleTolerance = msGeomConst_piOver2;
    IFacetOptionsPtr    opts = IFacetOptions::Create();

    opts->SetChordTolerance(chordTolerance);
    opts->SetAngleTolerance(s_defaultAngleTolerance);
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
size_t TileNode::GetNodeCount() const
    {
    size_t count = 1;
    for (auto const& child : m_children)
        count += child->GetNodeCount();

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
        size_t childDepth = child->GetMaxDepth();
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
        child->GetTiles(tiles);
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
TileGeometry::TileGeometry(TransformCR tf, DRange3dCR range, BeInt64Id entityId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db)
    : m_params(params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_hasTexture(params.IsValid() && params->QueryTexture(db).IsValid())
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometry::SetFacetCount(size_t numFacets)
    {
    m_facetCount = numFacets;
    double rangeVolume = m_tileRange.DiagonalDistance();
    m_facetCountDensity = (0.0 != rangeVolume) ? static_cast<double>(m_facetCount) / rangeVolume : 0.0;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveTileGeometry : TileGeometry
{
private:
    IGeometryPtr        m_geometry;

    PrimitiveTileGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, BeInt64Id elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry)
        {
        FacetCounter counter(facetOptions);
        SetFacetCount(counter.GetFacetCount(geometry));
        }

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) override;
    virtual bool _IsPolyface () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }

    virtual CurveVectorPtr _GetStrokedCurve(double chordTolerance) override;
public:
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, BeInt64Id elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
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

    SolidKernelTileGeometry(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, BeInt64Id elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, SolidKernelUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid)
        {
        FacetCounter counter(facetOptions);
        SetFacetCount(counter.GetFacetCount(solid));
        }

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) override;
    virtual CurveVectorPtr _GetStrokedCurve(double) override { return nullptr; }
    virtual bool _IsPolyface() const override { return false; }

public:
    static TileGeometryPtr Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, BeInt64Id elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db)
        {
        return new SolidKernelTileGeometry(solid, tf, range, elemId, params, facetOptions, db);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, BeInt64Id entityId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
    {
    return PrimitiveTileGeometry::Create(geometry, tf, range, entityId, params, facetOptions, isCurved, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, BeInt64Id entityId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db)
    {
    return SolidKernelTileGeometry::Create(solid, tf, range, entityId, params, facetOptions, db);
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

    DRange3d    entityRange = m_entity->GetEntityRange();
    if (entityRange.IsNull())
        return nullptr;

    double              rangeDiagonal = entityRange.DiagonalDistance();
    static double       s_minRangeRelTol = 1.0e-4;
    double              minChordTolerance = rangeDiagonal * s_minRangeRelTol;
    IFacetOptionsPtr    pFacetOptions;
    
    if (facetOptions.GetChordTolerance() < minChordTolerance)
        {
        pFacetOptions = facetOptions.Clone();
        pFacetOptions->SetChordTolerance (minChordTolerance);
        }
    else
        {
        pFacetOptions = &facetOptions;
        }


    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(*m_entity);


    auto polyface = nullptr != shape ? OCBRep::IncrementalMesh(*shape, *pFacetOptions) : nullptr;
    if (polyface.IsValid())
        {
        polyface->SetTwoSided(ISolidKernelEntity::EntityType::Solid != m_entity->GetEntityType());
        polyface->Transform(Transform::FromProduct(GetTransform(), m_entity->GetEntityTransform()));
    
        }


    return polyface;
#elif defined (BENTLEYCONFIG_PARASOLID)    
    // Cannot process the same solid entity simultaneously from multiple threads...
    BeMutexHolder lock(m_mutex);

    DRange3d entityRange = m_entity->GetEntityRange();
    if (entityRange.IsNull())
        return nullptr;

    double              rangeDiagonal = entityRange.DiagonalDistance();
    static double       s_minRangeRelTol = 1.0e-4;
    double              minChordTolerance = rangeDiagonal * s_minRangeRelTol;
    IFacetOptionsPtr    pFacetOptions;
    
    if (facetOptions.GetChordTolerance() < minChordTolerance)
        {
        pFacetOptions = facetOptions.Clone();
        pFacetOptions->SetChordTolerance (minChordTolerance);
        }
    else
        {
        pFacetOptions = &facetOptions;
        }

    return SolidKernelUtil::FacetEntity(*m_entity, *pFacetOptions);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::TileGenerator(TransformCR transformFromDgn, DgnDbR dgndb, ITileGenerationFilterP filter, ITileGenerationProgressMonitorP progress)
    : m_progressMeter(nullptr != progress ? *progress : s_defaultProgressMeter), m_transformFromDgn(transformFromDgn), m_dgndb(dgndb), 
      m_totalTiles(0), m_completedTiles(0), m_totalVolume(0.0), m_completedVolume (0.0), m_cache(TileGenerationCache::Options::CacheGeometrySources)
    {
    StopWatch timer(true);
    m_progressMeter._SetTaskName(ITileGenerationProgressMonitor::TaskName::PopulatingCache);
    m_progressMeter._IndicateProgress(0, 1);

    m_cache.Populate(m_dgndb, nullptr != filter ? *filter : s_defaultFilter);

    m_progressMeter._IndicateProgress(1, 1);

    m_statistics.m_cachePopulationTime = timer.GetCurrentSeconds();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGenerator::ProcessTile (ElementTileNodeR tile, ITileCollector& collector, double leafTolerance, size_t maxPointsPerTile)
    {
    auto&           host = T_HOST;

    folly::via( &BeFolly::IOThreadPool::GetPool(), [&, leafTolerance, maxPointsPerTile]()
        {
        double          tileTolerance = tile.GetDgnRange().DiagonalDistance() / s_minToleranceRatio;
        bool            isLeaf = tileTolerance < leafTolerance;
        bool            leafThresholdExceeded = false;
       
        DgnPlatformLib::AdoptHost(host);

        // This initial collection is done at the (target) leaf tolerance.  If this is not already a leaf (tolerance < leaf tolerance)
        // then collection will only happen until the maxPointsPerTile is exceeded.  If leaf size is exceeded we'll have to add
        // discard this geometry, recollect at tile tolerance and add children (below).   
        tile.CollectGeometry (m_cache, m_dgndb, &leafThresholdExceeded, leafTolerance, isLeaf ? 0 :maxPointsPerTile);

        if (!isLeaf && !leafThresholdExceeded)
            isLeaf = true;

        if (tile.GetGeometries().empty())
            {
            m_completedVolume += tile.GetDgnRange().Volume();
            m_completedTiles++;
            return;
            }

        tile.SetIsEmpty (false);
        tile.SetIsLeaf(isLeaf);

        if (isLeaf)
            {
            tile.SetTolerance (leafTolerance);
            collector._AcceptTile(tile);
            m_completedVolume += tile.GetDgnRange().Volume();
            }
        else
            {
            size_t              siblingIndex = 0;
            bvector<DRange3d>   subRanges;

            tile.ComputeChildTileRanges (subRanges, tile.GetDgnRange(), s_splitCount);
            for (auto& subRange : subRanges)
                {
                ElementTileNodePtr      child  = ElementTileNode::Create(subRange, m_transformFromDgn, tile.GetDepth()+1, siblingIndex++, &tile);

                m_totalTiles++;
                tile.GetChildren().push_back (child);
                ProcessTile (*child, collector, leafTolerance, maxPointsPerTile);
                }

            tile.SetTolerance (tileTolerance);
            tile.ClearGeometry();     // Discard initial geometry (collected at leaf tolerance).
            tile.CollectGeometry (m_cache, m_dgndb, nullptr, tileTolerance, 0);
            collector._AcceptTile(tile);
            }
 
        tile.ClearGeometry();
        DgnPlatformLib::ForgetHost();
        m_completedTiles++;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::GenerateTiles (TileNodePtr& root, ITileCollector& collector, double leafTolerance, size_t maxPointsPerTile)
    {
    StopWatch   timer(true);

    m_totalTiles++;
    m_progressMeter._SetTaskName(ITileGenerationProgressMonitor::TaskName::GeneratingTileNodes);
    m_progressMeter._IndicateProgress(0, 1);

    DRange3d viewRange = m_cache.GetRange();
    if (viewRange.IsNull())
        {
        root = ElementTileNode::Create(GetTransformFromDgn());
        return Status::NoGeometry;
        }

    ElementTileNodePtr  elementRoot =  ElementTileNode::Create(viewRange, GetTransformFromDgn(), 0, 0, nullptr);
    root = elementRoot;
    m_totalVolume = viewRange.Volume();

    T_HOST.GetFontAdmin().EnsureInitialized();
    GetDgnDb().Fonts().Update();

    ProcessTile (*elementRoot, collector, leafTolerance, maxPointsPerTile);

    static const uint32_t s_sleepMillis = 1000.0;
    do
        {
        m_progressMeter._IndicateProgress((uint32_t (100.0) * m_completedVolume / m_totalVolume), 100);
        BeThreadUtilities::BeSleep(s_sleepMillis);
        }
    while (m_completedTiles < m_totalTiles);

    m_statistics.m_tileCount = m_totalTiles;
    m_statistics.m_tileDepth = root->GetMaxDepth();

    return m_progressMeter._WasAborted() ? Status::Aborted : Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTileNode::ComputeChildTileRanges(bvector<DRange3d>& subRanges, DRange3dCR range, size_t splitCount)
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

    splitCount--;
    for (auto& bisectRange : bisectRanges)
        {
        if (0 == splitCount)
            subRanges.push_back (bisectRange);
        else
            ComputeChildTileRanges(subRanges, bisectRange, splitCount);
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometrySource
{
    struct GeomBlob
    {
        void const* m_blob;
        int         m_size;

        GeomBlob(void const* blob, int size) : m_blob(blob), m_size(size) { }
        template<typename T> GeomBlob(T& stmt, int columnIndex)
            {
#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
            m_blob = stmt.GetValueBinary(columnIndex, &m_size);
#else
            m_blob = stmt.GetValueBlob(columnIndex);
            m_size = stmt.GetColumnBytes(columnIndex);
#endif
            }
    };
protected:
    DgnCategoryId           m_categoryId;
    GeometryStream          m_geom;
    DgnDbR                  m_db;
    bool                    m_isGeometryValid;

    TileGeometrySource(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob) : m_categoryId(categoryId), m_db(db)
        {
        m_isGeometryValid = DgnDbStatus::Success == db.Elements().LoadGeometryStream(m_geom, geomBlob.m_blob, geomBlob.m_size);
        }
public:
    bool IsGeometryValid() const { return m_isGeometryValid; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometrySource3d : TileGeometrySource, GeometrySource3d
{
private:
    Placement3d     m_placement;

    TileGeometrySource3d(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement3dCR placement)
        : TileGeometrySource(categoryId, db, geomBlob), m_placement(placement) { }

    virtual DgnDbR _GetSourceDgnDb() const override { return m_db; }
    virtual DgnElementCP _ToElement() const override { return nullptr; }
    virtual GeometrySource3dCP _ToGeometrySource3d() const override { return this; }
    virtual DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    virtual GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    virtual Placement3dCR _GetPlacement() const override { return m_placement; }

    virtual Render::GraphicSet& _Graphics() const override { BeAssert(false && "No reason to access this"); return s_unusedDummyGraphicSet; }
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    virtual DgnDbStatus _SetPlacement(Placement3dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
public:
    static std::unique_ptr<GeometrySource> Create(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement3dCR placement)
        {
        std::unique_ptr<GeometrySource> pSrc(new TileGeometrySource3d(categoryId, db, geomBlob, placement));
        if (!static_cast<TileGeometrySource3d const&>(*pSrc).IsGeometryValid())
            return nullptr;

        return pSrc;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct TileGeometryProcessorContext : NullContext
{
private:
    IGeometryProcessorR     m_processor;
    TileGenerationCacheCR   m_cache;


#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
    BeSQLite::EC::CachedECSqlStatementPtr   m_statement;

    bool IsValueNull(int index) { return m_statement->IsValueNull(index); }
#else
    BeSQLite::CachedStatementPtr            m_statement;

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }
#endif

    virtual Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override
        {
        return new SimplifyGraphic(params, m_processor, *this);
        }

    virtual StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    virtual Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;
public:
    TileGeometryProcessorContext(IGeometryProcessorR processor, DgnDbR db, TileGenerationCacheCR cache) : m_processor(processor), m_cache(cache), 
#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
    m_statement(db.GetPreparedECSqlStatement(s_geometrySource3dECSql))
#else
    m_statement(db.GetCachedStatement(s_geometrySource3dNativeSql))
#endif
        {
        SetDgnDb(db);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TileGeometryProcessorContext::_VisitElement(DgnElementId elementId, bool allowLoad)
    {
    GeometrySourceCP pSrc = m_cache.GetCachedGeometrySource(elementId);
    if (nullptr != pSrc)
        return VisitGeometry(*pSrc);

    // Never load elements - but do use them if they're already loaded
    DgnElementCPtr el = GetDgnDb().Elements().FindElement(elementId);
    if (el.IsValid())
        {
        GeometrySourceCP geomElem = el->ToGeometrySource();
        return nullptr != geomElem ? VisitGeometry(*geomElem) : ERROR;
        }

    // Load only the data we actually need for processing geometry
    // NB: The Step() below as well as each column access requires acquiring the sqlite mutex.
    // Prevent micro-contention by locking the db here
    // Note we do not use a mutex holder because we want to release the mutex before processing the geometry.
    m_cache.GetDbMutex().Enter();
    StatusInt status = ERROR;
    auto& stmt = *m_statement;
    stmt.BindInt64(1, static_cast<int64_t>(elementId.GetValueUnchecked()));

    if (BeSQLite::BE_SQLITE_ROW == stmt.Step() && !IsValueNull(1))
        {
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

#if defined(MESHTILE_SELECT_GEOMETRY_USING_ECSQL)
        DPoint3d origin = stmt.GetValuePoint3d(5),
                 boxLo  = stmt.GetValuePoint3d(6),
                 boxHi  = stmt.GetValuePoint3d(7);
#else
        DPoint3d origin = DPoint3d::From(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7)),
                 boxLo  = DPoint3d::From(stmt.GetValueDouble(8), stmt.GetValueDouble(9), stmt.GetValueDouble(10)),
                 boxHi  = DPoint3d::From(stmt.GetValueDouble(11), stmt.GetValueDouble(12), stmt.GetValueDouble(13));
#endif

        Placement3d placement(origin,
                YawPitchRollAngles(Angle::FromDegrees(stmt.GetValueDouble(2)), Angle::FromDegrees(stmt.GetValueDouble(3)), Angle::FromDegrees(stmt.GetValueDouble(4))),
                ElementAlignedBox3d(boxLo.x, boxLo.y, boxLo.z, boxHi.x, boxHi.y, boxHi.z));

        auto geomSrcPtr = TileGeometrySource3d::Create(categoryId, GetDgnDb(), geomBlob, placement);

        stmt.Reset();
        m_cache.GetDbMutex().Leave();

        pSrc = m_cache.AddCachedGeometrySource(geomSrcPtr, elementId);

        if (nullptr != pSrc)
            status = VisitGeometry(*pSrc);
        }
    else
        {
        stmt.Reset();
        m_cache.GetDbMutex().Leave();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr TileGeometryProcessorContext::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = source.Draw(*this, pixelSize);
    return WasAborted() ? nullptr : graphic;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometryProcessor : IGeometryProcessor
{
private:
    IFacetOptionsR          m_facetOptions;
    IFacetOptionsPtr        m_targetFacetOptions;
    DgnElementId            m_curElemId;
    TileGenerationCacheCR   m_cache;
    DgnDbR                  m_dgndb;
    TileGeometryList&       m_geometries;
    DRange3d                m_range;
    DRange3d                m_tileRange;
    Transform               m_transformFromDgn;
    TileGeometryList        m_curElemGeometries;
    double                  m_minRangeDiagonal;
    bool*                   m_leafThresholdExceeded;
    size_t                  m_leafCountThreshold;
    size_t                  m_leafCount;


    void PushGeometry(TileGeometryR geom);
    void AddElementGeometry(TileGeometryR geom);
    bool ProcessGeometry(IGeometryR geometry, bool isCurved, SimplifyGraphic& gf);

    virtual IFacetOptionsP _GetFacetOptionsP() override { return &m_facetOptions; }

    virtual bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    virtual bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    virtual bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessBody(ISolidKernelEntityCR solid, SimplifyGraphic& gf) override;

    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override {return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(ISolidKernelEntityCR, SimplifyGraphic&) const override { return UnhandledPreference::Facet; }
public:
    TileGeometryProcessor(TileGeometryList& geometries, TileGenerationCacheCR cache, DgnDbR db, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, bool* leafThresholdExceeded, double tolerance, size_t leafCountThreshold) 
        : m_geometries (geometries), m_facetOptions(facetOptions), m_targetFacetOptions(facetOptions.Clone()), m_cache(cache), m_dgndb(db), m_range(range), m_transformFromDgn(transformFromDgn),
          m_leafThresholdExceeded(leafThresholdExceeded), m_leafCountThreshold(leafCountThreshold), m_leafCount(0)
        {
        m_targetFacetOptions->SetChordTolerance(facetOptions.GetChordTolerance() * transformFromDgn.ColumnXMagnitude());
        m_minRangeDiagonal = s_minRangeBoxSize * tolerance;
        m_transformFromDgn.Multiply (m_tileRange, m_range);
        }

    void ProcessElement(ViewContextR context, DgnElementId elementId, DRange3dCR range);
    virtual void _OutputGraphics(ViewContextR context) override;

    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any elements with range smaller than roughly half a pixel...
        return range.DiagonalDistance() < m_minRangeDiagonal;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddElementGeometry(TileGeometryR geom)
    {
    // ###TODO: Only if geometry caching enabled...
    m_curElemGeometries.push_back(&geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::PushGeometry(TileGeometryR geom)
    {
    if (BelowMinRange(geom.GetTileRange()))
        return;

    if (nullptr != m_leafThresholdExceeded)
        {
        DRange3d intersection = DRange3d::FromIntersection (geom.GetTileRange(), m_tileRange, true);

        if (intersection.IsNull())
            return;

        m_leafCount += intersection.DiagonalDistance() * geom.GetFacetCountDensity();
        *m_leafThresholdExceeded = (m_leafCount > m_leafCountThreshold);
        }
        
    m_geometries.push_back(&geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::ProcessElement(ViewContextR context, DgnElementId elemId, DRange3dCR dgnRange)
    {
    DRange3d    tileRange;

    m_transformFromDgn.Multiply (tileRange, dgnRange);
    if ((nullptr != m_leafThresholdExceeded && *m_leafThresholdExceeded) || BelowMinRange(tileRange))
        return;

    try
        {
        m_curElemGeometries.clear();
        bool haveCached = m_cache.GetCachedGeometry(m_curElemGeometries, elemId);
        if (!haveCached)
            {
            m_curElemId = elemId;
            context.VisitElement(elemId, false);
            }
        for (auto& geom : m_curElemGeometries)
            PushGeometry(*geom);

        if (!haveCached)
            m_cache.AddCachedGeometry(elemId, std::move(m_curElemGeometries));
        }
    catch (...)
        {
        // This shouldn't be necessary - but an uncaught interception will cause the processing to continue forever. (OpenCascade error in LargeHatchPlant.)
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::ProcessGeometry(IGeometryR geom, bool isCurved, SimplifyGraphic& gf)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;   // ignore and continue

    auto tf = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    tf.Multiply(range, range);
    
    TileDisplayParamsPtr displayParams = TileDisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());

    AddElementGeometry(*TileGeometry::Create(geom, tf, range, m_curElemId, displayParams, *m_targetFacetOptions, isCurved, m_dgndb));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
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

    clone->Transform(Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform()));

    DRange3d range = clone->PointRange();

    TileDisplayParamsPtr displayParams = TileDisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());

    IGeometryPtr geom = IGeometry::Create(clone);
    AddElementGeometry(*TileGeometry::Create(*geom, Transform::FromIdentity(), range, m_curElemId, displayParams, *m_targetFacetOptions, false, m_dgndb));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessBody(ISolidKernelEntityCR solid, SimplifyGraphic& gf) 
    {
    ISolidKernelEntityPtr clone = const_cast<ISolidKernelEntityP>(&solid);
    DRange3d range = clone->GetEntityRange();

    Transform localTo3mx = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    Transform solidTo3mx = Transform::FromProduct(localTo3mx, clone->GetEntityTransform());

    solidTo3mx.Multiply(range, range);

    TileDisplayParamsPtr displayParams = TileDisplayParams::Create(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams());

    AddElementGeometry(*TileGeometry::Create(*clone, localTo3mx, range, m_curElemId, displayParams, *m_targetFacetOptions, m_dgndb));

    return true;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct GatherGeometryHandler : XYZRangeTreeHandler
{
    TileGeometryProcessor&  m_processor;
    ViewContextR            m_context;
    DRange3d                m_range;
    double                  m_tolerance;

    GatherGeometryHandler(DRange3dCR range, TileGeometryProcessor& proc, ViewContextR viewContext)
        : m_range(range), m_processor(proc), m_context(viewContext) { }

    virtual bool ShouldRecurseIntoSubtree(XYZRangeTreeRootP, XYZRangeTreeInteriorP pInterior) override
        {
        return pInterior->Range().IntersectsWith(m_range);
        }
    virtual bool ShouldContinueAfterLeaf(XYZRangeTreeRootP, XYZRangeTreeInteriorP pInterior, XYZRangeTreeLeafP pLeaf) override
        {
        if (pLeaf->Range().IntersectsWith(m_range) && !m_processor.BelowMinRange(pLeaf->Range()))
            {
            auto const& node = *reinterpret_cast<RangeTreeNode const*>(pLeaf->GetData());
            m_processor.ProcessElement(m_context, node.m_elementId, pLeaf->Range());
            }

        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::_OutputGraphics(ViewContextR context)
    {
    GatherGeometryHandler handler(m_range, *this, context);
    m_cache.GetTree().Traverse(handler);

    // We sort by size in order to ensure the largest geometries are assigned batch IDs
    // If the number of geometries does not exceed the max number of batch IDs, they will all get batch IDs so sorting is unnecessary
    if (m_geometries.size() > s_maxGeometryIdCount)
        {
        std::sort(m_geometries.begin(), m_geometries.end(), [&](TileGeometryPtr const& lhs, TileGeometryPtr const& rhs)
            {
            DRange3d lhsRange, rhsRange;
            lhsRange.IntersectionOf(lhs->GetTileRange(), m_range);
            rhsRange.IntersectionOf(rhs->GetTileRange(), m_range);
            return lhsRange.DiagonalDistance() < rhsRange.DiagonalDistance();
            });
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTileNode::_CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double tolerance, size_t leafCountThreshold)
    {
    // Collect geometry from elements in this node, sorted by size
    IFacetOptionsPtr                facetOptions = createTileFacetOptions(GetTolerance());
    TileGeometryProcessor           processor(m_geometries, cache, db, GetDgnRange(), *facetOptions, m_transformFromDgn, leafThresholdExceeded, tolerance, leafCountThreshold);
    TileGeometryProcessorContext    context(processor, db, cache);

    processor._OutputGraphics(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshList ElementTileNode::_GenerateMeshes(DgnDbR db, TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doPolylines) const
    {
    static const double s_vertexToleranceRatio    = .1;
    static const double s_decimateThresholdPixels = 50.0;
    static const double s_facetAreaToleranceRatio = .1;
    static const size_t s_decimatePolyfacePointCount = 100;

    double tolerance = GetTolerance();
    double vertexTolerance = tolerance * s_vertexToleranceRatio;
    double facetAreaTolerance   = tolerance * tolerance * s_facetAreaToleranceRatio;

    // Convert to meshes
    MeshBuilderMap  builderMap;
    size_t          geometryCount = 0;
    DRange3d        myTileRange = GetTileRange();

    for (auto& geom : m_geometries)
        {
        DRange3dCR geomRange = geom->GetTileRange();
        double rangePixels = geomRange.DiagonalDistance() / tolerance;
        if (rangePixels < s_minRangeBoxSize)
            continue;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

        CurveVectorPtr strokes = geom->GetStrokedCurve(tolerance);
        PolyfaceHeaderPtr polyface = geom->GetPolyface(tolerance, normalMode);
        if (strokes.IsNull() && polyface.IsNull())
            continue;

        TileDisplayParamsPtr displayParams = geom->GetDisplayParams();
        MeshBuilderKey key(*displayParams, polyface.IsValid() && nullptr != polyface->GetNormalIndexCP(), polyface.IsValid());

        TileMeshBuilderPtr meshBuilder;
        auto found = builderMap.find(key);
        if (builderMap.end() != found)
            meshBuilder = found->second;
        else
            builderMap[key] = meshBuilder = TileMeshBuilder::Create(displayParams, vertexTolerance, facetAreaTolerance);

        bool isContained = geomRange.IsContained(myTileRange);

        ++geometryCount;
        bool maxGeometryCountExceeded = geometryCount > s_maxGeometryIdCount;

        if (polyface.IsValid())
            {
            // Decimate if the range of the geometry is small in the tile OR we are not in a leaf and we have geometry originating from polyface with many points (railings from Penn state building).
            // A polyface with many points is likely a tesselation from an outside source.
            bool        doDecimate  = !m_isLeaf && ((geom->IsPolyface() && polyface->GetPointCount() > s_decimatePolyfacePointCount) ||  rangePixels < s_decimateThresholdPixels);

            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                {
                if (isContained || myTileRange.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
                    {
                    BeInt64Id elemId;
                    if (!maxGeometryCountExceeded)
                        elemId = geom->GetEntityId();

                    meshBuilder->AddTriangle (*visitor, displayParams->GetMaterialId(), db, elemId, doDecimate, twoSidedTriangles);
                    }
                }
            }

        if (doPolylines && strokes.IsValid())
            {
            for (auto& curvePrimitive : *strokes)
                {
                bvector<DPoint3d> const* lineString = curvePrimitive->GetLineStringCP ();

                if (nullptr == lineString)
                    {
                    BeAssert (false);
                    continue;
                    }

                BeInt64Id elemId;
                if (!maxGeometryCountExceeded)
                    elemId = geom->GetEntityId();

                meshBuilder->AddPolyline (*lineString, elemId, rangePixels < s_decimateThresholdPixels);
                }
            }
        }

    TileMeshList meshes;
    size_t       triangleCount = 0;
       
    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            {
            meshes.push_back (builder.second->GetMesh());
            triangleCount += builder.second->GetMesh()->Triangles().size();
            }

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileModelCategoryFilter::TileModelCategoryFilter(DgnDbR db, DgnModelIdSet const* models, DgnCategoryIdSet const* categories) : m_set(models, categories)
    {
    static const Utf8CP s_sql = "SELECT g.ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " As g, " BIS_SCHEMA(BIS_CLASS_Element) " AS e "
                                " WHERE g.ECInstanceId=e.ECInstanceId AND InVirtualSet(?,e.ModelId,g.CategoryId)";

    m_stmt = db.GetPreparedECSqlStatement(s_sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileModelCategoryFilter::_AcceptElement(DgnElementId elementId)
    {
    m_stmt->BindVirtualSet(1, m_set);
    bool accepted = BE_SQLITE_ROW == m_stmt->Step();
    m_stmt->Reset();
    return accepted;
    }


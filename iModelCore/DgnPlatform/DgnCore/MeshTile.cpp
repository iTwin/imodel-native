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
    COMPARE_VALUES (m_textureImage.get(), rhs.m_textureImage.get());

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
void TileTextureImage::ResolveTexture(TileDisplayParamsR params, DgnDbR db)
    {
    if (params.TextureImage().IsValid())
        return;

    ImageSource renderImage  = TileTextureImage::Load(params, db);

    if (renderImage.IsValid())
        params.TextureImage() = TileTextureImage::Create(std::move(renderImage));
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

    m_validIdsPresent |= (elemId.IsValid());
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
TileGeometry::TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, bool isCurved, DgnDbR db)
    : m_params(params), m_transform(tf), m_tileRange(range), m_elementId(elemId), m_isCurved(isCurved), m_hasTexture(params.IsValid() && params->QueryTexture(db).IsValid())
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometry::SetFacetCount(size_t numFacets)
    {
    m_facetCount = numFacets;
    double rangeVolume = m_tileRange.Volume();
    m_facetCountDensity = (0.0 != rangeVolume) ? static_cast<double>(m_facetCount) / rangeVolume : 0.0;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveTileGeometry : TileGeometry
{
private:
    IGeometryPtr        m_geometry;

    PrimitiveTileGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry)
        {
        FacetCounter counter(facetOptions);
        SetFacetCount(counter.GetFacetCount(geometry));
        }

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) override;
    virtual CurveVectorPtr _GetStrokedCurve(double chordTolerance) override;
public:
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
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

    SolidKernelTileGeometry(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, SolidKernelUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid)
        {
#if defined (BENTLEYCONFIG_OPENCASCADE)
        FacetCounter counter(facetOptions);
        SetFacetCount(counter.GetFacetCount(solid));
#else
        SetFacetCount(0);
#endif
        }

    virtual PolyfaceHeaderPtr _GetPolyface(IFacetOptionsR facetOptions) override;
    virtual CurveVectorPtr _GetStrokedCurve(double) override { return nullptr; }
public:
    static TileGeometryPtr Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db)
        {
        return new SolidKernelTileGeometry(solid, tf, range, elemId, params, facetOptions, db);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, bool isCurved, DgnDbR db)
    {
    return PrimitiveTileGeometry::Create(geometry, tf, range, elemId, params, facetOptions, isCurved, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(ISolidKernelEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsPtr& params, IFacetOptionsR facetOptions, DgnDbR db)
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
        polyface->SetTwoSided(ISolidKernelEntity::EntityType::Solid != m_entity->GetEntityType());
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
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::TileGenerator(TransformCR transformFromDgn, TileGenerator::IProgressMeter* progressMeter) 
    : m_progressMeter(nullptr != progressMeter ? *progressMeter : s_defaultProgressMeter), m_transformFromDgn(transformFromDgn)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::CollectTiles(TileNodeR root, ITileCollector& collector)
    {
    m_progressMeter._SetTaskName(TaskName::CollectingTileMeshes);

    // Enqueue all tiles for processing on the IO thread pool...
    bvector<TileNodeP> tiles = root.GetTiles();
    m_statistics.m_tileCount = tiles.size();
    m_statistics.m_tileDepth = root.GetMaxDepth();

    auto numTotalTiles = static_cast<uint32_t>(tiles.size());
    BeAtomic<uint32_t> numCompletedTiles;

// ###TODO_FACET_COUNT: Make geometry processing thread-safe...
#define MESHTILE_SINGLE_THREADED
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
        m_progressMeter._IndicateProgress(numCompletedTiles, numTotalTiles);
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

    bvector<DRange3d>       subRanges;
    TileDisplayParamsPtr    displayParams = mesh.GetDisplayParamsPtr();

    ComputeSubRanges(subRanges, points, maxPoints, DRange3d::From(points));
    for (auto const& subRange : subRanges)
        {
        auto meshBuilder = TileMeshBuilder::Create(displayParams, 1.0E-6);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TileGenerator::GenerateTiles(TileNodePtr& root, ViewControllerR view, size_t maxPointsPerTile)
    {
    root = TileNode::Create(GetTransformFromDgn());

    // Filter elements by viewed models + categories
    ModelAndCategorySet vset(view);
    if (vset.IsEmpty())
        return Status::NoGeometry;

    m_progressMeter._SetTaskName(TaskName::GeneratingRangeTree);
    m_progressMeter._IndicateProgress(0, 1);
    StopWatch timer(true);

    // Compute union of ranges of all elements
    // ###TODO_FACET_COUNT: Assuming 3d spatial view for now...
    // ###TODO_FACET_COUNT: Split the range query from the additional (customizable) element selection criteria
    static const Utf8CP s_spatialSql =  "SELECT s.MinX,s.MinY,s.MinZ,s.MaxX,s.MaxY,s.MaxZ FROM " BIS_SCHEMA(BIS_CLASS_SpatialIndex) " AS s, "
                                        BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " As g, " BIS_SCHEMA(BIS_CLASS_Element) " AS e "
                                        "WHERE g.ECInstanceId=e.ECInstanceId AND s.ECInstanceId=e.ECInstanceId AND InVirtualSet(?,e.ModelId,g.CategoryId)";

    DRange3d viewRange = DRange3d::NullRange();
    DgnDbR db = view.GetDgnDb();

    auto stmt = db.GetPreparedECSqlStatement(s_spatialSql);
    stmt->BindVirtualSet(1, vset);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        DRange3d elemRange = DRange3d::From(stmt->GetValueDouble(0), stmt->GetValueDouble(1), stmt->GetValueDouble(2),
                                            stmt->GetValueDouble(3), stmt->GetValueDouble(4), stmt->GetValueDouble(5));
        viewRange.Extend(elemRange);
        }

    if (viewRange.IsNull())
        {
        m_statistics.m_collectionTime = timer.GetCurrentSeconds();
        m_progressMeter._IndicateProgress(1, 1);
        return Status::NoGeometry;
        }

    stmt->Reset();
    stmt = nullptr;

    // Collect the tiles
    static const double s_leafTolerance = 0.01;
    double tolerance = pow(viewRange.Volume()/maxPointsPerTile, 1.0/3.0);
    root = TileNode::Create(viewRange, GetTransformFromDgn(), 0, 0, tolerance, nullptr);
    root->ComputeTiles(s_leafTolerance, maxPointsPerTile, vset, db);

    m_statistics.m_collectionTime = timer.GetCurrentSeconds();
    m_progressMeter._IndicateProgress(1, 1);

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t countFacets(DRange3dCR range, VirtualSet const& vset, DgnDbR db, size_t maxFacets)
    {
    static const Utf8CP s_sql =
        "SELECT g.FacetCount,r.MinX,r.MinY,r.MinZ,r.MaxX,r.MaxY,r.MaxZ "
        "FROM " BIS_SCHEMA(BIS_CLASS_SpatialIndex) " AS r JOIN " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g ON (g.ECInstanceId = r.ECInstanceId) "
        "JOIN " BIS_SCHEMA(BIS_CLASS_Element) " AS e ON g.ECInstanceId = e.ECInstanceId "
        "WHERE NOT (r.MinX > ? OR r.MinY > ? OR r.MinZ > ? OR r.MaxX < ? OR r.MaxY < ? OR r.MaxZ < ?) "
        "AND InVirtualSet(?,e.ModelId,g.CategoryId) "
        "ORDER BY g.FacetCount";

    auto stmt = db.GetPreparedECSqlStatement(s_sql);
    stmt->BindDouble(1, range.high.x);
    stmt->BindDouble(2, range.high.y);
    stmt->BindDouble(3, range.high.z);
    stmt->BindDouble(4, range.low.x);
    stmt->BindDouble(5, range.low.y);
    stmt->BindDouble(6, range.low.z);
    stmt->BindVirtualSet(7, vset);

    size_t facetCount = 0;
    while (BE_SQLITE_ROW == stmt->Step() /*&& facetCount <= maxFacets*/) // NB: Caller wants the full facet count...can't halt when hit limit
        {
        DRange3d elRange = DRange3d::From(stmt->GetValueDouble(1), stmt->GetValueDouble(2), stmt->GetValueDouble(3),
                                          stmt->GetValueDouble(4), stmt->GetValueDouble(5), stmt->GetValueDouble(6));
        double elVolume = elRange.Volume();
        if (0.0 == elVolume)
            continue;

        DRange3d intersection;
        intersection.IntersectionOf(elRange, range);
        if (!intersection.IsNull())
            {
            double facetCountDensity = static_cast<double>(stmt->GetValueUInt64(0)) / elVolume;
            facetCount += static_cast<size_t>(facetCountDensity * intersection.Volume());
            }
        }

    stmt->Reset();
    stmt = nullptr;

    return facetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::ComputeTiles(double chordTolerance, size_t maxPointsPerTile, VirtualSet const& vset, DgnDbR db)
    {
    static const size_t s_depthLimit = 0xffff;
    static const double s_targetChildCount = 5.0;

    size_t facetCount = countFacets(m_dgnRange, vset, db, maxPointsPerTile);
    if (facetCount < maxPointsPerTile)
        {
        m_tolerance = chordTolerance;
        }
    else if (m_depth < s_depthLimit)
        {
        bvector<DRange3d> subRanges;
        size_t targetChildFacetCount = static_cast<size_t>(static_cast<double>(facetCount) / s_targetChildCount);
        size_t siblingIndex = 0;

        ComputeSubTiles(subRanges, m_dgnRange, targetChildFacetCount, vset, db);
        for (auto& subRange : subRanges)
            {
            double childTolerance = pow(subRange.Volume() / maxPointsPerTile, 1.0 / 3.0);
            m_children.push_back(TileNode::Create(subRange, m_transformFromDgn, m_depth+1, siblingIndex++, childTolerance, this));
            }

        for (auto& child : m_children)
            child->ComputeTiles(chordTolerance, maxPointsPerTile, vset, db);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::ComputeSubTiles(bvector<DRange3d>& subRanges, DRange3dCR range, size_t maxPointsPerSubTile, VirtualSet const& vset, DgnDbR db)
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
        size_t facetCount = countFacets(bisectRange, vset, db, maxPointsPerSubTile);
        if (facetCount < maxPointsPerSubTile)
            {
            if (facetCount != 0)
                subRanges.push_back(bisectRange);
            }
        else
            {
            ComputeSubTiles(subRanges, bisectRange, maxPointsPerSubTile, vset, db);
            }
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometryProcessor : IGeometryProcessor
{
private:
    IFacetOptionsR      m_facetOptions;
    IFacetOptionsPtr    m_targetFacetOptions;
    DgnElementId        m_curElemId;
    ViewControllerCR    m_view;
    TileGeometryList    m_geometries;
    DRange3d            m_range;
    Transform           m_transformFromDgn;

    void AddGeometry(TileGeometryR geom);
    bool ProcessGeometry(IGeometryR geometry, bool isCurved, SimplifyGraphic& gf);

    virtual IFacetOptionsP _GetFacetOptionsP() override { return &m_facetOptions; }
    virtual void _OutputGraphics(ViewContextR context) override;

    virtual bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    virtual bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    virtual bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    virtual bool _ProcessBody(ISolidKernelEntityCR solid, SimplifyGraphic& gf) override;

    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override {return UnhandledPreference::Facet;}
    virtual UnhandledPreference _GetUnhandledPreference(ISolidKernelEntityCR, SimplifyGraphic&) const override { return UnhandledPreference::Facet; }
public:
    TileGeometryProcessor(ViewControllerCR view, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn)
        : m_facetOptions(facetOptions), m_targetFacetOptions(facetOptions.Clone()), m_view(view), m_range(range), m_transformFromDgn(transformFromDgn)
        {
        m_targetFacetOptions->SetChordTolerance(facetOptions.GetChordTolerance() * transformFromDgn.ColumnXMagnitude());
        }

    TileGeometryList const& GetGeometries() const { return m_geometries; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddGeometry(TileGeometryR geom)
    {
    TileGeometryPtr geomPtr = &geom;
    auto pos = std::lower_bound(m_geometries.begin(), m_geometries.end(), geomPtr,
        [&](TileGeometryPtr const& lhs, TileGeometryPtr const& rhs) { return lhs->GetFacetCountDensity() < rhs->GetFacetCountDensity(); });

    m_geometries.insert(pos, geomPtr);
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
    TileTextureImage::ResolveTexture(*displayParams, m_view.GetDgnDb());

    AddGeometry(*TileGeometry::Create(geom, tf, range, m_curElemId, displayParams, *m_targetFacetOptions, isCurved, m_view.GetDgnDb()));
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
    TileTextureImage::ResolveTexture(*displayParams, m_view.GetDgnDb());

    IGeometryPtr geom = IGeometry::Create(clone);
    AddGeometry(*TileGeometry::Create(*geom, Transform::FromIdentity(), range, m_curElemId, displayParams, *m_targetFacetOptions, false, m_view.GetDgnDb()));

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
    TileTextureImage::ResolveTexture(*displayParams, m_view.GetDgnDb());

    AddGeometry(*TileGeometry::Create(*clone, localTo3mx, range, m_curElemId, displayParams, *m_targetFacetOptions, m_view.GetDgnDb()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::_OutputGraphics(ViewContextR context)
    {
    // ###TODO_FACET_COUNT: Separate range query from add'l criteria; allow add'l criteria to be customized
    ModelAndCategorySet vset(m_view);
    if (vset.IsEmpty())
        return;

    // ###TODO_FACET_COUNT: Support non-spatial views...
    static const Utf8CP s_sql =
        "SELECT r.ECInstanceId "
        "FROM " BIS_SCHEMA(BIS_CLASS_SpatialIndex) " AS r JOIN " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g ON (g.ECInstanceId = r.ECInstanceId) "
        "JOIN " BIS_SCHEMA(BIS_CLASS_Element) " AS e ON g.ECInstanceId = e.ECInstanceId "
        "WHERE NOT (r.MinX > ? OR r.MinY > ? OR r.MinZ > ? OR r.MaxX < ? OR r.MaxY < ? OR r.MaxZ < ?) "
        "AND InVirtualSet(?,e.ModelId,g.CategoryId)";

    DgnDbR db = m_view.GetDgnDb();
    context.SetDgnDb(db);

    auto stmt = db.GetPreparedECSqlStatement(s_sql);
    stmt->BindDouble(1, m_range.high.x);
    stmt->BindDouble(2, m_range.high.y);
    stmt->BindDouble(3, m_range.high.z);
    stmt->BindDouble(4, m_range.low.x);
    stmt->BindDouble(5, m_range.low.y);
    stmt->BindDouble(6, m_range.low.z);
    stmt->BindVirtualSet(7, vset);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        m_curElemId = stmt->GetValueId<DgnElementId>(0);
        context.VisitElement(m_curElemId, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshList TileNode::_GenerateMeshes(ViewControllerCR view, TileGeometry::NormalMode normalMode, bool twoSidedTriangles) const
    {
    static const double s_minRangeBoxSize = 0.5;
    static const double s_vertexToleranceRatio = 1.0;
    static const double s_decimateThresholdPixels = 50.0;
    static const size_t s_maxGeometryIdCount = 0xffff;

    double tolerance = GetTolerance();
    double vertexTolerance = tolerance * s_vertexToleranceRatio;

    // Collect geometry from elements in this node, sorted by facet count density
    IFacetOptionsPtr facetOptions = createTileFacetOptions(tolerance);
    TileGeometryProcessor processor(view, GetDgnRange(), *facetOptions, m_transformFromDgn);

    GeometryProcessor::Process(processor, view.GetDgnDb());

    // Convert to meshes
    MeshBuilderMap builderMap;
    size_t geometryCount = 0;
    DRange3d myTileRange = GetTileRange();
    for (auto& geom : processor.GetGeometries())
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
            builderMap[key] = meshBuilder = TileMeshBuilder::Create(displayParams, vertexTolerance);

        bool isContained = geomRange.IsContained(myTileRange);
        bool doVertexClustering = rangePixels < s_decimateThresholdPixels;

        ++geometryCount;
        bool maxGeometryCountExceeded = geometryCount > s_maxGeometryIdCount;

        if (polyface.IsValid())
            {
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                {
                if (isContained || myTileRange.IntersectsWith(DRange3d::From(visitor->GetPointCP(), static_cast<int32_t>(visitor->Point().size()))))
                    {
                    DgnElementId elemId;
                    if (!maxGeometryCountExceeded)
                        elemId = geom->GetElementId();

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
                    elemId = geom->GetElementId();

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


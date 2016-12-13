/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/ElementTileTree.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/ElementTileTree.h>
#include <folly/BeFolly.h>
#include <DgnPlatform/RangeIndex.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

#if defined(BENTLEYCONFIG_OS_WINDOWS)
#include <windows.h>
#endif

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

USING_NAMESPACE_ELEMENT_TILETREE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureCPtr DisplayParams::QueryTexture(DgnDbR db) const
    {
    JsonRenderMaterial mat;
    if (!m_materialId.IsValid() || SUCCESS != mat.Load(m_materialId, db))
        return nullptr;

    auto texMap = mat.GetPatternMap();
    DgnTextureId texId;
    if (!texMap.IsValid() || !(texId = texMap.GetTextureId()).IsValid())
        return nullptr;

    return DgnTexture::Get(db, texId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams::DisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting) :
    m_fillColor(nullptr != graphicParams ? graphicParams->GetFillColor().GetValue() : 0x00ffffff), m_ignoreLighting (ignoreLighting), m_rasterWidth(nullptr != graphicParams ? graphicParams->GetWidth() : 0)
    {
    if (nullptr != geometryParams)
        {
        m_categoryId = geometryParams->GetCategoryId();
        m_subCategoryId = geometryParams->GetSubCategoryId();
        m_materialId = geometryParams->GetMaterialId();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::operator<(DisplayParamsCR rhs) const
    {
    COMPARE_VALUES (m_fillColor, rhs.m_fillColor);
    COMPARE_VALUES (m_rasterWidth, rhs.m_rasterWidth);                                                           
    COMPARE_VALUES (m_materialId.GetValueUnchecked(), rhs.m_materialId.GetValueUnchecked());

    // Note - do not compare category and subcategory - These are used only for 
    // extracting BRep face attachments.  Comparing them would create seperate
    // meshes for geometry with same symbology but different category.
    // This was determined (empirically) to degrade performance. 

    // No need to compare textures -- if materials match then textures must too.
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource TextureImage::Load(DisplayParamsCR params, DgnDbR db)
    {
    DgnTextureCPtr tex = params.QueryTexture(db);
    return tex.IsValid() ? tex->GetImageSource() : ImageSource();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayParams::ResolveTextureImage(DgnDbR db) const
    {
    if (m_textureImage.IsValid())
        return;

    ImageSource renderImage  = TextureImage::Load(*this, db);

    if (renderImage.IsValid())
        m_textureImage = TextureImage::Create(std::move(renderImage));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetTriangleRange(TriangleCR triangle) const
    {
    return DRange3d::From (m_points.at (triangle.m_indices[0]), 
                           m_points.at (triangle.m_indices[1]),
                           m_points.at (triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d Mesh::GetTriangleNormal(TriangleCR triangle) const
    {
    return DVec3d::FromNormalizedCrossProductToPoints (m_points.at (triangle.m_indices[0]), 
                                                       m_points.at (triangle.m_indices[1]),
                                                       m_points.at (triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Mesh::HasNonPlanarNormals() const
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
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    Mesh::AddMesh (MeshCR mesh)
    {
    if (mesh.m_points.empty() ||
        m_normals.empty() != mesh.m_normals.empty() ||
        m_uvParams.empty() != mesh.m_uvParams.empty() ||
        m_entityIds.empty() != mesh.m_entityIds.empty())
        {
        BeAssert (false && "add mesh empty or not compatible");
        }

    size_t      baseIndex = m_points.size();

    m_points.insert (m_points.end(), mesh.m_points.begin(), mesh.m_points.end());
    if (!mesh.m_normals.empty())
        m_normals.insert (m_normals.end(), mesh.m_normals.begin(), mesh.m_normals.end());

    if (!mesh.m_uvParams.empty())
        m_uvParams.insert (m_uvParams.end(), mesh.m_uvParams.begin(), mesh.m_uvParams.end());

    if (!mesh.m_entityIds.empty())
        m_entityIds.insert (m_entityIds.end(), mesh.m_entityIds.begin(), mesh.m_entityIds.end());

    for (auto& triangle : mesh.m_triangles)
        AddTriangle (Triangle (triangle.m_indices[0] + baseIndex, triangle.m_indices[1] + baseIndex, triangle.m_indices[2] + baseIndex, triangle.m_singleSided));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool    Mesh::RemoveEntityGeometry (bset<DgnElementId> const& deleteIds)
    {
    bool                        deleteGeometryFound = false;
    bmap<uint32_t, uint32_t>    indexRemap;
    bvector<DPoint3d>           savePoints = m_points;
    bvector<DVec3d>             saveNormals = m_normals;
    bvector<DPoint2d>           saveParams = m_uvParams;
    bvector<DgnElementId>       saveEntityIds = m_entityIds;

    m_points.clear();
    m_normals.clear();
    m_uvParams.clear();
    m_entityIds.clear();

    for (size_t index = 0; index<saveEntityIds.size(); index++)
        {
        auto&   entityId = saveEntityIds[index];

        if (deleteIds.find(entityId) == deleteIds.end() &&
            indexRemap.find(index) == indexRemap.end())
            {
            indexRemap.Insert(index, (uint32_t) m_points.size());
            m_points.push_back(savePoints[index]);
            m_entityIds.push_back(entityId);
            if (!saveNormals.empty())
                m_normals.push_back(saveNormals[index]);
            if (!saveParams.empty())
                m_uvParams.push_back(saveParams[index]);
            }
        else
            {
            deleteGeometryFound = true;
            }
        }
    if (!deleteGeometryFound)
        return false;
        

    for (bvector<Triangle>::iterator  triangle = m_triangles.begin(); triangle != m_triangles.end(); )
        {
        if (indexRemap.find(triangle->m_indices[0]) == indexRemap.end())
            {
            m_triangles.erase (triangle);
            }
        else
            {
            for (size_t i=0; i<3; i++)
                triangle->m_indices[i] = indexRemap[triangle->m_indices[i]];

            triangle++;
            }
        }
    for (bvector<Polyline>::iterator  polyline = m_polylines.begin(); polyline != m_polylines.end(); )
        {
        auto& indices = polyline->GetIndices();
        if (indexRemap.find(indices[0]) == indexRemap.end())
            {
            m_polylines.erase (polyline);
            }
        else
            {
            for (size_t i=0; i<indices.size(); i++)
                indices[i] = indexRemap[indices[i]];

            polyline++;
            }                                                                                                         
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Mesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, DgnElementId entityId)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(point);
    m_entityIds.push_back(entityId);

    if (nullptr != normal)
        m_normals.push_back(*normal);
                                                                                                                 

    if (nullptr != param)
        m_uvParams.push_back(*param);

    m_validIdsPresent |= entityId.IsValid();
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TriangleKey::TriangleKey(TriangleCR triangle)
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
bool TriangleKey::operator<(TriangleKeyCR rhs) const
    {
    COMPARE_VALUES (m_sortedIndices[0], rhs.m_sortedIndices[0]);
    COMPARE_VALUES (m_sortedIndices[1], rhs.m_sortedIndices[1]);
    COMPARE_VALUES (m_sortedIndices[2], rhs.m_sortedIndices[2]);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool VertexKey::Comparator::operator()(VertexKeyCR lhs, VertexKeyCR rhs) const
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
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddTriangle(TriangleCR triangle)
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
void MeshBuilder::AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool doVertexCluster, bool duplicateTwoSidedTriangles, bool includeParams)
    {
    auto const&       points = visitor.Point();
    BeAssert(3 == points.size());

    if (doVertexCluster)
        {
        DVec3d      cross;

        cross.CrossProductToPoints (points.at(0), points.at(1), points.at(2));
        if (cross.MagnitudeSquared() < m_areaTolerance)
            return;
        }

    Triangle            newTriangle(!visitor.GetTwoSided());
    bvector<DPoint2d>   params = visitor.Param();

    if (includeParams &&
        !params.empty() &&
        (m_material.IsValid() || (materialId.IsValid() && SUCCESS == m_material.Load (materialId, dgnDb))))
        {
        auto const&         patternMap = m_material.GetPatternMap();
        bvector<DPoint2d>   computedParams;

        if (patternMap.IsValid())
            {
            BeAssert (m_mesh->Points().empty() || !m_mesh->Params().empty());
            if (SUCCESS == patternMap.ComputeUVParams (computedParams, visitor))
                params = computedParams;
            }
        }
            
    bool haveNormals = !visitor.Normal().empty();
    for (size_t i = 0; i < 3; i++)
        {
        VertexKey vertex(points.at(i), haveNormals ? &visitor.Normal().at(i) : nullptr, !includeParams || params.empty() ? nullptr : &params.at(i), entityId);
        newTriangle.m_indices[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
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

            VertexKey vertex(points.at(reverseIndex), haveNormals ? &reverseNormal : nullptr, includeParams || params.empty() ? nullptr : &params.at(reverseIndex), entityId);
            dupTriangle.m_indices[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
            }

        AddTriangle(dupTriangle);
        ++m_triangleIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline (bvector<DPoint3d>const& points, DgnElementId entityId, bool doVertexCluster)
    {
    Polyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, entityId);

        newPolyline.GetIndices().push_back (doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }
    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyface (PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, DgnElementId entityId, bool twoSidedTriangles, bool includeParams)
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        AddTriangle(*visitor, materialId, dgnDb, entityId, false, twoSidedTriangles, includeParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MeshBuilder::AddVertex(VertexKey const& vertex)
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
uint32_t MeshBuilder::AddClusteredVertex(VertexKey const& vertex)
    {
    auto found = m_clusteredVertexMap.find(vertex);
    if (m_clusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_entityId);
    m_clusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPart::GeomPart(DgnGeometryPartId partId, DRange3dCR range, GeometryList const& geometries) : m_partId(partId), m_range (range), m_instanceCount(0), m_facetCount(0), m_geometries(geometries)
    { 
    }                

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomPart::IsCurved() const
    {
    for (auto& geometry : m_geometries)
        if (geometry->IsCurved())
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList GeomPart::GetPolyfaces(IFacetOptionsR facetOptions, GeometryCR instance)
    {
    PolyfaceList polyfaces;
    for (auto& geometry : m_geometries) 
        {
        PolyfaceList thisPolyfaces = geometry->GetPolyfaces (facetOptions);

        if (!thisPolyfaces.empty())
            polyfaces.insert (polyfaces.end(), thisPolyfaces.begin(), thisPolyfaces.end());
        }

    for (auto& polyface : polyfaces)
        polyface.Transform(instance.GetTransform());

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes (IFacetOptionsR facetOptions, GeometryCR instance)
    {
    StrokesList strokes;

    for (auto& geometry : m_geometries) 
        {
        StrokesList   thisStrokes = geometry->GetStrokes(facetOptions);

        if (!thisStrokes.empty())
            strokes.insert (strokes.end(), thisStrokes.begin(), thisStrokes.end());
        }

    for (auto& stroke : strokes)
        stroke.Transform(instance.GetTransform());

    return strokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomPart::GetFacetCount(FacetCounter& counter, GeometryCR instance) const
    {
    if (0 == m_facetCount)
        for (auto& geometry : m_geometries) 
            m_facetCount += geometry->GetFacetCount(counter);
            
    return m_facetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsP params, bool isCurved, DgnDbR db)
    : m_params(params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(nullptr != params && params->QueryTexture(db).IsValid())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Geometry::GetFacetCount(IFacetOptionsR options) const
    {
    if (0 != m_facetCount)
        return m_facetCount;
    
    FacetCounter counter(options);
    return (m_facetCount = _GetFacetCount(counter));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void collectCurveStrokes (bvector<bvector<DPoint3d>>& strokes, CurveVectorCR curve, IFacetOptionsR facetOptions, TransformCR transform)
    {                    
    bvector <bvector<bvector<DPoint3d>>> strokesArray;

    curve.CollectLinearGeometry (strokesArray, &facetOptions);

    for (auto& loop : strokesArray)
        {
        for (auto& loopStrokes : loop)
            {
            transform.Multiply(loopStrokes, loopStrokes);
            strokes.push_back (std::move(loopStrokes));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Strokes::Transform(TransformCR transform)
    {
    for (auto& stroke : m_strokes)
        transform.Multiply (stroke, stroke);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveGeometry : Geometry
{
private:
    IGeometryPtr        m_geometry;

    PrimitiveGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, bool isCurved, DgnDbR db)
        : Geometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry) { }

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override;
    virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) override;
    virtual bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_geometry); }
public:
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, bool isCurved, DgnDbR db)
        {
        return new PrimitiveGeometry(geometry, tf, range, elemId, params, isCurved, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct SolidKernelGeometry : Geometry
{
private:
    IBRepEntityPtr      m_entity;
    BeMutex             m_mutex;

    SolidKernelGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, DgnDbR db)
        : Geometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid) { }

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override;
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_entity); }
public:
    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, DgnDbR db)
        {
        return new SolidKernelGeometry(solid, tf, range, elemId, params, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct TextStringGeometry : Geometry
{
private:
    TextStringPtr                   m_text;
    mutable bvector<CurveVectorPtr> m_glyphCurves;

    TextStringGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, DgnDbR db)
        : Geometry(transform, range, elemId, params, true, db), m_text(&text) 
        { 
        InitGlyphCurves();     // Should be able to defer this when font threaded ness is resolved.
        }

    virtual bool _DoVertexCluster() const override { return false; }

public:
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, DgnDbR db)
        {
        return new TextStringGeometry(textString, transform, range, elemId, params, db);
        }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool     DoGlyphBoxes (IFacetOptionsR facetOptions)
    {
    DRange2d            textRange = m_text->GetRange();
    double              minDimension = std::min (textRange.high.x - textRange.low.x, textRange.high.y - textRange.low.y) * GetTransform().ColumnXMagnitude();
    static const double s_minGlyphRatio = 1.0; 
    
    return minDimension < s_minGlyphRatio * facetOptions.GetChordTolerance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override
    {

    PolyfaceList                polyfaces;
    IPolyfaceConstructionPtr    polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    if (DoGlyphBoxes(facetOptions))
        {
        DVec3d              xAxis, yAxis;
        DgnGlyphCP const*   glyphs = m_text->GetGlyphs();
        DPoint3dCP          glyphOrigins = m_text->GetGlyphOrigins();

        m_text->ComputeGlyphAxes(xAxis, yAxis);
        Transform       rotationTransform = Transform::From (RotMatrix::From2Vectors(xAxis, yAxis));

        for (size_t iGlyph = 0; iGlyph <  m_text->GetNumGlyphs(); ++iGlyph)
            {
            if (nullptr != glyphs[iGlyph])
                {
                DRange2d                range = glyphs[iGlyph]->GetExactRange();
                bvector<DPoint3d>       box(5);

                box[0].x = box[3].x = box[4].x = range.low.x;
                box[1].x = box[2].x = range.high.x;

                box[0].y = box[1].y = box[4].y = range.low.y;
                box[2].y = box[3].y = range.high.y;

                Transform::FromProduct (Transform::From(glyphOrigins[iGlyph]), rotationTransform).Multiply (box, box);

                polyfaceBuilder->AddTriangulation (box);
                }
            }
        }
    else
        {
        for (auto& glyphCurve : m_glyphCurves)
            polyfaceBuilder->AddRegion(*glyphCurve);
        }

    PolyfaceHeaderPtr   polyface = polyfaceBuilder->GetClientMeshPtr();

    if (polyface.IsValid() && polyface->HasFacets())
        {
        polyface->Transform(Transform::FromProduct (GetTransform(), m_text->ComputeTransform()));
        polyfaces.push_back (Polyface(*GetDisplayParams(), *polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) override
    {
    StrokesList strokes;

    if (DoGlyphBoxes(facetOptions))
        return strokes;

    InitGlyphCurves();

    bvector<bvector<DPoint3d>>  strokePoints;
    Transform                   transform = Transform::FromProduct (GetTransform(), m_text->ComputeTransform());

    for (auto& glyphCurve : m_glyphCurves)
        if (!glyphCurve->IsAnyRegionType())
            collectCurveStrokes(strokePoints, *glyphCurve, facetOptions, transform);

    if (!strokePoints.empty())
        strokes.push_back(Strokes(*GetDisplayParams(), std::move(strokePoints)));

    return strokes;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t _GetFacetCount(FacetCounter& counter) const override 
    { 
    InitGlyphCurves();
    size_t              count = 0;

    for (auto& glyphCurve : m_glyphCurves)
        count += counter.GetFacetCount(*glyphCurve);

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/    
void  InitGlyphCurves() const
    {
    if (!m_glyphCurves.empty())
        return;

    DVec3d              xAxis, yAxis;
    DgnGlyphCP const*   glyphs = m_text->GetGlyphs();
    DPoint3dCP          glyphOrigins = m_text->GetGlyphOrigins();

    m_text->ComputeGlyphAxes(xAxis, yAxis);
    Transform       rotationTransform = Transform::From (RotMatrix::From2Vectors(xAxis, yAxis));

    for (size_t iGlyph = 0; iGlyph <  m_text->GetNumGlyphs(); ++iGlyph)
        {
        if (nullptr != glyphs[iGlyph])
            {
            bool            isFilled = false;
            CurveVectorPtr  glyphCurveVector = glyphs[iGlyph]->GetCurveVector(isFilled);

            if (glyphCurveVector.IsValid())
                {
                glyphCurveVector->TransformInPlace (Transform::FromProduct (Transform::From(glyphOrigins[iGlyph]), rotationTransform));
                m_glyphCurves.push_back(glyphCurveVector);
                }
            }
        }                                                                                                           
    }
};  // TextStringGeometry

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct GeomPartInstanceGeometry : Geometry
{
private:
    GeomPartPtr     m_part;

    GeomPartInstanceGeometry(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, DgnDbR db)
        : Geometry(tf, range, elemId, params, part.IsCurved(), db), m_part(&part) { }
public:
    static GeometryPtr Create(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsP params, DgnDbR db)  { return new GeomPartInstanceGeometry(part, tf, range, elemId, params, db); }

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override { return m_part->GetPolyfaces(facetOptions, *this); }
    virtual StrokesList _GetStrokes (IFacetOptionsR facetOptions) override { return m_part->GetStrokes(facetOptions, *this); }
    virtual size_t _GetFacetCount(FacetCounter& counter) const override { return m_part->GetFacetCount (counter, *this); }
    virtual GeomPartCPtr _GetPart() const override { return m_part; }

};  // GeomPartInstanceTileGeometry 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList PrimitiveGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    
    if (polyface.IsValid())
        {
        if (!HasTexture())
            polyface->ClearParameters(false);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return PolyfaceList (1, Polyface(*GetDisplayParams(), *polyface));
        }

    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();

    if (curveVector.IsValid() && !curveVector->IsAnyRegionType())       // Non region curveVectors....
        return PolyfaceList();

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    ISolidPrimitivePtr  solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    if (curveVector.IsValid())
        polyfaceBuilder->AddRegion(*curveVector);
    else if (solidPrimitive.IsValid())
        polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    else if (bsplineSurface.IsValid())
        polyfaceBuilder->Add(*bsplineSurface);

    PolyfaceList    polyfaces;

    polyface = polyfaceBuilder->GetClientMeshPtr();
    if (polyface.IsValid())
        {
        polyface->Transform(GetTransform());
        polyfaces.push_back (Polyface(*GetDisplayParams(), *polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList PrimitiveGeometry::_GetStrokes (IFacetOptionsR facetOptions)
    {
    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();
    StrokesList         tileStrokes;

    if (curveVector.IsValid() && ! curveVector->IsAnyRegionType())
        {
        bvector<bvector<DPoint3d>>  strokePoints;

        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            tileStrokes.push_back(Strokes(*GetDisplayParams(), std::move(strokePoints)));
        }

    return tileStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList SolidKernelGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
    PolyfaceList tilePolyfaces;
#if defined (BENTLEYCONFIG_PARASOLID)    
    // Cannot process the same solid entity simultaneously from multiple threads...
    BeMutexHolder lock(m_mutex);

    DRange3d entityRange = m_entity->GetEntityRange();
    if (entityRange.IsNull())
        return tilePolyfaces;

    double              rangeDiagonal = entityRange.DiagonalDistance();
    static double       s_minRangeRelTol = 1.0e-4;
    double              minChordTolerance = rangeDiagonal * s_minRangeRelTol;
    IFacetOptionsPtr    pFacetOptions = facetOptions.Clone();
    
    if (facetOptions.GetChordTolerance() < minChordTolerance)
        pFacetOptions->SetChordTolerance (minChordTolerance);

    pFacetOptions->SetParamsRequired (true); // Can't rely on HasTexture due to face attached material that may have texture.

    if (nullptr != m_entity->GetFaceMaterialAttachments())
        {
        bvector<PolyfaceHeaderPtr>  polyfaces;
        bvector<FaceAttachment>     params;

        if (!BRepUtil::FacetEntity(*m_entity, polyfaces, params, *pFacetOptions))
            return tilePolyfaces;

        GeometryParams baseParams;

        // Require valid category/subcategory for sub-category appearance color/material...
        baseParams.SetCategoryId(GetDisplayParams()->GetCategoryId());
        baseParams.SetSubCategoryId(GetDisplayParams()->GetSubCategoryId());

        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {
                GeometryParams faceParams;

                params[i].ToGeometryParams(faceParams, baseParams);

                DisplayParamsPtr displayParams = DisplayParams::Create (GetDisplayParams()->GetFillColor(), faceParams);

                tilePolyfaces.push_back (Polyface(*displayParams, *polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (Polyface(*GetDisplayParams(), *polyface));

        }

    if (!GetTransform().IsIdentity())
        for (auto& tilePolyface : tilePolyfaces)
            tilePolyface.m_polyface->Transform (GetTransform());

    return tilePolyfaces;

#else
    return tilePolyfaces;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsP params, DgnDbR db)
    {
    return TextStringGeometry::Create(textString, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsP params, bool isCurved, DgnDbR db)
    {
    return PrimitiveGeometry::Create(geometry, tf, range, entityId, params, isCurved, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsP params, DgnDbR db)
    {
    return SolidKernelGeometry::Create(solid, tf, range, entityId, params, db);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(GeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsP params, DgnDbR db)
    {
    return GeomPartInstanceGeometry::Create(part, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Loader::_GetFromSource()
    {
    return ERROR; // ###TODO
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::_LoadTile()
    {
    return ERROR; // ###TODO
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnModelR model, TransformCR transform)
    : T_Super(model.GetDgnDb(), transform, "", nullptr), m_modelId(model.GetModelId()), m_name(model.GetName())
    {
    // ###TODO: Determine range...
    m_range = DRange3d::NullRange();
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
Tile::Tile(Root& octRoot, TileId id, Tile const* parent, bool isLeaf)
    : T_Super(root, id, parent, isLeaf)
    {
    // ###TODO: Determine range...
    m_range = DRange3d::NullRange();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TileLoaderPtr Tile::_CreateTileLoader(TileTree::TileLoadStatePtr loads)
    {
    return new Loader(*this, loads);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TilePtr Tile::_CreateChild(TileId childId) const
    {
    return new Tile(GetElementRoot(), childId, this);
    }


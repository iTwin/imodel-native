/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MeshTile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <folly/BeFolly.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

USING_NAMESPACE_BENTLEY_RENDER

BEGIN_UNNAMED_NAMESPACE

constexpr double s_half2dDepthRange = 10.0;

static ITileGenerationProgressMonitor   s_defaultProgressMeter;

static const double s_minRangeBoxSize    = 1.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh
static const double s_minToleranceRatio  = 256.0;   // Nominally the screen size of a tile.  Increasing generally increases performance (fewer draw calls) at expense of higher load times.

END_UNNAMED_NAMESPACE

//#define POINT_SUPPORT

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerationCache::TileGenerationCache(Options options) : m_range(DRange3d::NullRange()), m_options(options),
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerationCache::~TileGenerationCache()
    {
    //
    }


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct RangeAccumulator
{
private:
    void AddElement(DRange3dCR range, DgnElementId elemId, bool is2d)
        {
        TileElementEntry entry(range, elemId, is2d);
        if (entry.m_sizeSq > 0.0)
            {
            m_range.Extend(range);
            m_elements.insert(entry);
            }
        }

    void Accumulate3d()
        {
        auto stmt = m_model.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId,Origin,Yaw,Pitch,Roll,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE Model.Id=?");
        stmt->BindId(1, m_model.GetModelId());
        while (BE_SQLITE_ROW == stmt->Step())
            {
            if (stmt->IsValueNull(1)) // has no placement
                continue;

            double yaw   = stmt->GetValueDouble(2);
            double pitch = stmt->GetValueDouble(3);
            double roll  = stmt->GetValueDouble(4);

            DPoint3d low = stmt->GetValuePoint3d(5);
            DPoint3d high = stmt->GetValuePoint3d(6);

            Placement3d placement(stmt->GetValuePoint3d(1),
                                  YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)),
                                  ElementAlignedBox3d(low.x, low.y, low.z, high.x, high.y, high.z));

            AddElement(placement.CalculateRange(), stmt->GetValueId<DgnElementId>(0), false);
            }
        }

    void Accumulate2d()
        {
        auto stmt = m_model.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId,Origin,Rotation,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE Model.Id=?");
        stmt->BindId(1, m_model.GetModelId());
        while (BE_SQLITE_ROW == stmt->Step())
            {
            if (stmt->IsValueNull(1)) // has no placement
                continue;

            DPoint2d low  = stmt->GetValuePoint2d(3);
            DPoint2d high = stmt->GetValuePoint2d(4);

            Placement2d placement(stmt->GetValuePoint2d(1),
                                  AngleInDegrees::FromDegrees(stmt->GetValueDouble(2)),
                                  ElementAlignedBox2d(low.x, low.y, high.x, high.y));

            AddElement(placement.CalculateRange(), stmt->GetValueId<DgnElementId>(0), true);
            }

        if (!m_range.IsNull())
            {
            m_range.low.z = -s_half2dDepthRange*2;  // times 2 so we don't stick geometry right on the boundary...
            m_range.high.z = s_half2dDepthRange*2;
            }
        }
public:
    TileElementSet& m_elements;
    DRange3dR       m_range;
    GeometricModel& m_model;

    RangeAccumulator(DRange3dR range, GeometricModel& model, TileElementSet& elements) : m_elements(elements), m_range(range), m_model(model)
        {
        m_range = DRange3d::NullRange();
        }

    TileGeneratorStatus Accumulate()
        {
        if (m_model.Is2dModel())
            Accumulate2d();
        else
            Accumulate3d();

        return m_elements.empty() ? TileGeneratorStatus::NoGeometry : TileGeneratorStatus::Success;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TileGenerationCache::Populate(DgnDbR db, DgnModelR model)       
    {
    m_model = &model;
    auto geomModel = model.ToGeometricModelP();
    if (nullptr == geomModel)
        return TileGeneratorStatus::NoGeometry;

    RangeAccumulator accum(m_range, *geomModel, m_elements);
    return accum.Accumulate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureCPtr TileDisplayParams::QueryTexture(DgnDbR db) const
    {
    RenderMaterialCPtr material = RenderMaterial::Get(db, m_materialId);
    if (!material.IsValid())
        return nullptr;

    auto& mat = material->GetRenderingAsset();
    auto texMap = mat.GetPatternMap();
    DgnTextureId texId;
    if (!texMap.IsValid() || !(texId = texMap.GetTextureId()).IsValid())
        return nullptr;

    return DgnTexture::Get(db, texId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileDisplayParams::TileDisplayParams(GraphicParamsCP graphicParams, GeometryParamsCP geometryParams, bool ignoreLighting, bool useLineParams) :
                   m_color(0x00ffffff), m_ignoreLighting (ignoreLighting), m_rasterWidth(0), m_linePixels(0)
    {
    if (nullptr != geometryParams)
        {
        m_categoryId = geometryParams->GetCategoryId();
        m_subCategoryId = geometryParams->GetSubCategoryId();
        m_materialId = geometryParams->GetMaterialId();
        m_class = geometryParams->GetGeometryClass();
        m_isColorFromBackground = geometryParams->IsFillColorFromViewBackground();
        }
    if (nullptr != graphicParams)
        {
        if (useLineParams)
            {
            m_color            = graphicParams->GetLineColor().GetValue();
            m_rasterWidth      = graphicParams->GetWidth();
            m_linePixels       = graphicParams->GetLinePixels();
            }
        else
            {
            m_color = graphicParams->GetFillColor().GetValue(); 
            if (nullptr != graphicParams->GetGradientSymb())
                {
                m_gradient = GradientSymb::Create();
                m_gradient->CopyFrom(*graphicParams->GetGradientSymb());
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::IsLessThan(TileDisplayParams const& rhs, bool compareColor) const
    {
    if (m_ignoreLighting != rhs.m_ignoreLighting)
        return m_ignoreLighting;

    if (m_isColorFromBackground != rhs.m_isColorFromBackground)
        return m_isColorFromBackground != rhs.m_isColorFromBackground;

    if (m_linePixels != rhs.m_linePixels)
        return m_linePixels < rhs.m_linePixels;                                                                                        

    if (m_gradient.get() != rhs.m_gradient.get())
        {
        if (!m_gradient.IsValid() || !rhs.m_gradient.IsValid() || ! (*m_gradient == *rhs.m_gradient))
            return m_gradient.get() < rhs.m_gradient.get();
        }

    if (m_color != rhs.m_color)
        {
        if (compareColor)
            return m_color < rhs.m_color;

        // cannot batch translucent and opaque meshes
        ColorDef lhsColor(m_color), rhsColor(rhs.m_color);
        bool lhsHasAlpha = 0 != lhsColor.GetAlpha(),
             rhsHasAlpha = 0 != rhsColor.GetAlpha();

        if (lhsHasAlpha != rhsHasAlpha)
            return lhsHasAlpha;

        }

    if (m_rasterWidth != rhs.m_rasterWidth)
        return m_rasterWidth < rhs.m_rasterWidth;

    if (m_materialId.GetValueUnchecked() != rhs.m_materialId.GetValueUnchecked())
        return m_materialId.GetValueUnchecked() < rhs.m_materialId.GetValueUnchecked();

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
ImageSource TileTextureImage::Load(TileDisplayParamsCR params, DgnDbR db)
    {
    DgnTextureCPtr tex = params.QueryTexture(db);
    return tex.IsValid() ? tex->GetImageSource() : ImageSource();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileTextureImagePtr TileTextureImage::Create(GradientSymbCR gradient)
    {
    static const size_t     s_size = 256;

    return TileTextureImage::Create(Render::ImageSource (gradient.GetImage(s_size, s_size), Render::ImageSource::Format::Png), false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileDisplayParams::ResolveTextureImage(DgnDbR db) const
    {
    if (m_textureImage.IsValid())
        return;

    if (m_gradient.IsValid())
        {
        m_textureImage = TileTextureImage::Create(*m_gradient);
        return;
        }

    ImageSource renderImage  = TileTextureImage::Load(*this, db);

    if (renderImage.IsValid())
        m_textureImage = TileTextureImage::Create(std::move(renderImage));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d TileMesh::GetTriangleRange(TileTriangleCR triangle) const
    {
    return DRange3d::From (m_points.at (triangle.m_indices[0]), 
                           m_points.at (triangle.m_indices[1]),
                           m_points.at (triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d TileMesh::GetTriangleNormal(TileTriangleCR triangle) const
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

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMesh::AddTriMesh(Render::TriMeshArgs const& triMesh, TransformCR transform, bool invertVParam)
    {
    m_points.resize(triMesh.m_numPoints);

    if (nullptr != triMesh.m_normals)
        m_normals.resize(triMesh.m_numPoints);

    if (nullptr != triMesh.m_textureUV)
        m_uvParams.resize(triMesh.m_numPoints);

    for (uint32_t i=0; i<triMesh.m_numPoints; i++)
        {
        DPoint3d unquantizedPoint = triMesh.m_points[i].Unquantize(triMesh.m_pointParams);
        transform.Multiply (m_points.at(i), unquantizedPoint.x, unquantizedPoint.y, unquantizedPoint.z);

        if (nullptr != triMesh.m_normals)
            m_normals[i] = triMesh.m_normals[i].Decode();

        if (nullptr != triMesh.m_textureUV)
            m_uvParams[i].Init((double) triMesh.m_textureUV[i].x, (double) (invertVParam ? (1.0 - triMesh.m_textureUV[i].y) : triMesh.m_textureUV[i].y));
        }
#define DEFAULT_ATTRIBUTE_VALUE 1
#ifdef DEFAULT_ATTRIBUTE_VALUE
        m_attributes.resize(triMesh.m_numPoints);

        for (uint32_t i=0; i<triMesh.m_numPoints; i++)
            m_attributes[i] = DEFAULT_ATTRIBUTE_VALUE;

        m_validIdsPresent = true;
#endif
    
    for (uint32_t i=0; i<triMesh.m_numIndices; i += 3)
        AddTriangle(TileTriangle(triMesh.m_vertIndex[i], triMesh.m_vertIndex[i+1], triMesh.m_vertIndex[i+2], false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    TileMesh::AddMesh (TileMeshCR mesh)
    {
    if (mesh.m_points.empty() ||
        m_normals.empty() != mesh.m_normals.empty() ||
        m_uvParams.empty() != mesh.m_uvParams.empty() ||
        m_attributes.empty() != mesh.m_attributes.empty())
        {
        BeAssert (false && "add mesh empty or not compatible");
        }
    size_t      baseIndex = m_points.size();

    m_points.insert (m_points.end(), mesh.m_points.begin(), mesh.m_points.end());
    if (!mesh.m_normals.empty())
        m_normals.insert (m_normals.end(), mesh.m_normals.begin(), mesh.m_normals.end());

    if (!mesh.m_uvParams.empty())
        m_uvParams.insert (m_uvParams.end(), mesh.m_uvParams.begin(), mesh.m_uvParams.end());

    if (!mesh.m_attributes.empty())
        m_attributes.insert(m_attributes.end(), mesh.m_attributes.begin(), mesh.m_attributes.end());

    for (auto& triangle : mesh.m_triangles)
        AddTriangle (TileTriangle (triangle.m_indices[0] + baseIndex, triangle.m_indices[1] + baseIndex, triangle.m_indices[2] + baseIndex, triangle.m_singleSided));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshPointCloud::TileMeshPointCloud(TileDisplayParamsCR params, DPoint3dCP points, Rgb const* colors, FPoint3dCP normals, size_t nPoints, TransformCR transform, double clusterTolerance) :  m_displayParams(&params)
    {
    TileUtil::PointComparator                       comparator(clusterTolerance);
    bset <DPoint3d, TileUtil::PointComparator>      clusteredPointSet(comparator);

    for (size_t i=0; i<nPoints; i++)
        {
        DPoint3d        testPoint;

        transform.Multiply (testPoint, points[i]);

        if (clusteredPointSet.find(testPoint) == clusteredPointSet.end())
            {
            m_points.push_back(testPoint);
            if (nullptr != colors)
                m_colors.push_back(colors[i]);

            if (nullptr != normals)
                m_normals.push_back(normals[i]);
    
            clusteredPointSet.insert(testPoint);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool    TileMesh::RemoveEntityGeometry (bset<DgnElementId> const& deleteIds)
    {
#if defined(TODO_ATTRIBUTES)
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
        

    for (bvector<TileTriangle>::iterator  triangle = m_triangles.begin(); triangle != m_triangles.end(); )
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
    for (bvector<TilePolyline>::iterator  polyline = m_polylines.begin(); polyline != m_polylines.end(); )
        {
        if (indexRemap.find(polyline->m_indices[0]) == indexRemap.end())
            {
            m_polylines.erase (polyline);
            }
        else
            {
            for (size_t i=0; i<polyline->m_indices.size(); i++)
                polyline->m_indices[i] = indexRemap[polyline->m_indices[i]];

            polyline++;
            }                                                                                                         
        }
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, uint32_t attribute, uint32_t color)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(point);
    m_attributes.push_back(attribute);

    if (nullptr != normal)
        m_normals.push_back(*normal);

    if (nullptr != param)
        m_uvParams.push_back(*param);
    else
        m_colors.push_back(m_colorIndex.GetIndex(color));

    m_validIdsPresent |= (0 != attribute);
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::VertexKey::Comparator::operator()(VertexKey const& lhs, VertexKey const& rhs) const
    {
    static const double s_normalTolerance = .1;     
    static const double s_paramTolerance  = .1;

    COMPARE_VALUES(lhs.m_color, rhs.m_color);

    COMPARE_VALUES (lhs.m_attributes.GetElementId(), rhs.m_attributes.GetElementId());
    COMPARE_VALUES (lhs.m_attributes.GetSubCategoryId(), rhs.m_attributes.GetSubCategoryId());
    COMPARE_VALUES (lhs.m_attributes.GetClass(), rhs.m_attributes.GetClass());

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
TileMeshBuilder::TileTriangleKey::TileTriangleKey(TileTriangleCR triangle)
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
bool TileMeshBuilder::TileTriangleKey::operator<(TileTriangleKey const& rhs) const
    {
    COMPARE_VALUES (m_sortedIndices[0], rhs.m_sortedIndices[0]);
    COMPARE_VALUES (m_sortedIndices[1], rhs.m_sortedIndices[1]);
    COMPARE_VALUES (m_sortedIndices[2], rhs.m_sortedIndices[2]);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(TileTriangleCR triangle)
    {
    if (triangle.IsDegenerate())
        return;

    TileTriangleKey key(triangle);

    if (m_triangleSet.insert(key).second)
        m_mesh->AddTriangle(triangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileMeshBuilder::GetMaterial(RenderMaterialId materialId, DgnDbR dgnDb)
    {
    if (!materialId.IsValid())
        return false;

    m_materialEl = RenderMaterial::Get(dgnDb, materialId);
    BeAssert(m_materialEl.IsValid());
    m_material = &m_materialEl->GetRenderingAsset();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddTriangle(PolyfaceVisitorR visitor, RenderMaterialId materialId, DgnDbR dgnDb, FeatureAttributesCR attributes, bool includeParams, uint32_t fillColor, bool requireNormals)
    {
    auto const&         points = visitor.Point();
    bool const*         visitorVisibility = visitor.GetVisibleCP();
    size_t              nTriangles = points.size() - 2;

    if (requireNormals && visitor.Normal().size() < points.size())
        return; // TFS#790263: Degenerate triangle - ignore

    for (size_t iTriangle =0; iTriangle< nTriangles; iTriangle++)
        {
        TileTriangle        newTriangle(!visitor.GetTwoSided());
        bvector<DPoint2d>   params = visitor.Param();

        newTriangle.m_edgeVisible[0] = (0 == iTriangle) ? visitorVisibility[0] : false;
        newTriangle.m_edgeVisible[1] = visitorVisibility[iTriangle+1];
        newTriangle.m_edgeVisible[2] = (iTriangle == nTriangles-1) ? visitorVisibility[iTriangle+2] : false;

        if (includeParams &&
            !params.empty() &&
            (m_material || GetMaterial(materialId, dgnDb)))
            {
            auto patternMap = m_material->GetPatternMap();
            bvector<DPoint2d>   computedParams;

            if (patternMap.IsValid())
                {
                BeAssert (m_mesh->Points().empty() || !m_mesh->Params().empty());
                if (SUCCESS == patternMap.ComputeUVParams (computedParams, visitor, &m_transformToDgn))
                    params = computedParams;
                }
            }
                
        for (size_t i = 0; i < 3; i++)
            {
            size_t index = (0 == i) ? 0 : iTriangle + i; 
            VertexKey vertex(points.at(index), requireNormals ? &visitor.Normal().at(index) : nullptr, !includeParams || params.empty() ? nullptr : &params.at(index), attributes, fillColor);
            newTriangle.m_indices[i] = AddVertex(vertex);
            }

        BeAssert(m_mesh->Params().empty() || m_mesh->Params().size() == m_mesh->Points().size());
        BeAssert(m_mesh->Normals().empty() || m_mesh->Normals().size() == m_mesh->Points().size());

        AddTriangle(newTriangle);
        ++m_triangleIndex;
        }
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyline (bvector<DPoint3d>const& points, FeatureAttributesCR attributes, uint32_t fillColor)
    {
    TilePolyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, attributes, fillColor);
        newPolyline.m_indices.push_back (AddVertex(vertex));
        }

    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileMeshBuilder::AddPolyface (PolyfaceQueryCR polyface, RenderMaterialId materialId, DgnDbR dgnDb, FeatureAttributesCR attributes, bool includeParams, uint32_t fillColor)
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        AddTriangle(*visitor, materialId, dgnDb, attributes,  includeParams, fillColor, nullptr != polyface.GetNormalCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t TileMeshBuilder::AddVertex(VertexKey const& vertex)
    {
    auto found = m_vertexMap.find(vertex);
    if (m_vertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), m_attributes.GetIndex(vertex.m_attributes), vertex.m_color);
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
WString TileNode::GetFileName (WCharCP rootName, WCharCP extension) const
    {
    WString     fileName;

    BeFileName::BuildName (fileName, nullptr, nullptr, (rootName + GetNameSuffix()).c_str(), extension);

    return fileName;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr TileGenerator::CreateTileFacetOptions(double chordTolerance)
    {
    static double       s_defaultAngleTolerance = msGeomConst_piOver2;
    IFacetOptionsPtr    opts = IFacetOptions::Create();

    opts->SetChordTolerance(chordTolerance);
    opts->SetAngleTolerance(s_defaultAngleTolerance);
    opts->SetMaxPerFace(100);
    opts->SetConvexFacetsRequired(true);
    opts->SetCurvedSurfaceMaxPerFace(3);
    opts->SetParamsRequired(true);
    opts->SetNormalsRequired(true);
    opts->SetBsplineSurfaceEdgeHiding(0);

    // Avoid Parasolid concurrency bottlenecks.
    opts->SetIgnoreHiddenBRepEntities(true);
    opts->SetOmitBRepEdgeChainIds(true);

    return opts;
    }

typedef bmap<TileMeshMergeKey, TileMeshBuilderPtr> MeshBuilderMap;

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
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileNodeCP TileNode::GetRoot() const
    {
    auto cur = this;
    auto parent = cur->GetParent();
    while (nullptr != parent)
        {
        cur = parent;
        parent = cur->GetParent();
        }

    return cur;
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

/*---------------------- -----------------------------------------------------------**//**
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
TileGeometry::TileGeometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, bool isCurved, DgnDbR db)
    : m_params(&params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(params.HasTexture(db))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TileGeometry::GetFacetCount(IFacetOptionsR options) const
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
void TileGeometry::TileStrokes::Transform(TransformCR transform)
    {
    for (auto& stroke : m_strokes)
        transform.Multiply (stroke, stroke);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveTileGeometry : TileGeometry
{
private:
    IGeometryPtr        m_geometry;
    bool                m_curvesAsWire;

    PrimitiveTileGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, bool isCurved, bool curvesAsWire, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry), m_curvesAsWire(curvesAsWire) { }

    T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override;
    bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override;

    static PolyfaceHeaderPtr FixPolyface(PolyfaceHeaderR, IFacetOptionsR);
    static void AddNormals(PolyfaceHeaderR, IFacetOptionsR);
    static void AddParams(PolyfaceHeaderR, IFacetOptionsR);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t _GetFacetCount(FacetCounter& counter) const override 
    { 
    // Limit polyfaces to count to 10000 facets - This may make more of them appear in leaves
    // but else they can cause overly deep trees (as their count is not dependent on tolerance).
    // Scene_3d from TFS# 805023 - XFrog trees.
    constexpr       size_t      s_maxPolyfaceCount = 10000;
    size_t          facetCount =  counter.GetFacetCount(*m_geometry);

    return (m_geometry->GetAsPolyfaceHeader().IsValid()) ? std::min(s_maxPolyfaceCount, facetCount) : facetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isDisjointCurvePrimitive(ICurvePrimitiveCR prim)
    {
    switch (prim.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            return true;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCR segment = *prim.GetLineCP();
            return segment.IsAlmostSinglePoint();
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const& points = *prim.GetLineStringCP();
            return 1 == points.size() || (2 == points.size() && points[0].AlmostEqual(points[1]));
            }
        default:
            return false;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool _IsPoint() const override
    {
#ifdef POINT_SUPPORT
    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();
    double              length = 0.0;

    if (!curveVector.IsValid() ||
        1 != curveVector->size() ||
        !isDisjointCurvePrimitive(*curveVector->front()))
        return false;
#endif
    return false;
    }

public:
    static TileGeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, bool isCurved, bool curvesAsWire, DgnDbR db)
        {
        return new PrimitiveTileGeometry(geometry, tf, range, elemId, params, isCurved, curvesAsWire, db);
        }
};

    
//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct SolidKernelTileGeometry : TileGeometry
{
private:
    IBRepEntityPtr      m_entity;
    BeMutex             m_mutex;
#if defined (BENTLEYCONFIG_PARASOLID) 
    // Otherwise, 'unused private member' warning from clang...
    DgnDbR              m_db;
#endif

    SolidKernelTileGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid)
#if defined (BENTLEYCONFIG_PARASOLID) 
        , m_db(db)
#endif
        {
#if defined (BENTLEYCONFIG_PARASOLID)    
        PK_BODY_change_partition(PSolidUtil::GetEntityTag (*m_entity), PSolidThreadUtil::GetThreadPartition());
#endif
        }

    T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override;
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_entity); }
public:
    static TileGeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        {
        return new SolidKernelTileGeometry(solid, tf, range, elemId, params, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct TextStringTileGeometry : TileGeometry
{
private:
    TextStringPtr                   m_text;
    mutable bvector<CurveVectorPtr> m_glyphCurves;

    TextStringTileGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        : TileGeometry(transform, range, elemId, params, true, db), m_text(&text) 
        { 
        InitGlyphCurves();     // Should be able to defer this when font threaded ness is resolved.
        }

public:
    static TileGeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        {
        return new TextStringTileGeometry(textString, transform, range, elemId, params, db);
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
T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override
    {
    T_TilePolyfaces             polyfaces;
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
        polyfaces.push_back (TileGeometry::TilePolyface (GetDisplayParams(), *polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override
    {
    T_TileStrokes               strokes;

    if (DoGlyphBoxes(facetOptions))
        return strokes;

    InitGlyphCurves();

    bvector<bvector<DPoint3d>>  strokePoints;
    Transform                   transform = Transform::FromProduct (GetTransform(), m_text->ComputeTransform());

    for (auto& glyphCurve : m_glyphCurves)
        if (!glyphCurve->IsAnyRegionType())
            collectCurveStrokes(strokePoints, *glyphCurve, facetOptions, transform);

    if (!strokePoints.empty())             
        strokes.push_back (TileGeometry::TileStrokes (GetDisplayParams(), std::move(strokePoints)));

    return strokes;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t _GetFacetCount(FacetCounter& counter) const override 
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
};  // TextStringTileGeometry


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct GeomPartInstanceTileGeometry : TileGeometry
{
private:
    TileGeomPartPtr     m_part;

    GeomPartInstanceTileGeometry(TileGeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)
        : TileGeometry(tf, range, elemId, params, part.IsCurved(), db), m_part(&part) { }

public:
    static TileGeometryPtr Create(TileGeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, TileDisplayParamsCR params, DgnDbR db)  { return new GeomPartInstanceTileGeometry(part, tf, range, elemId, params, db); }

    T_TilePolyfaces _GetPolyfaces(IFacetOptionsR facetOptions) override { return m_part->GetPolyfaces(facetOptions, *this); }
    T_TileStrokes _GetStrokes (IFacetOptionsR facetOptions) override { return m_part->GetStrokes(facetOptions, *this); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return m_part->GetInstanceCount() == 1 ? m_part->GetFacetCount (counter) : 0; }  // Only count a single definition rather than true instanced count. (TFS# 805023).
    TileGeomPartCPtr _GetPart() const override { return m_part; }

};  // GeomPartInstanceTileGeometry 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
static int     compareDoubleArray (double const* array1, double const* array2, size_t count, double tolerance)
    {
    for (uint32_t i=0; i<count; i++, array1++, array2++)
        if (*array1 < *array2 - tolerance)
            return -1;
        else if (*array1 > *array2 + tolerance)
            return 1;

    return 0;
    }

    static const double     s_compareTolerance = 1.0E-5;

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct SolidPrimitivePartMapKey 
{
    ISolidPrimitivePtr      m_solidPrimitive;
    DRange3d                m_range;
    TileDisplayParamsCPtr   m_displayParams;


    SolidPrimitivePartMapKey() { }
    SolidPrimitivePartMapKey(ISolidPrimitiveR solidPrimitive, DRange3dCR range, TileDisplayParamsCR displayParams) : m_range(range), m_solidPrimitive(&solidPrimitive), m_displayParams(&displayParams) { }

    bool operator < (SolidPrimitivePartMapKey const& rhs) const { return compareDoubleArray (&m_range.low.x, &rhs.m_range.low.x, 6, s_compareTolerance) < 0; }
    
    bool IsEqual (SolidPrimitivePartMapKey const& other) const { return !(*m_displayParams < *other.m_displayParams) && !(*other.m_displayParams < *m_displayParams) && m_solidPrimitive->IsSameStructureAndGeometry(*other.m_solidPrimitive, s_compareTolerance); }

};



//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct  SolidPrimitivePartMap : bmultimap<SolidPrimitivePartMapKey,  TileGeomPartPtr>
{
    TileGeomPartPtr Find (SolidPrimitivePartMapKey const& key)
        {
        auto const&   range = equal_range (key);

        for (iterator curr = range.first; curr != range.second; ++curr)
            {
            if (curr->first.IsEqual(key))
                return curr->second;
            }

        return TileGeomPartPtr();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db)
    {
    return TextStringTileGeometry::Create(textString, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, bool isCurved, bool curvesAsWire, DgnDbR db)
    {
    return PrimitiveTileGeometry::Create(geometry, tf, range, entityId, params, isCurved, curvesAsWire, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db)
    {
    return SolidKernelTileGeometry::Create(solid, tf, range, entityId, params, db);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometryPtr TileGeometry::Create(TileGeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, TileDisplayParamsCR params, DgnDbR db)
    {
    return GeomPartInstanceTileGeometry::Create(part, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PrimitiveTileGeometry::FixPolyface(PolyfaceHeaderR geom, IFacetOptionsR facetOptions)
    {
    // Avoid IPolyfaceConstruction if possible...AddPolyface_matched() does a ton of expensive remapping which is unnecessary for our use case.
    // (Plus we can avoid cloning the input if caller owns it)
    PolyfaceHeaderPtr polyface(&geom);
    size_t maxPerFace;
    if (geom.GetNumFacet(maxPerFace) > 0 && (int)maxPerFace > facetOptions.GetMaxPerFace())
        {
        IPolyfaceConstructionPtr builder = PolyfaceConstruction::New(facetOptions);
        builder->AddPolyface(geom);
        polyface = &builder->GetClientMeshR();
        }
    else
        {
        bool addNormals = facetOptions.GetNormalsRequired() && 0 == geom.GetNormalCount(),
             addParams = facetOptions.GetParamsRequired() && 0 == geom.GetParamCount(),
             addFaceData = addParams && 0 == geom.GetFaceCount();

        if (addNormals)
            AddNormals(*polyface, facetOptions);

        if (addParams)
            AddParams(*polyface, facetOptions);

        if (addFaceData)
            polyface->BuildPerFaceFaceData();

        if (!geom.HasConvexFacets() && facetOptions.GetConvexFacetsRequired())
            polyface->Triangulate(3);
        }

    return polyface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveTileGeometry::AddNormals(PolyfaceHeaderR polyface, IFacetOptionsR facetOptions)
    {
    static double s_defaultCreaseRadians = Angle::DegreesToRadians(45.0);
    static double s_defaultConeRadians = Angle::DegreesToRadians(90.0);
    polyface.BuildApproximateNormals(s_defaultCreaseRadians, s_defaultConeRadians, facetOptions.GetHideSmoothEdgesWhenGeneratingNormals());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveTileGeometry::AddParams(PolyfaceHeaderR polyface, IFacetOptionsR facetOptions)
    {
    LocalCoordinateSelect selector;
    switch (facetOptions.GetParamMode())
        {
        case FACET_PARAM_01BothAxes:
            selector = LOCAL_COORDINATE_SCALE_01RangeBothAxes;
            break;
        case FACET_PARAM_01LargerAxis:
            selector = LOCAL_COORDINATE_SCALE_01RangeLargerAxis;
            break;
        default:
            selector = LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft;
            break;
        }

    polyface.BuildPerFaceParameters(selector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces PrimitiveTileGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    
    if (polyface.IsValid())
        {
        if (!HasTexture())
            polyface->ClearParameters(false);

        polyface = FixPolyface(*polyface, facetOptions);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return TileGeometry::T_TilePolyfaces (1, TileGeometry::TilePolyface (GetDisplayParams(), *polyface));
        }

    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();

    if (curveVector.IsValid() && !curveVector->IsAnyRegionType())       // Non region curveVectors....
        return T_TilePolyfaces();

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    ISolidPrimitivePtr  solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    if (curveVector.IsValid() && !m_curvesAsWire)
        polyfaceBuilder->AddRegion(*curveVector);
    else if (solidPrimitive.IsValid())
        polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    else if (bsplineSurface.IsValid())
        polyfaceBuilder->Add(*bsplineSurface);

    T_TilePolyfaces     polyfaces;

    polyface = polyfaceBuilder->GetClientMeshPtr();
    if (polyface.IsValid())
        {
        polyface->Transform(GetTransform());
        polyfaces.push_back (TileGeometry::TilePolyface (GetDisplayParams(), *polyface));
        }
    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TileStrokes PrimitiveTileGeometry::_GetStrokes (IFacetOptionsR facetOptions)
    {
    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();
    T_TileStrokes       tileStrokes;

    if (!curveVector.IsValid())
        return tileStrokes;
    
    if (curveVector.IsValid() && (m_curvesAsWire || !curveVector->IsAnyRegionType()))
        {
        bvector<bvector<DPoint3d>>  strokePoints;

        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            tileStrokes.push_back (TileGeometry::TileStrokes (GetDisplayParams(), std::move(strokePoints), (1 == curveVector->size() && isDisjointCurvePrimitive(*curveVector->front()))));
        }

    return tileStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces SolidKernelTileGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
#if defined (BENTLEYCONFIG_PARASOLID)    
    // Set mark so that we can roll back if severe error occurs.
    PSolidThreadUtil::SetThreadPartitionMark();

    // Cannot process the same solid entity simultaneously from multiple threads...
    BeMutexHolder lock(m_mutex);

    DRange3d entityRange = m_entity->GetEntityRange();
    if (entityRange.IsNull())
        return TileGeometry::T_TilePolyfaces();;

    double              rangeDiagonal = entityRange.DiagonalDistance();
    static double       s_minRangeRelTol = 1.0e-4;
    double              minChordTolerance = rangeDiagonal * s_minRangeRelTol;
    IFacetOptionsPtr    pFacetOptions = facetOptions.Clone();
    
    if (facetOptions.GetChordTolerance() < minChordTolerance)
        pFacetOptions->SetChordTolerance (minChordTolerance);

    pFacetOptions->SetParamsRequired (true); // Can't rely on HasTexture due to face attached material that may have texture.

    TileGeometry::T_TilePolyfaces   tilePolyfaces;

    if (nullptr != m_entity->GetFaceMaterialAttachments())
        {
        bvector<PolyfaceHeaderPtr>  polyfaces;
        bvector<FaceAttachment>     params;

        if (!BRepUtil::FacetEntity(*m_entity, polyfaces, params, *pFacetOptions))
            return TileGeometry::T_TilePolyfaces();;

        GeometryParams baseParams;

        // Require valid category/subcategory for sub-category appearance color/material...
        baseParams.SetCategoryId(GetDisplayParams().GetCategoryId());
        baseParams.SetSubCategoryId(GetDisplayParams().GetSubCategoryId());
        baseParams.SetGeometryClass(GetDisplayParams().GetClass());

        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {                                                                    
                GeometryParams faceParams;
                params[i].ToGeometryParams(faceParams, baseParams);
                faceParams.Resolve(m_db);

                TileDisplayParamsCPtr displayParams = TileDisplayParams::Create(GetDisplayParams().GetColor(), faceParams);
                tilePolyfaces.push_back (TileGeometry::TilePolyface (*displayParams, *polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (TileGeometry::TilePolyface (GetDisplayParams(), *polyface));

        }

    if (!GetTransform().IsIdentity())
        for (auto& tilePolyface : tilePolyfaces)
            tilePolyface.m_polyface->Transform (GetTransform());

    return tilePolyfaces;

#else
    return TileGeometry::T_TilePolyfaces();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces TileGeometry::GetPolyfaces(double chordTolerance, NormalMode normalMode)
    {
    return _GetPolyfaces (*CreateFacetOptions(chordTolerance, normalMode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr TileGeometry::CreateFacetOptions(double chordTolerance, NormalMode normalMode) const
    {
    auto facetOptions = TileGenerator::CreateTileFacetOptions(chordTolerance / m_transform.ColumnXMagnitude());
    bool normalsRequired = false;

    switch (normalMode)
        {
        case NormalMode::Always:    
            normalsRequired = true; 
            break;
        case NormalMode::CurvedSurfacesOnly:    
            normalsRequired = m_isCurved; 
            break;
        }

    facetOptions->SetNormalsRequired(normalsRequired);
    facetOptions->SetParamsRequired(HasTexture());

    return facetOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::TileGenerator(DgnDbR dgndb, ITileCollectionFilterCP filter, ITileGenerationProgressMonitorP progress)
    : m_progressMeter(nullptr != progress ? *progress : s_defaultProgressMeter), m_dgndb(dgndb), m_totalTiles(0), m_totalModels(0), m_completedModels(0), m_filter(filter)
    {
#if defined(WIP_MESHTILE_3SM)
    m_spatialTransformFromDgn.InitIdentity();
#else
    DPoint3d origin = dgndb.GeoLocation().GetProjectExtents().GetCenter();
    m_spatialTransformFromDgn = Transform::From(-origin.x, -origin.y, -origin.z);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TileGenerator::GenerateTiles(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile)
    {
    auto nModels = static_cast<uint32_t>(modelIds.size());
    if (0 == nModels)
        return TileGeneratorStatus::NoGeometry;

    // unused - auto nCompletedModels = 0;

#if defined (BENTLEYCONFIG_PARASOLID) 
    PSolidKernelManager::StartSession();
    PSolidThreadUtil::MainThreadMark    psolidMainThreadMark;
#endif

    StopWatch timer(true);

    m_totalModels = nModels;
    m_progressMeter._IndicateProgress(0, nModels);

    auto future = GenerateTilesFromModels(collector, modelIds, leafTolerance, surfacesOnly, maxPointsPerTile);
    /* unused - auto status = */ future.get();

    m_completedModels.store(0);
    m_totalModels = 0;

    m_statistics.m_tileGenerationTime = timer.GetCurrentSeconds();
    m_statistics.m_tileCount = m_totalTiles;
    m_totalTiles.store(0);

    m_progressMeter._IndicateProgress(nModels, nModels);

    return TileGeneratorStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTilesFromModels(ITileCollector& collector, DgnModelIdSet const& modelIds, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile)
    {
    std::vector<FutureStatus> modelFutures;
    for (auto const& modelId : modelIds)
        {
        auto model = GetDgnDb().Models().GetModel(modelId);
        if (model.IsValid()) // && 0xca == modelId.GetValue())
            modelFutures.push_back(GenerateTiles(collector, leafTolerance, surfacesOnly, maxPointsPerTile, *model));
        }

    return folly::unorderedReduce(modelFutures, TileGeneratorStatus::Success, [=](TileGeneratorStatus reduced, TileGeneratorStatus next)
        {
        return TileGeneratorStatus::Aborted == reduced || TileGeneratorStatus::Aborted == next ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
        });
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::GenerateTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model)
    {
    // ###TODO: This is not ready for primetime...
#ifdef GENERATE_FROM_TILE_TREE_DIRECTLY
    DgnModelPtr         modelPtr(&model);
    auto                pCollector = &collector;
    auto                generateMeshTiles = dynamic_cast<IGenerateMeshTiles*>(&model);
    GeometricModelP     geometricModel = model.ToGeometricModelP();
    bool                isModel3d = nullptr != geometricModel->ToGeometricModel3d();

    if (!isModel3d)
        surfacesOnly = false;


    if (nullptr == geometricModel)
        {
        BeAssert (false);
        return folly::makeFuture(TileGeneratorStatus::NoGeometry);
        }

    double          rangeDiagonal = geometricModel->QueryModelRange().DiagonalDistance();
    double          minDiagonalToleranceRatio = isModel3d ? 1.0E-3 : 1.0E-5;   // Don't allow leaf tolerance to be less than this factor times range diagonal.
    static  double  s_minLeafTolerance = 1.0E-6;

    leafTolerance = std::max(s_minLeafTolerance, std::min(leafTolerance, rangeDiagonal * minDiagonalToleranceRatio));

    return GenerateTilesFromTileTree (&collector, leafTolerance, surfacesOnly, geometricModel);
#else
    DgnModelPtr         modelPtr(&model);
    auto                pCollector = &collector;
    auto                generateMeshTiles = dynamic_cast<IGenerateMeshTiles*>(&model);
    auto                getTileTree = dynamic_cast<IGetTileTreeForPublishing*>(&model); // ###TODO: empty interface; remove this once no longer needed
    auto                getPublishedURL = dynamic_cast<IGetPublishedTilesetInfo*>(&model);
    GeometricModelP     geometricModel = model.ToGeometricModelP();
    bool                isModel3d = nullptr != geometricModel->ToGeometricModel3d();
    
    if (!isModel3d)
        surfacesOnly = false;

    if (nullptr != geometricModel)
        {
        double          rangeDiagonal = geometricModel->QueryModelRange().DiagonalDistance();
        double          minDiagonalToleranceRatio = isModel3d ? 1.0E-3 : 1.0E-5;   // Don't allow leaf tolerance to be less than this factor times range diagonal.
        static  double  s_minLeafTolerance = 1.0E-6;

        leafTolerance = std::max(s_minLeafTolerance, std::min(leafTolerance, rangeDiagonal * minDiagonalToleranceRatio));
        }


    if (nullptr != getTileTree)
        {
        // ###TODO: Change point clouds to go through this path instead of _GenerateMeshTiles below.
        if (getTileTree->_AllowPublishing())                                                                                                                                                                                                      
            return GenerateTilesFromTileTree(&collector, leafTolerance, surfacesOnly, geometricModel);
        else if (nullptr != getPublishedURL)
            return collector._AcceptPublishedTilesetInfo(model, *getPublishedURL);
        else
            return TileGeneratorStatus::NoGeometry;
        }
    else if (nullptr != generateMeshTiles)
        {
        return folly::via(&BeFolly::ThreadPool::GetCpuPool(), [=]()
            {
            auto status = pCollector->_BeginProcessModel(*modelPtr);
            TileNodePtr root;
            if (TileGeneratorStatus::Success == status)
                {
                if (root.IsValid())
                    m_totalTiles += root->GetNodeCount();

                status = generateMeshTiles->_GenerateMeshTiles(root, m_spatialTransformFromDgn, leafTolerance, *pCollector, GetProgressMeter());
                }

            m_progressMeter._IndicateProgress(++m_completedModels, m_totalModels);
            return pCollector->_EndProcessModel(*modelPtr, root.get(), status);
            });
        }
    else
        {
        BeFileName          dataDirectory;

        return folly::via(&BeFolly::ThreadPool::GetCpuPool(), [=]()
            {
            return pCollector->_BeginProcessModel(*modelPtr);
            })
        .then([=](TileGeneratorStatus status)
            {
            if (TileGeneratorStatus::Success == status)
                return GenerateElementTiles(*pCollector, leafTolerance, surfacesOnly, maxPointsPerTile, *modelPtr);

            return folly::makeFuture(GenerateTileResult(status, nullptr));
            })
        .then([=](GenerateTileResult result)
            {
            if (result.m_tile.IsValid())
                m_totalTiles += result.m_tile->GetNodeCount();

            m_progressMeter._IndicateProgress(++m_completedModels, m_totalModels);
            return pCollector->_EndProcessModel(*modelPtr, result.m_tile.get(), result.m_status);
            })
        .then([=](TileGeneratorStatus status)
            {
            return status;                           
            });
        }

#endif
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::GenerateElementTiles(ITileCollector& collector, double leafTolerance, bool surfacesOnly, size_t maxPointsPerTile, DgnModelR model)
    {
    auto                cache = TileGenerationCache::Create(TileGenerationCache::Options::None);
    ElementTileContext  context(*cache, model, collector, leafTolerance, model.Is3dModel() && surfacesOnly, maxPointsPerTile);

    return PopulateCache(context).then([=](TileGeneratorStatus status)
        {
        return GenerateTileset(status, context);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureStatus TileGenerator::PopulateCache(ElementTileContext context)
    {
    return folly::via(&BeFolly::ThreadPool::GetCpuPool(), [=]                                                         
        {
        return context.m_cache->Populate(GetDgnDb(), *context.m_model);
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Transform   TileGenerator::GetTransformFromDgn(DgnModelCR model) const
    {
    return model.IsSpatialModel() ?  GetSpatialTransformFromDgn() : Transform::FromIdentity(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::GenerateTileset(TileGeneratorStatus status, ElementTileContext context)
    {
    auto& cache = *context.m_cache;
    auto sheet = context.m_model->ToSheetModel();

    DRange3d        range = cache.GetRange();
    if (nullptr != sheet)
        range.Extend(sheet->GetSheetExtents());

    if (TileGeneratorStatus::Success != status && nullptr == sheet)
        {
        GenerateTileResult result(status, ElementTileNode::Create(*context.m_model, range, GetTransformFromDgn(*context.m_model), 0, 0, nullptr).get());
        return folly::makeFuture(result);
        }
    
    ElementTileNodePtr parent = ElementTileNode::Create(*context.m_model, range, GetTransformFromDgn(*context.m_model), 0, 0, nullptr);
    return ProcessParentTile(parent, context).then([=](GenerateTileResult result) { return ProcessChildTiles(result.m_status, parent, context); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::ProcessParentTile (ElementTileNodePtr parent, ElementTileContext context)
    {
    return folly::via(&BeFolly::ThreadPool::GetCpuPool(), [=]()
        {
    #if defined (BENTLEYCONFIG_PARASOLID) 
        PSolidThreadUtil::WorkerThreadOuterMark     psolidWorkerThreadOuterMark;
    #endif

        auto& tile = *parent;
        auto& collector = *context.m_collector;
        auto leafTolerance = context.m_leafTolerance;
        auto maxPointsPerTile = context.m_maxPointsPerTile;
        auto const& generationCache = *context.m_cache;

        double          tileTolerance = tile.GetDgnRange().DiagonalDistance() / s_minToleranceRatio;
        bool            isLeaf = tileTolerance < leafTolerance && parent->GetChildren().empty();
        bool            leafThresholdExceeded = false;

        // Always collect geometry at the target leaf tolerance.
        // If maxPointsPerTile is exceeded, we will keep that geometry, but adjust this tile's target tolerance
        // Later that tolerance will be used in _GenerateMeshes() to facet appropriately (and to filter out 
        // elements too small to be included in this tile)
        tile.SetTolerance(leafTolerance);
        tile.CollectGeometry(generationCache, m_dgndb, &leafThresholdExceeded, leafTolerance, tileTolerance, context.m_surfacesOnly, isLeaf ? 0 : maxPointsPerTile, m_filter); // ###TODO: Check return status

        if (!isLeaf && !leafThresholdExceeded)
            isLeaf = true;

        GenerateTileResult result(m_progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success, tile.GetRoot());

        // If all the geometry was too small for this tile's tolerance, and geometry collected at leaf tolerance exceeded max facet count,
        // produce an empty parent tile and process children...
        tile.SetIsEmpty(tile.GetGeometries().empty());
        if (tile.GetIsEmpty() && isLeaf)
            return result;

        tile.SetIsLeaf(isLeaf);

        if (isLeaf)
            {
            collector._AcceptTile(tile);
            tile.ClearGeometry();

            return result;
            }

        bvector<DRange3d>   subRanges;

        tile.ComputeChildTileRanges(subRanges, tile.GetDgnRange());
        for (auto& subRange : subRanges)
            {
            ElementTileNodePtr child = ElementTileNode::Create(tile.GetModel(), subRange, tile.GetTransformFromDgn(), tile.GetDepth()+1, tile.GetChildren().size(), &tile);

            tile.GetChildren().push_back(child);
            }

        tile.SetTolerance(tileTolerance);

        collector._AcceptTile(tile);
        tile.ClearGeometry();

        result.m_status = m_progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
        return result;
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementTileNode::AdjustTolerance(double newTolerance)
    {
    if (newTolerance <= GetTolerance())
        return;

    // Change the tolerance at which _GetPolyface() will facet
    SetTolerance(newTolerance);

    // Remove any geometries too small for inclusion in this tile
    double minRangeDiagonal = s_minRangeBoxSize * newTolerance;
    auto eraseAt = std::remove_if(m_geometries.begin(), m_geometries.end(), [=](TileGeometryPtr const& geom) { return geom->GetTileRange().DiagonalDistance() < minRangeDiagonal; });
    if (eraseAt != m_geometries.end())
        m_geometries.erase(eraseAt, m_geometries.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::FutureGenerateTileResult TileGenerator::ProcessChildTiles(TileGeneratorStatus status, ElementTileNodePtr parent, ElementTileContext context)
    {
    auto root = static_cast<ElementTileNodeP>(parent->GetRoot());
    if (parent->GetChildren().empty() || TileGeneratorStatus::Success != status)
        return folly::makeFuture(GenerateTileResult(status, root));

    std::vector<FutureGenerateTileResult> childFutures;
    for (auto& child : parent->GetChildren())
        {
        auto elemChild = static_cast<ElementTileNodeP>(child.get());
        auto childFuture = ProcessParentTile(elemChild, context).then([=](GenerateTileResult result) { return ProcessChildTiles(result.m_status, elemChild, context); });
        childFutures.push_back(std::move(childFuture));
        }

    auto result = GenerateTileResult(status, root);
    return folly::unorderedReduce(childFutures, result, [=](GenerateTileResult, GenerateTileResult)
        {
        return GenerateTileResult(m_progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success, root);
        });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileNode::ComputeChildTileRanges(bvector<DRange3d>& subRanges, DRange3dCR range, size_t splitCount)
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
            m_blob = stmt.GetValueBlob(columnIndex);
            m_size = stmt.GetColumnBytes(columnIndex);
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

    DgnDbR _GetSourceDgnDb() const override { return m_db; }
    DgnElementCP _ToElement() const override { return nullptr; }
    GeometrySource3dCP _GetAsGeometrySource3d() const override { return this; }
    DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    Placement3dCR _GetPlacement() const override { return m_placement; }

    DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    DgnDbStatus _SetPlacement(Placement3dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
public:
    static std::unique_ptr<GeometrySource> Create(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement3dCR placement)
        {
        std::unique_ptr<GeometrySource> pSrc(new TileGeometrySource3d(categoryId, db, geomBlob, placement));
        if (!static_cast<TileGeometrySource3d const&>(*pSrc).IsGeometryValid())
            return nullptr;

        return pSrc;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct TileGeometrySource2d : TileGeometrySource, GeometrySource2d
{
private:
    Placement2d     m_placement;

    TileGeometrySource2d(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement2dCR placement)
        : TileGeometrySource(categoryId, db, geomBlob), m_placement(placement) { }

    DgnDbR _GetSourceDgnDb() const override { return m_db; }
    DgnElementCP _ToElement() const override { return nullptr; }
    GeometrySource2dCP _GetAsGeometrySource2d() const override { return this; }
    DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    Placement2dCR _GetPlacement() const override { return m_placement; }

    DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    DgnDbStatus _SetPlacement(Placement2dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
public:
    static std::unique_ptr<GeometrySource> Create(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement2dCR placement)
        {
        std::unique_ptr<GeometrySource> pSrc(new TileGeometrySource2d(categoryId, db, geomBlob, placement));
        if (!static_cast<TileGeometrySource2d const&>(*pSrc).IsGeometryValid())
            return nullptr;

        return pSrc;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometrySelector3d
{
    static bool Is3d() { return true; }

    static Utf8CP GetSql()
        {
        return "SELECT CategoryId,GeometryStream,Yaw,Pitch,Roll,Origin_X,Origin_Y,Origin_Z,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z FROM "
                BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHERE ElementId=?";
        }

    static std::unique_ptr<GeometrySource> ExtractGeometrySource(BeSQLite::CachedStatement& stmt, DgnDbR db)
        {
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

        DPoint3d origin = DPoint3d::From(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7)),
                 boxLo  = DPoint3d::From(stmt.GetValueDouble(8), stmt.GetValueDouble(9), stmt.GetValueDouble(10)),
                 boxHi  = DPoint3d::From(stmt.GetValueDouble(11), stmt.GetValueDouble(12), stmt.GetValueDouble(13));

        Placement3d placement(origin,
                YawPitchRollAngles(Angle::FromDegrees(stmt.GetValueDouble(2)), Angle::FromDegrees(stmt.GetValueDouble(3)), Angle::FromDegrees(stmt.GetValueDouble(4))),
                ElementAlignedBox3d(boxLo.x, boxLo.y, boxLo.z, boxHi.x, boxHi.y, boxHi.z));

        return TileGeometrySource3d::Create(categoryId, db, geomBlob, placement);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometrySelector2d
{
    static bool Is3d() { return false; }

    static Utf8CP GetSql()
        {
        return "SELECT CategoryId,GeometryStream,Rotation,Origin_X,Origin_Y,BBoxLow_X,BBoxLow_Y,BBoxHigh_X,BBoxHigh_Y FROM "
                BIS_TABLE(BIS_CLASS_GeometricElement2d) " WHERE ElementId=?";
        }

    static std::unique_ptr<GeometrySource> ExtractGeometrySource(BeSQLite::CachedStatement& stmt, DgnDbR db)
        {
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

        auto rotation = AngleInDegrees::FromDegrees(stmt.GetValueDouble(2));
        DPoint2d origin = DPoint2d::From(stmt.GetValueDouble(3), stmt.GetValueDouble(4));
        ElementAlignedBox2d bbox(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7), stmt.GetValueDouble(8));

        Placement2d placement(origin, rotation, bbox);
        return TileGeometrySource2d::Create(categoryId, db, geomBlob, placement);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometryProcessor : IGeometryProcessor
{
private:
    typedef bmap<DgnGeometryPartId, TileGeomPartPtr>  GeomPartMap;

    IFacetOptionsR              m_leafFacetOptions;
    IFacetOptionsPtr            m_targetFacetOptions;
    mutable IFacetOptionsPtr    m_lineStyleFacetOptions;
    DgnElementId                m_curElemId;
    TileGenerationCacheCR       m_cache;
    TileDisplayParamsCache      m_displayParamsCache;
    DgnDbR                      m_dgndb;
    TileGeometryList&           m_tileGeometries;
    mutable TileGeometryList    m_leafGeometries;
    DRange3d                    m_range;
    DRange3d                    m_tileRange;
    Transform                   m_transformFromDgn;
    TileGeometryList            m_curTileGeometries;
    TileGeometryList            m_curLeafGeometries;
    double                      m_minRangeDiagonal;
    double                      m_minTextBoxSize;
    bool*                       m_leafThresholdExceeded;
    size_t                      m_leafCountThreshold;
    size_t                      m_leafCount;
    bool                        m_is2d;
    bool                        m_surfacesOnly;
    GeomPartMap                 m_geomParts;
    SolidPrimitivePartMap       m_solidPrimitiveParts;
    ITileCollectionFilterCP     m_filter;
    bool                        m_processingPart = false;

    void AddElementGeometry(TileGeometryR geom);
    bool ProcessGeometry(IGeometryR geometry, bool isCurved, bool curvesAsWire, SimplifyGraphic& gf);

    IFacetOptionsP _GetFacetOptionsP() override { return &GetFacetOptions(); }
    IFacetOptionsR GetFacetOptions() const { return LeafThresholdExceeded() ? *m_targetFacetOptions : m_leafFacetOptions; }
    IFacetOptionsP GetLineStyleFacetOptions(LineStyleSymbCR) const;

    bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf) override;
    bool _ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) override;
    bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& gf) override;
    bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) override;
    bool _ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) override;
    bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;

    double _AdjustZDepth(double zDepthIn) override
        {
        // zDepth is obtained from GeometryParams::GetNetDisplayPriority(), which returns an int32_t.
        // Coming from mstn, priorities tend to be in [-500..500]
        // Let's assume that mstn's range is the full range and clamp anything outside that.
        // Map them to [-s_half2dDepthRange, s_half2dDepthRange]
        constexpr double priorityRange = 500;
        constexpr double ratio = s_half2dDepthRange / priorityRange;

        auto zDepth = std::min(zDepthIn, priorityRange);
        zDepth = std::max(zDepth, -priorityRange);

        return zDepth * ratio;
        }

    IncompatiblePolyfacePreference _GetIncompatiblePolyfacePreference(PolyfaceQueryCR, SimplifyGraphic&) const override
        {
        // We'll fix it up if necessary in PrimitiveTileGeometry::FixPolyface() - otherwise Brien uses IPolyfaceConstruction which is slow and eats memory.
        return IncompatiblePolyfacePreference::Original;
        }

    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override { return UnhandledPreference::Facet; }
    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }
    UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&)     const override { return UnhandledPreference::Facet; }

public:
    TileGeometryProcessor(TileGeometryList& geometries, TileGenerationCacheCR cache, DgnDbR db, ITileCollectionFilterCP filter, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, bool* leafThresholdExceeded, double tolerance, bool surfacesOnly, size_t leafCountThreshold, bool is2d) 
        : m_tileGeometries(geometries), m_leafFacetOptions(facetOptions), m_targetFacetOptions(facetOptions.Clone()), m_cache(cache), m_dgndb(db), m_filter(filter), m_range(range), m_transformFromDgn(transformFromDgn),
          m_leafThresholdExceeded(leafThresholdExceeded), m_leafCountThreshold(leafCountThreshold), m_leafCount(0), m_is2d(is2d), m_surfacesOnly (surfacesOnly)
        {
        static const double s_minTextBoxToleranceRatio = 1.0;           // Below this ratio to tolerance text is rendered as box.
        //static const double s_minLineStyleWidthToleranceRatio = 1.0;     // Below this ratio to tolerance line styles are rendered as continuous.

        double targetTolerance = tolerance * transformFromDgn.ColumnXMagnitude();
        m_targetFacetOptions->SetChordTolerance(targetTolerance);
        m_minRangeDiagonal = s_minRangeBoxSize * targetTolerance;
        m_minTextBoxSize  = s_minTextBoxToleranceRatio * targetTolerance;

        m_transformFromDgn.Multiply (m_tileRange, m_range);
        }

    void ProcessElement(ViewContextR context, DgnElementId elementId, DRange3dCR range);
    TileGeneratorStatus OutputGraphics(ViewContextR context);

    void AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext);
    bool IsGeomPartContained (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic) const;
    bool _DoLineStyleStroke(Render::LineStyleSymbCR lineStyleSymb, IFacetOptionsPtr&, SimplifyGraphic&) const override;

    DgnDbR GetDgnDb() const { return m_dgndb; }
    TileGenerationCacheCR GetCache() const { return m_cache; }
    DRange3dCR GetRange() const { return m_range; }

    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any elements with range smaller than roughly half a pixel...
        return range.DiagonalDistance() < m_minRangeDiagonal;
        }
    
    void PushCurrentGeometry()
        {
        for (auto& geom : m_curLeafGeometries)
            m_leafGeometries.push_back(geom);

        for (auto& geom : m_curTileGeometries)
            m_tileGeometries.push_back(geom);
        }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     11/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    UnhandledPreference _GetUnhandledPreference(TextStringCR textString, SimplifyGraphic& simplifyGraphic) const override 
        {
        DRange2d        range = textString.GetRange();
        Transform       transformToTile = Transform::FromProduct(m_transformFromDgn, simplifyGraphic.GetLocalToWorldTransform(), textString.ComputeTransform());
        double          minTileDimension = transformToTile.ColumnXMagnitude() * std::min(range.XLength(), range.YLength());

        return minTileDimension < m_minTextBoxSize ? UnhandledPreference::Box : UnhandledPreference::Curve;
        }

    bool LeafThresholdExceeded() const { return nullptr != m_leafThresholdExceeded && *m_leafThresholdExceeded; }
    void SetLeafThresholdExceeded() const
        {
        if (nullptr != m_leafThresholdExceeded)
            {
            *m_leafThresholdExceeded = true;
            m_leafGeometries.clear();
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_DoLineStyleStroke(Render::LineStyleSymbCR lsSymb, IFacetOptionsPtr& facetOptions, SimplifyGraphic& gf) const
    {
    facetOptions = GetLineStyleFacetOptions(lsSymb);
    return facetOptions.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsP TileGeometryProcessor::GetLineStyleFacetOptions(LineStyleSymbCR lsSymb) const
    {
    if (!lsSymb.GetUseStroker())
        return nullptr;

    // NB: Initially we collect geometry as if this were a leaf tile.
    // However large styled curves will quickly cause us to exceed our leaf facet count threshold.
    // So stroke only if the line width is at least 5 pixels at tile (not leaf) tolerance, or if
    // we have not yet exceeded the threshold.
    // GetFacetOptions() returns the options for leaf tolerance if threshold not exceeded; for tile tolerance otherwise.
    double pixelSize = GetFacetOptions().GetChordTolerance();
    double maxWidth = lsSymb.GetStyleWidth();
    constexpr double pixelThreshold = 5.0;

    // ###TODO: width of 0 indicates what?
    if (0.0 != pixelSize && 0.0 != maxWidth && maxWidth / pixelSize < pixelThreshold)
        return nullptr;

    if (m_lineStyleFacetOptions.IsNull())
        {
        m_lineStyleFacetOptions = IFacetOptions::CreateForCurves();
        m_lineStyleFacetOptions->SetAngleTolerance(Angle::FromDegrees(5.0).Radians());
        }

    return m_lineStyleFacetOptions.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddElementGeometry(TileGeometryR geom)
    {
    if (m_processingPart)
        {
        // We're just accumulating part geometry into our list - afterward we will use AddElementGeometry to
        // actually add the part instance. Our tile range is temporarily out of sync with our transform
        // anyway so we can't determine intersection, and geom's facet count is currently unavailable
        m_curTileGeometries.push_back(&geom);
        return;
        }

    bool tooSmall = !geom.IsPoint() && BelowMinRange(geom.GetTileRange());
    if (tooSmall && LeafThresholdExceeded())
        return;

    if (nullptr != m_leafThresholdExceeded)
        {
        DRange3d intersection = DRange3d::FromIntersection (geom.GetTileRange(), m_tileRange, true);
        if (intersection.IsNull())
            return;

        if (!geom.IsPoint())
            {
            // We don't publish point strings. Avoid divide-by-zero below.
            // Was causing TFS#799974 - division caused facetCount to become huge, preventing subdivision from ever terminating.
            double tileRangeDiagonalDistance = geom.GetTileRange().DiagonalDistance();
            if (0.0 >= tileRangeDiagonalDistance)
                return;

            double facetCount = static_cast<double>(geom.GetFacetCount(*m_targetFacetOptions)) * intersection.DiagonalDistance() / tileRangeDiagonalDistance;
            BeAssert(facetCount >= 0.0);

            m_leafCount += static_cast<size_t>(facetCount);
            if (m_leafCount > m_leafCountThreshold)
                SetLeafThresholdExceeded();
            }
        }
        
    if (!tooSmall)
        m_curTileGeometries.push_back(&geom);
    else if (!LeafThresholdExceeded())
        m_curLeafGeometries.push_back(&geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::IsGeomPartContained (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic) const
    {
    Transform               partToTile = Transform::FromProduct(m_transformFromDgn, graphic.GetLocalToWorldTransform(), subToGraphic);
    DRange3d                partTileRange;

    partToTile.Multiply (partTileRange, geomPart.GetBoundingBox());

    return partTileRange.IsContained (m_tileRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::AddGeomPart (Render::GraphicBuilderR graphic, DgnGeometryPartCR geomPart, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams, ViewContextR viewContext)
    {
    TileGeomPartPtr         tileGeomPart;
    Transform               partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
    TileDisplayParamsCR     displayParams = m_displayParamsCache.Get(graphicParams, geomParams);
    DRange3d                range;
    auto const&             foundPart = m_geomParts.find (geomPart.GetId());

    if (foundPart == m_geomParts.end())
        {
        Transform                       inverseLocalToWorld;
        AutoRestore<Transform>          saveTransform (&m_transformFromDgn, Transform::FromIdentity());
        AutoRestore<bool>               saveProcessingPart(&m_processingPart, true);
        GeometryStreamIO::Collection    collection(geomPart.GetGeometryStream().GetData(), geomPart.GetGeometryStream().GetSize());
        
        inverseLocalToWorld.InverseOf (graphic.GetLocalToWorldTransform());

        auto                            partBuilder = graphic.CreateSubGraphic(inverseLocalToWorld);
        TileGeometryList                saveLeafGeometries = m_curLeafGeometries;
        TileGeometryList                saveTileGeometries = m_curTileGeometries;
        
        m_curLeafGeometries.clear();
        m_curTileGeometries.clear();
        collection.Draw(*partBuilder, viewContext, geomParams, false, &geomPart);

        for (auto& leafGeom : m_curLeafGeometries)
            m_curTileGeometries.push_back(leafGeom);

        m_geomParts.Insert (geomPart.GetId(), tileGeomPart = TileGeomPart::Create(geomPart.GetBoundingBox(), m_curTileGeometries));
        m_curLeafGeometries = saveLeafGeometries;
        m_curTileGeometries = saveTileGeometries;
        }
    else
        {
        tileGeomPart = foundPart->second;
        }

    tileGeomPart->IncrementInstanceCount();

    Transform   tf = Transform::FromProduct(m_transformFromDgn, partToWorld);
    
    tf.Multiply(range, tileGeomPart->GetRange());
    AddElementGeometry(*TileGeometry::Create(*tileGeomPart, tf, range, m_curElemId, displayParams, m_dgndb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeomPart::TileGeomPart(DRange3dCR range, TileGeometryList const& geometries) :  m_range (range), m_instanceCount(0), m_facetCount(0), m_geometries(geometries)
    { 
    }                

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeomPart::IsWorthInstancing (double chordTolerance) const
    {
    static size_t               s_minInstanceCount = 2;
    static size_t               s_minFacetCompression = 50000;

    if (GetInstanceCount() < s_minInstanceCount)
        return false;

    auto            facetOptions = TileGenerator::CreateTileFacetOptions(chordTolerance);
    FacetCounter    counter(*facetOptions);
    size_t          facetCount = GetFacetCount(counter);

    return (m_instanceCount - 1) * facetCount > s_minFacetCompression;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeomPart::IsCurved() const
    {
    for (auto& geometry : m_geometries)
        if (geometry->IsCurved())
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TilePolyfaces TileGeomPart::GetPolyfaces(IFacetOptionsR facetOptions, TileGeometryCR instance)
    {
    TileGeometry::T_TilePolyfaces polyfaces;
    
    for (auto& geometry : m_geometries) 
        {
        TileGeometry::T_TilePolyfaces thisPolyfaces = geometry->GetPolyfaces (facetOptions);

        for (auto& thisPolyface : thisPolyfaces)
            {
            auto    polyface = thisPolyface.Clone();

            polyface.Transform(instance.GetTransform());
            polyfaces.push_back (polyface);
            }
        }


    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeometry::T_TileStrokes TileGeomPart::GetStrokes (IFacetOptionsR facetOptions, TileGeometryCR instance)
    {
    TileGeometry::T_TileStrokes strokes;

    for (auto& geometry : m_geometries) 
        {
        TileGeometry::T_TileStrokes   thisStrokes = geometry->GetStrokes(facetOptions);

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
size_t TileGeomPart::GetFacetCount(FacetCounter& counter) const
    {
    if (0 == m_facetCount)
        for (auto& geometry : m_geometries) 
            m_facetCount += geometry->GetFacetCount(counter);
            
    return m_facetCount;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileGeometryProcessor::ProcessElement(ViewContextR context, DgnElementId elemId, DRange3dCR dgnRange)
    {
    if (nullptr != m_filter && !m_filter->_AcceptElement(elemId))
        return;
    

    try
        {
        m_curTileGeometries.clear();
        m_curLeafGeometries.clear();
        bool haveCached = m_cache.GetCachedGeometry(m_curTileGeometries, elemId);
        if (!haveCached)
            {
            m_curElemId = elemId;
            context.VisitElement(elemId, false);
            }

        PushCurrentGeometry();
        if (!haveCached)
            m_cache.AddCachedGeometry(elemId, std::move(m_curTileGeometries));
        }
    catch (...)
        {
        // This shouldn't be necessary - but an uncaught interception will cause the processing to continue forever. 
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::ProcessGeometry(IGeometryR geom, bool isCurved, bool curvesAsWire, SimplifyGraphic& gf)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;   // ignore and continue

    auto tf = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    tf.Multiply(range, range);
    
    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d /* Ignore lighting */, curvesAsWire && geom.GetAsCurveVector().IsValid());

    AddElementGeometry(*TileGeometry::Create(geom, tf, range, m_curElemId, displayParams, isCurved, curvesAsWire, m_dgndb));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& gf)
    {
    bool        isRegion = curves.IsAnyRegionType();
    bool        isCurved = curves.ContainsNonLinearPrimitive();

    if (m_surfacesOnly && !isRegion)
        return true;

    if (m_is2d)
        {
        CurveVectorPtr clone = curves.Clone();

        // Always treat 2D models as with MicroStation wireframe mode.
        if (filled && isRegion)
            {
            CurveVectorPtr  fillRegion = clone;
            static double   s_blankingRegionOffset = s_half2dDepthRange / 2000.0;  // Arbitrary - but below the a single priority (-500,500).

            if (gf.GetCurrentGraphicParams().IsBlankingRegion())
                fillRegion = clone->Clone(Transform::From(0.0, 0.0, -s_blankingRegionOffset));

            ProcessGeometry(*IGeometry::Create(fillRegion), isCurved, false, gf);
            if (gf.GetCurrentGraphicParams().GetLineColor() == gf.GetCurrentGraphicParams().GetFillColor())
                return true;
            }
        return ProcessGeometry(*IGeometry::Create(clone), isCurved, true, gf);
        }
    else
        {
        if (curves.IsAnyRegionType() && !isCurved && !m_is2d)
            return false;   // process as facets (optimization).

        CurveVectorPtr clone = curves.Clone();
        return ProcessGeometry(*IGeometry::Create(clone), true, !isRegion, gf);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessSolidPrimitive(ISolidPrimitiveCR prim, SimplifyGraphic& gf) 
    {
    bool hasCurvedFaceOrEdge = prim.HasCurvedFaceOrEdge();

    DRange3d                range, thisTileRange;
    ISolidPrimitivePtr      clone = prim.Clone();
    Transform               tf = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    TileDisplayParamsCR     displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d);

    clone->GetRange(range);
    tf.Multiply(thisTileRange, range);

    if (!thisTileRange.IsContained(m_tileRange))
        {
        IGeometryPtr geom = IGeometry::Create(clone);
        return ProcessGeometry(*geom, hasCurvedFaceOrEdge, false, gf);
        }

    
    SolidPrimitivePartMapKey    key(*clone, range, displayParams);
    TileGeomPartPtr             tileGeomPart = m_solidPrimitiveParts.Find(key);

    if (!tileGeomPart.IsValid())
        {
        IGeometryPtr        geom = IGeometry::Create(clone);
        TileGeometryList    geometryList;

        geometryList.push_back(TileGeometry::Create(*geom, Transform::FromIdentity(), range, m_curElemId, displayParams, hasCurvedFaceOrEdge, false, m_dgndb));
        
        m_solidPrimitiveParts.Insert(key, tileGeomPart = TileGeomPart::Create(range, geometryList));
        }
    
    tileGeomPart->IncrementInstanceCount();
    AddElementGeometry(*TileGeometry::Create(*tileGeomPart, tf, thisTileRange, m_curElemId, displayParams, m_dgndb));
    return true;
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
    return ProcessGeometry(*geom, isCurved, false, gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& gf) 
    {
    PolyfaceHeaderPtr clone = polyface.Clone();

#ifdef PRE_TRIANGLE_CONVEX
    if (!clone->IsTriangulated())
        clone->Triangulate();
#endif

    clone->Transform(Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform()));

    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d);
    DRange3d range = clone->PointRange();
    IGeometryPtr geom = IGeometry::Create(clone);
    AddElementGeometry(*TileGeometry::Create(*geom, Transform::FromIdentity(), range, m_curElemId, displayParams, false, false, m_dgndb));
 
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessBody(IBRepEntityCR solid, SimplifyGraphic& gf) 
    {
    // We need to generate these in this threads parasolid partition so that we 
    // can roll them back correctly in the event of a server parasolid error.

    IBRepEntityPtr  clone = const_cast<IBRepEntityP>(&solid);
    DRange3d        range = clone->GetEntityRange();
    Transform       localToTile = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());

    localToTile.Multiply(range, range);

    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), m_is2d);

    AddElementGeometry(*TileGeometry::Create(*clone, localToTile, range, m_curElemId, displayParams, m_dgndb));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileGeometryProcessor::_ProcessTextString(TextStringCR textString, SimplifyGraphic& gf) 
    {
    if (m_surfacesOnly)
        return true;

    TextStringPtr   clone = textString.Clone();
    Transform       localToTile = Transform::FromProduct(m_transformFromDgn, gf.GetLocalToWorldTransform());
    DRange2d        range2d   = clone->GetRange();
    DRange3d        range     = DRange3d::From (range2d.low.x, range2d.low.y, 0.0, range2d.high.x, range2d.high.y, 0.0);

    Transform::FromProduct (localToTile, clone->ComputeTransform()).Multiply (range, range);
                               
    TileDisplayParamsCR displayParams = m_displayParamsCache.Get(gf.GetCurrentGraphicParams(), gf.GetCurrentGeometryParams(), true /* Ignore lighting */);

    AddElementGeometry(*TileGeometry::Create(*clone, localToTile, range, m_curElemId, displayParams, m_dgndb));

    return true;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometryCollector
{
    TileElementSet const&   m_elements;
    TileGeometryProcessor&  m_processor;
    ViewContextR            m_context;
    RangeIndex::FBox        m_range;

    GeometryCollector(DRange3dCR range, TileGeometryProcessor& proc, ViewContextR context, TileElementSet const& elements)
        : m_elements(elements), m_range(range), m_processor(proc), m_context(context) { }

    TileGeneratorStatus Collect()
        {
        for (auto const& entry : m_elements)
            {
            if (entry.m_range.IntersectsWith(m_range))
                {
                DRange3d range = entry.m_range.ToRange3d();
                bool belowMinRange = m_processor.BelowMinRange(range);
                if (belowMinRange && m_processor.LeafThresholdExceeded())
                    break;

                m_processor.ProcessElement(m_context, entry.m_id, range);
                }
            }

        return TileGeneratorStatus::Success;
        }
};
         
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TileGeometryProcessor::OutputGraphics(ViewContextR context)
    {
    GeometryCollector collector(m_range, *this, context, m_cache.GetElements());

    auto status = collector.Collect();
    if (TileGeneratorStatus::Aborted == status)
        {
        m_tileGeometries.clear();
        m_leafGeometries.clear();
        }
#if defined(NEEDSWORK_SHEET_BORDER)
    // Sheet::Model::CreateBorder() now creates border in *view* coords...Cesium needs it in world coords...
    else if (TileGeneratorStatus::Success == status)
        {
        Sheet::ModelCP sheetModel = m_cache.GetModel().ToSheetModel();

        if (nullptr != sheetModel)
            {
            m_curElemId.Invalidate();

            // Cheap workaround for TFS#743687. Not going to invest in a better fix because MeshTile.cpp is going bye-bye very soon.
            DPoint2d sheetSize = sheetModel->GetSheetSize();
            if (0.0 == sheetSize.x)
                sheetSize.x = 0.1;
            if (0.0 == sheetSize.y)
                sheetSize.y = 0.1;

            auto border = Sheet::Model::CreateBorder (context, sheetSize);
            context.OutputGraphic(*border, nullptr);
            PushCurrentGeometry();
            }
        }
#endif

    if (!LeafThresholdExceeded())
        {
        for (auto& leafGeom : m_leafGeometries)
            m_tileGeometries.push_back(leafGeom);

        m_leafGeometries.clear();
        }

    return status;
    }


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
template<typename T> struct TileGeometryProcessorContext : NullContext
{
DEFINE_T_SUPER(NullContext);

private:
    TileGeometryProcessor&          m_processor;
    TileGenerationCacheCR           m_cache;
    BeSQLite::CachedStatementPtr    m_statement;

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }

    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
        {
        return new SimplifyGraphic(params, m_processor, *this);
        }

    StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    bool _WantUndisplayed() override {return true;}

    static Render::ViewFlags GetDefaultViewFlags()
        {
        // Ensure all classes/types of elements included...visibility can be controlled by declarative styling.
        // Most default to on.
        Render::ViewFlags flags;
        flags.SetShowConstructions(true);
        return flags;
        }
public:
    TileGeometryProcessorContext(TileGeometryProcessor& processor, DgnDbR db, TileGenerationCacheCR cache) : m_processor(processor), m_cache(cache),
    m_statement(db.GetCachedStatement(T::GetSql()))
        {
        SetDgnDb(db);
        m_is3dView = T::Is3d(); // force Brien to call _AddArc2d() if we're in a 2d model...
        SetViewFlags(GetDefaultViewFlags());
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance) override
    {
    DRange3d        pointRange = DRange3d::From(worldPoints, nPts);

    return pointRange.IntersectsWith(m_processor.GetRange());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams) override
    {
    DgnGeometryPartCPtr     geomPart = m_processor.GetDgnDb().Elements().template Get<DgnGeometryPart>(partId);

    if (!geomPart.IsValid())
        {
//      BeAssert(false);
        return;
        }

    static  size_t  s_minInstancePartSize = 2000;
    double          scale;

    if (geomPart->GetGeometryStream().size() > s_minInstancePartSize &&
        m_processor.IsGeomPartContained(graphic, *geomPart, subToGraphic) && 
        graphic.GetLocalToWorldTransform().IsRigidScale(scale))
        {
        GraphicParams graphicParams;
        _CookGeometryParams(geomParams, graphicParams);

        m_processor.AddGeomPart(graphic, *geomPart, subToGraphic, geomParams, graphicParams, *this);
        }
    else
        {
        T_Super::_AddSubGraphic(graphic, partId, subToGraphic, geomParams);
        }
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> StatusInt TileGeometryProcessorContext<T>::_VisitElement(DgnElementId elementId, bool allowLoad)
    {
#ifdef DEBUG_ELEMENT_FILTER
    static  DgnElementId s_debugId = DgnElementId((uint64_t) 197);

    if (s_debugId.IsValid() && s_debugId != elementId)
        return SUCCESS;
#endif

    GeometrySourceCP pSrc = m_cache.GetCachedGeometrySource(elementId);
    if (nullptr != pSrc)
        return VisitGeometry(*pSrc);

    // Never load elements - but do use them if they're already loaded
    DgnElementCPtr el = GetDgnDb().Elements().FindLoadedElement(elementId);
    if (el.IsValid())
        {
        GeometrySourceCP geomElem = el->ToGeometrySource();
        return (nullptr == geomElem) ? ERROR : VisitGeometry(*geomElem);
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
        auto geomSrcPtr = T::ExtractGeometrySource(stmt, GetDgnDb());

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
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus ElementTileNode::_CollectGeometry(TileGenerationCacheCR cache, DgnDbR db, bool* leafThresholdExceeded, double leafTolerance, double tileTolerance, bool surfacesOnly, size_t leafCountThreshold, ITileCollectionFilterCP filter)
    {
    // Collect geometry from elements in this node, sorted by size
    auto is2d = cache.GetModel().Is2dModel();
    IFacetOptionsPtr                facetOptions = TileGenerator::CreateTileFacetOptions(leafTolerance);
    TileGeometryProcessor           processor(m_geometries, cache, db, filter, GetDgnRange(), *facetOptions, m_transformFromDgn, leafThresholdExceeded, tileTolerance, surfacesOnly, leafCountThreshold, is2d);

    if (is2d)
        {
        TileGeometryProcessorContext<GeometrySelector2d> context(processor, db, cache);

        return processor.OutputGraphics(context);
        }
    else
        {
        TileGeometryProcessorContext<GeometrySelector3d> context(processor, db, cache);
        return processor.OutputGraphics(context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PublishableTileGeometry ElementTileNode::_GeneratePublishableGeometry(DgnDbR db, TileGeometry::NormalMode normalMode,  bool doSurfacesOnly, bool doInstancing, ITileGenerationFilterCP filter) const
    {
    bmap<TileGeomPartCP, TileMeshPartPtr>   partMap;
    TileGeometryList            uninstancedGeometry;
    PublishableTileGeometry     publishedTileGeometry;
    TileMeshList&               meshes = publishedTileGeometry.Meshes();
    size_t                      minInstanceCount = m_geometries.size() / 50;               // If the part will include 1/50th of geometry, do instancing (even if part does not deem it worthy).
    minInstanceCount = std::max(minInstanceCount, (size_t)2);

    // Extract instances first...
    for (auto& geom : m_geometries)
        {
        auto const&     part = geom->GetPart();

        if (doInstancing && (part.IsValid() && (part->GetInstanceCount() > minInstanceCount || part->IsWorthInstancing(GetTolerance()))))
            {
            auto const&         found = partMap.find(part.get());
            TileMeshPartPtr     meshPart;

            if (found == partMap.end())
                {           
                TileMeshList    partMeshes = GenerateMeshes(db, normalMode, doSurfacesOnly, false, filter, part->GetGeometries());

                if (partMeshes.empty())
                    continue;
                
                for (auto& partMesh : partMeshes)
                    partMesh->SetValidIdsPresent(false);    // Ids are included on the instances only.

                publishedTileGeometry.Parts().push_back(meshPart = TileMeshPart::Create (std::move(partMeshes)));
                partMap.Insert(part.get(), meshPart);
                }
            else
                {
                meshPart = found->second;
                }

            meshPart->AddInstance (TileMeshInstance(geom->GetAttributes(), geom->GetTransform()));
            m_containsParts = true;
            }
        else
            {
            uninstancedGeometry.push_back(geom);
            }
        }
    TileMeshList    uninstancedMeshes = GenerateMeshes (db, normalMode, doSurfacesOnly, true, filter, uninstancedGeometry);

    meshes.insert (meshes.end(), uninstancedMeshes.begin(), uninstancedMeshes.end());

    return publishedTileGeometry;
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     12/2017
+===============+===============+===============+===============+===============+======*/
struct MeshTileClipOutput : PolyfaceQuery::IClipToPlaneSetOutput
{
    bvector<PolyfaceHeaderPtr>  m_clipped;
    bvector<PolyfaceQueryCP>    m_output;

    StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR mesh) override { m_output.push_back(&mesh); ; return SUCCESS; }
    StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override { m_output.push_back(&mesh); m_clipped.push_back(&mesh); return SUCCESS; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileMeshList ElementTileNode::GenerateMeshes(DgnDbR db, TileGeometry::NormalMode normalMode, bool doSurfacesOnly, bool doRangeTest, ITileGenerationFilterCP filter, TileGeometryList const& geometries) const
    {
    static const double         s_vertexToleranceRatio    = .1;
    static const double         s_vertexClusterThresholdPixels = 5.0;
    static const double         s_facetAreaToleranceRatio = .1;
    static const size_t         s_decimatePolyfacePointCount = 100;

    double                      tolerance = GetTolerance();
    double                      vertexTolerance = tolerance * s_vertexToleranceRatio;
    double                      facetAreaTolerance   = tolerance * tolerance * s_facetAreaToleranceRatio;

    // Convert to meshes
    MeshBuilderMap      builderMap;
    DRange3d            myTileRange = GetTileRange();

    for (auto& geom : geometries)
        {
        if (nullptr != filter && !filter->AcceptElement(DgnElementId(geom->GetEntityId().GetValue()), geom->GetDisplayParams()))
            continue;

        DRange3dCR  geomRange = geom->GetTileRange();
        double      rangePixels = geomRange.DiagonalDistance() / tolerance;

        if (!geom->IsPoint() && rangePixels < s_minRangeBoxSize)
            continue;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

        auto        polyfaces = geom->GetPolyfaces(tolerance, normalMode);

        FeatureAttributes attributes = geom->GetAttributes();
        for (auto& tilePolyface : polyfaces)
            {
            TileDisplayParamsCPtr   displayParams = tilePolyface.m_displayParams;
            PolyfaceHeaderPtr       polyface = tilePolyface.m_polyface;
            bool                    hasTexture = displayParams.IsValid() && displayParams->HasTexture(db);  // Can't rely on geom.HasTexture - this may come from a face attachment to a B-Rep.

            if (0 != polyface->NormalIndex().size() && polyface->GetPointIndexCount() != polyface->NormalIndex().size())
                {
                BeAssert(false && "mismatched Normal and Point index counts");        // Crash in ChinaPlant...
                continue;
                }

            if (0 == polyface->GetPointCount())
                continue;

            TileMeshMergeKey key(*displayParams, polyface.IsValid() && nullptr != polyface->GetNormalIndexCP(), polyface.IsValid(), geom->GetEntityId().IsValid());

            TileMeshBuilderPtr meshBuilder;
            auto found = builderMap.find(key);
            if (builderMap.end() != found)
                meshBuilder = found->second;
            else
                builderMap[key] = meshBuilder = TileMeshBuilder::Create(*displayParams, m_transformFromDgn, vertexTolerance, facetAreaTolerance, const_cast<FeatureAttributesMapR>(m_attributes));

            if (polyface.IsValid())
                {
                // Decimate if the range of the geometry is small in the tile OR we are not in a leaf and we have geometry originating from polyface with many points (railings from Penn state building).
                // A polyface with many points is likely a tesselation from an outside source.
                bool                doDecimate           = !m_isLeaf && geom->DoDecimate() && polyface->GetPointCount() > s_decimatePolyfacePointCount;
                PolyfaceHeaderPtr   decimated;

                if (doDecimate && (decimated = polyface->ClusteredVertexDecimate(tolerance * 2.0)).IsValid())
                    polyface = decimated;
                
                MeshTileClipOutput  clipOutput;

                if (doRangeTest)
                    polyface->ClipToRange(myTileRange, clipOutput, false);
                else
                    clipOutput.m_output.push_back(polyface.get());     // Skip clipping if not range test (instances).

                for (auto& outputPolyface : clipOutput.m_output)
                    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*outputPolyface); visitor->AdvanceToNextFace(); /**/)
                        meshBuilder->AddTriangle (*visitor, displayParams->GetRenderMaterialId(), db, attributes, hasTexture, hasTexture ? 0 : displayParams->GetColor(), nullptr != outputPolyface->GetNormalCP());
                }
            }

        if (!doSurfacesOnly)
            {
            auto                tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions (tolerance, TileGeometry::NormalMode::Never));
        
            for (auto& tileStrokes : tileStrokesArray)
                {
                TileDisplayParamsCPtr   displayParams = tileStrokes.m_displayParams;
                TileMeshMergeKey key(*displayParams, false, false, geom->GetEntityId().IsValid());

                TileMeshBuilderPtr meshBuilder;
                auto found = builderMap.find(key);
                if (builderMap.end() != found)
                    meshBuilder = found->second;
                else
                    builderMap[key] = meshBuilder = TileMeshBuilder::Create(*displayParams, m_transformFromDgn, vertexTolerance, facetAreaTolerance, const_cast<FeatureAttributesMapR>(m_attributes));

                for (auto& strokePoints : tileStrokes.m_strokes)
                    meshBuilder->AddPolyline (strokePoints, attributes,  displayParams->GetColor());
                }
            }
        }

    TileMeshList meshes;
       
    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back (builder.second->GetMesh());

    return meshes;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureAttributes::operator<(FeatureAttributesCR rhs) const
    {
    if (IsUndefined())
        return rhs.IsDefined();
    else if (rhs.IsUndefined())
        return false;
    else if (GetElementId() != rhs.GetElementId())
        return GetElementId() < rhs.GetElementId();
    else if (GetSubCategoryId() != rhs.GetSubCategoryId())
        return GetSubCategoryId() < rhs.GetSubCategoryId();
    else if (GetClass() != rhs.GetClass())
        return static_cast<uint8_t>(GetClass()) < static_cast<uint8_t>(rhs.GetClass());
    else
        return false;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureAttributesMap::FeatureAttributesMap()
    {
    // 0 always maps to "no attributes defined"
    FeatureAttributes undefined;
    m_map[undefined] = 0;

    BeAssert(1 == GetNumIndices());
    BeAssert(0 == GetIndex(undefined));
    BeAssert(1 == GetNumIndices());
    BeAssert(!AnyDefined());
    BeAssert(!IsFull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t FeatureAttributesMap::GetIndex(TileGeometryCR geom)
    {
    return GetIndex(geom.GetAttributes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureAttributesMap::RemoveUndefined()
    {
    // Cesium's instanced models require that indices range from [0, nInstances). Must remove the "undefined" entry for that to work.
    BeAssert(AnyDefined());

    FeatureAttributes undefined;
    m_map.erase(undefined);

    for (auto& kvp : m_map)
        kvp.second -= 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileDisplayParamsCR TileDisplayParamsCache::Get(TileDisplayParamsR toFind)
    {
    toFind.AddRef();
    TileDisplayParamsCPtr pToFind(&toFind);
    auto iter = m_set.find(pToFind);
    if (m_set.end() == iter)
        iter = m_set.insert(toFind.Clone()).first;

    return **iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileDisplayParamsCPtr TileDisplayParams::Clone() const
    {
    TileDisplayParamsPtr clone(new TileDisplayParams());
    clone->m_categoryId = m_categoryId;
    clone->m_subCategoryId = m_subCategoryId;
    clone->m_color = m_color;
    clone->m_rasterWidth = m_rasterWidth;
    clone->m_materialId = m_materialId;
    clone->m_class = m_class;
    clone->m_ignoreLighting = m_ignoreLighting;
    clone->m_textureImage = m_textureImage;
    clone->m_linePixels = m_linePixels;
    clone->m_isColorFromBackground = m_isColorFromBackground;
    if (m_gradient.IsValid())
        {
        clone->m_gradient = GradientSymb::Create();
        clone->m_gradient->CopyFrom(*m_gradient);
        }
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static int compareValues(T const& lhs, T const& rhs)
    {
    return lhs == rhs ? 0 : (lhs < rhs ? -1 : 1);
    }

#define TEST_LESS_THAN(LHS, RHS) \
    { \
    int cmp = compareValues(LHS, RHS); \
    if (0 != cmp) \
        return cmp < 0; \
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::IsStrictlyLessThan(TileDisplayParamsCR rhs) const
    {
    TEST_LESS_THAN(m_categoryId.GetValueUnchecked(), rhs.m_categoryId.GetValueUnchecked());
    TEST_LESS_THAN(m_subCategoryId.GetValueUnchecked(), rhs.m_subCategoryId.GetValueUnchecked());
    TEST_LESS_THAN(m_color, rhs.m_color);
    TEST_LESS_THAN(m_rasterWidth, rhs.m_rasterWidth);
    TEST_LESS_THAN(m_materialId.GetValueUnchecked(), rhs.m_materialId.GetValueUnchecked());
    TEST_LESS_THAN(m_textureImage.get(), rhs.m_textureImage.get());
    TEST_LESS_THAN(m_linePixels, rhs.m_linePixels);
    TEST_LESS_THAN(static_cast<uint32_t>(m_class), static_cast<uint32_t>(rhs.m_class));
    TEST_LESS_THAN(m_gradient.get(), rhs.m_gradient.get());

    if (m_ignoreLighting != rhs.m_ignoreLighting)
        return m_ignoreLighting;

    return false;
    }

#define TEST_EQUAL(MEMBER) if (MEMBER != rhs.MEMBER) return false

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileDisplayParams::IsStrictlyEqualTo(TileDisplayParamsCR rhs) const
    {
    TEST_EQUAL(m_categoryId.GetValueUnchecked());
    TEST_EQUAL(m_subCategoryId.GetValueUnchecked());
    TEST_EQUAL(m_color);
    TEST_EQUAL(m_rasterWidth);
    TEST_EQUAL(m_materialId.GetValueUnchecked());
    TEST_EQUAL(m_textureImage.get());
    TEST_EQUAL(m_ignoreLighting);
    TEST_EQUAL(m_class);
    TEST_EQUAL(m_linePixels);
    TEST_EQUAL(m_gradient);
    TEST_EQUAL(m_class);

    return true;
    }


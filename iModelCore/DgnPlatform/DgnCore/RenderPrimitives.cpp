/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/RenderPrimitives.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

BEGIN_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(int numPoints, DPoint3dP pts3d, DPoint2dCP pts2d, double priority)
    {
    double depth = Render::Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    for (int i = 0; i < numPoints; i++, pts3d++, pts2d++)
        pts3d->Init(pts2d->x, pts2d->y, depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(std::valarray<DPoint3d>& pts3d, DPoint2dCP pts2d, double priority)
    {
    copy2dTo3d(static_cast<int>(pts3d.size()), &pts3d[0], pts2d, priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static FPoint3d toFPoint3d(DPoint3dCR dpoint)
    {
    FPoint3d fpoint;
    fpoint.x = dpoint.x;
    fpoint.y = dpoint.y;
    fpoint.z = dpoint.z;
    return fpoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static FPoint2d toFPoint2d(DPoint2dCR dpoint)
    {
    FPoint2d fpoint;
    fpoint.x = dpoint.x;
    fpoint.y = dpoint.y;
    return fpoint;
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

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveGeometry : Geometry
{
private:
    IGeometryPtr        m_geometry;
    bool                m_inCache;

    PrimitiveGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db)
        : Geometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry) { }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions) override;
    bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_geometry); }
    void _SetInCache(bool inCache) override { m_inCache = inCache; }
public:
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db)
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

    SolidKernelGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        : Geometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid) { }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override;
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_entity); }
public:
    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
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

    TextStringGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        : Geometry(transform, range, elemId, params, true, db), m_text(&text) 
        { 
        InitGlyphCurves();     // Should be able to defer this when font threaded ness is resolved.
        }

    bool _DoVertexCluster() const override { return false; }

public:
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        {
        return new TextStringGeometry(textString, transform, range, elemId, params, db);
        }
    
    bool DoGlyphBoxes (IFacetOptionsR facetOptions);
    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions) override;
    size_t _GetFacetCount(FacetCounter& counter) const override;
    void InitGlyphCurves() const;
};  // TextStringGeometry

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct GeomPartInstanceGeometry : Geometry
{
private:
    GeomPartPtr     m_part;

    GeomPartInstanceGeometry(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        : Geometry(tf, range, elemId, params, part.IsCurved(), db), m_part(&part) { }
public:
    static GeometryPtr Create(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)  { return new GeomPartInstanceGeometry(part, tf, range, elemId, params, db); }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions) override { return m_part->GetPolyfaces(facetOptions, *this); }
    StrokesList _GetStrokes (IFacetOptionsR facetOptions) override { return m_part->GetStrokes(facetOptions, *this); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return m_part->GetFacetCount (counter, *this); }
    GeomPartCPtr _GetPart() const override { return m_part; }

};  // GeomPartInstanceTileGeometry 

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams::DisplayParams(Render::GraphicParamsCR graphicParams, Render::GeometryParamsCP geometryParams, bool ignoreLighting)
    : m_graphicParams(graphicParams), m_ignoreLighting(ignoreLighting), m_geometryParamsValid(nullptr != geometryParams)
    {
    if (nullptr != geometryParams)
        m_geometryParams = *geometryParams;

    if (nullptr != graphicParams.GetMaterial() && graphicParams.GetMaterial()->HasTextures())
        m_isTextured = IsTextured::Yes;
    else if (GetMaterialId().IsValid())
        m_isTextured = IsTextured::Maybe;
    else
        m_isTextured = nullptr != GetGradient() ? IsTextured::Yes : IsTextured::No;
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
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::IsLessThan(DisplayParamsCR rhs, ComparePurpose purpose) const
    {
    if (&rhs == this)
        return false;

    if (GetIgnoreLighting() != rhs.GetIgnoreLighting())
        return GetIgnoreLighting();

    TEST_LESS_THAN(GetRasterWidth(), rhs.GetRasterWidth());
    TEST_LESS_THAN(GetMaterialId().GetValueUnchecked(), rhs.GetMaterialId().GetValueUnchecked());
    TEST_LESS_THAN(GetLinePixels(), rhs.GetLinePixels());

    if (m_isTextured != rhs.m_isTextured && m_isTextured != IsTextured::Maybe)
        return m_isTextured < rhs.m_isTextured;

    if (GetGradient() != rhs.GetGradient())
        {
        if (nullptr == GetGradient() || nullptr == rhs.GetGradient() || !(*GetGradient() == *rhs.GetGradient()))
            return GetGradient() < rhs.GetGradient();
        }

    if (ComparePurpose::Merge == purpose)
        {
        // We can merge meshes of differing colors, but can't mix opaque with translucent
        return (HasTransparency() != rhs.HasTransparency()) && HasTransparency();
        }

    TEST_LESS_THAN(GetFillColor(), rhs.GetFillColor());
    TEST_LESS_THAN(GetCategoryId().GetValueUnchecked(), rhs.GetCategoryId().GetValueUnchecked());
    TEST_LESS_THAN(GetSubCategoryId().GetValueUnchecked(), rhs.GetSubCategoryId().GetValueUnchecked());
    TEST_LESS_THAN(static_cast<uint32_t>(GetClass()), static_cast<uint32_t>(rhs.GetClass()));

    return false;
    }

#define TEST_EQUAL(MEMBER) if (MEMBER != rhs.MEMBER) return false

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::IsEqualTo(DisplayParamsCR rhs, ComparePurpose purpose) const
    {
    if (&rhs == this)
        return true;

    TEST_EQUAL(GetIgnoreLighting());
    TEST_EQUAL(GetRasterWidth());
    TEST_EQUAL(GetMaterialId().GetValueUnchecked());
    TEST_EQUAL(GetLinePixels());

    if ((nullptr == GetGradient()) != (nullptr == rhs.GetGradient()))
        return false;
    else if (nullptr != GetGradient() && GetGradient() != rhs.GetGradient() && !(*GetGradient() == *rhs.GetGradient()))
        return false;
    else if (m_isTextured != rhs.m_isTextured && m_isTextured != IsTextured::Maybe)
        return false;

    if (ComparePurpose::Merge == purpose)
        return HasTransparency() == rhs.HasTransparency();

    TEST_EQUAL(GetFillColor());
    TEST_EQUAL(GetCategoryId().GetValueUnchecked());
    TEST_EQUAL(GetSubCategoryId().GetValueUnchecked());
    TEST_EQUAL(GetClass());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::Clone() const
    {
    DisplayParamsPtr clone = new DisplayParams(GetGraphicParams(), GetGeometryParams(), GetIgnoreLighting());
    clone->m_texture = m_texture;
    clone->m_isTextured = m_isTextured;
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCR DisplayParamsCache::Get(DisplayParamsCR toFind)
    {
    BeAssert(0 == toFind.GetRefCount());    // allocated on stack...
    toFind.AddRef();
    DisplayParamsCPtr pToFind(&toFind);
    auto iter = m_set.find(pToFind);
    if (m_set.end() == iter)
        iter = m_set.insert(toFind.Clone()).first;

    return **iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureCPtr DisplayParams::QueryTexture(DgnDbR db) const
    {
    BeAssert(IsTextured::Maybe == m_isTextured);

    DgnMaterialCPtr material = DgnMaterial::Get(db, GetMaterialId());
    if (material.IsNull())
        return nullptr;

    auto& mat = material->GetRenderingAsset();
    auto texMap = mat.GetPatternMap();
    if (!texMap.IsValid())
        return nullptr;

    DgnTextureId texId = texMap.GetTextureId();
    if (!texId.IsValid())
        return nullptr;

    return DgnTexture::Get(db, texId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::HasTexture(DgnDbR db) const
    {
    if (IsTextured::Maybe != m_isTextured)
        return IsTextured::Yes == m_isTextured;

    BeAssert(nullptr == GetGradient());

    auto tex = QueryTexture(db);

    m_isTextured = tex.IsValid() ? IsTextured::Yes : IsTextured::No;
    return IsTextured::Yes == m_isTextured;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureP DisplayParams::ResolveTexture(DgnDbR db, Render::System const& system) const
    {
    if (IsTextured::No == m_isTextured || m_texture.IsValid())
        return m_texture.get();
    else if (nullptr != m_graphicParams.GetMaterial() && m_graphicParams.GetMaterial()->HasTextures())
        return nullptr; // Textures already cooked into material...

    // ###TODO? This will not handle an element with gradient fill and also a textured material. Do people do that?
    Render::TexturePtr tex;
    if (nullptr != GetGradient())
        {
        BeAssert(IsTextured::Yes == m_isTextured);
        tex = system._GetTexture(*GetGradient(), db);
        }
    else
        {
        // ###TODO: Would be nice to avoid a second lookup here but meh for now...
        DgnTextureCPtr dgnTex = QueryTexture(db);
        if (dgnTex.IsValid())
            tex = system._GetTexture(dgnTex->GetTextureId(), db);
        }

    m_isTextured = tex.IsValid() ? IsTextured::Yes : IsTextured::No;
    if (IsTextured::No == m_isTextured)
        return nullptr;

    // DisplayParams potentially cross thread boundaries...need to synchronize on texture-related members
    static BeMutex s_mutex;
    BeMutexHolder lock(s_mutex);
    if (m_texture.IsNull())
        m_texture = tex;

    return m_texture.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d Mesh::GetDPoint3d(bvector<FPoint3d> const& from, uint32_t index) const
    {
    auto fpoint = from.at(index);
    return DPoint3d::FromXYZ(fpoint.x, fpoint.y, fpoint.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetTriangleRange(TriangleCR triangle) const
    {
    return DRange3d::From(GetDPoint3d(m_points, triangle.m_indices[0]),
                          GetDPoint3d(m_points, triangle.m_indices[1]),
                          GetDPoint3d(m_points, triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d Mesh::GetTriangleNormal(TriangleCR triangle) const
    {
    return DVec3d::FromNormalizedCrossProductToPoints(GetDPoint3d(m_points, triangle.m_indices[0]),
                                                      GetDPoint3d(m_points, triangle.m_indices[1]),
                                                      GetDPoint3d(m_points, triangle.m_indices[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Mesh::HasNonPlanarNormals() const
    {
    if (m_normals.empty())
        return false;

    for (auto& triangle : m_triangles)
        if(!GetDPoint3d(m_normals, triangle.m_indices[0]).IsEqual(GetDPoint3d(m_normals, triangle.m_indices[1])) ||
            !GetDPoint3d(m_normals, triangle.m_indices[0]).IsEqual(GetDPoint3d(m_normals, triangle.m_indices[2])) ||
            !GetDPoint3d(m_normals, triangle.m_indices[1]).IsEqual(GetDPoint3d(m_normals, triangle.m_indices[2])))
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, typename U> static void insertVertexAttribute(bvector<uint16_t>& indices, T& table, U const& value, bvector<FPoint3d> const& vertices)
    {
    // Don't allocate the indices until we have non-uniform values
    if (table.empty())
        {
        table.GetIndex(value);
        BeAssert(table.IsUniform());
        BeAssert(0 == table.GetIndex(value));
        }
    else if (!table.IsUniform() || table.begin()->first != value)
        {
        if (indices.empty())
            {
            // back-fill uniform value for existing vertices...
            indices.resize(vertices.size() - 1);
            std::fill(indices.begin(), indices.end(), (uint16_t)0);
            }

        indices.push_back(table.GetIndex(value));
        BeAssert(!table.IsUniform());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Mesh::AddVertex(DPoint3dCR point, DVec3dCP normal, DPoint2dCP param, uint32_t fillColor, FeatureCR feature)
    {
    auto index = static_cast<uint32_t>(m_points.size());

    m_points.push_back(toFPoint3d(point));
    m_features.Add(feature, m_points.size());

    if (nullptr != normal)
        m_normals.push_back(toFPoint3d(*normal));
                                                                                                                 
    if (nullptr != param)
        m_uvParams.push_back(toFPoint2d(*param));
    else
        insertVertexAttribute(m_colors, m_colorTable, fillColor, m_points);

    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::Features::Add(FeatureCR feat, size_t numVerts)
    {
    if (nullptr == m_table)
        return;

    // Avoid allocating + populating big buffer of feature IDs unless necessary...
    uint16_t index = m_table->GetIndex(feat);
    if (!m_initialized)
        {
        // First feature - uniform.
        m_uniform = index;
        m_initialized = true;
        }
    else if (!m_indices.empty())
        {
        // Already non-uniform
        m_indices.push_back(index);
        }
    else if (m_uniform != index)
        {
        // Second feature - back-fill uniform for existing verts
        m_indices.resize(numVerts - 1);
        std::fill(m_indices.begin(), m_indices.end(), m_uniform);
        m_indices.push_back(index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::Features::ToFeatureIndex(FeatureIndex& index) const
    {
    if (!m_initialized)
        {
        index.m_type = FeatureIndex::Type::Empty;
        }
    else if (m_indices.empty())
        {
        index.m_type = FeatureIndex::Type::Uniform;
        index.m_featureID = m_uniform;
        }
    else
        {
        index.m_type = FeatureIndex::Type::NonUniform;
        index.m_featureIDs = m_indices.data();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetRange() const
    {
    DRange3d range = DRange3d::NullRange();
    for (auto const& fpoint : m_points)
        range.Extend(DPoint3d::FromXYZ(fpoint.x, fpoint.y, fpoint.z));

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetUVRange() const
    {
    DRange3d range = DRange3d::NullRange();
    for (auto const& fpoint : m_uvParams)
        range.Extend(DPoint3d::FromXYZ(fpoint.x, fpoint.y, 0.0));

    return range;
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

    COMPARE_VALUES (lhs.m_feature.GetElementId(), rhs.m_feature.GetElementId());
    COMPARE_VALUES (lhs.m_feature.GetSubCategoryId(), rhs.m_feature.GetSubCategoryId());
    COMPARE_VALUES (lhs.m_feature.GetClass(), rhs.m_feature.GetClass());
    COMPARE_VALUES (lhs.m_fillColor, rhs.m_fillColor);

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
* @bsimethod                                                    Keith.Bentley   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshBuilder::GetMaterial(DgnMaterialId materialId, DgnDbR db)
    {
    if (!materialId.IsValid())
        return false;

    m_materialEl = DgnMaterial::Get(db, materialId);
    BeAssert(m_materialEl.IsValid());
    if (m_materialEl.IsNull())
        return false;

    m_material = &m_materialEl->GetRenderingAsset();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddTriangle(PolyfaceVisitorR visitor, DgnMaterialId materialId, DgnDbR dgnDb, FeatureCR feature, bool doVertexCluster, bool duplicateTwoSidedTriangles, bool includeParams, uint32_t fillColor)
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

    if (includeParams && !params.empty() && (m_material || GetMaterial(materialId, dgnDb)))
        {
        auto const&         patternMap = m_material->GetPatternMap();
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
        VertexKey vertex(points.at(i), haveNormals ? &visitor.Normal().at(i) : nullptr, !includeParams || params.empty() ? nullptr : &params.at(i), feature, fillColor);
        newTriangle.m_indices[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
        }

    BeAssert(m_mesh->Params().empty() || m_mesh->Params().size() == m_mesh->Points().size());
    BeAssert(m_mesh->Normals().empty() || m_mesh->Normals().size() == m_mesh->Points().size());

    AddTriangle(newTriangle);

    if (visitor.GetTwoSided() && duplicateTwoSidedTriangles)
        {
        Triangle dupTriangle(false);

        for (size_t i = 0; i < 3; i++)
            {
            size_t reverseIndex = 2 - i;
            DVec3d reverseNormal;
            if (haveNormals)
                reverseNormal.Negate(visitor.Normal().at(reverseIndex));

            VertexKey vertex(points.at(reverseIndex), haveNormals ? &reverseNormal : nullptr, includeParams || params.empty() ? nullptr : &params.at(reverseIndex), feature, fillColor);
            dupTriangle.m_indices[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
            }

        AddTriangle(dupTriangle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline (bvector<DPoint3d>const& points, FeatureCR feature, bool doVertexCluster, uint32_t fillColor)
    {
    Polyline    newPolyline;

    for (auto& point : points)
        {
        VertexKey vertex(point, nullptr, nullptr, feature, fillColor);
        newPolyline.GetIndices().push_back (doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }

    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyface (PolyfaceQueryCR polyface, DgnMaterialId materialId, DgnDbR dgnDb, FeatureCR feature, bool twoSidedTriangles, bool includeParams, uint32_t fillColor)
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        AddTriangle(*visitor, materialId, dgnDb, feature, false, twoSidedTriangles, includeParams, fillColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MeshBuilder::AddVertex(VertexKey const& vertex)
    {
    auto found = m_unclusteredVertexMap.find(vertex);
    if (m_unclusteredVertexMap.end() != found)
        return found->second;

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_fillColor, vertex.m_feature);
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

    auto index = m_mesh->AddVertex(vertex.m_point, vertex.GetNormal(), vertex.GetParam(), vertex.m_fillColor, vertex.m_feature);
    m_clusteredVertexMap[vertex] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPart::GeomPart(DRange3dCR range, GeometryList const& geometries) : m_range (range), m_facetCount(0), m_geometries(geometries)
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
    return GetPolyfaces(facetOptions, &instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList GeomPart::GetPolyfaces(IFacetOptionsR facetOptions, GeometryCP instance)
    {
    PolyfaceList polyfaces;
    for (auto& geometry : m_geometries) 
        {
        BeAssert(geometry->GetTransform().IsIdentity());
        PolyfaceList thisPolyfaces = geometry->GetPolyfaces (facetOptions);

        for (auto const& thisPolyface : thisPolyfaces)
            {
            Polyface polyface(*thisPolyface.m_displayParams, *thisPolyface.m_polyface->Clone());
            if (nullptr != instance)
                polyface.Transform(instance->GetTransform());

            polyfaces.push_back(polyface);
            }
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes (IFacetOptionsR facetOptions, GeometryCR instance)
    {
    return GetStrokes(facetOptions, &instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes(IFacetOptionsR facetOptions, GeometryCP instance)
    {
    StrokesList strokes;

    for (auto& geometry : m_geometries) 
        {
        StrokesList   thisStrokes = geometry->GetStrokes(facetOptions);

        if (!thisStrokes.empty())
            strokes.insert (strokes.end(), thisStrokes.begin(), thisStrokes.end());
        }

    if (nullptr != instance)
        {
        for (auto& stroke : strokes)
            stroke.Transform(instance->GetTransform());
        }

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
Geometry::Geometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db)
    : m_params(&params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(params.HasTexture(db))
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
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr Geometry::CreateFacetOptions(double chordTolerance)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr Geometry::CreateFacetOptions(double chordTolerance, NormalMode normalMode) const
    {
    auto facetOptions = CreateFacetOptions(chordTolerance / m_transform.ColumnXMagnitude());
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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList Geometry::GetPolyfaces(double chordTolerance, NormalMode normalMode)
    {
    return _GetPolyfaces(*CreateFacetOptions(chordTolerance, normalMode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Strokes::Transform(TransformCR transform)
    {
    for (auto& stroke : m_strokes)
        transform.Multiply (stroke, stroke);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList PrimitiveGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    
    if (polyface.IsValid())
        {
        if (m_inCache)
            polyface = polyface->Clone();

        if (!HasTexture())
            polyface->ClearParameters(false);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return PolyfaceList (1, Polyface(*GetDisplayParamsPtr(), *polyface));
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
        polyfaces.push_back (Polyface(*GetDisplayParamsPtr(), *polyface));
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

    bvector<bvector<DPoint3d>>  strokePoints;
    if (curveVector.IsValid() && ! curveVector->IsAnyRegionType())
        {
        strokePoints.clear();
        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        // ###TODO_ELEMENT_TILE: This is not precisely accurate. Need to handle:
        //  - boundary type 'none' not actually containing point string; and
        //  - boundary type 'none' containing multiple curves, any number of which may be point strings
        // where 'point string' refers to any of:
        //   - point string curve primitive type;
        //   - line string (with any boundary type) containing 1 point
        //   - line string (with any boundary type) containing 2 identical points
        bool disjoint = CurveVector::BOUNDARY_TYPE_None == curveVector->GetBoundaryType();
        if (!disjoint && 1 == strokePoints.size())
            {
            // A 'point' is actually a zero-length line...
            auto const& points = strokePoints.front();
            switch (points.size())
                {
                case 1:
                    disjoint = true;
                    break;
                case 2:
                    disjoint = *points.begin() == *(points.begin()+1);
                    break;
                }
            }

        if (!strokePoints.empty())
            tileStrokes.push_back(Strokes(*GetDisplayParamsPtr(), std::move(strokePoints), disjoint));
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
        baseParams.SetCategoryId(GetDisplayParams().GetCategoryId());
        baseParams.SetSubCategoryId(GetDisplayParams().GetSubCategoryId());

        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {
                GeometryParams faceParams;

                params[i].ToGeometryParams(faceParams, baseParams);

#if defined(ELEMENT_TILE_FILL_COLOR_ONLY)
                DisplayParamsPtr displayParams = DisplayParams::Create (GetDisplayParams().GetFillColorDef(), faceParams);
#else
                DisplayParamsCPtr displayParams = GetDisplayParamsPtr();
#endif

                tilePolyfaces.push_back (Polyface(*displayParams, *polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (Polyface(*GetDisplayParamsPtr(), *polyface));

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
GeometryPtr Geometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db)
    {
    return TextStringGeometry::Create(textString, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db)
    {
    return PrimitiveGeometry::Create(geometry, tf, range, entityId, params, isCurved, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db)
    {
    return SolidKernelGeometry::Create(solid, tf, range, entityId, params, db);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(GeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db)
    {
    return GeomPartInstanceGeometry::Create(part, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;

    auto tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    return AddGeometry(geom, isCurved, displayParams, tf, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, DRange3dCR range)
    {
    GeometryPtr geometry = Geometry::Create(geom, transform, range, GetElementId(), displayParams, isCurved, GetDgnDb());
    if (geometry.IsNull())
        return false;

    m_geometries.push_back(geometry);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddCurveVector(CurveVectorCR curves, bool filled, DisplayParamsCR displayParams, TransformCR transform)
    {
    if (m_surfacesOnly && !curves.IsAnyRegionType())
        return true;    // ignore...

    bool isCurved = curves.ContainsNonLinearPrimitive();
    CurveVectorPtr clone = curves.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return AddGeometry(*geom, isCurved, displayParams, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddSolidPrimitive(ISolidPrimitiveCR primitive, DisplayParamsCR displayParams, TransformCR transform)
    {
    bool isCurved = primitive.HasCurvedFaceOrEdge();
    ISolidPrimitivePtr clone = primitive.Clone();
    IGeometryPtr geom = IGeometry::Create(clone);
    return AddGeometry(*geom, isCurved, displayParams, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddSurface(MSBsplineSurfaceCR surface, DisplayParamsCR displayParams, TransformCR transform)
    {
    MSBsplineSurfacePtr clone = MSBsplineSurface::CreatePtr();
    clone->CopyFrom(surface);
    IGeometryPtr geom = IGeometry::Create(clone);

    bool isCurved = (clone->GetUOrder() > 2 || clone->GetVOrder() > 2);
    return AddGeometry(*geom, isCurved, displayParams, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddPolyface(PolyfaceQueryCR polyface, bool filled, DisplayParamsCR displayParams, TransformCR transform)
    {
    PolyfaceHeaderPtr clone = polyface.Clone();
    if (!clone->IsTriangulated() && SUCCESS != clone->Triangulate())
        {
        BeAssert(false && "Failed to triangulate...");
        return false;
        }

    if (m_haveTransform)
        clone->Transform(Transform::FromProduct(m_transform, transform));
    else if (!transform.IsIdentity())
        clone->Transform(transform);

    DRange3d range = clone->PointRange();
    IGeometryPtr geom = IGeometry::Create(clone);
    AddGeometry(*geom, false, displayParams, Transform::FromIdentity(), range);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddBody(IBRepEntityCR body, DisplayParamsCR displayParams, TransformCR transform)
    {
    IBRepEntityPtr clone = const_cast<IBRepEntityP>(&body);

    DRange3d range = clone->GetEntityRange();
    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    m_geometries.push_back(Geometry::Create(*clone, tf, range, GetElementId(), displayParams, GetDgnDb()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryListBuilder::AddTextString(TextStringCR textString, DisplayParamsCR displayParams, TransformCR transform)
    {
    if (m_surfacesOnly)
        return true;

    static BeMutex s_tempFontMutex;
    BeMutexHolder lock(s_tempFontMutex);    // Temporary - until we resolve the font threading issues.

    TextStringPtr clone = textString.Clone();
    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;

    DRange2d range2d = clone->GetRange();
    DRange3d range = DRange3d::From(range2d.low.x, range2d.low.y, 0.0, range2d.high.x, range2d.high.y, 0.0);
    Transform::FromProduct(tf, clone->ComputeTransform()).Multiply(range, range);

    m_geometries.push_back(Geometry::Create(*clone, tf, range, GetElementId(), displayParams, GetDgnDb()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList GeometryListBuilder::ToMeshes(GeometryOptionsCR options, double tolerance) const
    {
    MeshList meshes;
    if (m_geometries.empty())
        return meshes;

    double vertexTolerance = tolerance * ToleranceRatio::Vertex();
    double facetAreaTolerance = tolerance * tolerance * ToleranceRatio::FacetArea();

    bmap<MeshMergeKey, MeshBuilderPtr> builderMap;
    for (auto const& geom : m_geometries)
        {
        auto polyfaces = geom->GetPolyfaces(tolerance, options.m_normalMode);
        for (auto const& tilePolyface : polyfaces)
            {
            PolyfaceHeaderPtr polyface = tilePolyface.m_polyface;
            if (polyface.IsNull() || 0 == polyface->GetPointCount())
                continue;

            DisplayParamsCPtr displayParams = tilePolyface.m_displayParams;
            bool hasTexture = displayParams.IsValid() && displayParams->HasTexture(GetDgnDb());

            MeshMergeKey key(*displayParams, nullptr != polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh);

            MeshBuilderPtr meshBuilder;
            auto found = builderMap.find(key);
            if (builderMap.end() != found)
                meshBuilder = found->second;
            else
                builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance, nullptr, Mesh::PrimitiveType::Mesh);

            uint32_t fillColor = displayParams->GetFillColor();
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                meshBuilder->AddTriangle(*visitor, displayParams->GetMaterialId(), GetDgnDb(), geom->GetFeature(), false, options.WantTwoSidedTriangles(), hasTexture, fillColor);
            }

        if (!options.WantSurfacesOnly())
            {
            auto tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions(tolerance, NormalMode::Never));
            for (auto& tileStrokes : tileStrokesArray)
                {
                DisplayParamsCPtr displayParams = tileStrokes.m_displayParams;
                MeshMergeKey key(*displayParams, false, tileStrokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline);

                MeshBuilderPtr meshBuilder;
                auto found = builderMap.find(key);
                if (builderMap.end() != found)
                    meshBuilder = found->second;
                else
                    builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance, nullptr, key.m_primitiveType);

                uint32_t fillColor = displayParams->GetFillColor();
                for (auto& strokePoints : tileStrokes.m_strokes)
                    meshBuilder->AddPolyline(strokePoints, geom->GetFeature(), false, fillColor);
                }
            }
        }

    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            meshes.push_back(builder.second->GetMesh());

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::SaveToGraphicList(bvector<GraphicPtr>& graphics, Render::System const& system, GeometryOptionsCR options, double tolerance) const
    {
    MeshArgs meshArgs;
    PolylineArgs polylineArgs;

    MeshList meshes = ToMeshes(options, tolerance);
    for (auto const& mesh : meshes)
        {
        bool haveMesh = !mesh->Triangles().empty();
        bool havePolyline = !haveMesh && !mesh->Polylines().empty();
        if (!haveMesh && !havePolyline)
            continue;

        Render::GraphicPtr graphic;
        if (havePolyline)
            {
            if (polylineArgs.Init(*mesh))
                graphic = system._CreateIndexedPolylines(polylineArgs, m_dgndb, mesh->GetDisplayParams().GetGraphicParams());
            }
        else if (meshArgs.Init(*mesh, system, m_dgndb))
            {
            graphic = system._CreateTriMesh(meshArgs, m_dgndb, mesh->GetDisplayParams().GetGraphicParams());
            }

        if (graphic.IsValid())
            graphics.push_back(graphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t ColorTable::GetIndex(uint32_t color)
    {
    BeAssert(!IsFull());
    BeAssert(empty() || (0 != ColorDef(color).GetAlpha()) == m_hasAlpha);

    auto iter = m_map.find(color);
    if (m_map.end() != iter)
        return iter->second;
    else if (IsFull())
        {
        BeAssert(false);
        return 0;
        }

    // The table should never contain a mix of opaque and translucent colors
    if (empty())
        m_hasAlpha = (0 != ColorDef(color).GetAlpha());

    uint16_t index = GetNumIndices();
    m_map[color] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorTable::ToColorIndex(ColorIndex& index, bvector<uint32_t>& colors, bvector<uint16_t> const& indices) const
    {
    index.Reset();
    if (IsUniform() || empty())
        return;

    BeAssert(!indices.empty());

    colors.resize(size());
    for (auto const& kvp : *this)
        colors[kvp.second] = kvp.first;

    index.m_hasAlpha = m_hasAlpha;
    index.m_colors = colors.data();
    index.m_numColors = GetNumIndices();
    index.m_indices = indices.data();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextStringGeometry::DoGlyphBoxes (IFacetOptionsR facetOptions)
    {
    DRange2d            textRange = m_text->GetRange();
    double              minDimension = std::min (textRange.high.x - textRange.low.x, textRange.high.y - textRange.low.y) * GetTransform().ColumnXMagnitude();
    static const double s_minGlyphRatio = 1.0; 
    
    return minDimension < s_minGlyphRatio * facetOptions.GetChordTolerance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList TextStringGeometry::_GetPolyfaces(IFacetOptionsR facetOptions)
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
        polyfaces.push_back (Polyface(*GetDisplayParamsPtr(), *polyface));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList TextStringGeometry::_GetStrokes (IFacetOptionsR facetOptions)
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
        strokes.push_back(Strokes(*GetDisplayParamsPtr(), std::move(strokePoints), false));

    return strokes;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TextStringGeometry::_GetFacetCount(FacetCounter& counter) const
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
void  TextStringGeometry::InitGlyphCurves() const
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshArgs::Init(MeshCR mesh, Render::System const& system, DgnDbR db)
    {
    Clear();
    if (mesh.Triangles().empty())
        return false;

    for (auto const& triangle : mesh.Triangles())
        {
        m_indices.push_back(static_cast<int32_t>(triangle.m_indices[0]));
        m_indices.push_back(static_cast<int32_t>(triangle.m_indices[1]));
        m_indices.push_back(static_cast<int32_t>(triangle.m_indices[2]));
        }

    Set(m_numIndices, m_vertIndex, m_indices);
    Set(m_numPoints, m_points, mesh.Points());
    Set(m_textureUV, mesh.Params());
    if (!mesh.GetDisplayParams().GetIgnoreLighting())    // ###TODO: Avoid generating normals in the first place if no lighting...
        Set(m_normals, mesh.Normals());

    m_texture = mesh.GetDisplayParams().ResolveTexture(db, system);

    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshArgs::Clear()
    {
    m_indices.clear();
    m_numIndices = 0;
    m_vertIndex = nullptr;
    m_numPoints = 0;
    m_points = nullptr;
    m_normals = nullptr;
    m_textureUV = nullptr;
    m_texture = nullptr;
    m_flags = 0;

    m_colors.Reset();
    m_colorTable.clear();
    m_features.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshArgs::Transform(TransformCR tf)
    {
    for (int32_t i = 0; i < m_numPoints; i++)
        {
        FPoint3d& fpt = const_cast<FPoint3d&>(m_points[i]);
        DPoint3d dpt = DPoint3d::FromXYZ(fpt.x, fpt.y, fpt.z);
        tf.Multiply(dpt);
        fpt.x = dpt.x;
        fpt.y = dpt.y;
        fpt.z = dpt.z;

        if (nullptr != m_normals)
            {
            FPoint3d& fnm = const_cast<FPoint3d&>(m_normals[i]);
            dpt = DPoint3d::FromXYZ(fnm.x, fnm.y, fnm.z);
            tf.MultiplyMatrixOnly(dpt);
            fnm.x = dpt.x;
            fnm.y = dpt.y;
            fnm.z = dpt.z;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineArgs::Reset()
    {
    m_disjoint = false;
    m_numPoints = m_numLines = 0;
    m_points = nullptr;
    m_lines = nullptr;
    m_polylines.clear();

    m_colors.Reset();
    m_colorTable.clear();
    m_features.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool PolylineArgs::Init(MeshCR mesh)
    {
    Reset();

    m_numPoints = static_cast<uint32_t>(mesh.Points().size());
    m_points = &mesh.Points()[0];
    m_polylines.reserve(mesh.Polylines().size());
    m_disjoint = Mesh::PrimitiveType::Point == mesh.GetType();

    for (auto const& polyline : mesh.Polylines())
        {
        IndexedPolyline indexedPolyline;
        if (indexedPolyline.Init(polyline))
            m_polylines.push_back(indexedPolyline);
        }

    if (IsValid())
        {
        m_numLines = static_cast<uint32_t>(m_polylines.size());
        m_lines = &m_polylines[0];

        mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
        mesh.ToFeatureIndex(m_features);
        }

    return IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineArgs::Transform(TransformCR tf)
    {
    for (uint32_t i = 0; i < m_numPoints; i++)
        {
        FPoint3d fpt = m_points[i];
        DPoint3d dpt = DPoint3d::FromXYZ(fpt.x, fpt.y, fpt.z);
        tf.Multiply(dpt);
        fpt.x = dpt.x;
        fpt.y = dpt.y;
        fpt.z = dpt.z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_ActivateGraphicParams(GraphicParamsCR gfParams, GeometryParamsCP geomParams)
    {
    m_graphicParams = gfParams;
    m_geometryParamsValid = nullptr != geomParams;
    if (m_geometryParamsValid)
        m_geometryParams = *geomParams;
    else
        m_geometryParams = GeometryParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr System::_CreateTile(TextureCR tile, GraphicBuilder::TileCorners const& corners, DgnDbR db, GraphicParamsCR params) const
    {
    TriMeshArgs rasterTile;

    // corners
    // [0] [1]
    // [2] [3]
    FPoint3d vertex[4];
    for (uint32_t i = 0; i < 4; ++i)
        {
        vertex[i].x = corners.m_pts[i].x;
        vertex[i].y = corners.m_pts[i].y;
        vertex[i].z = corners.m_pts[i].z;
        }

    rasterTile.m_points = vertex;
    rasterTile.m_numPoints = 4;

    static int32_t indices[] = {0,1,2,2,1,3};
    rasterTile.m_numIndices = 6;
    rasterTile.m_vertIndex = indices;
    
    static FPoint2d textUV[] = 
        {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 1.0f},
        };

    rasterTile.m_textureUV = textUV;
    rasterTile.m_texture = const_cast<Render::Texture*>(&tile);

    return _CreateTriMesh(rasterTile, db, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                1/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddTile(TextureCR tile, TileCorners const& corners)
    {
    GraphicPtr gf = m_system._CreateTile(tile, corners, GetDgnDb(), GetGraphicParams());
    if (gf.IsValid())
        m_primitives.push_back(gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::AddTriMesh(TriMeshArgsCR args)
    {
    GraphicPtr gf = m_system._CreateTriMesh(args, GetDgnDb(), GetGraphicParams());
    if (gf.IsValid())
        m_primitives.push_back(gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddShape(int numPoints, DPoint3dCP points, bool filled)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, numPoints));
    _AddCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddArc(ellipse, isEllipse, filled);
        }
    else
        {
        auto ell = ellipse;
        ell.center.z = zDepth;
        _AddArc(ell, isEllipse, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled)
    {
    CurveVectorPtr curve = CurveVector::Create((isEllipse || filled) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(ellipse));
    if (filled && !isEllipse && !ellipse.IsFullEllipse())
        {
        DSegment3d segment;
        ICurvePrimitivePtr gapSegment;

        ellipse.EvaluateEndPoints(segment.point[1], segment.point[0]);
        gapSegment = ICurvePrimitive::CreateLine(segment);
        gapSegment->SetMarkerBit(ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
        curve->push_back(gapSegment);
        }

    _AddCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddLineString2d(int numPoints, DPoint2dCP points, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddLineString(numPoints, &pts3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddLineString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLineString(points, numPoints));
    _AddCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddPointString2d(int numPoints, DPoint2dCP points, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddPointString(numPoints, &pts3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddPointString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, ICurvePrimitive::CreatePointString(points, numPoints));
    _AddCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddPolyface(PolyfaceQueryCR meshDataIn, bool filled)
    {
    m_geomList.AddPolyface(meshDataIn, filled, GetDisplayParams(), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddBody(IBRepEntityCR entity)
    {
    m_geomList.AddBody(entity, GetDisplayParams(), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddShape2d(int numPoints, DPoint2dCP points, bool filled, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(numPoints, &pts3d[0], points, priority);
    _AddShape(numPoints, &pts3d[0], filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddTextString(TextStringCR text)
    {
    m_geomList.AddTextString(text, GetDisplayParams(true), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddTextString2d(TextStringCR text, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddTextString(text);
        }
    else
        {
        TextStringPtr ts = text.Clone();
        auto origin = ts->GetOrigin();
        origin.z = zDepth;
        ts->SetOrigin(origin);
        _AddTextString(*ts);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags)
    {
    if (1 == usageFlags) // represents thickened line...
        {
        int nPt = 0;
        auto tmpPts = reinterpret_cast<DPoint3dP>(_alloca((numPoints+1)*sizeof(DPoint3d)));
        
        for (int iPtS1 = 0; iPtS1 < numPoints; iPtS1 += 2)
            tmpPts[nPt++] = points[iPtS1];

        for (int iPtS2 = numPoints-1; iPtS2 > 0; iPtS2 -= 2)
            tmpPts[nPt++] = points[iPtS2];

        tmpPts[nPt] = tmpPts[0]; // Add closure point...simplifies drop of extrude thickness

        _AddShape(numPoints+1, tmpPts, true);
        }
    else
        {
        // spew triangles
        for (int iPt = 0; iPt < numPoints-2; iPt++)
            _AddShape(3, points+iPt, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddTriStrip(numPoints, &pts3d[0], usageFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddSolidPrimitive(ISolidPrimitiveCR primitive)
    {
    m_geomList.AddSolidPrimitive(primitive, GetDisplayParams(), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddCurveVector(CurveVectorCR curves, bool isFilled)
    {
    m_geomList.AddCurveVector(curves, isFilled, GetDisplayParams(), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddCurveVector2d(CurveVectorCR curves, bool isFilled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddCurveVector(curves, isFilled);
        }
    else
        {
        Transform tf = Transform::From(DPoint3d::FromXYZ(0.0, 0.0, zDepth));
        auto cv = curves.Clone(tf);
        _AddCurveVector(*cv, isFilled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddBSplineCurve(MSBsplineCurveCR bcurve, bool filled)
    {
    CurveVectorPtr cv = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurve(bcurve));
    _AddCurveVector(*cv, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddBSplineCurve2d(MSBsplineCurveCR bcurve, bool filled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddBSplineCurve(bcurve, filled);
        }
    else
        {
        MSBsplineCurve bs;
        bs.CopyFrom(bcurve);
        int nPoles = bs.GetNumPoles();
        DPoint3d* poles = bs.GetPoleP();
        for (int i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurve(bs, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddBSplineSurface(MSBsplineSurfaceCR surface)
    {
    m_geomList.AddSurface(surface, GetDisplayParams(), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddDgnOle(DgnOleDraw* dgnOle)
    {
    BeAssert("TODO");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddSubGraphic(Graphic& gf, TransformCR subToGf, GraphicParamsCR gfParams, ClipVectorCP clip)
    {
    // ###TODO_ELEMENT_TILE: Overriding GraphicParams?
    // ###TODO_ELEMENT_TILE: Clip...
    Render::GraphicPtr graphic(&gf);
    if (nullptr != clip || !subToGf.IsIdentity())
        {
        GraphicBranch branch;
        branch.Add(gf);
        graphic = m_system._CreateBranch(std::move(branch), GetDgnDb(), Transform::FromProduct(GetLocalToWorldTransform(), subToGf), clip);
        }

    if (graphic.IsValid())
        m_primitives.push_back(graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr PrimitiveBuilder::_CreateSubGraphic(TransformCR subToGf, ClipVectorCP clip) const
    {
    return m_system._CreateGraphic(GraphicBuilder::CreateParams(GetDgnDb(), subToGf.IsIdentity() ? GetLocalToWorldTransform() : Transform::FromIdentity()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr PrimitiveBuilder::_Finish()
    {
    BeAssert(IsOpen());

    GraphicPtr graphic;
    if (IsOpen())
        {
        m_isOpen = false;
        if (!m_geomList.IsEmpty())
            {
            GeometryOptions options;
            m_geomList.SaveToGraphicList(m_primitives, m_system, options);
            }

        switch (m_primitives.size())
            {
            case 1:
                graphic = *m_primitives.begin();
                m_primitives.clear();
                break;
            default:
                // NB: We may have zero primitives. Callers aren't going to check for null return value. So return an empty graphic list.
                // (See for example DrawGrid() - often produces empty Graphic and adds directly to world decorations)
                graphic = m_system._CreateGraphicList(std::move(m_primitives), GetDgnDb());
                break;
            }
        }

    m_geomList.Clear();
    return graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCR PrimitiveBuilder::GetDisplayParams(bool ignoreLighting) const
    {
    return m_geomList.GetDisplayParamsCache().Get(GetGraphicParams(), GetGeometryParams(), ignoreLighting);
    }


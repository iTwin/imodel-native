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
static void collectCurveStrokes (Strokes::PointLists& strokes, CurveVectorCR curve, IFacetOptionsR facetOptions, TransformCR transform)
    {                    
    bvector <bvector<bvector<DPoint3d>>> strokesArray;

    curve.CollectLinearGeometry (strokesArray, &facetOptions);

    for (auto& loop : strokesArray)
        {
        for (auto& loopStrokes : loop)
            {
            transform.Multiply(loopStrokes, loopStrokes);

            DRange3d    range = DRange3d::From(loopStrokes);
            strokes.push_back (Strokes::PointList(std::move(loopStrokes), DPoint3d::FromInterpolate(range.low, .5, range.high)));
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
    bool                m_inCache = false;
    bool                m_disjoint;

    PrimitiveGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
        : Geometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry), m_disjoint(disjoint) { }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override;
    bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_geometry); }
    void _SetInCache(bool inCache) override { m_inCache = inCache; }
public:
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
        {
        BeAssert(!disjoint || geometry.GetAsCurveVector().IsValid());
        return new PrimitiveGeometry(geometry, tf, range, elemId, params, isCurved, db, disjoint);
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
        : Geometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid)
        {
        //
        }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
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
    bool                            m_checkGlyphBoxes;

    TextStringGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
        : Geometry(transform, range, elemId, params, true, db), m_text(&text), m_checkGlyphBoxes(checkGlyphBoxes)
        { 
        InitGlyphCurves();     // Should be able to defer this when font threaded ness is resolved.
        }

    bool _DoVertexCluster() const override { return false; }

public:
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
        {
        return new TextStringGeometry(textString, transform, range, elemId, params, db, checkGlyphBoxes);
        }
    
    bool DoGlyphBoxes (IFacetOptionsR facetOptions);
    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override;
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

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override { return m_part->GetPolyfaces(facetOptions, *this, context); }
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override { return m_part->GetStrokes(facetOptions, *this, context); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return m_part->GetFacetCount (counter, *this); }
    GeomPartCPtr _GetPart() const override { return m_part; }

};  // GeomPartInstanceTileGeometry 

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams::DisplayParams(Type type, GraphicParamsCR gfParams, GeometryParamsCP geomParams, bool filled) : m_type(type)
    {
    m_lineColor = gfParams.GetLineColor();
    m_fillColor = m_lineColor;

    if (nullptr != geomParams)
        {
        m_categoryId = geomParams->GetCategoryId();
        m_subCategoryId = geomParams->GetSubCategoryId();
        m_class = geomParams->GetGeometryClass();
        }

    switch (type)
        {
        case Type::Mesh:
            m_material = gfParams.GetMaterial();
            m_gradient = gfParams.GetGradientSymb();
            m_fillColor = gfParams.GetFillColor();

            // We need these as well as line color for edges. Unfortunate side effect: may cause mesh params to compare inequal despite mesh itself not requiring these.
            m_width = gfParams.GetWidth();
            m_linePixels = static_cast<LinePixels>(gfParams.GetLinePixels());

            if (filled)
                {
                m_fillFlags = FillFlags::ByView;
                if (gfParams.IsBlankingRegion())
                    m_fillFlags = m_fillFlags | FillFlags::Blanking;
                }

            if (nullptr != geomParams)
                {
                m_materialId = geomParams->GetMaterialId();
                if (filled)
                    {
                    if (FillDisplay::Always == geomParams->GetFillDisplay())
                        {
                        m_fillFlags = m_fillFlags | FillFlags::Always;
                        m_fillFlags = m_fillFlags & ~FillFlags::ByView;
                        }

                    if (geomParams->IsFillColorFromViewBackground())
                        m_fillFlags = m_fillFlags | FillFlags::Background;
                    }
                }

            if (m_material.IsValid() && m_material->HasTextureMapping())
                {
                // Texture already baked into material...e.g. skybox.
                m_textureMapping = m_material->GetTextureMapping();
                }
            else
                {
                m_resolved = !m_materialId.IsValid() && m_gradient.IsNull();
                if (m_resolved)
                    m_hasRegionOutline = ComputeHasRegionOutline();
                }

            break;
        case Type::Linear:
            m_width = gfParams.GetWidth();
            m_linePixels = static_cast<LinePixels>(gfParams.GetLinePixels());
            m_fillColor = m_lineColor;
            break;
        case Type::Text:
            m_ignoreLighting = true;
            m_fillFlags = FillFlags::Always;
            m_fillColor = m_lineColor;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::CreateForTile(GraphicParamsCR gf, GeometryParamsCP geom, TextureCR texture)
    {
    // This is for Graphic::_AddTile() - a simple quad with an image texture.
    DisplayParamsPtr dp = new DisplayParams(Type::Mesh, gf, geom, true);
    dp->m_fillFlags |= (FillFlags::Always);
    dp->m_textureMapping = TextureMapping(texture, TextureMapping::Params());
    dp->m_resolved = true;
    return dp.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::CreateForGeomPartInstance(DisplayParamsCR part, DisplayParamsCR inst)
    {
    if (part.GetType() == inst.GetType())
        return &inst;

    // We initially create the instance params via CreateForMesh(), because we don't know what type(s) of geometry the part will contain.
    // Need to fix it up for linear/text
    BeAssert(Type::Mesh == inst.GetType());
    DisplayParamsPtr clone(new DisplayParams(part));
    clone->m_fillColor = clone->m_lineColor = inst.m_lineColor;
    clone->m_categoryId = inst.m_categoryId;
    clone->m_subCategoryId = inst.m_subCategoryId;
    clone->m_class = inst.m_class;

    if (Type::Linear == clone->GetType())
        {
        clone->m_width = inst.m_width;
        clone->m_linePixels = inst.m_linePixels;
        }

    return clone.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::ComputeHasRegionOutline() const
    {
    if (m_gradient.IsValid())
        return m_gradient->GetIsOutlined();
    else
        return !NeverRegionOutline() && m_fillColor != m_lineColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::HasRegionOutline() const
    {
    return m_resolved ? m_hasRegionOutline : ComputeHasRegionOutline();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::Clone() const
    {
    BeAssert(m_resolved);
    return new DisplayParams(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayParams::Resolve(DgnDbR db, System& sys)
    {
    if (m_resolved)
        return;

    if (m_gradient.IsValid())
        {
        m_textureMapping = TextureMapping(*sys._GetTexture(*m_gradient, db));
        }
    else if (m_materialId.IsValid())
        {
        auto dgnMaterial = RenderMaterial::Get(db, m_materialId);
        if (dgnMaterial.IsValid())
            {
            // This will also be used later by MeshBuilder...
            auto const& renderingAsset = dgnMaterial->GetRenderingAsset();
            DgnTextureId texId;
            auto const& texMap = renderingAsset.GetPatternMap();
            if (texMap.IsValid() && texMap.GetTextureId().IsValid())
                {
                auto texture = sys._GetTexture(texMap.GetTextureId(), db);
                if (texture.IsValid())
                    m_textureMapping = TextureMapping(*texture, texMap.GetTextureMapParams());
                }
            }
        }

    m_hasRegionOutline = ComputeHasRegionOutline();
    m_resolved = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static int compareValues(T const& lhs, T const& rhs)
    {
    return lhs == rhs ? 0 : (lhs < rhs ? -1 : 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<> int compareValues(bool const& lhs, bool const& rhs)
    {
    return compareValues(static_cast<uint8_t>(lhs), static_cast<uint8_t>(rhs));
    }

#define TEST_LESS_THAN(MEMBER) \
    { \
    int cmp = compareValues(MEMBER, rhs.MEMBER); \
    if (0 != cmp) \
        return cmp < 0; \
    }

#define TEST_EQUAL(MEMBER) if (MEMBER != rhs.MEMBER) return false

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::IsLessThan(DisplayParamsCR rhs, ComparePurpose purpose) const
    {
    if (&rhs == this)
        return false;

    TEST_LESS_THAN(GetType());
    TEST_LESS_THAN(IgnoresLighting());
    TEST_LESS_THAN(GetLineWidth());
    TEST_LESS_THAN(GetMaterialId());
    TEST_LESS_THAN(GetLinePixels());
    TEST_LESS_THAN(GetFillFlags());
    TEST_LESS_THAN(HasRegionOutline());

    if (m_resolved && rhs.m_resolved)
        {
        TEST_LESS_THAN(GetTextureMapping().GetTexture()); // ###TODO: Care about whether params match?
        }
    else if (m_gradient.get() != rhs.m_gradient.get())
        {
        if (m_gradient.IsNull() || rhs.m_gradient.IsNull() || !(*m_gradient == *rhs.m_gradient))
            return m_gradient.get() < rhs.m_gradient.get();
        }

    if (ComparePurpose::Merge == purpose)
        {
        TEST_LESS_THAN(HasFillTransparency());
        TEST_LESS_THAN(HasLineTransparency());

        if (GetTextureMapping().IsValid())
            TEST_LESS_THAN(GetFillColor());     // Textures may use color so they can't be merged. (could test if texture actually uses color).

        return false;
        }

    TEST_LESS_THAN(GetFillColor());
    TEST_LESS_THAN(GetLineColor());
    TEST_LESS_THAN(GetCategoryId().GetValueUnchecked());
    TEST_LESS_THAN(GetSubCategoryId().GetValueUnchecked());
    TEST_LESS_THAN(GetClass());

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::IsEqualTo(DisplayParamsCR rhs, ComparePurpose purpose) const
    {
    if (&rhs == this)
        return true;

    TEST_EQUAL(GetType());
    TEST_EQUAL(IgnoresLighting());
    TEST_EQUAL(GetLineWidth());
    TEST_EQUAL(GetMaterialId().GetValueUnchecked());
    TEST_EQUAL(GetLinePixels());
    TEST_EQUAL(GetFillFlags());
    TEST_EQUAL(HasRegionOutline());

    if (m_resolved && rhs.m_resolved)
        {
        TEST_EQUAL(GetTextureMapping().GetTexture()); // ###TODO: Care about whether params match?
        }
    else
        {
        if (m_gradient.IsNull() != rhs.m_gradient.IsNull())
            return false;
        else if (m_gradient.IsValid() && m_gradient.get() != rhs.m_gradient.get() && !(*m_gradient == *rhs.m_gradient))
            return false;
        }

    if (ComparePurpose::Merge == purpose)
        {
        TEST_EQUAL(HasFillTransparency());
        TEST_EQUAL(HasLineTransparency());
        if (GetTextureMapping().IsValid())
            TEST_EQUAL(GetFillColor());     // Textures may use color so they can't be merged. (could test if texture actually uses color).

        return true;
        }

    TEST_EQUAL(GetFillColor());
    TEST_EQUAL(GetLineColor());
    TEST_EQUAL(GetCategoryId().GetValueUnchecked());
    TEST_EQUAL(GetSubCategoryId().GetValueUnchecked());
    TEST_EQUAL(GetClass());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCR DisplayParamsCache::Get(DisplayParamsR toFind)
    {
    BeAssert(0 == toFind.GetRefCount()); // allocated on stack...
    toFind.AddRef();
    DisplayParamsCPtr pToFind(&toFind);
    auto iter = m_set.find(pToFind);
    if (m_set.end() == iter)
        {
        toFind.Resolve(m_db, m_system);
        BeAssert(toFind.m_resolved);
        iter = m_set.insert(toFind.Clone()).first;
        }

    return **iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d Mesh::GetPoint(uint32_t index) const
    {
    return Points().Unquantize(index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetTriangleRange(TriangleCR triangle) const
    {
    return DRange3d::From(GetPoint(triangle[0]),
                          GetPoint(triangle[1]),
                          GetPoint(triangle[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d Mesh::GetTriangleNormal(TriangleCR triangle) const
    {
    return DVec3d::FromNormalizedCrossProductToPoints(GetPoint(triangle[0]),
                                                      GetPoint(triangle[1]),
                                                      GetPoint(triangle[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Mesh::HasNonPlanarNormals() const
    {
    if (m_normals.empty())
        return false;

    OctEncodedNormal    normals[3];
    uint32_t const*     pIndex = m_triangles.Indices().data();
    uint32_t const*     pEnd = pIndex + m_triangles.Indices().size();

    for (pIndex = 0; pIndex < pEnd; pIndex += 3)
        {
        normals[0] = m_normals[pIndex[0]];
        normals[1] = m_normals[pIndex[1]];
        if (normals[0] != normals[1])
            return true;

        normals[2] = m_normals[pIndex[2]];
        if (normals[0] != normals[2] || normals[1] != normals[2])
            return true;
        }

    return false;
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::GetGraphics (bvector<Render::GraphicPtr>& graphics, Dgn::Render::SystemCR system, GetMeshGraphicsArgs& args, DgnDbR db) const
    {
    bool haveMesh = !Triangles().Empty();
    bool havePolyline = !haveMesh && !Polylines().empty();

    if (!haveMesh && !havePolyline)
        return;                                                                   

    Render::GraphicPtr thisGraphic;

    if (haveMesh)
        {
        if (args.m_meshArgs.Init(*this) &&
            (thisGraphic = system._CreateTriMesh(args.m_meshArgs, db)).IsValid())
            graphics.push_back (thisGraphic);

        MeshEdgesPtr    edges = GetEdges();

        if (args.m_visibleEdgesArgs.Init(*this) &&
            (thisGraphic = system._CreateVisibleEdges(args.m_visibleEdgesArgs, db)).IsValid())
            graphics.push_back(thisGraphic);

        if (args.m_invisibleEdgesArgs.Init(*this) &&
            (thisGraphic = system._CreateSilhouetteEdges(args.m_invisibleEdgesArgs, db)).IsValid())
            graphics.push_back(thisGraphic);

        if (args.m_polylineEdgesArgs.Init(*this) &&
            (thisGraphic = system._CreateIndexedPolylines(args.m_polylineEdgesArgs, db)).IsValid())
            graphics.push_back(thisGraphic);
        }
    else                           
        {
        if (args.m_polylineArgs.Init(*this) &&
            (thisGraphic = system._CreateIndexedPolylines(args.m_polylineArgs, db)).IsValid())
            graphics.push_back(thisGraphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, typename U> static void insertVertexAttribute(bvector<uint16_t>& indices, T& table, U const& value, QVertex3dListCR vertices)
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
uint32_t Mesh::AddVertex(QVertex3dCR vert, OctEncodedNormalCP normal, DPoint2dCP param, uint32_t fillColor, FeatureCR feature)
    {
    auto index = static_cast<uint32_t>(m_verts.size());

    m_verts.Add(vert);
    m_features.Add(feature, m_verts.size());

    if (nullptr != normal)
        m_normals.push_back(*normal);
                                                                                                                 
    if (nullptr != param)
        m_uvParams.push_back(toFPoint2d(*param));

    insertVertexAttribute(m_colors, m_colorTable, fillColor, m_verts);
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
    uint32_t index = m_table->GetIndex(feat);
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
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::Features::SetIndices(bvector<uint32_t>&& indices)
    {
    if (indices.empty())
        {
        BeAssert(false);
        m_initialized = false;
        }
    else if (1 == indices.size())
        {
        m_uniform = indices.front();
        }
    else
        {
        m_indices = std::move(indices);
        }
    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetRange() const
    {
    DRange3d range = DRange3d::NullRange();
    auto const& points = Points();
    size_t nPoints = points.size();
    for (size_t i = 0; i < nPoints; i++)
        range.Extend(points.Unquantize(i));

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
    if (triangle[0] < triangle[1])
        {
        if (triangle[0] < triangle[2])
            {
            m_sortedIndices[0] = triangle[0];
            if (triangle[1] < triangle[2])
                {
                m_sortedIndices[1] = triangle[1];
                m_sortedIndices[2] = triangle[2];
                }
            else
                {
                m_sortedIndices[1] = triangle[2];
                m_sortedIndices[2] = triangle[1];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle[2];
            m_sortedIndices[1] = triangle[0];
            m_sortedIndices[2] = triangle[1];
            }
        }
    else
        {
        if (triangle[1] < triangle[2])
            {
            m_sortedIndices[0] = triangle[1];
            if (triangle[0] < triangle[2])
                {
                m_sortedIndices[1] = triangle[0];
                m_sortedIndices[2] = triangle[2];
                }
            else
                {
                m_sortedIndices[1] = triangle[2];
                m_sortedIndices[2] = triangle[0];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle[2];
            m_sortedIndices[1] = triangle[1];
            m_sortedIndices[2] = triangle[0];
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
    // We never merge meshes where these differ...
    BeAssert(lhs.m_normalValid == rhs.m_normalValid);
    BeAssert(lhs.m_paramValid == rhs.m_paramValid);

    if (lhs.m_position.IsQuantized())
        {
        if (!rhs.m_position.IsQuantized())
            return false;

        auto const& lpos = lhs.m_position.GetQPoint3d();
        auto const& rpos = rhs.m_position.GetQPoint3d();

        COMPARE_VALUES(lpos.x, rpos.x);
        COMPARE_VALUES(lpos.y, rpos.y);
        COMPARE_VALUES(lpos.z, rpos.z);
        }
    else
        {
        if (rhs.m_position.IsQuantized())
            return true;

        auto const& lpos = lhs.m_position.GetFPoint3d();
        auto const& rpos = rhs.m_position.GetFPoint3d();

        // ###TODO: May want to use a tolerance here; if so must be relative to range...
        COMPARE_VALUES(lpos.x, rpos.x);
        COMPARE_VALUES(lpos.y, rpos.y);
        COMPARE_VALUES(lpos.z, rpos.z);
        }

    COMPARE_VALUES (lhs.m_fillColor, rhs.m_fillColor);
    COMPARE_VALUES (lhs.m_feature.GetElementId(), rhs.m_feature.GetElementId());

    if (lhs.m_normalValid)
        COMPARE_VALUES(lhs.m_normal, rhs.m_normal);

    COMPARE_VALUES (lhs.m_feature.GetSubCategoryId(), rhs.m_feature.GetSubCategoryId());

    constexpr double s_paramTolerance  = .1;
    if (lhs.m_paramValid)
        {
        BeAssert(rhs.m_paramValid);
        COMPARE_VALUES_TOLERANCE (lhs.m_param.x, rhs.m_param.x, s_paramTolerance);
        COMPARE_VALUES_TOLERANCE (lhs.m_param.y, rhs.m_param.y, s_paramTolerance);
        }

    COMPARE_VALUES (lhs.m_feature.GetClass(), rhs.m_feature.GetClass());

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
VertexKey::VertexKey(DPoint3dCR point, FeatureCR feature, uint32_t fillColor, QPoint3d::ParamsCR qParams, DVec3dCP normal, DPoint2dCP param)
    : m_position(point, qParams), m_fillColor(fillColor), m_feature(feature), m_normalValid(nullptr != normal), m_paramValid(nullptr != param)
    {
    if (m_normalValid)
        m_normal.InitFrom(*normal);

    if (m_paramValid)
        m_param = *param;
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
* @bsimethod                                                    Ray.Bentley     07/017
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddFromPolyfaceVisitor(PolyfaceVisitorR visitor, TextureMappingCR mappedTexture, DgnDbR dgnDb, FeatureCR feature, bool doVertexCluster, bool includeParams, uint32_t fillColor)
    {
    auto const&     points = visitor.Point();
    bool const*     visitorVisibility = visitor.GetVisibleCP();
    size_t          nTriangles = points.size() - 2;
    
    // The face represented by this visitor should be convex (we request that in facet options) - so we do a simple fan triangulation.
    for (size_t iTriangle =0; iTriangle < nTriangles; iTriangle++)
        {
        if (doVertexCluster)
            {
            DVec3d      cross;

            cross.CrossProductToPoints (points.at(0), points.at(iTriangle+1), points.at(iTriangle+2));
            if (cross.MagnitudeSquared() < m_areaTolerance)
                return;
            }

        Triangle            newTriangle(!visitor.GetTwoSided());
        bvector<DPoint2d>   params = visitor.Param();
        bool                visibility[3];

        visibility[0] = (0 == iTriangle) ? visitorVisibility[0] : false;
        visibility[1] = visitorVisibility[iTriangle+1];
        visibility[2] = (iTriangle == nTriangles-1) ? visitorVisibility[iTriangle+2] : false;

        bool haveParams = includeParams && !params.empty();
        newTriangle.SetEdgeFlags(visibility);
        if (haveParams && mappedTexture.IsValid())
            {
            auto const&         textureMapParams = mappedTexture.GetParams();
            bvector<DPoint2d>   computedParams;

            BeAssert (m_mesh->Verts().empty() || !m_mesh->Params().empty());
            if (SUCCESS == textureMapParams.ComputeUVParams (computedParams, visitor))
                params = computedParams;
            }
                
        bool haveNormals = !visitor.Normal().empty();

        for (size_t i = 0; i < 3; i++)
            {
            size_t      index = (0 == i) ? 0 : iTriangle + i; 
            VertexKey   vertex(points[index], feature, fillColor, m_mesh->Verts().GetParams(), haveNormals ? &visitor.Normal()[index] : nullptr, haveParams ? &params[index] : nullptr);

            newTriangle[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
            if (m_currentPolyface.IsValid())
                m_currentPolyface->m_vertexIndexMap.Insert(newTriangle[i], visitor.ClientPointIndex()[index]);
            }

        AddTriangle(newTriangle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline (bvector<DPoint3d>const& points, FeatureCR feature, bool doVertexCluster, uint32_t fillColor, double startDistance, DPoint3dCR  rangeCenter)
    {
    MeshPolyline    newPolyline(startDistance, FPoint3d::From(rangeCenter));

    for (auto& point : points)
        {
        VertexKey vertex(point, feature, fillColor, m_mesh->Verts().GetParams());
        newPolyline.GetIndices().push_back (doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }

    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::AddPolyline(MeshPolylineCR polyline) 
    { 
    BeAssert(PrimitiveType::Polyline == GetType() || PrimitiveType::Point == GetType()); 
    
    if (PrimitiveType::Polyline == GetType() && polyline.GetIndices().size() < 2)
        {
        BeAssert(false);
        return;
        }
        

    m_polylines.push_back(polyline); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MeshBuilder::AddVertex(VertexMap& verts, VertexKey const& vertex)
    {
    // Avoid doing lookup twice - once to find existing, again to add if not present
    auto index = static_cast<uint32_t>(m_mesh->Verts().size());
    auto insertPair = verts.Insert(vertex, index);
    if (insertPair.second)
        m_mesh->AddVertex(vertex.m_position, vertex.GetNormal(), vertex.GetParam(), vertex.m_fillColor, vertex.m_feature);

    return insertPair.first->second;
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
    return m_geometries.ContainsCurves();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList GeomPart::GetPolyfaces(IFacetOptionsR facetOptions, GeometryCR instance, ViewContextR context)
    {
    return GetPolyfaces(facetOptions, &instance, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList GeomPart::GetPolyfaces(IFacetOptionsR facetOptions, GeometryCP instance, ViewContextR context)
    {
    PolyfaceList polyfaces;
    for (auto& geometry : m_geometries) 
        {
        BeAssert(geometry->GetTransform().IsIdentity());
        PolyfaceList thisPolyfaces = geometry->GetPolyfaces (facetOptions, context);

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
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomPart::SetInCache(bool inCache)
    {
    for (auto& geometry : m_geometries)
        geometry->SetInCache(inCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes (IFacetOptionsR facetOptions, GeometryCR instance, ViewContextR context)
    {
    return GetStrokes(facetOptions, &instance, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes(IFacetOptionsR facetOptions, GeometryCP instance, ViewContextR context)
    {
    StrokesList strokes;

    for (auto& geometry : m_geometries) 
        {
        StrokesList   thisStrokes = geometry->GetStrokes(facetOptions, context);

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
    : m_params(&params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(params.IsTextured())
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
    opts->SetMaxPerFace(100);
    opts->SetConvexFacetsRequired(true);             // Defer triangulation of facets to simple fans.
    opts->SetCurvedSurfaceMaxPerFace(3);
    opts->SetParamsRequired(true);
    opts->SetNormalsRequired(true);
    opts->SetEdgeChainsRequired(true);

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
PolyfaceList Geometry::GetPolyfaces(double chordTolerance, NormalMode normalMode, ViewContextR context)
    {
    return _GetPolyfaces(*CreateFacetOptions(chordTolerance, normalMode), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Strokes::Transform(TransformCR transform)
    {
    for (auto& stroke : m_strokes)
        transform.Multiply (stroke.m_points, stroke.m_points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList PrimitiveGeometry::_GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    
    if (polyface.IsValid())
        {
        if (m_inCache)
            polyface = polyface->Clone();

        if (!HasTexture())
            polyface->ClearParameters(false);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return PolyfaceList (1, Polyface(GetDisplayParams(), *polyface));
        }

    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();

    if (curveVector.IsValid() && !curveVector->IsAnyRegionType())       // Non region curveVectors....
        return PolyfaceList();

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    ISolidPrimitivePtr  solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    // According to Brien we should not even bother checking planar.
    // The planar flag exists to allow us to address z-fighting when shapes are sketched onto surfaces (e.g. for push-pull modeling)
    bool isPlanar = curveVector.IsValid();
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
        if (!GetTransform().IsIdentity())
            polyface->Transform(GetTransform());

        // If there is a region outline, we will generate it separately as a polyline in _GetStrokes().
        // If there is no region outline, and never should be one, don't generate edges
        // See: text background (or anything else with blanking fill)
        DisplayParamsCR params = GetDisplayParams();
        bool wantEdges = curveVector.IsNull() || (!params.HasBlankingFill() && !params.HasRegionOutline());
        polyfaces.push_back(Polyface(GetDisplayParams(), *polyface, wantEdges, isPlanar));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList PrimitiveGeometry::_GetStrokes (IFacetOptionsR facetOptions, ViewContextR context)
    {
    StrokesList             tileStrokes;
    CurveVectorPtr          curveVector = m_geometry->GetAsCurveVector();
    
    if (!curveVector.IsValid())
        return tileStrokes;

    Strokes::PointLists     strokePoints;

    if (! curveVector->IsAnyRegionType() || GetDisplayParams().HasRegionOutline())
        {
        strokePoints.clear();
        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            {
            bool isPlanar = curveVector->IsAnyRegionType();
            BeAssert(isPlanar == GetDisplayParams().HasRegionOutline());
            tileStrokes.push_back(Strokes(GetDisplayParams(), std::move(strokePoints), m_disjoint, isPlanar));
            }
        }

    return tileStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList SolidKernelGeometry::_GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context)
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
        baseParams.SetGeometryClass(GetDisplayParams().GetClass());

        BeAssert(nullptr != context.GetRenderSystem());
        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {
                GeometryParams faceParams;
                params[i].ToGeometryParams(faceParams, baseParams);
                
                GraphicParams gfParams;
                context.CookGeometryParams(faceParams, gfParams);

                auto displayParams = DisplayParams::CreateForMesh(gfParams, &faceParams, false, context.GetDgnDb(), *context.GetRenderSystem());
                tilePolyfaces.push_back (Polyface(*displayParams, *polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (Polyface(GetDisplayParams(), *polyface));
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
GeometryPtr Geometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
    {
    return TextStringGeometry::Create(textString, transform, range, entityId, params, db, checkGlyphBoxes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
    {
    return PrimitiveGeometry::Create(geometry, tf, range, entityId, params, isCurved, db, disjoint);
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
bool GeometryAccumulator::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, bool disjoint)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;

    auto tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    return AddGeometry(geom, isCurved, displayParams, tf, range, disjoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, DRange3dCR range, bool disjoint)
    {
    GeometryPtr geometry = Geometry::Create(geom, transform, range, GetElementId(), displayParams, isCurved, GetDgnDb(), disjoint);
    if (geometry.IsNull())
        return false;

    m_geometries.push_back(*geometry);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* NB: This is just for testing the performance impact of cloning geometry (which is
* unnecessary in most cases).
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> RefCountedPtr<T> cloneGeometry(T const& geom)
    {
#if defined(ETT_PROFILE_CLONE)
    return const_cast<T*>(&geom);
#else
    return geom.Clone();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr cloneGeometry(PolyfaceQueryCR query)
    {
#if defined(ETT_PROFILE_CLONE)
    auto header = dynamic_cast<PolyfaceHeaderCP>(&query);
    if (nullptr != header)
        return const_cast<PolyfaceHeaderP>(header);
    else
#endif
        return query.Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(CurveVectorR curves, bool filled, DisplayParamsCR displayParams, TransformCR transform, bool disjoint)
    {
    if (m_surfacesOnly && !curves.IsAnyRegionType())
        return true;    // ignore...

    bool isCurved = curves.ContainsNonLinearPrimitive();
    IGeometryPtr geom = IGeometry::Create(CurveVectorPtr(&curves));
    return AddGeometry(*geom, isCurved, displayParams, transform, disjoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(ISolidPrimitiveR primitive, DisplayParamsCR displayParams, TransformCR transform)
    {
    bool isCurved = primitive.HasCurvedFaceOrEdge();
    IGeometryPtr geom = IGeometry::Create(ISolidPrimitivePtr(&primitive));
    return AddGeometry(*geom, isCurved, displayParams, transform, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(RefCountedMSBsplineSurface& surface, DisplayParamsCR displayParams, TransformCR transform)
    {
    bool isCurved = (surface.GetUOrder() > 2 || surface.GetVOrder() > 2);
    IGeometryPtr geom = IGeometry::Create(MSBsplineSurfacePtr(&surface));
    return AddGeometry(*geom, isCurved, displayParams, transform, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(PolyfaceHeaderR polyface, bool filled, DisplayParamsCR displayParams, TransformCR transform)
    {
    if (m_haveTransform)
        polyface.Transform(Transform::FromProduct(m_transform, transform));
    else if (!transform.IsIdentity())
        polyface.Transform(transform);

    DRange3d range = polyface.PointRange();
    IGeometryPtr geom = IGeometry::Create(PolyfaceHeaderPtr(&polyface));
    AddGeometry(*geom, false, displayParams, Transform::FromIdentity(), range, false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(IBRepEntityR body, DisplayParamsCR displayParams, TransformCR transform)
    {
    DRange3d range = body.GetEntityRange();
    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    m_geometries.push_back(*Geometry::Create(body, tf, range, GetElementId(), displayParams, GetDgnDb()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(TextStringR textString, DisplayParamsCR displayParams, TransformCR transform)
    {
    if (m_surfacesOnly)
        return true;

    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;

    DRange2d range2d;
        {
        // ###TODO: Fonts are a freaking mess.
        BeMutexHolder lock(DgnFonts::GetMutex());
        range2d = textString.GetRange();
        }

    DRange3d range = DRange3d::From(range2d.low.x, range2d.low.y, 0.0, range2d.high.x, range2d.high.y, 0.0);
    Transform::FromProduct(tf, textString.ComputeTransform()).Multiply(range, range);

    m_geometries.push_back(*Geometry::Create(textString, tf, range, GetElementId(), displayParams, GetDgnDb(), m_checkGlyphBoxes));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList GeometryAccumulator::ToMeshes(GeometryOptionsCR options, double tolerance, ViewContextR context) const
    {
    MeshList meshes;
    if (m_geometries.empty())
        return meshes;

    double vertexTolerance = tolerance * ToleranceRatio::Vertex();
    double facetAreaTolerance = tolerance * tolerance * ToleranceRatio::FacetArea();

    DRange3d range = m_geometries.ComputeRange();
    bool is2d = range.IsAlmostZeroZ();
    bmap<MeshMergeKey, MeshBuilderPtr> builderMap;
    for (auto const& geom : m_geometries)
        {
        auto polyfaces = geom->GetPolyfaces(tolerance, options.m_normalMode, context);
        for (auto const& tilePolyface : polyfaces)
            {
            PolyfaceHeaderPtr polyface = tilePolyface.m_polyface;
            if (polyface.IsNull() || 0 == polyface->GetPointCount())
                continue;

            DisplayParamsCPtr displayParams = tilePolyface.m_displayParams;
            bool hasTexture = displayParams.IsValid() && displayParams->IsTextured();

            MeshMergeKey key(*displayParams, nullptr != polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, tilePolyface.m_isPlanar);

            MeshBuilderPtr meshBuilder;
            auto found = builderMap.find(key);
            if (builderMap.end() != found)
                meshBuilder = found->second;
            else
                builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance, nullptr, Mesh::PrimitiveType::Mesh, range, is2d, tilePolyface.m_isPlanar);

            uint32_t fillColor = displayParams->GetFillColor();

            meshBuilder->BeginPolyface(*polyface, tilePolyface.m_displayEdges ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges);
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                meshBuilder->AddFromPolyfaceVisitor(*visitor, displayParams->GetTextureMapping(), GetDgnDb(), geom->GetFeature(), false, hasTexture, fillColor);

            meshBuilder->EndPolyface();
            }

        if (!options.WantSurfacesOnly())
            {
            auto tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions(tolerance, NormalMode::Never), context);
            for (auto& tileStrokes : tileStrokesArray)
                {
                DisplayParamsCPtr displayParams = tileStrokes.m_displayParams;
                MeshMergeKey key(*displayParams, false, tileStrokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline, tileStrokes.m_isPlanar);

                MeshBuilderPtr meshBuilder;
                auto found = builderMap.find(key);
                if (builderMap.end() != found)
                    meshBuilder = found->second;
                else
                    builderMap[key] = meshBuilder = MeshBuilder::Create(*displayParams, vertexTolerance, facetAreaTolerance, nullptr, key.m_primitiveType, range, is2d, tileStrokes.m_isPlanar);

                uint32_t fillColor = displayParams->GetLineColor();
                for (auto& strokePoints : tileStrokes.m_strokes)
                    meshBuilder->AddPolyline(strokePoints.m_points, geom->GetFeature(), false, fillColor, strokePoints.m_startDistance, strokePoints.m_rangeCenter);
                }
            }
        }

    for (auto& builder : builderMap)
        {
        MeshP mesh = builder.second->GetMesh();
        if (!mesh->IsEmpty())
            {
            mesh->Close();
            meshes.push_back(mesh);
            }
        }

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryAccumulator::SaveToGraphicList(bvector<GraphicPtr>& graphics, GeometryOptionsCR options, double tolerance, ViewContextR context) const
    {
    MeshList                meshes = ToMeshes(options, tolerance, context);
    GetMeshGraphicsArgs     args;

    for (auto const& mesh : meshes)
        mesh->GetGraphics (graphics, GetSystem(), args, GetDgnDb());
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
    if (empty())
        {
        BeAssert(false && "empty color table");
        }
    else if (IsUniform())
        {
        index.SetUniform(m_map.begin()->first);
        }
    else
        {
        BeAssert(!indices.empty());

        colors.resize(size());
        for (auto const& kvp : *this)
            colors[kvp.second] = kvp.first;

        index.SetNonUniform(GetNumIndices(), colors.data(), indices.data(), m_hasAlpha);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextStringGeometry::DoGlyphBoxes (IFacetOptionsR facetOptions)
    {
    if (!m_checkGlyphBoxes)
        return false;

    DRange2d            textRange = m_text->GetRange();
    double              minDimension = std::min (textRange.high.x - textRange.low.x, textRange.high.y - textRange.low.y) * GetTransform().ColumnXMagnitude();
    static const double s_minGlyphRatio = 1.0; 
    
    return minDimension < s_minGlyphRatio * facetOptions.GetChordTolerance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList TextStringGeometry::_GetPolyfaces(IFacetOptionsR facetOptionsIn, ViewContextR context)
    {
    PolyfaceList                polyfaces;
    IFacetOptionsPtr            facetOptions = facetOptionsIn.Clone();

    //facetOptions->SetNormalsRequired(false);     // No lighting so normals not required.

    IPolyfaceConstructionPtr    polyfaceBuilder = IPolyfaceConstruction::Create(*facetOptions);
    if (DoGlyphBoxes(*facetOptions))
        {
        // ###TODO: Fonts are a freaking mess.
        BeMutexHolder lock(DgnFonts::GetMutex());

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
        polyfaces.push_back (Polyface(GetDisplayParams(), *polyface, false, true));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList TextStringGeometry::_GetStrokes (IFacetOptionsR facetOptions, ViewContextR context)
    {
    StrokesList strokes;

    if (DoGlyphBoxes(facetOptions))
        return strokes;

    InitGlyphCurves();

    Strokes::PointLists         strokePoints;
    Transform                   transform = Transform::FromProduct (GetTransform(), m_text->ComputeTransform());

    for (auto& glyphCurve : m_glyphCurves)
        if (!glyphCurve->IsAnyRegionType())
            collectCurveStrokes(strokePoints, *glyphCurve, facetOptions, transform);

    if (!strokePoints.empty())
        strokes.push_back(Strokes(GetDisplayParams(), std::move(strokePoints), false, true));

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

    // ###TODO: Fonts are a freaking mess.
    BeMutexHolder lock(DgnFonts::GetMutex());

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
bool MeshArgs::Init(MeshCR mesh)
    {
    Clear();
    if (mesh.Triangles().Empty())
        return false;

    m_pointParams = mesh.Points().GetParams();

    Set(m_numIndices, m_vertIndex, mesh.Triangles().Indices());
    Set(m_numPoints, m_points, mesh.Points());
    Set(m_textureUV, mesh.Params());
    if (!mesh.GetDisplayParams().IgnoresLighting())    // ###TODO: Avoid generating normals in the first place if no lighting...
        Set(m_normals, mesh.Normals());

    m_texture = const_cast<TextureP>(mesh.GetDisplayParams().GetTextureMapping().GetTexture()); // ###TODO: constness...
    m_material = mesh.GetDisplayParams().GetMaterial();
    m_fillFlags = mesh.GetDisplayParams().GetFillFlags();
    m_isPlanar = mesh.IsPlanar();
    m_edgeWidth = mesh.GetDisplayParams().GetLineWidth();
    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());

    mesh.ToFeatureIndex(m_features);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshArgs::Clear()
    {
    m_numIndices = 0;
    m_vertIndex = nullptr;
    m_numPoints = 0;
    m_points = nullptr;
    m_normals = nullptr;
    m_textureUV = nullptr;
    m_texture = nullptr;
    m_isPlanar = false;

    m_colors.Reset();
    m_colorTable.clear();
    m_features.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void initLinearGraphicParams(T& args, MeshCR mesh)
    {
    args.m_linePixels = mesh.GetDisplayParams().GetLinePixels();
    args.m_width = mesh.GetDisplayParams().GetLineWidth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ElementMeshEdgeArgs::Init(MeshCR mesh)
    {
    MeshEdgesPtr    meshEdges = mesh.GetEdges();
    m_pointParams = mesh.Points().GetParams();

    initLinearGraphicParams(*this, mesh);

    if (!meshEdges.IsValid() || meshEdges->m_visible.empty())
        return false;

    m_points    = mesh.Points().data();
    m_edges     = meshEdges->m_visible.data();
    m_numEdges  = meshEdges->m_visible.size();
    m_isPlanar  = mesh.IsPlanar();
    
    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ElementSilhouetteEdgeArgs::Init(MeshCR mesh)
    {
    MeshEdgesPtr    meshEdges = mesh.GetEdges();
    m_pointParams = mesh.Points().GetParams();

    initLinearGraphicParams(*this, mesh);

    if (!meshEdges.IsValid() || meshEdges->m_silhouette.empty())
        return false;

    m_points    = mesh.Points().data();
    m_normals   = meshEdges->m_silhouetteNormals.data();
    m_edges     = meshEdges->m_silhouette.data();
    m_numEdges  = meshEdges->m_silhouette.size();

    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ElementPolylineEdgeArgs::Init(MeshCR mesh)
    {
    Reset();
    MeshEdgesPtr    meshEdges = mesh.GetEdges();
    m_pointParams = mesh.Points().GetParams();

    initLinearGraphicParams(*this, mesh);

    if (!meshEdges.IsValid() || meshEdges->m_polylines.empty())
        return false;

    m_disjoint = false;
    m_isEdge = true;
    m_is2d = mesh.Is2d();
    m_isPlanar  = mesh.IsPlanar();

    m_polylines.reserve(meshEdges->m_polylines.size());

    for (auto& polyline : meshEdges->m_polylines)
        {
        IndexedPolyline indexedPolyline;

        if (indexedPolyline.Init(polyline.GetIndices(), polyline.GetStartDistance(), polyline.GetRangeCenter()))
            m_polylines.push_back(indexedPolyline);
        }
                                                
    FinishInit(mesh);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineArgs::Reset()
    {
    m_disjoint = m_is2d = m_isPlanar = false;
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

    initLinearGraphicParams(*this, mesh);

    m_is2d = mesh.Is2d();
    m_isPlanar = mesh.IsPlanar();
    m_disjoint = Mesh::PrimitiveType::Point == mesh.GetType();
    m_isEdge = mesh.GetDisplayParams().HasRegionOutline();

    m_polylines.reserve(mesh.Polylines().size());

    for (auto const& polyline : mesh.Polylines())
        {
        IndexedPolyline indexedPolyline;
        if (indexedPolyline.Init(polyline))
            m_polylines.push_back(indexedPolyline);
        }
    if (!IsValid())
        return false;

    FinishInit(mesh);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineArgs::FinishInit(MeshCR mesh)
    {
    m_pointParams = mesh.Points().GetParams();
    m_numPoints = static_cast<uint32_t>(mesh.Points().size());
    m_points = &mesh.Points()[0];

    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    m_numLines = static_cast<uint32_t>(m_polylines.size());
    m_lines = &m_polylines[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_ActivateGraphicParams(GraphicParamsCR gfParams, GeometryParamsCP geomParams)
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
    DPoint3dCP pts = corners.m_pts;
    rasterTile.m_pointParams = QPoint3d::Params(DRange3d::From(pts, 4));
    QPoint3d vertex[4];
    for (uint32_t i = 0; i < 4; ++i)
        vertex[i] = QPoint3d(pts[i], rasterTile.m_pointParams);

    rasterTile.m_points = vertex;
    rasterTile.m_numPoints = 4;

    static uint32_t indices[] = {0,1,2,2,1,3};
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
    rasterTile.m_material = params.GetMaterial(); // not likely...
    rasterTile.m_isPlanar = true;

    return _CreateTriMesh(rasterTile, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                1/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddTile(TextureCR tile, TileCorners const& corners)
    {
    GraphicPtr gf = GetSystem()._CreateTile(tile, corners, GetDgnDb(), GetGraphicParams());
    if (gf.IsValid())
        m_primitives.push_back(gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::AddTriMesh(TriMeshArgsCR args)
    {
    // ###TODO: this is weird and not yet used...do we take the texture/material/etc from the args, or set them from the active GraphicParams?
    GraphicPtr gf = GetSystem()._CreateTriMesh(args, GetDgnDb());
    if (gf.IsValid())
        m_primitives.push_back(gf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddShape(int numPoints, DPoint3dCP points, bool filled)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, numPoints));
    m_accum.Add(*curve, filled, GetMeshDisplayParams(filled), GetLocalToWorldTransform(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::AddTile(TileCorners const& corners, DisplayParamsCR params)
    {
    DPoint3d shapePoints[5] =
        {
        corners.m_pts[0],
        corners.m_pts[1],
        corners.m_pts[3],
        corners.m_pts[2],
        corners.m_pts[0]
        };

    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(shapePoints, 5));
    m_accum.Add(*curve, false, params, GetLocalToWorldTransform(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double priority)
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
void GeometryListBuilder::_AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled)
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

    m_accum.Add(*curve, filled, curve->IsAnyRegionType() ? GetMeshDisplayParams(filled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddLineString2d(int numPoints, DPoint2dCP points, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddLineString(numPoints, &pts3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddLineString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, ICurvePrimitive::CreateLineString(points, numPoints));
    _AddCurveVectorR(*curve, false); // possibly a zero-length line segment (== disjoint)...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPointString2d(int numPoints, DPoint2dCP points, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddPointString(numPoints, &pts3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPointString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, ICurvePrimitive::CreatePointString(points, numPoints));
    m_accum.Add(*curve, false, GetLinearDisplayParams(), GetLocalToWorldTransform(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPolyface(PolyfaceQueryCR meshDataIn, bool filled)
    {
    _AddPolyfaceR(*meshDataIn.Clone(), filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPolyfaceR(PolyfaceHeaderR mesh, bool filled)
    {
    Add(mesh, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBody(IBRepEntityCR entity)
    {
    _AddBodyR(const_cast<IBRepEntityR>(entity));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBodyR(IBRepEntityR entity)
    {
    m_accum.Add(entity, GetMeshDisplayParams(false), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddShape2d(int numPoints, DPoint2dCP points, bool filled, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(numPoints, &pts3d[0], points, priority);
    _AddShape(numPoints, &pts3d[0], filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextString(TextStringCR text)
    {
    // ###TODO_ELEMENT_TILE: May want to treat as box if too small...
    _AddTextStringR(*text.Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextStringR(TextStringR text)
    {
    // ###TODO_ELEMENT_TILE: May want to treat as box if too small...
    m_accum.Add(text, GetTextDisplayParams(), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextString2d(TextStringCR text, double priority)
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
        _AddTextStringR(*ts);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextString2dR(TextStringR text, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddTextStringR(text);
        }
    else
        {
        TextStringPtr ts = text.Clone();
        auto origin = ts->GetOrigin();
        origin.z = zDepth;
        ts->SetOrigin(origin);
        _AddTextStringR(*ts);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine usageFlags)
    {
    if (AsThickenedLine::Yes == usageFlags) // represents thickened line...
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
void GeometryListBuilder::_AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine usageFlags, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddTriStrip(numPoints, &pts3d[0], usageFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddSolidPrimitive(ISolidPrimitiveCR primitive)
    {
    _AddSolidPrimitiveR(*primitive.Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddSolidPrimitiveR(ISolidPrimitiveR primitive)
    {
    m_accum.Add(primitive, GetMeshDisplayParams(false), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVector(CurveVectorCR curves, bool isFilled)
    {
    _AddCurveVectorR(*curves.Clone(), isFilled);
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
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::AddCurveVector(CurveVectorR curves, bool isFilled, bool disjoint)
    {
    BeAssert(!isFilled || !disjoint);
    m_accum.Add(curves, isFilled, curves.IsAnyRegionType() ? GetMeshDisplayParams(isFilled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), disjoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVectorR(CurveVectorR curves, bool isFilled)
    {
    bool haveDisjoint = false, haveContinuous = false;
    if (!isFilled && CurveVector::BOUNDARY_TYPE_None == curves.GetBoundaryType())
        {
        for (auto const& prim : curves)
            {
            if (isDisjointCurvePrimitive(*prim))
                haveDisjoint = true;
            else
                haveContinuous = true;
            }
        }
    else
        {
        haveContinuous = true;
        }

    BeAssert(haveDisjoint || haveContinuous);
    if (haveDisjoint != haveContinuous)
        {
        // The typical case...
        AddCurveVector(curves, isFilled, haveDisjoint);
        return;
        }

    // ###TODO: Must split up disjoint and continuous into two separate curve vectors...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVector2d(CurveVectorCR curves, bool isFilled, double priority)
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
        _AddCurveVectorR(*cv, isFilled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVector2dR(CurveVectorR curves, bool isFilled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddCurveVectorR(*curves.Clone(), isFilled);
        }
    else
        {
        Transform tf = Transform::From(DPoint3d::FromXYZ(0.0, 0.0, zDepth));
        auto cv = curves.Clone(tf);
        _AddCurveVectorR(*cv, isFilled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurve(MSBsplineCurveCR bcurve, bool filled)
    {
    CurveVectorPtr cv = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurve(bcurve));
    m_accum.Add(*cv, filled, bcurve.params.closed ? GetMeshDisplayParams(filled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurveR(RefCountedMSBsplineCurveR bcurve, bool filled)
    {
    MSBsplineCurvePtr pBcurve(&bcurve);
    CurveVectorPtr cv = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurve(pBcurve));
    m_accum.Add(*cv, filled, bcurve.params.closed ? GetMeshDisplayParams(filled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurve2d(MSBsplineCurveCR bcurve, bool filled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddBSplineCurve(bcurve, filled);
        }
    else
        {
        MSBsplineCurvePtr bs = bcurve.CreateCopy();
        int nPoles = bs->GetNumPoles();
        DPoint3d* poles = bs->GetPoleP();
        for (int i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurveR(*bs, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurve2dR(RefCountedMSBsplineCurveR bcurve, bool filled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddBSplineCurveR(bcurve, filled);
        }
    else
        {
        int nPoles = bcurve.GetNumPoles();
        DPoint3d* poles = bcurve.GetPoleP();
        for (int i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurveR(bcurve, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineSurface(MSBsplineSurfaceCR surface)
    {
    _AddBSplineSurfaceR(*surface.Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineSurfaceR(RefCountedMSBsplineSurfaceR surf)
    {
    m_accum.Add(surf, GetMeshDisplayParams(false), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddDgnOle(DgnOleDraw* dgnOle)
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
        graphic = GetSystem()._CreateBranch(std::move(branch), GetDgnDb(), Transform::FromProduct(GetLocalToWorldTransform(), subToGf), clip);
        }

    if (graphic.IsValid())
        m_primitives.push_back(graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr PrimitiveBuilder::_CreateSubGraphic(TransformCR subToGf, ClipVectorCP clip) const
    {
    auto tf = subToGf.IsIdentity() ? GetLocalToWorldTransform() : Transform::FromIdentity();
    auto params = m_createParams.SubGraphic(tf);
    return GetSystem()._CreateGraphic(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr GeometryListBuilder::_Finish()
    {
    if (!IsOpen())
        {
        BeAssert(false);
        return nullptr;
        }

    GraphicPtr graphic = _FinishGraphic(m_accum);
    BeAssert(graphic.IsValid()); // callers of Finish() assume they're getting back a non-null graphic...

    m_isOpen = false;
    m_accum.Clear();
    return graphic;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/17
//=======================================================================================
struct PrimitiveBuilderContext : ViewContext
{
    System& m_system;

    explicit PrimitiveBuilderContext(PrimitiveBuilderR gf) : m_system(gf.GetSystem())
        {
        SetDgnDb(gf.GetDgnDb());
        Attach(gf.GetViewport(), DrawPurpose::NotSpecified);
        }

    SystemP _GetRenderSystem() const override { return &m_system; }
    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const&) override { BeAssert(false); return nullptr; }
    GraphicPtr _CreateBranch(GraphicBranch&, DgnDbR, TransformCR, ClipVectorCP) override { BeAssert(false); return nullptr; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr PrimitiveBuilder::_FinishGraphic(GeometryAccumulatorR accum)
    {
    if (!accum.IsEmpty())
        {
        PrimitiveBuilderContext context(*this);
        double tolerance = ComputeTolerance(accum);
        GeometryOptions options;
        accum.SaveToGraphicList(m_primitives, options, tolerance, context);
        }

    if (1 != m_primitives.size())
        return GetSystem()._CreateGraphicList(std::move(m_primitives), GetDgnDb());

    GraphicPtr graphic = *m_primitives.begin();
    m_primitives.clear();
    return graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
double PrimitiveBuilder::ComputeTolerance(GeometryAccumulatorR accum) const
    {
    constexpr double s_sizeToToleranceRatio = 0.25;
    double tolerance;

    auto const& params = GetCreateParams();
    if (params.IsViewCoordinates())
        {
        tolerance = 1.0;
        }
    else if (nullptr == params.GetViewport())
        {
        BeAssert(!accum.GetGeometries().ContainsCurves() && "No viewport supplied to GraphicBuilder::CreateParams - falling back to default coarse tolerance");
        tolerance = 20.0;
        }
    else
        {
        BeAssert(!accum.IsEmpty());
        DRange3d range = accum.GetGeometries().ComputeRange(); // NB: Already multiplied by transform...

        // NB: Geometry::CreateFacetOptions() will apply any scale factors from transform...no need to do it here.
        DPoint3d pt = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
        tolerance = params.GetViewport()->GetPixelSizeAtPoint(&pt);
        }

    return tolerance * s_sizeToToleranceRatio;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::ReInitialize(TransformCR localToWorld, TransformCR accumTf, DgnElementId elemId)
    {
    m_accum.ReInitialize(accumTf, elemId);
    ActivateGraphicParams(GraphicParams(), nullptr);
    m_createParams.SetPlacement(localToWorld);
    m_isOpen = true;

    _Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d GeometryList::ComputeRange() const
    {
    DRange3d range = DRange3d::NullRange();
    for (auto const& geom : *this)
        range.Extend(geom->GetTileRange());

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryList GeometryList::Slice(size_t startIdx, size_t endIdx) const
    {
    BeAssert(startIdx < endIdx && endIdx <= size());

    GeometryList list;
    list.m_complete = m_complete;
    list.m_curved = m_curved;
    list.m_list.assign(begin() + startIdx, begin() + endIdx);

    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void QVertex3dList::Requantize()
    {
    if (IsFullyQuantized())
        return;

    m_qpoints.Requantize(QPoint3d::Params(m_range));
    m_qpoints.reserve(size());

    for (auto const& fpt : m_fpoints)
        m_qpoints.Add(DPoint3d::From(fpt));

    m_fpoints.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void QVertex3dList::Init(DRange3dCR range, QPoint3dCP qPoints, size_t nPoints)
    {
    m_range = range;
    m_qpoints.resize(nPoints);
    m_qpoints.SetParams(QPoint3d::Params(range));
    memcpy (m_qpoints.data(), qPoints, nPoints * sizeof(QPoint3d));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TriangleList::AddTriangle(TriangleCR triangle)
    {
    uint8_t         flags = triangle.m_singleSided ? 1 : 0;

    for (size_t i=0; i<3; i++)
        {
        if (triangle.GetEdgeVisible(i))
            flags |= (0x0002 << i);
        
        m_indices.push_back(triangle.m_indices[i]);
        }
    m_flags.push_back(flags);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Triangle  TriangleList::GetTriangle(size_t index) const
    {
    if (index > m_flags.size())
        {
        BeAssert(false);
        return Triangle();
        }
    
    uint8_t         flags = m_flags.at(index);
    Triangle        triangle(0 != (flags & 0x0001));
    uint32_t const* pIndex = &m_indices.at(index*3);

    for (size_t i=0; i<3; i++)
        {
        triangle.m_indices[i] = pIndex[i];
        triangle.m_edgeFlags[i] = (0 == (flags & (0x0002 << i))) ? MeshEdge::Flags::Invisible : MeshEdge::Flags::Visible;
        }

    return triangle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams::DisplayParams(Type type, DgnCategoryId catId, DgnSubCategoryId subCatId, GradientSymbCP gradient, RenderMaterialId matId, ColorDef lineColor, ColorDef fillColor, uint32_t width, LinePixels linePixels, FillFlags fillFlags, DgnGeometryClass geomClass, bool ignoreLights, DgnDbR dgnDb, System& renderSys) :
            m_type(type), m_categoryId(catId), m_subCategoryId(subCatId), m_gradient(gradient), m_materialId(matId), m_lineColor(lineColor), m_fillColor(fillColor), m_width(width), m_linePixels(linePixels), m_fillFlags(fillFlags), m_class(geomClass), m_ignoreLighting(ignoreLights), m_resolved(false) 
    { 
    // TFS#726824...
    if (m_materialId.IsValid())
        m_material = renderSys._GetMaterial(m_materialId, dgnDb);

    Resolve (dgnDb, renderSys); 
    }
